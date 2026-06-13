#pragma once
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHash>
#include "DatabaseManager.h"


class MessengerServer : public QObject {
    Q_OBJECT // Макрос, чтобы работали сигналы
public:
    explicit MessengerServer(QObject *parent = nullptr);
    void start();

private:
    QTcpServer *server;
    DatabaseManager dbManager; // Наш класс для работы с SQLite
    QHash<QTcpSocket*, QString> clients; // указатель на сокет -> Имя пользователя (Логин) подключенных клиентов 

private slots:
    void handleNewConnection();
    void handleReadyRead();
    void handleDisconnect();
};