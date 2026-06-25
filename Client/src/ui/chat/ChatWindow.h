#pragma once
#include <QMainWindow> // <--- ИЗМЕНИЛИ НА QMAINWINDOW
#include <QListWidget>
#include <QScrollArea>
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

class ChatWindow : public QMainWindow { // <--- ИЗМЕНИЛИ НА QMAINWINDOW
    Q_OBJECT
public:
    explicit ChatWindow(MessengerClient *client, QWidget *parent = nullptr);

private:
    MessengerClient *client;
    QPushButton *adminPanelBtn;
    AdminPanelWidget *adminWidget; 

    QListWidget *chatsList;
    QStackedWidget *mainArea; 
    QWidget *homeWidget; 
    QWidget *chatWidget; 

    QLabel *chatHeader;
    
    QScrollArea *messagesArea;
    QWidget *messagesContainer;
    QVBoxLayout *messagesLayout;
    
    QLineEdit *messageInput;

    QString currentRecipient; 
    
    QHash<QString, QPixmap> userAvatars;
    QHash<QString, bool> userAdmins;

    QPixmap createCircularAvatarFromBase64(const QString& base64, int size, bool /*isOnline*/ = false);

    void addMessageToChat(const QString& sender, const QString& text);
    void clearChatMessages();

private slots:
    void onSendClicked();
    void onMessageReceived(const QString& sender, const QString& text); 
    void onUserListReceived(const QJsonArray& users); 
    void onChatSelected(QListWidgetItem *item);  
    void onHistoryReceived(const QString& chatWith, const QJsonArray& messages);
};