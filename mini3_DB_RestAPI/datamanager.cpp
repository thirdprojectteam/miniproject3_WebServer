#include "datamanager.h"

DataManager::DataManager(QObject *parent) : QObject(parent)
{

}

DataManager::~DataManager()
{
    disconnect();
}

bool DataManager::connect(const QString &hostname, const QString &dbName, const QString &username, const QString &password, int port)
{
    m_db = QSqlDatabase::addDatabase("QMYSQL", "my_connection");
    m_db.setHostName(hostname);
    m_db.setDatabaseName(dbName);
    m_db.setUserName(username);
    m_db.setPassword(password);
    m_db.setPort(port);

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
