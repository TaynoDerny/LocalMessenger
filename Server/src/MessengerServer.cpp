#include "MessengerServer.h"
#include <QJsonDocument>
#include <QDebug>
#include <QHostAddress>

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
    clients[clientSocket] = ""; // Пока не авторизован, логин пустой

    connect(clientSocket, &QTcpSocket::readyRead, this, &MessengerServer::handleReadyRead);
    connect(clientSocket, &QTcpSocket::disconnected, this, &MessengerServer::handleDisconnect);
}

// ================== ИЗМЕНЕННАЯ РАССЫЛКА ==================
void MessengerServer::broadcastUserList() {
    // 1. Берем всех юзеров из БД
    QJsonArray dbUsers = dbManager.getAllUsersInfo();
    QJsonArray finalUserList;

    // 2. Пробегаемся по каждому и добавляем онлайн-статус
    for (const QJsonValue& val : dbUsers) {
        QJsonObject u = val.toObject();
        QString login = u["login"].toString();
        
        // Проверяем, есть ли этот логин в списке подключенных клиентов
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

    // Рассылаем всем авторизованным
    for (QTcpSocket *client : clients.keys()) {
        if (!clients[client].isEmpty()) {
            client->write(responseData);
        }
    }
}

void MessengerServer::handleReadyRead() {
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!clientSocket) return;

    while (clientSocket->canReadLine()) {
        QByteArray data = clientSocket->readLine();
        QJsonDocument doc = QJsonDocument::fromJson(data);

        if (!doc.isNull() && doc.isObject()) {
            QJsonObject json = doc.object();
            
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
                    
                    // ОТПРАВЛЯЕМ КЛИЕНТУ ЕГО ДАННЫЕ ПРИ ВХОДЕ
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
            
            // ========== НОВАЯ ЛОГИКА: СОХРАНЕНИЕ ПРОФИЛЯ ==========
            else if (json["type"].toString() == "update_profile") {
                QString senderName = clients.value(clientSocket);
                if (senderName.isEmpty()) return;

                QString newName = json["new_name"].toString();
                QString newAvatar = json["avatar_base64"].toString();

                qDebug() << "СЕРВЕР: Обновляем профиль для" << senderName << "имя:" << newName;
                
                if (dbManager.updateUserProfile(senderName, newName, newAvatar)) {
                    // ОБНОВЛЯЕМ СПИСОК ПОЛЬЗОВАТЕЛЕЙ У ВСЕХ КЛИЕНТОВ
                    broadcastUserList();
                }
            }
        }
    }
}

void MessengerServer::handleDisconnect() {
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!clientSocket) return;
    
    bool wasLoggedIn = !clients[clientSocket].isEmpty();
    clients.remove(clientSocket); 
    clientSocket->deleteLater();

    if (wasLoggedIn) {
        broadcastUserList(); // Обновляем список, так как кто-то вышел
    }
}