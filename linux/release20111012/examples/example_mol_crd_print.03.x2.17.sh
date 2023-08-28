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
infile_traj="$moldyn_dir/conicnanotube.03.x2/test.traj_v17"

rm -f $infile_traj.plots.gas.txt
rm -f $infile_traj.plots.membrane_number_0.txt
rm -f $infile_traj.plots.membrane_number_1.txt

./example_make_mol_plots -t 0 "$infile_gpr" "$infile_box" "$infile_traj" >> $infile_traj.plots.gas.txt
./example_make_mol_plots -t 1 -i 0 "$infile_gpr" "$infile_box" "$infile_traj" >> $infile_traj.plots.membrane_number_0.txt
./example_make_mol_plots -t 1 -i 1 "$infile_gpr" "$infile_box" "$infile_traj" >> $infile_traj.plots.membrane_number_1.txt
