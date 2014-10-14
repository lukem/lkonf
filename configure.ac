AC_PREREQ([2.69])
AC_INIT([luaconfig], [0.1], [Luke@Mewburn.net])

AC_COPYRIGHT([[
Copyright 2014, Luke Mewburn <Luke@Mewburn.net>.
All rights reserved.
]])
AC_REVISION([m4_esyscmd_s([git describe --tags --always])])

AC_CONFIG_AUX_DIR([buildaux])
AC_CONFIG_MACRO_DIR([buildaux])

AM_INIT_AUTOMAKE([foreign nostdinc silent-rules -Werror -Wall])
AM_MAINTAINER_MODE()

AS_SHELL_SANITIZE()

AC_CONFIG_SRCDIR([src/luaconfig.h])

dnl AC_CONFIG_TESTDIR([tests])
dnl AM_MISSING_PROG([AUTOM4TE], [autom4te])

AC_CONFIG_FILES([
	Makefile
	src/Makefile
])
dnl	tests/Makefile

AC_OUTPUT()
