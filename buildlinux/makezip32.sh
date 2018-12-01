#!/bin/bash
# Before executing this script make sure you have the 32 bit debian package installed.
cat files.txt | zip shieldtra-linux-32.zip -@
zip -r shieldtra-linux-32.zip /opt/shieldtra
