#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonDocument>
#include <QDebug>
#include <QVariant>

class Database : public QObject
{
    Q_OBJECT
public:
    explicit Database(QObject *parent = nullptr);
    ~Database();

    // 데이터베이스 연결 및 종료
    bool connect(const QString &hostname, const QString &dbName,
                 const QString &username, const QString &password, int port = 3306);
    void disconnect();

    // CRUD 연산
    QJsonArray getAll(const QString &table);
    QJsonObject getById(const QString &table, int id);
    QJsonObject getByCondition(const QString &table,const QString &header,const QString &id);
    bool insert(const QString &table, const QJsonObject &data);
    bool update(const QString &table, int id, const QJsonObject &data);
    bool remove(const QString &table, int id);

    // 에러 처리
    QSqlError lastError() const;

    // 연결 상태 확인
    bool isConnected;

signals:
    void connectionStatusChanged(bool connected);
    void operationCompleted(bool success, const QString &operation, const QSqlError &error = QSqlError());

private:
    QSqlDatabase m_db;
    QSqlError m_lastError;

    // 유틸리티 함수
    QString buildUpdateQuery(const QString &table, int id, const QJsonObject &data);
    QString buildInsertQuery(const QString &table, const QJsonObject &data);
    void bindJsonToQuery(QSqlQuery &query, const QJsonObject &data);
};

#endif // DATABASE_H
