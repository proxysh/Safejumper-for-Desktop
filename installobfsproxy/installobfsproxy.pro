QT += core
QT -= gui

CONFIG += c++11

TARGET = installobfsproxy
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

QMAKE_INCDIR += ../launchopenvpn/
QMAKE_INCDIR += ../netdown/

SOURCES += main.cpp\
    ../launchopenvpn/common.cpp\
    ../netdown/runit.cpp


HEADERS  += \
    ../launchopenvpn/common.h\
    ../netdown/runit.h
