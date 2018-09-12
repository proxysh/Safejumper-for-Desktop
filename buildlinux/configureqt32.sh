#!/bin/bash
export SKIP="qtlocation qtdeclarative qtgamepad qt3d qtchart qtwayland qtwebchannel qtquickcontrols qtquickcontrols2 qtmultimedia qtconnectivity qtwebengine"
for f in $SKIP; do
  rm -fR $f
done
export OPENSSL_LIBS="/usr/local/lib/libssl.a /usr/local/lib/libcrypto.a"
./configure -platform linux-g++-32 -confirm-license -debug -opensource -nomake examples -nomake tests -openssl-linked -static -qt-libpng -no-opengl -silent -v $1
