#include "dumi_web.h".h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    dumi_web w;
    w.show();
    return a.exec();
}
