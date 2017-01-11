#include "cn_client.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    clientMainWindow w;
    w.show();

    return a.exec();
}
