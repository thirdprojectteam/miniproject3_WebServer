#include "networkhandler.h"

#include <QDebug> // 디버깅 용도

NetworkHandler::NetworkHandler(QObject *parent) : QObject(parent)
{
    manager = new QNetworkAccessManager(this);
    // GET 요청 응답 처리 슬롯 연결
    connect(manager, &QNetworkAccessManager::finished, this, &NetworkHandler::onReplyFinished);
    // POST 요청 응답 처리 슬롯은 필요에 따라 분리하거나 onGetReplyFinished에서 처리 로직 추가
}

NetworkHandler::~NetworkHandler()
{
    // manager는 부모가 관리하므로 명시적 delete 필요 없음
}

// sendGetRequest 함수 구현
QNetworkReply* NetworkHandler::sendGetRequest(const QString &endpoint) // 여기도 QNetworkReply*로 변경
{
    qDebug() << "Sending GET request to:" << endpoint;
    QNetworkReply* reply = manager->get(QNetworkRequest(QUrl(endpoint)));
    return reply; // manager->get()이 반환한 QNetworkReply*를 그대로 반환합니다.
}

// sendPostRequest 함수 구현
QNetworkReply* NetworkHandler::sendPostRequest(const QString &endpoint, const QByteArray &data) // 여기도 QNetworkReply*로 변경
{
    QNetworkRequest request;
    request.setUrl(QUrl(endpoint));
    //아마 추가했을거임
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = manager->post(request, data);
    return reply; // manager->post()가 반환한 QNetworkReply*를 그대로 반환합니다.
}

// sendPutRequest 함수 구현
QNetworkReply* NetworkHandler::sendPutRequest(const QString &endpoint, const QByteArray &data) // 여기도 QNetworkReply*로 변경
{
    QNetworkRequest request;
    request.setUrl(QUrl(endpoint));
    //아마 추가했을거임
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = manager->put(request, data);
    return reply; // manager->post()가 반환한 QNetworkReply*를 그대로 반환합니다.
}


void NetworkHandler::onReplyFinished(QNetworkReply *reply)
{
    QNetworkAccessManager::Operation op = reply->operation();

    switch(op) {
    case QNetworkAccessManager::GetOperation:
        qDebug()<<"get";
        break;
    case QNetworkAccessManager::PostOperation:
        qDebug()<<"post";
        break;
    case QNetworkAccessManager::PutOperation:
        qDebug()<<"put";
        break;
    default:
        qWarning() << "Unhandled HTTP Method";
        break;
    }

    //여기서 reply의 method확인후 나눠줘야됨.
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
        if (!jsonDoc.isNull()) {
            // requestFinished 시그널에 'reply' 객체를 함께 보냅니다.
            emit requestFinished(jsonDoc, reply); // 수정됨
        }else {
            // requestFailed 시그널에 'reply' 객체를 함께 보냅니다.
            emit requestFailed("Invalid JSON response for GET: " + QString(responseData), reply); // 수정됨
        }
    } else {
        // requestFailed 시그널에 'reply' 객체를 함께 보냅니다.
        emit requestFailed("GET request error: " + reply->errorString(), reply); // 수정됨
    }
    // QNetworkReply 객체의 메모리 해제는 여기서 하는 것이 좋습니다.
    reply->deleteLater();
}
