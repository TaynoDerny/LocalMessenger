#include "ChatWindow.h"
#include "../settings/SettingsDialog.h"

ChatWindow::ChatWindow(MessengerClient *client, QWidget *parent)
    : QWidget(parent), client(client) {

    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("Мессенджер");
    resize(1280, 720);

    // ================= ОСНОВНОЙ ВЕРТИКАЛЬНЫЙ СЛОЙ =================
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0); 
    mainLayout->setSpacing(0);

    // ================= 1. ОБЩИЙ КОНТЕЙНЕР ВЕРХНЕЙ ПАНЕЛИ =================
    QWidget *topBarContainer = new QWidget(this);
    topBarContainer->setFixedHeight(48); // <--- ФИКС: Задаем общую высоту 48px
    QHBoxLayout *topBarLayout = new QHBoxLayout(topBarContainer);
    topBarLayout->setContentsMargins(0, 0, 0, 0); 
    topBarLayout->setSpacing(0);                  

    // --- 1.1. ЛЕВАЯ ЧАСТЬ ВЕРХНЕЙ ПАНЕЛИ (Цвет списка чатов) ---
    QWidget *leftTopBar = new QWidget(this);
    leftTopBar->setFixedWidth(280); 
    leftTopBar->setFixedHeight(48); // <--- ФИКС: Задаем высоту левой части
    // Задаём цвет в коде, чтобы он совпадал с friendsContainer (#2B2D31)
    // И добавляем нижнюю границу, как у правой части
    leftTopBar->setObjectName("leftTopBar");
    
    // --- 1.2. ПРАВАЯ ЧАСТЬ ВЕРХНЕЙ ПАНЕЛИ (Основной цвет шапки) ---
    QWidget *rightTopBar = new QWidget(this);
    rightTopBar->setObjectName("topBar"); // Подхватит стили (#1A1A1E и border) из style.qss
    rightTopBar->setFixedHeight(48); // <--- ФИКС: Задаем высоту правой части (для надежности)
    QHBoxLayout *rightTopBarLayout = new QHBoxLayout(rightTopBar);
    rightTopBarLayout->setContentsMargins(15, 0, 15, 0);

    // Кнопки перенесены сюда, В ПРАВУЮ ЧАСТЬ
    adminPanelBtn = new QPushButton("🛡️", this); 
    adminPanelBtn->setFixedSize(30, 30);
    adminPanelBtn->setObjectName("btnAdminPanel");
    if (!client->isAdmin()) adminPanelBtn->hide();

    connect(adminPanelBtn, &QPushButton::clicked, this, [this, client]() {
        mainArea->setCurrentIndex(2);
        client->requestAdminData();
    });

    QPushButton *settingsBtn = new QPushButton("⚙️", this);
    settingsBtn->setFixedSize(30, 30);
    settingsBtn->setObjectName("btnSettings"); 
    connect(settingsBtn, &QPushButton::clicked, this, [this]() {
        QWidget *overlay = new QWidget(this);
        overlay->setObjectName("overlayWidget");
        overlay->resize(this->size());
        overlay->show();
        SettingsDialog dialog(this->client, this); 
        dialog.exec();
        delete overlay;
    });

    // Верстка правой шапки: [Распорка] -> [Кнопка админа] -> [Кнопка настроек]
    rightTopBarLayout->addStretch(); 
    rightTopBarLayout->addWidget(adminPanelBtn);
    rightTopBarLayout->addWidget(settingsBtn);

    // Собираем верхнюю панель (Левая + Правая)
    topBarLayout->addWidget(leftTopBar);
    topBarLayout->addWidget(rightTopBar);

    // Добавляем верхнюю панель в основное окно
    mainLayout->addWidget(topBarContainer);


    // ================= 2. КОНТЕЙНЕР С ДВУМЯ КОЛОНКАМИ =================
    QWidget *contentContainer = new QWidget(this);
    QHBoxLayout *contentLayout = new QHBoxLayout(contentContainer);
    contentLayout->setContentsMargins(0, 0, 0, 0); 
    contentLayout->setSpacing(0);

    // --- ЛЕВАЯ КОЛОНКА (Список чатов) ---
    QWidget *friendsContainer = new QWidget(this);
    friendsContainer->setFixedWidth(280); 
    friendsContainer->setObjectName("friendsContainer");

    QVBoxLayout *friendsLayout = new QVBoxLayout(friendsContainer);
    friendsLayout->setContentsMargins(0, 0, 0, 0);

    chatsList = new QListWidget(this);
    chatsList->setIconSize(QSize(36, 36)); 
    chatsList->setObjectName("chatsList");

    friendsLayout->addWidget(chatsList);
    contentLayout->addWidget(friendsContainer);

    // --- ПРАВАЯ КОЛОНКА (Чат / Панель администратора) ---
    QWidget *rightContainer = new QWidget(this);
    rightContainer->setObjectName("rightContainer");

    QVBoxLayout *rightLayout = new QVBoxLayout(rightContainer);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    // Стек с разными страницами
    mainArea = new QStackedWidget(this);

    homeWidget = new QWidget(this);
    homeWidget->setObjectName("homeWidget");
    QVBoxLayout *homeLayout = new QVBoxLayout(homeWidget);
    QLabel *welcomeLabel = new QLabel("Выберите чат или группу слева", this);
    welcomeLabel->setAlignment(Qt::AlignCenter);
    QFont font = welcomeLabel->font();
    font.setPointSize(14);
    welcomeLabel->setFont(font);
    homeLayout->addWidget(welcomeLabel);
    mainArea->addWidget(homeWidget);

    chatWidget = new QWidget(this);
    chatWidget->setObjectName("chatWidget");
    QVBoxLayout *chatLayout = new QVBoxLayout(chatWidget);
    
    chatHeader = new QLabel("Выберите чат для начала общения", this);
    chatHeader->setObjectName("chatHeader");
    
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

    rightLayout->addWidget(mainArea);
    contentLayout->addWidget(rightContainer);

    // Добавляем контейнер с двумя колонками в основной вертикальный слой
    mainLayout->addWidget(contentContainer);
    setLayout(mainLayout);


    // ================= НАЧАЛЬНЫЙ ЭКРАН И КОННЕКТЫ =================
    mainArea->setCurrentIndex(0);

    connect(sendButton, &QPushButton::clicked, this, &ChatWindow::onSendClicked);
    connect(messageInput, &QLineEdit::returnPressed, this, &ChatWindow::onSendClicked);
    connect(client, &MessengerClient::messageReceived, this, &ChatWindow::onMessageReceived);
    connect(client, &MessengerClient::userListReceived, this, &ChatWindow::onUserListReceived);
    connect(chatsList, &QListWidget::itemClicked, this, &ChatWindow::onChatSelected);
    connect(client, &MessengerClient::historyReceived, this, &ChatWindow::onHistoryReceived);
    
    connect(client, &MessengerClient::adminDataReceived, adminWidget, &AdminPanelWidget::updateTable);
}

