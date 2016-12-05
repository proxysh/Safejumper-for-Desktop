#!/bin/bash

sudo ifconfig en0 down
sudo networksetup -setairportpower en1 off
sudo networksetup -setairportpower Wi-Fi 

echo "===== whoami ====="
whoami
echo "====="
echo "===== sudo whoami ====="
sudo whoami
echo "====="