#include "sjmainwindow.h"

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#endif

#include <QApplication>
#ifndef Q_OS_OSX
#include "singleapplication.h"
#define THE_APP_CLASS SingleApplication
#else
#define THE_APP_CLASS QApplication
#endif

#include "scr_connect.h"
#include "log.h"

QApplication * g_pTheApp;
int main(int argc, char *argv[])
{
#ifdef WIN32
	WORD ver = MAKEWORD(2, 2);
	WSADATA wsa;
	int r0 = WSAStartup(ver, &wsa);
	if (r0 != 0)
	{
		fprintf(stderr, "Cannot init winsock lib");
		return 2;
	}
#endif

	THE_APP_CLASS::setSetuidAllowed(true);
#ifndef Q_OS_OSX
	QApplication::setApplicationName("Safejumper");
	QApplication::setOrganizationName("proxysh");
#endif
	THE_APP_CLASS a(argc, argv);
//	QApplication a(argc, argv);
	g_pTheApp = &a;

	log::logt("Starting Application");
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	SjMainWindow::Instance()->show();    
	int res = a.exec();
	SjMainWindow::Cleanup();
	log::logt("Quit Application");
	return res;
}
