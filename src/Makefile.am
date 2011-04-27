INCLUDES = \
	@GDAL_CFLAGS@

LIBS = \
	@GDAL_LIBS@

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\"
	
    	
AM_CFLAGS =\
	 -Wall\
	 -g

bin_PROGRAMS = \
	otbPanSharp

otbPanSharp_SOURCES = \
	otbPanSharp.cpp

otbPanSharp_CPPFLAGS = @OTBINC@
otbPanSharp_LDFLAGS = @OTBLIB@ 

otbPanSharp_LDADD =

