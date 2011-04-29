cgibin_DATA = \
    default.cgi

CLEANFILES = $(cgibin_DATA)

cgibindir = $(pkgdatadir)/cgibin

EXTRA_DIST = \
        default.cgi.in


do_subst = cat
							 
default.cgi: default.cgi.in Makefile
	$(do_subst) < $(srcdir)/default.cgi.in > default.cgi

