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

class DataManager : public QObject
{
    Q_OBJECT
public:
    explicit DataManager(QObject *parent = nullptr);
    ~DataManager();

    // 데이터베이스 연결 및 종료
    bool connect(const QString &hostname, const QString &dbName,
                 const QString &username, const QString &password, int port = 3306);
    void disconnect();

private:
    QSqlDatabase m_db;
    QSqlError m_lastError;

    // 연결 상태 확인
    bool isConnected;
};

#endif // DATAMANAGER_H
