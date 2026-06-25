#include "SettingsDialog.h"
#include "../../network/MessengerClient.h" 
#include <QFileDialog>
#include <QBuffer>
#include <QDebug>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QAbstractSocket>
#include <QSettings>
#include <QRegularExpressionValidator>
#include <QIntValidator>
#include <QFormLayout> 

static QPixmap createCircularAvatar(const QPixmap& source, int size) {
    if (source.isNull()) return QPixmap();
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
    painter.setClipping(false);
    painter.setPen(QPen(QColor("#4f545c"), 3));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(1.5, 1.5, size - 3, size - 3);
    return final;
}

// ========== ОБНОВЛЕННЫЙ КОНСТРУКТОР ==========
SettingsDialog::SettingsDialog(MessengerClient *client, QWidget *parent, bool onlyNetwork) 
    : QDialog(parent), client(client), onlyNetwork(onlyNetwork) {
    setupUI();
    applyStyles();
    loadNetworkSettings(); 

    // ========== ЛОГИКА ДЛЯ ОКНА АВТОРИЗАЦИИ ==========
    if (onlyNetwork) {
        // Убираем сайдбар
        sidebarList->hide();
        sidebarList->setFixedWidth(0);
        // Сразу переключаем на вкладку Сети (индекс 2)
        pagesWidget->setCurrentIndex(2);
        // Меняем заголовок
        setWindowTitle("Настройки сервера");
    } else {
        resize(750, 550);
        setWindowTitle("Настройки");
    }
}

