#include "accountdb.h"

AccountDB::AccountDB(QSqlDatabase &Dm,QObject *parent)
    : DataBase(Dm,parent)
{

}
