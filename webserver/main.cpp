#include "dumi_web.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qDebug() << "웹서버 애플리케이션 시작";

    // WebServer 객체 생성
    dumi_web webServer;

    webServer.show();

    // 이벤트 루프 시작 (애플리케이션이 종료될 때까지 실행)
    return app.exec();
}
