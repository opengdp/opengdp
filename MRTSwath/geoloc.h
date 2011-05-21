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
  Img_coord_double_t band_offset[NBAND_OFFSET];
  Img_coord_double_t **img;
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

/*
!C****************************************************************************

!File: space.h

!Description: Header file for space.c - see space.c for more information.

!Revision History:
 Revision 1.0 2001/05/08
 Robert Wolfe
 Original Version.

 Revision 1.1 2001/07/24
 Robert Wolfe
 Changed the number of parameters from 16 to 15 to match the GCTP package.

 Revision 1.2 2002/03/02
 Robert Wolfe
 Added new type (Space_isin_t) for special ISIN input case.

 Revision 1.3 2002/06/13
 Gail Schmidt
 Added lower right corner definition.

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
   1. Structures are declared for the 'map_coord', 'geo_coord', 'space_def' and
      'space' data types.
   2. The number of projection parameters is set at 'NPROJ_PARAM'.
   3. GCTP stands for the General Cartographic Transformation Package.

!END*****************************************************************************/

#ifndef SPACE_H
#define SPACE_H

#include "resamp.h"
#include "bool.h"

/* Constants */

#define NPROJ_PARAM (15)

typedef enum {SPACE_NOT_ISIN, SPACE_ISIN_NEST_1, SPACE_ISIN_NEST_2,
              SPACE_ISIN_NEST_4} Space_isin_t;
#define SPACE_MAX_NEST (4)

/* Forward and inverse transformations for Geographic projections */

long geofor (double lon, double lat, double *x, double *y);
long geoinv (double x, double y, double *lon, double *lat);

/* Structure to store map projection coordinates */

typedef struct {
  double x;             /* Map projection X coordinate (meters) */
  double y;             /* Map projection Y coordinate (meters) */
  bool is_fill;         /* Flag to indicate whether the point is a fill value;
                           'true' = fill; 'false' = not fill */
} Map_coord_t;


/* Structure to store geodetic coordinates */

typedef struct {
  double lon;           /* Geodetic longitude coordinate (radians) */
  double lat;           /* Geodetic latitude coordinate (radians) */
  bool is_fill;         /* Flag to indicate whether the point is a fill value;
                           'true' = fill; 'false' = not fill */
} Geo_coord_t;

/* Structure to store the space definition */

typedef struct {
  int proj_num;         /* GCTP map projection number */
  double proj_param[NPROJ_PARAM]; /* GCTP map projection parameters (DMS) */
  double orig_proj_param[NPROJ_PARAM]; /* original GCTP map projection
                                          parameters (decimal degrees) */
  float pixel_size;       /* Pixel size (meters, degrees for GEO) */
  Map_coord_t ul_corner;  /* Map projection coordinates of the center of the
                             pixel in the upper left corner of the image */
  Map_coord_t lr_corner;  /* Map projection coordinates of the center of the
                             pixel in the lower right corner of the image */
  Geo_coord_t ul_corner_geo;  /* Lat/Long coordinates of the center of the
                             pixel in the upper left corner of the image */
  Geo_coord_t lr_corner_geo;  /* Lat/Long coordinates of the center of the
                             pixel in the lower right corner of the image */
  bool ul_corner_set;   /* Flag to indicate whether the upper left corner
                           has been set; 'true' = set; 'false' = not set */
  bool lr_corner_set;   /* Flag to indicate whether the lower right corner
                           has been set; 'true' = set; 'false' = not set */
  Img_coord_int_t img_size;  /* Image size (lines, samples) */
  int zone;             /* GCTP zone number */
  int sphere;           /* GCTP sphere number */
  bool zone_set;        /* Flag to indicate whether the zone has been set;
                           'true' = set; 'false' = not set */
  Space_isin_t isin_type;  /* Flag to indicate whether the projection is ISIN,
                              and if it is, the ISIN nesting */
} Space_def_t;

/* Structure to store the space information */

typedef struct {
  Space_def_t def;       /* Space definition structure */
  long (*for_trans)(double lon, double lat, double *x, double *y);
                         /* Forward transformation function call */
  long (*inv_trans)(double x, double y, double *lon, double *lat);
                         /* Inverse transformation function call */
} Space_t;

#endif
