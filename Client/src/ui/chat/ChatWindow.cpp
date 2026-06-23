#include "ChatWindow.h"
#include "../admin/CreateUserDialog.h"

ChatWindow::ChatWindow(MessengerClient *client, QWidget *parent)
    : QWidget(parent), client(client) {

    setAttribute(Qt::WA_DeleteOnClose);

    setWindowTitle("Мессенджер");
    resize(1000, 600); // Сделали чуть шире под три колонки

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0); // Убираем отступы окна для красоты
    mainLayout->setSpacing(0);

    // ================= 1. ЛЕВАЯ ПАНЕЛЬ (Группы / Серверы) =================
    QWidget *serversContainer = new QWidget(this);
    serversContainer->setFixedWidth(70); // Жестко фиксируем всю колонку!

    QVBoxLayout *serversLayout = new QVBoxLayout(serversContainer);
    serversLayout->setContentsMargins(5, 10, 5, 10); // Отступы от краев
    serversLayout->setSpacing(10); // Расстояние между элементами

    serversList = new QListWidget(this);
    serversList->setFrameShape(QFrame::NoFrame); // Убираем уродливую рамку
    serversList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // Убираем нижний ползунок
    serversList->setObjectName("serversList");

    createGroupButton = new QPushButton("+", this);
    createGroupButton->setFixedSize(50, 50); // Делаем кнопку ровным квадратом
    // Сразу накинем стиль, чтобы она стала круглой и зеленой при наведении, как в Discord
    createGroupButton->setStyleSheet(
        "QPushButton { border-radius: 25px; background-color: #36393f; color: #43b581; font-size: 28px; font-weight: bold; }"
        "QPushButton:hover { background-color: #43b581; color: white; }"
    );
    createGroupButton->setObjectName("createGroupButton");

    serversLayout->addWidget(serversList);
    // Добавляем кнопку и центрируем ее по горизонтали
    serversLayout->addWidget(createGroupButton, 0, Qt::AlignHCenter); 

    // --- ДОБАВЛЯЕМ КНОПКУ АДМИНКИ ---
    adminPanelBtn = new QPushButton("⚙️", this); 
    adminPanelBtn->setFixedSize(50, 50);
    adminPanelBtn->setStyleSheet(
        "QPushButton { border-radius: 25px; background-color: #36393f; color: white; font-size: 24px; }"
        "QPushButton:hover { background-color: #7289da; }"
    );
    serversLayout->addWidget(adminPanelBtn, 0, Qt::AlignHCenter); 

    // Скрываем кнопку, если мы не админ
    if (!client->isAdmin()) {
        adminPanelBtn->hide(); 
    }

    mainLayout->addWidget(serversContainer);



    

// ================= 2. СРЕДНЯЯ ПАНЕЛЬ (Личные сообщения) =================
    QWidget *friendsContainer = new QWidget(this);
    friendsContainer->setFixedWidth(200); // Примерная ширина

    QVBoxLayout *friendsLayout = new QVBoxLayout(friendsContainer);
    friendsLayout->setContentsMargins(0, 0, 0, 0);

    // 1. ОБЯЗАТЕЛЬНО СОЗДАЕМ СПИСОК (Используем правильное имя - chatsList)
    chatsList = new QListWidget(this);
    chatsList->setObjectName("chatsList");

    // 2. Добавляем список в layout
    friendsLayout->addWidget(chatsList);

    // 3. Создаем нашу новую кнопку и добавляем ее ПОД списком
    createUserBtn = new QPushButton("Создать пользователя", this);
    friendsLayout->addWidget(createUserBtn);

    // <-- ДОБАВИТЬ: ПРОВЕРКА НА АДМИНА
    if (!client->isAdmin()) {
        createUserBtn->hide(); // Если не админ, кнопка просто исчезает из интерфейса
    }

    // 4. Логика кнопки
    connect(createUserBtn, &QPushButton::clicked, this, [this, client]() {
        CreateUserDialog dialog(this);
        
        if (dialog.exec() == QDialog::Accepted) {
            QString login = dialog.getLogin();
            QString password = dialog.getPassword();
            bool isAdmin = dialog.isAdmin();
            
            client->createAccount(login, password, isAdmin);
        }
    });
    
    // Добавляем всю панель в главное окно
    mainLayout->addWidget(friendsContainer);

    // ================= 3. ПРАВАЯ ПАНЕЛЬ (Переключение экранов) =================
    mainArea = new QStackedWidget(this);

    // --- Экран 0: Заглушка (Главное меню) ---
    homeWidget = new QWidget(this);
    QVBoxLayout *homeLayout = new QVBoxLayout(homeWidget);
    QLabel *welcomeLabel = new QLabel("Выберите чат или группу слева", this);
    welcomeLabel->setAlignment(Qt::AlignCenter);
    
    QFont font = welcomeLabel->font();
    font.setPointSize(14);
    welcomeLabel->setFont(font);
    
    homeLayout->addWidget(welcomeLabel);
    mainArea->addWidget(homeWidget); // Индекс 0

    // --- Экран 1: Сам чат ---
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
    
    mainArea->addWidget(chatWidget); // Индекс 1

    // --- Экран 2: Админ панель ---
    adminWidget = new AdminPanelWidget(client, this);
    mainArea->addWidget(adminWidget); // Индекс 2

    mainLayout->addWidget(mainArea);
    setLayout(mainLayout);

    // По умолчанию прячем чат и показываем заглушку
    mainArea->setCurrentIndex(0);

    // ================= КОННЕКТЫ =================
    connect(sendButton, &QPushButton::clicked, this, &ChatWindow::onSendClicked);
    connect(messageInput, &QLineEdit::returnPressed, this, &ChatWindow::onSendClicked); // Отправка по Enter
    connect(client, &MessengerClient::messageReceived, this, &ChatWindow::onMessageReceived);
    connect(client, &MessengerClient::userListReceived, this, &ChatWindow::onUserListReceived);
    connect(chatsList, &QListWidget::itemClicked, this, &ChatWindow::onChatSelected);
    connect(client, &MessengerClient::historyReceived, this, &ChatWindow::onHistoryReceived);
    // --- КОННЕКТЫ ДЛЯ АДМИНКИ ---
    connect(adminPanelBtn, &QPushButton::clicked, this, [this, client]() {
        mainArea->setCurrentIndex(2);
        client->requestAdminData();
    });

    connect(client, &MessengerClient::adminDataReceived, adminWidget, &AdminPanelWidget::updateTable);

}

// ================= МЕТОДЫ =================

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
    messagesDisplay->clear(); // Очищаем экран
    
    // Запрашиваем историю у сервера
    client->requestHistory(currentRecipient);
}

void ChatWindow::onHistoryReceived(const QString& chatWith, const QJsonArray& messages) {
    // Если история пришла именно для того чата, который сейчас открыт
    if (chatWith == currentRecipient) {
        messagesDisplay->clear(); // На всякий случай чистим еще раз
        
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
