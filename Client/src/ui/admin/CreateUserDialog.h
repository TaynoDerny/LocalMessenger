#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>

class CreateUserDialog : public QDialog {
    Q_OBJECT
public:
    explicit CreateUserDialog(QWidget *parent = nullptr);
    
    QString getLogin() const;
    QString getPassword() const;
    bool isAdmin() const;

private:
    QLineEdit *loginEdit;
    QLineEdit *passwordEdit;
    QCheckBox *adminCheckBox;
    QPushButton *createBtn;
    QPushButton *cancelBtn;
};