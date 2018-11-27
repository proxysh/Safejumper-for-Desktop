#!/bin/sh

echo "Shieldtra Removal Tool"

username=`echo $USER`

echo "Stopping any running Shieldtra applications"

killall Shieldtra

found=0
if [ -d "/Applications/Shieldtra.app" ]; then
    found=1;
fi

echo "Stopping Shieldtra helper service"
# stop helper 
launchctl unload /Library/LaunchDaemons/com.shieldtra.ShieldtraHelper.plist
rm -rf /Library/LaunchDaemons/com.shieldtra.ShieldtraHelper.plist

killall com.shieldtra.ShieldtraHelper
rm -rf /Library/PrivilegedHelperTools/com.shieldtra.ShieldtraHelper

echo "Deleting previous installation of Shieldtra application"

rm -rf /Applications/Shieldtra.app

if [ "$found" = "1" ]; then
    echo "Successfully removed Shieldtra"
fi
