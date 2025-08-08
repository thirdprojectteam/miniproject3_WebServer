#ifndef DUMI_WEB_H
#define DUMI_WEB_H

#include <QWidget>
#include <QTcpServer>
#include <QTextEdit>
#include <QJsonArray>
#include <QJsonObject> // QJsonObject 필요하면 그대로 유지

#include <QWebSocketServer> //websocketServer
#include <QWebSocket>       //webSocket

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
    void handleResult(const QJsonDocument &data, QNetworkReply* reply); // QNetworkReply* 인자 추가
    void handleRequestError(const QString &errorString, QNetworkReply* reply); // QNetworkReply* 인자 추가
private:
    QTcpServer* tcpServer;
    QTextEdit* InfoMsg;
    NetworkHandler* networkHandler; // NetworkAccessManager 대신 NetworkHandler 객체
    QMap<QNetworkReply*, QTcpSocket*> pendingApiReplies;
    //Websocket server;
    QWebSocketServer* wsServer;
    QList<QWebSocket*> chatClients;
    QMap<QWebSocket*, QString> clientNames;
};
#endif // DUMI_WEB_H
