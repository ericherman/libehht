# SPDX-License-Identifier: LGPL-3.0-or-later
# configure.ac: Autoconf for a simple OO hashtable library
# Copyright (C) 2016, 2018, 2019, 2020 Eric Herman <eric@freesa.org>
# https://github.com/ericherman/libehht
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([libehht], [2.0.5-dev], [eric@freesa.org])
AC_CONFIG_SRCDIR([src/ehht.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AC_USE_SYSTEM_EXTENSIONS

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([echeck])

# Checks for header files.
AC_CHECK_HEADERS([stddef.h stdlib.h string.h strings.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([strdup], [check_unsigned_int_m])

AC_ARG_ENABLE(debug,
AS_HELP_STRING([--enable-debug],
               [enable debugging, default: no]),
[case "${enableval}" in
             yes) debug=true ;;
             no)  debug=false ;;
             *)   AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
esac],
[debug=false])

AM_CONDITIONAL(DEBUG, test x"$debug" = x"true")

AM_INIT_AUTOMAKE([subdir-objects -Werror -Wall])
AM_PROG_AR
LT_INIT

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