// ================= ОСТАЛЬНЫЕ МЕТОДЫ (БЕЗ ИЗМЕНЕНИЙ) =================
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

QPixmap ChatWindow::createCircularAvatarFromBase64(const QString& base64, int size, bool isOnline) {
    QPixmap source;
    if (!base64.isEmpty()) {
        QByteArray data = QByteArray::fromBase64(base64.toLatin1());
        source.loadFromData(data);
    }

    if (source.isNull()) {
        source.load(":/images/avatar_placeholder.png"); 
        
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

    QPixmap scaled = source.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    int x = (scaled.width() - size) / 2;
    int y = (scaled.height() - size) / 2;
    QPixmap cropped = scaled.copy(x, y, size, size);

    QPixmap final(size, size);
    final.fill(Qt::transparent);
    QPainter painter(&final);
    painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addEllipse(0, 0, size, size);
    painter.setClipPath(path);
    painter.fillPath(path, Qt::white); 
    painter.drawPixmap(0, 0, cropped);

    if (isOnline) {
        painter.setClipping(false); 
        int statusSize = size / 3.0; 
        int offset = size / 12;
        painter.setPen(QPen(QColor("#202225"), 2.5)); 
        painter.setBrush(QColor("#45A366")); 
        painter.drawEllipse(size - statusSize - offset, size - statusSize - offset, statusSize, statusSize);
    }

    return final;
}

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

        QPixmap avatarPixmap = createCircularAvatarFromBase64(avatarBase64, 36, isOnline);
        if (!avatarPixmap.isNull()) {
            QIcon icon(avatarPixmap);
            icon.addPixmap(avatarPixmap, QIcon::Selected);
            icon.addPixmap(avatarPixmap, QIcon::Active);
            item->setIcon(icon);
        }
        item->setForeground(QBrush(QColor("#dbdee1")));
    }
}