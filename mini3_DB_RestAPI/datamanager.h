#ifndef DATAMANAGER_H
#define DATAMANAGER_H

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

class DataManager
{
public:
    static DataManager& instance(){
        static DataManager inst;
        return inst;
    }
    // 데이터베이스 연결 및 종료
    bool connect(const QString &hostname, const QString &dbName,
                 const QString &username, const QString &password, int port = 3306);
    void disconnect();
    QSqlDatabase createThreadConnection(QString &connName);
    void closeThreadConnection(QString &connName);
    QString getConnectionName();
    QSqlDatabase getDB();

private:
    DataManager() = default;
    ~DataManager();
    DataManager(const DataManager&) = delete;
    DataManager& operator=(const DataManager&) = delete;
    QSqlDatabase m_db;
    QSqlError m_lastError;
    QString h_n;
    QString d_n;
    QString u_n;
    QString pass;
    int p;

    // 연결 상태 확인
    bool isConnected;
};

#endif // DATAMANAGER_H
