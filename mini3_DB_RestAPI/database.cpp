#include "database.h"
#include <QJsonObject>
#include <QSqlQuery>
DataBase::DataBase(QObject *parent) : QObject(parent)
{
}

QString DataBase::buildUpdateQuery(const QString &table,int id, const QJsonObject &data)
{
    QStringList setStatements;

    for (auto it = data.begin(); it != data.end(); ++it) {
        setStatements.append(it.key() + " = :" + it.key());
    }

    return QString("UPDATE %1 SET %2 WHERE id = :id")
        .arg(table)
        .arg(setStatements.join(", "));
}

QString DataBase::buildInsertQuery(const QString &table,const QJsonObject &data)
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

void DataBase::bindJsonToQuery(QSqlQuery &query, const QJsonObject &data)
{
    for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
        QString placeholder = ":" + it.key();
        QVariant value = it.value().toVariant();
        query.bindValue(placeholder, value);
    }
}
