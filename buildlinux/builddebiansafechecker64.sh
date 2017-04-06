#!/bin/bash
# Run this script from the buildlinux folder to build a debian/ubuntu package
qmake ../src
make
cp safechecker/safechecker linuxfiles
cp launchopenvpn/launchopenvpn linuxfiles
cp netdown/netdown linuxfiles
cp openvpn64 linuxfiles/openvpn
rm -fR linuxfiles/env
cp -r env64 linuxfiles/env

# Then the content of linuxfiles mostly goes into /opt/safechecker/.

# To package debian/ubuntu do the following:

tar -zcpvf ../safechecker_$1_orig.tar.gz linuxfiles
cd ../
tar -zxpvf safechecker_$1_orig.tar.gz
cp -r buildlinux/safecheckerdebian linuxfiles/debian
cd linuxfiles
dpkg-buildpackage -b -uc -us

