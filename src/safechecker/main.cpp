/***************************************************************************
 *   Copyright (C) 2017 by Jeremy Whiting <jeremypwhiting@gmail.com>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation version 2 of the License.                *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

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

#include "testdialog.h"
#include "log.h"
#include "setting.h"

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
    QApplication::setApplicationName("Safechecker");
    g_pTheApp = new THE_APP_CLASS(argc, argv);
    Setting::instance()->setTesting(true); // This application uses "testing" settings

    log::logt("Starting Application");
    LoginWindow::Instance()->show();
    int res = g_pTheApp->exec();
    LoginWindow::Cleanup();
    log::logt("Quit Application");
    delete g_pTheApp;
    return res;
}
