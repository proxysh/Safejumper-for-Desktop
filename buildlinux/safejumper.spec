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
Summary:        The Open Build Service -- Server Component
License:        GPL-2.0 and GPL-3.0
Group:          Productivity/Networking/Web/Utilities
Version:        2017.01.20
Release:        0
Url:            http://proxy.sh
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
Source:         safejumper-%version.tar.gz
Conflicts:      safejumper < %version-%release
Requires:	net-tools
Requires:	redhat-rpm-config # Remove once obfsproxy is bundled

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
# configure lighttpd web service (default until OBS 2.1)
mkdir -p $RPM_BUILD_ROOT/opt/safejumper/
install -m 0755 safejumper                $RPM_BUILD_ROOT/opt/safejumper/safejumper
install -m 0755 launchopenvpn             $RPM_BUILD_ROOT/opt/safejumper/launchopenvpn
install -m 0755 netdown                   $RPM_BUILD_ROOT/opt/safejumper/netdown
install -m 0755 openvpn                   $RPM_BUILD_ROOT/opt/safejumper/openvpn
install -m 0744 client.down.safejumper.sh $RPM_BUILD_ROOT/opt/safejumper/client.down.safejumper.sh
install -m 0744 client.up.safejumper.sh   $RPM_BUILD_ROOT/opt/safejumper/client.up.safejumper.sh
install -m 0644 proxysh.crt               $RPM_BUILD_ROOT/opt/safejumper/proxysh.crt

%pre

%preun

%post

%posttrans

%postun

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc {README,LICENSE}
%dir /opt/safejumper/
/opt/safejumper/safejumper
/opt/safejumper/launchopenvpn
/opt/safejumper/netdown
/opt/safejumper/openvpn
/opt/safejumper/client.down.safejumper.sh
/opt/safejumper/client.up.safejumper.sh
/opt/safejumper/proxysh.crt

%changelog
