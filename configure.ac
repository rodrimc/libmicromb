dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(Libmicromb, 0.10)

VERSION=0.10

AC_ENABLE_SHARED
AC_DISABLE_STATIC
LT_INIT

AM_CONFIG_HEADER(config.h)

AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE([subdir-objects])

AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CC

link_tests=
AC_ARG_ENABLE([link_tests],
              [  --enable-link-tests     link tests with libmicromb.so],
              [case "${enableval}" in
               yes) link_tests=true ;;
               no)  link_tests=false ;;
               *) AC_MSG_ERROR([bad value ${enableval} for --enable-link-tests]) ;;
             esac],[link_tests=true])
AM_CONDITIONAL([LINK_TESTS],[test x$link_tests = xtrue])

PKG_CHECK_MODULES([GSTREAMER], [gstreamer-1.0], [],
   	[AC_MSG_ERROR([The gstreamer-1.0 is required.])])

AC_CONFIG_FILES(Makefile src/Makefile src/tests/Makefile)

AC_OUTPUT

