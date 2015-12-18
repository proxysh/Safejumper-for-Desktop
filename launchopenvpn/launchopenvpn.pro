#-------------------------------------------------
#
# Project created by QtCreator 2015-06-21T18:49:37
#
#-------------------------------------------------

QT       += core
QT       -= gui

# QApplication is widget
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = launchopenvpn
TEMPLATE = app

QMAKE_INCDIR += ../safejumper/

SOURCES += main.cpp \
    launcherworker.cpp \
	../safejumper/pathhelper.cpp \
    common.cpp

HEADERS  += \
    launcherworker.h \
	../safejumper/pathhelper.h \
    common.h

#FORMS    +=
