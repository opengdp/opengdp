LIBS = \
	@GDAL_LIBS@

if HAVE_OTB

INCLUDES = \
	@GDAL_CFLAGS@



AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\"
	
    	
AM_CFLAGS =\
	 -Wall\
	 -g

bin_PROGRAMS = \
	otbPanSharp \
	otbCloudMask \
	otbCalc

otbPanSharp_SOURCES = \
	otbPanSharp.cpp

otbPanSharp_CPPFLAGS = @OTBINC@
otbPanSharp_LDFLAGS = @OTBLIB@ 

otbPanSharp_LDADD =

otbCloudMask_SOURCES = \
	otbCloudMask.cpp

otbCloudMask_CPPFLAGS = @OTBINC@
otbCloudMask_LDFLAGS = @OTBLIB@ 

otbCloudMask_LDADD =

otbCalc_SOURCES = \
	otbCalc.cpp

otbCalc_CPPFLAGS = @OTBINC@
otbCalc_LDFLAGS = @OTBLIB@ 

otbCalc_LDADD =

endif
