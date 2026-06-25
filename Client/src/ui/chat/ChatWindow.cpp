#include "ChatWindow.h"
#include "../settings/SettingsDialog.h"
#include <QScrollBar>
#include <QTime>
#include <QStatusBar> // ДОБАВИЛИ
#include <QLabel>     // ДОБАВИЛИ

// ==========================================================
// ВНУТРЕННИЙ ВИДЖЕТ ОДНОГО СООБЩЕНИЯ
// ==========================================================
class MessageWidget : public QWidget {
public:
    MessageWidget(const QString& sender, const QString& text, const QPixmap& avatar, bool isAdmin, QWidget *parent = nullptr)
        : QWidget(parent) {
        
        this->setObjectName("messageWidget");
        this->setAttribute(Qt::WA_Hover, true);

        QHBoxLayout *hLayout = new QHBoxLayout(this);
        hLayout->setContentsMargins(15, 8, 15, 8);
        hLayout->setSpacing(15);

        QLabel *avatarLabel = new QLabel(this);
        avatarLabel->setPixmap(avatar);
        avatarLabel->setFixedSize(40, 40);
        avatarLabel->setObjectName("msgAvatarLabel");
        hLayout->addWidget(avatarLabel, 0, Qt::AlignTop);

        QVBoxLayout *vLayout = new QVBoxLayout();
        vLayout->setSpacing(4);

        QHBoxLayout *metaLayout = new QHBoxLayout();
        QLabel *nameLabel = new QLabel(sender, this);
        
        QString color = isAdmin ? "#FAA61A" : "#DBDEE1";
        nameLabel->setStyleSheet(QString("font-weight: bold; font-size: 15px; color: %1; background-color: transparent;").arg(color));

        QLabel *timeLabel = new QLabel(QTime::currentTime().toString("HH:mm"), this);
        timeLabel->setObjectName("msgTimeLabel");
        timeLabel->setStyleSheet("color: #72767d; font-size: 12px; background-color: transparent;");
        
        metaLayout->addWidget(nameLabel);
        metaLayout->addWidget(timeLabel);
        metaLayout->addStretch();

        QLabel *textLabel = new QLabel(text, this);
        textLabel->setWordWrap(true);
        textLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        textLabel->setObjectName("msgTextLabel");
        textLabel->setStyleSheet("color: #dcddde; font-size: 15px; line-height: 1.4; background-color: transparent;");

        vLayout->addLayout(metaLayout);
        vLayout->addWidget(textLabel);
        hLayout->addLayout(vLayout);
        hLayout->addStretch();
    }
};

