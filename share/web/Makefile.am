web_DATA = \
    base.js \
    finish.js \
    index.html \
    osm.js \
    proj-def.js \
    setup.js \
    tilecache.conf \
    kml.js \
    index.kml
    
CLEANFILES = $(web_DATA)

webdir = $(pkgdatadir)/web

EXTRA_DIST = \
    base.js.in \
    finish.js.in \
    index.html.in \
    osm.js.in \
    proj-def.js.in \
    setup.js.in \
    tilecache.conf.in \
    kml.js.in \
    index.kml.in

do_subst = cat

base.js: base.js.in Makefile
	$(do_subst) < $(srcdir)/base.js.in > base.js

finish.js: finish.js.in Makefile
	$(do_subst) < $(srcdir)/finish.js.in > finish.js

index.html: index.html.in Makefile
	$(do_subst) < $(srcdir)/index.html.in > index.html

osm.js: osm.js.in Makefile
	$(do_subst) < $(srcdir)/osm.js.in > osm.js

proj-def.js: proj-def.js.in Makefile
	$(do_subst) < $(srcdir)/proj-def.js.in > proj-def.js

setup.js: setup.js.in Makefile
	$(do_subst) < $(srcdir)/setup.js.in > setup.js

tilecache.conf: tilecache.conf.in Makefile
	$(do_subst) < $(srcdir)/tilecache.conf.in > tilecache.conf

kml.js: kml.js.in Makefile
	$(do_subst) < $(srcdir)/kml.js.in > kml.js

index.kml: index.kml.in Makefile
	$(do_subst) < $(srcdir)/index.kml.in > index.kml