void SettingsDialog::setupUI() {
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    sidebarList = new QListWidget(this);
    sidebarList->setFixedWidth(220);
    sidebarList->addItem("Профиль");
    sidebarList->addItem("Внешний вид");
    sidebarList->addItem("Сеть");
    sidebarList->addItem("О приложении");

    pagesWidget = new QStackedWidget(this);

    // --- СТРАНИЦА: Профиль ---
    QWidget* profilePage = new QWidget();
    QVBoxLayout* profileLayout = new QVBoxLayout(profilePage);
    profileLayout->setContentsMargins(40, 40, 40, 40);
    QLabel* profileTitle = new QLabel("Профиль пользователя", profilePage);
    profileTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: white; margin-bottom: 20px;");
    profileLayout->addWidget(profileTitle);

    QHBoxLayout* mainRowLayout = new QHBoxLayout();
    mainRowLayout->setSpacing(30);
    
    QVBoxLayout* leftCol = new QVBoxLayout();
    leftCol->setAlignment(Qt::AlignTop);
    avatarLabel = new QLabel(this);
    avatarLabel->setFixedSize(100, 100);
    avatarLabel->setObjectName("avatarLabel");
    avatarLabel->setAlignment(Qt::AlignCenter);
    leftCol->addWidget(avatarLabel, 0, Qt::AlignHCenter);
    
    changeAvatarBtn = new QPushButton("Сменить аватар", this);
    changeAvatarBtn->setStyleSheet("background: transparent; color: #8ea1e1; border: none; font-size: 12px; margin-top: 10px;");
    connect(changeAvatarBtn, &QPushButton::clicked, this, &SettingsDialog::onAvatarSelected);
    leftCol->addWidget(changeAvatarBtn, 0, Qt::AlignHCenter);

    QWidget* rightWidget = new QWidget();
    QFormLayout* formLayout = new QFormLayout(rightWidget);
    formLayout->setLabelAlignment(Qt::AlignLeft);
    formLayout->setSpacing(12);
    formLayout->setContentsMargins(0, 0, 0, 0);

    loginDisplay = new QLineEdit(client->getMyLogin(), this);
    loginDisplay->setReadOnly(true);
    loginDisplay->setStyleSheet("background-color: #202225; color: #b9bbbe; border-radius: 4px; padding: 8px; border: none;");
    formLayout->addRow("Логин:", loginDisplay);

    firstNameEdit = new QLineEdit(this);
    firstNameEdit->setStyleSheet("background-color: #202225; color: white; border-radius: 4px; padding: 8px; border: none;");
    formLayout->addRow("Имя:", firstNameEdit);

    lastNameEdit = new QLineEdit(this);
    lastNameEdit->setStyleSheet("background-color: #202225; color: white; border-radius: 4px; padding: 8px; border: none;");
    formLayout->addRow("Фамилия:", lastNameEdit);

    jobTitleEdit = new QLineEdit(this);
    jobTitleEdit->setStyleSheet("background-color: #202225; color: white; border-radius: 4px; padding: 8px; border: none;");
    formLayout->addRow("Должность:", jobTitleEdit);

    bioEdit = new QPlainTextEdit(this);
    bioEdit->setFixedHeight(80);
    bioEdit->setStyleSheet("background-color: #202225; color: white; border-radius: 4px; padding: 8px; border: none;");
    formLayout->addRow("Описание:", bioEdit);

    saveProfileBtn = new QPushButton("Сохранить изменения", this);
    saveProfileBtn->setObjectName("saveProfileBtn");
    connect(saveProfileBtn, &QPushButton::clicked, this, &SettingsDialog::onSaveProfile);
    formLayout->addRow(saveProfileBtn); 

    mainRowLayout->addLayout(leftCol);
    mainRowLayout->addWidget(rightWidget);
    mainRowLayout->addStretch();
    profileLayout->addLayout(mainRowLayout);
    profileLayout->addStretch();

    // --- Остальные страницы ---
    QWidget* appearancePage = new QWidget();
    QVBoxLayout* appearanceLayout = new QVBoxLayout(appearancePage);
    appearanceLayout->setContentsMargins(40, 40, 40, 40);
    QLabel* appearanceLabel = new QLabel("Настройки внешнего вида", appearancePage);
    appearanceLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: white;");
    appearanceLayout->addWidget(appearanceLabel);
    appearanceLayout->addStretch();

    QWidget* networkPage = new QWidget();
    QVBoxLayout* networkLayout = new QVBoxLayout(networkPage);
    networkLayout->setContentsMargins(40, 40, 40, 40);
    QLabel* networkTitle = new QLabel("Настройки сети", networkPage);
    networkTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: white; margin-bottom: 20px;");
    networkLayout->addWidget(networkTitle);
    
    QString localIp = "127.0.0.1";
    const QList<QHostAddress> &addresses = QNetworkInterface::allAddresses();
    for (const QHostAddress &address : addresses) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost)) {
            localIp = address.toString();
            break;
        }
    }
    QHBoxLayout* userIpLayout = new QHBoxLayout();
    QLabel* userIpLabel = new QLabel("Ваш локальный IP:", networkPage);
    userIpLabel->setStyleSheet("color: #b9bbbe; font-size: 15px;");
    QLineEdit* userIpEdit = new QLineEdit(localIp, networkPage);
    userIpEdit->setReadOnly(true);
    userIpEdit->setStyleSheet("background-color: #202225; color: white; border-radius: 4px; padding: 8px; border: none;");
    userIpLayout->addWidget(userIpLabel);
    userIpLayout->addWidget(userIpEdit);
    userIpLayout->addStretch();
    networkLayout->addLayout(userIpLayout);
    networkLayout->addSpacing(20);

    QHBoxLayout* serverLayout = new QHBoxLayout();
    QLabel* serverIpLabel = new QLabel("IP сервера:", networkPage);
    serverIpLabel->setStyleSheet("color: #b9bbbe; font-size: 15px;");
    serverIpEdit = new QLineEdit(networkPage);
    serverIpEdit->setStyleSheet("background-color: #202225; color: white; border-radius: 4px; padding: 8px; border: none;");
    serverIpEdit->setFixedWidth(170);
    QRegularExpression ipRegex("^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    QValidator *ipValidator = new QRegularExpressionValidator(ipRegex, serverIpEdit);
    serverIpEdit->setValidator(ipValidator);
    QLabel* portLabel = new QLabel("Порт:", networkPage);
    portLabel->setStyleSheet("color: #b9bbbe; font-size: 15px;");
    portEdit = new QLineEdit(networkPage);
    portEdit->setFixedWidth(80);
    portEdit->setStyleSheet("background-color: #202225; color: white; border-radius: 4px; padding: 8px; border: none;");
    QValidator *portValidator = new QIntValidator(1, 65535, portEdit);
    portEdit->setValidator(portValidator);
    serverLayout->addWidget(serverIpLabel);
    serverLayout->addWidget(serverIpEdit);
    serverLayout->addSpacing(15);
    serverLayout->addWidget(portLabel);
    serverLayout->addWidget(portEdit);
    serverLayout->addStretch();
    networkLayout->addLayout(serverLayout);
    networkLayout->addSpacing(30);

    QHBoxLayout* networkBtnLayout = new QHBoxLayout();
    networkBtnLayout->setSpacing(15);
    saveNetworkBtn = new QPushButton("Сохранить и применить", networkPage);
    saveNetworkBtn->setObjectName("saveProfileBtn");
    connect(saveNetworkBtn, &QPushButton::clicked, this, &SettingsDialog::onSaveNetworkSettings);
    networkBtnLayout->addWidget(saveNetworkBtn);
    resetNetworkBtn = new QPushButton("Сбросить на стандартный", networkPage);
    resetNetworkBtn->setStyleSheet(
        "background-color: transparent; color: #8ea1e1; border: 1px solid #4f545c; border-radius: 4px; padding: 8px 16px;"
        "QPushButton:hover { background-color: #4f545c; color: white; }"
    );
    connect(resetNetworkBtn, &QPushButton::clicked, this, &SettingsDialog::onResetNetworkSettings);
    networkBtnLayout->addWidget(resetNetworkBtn);
    networkBtnLayout->addStretch();
    networkLayout->addLayout(networkBtnLayout);
    networkLayout->addStretch();

    QWidget* aboutPage = new QWidget();
    QVBoxLayout* aboutLayout = new QVBoxLayout(aboutPage);
    aboutLayout->setContentsMargins(40, 40, 40, 40);
    QLabel* aboutTitle = new QLabel("О приложении", aboutPage);
    aboutTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: white; margin-bottom: 15px;");
    aboutLayout->addWidget(aboutTitle);
    QLabel* aboutText = new QLabel(
        "Local Messenger v1.0\n\n"
        "Мессенджер для организаций.\n"
        "Разработан на C++ с использованием Qt6.\n"
        "Все данные хранятся локально на сервере.", aboutPage);
    aboutText->setStyleSheet("color: #b9bbbe; font-size: 15px; line-height: 1.5;");
    aboutLayout->addWidget(aboutText);
    aboutLayout->addStretch();

    pagesWidget->addWidget(profilePage);
    pagesWidget->addWidget(appearancePage);
    pagesWidget->addWidget(networkPage);
    pagesWidget->addWidget(aboutPage);

    mainLayout->addWidget(sidebarList);
    mainLayout->addWidget(pagesWidget);

    connect(sidebarList, &QListWidget::currentRowChanged, pagesWidget, &QStackedWidget::setCurrentIndex);
    sidebarList->setCurrentRow(0);

    loadProfileData(); 
}

