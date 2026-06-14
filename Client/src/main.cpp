#include <QApplication> // ВАЖНО: Меняем QCoreApplication на QApplication
#include "network/MessengerClient.h"
#include "ui/auth/AuthWindow.h" // Подключаем наше новое окно

int main(int argc, char *argv[]) {
    QApplication a(argc, argv); // И здесь тоже QApplication

    // Создаем наш сетевой движок и подключаемся
    MessengerClient client;
    client.connectToServer("127.0.0.1", 8080);

    // Создаем окно авторизации, передаем ему наш движок и показываем на экране
    AuthWindow auth(&client);
    auth.show();

    return a.exec();
}