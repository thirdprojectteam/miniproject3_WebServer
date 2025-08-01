#include "dumi_network.h"

#include <QDebug> // 디버깅 용도

NetworkHandler::NetworkHandler(QObject *parent) : QObject(parent)
{
    manager = new QNetworkAccessManager(this);
    // GET 요청 응답 처리 슬롯 연결
    connect(manager, &QNetworkAccessManager::finished, this, &NetworkHandler::onGetReplyFinished);
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
QNetworkReply* NetworkHandler::sendPostRequest(const QString &endpoint, const QJsonObject &data) // 여기도 QNetworkReply*로 변경
{
    qDebug() << "Sending POST request to:" << endpoint;
    QNetworkRequest request;
    request.setUrl(QUrl(endpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QJsonDocument doc(data);
    QNetworkReply* reply = manager->post(request, doc.toJson());
    return reply; // manager->post()가 반환한 QNetworkReply*를 그대로 반환합니다.
}


void NetworkHandler::onGetReplyFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
        if (!jsonDoc.isNull() && jsonDoc.isObject()) {
            // getRequestFinished 시그널에 'reply' 객체를 함께 보냅니다.
            emit getRequestFinished(jsonDoc.object(), reply); // 수정됨
        } else {
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

void NetworkHandler::onPostReplyFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);

        if (!jsonDoc.isNull() && jsonDoc.isObject()) {
            // --- 여기를 수정합니다! ---
            // POST 요청의 결과이므로 postRequestFinished 시그널을 방출합니다.
            emit postRequestFinished(jsonDoc.object(), reply); // postRequestFinished로 변경
        } else {
            // JSON 파싱 오류 시 requestFailed 시그널에 reply 객체를 함께 보냅니다.
            emit requestFailed("Invalid JSON response or non-JSON content: " + QString(responseData), reply);
        }
    } else {
        // 네트워크 오류 시 requestFailed 시그널에 reply 객체를 함께 보냅니다.
        emit requestFailed("Network request error: " + reply->errorString(), reply);
    }
    // QNetworkReply 객체는 여기서 메모리를 해제하는 것이 좋습니다.
    reply->deleteLater();
}
