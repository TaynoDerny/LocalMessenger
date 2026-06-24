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
#include <QMenu>
#include <QAction>

class MessengerClient; 

class AdminPanelWidget : public QWidget {
    Q_OBJECT
public:
    explicit AdminPanelWidget(MessengerClient* client, QWidget *parent = nullptr);

public slots:
    void updateTable(const QJsonArray& users);

private:
    MessengerClient* client;
    QJsonArray allUsersData;

    QLineEdit* searchEdit;
    QComboBox* filterCombo;
    QTableWidget* usersTable;
    QPushButton* refreshBtn;
    QPushButton* createUserBtn; // Новая кнопка

    void setupUI();

private slots:
    void applyFilters();
    void showContextMenu(const QPoint& pos); // Слот для открытия меню
};