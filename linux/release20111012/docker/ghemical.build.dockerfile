FROM ubuntu:16.04

ARG USER_NAME
ARG GROUP_NAME
ARG USER_ID
ARG GROUP_ID

ARG DEBIAN_FRONTEND=noninteractive

# Configure a 'docker' sudo user without password
RUN apt-get update && apt-get -y install sudo
RUN addgroup --gid ${GROUP_ID} ${GROUP_NAME}
RUN adduser --disabled-password --gecos '' --uid ${USER_ID} --gid ${GROUP_ID} ${USER_NAME}
RUN usermod -aG sudo ${USER_NAME}
RUN echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers

# Install build-essential package (for cross compilation)
RUN apt-get update
RUN apt-get -y install build-essential aptitude

# Install vim, nano
RUN apt-get -y install vim nano

# Install a bunch of packages for ghemical project

RUN apt -y install software-properties-common apt-utils
RUN add-apt-repository "deb http://archive.canonical.com/ $(lsb_release -sc) partner"
RUN apt-get update
RUN apt-cache search fonts | grep '^xfonts-'
RUN apt-get install -y xfonts-75dpi xfonts-100dpi

RUN apt-get -y install intltool mesa-common-dev libglu1-mesa-dev freeglut3-dev

RUN apt-get -y install wget gfortran pkg-config libglib2.0-dev libgtk2.0-dev libgtkglext1-dev libglade2-dev

RUN apt-get -y install git

USER ${USER_NAME}

# CMD ["/bin/bash"]










