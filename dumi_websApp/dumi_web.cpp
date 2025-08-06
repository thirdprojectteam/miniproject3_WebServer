#include "dumi_web.h"

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

dumi_web::dumi_web(QWidget *parent) : QWidget(parent)
{
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
    connect(tcpServer, &QTcpServer::newConnection, this, &dumi_web::newConnect);

    // QNetworkAccessManager는 NetworkHandler 내부에서 초기화됨
    networkHandler = new NetworkHandler(this);
    // -- connect 시그니처를 변경된 슬롯에 맞게 수정 --
    connect(networkHandler, &NetworkHandler::getRequestFinished, this, &dumi_web::handleGetResult);
    connect(networkHandler, &NetworkHandler::postRequestFinished, this, &dumi_web::handlePostResult);
    connect(networkHandler, &NetworkHandler::requestFailed, this, &dumi_web::handleRequestError);

    // --- 여기에 서버 실행 코드 추가 ---
    // 포트는 portInput에 설정된 8081을 사용하도록 합니다.
    // QHostAddress::Any는 모든 네트워크 인터페이스로부터의 연결을 허용합니다.
    int port = portInput->text().toInt(); // QLineEdit에서 포트 번호를 가져옵니다.

    if (!tcpServer->listen(QHostAddress::Any, port)) {
        QMessageBox::critical(this, tr("Echo Server"),
                              tr("Unable to start the server: %1.")
                                  .arg(tcpServer->errorString()));
        qApp->quit(); // 서버 시작 실패 시 애플리케이션 종료
    } else {
        InfoMsg->append(tr("The server is running on\n\nIP: %1\nport: %2\n\n"
                           "Run the WebClient application now.")
                            .arg(tcpServer->serverAddress().toString())
                            .arg(tcpServer->serverPort()));
    }

}

dumi_web::~dumi_web(){
    InfoMsg->append("Server is closed");
}

void dumi_web::newConnect(){
    InfoMsg->append("New Connection");
    QTcpSocket* sock = tcpServer->nextPendingConnection();
    connect(sock, SIGNAL(readyRead()),this, SLOT(readClient()));
    connect(sock,SIGNAL(disconnected()),sock,SLOT(deleteLater()));
}

void dumi_web::handleGetResult(const QJsonObject &data, QNetworkReply* apiReply) // apiReply로 이름을 바꿔 명확하게 함
{
    qDebug() << "GET Request Result:" << data;
    InfoMsg->append("GET Result: " + QJsonDocument(data).toJson(QJsonDocument::Compact));

    // 맵에서 해당 QNetworkReply*와 연결된 클라이언트 QTcpSocket*을 찾습니다.
    QTcpSocket* clientSocket = pendingApiReplies.take(apiReply); // 사용 후 맵에서 제거

    if (clientSocket && clientSocket->isOpen() && clientSocket->state() == QAbstractSocket::ConnectedState) {
        QJsonDocument doc(data);
        QByteArray jsonResponse = doc.toJson(QJsonDocument::Compact); // JSON 데이터를 압축된 형태로 변환

        // HTTP 응답 헤더 작성
        QString httpResponse = "HTTP/1.1 200 OK\r\n"
                               "Content-Type: application/json\r\n" // JSON임을 명시
                               "Content-Length: " + QString::number(jsonResponse.size()) + "\r\n"
                                                                        "Connection: close\r\n" // 연결 닫기 (간단한 서버 예시)
                                                                        "\r\n"; // 헤더와 본문 분리

        // 클라이언트 소켓에 HTTP 응답 보내기
        clientSocket->write(httpResponse.toUtf8());
        clientSocket->write(jsonResponse);
        clientSocket->flush(); // 버퍼 비우기
        clientSocket->disconnectFromHost(); // 클라이언트 연결 종료 (또는 Keep-Alive 설정 가능)
        InfoMsg->append("Sent API JSON response back to client.");
    } else {
        qWarning() << "Client socket for API reply not found or disconnected!";
        InfoMsg->append("Warning: Client socket for API reply was invalid or disconnected.");
    }

    // apiReply->deleteLater(); // NetworkHandler에서 이미 호출하고 있다면 여기서는 필요 없음
    // 만약 NetworkHandler에서 deleteLater를 호출하지 않았다면 여기에 추가
}

// handlePostResult 및 handleRequestError도 유사하게 QNetworkReply* 인자를 받고 처리해야 합니다.
void dumi_web::handlePostResult(const QJsonObject &data, QNetworkReply* apiReply)
{
    qDebug() << "POST Request Result:" << data;
    InfoMsg->append("POST Result: " + QJsonDocument(data).toJson(QJsonDocument::Compact));

    QTcpSocket* clientSocket = pendingApiReplies.take(apiReply);

    if (clientSocket && clientSocket->isOpen() && clientSocket->state() == QAbstractSocket::ConnectedState) {
        QJsonDocument doc(data);
        QByteArray jsonResponse = doc.toJson(QJsonDocument::Compact);

        QString httpResponse = "HTTP/1.1 200 OK\r\n"
                               "Content-Type: application/json\r\n"
                               "Content-Length: " + QString::number(jsonResponse.size()) + "\r\n"
                                                                        "Connection: close\r\n"
                                                                        "\r\n";

        clientSocket->write(httpResponse.toUtf8());
        clientSocket->write(jsonResponse);
        clientSocket->flush();
        clientSocket->disconnectFromHost();
        InfoMsg->append("Sent API JSON response back to client for POST.");
    } else {
        qWarning() << "Client socket for POST API reply not found or disconnected!";
        InfoMsg->append("Warning: Client socket for POST API reply was invalid or disconnected.");
    }
}

