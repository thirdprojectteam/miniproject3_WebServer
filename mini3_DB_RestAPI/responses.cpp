#include "responses.h"

Responses::Responses(const EndPoints &eps): eps_(eps) {}

QFuture<QHttpServerResponse> Responses::asyncResponse(const QString &table) const
{
    qDebug() << "Responses asyncResponse start";
    return QtConcurrent::run([this, table]() {
        return eps_.buildResponse(table);
    });
}

QFuture<QHttpServerResponse> Responses::asyncPostResponse(const QString &table, const bool &isPost) const
{
    return QtConcurrent::run([this, table,isPost]() {
        return eps_.buildPostResponse(table,isPost);
    });
}
