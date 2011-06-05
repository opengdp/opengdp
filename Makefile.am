

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
	NEWS

dwhhtmlsearchdir = ${prefix}/doc/dwh/html/search
dwhhtmldir = ${prefix}/doc/dwh/html

dwhhtmlsearch_DATA = \
	doc/html/search/*.*

dwhhtml_DATA = \
	doc/html/*.*

EXTRA_DIST = $(dwhdoc_DATA) \
    $(dwhhtml_DATA) \
    $(dwhhtmlsearch_DATA)

if MAINTAINER_MODE

CLEANFILES = \
	doc/html/search/*.* \
	doc/html/*.*

doc/html/*.*: Doxyfile Makefile
	doxygen

doc/html/search/*.*: Doxyfile Makefile
	doxygen

endif

# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(dwhdocdir)
