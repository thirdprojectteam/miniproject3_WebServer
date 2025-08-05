#include "database.h"
Database::Database(QObject *parent) : QObject(parent), isConnected(false)
{
}

Database::~Database()
{
    disconnect();
}

bool Database::connect(const QString &hostname, const QString &dbName,
                       const QString &username, const QString &password, int port)
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
        emit connectionStatusChanged(false);
        emit operationCompleted(false, "connect", m_lastError);
        return false;
    }

    qDebug() << "데이터베이스 연결 성공!";
    emit connectionStatusChanged(true);
    emit operationCompleted(true, "connect");
    return true;
}

void Database::disconnect()
{
    if (isConnected) {
        m_db.close();
        isConnected = false;
        qDebug() << "데이터베이스 연결 종료";
        emit connectionStatusChanged(false);
    }
}

QJsonArray Database::getAll(const QString &table)
{
    QJsonArray result;
    if (!isConnected) {
        m_lastError = QSqlError("Not connected", "데이터베이스에 연결되지 않았습니다.", QSqlError::ConnectionError);
        emit operationCompleted(false, "getAll", m_lastError);
        return result;
    }

    QSqlQuery query;
    if (query.exec("SELECT * FROM " + table)) {
        while (query.next()) {
            QJsonObject item;
            for (int i = 0; i < query.record().count(); i++) {
                item.insert(query.record().fieldName(i), QJsonValue::fromVariant(query.value(i)));
            }
            result.append(item);
        }
        emit operationCompleted(true, "getAll");
    } else {
        m_lastError = query.lastError();
        qDebug() << "조회 실패:" << m_lastError.text();
        emit operationCompleted(false, "getAll", m_lastError);
    }

    return result;
}

QJsonObject Database::getById(const QString &table, int id)
{
    QJsonObject result;
    if (!isConnected) {
        m_lastError = QSqlError("Not connected", "데이터베이스에 연결되지 않았습니다.", QSqlError::ConnectionError);
        emit operationCompleted(false, "getById", m_lastError);
        return result;
    }

    QSqlQuery query;
    query.prepare("SELECT * FROM " + table + " WHERE id = :id");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        for (int i = 0; i < query.record().count(); i++) {
            result.insert(query.record().fieldName(i), QJsonValue::fromVariant(query.value(i)));
        }
        emit operationCompleted(true, "getById");
    } else {
        m_lastError = query.lastError();
        qDebug() << "ID로 조회 실패:" << m_lastError.text();
        emit operationCompleted(false, "getById", m_lastError);
    }

    return result;
}

