
#include "common.h"
#include "runit.h"

#include <QStringList>

int main(int argc, char *argv[])
{
    QStringList commands;
#ifdef Q_OS_DARWIN
    commands << "/sbin/ifconfig en0 down" <<
        "/sbin/ifconfig en1 down" <<
        "/usr/sbin/networksetup -setairportpower Wi-Fi off";
#else
    commands << "/sbin/ifconfig eth0 down" <<
        "/sbin/ifconfig eth1 down";
#endif

    become_root();

    for (int k = 0; k < commands.size(); ++k)
        runit(commands[k]);

    return 0;
}
