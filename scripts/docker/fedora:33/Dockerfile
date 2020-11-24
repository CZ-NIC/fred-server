FROM fedora:33
RUN dnf -y clean all                    \
        && dnf -y update                \
        && dnf -y install               \
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
                omniORB-devel
RUN rpm -Uvh https://download.postgresql.org/pub/repos/yum/reporpms/F-33-x86_64/pgdg-fedora-repo-latest.noarch.rpm \
        && dnf -y update \
        && dnf -y install postgresql96-devel
