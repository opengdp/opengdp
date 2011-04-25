
haiti_DATA = \
    AERIAL_CAP.bash \
    AERIAL_CAP_HP.bash \
    AERIAL_CAP_IR.bash \
    AERIAL_GOOGLE_17.bash \
    AERIAL_GOOGLE_21.bash \
    AERIAL_MICROSOFT.bash \
    AERIAL_NASA.bash \
    AERIAL_NASA_AVIRIS.bash \
    AERIAL_NASA_UAVSAR.bash \
    AERIAL_NOAA.bash \
    AERIAL_NOAA_MOSAIC.bash \
    AERIAL_NOAA_ORTHO.bash \
    AERIAL_USACE.bash \
    AERIAL_WorldBank_ImageCat_RIT.bash \
    ASTER.bash \
    ASTER_321.bash \
    E01.bash \
    LANDSAT_ETM+_USGS.bash \
    LANDSAT_TM_USGS.bash \
    Quickbird.bash \
    Worldview-2.bash \
    google-Ikonos.bash \
    google-geoeye.bash

CLEANFILES = $(haiti_DATA)

haitidir = $(pkgdatadir)/examples/haiti

EXTRA_DIST = \
    AERIAL_CAP.bash.in \
    AERIAL_CAP_HP.bash.in \
    AERIAL_CAP_IR.bash.in \
    AERIAL_GOOGLE_17.bash.in \
    AERIAL_GOOGLE_21.bash.in \
    AERIAL_MICROSOFT.bash.in \
    AERIAL_NASA.bash.in \
    AERIAL_NASA_AVIRIS.bash.in \
    AERIAL_NASA_UAVSAR.bash.in \
    AERIAL_NOAA.bash.in \
    AERIAL_NOAA_MOSAIC.bash.in \
    AERIAL_NOAA_ORTHO.bash.in \
    AERIAL_USACE.bash.in \
    AERIAL_WorldBank_ImageCat_RIT.bash.in \
    ASTER.bash.in \
    ASTER_321.bash.in \
    E01.bash.in \
    LANDSAT_ETM+_USGS.bash.in \
    LANDSAT_TM_USGS.bash.in \
    Quickbird.bash.in \
    Worldview-2.bash.in \
    google-Ikonos.bash.in \
    google-geoeye.bash.in

do_subst = sed -e 's,[@]genericdir[@],$(pkgdatadir)/generic,g' \
							 -e 's,[@]wwwdisk[@],$(wwwdisk),g' \
							 -e 's,[@]www[@],$(www),g'

AERIAL_CAP.bash: AERIAL_CAP.bash.in Makefile
	$(do_subst) < $(srcdir)/AERIAL_CAP.bash.in > AERIAL_CAP.bash

AERIAL_CAP_HP.bash: AERIAL_CAP_HP.bash.in Makefile
	$(do_subst) < $(srcdir)/AERIAL_CAP_HP.bash.in > AERIAL_CAP_HP.bash

AERIAL_CAP_IR.bash: AERIAL_CAP_IR.bash.in Makefile
	$(do_subst) < $(srcdir)/AERIAL_CAP_IR.bash.in > AERIAL_CAP_IR.bash

AERIAL_GOOGLE_17.bash: AERIAL_GOOGLE_17.bash.in Makefile
	$(do_subst) < $(srcdir)/AERIAL_GOOGLE_17.bash.in > AERIAL_GOOGLE_17.bash

AERIAL_GOOGLE_21.bash: AERIAL_GOOGLE_21.bash.in Makefile
	$(do_subst) < $(srcdir)/AERIAL_GOOGLE_21.bash.in > AERIAL_GOOGLE_21.bash

AERIAL_MICROSOFT.bash: AERIAL_MICROSOFT.bash.in Makefile
	$(do_subst) < $(srcdir)/AERIAL_MICROSOFT.bash.in > AERIAL_MICROSOFT.bash

AERIAL_NASA.bash: AERIAL_NASA.bash.in Makefile
	$(do_subst) < $(srcdir)/AERIAL_NASA.bash.in > AERIAL_NASA.bash

AERIAL_NASA_AVIRIS.bash: AERIAL_NASA_AVIRIS.bash.in Makefile
	$(do_subst) < $(srcdir)/AERIAL_NASA_AVIRIS.bash.in > AERIAL_NASA_AVIRIS.bash

AERIAL_NASA_UAVSAR.bash: AERIAL_NASA_UAVSAR.bash.in Makefile
	$(do_subst) < $(srcdir)/AERIAL_NASA_UAVSAR.bash.in > AERIAL_NASA_UAVSAR.bash

AERIAL_NOAA.bash: AERIAL_NOAA.bash.in Makefile
	$(do_subst) < $(srcdir)/AERIAL_NOAA.bash.in > AERIAL_NOAA.bash

AERIAL_NOAA_MOSAIC.bash: AERIAL_NOAA_MOSAIC.bash.in Makefile
	$(do_subst) < $(srcdir)/AERIAL_NOAA_MOSAIC.bash.in > AERIAL_NOAA_MOSAIC.bash

AERIAL_NOAA_ORTHO.bash: AERIAL_NOAA_ORTHO.bash.in Makefile
	$(do_subst) < $(srcdir)/AERIAL_NOAA_ORTHO.bash.in > AERIAL_NOAA_ORTHO.bash

AERIAL_USACE.bash: AERIAL_USACE.bash.in Makefile
	$(do_subst) < $(srcdir)/AERIAL_USACE.bash.in > AERIAL_USACE.bash

AERIAL_WorldBank_ImageCat_RIT.bash: AERIAL_WorldBank_ImageCat_RIT.bash.in Makefile
	$(do_subst) < $(srcdir)/AERIAL_WorldBank_ImageCat_RIT.bash.in > AERIAL_WorldBank_ImageCat_RIT.bash

ASTER.bash: ASTER.bash.in Makefile
	$(do_subst) < $(srcdir)/ASTER.bash.in > ASTER.bash

ASTER_321.bash: ASTER_321.bash.in Makefile
	$(do_subst) < $(srcdir)/ASTER_321.bash.in > ASTER_321.bash

E01.bash: E01.bash.in Makefile
	$(do_subst) < $(srcdir)/E01.bash.in > E01.bash

LANDSAT_ETM+_USGS.bash: LANDSAT_ETM+_USGS.bash.in Makefile
	$(do_subst) < $(srcdir)/LANDSAT_ETM+_USGS.bash.in > LANDSAT_ETM+_USGS.bash

LANDSAT_TM_USGS.bash: LANDSAT_TM_USGS.bash.in Makefile
	$(do_subst) < $(srcdir)/LANDSAT_TM_USGS.bash.in > LANDSAT_TM_USGS.bash

Quickbird.bash: Quickbird.bash.in Makefile
	$(do_subst) < $(srcdir)/Quickbird.bash.in > Quickbird.bash

Worldview-2.bash: Worldview-2.bash.in Makefile
	$(do_subst) < $(srcdir)/Worldview-2.bash.in > Worldview-2.bash

google-Ikonos.bash: google-Ikonos.bash.in Makefile
	$(do_subst) < $(srcdir)/google-Ikonos.bash.in > google-Ikonos.bash

google-geoeye.bash: google-geoeye.bash.in Makefile
	$(do_subst) < $(srcdir)/google-geoeye.bash.in > google-geoeye.bash


