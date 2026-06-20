#include "ChatWindow.h"

ChatWindow::ChatWindow(MessengerClient *client, QWidget *parent)
    : QWidget(parent), client(client) {

    setWindowTitle("Мессенджер");
    resize(800, 600); 

    QHBoxLayout *mainLayout = new QHBoxLayout(this);

    // ================= ЛЕВАЯ ПАНЕЛЬ =================
    QVBoxLayout *leftLayout = new QVBoxLayout();
    
    QLabel *leftHeader = new QLabel("Чаты и Группы", this);
    chatsList = new QListWidget(this);
    chatsList->setObjectName("chatsList");

    createGroupButton = new QPushButton("Создать группу", this);
    createGroupButton->setObjectName("createGroupButton");

    leftLayout->addWidget(leftHeader);
    leftLayout->addWidget(chatsList);
    leftLayout->addWidget(createGroupButton);

    // ================= ПРАВАЯ ПАНЕЛЬ =================
    QVBoxLayout *rightLayout = new QVBoxLayout();

    chatHeader = new QLabel("Выберите чат для начала общения", this);
    
    messagesDisplay = new QTextEdit(this);
    messagesDisplay->setReadOnly(true); 
    messagesDisplay->setObjectName("messagesDisplay");

    QHBoxLayout *inputLayout = new QHBoxLayout();
    messageInput = new QLineEdit(this);
    messageInput->setPlaceholderText("Напишите сообщение...");
    messageInput->setObjectName("messageInput");

    sendButton = new QPushButton("Отправить", this);
    sendButton->setObjectName("sendButton");

    inputLayout->addWidget(messageInput);
    inputLayout->addWidget(sendButton);

    rightLayout->addWidget(chatHeader);
    rightLayout->addWidget(messagesDisplay);
    rightLayout->addLayout(inputLayout); 

    // ================= СБОРКА ВСЕГО ОКНА =================
    mainLayout->addLayout(leftLayout, 1);
    mainLayout->addLayout(rightLayout, 3);

    setLayout(mainLayout);

    // === ТВОИ СТАРЫЕ КОННЕКТЫ ===
    connect(sendButton, &QPushButton::clicked, this, &ChatWindow::onSendClicked);
    connect(client, &MessengerClient::messageReceived, this, &ChatWindow::onMessageReceived);

    // === НОВЫЕ КОННЕКТЫ ДЛЯ ЛЕВОЙ ПАНЕЛИ (ЖДЕМ СПИСОК ОТ СЕРВЕРА И КЛИКАЕМ) ===
    connect(client, &MessengerClient::userListReceived, this, &ChatWindow::onUserListReceived);
    connect(chatsList, &QListWidget::itemClicked, this, &ChatWindow::onChatSelected);
}

// === ИЗМЕНЕННАЯ КНОПКА ОТПРАВКИ (ТЕПЕРЬ ШЛЕМ КОНКРЕТНОМУ ЮЗЕРУ) ===
void ChatWindow::onSendClicked() {
    QString text = messageInput->text();
    
    if (!text.isEmpty() && !currentRecipient.isEmpty()) {
        client->sendMessage(text, currentRecipient);
        
        // Убрали имя получателя из строки, так как мы и так в его чате
        QString displayString = QString("<b>[Я]:</b> %1").arg(text); 
        
        // 1. Сохраняем сообщение в историю конкретного чата
        chatHistories[currentRecipient].append(displayString);
        
        // 2. Сразу выводим его на экран
        messagesDisplay->append(displayString);
        
        messageInput->clear(); 
    } else if (currentRecipient.isEmpty()) {
        messagesDisplay->append("<i>Выберите чат слева, чтобы отправить сообщение!</i>");
    }
}

// === ТВОЙ СТАРЫЙ МЕТОД ПОЛУЧЕНИЯ СООБЩЕНИЙ ===
void ChatWindow::onMessageReceived(const QString& sender, const QString& text) {
    QString displayString = QString("<b>[%1]:</b> %2").arg(sender, text);
    
    // 1. Всегда сохраняем прилетевшее сообщение в память чата с этим отправителем
    chatHistories[sender].append(displayString);

    // 2. Если мы ПРЯМО СЕЙЧАС смотрим на чат с этим человеком — обновляем экран
    if (currentRecipient == sender) {
        messagesDisplay->append(displayString);
    }
}

// === ДВА АБСОЛЮТНО НОВЫХ МЕТОДА ДЛЯ СПИСКА СЛЕВА ===
void ChatWindow::onUserListReceived(const QStringList& users) {
    chatsList->clear(); // Очищаем список
    for (const QString& user : users) {
        chatsList->addItem(user); // Добавляем юзеров из сети
    }
}

void ChatWindow::onChatSelected(QListWidgetItem *item) {
    currentRecipient = item->text(); 
    chatHeader->setText("Чат с: " + currentRecipient); 
    
    // 1. Очищаем экран от старой переписки
    messagesDisplay->clear();
    
    // 2. Загружаем историю выбранного чата из памяти
    // Если мы кликнули по юзеру впервые, список будет пустым, и экран просто очистится
    for (const QString& msg : chatHistories[currentRecipient]) {
        messagesDisplay->append(msg);
    }
}