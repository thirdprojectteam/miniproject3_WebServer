#ifndef ENDPOINTS_H
#define ENDPOINTS_H

#include <QObject>

class EndPoints : public QObject
{
    Q_OBJECT
public:
    EndPoints(QObject *parent = nullptr);
    ~EndPoints();

private:
};

#endif // ENDPOINTS_H
