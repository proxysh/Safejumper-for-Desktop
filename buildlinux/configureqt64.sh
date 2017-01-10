#!/bin/bash
export SKIP="qtlocation qtdeclarative qt3d qtwayland qtwebchannel qtquickcontrols qtquickcontrols2 qtmultimedia qtconnectivity qtwebengine"
for f in $SKIP; do
  rm -fR $f
done
./configure -platform linux-g++-64 -confirm-license -debug -developer-build -opensource -nomake examples -nomake tests -openssl -static -qt-libpng -v $1
