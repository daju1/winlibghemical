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

T=295

infile_gpr="$folder/28i2lenty_wbp_yz.gpr"
infile_box="$folder/28i2lenty_wbp_yz.box"
outfile="$folder/$T/28i2lenty_""$T""_H"

mkdir -p "$folder/$T"

./example_trimtraj -N 64 -v 17 "$infile_gpr" "$infile_box" "$outfile"_P.traj &
./example_trimtraj -N 64 -v 17 "$infile_gpr" "$infile_box" "$outfile"_P_I.traj &

