#!/bin/bash
# Before executing this script make sure you have the 64 bit debian package installed.
cat safecheckerfiles.txt | zip safechecker-linux-64.zip -@
zip -r safechecker-linux-64.zip /opt/safechecker
