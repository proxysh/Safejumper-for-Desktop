#include "runit.h"

#include <QProcess>
#include <QString>

#include <QDebug>

void runit(const QString &command, unsigned int ms_delay)
{
    QProcess process;

    qDebug() << "@@ Running cmd '" << command;
    process.start(command);
    if (!process.waitForFinished(ms_delay)) {
        QString errors(process.readAllStandardError());
        qDebug() << "@@ error: " << errors;
        if (process.state() != QProcess::NotRunning) {
            process.terminate();
            process.kill();
        }
    } else {
        QString output(process.readAllStandardOutput());
        if (!output.isEmpty())
            qDebug() << "@@ output: " << output;
    }
}
