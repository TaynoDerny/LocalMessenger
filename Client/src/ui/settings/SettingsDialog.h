#pragma once
#include <QDialog>
#include <QListWidget>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

class MessengerClient; 

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(MessengerClient *client, QWidget *parent = nullptr);

private:
    MessengerClient *client;

    QListWidget* sidebarList;
    QStackedWidget* pagesWidget;

    // Виджеты для профиля
    QLabel* avatarLabel;
    QLabel* usernameLabel;     // <--- Заменили QLineEdit на QLabel
    QPushButton* changeAvatarBtn;
    QPushButton* saveProfileBtn;

    // Виджеты для сети
    QLineEdit* serverIpEdit;
    QLineEdit* portEdit;
    QPushButton* saveNetworkBtn;
    QPushButton* resetNetworkBtn;

    void setupUI();
    void applyStyles();
    void loadDefaultAvatar();
    void loadNetworkSettings();

private slots:
    void onAvatarSelected();
    void onSaveProfile();
    void onSaveNetworkSettings();
    void onResetNetworkSettings();
};