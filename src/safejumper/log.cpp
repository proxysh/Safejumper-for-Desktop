#include "log.h"


#ifndef NDEBUG
#include <QtDebug>
#endif
#include <QFile>
#include <QDateTime>

#include "scr_logs.h"
#include "pathhelper.h"

void log::logt(const QString & s)
{
    QDateTime now = QDateTime::currentDateTimeUtc();
    QString s1 = now.toString("yyyy-MM-dd-HH-mm-ss ") + s;
#ifndef NDEBUG
    QMessageLogger(__FILE__, __LINE__, 0).debug() << s1;
#endif
    s1 +=  + "\n";
    QFile ff(PathHelper::Instance()->LogPfn());
    if (ff.open(QIODevice::Append)) {
        ff.write(s1.toLatin1());
        ff.flush();
        ff.close();
    }

    if (Scr_Logs::IsExists())
        Scr_Logs::Instance()->Log(s1);
}



