#-------------------------------------------------
#
# Project created by QtCreator 2015-02-25T18:16:55
#
#-------------------------------------------------

QT	   += network xml core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = safejumper
TEMPLATE = app

macx: {
	TARGET = Safejumper
	QMAKE_INFO_PLIST = ./Info.plist
	QMAKE_LFLAGS += -F /System/Library/Frameworks/
        QMAKE_RPATHDIR += @executable_path/../Frameworks
        LIBS += -framework Security
    target.path = /Applications
    resources.path = /Applications/Safejumper.app/Contents/Resources
    resources.files = ./resources/*
    INSTALLS = target resources
	ICON = Safejumper.icns
}

#	DEFINES += MONITOR_TOOL
#	DEFINES += Q_OS_REDHAT

win32: {
	WINSDK_DIR = C:/Program Files/Microsoft SDKs/Windows/v7.1A
	WIN_PWD = $$replace(PWD, /, \\)
	OUT_PWD_WIN = $$replace(OUT_PWD, /, \\)
	QMAKE_POST_LINK = "$$WINSDK_DIR/bin/mt.exe -manifest $$quote($$WIN_PWD\\$$basename(TARGET).manifest) -outputresource:$$quote($$OUT_PWD_WIN\\release\\safejumper.exe;1)"
	RC_FILE = safejumper.rc
        LIBS += -lws2_32 -lIphlpapi
}

SOURCES += main.cpp\
	sjmainwindow.cpp \
	retina.cpp \
	scr_connect.cpp \
	scr_settings.cpp \
	common.cpp \
	scr_logs.cpp \
	authmanager.cpp \
	scr_map.cpp \
	wndmanager.cpp \
	dlg_error.cpp \
	setting.cpp \
	ctr_openvpn.cpp \
	osspecific.cpp \
	log.cpp \
	flag.cpp \
    stun.cpp \
    thread_oldip.cpp \
    ministun.c \
    pingwaiter.cpp \
    dlg_confirmation.cpp \
    pathhelper.cpp \
    thread_forwardports.cpp \
    portforwarder.cpp \
    acconnectto.cpp \
    lvrowdelegate.cpp \
    lvrowdelegateprotocol.cpp \
    fonthelper.cpp \
    singleapplication.cpp \
    dlg_newnode.cpp \
    lvrowdelegateencryption.cpp \
    scr_table.cpp

HEADERS  += sjmainwindow.h \
	retina.h \
	scr_connect.h \
	scr_settings.h \
	common.h \
	scr_logs.h \
	authmanager.h \
	scr_map.h \
	wndmanager.h \
	dlg_error.h \
	setting.h \
	ctr_openvpn.h \
	osspecific.h \
	log.h \
	flag.h \
    stun.h \
    thread_oldip.h \
    ministun.h \
    pingwaiter.h \
    dlg_confirmation.h \
    pathhelper.h \
    thread_forwardports.h \
    portforwarder.h \
    acconnectto.h \
    lvrowdelegate.h \
    lvrowdelegateprotocol.h \
    fonthelper.h \
    version.h \
    update.h \
    singleapplication.h \
    dlg_newnode.h \
    lvrowdelegateencryption.h \
    scr_table.h

FORMS	+= sjmainwindow.ui \
	scr_connect.ui \
	scr_settings.ui \
	scr_logs.ui \
	scr_map.ui \
	dlg_error.ui \
    dlg_confirmation.ui \
    dlg_newnode.ui \
    scr_table.ui

RESOURCES += \
	imgs.qrc

