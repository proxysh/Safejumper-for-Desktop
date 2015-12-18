#-------------------------------------------------
#
# Project created by QtCreator 2015-12-17T20:43:15
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = netdown
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

#QMAKE_INCDIR += ../safejumper/
QMAKE_INCDIR += ../launchopenvpn/

SOURCES += main.cpp \
#	../safejumper/pathhelper.cpp \
    ../launchopenvpn/common.cpp

HEADERS  += \
#	../safejumper/pathhelper.h \
    ../launchopenvpn/common.h
