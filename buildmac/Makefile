all : install release

install:
	macdeployqt Safejumper.app -verbose=1
	install safejumper.icns Safejumper.app/Contents/Resources/
	/usr/libexec/PlistBuddy -c "Set :CFBundleIconFile safejumper.icns" Safejumper.app/Contents/Info.plist
	sh package.sh 
	hdiutil mount safejumper.dmg
	./ddstoregen.sh safejumper
	hdiutil detach /Volumes/safejumper/

release:
	mv safejumper.dmg tmp.dmg
	hdiutil convert -format UDRO -o safejumper.dmg tmp.dmg
	rm tmp.dmg
	hdiutil convert safejumper.dmg -format UDZO -imagekey -zlib-level=9 -o safejumper-compressed.dmg
	rm safejumper.dmg
	mv safejumper-compressed.dmg safejumper.dmg

clean:
	rm -f safejumper.dmg
	rm -f tmp.dmg
