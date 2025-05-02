# Verwende Ubuntu 22.04 als Basis
FROM ubuntu:22.04

# Erstelle Benutzer 'yocto' mit UID und GID 1000
RUN groupadd -g 1000 yocto && useradd -m -u 1000 -g 1000 -s /bin/bash yocto && \
    echo 'yocto:yocto' | chpasswd && adduser yocto sudo

# Setze Arbeitsverzeichnis
WORKDIR /home/yocto

# Installiere notwendige Tools für Yocto inkl. zstd, file etc.
RUN apt update 

RUN apt-get install -y \
    sudo \
    git \
    wget \
    curl \
    python3 \
    python3-pip \
    g++ \
    make \
    gcc \
    chrpath \
    diffstat \
    lz4 \
    zstd \
    libzstd-dev \
    file \
    libxml2-dev \
    zlib1g-dev \
    unzip \
    build-essential \
    flex \
    bison \
    cpio \
    gawk \
    nano \
    locales \
    && rm -rf /var/lib/apt/lists/*

# Setze locale auf en_US.UTF-8
RUN locale-gen en_US.UTF-8 && update-locale LANG=en_US.UTF-8
ENV LANG=en_US.UTF-8
ENV LANGUAGE=en_US:en
ENV LC_ALL=en_US.UTF-8

# Wechsle zu Benutzer 'yocto'
USER yocto

# Arbeitsverzeichnis
WORKDIR /home/yocto

# Erstelle yocto-project Ordner und clone alle Repos
RUN mkdir -p ~/yocto-project && cd ~/yocto-project && \
    git clone git://git.yoctoproject.org/poky && \
    git clone https://github.com/STMicroelectronics/meta-st-stm32mp.git && \
    git clone git://git.openembedded.org/meta-openembedded

RUN cd yocto-project/poky/

RUN source oe-init-build-env ../build-stm32

RUN cd ../meta-st-stm32mp && git checkout kirkstone

RUN cd ../poky && git checkout -b kirkstone origin/kirkstone

RUN cd ../meta-openembedded && git checkout -b kirkstone origin/kirkstone

RUN bitbake-layers add-layer ../meta-openembedded/meta-oe

RUN bitbake-layers add-layer ../meta-openembedded/meta-python

RUN bitbake-layers add-layer ../meta-st-stm32mp

RUN bitbake-layers add-layer ../meta-openembedded/meta-networking


# Öffne bash
CMD ["bash"]
