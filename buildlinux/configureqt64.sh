#!/bin/bash
export SKIP="qtlocation qtgamepad qt3d qtchart qtwayland qtwebchannel qtmultimedia qtconnectivity qtwebengine"
for f in $SKIP; do
  rm -fR $f
done
export OPENSSL_LIBS="-L/usr/local/lib64 -lssl -lcrypto"
./configure -platform linux-g++-64 -confirm-license -debug -opensource -nomake examples -nomake tests -openssl-linked -static -qt-libpng -no-opengl -v $1
