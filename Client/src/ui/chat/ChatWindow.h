#pragma once
#include <QWidget>
#include <QListWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QListWidgetItem> 
#include <QHash>
#include <QStringList>
#include <QStackedWidget>

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

#include "../../network/MessengerClient.h"

class ChatWindow : public QWidget {
    Q_OBJECT
public:
    explicit ChatWindow(MessengerClient *client, QWidget *parent = nullptr);

private:
    MessengerClient *client;

    // --- 1 колонка (в стиле боковой панели Дискорда для серверов/групп)
    QListWidget *serversList;
    QPushButton *createGroupButton; 

    // --- 2 колонка (Личные чаты)
    QListWidget *chatsList;

    // --- 3 колонка (Рабочая зона с переключением экранов)
    QStackedWidget *mainArea; 
    QWidget *homeWidget; // Экран-заглушка (когда никто не выбран)
    QWidget *chatWidget; // Экран самого чата

    // Элементы внутри экрана чата
    QLabel *chatHeader;
    QTextEdit *messagesDisplay;
    QLineEdit *messageInput;
    QPushButton *sendButton;

    QPushButton *createUserBtn; // <-- Новая кнопка создание пользователя

    QString currentRecipient; 

    // Ключ - логин собеседника, Значение - список сообщений
    QHash<QString, QStringList> chatHistories;

private slots:
    void onSendClicked();
    void onMessageReceived(const QString& sender, const QString& text); 
    void onUserListReceived(const QStringList& users); 
    void onChatSelected(QListWidgetItem *item);  

    void onHistoryReceived(const QString& chatWith, const QJsonArray& messages);
    


};