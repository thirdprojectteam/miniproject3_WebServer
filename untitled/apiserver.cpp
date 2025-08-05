#include "apiserver.h"
#include <QJsonParseError>
#include <QVariant>
#include <QDebug>


ApiServer::ApiServer(Database* db, QObject *parent)
    : QObject(parent), m_server(new QHttpServer(this)),
    m_database(db),tcpServer(new QTcpServer(this)), m_isRunning(false)
{
}

ApiServer::~ApiServer()
{
    stop(); // 서버가 실행 중이면 중지
}

bool ApiServer::start(int port)
{
    if (m_isRunning) {
        qDebug() << "서버가 이미 실행 중입니다.";
        return false;
    }

    // 데이터베이스 연결 확인
    if (!m_database || !m_database->connect("192.168.2.57", "bank", "master", "1107", 3306)) {
        emit errorOccurred("데이터베이스 연결 실패. 서버를 시작할 수 없습니다.");
        return false;
    }

    setupEndpoints(); // API 엔드포인트 설정
    // 수정된 코드 (Qt 6.9.1에서 작동)
    bool success = false;
    quint16 serverPort = 0;
    // 1. QTcpServer를 사용하여 바인딩

    if (tcpServer.listen(QHostAddress::Any, port)) {
        serverPort = tcpServer.serverPort();

        // 2. QHttpServer에 QTcpServer 연결
        success = m_server->bind(&tcpServer);

        if (!success) {
            emit errorOccurred(QString("포트 %1에서 서버를 시작할 수 없습니다.").arg(port));
            return false;
        }
    } else {
        emit errorOccurred(QString("포트 %1에서 TCP 서버를 바인딩할 수 없습니다.").arg(port));
        return false;
    }

    // 성공 시 처리
    m_isRunning = true;
    emit serverStarted(serverPort);
    qDebug() << "API 서버가 성공적으로 시작되었습니다. 포트:" << serverPort;

    return true;
}

void ApiServer::stop()
{
    if (m_isRunning) {
        tcpServer.close();
        delete m_server;// 서버 리스닝 중지
        m_server = new QHttpServer(this);
        m_database->disconnect(); // 데이터베이스 연결 해제
        m_isRunning = false;
        emit serverStopped();
        qDebug() << "API 서버가 중지되었습니다.";
    }
}

