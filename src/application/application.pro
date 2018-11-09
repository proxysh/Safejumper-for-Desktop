#-------------------------------------------------
#
# Project created by QtCreator 2015-02-25T18:16:55
#
#-------------------------------------------------

QT	   += network xml core gui widgets qml quick

TARGET = shieldtra
APPNAME = Shieldtra
TEMPLATE = app
INCLUDEPATH += ../common

macx: {
    TARGET = $${APPNAME}
    QMAKE_INFO_PLIST = ./Info.plist
    QMAKE_LFLAGS += -F /System/Library/Frameworks/
    QMAKE_RPATHDIR += @executable_path/../Frameworks
    LIBS += -framework Security -framework ServiceManagement -framework Foundation
    target.path = /Applications
    resources.path = /Applications/$${APPNAME}.app/Contents/Resources
    resources.files = ./resources/*
    INSTALLS = target resources
    ICON = application.icns

    include (../common/certificate.pri)

    INFO_PLIST_PATH = $$shell_quote($$(PWD)/$${APPNAME}.app/Contents/Info.plist)

    APP_IDENTIFIER = com.shieldtra.$${APPNAME}
    HELPER_IDENTIFIER = com.shieldtra.$${APPNAME}Helper

    plist.commands += $(COPY) $$PWD/Info.plist $${INFO_PLIST_PATH};
    # plist.commands += /usr/libexec/PlistBuddy -c \"Set :CFBundleIdentifier $${APP_IDENTIFIER}\" $${INFO_PLIST_PATH};
    # plist.commands += /usr/libexec/PlistBuddy -c \'Set :SMPrivilegedExecutables:$${HELPER_IDENTIFIER} 'anchor apple generic and identifier \\\"$${HELPER_IDENTIFIER}\\\" and (certificate leaf[field.1.2.840.113635.100.6.1.9] /* exists */ or certificate 1[field.1.2.840.113635.100.6.2.6] /* exists */ and certificate leaf[field.1.2.840.113635.100.6.1.13] /* exists */ and certificate leaf[subject.OU] = $${CERT_OU})'\' $${INFO_PLIST_PATH};
    first.depends = $(first) plist
    export(first.depends)
    export(plist.commands)
    QMAKE_EXTRA_TARGETS += first plist

    SOURCES += smjobbless.mm
    HEADERS += smjobbless.h
}

win32: {
    WINSDK_DIR = C:/Program Files/Microsoft SDKs/Windows/v7.1
    WIN_PWD = $$replace(PWD, /, \\)
    OUT_PWD_WIN = $$replace(OUT_PWD, /, \\)
    QMAKE_POST_LINK = "$$quote($$OUT_PWD_WIN\\..\\fixbinary.bat) $$quote($$OUT_PWD_WIN\\..\\shieldtra.exe)"
    RC_FILE = application.rc
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

lupdate_only{
SOURCES = *.qml
}

SOURCES += \
    authmanager.cpp \
    ../common/common.cpp \
    flag.cpp \
    ../common/log.cpp \
    main.cpp \
    ../common/osspecific.cpp \
    ../common/pathhelper.cpp \
    pingwaiter.cpp \
    portforwarder.cpp \
    server.cpp \
    serversmodel.cpp \
    setting.cpp \
    trayiconmanager.cpp \
    vpnservicemanager.cpp \
    mainwindow.cpp

HEADERS += \
    application.h \
    authmanager.h \
    ../common/common.h \
    flag.h \
    ../common/log.h \
    ../common/osspecific.h \
    ../common/pathhelper.h \
    pingwaiter.h \
    portforwarder.h \
    server.h \
    serversmodel.h \
    setting.h \
    trayiconmanager.h \
    vpnservicemanager.h \
    application.h \
    mainwindow.h

RESOURCES += \
    images.qrc \
    qml.qrc \
    maps.qrc \
    fonts.qrc

