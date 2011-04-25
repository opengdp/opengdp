dnl Process this file with autoconf to produce a configure script.
dnl Created by Anjuta application wizard.

AC_INIT(dwh, 0.1.0)
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AC_CONFIG_HEADERS([config.h])
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC






AM_PROG_LIBTOOL

AC_CHECK_PROG(HAVE_BASH, bash, true, false)
if test $HAVE_BASH = "false"; then
  AC_MSG_FAILURE([bash not found])
fi

AC_CHECK_PROGS([TAR], [tar gtar], [:])
if test "$TAR" = :; then
AC_MSG_ERROR([tar not found])
fi
AC_MSG_RESULT([tar $TAR])

AC_CHECK_PROGS([UNZIP], [unzip], [:])
if test "$" = :; then
  AC_MSG_ERROR([unzip not found])
fi
AC_MSG_RESULT([unzip $UNZIP])


AC_PROG_GREP

AC_PROG_SED

AC_ARG_WITH(gdal-config,
[  --with-gdal-config=GDAL_CONFIG    use gdal-config specified ],
[
  GDAL_CONFIG=$withval;
  echo "using "$GDAL_CONFIG" for gdal-config";
],[
  PROG="gdal-config";
  AC_PATH_PROG(GDAL_CONFIG, $PROG, "", $PATH)
])

if test "x$GDAL_CONFIG" != "x"; then
  GDAL_CFLAGS=`$GDAL_CONFIG --cflags`
  GDAL_LIBS=`$GDAL_CONFIG --libs`
  GDAL_VERSION=`$GDAL_CONFIG --version`
  AC_MSG_RESULT([$GDAL_CONFIG reports version $GDAL_VERSION])
  AC_SUBST(GDAL_CFLAGS)
  AC_SUBST(GDAL_LIBS)
  found_gdal="yes"
else
  found_gdal="no"
fi
AM_CONDITIONAL(HAVE_GDAL, test "x$GDAL_CONFIG" != "x")



AC_OUTPUT([
Makefile
share/Makefile
share/generic/Makefile
src/Makefile

])
