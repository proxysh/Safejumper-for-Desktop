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
CONFIG += static

TEMPLATE = app

QMAKE_INCDIR += ../safejumper/ \
    ../common/

SOURCES += main.cpp \
        ../common/pathhelper.cpp \
    common.cpp

HEADERS  += \
        ../common/pathhelper.h \
    common.h
