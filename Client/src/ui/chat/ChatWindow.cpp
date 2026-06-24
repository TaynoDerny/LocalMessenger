#include "ChatWindow.h"
#include "../settings/SettingsDialog.h"

ChatWindow::ChatWindow(MessengerClient *client, QWidget *parent)
    : QWidget(parent), client(client) {

    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("Мессенджер");
    resize(1280, 720);

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
    createGroupButton->setObjectName("btnCreateGroup"); // <--- Убрали setStyleSheet, дали имя

    serversLayout->addWidget(serversList);
    serversLayout->addWidget(createGroupButton, 0, Qt::AlignHCenter); 

    adminPanelBtn = new QPushButton("⚙️", this); 
    adminPanelBtn->setFixedSize(50, 50);
    adminPanelBtn->setObjectName("btnAdminPanel"); // <--- Убрали setStyleSheet, дали имя
    if (!client->isAdmin()) adminPanelBtn->hide(); 

    serversLayout->addWidget(adminPanelBtn, 0, Qt::AlignHCenter); 
    mainLayout->addWidget(serversContainer);

    // ================= 2. СРЕДНЯЯ ПАНЕЛЬ =================
    QWidget *friendsContainer = new QWidget(this);
    friendsContainer->setFixedWidth(250);
    friendsContainer->setObjectName("friendsContainer"); // <--- Дали имя

    QVBoxLayout *friendsLayout = new QVBoxLayout(friendsContainer);
    friendsLayout->setContentsMargins(0, 0, 0, 0);

    chatsList = new QListWidget(this);
    chatsList->setIconSize(QSize(36, 36)); 
    // Убрали chatsList->setStyleSheet(...), стили теперь в style.qss через #chatsList
    chatsList->setObjectName("chatsList");

    friendsLayout->addWidget(chatsList);
    mainLayout->addWidget(friendsContainer);

    // ================= 3. ПРАВАЯ ПАНЕЛЬ =================
    QWidget *rightContainer = new QWidget(this);
    rightContainer->setObjectName("rightContainer"); // <--- Дали имя

    QVBoxLayout *rightLayout = new QVBoxLayout(rightContainer);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    QWidget *topBar = new QWidget(this);
    topBar->setFixedHeight(48);
    topBar->setObjectName("topBar"); // <--- Убрали setStyleSheet, дали имя

    QHBoxLayout *topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(15, 0, 15, 0);
    topBarLayout->addStretch();

    QPushButton *settingsBtn = new QPushButton("⚙️", this);
    settingsBtn->setFixedSize(30, 30);
    settingsBtn->setObjectName("btnSettings"); // <--- Убрали setStyleSheet, дали имя
    topBarLayout->addWidget(settingsBtn);

    connect(settingsBtn, &QPushButton::clicked, this, [this]() {
        QWidget *overlay = new QWidget(this);
        overlay->setObjectName("overlayWidget"); // <--- Убрали setStyleSheet, дали имя
        overlay->resize(this->size());
        overlay->show();
        SettingsDialog dialog(this);
        dialog.exec();
        delete overlay;
    });

    mainArea = new QStackedWidget(this);

    homeWidget = new QWidget(this);
    homeWidget->setObjectName("homeWidget"); // <--- Дали имя
    QVBoxLayout *homeLayout = new QVBoxLayout(homeWidget);
    QLabel *welcomeLabel = new QLabel("Выберите чат или группу слева", this);
    welcomeLabel->setAlignment(Qt::AlignCenter);
    QFont font = welcomeLabel->font();
    font.setPointSize(14);
    welcomeLabel->setFont(font);
    homeLayout->addWidget(welcomeLabel);
    mainArea->addWidget(homeWidget);

    chatWidget = new QWidget(this);
    chatWidget->setObjectName("chatWidget"); // <--- Дали имя
    QVBoxLayout *chatLayout = new QVBoxLayout(chatWidget);
    
    chatHeader = new QLabel("Выберите чат для начала общения", this);
    chatHeader->setObjectName("chatHeader"); // <--- Дали имя
    
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

// ================= ОСТАЛЬНЫЕ МЕТОДЫ =================
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

void ChatWindow::onChatSelected(QListWidgetItem *item) {
    currentRecipient = item->data(Qt::UserRole).toString();
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

// ================= ФУНКЦИЯ ОТРИСОВКИ АВАТАРКИ =================
QPixmap ChatWindow::createCircularAvatarFromBase64(const QString& base64, int size, bool isOnline) {
    QPixmap source;
    if (!base64.isEmpty()) {
        QByteArray data = QByteArray::fromBase64(base64.toLatin1());
        source.loadFromData(data);
    }

    // Если аватарки нет, загружаем стандартную картинку из ресурсов
    if (source.isNull()) {
        source.load(":/images/avatar_placeholder.png"); 
        
        // Если файла нет в папке, рисуем темно-серую заглушку
        if (source.isNull()) {
            QPixmap fallback(size, size);
            fallback.fill(Qt::transparent);
            QPainter p(&fallback);
            p.setRenderHint(QPainter::Antialiasing);
            p.setBrush(QColor(54, 57, 63));
            p.setPen(Qt::NoPen);
            p.drawEllipse(0, 0, size, size);
            p.setPen(QColor(185, 187, 190));
            p.setFont(QFont("Segoe UI", size/2.5, QFont::Bold));
            p.drawText(fallback.rect(), Qt::AlignCenter, "?");
            source = fallback;
        }
    }

    // Масштабируем и вырезаем центр
    QPixmap scaled = source.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    int x = (scaled.width() - size) / 2;
    int y = (scaled.height() - size) / 2;
    QPixmap cropped = scaled.copy(x, y, size, size);

    // Отрисовываем круг из картинки
    QPixmap final(size, size);
    final.fill(Qt::transparent);
    QPainter painter(&final);
    painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addEllipse(0, 0, size, size);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, cropped);

    // Отрисовываем точку статуса (для 36px она будет 12px - идеально)
    if (isOnline) {
        painter.setClipping(false); 
        
        // Расчет размера точки (1/3 от размера аватарки) = 12px
        int statusSize = size / 3.0; 
        int offset = size / 12;
        
        painter.setPen(QPen(QColor("#202225"), 2.5)); 
        painter.setBrush(QColor("#45A366")); 
        painter.drawEllipse(size - statusSize - offset, size - statusSize - offset, statusSize, statusSize);
    }

    return final;
}

// ================= ОБНОВЛЕННЫЙ СПИСОК ПОЛЬЗОВАТЕЛЕЙ =================
void ChatWindow::onUserListReceived(const QJsonArray& users) {
    chatsList->clear(); 

    for (const QJsonValue& val : users) {
        QJsonObject userObj = val.toObject();
        
        QString login = userObj["login"].toString();
        QString displayName = userObj["display_name"].toString();
        if (displayName.isEmpty()) displayName = login; 
        bool isOnline = userObj["online"].toBool();
        QString avatarBase64 = userObj["avatar_base64"].toString();

        QListWidgetItem *item = new QListWidgetItem(chatsList);
        item->setText(displayName);
        item->setData(Qt::UserRole, login);

        // Генерируем аватарку ровно 36 пикселей!
        QPixmap avatarPixmap = createCircularAvatarFromBase64(avatarBase64, 36, isOnline);
        if (!avatarPixmap.isNull()) {
            QIcon icon(avatarPixmap);
            // Запрещаем Qt менять цвета иконки при выборе элемента
            icon.addPixmap(avatarPixmap, QIcon::Selected);
            icon.addPixmap(avatarPixmap, QIcon::Active); // на случай, если список активен
            item->setIcon(icon);
        }
        
        item->setForeground(QBrush(QColor("#dbdee1")));
    }
}