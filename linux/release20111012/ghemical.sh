#!/bin/sh

set -e
set -x

lib_dir=${PWD}/local/lib/
moldyn_dir=$(dirname $(dirname $(dirname "$(pwd)")))/moldyn

echo "lib_dir=$lib_dir"
echo "moldyn_dir=$moldyn_dir"

export PKG_CONFIG_PATH=${PWD}/local/lib/pkgconfig
export LD_LIBRARY_PATH=$lib_dir

./local/bin/ghemical
