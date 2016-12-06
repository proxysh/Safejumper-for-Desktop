
#include "common.h"
#include "runit.h"

#include <QStringList>

#include <QDebug>

const QStringList commands = {
#ifdef Q_OS_MAC
    "/sbin/ifconfig en0 down",
    "/sbin/ifconfig en1 down",
    "/usr/sbin/networksetup -setairportpower Wi-Fi off"
#else
    "/sbin/ifconfig eth0 down",
    "/sbin/ifconfig eth1 down",
#endif
};

int main(int argc, char *argv[])
{
    become_root();

    for (int k = 0; k < commands.size(); ++k)
        runit(commands[k]);

    return 0;
}
