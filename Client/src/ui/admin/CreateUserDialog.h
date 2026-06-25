#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>

// Диалог для создания нового пользователя администратором
class CreateUserDialog : public QDialog {
    Q_OBJECT
public:
    // Конструктор: инициализация и настройка диалогового окна
    explicit CreateUserDialog(QWidget *parent = nullptr);
    
    // Геттер: возвращает введённый логин
    QString getLogin() const;
    
    // Геттер: возвращает введённый пароль
    QString getPassword() const;
    
    // Геттер: возвращает true, если выбран флаг администратора
    bool isAdmin() const;

private:
    QLineEdit *loginEdit;       // Поле ввода логина
    QLineEdit *passwordEdit;    // Поле ввода пароля (скрытое)
    QCheckBox *adminCheckBox;   // Чекбокс: сделать администратором
    QPushButton *createBtn;     // Кнопка "Создать"
    QPushButton *cancelBtn;     // Кнопка "Отмена"
};