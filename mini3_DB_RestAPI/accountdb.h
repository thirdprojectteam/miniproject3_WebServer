#ifndef ACCOUNTDB_H
#define ACCOUNTDB_H

#include <database.h>

class AccountDB : public Database
{
public:
    explicit AccountDB(QObject *parent = nullptr);
};

#endif // ACCOUNTDB_H
