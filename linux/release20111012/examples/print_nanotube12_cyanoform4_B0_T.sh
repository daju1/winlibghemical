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
infile_traj="$moldyn_dir/nanotube_12_membrane_4_tricyanomethyl/12.0_4cyanoforms_O2.B0_T.traj"

rm -f $infile_traj.plots.gas.txt
rm -f $infile_traj.plots.membrane.txt

./example_make_mol_plots -t 0 "$infile_gpr" "$infile_box" "$infile_traj" >> $infile_traj.plots.gas.txt
./example_make_mol_plots -t 1 "$infile_gpr" "$infile_box" "$infile_traj" >> $infile_traj.plots.membrane.txt
