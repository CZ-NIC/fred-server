FROM fedora:latest
RUN dnf -y upgrade
RUN dnf -y install       \
        autoconf         \
        automake         \
        pkgconf          \
        libtool          \
        make             \
        gcc-c++          \
        git              \
        boost-devel      \
        postgresql-devel \
        libxml2-devel    \
        libcurl-devel    \
        libidn-devel     \
        mpdecimal-devel  \
        libssh-devel     \
        minizip-devel
RUN dnf -y --disablerepo=updates install \
        omniORB-devel
