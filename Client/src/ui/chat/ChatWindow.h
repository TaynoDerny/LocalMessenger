#pragma once
#include <QMainWindow> 
#include <QListWidget>
#include <QScrollArea>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidgetItem> 
#include <QHash>
#include <QStackedWidget>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>

#include "../../network/MessengerClient.h"
#include "../admin/AdminPanelWidget.h"

// Главное окно мессенджера, наследуемое от QMainWindow
class ChatWindow : public QMainWindow {
    Q_OBJECT
public:
    // Конструктор: инициализация клиента и построение интерфейса чата
    explicit ChatWindow(MessengerClient *client, QWidget *parent = nullptr);

private:
    MessengerClient *client;        // Указатель на сетевой клиент для общения с сервером
    QPushButton *adminPanelBtn;     // Кнопка открытия административной панели
    AdminPanelWidget *adminWidget;  // Виджет административной панели

    QListWidget *chatsList;         // Список чатов / пользователей в левой панели
    QStackedWidget *mainArea;       // Стек для переключения между главным экраном, чатом и админ-панелью
    QWidget *homeWidget;            // Приветственный экран при отсутствии выбранного чата
    QWidget *chatWidget;            // Виджет непосредственно чата

    QLabel *chatHeader;             // Заголовок текущего чата с именем собеседника
    
    QScrollArea *messagesArea;      // Область прокрутки для истории сообщений
    QWidget *messagesContainer;     // Контейнер для размещения виджетов сообщений
    QVBoxLayout *messagesLayout;    // Слой для вертикального размещения сообщений
    
    QLineEdit *messageInput;        // Поле ввода текста нового сообщения

    QString currentRecipient;       // Логин текущего собеседника
    
    QHash<QString, QPixmap> userAvatars;      // Кеш аватарок пользователей
    QHash<QString, bool> userAdmins;          // Кеш статусов администратора
    QHash<QString, QString> userDisplayNames; // Кеш отображаемых имён и должностей пользователей

    // Создание круглой аватарки из Base64-строки
    QPixmap createCircularAvatarFromBase64(const QString& base64, int size, bool /*isOnline*/ = false);

    // Добавление виджета сообщения в область чата
    void addMessageToChat(const QString& senderLogin, const QString& text);
    
    // Очистка всех сообщений из области чата
    void clearChatMessages();

private slots:
    // Обработка нажатия Enter: отправка сообщения и очистка поля
    void onSendClicked();
    
    // Обработка получения нового сообщения от сервера
    void onMessageReceived(const QString& sender, const QString& text); 
    
    // Обновление левого списка при получении данных о пользователях
    void onUserListReceived(const QJsonArray& users); 
    
    // Переключение на выбранный чат и запрос истории сообщений
    void onChatSelected(QListWidgetItem *item);  
    
    // Отображение полученной истории переписки
    void onHistoryReceived(const QString& chatWith, const QJsonArray& messages);
};