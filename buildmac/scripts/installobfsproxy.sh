#!/bin/bash
sudo -H pip install virtualenv
virtualenv --always-copy env
source env/bin/activate
pip install obfsproxy
pip install service_identity
virtualenv --relocatable env
