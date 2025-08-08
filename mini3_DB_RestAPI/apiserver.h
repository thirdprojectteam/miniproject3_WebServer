#ifndef APISERVER_H
#define APISERVER_H

#include <QHttpServer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QUrlQuery>
#include <QTcpServer>

#include "clientdb.h"
#include "accountdb.h"
#include "announcedb.h"
#include "announcelogdb.h"
#include "atmlogdb.h"
#include "endpoints.h"
#include "responses.h"

class APIServer : public QObject
{
    Q_OBJECT
public:
    explicit APIServer(QObject *parent = nullptr);
    ~APIServer();

    bool start(int port = 8080);
    void stop();

signals:
    void errorOccurred(const QString& message);

private:
    // API 엔드포인트 설정
    void setupRoutes();

    QHttpServer*  httpServer;
    QTcpServer    tcpServer; //QTcpServer를 사용하여 바인딩
    bool          isRunning;
    ClientDB      clientdb;
    AccountDB     accdb;
    AnnounceDB    anndb;
    AnnounceLogDB annlogdb;
    AtmLogDB      atmlogdb;
    EndPoints     endpoints;
    Responses     response;
};

#endif // APISERVER_H
