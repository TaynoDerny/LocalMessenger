#pragma once
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHash>
#include <QJsonArray> // <-- Добавили для сборки списка пользователей
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

    // <-- Новая функция для рассылки списка кто онлайн
    void broadcastUserList(); 

private slots:
    void handleNewConnection();
    void handleReadyRead();
    void handleDisconnect();
};