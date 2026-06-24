#pragma once
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHash>
#include <QJsonObject>
#include <QJsonArray>
#include "DatabaseManager.h"

class MessengerServer : public QObject {
    Q_OBJECT 
public:
    explicit MessengerServer(QObject *parent = nullptr);
    void start();

private:
    QTcpServer *server;
    DatabaseManager dbManager; 
    QHash<QTcpSocket*, QString> clients; 

    // Новая функция: рассылает список пользователей с их данными
    void broadcastUserList(); 

private slots:
    void handleNewConnection();
    void handleReadyRead();
    void handleDisconnect();
};