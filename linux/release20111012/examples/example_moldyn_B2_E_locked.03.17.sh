#!/bin/sh

set -e
set -x

lib_dir=$(dirname ${PWD})/local/lib/
moldyn_dir=$(dirname $(dirname $(dirname $(dirname "$(pwd)"))))/moldyn

echo "lib_dir=$lib_dir"
echo "moldyn_dir=$moldyn_dir"

export PKG_CONFIG_PATH=$(dirname ${PWD})/local/lib/pkgconfig
export LD_LIBRARY_PATH=$lib_dir

infile_gpr="$moldyn_dir/conicnanotube.03/conicnanotube.membrana Ne 14 locked 8.gpr"
infile_box="$moldyn_dir/conicnanotube.03/box"
outfile_traj="$moldyn_dir/conicnanotube.03/first_frame_B2_E_locked.traj_v17"

./example_moldyn -E -t 2 -B 2 -N 2000000000 -f 1000 -v 17 "$infile_gpr" "$infile_box" "$outfile_traj" &
