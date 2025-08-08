#ifndef ATMLOGDB_H
#define ATMLOGDB_H

#include "database.h"

class AtmLogDB : public DataBase
{
public:
    AtmLogDB();
    ~AtmLogDB();
    //출력
    QJsonArray  getAll() override;
    QJsonObject getById(int id) override;
    QJsonObject getByCondition(const QString &cond,const QString &id) override;
    QJsonObject getLatest() override;
    //입력
    bool insert(const QJsonObject &data) override;
    bool update(int id, const QJsonObject &data) override;
    bool remove(int id) override;

private:
    QString buildUpdateQuery(const QString &table, int id, const QJsonObject &data) override;
    QString buildInsertQuery(const QString &table, const QJsonObject &data) override;
    void bindJsonToQuery(QSqlQuery &query, const QJsonObject &data) override;
};
#endif // ATMLOGDB_H
