#!/bin/sh

echo "Safejumper Removal Tool"

username=`echo $USER`

echo "Stopping any running Safejumper applications"

killall Safejumper

found=0
if [ -d "/Applications/Safejumper.app" ]; then
    found=1;
fi

echo "Stopping Safejumper helper service"
# stop helper 
launchctl unload /Library/LaunchDaemons/sh.proxy.SafejumperHelper.plist
rm -rf /Library/LaunchDaemons/sh.proxy.SafejumperHelper.plist

killall sh.proxy.SafejumperHelper
rm -rf /Library/PrivilegedHelperTools/sh.proxy.SafejumperHelper

echo "Deleting previous installation of Safejumper application"

rm -rf /Applications/Safejumper.app

if [ "$found" = "1" ]; then
    echo "Successfully removed Safejumper"
fi
