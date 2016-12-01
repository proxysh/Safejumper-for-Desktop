#!/bin/bash
sudo pip install virtualenv
virtualenv --always-copy env
source env/bin/activate
pip install obfsproxy
