#include "CreateUserDialog.h"
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>

CreateUserDialog::CreateUserDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Создание нового пользователя");
    setFixedSize(300, 150); // Фиксируем размер, чтобы не растягивали

    loginEdit = new QLineEdit(this);
    passwordEdit = new QLineEdit(this);
    passwordEdit->setEchoMode(QLineEdit::Password); // Скрываем символы пароля
    adminCheckBox = new QCheckBox("Сделать администратором", this);

    createBtn = new QPushButton("Создать", this);
    cancelBtn = new QPushButton("Отмена", this);

    // Привязываем кнопки к встроенным слотам диалога (Принять / Отклонить)
    connect(createBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    // Расположение элементов
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
QString CreateUserDialog::getLogin() const { return loginEdit->text(); }
QString CreateUserDialog::getPassword() const { return passwordEdit->text(); }
bool CreateUserDialog::isAdmin() const { return adminCheckBox->isChecked(); }