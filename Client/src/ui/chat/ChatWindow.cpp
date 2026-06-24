#include "ChatWindow.h"
#include "../settings/SettingsDialog.h"

ChatWindow::ChatWindow(MessengerClient *client, QWidget *parent)
    : QWidget(parent), client(client) {

    setAttribute(Qt::WA_DeleteOnClose);

    setWindowTitle("Мессенджер");
    resize(1000, 600);

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0); 
    mainLayout->setSpacing(0);

    // ================= 1. ЛЕВАЯ ПАНЕЛЬ =================
    QWidget *serversContainer = new QWidget(this);
    serversContainer->setFixedWidth(70);

    QVBoxLayout *serversLayout = new QVBoxLayout(serversContainer);
    serversLayout->setContentsMargins(5, 10, 5, 10);
    serversLayout->setSpacing(10);

    serversList = new QListWidget(this);
    serversList->setFrameShape(QFrame::NoFrame);
    serversList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    serversList->setObjectName("serversList");

    createGroupButton = new QPushButton("+", this);
    createGroupButton->setFixedSize(50, 50);
    createGroupButton->setStyleSheet(
        "QPushButton { border-radius: 25px; background-color: #36393f; color: #43b581; font-size: 28px; font-weight: bold; }"
        "QPushButton:hover { background-color: #43b581; color: white; }"
    );
    createGroupButton->setObjectName("createGroupButton");

    serversLayout->addWidget(serversList);
    serversLayout->addWidget(createGroupButton, 0, Qt::AlignHCenter); 

    adminPanelBtn = new QPushButton("⚙️", this); 
    adminPanelBtn->setFixedSize(50, 50);
    adminPanelBtn->setStyleSheet(
        "QPushButton { border-radius: 25px; background-color: #36393f; color: white; font-size: 24px; }"
        "QPushButton:hover { background-color: #7289da; }"
    );
    serversLayout->addWidget(adminPanelBtn, 0, Qt::AlignHCenter); 

    if (!client->isAdmin()) {
        adminPanelBtn->hide(); 
    }

    mainLayout->addWidget(serversContainer);

    // ================= 2. СРЕДНЯЯ ПАНЕЛЬ =================
    QWidget *friendsContainer = new QWidget(this);
    friendsContainer->setFixedWidth(200);

    QVBoxLayout *friendsLayout = new QVBoxLayout(friendsContainer);
    friendsLayout->setContentsMargins(0, 0, 0, 0);

    chatsList = new QListWidget(this);
    chatsList->setObjectName("chatsList");
    friendsLayout->addWidget(chatsList); // Кнопка "Создать пользователя" убрана!

    mainLayout->addWidget(friendsContainer);

    // ================= 3. ПРАВАЯ ПАНЕЛЬ =================
    QWidget *rightContainer = new QWidget(this);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightContainer);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    // --- ВЕРХНЯЯ ПАНЕЛЬ ---
    QWidget *topBar = new QWidget(this);
    topBar->setFixedHeight(48);
    topBar->setStyleSheet("background-color: #36393f; border-bottom: 1px solid #202225;");
    
    QHBoxLayout *topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(15, 0, 15, 0);
    topBarLayout->addStretch();

    QPushButton *settingsBtn = new QPushButton("⚙️", this);
    settingsBtn->setFixedSize(30, 30);
    settingsBtn->setStyleSheet(
        "QPushButton { background: transparent; color: #b9bbbe; font-size: 20px; border: none; }"
        "QPushButton:hover { color: white; }"
    );
    topBarLayout->addWidget(settingsBtn);

    connect(settingsBtn, &QPushButton::clicked, this, [this]() {
        QWidget *overlay = new QWidget(this);
        overlay->setStyleSheet("background-color: rgba(0, 0, 0, 170);");
        overlay->resize(this->size());
        overlay->show();

        SettingsDialog dialog(this);
        dialog.exec();

        delete overlay;
    });

    // --- РАБОЧАЯ ЗОНА ---
    mainArea = new QStackedWidget(this);

    homeWidget = new QWidget(this);
    QVBoxLayout *homeLayout = new QVBoxLayout(homeWidget);
    QLabel *welcomeLabel = new QLabel("Выберите чат или группу слева", this);
    welcomeLabel->setAlignment(Qt::AlignCenter);
    QFont font = welcomeLabel->font();
    font.setPointSize(14);
    welcomeLabel->setFont(font);
    homeLayout->addWidget(welcomeLabel);
    mainArea->addWidget(homeWidget);

    chatWidget = new QWidget(this);
    QVBoxLayout *chatLayout = new QVBoxLayout(chatWidget);

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

    chatLayout->addWidget(chatHeader);
    chatLayout->addWidget(messagesDisplay);
    chatLayout->addLayout(inputLayout); 
    
    mainArea->addWidget(chatWidget);

    adminWidget = new AdminPanelWidget(client, this);
    mainArea->addWidget(adminWidget);

    rightLayout->addWidget(topBar);
    rightLayout->addWidget(mainArea);

    mainLayout->addWidget(rightContainer);
    setLayout(mainLayout);
    
    mainArea->setCurrentIndex(0);

    // ================= КОННЕКТЫ =================
    connect(sendButton, &QPushButton::clicked, this, &ChatWindow::onSendClicked);
    connect(messageInput, &QLineEdit::returnPressed, this, &ChatWindow::onSendClicked);
    connect(client, &MessengerClient::messageReceived, this, &ChatWindow::onMessageReceived);
    connect(client, &MessengerClient::userListReceived, this, &ChatWindow::onUserListReceived);
    connect(chatsList, &QListWidget::itemClicked, this, &ChatWindow::onChatSelected);
    connect(client, &MessengerClient::historyReceived, this, &ChatWindow::onHistoryReceived);
    
    connect(adminPanelBtn, &QPushButton::clicked, this, [this, client]() {
        mainArea->setCurrentIndex(2);
        client->requestAdminData();
    });

    connect(client, &MessengerClient::adminDataReceived, adminWidget, &AdminPanelWidget::updateTable);
}

