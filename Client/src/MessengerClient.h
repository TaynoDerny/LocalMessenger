#pragma once
#include <QObject>
#include <QTcpSocket>

class MessengerClient : public QObject {
    Q_OBJECT // Важно для работы сигналов
public:
    explicit MessengerClient(QObject *parent = nullptr);
    void connectToServer(const QString& ip, quint16 port);
    void sendAuthData(const QString& login, const QString& password);

private:
    QTcpSocket *socket;

private slots:
    void handleConnected();
    void handleReadyRead();
    void handleError(QAbstractSocket::SocketError socketError);
};