#!/bin/sh

set -e
set -x

lib_dir=$(dirname ${PWD})/local/lib/
moldyn_dir=$(dirname $(dirname $(dirname $(dirname "$(pwd)"))))/moldyn.remote

echo "lib_dir=$lib_dir"
echo "moldyn_dir=$moldyn_dir"

export PKG_CONFIG_PATH=$(dirname ${PWD})/local/lib/pkgconfig
export LD_LIBRARY_PATH=$lib_dir

folder="$moldyn_dir/nanotube_12_membrane_4_tricyanomethyl"

infile_gpr="$folder/12.0_4cyanoforms_O2_.gpr"
infile_box="$folder/box4"

dim=2

for infile_traj in $(ls $folder/*.traj); do

    rm -f $infile_traj.plots.dim$dim.gas.txt
    rm -f $infile_traj.plots.dim$dim.membrane.txt

    ./example_make_mol_plots -d $dim -t 0 "$infile_gpr" "$infile_box" "$infile_traj" >> $infile_traj.plots.dim$dim.gas.txt
    ./example_make_mol_plots -d $dim -t 1 "$infile_gpr" "$infile_box" "$infile_traj" >> $infile_traj.plots.dim$dim.membrane.txt

done
