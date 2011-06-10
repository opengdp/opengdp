if HAVE_HDF

bin_PROGRAMS = \
	crefl

crefl_SOURCES = \
	crefl.c

crefl_CFLAGS = \
    -DCREFL_DATA_DIR=\"$(pkgdatadir)/crefl\" \
    @HDFEOSINC@ @HDFINC@ @JPEGINC@ @ZINC@ @SZINC@
  
crefl_LDFLAGS = @HDFEOSLIB@ @HDFLIB@ @JPEGLIB@ @ZLIB@ @SZLIB@

SUBDIRS = \
	data

endif