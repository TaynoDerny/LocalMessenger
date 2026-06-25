#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QJsonArray>

// Класс сетевого клиента для взаимодействия с сервером мессенджера
class MessengerClient : public QObject {
    Q_OBJECT
public:
    // Конструктор: инициализация сокета и подключение сигналов
    explicit MessengerClient(QObject *parent = nullptr);
    
    // Установка соединения с сервером по IP и порту
    void connectToServer(const QString& ip, quint16 port);
    
    // Отправка данных для авторизации (пароль хешируется SHA256)
    void sendAuthData(const QString& login, const QString& password);
    
    // Отправка данных для регистрации нового пользователя
    void sendRegisterData(const QString& login, const QString& password);
    
    // Отправка текстового сообщения выбранному получателю
    void sendMessage(const QString& text, const QString& recipient);
    
    // Запрос истории сообщений с указанным пользователем
    void requestHistory(const QString& chatWith);
    
    // Геттер: возвращает логин текущего пользователя
    QString getMyLogin() const { return myLogin; }
    
    // Геттер: возвращает статус администратора (true/false)
    bool isAdmin() const { return adminStatus; }

    // Геттеры для данных профиля (кешируются при авторизации)
    QString getMyAvatarBase64() const { return myAvatarBase64; }
    QString getMyFirstName() const { return myFirstName; }
    QString getMyLastName() const { return myLastName; }
    QString getMyJobTitle() const { return myJobTitle; }
    QString getMyBio() const { return myBio; }

    // Админ-функция: создание нового пользователя
    void createAccount(const QString& login, const QString& password, bool isAdmin);
    
    // Админ-функция: запрос списка пользователей для админ-панели
    void requestAdminData();
    
    // Админ-функция: сброс пароля другому пользователю
    void sendResetPassword(const QString& targetLogin, const QString& newPassword);
    
    // Админ-функция: очистка истории сообщений другого пользователя
    void sendWipeUser(const QString& targetLogin);
    
    // Отправка обновленных данных профиля на сервер
    void updateProfile(const QString& firstName, const QString& lastName, const QString& jobTitle, const QString& bio, const QString& avatarBase64);

private:
    QTcpSocket *socket;     // Сокет для TCP-соединения с сервером
    QString myLogin;        // Логин текущего авторизованного пользователя
    bool adminStatus = false; // Флаг, является ли пользователь администратором
    
    // Кешированные данные профиля (загружаются при успешной авторизации)
    QString myAvatarBase64;
    QString myFirstName;
    QString myLastName;
    QString myJobTitle;
    QString myBio;

private slots:
    // Слот: вызывается при успешном подключении к серверу
    void handleConnected();
    
    // Слот: вызывается при поступлении данных от сервера (обработка JSON)
    void handleReadyRead();
    
    // Слот: вызывается при возникновении ошибки сокета
    void handleError(QAbstractSocket::SocketError socketError);

signals:
    // Сигнал: авторизация прошла успешно
    void authSuccess();
    
    // Сигнал: ошибка авторизации (неверный логин/пароль)
    void authError();
    
    // Сигнал: получено новое входящее сообщение
    void messageReceived(const QString& sender, const QString& text); 
    
    // Сигнал: обновлён список пользователей (для левой панели чата)
    void userListReceived(const QJsonArray& users);
    
    // Сигнал: получена история переписки с выбранным пользователем
    void historyReceived(const QString& chatWith, const QJsonArray& messages);
    
    // Сигнал: получены данные пользователей для отображения в админ-панели
    void adminDataReceived(const QJsonArray& users);
};