#ifndef DUMI_WEB_H
#define DUMI_WEB_H

#include <QWidget>
#include <QNetworkAccessManager>
class QTcpServer;
class QTextEdit;
class QJsonObject;
class dumi_web : public QWidget
{
    Q_OBJECT

public:
    dumi_web(QWidget *parent = nullptr);
    ~dumi_web();

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
#endif // DUMI_WEB_H
