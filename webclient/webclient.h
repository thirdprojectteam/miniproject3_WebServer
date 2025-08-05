// webclient.h
#ifndef WEBCLIENT_H
#define WEBCLIENT_H

#include <QWidget>
#include <QNetworkAccessManager> // 네트워크 요청을 보낼 때 사용
#include <QNetworkReply>         // 네트워크 응답을 받을 때 사용
#include <QJsonObject>           // JSON 데이터 처리

QT_BEGIN_NAMESPACE
namespace Ui { class webclient; }
QT_END_NAMESPACE

class WebClient : public QWidget
{
    Q_OBJECT

public:
    WebClient(QWidget *parent = nullptr);
    ~WebClient();

private slots:
    void onRequestButtonClicked(); // "요청 보내기" 버튼 클릭 시 호출될 슬롯
    void onNetworkReplyFinished(QNetworkReply *reply); // 네트워크 요청 완료 시 호출될 슬롯

    void on_PostButton_clicked();

private:
    Ui::webclient *ui;
    QNetworkAccessManager *networkManager; // 네트워크 요청을 관리하는 객체
    const QString SERVER_URL = "http://127.0.0.1:8081/api/users"; // 웹서버의 API 엔드포인트
    // 참고: 웹서버의 포트가 다르면 여기를 수정하세요 (예: 8080)
};
#endif // WEBCLIENT_H
