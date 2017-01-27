#-------------------------------------------------
#
# Project created by QtCreator 2015-02-25T18:16:55
#
#-------------------------------------------------

QT	   += network xml core gui widgets
u
TARGET = serverchecker
TEMPLATE = app
INCLUDEPATH += ../safejumper/

macx: {
    TARGET = ServerChecker
    QMAKE_INFO_PLIST = ./Info.plist
    QMAKE_LFLAGS += -F /System/Library/Frameworks/
    QMAKE_RPATHDIR += @executable_path/../Frameworks
    LIBS += -framework Security
    target.path = /Applications
    resources.path = /Applications/ServerChecker.app/Contents/Resources
    resources.files = ./resources/*
    INSTALLS = target resources
    ICON = Safejumper.icns
}

win32: {
    WINSDK_DIR = C:/Program Files/Microsoft SDKs/Windows/v7.1
    WIN_PWD = $$replace(PWD, /, \\)
    OUT_PWD_WIN = $$replace(OUT_PWD, /, \\)
#    QMAKE_POST_LINK = "$$quote($$OUT_PWD_WIN\\..\\fixbinary.bat) $$quote($$OUT_PWD_WIN\\..\\serverchecker.exe) $$quote($$WIN_PWD\\$$basename(TARGET).manifest)"
    RC_FILE = ../safejumper/safejumper.rc
    LIBS += -lws2_32 -lIphlpapi
    DESTDIR = ../../buildwindows/
    MOC_DIR = ../.obj/
    HEADERS += \
        ../safejumper/qtlocalpeer.h \
        ../safejumper/qtlockedfile.h \
        ../safejumper/qtsingleapplication.h \

    SOURCES += \
        ../safejumper/qtlocalpeer.cpp \
        ../safejumper/qtlockedfile.cpp \
        ../safejumper/qtsingleapplication.cpp \
}

linux: {
    HEADERS += \
        ../safejumper/qtlocalpeer.h \
        ../safejumper/qtlockedfile.h \
        ../safejumper/qtsingleapplication.h \

    SOURCES += \
        ../safejumper/qtlocalpeer.cpp \
        ../safejumper/qtlockedfile.cpp \
        ../safejumper/qtsingleapplication.cpp \

    CONFIG += static
}

SOURCES += \
    ../safejumper/authmanager.cpp \
    ../safejumper/common.cpp \
    ../safejumper/confirmationdialog.cpp \
    ../safejumper/dlg_error.cpp \
    ../safejumper/dlg_newnode.cpp \
    ../safejumper/flag.cpp \
    ../safejumper/fonthelper.cpp \
    ../safejumper/log.cpp \
    ../safejumper/loginwindow.cpp \
    ../safejumper/lvrowdelegate.cpp \
    ../safejumper/lvrowdelegateencryption.cpp \
    ../safejumper/lvrowdelegateprotocol.cpp \
    main.cpp \
    ../safejumper/ministun.c \
    ../safejumper/openvpnmanager.cpp \
    ../safejumper/osspecific.cpp \
    ../safejumper/pathhelper.cpp \
    ../safejumper/pingwaiter.cpp \
    ../safejumper/portforwarder.cpp \
    scr_settings.cpp \
    scr_logs.cpp \
    scr_map.cpp \
    setting.cpp \
    ../safejumper/stun.cpp \
    testdialog.cpp \
    ../safejumper/thread_forwardports.cpp \
    ../safejumper/thread_oldip.cpp \
    ../safejumper/trayiconmanager.cpp \
    wndmanager.cpp

HEADERS += \
    ../safejumper/authmanager.h \
    ../safejumper/common.h \
    ../safejumper/confirmationdialog.h \
    ../safejumper/dlg_error.h \
    ../safejumper/dlg_newnode.h \
    ../safejumper/flag.h \
    ../safejumper/fonthelper.h \
    ../safejumper/log.h \
    ../safejumper/loginwindow.h \
    ../safejumper/lvrowdelegate.h \
    ../safejumper/lvrowdelegateencryption.h \
    ../safejumper/lvrowdelegateprotocol.h \
    ../safejumper/ministun.h \
    ../safejumper/openvpnmanager.h \
    ../safejumper/osspecific.h \
    ../safejumper/pathhelper.h \
    ../safejumper/pingwaiter.h \
    ../safejumper/portforwarder.h \
    scr_logs.h \
    scr_map.h \
    scr_settings.h \
    setting.h \
    ../safejumper/stun.h \
    testdialog.h \
    ../safejumper/thread_forwardports.h \
    ../safejumper/thread_oldip.h \
    ../safejumper/trayiconmanager.h \
    ../safejumper/version.h \
    wndmanager.h

FORMS += \
    ../safejumper/confirmationdialog.ui \
    ../safejumper/dlg_error.ui \
    ../safejumper/dlg_newnode.ui \
    ../safejumper/loginwindow.ui \
    scr_logs.ui \
    scr_map.ui \
    scr_settings.ui \
    testdialog.ui

RESOURCES += \
    ../safejumper/imgs.qrc

