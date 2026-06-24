#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QStringList> // Добавь этот инклуд наверх
#include <QJsonArray>

class MessengerClient : public QObject {
    Q_OBJECT // Важно для работы сигналов
public:
    explicit MessengerClient(QObject *parent = nullptr);
    void connectToServer(const QString& ip, quint16 port);
    void sendAuthData(const QString& login, const QString& password);
    void sendRegisterData(const QString& login, const QString& password);
    // Обновляем отправку сообщения: теперь нужен текст и получатель
    void sendMessage(const QString& text, const QString& recipient);


    void requestHistory(const QString& chatWith); // Запросить историю
    QString getMyLogin() const { return myLogin; } // Узнать свой логин
    bool isAdmin() const { return adminStatus; } //  Узнать свой статус админа

    void createAccount(const QString& login, const QString& password, bool isAdmin);

    void requestAdminData();
    void sendResetPassword(const QString& targetLogin, const QString& newPassword);
    void sendWipeUser(const QString& targetLogin);

        // Добавить в публичные методы:
    void updateProfile(const QString& newName, const QString& avatarBase64);

private:
    QTcpSocket *socket;
    QString myLogin; 
    bool adminStatus = false; //  Переменная для хранения статуса

private slots:
    void handleConnected();
    void handleReadyRead();
    void handleError(QAbstractSocket::SocketError socketError);

signals: // <-- ДОБАВЛЯЕМ СЕКЦИЮ СИГНАЛОВ
    void authSuccess();
    void authError();
    void messageReceived(const QString& sender, const QString& text); 
    void userListReceived(const QStringList& users);
    void historyReceived(const QString& chatWith, const QJsonArray& messages); // Сигнал для интерфейса

    void adminDataReceived(const QJsonArray& users);
};