void dumi_web::handleRequestError(const QString &errorString, QNetworkReply* apiReply)
{
    qWarning() << "Network Request Error:" << errorString;
    InfoMsg->append("Network Error: " + errorString);

    QTcpSocket* clientSocket = pendingApiReplies.take(apiReply);

    if (clientSocket && clientSocket->isOpen() && clientSocket->state() == QAbstractSocket::ConnectedState) {
        QString errorHttpResponse = "HTTP/1.1 500 Internal Server Error\r\n"
                                    "Content-Type: text/plain\r\n"
                                    "Content-Length: " + QString::number(errorString.toUtf8().size()) + "\r\n"
                                                                                     "Connection: close\r\n"
                                                                                     "\r\n" + errorString; // 에러 메시지를 본문에 포함

        clientSocket->write(errorHttpResponse.toUtf8());
        clientSocket->flush();
        clientSocket->disconnectFromHost();
        InfoMsg->append("Sent error response to client: " + errorString);
    } else {
        qWarning() << "Client socket for error reply not found or disconnected!";
        InfoMsg->append("Warning: Client socket for error reply was invalid or disconnected.");
    }
}
// 클라이언트 요청 처리 함수 수정
void dumi_web::readClient()
{
    QTcpSocket *socket = dynamic_cast<QTcpSocket*>(sender());
    if (!socket || !socket->canReadLine()) {
        return; // 유효하지 않은 소켓이거나 읽을 데이터가 없음
    }

    QString requestLine = socket->readLine().trimmed();
    qDebug() << "Received from client:" << requestLine;
    InfoMsg->append(QString("Client Request: %1").arg(requestLine));

    QStringList parts = requestLine.split(" ");
    QString path;
    if(parts.size()>=2){
        path = parts[1];
    }

    // HTTP 요청 파싱 (간단한 GET 예시)
    if (requestLine.startsWith("GET")) {
        // 특정 경로에 대해 RESTful API 요청 처리
        if (path.startsWith("/client/")) {
            QString apiEndpoint = "http://localhost:8080" + path;
            InfoMsg->append("Forwarding API GET request to: " + apiEndpoint);

            QNetworkReply* reply = networkHandler->sendGetRequest(apiEndpoint);
            //if (requestLine.startsWith("GET")) {
                //reply = networkHandler->sendGetRequest(apiEndpoint);
                //return;
            //}
            if (reply) {
                // QNetworkReply*를 키로 현재 클라이언트 소켓을 저장
                pendingApiReplies.insert(reply, socket);
            } else {
                QString errorMsg = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
                socket->write(errorMsg.toUtf8());
                socket->flush();
                socket->disconnectFromHost();
                InfoMsg->append("Error: Failed to send internal API request.");
            }
        } else {
            // /api/가 아닌 일반 GET 요청 처리 (예: 웹 페이지 제공)
            QString responseContent = "<html><body><h1>Hello from simple web server!</h1><p>Path: " + path + "</p></body></html>";
            QString httpResponse = "HTTP/1.1 200 OK\r\n"
                                   "Content-Type: text/html\r\n"
                                   "Content-Length: " + QString::number(responseContent.toUtf8().size()) + "\r\n"
                                                                                        "\r\n" + responseContent;
            socket->write(httpResponse.toUtf8());
            socket->flush();
            socket->disconnectFromHost(); // 응답 후 연결 종료 (간단한 서버 예시)
            InfoMsg->append("Sent HTML response to client for path: " + path);
        }
    } else if(requestLine.startsWith("POST")){
        QString apiEndpoint = "http://localhost:8080" + path;
        InfoMsg->append("Forwarding API Post request to: " + apiEndpoint);

        QByteArray requestData;
        // Body를 다 읽어온 경우
        while (socket->bytesAvailable()) {
            requestData.append(socket->readAll());
        }
        //여기 index값 수정되었습니다. -> 이제 header빼고 body부분만 보냅니다.
        InfoMsg->append("POST Body Data: " + QString::fromUtf8(requestData));
        int headerEndIndex = requestData.indexOf("\r\n\r\n");

        QByteArray bodyData = requestData.mid(headerEndIndex + 4);

        QNetworkReply* reply = networkHandler->sendPostRequest(apiEndpoint, bodyData);

        if(reply){
            pendingApiReplies.insert(reply, socket);
        } else {
            QString errorMsg = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
            socket->write(errorMsg.toUtf8());
            socket->flush();
            socket->disconnectFromHost();
            InfoMsg->append("Error: Failed to send internal API request.");
        }
    } else {
        // 다른 HTTP 메서드 (POST 등) 처리 또는 오류 응답
        QString errorMsg = "HTTP/1.1 405 Method Not Allowed\r\n\r\n";
        socket->write(errorMsg.toUtf8());
        socket->flush();
        socket->disconnectFromHost();
        InfoMsg->append("Unsupported method received: " + requestLine);
    }
}
