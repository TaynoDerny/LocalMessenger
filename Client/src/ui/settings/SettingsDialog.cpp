#include "SettingsDialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent) {
    setupUI();
    applyStyles();
    
    // Делаем окно без стандартных виндовых рамок (по желанию, но так стильнее)
    // setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint); 
    
    resize(750, 500); // Размер как в Дискорде
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

    // 1. Страница: Учетная запись (Заглушка)
    QWidget* accountPage = new QWidget();
    QVBoxLayout* accountLayout = new QVBoxLayout(accountPage);
    accountLayout->setContentsMargins(30, 30, 30, 30);
    
    QLabel* accountTitle = new QLabel("Информация об учётной записи", accountPage);
    accountTitle->setStyleSheet("font-size: 22px; font-weight: bold; color: white; margin-bottom: 20px;");
    
    QLabel* dummyInfo = new QLabel("Имя пользователя: pokemon1903\n\nЭлектронная почта: ****@gmail.com\n\nНомер телефона: ********6352\n\n\n[ Здесь будут реальные настройки профиля ]", accountPage);
    dummyInfo->setStyleSheet("color: #b9bbbe; font-size: 15px;");
    
    accountLayout->addWidget(accountTitle);
    accountLayout->addWidget(dummyInfo);
    accountLayout->addStretch(); // Прижимаем всё наверх

    // 2. Страница: Внешний вид (Заглушка)
    QWidget* appearancePage = new QWidget();
    QVBoxLayout* appLayout = new QVBoxLayout(appearancePage);
    appLayout->setContentsMargins(30, 30, 30, 30);
    QLabel* appTitle = new QLabel("Внешний вид", appearancePage);
    appTitle->setStyleSheet("font-size: 22px; font-weight: bold; color: white;");
    appLayout->addWidget(appTitle);
    appLayout->addStretch();

    // Добавляем страницы в StackedWidget
    pagesWidget->addWidget(accountPage);
    pagesWidget->addWidget(appearancePage);
    pagesWidget->addWidget(new QWidget()); // Пустая заглушка для остальных
    pagesWidget->addWidget(new QWidget()); // Пустая заглушка для остальных

    mainLayout->addWidget(sidebarList);
    mainLayout->addWidget(pagesWidget);

    // Связываем клик по списку с переключением страниц
    connect(sidebarList, &QListWidget::currentRowChanged, pagesWidget, &QStackedWidget::setCurrentIndex);
    sidebarList->setCurrentRow(0); // По умолчанию открыта первая вкладка
}

void SettingsDialog::applyStyles() {
    this->setStyleSheet(
        "QDialog { background-color: #36393f; }" // Фон правой части (как в Discord)
        "QListWidget { background-color: #2f3136; color: #8e9297; border: none; font-size: 15px; padding: 15px 10px; }"
        "QListWidget::item { padding: 10px; border-radius: 5px; margin-bottom: 2px; }"
        "QListWidget::item:selected { background-color: #393c43; color: white; }"
        "QListWidget::item:hover:!selected { background-color: #32353a; color: #dcddde; }"
    );
}