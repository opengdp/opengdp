cgibin_DATA = \
    default.cgi \
    kmlrepeater.cgi

CLEANFILES = $(cgibin_DATA)

cgibindir = $(pkgdatadir)/cgibin

EXTRA_DIST = \
    default.cgi.in \
    kmlrepeater.cgi.in


do_subst = cat
							 
default.cgi: default.cgi.in Makefile
	$(do_subst) < $(srcdir)/default.cgi.in > default.cgi
	
kmlrepeater.cgi: kmlrepeater.cgi.in Makefile
	$(do_subst) < $(srcdir)/kmlrepeater.cgi.in > kmlrepeater.cgi

