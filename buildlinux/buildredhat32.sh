#!/bin/bash
# Run this script from the buildlinux folder to build a 32 bit redhat package
qmake DEFINES+="Q_OS_REDHAT" ../src
make
cp application/shieldtra linuxfiles
cp service/shieldtraservice linuxfiles
cp netdown/netdown linuxfiles
cp openvpn32 linuxfiles/openvpn
cp debian/shieldtra.service linuxfiles

# Then the content of linuxfiles mostly goes into /opt/shieldtra/.

# To package redhat do the following:

tar --transform "s/^linuxfiles/shieldtra-$1/" -zcpvf ~/rpmbuild/SOURCES/shieldtra-$1.tar.gz linuxfiles
rpmbuild --define "debug_package %{nil}" -ba --sign -v --target=i686 ./shieldtra.spec

