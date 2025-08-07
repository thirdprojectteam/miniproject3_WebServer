#include "endpoints.h"

#include <QHttpServerRequest>
#include <QJsonObject>

EndPoints::EndPoints()
{}

EndPoints::~EndPoints()
{

}

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
