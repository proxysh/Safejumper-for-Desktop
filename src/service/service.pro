QT       += core network
CONFIG   += console qt c++11
# DEFINES += QTSERVICE_DEBUG

lessThan(QT_MAJOR_VERSION, 5) | lessThan(QT_MINOR_VERSION, 4) : error(Shieldtra requires Qt 5.6 or newer but Qt $$[QT_VERSION] was detected.)

TEMPLATE = app

CONFIG(debug, debug|release)::TARGET = shieldtraservice
CONFIG(release, debug|release)::TARGET = shieldtraservice

INCLUDEPATH += ../common

SOURCES  += main.cpp \
    ../common/common.cpp \
    ../common/osspecific.cpp \
    service.cpp \
    servicelog.cpp \
    servicepathhelper.cpp \
    qvpnserver.cpp \
    qvpnclientconnection.cpp \
    openvpnmanager.cpp

HEADERS  += service.h \
    ../common/common.h \
    ../common/osspecific.h \
    qvpnserver.h \
    qvpnclientconnection.h \
    openvpnmanager.h \
    servicelog.h \
    servicepathhelper.h

include(../common/qtservice/qtservice.pri)

#win32:RC_FILE = resource.rc

win32 {
    QMAKE_LFLAGS_WINDOWS = /SUBSYSTEM:WINDOWS,5.01
    QMAKE_LFLAGS_CONSOLE = /SUBSYSTEM:CONSOLE,5.01

    WIN_PWD = $$replace(PWD, /, \\)
    OUT_PWD_WIN = $$replace(OUT_PWD, /, \\)
    QMAKE_POST_LINK = "$$quote($$OUT_PWD_WIN\\..\\fixbinaryadmin.bat) $$quote($$OUT_PWD_WIN\\..\\shieldtraservice.exe) $$quote($$OUT_PWD_WIN\\..\\runasadmin.manifest)"
    DESTDIR = ../../buildwindows/
    MOC_DIR = ../.obj/
#    DEFINES += _ATL_XP_TARGETING
#    QMAKE_CFLAGS += /D _USING_V110_SDK71
#    QMAKE_CXXFLAGS += /D _USING_V110_SDK71
#    LIBS += -L”%ProgramFiles(x86)%/Microsoft SDKs/Windows/v7.1A/Lib/”
#    INCLUDEPATH += “%ProgramFiles(x86)%/Microsoft SDKs/Windows/v7.1A/Include/”

    LIBS   += -lShell32 -lws2_32  -liphlpapi -lrasapi32 -ladvapi32 -lversion
}

macx: {
    TARGET = com.shieldtra.ShieldtraHelper
    DESTDIR = ../
    CONFIG -= app_bundle
    target.path = /Applications
    resources.path = /Applications/Shieldtra.app/Contents/Resources
    resources.files = ./resources/*

    include (../common/certificate.pri)

    QMAKE_LFLAGS += -F /System/Library/Frameworks/Security.framework/ -sectcreate __TEXT __info_plist $$PWD/ShieldtraHelper-Info.plist -sectcreate __TEXT __launchd_plist $$PWD/com.shieldtra.ShieldtraHelper.plist
    LIBS += -framework Security -framework Cocoa

    DISTFILES += ShieldtraHelper-Info.plist \
                 com.shieldtra.ShieldtraHelper.plist

    # Bundle identifier for your application
    BUNDLEID = $${TARGET}

    HOSTAPP_IDENTIFIER = com.shieldtra.Shieldtra
    # QMAKE_PRE_LINK += /usr/libexec/PlistBuddy -c \'Set :SMAuthorizedClients:0 'anchor apple generic and identifier \\\"$${HOSTAPP_IDENTIFIER}\\\" and (certificate leaf[field.1.2.840.113635.100.6.1.9] /* exists */ or certificate 1[field.1.2.840.113635.100.6.2.6] /* exists */ and certificate leaf[field.1.2.840.113635.100.6.1.13] /* exists */ and certificate leaf[subject.OU] = $${CERT_OU})'\' $$PWD/ShieldtraHelper-Info.plist;


    # Extract debug symbols
    codesigner.commands += dsymutil $${DESTDIR}$${TARGET} -o $${DESTDIR}$${TARGET}.dSYM;

    # Sign the application, using the provided entitlements
    CODESIGN_ALLOCATE_PATH=$$system(xcrun -find codesign_allocate)
    codesigner.commands += export CODESIGN_ALLOCATE=$${CODESIGN_ALLOCATE_PATH};
    codesigner.commands += codesign --force --sign $${CERTSHA1} -r=\'designated => anchor apple generic and identifier \"$${BUNDLEID}\" and ((cert leaf[field.1.2.840.113635.100.6.1.9] exists) or (certificate 1[field.1.2.840.113635.100.6.2.6] exists and certificate leaf[field.1.2.840.113635.100.6.1.13] exists and certificate leaf[subject.OU]=$${CERT_OU}))\' --timestamp=none $${DESTDIR}$${TARGET} > /dev/null 2>&1;

    first.depends = $(first) codesigner
    export(first.depends)
    export(codesigner.commands)
    QMAKE_EXTRA_TARGETS += first codesigner
}
