#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QJsonArray>

class MessengerClient : public QObject {
    Q_OBJECT
public:
    explicit MessengerClient(QObject *parent = nullptr);
    void connectToServer(const QString& ip, quint16 port);
    void sendAuthData(const QString& login, const QString& password);
    void sendRegisterData(const QString& login, const QString& password);
    void sendMessage(const QString& text, const QString& recipient);
    void requestHistory(const QString& chatWith);
    QString getMyLogin() const { return myLogin; }
    bool isAdmin() const { return adminStatus; }

    void createAccount(const QString& login, const QString& password, bool isAdmin);
    void requestAdminData();
    void sendResetPassword(const QString& targetLogin, const QString& newPassword);
    void sendWipeUser(const QString& targetLogin);
    void updateProfile(const QString& newName, const QString& avatarBase64);

private:
    QTcpSocket *socket;
    QString myLogin; 
    bool adminStatus = false;

private slots:
    void handleConnected();
    void handleReadyRead();
    void handleError(QAbstractSocket::SocketError socketError);

signals:
    void authSuccess();
    void authError();
    void messageReceived(const QString& sender, const QString& text); 
    void userListReceived(const QJsonArray& users); // <--- ИЗМЕНЕНО ТУТ
    void historyReceived(const QString& chatWith, const QJsonArray& messages);
    void adminDataReceived(const QJsonArray& users);
};