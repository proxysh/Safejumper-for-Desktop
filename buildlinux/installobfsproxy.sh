#!/bin/bash
sudo -H pip install --no-binary :all: virtualenv
virtualenv --always-copy env
source env/bin/activate
pip install --no-binary :all: obfsproxy
pip install --no-binary :all: service_identity
virtualenv --relocatable env
