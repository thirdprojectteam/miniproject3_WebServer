#include "apiserver.h"

#include <QSqlDatabase>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
APIServer::APIServer(QObject *parent)
    : QObject(parent), httpServer(new QHttpServer(this)),
    tcpServer(new QTcpServer(this)), isRunning(false),response(endpoints)
{
    // 1) Endpoints에 DB 객체 등록
    endpoints.registerDb("clientdb", &clientdb);
    endpoints.registerDb("accountdb",&accdb);
    endpoints.registerDb("announcedb",&anndb);
    endpoints.registerDb("announcelogdb",&annlogdb);
}

APIServer::~APIServer()
{}

bool APIServer::start(int port)
{
    if (isRunning) {
        qDebug() << "서버가 이미 실행 중입니다.";
        return false;
    }

    setupRoutes(); // API 엔드포인트 설정

    bool success = false;
    quint16 serverPort = 0;
    // QTcpServer를 사용하여 바인딩
    if (tcpServer.listen(QHostAddress::Any, port)) {
        serverPort = tcpServer.serverPort();

        // QHttpServer에 QTcpServer 연결
        success = httpServer->bind(&tcpServer);

        if (!success) {
            emit errorOccurred(QString("포트 %1에서 서버를 시작할 수 없습니다.").arg(port));
            return false;
        }
    } else {
        emit errorOccurred(QString("포트 %1에서 TCP 서버를 바인딩할 수 없습니다.").arg(port));
        return false;
    }

    // 성공 시 처리
    isRunning = true;
    //emit serverStarted(serverPort);
    qDebug() << "API 서버가 성공적으로 시작되었습니다. 포트:" << serverPort;

    return true;
}

void APIServer::stop()
{
    if (isRunning) {
        tcpServer.close();
        delete httpServer;// 서버 리스닝 중지
        httpServer = new QHttpServer(this);
        isRunning = false;
        //emit serverStopped();
        qDebug() << "API 서버가 중지되었습니다.";
    }
}

void APIServer::setupRoutes()
{
    httpServer->route("/client/<arg>",\
    QHttpServerRequest::Method::Get,[this](const QString &table) -> QFuture<QHttpServerResponse>{
        return response.asyncResponse(table); });

    httpServer->route("/client/<arg>",\
    QHttpServerRequest::Method::Post,[this](const QString &table,const QHttpServerRequest &request) -> QFuture<QHttpServerResponse>{
        bool isInsert = endpoints.InsertSuccess(request,table);
        return response.asyncPostResponse(table,isInsert); });
}

