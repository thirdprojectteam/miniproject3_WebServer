#include "endpoints.h"

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
    QJsonDocument doc;
    if (auto it = dbMap.find(table); it != dbMap.end()) {
        doc = QJsonDocument(it.value()->getAll());
    }
    return QHttpServerResponse{
        "application/json",
        doc.toJson(QJsonDocument::Compact)
    };
}
