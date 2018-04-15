#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#ifndef Q_OS_OSX
#include "qtsingleapplication.h"
#define THE_APP_CLASS QtSingleApplication
#else
#define THE_APP_CLASS QApplication
#endif

extern THE_APP_CLASS * g_pTheApp;

#endif // APPLICATION_H
