map_DATA = \
    default.map

CLEANFILES = $(map_DATA)

mapdir = $(pkgdatadir)/generic

EXTRA_DIST = \
    default.map.in

do_subst = cat
							 
default.map: default.map.in Makefile
	$(do_subst) < $(srcdir)/default.map.in > default.map