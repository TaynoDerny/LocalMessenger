#include <QCoreApplication>
#include "src/MessengerServer.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    MessengerServer server;
    server.start();

    return a.exec();
}