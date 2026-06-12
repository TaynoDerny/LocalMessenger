#pragma once
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>

class DatabaseManager {
public:
    DatabaseManager();
    bool checkUser(const QString& login, const QString& password);
private:
    QSqlDatabase db;
};