#
# spec file for package safejumper
#
# Copyright (c) 2016 Jeremy Whiting <jeremypwhiting@gmail.com>
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

Name:           safejumper
Summary:        VPN client for proxy_sh.
License:        GPL-2.0 and GPL-3.0
Group:          Productivity/Networking/Web/Utilities
Version:        2018.07.06
Release:        0
Url:            http://proxy.sh
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
Source:         safejumper-%version.tar.gz
Conflicts:      safejumper < %version-%release
Requires:	net-tools
Requires:	redhat-rpm-config

# Do not check any files in env for requires
%global __requires_exclude_from ^/opt/safejumper/env/.*$
%global __requires_exclude ^/opt/safejumper/env/.*$

%description
VPN client for proxy.sh.
Safejumper is a lightweight OpenVPN client specifically designed for the Proxy.sh VPN network.

Authors:
--------
    Jeremy Whiting <info@3monkeysinternational.com>
#--------------------------------------------------------------------------------
%prep
%setup -q 

%build

%install
#
# First install all dist files
#
mkdir -p $RPM_BUILD_ROOT/opt/safejumper/
mkdir -p $RPM_BUILD_ROOT/usr/share/applications/
mkdir -p $RPM_BUILD_ROOT/usr/share/icons/hicolor/128x128/apps
mkdir -p $RPM_BUILD_ROOT/usr/share/icons/hicolor/16x16/apps
mkdir -p $RPM_BUILD_ROOT/usr/share/icons/hicolor/192x192/apps
mkdir -p $RPM_BUILD_ROOT/usr/share/icons/hicolor/22x22/apps
mkdir -p $RPM_BUILD_ROOT/usr/share/icons/hicolor/24x24/apps
mkdir -p $RPM_BUILD_ROOT/usr/share/icons/hicolor/256x256/apps
mkdir -p $RPM_BUILD_ROOT/usr/share/icons/hicolor/32x32/apps
mkdir -p $RPM_BUILD_ROOT/usr/share/icons/hicolor/64x64/apps
mkdir -p $RPM_BUILD_ROOT/usr/share/icons/hicolor/512x512/apps
mkdir -p $RPM_BUILD_ROOT/usr/share/icons/hicolor/72x72/apps
mkdir -p $RPM_BUILD_ROOT/usr/lib/systemd/system/
install -m 0755 safejumper                $RPM_BUILD_ROOT/opt/safejumper/safejumper
install -m 0755 safejumperservice         $RPM_BUILD_ROOT/opt/safejumper/safejumperservice
install -m 0755 launchopenvpn             $RPM_BUILD_ROOT/opt/safejumper/launchopenvpn
install -m 0755 netdown                   $RPM_BUILD_ROOT/opt/safejumper/netdown
install -m 0755 openvpn                   $RPM_BUILD_ROOT/opt/safejumper/openvpn
install -m 0744 client.down.safejumper.sh $RPM_BUILD_ROOT/opt/safejumper/client.down.safejumper.sh
install -m 0744 client.up.safejumper.sh   $RPM_BUILD_ROOT/opt/safejumper/client.up.safejumper.sh
install -m 0644 proxysh.crt               $RPM_BUILD_ROOT/opt/safejumper/proxysh.crt
install -d 0644 env                       $RPM_BUILD_ROOT/opt/safejumper/env
install -m 0755 safejumper.desktop        $RPM_BUILD_ROOT/usr/share/applications
install -m 0644 safejumper.service $RPM_BUILD_ROOT/usr/lib/systemd/system/safejumper.service
install -m 0744 icons/128x128/apps/safejumper.png   $RPM_BUILD_ROOT/usr/share/icons/hicolor/128x128/apps
install -m 0744 icons/16x16/apps/safejumper.png     $RPM_BUILD_ROOT/usr/share/icons/hicolor/16x16/apps
install -m 0744 icons/192x192/apps/safejumper.png   $RPM_BUILD_ROOT/usr/share/icons/hicolor/192x192/apps
install -m 0744 icons/22x22/apps/safejumper.png     $RPM_BUILD_ROOT/usr/share/icons/hicolor/22x22/apps
install -m 0744 icons/24x24/apps/safejumper.png     $RPM_BUILD_ROOT/usr/share/icons/hicolor/24x24/apps
install -m 0744 icons/256x256/apps/safejumper.png   $RPM_BUILD_ROOT/usr/share/icons/hicolor/256x256/apps
install -m 0744 icons/32x32/apps/safejumper.png     $RPM_BUILD_ROOT/usr/share/icons/hicolor/32x32/apps
install -m 0744 icons/512x512/apps/safejumper.png   $RPM_BUILD_ROOT/usr/share/icons/hicolor/512x512/apps
install -m 0744 icons/64x64/apps/safejumper.png     $RPM_BUILD_ROOT/usr/share/icons/hicolor/64x64/apps
install -m 0744 icons/72x72/apps/safejumper.png     $RPM_BUILD_ROOT/usr/share/icons/hicolor/72x72/apps
cp      -avr env/*                        $RPM_BUILD_ROOT/opt/safejumper/env/

%pre

%preun
systemctl disable safejumper
systemctl stop safejumper

%post
systemctl enable safejumper
systemctl start safejumper

%posttrans

%postun

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc {README,LICENSE}
%dir /opt/safejumper/
/opt/safejumper/safejumper
/opt/safejumper/safejumperservice
/opt/safejumper/launchopenvpn
/opt/safejumper/netdown
/opt/safejumper/openvpn
/opt/safejumper/client.down.safejumper.sh
/opt/safejumper/client.up.safejumper.sh
/opt/safejumper/proxysh.crt
/opt/safejumper/env
/usr/share/applications/safejumper.desktop
/usr/share/icons/hicolor/128x128/apps/safejumper.png
/usr/share/icons/hicolor/16x16/apps/safejumper.png
/usr/share/icons/hicolor/192x192/apps/safejumper.png
/usr/share/icons/hicolor/22x22/apps/safejumper.png
/usr/share/icons/hicolor/24x24/apps/safejumper.png
/usr/share/icons/hicolor/256x256/apps/safejumper.png
/usr/share/icons/hicolor/32x32/apps/safejumper.png
/usr/share/icons/hicolor/512x512/apps/safejumper.png
/usr/share/icons/hicolor/64x64/apps/safejumper.png
/usr/share/icons/hicolor/72x72/apps/safejumper.png
/usr/lib/systemd/system/safejumper.service

%changelog
