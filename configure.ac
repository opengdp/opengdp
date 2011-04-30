dnl Process this file with autoconf to produce a configure script.
dnl Created by Anjuta application wizard.

AC_INIT(dwh, 0.1.0)
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AC_CONFIG_HEADERS([config.h])
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CXX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC

AM_PROG_LIBTOOL

dnl bash

AC_CHECK_PROG(HAVE_BASH, bash, true, false)
if test $HAVE_BASH = "false"; then
  AC_MSG_FAILURE([bash not found])
fi


dnl Check for dialog
AC_CHECK_PROGS([DIALOG], [dialog], [:])
if test "$DIALOG" = :; then
AC_MSG_ERROR([dialog not found])
fi
AC_MSG_RESULT([dialog $DIALOG])


dnl tar

AC_CHECK_PROGS([TAR], [tar gtar], [:])
if test "$TAR" = :; then
AC_MSG_ERROR([tar not found])
fi
AC_MSG_RESULT([tar $TAR])

dnl unzip

AC_CHECK_PROGS([UNZIP], [unzip], [:])
if test "$" = :; then
  AC_MSG_ERROR([unzip not found])
fi
AC_MSG_RESULT([unzip $UNZIP])

dnl grep

AC_PROG_GREP

dnl sed

AC_PROG_SED

dnl gdal

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
  AC_SUBST(GDAL_CFLAGS)
  AC_SUBST(GDAL_LIBS)
  found_gdal="yes"
else
  found_gdal="no"
fi
AM_CONDITIONAL(HAVE_GDAL, test "x$GDAL_CONFIG" != "x")

dnl OTB

AC_ARG_WITH(otb-lib,
  [AS_HELP_STRING([--with-otb-lib=/some/path],
    [path to the OTBConfig.cmake file])],
  if test -n "$withval"
  then
    otblibpath="$withval"
  else
    [AC_MSG_FAILURE([cannot find OTBConfig.cmake try setting --with-otb-lib=/some/path])]
   fi,
  otblibpath=`grep -v -e "^#" /etc/ld.so.conf`
)

otbconfig=`find $otblibpath -name OTBConfig.cmake 2> /dev/null | head -n 1 `
otblibdep=`find $otblibpath -name OTBLibraryDepends.cmake 2> /dev/null | head -n 1 `
otbbase="${otbconfig%/*}"
otbbase="${otbbase%/*}"
otbbase="${otbbase%/*}"
if test -n "$otbconfig"
then
  OTBINC=`(echo -n ";" ; grep "SET(OTB_INCLUDE_DIRS" "$otbconfig" | cut -d '"' -f 2) | sed 's/;/ -I/g' | sed "s,..OTB_INSTALL_PREFIX.,$otbbase,g" `
  OTBLIB=`(echo -n ";" ; grep "SET(OTB_LIBRARY_DIRS" "$otbconfig" | cut -d '"' -f 2) | sed 's/;/ -L/g' | sed "s,..OTB_INSTALL_PREFIX.,$otbbase,g" `
  
  while read line
  do
    OTBLIB="$OTBLIB ` (echo -n ";" ; echo "$line" | grep "SET" | cut -d '"' -f 4 ) | sed -e 's/general;//g' -e 's/;$//' -e 's/-l//g' -e 's/;/ -l/g' -e 's,-l/[-_./a-zA-Z0-9]*lib,-l,g' -e 's/.so[.0-9]*//g'  ` "
  done < "$otblibdep"

  AC_SUBST(OTBINC)
  AC_SUBST(OTBLIB)
else
  AC_MSG_FAILURE([cannot find OTBConfig.cmake try setting --with-otb-lib=/some/path])
fi



AC_OUTPUT([
Makefile
scripts/Makefile
share/Makefile
share/cgibin/Makefile
share/examples/Makefile
share/examples/floods_austrailia/Makefile
share/examples/haiti/Makefile
share/examples/jp_earthquake/Makefile
share/examples/naip/Makefile
share/generic/Makefile
share/map/Makefile
share/scripts/Makefile
share/web/Makefile
src/Makefile

])
