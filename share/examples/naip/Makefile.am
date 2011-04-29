naip_DATA = \
    BL_AL_200906_alabama_naip_al_1m.bash \
    BL_OR_200508_oregon_naip_or_1m.bash \
    arizona_naip_2007.bash \
    iowa_2009_naip.bash \
    nh_2009.bash \
    or_naip_2009.bash \
    BL_CT_2008_connecticut_naip_ct_1m.bash \
    BL_RI_200807_rhode_island_naip_ri_1m.bash \
    ca_naip_2009.bash \
    ky_naip_2008.bash \
    nm_2009.bash \
    penn_naip_2010.bash \
    BL_LA_200905_louisiana_naip_la_1m.bash \
    BL_VA_200906_virginia_naip_va_1m.bash \
    co_naip_2009.bash \
    md_2009.bash \
    north_dakota_2009.bash \
    sc_naip_2009.bash \
    BL_MA_200807_massachusetts_naip_ma_1m.bash \
    BL_VT_200908_vermont_naip_vt_1m.bash \
    de-2009.bash \
    mi_naip_2010.bash \
    nv_naip_2006.bash \
    south_dakota_2008.bash \
    BL_ME_200908_maine_naip_me_1m.bash \
    ID-2009.bash \
    fl_naip_2010.bash \
    mn_2010.bash \
    nv_naip_2010.bash \
    tn_naip_2010.bash \
    BL_MS_200906_mississippi_naip_ms_1m.bash \
    Kansas_2010.bash \
    ga_naip_2010.bash \
    montana_naip_2009.bash \
    ny_naip_2009.bash \
    tx_2010.bash \
    BL_NC_200906_north_carolina_naip_nc_1m.bash \
    Missouri-2009.bash \
    il_naip_2010.bash \
    naip2009ccm_wa.bash \
    oh_naip_2010.bash \
    BL_NJ_2006_new_jersey_naip_nj_1m.bash \
    ak_naip_2009.bash \
    in_2010.bash \
    nebr_naip_2010.bash \
    ok_2010.bash

CLEANFILES = $(naip_DATA)

naipdir = $(pkgdatadir)/examples/naip

EXTRA_DIST = \
    BL_AL_200906_alabama_naip_al_1m.bash.in \
    BL_OR_200508_oregon_naip_or_1m.bash.in \
    arizona_naip_2007.bash.in \
    iowa_2009_naip.bash.in \
    nh_2009.bash.in \
    or_naip_2009.bash.in \
    BL_CT_2008_connecticut_naip_ct_1m.bash.in \
    BL_RI_200807_rhode_island_naip_ri_1m.bash.in \
    ca_naip_2009.bash.in \
    ky_naip_2008.bash.in \
    nm_2009.bash.in \
    penn_naip_2010.bash.in \
    BL_LA_200905_louisiana_naip_la_1m.bash.in \
    BL_VA_200906_virginia_naip_va_1m.bash.in \
    co_naip_2009.bash.in \
    md_2009.bash.in \
    north_dakota_2009.bash.in \
    sc_naip_2009.bash.in \
    BL_MA_200807_massachusetts_naip_ma_1m.bash.in \
    BL_VT_200908_vermont_naip_vt_1m.bash.in \
    de-2009.bash.in \
    mi_naip_2010.bash.in \
    nv_naip_2006.bash.in \
    south_dakota_2008.bash.in \
    BL_ME_200908_maine_naip_me_1m.bash.in \
    ID-2009.bash.in \
    fl_naip_2010.bash.in \
    mn_2010.bash.in \
    nv_naip_2010.bash.in \
    tn_naip_2010.bash.in \
    BL_MS_200906_mississippi_naip_ms_1m.bash.in \
    Kansas_2010.bash.in \
    ga_naip_2010.bash.in \
    montana_naip_2009.bash.in \
    ny_naip_2009.bash.in \
    tx_2010.bash.in \
    BL_NC_200906_north_carolina_naip_nc_1m.bash.in \
    Missouri-2009.bash.in \
    il_naip_2010.bash.in \
    naip2009ccm_wa.bash.in \
    oh_naip_2010.bash.in \
    BL_NJ_2006_new_jersey_naip_nj_1m.bash.in \
    ak_naip_2009.bash.in \
    in_2010.bash.in \
    nebr_naip_2010.bash.in \
    ok_2010.bash.in
    
do_subst = sed -e 's,[@]GENERICDIR[@],$(pkgdatadir)/generic,g'

BL_AL_200906_alabama_naip_al_1m.bash: BL_AL_200906_alabama_naip_al_1m.bash.in Makefile
	$(do_subst) < $(srcdir)/BL_AL_200906_alabama_naip_al_1m.bash.in > BL_AL_200906_alabama_naip_al_1m.bash

