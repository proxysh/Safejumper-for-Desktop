#!/bin/bash
patch -p1 < patches/02-tunnelblick-openvpn_xorpatch-a.diff
patch -p1 < patches/03-tunnelblick-openvpn_xorpatch-b.diff
patch -p1 < patches/04-tunnelblick-openvpn_xorpatch-c.diff
patch -p1 < patches/05-tunnelblick-openvpn_xorpatch-d.diff
patch -p1 < patches/06-tunnelblick-openvpn_xorpatch-e.diff
autoreconf -i -v -f
export CFLAGS="-m32"
export CPPFLAGS="-m32"
export OPENSSL_CFLAGS="-I/usr/include" 
export OPENSSL_LIBS="/usr/lib/i386-linux-gnu/libssl.a /usr/lib/i386-linux-gnu/libcrypto.a /usr/lib/i386-linux-gnu/libz.a -lpthread -ldl" 
export LZO_CFLAGS="-I/usr/include" 
export LZO_LIBS="-llzo2 /usr/lib/i386-linux-gnu/libssl.a /usr/lib/i386-linux-gnu/libcrypto.a /usr/lib/i386-linux-gnu/libz.a -lpthread -ldl" 
./configure --enable-static=yes --enable-shared=no --enable-crypto --disable-debug --disable-plugin-auth-pam --disable-plugin-down-root --disable-dependency-tracking
make
