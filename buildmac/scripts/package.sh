#!/bin/sh

VERSION=$1

scripts/pkg-dmg \
    --verbosity 2 \
    --volname "safejumper" \
    --source safejumper/Safejumper.app \
    --sourcefile \
    --format UDRW \
    --target safejumper.dmg \
    --icon safejumper.icns  \
    --mkdir .background \
    --symlink  /Applications:Applications \
