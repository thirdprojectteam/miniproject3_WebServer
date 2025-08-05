#include "apiserver.h"

APIServer::APIServer(DataManager &manager,QObject *parent)
    : QObject(parent), httpServer(new QHttpServer(this)),
    tcpServer(new QTcpServer(this)),dbManager(manager), isRunning(false)
{}

APIServer::~APIServer()
{

}

bool APIServer::start(int port)
{
    if (isRunning) {
        qDebug() << "서버가 이미 실행 중입니다.";
        return false;
    }

    // 데이터베이스 연결 확인
    if (!dbManager.connect("192.168.2.57", "bank", "master", "1107", 3306)) {
        emit errorOccurred("데이터베이스 연결 실패. 서버를 시작할 수 없습니다.");
        return false;
    }

    setupEndpoints(); // API 엔드포인트 설정

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
        dbManager.disconnect(); // 데이터베이스 연결 해제
        isRunning = false;
        //emit serverStopped();
        qDebug() << "API 서버가 중지되었습니다.";
    }
}

void APIServer::setupEndpoints()
{
    // httpServer->route("/api/users", QHttpServerRequest::Method::Get, [](const QHttpServerRequest &req)
    //                 {
    //                  // 1. 메서드는 route에서 이미 확인됨 (POST)

    //                  // 2. 헤더 확인
    //                  QString contentType = QString::fromUtf8(req.headers().value("Content-Type"));
    //                  qDebug() << "Content-Type:" << contentType;

    //                  // 3. 바디 확인
    //                  QByteArray body = req.body();
    //                  qDebug() << "Body:" << body;

    //                  return QHttpServerResponse("application/json", "{\"status\":\"ok\"}");
    //              });
    QJsonObject response;
    response["success"] = "success";
    response["message"] = "message";

    httpServer->route("/api/users", [this, response]() {
        qDebug() << "받아랑";

        return QHttpServerResponse(
            "application/json",
            QJsonDocument(response).toJson(QJsonDocument::Compact));
    });

}
