# steps for environment setting

# sudo apt-get install intltool
# configure: error: Package requirements (gtk+-2.0 >= 2.6.0 gtkglext-1.0 >= 1.0.5 libglade-2.0 >= 2.4.0) were not met:
# sudo apt-get install libgtkglextmm-x11-1.2-dev
# sudo apt-get install libglade2-dev
# base_app.h:32:19: fatal error: GL/gl.h: No such file or directory
# sudo apt install mesa-common-dev
# base_app.h:33:20: fatal error: GL/glu.h: No such file or directory
# sudo apt install libglu1-mesa-dev freeglut3-dev

#(ghemical:12505): GdkGLExt-WARNING **: cannot load PangoFont
# *** ERROR : Can't load font 'courier 12'

# sudo add-apt-repository "deb http://archive.canonical.com/ $(lsb_release -sc) partner"
# sudo apt-get update
# apt-cache search fonts | grep '^xfonts-'
# sudo apt-get install -y xfonts-75dpi xfonts-100dpi

# ~/test/moreutils$ ./isutf8 /ghemical/release20111012/libghemical-3.0.0/src/model.cpp
# /ghemical/release20111012/libghemical-3.0.0/src/model.cpp: line 4441, char 1, byte offset 82: invalid UTF-8 code

# (ghemical:28809): Gtk-CRITICAL **: IA__gtk_text_buffer_get_iter_at_mark: assertion '!gtk_text_mark_get_deleted (mark)' failed


# steps for building

# make download
# make getcodes
# make all

current_dir = $(shell pwd)
install_dir=${current_dir}/local

OPEN_BABEL_INCDIR=$(install_dir)/include/openbabel-2.0
openbabel_lib_path=$(install_dir)/lib

LIBGHEMICAL_INCDIR=$(install_dir)/include
libghemical_lib_path=$(install_dir)/lib

MOPAC7_INCDIR=$(install_dir)/include
mopac7_lib_path=$(install_dir)/lib

download_archive:
	wget https://www.bioinformatics.org/ghemical/download/archive/gt2-w32-100500.zip
	wget https://www.bioinformatics.org/ghemical/download/archive/ghemical-1.90.tgz
	wget https://www.bioinformatics.org/ghemical/download/archive/ghemical-2.00.tar.gz
	wget https://www.bioinformatics.org/ghemical/download/archive/libghemical-1.90.tgz

#  [DIR], release20070923/, 31-Jul-2008 06:49 ;
download_295:
	wget https://www.bioinformatics.org/ghemical/download/release20070923/ghemical-2.95.tar.gz
	wget https://www.bioinformatics.org/ghemical/download/release20070923/libghemical-2.95.tar.gz
	wget https://www.bioinformatics.org/ghemical/download/release20070923/liboglappth-0.96.tar.gz
	wget https://www.bioinformatics.org/ghemical/download/release20070923/ghemical-2.96a-w32-setup.exe

#  [DIR], release20080731/, 31-Jul-2008 06:49 ;
download_298:
	wget https://www.bioinformatics.org/ghemical/download/release20080731/ghemical-2.98.tar.gz
	wget https://www.bioinformatics.org/ghemical/download/release20080731/libghemical-2.98.tar.gz
	wget https://www.bioinformatics.org/ghemical/download/release20080731/liboglappth-0.98.tar.gz

# [DIR], release20090904/, 04-Sep-2009 08:51 ;
download_299:
	wget https://www.bioinformatics.org/ghemical/download/release20090904/ghemical-2.99.tar.gz
	wget https://www.bioinformatics.org/ghemical/download/release20090904/libghemical-2.99.tar.gz
	wget https://www.bioinformatics.org/ghemical/download/release20090904/liboglappth-0.98.tar.gz

# [DIR], release20090911/, 04-Nov-2009 06:31 ;
download_2991:
	wget https://www.bioinformatics.org/ghemical/download/release20090911/ghemical-2.99.1.tar.gz
	wget https://www.bioinformatics.org/ghemical/download/release20090911/ghemical-2.99.2.tar.gz
	wget https://www.bioinformatics.org/ghemical/download/release20090911/libghemical-2.99.1.tar.gz

# [DIR], release20111012/, 12-Oct-2011 15:36 ...
download_300:
	wget https://www.bioinformatics.org/ghemical/download/release20111012/ghemical-3.0.0.tar.gz
	wget https://www.bioinformatics.org/ghemical/download/release20111012/libghemical-3.0.0.tar.gz
	wget https://www.bioinformatics.org/ghemical/download/release20111012/liboglappth-1.0.0.tar.gz
	wget https://www.bioinformatics.org/ghemical/download/release20111012/mopac7-1.15.tar.gz
	wget https://www.bioinformatics.org/ghemical/download/archive/openbabel-2.0.0.tar.gz

