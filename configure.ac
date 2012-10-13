# Configuration for ghemical.
# Copyright (c) 2000- Geoff Hutchison, Michael Banck, Tommi Hassinen.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.

# Process this file with aclocal/autoconf to produce a configure script.

AC_INIT(ghemical, 1.90)
AC_DEFINE([APPVERSION], ["1.90"], [This is the version of ghemical to be built--it is set in the configure.ac])
AC_SUBST(APPVERSION, "1.90")
AC_DEFINE([APPRELEASEDATE], ["2005-06-30"], [This is the releasedate of ghemical--it is set in the configure.ac])
AC_SUBST(APPRELEASEDATE, "2005-06-30")

AC_PREREQ(2.48)
AC_CONFIG_SRCDIR(src/main.cpp)
AC_CONFIG_HEADER(src/appconfig.h)

AC_CANONICAL_HOST

top_builddir_=`pwd`
AC_SUBST(top_builddir_)

AC_DEFINE_DIR([APPDATA_PATH], [datadir/ghemical], [Where the data files are ; set in the configure.ac])

# Get any build/host and enable/disable flags now.
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# if a mingw32 host is specified, then build code that links against native (MS) runtime libs.
case $host in
	*-*-mingw32*)
		CPPFLAGS="-Dmno-cygwin -Dmms_bitfields $CPPFLAGS"
		LIBS="$LIBS -mwindows"
		;;
esac

AC_ARG_ENABLE(threads,[  --enable-threads        compile into a multithreaded app], WITH_THREADS="$enableval", WITH_THREADS="no")
AC_ARG_ENABLE(bonobo,[  --enable-bonobo         compile into a bonobo2 server], WITH_BONOBO="$enableval", WITH_BONOBO="no")

# Checks for programs.
# ^^^^^^^^^^^^^^^^^^^^

AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_CXXCPP
AC_PROG_INSTALL
AC_PROG_RANLIB
AC_PATH_PROG(AR, ar, ar)
AC_PATH_PROG(SHELL, sh, /bin/sh)
AC_PROG_MAKE_SET
AM_SANITY_CHECK

# Checks for libraries.
# ^^^^^^^^^^^^^^^^^^^^^

AC_CHECK_LIB([m], [acos])

if test $WITH_THREADS = yes; then
	AC_DEFINE([ENABLE_THREADS], [], [Define if you are building a multithreaded app])
	PKG_CHECK_MODULES(GTHREAD, gthread-2.0 >= 2.4.0)
	CPPFLAGS="$CPPFLAGS $GTHREAD_CFLAGS"
	LIBS="$LIBS $GTHREAD_LIBS"
fi

if test $WITH_BONOBO = yes; then
	AC_DEFINE([ENABLE_BONOBO], [], [Define if you are building a bonobo2 server version])
	## todo ; include the cflags/libs...
fi

PKG_CHECK_MODULES(GTK, gtk+-2.0 >= 2.4.0)
CPPFLAGS="$CPPFLAGS $GTK_CFLAGS"
LIBS="$LIBS $GTK_LIBS"

PKG_CHECK_MODULES(GTKGLEXT, gtkglext-1.0 >= 1.0.5)
CPPFLAGS="$CPPFLAGS $GTKGLEXT_CFLAGS"
LIBS="$LIBS $GTKGLEXT_LIBS"

PKG_CHECK_MODULES(LIBGLADE, libglade-2.0 >= 2.4.0)
CPPFLAGS="$CPPFLAGS $LIBGLADE_CFLAGS"
LIBS="$LIBS $LIBGLADE_LIBS"

PKG_CHECK_MODULES(LIBGHEMICAL, libghemical >= 1.90)
CPPFLAGS="$CPPFLAGS $LIBGHEMICAL_CFLAGS"
LIBS="$LIBS $LIBGHEMICAL_LIBS"

## todo ; any checks missing here???

AC_CHECK_LIB([GL], [glMatrixMode])
AC_CHECK_LIB([GLU], [main])
AC_CHECK_LIB([glut], [glutInit])
AC_CHECK_LIB([X11], [main])
AC_CHECK_LIB([Xmu], [main])
AC_CHECK_LIB([Xext], [main])
AC_CHECK_LIB([Xi], [main])

# Checks for header files.
# ^^^^^^^^^^^^^^^^^^^^^^^^

AC_CHECK_HEADERS(GL/glut.h)

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

AC_OUTPUT([Makefile
	src/glade/Makefile
	src/Makefile])

echo
echo build options:
echo ^^^^^^^^^^^^^^
echo enable threads : $WITH_THREADS
echo enable bonobo : $WITH_BONOBO
echo

echo timestamp > stamp.h
