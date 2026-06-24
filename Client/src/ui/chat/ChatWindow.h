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
#include <QStackedWidget>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>

#include "../../network/MessengerClient.h"
#include "../admin/AdminPanelWidget.h"

class ChatWindow : public QWidget {
    Q_OBJECT
public:
    explicit ChatWindow(MessengerClient *client, QWidget *parent = nullptr);

private:
    MessengerClient *client;

    QListWidget *serversList;
    QPushButton *createGroupButton; 
    QPushButton *adminPanelBtn;
    AdminPanelWidget *adminWidget; 

    QListWidget *chatsList;
    QStackedWidget *mainArea; 
    QWidget *homeWidget; 
    QWidget *chatWidget; 

    QLabel *chatHeader;
    QTextEdit *messagesDisplay;
    QLineEdit *messageInput;
    QPushButton *sendButton;

    QString currentRecipient; 
    QHash<QString, QStringList> chatHistories;

    // Обновленная функция с параметром isOnline
    QPixmap createCircularAvatarFromBase64(const QString& base64, int size, bool isOnline);

private slots:
    void onSendClicked();
    void onMessageReceived(const QString& sender, const QString& text); 
    void onUserListReceived(const QJsonArray& users); 
    void onChatSelected(QListWidgetItem *item);  
    void onHistoryReceived(const QString& chatWith, const QJsonArray& messages);
};