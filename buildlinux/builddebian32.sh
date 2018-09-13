#!/bin/bash
# Run this script from the buildlinux folder to build a debian/ubuntu package
qmake ../src
make
cp safejumper/safejumper linuxfiles
cp service/safejumperservice linuxfiles
cp launchopenvpn/launchopenvpn linuxfiles
cp netdown/netdown linuxfiles
cp openvpn32 linuxfiles/openvpn
rm -fR linuxfiles/env
cp -r env32 linuxfiles/env

# Then the content of linuxfiles mostly goes into /opt/safejumper/.

# To package debian/ubuntu do the following:

tar -zcpvf ../safejumper_$1_orig.tar.gz linuxfiles
cd ../
tar -zxpvf safejumper_$1_orig.tar.gz
cp -r buildlinux/debian linuxfiles/
cd linuxfiles
dpkg-buildpackage -b -uc -us -ai386

