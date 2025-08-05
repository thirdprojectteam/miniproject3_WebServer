#ifndef APISERVER_H
#define APISERVER_H

#include <QHttpServer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QUrlQuery>
#include <QTcpServer>

#include "datamanager.h"
class APIServer : public QObject
{
    Q_OBJECT
public:
    explicit APIServer(DataManager &manager,QObject *parent = nullptr);
    ~APIServer();

    bool start(int port = 8080);
    void stop();

signals:
    void errorOccurred(const QString& message);

private:
    // API 엔드포인트 설정
    void setupEndpoints();

    QHttpServer* httpServer;
    QTcpServer tcpServer; //QTcpServer를 사용하여 바인딩
    DataManager& dbManager;
    bool isRunning;
};

#endif // APISERVER_H
