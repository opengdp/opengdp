## Process this file with automake to produce Makefile.in
## Created by Anjuta

SUBDIRS = src \
	share\
	scripts\
	gctpc\
	MRTSwath

dwhdocdir = ${prefix}/doc/dwh
dwhdoc_DATA = \
	README\
	COPYING\
	AUTHORS\
	ChangeLog\
	INSTALL\
	NEWS


EXTRA_DIST = $(dwhdoc_DATA)


# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(dwhdocdir)
