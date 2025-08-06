#include <QApplication>
#include "datamanager.h"
#include "apiserver.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //Widget w;
    //w.show();
    DataManager::instance().connect("192.168.2.57", "bank", "master", "1107", 3306);
    APIServer apis;
    apis.start();
    return a.exec();
}