// ==========================================================
// ОСНОВНОЙ КЛАСС CHATWINDOW (ТЕПЕРЬ ОТ QMAINWINDOW)
// ==========================================================
ChatWindow::ChatWindow(MessengerClient *client, QWidget *parent)
    : QMainWindow(parent), client(client) { // <--- ИЗМЕНИЛИ БАЗОВЫЙ КЛАСС

    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("Мессенджер");
    resize(1280, 720);

    // ================= ЦЕНТРАЛЬНЫЙ ВИДЖЕТ (ВМЕСТО ПРЯМОГО НАСЛЕДОВАНИЯ) =================
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0); 
    mainLayout->setSpacing(0);

    // ================= 1. ОБЩИЙ КОНТЕЙНЕР ВЕРХНЕЙ ПАНЕЛИ =================
    QWidget *topBarContainer = new QWidget(this);
    topBarContainer->setFixedHeight(48);
    QHBoxLayout *topBarLayout = new QHBoxLayout(topBarContainer);
    topBarLayout->setContentsMargins(0, 0, 0, 0); 
    topBarLayout->setSpacing(0);                  

    QWidget *leftTopBar = new QWidget(this);
    leftTopBar->setFixedWidth(280); 
    leftTopBar->setFixedHeight(48);
    leftTopBar->setObjectName("leftTopBar");
    
    QWidget *rightTopBar = new QWidget(this);
    rightTopBar->setObjectName("topBar");
    rightTopBar->setFixedHeight(48);
    QHBoxLayout *rightTopBarLayout = new QHBoxLayout(rightTopBar);
    rightTopBarLayout->setContentsMargins(15, 0, 15, 0);

    adminPanelBtn = new QPushButton("🛡️", this); 
    adminPanelBtn->setFixedSize(30, 30);
    adminPanelBtn->setObjectName("btnAdminPanel");
    if (!client->isAdmin()) adminPanelBtn->hide();

    connect(adminPanelBtn, &QPushButton::clicked, this, [this, client]() {
        mainArea->setCurrentIndex(2);
        client->requestAdminData();
    });

    QPushButton *settingsBtn = new QPushButton(this);
    settingsBtn->setIcon(QIcon(":/images/settings_icon.png"));
    settingsBtn->setIconSize(QSize(24, 24));
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

    rightTopBarLayout->addStretch(); 
    rightTopBarLayout->addWidget(adminPanelBtn);
    rightTopBarLayout->addWidget(settingsBtn);

    topBarLayout->addWidget(leftTopBar);
    topBarLayout->addWidget(rightTopBar);
    mainLayout->addWidget(topBarContainer);

    // ================= 2. КОНТЕЙНЕР С ДВУМЯ КОЛОНКАМИ =================
    QWidget *contentContainer = new QWidget(this);
    QHBoxLayout *contentLayout = new QHBoxLayout(contentContainer);
    contentLayout->setContentsMargins(0, 0, 0, 0); 
    contentLayout->setSpacing(0);

    QWidget *friendsContainer = new QWidget(this);
    friendsContainer->setFixedWidth(280); 
    friendsContainer->setObjectName("friendsContainer");
    QVBoxLayout *friendsLayout = new QVBoxLayout(friendsContainer);
    friendsLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *dmLabel = new QLabel("Личные сообщения", this);
    dmLabel->setObjectName("dmLabel");
    friendsLayout->addWidget(dmLabel);

    chatsList = new QListWidget(this);
    chatsList->setIconSize(QSize(36, 36)); 
    chatsList->setObjectName("chatsList");
    friendsLayout->addWidget(chatsList);
    contentLayout->addWidget(friendsContainer);

    QWidget *rightContainer = new QWidget(this);
    rightContainer->setObjectName("rightContainer");
    QVBoxLayout *rightLayout = new QVBoxLayout(rightContainer);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    mainArea = new QStackedWidget(this);

    homeWidget = new QWidget(this);
    homeWidget->setObjectName("homeWidget");
    QVBoxLayout *homeLayout = new QVBoxLayout(homeWidget);
    QLabel *welcomeLabel = new QLabel("Выберите чат или группу слева", this);
    welcomeLabel->setAlignment(Qt::AlignCenter);
    welcomeLabel->setStyleSheet("color: #72767d; font-size: 18px;");
    homeLayout->addWidget(welcomeLabel);
    mainArea->addWidget(homeWidget);

    chatWidget = new QWidget(this);
    chatWidget->setObjectName("chatWidget");
    QVBoxLayout *chatLayout = new QVBoxLayout(chatWidget);
    chatLayout->setContentsMargins(0, 0, 0, 0);
    
    chatHeader = new QLabel("Выберите чат для начала общения", this);
    chatHeader->setObjectName("chatHeader");
    chatLayout->addWidget(chatHeader);

    messagesContainer = new QWidget();
    messagesLayout = new QVBoxLayout(messagesContainer);
    messagesLayout->setContentsMargins(0, 5, 0, 10);
    messagesLayout->setSpacing(0);
    messagesLayout->addStretch(); 

    messagesArea = new QScrollArea();
    messagesArea->setWidget(messagesContainer);
    messagesArea->setWidgetResizable(true);
    messagesArea->setFrameShape(QFrame::NoFrame);
    messagesArea->setObjectName("messagesArea"); 
    
    chatLayout->addWidget(messagesArea);

    messageInput = new QLineEdit(this);
    messageInput->setPlaceholderText("Напишите сообщение...");
    messageInput->setObjectName("messageInput");
    messageInput->setFixedHeight(44);
    messageInput->setContentsMargins(15, 0, 15, 0);
    
    chatLayout->addWidget(messageInput);
    mainArea->addWidget(chatWidget);

    adminWidget = new AdminPanelWidget(client, this);
    mainArea->addWidget(adminWidget);

    rightLayout->addWidget(mainArea);
    contentLayout->addWidget(rightContainer);

    mainLayout->addWidget(contentContainer);

    // ================= УСТАНАВЛИВАЕМ ЦЕНТРАЛЬНЫЙ ВИДЖЕТ =================
    this->setCentralWidget(centralWidget);

    // ================= СТРОКА СОСТОЯНИЯ (STATUS BAR) =================
    QStatusBar *statusBar = this->statusBar();
    QLabel *statusLabel = new QLabel("⚡ Статус: Подключено к серверу", this);
    statusBar->addWidget(statusLabel);
    
    // Стилизуем строку состояния, чтобы она сливалась с вашей темой
    statusBar->setStyleSheet("QStatusBar { background-color: #1A1A1E; color: #b9bbbe; border-top: 1px solid #3D3D3D; } QStatusBar QLabel { color: #b9bbbe; }");

    // ================= НАЧАЛЬНЫЙ ЭКРАН И КОННЕКТЫ =================
    mainArea->setCurrentIndex(0);

    connect(messageInput, &QLineEdit::returnPressed, this, &ChatWindow::onSendClicked);
    connect(client, &MessengerClient::messageReceived, this, &ChatWindow::onMessageReceived);
    connect(client, &MessengerClient::userListReceived, this, &ChatWindow::onUserListReceived);
    connect(chatsList, &QListWidget::itemClicked, this, &ChatWindow::onChatSelected);
    connect(client, &MessengerClient::historyReceived, this, &ChatWindow::onHistoryReceived);
    connect(client, &MessengerClient::adminDataReceived, adminWidget, &AdminPanelWidget::updateTable);
}

