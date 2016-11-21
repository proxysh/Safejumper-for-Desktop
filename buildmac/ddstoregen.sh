#!/usr/bin/osascript

on run argv

tell application "Finder"
    set diskname to item 1 of argv
    tell disk diskname
        open
        set current view of container window to icon view
        set toolbar visible of container window to false
        set statusbar visible of container window to false
        set bounds of container window to {400, 100, 1500, 1500}
        set theViewOptions to the icon view options of container window
        set arrangement of theViewOptions to not arranged
        set icon size of theViewOptions to 100
        set file_list to entire contents
        repeat with i in file_list
            if the name of i is "license.txt" then
                set the position of i to {415, 400}
            else if the name of i is "Applications" then
                set the position of i to {615, 225}
            else if the name of i ends with ".app" then
                set the position of i to {215, 225}
            end if

            -- Change the 7 to change the color: 0 is no label, then red,
            -- orange, yellow, green, blue, purple, or gray.
            set the label index of i to 7
        end repeat
        update without registering applications
        delay 4
    end tell
end tell
end run

