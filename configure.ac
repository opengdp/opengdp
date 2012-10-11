dnl Process this file with autoconf to produce a configure script.
dnl Created by Anjuta application wizard.

AC_INIT(opengdp, 0.1.0)
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AC_CONFIG_HEADERS([config.h])
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CXX
AC_PROG_CC
AC_USE_SYSTEM_EXTENSIONS

AM_PROG_CC_STDC
AC_HEADER_STDC

AM_PROG_LIBTOOL
AC_CONFIG_MACRO_DIR([m4])


AC_ARG_WITH(wwwdisk,
  [AS_HELP_STRING([--with-wwwdisk=/some/path],
    [base dir to output kml and images])],
  if test -n "$withval"
  then
    WWWDISK="$withval"
    AC_SUBST(WWWDISK)
  else
		[AC_MSG_FAILURE([--with-wwwdisk=/some/path MUST be specified])]
	fi,
  [AC_MSG_FAILURE([--with-wwwdisk=/some/path MUST be specified])]
)


cflagsold="$CFLAGS"
ldflagsold="$LDFLAGS"
dnl -----------------------------------------------------------------
dnl Check for libraries.  Most installs will need to have
dnl -lm but under OS X it is not needed.  Thus, we look
dnl for sin in -lm which should set -lm on most platforms.
dnl We also look for sincos in -m and -lsunmath for Solaris.
dnl -----------------------------------------------------------------
dnl
AC_SEARCH_LIBS(sin, m)
AC_SEARCH_LIBS(sincos, [m sunmath], [SINCOS=1], [SINCOS=0])
dnl
dnl OSF specific check for broken sincos from Lloyd Parkes (lloyd@must-have-coffee.gen.nz)
dnl
if test "$SINCOS" = "1" ; then
	AC_MSG_CHECKING(for a broken sincos function)
	AC_TRY_RUN([int main ()
	{
		double s = 0.1, c = 0.2;
		double s1, c1;

		s1 = s; c1 = c;
		sincos (0.5, &s, &c);
		return (s == s1 || c == c1);	/* FALSE if sincos works ok */
	}
	], [ AC_MSG_RESULT(no)
	AC_DEFINE([HAVE_SINCOS],[],[System has sincos])
	], [
	case $target in
		*)
			AC_MSG_RESULT([yes.])
			AC_MSG_RESULT([I don't know how to fix sincos on this machine.])
	esac
	], [
	AC_MSG_RESULT([no sincos found.])
	])
fi


##### bash #####

AC_CHECK_PROG(HAVE_BASH, bash, true, false)
if test $HAVE_BASH = "false"; then
  AC_MSG_FAILURE([bash not found])
fi


##### dialog #####

AC_CHECK_PROGS([DIALOG], [dialog], [:])
if test "$DIALOG" = :
then
    AC_MSG_RESULT([dialog not found])
else
    AC_MSG_RESULT([dialog $DIALOG])
    HAVE_DIALOG=true
    AC_SUBST(HAVE_DIALOG)
fi

##### mc #####

AC_CHECK_PROGS([MC], [mc], [:])
if test "$MC" = :
then
    AC_MSG_RESULT([mc not found])
else
    AC_MSG_RESULT([mc $MC])
    HAVE_MC=true
    AC_SUBST(HAVE_MC)
fi

##### mrsiddecode #####

AC_CHECK_PROGS([MRSIDDECODE], [mrsiddecode], [:])
if test "$MC" = :
then
    AC_MSG_RESULT([mrsiddecode not found])
else
    AC_MSG_RESULT([mrsiddecode $MRSIDDECODE])
    HAVE_MRSIDDECODE=true
    AC_SUBST(HAVE_MRSIDDECODE)
fi

##### pbs / torque #####

AC_ARG_WITH(pbs-config,
[  --with-pbs-config=PBS_CONFIG    use pbs-config specified ],
[
  PBS_CONFIG=$withval;
  echo "using "$PBS_CONFIG" for pbs-config";
],[
  PROG="pbs-config";
  AC_PATH_PROG(PBS_CONFIG, $PROG, "", $PATH)
])

if test "x$PBS_CONFIG" != "x"; then
  PBS_PREFIX=`$PBS_CONFIG --prefix`
  PBS_BINDIR="$PBS_PREFIX/bin"
  AC_SUBST(PBS_BINDIR)
  found_pbs="yes"
else
  found_pbs="no"
fi
AC_SUBST(HAVE_PBS)
AM_CONDITIONAL(HAVE_PBS, test "x$PBS_CONFIG" != "x")

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

##### lftp #####

AC_CHECK_PROGS([LFTP], [lftp], [:])
if test "$" = :; then
  AC_MSG_ERROR([lftp not found])
fi
AC_MSG_RESULT([lftp $LFTP])

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
  HAVE_OTB="true"
  AC_SUBST(OTBINC)
  AC_SUBST(OTBLIB)
  AC_SUBST(HAVE_OTB)
  
else
  AC_MSG_RESULT([cannot find OTBConfig.cmake try setting --with-otb-lib=/some/path])
fi
AM_CONDITIONAL(HAVE_OTB, test "$HAVE_OTB" = "true")
  
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

AC_CHECK_HEADERS([tiffio.h],
    [
        HAVE_TIFFIO_H=1
        HAVE_HDF="true"
    ],
    [
        AC_MSG_RESULT(libtiff is required for hdf support.)
        unset HAVE_HDF
    ])
AC_CHECK_LIB([tiff],[TIFFGetVersion],
    [
        HAVE_LIBTIFF=1
        HAVE_HDF="true"
    ],
    [
        AC_MSG_RESULT(libtiff is required for hdf support.)
        unset HAVE_HDF
        
    ])

AC_SUBST(TIFFINC)
AC_SUBST(TIFFLIB)

##### geotiff #####

if test "$HAVE_HDF" = "true"
then
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

    AC_CHECK_HEADERS([geotiff.h],
        [
            HAVE_GEOTIFFIO_H=1
            HAVE_HDF="true"
        ],
        [
            AC_MSG_RESULT(libgeotiff is required for hdf support.)
            unset HAVE_HDF
        ])
    AC_CHECK_LIB([geotiff],[GTIFNew],
        [
            HAVE_LIBGEOTIFF=1
            HAVE_HDF="true"
        ],
        [
            AC_MSG_RESULT(libgeotiff is required for hdf support.)
            unset HAVE_HDF
        ])

    AC_SUBST(GEOTIFFINC)
    AC_SUBST(GEOTIFFLIB)
fi

    ##### jpeg #####

if test "$HAVE_HDF" = "true"
then

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

    AC_CHECK_HEADERS([jpeglib.h],
        [
            HAVE_JPEG_H=1
            HAVE_HDF="true"
        ],
        [
            AC_MSG_RESULT(jpeg is required for hdf support.)
            unset HAVE_HDF
        ])
    AC_CHECK_LIB([jpeg],[jpeg_set_defaults],
        [
            HAVE_JPEG=1
            HAVE_HDF="true"
        ],
        [
            AC_MSG_RESULT(jpeg is required for hdf support.)
            unset HAVE_HDF
        ])

    AC_SUBST(JPEGINC)
    AC_SUBST(JPEGLIB)
fi

    ##### szip #####

if test "$HAVE_HDF" = "true"
then

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

    AC_CHECK_HEADERS([szlib.h],
        [
            HAVE_SZLIB_H=1
            HAVE_HDF="true"
        ],
        [
            AC_MSG_RESULT(szip is required for hdf support.)
            unset HAVE_HDF
        ])
    AC_CHECK_LIB([sz],[SZ_Decompress],
        [
            HAVE_SZ=1
            HAVE_HDF="true"
        ],
        [
            AC_MSG_RESULT(szip is required for hdf support.)
            unset HAVE_HDF
        ])

    AC_SUBST(SZINC)
    AC_SUBST(SZLIB)
fi

    ##### zlib #####

if test "$HAVE_HDF" = "true"
then

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

    AC_CHECK_HEADERS([szlib.h],
        [
            HAVE_SZLIB_H=1
            HAVE_HDF="true"
        ],
        [
            AC_MSG_RESULT(zlib is required for hdf support.)
            unset HAVE_HDF
        ])
    AC_CHECK_LIB([z],[SZ_Decompress],
        [
            HAVE_SZ=1
            HAVE_HDF="true"
        ],
        [
            AC_MSG_RESULT(zlib is required for hdf support.)
            unset HAVE_HDF
        ])

    AC_SUBST(ZINC)
    AC_SUBST(ZLIB)
fi

    ##### hdf #####

if test "$HAVE_HDF" = "true"
then
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
        HDFLIB="-L${withval} -lmfhdf -ldf"
        LDFLAGS="$LDFLAGS -L${withval} -lmfhdf -ldf"
      ],
      [
        HDFLIB="-lmfhdf -ldf"
        LDFLAGS="$LDFLAGS -lmfhdf -ldf"
      ]
    )

    AC_CHECK_HEADERS([mfhdf.h],
        [
            HAVE_MFHDF_H=1
            HAVE_HDF="true"
        ],
        [
            AC_MSG_RESULT(hdf4 is required for hdf support.)
            unset HAVE_HDF
        ])
    AC_CHECK_HEADERS([df.h],
        [
            HAVE_DF_H=1
            HAVE_HDF="true"
        ],
        [
            AC_MSG_RESULT(hdf4 is required for hdf support.)
            unset HAVE_HDF
        ])
    AC_CHECK_LIB([df],[DFopen],
        [
            HAVE_DF=1
            HAVE_HDF="true"
        ],
        [
            AC_MSG_RESULT(hdf4 is required for hdf support.)
            unset HAVE_HDF
        ])
    AC_CHECK_LIB([mfhdf -ldf],[SDstart],
        [
            HAVE_MFHDF=1
            HAVE_HDF="true"
        ],
        [
            AC_MSG_RESULT(hdf4 is required for hdf support.)
            unset HAVE_HDF
        ])

    AC_SUBST(HDFINC)
    AC_SUBST(HDFLIB)
fi

    ##### hdfeos #####

if test "$HAVE_HDF" = "true"
then
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

    AC_CHECK_HEADERS([HDFEOSVersion.h],
        [
            HAVE_HdfEosDef_H=1
            HAVE_HDF="true"
        ],
        [
            AC_MSG_RESULT(hdfeos is required for hdf support.)
            unset HAVE_HDF
        ])
    AC_CHECK_LIB([hdfeos -lmfhdf -ldf],[SWopen],
        [
            HAVE_LIBHdfEos=1
            HAVE_HDF="true"
        ],
        [
            AC_MSG_RESULT(hdfeos is required for hdf support.)
            unset HAVE_HDF
        ])
    

    AC_SUBST(HDFEOSINC)
    AC_SUBST(HDFEOSLIB)
fi

AC_SUBST(HAVE_HDF)
AM_CONDITIONAL(HAVE_HDF, test "$HAVE_HDF" = "true")

CFLAGS="$cflagsold"
LDFLAGS="$ldflagsold"

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
MOD_PRDS/Makefile
MOD_PRDS/data/Makefile
crefl/Makefile
crefl/data/Makefile
js/Makefile
img/Makefile
])
