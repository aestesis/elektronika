# Process this file with autoconf to produce a configure script.
# $Id: configure.ac 8946 2005-02-19 17:49:25Z brendan $

m4_define(libshout_major, 2)
m4_define(libshout_minor, 1)
m4_define(libshout_micro, 0)
m4_define(libshout_version,
  m4_if(libshout_micro, 0, libshout_major.libshout_minor,
   libshout_major.libshout_minor.libshout_micro))

AC_INIT([libshout], libshout_version, [icecast-dev@xiph.org])
AC_PREREQ([2.54])
AC_CONFIG_SRCDIR([src/shout.c])
AM_CONFIG_HEADER(config.h)
# config.h guard
AH_TOP([#ifndef __CONFIG_H__
#define __CONFIG_H__ 1])
AH_BOTTOM([#endif])

AC_DEFINE([LIBSHOUT_MAJOR], libshout_major, [Shout library major version])
AC_DEFINE([LIBSHOUT_MINOR], libshout_minor, [Shout library minor version])
AC_DEFINE([LIBSHOUT_MICRO], libshout_micro, [Shout library patch version])

VERSION=libshout_version

AM_INIT_AUTOMAKE([libshout], libshout_version)
AM_MAINTAINER_MODE

dnl create our name mangling macro
dnl the prefix must be hardwired because of AH limitations
AH_VERBATIM([_mangle], [
/* name mangling to protect code we share with other libraries */
#define _mangle(proc) _shout_ ## proc
])

AC_PROG_CC
AM_PROG_LIBTOOL

dnl Set some options based on environment

dnl openbsd headers break when _XOPEN_SOURCE is defined but without it seems
dnl to be fine
case "$ac_cv_host" in
   *openbsd* | *solaris* | *irix*)
   ;;
   *) AC_DEFINE(_XOPEN_SOURCE, 600, [Define if you have POSIX and XPG specifications])
   ;;
esac
if test -z "$GCC"; then
        case $host in 
        *-*-irix*)
                DEBUG="-g -signed" 
                CFLAGS="-O2 -w -signed"
                PROFILE="-p -g3 -O2 -signed"
		;;
        sparc-sun-solaris*)
                DEBUG="-v -g"
                CFLAGS="-xO4 -fast -w -fsimple -native -xcg92"
                PROFILE="-v -xpg -g -xO4 -fast -native -fsimple -xcg92 -Dsuncc"
		;;
        *)
                DEBUG="-g"
                CFLAGS="-O"
                PROFILE="-g -p" 
		;;
        esac
else
        XIPH_CFLAGS="-Wall -ffast-math -fsigned-char"
        AC_DEFINE(_GNU_SOURCE, ,[Define if you have POSIX and GNU specifications])
        DEBUG="-g"
        PROFILE="-pg -g"
fi

dnl Checks for programs.

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([strings.h])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
XIPH_C99_INTTYPES

dnl Checks for library functions.
AC_SEARCH_LIBS([nanosleep], [rt],
  [AC_DEFINE([HAVE_NANOSLEEP], [1],
    [Define if you have the nanosleep function])])

dnl Module checks
XIPH_NET

dnl Extra dependencies
AC_ARG_ENABLE([thread],
  AC_HELP_STRING([--disable-thread],[do not build with thread support even if it is available]))

SHOUT_THREADSAFE="0"
if test "$enable_thread" != "no"
then
  ACX_PTHREAD([
    LIBS="$LIBS $PTHREAD_LIBS"
    XIPH_CFLAGS="$XIPH_CFLAGS $PTHREAD_CFLAGS $PTHREAD_CPPFLAGS"
    CC="$PTHREAD_CC"
    SHOUT_THREADSAFE="1"
    ])
fi
AC_SUBST([SHOUT_THREADSAFE])
AM_CONDITIONAL([HAVE_THREAD], [test "$SHOUT_THREADSAFE" = "1"])
if test "$SHOUT_THREADSAFE" != "1"
then
  AC_DEFINE([NO_THREAD], 1, [Define if you don't want to use the thread library])
fi

XIPH_PATH_VORBIS(, [AC_MSG_ERROR([requisite Ogg Vorbis library not found])])
VORBIS_LIBS="$VORBIS_LDFLAGS $VORBIS_LIBS"
XIPH_CFLAGS="$XIPH_CFLAGS $VORBIS_CFLAGS"

XIPH_PATH_THEORA(, [AC_MSG_WARN([Theora library not found, disabling])])
XIPH_VAR_APPEND([XIPH_CPPFLAGS],[$THEORA_CFLAGS])
XIPH_VAR_PREPEND([XIPH_LIBS],[$THEORA LDFLAGS $THEORA_LIBS])
AM_CONDITIONAL(HAVE_THEORA, test -n "$THEORA_LIBS")

dnl pkgconfig/shout-config.
dnl If pkgconfig is found, use it and disable shout-config, otherwise do the
dnl opposite, unless the user overrides.

AC_ARG_ENABLE([pkgconfig],
  AC_HELP_STRING([--disable-pkgconfig],[disable pkgconfig data files (auto)]),
  [dopkgconfig="$enableval"], [dopkgconfig="maybe"])
if test "$dopkgconfig" = "maybe"
then
  AC_CHECK_PROG([PKGCONFIG], [pkg-config], [yes], [no])
else
  AC_MSG_CHECKING([whether pkgconfig should be used])
  PKGCONFIG="$dopkgconfig"
  AC_MSG_RESULT([$PKGCONFIG])
fi
AM_CONDITIONAL([HAVE_PKGCONFIG], [test "$PKGCONFIG" != "no"])

# Build shout-config, shout.pc

# I hate myself for doing this.
save_prefix="$prefix"
if test "$prefix" = "NONE"
then
  prefix="$ac_default_prefix"
fi
eval shout_includedir="$includedir"
prefix="$save_prefix"

SHOUT_VERSION="$VERSION"
SHOUT_CPPFLAGS="-I$shout_includedir $VORBIS_CFLAGS $PTHREAD_CPPFLAGS"
SHOUT_CFLAGS="$PTHREAD_CFLAGS"
SHOUT_LIBS="-lshout"

XIPH_CLEAN_CCFLAGS([$SHOUT_CPPFLAGS], [SHOUT_CPPFLAGS])
XIPH_CLEAN_CCFLAGS([$SHOUT_CFLAGS], [SHOUT_CFLAGS])
XIPH_CLEAN_CCFLAGS([$VORBIS_LIBS $PTHREAD_LIBS $LIBS], [SHOUT_LIBDEPS])
AC_SUBST(SHOUT_LIBDEPS)
AC_SUBST(SHOUT_CPPFLAGS)
AC_SUBST(SHOUT_CFLAGS)
AC_CONFIG_LIBCONFIG_IN_STATIC
AC_CONFIG_LIBCONFIG_IN([shout])

dnl Make substitutions

AC_SUBST(LIBTOOL_DEPS)
AC_SUBST(OPT)
AC_SUBST(LIBS)
AC_SUBST(DEBUG)
AC_SUBST(CFLAGS)
AC_SUBST(PROFILE)
AC_SUBST(XIPH_CFLAGS)
AC_SUBST(XIPH_CPPFLAGS)

AC_OUTPUT([Makefile debian/Makefile include/Makefile include/shout/Makefile
  include/shout/shout.h src/Makefile src/net/Makefile src/timing/Makefile
  src/thread/Makefile src/avl/Makefile src/httpp/Makefile doc/Makefile
  examples/Makefile win32/Makefile shout-config shout.pc])
