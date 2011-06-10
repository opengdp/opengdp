bin_SCRIPTS = \
	dwh-mkproject.bash
    
CLEANFILES = $(bin_SCRIPTS)

EXTRA_DIST = \
	dwh-mkproject.bash.in
	



do_subst = sed -e 's,[@]PKGDATADIR[@],$(pkgdatadir),g' \
               -e 's,[@]HAVE_DIALOG[@],$(HAVE_DIALOG),g' \
               -e 's,[@]HAVE_MC[@],$(HAVE_MC),g'
     
dwh-mkproject.bash: dwh-mkproject.bash.in Makefile
	$(do_subst) < $(srcdir)/dwh-mkproject.bash.in > dwh-mkproject.bash
	chmod +x dwh-mkproject.bash

