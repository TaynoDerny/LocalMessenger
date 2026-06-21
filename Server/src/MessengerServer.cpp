#include "MessengerServer.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

MessengerServer::MessengerServer(QObject *parent) : QObject(parent) {
    server = new QTcpServer(this);
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
    
    clients[clientSocket] = ""; 

    connect(clientSocket, &QTcpSocket::readyRead, this, &MessengerServer::handleReadyRead);
    connect(clientSocket, &QTcpSocket::disconnected, this, &MessengerServer::handleDisconnect);
}

void MessengerServer::broadcastUserList() {
    QJsonArray usersArray;
    
    for (const QString &login : clients.values()) {
        if (!login.isEmpty()) {
            usersArray.append(login);
        }
    }

    QJsonObject response;
    response["type"] = "user_list";
    response["users"] = usersArray;

    // ДОБАВИЛИ \n ДЛЯ РАЗДЕЛЕНИЯ ПАКЕТОВ
    QByteArray responseData = QJsonDocument(response).toJson(QJsonDocument::Compact) + "\n";

    for (QTcpSocket *client : clients.keys()) {
        if (!clients[client].isEmpty()) {
            client->write(responseData);
        }
    }
}

void MessengerServer::handleReadyRead() {
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!clientSocket) return;

    // ЧИТАЕМ СТРОГО ПО ОДНОЙ СТРОКЕ
    while (clientSocket->canReadLine()) {
        QByteArray data = clientSocket->readLine();
        QJsonDocument doc = QJsonDocument::fromJson(data);

        if (!doc.isNull() && doc.isObject()) {
            QJsonObject json = doc.object();
            
            if (json["type"].toString() == "auth") {
                QString login = json["login"].toString();
                QString password = json["password"].toString();

                if (dbManager.checkUser(login, password)) {
                    qDebug() << "СЕРВЕР: Пользователь" << login << "авторизован.";
                    clients[clientSocket] = login; 

                    QJsonObject response;
                    response["type"] = "auth_success";
                    // ДОБАВИЛИ \n
                    clientSocket->write(QJsonDocument(response).toJson(QJsonDocument::Compact) + "\n");

                    broadcastUserList(); 
                } else {
                    qDebug() << "СЕРВЕР: Ошибка авторизации.";
                    QJsonObject response;
                    response["type"] = "auth_error";
                    clientSocket->write(QJsonDocument(response).toJson(QJsonDocument::Compact) + "\n");
                }
            }
            else if (json["type"].toString() == "register") {
                QString login = json["login"].toString();
                QString password = json["password"].toString();

                QJsonObject response;
                if (dbManager.registerUser(login, password)) {
                    qDebug() << "СЕРВЕР: Зарегистрирован:" << login;
                    response["type"] = "register_success";
                } else {
                    qDebug() << "СЕРВЕР: Ошибка регистрации:" << login;
                    response["type"] = "register_error";
                }
                clientSocket->write(QJsonDocument(response).toJson(QJsonDocument::Compact) + "\n");
            }
            else if (json["type"].toString() == "message") {
                QString text = json["text"].toString();
                QString recipient = json["recipient"].toString(); 
                QString senderName = clients.value(clientSocket, "Аноним"); 

                qDebug() << "СЕРВЕР: Сообщение от" << senderName << "для" << recipient;

                // <-- ДОБАВЛЕНО: Сохраняем в базу!
                dbManager.saveMessage(senderName, recipient, text); 

                QJsonObject response;
                response["type"] = "message";
                response["sender"] = senderName; 
                response["text"] = text;
                // ДОБАВИЛИ \n
                QByteArray responseData = QJsonDocument(response).toJson(QJsonDocument::Compact) + "\n";

                for (auto it = clients.begin(); it != clients.end(); ++it) {
                    if (it.value() == recipient) {
                        it.key()->write(responseData);
                        break;
                    }
                }
            }
            else if (json["type"].toString() == "get_history") {
                QString withUser = json["with"].toString();
                QString currentUser = clients.value(clientSocket);
                
                QJsonArray history = dbManager.getChatHistory(currentUser, withUser);
                
                QJsonObject response;
                response["type"] = "history";
                response["with"] = withUser;
                response["messages"] = history;
                
                clientSocket->write(QJsonDocument(response).toJson(QJsonDocument::Compact) + "\n");
            }
        }
    }
}

void MessengerServer::handleDisconnect() {
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!clientSocket) return;
    
    qDebug() << "СЕРВЕР: Клиент отключился.";
    
    bool wasLoggedIn = !clients[clientSocket].isEmpty();
    
    clients.remove(clientSocket); 
    clientSocket->deleteLater();

    if (wasLoggedIn) {
        broadcastUserList();
    }
}