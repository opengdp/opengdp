scripts_DATA = \
    dwh-proj_def.bash

CLEANFILES = $(scripts_DATA)

scriptsdir = $(pkgdatadir)/generic

EXTRA_DIST = \
        dwh-proj_def.bash.in


do_subst = cat
							 
dwh-proj_def.bash: dwh-proj_def.bash.in Makefile
	$(do_subst) < $(srcdir)/dwh-proj_def.bash.in > dwh-proj_def.bash