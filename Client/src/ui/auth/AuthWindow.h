#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include "../../network/MessengerClient.h"

class SettingsDialog; // Предварительное объявление для использования в коде

// Окно авторизации для входа в мессенджер
class AuthWindow : public QWidget {
    Q_OBJECT 
public:
    // Конструктор: инициализация клиента и построение интерфейса
    explicit AuthWindow(MessengerClient *client, QWidget *parent = nullptr);

private:
    QLineEdit *loginInput;      // Поле ввода логина
    QLineEdit *passwordInput;   // Поле ввода пароля (скрытое)
    QPushButton *loginButton;   // Кнопка отправки запроса авторизации
    MessengerClient *client;    // Клиент для связи с сервером

private slots:
    // Обработка нажатия кнопки "Войти": отправка данных на сервер
    void onLoginClicked();
    
    // Успешная авторизация: открытие окна чата и закрытие окна входа
    void handleAuthSuccess(); 
    
    // Ошибка авторизации: вывод отладочного сообщения
    void handleAuthError();
};