#include "webclient.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    WebClient w;
    w.show();
    return a.exec();
}
