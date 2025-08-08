#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QJsonArray>
class DataBase: public QObject
{
    Q_OBJECT
public:
    explicit DataBase(QObject *parent = nullptr);
    //출력
    virtual QJsonArray  getAll() = 0;
    virtual QJsonObject getLatest() = 0;
    virtual QJsonObject getById(int id)= 0;
    virtual QJsonObject getByCondition(const QString &cond,const QString &id)= 0;
    //입력
    virtual bool insert(const QJsonObject &data)= 0;
    virtual bool update(int id, const QJsonObject &data)= 0;
    virtual bool remove(int id)= 0;

protected:
    QString       TableName;
    qint64        TotalSize;
    QSqlError     m_lastError;

signals:
    void Finish(bool success, const QString &operation, const QSqlError &error = QSqlError());

private:
    // 유틸리티 함수
    virtual QString buildUpdateQuery(const QString &table, int id, const QJsonObject &data);
    virtual QString buildInsertQuery(const QString &table, const QJsonObject &data);
    virtual void bindJsonToQuery(QSqlQuery &query, const QJsonObject &data);

};

#endif // DATABASE_H
