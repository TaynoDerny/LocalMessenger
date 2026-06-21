#include <QApplication>
#include <QFile>
#include "network/MessengerClient.h"
#include "ui/auth/AuthWindow.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // Подключение стилей...
    QFile styleFile(":/style.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString style = QLatin1String(styleFile.readAll());
        a.setStyleSheet(style);
        styleFile.close();
    }

    MessengerClient client;
    client.connectToServer("127.0.0.1", 8080);

    // ИСПРАВЛЕНО: Создаем через new
    AuthWindow *auth = new AuthWindow(&client);
    auth->show();

    return a.exec();
}