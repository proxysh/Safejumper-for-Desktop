#!/bin/bash
sudo -H pip install virtualenv
virtualenv --always-copy env
virtualenv --relocatable env
source env/bin/activate
pip install obfsproxy
