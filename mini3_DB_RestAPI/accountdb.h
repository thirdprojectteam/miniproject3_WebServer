#ifndef ACCOUNTDB_H
#define ACCOUNTDB_H

#include "database.h"

class AccountDB : public DataBase
{
public:
    explicit AccountDB(QSqlDatabase &Dm, QObject *parent = nullptr);
};

#endif // ACCOUNTDB_H
