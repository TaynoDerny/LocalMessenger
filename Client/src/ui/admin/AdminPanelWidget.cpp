#include "AdminPanelWidget.h"
#include "../../network/MessengerClient.h" 
#include "CreateUserDialog.h" // <-- Добавили диалог создания

AdminPanelWidget::AdminPanelWidget(MessengerClient* client, QWidget *parent)
    : QWidget(parent), client(client) {
    setupUI();
}

void AdminPanelWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // ВЕРХНЯЯ ПАНЕЛЬ: Поиск, фильтры, и кнопки
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

    // ==== НОВАЯ КНОПКА ЗДЕСЬ ====
    createUserBtn = new QPushButton("Создать пользователя", this);
    connect(createUserBtn, &QPushButton::clicked, this, [this]() {
        CreateUserDialog dialog(this);
        if (dialog.exec() == QDialog::Accepted) {
            QString login = dialog.getLogin();
            QString password = dialog.getPassword();
            bool isAdmin = dialog.isAdmin();
            client->createAccount(login, password, isAdmin);
        }
    });

    topLayout->addWidget(searchEdit);
    topLayout->addWidget(filterCombo);
    topLayout->addWidget(refreshBtn);
    topLayout->addWidget(createUserBtn); // Добавляем в раскладку

    // ТАБЛИЦА ПОЛЬЗОВАТЕЛЕЙ
    usersTable = new QTableWidget(0, 4, this);
    usersTable->setHorizontalHeaderLabels({"Логин", "Статус", "Роль", "IP Адрес"});
    usersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    usersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    usersTable->setSelectionMode(QAbstractItemView::SingleSelection);
    usersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // ==== ДОБАВЛЯЕМ КОНТЕКСТНОЕ МЕНЮ ====
    usersTable->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(usersTable, &QTableWidget::customContextMenuRequested, this, &AdminPanelWidget::showContextMenu);

    // СБОРКА ИНТЕРФЕЙСА
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(usersTable);
    // Нижняя панель с кнопками (reset / wipe) убрана полностью!
}

void AdminPanelWidget::updateTable(const QJsonArray& users) {
    allUsersData = users; 
    applyFilters();
}

void AdminPanelWidget::applyFilters() {
    usersTable->setRowCount(0); 

    QString searchText = searchEdit->text().toLower();
    QString filterType = filterCombo->currentData().toString();

    for (const QJsonValue& val : allUsersData) {
        QJsonObject u = val.toObject();
        QString login = u["login"].toString();
        bool isOnline = u["online"].toBool();
        bool isAdmin = u["is_admin"].toBool();
        QString ip = u["ip"].toString();

        if (!searchText.isEmpty() && !login.toLower().contains(searchText)) continue;
        if (filterType == "online" && !isOnline) continue;
        if (filterType == "admins" && !isAdmin) continue;

        int row = usersTable->rowCount();
        usersTable->insertRow(row);

        usersTable->setItem(row, 0, new QTableWidgetItem(login));
        usersTable->setItem(row, 1, new QTableWidgetItem(isOnline ? "Онлайн" : "Оффлайн"));
        usersTable->setItem(row, 2, new QTableWidgetItem(isAdmin ? "Админ" : "Юзер"));
        usersTable->setItem(row, 3, new QTableWidgetItem(ip));
    }
}

// ==== НОВЫЙ МЕТОД: ОБРАБОТКА ПРАВОГО КЛИКА ====
void AdminPanelWidget::showContextMenu(const QPoint& pos) {
    QModelIndex index = usersTable->indexAt(pos);
    if (!index.isValid()) return;

    int row = index.row();
    QString login = usersTable->item(row, 0)->text();

    QMenu contextMenu(this);
    QAction *resetAction = contextMenu.addAction("Сбросить пароль");
    QAction *wipeAction = contextMenu.addAction("Обнулить (Удалить историю)");

    // Логика для "Сбросить пароль"
    connect(resetAction, &QAction::triggered, this, [this, login]() {
        bool ok;
        QString newPass = QInputDialog::getText(this, "Сброс пароля", 
                                                "Введите новый пароль для " + login + ":", 
                                                QLineEdit::Normal, "", &ok);
        if (ok && !newPass.isEmpty()) {
            client->sendResetPassword(login, newPass);
            QMessageBox::information(this, "Успех", "Запрос на смену пароля отправлен.");
        }
    });

    // Логика для "Обнулить (Удалить историю)"
    connect(wipeAction, &QAction::triggered, this, [this, login]() {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Подтверждение", 
            "Вы уверены, что хотите обнулить пользователя " + login + "?\nВся его история переписки будет удалена!",
            QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            client->sendWipeUser(login);
            QMessageBox::information(this, "Успех", "Команда обнуления отправлена.");
            client->requestAdminData(); 
        }
    });

    contextMenu.exec(usersTable->viewport()->mapToGlobal(pos));
}