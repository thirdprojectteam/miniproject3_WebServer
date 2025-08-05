#ifndef DUMI_NETWORK_H
#define DUMI_NETWORK_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument> // QJsonObject를 사용할 때 필요

class NetworkHandler : public QObject
{
    Q_OBJECT

public:
    explicit NetworkHandler(QObject *parent = nullptr);
    ~NetworkHandler();

    QNetworkReply* sendGetRequest(const QString &endpoint);
    QNetworkReply* sendPostRequest(const QString &endpoint, const QByteArray &data);

signals:
    // -- 이 부분 시그널에 QNetworkReply* 인자를 추가합니다 --
    void getRequestFinished(const QJsonObject &data, QNetworkReply* reply);
    void postRequestFinished(const QJsonObject &data, QNetworkReply* reply);
    void requestFailed(const QString &errorString, QNetworkReply* reply);
private slots:
    void onGetReplyFinished(QNetworkReply *reply);
    void onPostReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *manager;
};

#endif // DUMI_NETWORK_H
