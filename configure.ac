AC_PREREQ([2.69])
AC_INIT([lkonf], [0.4], [Luke@Mewburn.net])

AC_COPYRIGHT([[
Copyright 2014, Luke Mewburn <Luke@Mewburn.net>.
All rights reserved.
]])
AC_REVISION([m4_esyscmd_s([git describe --tags --always])])

AC_CONFIG_AUX_DIR([buildaux])
AC_CONFIG_MACRO_DIR([buildaux])

m4_pattern_forbid([^AX_])
m4_pattern_forbid([^PKG_])

AM_INIT_AUTOMAKE([foreign nostdinc silent-rules -Werror -Wall])
AM_MAINTAINER_MODE()

LT_PREREQ([2.4])

AS_SHELL_SANITIZE()

AC_CONFIG_SRCDIR([src/lkonf.h])

AC_PROG_CC()
AC_PROG_LIBTOOL()

AX_APPEND_COMPILE_FLAGS([-Werror -Wall -Wextra -Wpointer-arith -Wshadow], [WARNCFLAGS])
AC_SUBST([WARNCFLAGS])

PKG_CHECK_MODULES([LUA], [lua])

dnl TODO doxygen
dnl

dnl Initialise the testsuite
dnl
AC_CONFIG_TESTDIR([tests])
AM_MISSING_PROG([AUTOM4TE], [autom4te])


dnl Create the Makefiles
dnl
AC_CONFIG_FILES([
	Makefile
	src/Makefile
	tests/Makefile
])

AC_OUTPUT()


dnl Display feature results
dnl
AC_MSG_NOTICE([ === Configuration results ===])
AC_MSG_NOTICE([Package:             $PACKAGE_STRING])
AC_MSG_NOTICE([c compiler:          $CC])
AC_MSG_NOTICE([WARNCFLAGS:          $WARNCFLAGS])
AC_MSG_NOTICE([ === Configuration results ===])
