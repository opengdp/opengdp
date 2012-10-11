

SUBDIRS = src \
	share\
	scripts\
	MRTSwath \
	crefl \
	MOD_PRDS \
	js \
	img

opengdpdocdir = ${prefix}/doc/opengdp
opengdpdoc_DATA = \
	README\
	COPYING\
	AUTHORS\
	ChangeLog\
	INSTALL\
	NEWS

opengdphtmlsearchdir = ${prefix}/doc/opengdp/html/search
opengdphtmldir = ${prefix}/doc/opengdp/html

opengdphtmlsearch_DATA = \
	doc/html/search/*.*

opengdphtml_DATA = \
	doc/html/*.*

EXTRA_DIST = $(opengdpdoc_DATA) \
    $(opengdphtml_DATA) \
    $(opengdphtmlsearch_DATA)

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
	-rm -r $(opengdpdocdir)
