#include "AuthWindow.h"

AuthWindow::AuthWindow(MessengerClient *client, QWidget *parent) 
    : QWidget(parent), client(client) {
    
    // Настройки самого окна
    setWindowTitle("Вход в Мессенджер");
    resize(300, 200);

    // 1. Создаем поля ввода
    loginInput = new QLineEdit(this);
    loginInput->setPlaceholderText("Логин");
    loginInput->setObjectName("loginField"); // Имя для будущего CSS

    passwordInput = new QLineEdit(this);
    passwordInput->setPlaceholderText("Пароль");
    passwordInput->setEchoMode(QLineEdit::Password); // Скрываем символы звездочками
    passwordInput->setObjectName("passwordField"); // Имя для будущего CSS

    // 2. Создаем кнопку
    loginButton = new QPushButton("Войти", this);
    loginButton->setObjectName("loginButton"); // Имя для будущего CSS

    // 3. Создаем вертикальный слой и закидываем туда наши виджеты
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(loginInput);
    layout->addWidget(passwordInput);
    layout->addWidget(loginButton);

    // Устанавливаем этот слой основным для окна
    setLayout(layout);

    // 4. Связываем клик по кнопке с нашей функцией
    connect(loginButton, &QPushButton::clicked, this, &AuthWindow::onLoginClicked);
}

void AuthWindow::onLoginClicked() {
    // Забираем текст из полей
    QString login = loginInput->text();
    QString password = passwordInput->text();

    // Просим нашего клиента отправить эти данные на сервер!
    client->sendAuthData(login, password);
}