download: download_300

all: mopac7 liboglappth libghemical ghemical openbabel ghemical_examples
all_configure: mopac7_configure liboglappth_configure libghemical_configure ghemical_configure openbabel_configure
clean: mopac7_clean liboglappth_clean libghemical_clean ghemical_clean openbabel_clean
remove:
	rm -rf ghemical-3.0.0
	rm -rf libghemical-3.0.0
	rm -rf liboglappth-1.0.0
	rm -rf mopac7-1.15
	rm -rf openbabel-2.0.0
	rm -f untar_ghemical
	rm -f untar_libghemical
	rm -f untar_liboglappth
	rm -f untar_mopac7
	rm -f untar_openbabel

untar: untar_ghemical untar_libghemical untar_liboglappth untar_mopac7 untar_openbabel

getcodes: untar
	patch -p1 < ./ghemical.release20111012.fix_build.patch
	patch -p1 < ./ghemical.release20111012.fix_fonts_crash.patch
	patch -p1 < ./ghemical.release20111012.load_periodic_box.patch
	patch -p1 < ./ghemical.release20111012.import_box.patch
	patch -p1 < ./ghemical.release20111012.traj_plots.patch
	patch -p1 < ./ghemical.release20111012.calc_real_traj_frames.patch
	patch -p1 < ./ghemical.release20111012.frame_save_frq_increasing.patch
	patch -p1 < ./ghemical.release20111012.HAVE_FIX_OF_GTK_IS_TEXT_MARK_FAIL.patch
	patch -p1 < ./ghemical.release20111012.reopen_traj_file_on_every_frame_save.patch
	patch -p1 < ./ghemical.release20111012.fix_openbabel_atoi_declaration.patch
	patch -p1 < ./ghemical.release20111012.boundary_opt.patch
	patch -p1 < ./ghemical.release20111012.apply_periodic_cond.patch
	patch -p1 < ./ghemical.release20111012.check_atom_locations.patch
	patch -p1 < ./ghemical.release20111012.boundary_opt_logging.patch
	patch -p1 < ./ghemical.release20111012.make_const_inpuls_while_set_ekin.patch
	patch -p1 < ./ghemical.release20111012.maxwell_distribution_init.patch
	patch -p1 < ./ghemical.release20111012.force_molecules_momentum_to_zero.patch
	patch -p1 < ./ghemical.release20111012.force_molecules_momentum_to_zero_locked.patch
	patch -p1 < ./ghemical.release20111012.engine_pbc_hex.patch
	patch -p1 < ./ghemical.release20111012.traj_v.patch
	patch -p1 < ./ghemical.release20111012.molgroups.patch
	patch -p1 < ./ghemical.release20111012.traj_v2.patch
	patch -p1 < ./ghemical.release20111012.make_plot_crd_diff.patch
	patch -p1 < ./ghemical.release20111012.save_plot_1d_data.patch
	patch -p1 < ./ghemical.release20111012.eng1_mm_default_wbp.patch
	patch -p1 < ./ghemical.release20111012.eng1_mm_default_wbp_2.patch
	patch -p1 < ./ghemical.release20111012.make_plot_crd_diff_2.patch
	patch -p1 < ./ghemical.release20111012.eng1_mm_default_wbp_3.patch
	patch -p1 < ./ghemical.release20111012.log_box_optimization_set_precision.patch
	patch -p1 < ./ghemical.release20111012.load_selected.patch
	patch -p1 < ./ghemical.release20111012.render_xyz_axis.patch
	patch -p1 < ./ghemical.release20111012.molgroups_coordinate_cumsum_velocity_and_acceleration.patch
	patch -p1 < ./ghemical.release20111012.molgroups_read_trajectory_frame_atom.patch
	patch -p1 < ./ghemical.release20111012.molgroups_make_mol_plot_in_model.patch
	# patch -p1 < ./ghemical.release20111012.cumsum_work.patch
	# patch -p1 < ./ghemical.release20111012.cumsum_work_power.patch
	patch -p1 < ./ghemical.release20111012.cumulative_mean_force.new.patch
	patch -p1 < ./ghemical.release20111012.eng1_mm_default_wbp_4.patch
	patch -p1 < ./ghemical.release20111012.nosehoover.patch
	patch -p1 < ./ghemical.release20111012.nosehoover_app.patch
	patch -p1 < ./ghemical.release20111012.break_molplot_when_nan.patch
	patch -p1 < ./ghemical.release20111012.is_trajectory_file_already_exists.patch
	patch -p1 < ./ghemical.release20111012.nosehoover_file_version2.patch
	patch -p1 < ./ghemical.release20111012.pressure-coupling.patch
	patch -p1 < ./ghemical.release20111012.trimtraj.patch
	patch -p1 < ./ghemical.release20111012.time_correlation_function_lib.patch
	patch -p1 < ./ghemical.release20111012.time_correlation_function_proj.patch
	patch -p1 < ./ghemical.release20111012.moldyn_langevin.patch
	patch -p1 < ./ghemical.release20111012.moldyn_nosehoover.patch
	patch -p1 < ./ghemical.release20111012.make_all_conjugate_carbon_bonds_lib.patch
	patch -p1 < ./ghemical.release20111012.make_all_conjugate_carbon_bonds_proj.patch
	patch -p1 < ./ghemical.release20111012.langevin_random_force_energy_distribution.patch

