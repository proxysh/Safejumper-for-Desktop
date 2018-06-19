#!/bin/bash
autoreconf -i -v -f
export OPENSSL_CFLAGS="-I/opt/local/include" 
export OPENSSL_LIBS="/opt/local/lib/libssl.a /opt/local/lib/libcrypto.a /opt/local/lib/libz.a" 
export LZO_CFLAGS="-I/opt/local/include" 
export LZO_LIBS="/opt/local/lib/liblzo2.a /opt/local/lib/libssl.a /opt/local/lib/libcrypto.a /opt/local/lib/libz.a" 
export LZ4_CFLAGS="-I/opt/local/include"
export LZ4_LIBS="/opt/local/lib/liblz4.a /opt/local/lib/libssl.a /opt/local/lib/libcrypto.a /opt/local/lib/libz.a"
./configure --enable-static=yes --enable-shared=yes --enable-crypto --disable-debug --disable-plugin-auth-pam --disable-dependency-tracking
make
