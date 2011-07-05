if HAVE_HDF

EXTRA_DIST = \
	resamp.h param.h geoloc.h input.h scan.h output.h space.h kernel.h \
	patches.h myhdf.h mystring.h myerror.h bool.h range.h parser.h \
	myproj.h myproj_const.h usage.h const.h deg2dms.h GeoS2G.h addmeta.h \
	geowrpr.h myendian.h
        
bin_PROGRAMS = \
	swath2grid \
    dumpmeta

swath2grid_SOURCES = \
	param.c geoloc.c input.c scan.c output.c space.c kernel.c \
	patches.c myhdf.c mystring.c parser.c myerror.c \
	InitGeoTiff.c deg2dms.c degdms.c convert_corners.c metadata.c \
	geo_trans.c write_hdr.c write_rb.c addmeta.c logh.c geowrpr.c \
	filegeo.c myendian.c resamp.c

swath2grid_CFLAGS = \
    -DH4_HAVE_NETCDF -DHAVE_INT8 \
    -DMRTSWATH_DATA_DIR=\"$(pkgdatadir)/MRTSwath\" \
    -I ../gctpc \
    @HDFEOSINC@ @HDFINC@ @JPEGINC@ @ZINC@ @SZINC@ @TIFFINC@ @GEOTIFFINC@
  
swath2grid_LDFLAGS = -L../gctpc/ -lgctp \
    @HDFEOSLIB@ @HDFLIB@ @JPEGLIB@ @ZLIB@ @SZLIB@ @TIFFLIB@ @GEOTIFFLIB@ 

dumpmeta_SOURCES = \
	dumpmeta.c

dumpmeta_CFLAGS = \
    -DH4_HAVE_NETCDF -DHAVE_INT8 \
    -DMRTSWATH_DATA_DIR=\"$(pkgdatadir)/MRTSwath\" \
    -I ../gctpc \
    @HDFEOSINC@ @HDFINC@ @JPEGINC@ @ZINC@ @SZINC@ @TIFFINC@ @GEOTIFFINC@
  
dumpmeta_LDFLAGS = -L../gctpc/ -lgctp \
    @HDFEOSLIB@ @HDFLIB@ @JPEGLIB@ @ZLIB@ @SZLIB@ @TIFFLIB@ @GEOTIFFLIB@ 

SUBDIRS = \
	data

endif

