FROM fedora:28
RUN dnf -y clean all                  \
        && dnf -y update              \
        && dnf -y upgrade             \
        && dnf -y install             \
                  cmake               \
                  pkgconf             \
                  make                \
                  gcc-c++             \
                  git                 \
                  boost-devel         \
                  libxml2-devel       \
                  libcurl-devel       \
                  libidn-devel        \
                  mpdecimal-devel     \
                  libssh-devel        \
                  openssl-devel       \
                  minizip-devel       \
                  omniORB-devel

RUN rpm -Uvh https://yum.postgresql.org/9.6/fedora/fedora-28-x86_64/pgdg-fedora96-9.6-4.noarch.rpm \
        && dnf -y update \
        && dnf -y install postgresql96-devel
