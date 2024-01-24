#!/bin/sh

set -e
set -x

lib_dir=$(dirname ${PWD})/local/lib/
moldyn_dir=$(dirname $(dirname $(dirname $(dirname "$(pwd)"))))/moldyn

echo "lib_dir=$lib_dir"
echo "moldyn_dir=$moldyn_dir"

export PKG_CONFIG_PATH=$(dirname ${PWD})/local/lib/pkgconfig
export LD_LIBRARY_PATH=$lib_dir

folder="$moldyn_dir/laboro/new_LC/5CB/28i2lenty_work"

infile_gpr="$folder/28i2lenty geomopt wbp.gpr"
infile_box="$folder/28i2lenty geomopt wbp.box"

dim=1

for infile_traj in $(ls $folder/*.traj); do

    rm -f $infile_traj.plots.dim$dim.fluid.txt
    rm -f $infile_traj.plots.dim$dim.lenta_number_0.txt
    rm -f $infile_traj.plots.dim$dim.lenta_number_1.txt

    ./example_make_mol_plots -d $dim -t 0 "$infile_gpr" "$infile_box" "$infile_traj" >> $infile_traj.plots.dim$dim.fluid.txt
    ./example_make_mol_plots -d $dim -t 1 -i 0 "$infile_gpr" "$infile_box" "$infile_traj" >> $infile_traj.plots.dim$dim.lenta_number_0.txt
    ./example_make_mol_plots -d $dim -t 1 -i 1 "$infile_gpr" "$infile_box" "$infile_traj" >> $infile_traj.plots.dim$dim.lenta_number_1.txt

done