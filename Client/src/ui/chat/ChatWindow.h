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
#include "../admin/AdminPanelWidget.h"

class ChatWindow : public QWidget {
    Q_OBJECT
public:
    explicit ChatWindow(MessengerClient *client, QWidget *parent = nullptr);

private:
    MessengerClient *client;

    // --- 1 колонка
    QListWidget *serversList;
    QPushButton *createGroupButton; 

    QPushButton *adminPanelBtn;
    AdminPanelWidget *adminWidget; 

    // --- 2 колонка (Личные чаты)
    QListWidget *chatsList;

    // --- 3 колонка
    QStackedWidget *mainArea; 
    QWidget *homeWidget; 
    QWidget *chatWidget; 

    QLabel *chatHeader;
    QTextEdit *messagesDisplay;
    QLineEdit *messageInput;
    QPushButton *sendButton;

    QString currentRecipient; 
    QHash<QString, QStringList> chatHistories;

private slots:
    void onSendClicked();
    void onMessageReceived(const QString& sender, const QString& text); 
    void onUserListReceived(const QStringList& users); 
    void onChatSelected(QListWidgetItem *item);  
    void onHistoryReceived(const QString& chatWith, const QJsonArray& messages);
};