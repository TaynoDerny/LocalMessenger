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

    // ========== ГЕТТЕРЫ ДЛЯ НОВЫХ ПОЛЕЙ ==========
    QString getMyAvatarBase64() const { return myAvatarBase64; }
    QString getMyFirstName() const { return myFirstName; }
    QString getMyLastName() const { return myLastName; }
    QString getMyJobTitle() const { return myJobTitle; }
    QString getMyBio() const { return myBio; }
    // =============================================

    void createAccount(const QString& login, const QString& password, bool isAdmin);
    void requestAdminData();
    void sendResetPassword(const QString& targetLogin, const QString& newPassword);
    void sendWipeUser(const QString& targetLogin);
    
    // ОБНОВЛЕННЫЙ МЕТОД
    void updateProfile(const QString& firstName, const QString& lastName, const QString& jobTitle, const QString& bio, const QString& avatarBase64);

private:
    QTcpSocket *socket;
    QString myLogin; 
    bool adminStatus = false;
    
    QString myAvatarBase64;
    QString myFirstName;
    QString myLastName;
    QString myJobTitle;
    QString myBio;

private slots:
    void handleConnected();
    void handleReadyRead();
    void handleError(QAbstractSocket::SocketError socketError);

signals:
    void authSuccess();
    void authError();
    void messageReceived(const QString& sender, const QString& text); 
    void userListReceived(const QJsonArray& users);
    void historyReceived(const QString& chatWith, const QJsonArray& messages);
    void adminDataReceived(const QJsonArray& users);
};