void SettingsDialog::applyStyles() {
    this->setStyleSheet(
        "QDialog { background-color: #36393f; }"
        "QListWidget { background-color: #2f3136; color: #8e9297; border: none; font-size: 15px; padding: 15px 10px; }"
        "QListWidget::item { padding: 10px; border-radius: 5px; margin-bottom: 2px; }"
        "QListWidget::item:selected { background-color: #393c43; color: white; }"
        "QListWidget::item:hover:!selected { background-color: #32353a; color: #dcddde; }"
        "QFormLayout QLabel { color: #b9bbbe; font-size: 14px; }"
    );
}

void SettingsDialog::loadProfileData() {
    QString myBase64 = client->getMyAvatarBase64();
    if (!myBase64.isEmpty()) {
        QByteArray data = QByteArray::fromBase64(myBase64.toLatin1());
        QPixmap myAvatar;
        if (myAvatar.loadFromData(data)) {
            avatarLabel->setPixmap(createCircularAvatar(myAvatar, 100));
        } else {
            loadDefaultAvatarFallback();
        }
    } else {
        loadDefaultAvatarFallback();
    }

    firstNameEdit->setText(client->getMyFirstName());
    lastNameEdit->setText(client->getMyLastName());
    jobTitleEdit->setText(client->getMyJobTitle());
    bioEdit->setPlainText(client->getMyBio());
}

