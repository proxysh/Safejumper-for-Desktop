#!/bin/bash
export PATH="/bin:/sbin:/usr/sbin:/usr/bin:${PATH}"


# allow OS X to process network settings
sleep 2

echo "Start of UP script"

echo "===== whoami ====="
whoami
echo "====="

#set +e # "grep" will return error status (1) if no matches are found, so don't fail on individual errors
PSID=$( (scutil | grep PrimaryService | sed -e 's/.*PrimaryService : //')<<- EOF
        open
        show State:/Network/Global/IPv4
        quit
EOF )
#set -e # resume abort on error

echo "===== PSID ====="
echo ${PSID}

Current_DNSs="$( (scutil | sed -e 's/^[[:space:]]*[[:digit:]]* : //g' | tr '\n' ' ')<<- EOF
        open
        show Setup:/Network/Service/${PSID}/DNS
        quit
EOF )"


echo "===== Current_DNSs ====="
echo ${Current_DNSs}
echo "====="


if echo "${Current_DNSs}" | grep -q "ServerAddresses" ; then
    Current_DNSs="$( echo $( echo "${Current_DNSs}" | sed -e 's/^.*ServerAddresses[^{]*{[[:space:]]*\([^}]*\)[[:space:]]*}.*$/\1/g' ) )"
fi

echo "===== Current_DNSs ====="
echo ${Current_DNSs}
echo "====="

unset DNServers

fo_index=1
dns_index=0

while f_option=foreign_option_$fo_index; [ -n "${!f_option}" ]; do
	if [[ "${!f_option}" == *'dhcp-option DNS'* ]]; then
		Pushed_DNS="$(echo "${!f_option//dhcp-option DNS /}")"
		echo ${Pushed_DNS}
		if ! (echo "${Current_DNSs}" | grep "${Pushed_DNS}" >/dev/null 2>&1); then
			DNServers[dns_index]="${Pushed_DNS}"
			let dns_index++ 
		fi
	fi
	let fo_index++
done

echo "===== APP_NAME ====="
echo $APP_NAME

echo "BBBBBBBBBBBBB"
echo $dns_index
echo "=====DNServers======"
echo ${DNServers[*]} 
echo "BBBBBBBBBBBBB"

if [ $dns_index -eq 0 ]; then
	exit 0
fi

echo "BBBBBBBBBBBBB"
echo $dns_index
echo "=====DNServers======"
echo ${DNServers[*]} 
echo "BBBBBBBBBBBBB"


echo "===== APP_NAME ====="
echo $APP_NAME

# down script uses OpenVPN
APP_NAME="OpenVPN"
echo "===== APP_NAME ====="
echo $APP_NAME

echo "===== Current_DNSs ====="
echo ${Current_DNSs}
echo "CCCCCCCCCC"
 
sudo scutil <<- EOF
	open
	d.init
	d.add Service ${PSID}
	set State:/Network/${APP_NAME}
EOF
echo "AAAAA"
sudo scutil <<- EOF
	d.init
    d.add ${APP_NAME}NoSuchKey true
    get Setup:/Network/Service/${PSID}/DNS
    set State:/Network/${APP_NAME}/OldDNSSetup
EOF
echo "AAAAA222"
sudo scutil <<- EOF
	d.init
	d.add ServerAddresses * ${DNServers[*]} ${Current_DNSs}
	set Setup:/Network/Service/${PSID}/DNS
	quit
EOF
echo "The end of UP script"
exit 0