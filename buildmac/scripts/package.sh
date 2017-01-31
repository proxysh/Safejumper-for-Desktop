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

scripts/pkg-dmg \
    --verbosity 2 \
    --volname "safechecker" \
    --source safechecker/Safechecker.app \
    --sourcefile \
    --format UDRW \
    --target safechecker.dmg \
    --icon safejumper.icns  \
    --mkdir .background \
    --symlink  /Applications:Applications \
