#ifndef APISERVER_H
#define APISERVER_H

#include <QObject>
#include <QHttpServer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QUrlQuery>

#include <QTcpServer>
#include "database.h"

class ApiServer : public QObject
{
    Q_OBJECT
public:
    explicit ApiServer(Database* db, QObject *parent = nullptr);
    ~ApiServer();

    bool start(int port = 8080);
    void stop();

    // API 엔드포인트 설정
    void setupEndpoints();

signals:
    void serverStarted(int port);
    void serverStopped();
    void requestReceived(const QString& endpoint, const QJsonObject& data);
    void errorOccurred(const QString& message);

private:
    QHttpServer* m_server;
    Database* m_database;
    QTcpServer tcpServer;
    bool m_isRunning;

    // 요청 핸들러 메소드
    QJsonObject handleGetItem(const QString& table, int id);

    // 유틸리티 메소드
    QJsonObject createResponse(bool success, const QString& message, const QJsonValue& data = QJsonValue());
    int extractIdFromPath(const QString& path);
    QJsonObject parseRequestBody(const QByteArray& body);
};

#endif // APISERVER_H
