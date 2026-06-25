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

// Виджет административной панели для управления пользователями
class AdminPanelWidget : public QWidget {
    Q_OBJECT
public:
    // Конструктор: принимает клиент для отправки команд и родительский виджет
    explicit AdminPanelWidget(MessengerClient* client, QWidget *parent = nullptr);

public slots:
    // Обновление таблицы при получении нового списка пользователей от сервера
    void updateTable(const QJsonArray& users);

private:
    MessengerClient* client;        // Указатель на клиент для связи с сервером
    QJsonArray allUsersData;        // Кеш всех данных о пользователях

    QLineEdit* searchEdit;          // Поле поиска по логину
    QComboBox* filterCombo;         // Выпадающий список для фильтрации (все/онлайн/админы)
    QTableWidget* usersTable;       // Таблица для отображения пользователей
    QPushButton* refreshBtn;        // Кнопка обновления данных
    QPushButton* createUserBtn;     // Кнопка создания нового пользователя

    // Инициализация и компоновка интерфейса
    void setupUI();

private slots:
    // Применение текущих фильтров (поиск и выбор категории)
    void applyFilters();
    
    // Открытие контекстного меню при правом клике по таблице
    void showContextMenu(const QPoint& pos);
};