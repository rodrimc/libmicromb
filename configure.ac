dnl Process this file with autoconf to produce a configure script.

VERSION=0.10

AC_PREREQ(2.59)
AC_INIT(libmicromb, 0.10)
AC_CONFIG_HEADERS([config.h])
AC_ENABLE_SHARED
AC_DISABLE_STATIC
AC_CANONICAL_SYSTEM
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_CC

LT_INIT

AM_INIT_AUTOMAKE([subdir-objects])

AH_TEMPLATE([HAVE_INPUT_EVENTS], [])
AH_TEMPLATE([HELLO], [])

# Tests linking
AC_ARG_ENABLE([tests_linking],
              AS_HELP_STRING([--disable-tests-linking], 
                             [disable linking tests with libmicromb.so]),
             [link_tests=false],[link_tests=true])
AM_CONDITIONAL([LINK_TESTS],[test x$link_tests = xtrue])

# Input Events
#AC_ARG_ENABLE([input_events],
#              AS_HELP_STRING([--disable-input-events], 
#                             [disable input events support]),
#             [input_events=no],[input_events=yes])
#AM_CONDITIONAL([INPUT_EVENTS],[test x$link_tests = xtrue])

#if test "$input_events" = yes; then
#    AC_DEFINE (HELLO)
#    AC_DEFINE ([HAVE_INPUT_EVENTS], [1],
#        [Define to 1 if the library should generate input events.])
#    PKG_CHECK_MODULES([GSTREAMER_VIDEO], [gstreamer-video-1.0], [],
#        [AC_MSG_ERROR([gstreamer-video-1.0 is required.])])
#  AC_DEFINE ([HAVE_INPUT_EVENTS], [0],
#             [Define to 1 if the library should generate input events.])
#fi

PKG_CHECK_MODULES([GSTREAMER_VIDEO], [gstreamer-video-1.0], [],
    [AC_MSG_ERROR([gstreamer-video-1.0 is required.])])

PKG_CHECK_MODULES([GSTREAMER], [gstreamer-1.0], [],
   	[AC_MSG_ERROR([gstreamer-1.0 is required.])])

AC_CONFIG_FILES(Makefile src/Makefile src/tests/Makefile)

AC_OUTPUT

