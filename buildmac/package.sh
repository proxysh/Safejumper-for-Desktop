#!/bin/sh

VERSION=$1

./pkg-dmg \
    --verbosity 2 \
    --volname "safejumper" \
    --source Safejumper.app \
    --sourcefile \
    --format UDRW \
    --target safejumper.dmg \
    --icon safejumper.icns  \
    --mkdir .background \
    --symlink  /Applications:Applications \
