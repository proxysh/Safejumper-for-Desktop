#!/bin/bash
result=`dbus-send --system --dest=org.freedesktop.DBus --type=method_call --print-reply /org/freedesktop/DBus org.freedesktop.DBus.ListNames | grep resolve`

if [ "$result" = "" ]; then
   echo "0"
else
   echo "1"
fi
