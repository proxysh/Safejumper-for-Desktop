#!/bin/bash
# Run this script from the buildlinux folder to build a debian/ubuntu package
qmake ../src
make
cp application/shieldtra linuxfiles
cp service/shieldtraservice linuxfiles
cp netdown/netdown linuxfiles
cp openvpn32 linuxfiles/openvpn

# Then the content of linuxfiles mostly goes into /opt/shieldtra/.

# To package debian/ubuntu do the following:

tar -zcpvf ../shieldtra_$1_orig.tar.gz linuxfiles
cd ../
tar -zxpvf shieldtra_$1_orig.tar.gz
cp -r buildlinux/debian linuxfiles/
cd linuxfiles
dpkg-buildpackage -b -uc -us -ai386