cash:
	mv ghemical-3.0.0    ghemical-3.0.0.cashed
	mv libghemical-3.0.0 libghemical-3.0.0.cashed
	mv liboglappth-1.0.0 liboglappth-1.0.0.cashed
	mv mopac7-1.15       mopac7-1.15.cashed
	mv openbabel-2.0.0   openbabel-2.0.0.cashed

cash2:
	mv ghemical-3.0.0    ghemical-3.0.0.cashed2
	mv libghemical-3.0.0 libghemical-3.0.0.cashed2
	mv liboglappth-1.0.0 liboglappth-1.0.0.cashed2
	mv mopac7-1.15       mopac7-1.15.cashed2
	mv openbabel-2.0.0   openbabel-2.0.0.cashed2

originals:
	cp -r ghemical-3.0.0    ghemical-3.0.0.original
	cp -r libghemical-3.0.0 libghemical-3.0.0.original
	cp -r liboglappth-1.0.0 liboglappth-1.0.0.original
	cp -r mopac7-1.15       mopac7-1.15.original
	cp -r openbabel-2.0.0   openbabel-2.0.0.original

prepare_originals_for_patch:
	make clean

	mv ghemical-3.0.0    ghemical-3.0.0.edited
	mv libghemical-3.0.0 libghemical-3.0.0.edited
	mv liboglappth-1.0.0 liboglappth-1.0.0.edited
	mv mopac7-1.15       mopac7-1.15.edited
	mv openbabel-2.0.0   openbabel-2.0.0.edited

	rm -f untar_ghemical
	rm -f untar_libghemical
	rm -f untar_liboglappth
	rm -f untar_mopac7
	rm -f untar_openbabel

	make getcodes
	make all_configure

	mv ghemical-3.0.0    ghemical-3.0.0.original
	mv libghemical-3.0.0 libghemical-3.0.0.original
	mv liboglappth-1.0.0 liboglappth-1.0.0.original
	mv mopac7-1.15       mopac7-1.15.original
	mv openbabel-2.0.0   openbabel-2.0.0.original

	mv ghemical-3.0.0.edited    ghemical-3.0.0
	mv libghemical-3.0.0.edited libghemical-3.0.0
	mv liboglappth-1.0.0.edited liboglappth-1.0.0
	mv mopac7-1.15.edited       mopac7-1.15
	mv openbabel-2.0.0.edited   openbabel-2.0.0

define clear_before_patch
	rm -r -f $(1)/*.cache

	rm -r -f $(1)/po/*
	rm -r -f $(1).original/po/*

	rm -r -f $(1)*/.deps
	rm -r -f $(1).original*/.deps

	rm -r -f $(1)/fortran/.deps
	rm -r -f $(1).original/fortran/.deps

	rm -r -f $(1)/src/.deps
	rm -r -f $(1).original/src/.deps

	rm -r -f $(1)/src/fingerprints/.deps
	rm -r -f $(1).original/src/fingerprints/.deps

	rm -r -f $(1)/src/formats/.deps
	rm -r -f $(1).original/src/formats/.deps

	rm -r -f $(1)/src/formats/xml/.deps
	rm -r -f $(1).original/src/formats/xml/.deps

	rm -r -f $(1)/src/formats/inchi/.deps
	rm -r -f $(1).original/src/formats/inchi/.deps

	rm -r -f $(1)/src/math/.deps
	rm -r -f $(1).original/src/math/.deps

	rm -r -f $(1)/test/.deps
	rm -r -f $(1).original/test/.deps

	rm -r -f $(1)/tools/.deps
	rm -r -f $(1).original/tools/.deps

	rm -f    $(1)/config.log
	rm -f    $(1).original/config.log

	rm -f    $(1)/config.status
	cp       $(1).original/config.status $(1)/config.status

	rm -f    $(1)/configure
	cp       $(1).original/configure $(1)/configure
endef

.PHONY: clean_before_patch

clean_before_patch:
	$(call clear_before_patch, ghemical-3.0.0)
	$(call clear_before_patch, libghemical-3.0.0)
	$(call clear_before_patch, liboglappth-1.0.0)
	$(call clear_before_patch, openbabel-2.0.0)
	$(call clear_before_patch, mopac7-1.15)

