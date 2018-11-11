/***************************************************************************
 *   Copyright (C) 2018 by Jeremy Whiting <jeremypwhiting@gmail.com>       *
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

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#endif

#include <QApplication>
#include <QFontDatabase>
#include <QTranslator>
#ifndef Q_OS_OSX
#include "qtsingleapplication.h"
#define THE_APP_CLASS QtSingleApplication
#else
#define THE_APP_CLASS QApplication
#endif

#include "common.h"
#include "log.h"
#include "mainwindow.h"

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

//    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setSetuidAllowed(true);
    QApplication::setOrganizationName(kOrgName);
    QApplication::setApplicationName(kAppName);
    QApplication::setQuitOnLastWindowClosed(false);

    g_pTheApp = new THE_APP_CLASS(argc, argv);

    QFontDatabase::addApplicationFont(":/fonts/Roboto.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Roboto-Black.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Roboto-Bold.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Roboto-Medium.ttf");

    QFontDatabase database;
    Q_FOREACH(const QString &family, database.families()) {
        qDebug() << "Found font with family: " << family;
    }

    Log::logt("Starting Application");
    MainWindow::instance()->show();
    int res = g_pTheApp->exec();
    MainWindow::cleanup();
    Log::logt("Quit Application");
    delete g_pTheApp;
    return res;
}
