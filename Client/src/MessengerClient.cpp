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

void MessengerClient::handleConnected() {
    qDebug() << "КЛИЕНТ: Успешно подключено к серверу!";
    // Для теста сразу после подключения отправляем логин и пароль
    sendAuthData("admin", "1234");
}

void MessengerClient::handleReadyRead() {
    QByteArray data = socket->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (!doc.isNull() && doc.isObject()) {
        QString type = doc.object()["type"].toString();

        if (type == "auth_success") {
            qDebug() << "СЕРВЕР ОТВЕТИЛ: Доступ разрешен! Можно открывать окно чатов.";
        } else if (type == "auth_error") {
            qDebug() << "СЕРВЕР ОТВЕТИЛ: Неверный логин или пароль!";
        }
    } else {
        qDebug() << "КЛИЕНТ: Получены странные данные (не JSON):" << data;
    }
}

void MessengerClient::handleError(QAbstractSocket::SocketError socketError) {
    qDebug() << "КЛИЕНТ ОШИБКА:" << socket->errorString();
}