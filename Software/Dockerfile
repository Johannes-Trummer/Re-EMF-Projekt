FROM ubuntu:22.04
#docker build -t yocto-stm32mp1 .
#docker run -it -v C:\Users\Leon\Desktop\Test_ES3 --name yocto-dev yocto-stm32mp1

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
#cd /stm32mp1-yocto/ && repo init -u https://github.com/STMicroelectronics/oe-manifest.git -b kirkstone
#repo sync -j4
#sudo apt-get install bsdmainutils gcc-multilib libegl1-mesa libgmp-dev libmpc-dev pylint python3-git python3-jinja2 socat
#cd ~/stm32mp1-yocto && source layers/meta-st/scripts/envsetup.sh
#cd ~/stm32mp1-yocto && bitbake-layers create-layer meta-myapp
#cd ~/stm32mp1-yocto/build-openstlinuxweston-stm32mp13-disco && bitbake-layers add-layer ../meta-myapp
#mkdir -p ~/stm32mp1-yocto/meta-myapp/recipes-apps/hello && cd ~/stm32mp1-yocto/meta-myapp/recipes-apps/hello
#Erstellen der Programme und bb-Datein
#nano ~/stm32mp1-yocto/build-openstlinuxweston-stm32mp13-disco/conf/local.conf
#Einfügen von IMAGE_INSTALL:append = " hello"
#cd ~/stm32mp1-yocto/build-openstlinuxweston-stm32mp13-disco
#bitbake st-image-core #oder# bitbake st-image-weston
#cd ~/stm32mp1-yocto/build-openstlinuxweston-stm32mp13-disco/tmp-glibc/deploy/images/stm32mp13-disco/scripts
#./create_sdcard_from_flashlayout.sh ../flashlayout_st-image-weston/extensible/FlashLayout_sdcard_stm32mp135f-dk-extensible.tsv
#docker cp 39261739a2b5:/home/yocto/stm32mp1-yocto/build-openstlinuxweston-stm32mp13-disco/tmp-glibc/deploy/images/stm32mp13-disco/FlashLayout_sdcard_stm32mp135f-dk-extensible.raw C:\Users\Leon\Documents\Yocto_Project
#sudo wipefs -a /dev/sda
#sudo dd if=FlashLayout_sdcard_stm32mp135f-dk-extensible.raw of=/dev/sda bs=4M status=progress
CMD [ "bash" ]
