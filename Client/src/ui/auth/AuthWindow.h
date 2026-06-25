#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include "../../network/MessengerClient.h"

class SettingsDialog; // <--- Прототип класса

class AuthWindow : public QWidget {
    Q_OBJECT 
public:
    explicit AuthWindow(MessengerClient *client, QWidget *parent = nullptr);

private:
    QLineEdit *loginInput;
    QLineEdit *passwordInput;
    QPushButton *loginButton;
    MessengerClient *client;

    // Не обязательно хранить указатель на settingsBtn в классе, если мы не используем его за пределами конструктора.
    // Но если нужно, добавим: QPushButton *settingsBtn;

private slots:
    void onLoginClicked();
    void handleAuthSuccess(); 
    void handleAuthError();
};