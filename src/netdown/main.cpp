
#include <sys/unistd.h>

#include <QtGlobal>
#ifdef Q_OS_DARWIN
#include <unistd.h>
#endif

#include <string>
#include <cstdio>
#include <vector>

#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QVector>

void become_root()
{
    uid_t uidBefore  = getuid();
    uid_t euidBefore = geteuid();
    printf("@@ uidBefore = %d, euidBefore = %d, %s\n", uidBefore, euidBefore, (uidBefore == euidBefore ? "EQUAL" : "NOT EQUAL") );

    if ( (uidBefore != 0) || (euidBefore != 501))
        fprintf(stderr, "@@ Error: become root: Not root and not non-root\n");

    if (euidBefore != 0) {
        int result = seteuid(0);
        if (result != 0)
            fprintf(stderr, "Error: Unable to become root, res = %d\n", result);
        else
            printf("@@seteuid(0), res = %d\n", result);
        printf("@@ euid = %d\n", geteuid());
    }

    if (uidBefore != 0) {
        int r3 = setuid(0);
        if (r3 != 0)
            fprintf(stderr, "Error: Unable to setuid(0), res = %d\n", r3);
        else
            printf("@@setuid(0), res = %d\n", r3);
    }

    printf("@@ uid = %d\n", getuid());
    printf("@@ euid = %d\n", geteuid());
}

void runit(const QString &command, unsigned int ms_delay = 500)
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

int main(int argc, char *argv[])
{
    QStringList commands;
#ifdef Q_OS_DARWIN
    commands << "/sbin/ifconfig en0 down" <<
        "/sbin/ifconfig en1 down" <<
        "/usr/sbin/networksetup -setairportpower Wi-Fi off";
#else
    if (QFile::exists("/sbin/ifconfig")) {
        commands << "/sbin/ifconfig eth0 down" <<
            "/sbin/ifconfig eth1 down";
    } else {
        // No ifconfig, so use ip
        commands << "/sbin/ip link set group default down";
    }
#endif
    become_root();

    for (int k = 0; k < commands.size(); ++k)
        runit(commands[k]);

    return 0;
}
