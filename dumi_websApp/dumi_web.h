#ifndef DUMI_WEB_H
#define DUMI_WEB_H

#include <QWidget>
#include <QTcpServer>
#include <QTextEdit>
#include <QJsonArray>
#include <QJsonObject> // QJsonObject 필요하면 그대로 유지
#include "dumi_network.h" // 새로 만든 NetworkHandler 헤더 포함

class dumi_web : public QWidget
{
    Q_OBJECT

public:
    dumi_web(QWidget *parent = nullptr);
    ~dumi_web();

private slots:
    void newConnect();
    void readClient();
    void handleGetResult(const QJsonObject &data, QNetworkReply* reply); // QNetworkReply* 인자 추가
    void handlePostResult(const QJsonObject &data, QNetworkReply* reply); // QNetworkReply* 인자 추가
    void handleRequestError(const QString &errorString, QNetworkReply* reply); // QNetworkReply* 인자 추가
    void handleGetArrayResult(const QJsonArray &data, QNetworkReply* reply);
private:
    QTcpServer* tcpServer;
    QTextEdit* InfoMsg;
    NetworkHandler* networkHandler; // NetworkAccessManager 대신 NetworkHandler 객체
    QMap<QNetworkReply*, QTcpSocket*> pendingApiReplies;

    // RESTful API 요청 함수는 NetworkHandler로 위임하므로 제거하거나,
    // 필요하면 NetworkHandler 객체를 통해 호출하는 방식으로 변경
    // QJsonObject getRequest(const QString &endpoint); // 제거
    // QJsonObject postRequest(const QString &endpoint, const QJsonObject &data); // 제거
};
#endif // DUMI_WEB_H
