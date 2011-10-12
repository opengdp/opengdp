if HAVE_HDF

EXTRA_DIST = \
	interp.h      \
	modis_edf_destripe.h \
	HISTORY.txt
	README.txt
	
bin_PROGRAMS = \
	modis_destripe

modis_destripe_SOURCES = \
	modis_destripe.c      \
	interp.c      \
	modis_edf_destripe.c
   
modis_destripe_LDFLAGS = \
	@HDFLIB@ @JPEGLIB@ @ZLIB@ @SZLIB@ 

modis_destripe_CFLAGS = \
    -DH4_HAVE_NETCDF -DHAVE_INT8 \
    @HDFINC@ @JPEGINC@ @ZINC@ @SZINC@ \
    -DMOD_PRDS_DATA_DIR=\"$(pkgdatadir)/MOD_PRDS\"

SUBDIRS = \
	data

endif