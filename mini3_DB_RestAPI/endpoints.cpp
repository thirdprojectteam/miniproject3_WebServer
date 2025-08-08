#include "endpoints.h"

#include <QHttpServerRequest>
#include <QJsonObject>

EndPoints::EndPoints(){}
EndPoints::~EndPoints(){}

void EndPoints::registerDb(const QString &name, DataBase *db)
{
    dbMap[name] = db;
}

QHttpServerResponse EndPoints::buildResponse(const QString &table) const
{
    qDebug() << "EndPoints buildResponse start";
    QJsonDocument doc;
    if (auto it = dbMap.find(table); it != dbMap.end()) {
        doc = QJsonDocument(it.value()->getAll());
        qDebug() << "EndPoints get data : "<<doc.toJson();
    }
    return QHttpServerResponse{
        "application/json",
        doc.toJson(QJsonDocument::Compact)
    };
}

QHttpServerResponse EndPoints::buildResponseWhere(const QString &table, QString con1, QString con2) const
{
    qDebug() << "EndPoints Where start";
    QJsonDocument doc;
    if (auto it = dbMap.find(table); it != dbMap.end()) {
        doc = QJsonDocument(it.value()->getByCondition(con1,con2));
        qDebug() << "EndPoints Where data : "<<doc.toJson();
    }
    return QHttpServerResponse{
        "application/json",
        doc.toJson(QJsonDocument::Compact)
    };
}

QHttpServerResponse EndPoints::buildResponseRecent(const QString &table) const
{
    qDebug() << "EndPoints buildResponseRecent start, table=" << table;
    QJsonDocument doc;
    if (auto it = dbMap.find(table); it != dbMap.end()) {
        doc = QJsonDocument(it.value()->getLatest());
        qDebug() << "EndPoints recent data:" << doc.toJson();
    }
    return QHttpServerResponse{
        "application/json",
        doc.toJson(QJsonDocument::Compact)
    };
}

QHttpServerResponse EndPoints::buildPostResponse(const QString &table, const bool &isPost) const
{
    QJsonDocument doc;
    if (!isPost) {
        qWarning() << "Failed to insert";

        // 실패 응답 객체
        QJsonObject res;
        res["success"] = false;
        res["message"] = QStringLiteral("사용자 추가 실패");
        res["code"]    = 500;

        doc = QJsonDocument(res);
        return QHttpServerResponse{
            "application/json",
            doc.toJson(QJsonDocument::Compact),
            QHttpServerResponse::StatusCode::InternalServerError
        };
    } else {
        // 성공 응답 객체
        QJsonObject res;
        res["success"]     = true;
        res["message"]     = QStringLiteral("사용자 추가 성공");
        res["memberTable"] = table;   // POST한 테이블 이름
        res["code"]        = 200;

        doc = QJsonDocument(res);
        // 기본 상태코드(OK) 사용
        return QHttpServerResponse{
            "application/json",
            doc.toJson(QJsonDocument::Compact)
        };
    }
}

bool EndPoints::InsertSuccess(const QHttpServerRequest &request, const QString &table)
{
    QJsonParseError parseError;
    QByteArray reqbody = request.body();
    bool insertSuccess = false;
    qDebug()<<reqbody;
    QJsonDocument doc = QJsonDocument::fromJson(request.body(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse JSON body:" << parseError.errorString();
        return false;
    }
    QJsonObject postData = doc.object();
    auto data = postData["data"].toObject();

    qDebug() << "POST Data received:"<<postData;

    for (auto it = data.begin(); it != data.end(); ++it) {
        qDebug() << it.key() << ":" << it.value().toVariant();
    }

    if (auto it = dbMap.find(table); it != dbMap.end()) {
        insertSuccess = it.value()->insert(data);
    }
    return insertSuccess;
}

bool EndPoints::UpdateSuccess(const QHttpServerRequest &request, const QString &table)
{
    QByteArray reqbody = request.body();
    qDebug()<<reqbody;
    QJsonDocument doc = QJsonDocument::fromJson(request.body());

    QJsonObject putData = doc.object();
    auto data = putData["data"].toObject();

    //디버그 문구
    qDebug() << "put Data received:"<<putData;
    for (auto it = data.begin(); it != data.end(); ++it) {
        qDebug() << it.key() << ":" << it.value().toVariant();
    }
    //put은 동작이 3개
    bool insertSuccess=0;
    QString action = data["action"].toString();
    int id = 0;
    if(action=="Deposit"){
        //덧셈
        id = 0;
    }else if(action == "Withdraw"){
        //뺄셈
        id = 1;
    }else if(action == "Send"){
        //put 2개 동시.
        id = 2;
    }
    if (auto it = dbMap.find(table); it != dbMap.end()) {
        insertSuccess = it.value()->update(id,putData);
    }
    return insertSuccess;

}
