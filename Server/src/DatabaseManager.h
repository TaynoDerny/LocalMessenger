#pragma once
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>
#include <QJsonArray> 
#include <QJsonObject>

class DatabaseManager {
public:
    DatabaseManager();
    
    // Авторизация и админ
    bool checkUser(const QString& login, const QString& password);
    bool isAdmin(const QString& login);

    // Регистрация и создание пользователя
    bool registerUser(const QString& login, const QString& password, bool isAdmin, const QString& displayName = "", const QString& avatarBase64 = "");

    // Сообщения
    bool saveMessage(const QString& sender, const QString& recipient, const QString& text);
    QJsonArray getChatHistory(const QString& user1, const QString& user2);

    // Работа с профилями пользователей
    QJsonObject getUserInfo(const QString& login); // Данные конкретного юзера
    QJsonArray getAllUsersInfo(); // Список всех юзеров с их данными
    
    // ========== ИСПРАВЛЕНИЕ: Обновлённая сигнатура с 6 аргументами ==========
    bool updateUserProfile(const QString& login, const QString& firstName, const QString& lastName, const QString& jobTitle, const QString& bio, const QString& avatarBase64);
    // ========================================================================

    // Админ функции
    bool resetUserPassword(const QString& targetLogin, const QString& newPasswordHash);
    bool wipeUserData(const QString& targetLogin);

private:
    QSqlDatabase db;
};