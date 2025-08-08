#ifndef DUMI_NETWORK_H
#define DUMI_NETWORK_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument> // QJsonObject를 사용할 때 필요

class NetworkHandler : public QObject
{
    Q_OBJECT

public:
    explicit NetworkHandler(QObject *parent = nullptr);
    ~NetworkHandler();
    //http Rest
    QNetworkReply* sendGetRequest(const QString &endpoint);
    QNetworkReply* sendPostRequest(const QString &endpoint, const QByteArray &data);
    QNetworkReply* sendPutRequest(const QString &endpoint, const QByteArray &data);

signals:
    // -- 이 부분 시그널에 QNetworkReply* 인자를 추가합니다 --
    //http Rest
    void requestFinished(const QJsonDocument &data, QNetworkReply* reply);
    void requestFailed(const QString &errorString, QNetworkReply* reply);
private slots:
    //http Rest
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *manager;
};

#endif // DUMI_NETWORK_H
