Name:           %{project_name}
Version:        %{our_version}
Release:        %{?our_release}%{!?our_release:1}%{?dist}
Summary:        FRED - CORBA backend C++ server
Group:          Applications/Utils
License:        GPLv3+
URL:            http://fred.nic.cz
Source0:        %{name}-%{version}.tar.gz
Requires(pre):  /usr/sbin/useradd, /usr/bin/getent
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-%(%{__id_u} -n)
BuildRequires:  git, omniORB-devel, boost-devel, postgresql-devel >= 9.6, gcc-c++, libxml2-devel, libcurl-devel, libidn-devel, mpdecimal-devel, libssh-devel, openssl-devel, systemd

%if 0%{?el7}
BuildRequires: centos-release-scl, devtoolset-7, devtoolset-7-build, cmake3
%else
BuildRequires: cmake
%endif

%if 0%{?el8} || 0%{?fedora} >= 30
BuildRequires: libpq-devel, postgresql-server-devel, minizip-compat-devel
%else
BuildRequires: minizip-devel
%endif

Requires: omniORB, boost, libxml2, libcurl, libidn, fred-pyfred, fred-doc2pdf, fred-db, redhat-lsb, mpdecimal, openssl-libs, libxslt

%if 0%{?el8} || 0%{?fedora} >= 30
Requires: libpq
%else
Requires: postgresql-libs > 9.6
%endif

%description
FRED (Free Registry for Enum and Domain) is free registry system for 
managing domain registrations. This package contains binaries for running
CORBA backend server which provides core bussiness logic for its numerous
clients. 

%prep
%setup

%build
%if 0%{?el7}
%{?scl:scl enable devtoolset-7 - << \EOF}
%cmake3 -DCMAKE_INSTALL_PREFIX=/ -DUSE_USR_PREFIX=1 -DDO_NOT_INSTALL_TESTS=1 -DVERSION=%{version} .
%else
%cmake -DCMAKE_INSTALL_PREFIX=/ -DUSE_USR_PREFIX=1 -DDO_NOT_INSTALL_TESTS=1 -DVERSION=%{version} .
%endif
%if 0%{?el7}
%make_build
%else
%cmake_build
%endif
%if 0%{?el7}
%{?scl:EOF}
%endif

%install
rm -rf ${RPM_BUILD_ROOT}
%if 0%{?el7}
%make_install
%else
%cmake_install
%endif
mkdir -p $RPM_BUILD_ROOT/%{_sysconfdir}/fred/
grep ExecStart contrib/fedora/*.service | grep -h -o 'fred-[^/]*\.conf' | while read FILE; do ln -s %{_sysconfdir}/fred/server.conf $RPM_BUILD_ROOT/%{_sysconfdir}/fred/$FILE; done
mkdir -p $RPM_BUILD_ROOT/%{_unitdir}
install -m 644 contrib/fedora/*.service $RPM_BUILD_ROOT/%{_unitdir}

%pre
/usr/bin/getent passwd fred || /usr/sbin/useradd -r -d /etc/fred -s /bin/bash fred

%post
test -f /var/log/fred.log  || touch /var/log/fred.log
chown fred /var/log/fred.log

%clean
rm -rf ${RPM_BUILD_ROOT}

%files
%defattr(-,root,root,-)
%config %{_sysconfdir}/fred/*
%{_sysconfdir}/init.d/*
%{_sbindir}/*
%{_docdir}/fred-server/*
%{_unitdir}/*
