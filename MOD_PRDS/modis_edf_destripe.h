/******************************************************************************
 Program to destripe modis data
 
 Copyright(C) 2004, University of Wisconsin-Madison MODIS Group
 Created by Liam.Gumley@ssec.wisc.edu
     
 ported to C by Brian Case

******************************************************************************/

#ifndef _MODIS_EDF_DESTRIPE_H
#define _MODIS_EDF_DESTRIPE_H

int modis_edf_destripe( int32 npixel, int32 nscan, int32 stripsize,
                        int32 ref, int32 *mir,
                        int16 *image, int16 *destripe );

#endif