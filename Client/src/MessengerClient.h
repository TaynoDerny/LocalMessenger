#pragma once
#include <QObject>
#include <QTcpSocket>

class MessengerClient : public QObject {
    Q_OBJECT // Важно для работы сигналов
public:
    explicit MessengerClient(QObject *parent = nullptr);
    void connectToServer(const QString& ip, quint16 port);
    void sendAuthData(const QString& login, const QString& password);
    void sendRegisterData(const QString& login, const QString& password);
    void sendMessage(const QString& text);


private:
    QTcpSocket *socket;

private slots:
    void handleConnected();
    void handleReadyRead();
    void handleError(QAbstractSocket::SocketError socketError);
};