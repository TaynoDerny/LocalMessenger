#include <QSqlQuery>
#include <QSqlError>
#include "DatabaseManager.h"

DatabaseManager::DatabaseManager() {
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("messenger.sqlite");
    if (db.open()) {
        QSqlQuery query;
        query.exec("CREATE TABLE IF NOT EXISTS Users (id INTEGER PRIMARY KEY AUTOINCREMENT, login TEXT UNIQUE, password_hash TEXT, is_admin INTEGER)");
        query.exec("INSERT OR IGNORE INTO Users (login, password_hash, is_admin) VALUES ('admin', '1234', 1)");
    }
}

bool DatabaseManager::checkUser(const QString& login, const QString& password) {
    QSqlQuery query;
    query.prepare("SELECT * FROM Users WHERE login = :login AND password_hash = :password");
    query.bindValue(":login", login);
    query.bindValue(":password", password);
    return query.exec() && query.next();
}

bool DatabaseManager::registerUser(const QString& login, const QString& password) {
    QSqlQuery query;
    // Используем правильные названия колонок
    query.prepare("INSERT INTO Users (login, password_hash, is_admin) VALUES (:login, :password, 0)");
    query.bindValue(":login", login);
    query.bindValue(":password", password);

    if (!query.exec()) {
        qDebug() << "БАЗА ДАННЫХ: Ошибка регистрации пользователя:" << query.lastError().text();
        return false; 
    }
    
    return true;
}