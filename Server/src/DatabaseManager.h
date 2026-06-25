#pragma once
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>
#include <QJsonArray> 
#include <QJsonObject>

// Класс для управления базой данных SQLite (пользователи, сообщения и профили)
class DatabaseManager {
public:
    DatabaseManager();
    
    // Проверка авторизации пользователя по логину и хешу пароля
    bool checkUser(const QString& login, const QString& password);

    // Проверка, является ли пользователь администратором
    bool isAdmin(const QString& login);

    // Регистрация нового пользователя в системе
    bool registerUser(const QString& login, const QString& password, bool isAdmin, const QString& displayName = "", const QString& avatarBase64 = "");

    // Сохранение нового отправленного сообщения в историю
    bool saveMessage(const QString& sender, const QString& recipient, const QString& text);

    // Получение истории чата между двумя пользователями
    QJsonArray getChatHistory(const QString& user1, const QString& user2);

    // Получение полных данных одного пользователя по его логину
    QJsonObject getUserInfo(const QString& login);

    // Получение списка всех зарегистрированных пользователей с их данными
    QJsonArray getAllUsersInfo();

    // Обновление всех данных профиля пользователя (имя, фамилия, должность, био, аватар)
    bool updateUserProfile(const QString& login, const QString& firstName, const QString& lastName, const QString& jobTitle, const QString& bio, const QString& avatarBase64);

    // Сброс пароля указанного пользователя (замена хеша)
    bool resetUserPassword(const QString& targetLogin, const QString& newPasswordHash);

    // Полное удаление всей истории сообщений указанного пользователя
    bool wipeUserData(const QString& targetLogin);

private:
    QSqlDatabase db; // Подключение к базе данных SQLite
};