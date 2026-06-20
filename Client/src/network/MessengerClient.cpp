#include "MessengerClient.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

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
    QJsonObject json;
    json["type"] = "auth";
    json["login"] = login;
    json["password"] = password;

    QJsonDocument doc(json);
    socket->write(doc.toJson(QJsonDocument::Compact));
}
// ОТПРАВКА ДАННЫХ РЕГЕСТРАЦИИ 
void MessengerClient::sendRegisterData(const QString& login, const QString& password) {
    QJsonObject json;
    json["type"] = "register";
    json["login"] = login;
    json["password"] = password;

    QJsonDocument doc(json);
    socket->write(doc.toJson(QJsonDocument::Compact));
}

void MessengerClient::handleConnected() {
    qDebug() << "КЛИЕНТ: Успешно подключено к серверу!";
    // закомментированно так как данные берутся с окна
    ///sendAuthData("admin", "1234");
}
void MessengerClient::handleReadyRead() {
    QByteArray data = socket->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (!doc.isNull() && doc.isObject()) {
        QString type = doc.object()["type"].toString();

        if (type == "auth_success") {
            qDebug() << "СЕРВЕР ОТВЕТИЛ: Доступ разрешен! Можно открывать окно чатов.";

            emit authSuccess();
            // Как только админ успешно зашел, он отправляет запрос на регистрацию нового юзера
           // sendRegisterData("user2", "5678"); 
            
            //sendMessage("Всем привет! Я в чате!");
        } else if (type == "auth_error") {
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
    } else {
        qDebug() << "КЛИЕНТ: Получены странные данные (не JSON):" << data;
    }
}

void MessengerClient::handleError(QAbstractSocket::SocketError socketError) {
    qDebug() << "КЛИЕНТ ОШИБКА:" << socket->errorString();
}

void MessengerClient::sendMessage(const QString& text) {
    QJsonObject json;
    json["type"] = "message";
    json["text"] = text;

    QJsonDocument doc(json);
    socket->write(doc.toJson(QJsonDocument::Compact));
}