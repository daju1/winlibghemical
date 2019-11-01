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

download:
	wget https://www.bioinformatics.org/ghemical/download/release20111012/ghemical-3.0.0.tar.gz
	wget https://www.bioinformatics.org/ghemical/download/release20111012/libghemical-3.0.0.tar.gz
	wget https://www.bioinformatics.org/ghemical/download/release20111012/liboglappth-1.0.0.tar.gz
	wget https://www.bioinformatics.org/ghemical/download/release20111012/mopac7-1.15.tar.gz
	wget https://www.bioinformatics.org/ghemical/download/archive/openbabel-2.0.0.tar.gz

all: mopac7 liboglappth libghemical ghemical openbabel
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
	patch -p1 < ./ghemical.release20111012.apply_periodic_bond.patch
	patch -p1 < ./ghemical.release20111012.import_box.patch

untar_ghemical: ghemical-3.0.0.tar.gz
	tar -xzmf $?
	echo > $@

untar_libghemical: libghemical-3.0.0.tar.gz
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

ghemical_app:
	(cd ghemical-3.0.0 && ./configure --prefix=${install_dir} OPENBABEL_CFLAGS="-I${OPEN_BABEL_INCDIR}" OPENBABEL_LIBS="-L${openbabel_lib_path} -lopenbabel" LIBGHEMICAL_CFLAGS="-I${LIBGHEMICAL_INCDIR}" LIBGHEMICAL_LIBS="-L${libghemical_lib_path} -lghemical -loglappth" && make && make install)

ghemical_app_make:
	(cd ghemical-3.0.0 && make && make install)

ghemical_clean:
	(cd ghemical-3.0.0 && make clean)


libghemical: mopac7
	#(cd libghemical-3.0.0 && ./configure --prefix=${install_dir} --enable-mopac7 --enable-mpqc && make)
	(cd libghemical-3.0.0 && ./configure --prefix=${install_dir} --enable-mopac7 MOPAC7_CFLAGS="-I${MOPAC7_INCDIR}" MOPAC7_LIBS="-L${mopac7_lib_path} -lmopac7" && make && make install)

libghemical_make:
	(cd libghemical-3.0.0 && make && make install)

libghemical_clean:
	(cd libghemical-3.0.0 && make clean)
	
	
mopac7:
	(cd mopac7-1.15 && ./configure --prefix=${install_dir} && make && make install)

mopac7_clean:
	(cd mopac7-1.15 && make clean)

liboglappth:
	(cd liboglappth-1.0.0 && ./configure --prefix=${install_dir} && make && make install)

liboglappth_clean:
	(cd liboglappth-1.0.0 && make clean)

openbabel:
	(cd openbabel-2.0.0 && ./configure --prefix=${install_dir} && make CXXFLAGS='-fpermissive' && make install)

openbabel_clean:
	(cd openbabel-2.0.0 && make clean)
