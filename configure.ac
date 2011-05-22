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
AC_CONFIG_MACRO_DIR([m4])
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


##### tar #####

AC_CHECK_PROGS([TAR], [tar gtar], [:])
if test "$TAR" = :; then
AC_MSG_ERROR([tar not found])
fi
AC_MSG_RESULT([tar $TAR])

##### unzip #####

AC_CHECK_PROGS([UNZIP], [unzip], [:])
if test "$" = :; then
  AC_MSG_ERROR([unzip not found])
fi
AC_MSG_RESULT([unzip $UNZIP])

##### grep #####

AC_PROG_GREP

##### sed #####

AC_PROG_SED

##### gdal #####

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

##### OTB #####


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
  
  prefrm=`grep "SET" "$otblibdep" | cut -d '"' -f 4 | tr ";" "\n" | sed 's/^-l//' | grep -v -e "^$" `
  while read lib
  do
    if test "$lib" = "general"
    then
        continue;
    fi
    
    dir="${lib%/*}"
    so="${lib##*/}"
    soa="${so%%.*}"
    sob=`echo $soa | sed 's/^lib//'`
    if test "$dir" = "$so"
    then
        OTBLIB="$OTBLIB -l$so"
    else
        OTBLIB="$OTBLIB -L$dir -l$sob"
    fi
  done << EOF
$prefrm
EOF
  

  AC_SUBST(OTBINC)
  AC_SUBST(OTBLIB)
else
  AC_MSG_FAILURE([cannot find OTBConfig.cmake try setting --with-otb-lib=/some/path])
fi

##### tiff #####

AC_ARG_WITH([tiff-includes],
  [AC_HELP_STRING([--with-tiff-includes],[Location of libtiff include files.])],
  [
    TIFFINC="-I${withval}"
    CFLAGS="$CFLAGS -I${withval}"
  ]
)

AC_ARG_WITH([tiff-libs],
  [AC_HELP_STRING([--with-tiff-libs],[Location of libtiff library files.])],
  [
    TIFFLIB="-L${withval} -ltiff"
    LDFLAGS="$LDFLAGS -L${withval} -ltiff"
  ],
  [
    TIFFLIB="-ltiff"
    LDFLAGS="$LDFLAGS -ltiff"
  ]
)

AC_CHECK_HEADERS([tiffio.h],[HAVE_TIFFIO_H=1],[AC_MSG_ERROR(libtiff is required to compile dwh.)])
AC_CHECK_LIB([tiff],[TIFFGetVersion],[HAVE_LIBTIFF=1],[AC_MSG_ERROR(libtiff is required to compile dwh.)])

AC_SUBST(TIFFINC)
AC_SUBST(TIFFLIB)

##### geotiff #####

AC_ARG_WITH([geotiff-includes],
  [AC_HELP_STRING([--with-geotiff-includes],[Location of libgeotiff include files.])],
  [
    GEOTIFFINC="-I${withval}"
    CFLAGS="$CFLAGS -I${withval}"
  ]
)

AC_ARG_WITH([geotiff-libs],
  [AC_HELP_STRING([--with-geotiff-libs],[Location of libgeotiff library files.])],
  [
    GEOTIFFLIB="-L${withval} -lgeotiff"
    LDFLAGS="$LDFLAGS -L${withval} -lgeotiff"
  ],
  [
    GEOTIFFLIB="-lgeotiff"
    LDFLAGS="$LDFLAGS -lgeotiff"
  ]
)

AC_CHECK_HEADERS([geotiff.h],[HAVE_GEOTIFFIO_H=1],[AC_MSG_ERROR(libgeotiff is required to compile dwh.)])
AC_CHECK_LIB([geotiff],[GTIFNew],[HAVE_LIBGEOTIFF=1],[AC_MSG_ERROR(libgeotiff is required to compile dwh.)])

AC_SUBST(GEOTIFFINC)
AC_SUBST(GEOTIFFLIB)

##### jpeg #####

AC_ARG_WITH([jpeg-includes],
  [AC_HELP_STRING([--with-jpeg-includes],[Location of jpeg include files.])],
  [
    JPEGINC="-I${withval}"
    CFLAGS="$CFLAGS -I${withval}"
  ]
)

AC_ARG_WITH([jpeg-libs],
  [AC_HELP_STRING([--with-jpeg-libs],[Location of jpeg library files.])],
  [
    JPEGLIB="-L${withval} -ljpeg"
    LDFLAGS="$LDFLAGS -L${withval} -ljpeg"
  ],
  [
    JPEGLIB="-ljpeg"
    LDFLAGS="$LDFLAGS -ljpeg"
  ]
)

AC_CHECK_HEADERS([jpeglib.h],[HAVE_JPEG_H=1],[AC_MSG_ERROR(jpeg is required to compile dwh.)])
AC_CHECK_LIB([jpeg],[jpeg_set_defaults],[HAVE_JPEG=1],[AC_MSG_ERROR(jpeg is required to compile dwh.)])

