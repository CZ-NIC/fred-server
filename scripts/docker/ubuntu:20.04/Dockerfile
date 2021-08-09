FROM ubuntu:20.04
ENV DEBIAN_FRONTEND noninteractive

RUN apt-get -y clean     \
    && apt-get -y update \
    && apt-get -y install curl gnupg2

RUN printf 'deb http://archive.nic.cz/ubuntu focal main\n' > /etc/apt/sources.list.d/cz.nic.list \
    && apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 1C0200016A9AC5C6 \
    && printf 'deb http://apt.postgresql.org/pub/repos/apt/ focal-pgdg main\n' > /etc/apt/sources.list.d/postgres.list \
    && curl https://www.postgresql.org/media/keys/ACCC4CF8.asc | apt-key add - \
    && apt-get -y update                 \
    && apt-get -y install                \
            cmake                        \
            postgresql-server-dev-9.6    \
            pkg-config                   \
            build-essential              \
            ninja-build                  \
            bash-completion              \
            git                          \
            libomniorb4-dev              \
            omniidl                      \
            omniidl-python               \
            omniorb                      \
            omniorb-idl                  \
            omniorb-nameserver           \
            libboost-date-time-dev       \
            libboost-program-options-dev \
            libboost-regex-dev           \
            libboost-filesystem-dev      \
            libboost-random-dev          \
            libboost-thread-dev          \
            libboost-system-dev          \
            libboost-serialization-dev   \
            libboost-test-dev            \
            libpq-dev                    \
            libcurl4-openssl-dev         \
            libmpdec-dev                 \
            libssh-dev                   \
            libminizip-dev               \
            libxml2-dev                  \
            libidn11-dev                 \
    && apt-get -y clean                  \
    && rm -rf /var/lib/apt/lists/*
