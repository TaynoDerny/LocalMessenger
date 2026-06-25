#include "MessengerServer.h"
#include <QJsonDocument>
#include <QDebug>
#include <QHostAddress>

// Конструктор: инициализация сервера и обработка новых подключений
MessengerServer::MessengerServer(QObject *parent) : QObject(parent) {
    server = new QTcpServer(this);
    connect(server, &QTcpServer::newConnection, this, &MessengerServer::handleNewConnection);
}

// Запуск сервера на порту 8080
void MessengerServer::start() {
    if (!server->listen(QHostAddress::Any, 8080)) {
        qDebug() << "Ошибка запуска сервера:" << server->errorString();
    } else {
        qDebug() << "Сервер слушает порт 8080...";
    }
}

// Обработка нового подключения от клиента
void MessengerServer::handleNewConnection() {
    QTcpSocket *clientSocket = server->nextPendingConnection();
    qDebug() << "СЕРВЕР: Новый клиент подключился!";
    clients[clientSocket] = "";

    connect(clientSocket, &QTcpSocket::readyRead, this, &MessengerServer::handleReadyRead);
    connect(clientSocket, &QTcpSocket::disconnected, this, &MessengerServer::handleDisconnect);
}

// Формирование и отправка обновлённого списка пользователей всем авторизованным клиентам
void MessengerServer::broadcastUserList() {
    QJsonArray dbUsers = dbManager.getAllUsersInfo();
    QJsonArray finalUserList;

    for (const QJsonValue& val : dbUsers) {
        QJsonObject u = val.toObject();
        QString login = u["login"].toString();
        
        bool isOnline = false;
        for (const QString& clientLogin : clients.values()) {
            if (clientLogin == login) {
                isOnline = true;
                break;
            }
        }
        u["online"] = isOnline;
        finalUserList.append(u);
    }

    QJsonObject response;
    response["type"] = "user_list";
    response["users"] = finalUserList;

    QByteArray responseData = QJsonDocument(response).toJson(QJsonDocument::Compact) + "\n";

    for (QTcpSocket *client : clients.keys()) {
        if (!clients[client].isEmpty()) {
            client->write(responseData);
        }
    }
}

// Обработка входящих JSON-пакетов от клиентов
void MessengerServer::handleReadyRead() {
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!clientSocket) return;

    while (clientSocket->canReadLine()) {
        QByteArray data = clientSocket->readLine();
        QJsonDocument doc = QJsonDocument::fromJson(data);

        if (!doc.isNull() && doc.isObject()) {
            QJsonObject json = doc.object();
            
            // Разбор типа запроса и вызов соответствующих методов
            if (json["type"].toString() == "auth") {
                QString login = json["login"].toString();
                QString password = json["password"].toString();

                if (dbManager.checkUser(login, password)) {
                    qDebug() << "СЕРВЕР: Авторизован" << login;
                    clients[clientSocket] = login; 

                    bool isUserAdmin = dbManager.isAdmin(login);
                    
                    QJsonObject response;
                    response["type"] = "auth_success";
                    response["is_admin"] = isUserAdmin;
                    
                    QJsonObject myInfo = dbManager.getUserInfo(login);
                    response["my_info"] = myInfo;

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
                if (dbManager.registerUser(login, password, false)) {
                    response["type"] = "register_success";
                } else {
                    response["type"] = "register_error";
                }
                clientSocket->write(QJsonDocument(response).toJson(QJsonDocument::Compact) + "\n");
            }
            else if (json["type"].toString() == "message") {
                QString text = json["text"].toString();
                QString recipient = json["recipient"].toString(); 
                QString senderName = clients.value(clientSocket, "Аноним"); 

                dbManager.saveMessage(senderName, recipient, text); 

                QJsonObject response;
                response["type"] = "message";
                response["sender"] = senderName; 
                response["text"] = text;
                
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
            else if (json["type"].toString() == "create_user") {
                QString senderName = clients.value(clientSocket);
                if (!dbManager.isAdmin(senderName)) return;

                QString newLogin = json["login"].toString();
                QString newPassword = json["password"].toString();
                bool isAdmin = json["is_admin"].toBool();

                if (dbManager.registerUser(newLogin, newPassword, isAdmin)) {
                    broadcastUserList();
                }
            }
            else if (json["type"].toString() == "get_admin_data") {
                QString senderName = clients.value(clientSocket);
                if (!dbManager.isAdmin(senderName)) return;

                QJsonArray dbUsers = dbManager.getAllUsersInfo();
                QJsonArray finalUsers;
                for (const QJsonValue& val : dbUsers) {
                    QJsonObject u = val.toObject();
                    u["online"] = false;
                    u["ip"] = "Не в сети";
                    for (auto it = clients.begin(); it != clients.end(); ++it) {
                        if (it.value() == u["login"].toString()) {
                            u["online"] = true;
                            QString ip = it.key()->peerAddress().toString();
                            if (ip.startsWith("::ffff:")) {
                                ip = ip.mid(7);
                            }
                            u["ip"] = ip;
                            break;
                        }
                    }
                    finalUsers.append(u);
                }
                QJsonObject response;
                response["type"] = "admin_data_result";
                response["users"] = finalUsers;
                clientSocket->write(QJsonDocument(response).toJson(QJsonDocument::Compact) + "\n");
            }
            else if (json["type"].toString() == "reset_password") {
                QString senderName = clients.value(clientSocket);
                if (!dbManager.isAdmin(senderName)) return;

                QString targetUser = json["target_user"].toString();
                QString newHash = json["new_password_hash"].toString();
                dbManager.resetUserPassword(targetUser, newHash);
            }
            else if (json["type"].toString() == "wipe_user") {
                QString senderName = clients.value(clientSocket);
                if (!dbManager.isAdmin(senderName)) return;

                QString targetUser = json["target_user"].toString();
                if (dbManager.wipeUserData(targetUser)) {
                    for (auto it = clients.begin(); it != clients.end(); ++it) {
                        if (it.value() == targetUser) {
                            it.key()->disconnectFromHost(); 
                            break;
                        }
                    }
                }
            }
            // Обработка обновления профиля пользователя
            else if (json["type"].toString() == "update_profile") {
                QString senderName = clients.value(clientSocket);
                if (senderName.isEmpty()) return;

                QString firstName = json["first_name"].toString();
                QString lastName = json["last_name"].toString();
                QString jobTitle = json["job_title"].toString();
                QString bio = json["bio"].toString();
                QString newAvatar = json["avatar_base64"].toString();

                qDebug() << "СЕРВЕР: Обновляем профиль для" << senderName;
                
                if (dbManager.updateUserProfile(senderName, firstName, lastName, jobTitle, bio, newAvatar)) {
                    broadcastUserList();
                }
            }
        }
    }
}

// Обработка отключения клиента и обновление списка пользователей
void MessengerServer::handleDisconnect() {
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!clientSocket) return;
    
    bool wasLoggedIn = !clients[clientSocket].isEmpty();
    clients.remove(clientSocket); 
    clientSocket->deleteLater();

    if (wasLoggedIn) {
        broadcastUserList();
    }
}