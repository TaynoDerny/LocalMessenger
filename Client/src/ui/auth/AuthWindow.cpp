#include "AuthWindow.h"
#include "../chat/ChatWindow.h"

AuthWindow::AuthWindow(MessengerClient *client, QWidget *parent) 
    : QWidget(parent), client(client) {
    
    setAttribute(Qt::WA_DeleteOnClose);

    setWindowTitle("Вход в Мессенджер");
    resize(400, 300);

    // Элементы
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

    // Создаем плашку с контентом
    QFrame *loginFrame = new QFrame(this);
    loginFrame->setStyleSheet("QFrame { background-color: #2b2d31; border-radius: 12px; }");
    
    QVBoxLayout *frameLayout = new QVBoxLayout(loginFrame);
    frameLayout->setContentsMargins(30, 30, 30, 30);
    frameLayout->setSpacing(15);
    
    QLabel *title = new QLabel("Local Messenger", loginFrame);
    title->setStyleSheet("font-size: 24px; font-weight: bold; color: #5865F2; margin-bottom: 20px;");
    title->setAlignment(Qt::AlignCenter);

    frameLayout->addWidget(title);
    frameLayout->addWidget(loginInput);
    frameLayout->addWidget(passwordInput);
    frameLayout->addWidget(loginButton);

    // Центрируем плашку в главном окне
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