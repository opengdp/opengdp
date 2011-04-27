generic_DATA = \
    generic.bash \
    addo.bash \
    file.bash \
    geoext.bash \
    image.bash \
    mapfile.bash \
    misc.bash \
    ms_overview.bash \
    pan.bash \
    tc.bash \
    LANDSAT_ETM+_USGS.bash \
    LANDSAT_TM_USGS.bash \
    ASTER_USGS.bash \
    rapidfire_modis.bash \
    EO1_ALI_PAN_USGS.bash \
    EO1_ALI_PAN_USGS.bash \
    google_superoverlay_kml.bash
        
CLEANFILES = $(generic_DATA)

genericdir = $(pkgdatadir)/generic

EXTRA_DIST = \
    addo.bash.in \
    file.bash.in \
    generic.bash.in \
    geoext.bash.in \
    image.bash.in \
    mapfile.bash.in \
    misc.bash.in \
    ms_overview.bash.in \
    pan.bash.in \
    tc.bash.in \
    LANDSAT_ETM+_USGS.bash.in \
    LANDSAT_TM_USGS.bash.in \
    ASTER_USGS.bash.in \
    rapidfire_modis.bash.in \
    EO1_ALI_PAN_USGS.bash.in \
    google_superoverlay_kml.bash


do_subst = sed -e 's,[@]genericdir[@],$(pkgdatadir)/generic,g' \
							 -e 's,[@]wwwdisk[@],$(wwwdisk),g' \
							 -e 's,[@]www[@],$(www),g'
							 
generic.bash: generic.bash.in Makefile
	$(do_subst) < $(srcdir)/generic.bash.in > generic.bash
	
addo.bash: addo.bash.in Makefile
	$(do_subst) < $(srcdir)/addo.bash.in > addo.bash
	
file.bash: file.bash.in Makefile
	$(do_subst) < $(srcdir)/file.bash.in > file.bash
	
geoext.bash: geoext.bash.in Makefile
	$(do_subst) < $(srcdir)/geoext.bash.in > geoext.bash
	
image.bash: image.bash.in Makefile
	$(do_subst) < $(srcdir)/image.bash.in > image.bash
	
mapfile.bash: mapfile.bash.in Makefile
	$(do_subst) < $(srcdir)/mapfile.bash.in > mapfile.bash
	
misc.bash: misc.bash.in Makefile
	$(do_subst) < $(srcdir)/misc.bash.in > misc.bash
	
ms_overview.bash: ms_overview.bash.in Makefile
	$(do_subst) < $(srcdir)/ms_overview.bash.in > ms_overview.bash
	
pan.bash: pan.bash.in Makefile
	$(do_subst) < $(srcdir)/pan.bash.in > pan.bash
	
tc.bash: tc.bash.in Makefile
	$(do_subst) < $(srcdir)/tc.bash.in > tc.bash
	
LANDSAT_ETM+_USGS.bash: LANDSAT_ETM+_USGS.bash.in Makefile
	$(do_subst) < $(srcdir)/LANDSAT_ETM+_USGS.bash.in > LANDSAT_ETM+_USGS.bash
	
LANDSAT_TM_USGS.bash: LANDSAT_TM_USGS.bash.in Makefile
	$(do_subst) < $(srcdir)/LANDSAT_TM_USGS.bash.in > LANDSAT_TM_USGS.bash

ASTER_USGS.bash: ASTER_USGS.bash.in Makefile
	$(do_subst) < $(srcdir)/ASTER_USGS.bash.in > ASTER_USGS.bash

rapidfire_modis.bash: rapidfire_modis.bash.in Makefile
	$(do_subst) < $(srcdir)/rapidfire_modis.bash.in > rapidfire_modis.bash

EO1_ALI_PAN_USGS.bash: EO1_ALI_PAN_USGS.bash.in Makefile
	$(do_subst) < $(srcdir)/EO1_ALI_PAN_USGS.bash.in > EO1_ALI_PAN_USGS.bash

google_superoverlay_kml.bash: google_superoverlay_kml.bash.in Makefile
	$(do_subst) < $(srcdir)/google_superoverlay_kml.bash.in > google_superoverlay_kml.bash

## File created by the gnome-build tools