void ApiServer::setupEndpoints()
{
    // Health Check Endpoint
    m_server->route("/health", []() {
        return QJsonObject{{"status", "ok"}, {"message", "API Server is running"}};
    });

    // 404 Not Found 처리
    m_server->route("*", [](const QHttpServerRequest &) {
        return QHttpServerResponse("text/plain", QByteArrayLiteral("404 Not Found"), QHttpServerResponse::StatusCode::NotFound);
    });
    //users get
    m_server->route("/api/users", QHttpServerRequest::Method::Get,[this](const QHttpServerRequest &request) { // [this] 캡처를 통해 ApiServer 멤버(m_database)에 접근
        // 데이터베이스 연결 확인
        if (!m_database || !m_database->isConnected) {
            qWarning() << "Database not connected for /api/users request.";
            return QHttpServerResponse("application/json",
                                       QJsonDocument(createResponse(false, "데이터베이스에 연결되지 않았습니다.", QJsonObject{{"code", 500}})).toJson(QJsonDocument::Compact),
                                       QHttpServerResponse::StatusCode::InternalServerError);
        }

        // 'membertbl' 테이블에서 'memberID'가 "Edward"인 데이터 조회
        // 가정: m_database에 getByCondition(tableName, conditionColumn, conditionValue) 같은 메소드가 있다고 가정합니다.
        // 이 메소드는 QJsonObject를 반환하거나 QJsonArray를 반환할 수 있습니다.
        // 여기서는 단일 'Edward' 항목을 가정하고 QJsonObject를 반환하도록 합니다.
        QJsonObject userData = m_database->getByCondition("clientdb", "id", "1");

        if (userData.isEmpty()) {
            if (m_database->lastError().isValid()) { // 조회 중 SQL 에러 발생 시
                qWarning() << "SQL Error retrieving Edward from membertbl:" << m_database->lastError().text();
                return QHttpServerResponse("application/json",
                                           QJsonDocument(createResponse(false, "데이터 조회 실패: " + m_database->lastError().text(), QJsonObject{{"code", 500}})).toJson(QJsonDocument::Compact),
                                           QHttpServerResponse::StatusCode::InternalServerError);
            } else { // 'Edward'를 찾을 수 없을 때
                qDebug() << "Member 'Edward' not found in membertbl.";
                return QHttpServerResponse("application/json",
                                           QJsonDocument(createResponse(false, "사용자 'Edward'를 찾을 수 없습니다.", QJsonObject{{"code", 404}})).toJson(QJsonDocument::Compact),
                                           QHttpServerResponse::StatusCode::NotFound);
            }
        }

        // 성공적으로 데이터를 찾았을 때 JSON 응답 반환
        return QHttpServerResponse("application/json",
                                   QJsonDocument(createResponse(true, "사용자 'Edward' 정보 조회 성공", userData)).toJson(QJsonDocument::Compact));
    });
    //users post 건드린부분
    m_server->route("/api/users", QHttpServerRequest::Method::Post,[this](const QHttpServerRequest &request){
        QJsonParseError parseError;
        QByteArray reqbody = request.body();
        qDebug()<<reqbody;
        QJsonDocument doc = QJsonDocument::fromJson(request.body(), &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            qWarning() << "Failed to parse JSON body:" << parseError.errorString();
            return QHttpServerResponse("application/json",
                                       QJsonDocument(createResponse(false, "잘못된 JSON 형식입니다.", QJsonObject{{"code", 400}})).toJson(QJsonDocument::Compact),
                                       QHttpServerResponse::StatusCode::BadRequest);
        }

        QJsonObject postData = doc.object();
        auto data = postData["data"].toObject();

        qDebug() << "POST Data received:"<<postData;

        for (auto it = data.begin(); it != data.end(); ++it) {
            qDebug() << it.key() << ":" << it.value().toVariant();
        }

        //tablename, jsonobjectfile
        //tablename은 membertable로 따로 보내줘야할듯.
        bool insertSuccess = m_database->insert("clientdb", data);

        if (!insertSuccess) {
            qWarning() << "Failed to insert user:" << m_database->lastError().text();
            return QHttpServerResponse("application/json",
                                       QJsonDocument(createResponse(false, "사용자 추가 실패: " + m_database->lastError().text(), QJsonObject{{"code", 500}})).toJson(QJsonDocument::Compact),
                                       QHttpServerResponse::StatusCode::InternalServerError);
        } else {
            return QHttpServerResponse("application/json",
                                       QJsonDocument(createResponse(true, "사용자 추가 성공", QJsonObject{{"memberTable", "membertbl"}, {"Data", postData}})).toJson(QJsonDocument::Compact),
                                       QHttpServerResponse::StatusCode::Ok);
        }
    });
}

// Private 요청 핸들러 메소드 구현
QJsonObject ApiServer::handleGetItem(const QString &table, int id)
{
    if (!m_database || !m_database->isConnected) {
        return createResponse(false, "데이터베이스에 연결되지 않았습니다.", QJsonObject{{"code", 500}});
    }

    QJsonObject item = m_database->getById(table, id);
    if (item.isEmpty()) {
        if (m_database->lastError().isValid()) { // 조회 중 에러 발생 시
            return createResponse(false, "항목 조회 실패: " + m_database->lastError().text(), QJsonObject{{"code", 500}});
        } else { // 항목을 찾을 수 없을 때
            return createResponse(false, QString("ID %1인 항목을 찾을 수 없습니다.").arg(id), QJsonObject{{"code", 404}});
        }
    }
    return createResponse(true, "항목 조회 성공", item);
}


// 유틸리티 메소드 구현
QJsonObject ApiServer::createResponse(bool success, const QString &message, const QJsonValue &data)
{
    QJsonObject response;
    response["success"] = success;
    response["message"] = message;

    if (!data.isUndefined() && !data.isNull()) {
        response["data"] = data;
    }

    return response;
}

// 이 메소드는 경로에서 ID를 추출하는 데 사용되지 않습니다.
// QHttpServer가 경로 인자를 int로 자동 변환하므로 필요하지 않지만,
// QUrl::path()를 직접 파싱하는 경우를 대비하여 그대로 둡니다.
int ApiServer::extractIdFromPath(const QString &path)
{
    // 예: "/api/products/123" -> 123 추출
    QStringList parts = path.split('/');
    if (parts.isEmpty()) return -1;
    bool ok;
    int id = parts.last().toInt(&ok);
    if (ok) {
        return id;
    }
    return -1; // 유효한 ID가 아님
}

QJsonObject ApiServer::parseRequestBody(const QByteArray& body)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "JSON 파싱 오류:" << parseError.errorString();
        return QJsonObject(); // 빈 객체 반환 또는 오류 처리
    }
    return doc.object();
}