AC_SUBST(JPEGINC)
AC_SUBST(JPEGLIB)


##### szip #####

AC_ARG_WITH([szip-includes],
  [AC_HELP_STRING([--with-szip-includes],[Location of szip include files.])],
  [
    JPEGINC="-I${withval}"
    CFLAGS="$CFLAGS -I${withval}"
  ]
)

AC_ARG_WITH([szip-libs],
  [AC_HELP_STRING([--with-szip-libs],[Location of szip library files.])],
  [
    SZLIB="-L${withval} -lsz"
    LDFLAGS="$LDFLAGS -L${withval} -lsz"
  ],
  [
    SZLIB="-lsz"
    LDFLAGS="$LDFLAGS -lsz"
  ]
)

AC_CHECK_HEADERS([szlib.h],[HAVE_SZLIB_H=1],[AC_MSG_ERROR(szip is required to compile dwh.)])
AC_CHECK_LIB([sz],[SZ_Decompress],[HAVE_SZ=1],[AC_MSG_ERROR(szip is required to compile dwh.)])

AC_SUBST(SZINC)
AC_SUBST(SZLIB)


##### zlib #####

AC_ARG_WITH([zlib-includes],
  [AC_HELP_STRING([--with-zlib-includes],[Location of zlib include files.])],
  [
    ZINC="-I${withval}"
    CFLAGS="$CFLAGS -I${withval}"
  ]
)

AC_ARG_WITH([zlib-libs],
  [AC_HELP_STRING([--with-zlib-libs],[Location of zlib library files.])],
  [
    ZLIB="-L${withval} -lz"
    LDFLAGS="$LDFLAGS -L${withval} -lz"
  ],
  [
    ZLIB="-lz"
    LDFLAGS="$LDFLAGS -lz"
  ]
)

AC_CHECK_HEADERS([szlib.h],[HAVE_SZLIB_H=1],[AC_MSG_ERROR(zlib is required to compile dwh.)])
AC_CHECK_LIB([z],[SZ_Decompress],[HAVE_SZ=1],[AC_MSG_ERROR(zlib is required to compile dwh.)])

AC_SUBST(ZINC)
AC_SUBST(ZLIB)


##### hdf #####

AC_ARG_WITH([hdf4-includes],
  [AC_HELP_STRING([--with-hdf4-includes],[Location of hdf4 include files.])],
  [
    HDFINC="-I${withval}"
    CFLAGS="$CFLAGS -I${withval}"
  ]
)

AC_ARG_WITH([hdf4-libs],
  [AC_HELP_STRING([--with-hdf4-libs],[Location of hdf4 library files.])],
  [
    HDFLIB="-L${withval} -ldf -lmfhdf"
    LDFLAGS="$LDFLAGS -L${withval} -ldf -lmfhdf"
  ],
  [
    HDFLIB="-ldf -lmfhdf"
    LDFLAGS="$LDFLAGS -ldf -lmfhdf"
  ]
)

AC_CHECK_HEADERS([mfhdf.h],[HAVE_MFHDF_H=1],[AC_MSG_ERROR(hdf4 is required to compile dwh.)])
AC_CHECK_HEADERS([df.h],[HAVE_DF_H=1],[AC_MSG_ERROR(hdf4 is required to compile dwh.)])
AC_CHECK_LIB([df],[DFopen],[HAVE_DF=1],[AC_MSG_ERROR(hdf4 is required to compile dwh.)])
AC_CHECK_LIB([mfhdf -ldf],[SDstart],[HAVE_MFHDF=1],[AC_MSG_ERROR(hdf4 is required to compile dwh.)])

AC_SUBST(HDFINC)
AC_SUBST(HDFLIB)

##### hdfeos #####

AC_ARG_WITH([hdfeos-includes],
  [AC_HELP_STRING([--with-hdfeos-includes],[Location of hdfeos include files.])],
  [
    HDFEOSINC="-I${withval}"
    CFLAGS="$CFLAGS --include hdf.h -I${withval}"
  ]
)

AC_ARG_WITH([hdfeos-libs],
  [AC_HELP_STRING([--with-hdfeos-libs],[Location of hdfeos library files.])],
  [
    HDFEOSLIB="-L${withval} -lhdfeos"
    LDFLAGS="$LDFLAGS -L${withval} -lhdfeos"
  ],
  [
    HDFEOSLIB="-lhdfeos"
    LDFLAGS="$LDFLAGS -lhdfeos"
  ]
)

AC_CHECK_HEADERS([HDFEOSVersion.h],[HAVE_HdfEosDef_H=1],[AC_MSG_ERROR(hdfeos is required to compile dwh.)])
AC_CHECK_LIB([hdfeos],[SWopen],[HAVE_LIBGEOTIFF=1],[AC_MSG_ERROR(hdfeos is required to compile dwh.)])

AC_SUBST(HDFEOSINC)
AC_SUBST(HDFEOSLIB)


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
MRTSwath/Makefile
MRTSwath/data/Makefile
gctpc/Makefile
])
