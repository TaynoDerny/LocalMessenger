#include <QApplication>
#include <QFile> // Для работы с файлами
#include "network/MessengerClient.h"
#include "ui/auth/AuthWindow.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // ---- ПОДКЛЮЧАЕМ НАШИ СТИЛИ ИЗ ФИГМЫ ----
    QFile styleFile(":/style.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString style = QLatin1String(styleFile.readAll());
        a.setStyleSheet(style); // Применяем стиль ко всему приложению!
        styleFile.close();
    } else {
        qDebug() << "Не удалось найти файл style.qss";
    }
    // ----------------------------------------

    MessengerClient client;
    client.connectToServer("127.0.0.1", 8080);

    AuthWindow auth(&client);
    auth.show();

    return a.exec();
}