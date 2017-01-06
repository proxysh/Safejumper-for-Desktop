#!/bin/bash
./configure -confirm-license -debug -developer-build -opensource -nomake examples -nomake tests -skip qtconnectivity -skip qtwebengine -openssl -static -qt-libpng -v $1
