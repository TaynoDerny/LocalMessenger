#include "MessengerServer.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

MessengerServer::MessengerServer(QObject *parent) : QObject(parent) {
    server = new QTcpServer(this);
    // Как только сервер ловит новое подключение, вызываем функцию handleNewConnection
    connect(server, &QTcpServer::newConnection, this, &MessengerServer::handleNewConnection);
}

void MessengerServer::start() {
    if (!server->listen(QHostAddress::Any, 8080)) {
        qDebug() << "Ошибка запуска сервера:" << server->errorString();
    } else {
        qDebug() << "Сервер слушает порт 8080...";
    }
}

void MessengerServer::handleNewConnection() {
    QTcpSocket *clientSocket = server->nextPendingConnection();
    qDebug() << "СЕРВЕР: Новый клиент подключился!";

    // Привязываем чтение и отключение
    connect(clientSocket, &QTcpSocket::readyRead, this, &MessengerServer::handleReadyRead);
    connect(clientSocket, &QTcpSocket::disconnected, this, &MessengerServer::handleDisconnect);
}

void MessengerServer::handleReadyRead() {
    // Получаем сокет, который прислал данные
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!clientSocket) return;

    QByteArray data = clientSocket->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (!doc.isNull() && doc.isObject()) {
        QJsonObject json = doc.object();
        
        if (json["type"].toString() == "auth") {
            QString login = json["login"].toString();
            QString password = json["password"].toString();

            // Проверяем через наш DatabaseManager!
            if (dbManager.checkUser(login, password)) {
                qDebug() << "СЕРВЕР: Пользователь" << login << "авторизован.";
                QJsonObject response;
                response["type"] = "auth_success";
                clientSocket->write(QJsonDocument(response).toJson(QJsonDocument::Compact));
            } else {
                qDebug() << "СЕРВЕР: Ошибка авторизации.";
                QJsonObject response;
                response["type"] = "auth_error";
                clientSocket->write(QJsonDocument(response).toJson(QJsonDocument::Compact));
            }
        }
    }
}

void MessengerServer::handleDisconnect() {
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!clientSocket) return;
    
    qDebug() << "СЕРВЕР: Клиент отключился.";
    clientSocket->deleteLater();
}