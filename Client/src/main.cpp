#include <QApplication>
#include <QFile>
#include "network/MessengerClient.h"
#include "ui/auth/AuthWindow.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv); // Инициализация приложения Qt

    // Загрузка и применение глобальных стилей (QSS)
    QFile styleFile(":/style.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString style = QLatin1String(styleFile.readAll());
        a.setStyleSheet(style);
        styleFile.close();
    }

    MessengerClient client; // Создание экземпляра клиента для связи с сервером
    // Подключение к серверу происходит внутри окна авторизации при его открытии
    
    AuthWindow *auth = new AuthWindow(&client); // Создание и отображение окна входа
    auth->show();

    return a.exec(); // Запуск главного цикла событий приложения
}