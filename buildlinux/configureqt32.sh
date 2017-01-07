#!/bin/bash
export CFLAGS="-m32"
export CPPFLAGS="-m32"
./configure -platform linux-g++-32 -confirm-license -debug -developer-build -opensource -nomake examples -nomake tests -skip qtconnectivity -skip qtwebengine -openssl -static -qt-libpng -v $1
