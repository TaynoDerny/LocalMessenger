#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonObject>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>

// Подключаем нашего клиента
class MessengerClient; 

class AdminPanelWidget : public QWidget {
    Q_OBJECT
public:
    explicit AdminPanelWidget(MessengerClient* client, QWidget *parent = nullptr);

public slots:
    void updateTable(const QJsonArray& users);

private slots:
    void applyFilters();
    void onResetPasswordClicked();
    void onWipeUserClicked();

private:
    MessengerClient* client;
    QJsonArray allUsersData; // Сохраняем все данные, чтобы фильтровать локально

    QLineEdit* searchEdit;
    QComboBox* filterCombo;
    QTableWidget* usersTable;
    QPushButton* refreshBtn;
    QPushButton* resetPassBtn;
    QPushButton* wipeUserBtn;

    void setupUI();
};