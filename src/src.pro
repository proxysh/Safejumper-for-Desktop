#-------------------------------------------------
# Safejumper sources in one project so qmake can
# create a makefile to build it all at once.
#-------------------------------------------------

TEMPLATE = subdirs

SUBDIRS = \
	launchopenvpn \
	netdown \
	safejumper

