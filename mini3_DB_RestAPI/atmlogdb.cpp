#include "atmlogdb.h"
#include "datamanager.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QThread>

AtmLogDB::AtmLogDB()
{
    TableName = "atmlogdb";
}

AtmLogDB::~AtmLogDB()
{

}

QJsonObject AtmLogDB::getLatest()
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

QJsonArray AtmLogDB::getAll()
{
    qDebug() << "AtmLogDB getAll start";
    QJsonArray result;
    QString sql = "SELECT * FROM " + TableName +
                  " ORDER BY id DESC LIMIT 10";
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

QJsonObject AtmLogDB::getById(int id)
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

QJsonObject AtmLogDB::getByCondition(const QString &cond, const QString &id)
{
    QJsonObject resultObject; // 결과를 담을 QJsonObject
    // 2. SQL 쿼리 준비 (플레이스홀더 사용)
    QString connName;
    connName = QString("conn_%1").arg((quintptr)QThread::currentThreadId());
    QSqlQuery query(DataManager::instance().createThreadConnection(connName)); // m_db는 Database 클래스의 QSqlDatabase 멤버 변수입니다.
    QString queryString = QString("SELECT balance FROM %1 WHERE UID = :uid_value AND name = :name_value").arg(TableName);

    if (!query.prepare(queryString)) {
        qWarning() << "Failed to prepare query for getByCondition:" << query.lastError().text();
        return resultObject;
    }
    // 3. 플레이스홀더에 값 바인딩
    query.bindValue(":uid_value", cond);
    query.bindValue(":name_value", id);

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

bool AtmLogDB::insert(const QJsonObject &data)
{
    QString date = data["time"].toString();
    QString clientName = data["clientName"].toString();
    int datatype = data["SensorType"].toInt();

    // 2. SQL 쿼리 준비
    QString connName;
    connName = QString("conn_%1").arg((quintptr)QThread::currentThreadId());
    {
        QSqlQuery query(DataManager::instance().createThreadConnection(connName));
        QString queryString =QString("INSERT INTO %1 (dates, RFID_name, data_type) "
                                      "VALUES (:time, :clientName, :sensor)").arg(TableName);

        if (!query.prepare(queryString)) {
            qWarning() << "Failed to prepare query for getByUidAndName:" << query.lastError().text();
            return false;
        }
        query.bindValue(":time", date);
        query.bindValue(":clientName", clientName);
        query.bindValue(":sensor", datatype);
        if (!query.exec()) {
            qDebug() << "Insert failed:" << query.lastError().text();
            return false;
        } else {
            qDebug() << "Insert success!";
            return true;
        }
        if (query.isActive()) query.finish();
        query.clear();
    }
    DataManager::instance().closeThreadConnection(connName);
    return true;
}

bool AtmLogDB::update(int id, const QJsonObject &data)
{
    QJsonObject recvdata;
    if(id==0){ //deposit
        //여기서 data열어서 UID랑 amount랑 name들고옴.
        if(!data.isEmpty()&&data["data"].isObject()){
            recvdata = data["data"].toObject();
        }
        QString UID = recvdata["UID"].toString();
        qDebug()<<UID;
        QString Ruid = "-1";
        if(UID=="B1457D09"){
            qDebug()<<"here";
            Ruid="12345678";
        }else if(UID=="F3CC65BD"){
            Ruid="87654321";
        }else {
            Ruid="-1";
        }
        QString amount = recvdata["amount"].toString();
        QString connName;
        connName = QString("conn_%1").arg((quintptr)QThread::currentThreadId());
        {
            QSqlQuery query(DataManager::instance().createThreadConnection(connName));
            QString queryString = QString("UPDATE %1 SET balance = balance + :amount WHERE UID = :uid").arg(TableName);
            //prepare
            if (!query.prepare(queryString)) {
                qWarning() << "Deposit query prepare failed:" << query.lastError().text();
                return false;
            }
            //bind value
            query.bindValue(":amount", amount);
            query.bindValue(":uid", Ruid);

            if (!query.exec()) {
                m_lastError = query.lastError();
                qDebug() << "Deposit 실패:" << m_lastError.text();
                return false;
            }
            if (query.numRowsAffected() == 0) {
                qDebug() << "Deposit failed No Data" << Ruid;
                return false;
            }
            if (query.isActive()) query.finish();
            query.clear();
        }
        DataManager::instance().closeThreadConnection(connName);
        qDebug() << "Deposit success UID:" << UID << "Amount:" << amount;
        return true;
    }
    else if(id==1){//withdraw
        if(!data.isEmpty()&&data["data"].isObject()){
            recvdata = data["data"].toObject();
        }
        QString UID = recvdata["UID"].toString();
        QString Ruid = "-1";
        QString amount = recvdata["amount"].toString();
        QString connName;
        connName = QString("conn_%1").arg((quintptr)QThread::currentThreadId());
        {
            QSqlQuery query(DataManager::instance().createThreadConnection(connName));
            QString queryString = QString("UPDATE %1 SET balance = balance - :amount WHERE UID = :uid AND balance >= :amount").arg(TableName);
            //prepare
            if (!query.prepare(queryString)) {
                qWarning() << "withdraw query prepare failed:" << query.lastError().text();
                return false;
            }
            if(UID=="B1457D09"){
                Ruid="12345678";
            }else if(UID=="F3CC65BD"){
                Ruid="87654321";
            }else {
                Ruid="-1";
            }
            //bind value
            query.bindValue(":amount", amount);
            query.bindValue(":uid", Ruid);

            if (!query.exec()) {
                qWarning() << "withdraw query exec failed:" << query.lastError().text();
                return false;
            }
            if (query.numRowsAffected() == 0) {
                qDebug() << "Withdraw failed (Insufficient funds) UID:" << UID;
                return false;
            }
            if (query.isActive()) query.finish();
            query.clear();
        }
        DataManager::instance().closeThreadConnection(connName);
        qDebug() << "Withdraw success UID:" << UID << "Amount:" << amount;
        return true;
    }
    else if(id==2){//send
        if (!DataManager::instance().getDB().transaction()) {
            qWarning() << "Failed to start transaction:" << DataManager::instance().getDB().lastError().text();
            return false;
        }
        if(!data.isEmpty()&&data["data"].isObject()){
            recvdata = data["data"].toObject();
        }
        QString FromUid = recvdata["UID"].toString();
        QString FromUID="-1";
        if(FromUid=="B1457D09"){
            FromUID="12345678";
        }else if(FromUid=="F3CC65BD"){
            FromUID="87654321";
        }else {
            FromUID="-1";
        }
        QString amount = recvdata["amount"].toString();
        QString ToUID = recvdata["targetUID"].toString();

        // 1. 출금 (보내는 사람)
        QSqlQuery withdrawQuery(DataManager::instance().getDB());
        QString withdrawStr = QString("UPDATE %1 SET balance = balance - :amount WHERE UID = :uid AND balance >= :amount").arg(TableName);

        if (!withdrawQuery.prepare(withdrawStr)) {
            qWarning() << "Withdraw prepare failed:" << withdrawQuery.lastError().text();
            DataManager::instance().getDB().rollback();
            return false;
        }
        withdrawQuery.bindValue(":amount", amount);
        withdrawQuery.bindValue(":uid", FromUID);

        if (!withdrawQuery.exec() || withdrawQuery.numRowsAffected() == 0) {
            qWarning() << "Withdraw failed (Insufficient funds):" << withdrawQuery.lastError().text();
            DataManager::instance().getDB().rollback();
            return false;
        }

        // 2. 예금 (받는 사람)
        QSqlQuery depositQuery(DataManager::instance().getDB());
        QString depositStr = QString("UPDATE %1 SET balance = balance + :amount WHERE UID = :uid").arg(TableName);
        if (!depositQuery.prepare(depositStr)) {
            qWarning() << "Deposit prepare failed:" << depositQuery.lastError().text();
            DataManager::instance().getDB().rollback();
            return false;
        }
        depositQuery.bindValue(":amount", amount);
        depositQuery.bindValue(":uid", ToUID);

        if (!depositQuery.exec() || depositQuery.numRowsAffected() == 0) {
            qWarning() << "Deposit failed (Receiver not found):" << depositQuery.lastError().text();
            DataManager::instance().getDB().rollback();
            return false;
        }

        // 3. 커밋
        if (!DataManager::instance().getDB().commit()) {
            qWarning() << "Transaction commit failed:" << DataManager::instance().getDB().lastError().text();
            DataManager::instance().getDB().rollback();
            return false;
        }
        return true;
    }
    else
    {
        qDebug()<<"error came exit";
        return false;
    }
}

bool AtmLogDB::remove(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM " + TableName + " WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        m_lastError = query.lastError();
        qDebug() << "삭제 실패:" << m_lastError.text();
        return false;
    }
    return true;
}

QString AtmLogDB::buildUpdateQuery(const QString &table, int id, const QJsonObject &data)
{
    QStringList setStatements;

    for (auto it = data.begin(); it != data.end(); ++it) {
        setStatements.append(it.key() + " = :" + it.key());
    }

    return QString("UPDATE %1 SET %2 WHERE id = :id")
        .arg(table)
        .arg(setStatements.join(", "));
}

QString AtmLogDB::buildInsertQuery(const QString &table, const QJsonObject &data)
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

void AtmLogDB::bindJsonToQuery(QSqlQuery &query, const QJsonObject &data)
{
    for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
        QString placeholder = ":" + it.key();
        QVariant value = it.value().toVariant();
        query.bindValue(placeholder, value);
    }
}
