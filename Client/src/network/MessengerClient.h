#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QStringList> // Добавь этот инклуд наверх

class MessengerClient : public QObject {
    Q_OBJECT // Важно для работы сигналов
public:
    explicit MessengerClient(QObject *parent = nullptr);
    void connectToServer(const QString& ip, quint16 port);
    void sendAuthData(const QString& login, const QString& password);
    void sendRegisterData(const QString& login, const QString& password);
    // Обновляем отправку сообщения: теперь нужен текст и получатель
    void sendMessage(const QString& text, const QString& recipient);


private:
    QTcpSocket *socket;

private slots:
    void handleConnected();
    void handleReadyRead();
    void handleError(QAbstractSocket::SocketError socketError);

signals: // <-- ДОБАВЛЯЕМ СЕКЦИЮ СИГНАЛОВ
    void authSuccess();
    void authError();
    void messageReceived(const QString& sender, const QString& text); 
    void userListReceived(const QStringList& users);
};