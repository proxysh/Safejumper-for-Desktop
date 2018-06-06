#ifndef QARPFLUSH_H
#define QARPFLUSH_H

#include <QProcess>
#include <QObject>

class QArpFlush : public QObject
{
    Q_OBJECT
public:
    explicit QArpFlush(QObject *parent = 0);
    void flushARP();

signals:
    void finished();

private slots:
    void processFinished(int exitCode);

private:
    QProcess m_process;
};

#endif // QARPFLUSH_H