// ... Остальные методы (onSendClicked, onMessageReceived...) остаются без изменений. 
// Вставляйте их сюда как были из оригинального файла.
void ChatWindow::onSendClicked() {
    QString text = messageInput->text();
    
    if (!text.isEmpty() && !currentRecipient.isEmpty()) {
        client->sendMessage(text, currentRecipient);
        
        QString displayString = QString("<b>[Я]:</b> %1").arg(text); 
        
        chatHistories[currentRecipient].append(displayString);
        messagesDisplay->append(displayString);
        
        messageInput->clear(); 
    }
}

void ChatWindow::onMessageReceived(const QString& sender, const QString& text) {
    QString displayString = QString("<b>[%1]:</b> %2").arg(sender, text);
    
    chatHistories[sender].append(displayString);

    if (currentRecipient == sender) {
        messagesDisplay->append(displayString);
    }
}

void ChatWindow::onUserListReceived(const QStringList& users) {
    chatsList->clear(); 
    for (const QString& user : users) {
        chatsList->addItem(user); 
    }
}

void ChatWindow::onChatSelected(QListWidgetItem *item) {
    currentRecipient = item->text(); 
    chatHeader->setText("<b>Чат с:</b> " + currentRecipient); 
    
    mainArea->setCurrentIndex(1);
    messagesDisplay->clear(); 
    
    client->requestHistory(currentRecipient);
}

void ChatWindow::onHistoryReceived(const QString& chatWith, const QJsonArray& messages) {
    if (chatWith == currentRecipient) {
        messagesDisplay->clear(); 
        
        for (const QJsonValue& val : messages) {
            QJsonObject msgObj = val.toObject();
            QString sender = msgObj["sender"].toString();
            QString text = msgObj["text"].toString();
            
            QString displayString;
            if (sender == client->getMyLogin()) {
                displayString = QString("<b>[Я]:</b> %1").arg(text);
            } else {
                displayString = QString("<b>[%1]:</b> %2").arg(sender, text);
            }
            messagesDisplay->append(displayString);
        }
    }
}