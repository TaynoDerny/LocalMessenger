#include "AuthWindow.h"
#include "../chat/ChatWindow.h"
#include "../settings/SettingsDialog.h"
#include <QSettings>
#include <QHBoxLayout>

AuthWindow::AuthWindow(MessengerClient *client, QWidget *parent) 
    : QWidget(parent), client(client) {
    
    setAttribute(Qt::WA_DeleteOnClose);

    setWindowTitle("Вход в Мессенджер");
    resize(400, 350);

    // Загрузка настроек и подключение
    QSettings settings;
    QString ip = settings.value("network/server_ip", "127.0.0.1").toString();
    int port = settings.value("network/server_port", 8080).toInt();
    client->connectToServer(ip, port);

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
    frameLayout->setContentsMargins(30, 30, 30, 30);
    frameLayout->setSpacing(15);

    // ========== ИСПРАВЛЕНИЕ: Идеальное выравнивание заголовка и иконки ==========
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QLabel *title = new QLabel("Local Messenger", loginFrame);
    title->setStyleSheet("font-size: 24px; font-weight: bold; color: #5865F2;");
    title->setAlignment(Qt::AlignCenter);
    
    // 1. Растяжка слева
    headerLayout->addStretch(1);
    // 2. Заголовок по центру
    headerLayout->addWidget(title, 2, Qt::AlignCenter);
    // 3. Растяжка справа (с отступом, чтобы дать место иконке)
    headerLayout->addStretch(3);
    
    QPushButton *settingsBtn = new QPushButton(loginFrame);
    settingsBtn->setIcon(QIcon(":/images/settings_icon.png"));
    settingsBtn->setIconSize(QSize(20, 20));
    settingsBtn->setFixedSize(32, 32);
    // ИСПРАВЛЕНИЕ ОШИБКИ STYLESHEET: background: transparent вызывал краш. 
    // Используем setFlat(true) для прозрачности.
    settingsBtn->setFlat(true);
    
    connect(settingsBtn, &QPushButton::clicked, this, [this]() {
        SettingsDialog dialog(this->client, this, true); // true = только сеть
        dialog.exec();
    });
    // 4. Добавляем иконку (прижимаем её к центру/правой части с небольшим отступом)
    headerLayout->addWidget(settingsBtn, 0, Qt::AlignRight | Qt::AlignVCenter);
    
    frameLayout->addLayout(headerLayout);
    // ============================================================================

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

void AuthWindow::onLoginClicked() {
    QString login = loginInput->text();
    QString password = passwordInput->text();
    client->sendAuthData(login, password);
}

void AuthWindow::handleAuthSuccess() {
    qDebug() << "ОКНО: Логин прошел, открываем чат...";
    ChatWindow *chatWindow = new ChatWindow(client);
    chatWindow->show(); 
    this->close(); 
}

void AuthWindow::handleAuthError() {
    qDebug() << "ОКНО: Ошибка входа!";
}