FROM centos/devtoolset-7-toolchain-centos7
USER root
RUN yum -y install https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm
RUN yum -y install https://download.postgresql.org/pub/repos/yum/9.6/redhat/rhel-7-x86_64/pgdg-redhat96-9.6-3.noarch.rpm
RUN yum -y clean all
RUN yum -y upgrade
RUN yum -y install       \
        cmake3           \
        pkgconf          \
        make             \
        gcc-c++          \
        git              \
        boost-devel      \
        postgresql96-devel \
        libxml2-devel    \
        libcurl-devel    \
        libidn-devel     \
        mpdecimal-devel  \
        libssh-devel     \
        openssl-devel    \
        minizip-devel    \
        omniORB-devel
USER 1001
