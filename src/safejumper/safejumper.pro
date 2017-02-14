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
    authmanager.cpp \
    common.cpp \
    confirmationdialog.cpp \
    connectiondialog.cpp \
    dlg_newnode.cpp \
    encryptiondelegate.cpp \
    errordialog.cpp \
    flag.cpp \
    fonthelper.cpp \
    locationdelegate.cpp \
    log.cpp \
    loginwindow.cpp \
    main.cpp \
    mapscreen.cpp \
    ministun.c \
    openvpnmanager.cpp \
    osspecific.cpp \
    pathhelper.cpp \
    pingwaiter.cpp \
    portforwarder.cpp \
    protocoldelegate.cpp \
    scr_logs.cpp \
    scr_settings.cpp \
    setting.cpp \
    stun.cpp \
    trayiconmanager.cpp \
    wndmanager.cpp

HEADERS += \
    authmanager.h \
    common.h \
    confirmationdialog.h \
    connectiondialog.h \
    dlg_newnode.h \
    encryptiondelegate.h \
    errordialog.h \
    flag.h \
    fonthelper.h \
    locationdelegate.h \
    log.h \
    loginwindow.h \
    mapscreen.h \
    ministun.h \
    openvpnmanager.h \
    osspecific.h \
    pathhelper.h \
    pingwaiter.h \
    portforwarder.h \
    protocoldelegate.h \
    scr_logs.h \
    scr_settings.h \
    setting.h \
    stun.h \
    trayiconmanager.h \
    update.h \
    version.h \
    wndmanager.h

FORMS += \
    confirmationdialog.ui \
    connectiondialog.ui \
    dlg_newnode.ui \
    errordialog.ui \
    loginwindow.ui \
    mapscreen.ui \
    scr_logs.ui \
    scr_settings.ui

RESOURCES += \
    imgs.qrc

