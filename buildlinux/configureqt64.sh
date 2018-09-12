#!/bin/bash
export SKIP="qtlocation qtdeclarative qtgamepad qt3d qtchart qtwayland qtwebchannel qtquickcontrols qtquickcontrols2 qtmultimedia qtconnectivity qtwebengine"
for f in $SKIP; do
  rm -fR $f
done
export OPENSSL_LIBS="/usr/local/lib64/libssl.a /usr/local/lib64/libcrypto.a -lssl -lcrypto"
./configure -platform linux-g++-64 -confirm-license -debug -opensource -nomake examples -nomake tests -openssl-linked -static -qt-libpng -no-opengl -v $1
