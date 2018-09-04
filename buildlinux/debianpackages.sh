#!/bin/bash
VERSION=2018.09.04
QTVERSION=5.11.0
OLDPATH=$PATH
./cleanup.sh
export PATH=../../qt-$QTVERSION-32bit/qtbase/bin:$OLDPATH
./builddebian32.sh $VERSION
./cleanup.sh
export PATH=../../qt-$QTVERSION-64bit/qtbase/bin:$OLDPATH
./builddebian64.sh $VERSION
./cleanup.sh
export PATH=$OLDPATH
