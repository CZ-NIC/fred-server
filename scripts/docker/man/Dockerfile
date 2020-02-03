FROM ubuntu:16.04
ENV DEBIAN_FRONTEND noninteractive

RUN apt-get -y clean     \
    && apt-get -y update \
    && apt-get -y install --no-install-recommends \
        make \
        git \
        python3-pip \
    && apt-get -y clean \
    && rm -rf /var/lib/apt/lists/* \
    && pip3 install Sphinx
