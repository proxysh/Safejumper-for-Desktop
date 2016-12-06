#ifndef UPDATE_H
#define UPDATE_H

#include <QString>

#ifdef Q_OS_MAC
#define SJ_UPDATE_URL "https://proxy.sh/safejumper_mac.xml"
#endif

#ifdef Q_OS_WIN
#define SJ_UPDATE_URL "https://proxy.sh/safejumper_windows.xml"
#endif

#ifdef Q_OS_LINUX
#define SJ_UPDATE_URL "https://proxy.sh/safejumper_linux.xml"
#endif

#endif // UPDATE_H

