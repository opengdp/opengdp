## Process this file with automake to produce Makefile.in
## Created by Anjuta

SUBDIRS = src \
	share\
	scripts\
	gctpc\
	MRTSwath \
	crefl

dwhdocdir = ${prefix}/doc/dwh
dwhdoc_DATA = \
	README\
	COPYING\
	AUTHORS\
	ChangeLog\
	INSTALL\
	NEWS\
	doc/*
    

EXTRA_DIST = $(dwhdoc_DATA)



if MAINTAINER_MODE

CLEANFILES = doc/html*

doc/*: Doxyfile Makefile
	doxygen

endif

# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(dwhdocdir)
