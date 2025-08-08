#include "datamanager.h"
DataManager::~DataManager()
{
    disconnect();
}

QSqlDatabase DataManager::createThreadConnection(QString &connName)
{
    //QString connName = QString("conn_%1").arg((quintptr)QThread::currentThreadId());

    if (QSqlDatabase::contains(connName)) {
        return QSqlDatabase::database(connName);
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", connName);
    db.setHostName(h_n);
    db.setDatabaseName(d_n);
    db.setUserName(u_n);
    db.setPassword(pass);
    db.setPort(p);
    db.setConnectOptions(
        "SSL_KEY=C:/MySQL/certs/client-key.pem;"
        "SSL_CERT=C:/MySQL/certs/client-cert.pem;"
        "SSL_CA=C:/MySQL/certs/ca.pem;"
        );

    if (!db.open()) {
        qCritical() << "DB 재사용 안됨. DB 매니저 확인" << db.lastError().text();
    }

    return db;
}

void DataManager::closeThreadConnection(QString &connName)
{
    if (!QSqlDatabase::contains(connName))
        return;
    // connName 으로 등록된 데이터베이스 연결이 있으면
    if (QSqlDatabase::contains(connName)) {
        {
            // 데이터베이스 객체 얻고
            QSqlDatabase db = QSqlDatabase::database(connName);
            // 열려 있으면 닫아주고
            if (db.isOpen()) {
                db.close();
            }
        }
        // 연결 제거 (removeDatabase 호출 시, db 객체는 삭제되므로 복사본을 쓰세요)
        QSqlDatabase::removeDatabase(connName);
    }
}

QSqlDatabase DataManager::getDB()
{
    return this->m_db;
}

bool DataManager::connect(const QString &hostname, const QString &dbName, const QString &username, const QString &password, int port)
{
    this->h_n = hostname;
    this->d_n = dbName;
    this->u_n = username;
    this->pass = password;
    this->p = port;

    m_db = QSqlDatabase::addDatabase("QMYSQL", "main_connection");
    m_db.setHostName(h_n);
    m_db.setDatabaseName(d_n);
    m_db.setUserName(u_n);
    m_db.setPassword(pass);
    m_db.setPort(p);

    m_db.setConnectOptions(
        "SSL_KEY=C:/MySQL/certs/client-key.pem;"
        "SSL_CERT=C:/MySQL/certs/client-cert.pem;"
        "SSL_CA=C:/MySQL/certs/ca.pem;"
        );


    isConnected = m_db.open();
    if (!isConnected) {
        m_lastError = m_db.lastError();
        qDebug() << "데이터베이스 연결 실패:" << m_lastError.text();
        //emit connectionStatusChanged(false);
        //emit operationCompleted(false, "connect", m_lastError);
        return false;
    }

    qDebug() << "데이터베이스 연결 성공!";
    //emit connectionStatusChanged(true);
    //emit operationCompleted(true, "connect");
    return true;
}

void DataManager::disconnect()
{
    if (isConnected) {
        m_db.close();
        isConnected = false;
        qDebug() << "데이터베이스 연결 종료";
        //emit connectionStatusChanged(false);
    }
}
