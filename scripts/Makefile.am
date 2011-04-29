bin_SCRIPTS = \
	dwh-mkproject.bash
    
CLEANFILES = $(bin_SCRIPTS)

EXTRA_DIST = \
	dwh-mkproject.bash.in
	



do_subst = sed -e 's,[@]PKGDATADIR[@],$(pkgdatadir),g'
     
dwh-mkproject.bash: dwh-mkproject.bash.in Makefile
	$(do_subst) < $(srcdir)/dwh-mkproject.bash.in > dwh-mkproject.bash
	chmod +x dwh-mkproject.bash

