#-------------------------------------------------
# Safejumper sources in one project so qmake can
# create a makefile to build it all at once.
#-------------------------------------------------

TEMPLATE = subdirs

SUBDIRS = \
	safechecker \
	safejumper

macx: {
SUBDIRS += \
    launchopenvpn \
    netdown
}

linux: {
SUBDIRS += \
    launchopenvpn \
    netdown
}

