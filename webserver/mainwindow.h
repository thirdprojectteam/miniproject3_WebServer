#ifndef APIHANDLER_H
#define APIHANDLER_H

#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QString>
#include <QUrl>
#include <QEventLoop>

class ApiHandler : public QObject
{
    Q_OBJECT
public:
    explicit ApiHandler(QObject *parent = nullptr) : QObject(parent) {
        manager = new QNetworkAccessManager(this);
    }

    // 외부 API에 GET 요청을 보내는 함수
    QJsonObject getRequest(const QString &endpoint) {
        QNetworkRequest request;
        request.setUrl(QUrl(endpoint));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QNetworkReply *reply = manager->get(request);

        // 동기식으로 응답 기다리기 (실제 프로덕션에서는 비동기 방식 권장)
        QEventLoop loop;
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        QJsonObject responseObject;
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response = reply->readAll();
            QJsonDocument jsonDoc = QJsonDocument::fromJson(response);
            responseObject = jsonDoc.object();
        } else {
            responseObject["error"] = reply->errorString();
        }

        reply->deleteLater();
        return responseObject;
    }

    // 외부 API에 POST 요청을 보내는 함수
    QJsonObject postRequest(const QString &endpoint, const QJsonObject &data) {
        QNetworkRequest request;
        request.setUrl(QUrl(endpoint));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QJsonDocument doc(data);
        QByteArray jsonData = doc.toJson();

        QNetworkReply *reply = manager->post(request, jsonData);

        // 동기식으로 응답 기다리기
        QEventLoop loop;
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        QJsonObject responseObject;
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response = reply->readAll();
            QJsonDocument jsonDoc = QJsonDocument::fromJson(response);
            responseObject = jsonDoc.object();
        } else {
            responseObject["error"] = reply->errorString();
        }

        reply->deleteLater();
        return responseObject;
    }

private:
    QNetworkAccessManager *manager;
};

#endif // APIHANDLER_H
