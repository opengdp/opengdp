/*
!C****************************************************************************

!File: resamp.h

!Description: Header file for resamp.c - see resamp.c for more information.

!Revision History:
 Revision 1.0 2001/05/08
 Robert Wolfe
 Original Version.

!Team Unique Header:
  This software was developed by the MODIS Land Science Team Support 
  Group for the Labatory for Terrestrial Physics (Code 922) at the 
  National Aeronautics and Space Administration, Goddard Space Flight 
  Center, under NASA Task 92-012-00.

 ! References and Credits:

  ! MODIS Science Team Member:
      Christopher O. Justice
      MODIS Land Science Team           University of Maryland
      justice@hermes.geog.umd.edu       Dept. of Geography
      phone: 301-405-1600               1113 LeFrak Hall
                                        College Park, MD, 20742

  ! Developers:
      Robert E. Wolfe (Code 922)
      MODIS Land Team Support Group     Raytheon ITSS
      robert.e.wolfe.1@gsfc.nasa.gov    4400 Forbes Blvd.
      phone: 301-614-5508               Lanham, MD 20706  

 ! Design Notes:
   1. Structures are declared for the 'img_coord_int' and 'img_coord_float' 
      data types.
 
!END****************************************************************************
*/

#ifndef RESAMP_H
#define RESAMP_H

#define RESAMPLER_NAME    "MODIS Reprojection Tool Swath"
#define RESAMPLER_VERSION "v2.2 September 2010"

#include "bool.h"
#include "common.h"

/* Constants */

#define NDET_1KM_MODIS (10)  /* Number of 1km detectors in a MODIS scan */
#define NFRAME_1KM_MODIS (1354)  /* Nominal number of 1km frames per MODIS 
                                    scan */

/* Integer image coordinates data structure */

typedef struct {
  int l;                /* line number */        
  int s;                /* sample number */
} Img_coord_int_t;

/* Floating point image coordinates data structure */

typedef struct {
  double l;               /* line number */
  double s;               /* sample number */
  bool is_fill;          /* fill value flag; 
                            'true' = is fill; 'false' = is not fill */
} Img_coord_double_t;

#endif
