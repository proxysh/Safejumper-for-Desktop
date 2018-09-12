#!/bin/bash
# Run this script from the buildlinux folder to build a redhat package
qmake DEFINES+="Q_OS_REDHAT" ../src
make
cp safejumper/safejumper linuxfiles
cp service/safejumperservice linuxfiles
cp launchopenvpn/launchopenvpn linuxfiles
cp netdown/netdown linuxfiles
cp openvpn64 linuxfiles/openvpn
cp -r env64 linuxfiles/env
cp debian/safejumper.service linuxfiles

# Then the content of linuxfiles mostly goes into /opt/safejumper/.

# To package redhat do the following:

tar --transform "s/^linuxfiles/safejumper-$1/" -zcpvf ~/rpmbuild/SOURCES/safejumper-$1.tar.gz linuxfiles
rpmbuild --define "debug_package %{nil}" -ba --sign -v ./safejumper.spec

