#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>

class DataBase: public QObject
{
    Q_OBJECT
public:
    explicit DataBase(QSqlDatabase &Dm, QObject *parent = nullptr);
    //출력
    virtual QJsonArray getAll() = 0;
    virtual QJsonObject getById(int id)= 0;
    virtual QJsonObject getByCondition(const QString &cond,const QString &id)= 0;
    //입력
    virtual bool insert(const QJsonObject &data)= 0;
    virtual bool update(int id, const QJsonObject &data)= 0;
    virtual bool remove(int id)= 0;

protected:
    QString       TableName;
    QString       SchemaName;
    qint64        TotalSize;
    QSqlDatabase& m_db;

};

#endif // DATABASE_H
