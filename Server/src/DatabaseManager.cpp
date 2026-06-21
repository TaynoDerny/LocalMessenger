#include <QSqlQuery>
#include <QSqlError>
#include <QCryptographicHash>
#include <QJsonObject>
#include "DatabaseManager.h"

DatabaseManager::DatabaseManager() {
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("messenger.sqlite");
    if (db.open()) {
        QSqlQuery query;
        query.exec("CREATE TABLE IF NOT EXISTS Users (id INTEGER PRIMARY KEY AUTOINCREMENT, login TEXT UNIQUE, password_hash TEXT, is_admin INTEGER)");
        
        // Сразу после создания таблицы Users добавь:
        query.exec("CREATE TABLE IF NOT EXISTS Messages (id INTEGER PRIMARY KEY AUTOINCREMENT, sender TEXT, recipient TEXT, message_text TEXT, timestamp DATETIME DEFAULT CURRENT_TIMESTAMP)");
        // Надежное хеширование с явным указанием кодировки
        QByteArray hashBytes = QCryptographicHash::hash(QString("1234").toUtf8(), QCryptographicHash::Sha256);
        QString defaultAdminHash = QString(hashBytes.toHex());
        
        qDebug() << "БАЗА: Хеш админа при создании:" << defaultAdminHash;

        query.prepare("INSERT OR IGNORE INTO Users (login, password_hash, is_admin) VALUES ('admin', :hash, 1)");
        query.bindValue(":hash", defaultAdminHash);
        
        if (!query.exec()) {
            qDebug() << "БАЗА: Ошибка создания админа:" << query.lastError().text();
        }
    } else {
        qDebug() << "БАЗА: Ошибка открытия файла БД!";
    }
}

bool DatabaseManager::checkUser(const QString& login, const QString& password) {
    QSqlQuery query;
    query.prepare("SELECT * FROM Users WHERE login = :login AND password_hash = :password");
    query.bindValue(":login", login);
    query.bindValue(":password", password); // Сюда уже прилетает хеш от клиента
    
    if (!query.exec()) {
        qDebug() << "БАЗА: Ошибка при поиске юзера:" << query.lastError().text();
        return false;
    }
    
    bool found = query.next();
    if (!found) {
        qDebug() << "БАЗА: Юзер не найден. Искали логин:" << login << "с хешем:" << password;
    }
    return found;
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


bool DatabaseManager::saveMessage(const QString& sender, const QString& recipient, const QString& text) {
    QSqlQuery query;
    query.prepare("INSERT INTO Messages (sender, recipient, message_text) VALUES (:sender, :recipient, :text)");
    query.bindValue(":sender", sender);
    query.bindValue(":recipient", recipient);
    query.bindValue(":text", text);
    
    if (!query.exec()) {
        qDebug() << "БАЗА: Ошибка сохранения сообщения:" << query.lastError().text();
        return false;
    }
    return true;
}

QJsonArray DatabaseManager::getChatHistory(const QString& user1, const QString& user2) {
    QJsonArray history;
    QSqlQuery query;
    
    // Ищем переписку между двумя людьми (либо 1->2, либо 2->1) и сортируем по времени (по id)
    query.prepare("SELECT sender, message_text FROM Messages WHERE (sender = :u1 AND recipient = :u2) OR (sender = :u2 AND recipient = :u1) ORDER BY id ASC");
    query.bindValue(":u1", user1);
    query.bindValue(":u2", user2);
    
    if (query.exec()) {
        while (query.next()) {
            QJsonObject msg;
            msg["sender"] = query.value(0).toString();
            msg["text"] = query.value(1).toString();
            history.append(msg);
        }
    } else {
        qDebug() << "БАЗА: Ошибка получения истории:" << query.lastError().text();
    }
    return history;
}