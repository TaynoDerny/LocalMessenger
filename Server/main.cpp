#include <QCoreApplication>
#include "src/MessengerServer.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv); // Инициализация консольного приложения и цикла событий Qt

    MessengerServer server; // Создание экземпляра сервера для обработки клиентских подключений
    server.start(); // Запуск сервера на порту 8080 (прослушивание входящих соединений)

    return a.exec(); // Запуск главного цикла обработки событий (ожидание подключений)
}