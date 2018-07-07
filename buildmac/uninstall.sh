#!/bin/sh

echo "Safejumper Removal Tool"

username=`echo $USER`

killall Safejumper > /dev/null 2>&1

found=0
if [ -d "/Applications/Safejumper.app" ]; then
    found=1;
fi

# stop helper 
launchctl unload /Library/LaunchDaemons/sh.proxy.SafejumperHelper.plist > /dev/null 2>&1
rm -rf /Library/LaunchDaemons/sh.proxy.SafejumperHelper.plist > /dev/null 2>&1
rm -rf /Library/PrivilegedHelperTools/sh.proxy.SafejumperHelper > /dev/null 2>&1
rm -rf /Applications/Safejumper.app > /dev/null 2>&1

if [ "$found" = "1" ]; then
    echo "Successfully removed Safejumper"
fi
