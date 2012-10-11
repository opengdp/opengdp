
wwwdisk = @WWWDISK@

js_DATA = \
    base.js \
    finish.js \
    osm.js \
    setup.js \
    kml.js \
    OpenLayers.js \
    GeoExt.js
    
CLEANFILES = $(js_DATA)

jsdir = $(wwwdisk)/opengdp

EXTRA_DIST = \
    base.js.in \
    finish.js.in \
    osm.js.in \
    setup.js.in \
    kml.js.in \
    OpenLayers.js.in \
    GeoExt.js.in

do_subst = cat

base.js: base.js.in Makefile
	$(do_subst) < $(srcdir)/base.js.in > base.js

finish.js: finish.js.in Makefile
	$(do_subst) < $(srcdir)/finish.js.in > finish.js

osm.js: osm.js.in Makefile
	$(do_subst) < $(srcdir)/osm.js.in > osm.js

setup.js: setup.js.in Makefile
	$(do_subst) < $(srcdir)/setup.js.in > setup.js

kml.js: kml.js.in Makefile
	$(do_subst) < $(srcdir)/kml.js.in > kml.js

OpenLayers.js: OpenLayers.js.in Makefile
	$(do_subst) < $(srcdir)/OpenLayers.js.in > OpenLayers.js

GeoExt.js: GeoExt.js.in Makefile
	$(do_subst) < $(srcdir)/GeoExt.js.in > GeoExt.js

