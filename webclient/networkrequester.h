#ifndef NETWORKREQUESTER_H
#define NETWORKREQUESTER_H

#include <QObject>

class NetWorkRequester : public QObject
{
    Q_OBJECT
public:
    explicit NetWorkRequester(QObject *parent = nullptr);

signals:
};

#endif // NETWORKREQUESTER_H
