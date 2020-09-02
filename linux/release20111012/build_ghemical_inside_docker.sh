#!/bin/sh

set -e
set -x

make download
make getcodes
make all
