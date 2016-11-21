#-------------------------------------------------
#
# Project created by QtCreator 2015-06-21T18:49:37
#
#-------------------------------------------------

QT       += core
QT       -= gui

TARGET = launchopenvpn
#CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

QMAKE_INCDIR += ../safejumper/

SOURCES += main.cpp \
	../safejumper/pathhelper.cpp \
    common.cpp

HEADERS  += \
	../safejumper/pathhelper.h \
    common.h
