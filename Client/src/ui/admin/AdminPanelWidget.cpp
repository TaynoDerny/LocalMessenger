#include "AdminPanelWidget.h"
// Укажи правильный путь до твоего MessengerClient.h !
#include "../../network/MessengerClient.h" 

AdminPanelWidget::AdminPanelWidget(MessengerClient* client, QWidget *parent)
    : QWidget(parent), client(client) {
    setupUI();
}

void AdminPanelWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // ВЕРХНЯЯ ПАНЕЛЬ: Поиск и фильтры
    QHBoxLayout* topLayout = new QHBoxLayout();
    
    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("Поиск по логину...");
    connect(searchEdit, &QLineEdit::textChanged, this, &AdminPanelWidget::applyFilters);

    filterCombo = new QComboBox(this);
    filterCombo->addItem("Все пользователи", "all");
    filterCombo->addItem("Только онлайн", "online");
    filterCombo->addItem("Только админы", "admins");
    connect(filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AdminPanelWidget::applyFilters);

    refreshBtn = new QPushButton("Обновить", this);
    connect(refreshBtn, &QPushButton::clicked, client, &MessengerClient::requestAdminData);

    topLayout->addWidget(searchEdit);
    topLayout->addWidget(filterCombo);
    topLayout->addWidget(refreshBtn);

    // ТАБЛИЦА ПОЛЬЗОВАТЕЛЕЙ
    usersTable = new QTableWidget(0, 4, this);
    usersTable->setHorizontalHeaderLabels({"Логин", "Статус", "Роль", "IP Адрес"});
    usersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    usersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    usersTable->setSelectionMode(QAbstractItemView::SingleSelection);
    usersTable->setEditTriggers(QAbstractItemView::NoEditTriggers); // Запрещаем редактировать ячейки руками

    // НИЖНЯЯ ПАНЕЛЬ: Кнопки действий
    QHBoxLayout* bottomLayout = new QHBoxLayout();
    resetPassBtn = new QPushButton("Сбросить пароль", this);
    wipeUserBtn = new QPushButton("Обнулить (Удалить историю)", this);
    wipeUserBtn->setStyleSheet("background-color: #d32f2f; color: white; font-weight: bold;");

    connect(resetPassBtn, &QPushButton::clicked, this, &AdminPanelWidget::onResetPasswordClicked);
    connect(wipeUserBtn, &QPushButton::clicked, this, &AdminPanelWidget::onWipeUserClicked);

    bottomLayout->addWidget(resetPassBtn);
    bottomLayout->addWidget(wipeUserBtn);

    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(usersTable);
    mainLayout->addLayout(bottomLayout);
}

void AdminPanelWidget::updateTable(const QJsonArray& users) {
    allUsersData = users; // Сохраняем для фильтрации
    applyFilters();
}

void AdminPanelWidget::applyFilters() {
    usersTable->setRowCount(0); // Очищаем таблицу

    QString searchText = searchEdit->text().toLower();
    QString filterType = filterCombo->currentData().toString();

    for (const QJsonValue& val : allUsersData) {
        QJsonObject u = val.toObject();
        QString login = u["login"].toString();
        bool isOnline = u["online"].toBool();
        bool isAdmin = u["is_admin"].toBool();
        QString ip = u["ip"].toString();

        // 1. Проверка поиска
        if (!searchText.isEmpty() && !login.toLower().contains(searchText)) continue;

        // 2. Проверка комбобокса
        if (filterType == "online" && !isOnline) continue;
        if (filterType == "admins" && !isAdmin) continue;

        // Если прошли проверки, добавляем строку
        int row = usersTable->rowCount();
        usersTable->insertRow(row);

        usersTable->setItem(row, 0, new QTableWidgetItem(login));
        usersTable->setItem(row, 1, new QTableWidgetItem(isOnline ? "Онлайн" : "Оффлайн"));
        usersTable->setItem(row, 2, new QTableWidgetItem(isAdmin ? "Админ" : "Юзер"));
        usersTable->setItem(row, 3, new QTableWidgetItem(ip));
    }
}

void AdminPanelWidget::onResetPasswordClicked() {
    int row = usersTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Ошибка", "Выберите пользователя в таблице!");
        return;
    }

    QString targetLogin = usersTable->item(row, 0)->text();
    
    bool ok;
    QString newPass = QInputDialog::getText(this, "Сброс пароля", 
                                            "Введите новый пароль для " + targetLogin + ":", 
                                            QLineEdit::Normal, "", &ok);
    if (ok && !newPass.isEmpty()) {
        client->sendResetPassword(targetLogin, newPass);
        QMessageBox::information(this, "Успех", "Запрос на смену пароля отправлен.");
    }
}

void AdminPanelWidget::onWipeUserClicked() {
    int row = usersTable->currentRow();
    if (row < 0) return;

    QString targetLogin = usersTable->item(row, 0)->text();

    QMessageBox::StandardButton reply = QMessageBox::question(this, "Подтверждение", 
        "Вы уверены, что хотите обнулить пользователя " + targetLogin + "?\nВся его история переписки будет удалена!",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        client->sendWipeUser(targetLogin);
        QMessageBox::information(this, "Успех", "Команда обнуления отправлена.");
        client->requestAdminData(); // Сразу просим сервер обновить список
    }
}