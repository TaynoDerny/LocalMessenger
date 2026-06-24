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
    painter.setRenderHint(QPainter::Antialiasing); // ВКЛЮЧАЕМ СГЛАЖИВАНИЕ (делает круг идеально ровным)

    // 4. Применяем обтравочную маску (Клиппинг) к кругу
    QPainterPath path;
    path.addEllipse(0, 0, size, size);
    painter.setClipPath(path);
    painter.fillPath(path, Qt::white);
    painter.drawPixmap(0, 0, cropped);

    // 5. Отключаем клиппинг и рисуем круглую рамочку (границу) поверх картинки
    painter.setClipping(false);
    painter.setPen(QPen(QColor("#4f545c"), 3)); // Цвет и толщина границы, как в Дискорде
    painter.setBrush(Qt::NoBrush);
    // Рисуем чуть внутри (на 1.5 пикселя), чтобы граница была ровной и не обрезалась
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
    sidebarList->addItem("Учётная запись");
    sidebarList->addItem("Профиль");
    sidebarList->addItem("Конфиденциальность");
    sidebarList->addItem("Внешний вид");

    // --- ПРАВАЯ ПАНЕЛЬ (КОНТЕНТ) ---
    pagesWidget = new QStackedWidget(this);

    // 1. Страница: Учетная запись
    QWidget* accountPage = new QWidget();
    QVBoxLayout* accountLayout = new QVBoxLayout(accountPage);
    accountLayout->setContentsMargins(40, 40, 40, 40);
    
    QLabel* accountTitle = new QLabel("Моя учётная запись", accountPage);
    accountTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: white; margin-bottom: 20px;");
    QLabel* emailInfo = new QLabel("Email: ****@gmail.com", accountPage);
    emailInfo->setStyleSheet("color: #b9bbbe; font-size: 15px; margin-bottom: 5px;");
    QLabel* phoneInfo = new QLabel("Телефон: +7 (***) ***-**-**", accountPage);
    phoneInfo->setStyleSheet("color: #b9bbbe; font-size: 15px; margin-bottom: 20px;");
    
    QPushButton* changePassBtn = new QPushButton("Сменить пароль", accountPage);
    changePassBtn->setStyleSheet("background-color: #4f545c; color: white; border-radius: 4px; padding: 8px 16px;");
    connect(changePassBtn, &QPushButton::clicked, this, []() {
        QMessageBox::information(nullptr, "Смена пароля", "Здесь будет логика смены пароля через сервер (в разработке)");
    });

    accountLayout->addWidget(accountTitle);
    accountLayout->addWidget(emailInfo);
    accountLayout->addWidget(phoneInfo);
    accountLayout->addWidget(changePassBtn);
    accountLayout->addStretch();

    // --- 2. Страница: Профиль (Аватар + Никнейм) ---
    QWidget* profilePage = new QWidget();
    QVBoxLayout* profileLayout = new QVBoxLayout(profilePage);
    profileLayout->setContentsMargins(40, 40, 40, 40);
    
    QLabel* profileTitle = new QLabel("Профиль пользователя", profilePage);
    profileTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: white; margin-bottom: 20px;");
    profileLayout->addWidget(profileTitle);

    QHBoxLayout* mainRowLayout = new QHBoxLayout();
    mainRowLayout->setSpacing(30);

    // === Левая колонка ===
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

    // === Правая колонка ===
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

    // --- 3. Остальные страницы ---
    QWidget* privacyPage = new QWidget();
    QVBoxLayout* privacyLayout = new QVBoxLayout(privacyPage);
    privacyLayout->setContentsMargins(40, 40, 40, 40);
    QLabel* privacyLabel = new QLabel("Настройки конфиденциальности", privacyPage);
    privacyLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: white;");
    privacyLayout->addWidget(privacyLabel);
    privacyLayout->addStretch();

    QWidget* appearancePage = new QWidget();
    QVBoxLayout* appearanceLayout = new QVBoxLayout(appearancePage);
    appearanceLayout->setContentsMargins(40, 40, 40, 40);
    QLabel* appearanceLabel = new QLabel("Настройки внешнего вида", appearancePage);
    appearanceLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: white;");
    appearanceLayout->addWidget(appearanceLabel);
    appearanceLayout->addStretch();

    pagesWidget->addWidget(accountPage);
    pagesWidget->addWidget(profilePage);
    pagesWidget->addWidget(privacyPage);
    pagesWidget->addWidget(appearancePage);

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
    QPixmap resourcePixmap(":/images/avatar_placeholder.png"); // <-- Убедитесь, что в .qrc файле путь совпадает!

    if (!resourcePixmap.isNull()) {
        // Если файл найден - отрисовываем его в идеальный круг
        defaultImage = createCircularAvatar(resourcePixmap, 100);
    } else {
        // Если файла нет - рисуем синий круг с вопросиком (чтобы не было пустоты)
        QPainter painter(&defaultImage);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(QPen(QColor("#4f545c"), 3)); // Та же рамочка
        painter.setBrush(QColor(88, 101, 242)); // Синий Discord-цвет
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
        // Применяем наше идеальное круглое обрезание
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

    // ЗДЕСЬ ОТПРАВЛЯЕМ ВСЁ НА СЕРВЕР!
    // client->updateProfile(newName, avatarBase64);
    
    QMessageBox::information(this, "Успех", "Изменения профиля сохранены (локально).");
}