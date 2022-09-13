FROM fedora:35

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
        grpc-devel              \
        grpc-plugins            \
        protobuf-devel          \
        protobuf-compiler       \
        json-devel              \
    && dnf clean all            \
    && rm -rf /var/cache/yum

RUN rpm -Uvh https://download.postgresql.org/pub/repos/yum/reporpms/F-35-x86_64/pgdg-fedora-repo-latest.noarch.rpm \
        && dnf -y update \
        && dnf -y install postgresql13-devel
