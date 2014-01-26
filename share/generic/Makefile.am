generic_DATA = \
    generic.bash \
    addo.bash \
    file.bash \
    hdf.bash \
    index.bash \
    geoext.bash \
    image.bash \
    msimage.bash \
    vector.bash \
    mapfile.bash \
    misc.bash \
    ms_overview.bash \
    pan.bash \
    tc.bash \
    rapidfire_modis.bash \
    rapidfire_modis_othersubdataset.bash \
    newearthexplorer.bash \
    hdds.bash \
    mkkey.bash \
    google_superoverlay_kml.bash
        
CLEANFILES = $(generic_DATA)

genericdir = $(pkgdatadir)/generic

EXTRA_DIST = \
    addo.bash.in \
    file.bash.in \
    hdf.bash.in \
    generic.bash.in \
    index.bash.in \
    geoext.bash.in \
    image.bash.in \
    msimage.bash.in \
    vector.bash.in \
    mapfile.bash.in \
    misc.bash.in \
    ms_overview.bash.in \
    pan.bash.in \
    tc.bash.in \
    rapidfire_modis.bash.in \
    rapidfire_modis_othersubdataset.bash.in \
    newearthexplorer.bash.in \
    blend.bash.in \
    hdds.bash.in \
    mkkey.bash.in \
    google_superoverlay_kml.bash


do_subst = sed -e 's,[@]GENERICDIR[@],$(pkgdatadir)/generic,g' \
               -e 's,[@]HAVE_OTB[@],$(HAVE_OTB),g' \
               -e 's,[@]HAVE_MRSIDDECODE[@],$(HAVE_MRSIDDECODE),g' \
               -e 's,[@]HAVE_PBS[@],$(HAVE_PBS),g' \
               -e 's,[@]HAVE_HDF[@],$(HAVE_HDF),g'
               
generic.bash: generic.bash.in Makefile
	$(do_subst) < $(srcdir)/generic.bash.in > generic.bash
	
addo.bash: addo.bash.in Makefile
	$(do_subst) < $(srcdir)/addo.bash.in > addo.bash
	
file.bash: file.bash.in Makefile
	$(do_subst) < $(srcdir)/file.bash.in > file.bash
	
hdf.bash: hdf.bash.in Makefile
	$(do_subst) < $(srcdir)/hdf.bash.in > hdf.bash
	
geoext.bash: geoext.bash.in Makefile
	$(do_subst) < $(srcdir)/geoext.bash.in > geoext.bash
	
index.bash: index.bash.in Makefile
	$(do_subst) < $(srcdir)/index.bash.in > index.bash
	
image.bash: image.bash.in Makefile
	$(do_subst) < $(srcdir)/image.bash.in > image.bash
	
msimage.bash: msimage.bash.in Makefile
	$(do_subst) < $(srcdir)/msimage.bash.in > msimage.bash
	
vector.bash: vector.bash.in Makefile
	$(do_subst) < $(srcdir)/vector.bash.in > vector.bash
	
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
	
rapidfire_modis.bash: rapidfire_modis.bash.in Makefile
	$(do_subst) < $(srcdir)/rapidfire_modis.bash.in > rapidfire_modis.bash

rapidfire_modis_othersubdataset.bash: rapidfire_modis_othersubdataset.bash.in Makefile
	$(do_subst) < $(srcdir)/rapidfire_modis_othersubdataset.bash.in > rapidfire_modis_othersubdataset.bash

newearthexplorer.bash: newearthexplorer.bash.in Makefile
	$(do_subst) < $(srcdir)/newearthexplorer.bash.in > newearthexplorer.bash

blend.bash: blend.bash.in Makefile
	$(do_subst) < $(srcdir)/blend.bash.in > blend.bash

hdds.bash: hdds.bash.in Makefile
	$(do_subst) < $(srcdir)/hdds.bash.in > hdds.bash

mkkey.bash: mkkey.bash.in Makefile
	$(do_subst) < $(srcdir)/mkkey.bash.in > mkkey.bash

google_superoverlay_kml.bash: google_superoverlay_kml.bash.in Makefile
	$(do_subst) < $(srcdir)/google_superoverlay_kml.bash.in > google_superoverlay_kml.bash

## File created by the gnome-build tools


