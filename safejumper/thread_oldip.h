#ifndef THREAD_OLDIP_H
#define THREAD_OLDIP_H

#include <QThread>

class Thread_OldIp : public QThread
{
	Q_OBJECT
public:
	Thread_OldIp(QObject *parent = 0) : QThread(parent) {}
    void run() Q_DECL_OVERRIDE;

signals:
    void resultReady(const QString &s);
};

#endif // THREAD_OLDIP_H
