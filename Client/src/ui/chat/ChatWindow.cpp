#include "ChatWindow.h"

ChatWindow::ChatWindow(MessengerClient *client, QWidget *parent)
    : QWidget(parent), client(client) {

    setWindowTitle("Мессенджер");
    resize(800, 600); // Стартовый размер как у полноценного приложения

    // Главный горизонтальный слой (делит окно на Лево и Право)
    QHBoxLayout *mainLayout = new QHBoxLayout(this);

    // ================= ЛЕВАЯ ПАНЕЛЬ (Список каналов/чатов) =================
    QVBoxLayout *leftLayout = new QVBoxLayout();
    
    QLabel *leftHeader = new QLabel("Чаты и Группы", this);
    chatsList = new QListWidget(this);
    chatsList->setObjectName("chatsList");

    // Кнопка создания групп (пока просто добавляем в каркас)
    createGroupButton = new QPushButton("Создать группу", this);
    createGroupButton->setObjectName("createGroupButton");

    leftLayout->addWidget(leftHeader);
    leftLayout->addWidget(chatsList);
    leftLayout->addWidget(createGroupButton);

    // ================= ПРАВАЯ ПАНЕЛЬ (Окно переписки) =================
    QVBoxLayout *rightLayout = new QVBoxLayout();

    chatHeader = new QLabel("Выберите чат для начала общения", this);
    
    messagesDisplay = new QTextEdit(this);
    messagesDisplay->setReadOnly(true); // Запрещаем юзеру редактировать историю чата
    messagesDisplay->setObjectName("messagesDisplay");

    // Нижняя строчка: поле ввода + кнопка отправки
    QHBoxLayout *inputLayout = new QHBoxLayout();
    messageInput = new QLineEdit(this);
    messageInput->setPlaceholderText("Напишите сообщение...");
    messageInput->setObjectName("messageInput");

    sendButton = new QPushButton("Отправить", this);
    sendButton->setObjectName("sendButton");

    inputLayout->addWidget(messageInput);
    inputLayout->addWidget(sendButton);

    // Собираем правую панель воедино
    rightLayout->addWidget(chatHeader);
    rightLayout->addWidget(messagesDisplay);
    rightLayout->addLayout(inputLayout); 

    // ================= СБОРКА ВСЕГО ОКНА =================
    // Добавляем слои в главный слой и задаем пропорции (левый займет 1 часть, правый — 3 части)
    mainLayout->addLayout(leftLayout, 1);
    mainLayout->addLayout(rightLayout, 3);

    setLayout(mainLayout);

    // Коннектим кнопку отправки
    connect(sendButton, &QPushButton::clicked, this, &ChatWindow::onSendClicked);
}

void ChatWindow::onSendClicked() {
    QString text = messageInput->text();
    if (!text.isEmpty()) {
        client->sendMessage(text);
        messageInput->clear(); // Очищаем поле после отправки
    }
}