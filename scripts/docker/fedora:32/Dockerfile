FROM fedora:32

RUN dnf -y install              \
        cmake                   \
        pkgconf                 \
        make                    \
        gcc-c++                 \
        git                     \
        boost-devel             \
        libxml2-devel           \
        libcurl-devel           \
        libidn-devel            \
        mpdecimal-devel         \
        libssh-devel            \
        openssl-devel           \
        minizip-compat-devel    \
        omniORB-devel           \
    && dnf clean all            \
    && rm -rf /var/cache/yum

RUN rpm -Uvh https://download.postgresql.org/pub/repos/yum/reporpms/F-32-x86_64/pgdg-fedora-repo-latest.noarch.rpm \
        && dnf -y update \
        && dnf -y install postgresql96-devel
