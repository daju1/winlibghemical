# Configuration for libghemical.
# Copyright (c) 2000- Geoff Hutchison, Michael Banck, Tommi Hassinen.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.

# Process this file with aclocal/autoconf to produce a configure script.

AC_INIT(libghemical, 1.90)
AC_DEFINE([LIBVERSION], ["1.90"], [This is the version of libghemical to be built--it is set in the configure.ac])
AC_SUBST(LIBVERSION, "1.90")
AC_DEFINE([LIBRELEASEDATE], ["2005-06-30"], [This is the releasedate of libghemical--it is set in the configure.ac])
AC_SUBST(LIBRELEASEDATE, "2005-06-30")

AC_PREREQ(2.48)
AC_CONFIG_SRCDIR(src/model.cpp)
AM_CONFIG_HEADER(src/libconfig.h)

AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)

AC_CANONICAL_HOST

top_builddir_=`pwd`
AC_SUBST(top_builddir_)

AC_DEFINE_DIR([LIBDATA_PATH], [datadir/libghemical], [Where the data files are ; set in the configure.ac])

# Get any build/host and enable/disable flags now.
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# if a mingw32 host is specified, then build code that links against native (MS) runtime libs.
case $host in
	*-*-mingw32*)
		CPPFLAGS="-Dmno-cygwin -Dmms_bitfields $CPPFLAGS"
		LIBS="$LIBS -mwindows"
		;;
esac

AC_ARG_ENABLE(mpqc,[  --enable-mpqc           compile with MPQC support],WITH_MPQC="$enableval",WITH_MPQC="no")
AC_ARG_ENABLE(mopac7,[  --enable-mopac7         compile with MOPAC7 support],WITH_MOPAC7="$enableval",WITH_MOPAC7="no")
AC_ARG_ENABLE(openbabel,[  --enable-openbabel      compile with OpenBabel support],WITH_OPENBABEL="$enableval",WITH_OPENBABEL="no")

# Checks for programs.
# ^^^^^^^^^^^^^^^^^^^^

AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_CXXCPP
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_PATH_PROG(AR, ar, ar)
AC_PATH_PROG(SHELL, sh, /bin/sh)
AC_PROG_MAKE_SET

# Checks for libraries.
# ^^^^^^^^^^^^^^^^^^^^^

AC_CHECK_LIB([m], [acos])

if test $WITH_MPQC = yes; then
	AC_CHECK_LIB([blas], [main], , AC_MSG_ERROR([Cannot find blas library needed for MPQC support]))
	AC_CHECK_LIB([lapack], [main], , AC_MSG_ERROR([Cannot find lapack library needed for MPQC support]))
	AM_PATH_SC(1.2.5, HAVE_MPQC="yes", HAVE_MPQC="no")
	if test "${HAVE_MPQC}" = "yes"; then
		AC_DEFINE([ENABLE_MPQC], [], [Define if you are building a version that interfaces directly with MPQC])
		AC_DEFINE_UNQUOTED([SC_MAJOR_VERSION], [$SC_MAJOR_VERSION], [This is the major version of SC (MPQC's underlying library) found by configure])
		AC_DEFINE_UNQUOTED([SC_MINOR_VERSION], [$SC_MINOR_VERSION], [This is the minor version of SC (MPQC's underlying library) found by configure])
		AC_DEFINE_UNQUOTED([SC_MICRO_VERSION], [$SC_MICRO_VERSION], [This is the micro version of SC (MPQC's underlying library) found by configure])
		CPPFLAGS="$CPPFLAGS $SC_CPPFLAGS"
		LIBS="$LIBS $SC_LIBS"
	else AC_MSG_ERROR([Cannot find proper SC version])
	fi
fi

# obs! here we get CFLAGS but set it in CPPFLAGS.
if test $WITH_MOPAC7 = yes; then
	AC_DEFINE([ENABLE_MOPAC7], [], [Define if you are building a version that interfaces directly with MOPAC7])
	PKG_CHECK_MODULES(MOPAC7, libmopac7 >= 1.00)
	AC_SUBST(MOPAC7_CFLAGS)
	AC_SUBST(MOPAC7_LIBS)
	CPPFLAGS="$CPPFLAGS $MOPAC7_CFLAGS"
	LIBS="$LIBS $MOPAC7_LIBS"
fi

# obs! here we get CFLAGS but set it in CPPFLAGS.
if test $WITH_OPENBABEL = yes; then
	AC_DEFINE([ENABLE_OPENBABEL], [], [Define if you are building a version that interfaces directly with OpenBabel])
	PKG_CHECK_MODULES(OPENBABEL, openbabel >= 1.100.2)
	AC_SUBST(OPENBABEL_CFLAGS)
	AC_SUBST(OPENBABEL_LIBS)
	CPPFLAGS="$CPPFLAGS $OPENBABEL_CFLAGS"
	LIBS="$LIBS $OPENBABEL_LIBS"
fi

# Checks for header files.
# ^^^^^^^^^^^^^^^^^^^^^^^^

# Checks for typedefs, structures, and compiler characteristics.
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

AC_PROG_CC_STDC
AC_C_CONST
AC_C_INLINE

AC_SUBST(CXXFLAGS)
AC_SUBST(LDFLAGS)

# Checks for library functions.
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

AC_HEADER_STDC

AC_OUTPUT([Makefile src/Makefile
	bin/Makefile bin/builder/Makefile bin/amber/Makefile
	bin/param_mm/Makefile bin/param_mm/default/Makefile bin/param_mm/prmfit/Makefile bin/param_mm/tripos52/Makefile
	bin/param_sf/Makefile bin/param_sf/default/Makefile
	libghemical.pc])

echo
echo build options:
echo ^^^^^^^^^^^^^^
echo enable MPQC : $WITH_MPQC
echo enable MOPAC7 : $WITH_MOPAC7
echo enable OpenBabel : $WITH_OPENBABEL
echo

echo timestamp > stamp.h
