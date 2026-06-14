#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include "../../network/MessengerClient.h"

class AuthWindow : public QWidget {
    Q_OBJECT 
public:
    // Передаем указатель на нашего клиента, чтобы окно могло общаться с сервером
    explicit AuthWindow(MessengerClient *client, QWidget *parent = nullptr);

private:
    QLineEdit *loginInput;
    QLineEdit *passwordInput;
    QPushButton *loginButton;
    MessengerClient *client; // Ссылка на наш движок

private slots:
    void onLoginClicked(); // Функция, которая сработает при нажатии кнопки
    void handleAuthSuccess(); 
    void handleAuthError();
};