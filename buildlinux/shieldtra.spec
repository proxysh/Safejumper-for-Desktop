#
# spec file for package shieldtra
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

Name:           shieldtra
Summary:        VPN client for Shieldtra network.
License:        GPL-2.0 and GPL-3.0
Group:          Productivity/Networking/Web/Utilities
Version:        2018.11.06
Release:        0
Url:            http://proxy.sh
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
Source:         shieldtra-%version.tar.gz
Conflicts:      shieldtra < %version-%release
Requires:	net-tools

# Do not check any files in env for requires
# %global __requires_exclude_from ^/opt/shieldtra/env/.*$
# %global __requires_exclude ^/opt/shieldtra/env/.*$

%description
VPN client for Shieldtra network.
Shieldtra is a lightweight OpenVPN client specifically designed for the Shieldtra VPN network.

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
mkdir -p $RPM_BUILD_ROOT/opt/shieldtra/
mkdir -p $RPM_BUILD_ROOT/usr/share/applications/
mkdir -p $RPM_BUILD_ROOT/usr/share/icons/hicolor/16x16/apps
mkdir -p $RPM_BUILD_ROOT/usr/share/icons/hicolor/32x32/apps
mkdir -p $RPM_BUILD_ROOT/usr/share/icons/hicolor/48x48/apps
mkdir -p $RPM_BUILD_ROOT/usr/share/icons/hicolor/64x64/apps
mkdir -p $RPM_BUILD_ROOT/usr/lib/systemd/system/
install -m 0755 shieldtra                 $RPM_BUILD_ROOT/opt/shieldtra/shieldtra
install -m 0755 shieldtraservice          $RPM_BUILD_ROOT/opt/shieldtra/shieldtraservice
install -m 0755 netdown                   $RPM_BUILD_ROOT/opt/shieldtra/netdown
install -m 0755 openvpn                   $RPM_BUILD_ROOT/opt/shieldtra/openvpn
install -m 0744 client.down.sh            $RPM_BUILD_ROOT/opt/shieldtra/client.down.sh
install -m 0744 client.up.sh              $RPM_BUILD_ROOT/opt/shieldtra/client.up.sh
install -m 0744 update-systemd-resolved   $RPM_BUILD_ROOT/opt/shieldtra/update-systemd-resolved
install -m 0744 detectresolve.sh          $RPM_BUILD_ROOT/opt/shieldtra/detectresolve.sh
install -m 0755 shieldtra.desktop         $RPM_BUILD_ROOT/usr/share/applications
install -m 0644 shieldtra.service         $RPM_BUILD_ROOT/usr/lib/systemd/system/shieldtra.service
install -m 0744 icons/16x16/apps/shieldtra.png     $RPM_BUILD_ROOT/usr/share/icons/hicolor/16x16/apps
install -m 0744 icons/32x32/apps/shieldtra.png     $RPM_BUILD_ROOT/usr/share/icons/hicolor/32x32/apps
install -m 0744 icons/48x48/apps/shieldtra.png   $RPM_BUILD_ROOT/usr/share/icons/hicolor/48x48/apps
install -m 0744 icons/64x64/apps/shieldtra.png     $RPM_BUILD_ROOT/usr/share/icons/hicolor/64x64/apps

%pre

%preun
systemctl disable shieldtra
systemctl stop shieldtra

%post
systemctl enable shieldtra
systemctl start shieldtra

%posttrans

%postun

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc {README,LICENSE}
%dir /opt/shieldtra/
/opt/shieldtra/shieldtra
/opt/shieldtra/shieldtraservice
/opt/shieldtra/netdown
/opt/shieldtra/openvpn
/opt/shieldtra/client.down.sh
/opt/shieldtra/client.up.sh
/opt/shieldtra/update-systemd-resolved
/opt/shieldtra/detectresolve.sh
/usr/share/applications/shieldtra.desktop
/usr/share/icons/hicolor/16x16/apps/shieldtra.png
/usr/share/icons/hicolor/32x32/apps/shieldtra.png
/usr/share/icons/hicolor/48x48/apps/shieldtra.png
/usr/share/icons/hicolor/64x64/apps/shieldtra.png
/usr/lib/systemd/system/shieldtra.service

%changelog
