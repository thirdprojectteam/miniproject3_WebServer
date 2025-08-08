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
    endpoints.registerDb("atmlogdb",&atmlogdb);
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
    // ----- webclient GET
    httpServer->route("/client/<arg>",\
    QHttpServerRequest::Method::Get,[this](const QString &table) -> QFuture<QHttpServerResponse>{
        return response.asyncResponse(table); });

    // ----- webclient GET LATEST
    httpServer->route("/client/<arg>/latest",\
    QHttpServerRequest::Method::Get,[this](const QString &table) -> QFuture<QHttpServerResponse>{
        return response.asyncResponse(table); });

    // ----- webclient POST
    httpServer->route("/client/<arg>",\
    QHttpServerRequest::Method::Post,[this](const QString &table,const QHttpServerRequest &request) -> QFuture<QHttpServerResponse>{
        bool isInsert = endpoints.InsertSuccess(request,table);
        return response.asyncPostResponse(table,isInsert); });

    // ----- raspberry pi GET
    httpServer->route("/api/atm", QHttpServerRequest::Method::Get,[this](const QHttpServerRequest &request) { // [this] 캡처를 통해 ApiServer 멤버(m_database)에 접근
        //query문 받기.
        QUrlQuery query(request.url());
        QString uid = query.queryItemValue("uid");
        QString name = query.queryItemValue("name");
        QString Ruid;
        if(uid=="B1457D09"){
            Ruid="12345678";
        }else if(uid=="F3CC65BD"){
            Ruid="87654321";
        }else {
            Ruid="-1";
        }
        qDebug() << "Received Query Parameters: uid=" << Ruid << ", name=" << name;
        return response.asyncResponseWhere("accountdb",Ruid,name);});

    // ----- raspberry pi POST
    httpServer->route("/api/atm", QHttpServerRequest::Method::Post,[this](const QHttpServerRequest &request){
        bool isInsert = endpoints.InsertSuccess(request,"atmlogdb");
        return response.asyncPostResponse("atmlogdb",isInsert);});

    // ----- raspberry pi PUT
    httpServer->route("/api/atm", QHttpServerRequest::Method::Put,[this](const QHttpServerRequest &request){
        bool isUpdate = endpoints.UpdateSuccess(request,"accountdb");
        return response.asyncPostResponse("accountdb",isUpdate);});
}

