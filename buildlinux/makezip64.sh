#!/bin/bash
# Before executing this script make sure you have the 64 bit debian package installed.
cat files.txt | zip shieldtra-linux-64.zip -@
zip -r shieldtra-linux-64.zip /opt/shieldtra
