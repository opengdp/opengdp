jp_DATA = \
    ASTER_231_L1B.bash \
    ASTER_231_L1BE.bash \
    ASTER_321_L1B.bash \
    ASTER_321_L1BE.bash \
    DEIMOS1.bash \
    EO1_2010_ALI_543.bash \
    FORMOSAT2.bash \
    LANDSAT_ETM+_USGS.bash \
    LANDSAT_TM_USGS.bash \
    Modis_Aqua.bash \
    Modis_Aqua_721.bash \
    Modis_Terra.bash \
    Modis_Terra_721.bash \
    dg.bash \
    digitalglobe_ge_outreach.bash \
    geoeye_ge_outreach.bash \
    gsi.bash \
    jsitopo.bash \
    panformosat.bash

CLEANFILES = $(jp_DATA)

jpdir = $(pkgdatadir)/jp_earthquake

EXTRA_DIST = \
    ASTER_231_L1B.bash.in \
    ASTER_231_L1BE.bash.in \
    ASTER_321_L1B.bash.in \
    ASTER_321_L1BE.bash.in \
    DEIMOS1.bash.in \
    EO1_2010_ALI_543.bash.in \
    FORMOSAT2.bash.in \
    LANDSAT_ETM+_USGS.bash.in \
    LANDSAT_TM_USGS.bash.in \
    Modis_Aqua.bash.in \
    Modis_Aqua_721.bash.in \
    Modis_Terra.bash.in \
    Modis_Terra_721.bash.in \
    dg.bash.in \
    digitalglobe_ge_outreach.bash.in \
    geoeye_ge_outreach.bash.in \
    gsi.bash.in \
    jsitopo.bash.in \
    panformosat.bash.in

do_subst = sed -e 's,[@]genericdir[@],$(pkgdatadir)/generic,g' \
							 -e 's,[@]wwwdisk[@],$(wwwdisk),g' \
							 -e 's,[@]www[@],$(www),g'

ASTER_231_L1B.bash: ASTER_231_L1B.bash.in Makefile
	$(do_subst) < $(srcdir)/ASTER_231_L1B.bash.in > ASTER_231_L1B.bash

ASTER_231_L1BE.bash: ASTER_231_L1BE.bash.in Makefile
	$(do_subst) < $(srcdir)/ASTER_231_L1BE.bash.in > ASTER_231_L1BE.bash

ASTER_321_L1B.bash: ASTER_321_L1B.bash.in Makefile
	$(do_subst) < $(srcdir)/ASTER_321_L1B.bash.in > ASTER_321_L1B.bash

ASTER_321_L1BE.bash: ASTER_321_L1BE.bash.in Makefile
	$(do_subst) < $(srcdir)/ASTER_321_L1BE.bash.in > ASTER_321_L1BE.bash

DEIMOS1.bash: DEIMOS1.bash.in Makefile
	$(do_subst) < $(srcdir)/DEIMOS1.bash.in > DEIMOS1.bash

EO1_2010_ALI_543.bash: EO1_2010_ALI_543.bash.in Makefile
	$(do_subst) < $(srcdir)/EO1_2010_ALI_543.bash.in > EO1_2010_ALI_543.bash

FORMOSAT2.bash: FORMOSAT2.bash.in Makefile
	$(do_subst) < $(srcdir)/FORMOSAT2.bash.in > FORMOSAT2.bash

LANDSAT_ETM+_USGS.bash: LANDSAT_ETM+_USGS.bash.in Makefile
	$(do_subst) < $(srcdir)/LANDSAT_ETM+_USGS.bash.in > LANDSAT_ETM+_USGS.bash

LANDSAT_TM_USGS.bash: LANDSAT_TM_USGS.bash.in Makefile
	$(do_subst) < $(srcdir)/LANDSAT_TM_USGS.bash.in > LANDSAT_TM_USGS.bash

Modis_Aqua.bash: Modis_Aqua.bash.in Makefile
	$(do_subst) < $(srcdir)/Modis_Aqua.bash.in > Modis_Aqua.bash

Modis_Aqua_721.bash: Modis_Aqua_721.bash.in Makefile
	$(do_subst) < $(srcdir)/Modis_Aqua_721.bash.in > Modis_Aqua_721.bash

Modis_Terra.bash: Modis_Terra.bash.in Makefile
	$(do_subst) < $(srcdir)/Modis_Terra.bash.in > Modis_Terra.bash

Modis_Terra_721.bash: Modis_Terra_721.bash.in Makefile
	$(do_subst) < $(srcdir)/Modis_Terra_721.bash.in > Modis_Terra_721.bash

dg.bash: dg.bash.in Makefile
	$(do_subst) < $(srcdir)/dg.bash.in > dg.bash

digitalglobe_ge_outreach.bash: digitalglobe_ge_outreach.bash.in Makefile
	$(do_subst) < $(srcdir)/digitalglobe_ge_outreach.bash.in > digitalglobe_ge_outreach.bash

geoeye_ge_outreach.bash: geoeye_ge_outreach.bash.in Makefile
	$(do_subst) < $(srcdir)/geoeye_ge_outreach.bash.in > geoeye_ge_outreach.bash

gsi.bash: gsi.bash.in Makefile
	$(do_subst) < $(srcdir)/gsi.bash.in > gsi.bash

jsitopo.bash: jsitopo.bash.in Makefile
	$(do_subst) < $(srcdir)/jsitopo.bash.in > jsitopo.bash

panformosat.bash: panformosat.bash.in Makefile
	$(do_subst) < $(srcdir)/panformosat.bash.in > panformosat.bash

