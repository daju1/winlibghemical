#!/bin/sh

set -e
set -x

lib_dir=$(dirname ${PWD})/local/lib/
moldyn_dir=$(dirname $(dirname $(dirname $(dirname "$(pwd)"))))/moldyn

echo "lib_dir=$lib_dir"
echo "moldyn_dir=$moldyn_dir"

export PKG_CONFIG_PATH=$(dirname ${PWD})/local/lib/pkgconfig
export LD_LIBRARY_PATH=$lib_dir

infile_gpr="$moldyn_dir/conicnanotube.03.x2/conicnanotube.membrana Ne 14 x2.gpr"
infile_box="$moldyn_dir/conicnanotube.03.x2/box"
outfile_traj="$moldyn_dir/conicnanotube.03.x2/test.traj_v17"

make all
./example_moldyn -N 2000000000 -f 1000 -v 17 "$infile_gpr" "$infile_box" "$outfile_traj" &
