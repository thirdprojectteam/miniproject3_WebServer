#ifndef NETWORKREADER_H
#define NETWORKREADER_H

#include <QObject>

class NetWorkReader : public QObject
{
    Q_OBJECT
public:
    explicit NetWorkReader(QObject *parent = nullptr);

signals:
};

#endif // NETWORKREADER_H
