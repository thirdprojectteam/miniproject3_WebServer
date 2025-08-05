// webclient.cpp
#include "webclient.h"
#include "ui_webclient.h" // UI 헤더
#include <QDebug>
#include <QJsonDocument> // JSON 문서로 변환
#include <QJsonParseError> // JSON 파싱 에러 처리
#include <QUrl> // URL 생성
#include <QNetworkRequest> // 네트워크 요청 객체
#include <QJsonArray>
#include <QJsonDocument>
WebClient::WebClient(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::webclient)
{
    ui->setupUi(this);

    // QNetworkAccessManager 초기화
    networkManager = new QNetworkAccessManager(this);

    // "요청 보내기" 버튼의 클릭 시그널을 onRequestButtonClicked 슬롯에 연결
    connect(ui->requestButton, &QPushButton::clicked, this, &WebClient::onRequestButtonClicked);

    // networkManager의 finished 시그널을 onNetworkReplyFinished 슬롯에 연결
    // 이 시그널은 네트워크 요청이 완료될 때 (성공/실패 무관) 발생합니다
    // 파싱위치
    connect(networkManager, &QNetworkAccessManager::finished, this, &WebClient::onNetworkReplyFinished);

    ui->responseDisplay->setText("웹서버로 /api/users GET 요청을 보낼 준비가 되었습니다.");
}

WebClient::~WebClient()
{
    delete ui;
}

// "요청 보내기" 버튼 클릭 시 호출될 슬롯 -> 이게 선택문이네
void WebClient::onRequestButtonClicked()
{
    ui->responseDisplay->clear();
    ui->responseDisplay->append("웹서버로 요청 중...");

    // 요청할 URL 생성 (웹서버의 /api/users 경로)
    QUrl url(SERVER_URL);
    QNetworkRequest request(url);

    // GET 요청 보내기
    networkManager->get(request);
}

// 네트워크 요청 완료 시 호출될 슬롯
void WebClient::onNetworkReplyFinished(QNetworkReply *reply)
{
    // 1. 오류 확인
    if (reply->error() != QNetworkReply::NoError) {
        ui->responseDisplay->append("오류 발생: " + reply->errorString());
        qWarning() << "Network error:" << reply->errorString();
        reply->deleteLater(); // 메모리 해제
        return;
    }

    // 2. 응답 데이터 읽기
    QByteArray responseData = reply->readAll();

    //프리픽스문
    ui->responseDisplay->append("\n--- 웹서버 응답 ---");
    ui->responseDisplay->append("RAW 응답:\n" + QString(responseData));

    // 3. JSON 파싱
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        ui->responseDisplay->append("JSON 파싱 오류: " + parseError.errorString());
        qWarning() << "JSON parse error:" << parseError.errorString();
    } else {
        if (jsonDoc.isObject()) {
            QJsonObject jsonObject = jsonDoc.object();
            ui->responseDisplay->append("\n--- 파싱된 JSON (객체) ---");
            // JSON 객체를 보기 좋게 포매팅하여 표시
            ui->responseDisplay->append(QJsonDocument(jsonObject).toJson(QJsonDocument::Indented));
            qDebug() << "Client received JSON object:" << jsonObject;
        } else if (jsonDoc.isArray()) {
            QJsonArray jsonArray = jsonDoc.array();
            ui->responseDisplay->append("\n--- 파싱된 JSON (배열) ---");
            // JSON 배열을 보기 좋게 포매팅하여 표시
            qDebug() << "Client received JSON array:" << jsonArray;
            ui->responseDisplay->append(QJsonDocument(jsonArray).toJson(QJsonDocument::Indented));
        } else {
            ui->responseDisplay->append("응답이 유효한 JSON 객체나 배열이 아닙니다.");
        }
    }

    // 4. QNetworkReply 객체 메모리 해제 (매우 중요!)
    reply->deleteLater();
}
//하드코딩 되어있습니다.
void WebClient::on_PostButton_clicked()
{
    ui->responseDisplay->clear();
    ui->responseDisplay->append("웹서버로 요청 중...");

    // 요청할 URL 생성 (웹서버의 /api/users 경로)
    QUrl url(SERVER_URL);
    QNetworkRequest request(url);

    QJsonObject obj;
    QJsonObject members;
    members["client_resident"] = "서울 양천구 목동";
    members["RFID_name"] = "Yang";
    members["client_phone"] = "양준혁";
    obj["data"] = members;
    QJsonDocument doc(obj);
    QByteArray postData = doc.toJson(QJsonDocument::Compact);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    // GET 요청 보내기
    networkManager->post(request,postData);
}

