#!/bin/sh

VERSION=$1

scripts/pkg-dmg \
    --verbosity 2 \
    --volname "shieldtra" \
    --source application/Shieldtra.app \
    --sourcefile \
    --format UDRW \
    --target shieldtra.dmg \
    --icon application.icns  \
    --mkdir .background \
    --symlink  /Applications:Applications \
    --copy uninstall.sh:uninstall.sh \
    --copy README.txt:README.txt