BL_CT_2008_connecticut_naip_ct_1m.bash: BL_CT_2008_connecticut_naip_ct_1m.bash.in Makefile
	$(do_subst) < $(srcdir)/BL_CT_2008_connecticut_naip_ct_1m.bash.in > BL_CT_2008_connecticut_naip_ct_1m.bash

BL_LA_200905_louisiana_naip_la_1m.bash: BL_LA_200905_louisiana_naip_la_1m.bash.in Makefile
	$(do_subst) < $(srcdir)/BL_LA_200905_louisiana_naip_la_1m.bash.in > BL_LA_200905_louisiana_naip_la_1m.bash

BL_MA_200807_massachusetts_naip_ma_1m.bash: BL_MA_200807_massachusetts_naip_ma_1m.bash.in Makefile
	$(do_subst) < $(srcdir)/BL_MA_200807_massachusetts_naip_ma_1m.bash.in > BL_MA_200807_massachusetts_naip_ma_1m.bash

BL_ME_200908_maine_naip_me_1m.bash: BL_ME_200908_maine_naip_me_1m.bash.in Makefile
	$(do_subst) < $(srcdir)/BL_ME_200908_maine_naip_me_1m.bash.in > BL_ME_200908_maine_naip_me_1m.bash

BL_MS_200906_mississippi_naip_ms_1m.bash: BL_MS_200906_mississippi_naip_ms_1m.bash.in Makefile
	$(do_subst) < $(srcdir)/BL_MS_200906_mississippi_naip_ms_1m.bash.in > BL_MS_200906_mississippi_naip_ms_1m.bash

BL_NC_200906_north_carolina_naip_nc_1m.bash: BL_NC_200906_north_carolina_naip_nc_1m.bash.in Makefile
	$(do_subst) < $(srcdir)/BL_NC_200906_north_carolina_naip_nc_1m.bash.in > BL_NC_200906_north_carolina_naip_nc_1m.bash

BL_NJ_2006_new_jersey_naip_nj_1m.bash: BL_NJ_2006_new_jersey_naip_nj_1m.bash.in Makefile
	$(do_subst) < $(srcdir)/BL_NJ_2006_new_jersey_naip_nj_1m.bash.in > BL_NJ_2006_new_jersey_naip_nj_1m.bash

BL_OR_200508_oregon_naip_or_1m.bash: BL_OR_200508_oregon_naip_or_1m.bash.in Makefile
	$(do_subst) < $(srcdir)/BL_OR_200508_oregon_naip_or_1m.bash.in > BL_OR_200508_oregon_naip_or_1m.bash

BL_RI_200807_rhode_island_naip_ri_1m.bash: BL_RI_200807_rhode_island_naip_ri_1m.bash.in Makefile
	$(do_subst) < $(srcdir)/BL_RI_200807_rhode_island_naip_ri_1m.bash.in > BL_RI_200807_rhode_island_naip_ri_1m.bash

BL_VA_200906_virginia_naip_va_1m.bash: BL_VA_200906_virginia_naip_va_1m.bash.in Makefile
	$(do_subst) < $(srcdir)/BL_VA_200906_virginia_naip_va_1m.bash.in > BL_VA_200906_virginia_naip_va_1m.bash

BL_VT_200908_vermont_naip_vt_1m.bash: BL_VT_200908_vermont_naip_vt_1m.bash.in Makefile
	$(do_subst) < $(srcdir)/BL_VT_200908_vermont_naip_vt_1m.bash.in > BL_VT_200908_vermont_naip_vt_1m.bash

ID-2009.bash: ID-2009.bash.in Makefile
	$(do_subst) < $(srcdir)/ID-2009.bash.in > ID-2009.bash

Kansas_2010.bash: Kansas_2010.bash.in Makefile
	$(do_subst) < $(srcdir)/Kansas_2010.bash.in > Kansas_2010.bash

Missouri-2009.bash: Missouri-2009.bash.in Makefile
	$(do_subst) < $(srcdir)/Missouri-2009.bash.in > Missouri-2009.bash

ak_naip_2009.bash: ak_naip_2009.bash.in Makefile
	$(do_subst) < $(srcdir)/ak_naip_2009.bash.in > ak_naip_2009.bash

arizona_naip_2007.bash: arizona_naip_2007.bash.in Makefile
	$(do_subst) < $(srcdir)/arizona_naip_2007.bash.in > arizona_naip_2007.bash

ca_naip_2009.bash: ca_naip_2009.bash.in Makefile
	$(do_subst) < $(srcdir)/ca_naip_2009.bash.in > ca_naip_2009.bash

co_naip_2009.bash: co_naip_2009.bash.in Makefile
	$(do_subst) < $(srcdir)/co_naip_2009.bash.in > co_naip_2009.bash

