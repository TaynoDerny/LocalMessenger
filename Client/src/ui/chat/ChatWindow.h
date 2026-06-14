#pragma once
#include <QWidget>
#include <QListWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "../../network/MessengerClient.h"

class ChatWindow : public QWidget {
    Q_OBJECT
public:
    explicit ChatWindow(MessengerClient *client, QWidget *parent = nullptr);

private:
    MessengerClient *client;

    // Левая панель (в стиле боковой панели Дискорда)
    QListWidget *chatsList;
    QPushButton *createGroupButton; // Кнопка, которую мы потом спрячем от не-админов

    // Правая панель (зона самого чата)
    QLabel *chatHeader;
    QTextEdit *messagesDisplay;
    QLineEdit *messageInput;
    QPushButton *sendButton;

private slots:
    void onSendClicked();
};