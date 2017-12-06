#!/bin/bash
VERSION=2017.12.05
QTVERSION=5.7.0
OLDPATH=$PATH
./cleanup.sh
export PATH=../../qt-$QTVERSION-32bit/qtbase/bin:$OLDPATH
./buildredhat32.sh $VERSION
./cleanup.sh
export PATH=../../qt-$QTVERSION-64bit/qtbase/bin:$OLDPATH
./buildredhat64.sh $VERSION
./cleanup.sh
export PATH=$OLDPATH
