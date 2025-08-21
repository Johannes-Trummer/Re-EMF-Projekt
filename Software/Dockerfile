FROM ubuntu:22.04

# Set environment variables to noninteractive to avoid tzdata prompts
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && apt-get upgrade -y && \
    apt-get install -y \
        sudo \
        git \
        wget \
        nano \
        curl \
        locales \
        build-essential \
        gawk \
        chrpath \
        diffstat \
        texinfo \
        python3 \
        python3-pip \
        python3-pexpect \
        xz-utils \
        debianutils \
        iputils-ping \
        vim \
        libncurses5-dev \
        libncursesw5-dev \
        unzip \
        screen \
        bison \
        flex \
        sudo \
        cpio \
        rsync \
        file \
        lz4 \
        zstd \
        libsdl1.2-dev \
        xterm \
        libssl-dev \
        repo \
        bc && \
    apt-get clean

# Repo-Tool (falls nicht im Paket)
RUN curl https://storage.googleapis.com/git-repo-downloads/repo > /usr/local/bin/repo && \
    chmod a+x /usr/local/bin/repo

RUN locale-gen en_US.UTF-8
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US:en
ENV LC_ALL en_US.UTF-8

RUN groupadd -g 1000 yocto && useradd -m -u 1000 -g 1000 -s /bin/bash yocto && \
    echo 'yocto:yocto' | chpasswd && adduser yocto sudo

USER yocto
WORKDIR /home/yocto

RUN git config --global user.name "lrncke" && \
    git config --global user.email "leon.arnecke@stud.htwk-leipzig.de"

# Shared output
RUN mkdir -p /home/yocto/shared_output
VOLUME /home/yocto/shared_output

RUN mkdir -p stm32mp1-yocto && cd stm32mp1-yocto
CMD [ "bash" ]
