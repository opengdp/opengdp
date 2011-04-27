au_DATA = \
    ASTER_321.bash \
    ASTER_321_L1B.bash \
    ASTER_321_L1BE.bash \
    EO1_2010_ALI_543.bash \
    LANDSAT_ETM+_USGS.bash \
    LANDSAT_ETM+_USGS_543.bash \
    LANDSAT_ETM+_USGS_753.bash \
    LANDSAT_TM_USGS.bash \
    LANDSAT_TM_USGS_543.bash \
    LANDSAT_TM_USGS_753.bash \
    dg.bash \
    Modis_Aqua_721.bash \
    Modis_Terra_721.bash \
    floodobservatory_colorado_edu.bash \
    geoeye.bash \
    iss.bash

CLEANFILES = $(au_DATA)

audir = $(pkgdatadir)/examples/floods_austrailia

EXTRA_DIST = \
    ASTER_321.bash.in \
    ASTER_321_L1B.bash.in \
    ASTER_321_L1BE.bash.in \
    EO1_2010_ALI_543.bash.in \
    LANDSAT_ETM+_USGS.bash.in \
    LANDSAT_ETM+_USGS_543.bash.in \
    LANDSAT_ETM+_USGS_753.bash.in \
    LANDSAT_TM_USGS.bash.in \
    LANDSAT_TM_USGS_543.bash.in \
    LANDSAT_TM_USGS_753.bash.in \
    dg.bash.in \
    Modis_Aqua_721.bash.in \
    Modis_Terra_721.bash.in \
    floodobservatory_colorado_edu.bash.in \
    geoeye.bash.in \
    iss.bash.in

do_subst = sed -e 's,[@]genericdir[@],$(pkgdatadir)/generic,g' \
							 -e 's,[@]wwwdisk[@],$(wwwdisk),g' \
							 -e 's,[@]www[@],$(www),g'

ASTER_321.bash: ASTER_321.bash.in Makefile
	$(do_subst) < $(srcdir)/ASTER_321.bash.in > ASTER_321.bash

ASTER_321_L1B.bash: ASTER_321_L1B.bash.in Makefile
	$(do_subst) < $(srcdir)/ASTER_321_L1B.bash.in > ASTER_321_L1B.bash

ASTER_321_L1BE.bash: ASTER_321_L1BE.bash.in Makefile
	$(do_subst) < $(srcdir)/ASTER_321_L1BE.bash.in > ASTER_321_L1BE.bash

EO1_2010_ALI_543.bash: EO1_2010_ALI_543.bash.in Makefile
	$(do_subst) < $(srcdir)/EO1_2010_ALI_543.bash.in > EO1_2010_ALI_543.bash

LANDSAT_ETM+_USGS.bash: LANDSAT_ETM+_USGS.bash.in Makefile
	$(do_subst) < $(srcdir)/LANDSAT_ETM+_USGS.bash.in > LANDSAT_ETM+_USGS.bash

LANDSAT_ETM+_USGS_543.bash: LANDSAT_ETM+_USGS_543.bash.in Makefile
	$(do_subst) < $(srcdir)/LANDSAT_ETM+_USGS_543.bash.in > LANDSAT_ETM+_USGS_543.bash

LANDSAT_ETM+_USGS_753.bash: LANDSAT_ETM+_USGS_753.bash.in Makefile
	$(do_subst) < $(srcdir)/LANDSAT_ETM+_USGS_753.bash.in > LANDSAT_ETM+_USGS_753.bash

LANDSAT_TM_USGS.bash: LANDSAT_TM_USGS.bash.in Makefile
	$(do_subst) < $(srcdir)/LANDSAT_TM_USGS.bash.in > LANDSAT_TM_USGS.bash

LANDSAT_TM_USGS_543.bash: LANDSAT_TM_USGS_543.bash.in Makefile
	$(do_subst) < $(srcdir)/LANDSAT_TM_USGS_543.bash.in > LANDSAT_TM_USGS_543.bash

LANDSAT_TM_USGS_753.bash: LANDSAT_TM_USGS_753.bash.in Makefile
	$(do_subst) < $(srcdir)/LANDSAT_TM_USGS_753.bash.in > LANDSAT_TM_USGS_753.bash

dg.bash: dg.bash.in Makefile
	$(do_subst) < $(srcdir)/dg.bash.in > dg.bash

Modis_Aqua_721.bash: Modis_Aqua_721.bash.in Makefile
	$(do_subst) < $(srcdir)/Modis_Aqua_721.bash.in > Modis_Aqua_721.bash

Modis_Terra_721.bash: Modis_Terra_721.bash.in Makefile
	$(do_subst) < $(srcdir)/Modis_Terra_721.bash.in > Modis_Terra_721.bash

floodobservatory_colorado_edu.bash: floodobservatory_colorado_edu.bash.in Makefile
	$(do_subst) < $(srcdir)/floodobservatory_colorado_edu.bash.in > floodobservatory_colorado_edu.bash

geoeye.bash: geoeye.bash.in Makefile
	$(do_subst) < $(srcdir)/geoeye.bash.in > geoeye.bash

iss.bash: iss.bash.in Makefile
	$(do_subst) < $(srcdir)/iss.bash.in > iss.bash

