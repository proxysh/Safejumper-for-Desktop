#!/bin/bash
cd "$(dirname "$0")" # Change to Resources folder
sudo /usr/bin/easy_install pip
sudo /usr/local/bin/pip install virtualenv
/usr/local/bin/virtualenv env
source env/bin/activate
pip install obfsproxy
