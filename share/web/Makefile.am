web_DATA = \
    index.html \
    proj-def.js \
    tilecache.conf \
    kml.js \
    index.kml
    
CLEANFILES = $(web_DATA)

webdir = $(pkgdatadir)/web

EXTRA_DIST = \
    index.html.in \
    proj-def.js.in \
    tilecache.conf.in \
    kml.js.in \
    index.kml.in

do_subst = cat

index.html: index.html.in Makefile
	$(do_subst) < $(srcdir)/index.html.in > index.html

proj-def.js: proj-def.js.in Makefile
	$(do_subst) < $(srcdir)/proj-def.js.in > proj-def.js

tilecache.conf: tilecache.conf.in Makefile
	$(do_subst) < $(srcdir)/tilecache.conf.in > tilecache.conf

kml.js: kml.js.in Makefile
	$(do_subst) < $(srcdir)/kml.js.in > kml.js

index.kml: index.kml.in Makefile
	$(do_subst) < $(srcdir)/index.kml.in > index.kml

