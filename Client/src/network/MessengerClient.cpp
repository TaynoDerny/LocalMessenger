#include "MessengerClient.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>  
#include <QJsonValue>
#include <QDebug>
#include <QCryptographicHash>

MessengerClient::MessengerClient(QObject *parent) : QObject(parent) {
    socket = new QTcpSocket(this);

    connect(socket, &QTcpSocket::connected, this, &MessengerClient::handleConnected);
    connect(socket, &QTcpSocket::readyRead, this, &MessengerClient::handleReadyRead);
    connect(socket, &QTcpSocket::errorOccurred, this, &MessengerClient::handleError);
}

void MessengerClient::connectToServer(const QString& ip, quint16 port) {
    socket->connectToHost(ip, port);
}

void MessengerClient::sendAuthData(const QString& login, const QString& password) {

    this->myLogin = login; // <-- ЗАПОМИНАЕМ, КТО МЫ
    // Надежное хеширование
    QByteArray hashBytes = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    QString hash = QString(hashBytes.toHex());

    qDebug() << "КЛИЕНТ: Отправляем логин:" << login << "и хеш:" << hash;

    QJsonObject json;
    json["type"] = "auth";
    json["login"] = login;
    json["password"] = hash; 

    QJsonDocument doc(json);
    socket->write(doc.toJson(QJsonDocument::Compact) + "\n");
}

// ОТПРАВКА ДАННЫХ РЕГЕСТРАЦИИ 
void MessengerClient::sendRegisterData(const QString& login, const QString& password) {
    // Здесь делаем то же самое
    QString hash = QString(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());

    QJsonObject json;
    json["type"] = "register";
    json["login"] = login;
    json["password"] = hash; 

    QJsonDocument doc(json);
    socket->write(doc.toJson(QJsonDocument::Compact) + "\n");
}

void MessengerClient::handleConnected() {
    qDebug() << "КЛИЕНТ: Успешно подключено к серверу!";
}

void MessengerClient::handleReadyRead() {
    // ЧИТАЕМ СТРОГО ПО ОДНОЙ СТРОКЕ, РАЗДЕЛЕННОЙ \n
    while (socket->canReadLine()) {
        QByteArray data = socket->readLine();
        QJsonDocument doc = QJsonDocument::fromJson(data);

        if (!doc.isNull() && doc.isObject()) {
            QString type = doc.object()["type"].toString();

            if (type == "auth_success") {
                qDebug() << "СЕРВЕР ОТВЕТИЛ: Доступ разрешен! Можно открывать окно чатов.";
                emit authSuccess();
            } 
            else if (type == "auth_error") {
                qDebug() << "СЕРВЕР ОТВЕТИЛ: Неверный логин или пароль!";
                emit authError();
            } 
            else if (type == "register_success") {
                qDebug() << "КЛИЕНТ: Регистрация прошла успешно! Теперь можно входить.";
            } 
            else if (type == "register_error") {
                qDebug() << "КЛИЕНТ: Ошибка регистрации! Возможно, такое имя уже занято.";
            }
            else if (type == "message") {
                QString sender = doc.object()["sender"].toString(); // Достаем имя
                QString text = doc.object()["text"].toString();
                
                // Отправляем данные прямиком в интерфейс
                emit messageReceived(sender, text);
            }
            else if (type == "user_list") {
                QJsonArray usersArray = doc.object()["users"].toArray();
                QStringList users;
                for (const QJsonValue &val : usersArray) {
                    users.append(val.toString()); // Достаем никнеймы из JSON
                }
                emit userListReceived(users); // Передаем список в интерфейс
            }
            else if (type == "history") {
                QString withUser = doc.object()["with"].toString();
                QJsonArray messages = doc.object()["messages"].toArray();
                emit historyReceived(withUser, messages); // Передаем в окно чата
            }

        } else {
            qDebug() << "КЛИЕНТ: Получены странные данные (не JSON):" << data;
        }
    }
}

void MessengerClient::handleError(QAbstractSocket::SocketError socketError) {
    qDebug() << "КЛИЕНТ ОШИБКА:" << socket->errorString();
}

void MessengerClient::sendMessage(const QString& text, const QString& recipient) {
    if (socket->state() == QAbstractSocket::ConnectedState) {
        QJsonObject json;
        json["type"] = "message";
        json["text"] = text;
        json["recipient"] = recipient; // <-- Кому отправляем

        QJsonDocument doc(json);
        // ЗДЕСЬ \n УЖЕ БЫЛ, ОСТАВЛЯЕМ КАК ЕСТЬ
        socket->write(doc.toJson(QJsonDocument::Compact) + "\n");
    }
}

void MessengerClient::requestHistory(const QString& chatWith) {
    QJsonObject json;
    json["type"] = "get_history";
    json["with"] = chatWith;
    QJsonDocument doc(json);
    socket->write(doc.toJson(QJsonDocument::Compact) + "\n");
}