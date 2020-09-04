#!/bin/sh

set -e
set -x

DISPLAY=$(env | grep DISPLAY= | sed 's/DISPLAY=//')
export DISPLAY=$DISPLAY
PROJECT_ROOT=${PWD}
work_dir=$(dirname $(dirname "$(pwd)"))/work

DOCKER_IMAGE=ubuntu_ghemical_build:16.04

docker run -it --rm --name ghemical_build_container --cap-add=NET_ADMIN --device /dev/net/tun \
    -v ${PROJECT_ROOT}:${PROJECT_ROOT} \
    -v ${work_dir}:${work_dir} \
    -v /home/${USER}/.ssh/id_rsa:/home/${USER}/.ssh/id_rsa \
    -v /home/${USER}/.ssh/id_rsa.pub:/home/${USER}/.ssh/id_rsa.pub \
    -v /home/${USER}/.ssh/known_hosts:/home/${USER}/.ssh/known_hosts \
    -e DISPLAY=$DISPLAY --volume="/tmp/.X11-unix:/tmp/.X11-unix:rw" \
    --user ${USER}:${USER} ${DOCKER_IMAGE} /bin/bash


