#!/bin/sh

set -e
set -x

lib_dir=$(dirname ${PWD})/local/lib/
moldyn_dir=$(dirname $(dirname $(dirname $(dirname "$(pwd)"))))/moldyn

echo "lib_dir=$lib_dir"
echo "moldyn_dir=$moldyn_dir"

export PKG_CONFIG_PATH=$(dirname ${PWD})/local/lib/pkgconfig
export LD_LIBRARY_PATH=$lib_dir

infile_gpr="$moldyn_dir/conicnanotube.03/conicnanotube.membrana Ne 14.gpr"
infile_box="$moldyn_dir/conicnanotube.03/box"
outfile_traj="$moldyn_dir/conicnanotube.03/H_I_B0.traj"

make all
./example_moldyn -H -I -B 0 -N 2000000000 -f 1000 -v 17 "$infile_gpr" "$infile_box" "$outfile_traj" &
