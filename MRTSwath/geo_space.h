/*
!C****************************************************************************

!File: geoloc.h

!Description: Header file for 'geoloc.c' - see 'geoloc.c' for more information.

!Revision History:
 Revision 1.0 2001/05/08
 Robert Wolfe
 Original Version.

 Revision 1.1 2002/03/02
 Robert Wolfe
 Modified Geoloc_t sturcture for special input ISIN grid case.

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
      phone: 301-614-5508               Lanham, MD 20770  

 ! Design Notes:
   1. Structure is declared for the 'geoloc_type' and 'geoloc' data types.
  
!END****************************************************************************
*/

#ifndef GEOLOC_H
#define GEOLOC_H

#include "resamp.h"
#include "input.h"
#include "bool.h"
#include "space.h"
#include "hdf.h"
#include "mfhdf.h"
#include "myhdf.h"

/* Constants */

#define NBAND_MODIS (38)
#define NBAND_OFFSET_GEN (11)
#define NBAND_OFFSET (NBAND_MODIS + NBAND_OFFSET_GEN)
enum {BAND_GEN_250M = NBAND_MODIS, BAND_GEN_500M, BAND_GEN_1KM, BAND_GEN_NONE};
   /* 1 generic 250m, 1 generic 500m, 1 generic 1km, 1 generic no offset, 
      1 focal planes 250m, 2 focal planes 500m, 4 focal planes 1 km */
#define BAND_OFFSET_GEN (BAND_GEN_250M)

/* Geolocation type definition */

typedef enum {GRID_GEOLOC, SWATH_GEOLOC} Geoloc_type_t;

/* Structure for the 'geoloc' data type */

typedef struct {
  Geoloc_type_t geoloc_type;
  char *file_name;
  Img_coord_int_t size;
  Img_coord_int_t scan_size;
  Img_coord_int_t scan_size_geo;
  int nscan;
  bool open;
  Space_def_t space_def;
  int32 sds_file_id;
  Myhdf_sds_t sds_lat;
  Myhdf_sds_t sds_lon;
  float32 lat_fill;
  float32 lon_fill;
  int n_nest;
  Img_coord_float_t band_offset[NBAND_OFFSET];
  Img_coord_float_t **img;
  Geo_coord_t **geo;
  float32 *lat_buf;
  float32 *lon_buf;
  Geo_coord_t **geo_isin_nest[SPACE_MAX_NEST];
} Geoloc_t;

/* Prototypes */

Geoloc_t *OpenGeolocSwath(char *file_name);
Geoloc_t *SetupGeolocGrid(Space_def_t *space, Input_t *input, 
                          Kernel_t *kernel);
bool CloseGeoloc(Geoloc_t *this);
bool FreeGeoloc(Geoloc_t *this);
bool GetGeolocSwath(Geoloc_t *this, Space_t *space, int iscan);

#endif
