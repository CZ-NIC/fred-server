Name:           fred-server
Version:        1.7
Release:        1%{?dist}
Summary:        Free Registry for Enum and Domain

Group:          Applications/Utils
License:        GPL
URL:            http://fred.nic.cz
Source:         %{name}-%{version}.tar.gz
Source:         http://fred.nic.cz/download/%{name}-%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-%(%{__id_u} -n)
BuildRequires:  libomniORB4.1-devel, fred-idl, boost-devel, postgresql-devel
Requires: libomniORB4.1, boost, postgresql-libs

%description
Server c++ backend for Fred registry system

%prep
%setup

%build
%configure --sysconfdir=/etc
make

%install
rm -rf ${RPM_BUILD_ROOT}
make DESTDIR=${RPM_BUILD_ROOT} install

%clean
rm -rf ${RPM_BUILD_ROOT}

%files
%defattr(-,root,root,-)
/etc/fred/server.conf
/etc/init.d/fred-server
/usr/sbin/fred-rifd
/usr/sbin/fred-adifd
/usr/sbin/fred-pifd
/usr/sbin/fred-admin
/usr/sbin/fred-banking
/usr/sbin/banking.sh

%changelog
* Mon Sep 07 2008 Jaromir Talir <jaromir.talir@nic.cz>
- initial spec file
