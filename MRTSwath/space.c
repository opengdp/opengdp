/*
!C****************************************************************************

!File: space.c
  
!Description: Functions for mapping from an image space to latitude and
 longitude and visa versa.

!Revision History:
 Revision 1.0 2001/05/08
 Robert Wolfe
 Original Version.

 Revision 1.1 2002/03/02
 Robert Wolfe
 Added special handling for input ISIN case.

 Revision 1.5 2002/12/01
 Gail Schmidt
 The for_trans and inv_trans are undefined for Geographic projections.
 So, create our own to handle geographic projections.

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
      phone: 301-614-5508               Lanham, MD 20770  
  
 ! Design Notes:
   1. The following functions handle the image space mapping:

       SetupSpace - Setup the space mapping.
       ToSpace - Map from geodetic to image coordinates.
       FromSpace - Map from image to geodetic coordinates. 
       FreeSpace - Frees space data structure memory.

   2. Geodetic coordinates are the geodetic latitude and longitude of the 
      point to be mapped. Geodetic cooridnates are in radians.
   3. Only one space mapping definition (instance) should be used at any 
      one time because the GCTP package is not re-entrant.
   4. Image coordinates are in pixels with the origin (0.0, 0.0) at the 
      center of the upper left corner pixel.  Sample coordinates are positive 
      to the right and line coordinates are positive downward.
   5. 'SetupSpace' must be called before any of the other routines.  'FreeSpace'
      should be to free the 'space' data structure.
   5. GCTP stands for the General Cartographic Transformation Package.

!END****************************************************************************
*/

#include <stdlib.h>
#include "space.h"
#include "hdf.h"
#include "mfhdf.h"
#include "HdfEosDef.h"
#include "gctp_wrap.h"
//#include "proj.h"
//#include "cproj.h"
#include "bool.h"
#include "myerror.h"

/* Constants */

#define MAX_PROJ (31)  /* Maximum map projection number */
/* #define MAX_PROJ (99) */  /* Maximum map projection number */

/* Prototypes for initializing the GCTP projections */

/*void for_init(long proj_num, long zone, double *proj_param, long sphere,
              char *file27, char *file83, long *iflag, 
	      long (*for_trans[MAX_PROJ + 1])());
void inv_init(long proj_num, long zone, double *proj_param, long sphere,
              char *file27, char *file83, long *iflag, 
	      long (*inv_trans[MAX_PROJ + 1])());
*/
/* Functions */

Space_t *SetupSpace(Space_def_t *space_def)
/* 
!C******************************************************************************

!Description: 'SetupSpace' sets up the 'space' data structure and initializes 
 the forward and inverse mapping.
 
!Input Parameters:
 space_def      space definition; the following fields are input:
                  pixel_size, ul_corner.x, ul_corner.y, img_size.l,
		  img_size.s, proj_num, zone, sphere, proj_param[*], isin_type

!Output Parameters:
 (returns)      'space' data structure or NULL when error occurs

!Team Unique Header:

 ! Design Notes:
   1. An error status is returned when:
       a. either image size dimension is zero or negative
       b. the pixel size is zero or negative
       c. the project number is less than zero or greater than 'MAX_PROJ'
       d. there is an error allocating memory
       e. an error occurs initializing either the forward or inverse map 
          projection.
   2. Error messages are handled with the 'LOG_RETURN_ERROR' macro.
   3. 'FreeSpace' should be called to deallocate the space structure.
   4. Angular values should be in radians for any calls to GCTP.

!END****************************************************************************
*/
{
  Space_t *this;
  char file27[1024];          /* name of NAD 1927 parameter file */
  char file83[1024];          /* name of NAD 1983 parameter file */
  char mrttables[1024];       /* storage for mrttables */
  char *ptr;                  /* point to mrttables */
  int32 (*for_trans[MAX_PROJ + 1])();
  int32 (*inv_trans[MAX_PROJ + 1])();
  int ip;
  int32 iflag;

  /* Place State Plane directory in file27, file83 */
  ptr = (char *)getenv("MRTSWATH_DATA_DIR");
  if( ptr == NULL ) {
     ptr = (char *)getenv("MRTDATADIR");
     if (ptr == NULL)
        ptr = MRTSWATH_DATA_DIR;
   }

  strcpy(mrttables, ptr);
  sprintf(file27, "%s/nad27sp", mrttables);
  sprintf(file83, "%s/nad83sp", mrttables);

  /* Verify some of the space definition parameters */
  
  if (space_def->img_size.l < 1) 
    LOG_RETURN_ERROR("invalid number of lines", "SetupSpace",  
                 (Space_t *)NULL);
  if (space_def->img_size.s < 1) 
    LOG_RETURN_ERROR("invalid number of samples per line", "SetupSpace",  
                 (Space_t *)NULL);
  if (space_def->pixel_size <= 0.0)
    LOG_RETURN_ERROR("invalid pixel size", "SetupSpace",  
                 (Space_t *)NULL);
  if (space_def->proj_num < 0  ||  space_def->proj_num > MAX_PROJ)
    LOG_RETURN_ERROR("invalid projection number", "SetupSpace",  
                 (Space_t *)NULL);

  /* Create the space data structure */

  this = (Space_t *)malloc(sizeof(Space_t));
  if (this == (Space_t *)NULL) 
    LOG_RETURN_ERROR("allocating Space structure", "SetupSpace", 
                 (Space_t *)NULL);

  /* Copy the space definition */

  this->def.pixel_size = space_def->pixel_size;
  this->def.ul_corner.x = space_def->ul_corner.x;
  this->def.ul_corner.y = space_def->ul_corner.y;
  this->def.lr_corner.x = space_def->lr_corner.x;
  this->def.lr_corner.y = space_def->lr_corner.y;
  this->def.img_size.l = space_def->img_size.l;
  this->def.img_size.s = space_def->img_size.s;
  this->def.proj_num = space_def->proj_num;
  this->def.zone = space_def->zone;
  this->def.sphere = space_def->sphere;
  this->def.isin_type = space_def->isin_type;
  for (ip = 0; ip < NPROJ_PARAM; ip++) {
    this->def.proj_param[ip] = space_def->proj_param[ip];
  }

  /* Setup the forward transform */

  for_init(this->def.proj_num, this->def.zone, this->def.proj_param, 
           this->def.sphere, file27, file83, &iflag, for_trans);
  if (this->def.proj_num != GEO) {
    printf ("iflag=%i\n", iflag);
    if (iflag) {
      free(this);
      LOG_RETURN_ERROR("bad return from for_init", "SetupSpace",
                       (Space_t *)NULL);
    }
    this->for_trans = for_trans[this->def.proj_num];
  }
  else
  {
    /* for_trans is not defined for geographic so call our own */
    this->for_trans = geofor;
  }

  /* Setup the inverse transform */

  inv_init(this->def.proj_num, this->def.zone, this->def.proj_param, 
           this->def.sphere, file27, file83, &iflag, inv_trans);
  if (this->def.proj_num != GEO) {
    if (iflag) {
      free(this);
      LOG_RETURN_ERROR("bad return from inv_init", "SetupSpace",
                       (Space_t *)NULL);
    }
    this->inv_trans = inv_trans[this->def.proj_num];
  }
  else
  {
    /* inv_trans is not defined for geographic so call our own */
    this->inv_trans = geoinv;
  }

  return this;
}


