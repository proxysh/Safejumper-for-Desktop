#!/bin/bash
patch -p1 < patches/02-tunnelblick-openvpn_xorpatch-a.diff
patch -p1 < patches/03-tunnelblick-openvpn_xorpatch-b.diff
patch -p1 < patches/04-tunnelblick-openvpn_xorpatch-c.diff
patch -p1 < patches/05-tunnelblick-openvpn_xorpatch-d.diff
patch -p1 < patches/06-tunnelblick-openvpn_xorpatch-e.diff
autoreconf -i -v -f
export OPENSSL_CFLAGS="-I/opt/local/include" 
export OPENSSL_LIBS="/opt/local/lib/libssl.a /opt/local/lib/libcrypto.a /opt/local/lib/libz.a" 
export LZO_CFLAGS="-I/opt/local/include" 
export LZO_LIBS="/opt/local/lib/liblzo2.a /opt/local/lib/libssl.a /opt/local/lib/libcrypto.a /opt/local/lib/libz.a" 
./configure --enable-static=yes --enable-shared=yes --enable-crypto --disable-debug --disable-plugin-auth-pam --disable-dependency-tracking
make
