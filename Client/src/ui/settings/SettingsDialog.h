#pragma once
#include <QDialog>
#include <QListWidget>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget *parent = nullptr);

private:
    QListWidget* sidebarList;
    QStackedWidget* pagesWidget;

    // Виджеты для профиля
    QLabel* avatarLabel;
    QLineEdit* usernameEdit;
    QPushButton* changeAvatarBtn;
    QPushButton* saveProfileBtn;

    void setupUI();
    void applyStyles();
    void loadDefaultAvatar();

private slots:
    void onAvatarSelected();
    void onSaveProfile();
};