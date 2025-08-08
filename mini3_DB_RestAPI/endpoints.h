#ifndef ENDPOINTS_H
#define ENDPOINTS_H

#include <QMap>
#include <QString>
#include <QHttpServerResponse>
#include <QJsonDocument>
#include "database.h"

class EndPoints
{
public:
    EndPoints();
    ~EndPoints();
    // "clientdb" 같은 키와, 해당 DB 객체 포인터를 등록
    void registerDb(const QString &name, DataBase *db);

    // table 이름으로 QHttpServerResponse 생성
    QHttpServerResponse buildResponse(const QString &table) const;
    QHttpServerResponse buildResponseWhere(const QString &table,QString con1, QString con2) const;
    QHttpServerResponse buildResponseRecent(const QString &table) const;
    QHttpServerResponse buildPostResponse(const QString &table, const bool &isPost) const;
    // post
    bool InsertSuccess(const QHttpServerRequest &request,const QString &table);
    bool UpdateSuccess(const QHttpServerRequest &request,const QString &table);
private:
    QMap<QString, DataBase*> dbMap;
};

#endif // ENDPOINTS_H
