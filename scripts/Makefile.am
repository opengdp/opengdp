scripts_DATA = \
	dwh-mkproject.bash
    
CLEANFILES = $(scripts_DATA)

EXTRA_DIST = \
	dwh-mkproject.bash.in
	
scriptsdir = $(bindir)


do_subst = sed -e 's,[@]PKGDATADIR[@],$(pkgdatadir),g'
     
dwh-mkproject.bash: dwh-mkproject.bash.in Makefile
	$(do_subst) < $(srcdir)/dwh-mkproject.bash.in > dwh-mkproject.bash
	chmod +x dwh-mkproject.bash

