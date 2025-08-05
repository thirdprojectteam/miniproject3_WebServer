#include <QApplication>
#include "datamanager.h"
#include "apiserver.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //Widget w;
    //w.show();
    DataManager db;
    APIServer apis(db);
    apis.start();
    return a.exec();
}
