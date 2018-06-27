#!/bin/bash
hdiutil detach /Volumes/safejumper/
hdiutil detach /Volumes/safechecker/

sudo cp launchopenvpn/launchopenvpn Resources/
sudo cp netdown/netdown Resources/
install safejumper.icns safejumper/Safejumper.app/Contents/Resources/
install safechecker.icns safechecker/Safechecker.app/Contents/Resources/
sudo cp -R Resources/* safejumper/Safejumper.app/Contents/Resources/
sudo cp -R Resources/* safechecker/Safechecker.app/Contents/Resources/
/usr/libexec/PlistBuddy -c "Set :CFBundleIconFile safejumper.icns" safejumper/Safejumper.app/Contents/Info.plist
/usr/libexec/PlistBuddy -c "Set :CFBundleIconFile safechecker.icns" safechecker/Safechecker.app/Contents/Info.plist
# macdeployqt Safejumper.app -verbose=1 
# macdeployqt safejumper/Safejumper.app -verbose=1 -codesign="Developer ID Application: Three Monkeys International Inc." $1
# codesign -f -o library -s "Developer ID Application: Three Monkeys International Inc." safejumper/Safejumper.app
# macdeployqt safechecker/Safechecker.app -verbose=1 -codesign="Developer ID Application: Three Monkeys International Inc." $1
codesign -f -o library -s "Developer ID Application: Three Monkeys International Inc." safechecker/Safechecker.app
sh scripts/package.sh 
hdiutil mount safejumper.dmg
hdiutil mount safechecker.dmg
scripts/ddstoregen.sh safejumper
scripts/ddstoregen.sh safechecker
hdiutil detach /Volumes/safejumper/
hdiutil detach /Volumes/safechecker/

mv safejumper.dmg tmp.dmg
mv safechecker.dmg tmp2.dmg
hdiutil convert -format UDRO -o safejumper.dmg tmp.dmg
hdiutil convert -format UDRO -o safechecker.dmg tmp2.dmg
rm tmp.dmg
rm tmp2.dmg
hdiutil convert safejumper.dmg -format UDZO -imagekey -zlib-level=9 -o safejumper-compressed.dmg
hdiutil convert safechecker.dmg -format UDZO -imagekey -zlib-level=9 -o safechecker-compressed.dmg
rm safejumper.dmg
rm safechecker.dmg
mv safejumper-compressed.dmg safejumper.dmg
mv safechecker-compressed.dmg safechecker.dmg
