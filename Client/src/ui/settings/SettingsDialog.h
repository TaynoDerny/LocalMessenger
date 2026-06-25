#pragma once
#include <QDialog>
#include <QListWidget>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit> 
#include <QPushButton>

class MessengerClient; 

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    // ========== ДОБАВИЛИ ПАРАМЕТР onlyNetwork ==========
    explicit SettingsDialog(MessengerClient *client, QWidget *parent = nullptr, bool onlyNetwork = false);

private:
    MessengerClient *client;
    bool onlyNetwork; // <--- Переменная для флага

    QListWidget* sidebarList;
    QStackedWidget* pagesWidget;

    // Виджеты для профиля
    QLabel* avatarLabel;
    QPushButton* changeAvatarBtn;
    QPushButton* saveProfileBtn;

    QLineEdit* loginDisplay;
    QLineEdit* firstNameEdit;
    QLineEdit* lastNameEdit;
    QLineEdit* jobTitleEdit;
    QPlainTextEdit* bioEdit;

    // Виджеты для сети
    QLineEdit* serverIpEdit;
    QLineEdit* portEdit;
    QPushButton* saveNetworkBtn;
    QPushButton* resetNetworkBtn;

    void setupUI();
    void applyStyles();
    void loadProfileData();
    void loadNetworkSettings();
    void loadDefaultAvatarFallback(); 

private slots:
    void onAvatarSelected();
    void onSaveProfile();
    void onSaveNetworkSettings();
    void onResetNetworkSettings();
};