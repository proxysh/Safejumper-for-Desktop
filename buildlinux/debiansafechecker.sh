#!/bin/bash
VERSION=2017.06.08
QTVERSION=5.10.1
OLDPATH=$PATH
./cleanup.sh
export PATH=../../qt-$QTVERSION-32bit/qtbase/bin:$OLDPATH
./builddebiansafechecker32.sh $VERSION
./cleanup.sh
export PATH=../../qt-$QTVERSION-64bit/qtbase/bin:$OLDPATH
./builddebiansafechecker64.sh $VERSION
./cleanup.sh
export PATH=$OLDPATH
