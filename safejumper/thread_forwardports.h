#ifndef THREAD_FORWARDPORTS_H
#define THREAD_FORWARDPORTS_H

#include <QThread>
#include <QNetworkAccessManager>

#include "common.h"

class Thread_ForwardPorts : public QThread
{
    Q_OBJECT
public:
    Thread_ForwardPorts(QNetworkAccessManager & nam, const UVec & ports, QObject *parent = 0)
        : QThread(parent), _nam(nam) , _ports(ports) {}
    void run() Q_DECL_OVERRIDE;

private:
    QNetworkAccessManager & _nam;
    UVec _ports;
};

#endif // THREAD_FORWARDPORTS_H
