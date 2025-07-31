#include "WebServer.h"

#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QBoxLayout>
#include <QMessageBox>
#include <QApplication>
#include <QTcpSocket>
#include <QTcpServer>
#include <QStringList>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkReply>

WebServer::WebServer(QWidget *parent) : QWidget(parent)
{
    networkManager = new QNetworkAccessManager(this);
    QLineEdit* portInput = new QLineEdit("8081",this);
    portInput->setReadOnly(true);
    QPushButton *quitButton = new QPushButton("Quit", this);
    connect(quitButton, &QPushButton::clicked, qApp, &QApplication::quit);

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget(portInput);
    hlayout->addStretch(1);
    hlayout->addWidget(quitButton);

    InfoMsg = new QTextEdit(this);
    InfoMsg->setReadOnly(true);

    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->addLayout(hlayout);
    vlayout->addStretch(1);
    vlayout->addWidget(InfoMsg);

    tcpServer = new QTcpServer(this);
    if(!tcpServer->listen(QHostAddress::Any, portInput->text().toInt())){
        QMessageBox::critical(this, tr("HTTP Server"),\
                                                      tr("Unable to start the server %1").arg(tcpServer->errorString()));
        close();
    }else{
        qWarning("Success to bind port");
        connect(tcpServer, SIGNAL(newConnection()), this, SLOT(newConnect()));
        InfoMsg->append("Server Start!!");
    }
}

WebServer::~WebServer(){
    InfoMsg->append("Server is closed");
}

void WebServer::newConnect(){
    InfoMsg->append("New Connection");
    QTcpSocket* sock = tcpServer->nextPendingConnection();
    connect(sock, SIGNAL(readyRead()),this, SLOT(readClient()));
    connect(sock,SIGNAL(disconnected()),sock,SLOT(deleteLater()));
}

// RESTful API GET 요청 함수
QJsonObject WebServer::getRequest(const QString &endpoint)
{
    InfoMsg->append("Sending GET request to: " + endpoint);

    // QNetworkRequest 객체 생성
    QNetworkRequest request;
    request.setUrl(QUrl(endpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // API에 GET 요청 보내기
    QNetworkReply *reply = networkManager->get(request);

    // 동기식으로 응답 기다리기 (실제로는 비동기 방식 권장)
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QJsonObject responseObject;
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(responseData);
        responseObject = doc.object();
        InfoMsg->append("Response received: " + QString(responseData));
    } else {
        InfoMsg->append("Error: " + reply->errorString());
        responseObject["error"] = reply->errorString();
    }

    reply->deleteLater();
    return responseObject;
}

// RESTful API POST 요청 함수
QJsonObject WebServer::postRequest(const QString &endpoint, const QJsonObject &data)
{
    InfoMsg->append("Sending POST request to: " + endpoint);

    // QNetworkRequest 객체 생성
    QNetworkRequest request;
    request.setUrl(QUrl(endpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonDocument doc(data);
    QByteArray jsonData = doc.toJson();

    // API에 POST 요청 보내기
    QNetworkReply *reply = networkManager->post(request, jsonData);

    // 동기식으로 응답 기다리기
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QJsonObject responseObject;
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(responseData);
        responseObject = doc.object();
        InfoMsg->append("Response received: " + QString(responseData));
    } else {
        InfoMsg->append("Error: " + reply->errorString());
        responseObject["error"] = reply->errorString();
    }

    reply->deleteLater();
    return responseObject;
}

// 클라이언트 요청 처리 함수 수정
void WebServer::readClient()
{
    QTcpSocket *socket = dynamic_cast<QTcpSocket*>(sender());
    if(socket->canReadLine()){
        QString str = socket->readLine();
        str = str.trimmed();
        qDebug() << "socket : " << str;

        // HTTP 요청 파싱 (간단한 예시)
        if (str.startsWith("GET")) {
            QStringList parts = str.split(" ");
            if (parts.size() >= 2) {
                QString path = parts[1];

                // 특정 경로에 대해 RESTful API 요청 처리
                if (path.startsWith("/api/")) {
                    // 예: /api/users -> RESTful API의 http://localhost:8080/api/users로 요청
                    QString apiEndpoint = "http://localhost:8080" + path;
                    QJsonObject response = getRequest(apiEndpoint);

                    // JSON 응답을 클라이언트에게 보내기
                    QJsonDocument doc(response);
                    QByteArray jsonResponse = doc.toJson();

                    // HTTP 응답 헤더
                    QString httpResponse = "HTTP/1.1 200 OK\r\n"
                                           "Content-Type: application/json\r\n"
                                           "Content-Length: " + QString::number(jsonResponse.size()) + "\r\n"
                                                                                    "\r\n";

                    socket->write(httpResponse.toUtf8());
                    socket->write(jsonResponse);
                    socket->flush();
                    InfoMsg->append("Sent API response to client");
                }
            }
        }
    }
}
