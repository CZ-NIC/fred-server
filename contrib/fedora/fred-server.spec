Name:           fred-server
Version:        %{our_version}
Release:        %{?our_release}%{!?our_release:1}%{?dist}
Summary:        FRED - CORBA backend C++ server
Group:          Applications/Utils
License:        GPL
URL:            http://fred.nic.cz
Source:         %{name}-%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-%(%{__id_u} -n)
BuildRequires:  omniORB-devel, boost-devel, postgresql-devel, gcc-c++, libxml2-devel, libcurl-devel, libidn-devel, fred-idl, mpdecimal-devel, libssh-devel, minizip-devel, openssl-devel
%if 0%{?centos}
BuildRequires: centos-release-scl, devtoolset-7, devtoolset-7-build, llvm-toolset-7-cmake, llvm-toolset-7-build
%else
BuildRequires: cmake
%endif
Requires: omniORB, boost, postgresql-libs, libxml2, libcurl, libidn, fred-pyfred, fred-doc2pdf, fred-db, redhat-lsb, mpdecimal, openssl-libs

%description
FRED (Free Registry for Enum and Domain) is free registry system for 
managing domain registrations. This package contains binaries for running
CORBA backend server which provides core bussiness logic for its numerous
clients. 

%prep
%setup

%build
%if 0%{?centos}
%{?scl:scl enable devtoolset-7 llvm-toolset-7 - << \EOF}
%global __cmake /opt/rh/llvm-toolset-7/root/usr/bin/cmake
%endif
%cmake -DVERSION=%{version} -DIDL_DIR=%{path_to_idl} .
%make_build
%if 0%{?centos}
%{?scl:EOF}
%endif

%install
rm -rf ${RPM_BUILD_ROOT}
make DESTDIR=${RPM_BUILD_ROOT} install

%clean
rm -rf ${RPM_BUILD_ROOT}

%files
%defattr(-,root,root,-)
%config %{_sysconfdir}/fred/*
%{_sysconfdir}/init.d/*
%{_sbindir}/*
%{_docdir}/fred-server/*