de-2009.bash: de-2009.bash.in Makefile
	$(do_subst) < $(srcdir)/de-2009.bash.in > de-2009.bash

fl_naip_2010.bash: fl_naip_2010.bash.in Makefile
	$(do_subst) < $(srcdir)/fl_naip_2010.bash.in > fl_naip_2010.bash

ga_naip_2010.bash: ga_naip_2010.bash.in Makefile
	$(do_subst) < $(srcdir)/ga_naip_2010.bash.in > ga_naip_2010.bash

il_naip_2010.bash: il_naip_2010.bash.in Makefile
	$(do_subst) < $(srcdir)/il_naip_2010.bash.in > il_naip_2010.bash

in_2010.bash: in_2010.bash.in Makefile
	$(do_subst) < $(srcdir)/in_2010.bash.in > in_2010.bash

iowa_2009_naip.bash: iowa_2009_naip.bash.in Makefile
	$(do_subst) < $(srcdir)/iowa_2009_naip.bash.in > iowa_2009_naip.bash

ky_naip_2008.bash: ky_naip_2008.bash.in Makefile
	$(do_subst) < $(srcdir)/ky_naip_2008.bash.in > ky_naip_2008.bash

md_2009.bash: md_2009.bash.in Makefile
	$(do_subst) < $(srcdir)/md_2009.bash.in > md_2009.bash

mi_naip_2010.bash: mi_naip_2010.bash.in Makefile
	$(do_subst) < $(srcdir)/mi_naip_2010.bash.in > mi_naip_2010.bash

mn_2010.bash: mn_2010.bash.in Makefile
	$(do_subst) < $(srcdir)/mn_2010.bash.in > mn_2010.bash

montana_naip_2009.bash: montana_naip_2009.bash.in Makefile
	$(do_subst) < $(srcdir)/montana_naip_2009.bash.in > montana_naip_2009.bash

naip2009ccm_wa.bash: naip2009ccm_wa.bash.in Makefile
	$(do_subst) < $(srcdir)/naip2009ccm_wa.bash.in > naip2009ccm_wa.bash

nebr_naip_2010.bash: nebr_naip_2010.bash.in Makefile
	$(do_subst) < $(srcdir)/nebr_naip_2010.bash.in > nebr_naip_2010.bash

nh_2009.bash: nh_2009.bash.in Makefile
	$(do_subst) < $(srcdir)/nh_2009.bash.in > nh_2009.bash

nm_2009.bash: nm_2009.bash.in Makefile
	$(do_subst) < $(srcdir)/nm_2009.bash.in > nm_2009.bash

north_dakota_2009.bash: north_dakota_2009.bash.in Makefile
	$(do_subst) < $(srcdir)/north_dakota_2009.bash.in > north_dakota_2009.bash

nv_naip_2006.bash: nv_naip_2006.bash.in Makefile
	$(do_subst) < $(srcdir)/nv_naip_2006.bash.in > nv_naip_2006.bash

nv_naip_2010.bash: nv_naip_2010.bash.in Makefile
	$(do_subst) < $(srcdir)/nv_naip_2010.bash.in > nv_naip_2010.bash

ny_naip_2009.bash: ny_naip_2009.bash.in Makefile
	$(do_subst) < $(srcdir)/ny_naip_2009.bash.in > ny_naip_2009.bash

oh_naip_2010.bash: oh_naip_2010.bash.in Makefile
	$(do_subst) < $(srcdir)/oh_naip_2010.bash.in > oh_naip_2010.bash

ok_2010.bash: ok_2010.bash.in Makefile
	$(do_subst) < $(srcdir)/ok_2010.bash.in > ok_2010.bash

or_naip_2009.bash: or_naip_2009.bash.in Makefile
	$(do_subst) < $(srcdir)/or_naip_2009.bash.in > or_naip_2009.bash

penn_naip_2010.bash: penn_naip_2010.bash.in Makefile
	$(do_subst) < $(srcdir)/penn_naip_2010.bash.in > penn_naip_2010.bash

sc_naip_2009.bash: sc_naip_2009.bash.in Makefile
	$(do_subst) < $(srcdir)/sc_naip_2009.bash.in > sc_naip_2009.bash

south_dakota_2008.bash: south_dakota_2008.bash.in Makefile
	$(do_subst) < $(srcdir)/south_dakota_2008.bash.in > south_dakota_2008.bash

tn_naip_2010.bash: tn_naip_2010.bash.in Makefile
	$(do_subst) < $(srcdir)/tn_naip_2010.bash.in > tn_naip_2010.bash

tx_2010.bash: tx_2010.bash.in Makefile
	$(do_subst) < $(srcdir)/tx_2010.bash.in > tx_2010.bash

