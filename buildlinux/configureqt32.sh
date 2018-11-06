#!/bin/bash
export SKIP="qtlocation qtgamepad qt3d qtchart qtwayland qtwebchannel qtmultimedia qtconnectivity qtwebengine"
for f in $SKIP; do
  rm -fR $f
done
export OPENSSL_LIBS="-L/usr/local/lib -lssl -lcrypto"
./configure -platform linux-g++-32 -confirm-license -debug -opensource -nomake examples -nomake tests -openssl-linked -static -qt-libpng -no-opengl -silent -v $1
