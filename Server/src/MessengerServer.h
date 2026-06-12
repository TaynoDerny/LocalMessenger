#pragma once
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include "DatabaseManager.h"

class MessengerServer : public QObject {
    Q_OBJECT // Макрос, чтобы работали сигналы
public:
    explicit MessengerServer(QObject *parent = nullptr);
    void start();

private:
    QTcpServer *server;
    DatabaseManager dbManager; // Наш класс для работы с SQLite

private slots:
    void handleNewConnection();
    void handleReadyRead();
    void handleDisconnect();
};