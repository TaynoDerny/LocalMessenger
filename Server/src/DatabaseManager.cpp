#include "DatabaseManager.h"
#include <QSqlError>
#include <QCryptographicHash>

DatabaseManager::DatabaseManager() {
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("messenger.sqlite");
    if (db.open()) {
        QSqlQuery query;
        // Создаем таблицу Users, если нет
        query.exec("CREATE TABLE IF NOT EXISTS Users (id INTEGER PRIMARY KEY AUTOINCREMENT, login TEXT UNIQUE, password_hash TEXT, is_admin INTEGER)");
        
        // Создаем таблицу Messages, если нет
        query.exec("CREATE TABLE IF NOT EXISTS Messages (id INTEGER PRIMARY KEY AUTOINCREMENT, sender TEXT, recipient TEXT, message_text TEXT, timestamp DATETIME DEFAULT CURRENT_TIMESTAMP)");

        // ДОБАВЛЯЕМ НОВЫЕ СТОЛБЦЫ, ЕСЛИ ИХ НЕТ (ALTER TABLE)
        // Добавляем display_name (если нет)
        if (!query.exec("SELECT display_name FROM Users LIMIT 1")) {
            query.exec("ALTER TABLE Users ADD COLUMN display_name TEXT DEFAULT ''");
        }
        // Добавляем avatar_base64 (если нет)
        if (!query.exec("SELECT avatar_base64 FROM Users LIMIT 1")) {
            query.exec("ALTER TABLE Users ADD COLUMN avatar_base64 TEXT DEFAULT ''");
        }

        // Создаем админа по умолчанию (если его нет)
        QByteArray hashBytes = QCryptographicHash::hash(QString("1234").toUtf8(), QCryptographicHash::Sha256);
        QString defaultAdminHash = QString(hashBytes.toHex());
        
        query.prepare("INSERT OR IGNORE INTO Users (login, password_hash, is_admin, display_name, avatar_base64) VALUES ('admin', :hash, 1, 'Админ', '')");
        query.bindValue(":hash", defaultAdminHash);
        query.exec();
    } else {
        qDebug() << "БАЗА: Ошибка открытия файла БД!";
    }
}

bool DatabaseManager::checkUser(const QString& login, const QString& password) {
    QSqlQuery query;
    query.prepare("SELECT * FROM Users WHERE login = :login AND password_hash = :password");
    query.bindValue(":login", login);
    query.bindValue(":password", password);
    return query.exec() && query.next();
}

bool DatabaseManager::isAdmin(const QString& login) {
    QSqlQuery query;
    query.prepare("SELECT is_admin FROM Users WHERE login = :login");
    query.bindValue(":login", login);
    if (query.exec() && query.next()) {
        return query.value(0).toInt() == 1;
    }
    return false;
}

bool DatabaseManager::registerUser(const QString& login, const QString& password, bool isAdmin, const QString& displayName, const QString& avatarBase64) {
    QSqlQuery query;
    // Используем переданные displayName и avatarBase64, либо ставим дефолт (пустую строку)
    QString name = displayName.isEmpty() ? login : displayName;
    
    query.prepare("INSERT INTO Users (login, password_hash, is_admin, display_name, avatar_base64) VALUES (:login, :password, :is_admin, :name, :avatar)");
    query.bindValue(":login", login);
    query.bindValue(":password", password);
    query.bindValue(":is_admin", isAdmin ? 1 : 0);
    query.bindValue(":name", name);
    query.bindValue(":avatar", avatarBase64);

    if (!query.exec()) {
        qDebug() << "БАЗА: Ошибка регистрации:" << query.lastError().text();
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
    return query.exec();
}

QJsonArray DatabaseManager::getChatHistory(const QString& user1, const QString& user2) {
    QJsonArray history;
    QSqlQuery query;
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
    }
    return history;
}

// НОВЫЙ МЕТОД: Получить данные одного юзера
QJsonObject DatabaseManager::getUserInfo(const QString& login) {
    QJsonObject userObj;
    QSqlQuery query;
    query.prepare("SELECT login, display_name, avatar_base64, is_admin FROM Users WHERE login = :login");
    query.bindValue(":login", login);
    
    if (query.exec() && query.next()) {
        userObj["login"] = query.value(0).toString();
        userObj["display_name"] = query.value(1).toString();
        userObj["avatar_base64"] = query.value(2).toString();
        userObj["is_admin"] = query.value(3).toInt() == 1;
    }
    return userObj;
}

// НОВЫЙ МЕТОД: Получить данные ВСЕХ юзеров из БД
QJsonArray DatabaseManager::getAllUsersInfo() {
    QJsonArray usersArray;
    QSqlQuery query("SELECT login, display_name, avatar_base64, is_admin FROM Users");
    
    while (query.next()) {
        QJsonObject userObj;
        userObj["login"] = query.value(0).toString();
        userObj["display_name"] = query.value(1).toString();
        userObj["avatar_base64"] = query.value(2).toString();
        userObj["is_admin"] = query.value(3).toInt() == 1;
        usersArray.append(userObj);
    }
    return usersArray;
}

// НОВЫЙ МЕТОД: Обновить профиль
bool DatabaseManager::updateUserProfile(const QString& login, const QString& displayName, const QString& avatarBase64) {
    QSqlQuery query;
    query.prepare("UPDATE Users SET display_name = :name, avatar_base64 = :avatar WHERE login = :login");
    query.bindValue(":name", displayName);
    query.bindValue(":avatar", avatarBase64);
    query.bindValue(":login", login);
    return query.exec();
}

bool DatabaseManager::resetUserPassword(const QString& targetLogin, const QString& newPasswordHash) {
    QSqlQuery query;
    query.prepare("UPDATE Users SET password_hash = :hash WHERE login = :login");
    query.bindValue(":hash", newPasswordHash);
    query.bindValue(":login", targetLogin);
    return query.exec();
}

bool DatabaseManager::wipeUserData(const QString& targetLogin) {
    QSqlQuery query;
    query.prepare("DELETE FROM Messages WHERE sender = :login OR recipient = :login");
    query.bindValue(":login", targetLogin);
    return query.exec();
}