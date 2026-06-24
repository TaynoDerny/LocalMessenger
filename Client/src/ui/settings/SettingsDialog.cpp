#include "SettingsDialog.h"
#include <QFileDialog>
#include <QBuffer>
#include <QDebug>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>

// Вспомогательная функция: обрезает ЛЮБУЮ картинку в идеальный гладкий круг и добавляет рамочку
static QPixmap createCircularAvatar(const QPixmap& source, int size) {
    if (source.isNull()) {
        QPixmap empty(size, size);
        empty.fill(Qt::transparent);
        return empty;
    }

    // 1. Масштабируем с заполнением
    QPixmap scaled = source.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    
    // 2. Обрезаем ровно центр (чтобы не вылезало)
    int x = (scaled.width() - size) / 2;
    int y = (scaled.height() - size) / 2;
    QPixmap cropped = scaled.copy(x, y, size, size);

    // 3. Создаем финальный прозрачный холст
    QPixmap final(size, size);
    final.fill(Qt::transparent);

    QPainter painter(&final);
    painter.setRenderHint(QPainter::Antialiasing); // ВКЛЮЧАЕМ СГЛАЖИВАНИЕ

    // 4. Применяем обтравочную маску (Клиппинг) к кругу
    QPainterPath path;
    path.addEllipse(0, 0, size, size);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, cropped);

    // 5. Отключаем клиппинг и рисуем круглую рамочку (границу) поверх картинки
    painter.setClipping(false);
    painter.setPen(QPen(QColor("#4f545c"), 3));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(1.5, 1.5, size - 3, size - 3);

    return final;
}

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent) {
    setupUI();
    applyStyles();
    
    resize(750, 500);
    setWindowTitle("Настройки");
}

void SettingsDialog::setupUI() {
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // --- ЛЕВАЯ ПАНЕЛЬ (КАТЕГОРИИ) ---
    sidebarList = new QListWidget(this);
    sidebarList->setFixedWidth(220);
    sidebarList->addItem("Профиль");
    sidebarList->addItem("Внешний вид");
    sidebarList->addItem("О приложении");

    // --- ПРАВАЯ ПАНЕЛЬ (КОНТЕНТ) ---
    pagesWidget = new QStackedWidget(this);

    // --- 1. Страница: Профиль (Аватар + Никнейм) ---
    QWidget* profilePage = new QWidget();
    QVBoxLayout* profileLayout = new QVBoxLayout(profilePage);
    profileLayout->setContentsMargins(40, 40, 40, 40);
    
    QLabel* profileTitle = new QLabel("Профиль пользователя", profilePage);
    profileTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: white; margin-bottom: 20px;");
    profileLayout->addWidget(profileTitle);

    QHBoxLayout* mainRowLayout = new QHBoxLayout();
    mainRowLayout->setSpacing(30);

    // Левая колонка (Аватар)
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

    // Правая колонка (Имя)
    QVBoxLayout* rightCol = new QVBoxLayout();
    rightCol->setAlignment(Qt::AlignTop);

    QLabel* nickLabel = new QLabel("Отображаемое имя", this);
    nickLabel->setStyleSheet("color: #b9bbbe; font-size: 14px;");
    rightCol->addWidget(nickLabel);
    
    usernameEdit = new QLineEdit(this);
    usernameEdit->setText("pokemon1903");
    usernameEdit->setStyleSheet("background-color: #202225; color: white; border-radius: 4px; padding: 8px; border: none;");
    rightCol->addWidget(usernameEdit);

    QLabel* discriminator = new QLabel("#1234", this);
    discriminator->setStyleSheet("color: #b9bbbe; font-size: 14px;");
    rightCol->addWidget(discriminator);
    rightCol->addSpacing(20);

    saveProfileBtn = new QPushButton("Сохранить изменения", this);
    saveProfileBtn->setObjectName("saveProfileBtn");
    connect(saveProfileBtn, &QPushButton::clicked, this, &SettingsDialog::onSaveProfile);
    rightCol->addWidget(saveProfileBtn);

    mainRowLayout->addLayout(leftCol);
    mainRowLayout->addLayout(rightCol);
    mainRowLayout->addStretch();

    profileLayout->addLayout(mainRowLayout);
    profileLayout->addStretch();

    // --- 2. Страница: Внешний вид ---
    QWidget* appearancePage = new QWidget();
    QVBoxLayout* appearanceLayout = new QVBoxLayout(appearancePage);
    appearanceLayout->setContentsMargins(40, 40, 40, 40);
    QLabel* appearanceLabel = new QLabel("Настройки внешнего вида", appearancePage);
    appearanceLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: white;");
    appearanceLayout->addWidget(appearanceLabel);
    appearanceLayout->addStretch();

    // --- 3. Страница: О приложении (НОВАЯ) ---
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

    // Добавляем страницы в QStackedWidget строго по порядку списка
    pagesWidget->addWidget(profilePage);    // Индекс 0
    pagesWidget->addWidget(appearancePage); // Индекс 1
    pagesWidget->addWidget(aboutPage);      // Индекс 2

    mainLayout->addWidget(sidebarList);
    mainLayout->addWidget(pagesWidget);

    connect(sidebarList, &QListWidget::currentRowChanged, pagesWidget, &QStackedWidget::setCurrentIndex);
    sidebarList->setCurrentRow(0);

    // Загружаем стандартную аватарку
    loadDefaultAvatar();
}

void SettingsDialog::applyStyles() {
    this->setStyleSheet(
        "QDialog { background-color: #36393f; }"
        "QListWidget { background-color: #2f3136; color: #8e9297; border: none; font-size: 15px; padding: 15px 10px; }"
        "QListWidget::item { padding: 10px; border-radius: 5px; margin-bottom: 2px; }"
        "QListWidget::item:selected { background-color: #393c43; color: white; }"
        "QListWidget::item:hover:!selected { background-color: #32353a; color: #dcddde; }"
    );
}

void SettingsDialog::loadDefaultAvatar() {
    QPixmap defaultImage(100, 100);
    defaultImage.fill(Qt::transparent);

    // Пытаемся загрузить вашу стандартную картинку из ресурсов
    QPixmap resourcePixmap(":/images/avatar_placeholder.png"); 

    if (!resourcePixmap.isNull()) {
        defaultImage = createCircularAvatar(resourcePixmap, 100);
    } else {
        // Если файла нет - рисуем синий круг с вопросиком
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

void SettingsDialog::onAvatarSelected() {
    QString fileName = QFileDialog::getOpenFileName(this, "Выберите аватар", "", "Изображения (*.png *.jpg *.jpeg *.bmp)");
    if (!fileName.isEmpty()) {
        QPixmap newAvatar(fileName);
        avatarLabel->setPixmap(createCircularAvatar(newAvatar, 100));
    }
}

void SettingsDialog::onSaveProfile() {
    QString newName = usernameEdit->text();
    if (newName.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Имя пользователя не может быть пустым!");
        return;
    }

    QPixmap currentAvatar = avatarLabel->pixmap(Qt::ReturnByValue);
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    currentAvatar.save(&buffer, "PNG");
    QString avatarBase64 = QString::fromLatin1(byteArray.toBase64());

    qDebug() << "НАСТРОЙКИ: Сохраняем профиль. Имя:" << newName;
    qDebug() << "НАСТРОЙКИ: Аватарка (Base64 length):" << avatarBase64.length();
    
    // В будущем раскомментировать для отправки на сервер:
    // client->updateProfile(newName, avatarBase64);
    
    QMessageBox::information(this, "Успех", "Изменения профиля сохранены (локально).");
}