do_patch_295:
	diff -Nur ./libghemical-2.95 ./libghemical-3.0.0 > file_295.patch


do_patch1:
	diff -Nur ./ghemical-3.0.0.original    ./ghemical-3.0.0 > file1.patch

do_patch2:
	diff -Nur ./libghemical-3.0.0.original ./libghemical-3.0.0 > file.patch
	diff -Nur ./liboglappth-1.0.0.original ./liboglappth-1.0.0
	diff -Nur ./mopac7-1.15.original       ./mopac7-1.15
	diff -Nur ./openbabel-2.0.0.original   ./openbabel-2.0.0

remove_originals:
	rm -f -r ghemical-3.0.0.original
	rm -f -r libghemical-3.0.0.original
	rm -f -r liboglappth-1.0.0.original
	rm -f -r mopac7-1.15.original
	rm -f -r openbabel-2.0.0.original

remove_cashed:
	rm -f -r ghemical-3.0.0.cashed
	rm -f -r libghemical-3.0.0.cashed
	rm -f -r liboglappth-1.0.0.cashed
	rm -f -r mopac7-1.15.cashed
	rm -f -r openbabel-2.0.0.cashed

remove_cashed2:
	rm -f -r ghemical-3.0.0.cashed2
	rm -f -r libghemical-3.0.0.cashed2
	rm -f -r liboglappth-1.0.0.cashed2
	rm -f -r mopac7-1.15.cashed2
	rm -f -r openbabel-2.0.0.cashed2

untar_ghemical: ghemical-3.0.0.tar.gz
	tar -xzmf $?
	echo > $@

untar_ghemical_295: ghemical-2.95.tar.gz
	tar -xzmf $?
	echo > $@

untar_libghemical: libghemical-3.0.0.tar.gz
	tar -xzmf $?
	echo > $@

untar_libghemical_295: libghemical-2.95.tar.gz
	tar -xzmf $?
	echo > $@

untar_liboglappth: liboglappth-1.0.0.tar.gz
	tar -xzmf $?
	echo > $@

untar_mopac7: mopac7-1.15.tar.gz
	tar -xzmf $?
	echo > $@

untar_openbabel: openbabel-2.0.0.tar.gz
	tar -xzmf $?
	echo > $@

ghemical: libghemical liboglappth openbabel ghemical_app

ghemical_configure:
	(cd ghemical-3.0.0 && ./configure --prefix=${install_dir} OPENBABEL_CFLAGS="-I${OPEN_BABEL_INCDIR}" OPENBABEL_LIBS="-L${openbabel_lib_path} -lopenbabel" LIBGHEMICAL_CFLAGS="-I${LIBGHEMICAL_INCDIR}" LIBGHEMICAL_LIBS="-L${libghemical_lib_path} -lghemical -loglappth")

ghemical_app:
	make ghemical_configure && (cd ghemical-3.0.0 && make && make install)

ghemical_app_make:
	(cd ghemical-3.0.0 && make && make install)

ghemical_clean:
	(cd ghemical-3.0.0 && make clean)

libghemical_configure:
	#(cd libghemical-3.0.0 && ./configure --prefix=${install_dir} --enable-mopac7 --enable-mpqc)
	(cd libghemical-3.0.0 && ./configure --prefix=${install_dir} --enable-mopac7 MOPAC7_CFLAGS="-I${MOPAC7_INCDIR}" MOPAC7_LIBS="-L${mopac7_lib_path} -lmopac7")

ghemical_examples: # mopac7 liboglappth libghemical openbabel
	(cd examples && make clean && make all)

libghemical: mopac7
	make libghemical_configure && (cd libghemical-3.0.0 && make && make install)

libghemical_make:
	(cd libghemical-3.0.0 && make && make install)

libghemical_clean:
	(cd libghemical-3.0.0 && make clean)


mopac7_configure:
	(cd mopac7-1.15 && ./configure --prefix=${install_dir})

mopac7:
	make mopac7_configure && (cd mopac7-1.15 && make && make install)

mopac7_clean:
	(cd mopac7-1.15 && make clean)

liboglappth_configure:
	(cd liboglappth-1.0.0 && ./configure --prefix=${install_dir})

liboglappth:
	make liboglappth_configure && (cd liboglappth-1.0.0 && make && make install)

liboglappth_clean:
	(cd liboglappth-1.0.0 && make clean)

openbabel_configure:
	(cd openbabel-2.0.0 && ./configure --prefix=${install_dir})

openbabel:
	make openbabel_configure && (cd openbabel-2.0.0 && make CXXFLAGS='-fpermissive' && make install)

openbabel_clean:
	(cd openbabel-2.0.0 && make clean)