void ChatWindow::clearChatMessages() {
    while (QLayoutItem* item = messagesLayout->takeAt(0)) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
    messagesLayout->addStretch();
}

void ChatWindow::addMessageToChat(const QString& sender, const QString& text) {
    QPixmap avatar = userAvatars.value(sender, QPixmap());
    if (avatar.isNull()) {
        avatar = QPixmap(40, 40);
        avatar.fill(Qt::transparent);
        QPainter p(&avatar);
        p.setBrush(QColor(54, 57, 63));
        p.setPen(Qt::NoPen);
        p.drawEllipse(0, 0, 40, 40);
        p.setPen(QColor(185, 187, 190));
        p.drawText(avatar.rect(), Qt::AlignCenter, "?");
    }

    bool isAdmin = userAdmins.value(sender, false);

    MessageWidget *widget = new MessageWidget(sender, text, avatar, isAdmin);
    messagesLayout->insertWidget(messagesLayout->count() - 1, widget);

    QScrollBar* bar = messagesArea->verticalScrollBar();
    bar->setValue(bar->maximum());
}

void ChatWindow::onSendClicked() {
    QString text = messageInput->text();
    if (!text.isEmpty() && !currentRecipient.isEmpty()) {
        client->sendMessage(text, currentRecipient);
        addMessageToChat(client->getMyLogin(), text);
        messageInput->clear(); 
    }
}

void ChatWindow::onMessageReceived(const QString& sender, const QString& text) {
    if (currentRecipient == sender) {
        addMessageToChat(sender, text);
    }
}

void ChatWindow::onHistoryReceived(const QString& chatWith, const QJsonArray& messages) {
    if (chatWith == currentRecipient) {
        clearChatMessages();
        for (const QJsonValue& val : messages) {
            QJsonObject msgObj = val.toObject();
            QString sender = msgObj["sender"].toString();
            QString text = msgObj["text"].toString();
            addMessageToChat(sender, text);
        }
    }
}

void ChatWindow::onChatSelected(QListWidgetItem *item) {
    currentRecipient = item->data(Qt::UserRole).toString();
    chatHeader->setText("<b>Чат с:</b> " + currentRecipient); 
    mainArea->setCurrentIndex(1);
    clearChatMessages();
    client->requestHistory(currentRecipient);
}

void ChatWindow::onUserListReceived(const QJsonArray& users) {
    chatsList->clear(); 
    userAvatars.clear();
    userAdmins.clear();

    for (const QJsonValue& val : users) {
        QJsonObject userObj = val.toObject();
        
        QString login = userObj["login"].toString();
        QString displayName = userObj["display_name"].toString();
        if (displayName.isEmpty()) displayName = login; 
        bool isOnline = userObj["online"].toBool(); 
        bool isAdmin = userObj["is_admin"].toBool();
        QString avatarBase64 = userObj["avatar_base64"].toString();

        userAdmins[login] = isAdmin;

        QPixmap avatarPixmapClean = createCircularAvatarFromBase64(avatarBase64, 36, false);
        userAvatars[login] = avatarPixmapClean;

        if (login == client->getMyLogin()) {
            continue; 
        }

        QListWidgetItem *item = new QListWidgetItem(chatsList);
        item->setText(displayName);
        item->setData(Qt::UserRole, login);

        QPixmap avatarPixmapWithStatus = createCircularAvatarFromBase64(avatarBase64, 36, isOnline);
        if (!avatarPixmapWithStatus.isNull()) {
            QIcon icon(avatarPixmapWithStatus);
            icon.addPixmap(avatarPixmapWithStatus, QIcon::Selected);
            icon.addPixmap(avatarPixmapWithStatus, QIcon::Active);
            item->setIcon(icon);
        }

        item->setForeground(QBrush(QColor("#dbdee1")));
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