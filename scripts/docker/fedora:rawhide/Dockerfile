FROM fedora:rawhide

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
        postgresql-server-devel \
        grpc-devel              \
        grpc-plugins            \
        protobuf-devel          \
        protobuf-compiler       \
    && dnf clean all            \
    && rm -rf /var/cache/yum
