#ifndef RESPONSES_H
#define RESPONSES_H

#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QHttpServerResponse>
#include "Endpoints.h"

class Responses
{
public:
    Responses(const EndPoints &eps);
    QFuture<QHttpServerResponse> asyncResponse(const QString &table) const;

private:
    const EndPoints &eps_;
};

#endif // RESPONSES_H
