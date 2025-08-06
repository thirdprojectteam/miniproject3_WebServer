#include "responses.h"

Responses::Responses(const EndPoints &eps): eps_(eps) {}

QFuture<QHttpServerResponse> Responses::asyncResponse(const QString &table) const
{
    // m_endpoints는 Endpoints의 레퍼런스 혹은 멤버 변수
    return QtConcurrent::run([this, table]() {
        return eps_.buildResponse(table);
    });
}
