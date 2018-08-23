Name:           %{project_name}
Version:        %{our_version}
Release:        %{?our_release}%{!?our_release:1}%{?dist}
Summary:        FRED - CORBA backend C++ server
Group:          Applications/Utils
License:        GPL
URL:            http://fred.nic.cz
Source0:        %{name}-%{version}.tar.gz
Source1:        idl-%{idl_branch}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-%(%{__id_u} -n)
BuildRequires:  git, omniORB-devel, boost-devel, postgresql-devel, gcc-c++, libxml2-devel, libcurl-devel, libidn-devel, mpdecimal-devel, libssh-devel, minizip-devel, openssl-devel
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
%setup -b 1

%build
%if 0%{?centos}
%{?scl:scl enable devtoolset-7 llvm-toolset-7 - << \EOF}
%global __cmake /opt/rh/llvm-toolset-7/root/usr/bin/cmake
%endif
%cmake -DCMAKE_INSTALL_PREFIX=/ -DDISTRO_PREFIX=usr/ -DDO_NOT_INSTALL_TESTS=1 -DVERSION=%{version} -DIDL_DIR=%{_topdir}/BUILD/idl-%{idl_branch}/idl .
%make_build
%if 0%{?centos}
%{?scl:EOF}
%endif

%install
rm -rf ${RPM_BUILD_ROOT}
%make_install

%clean
rm -rf ${RPM_BUILD_ROOT}

%files
%defattr(-,root,root,-)
%config %{_sysconfdir}/fred/*
%{_sysconfdir}/init.d/*
%{_sbindir}/*
%{_docdir}/fred-server/*
