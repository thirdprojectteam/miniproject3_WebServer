#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <QWidget>
#include <QNetworkAccessManager>
class QTcpServer;
class QTextEdit;
class WebServer : public QWidget
{
    Q_OBJECT

public:
    WebServer(QWidget *parent = nullptr);
    ~WebServer();

private slots:
    void newConnect();
    void readClient();

    // RESTful API 요청을 처리하는 함수 추가
    QJsonObject getRequest(const QString &endpoint);
    QJsonObject postRequest(const QString &endpoint, const QJsonObject &data);

private:
    QTcpServer* tcpServer;
    QTextEdit* InfoMsg;
    QNetworkAccessManager* networkManager;
};

#endif // WEBSERVER_H
