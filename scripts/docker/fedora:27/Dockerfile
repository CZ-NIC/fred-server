FROM fedora:27
RUN dnf -y clean all
RUN dnf -y upgrade
RUN dnf -y install       \
        autoconf         \
        autoconf-archive \
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
        minizip-devel    \
        omniORB-devel
