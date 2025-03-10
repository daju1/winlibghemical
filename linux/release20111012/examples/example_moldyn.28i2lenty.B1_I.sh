#!/bin/sh

set -e
set -x

lib_dir=$(dirname ${PWD})/local/lib/
moldyn_dir=$(dirname $(dirname $(dirname $(dirname "$(pwd)"))))/moldyn

echo "lib_dir=$lib_dir"
echo "moldyn_dir=$moldyn_dir"

export PKG_CONFIG_PATH=$(dirname ${PWD})/local/lib/pkgconfig
export LD_LIBRARY_PATH=$lib_dir

infile_gpr="$moldyn_dir/laboro/new_LC/5CB/28i2lenty_work/28i2lenty geomopt wbp.gpr"
infile_box="$moldyn_dir/laboro/new_LC/5CB/28i2lenty_work/28i2lenty geomopt wbp.box"
outfile_traj="$moldyn_dir/laboro/new_LC/5CB/28i2lenty_work/28i2lenty B1 I.traj"

./example_moldyn -t 2 -T 295 -I -B 1 -N 2000000000 -f 5000 -v 17 "$infile_gpr" "$infile_box" "$outfile_traj" &
