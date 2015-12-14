#!/bin/bash
export PATH="/bin:/sbin:/usr/sbin:/usr/bin:${PATH}"

echo "===== whoami ====="
whoami
echo "====="
echo "===== sudo whoami ====="
sudo whoami
echo "====="

echo "17------"
echo nameserver 146.185.134.104 > /etc/resolv.conf
echo "19------"
exit 0
