#include "CreateUserDialog.h"
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>

// Конструктор: инициализация и настройка диалога создания пользователя
CreateUserDialog::CreateUserDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Создание нового пользователя");
    setFixedSize(300, 150);

    // Поля ввода и чекбокс
    loginEdit = new QLineEdit(this);
    passwordEdit = new QLineEdit(this);
    passwordEdit->setEchoMode(QLineEdit::Password);
    adminCheckBox = new QCheckBox("Сделать администратором", this);

    // Кнопки действий
    createBtn = new QPushButton("Создать", this);
    cancelBtn = new QPushButton("Отмена", this);

    // Подключение кнопок к стандартным слотам принятия/отмены
    connect(createBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    // Размещение элементов с помощью менеджеров компоновки
    QFormLayout *formLayout = new QFormLayout();
    formLayout->addRow("Логин:", loginEdit);
    formLayout->addRow("Пароль:", passwordEdit);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addWidget(createBtn);
    btnLayout->addWidget(cancelBtn);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(adminCheckBox);
    mainLayout->addLayout(btnLayout);
}

// Геттер: возвращает введенный логин
QString CreateUserDialog::getLogin() const { return loginEdit->text(); }

// Геттер: возвращает введенный пароль
QString CreateUserDialog::getPassword() const { return passwordEdit->text(); }

// Геттер: возвращает статус флага администратора
bool CreateUserDialog::isAdmin() const { return adminCheckBox->isChecked(); }