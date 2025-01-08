#!/bin/sh

set -e
set -x

lib_dir=$(dirname ${PWD})/local/lib/
moldyn_dir=$(dirname $(dirname $(dirname $(dirname "$(pwd)"))))/moldyn

echo "lib_dir=$lib_dir"
echo "moldyn_dir=$moldyn_dir"

export PKG_CONFIG_PATH=$(dirname ${PWD})/local/lib/pkgconfig
export LD_LIBRARY_PATH=$lib_dir

infile_gpr="$moldyn_dir/nanotube_12_membrane_4_tricyanomethyl/12.0_4cyanoforms_O2.gpr"
infile_box="$moldyn_dir/nanotube_12_membrane_4_tricyanomethyl/box4"

outfile_traj_b0="$moldyn_dir/nanotube_12_membrane_4_tricyanomethyl/12.0_4cyanoforms_O2.B0_L.traj"
outfile_traj_b0_i="$moldyn_dir/nanotube_12_membrane_4_tricyanomethyl/12.0_4cyanoforms_O2.B0_L_I.traj"

outfile_traj_b2="$moldyn_dir/nanotube_12_membrane_4_tricyanomethyl/12.0_4cyanoforms_O2.B2_L.traj"
outfile_traj_b2_i="$moldyn_dir/nanotube_12_membrane_4_tricyanomethyl/12.0_4cyanoforms_O2.B2_L_I.traj"

./example_moldyn -L    -B 0 -N 2000000000 -f 1000 -v 17 -t 2 "$infile_gpr" "$infile_box" "$outfile_traj_b0" &
./example_moldyn -L -I -B 0 -N 2000000000 -f 1000 -v 17 -t 2 "$infile_gpr" "$infile_box" "$outfile_traj_b0_i" &

./example_moldyn -L    -B 2 -N 2000000000 -f 1000 -v 17 -t 2 "$infile_gpr" "$infile_box" "$outfile_traj_b2" &
./example_moldyn -L -I -B 2 -N 2000000000 -f 1000 -v 17 -t 2 "$infile_gpr" "$infile_box" "$outfile_traj_b2_i" &

