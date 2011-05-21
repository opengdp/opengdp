/*
!C****************************************************************************

!File: scan.h

!Description: Header file for 'scan.c' - see 'scan.c' for more information.

!Revision History:
 Revision 1.0 2001/05/08
 Robert Wolfe
 Original Version.

 Revision 1.1 2002/03/02
 Robert Wolfe
 Added special handling for input ISIN case.

!Team Unique Header:
  This software was developed by the MODIS Land Science Team Support 
  Group for the Laboratory for Terrestrial Physics (Code 922) at the 
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
   1. Structure is declared for the 'scan' and 'scan_buf' data types.
  
!END****************************************************************************
*/

#ifndef SCAN_H
#define SCAN_H

#include "resamp.h"
#include "bool.h"
#include "patches.h"
#include "geoloc.h"
#include "input.h"
#include "space.h"
#include "kernel.h"

/* Structure for the 'scan_buf' data type */

typedef struct {
  Img_coord_double_t img;  /* Output image space coordinates of the input 
                             image pixel value */
  double v;              /* Input image pixel value */
} Scan_buf_t;

typedef struct {
  double ds;             /* Input image delta-sample between rows  
                           (special input ISIN case only) */
  Img_coord_double_t vir_img;  /* Output image space coordinates of the virtual 
                                 input image pixel value (special input ISIN 
				 case only) */
} Scan_isin_buf_t;

/* Structure for the 'scan' data type */

typedef struct {
  Img_coord_int_t size;  /* Scan buffer size, including extra lines before and
                            after */
  Img_coord_int_t extra_before;  /* Number of extra lines and samples after 
                                    the actual input image buffer ends */
  Img_coord_int_t extra_after;  /* Number of extra lines and samples before 
                                   the actual input image buffer starts */
  Img_coord_double_t band_offset;  /* Band offset */
  int ires;             /* Input resolution (relative to 1 km input); 
                           1 = 1 km; 2 = 500 m; 4 = 250 m */
  Scan_buf_t **buf;     /* Input scan buffer */
  Scan_isin_buf_t **isin_buf;  /* Input scan buffer (special ISIN input case 
                                  only) */
  Space_isin_t isin_type; /* Flag to indicate whether the input 
                             projection is ISIN, and if it is, the 
			     ISIN nesting */
} Scan_t;

/* Prototypes */

Scan_t *SetupScan(Geoloc_t *geoloc, Input_t *input, Kernel_t *kernel);
bool FreeScan(Scan_t *this);
bool MapScanSwath(Scan_t *this, Geoloc_t *geoloc);
bool MapScanGrid(Scan_t *this, Geoloc_t *geoloc, Space_def_t *output_space_def, 
                 int iscan);
bool ExtendScan(Scan_t *this);
bool GetScanInput(Scan_t *this, Input_t *input, int il, int nl);
bool ProcessScan(Scan_t *this, Kernel_t *kernel, Patches_t *patches, int nl,
                 Kernel_type_t kernel_type);

#endif
