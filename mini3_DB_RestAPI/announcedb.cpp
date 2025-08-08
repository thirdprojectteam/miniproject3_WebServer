#include "announcedb.h"
#include "datamanager.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QThread>

AnnounceDB::AnnounceDB()
{
    TableName = "announcedb";
}

AnnounceDB::~AnnounceDB()
{

}

QJsonObject AnnounceDB::getLatest()
{
    qDebug() << "AtmLogDB getLatest start";
    QJsonObject result;
    QString sql = "SELECT * FROM " + TableName + " ORDER BY id DESC LIMIT 1";
    QString connName = QString("conn_%1").arg((quintptr)QThread::currentThreadId());
    {
        QSqlQuery query(DataManager::instance().createThreadConnection(connName));

        if (!query.exec(sql)) {
            m_lastError = query.lastError();
            qDebug() << "조회 실패:" << m_lastError.text();
        } else if (query.next()) {
            const auto rec = query.record();
            for (int i = 0; i < rec.count(); ++i) {
                result.insert(rec.fieldName(i), QJsonValue::fromVariant(query.value(i)));
            }
        } else {
            // 레코드가 없을 때: 빈 객체 반환 (필요하면 에러 메시지/상태 코드로 처리)
            qDebug() << "조회 결과 없음";
        }
        if (query.isActive()) query.finish();
        query.clear();
    }
    DataManager::instance().closeThreadConnection(connName);
    return result;
}

QJsonArray AnnounceDB::getAll()
{
    qDebug() << "AnnounceDB getAll start";
    QJsonArray result;
    QString sql = "SELECT * FROM " + TableName;
    QString connName;
    connName = QString("conn_%1").arg((quintptr)QThread::currentThreadId());
    {
        QSqlQuery query(DataManager::instance().createThreadConnection(connName));
        if (query.exec(sql)) {
            while (query.next()) {
                QJsonObject item;
                for (int i = 0; i < query.record().count(); i++) {
                    item.insert(query.record().fieldName(i), QJsonValue::fromVariant(query.value(i)));
                }
                result.append(item);
            }
            //emit operationCompleted(true, "getAll");
        } else {
            m_lastError = query.lastError();
            qDebug() << "조회 실패:" << m_lastError.text();
            //emit operationCompleted(false, "getAll", m_lastError);
        }
        if(query.isActive())
            query.finish();
        query.clear();
    }
    DataManager::instance().closeThreadConnection(connName);
    return result;
}

QJsonObject AnnounceDB::getById(int id)
{
    QJsonObject result;

    QSqlQuery query;
    query.prepare("SELECT * FROM " + TableName + " WHERE id = :id");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        for (int i = 0; i < query.record().count(); i++) {
            result.insert(query.record().fieldName(i), QJsonValue::fromVariant(query.value(i)));
        }
        //emit operationCompleted(true, "getById");
    } else {
        m_lastError = query.lastError();
        qDebug() << "ID로 조회 실패:" << m_lastError.text();
        //emit operationCompleted(false, "getById", m_lastError);
    }

    return result;
}

QJsonObject AnnounceDB::getByCondition(const QString &cond, const QString &id)
{
    QJsonObject resultObject; // 결과를 담을 QJsonObject
    // 2. SQL 쿼리 준비 (플레이스홀더 사용)
    QString connName;
    connName = QString("conn_%1").arg((quintptr)QThread::currentThreadId());
    QSqlQuery query(DataManager::instance().createThreadConnection(connName)); // m_db는 Database 클래스의 QSqlDatabase 멤버 변수입니다.

    // SQL Injection을 방지하기 위해 플레이스홀더를 사용하는 것이 중요합니다.
    // SELECT * FROM membertbl WHERE memberID = :id_value
    QString queryString = QString("SELECT * FROM %1 WHERE %2 = :id_value")
                              .arg(TableName)   // 테이블 이름
                              .arg(cond); // 조건 컬럼 (헤더) 이름

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
            qDebug() << "No data found for" << cond << "=" << id << "in table:" << TableName;
        }
    } else {
        // 쿼리 실행 실패
        qWarning() << "SQL Error in getByCondition for table" << TableName << ":" << query.lastError().text();
    }

    return resultObject;
}

bool AnnounceDB::insert(const QJsonObject &data)
{
    QString connName;
    connName = QString("conn_%1").arg((quintptr)QThread::currentThreadId());
    {
        QString sql = buildInsertQuery(TableName, data);
        QSqlQuery query(DataManager::instance().createThreadConnection(connName));
        query.prepare(sql);

        bindJsonToQuery(query, data);

        if (!query.exec()) {
            m_lastError = query.lastError();
            qDebug() << "삽입 실패:" << m_lastError.text();
            return false;
        }
    }
    DataManager::instance().closeThreadConnection(connName);
    return true;
}

bool AnnounceDB::update(int id, const QJsonObject &data)
{
    QString sql = buildUpdateQuery(TableName, id, data);
    QSqlQuery query;
    query.prepare(sql);

    bindJsonToQuery(query, data);
    query.bindValue(":id", id);

    if (!query.exec()) {
        m_lastError = query.lastError();
        qDebug() << "업데이트 실패:" << m_lastError.text();
        //emit operationCompleted(false, "update", m_lastError);
        return false;
    }

    //emit operationCompleted(true, "update");
    return true;
}

bool AnnounceDB::remove(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM " + TableName + " WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        m_lastError = query.lastError();
        qDebug() << "삭제 실패:" << m_lastError.text();
        //emit operationCompleted(false, "remove", m_lastError);
        return false;
    }

    //emit operationCompleted(true, "remove");
    return true;
}

QString AnnounceDB::buildUpdateQuery(const QString &table, int id, const QJsonObject &data)
{
    QStringList setStatements;

    for (auto it = data.begin(); it != data.end(); ++it) {
        setStatements.append(it.key() + " = :" + it.key());
    }

    return QString("UPDATE %1 SET %2 WHERE id = :id")
        .arg(table)
        .arg(setStatements.join(", "));
}

QString AnnounceDB::buildInsertQuery(const QString &table, const QJsonObject &data)
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

void AnnounceDB::bindJsonToQuery(QSqlQuery &query, const QJsonObject &data)
{
    for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
        QString placeholder = ":" + it.key();
        QVariant value = it.value().toVariant();
        query.bindValue(placeholder, value);
    }
}