QJsonObject Database::getByCondition(const QString &table, const QString &header, const QString &id)
{
    QJsonObject resultObject; // 결과를 담을 QJsonObject

    // 1. 데이터베이스 연결 상태 확인
    if (!m_db.isOpen()) {
        qWarning() << "Database not open. Cannot perform getByCondition for table:" << table;
        // 연결이 안 되어 있다면 비어있는 QJsonObject 반환
        return resultObject;
    }

    // 2. SQL 쿼리 준비 (플레이스홀더 사용)
    QSqlQuery query(m_db); // m_db는 Database 클래스의 QSqlDatabase 멤버 변수입니다.

    // SQL Injection을 방지하기 위해 플레이스홀더를 사용하는 것이 중요합니다.
    // SELECT * FROM membertbl WHERE memberID = :id_value
    QString queryString = QString("SELECT * FROM %1 WHERE %2 = :id_value")
                              .arg(table)   // 테이블 이름
                              .arg(header); // 조건 컬럼 (헤더) 이름

    if (!query.prepare(queryString)) {
        qWarning() << "Failed to prepare query for getByCondition:" << query.lastError().text();
        return resultObject;
    }

    // 3. 플레이스홀더에 값 바인딩
    query.bindValue(":id_value", id); // 여기서는 id가 QString이므로 직접 바인딩

    // 4. 쿼리 실행
    if (query.exec()) {
        // 5. 결과 처리: 첫 번째 레코드만 가져옵니다.
        if (query.next()) { // 결과가 하나라도 있다면
            QSqlRecord record = query.record(); // 현재 레코드(행) 정보 가져오기

            // 레코드의 각 필드를 QJsonObject에 추가
            for (int i = 0; i < record.count(); ++i) {
                QString fieldName = record.fieldName(i); // 컬럼 이름
                QVariant fieldValue = query.value(i);   // 컬럼 값

                // QVariant를 QJsonValue로 변환하여 QJsonObject에 추가
                resultObject[fieldName] = QJsonValue::fromVariant(fieldValue);
            }
        } else {
            // 조건에 맞는 데이터가 없음
            qDebug() << "No data found for" << header << "=" << id << "in table:" << table;
        }
    } else {
        // 쿼리 실행 실패
        qWarning() << "SQL Error in getByCondition for table" << table << ":" << query.lastError().text();
    }

    return resultObject;
}
//추가된 부분
bool Database::insert(const QString &table, const QJsonObject &data)
{
    if (!isConnected) {
        m_lastError = QSqlError("Not connected", "데이터베이스에 연결되지 않았습니다.", QSqlError::ConnectionError);
        emit operationCompleted(false, "insert", m_lastError);
        return false;
    }

    QString sql = buildInsertQuery(table, data);
    QSqlQuery query(m_db);
    query.prepare(sql);

    bindJsonToQuery(query, data);

    if (!query.exec()) {
        m_lastError = query.lastError();
        qDebug() << "삽입 실패:" << m_lastError.text();
        emit operationCompleted(false, "insert", m_lastError);
        return false;
    }

    emit operationCompleted(true, "insert");
    return true;
}

bool Database::update(const QString &table, int id, const QJsonObject &data)
{
    if (!isConnected) {
        m_lastError = QSqlError("Not connected", "데이터베이스에 연결되지 않았습니다.", QSqlError::ConnectionError);
        emit operationCompleted(false, "update", m_lastError);
        return false;
    }

    QString sql = buildUpdateQuery(table, id, data);
    QSqlQuery query;
    query.prepare(sql);

    bindJsonToQuery(query, data);
    query.bindValue(":id", id);

    if (!query.exec()) {
        m_lastError = query.lastError();
        qDebug() << "업데이트 실패:" << m_lastError.text();
        emit operationCompleted(false, "update", m_lastError);
        return false;
    }

    emit operationCompleted(true, "update");
    return true;
}

bool Database::remove(const QString &table, int id)
{
    if (!isConnected) {
        m_lastError = QSqlError("Not connected", "데이터베이스에 연결되지 않았습니다.", QSqlError::ConnectionError);
        emit operationCompleted(false, "remove", m_lastError);
        return false;
    }

    QSqlQuery query;
    query.prepare("DELETE FROM " + table + " WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        m_lastError = query.lastError();
        qDebug() << "삭제 실패:" << m_lastError.text();
        emit operationCompleted(false, "remove", m_lastError);
        return false;
    }

    emit operationCompleted(true, "remove");
    return true;
}

QSqlError Database::lastError() const
{
    return m_lastError;
}

QString Database::buildUpdateQuery(const QString &table, int id, const QJsonObject &data)
{
    QStringList setStatements;

    for (auto it = data.begin(); it != data.end(); ++it) {
        setStatements.append(it.key() + " = :" + it.key());
    }

    return QString("UPDATE %1 SET %2 WHERE id = :id")
        .arg(table)
        .arg(setStatements.join(", "));
}

QString Database::buildInsertQuery(const QString &table, const QJsonObject &data)
{
    QStringList fields;
    QStringList placeholders;

    for (auto it = data.begin(); it != data.end(); ++it) {
        fields.append(it.key());
        placeholders.append(":" + it.key());
    }

    return QString("INSERT INTO %1 (%2) VALUES (%3)")
        .arg(table)
        .arg(fields.join(", "))
        .arg(placeholders.join(", "));
}

void Database::bindJsonToQuery(QSqlQuery &query, const QJsonObject &data)
{
    for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
        QString placeholder = ":" + it.key();
        QVariant value = it.value().toVariant();
        query.bindValue(placeholder, value);
    }
}
