#-------------------------------------------------
#
# Project created by QtCreator 2015-02-25T18:16:55
#
#-------------------------------------------------

QT	   += network xml core gui widgets

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

win32: {
    WINSDK_DIR = C:/Program Files/Microsoft SDKs/Windows/v7.1
    WIN_PWD = $$replace(PWD, /, \\)
    OUT_PWD_WIN = $$replace(OUT_PWD, /, \\)
    QMAKE_POST_LINK = "$$quote($$OUT_PWD_WIN\\..\\fixbinary.bat) $$quote($$OUT_PWD_WIN\\..\\safejumper.exe) $$quote($$WIN_PWD\\$$basename(TARGET).manifest)"
    RC_FILE = safejumper.rc
    LIBS += -lws2_32 -lIphlpapi
    DESTDIR = ../../buildwindows/
    MOC_DIR = ../.obj/
    HEADERS += \
        qtlocalpeer.h \
        qtlockedfile.h \
        qtsingleapplication.h \

    SOURCES += \
        qtlocalpeer.cpp \
        qtlockedfile.cpp \
        qtsingleapplication.cpp \
}

linux: {
    HEADERS += \
        qtlocalpeer.h \
        qtlockedfile.h \
        qtsingleapplication.h \

    SOURCES += \
        qtlocalpeer.cpp \
        qtlockedfile.cpp \
        qtsingleapplication.cpp \

    CONFIG += static
}

SOURCES += \
    acconnectto.cpp \
    authmanager.cpp \
    common.cpp \
    confirmationdialog.cpp \
    dlg_error.cpp \
    dlg_newnode.cpp \
    flag.cpp \
    fonthelper.cpp \
    log.cpp \
    loginwindow.cpp \
    lvrowdelegate.cpp \
    lvrowdelegateencryption.cpp \
    lvrowdelegateprotocol.cpp \
    main.cpp \
    ministun.c \
    openvpnmanager.cpp \
    osspecific.cpp \
    pathhelper.cpp \
    pingwaiter.cpp \
    portforwarder.cpp \
    scr_connect.cpp \
    scr_logs.cpp \
    scr_map.cpp \
    scr_settings.cpp \
    scr_table.cpp \
    setting.cpp \
    stun.cpp \
    thread_oldip.cpp \
    thread_forwardports.cpp \
    trayiconmanager.cpp \
    wndmanager.cpp

HEADERS += \
    acconnectto.h \
    authmanager.h \
    common.h \
    confirmationdialog.h \
    dlg_error.h \
    dlg_newnode.h \
    flag.h \
    fonthelper.h \
    log.h \
    loginwindow.h \
    lvrowdelegate.h \
    lvrowdelegateencryption.h \
    lvrowdelegateprotocol.h \
    ministun.h \
    openvpnmanager.h \
    osspecific.h \
    pathhelper.h \
    pingwaiter.h \
    portforwarder.h \
    scr_connect.h \
    scr_logs.h \
    scr_map.h \
    scr_settings.h \
    scr_table.h \
    setting.h \
    stun.h \
    thread_forwardports.h \
    thread_oldip.h \
    trayiconmanager.h \
    update.h \
    version.h \
    wndmanager.h

FORMS += \
    confirmationdialog.ui \
    dlg_error.ui \
    dlg_newnode.ui \
    loginwindow.ui \
    scr_connect.ui \
    scr_logs.ui \
    scr_map.ui \
    scr_settings.ui \
    scr_table.ui

RESOURCES += \
    imgs.qrc

