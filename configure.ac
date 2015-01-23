dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(Libmb, 1.0)

AM_CONFIG_HEADER(config.h)

AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE([subdir-objects])

AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CC

PKG_CHECK_MODULES([GSTREAMER], [gstreamer-1.0], [],
   	[AC_MSG_ERROR([The gstreamer-1.0 is required.])])

AC_CONFIG_FILES(Makefile src/Makefile src/tests/Makefile)

AC_OUTPUT

