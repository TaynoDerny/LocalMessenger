#include "AuthWindow.h"
#include "../chat/ChatWindow.h"
#include "../settings/SettingsDialog.h"
#include <QSettings>
#include <QHBoxLayout>

// Конструктор: инициализация окна, загрузка сетевых настроек и построение интерфейса
AuthWindow::AuthWindow(MessengerClient *client, QWidget *parent) 
    : QWidget(parent), client(client) {
    
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("Вход в Мессенджер");
    resize(400, 350);

    // Загрузка сохранённых IP и порта сервера, затем подключение
    QSettings settings;
    QString ip = settings.value("network/server_ip", "127.0.0.1").toString();
    int port = settings.value("network/server_port", 8080).toInt();
    client->connectToServer(ip, port);

    // Поля ввода логина и пароля
    loginInput = new QLineEdit(this);
    loginInput->setPlaceholderText("Логин");
    loginInput->setFixedHeight(40);

    passwordInput = new QLineEdit(this);
    passwordInput->setPlaceholderText("Пароль");
    passwordInput->setEchoMode(QLineEdit::Password);
    passwordInput->setFixedHeight(40);

    loginButton = new QPushButton("Войти", this);
    loginButton->setFixedHeight(40);
    loginButton->setCursor(Qt::PointingHandCursor);
    loginButton->setStyleSheet(
        "QPushButton { background-color: #5865F2; border-radius: 4px; color: white; font-weight: bold; }"
        "QPushButton:hover { background-color: #4752C4; }"
    );

    QFrame *loginFrame = new QFrame(this);
    loginFrame->setStyleSheet("QFrame { background-color: #2b2d31; border-radius: 12px; }");
    
    QVBoxLayout *frameLayout = new QVBoxLayout(loginFrame);
    frameLayout->setContentsMargins(30, 15, 30, 30); 
    frameLayout->setSpacing(15);

    // Верхняя часть с заголовком и кнопкой настроек
    QHBoxLayout *headerLayout = new QHBoxLayout();
    
    QLabel *title = new QLabel("Local Messenger", loginFrame);
    title->setStyleSheet("font-size: 24px; font-weight: bold; color: #5865F2;");
    title->setAlignment(Qt::AlignCenter);
    
    // Кнопка открытия настроек (только вкладка сети)
    QPushButton *settingsBtn = new QPushButton(loginFrame);
    settingsBtn->setIcon(QIcon(":/images/settings_icon.png"));
    settingsBtn->setIconSize(QSize(20, 20));
    settingsBtn->setFixedSize(32, 32);
    settingsBtn->setStyleSheet("QPushButton { background: transparent; border: none; }");
    
    connect(settingsBtn, &QPushButton::clicked, this, [this]() {
        SettingsDialog dialog(this->client, this, true);
        dialog.exec();
    });

    // Заголовок растягивается на всё свободное место, кнопка прижимается к правому краю
    headerLayout->addWidget(title, 1, Qt::AlignCenter);
    headerLayout->addWidget(settingsBtn, 0, Qt::AlignRight | Qt::AlignVCenter);
    
    frameLayout->addLayout(headerLayout);

    // Основной контейнер и верстка полей ввода и кнопки
    frameLayout->addWidget(loginInput);
    frameLayout->addWidget(passwordInput);
    frameLayout->addWidget(loginButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(loginFrame);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    setLayout(mainLayout);

    connect(loginButton, &QPushButton::clicked, this, &AuthWindow::onLoginClicked);
    connect(client, &MessengerClient::authSuccess, this, &AuthWindow::handleAuthSuccess);
    connect(client, &MessengerClient::authError, this, &AuthWindow::handleAuthError);
}

// Обработка нажатия кнопки "Войти": отправка данных авторизации на сервер
void AuthWindow::onLoginClicked() {
    QString login = loginInput->text();
    QString password = passwordInput->text();
    client->sendAuthData(login, password);
}

// Успешная авторизация: открытие окна чата и закрытие окна входа
void AuthWindow::handleAuthSuccess() {
    qDebug() << "ОКНО: Логин прошел, открываем чат...";
    ChatWindow *chatWindow = new ChatWindow(client);
    chatWindow->show(); 
    this->close(); 
}

// Ошибка авторизации: вывод отладочного сообщения
void AuthWindow::handleAuthError() {
    qDebug() << "ОКНО: Ошибка входа!";
}