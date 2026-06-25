#include "MessengerClient.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>  
#include <QDebug>
#include <QCryptographicHash>

// Конструктор: инициализация сокета и подключение его сигналов к слотам
MessengerClient::MessengerClient(QObject *parent) : QObject(parent) {
    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::connected, this, &MessengerClient::handleConnected);
    connect(socket, &QTcpSocket::readyRead, this, &MessengerClient::handleReadyRead);
    connect(socket, &QTcpSocket::errorOccurred, this, &MessengerClient::handleError);
}

// Подключение к серверу с принудительным сбросом, если уже было активное соединение
void MessengerClient::connectToServer(const QString& ip, quint16 port) {
    if (socket->state() == QAbstractSocket::ConnectingState || 
        socket->state() == QAbstractSocket::ConnectedState) {
        socket->abort();
        qDebug() << "КЛИЕНТ: Принудительно разорвано старое соединение";
    }
    socket->connectToHost(ip, port);
}

// Формирование JSON-запроса для авторизации (пароль хешируется SHA256)
void MessengerClient::sendAuthData(const QString& login, const QString& password) {
    this->myLogin = login;
    QByteArray hashBytes = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    QString hash = QString(hashBytes.toHex());

    QJsonObject json;
    json["type"] = "auth";
    json["login"] = login;
    json["password"] = hash; 
    socket->write(QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n");
}

// Формирование JSON-запроса для регистрации нового пользователя
void MessengerClient::sendRegisterData(const QString& login, const QString& password) {
    QString hash = QString(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());
    
    QJsonObject json;
    json["type"] = "register";
    json["login"] = login;
    json["password"] = hash; 
    socket->write(QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n");
}

// Отправка всех данных профиля (имя, фамилия, должность, био, аватар) на сервер
void MessengerClient::updateProfile(const QString& firstName, const QString& lastName, const QString& jobTitle, const QString& bio, const QString& avatarBase64) {
    if (socket->state() == QAbstractSocket::ConnectedState) {
        QJsonObject json;
        json["type"] = "update_profile";
        json["first_name"] = firstName;
        json["last_name"] = lastName;
        json["job_title"] = jobTitle;
        json["bio"] = bio;
        json["avatar_base64"] = avatarBase64;
        socket->write(QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n");
        qDebug() << "КЛИЕНТ: Отправляем обновление профиля на сервер";
    }
}

// Слот успешного подключения к серверу
void MessengerClient::handleConnected() {
    qDebug() << "КЛИЕНТ: Успешно подключено к серверу!";
}

// Слот обработки входящих JSON-пакетов от сервера
void MessengerClient::handleReadyRead() {
    while (socket->canReadLine()) {
        QByteArray data = socket->readLine();
        QJsonDocument doc = QJsonDocument::fromJson(data);

        if (!doc.isNull() && doc.isObject()) {
            QString type = doc.object()["type"].toString();

            if (type == "auth_success") {
                // Авторизация успешна: запоминаем права админа и данные своего профиля
                adminStatus = doc.object()["is_admin"].toBool();
                if (doc.object().contains("my_info")) {
                    QJsonObject myInfo = doc.object()["my_info"].toObject();
                    myAvatarBase64 = myInfo["avatar_base64"].toString();
                    myFirstName = myInfo["first_name"].toString();
                    myLastName = myInfo["last_name"].toString();
                    myJobTitle = myInfo["job_title"].toString();
                    myBio = myInfo["bio"].toString();
                }
                qDebug() << "СЕРВЕР ОТВЕТИЛ: Доступ разрешен! Админ ли мы:" << adminStatus;
                emit authSuccess();
            }
            else if (type == "auth_error") {
                qDebug() << "СЕРВЕР ОТВЕТИЛ: Неверный логин или пароль!";
                emit authError();
            } 
            else if (type == "register_success") {
                qDebug() << "КЛИЕНТ: Регистрация прошла успешно!";
            } 
            else if (type == "register_error") {
                qDebug() << "КЛИЕНТ: Ошибка регистрации!";
            }
            else if (type == "message") {
                // Пришло новое сообщение от другого пользователя
                QString sender = doc.object()["sender"].toString();
                QString text = doc.object()["text"].toString();
                emit messageReceived(sender, text);
            }
            else if (type == "user_list") {
                // Обновлённый список пользователей (для левой панели)
                QJsonArray usersArray = doc.object()["users"].toArray();
                emit userListReceived(usersArray);
            }
            else if (type == "history") {
                // История переписки с выбранным пользователем
                QString withUser = doc.object()["with"].toString();
                QJsonArray messages = doc.object()["messages"].toArray();
                emit historyReceived(withUser, messages);
            }
            else if (type == "admin_data_result") {
                // Список всех пользователей для админ-панели
                QJsonArray users = doc.object()["users"].toArray();
                emit adminDataReceived(users);
            }
        } else {
            qDebug() << "КЛИЕНТ: Получены странные данные (не JSON):" << data;
        }
    }
}

// Слот обработки ошибок сокета (например, обрыв соединения)
void MessengerClient::handleError(QAbstractSocket::SocketError socketError) {
    qDebug() << "КЛИЕНТ ОШИБКА:" << socket->errorString();
}

// Формирование JSON и отправка сообщения другому пользователю
void MessengerClient::sendMessage(const QString& text, const QString& recipient) {
    if (socket->state() == QAbstractSocket::ConnectedState) {
        QJsonObject json;
        json["type"] = "message";
        json["text"] = text;
        json["recipient"] = recipient;
        socket->write(QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n");
    }
}

// Запрос истории сообщений с указанным пользователем
void MessengerClient::requestHistory(const QString& chatWith) {
    QJsonObject json;
    json["type"] = "get_history";
    json["with"] = chatWith;
    socket->write(QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n");
}

// Админ-функция: создание нового пользователя
void MessengerClient::createAccount(const QString& login, const QString& password, bool isAdmin) {
    QByteArray hashBytes = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    QString hash = QString(hashBytes.toHex());
    
    QJsonObject json;
    json["type"] = "create_user";
    json["login"] = login;
    json["password"] = hash;
    json["is_admin"] = isAdmin;
    socket->write(QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n");
}

// Админ-функция: запрос данных всех пользователей для админ-панели
void MessengerClient::requestAdminData() {
    QJsonObject json;
    json["type"] = "get_admin_data";
    socket->write(QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n");
}

// Админ-функция: сброс пароля другому пользователю
void MessengerClient::sendResetPassword(const QString& targetLogin, const QString& newPassword) {
    QString hash = QString(QCryptographicHash::hash(newPassword.toUtf8(), QCryptographicHash::Sha256).toHex());
    
    QJsonObject json;
    json["type"] = "reset_password";
    json["target_user"] = targetLogin;
    json["new_password_hash"] = hash;
    socket->write(QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n");
}

// Админ-функция: полная очистка истории сообщений другого пользователя
void MessengerClient::sendWipeUser(const QString& targetLogin) {
    QJsonObject json;
    json["type"] = "wipe_user";
    json["target_user"] = targetLogin;
    socket->write(QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n");
}