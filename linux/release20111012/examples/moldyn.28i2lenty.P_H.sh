#!/bin/sh

set -e
set -x

lib_dir=$(dirname ${PWD})/local/lib/
moldyn_dir=$(dirname $(dirname $(dirname $(dirname "$(pwd)"))))/moldyn

echo "lib_dir=$lib_dir"
echo "moldyn_dir=$moldyn_dir"

export PKG_CONFIG_PATH=$(dirname ${PWD})/local/lib/pkgconfig
export LD_LIBRARY_PATH=$lib_dir

folder="$moldyn_dir/new_laboro/5CB/28i2lenty_wbp_yz"

infile_gpr="$folder/28i2lenty_wbp_yz.gpr"
infile_box="$folder/28i2lenty_wbp_yz.box"
outfile="$folder/28i2lenty_295_H"

./example_moldyn -P -H    -t 2 -T 295 -B 0 -N 2000000000 -f 5000 -v 17 "$infile_gpr" "$infile_box" "$outfile"_P.traj &
./example_moldyn -P -H -I -t 2 -T 295 -B 0 -N 2000000000 -f 5000 -v 17 "$infile_gpr" "$infile_box" "$outfile"_P_I.traj &

