#pragma once
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>
#include <QJsonArray> // <-- Добавили

class DatabaseManager {
public:
    DatabaseManager();
    bool checkUser(const QString& login, const QString& password);
    bool registerUser(const QString& login, const QString& password);
    
    // Новые методы для истории
    bool saveMessage(const QString& sender, const QString& recipient, const QString& text);
    QJsonArray getChatHistory(const QString& user1, const QString& user2);

private:
    QSqlDatabase db;
};