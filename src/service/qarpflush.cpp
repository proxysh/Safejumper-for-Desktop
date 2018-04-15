#include "qarpflush.h"

QArpFlush::QArpFlush(QObject *parent) : QObject(parent)
{
    connect(&m_process, SIGNAL(finished(int)), this, SLOT(processFinished(int)));
}

void QArpFlush::flushARP()
{
    m_process.execute("arp -d *");
}

void QArpFlush::processFinished(int exitCode)
{
    Q_UNUSED(exitCode);

    emit finished();
}
