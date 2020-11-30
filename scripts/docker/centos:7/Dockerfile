FROM centos/devtoolset-7-toolchain-centos7
USER root
RUN yum -y install https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm
RUN yum -y install https://download.postgresql.org/pub/repos/yum/reporpms/EL-7-x86_64/pgdg-redhat-repo-latest.noarch.rpm
RUN yum -y clean all        \
    && yum -y upgrade       \
    && yum -y install       \
        cmake3              \
        pkgconf             \
        make                \
        gcc-c++             \
        git                 \
        boost-devel         \
        postgresql96-devel  \
        libxml2-devel       \
        libcurl-devel       \
        libidn-devel        \
        mpdecimal-devel     \
        libssh-devel        \
        openssl-devel       \
        minizip-devel       \
        omniORB-devel       \
    && rm -rf /var/cache/yum

USER 1001