bool ToSpace(Space_t *this, Geo_coord_t *geo, Img_coord_double_t *img)
/* 
!C******************************************************************************

!Description: 'ToSpace' maps a point from geodetic to image coordinates.
 
!Input Parameters:
 this           'space' data structure; the following fields are input:
                   for_trans
 geo            geodetic coordinates (radians)

!Output Parameters:
 img            image space coordinates
 (returns)      status:
                  'true' = okay
		  'false' = error return

!Team Unique Header:

 ! Design Notes:
   1. 'SetupSpace' must be called before this routine is called.
   2. An error status is returned when:
       a. a fill value ('geo->is_fill' = 'true') is given
       b. an error occurs in the forward transformation from 
          coordinates to map projection coordinates.
   3. Error messages are handled with the 'LOG_RETURN_ERROR' macro.
   4. The image space coordinate is set to fill ('img->is_fill' = 'true') if 
      an error occurs.

!END****************************************************************************
*/
{
  Map_coord_t map;

  img->is_fill = true;
  if (geo->is_fill)
    LOG_RETURN_ERROR("called with fill value", "ToSpace", false);

  if (this->for_trans == NULL)
    LOG_RETURN_ERROR("forward transform is null", "ToSpace", false);
  if (this->for_trans(geo->lon, geo->lat, &map.x, &map.y) != GCTP_OK) 
    LOG_RETURN_ERROR("forward transform", "ToSpace", false);

  img->l = (this->def.ul_corner.y - map.y) / this->def.pixel_size;
  img->s = (map.x - this->def.ul_corner.x) / this->def.pixel_size;
  img->is_fill = false;

  return true;
}


bool FromSpace(Space_t *this, Img_coord_double_t *img, Geo_coord_t *geo)
/* 
!C******************************************************************************

!Description: 'FromSpace' maps a point from image to geodetic coordinates.
 
!Input Parameters:
 this           'space' data structure; the following fields are input:
                   inv_trans
 img            image space coordinates

!Output Parameters:
 geo            geodetic coordinates (radians)
 (returns)      status:
                  'true' = okay
		  'false' = error return

!Team Unique Header:

 ! Design Notes:
   1. 'SetupSpace' must be called before this routine is called.
   2. An error status is returned when:
       a. a fill value ('img->is_fill' = 'true') is given
       b. an error occurs in the inverse transformation from map projection 
          to geodetic coordinates.
   3. Error messages are handled with the 'LOG_RETURN_ERROR' macro.
   4. The image space coordinate is set to fill ('geo->is_fill' = 'true') if 
      an error occurs.

!END****************************************************************************
*/
{
  Map_coord_t map;

  geo->is_fill = true;
  if (img->is_fill) 
    LOG_RETURN_ERROR("called with fill value", "FromSpace", false);

  map.y = this->def.ul_corner.y - (img->l * this->def.pixel_size);
  map.x = this->def.ul_corner.x + (img->s * this->def.pixel_size);

  if (this->inv_trans == NULL)
    LOG_RETURN_ERROR("inverse transform is null", "FromSpace", false);
  if (this->inv_trans(map.x, map.y, &geo->lon, &geo->lat) != GCTP_OK) 
    LOG_RETURN_ERROR("inverse transform", "FromSpace", false);
  geo->is_fill = false;

  return true;
}

bool FreeSpace(Space_t *this)
/* 
!C******************************************************************************

!Description: 'FreeSpace' frees 'space' data structure memory.
 
!Input Parameters:
 this           'space' data structure

!Output Parameters:
 (returns)      status:
                  'true' = okay (always returned)

!Team Unique Header:

 ! Design Notes:
   1. 'SetupSpace' must be called before this routine is called.
   2. An error status is never returned.

!END****************************************************************************
*/
{
  if (this != (Space_t *)NULL) {
    free(this);
  }

  return true;
}

