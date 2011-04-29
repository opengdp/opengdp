scripts_DATA = \
	dwh-mkproject.bash
    
CLEANFILES = $(scripts_DATA)

EXTRA_DIST = \
	dwh-mkproject.bash.in
	
scriptsdir = $(bindir)


do_subst = sed -e 's,[@]PKGDATADIR[@],$(pkgdatadir),g'
     
dwh-mkproject.sh: dwh-mkproject.in Makefile
	$(do_subst) < $(srcdir)/dwh-mkproject.in > dwh-mkproject.sh
	chmod +x dwh-mkproject.sh

