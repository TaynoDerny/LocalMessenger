#include <QCoreApplication>
#include "src/MessengerClient.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    MessengerClient client;
    client.connectToServer("127.0.0.1", 8080);

    return a.exec();
}