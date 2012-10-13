# Configure paths for SC
# Michael Banck 12/26/01
# Adapted from the GTK+ work done by Owen Taylor

######################################################################

dnl AM_PATH_SC([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND [, MODULES]]]])
dnl Test for SC, and define SC_CPPFLAGS and SC_LIBS
dnl
AC_DEFUN(AM_PATH_SC,
[dnl 
dnl Get the cflags and libraries from the sc-config script
dnl
AC_ARG_WITH(sc-prefix,[  --with-sc-prefix=PFX    Prefix where SC is installed (optional)],
            sc_config_prefix="$withval", sc_config_prefix="")
AC_ARG_WITH(sc-exec-prefix,[  --with-sc-exec-prefix=PFX Exec prefix where SC is installed (optional)],
            sc_config_exec_prefix="$withval", sc_config_exec_prefix="")
AC_ARG_ENABLE(sctest, [  --disable-sctest        Do not try to compile and run a test SC program],
		    , enable_sctest=yes)

  for module in . $4
  do
      case "$module" in
         gthread) 
             sc_config_args="$sc_config_args gthread"
         ;;
      esac
  done

  if test x$sc_config_exec_prefix != x ; then
     sc_config_args="$sc_config_args --exec-prefix=$sc_config_exec_prefix"
     if test x${SC_CONFIG+set} != xset ; then
        SC_CONFIG=$sc_config_exec_prefix/bin/sc-config
     fi
  fi
  if test x$sc_config_prefix != x ; then
     sc_config_args="$sc_config_args --prefix=$sc_config_prefix"
     if test x${SC_CONFIG+set} != xset ; then
        SC_CONFIG=$sc_config_prefix/bin/sc-config
     fi
  fi

  AC_PATH_PROG(SC_CONFIG, sc-config, no)
  min_sc_version=ifelse([$1], ,1.2.5,$1)
  AC_MSG_CHECKING(SC - version)
  no_sc=""
  if test "$SC_CONFIG" = "no" ; then
    no_sc=yes
  else
    SC_CPPFLAGS=`$SC_CONFIG $sc_config_args --cppflags`
    SC_LIBS=`$SC_CONFIG $sc_config_args --libs`
    sc_major_version=`$SC_CONFIG $sc_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    sc_minor_version=`$SC_CONFIG $sc_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    sc_micro_version=`$SC_CONFIG $sc_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_sctest" = "xyes" ; then
      ac_save_CPPFLAGS="$CPPFLAGS"
      ac_save_LIBS="$LIBS"
      CPPFLAGS="$CPPFLAGS $SC_CPPFLAGS"
      LIBS="$SC_LIBS $LIBS"
dnl
dnl Now check if the installed SC is sufficiently new. (Also sanity
dnl checks the results of sc-config to some extent
dnl
      rm -f conf.sctest
      AC_TRY_RUN([
#include <scconfig.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char*
my_strdup (char *str)
{
  char *new_str;

  if (str)
    {
      new_str = (char *)malloc ((strlen (str) + 1) * sizeof(char));
      strcpy (new_str, str);
    }
  else
    new_str = NULL;

  return new_str;
}

int main (int argc, char *argv[])
{
  int major, minor, micro;
  char *tmp_version;

  /* This hangs on some systems (?)
  system ("touch conf.sctest");
  */
  { FILE *fp = fopen("conf.sctest", "a"); if ( fp ) fclose(fp); }

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = my_strdup("$min_sc_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_sc_version");
     exit(1);
   }

   if (($sc_major_version > major) ||
      (($sc_major_version == major) && ($sc_minor_version > minor)) ||
      (($sc_major_version == major) && ($sc_minor_version == minor) && ($sc_micro_version >= micro)))
    {
      return 0;
    }
  else
    {
      if ($sc_major_version > major) {
 	printf("\n*** 'sc-config --version' returned %d.%d.%d, but the version\n", $sc_major_version, $sc_minor_version, $sc_micro_version);
	printf("*** of SC required is %d.x.x. If sc-config is correct, then it is\n", major, minor,
micro);
	printf("*** best to downgrade to the required version.\n");
	printf("*** If sc-config was wrong, set the environment variable SC_CONFIG\n");
	printf("*** to point to the correct copy of sc-config, and remove the file\n");
	printf("*** config.cache before re-running configure\n");
	return 1;
      }
      printf("\n*** 'sc-config --version' returned %d.%d.%d, but the minimum version\n", $sc_major_version, $sc_minor_version, $sc_micro_version);
      printf("*** of SC required is %d.%d.%d. If sc-config is correct, then it is\n", major, minor, micro);
      printf("*** best to upgrade to the required version.\n");
      printf("*** If sc-config was wrong, set the environment variable SC_CONFIG\n");
      printf("*** to point to the correct copy of sc-config, and remove the file\n");
      printf("*** config.cache before re-running configure\n");
      return 1;
    }
}

],, no_sc=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CPPFLAGS="$ac_save_CPPFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_sc" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$SC_CONFIG" = "no" ; then
       echo "*** The sc-config script installed by SC could not be found"
       echo "*** If SC was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the SC_CONFIG environment variable to the"
       echo "*** full path to sc-config."
     else
       if test -f conf.sctest ; then
        :
       else
          echo "*** Could not run SC test program, checking why..."
          CPPFLAGS="$CPPFLAGS $SC_CPPFLAGS"
          LIBS="$LIBS $SC_LIBS"
          AC_TRY_LINK([
#include <scconfig.h>
#include <stdio.h>
],      [ return (SC_VERSION); ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding SC or finding the wrong"
          echo "*** version of SC. If it is not finding SC, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"
          echo "***"
          echo "*** If you have a RedHat 5.0 system, you should remove the SC package that"
          echo "*** came with the system with the command"
          echo "***"
          echo "***    rpm --erase --nodeps sc sc-devel" ],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means SC was incorrectly installed"
          echo "*** or that you have moved SC since it was installed. In the latter case, you"
          echo "*** may want to edit the sc-config script: $SC_CONFIG" ])
          CPPFLAGS="$ac_save_CPPFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     SC_CPPFLAGS=""
     SC_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  SC_MAJOR_VERSION="$sc_major_version"
  SC_MINOR_VERSION="$sc_minor_version"
  SC_MICRO_VERSION="$sc_micro_version"
  AC_SUBST(SC_MAJOR_VERSION)
  AC_SUBST(SC_MINOR_VERSION)
  AC_SUBST(SC_MICRO_VERSION)
  AC_SUBST(SC_CPPFLAGS)
  AC_SUBST(SC_LIBS)
  rm -f conf.sctest
])

######################################################################

dnl @synopsis AC_DEFINE_DIR(VARNAME, DIR [, DESCRIPTION])
dnl
dnl This macro _AC_DEFINEs VARNAME to the expansion of the DIR
dnl variable, taking care of fixing up ${prefix} and such.
dnl
dnl Note that the 3 argument form is only supported with autoconf 2.13 and
dnl later (i.e. only where _AC_DEFINE supports 3 arguments).
dnl
dnl Examples:
dnl
dnl    AC_DEFINE_DIR(DATADIR, datadir)
dnl    AC_DEFINE_DIR(PROG_PATH, bindir, [Location of installed binaries])
dnl
dnl @version $Id: acinclude.m4,v 1.3 2005/06/30 13:01:28 thassine Exp $
dnl @author Guido Draheim <guidod@gmx.de>, original by Alexandre Oliva

AC_DEFUN([AC_DEFINE_DIR], [
  test "x$prefix" = xNONE && prefix="$ac_default_prefix"
  test "x$exec_prefix" = xNONE && exec_prefix='${prefix}'
  ac_define_dir=`eval echo [$]$2`
  ac_define_dir=`eval echo [$]ac_define_dir`
  ifelse($3, ,
    AC_DEFINE_UNQUOTED($1, "$ac_define_dir"),
    AC_DEFINE_UNQUOTED($1, "$ac_define_dir", $3))
])

######################################################################

#
# Check to make sure that the build environment is sane.
#

AC_DEFUN(AM_SANITY_CHECK,
[AC_MSG_CHECKING([whether build environment is sane])
# Just in case
sleep 1
echo timestamp > conftestfile
# Do `set' in a subshell so we don't clobber the current shell's
# arguments.  Must try -L first in case configure is actually a
# symlink; some systems play weird games with the mod time of symlinks
# (eg FreeBSD returns the mod time of the symlink's containing
# directory).
if (
   set X `ls -Lt $srcdir/configure conftestfile 2> /dev/null`
   if test "[$]*" = "X"; then
      # -L didn't work.
      set X `ls -t $srcdir/configure conftestfile`
   fi
   if test "[$]*" != "X $srcdir/configure conftestfile" \
      && test "[$]*" != "X conftestfile $srcdir/configure"; then

      # If neither matched, then we have a broken ls.  This can happen
      # if, for instance, CONFIG_SHELL is bash and it inherits a
      # broken ls alias from the environment.  This has actually
      # happened.  Such a system could not be considered "sane".
      AC_MSG_ERROR([ls -t appears to fail.  Make sure there is not a broken
alias in your environment])
   fi

   test "[$]2" = conftestfile
   )
then
   # Ok.
   :
else
   AC_MSG_ERROR([newly created file is older than distributed files!
Check your system clock])
fi
rm -f conftest*
AC_MSG_RESULT(yes)])

######################################################################
