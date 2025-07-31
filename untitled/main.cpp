#include <QCoreApplication>
#include "apiserver.h"
#include "database.h" // Database 클래스를 포함

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // 데이터베이스 인스턴스 생성 및 연결 시도
    Database db;
    // 실제 MySQL 설정으로 변경해주세요!

    // ApiServer 인스턴스 생성
    ApiServer apiServer(&db); // db 인스턴스를 ApiServer에 전달

    // 서버 시작 시그널 연결 (선택 사항)
    QObject::connect(&apiServer, &ApiServer::serverStarted, [](int port){
        qInfo() << QString("API 서버가 %1번 포트에서 실행 중입니다.").arg(port);
        // qInfo() << "테스트 예시:";
        // qInfo() << " - 모든 항목 조회: GET http://localhost:8080/api/products/";
        // qInfo() << " - 특정 항목 조회: GET http://localhost:8080/api/products/1/";
        // qInfo() << " - 항목 생성: POST http://localhost:8080/api/products/ (Body: JSON)";
        // qInfo() << " - 항목 업데이트: PUT http://localhost:8080/api/products/1/ (Body: JSON)";
        // qInfo() << " - 항목 삭제: DELETE http://localhost:8080/api/products/1/";
    });

    // 에러 발생 시그널 연결 (선택 사항)
    QObject::connect(&apiServer, &ApiServer::errorOccurred, [](const QString& message){
        qCritical() << "API 서버 오류:" << message;
    });

    // 서버 시작
    if (!apiServer.start(8080)) { // 8080 포트에서 시작
        qCritical() << "API 서버 시작에 실패했습니다.";
        return -1;
    }

    return a.exec();
}
