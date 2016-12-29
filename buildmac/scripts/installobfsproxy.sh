#!/bin/bash
sudo -H pip install virtualenv
virtualenv --always-copy env
source env/bin/activate
pip install obfsproxy
virtualenv --relocatable env
