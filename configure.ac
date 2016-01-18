#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([xstarter], [0.1])
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_SRCDIR([src/settings.h])
PKG_CHECK_MODULES([GLIB], [glib-2.0])
AC_CONFIG_HEADERS([src/config.h])

AC_OPENMP

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([limits.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([strdup])

AC_OUTPUT(Makefile src/Makefile)
