#pragma once
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHash>
#include <QJsonObject>
#include <QJsonArray>
#include "DatabaseManager.h"

// Класс сервера для обработки клиентских подключений и запросов
class MessengerServer : public QObject {
    Q_OBJECT 
public:
    // Конструктор: инициализация сервера и подключение сигнала новых подключений
    explicit MessengerServer(QObject *parent = nullptr);
    
    // Запуск сервера на порту 8080 и начало прослушивания
    void start();

private:
    QTcpServer *server;                     // Серверный сокет для приёма входящих соединений
    DatabaseManager dbManager;              // Менеджер базы данных для работы с пользователями и сообщениями
    QHash<QTcpSocket*, QString> clients;    // Хеш-таблица для связи сокета клиента с его логином

    // Рассылка обновлённого списка пользователей всем авторизованным клиентам
    void broadcastUserList(); 

private slots:
    // Обработка нового входящего подключения
    void handleNewConnection();
    
    // Обработка входящих данных от клиента (парсинг JSON-запросов)
    void handleReadyRead();
    
    // Обработка отключения клиента и обновление списка пользователей
    void handleDisconnect();
};