void SettingsDialog::loadDefaultAvatarFallback() {
    QPixmap defaultImage(100, 100);
    defaultImage.fill(Qt::transparent);
    QPixmap resourcePixmap(":/images/avatar_placeholder.png"); 
    if (!resourcePixmap.isNull()) {
        defaultImage = createCircularAvatar(resourcePixmap, 100);
    } else {
        QPainter painter(&defaultImage);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(QPen(QColor("#4f545c"), 3));
        painter.setBrush(QColor(88, 101, 242));
        painter.drawEllipse(0, 0, 100, 100);
        painter.setPen(Qt::white);
        painter.setFont(QFont("Segoe UI", 20, QFont::Bold));
        painter.drawText(defaultImage.rect(), Qt::AlignCenter, "?");
    }
    avatarLabel->setPixmap(defaultImage);
}

void SettingsDialog::loadNetworkSettings() {
    QSettings settings;
    QString savedIp = settings.value("network/server_ip", "127.0.0.1").toString();
    int savedPort = settings.value("network/server_port", 8080).toInt();
    serverIpEdit->setText(savedIp);
    portEdit->setText(QString::number(savedPort));
}

void SettingsDialog::onAvatarSelected() {
    QString fileName = QFileDialog::getOpenFileName(this, "Выберите аватар", "", "Изображения (*.png *.jpg *.jpeg *.bmp)");
    if (!fileName.isEmpty()) {
        QPixmap newAvatar(fileName);
        avatarLabel->setPixmap(createCircularAvatar(newAvatar, 100));
    }
}

void SettingsDialog::onSaveProfile() {
    QPixmap currentAvatar = avatarLabel->pixmap(Qt::ReturnByValue);
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    currentAvatar.save(&buffer, "PNG");
    QString avatarBase64 = QString::fromLatin1(byteArray.toBase64());

    if (client) {
        client->updateProfile(
            firstNameEdit->text(),
            lastNameEdit->text(),
            jobTitleEdit->text(),
            bioEdit->toPlainText(),
            avatarBase64
        );
    }
}

void SettingsDialog::onSaveNetworkSettings() {
    QString ip = serverIpEdit->text().trimmed();
    QString portStr = portEdit->text().trimmed();
    QHostAddress address;
    if (!address.setAddress(ip) || address.protocol() != QAbstractSocket::IPv4Protocol) {
        QMessageBox::warning(this, "Ошибка", "Введите корректный IPv4 адрес!");
        return;
    }
    bool ok;
    int port = portStr.toInt(&ok);
    if (!ok || port <= 0 || port > 65535) {
        QMessageBox::warning(this, "Ошибка", "Введите корректный порт (1-65535)!");
        return;
    }
    QSettings settings;
    settings.setValue("network/server_ip", ip);
    settings.setValue("network/server_port", port);
    if (client) {
        client->connectToServer(ip, port);
        // ========== ИЗМЕНЕННЫЙ ТЕКСТ УВЕДОМЛЕНИЯ ==========
        if (onlyNetwork) {
            QMessageBox::information(this, "Успех", "Настройки сервера сохранены. Теперь вы можете войти в систему.");
        } else {
            QMessageBox::information(this, "Успех", "Настройки сети сохранены и применены!");
        }
        // ==================================================
    } else {
        QMessageBox::warning(this, "Ошибка", "Клиент не найден. Перезапустите приложение, чтобы изменения вступили в силу.");
    }
}
void SettingsDialog::onResetNetworkSettings() {
    serverIpEdit->setText("127.0.0.1");
    portEdit->setText("8080");
}