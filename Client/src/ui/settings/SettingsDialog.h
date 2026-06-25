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

// Диалог настроек с возможностью открытия только вкладки сети (для окна авторизации)
class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    // Конструктор: принимает клиент, родительский виджет и флаг для открытия только вкладки сети
    explicit SettingsDialog(MessengerClient *client, QWidget *parent = nullptr, bool onlyNetwork = false);

private:
    MessengerClient *client;        // Указатель на сетевой клиент для связи с сервером
    bool onlyNetwork;               // Флаг: открывать диалог только с настройками сети (для авторизации)

    QListWidget* sidebarList;       // Боковое меню для выбора вкладок
    QStackedWidget* pagesWidget;    // Стек страниц для переключения между вкладками

    // Виджеты на странице профиля
    QLabel* avatarLabel;            // Метка для отображения аватарки
    QPushButton* changeAvatarBtn;   // Кнопка открытия диалога выбора нового аватара
    QPushButton* saveProfileBtn;    // Кнопка сохранения изменений профиля

    QLineEdit* loginDisplay;        // Поле для отображения логина (только для чтения)
    QLineEdit* firstNameEdit;       // Поле ввода имени
    QLineEdit* lastNameEdit;        // Поле ввода фамилии
    QLineEdit* jobTitleEdit;        // Поле ввода должности
    QPlainTextEdit* bioEdit;        // Поле ввода биографии (многострочное)

    // Виджеты на странице сетевых настроек
    QLineEdit* serverIpEdit;        // Поле ввода IP-адреса сервера
    QLineEdit* portEdit;            // Поле ввода порта сервера
    QPushButton* saveNetworkBtn;    // Кнопка сохранения и применения сетевых настроек
    QPushButton* resetNetworkBtn;   // Кнопка сброса настроек сети к значениям по умолчанию

    // Инициализация и компоновка интерфейса
    void setupUI();
    
    // Применение глобальных стилей к диалогу
    void applyStyles();
    
    // Загрузка данных профиля из клиента (аватарка, имя, фамилия, должность, био)
    void loadProfileData();
    
    // Загрузка сохранённых сетевых настроек из QSettings
    void loadNetworkSettings();
    
    // Установка аватарки-заглушки, если пользователь не загрузил свою
    void loadDefaultAvatarFallback();

private slots:
    // Обработка выбора новой аватарки через диалог файлов
    void onAvatarSelected();
    
    // Сохранение изменений профиля: отправка всех полей на сервер
    void onSaveProfile();
    
    // Сохранение и применение новых сетевых настроек с переподключением к серверу
    void onSaveNetworkSettings();
    
    // Сброс настроек сети до значений по умолчанию (localhost:8080)
    void onResetNetworkSettings();
};