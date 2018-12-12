FROM fedora:27
RUN dnf -y clean all
RUN dnf -y upgrade
RUN dnf -y install       \
        cmake            \
        pkgconf          \
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
        openssl-devel    \
        minizip-devel    \
        omniORB-devel
