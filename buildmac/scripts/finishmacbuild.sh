#!/bin/bash
hdiutil detach /Volumes/shieldtra/

sh scripts/package.sh 
hdiutil mount shieldtra.dmg
scripts/ddstoregen.sh shieldtra
hdiutil detach /Volumes/shieldtra/

mv shieldtra.dmg tmp.dmg
hdiutil convert -format UDRO -o shieldtra.dmg tmp.dmg
rm tmp.dmg
hdiutil convert shieldtra.dmg -format UDZO -imagekey -zlib-level=9 -o shieldtra-compressed.dmg
rm shieldtra.dmg
mv shieldtra-compressed.dmg shieldtra.dmg
