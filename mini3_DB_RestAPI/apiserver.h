#ifndef APISERVER_H
#define APISERVER_H

#include <QThread>

class APIServer : public QThread
{
    Q_OBJECT
public:
    explicit APIServer(QObject *parent = nullptr);
};

#endif // APISERVER_H
