#include "loginwindow.h"

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#endif

#include <QApplication>
#ifndef Q_OS_OSX
#include "qtsingleapplication.h"
#define THE_APP_CLASS QtSingleApplication
#else
#define THE_APP_CLASS QApplication
#endif

#include "log.h"

THE_APP_CLASS * g_pTheApp;
int main(int argc, char *argv[])
{
#ifdef WIN32
    WORD ver = MAKEWORD(2, 2);
    WSADATA wsa;
    int r0 = WSAStartup(ver, &wsa);
    if (r0 != 0) {
        fprintf(stderr, "Cannot init winsock lib");
        return 2;
    }
#endif

    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setSetuidAllowed(true);
    QApplication::setOrganizationName("proxy.sh");
    QApplication::setApplicationName("Safejumper");
    g_pTheApp = new THE_APP_CLASS(argc, argv);

    log::logt("Starting Application");
    LoginWindow::Instance()->show();
    int res = g_pTheApp->exec();
    LoginWindow::Cleanup();
    log::logt("Quit Application");
    delete g_pTheApp;
    return res;
}
