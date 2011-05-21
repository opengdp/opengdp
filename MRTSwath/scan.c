/*
!C****************************************************************************

!File: scan.c
  
!Description: Functions to handle and extend a scan of input data.

!Revision History:
 Revision 1.0 2001/05/08
 Robert Wolfe
 Original Version.

 Revision 1.1 2002/03/02
 Robert Wolfe
 a. Added special handling for input ISIN case.
 b. Fixed compilation warning (enumerated type is mixed ...).

 Revision 1.5 2002/12/02
 Gail Schmidt
 Added support for INT8 data types.

 Revision 2.2.0 2010/07/29
 Kirk Evenson

 Reworked nearest neighbor resampling so that it will use the bilinear
 mechanism to identify which input pixel is closest to an output pixel.
 Plus corrected nearest neighbor so it selects the nearest input pixel 
 rather than sum and average a number of input pixels.
 

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
   1. The following public functions handle the scans:

        SetupScan - Setup 'scan' data structure.
        FreeScan - Free the 'scan' data structure memory.
        MapScanSwath - Interpolate the input geolocation data to the correct
          band and resolution and store it in the 'scan' data structure.
        MapScanGrid - copy the input grid locations to the output grid
          and store it in the the 'scan' data structure.
        ExtendScan - extend the scan to allow for large kernels and
          to handle the scan overlap region.
        GetScanInput - reads a scan of input data.
        ProcessScan - processes a scan of input data and updates all of
          the output patches the scan overlaps.

   2. The following internal function is also used to handle the scan:

        Extend1d - linearly interpolate/extrapolate a line in two 
          dimensions.
        Extend2d - linearly interpolates/extrapolates a point in two 
          dimensions.
        PointInTriangle - determine if a point lies within a triangle 
          and, if it is, return the location within the triangle.

   3. 'MapScanSwath' should be called when the input is swath data, and 
      'MapScanGrid' should be called when the input is grid data.
   4. 'SetupScan' must be called before any of the other routines.  
   5. 'FreeScan' should be used to free the 'scan' data structure.

!END****************************************************************************
*/

#include <stdlib.h>
#include <string.h>

#include "scan.h"
#include "myerror.h"

/* Constants */

#define NSCAN_TOUCH (2)   /* Value to set 'ntouch' to when a scan is touched */
#define MIN_WEIGHT (0.10) /* Minimum weight for a valid output pixel */

/* #define DEBUG_ZEROS */

Scan_t *SetupScan(Geoloc_t *geoloc, Input_t *input, Kernel_t *kernel)
/* 
!C******************************************************************************

!Description: 'SetupScan' sets up the 'scan' data structure.
 
!Input Parameters:
 geoloc         'geoloc' data structure; the following fields are input:
                  geoloc_type, size, band_offset, space_def.isin_type
 input          'input' data structure; the following fields are input:
                  size, ires, scan_size, space_def.isin_type
 kernel         'kernel' data structure; the following fields are input:
                  after, before

!Output Parameters:
 (returns)      'scan' data structure or NULL when an error occurs

!Team Unique Header:

 ! Design Notes:
   1. An error status is returned when:
       a. the size of the geolocation and input arrays are not compatible
       b. memory allocation is not successful.
   2. 'OpenInput', either 'OpenGeolocSwath' or 'SetupGeolocGrid', and either 
      'GetUserKernel' or 'GenKernel' must be called before this routine is 
      called.
   3. Error messages are handled with the 'LOG_RETURN_ERROR' macro.
   4. 'FreeScan' should be called to deallocate memory used by the 
      'scan' data structures.

!END****************************************************************************
*/
{
  Scan_t *this;
  size_t n;
  Scan_buf_t *buf_p;
  Scan_isin_buf_t *isin_buf_p;
  int il;
  char *error_string = (char *)NULL;

  /* Check the input sizes */

  if (((geoloc->geoloc_type == GRID_GEOLOC)  &&   
       (geoloc->size.l != input->size.l))  ||
      ((geoloc->geoloc_type == SWATH_GEOLOC)  &&     
       ((input->ires * geoloc->size.l) != input->size.l))) 
      LOG_RETURN_ERROR("number of lines in geolocation and input files " 
                   "are not compatible", "SetupScan", (Scan_t *)NULL);

  if (((geoloc->geoloc_type == GRID_GEOLOC)  &&   
       (geoloc->size.s != input->size.s))  ||
      ((geoloc->geoloc_type == SWATH_GEOLOC)  &&     
       ((input->ires * geoloc->size.s) != input->size.s))) 
    LOG_RETURN_ERROR("number of samples in geolocation and input files "
                 "are not compatible", "SetupScan", (Scan_t *)NULL);

  /* Create the scan data structure */

  this = (Scan_t *)malloc(sizeof(Scan_t));
  if (this == (Scan_t *)NULL) 
    LOG_RETURN_ERROR("allocating scan structure", "SetupScan",
                          (Scan_t *)NULL);

  /* Add some extra lines to include the kernel */

  this->ires = input->ires;
  this->band_offset.l = geoloc->band_offset[input->iband].l;
  this->band_offset.s = geoloc->band_offset[input->iband].s;
  this->extra_before.l = kernel->after.l;
  this->extra_before.s = kernel->after.s;
  this->extra_after.l = kernel->before.l + 1;
  this->extra_after.s = kernel->before.s + 1;

  this->size.l = input->scan_size.l + 
                 this->extra_before.l + this->extra_after.l;
  this->size.s = input->scan_size.s + 
                 this->extra_before.s + this->extra_after.s;

  if (geoloc->geoloc_type == GRID_GEOLOC) 
    this->isin_type = geoloc->space_def.isin_type;
  else
    this->isin_type = SPACE_NOT_ISIN;

  /* Set up two dimensional buffers */

  this->buf = (Scan_buf_t **)NULL;

  if (error_string == (char *)NULL) {
    this->buf = (Scan_buf_t **)calloc((size_t)this->size.l, 
                                      sizeof(Scan_buf_t *));
    if (this->buf == (Scan_buf_t **)NULL) 
    error_string = "allocating scan buffer array";
  }

  if (error_string == (char *)NULL) {
    n = (size_t)(this->size.l * this->size.s);
    buf_p = (Scan_buf_t *)calloc(n, sizeof(Scan_buf_t));
    if (buf_p == (Scan_buf_t *)NULL) 
      error_string = "allocating scan buffer";
  }

  if (error_string == (char *)NULL) {
    for (il = 0; il < this->size.l; il++) {
      this->buf[il] = buf_p;
      buf_p += this->size.s;
    }
  }

  this->isin_buf = (Scan_isin_buf_t **)NULL;

  if (this->isin_type != SPACE_NOT_ISIN) {

    if (error_string == (char *)NULL) {
      this->isin_buf = (Scan_isin_buf_t **)calloc((size_t)this->size.l, 
                                                  sizeof(Scan_isin_buf_t *));
      if (this->isin_buf == (Scan_isin_buf_t **)NULL) 
        error_string = "allocating scan isin buffer array";
    }

    if (error_string == (char *)NULL) {
      n = (size_t)(this->size.l * this->size.s);
      isin_buf_p = (Scan_isin_buf_t *)calloc(n, sizeof(Scan_isin_buf_t));
      if (isin_buf_p == (Scan_isin_buf_t *)NULL) 
        error_string = "allocating scan isin buffer";
    }

    if (error_string == (char *)NULL) {
      for (il = 0; il < this->size.l; il++) {
        this->isin_buf[il] = isin_buf_p;
        isin_buf_p += this->size.s;
      }
    }
  }

  if (error_string != (char *)NULL) {
    if (this->buf != (Scan_buf_t **)NULL) {
      if (this->buf[0] != (Scan_buf_t *)NULL) free(this->buf[0]);
      free(this->buf);
    }
    if (this->isin_buf != (Scan_isin_buf_t **)NULL) {
      if (this->isin_buf[0] != (Scan_isin_buf_t *)NULL) free(this->isin_buf[0]);
      free(this->isin_buf);
    }
    free(this);
    LOG_RETURN_ERROR(error_string, "SetupScan", (Scan_t *)NULL);
  }

  return (this);
}


bool FreeScan(Scan_t *this)
/* 
!C******************************************************************************

!Description: 'FreeScan' frees the 'scan' data structure memory.
 
!Input Parameters:
 this           'input' data structure; the following fields are input:
                   buf

!Output Parameters:
 (returns)      status:
                  'true' = okay (always returned)

!Team Unique Header:

 ! Design Notes:
   1. 'SetupScan' must be called before this routine is called.
   2. An error status is never returned.

!END****************************************************************************
*/
{
  if (this != (Scan_t *)NULL) {

    if (this->buf != (Scan_buf_t **)NULL) {
      if (this->buf[0] != (Scan_buf_t *)NULL) free(this->buf[0]);
      free(this->buf);
    }

    if (this->isin_type != SPACE_NOT_ISIN) {

      if (this->isin_buf != (Scan_isin_buf_t **)NULL) {
        if (this->isin_buf[0] != (Scan_isin_buf_t *)NULL) 
          free(this->isin_buf[0]);
        free(this->isin_buf);
      }
    }

    free(this);
  }
  return true;
}


void Extend1d(Img_coord_double_t *p, double d, 
              Img_coord_double_t *a, Img_coord_double_t *b)
/* 
!C******************************************************************************

!Description: 'Extend1d' linearly interpolates/extrapolates a line in two 
 dimensions.

!Input Parameters:
 a              first point
 b              second point
 d              amount to extrapolate

!Output Parameters:
 p              output point; the value returned is '(a + (d * (b - a))'

!Team Unique Header:

 ! Design Notes:
   1. If either of the inputs are fill values, a fill value is returned.

!END****************************************************************************
*/
{
  double d1;

  if (a->is_fill  ||  b->is_fill) {
    p->is_fill = true;
    return;
  } else p->is_fill = false;

  d1 = 1.0 - d;
  p->l = (a->l * d1) + (b->l * d);
  p->s = (a->s * d1) + (b->s * d);

  return;
}

void Extend2d(Img_coord_double_t *p, Img_coord_double_t d, 
              Img_coord_double_t *l0s0, Img_coord_double_t *l0s1, 
              Img_coord_double_t *l1s0, Img_coord_double_t *l1s1)
/* 
!C******************************************************************************

!Description: 'Extend2d' linearly interpolates/extrapolates a point in two 
 dimensions.

!Input Parameters:
 l0s0           point for line 0, sample 0
 l0s1           point for line 0, sample 1
 l1s0           point for line 1, sample 0
 l1s1           point for line 1, sample  1
 d              amount to extrapolate in each dimension

!Output Parameters:
 p              output point; the value returned is 
                  's0 + (d.s * (s1 - s0))'
                where 
                  's0 = (l0s0 + (d.l * (l1s0 - l0s0))'
                  's1 = (l0s1 + (d.l * (l1s1 - l0s1))'

!Team Unique Header:

 ! Design Notes:
   1. If any of the inputs are fill values, a fill value is returned.

!END****************************************************************************
*/
{
  Img_coord_double_t s0, s1, d1;

  if (l0s0->is_fill  ||  l0s1->is_fill  ||
      l1s0->is_fill  ||  l1s1->is_fill) {
    p->is_fill = true;
    return;
  } else p->is_fill = false;

  d1.l = 1.0 - d.l;
  d1.s = 1.0 - d.s;

  s0.l = (l0s0->l * d1.l) + (l1s0->l * d.l);
  s0.s = (l0s0->s * d1.l) + (l1s0->s * d.l);

  s1.l = (l0s1->l * d1.l) + (l1s1->l * d.l);
  s1.s = (l0s1->s * d1.l) + (l1s1->s * d.l);

  p->l = (s0.l * d1.s) + (s1.l * d.s);
  p->s = (s0.s * d1.s) + (s1.s * d.s);

  return;
}


bool MapScanSwath(Scan_t *this, Geoloc_t *geoloc)
/* 
!C******************************************************************************

!Description: 'MapScanSwath' interpolates input geolocation data to the 
 correct band and resolution and stores it in the 'scan' data structure.
 
!Input Parameters:
 this           'scan' data structure; the following fields are input:
                   extra_before, size, extra_after, ires, band_offset
 geoloc         'geoloc' data structure; the following fields are input:
                   geoloc_type, scan_size, img

!Output Parameters:
 this           'scan' data structure; the following field is modified:
                   buf[*][*].img
 (returns)      status:
                  'true' = okay (always returned)

!Team Unique Header:

 ! Design Notes:
   1. An error status is never returned.
   2. 'SetupScan' must be called before this routine is called.

!END****************************************************************************
*/
{
  int il1, il2;
  int is1, is2;
  int il, is;
  int il_geo, is_geo;
  Img_coord_double_t dist;
  Img_coord_double_t geo;
  double res_inv;

  il1 = this->extra_before.l;
  il2 = this->size.l - this->extra_after.l;
  is1 = this->extra_before.s;
  is2 = this->size.s - this->extra_after.s;
  res_inv = 1.0 / (double)this->ires;

  /* For each line in the output space (non extended scan) */

  for (il = il1; il < il2; il++) {
    geo.l = this->band_offset.l + ((il - il1) * res_inv);
    if (geoloc->geoloc_type == SWATH_GEOLOC)
      geo.l -= 0.5 * (1.0 - res_inv);
    il_geo = (int)geo.l;
    if (il_geo < 0) 
      il_geo = 0;
    else if (il_geo > (geoloc->scan_size.l - 2)) 
      il_geo = geoloc->scan_size.l - 2;
    dist.l = geo.l - il_geo;

    for (is = is1; is < is2; is++) {
      geo.s = this->band_offset.s + ((is - is1) * res_inv);
      is_geo = (int)geo.s;
      if (is_geo < 0)
        is_geo = 0;
      else if (is_geo > (geoloc->scan_size.s - 2)) 
        is_geo = geoloc->scan_size.s - 2;
      dist.s = geo.s - is_geo;

      /* Resample to input image resolution and apply band offset */
      Extend2d(&this->buf[il][is].img, dist, 
               &geoloc->img[il_geo][is_geo],     
               &geoloc->img[il_geo][is_geo + 1],
               &geoloc->img[il_geo + 1][is_geo], 
               &geoloc->img[il_geo + 1][is_geo + 1]);
    }
  }

  return true;
}

#define OFFSET_ISIN_NEST_2 (-0.25)
#define DELTA_ISIN_NEST_2  (0.5)
#define OFFSET_ISIN_NEST_4 (-0.375)
#define DELTA_ISIN_NEST_4  (0.25)
#define EPS_LAT_ISIN (1.0e-7)

bool MapScanGrid(Scan_t *this, Geoloc_t *geoloc, Space_def_t *output_space_def, 
                 int iscan)
/* 
!C******************************************************************************

!Description: 'MapScanGrid' copies the input geolocation data to the 
 'scan' data structure.
 
!Input Parameters:
 this           'scan' data structure; the following fields are input:
                   extra_before, size, isin_type
 geoloc         'geoloc' data structure; the following fields are input:
                   geoloc_type, space_def, nscan, scan_size.l, scan_size_geo, 
                   n_nest, 
 output_space_def  output grid space definition; the following fields are input:
                  pixel_size, ul_corner.x, ul_corner.y, img_size.l,
                  img_size.s, proj_num, zone, sphere, proj_param[*], isin_type
 iscan          scan number

!Output Parameters:
 this           'input' data structure; the following field is modified:
                   buf[*][*].img
                for special input ISIN case the following fields are also 
                modified:
                   isin_buf
 geoloc         'geoloc' data structure; the following field is modified:
                   geo, geo_isin_nest
 (returns)      status:
                  'true' = okay (always returned)

!Team Unique Header:

 ! Design Notes:
   1. For each call the input and output image space mapping are reinitialized, 
      used and then freed.  Since the image space mapping is non-reentrant, the
      GCTP library should not be use outside of this routine when this routine 
      is called.
   2. An error status is returned when:
       a. the input is not a grid
       b. the input grid types are not the same in the scan and geoloc 
          data structures
       c. the scan number is not in the valid range
       d. there is an error setting up the input or output image space mapping
       e. there is an error converting to/from the input map projection 
          coordinates.
       f. there is an error converting to the output map projection coordinates.
   3. 'SetupScan' and 'SetupGeolocGrid' must be called before this routine is 
      called.

!END****************************************************************************
*/
{
  int il, is;
  int il_geo, is_geo;
  Geo_coord_t *geo_p1, *geo_p2;
  Img_coord_double_t img;
  Space_t *input_space, *output_space;
  Geo_coord_t geo;
  Scan_isin_buf_t *isin_buf_p;
  Scan_buf_t *buf_p;
  double offset_isin_nest, delta_isin_nest;
  int in;
  Geo_coord_t *geo_isin_nest_p[SPACE_MAX_NEST];
  int n_not_fill;
  Img_coord_double_t img_isin_nest[SPACE_MAX_NEST];

  /* Check inputs */

  if (geoloc->geoloc_type != GRID_GEOLOC)
    LOG_RETURN_ERROR("not a grid", "MapScanGrid", false);

  if (this->isin_type != geoloc->space_def.isin_type)
    LOG_RETURN_ERROR("input grid types not the same", "MapScanGrid",false);

  if (iscan < 0  ||  iscan >= geoloc->nscan)
    LOG_RETURN_ERROR("invalid scan number", "MapScanGrid", false);

  /* Map from input space to geographic coordinates */

  input_space = SetupSpace(&geoloc->space_def);
  if (input_space == (Space_t *)NULL) 
    LOG_RETURN_ERROR("setting up input space", "MapScanGrid", false);

  il_geo = iscan * geoloc->scan_size.l;
  il_geo -= this->extra_before.l;

  if (this->isin_type == SPACE_NOT_ISIN  ||
      this->isin_type == SPACE_ISIN_NEST_1) {

    /* Single point mapping for non-ISIN grid and ISIN grid with  no nesting */

    for (il = 0; il < geoloc->scan_size_geo.l; il++) {
      geo_p1 = geoloc->geo[il];

      img.is_fill = false;
      img.l = il_geo;
      is_geo = -this->extra_before.s;
      for (is = 0; is < geoloc->scan_size_geo.s; is++) {
        img.s = is_geo;
        if (!FromSpace(input_space, &img, geo_p1)) {
          FreeSpace(input_space);
          LOG_RETURN_ERROR("converting from input map coordinates (a)", 
                       "MapScanGrid", false);
        }
     
        geo_p1++;
        is_geo++;
      }

      il_geo++;
    }

  } else {

    /* Multi-point mapping (one for each nested row) for ISIN grid with 
       2 (500m) and 4 (1km) rows nested */

    if (this->isin_type == SPACE_ISIN_NEST_2) {
      offset_isin_nest = OFFSET_ISIN_NEST_2; 
      delta_isin_nest = DELTA_ISIN_NEST_2;
    } else {
      offset_isin_nest = OFFSET_ISIN_NEST_4; 
      delta_isin_nest = DELTA_ISIN_NEST_4; 
    }

    for (il = 0; il < geoloc->scan_size_geo.l; il++) {
      geo_p1 = geoloc->geo[il];
      for (in = 0; in < geoloc->n_nest; in++)
        geo_isin_nest_p[in] = geoloc->geo_isin_nest[in][il];

      is_geo = -this->extra_before.s;
      for (is = 0; is < geoloc->scan_size_geo.s; is++) {
        img.l = il_geo + offset_isin_nest;
        img.s = is_geo;
        img.is_fill = false;
        for (in = 0; in < geoloc->n_nest; in++) {
          if (!FromSpace(input_space, &img, geo_isin_nest_p[in])) {
            FreeSpace(input_space);
            LOG_RETURN_ERROR("converting from input map coordinates (b)", 
                         "MapScanGrid", false);
          }
          img.l += delta_isin_nest;
        }
        
        /* Compute the average location of the nested points */ 

        n_not_fill = 0;
        geo_p1->lat = 0.0;
        geo_p1->lon = 0.0;
        for (in = 0; in < geoloc->n_nest; in++) {
          if (!geo_isin_nest_p[in]->is_fill) {
            n_not_fill++;
            geo_p1->lat += geo_isin_nest_p[in]->lat;
            geo_p1->lon += geo_isin_nest_p[in]->lon;
          }
          geo_isin_nest_p[in]++;
        }
        if (n_not_fill < 1) geo_p1->is_fill = true;
        else {
          geo_p1->lat /= (double)n_not_fill;
          geo_p1->lon /= (double)n_not_fill;
          geo_p1->is_fill = false;
        }
        geo_p1++;
        is_geo++;
      } /* end for (is ... ) */

      il_geo++;
    } /* end for (il ... ) */
  }

  /* Special for input ISIN case: calculate delta-sample between lines */

  if (geoloc->space_def.isin_type != SPACE_NOT_ISIN) {

    /* Single point mapping for ISIN grid with no nesting */

    if (geoloc->space_def.isin_type == SPACE_ISIN_NEST_1) {

      for (il = 0; il < this->size.l; il++) {
        geo_p1 = geoloc->geo[il];
        geo_p2 = geoloc->geo[il + 1];
        isin_buf_p = this->isin_buf[il];
        is_geo = -this->extra_before.s;

        for (is = 0; is < this->size.s; is++) {
          geo.lon = geo_p1->lon;
          geo.lat = geo_p2->lat;
          geo.is_fill = (geo_p1->is_fill == true  ||
                         geo_p2->is_fill == true) ? true : false;
          if (!ToSpace(input_space, &geo, &img)) {
            FreeSpace(input_space);
            LOG_RETURN_ERROR("converting to input map coordinates (a)", 
                         "MapScanGrid", false);
          }
          if (!img.is_fill)
            isin_buf_p->ds = img.s - (double)is_geo;
          else
            isin_buf_p->ds = 0.0;
      
          geo_p1++;
          geo_p2++;
          isin_buf_p++;
          is_geo++;

        } /* end for (is ... ) */
      } /* end for (il ... ) */

    } else {

      /* Multi-point mapping (one for each nested row) for ISIN grid with 
         2 (500m) and 4 (1km) rows nested */

      for (il = 0; il < this->size.l; il++) {
        geo_p1 = geoloc->geo[il];
        for (in = 0; in < geoloc->n_nest; in++)
          geo_isin_nest_p[in] = geoloc->geo_isin_nest[in][il + 1];
        isin_buf_p = this->isin_buf[il];
        is_geo = -this->extra_before.s;

        for (is = 0; is < this->size.s; is++) {
          for (in = 0; in < geoloc->n_nest; in++) {
            geo.lon = geo_p1->lon;
            geo.lat = geo_isin_nest_p[in]->lat;
            geo.is_fill = (geo_p1->is_fill == true  ||  
                           geo_isin_nest_p[in]->is_fill == true) ?
                           true : false;
            if (!ToSpace(input_space, &geo, &img_isin_nest[in])) {
              FreeSpace(input_space);
              LOG_RETURN_ERROR("converting to input map coordinates (b)", 
                           "MapScanGrid", false);
            }
            geo_isin_nest_p[in]++;
          }

          /* Compute the average sample location of the nested points */ 

          n_not_fill = 0;
          img.s = 0.0;
          for (in = 0; in < geoloc->n_nest; in++) {
            if (!img_isin_nest[in].is_fill) {
              n_not_fill++;
              img.s += img_isin_nest[in].s;
            }
          }
          if (n_not_fill < 1) img.is_fill = true;
          else {
            img.s /= (double)n_not_fill;
            img.is_fill = false;
          }

          if (!img.is_fill)
            isin_buf_p->ds = img.s - (double)is_geo;
          else
            isin_buf_p->ds = 0.0;
      
          geo_p1++;
          isin_buf_p++;
          is_geo++;

        } /* end for (is ... ) */
      } /* end for (il ... ) */
      
    }
  }
  
  /* Free input space grid data structure */

  if (!FreeSpace(input_space)) 
    LOG_RETURN_ERROR("freeing input space structure", "MapScanGrid",false);

  /* Map from geographic to output space coordinates */

  output_space = SetupSpace(output_space_def);
  if (output_space == (Space_t *)NULL) 
    LOG_RETURN_ERROR("setting up output space", "MapScanGrid", false);

  for (il = 0; il < this->size.l; il++) {
    geo_p1 = geoloc->geo[il];
    buf_p = this->buf[il];

    for (is = 0; is < this->size.s; is++) {
      if (!ToSpace(output_space, geo_p1, &buf_p->img)) {
        FreeSpace(output_space);
        LOG_RETURN_ERROR("converting to output map coordinates", 
                     "MapScanGrid", false);
      }

      geo_p1++;
      buf_p++;
    }
  }

  /* Special for input ISIN case: calculate virtual point in following line */

  if (this->isin_type != SPACE_NOT_ISIN) {

    /* Single point for ISIN grid with no nesting */

    if (geoloc->space_def.isin_type == SPACE_ISIN_NEST_1) {

      for (il = 0; il < this->size.l; il++) {
        geo_p1 = geoloc->geo[il];
        geo_p2 = geoloc->geo[il + 1];
        isin_buf_p = this->isin_buf[il];

        for (is = 0; is < this->size.s; is++) {
          geo.lon = geo_p1->lon;
          geo.lat = geo_p2->lat - EPS_LAT_ISIN;
          geo.is_fill = (geo_p1->is_fill == true  ||
                         geo_p2->is_fill == true ) ? true : false;
          if (!ToSpace(output_space, &geo, &isin_buf_p->vir_img)) {
            FreeSpace(output_space);
            LOG_RETURN_ERROR("converting to output map coordinates (b1)", 
                         "MapScanGrid", false);
          }
      
          geo_p1++;
          geo_p2++;
          isin_buf_p++;
        } /* end for (is ... ) */
      } /* end for (il ... ) */

    } else {

      /* Multi-point (one for each nested row) for ISIN grid with 
         2 (500m) and 4 (1km) rows nested */

      for (il = 0; il < this->size.l; il++) {
        geo_p1 = geoloc->geo[il];
        for (in = 0; in < geoloc->n_nest; in++)
          geo_isin_nest_p[in] = geoloc->geo_isin_nest[in][il + 1];
        isin_buf_p = this->isin_buf[il];

        for (is = 0; is < this->size.s; is++) {
          for (in = 0; in < geoloc->n_nest; in++) {
            geo.lon = geo_p1->lon;
            geo.lat = geo_isin_nest_p[in]->lat - EPS_LAT_ISIN;
            geo.is_fill = (geo_p1->is_fill == true  ||
                           geo_isin_nest_p[in]->is_fill == true) ?
                           true : false;
            if (!ToSpace(output_space, &geo, &img_isin_nest[in])) {
              FreeSpace(output_space);
              LOG_RETURN_ERROR("converting to output map coordinates (b2)",
                           "MapScanGrid", false);
            }
            geo_isin_nest_p[in]++;
          }

          /* Compute the average location of the nested points */ 

          n_not_fill = 0;
          isin_buf_p->vir_img.s = 0.0;
          isin_buf_p->vir_img.l = 0.0;
          for (in = 0; in < geoloc->n_nest; in++) {
            if (!img_isin_nest[in].is_fill) {
              n_not_fill++;
              isin_buf_p->vir_img.s += img_isin_nest[in].s;
              isin_buf_p->vir_img.l += img_isin_nest[in].l;
            }
          }
          if (n_not_fill < 1) isin_buf_p->vir_img.is_fill = true;
          else {
            isin_buf_p->vir_img.s /= (double)n_not_fill;
            isin_buf_p->vir_img.l /= (double)n_not_fill;
            isin_buf_p->vir_img.is_fill = false;
          }

          geo_p1++;
          isin_buf_p++;

        } /* end for (is ... ) */
      } /* end for (il ... ) */

    }
  }

  if (!FreeSpace(output_space)) 
    LOG_RETURN_ERROR("freeing output space structure", "MapScanGrid",
                          false);

  return true;
}

bool ExtendScan(Scan_t *this)
/* 
!C******************************************************************************

!Description: 'ExtendScan' extends the scan to allow for large kernels and
 to handle the scan overlap region.
 
!Input Parameters:
 this           'scan' data structure; the following fields are input:
                   extra_before, size, extra_after, buf[*][*].img

!Output Parameters:
 this           'input' data structure; the following field is modified:
                   buf[*][*].img
 (returns)      status:
                  'true' = okay (always returned)

!Team Unique Header:

 ! Design Notes:
   1. This interpolation may cause sample overlap in 
      areas with significant terrain relief and cause other artificats in 
      some spacial cases. These degenerative cases need to be handled in the 
      future.
   2. An error status is never returned.
   3. 'SetupScan' must be called before this routine is called.

!END****************************************************************************
*/
{
  int il1, il2;
  int is1, is2;
  int il, is;
  Img_coord_double_t dist;

  il1 = this->extra_before.l;
  il2 = this->size.l - this->extra_after.l;
  is1 = this->extra_before.s;
  is2 = this->size.s - this->extra_after.s;

  for (il = 0; il < il1; il++) {

    dist.l = (double)(il - il1);

    for (is = 0; is < is1; is++) {
      dist.s = (double)(is - is1);

      Extend2d(&this->buf[il][is].img, dist, 
               &this->buf[il1][is1].img,     
               &this->buf[il1][is1 + 1].img,
               &this->buf[il1 + 1][is1].img, 
               &this->buf[il1 + 1][is1 + 1].img);
    }

    for (is = is1; is < is2; is++) {
      Extend1d(&this->buf[il][is].img, dist.l, 
               &this->buf[il1][is].img, 
               &this->buf[il1 + 1][is].img);
    }

    for (is = is2; is < this->size.s; is++) {
      dist.s = (double)(is - (is2 - 2));
      Extend2d(&this->buf[il][is].img, dist, 
               &this->buf[il1][is2 - 2].img,     
               &this->buf[il1][is2 - 1].img,
               &this->buf[il1 + 1][is2 - 2].img, 
               &this->buf[il1 + 1][is2 - 1].img);
    }

  }

  for (il = il1; il < il2; il++) {

    for (is = 0; is < is1; is++) {
      dist.s = (double)(is - is1);
      Extend1d(&this->buf[il][is].img, dist.s, 
               &this->buf[il][is1].img, 
               &this->buf[il][is1 + 1].img);
    }

    for (is = is2; is < this->size.s; is++) {
      dist.s = (double)(is - (is2 - 2));
      Extend1d(&this->buf[il][is].img, dist.s, 
               &this->buf[il][is2 - 2].img, 
               &this->buf[il][is2 - 1].img);
    }

  }

  for (il = il2; il < this->size.l; il++) {

    dist.l = (double)(il - (il2 - 2));
    for (is = 0; is < is1; is++) {
      dist.s = (double)(is - is1);
      Extend2d(&this->buf[il][is].img, dist, 
               &this->buf[il2 - 2][is1].img, 
               &this->buf[il2 - 2][is1 + 1].img,
               &this->buf[il2 - 1][is1].img, 
               &this->buf[il2 - 1][is1 + 1].img);
    }

    for (is = is1; is < is2; is++) {
      Extend1d(&this->buf[il][is].img, dist.l, 
               &this->buf[il2 - 2][is].img, 
               &this->buf[il2 - 1][is].img);
    }

    for (is = is2; is < this->size.s; is++) {
      dist.s = (double)(is - (is2 - 2));
      Extend2d(&this->buf[il][is].img, dist, 
               &this->buf[il2 - 2][is2 - 2].img, 
               &this->buf[il2 - 2][is2 - 1].img,
               &this->buf[il2 - 1][is2 - 2].img, 
               &this->buf[il2 - 1][is2 - 1].img);
    }

  }

  return true;
}


bool GetScanInput(Scan_t *this, Input_t *input, int il, int nl)
/* 
!C******************************************************************************

!Description: 'GetScanInput' reads a scan of input data.
 
!Input Parameters:
 this           'scan' data structure; the following field is modified:
                  extra_before 
 input          'input' data structure; the following fields are input:
                  open, sds.rank, extra_dim, dim, sds.id
 il             start line number
 nl             number of lines to read

!Output Parameters:
 this           'scan' data structure; the following field is modified:
                  buf[*][*].v
 input          'input' data structure; the following field is modified:
                  buf
 (returns)      status:
                  'true' = okay
                  'false' = error return

!Team Unique Header:

 ! Design Notes:
   1. The only input HDF data types supported are CHAR8, UINT8, INT16 and
      UINT16.
   2. All input data types are converted to floating point values.
   3. An error status is returned when:
       a. the input file is not open for access
       b. either the start and end line numbers are not in the valid range
       c. there is an error reading the SDS
       d. the input data type is invalid.
   4. Error messages are handled with the 'LOG_RETURN_ERROR' macro.
   5. 'SetupScan' and 'OpenInput' must be called before this routine is 
      called.

!END****************************************************************************
*/
{
  int32 start[MYHDF_MAX_RANK], nval[MYHDF_MAX_RANK];
  int ir;
  int is;
  int il_r;

  Scan_buf_t *scan_buf_p;

  if (!input->open)
    LOG_RETURN_ERROR("file not open", "GetScanInput", false);

  if (il < 0  ||  (il + nl) > input->size.l)
    LOG_RETURN_ERROR("invalid scan number", "GetScanInput", false);

  for (ir = 0; ir < input->sds.rank; ir++) {
    start[ir] = input->extra_dim[ir];
    nval[ir] = 1;
  }
  nval[input->dim.s] = input->scan_size.s;

  for (il_r = 0; il_r < nl; il_r++) {

    start[input->dim.l] = il++; 

    if (SDreaddata(input->sds.id, start, NULL, nval, 
                   input->buf.val_void) == HDF_ERROR)
      LOG_RETURN_ERROR("reading input", "GetScanInput", false);

    scan_buf_p = this->buf[il_r + this->extra_before.l];
    scan_buf_p += this->extra_before.s;

    switch (input->sds.type) {
      case DFNT_CHAR8:
        for (is = 0; is < input->scan_size.s; is++) {
          scan_buf_p->v = (double)input->buf.val_char8[is];
          scan_buf_p++;
        }
        break;
      case DFNT_UINT8:
        for (is = 0; is < input->scan_size.s; is++) {
          scan_buf_p->v = (double)input->buf.val_uint8[is];
          scan_buf_p++;
        }
        break;
      case DFNT_INT8:
        for (is = 0; is < input->scan_size.s; is++) {
          scan_buf_p->v = (double)input->buf.val_int8[is];
          scan_buf_p++;
        }
        break;
      case DFNT_INT16:
        for (is = 0; is < input->scan_size.s; is++) {
          scan_buf_p->v = (double)input->buf.val_int16[is];
          scan_buf_p++;
        }
        break;
      case DFNT_UINT16:
        for (is = 0; is < input->scan_size.s; is++) {
          scan_buf_p->v = (double)input->buf.val_uint16[is];
          scan_buf_p++;
        }
        break;
      case DFNT_INT32:
        for (is = 0; is < input->scan_size.s; is++) {
          scan_buf_p->v = (double)input->buf.val_int32[is];
          scan_buf_p++;
        }
        break;
      case DFNT_UINT32:
        for (is = 0; is < input->scan_size.s; is++) {
          scan_buf_p->v = (double)input->buf.val_uint32[is];
          scan_buf_p++;
        }
        break;
      default:
        LOG_RETURN_ERROR("invalid data type", "GetScanInput", false);
    }

  }

  return true;
}

/* Constant for PointInTriangle */

#define EPS_TRIANGLE (1e-20)

bool PointInTriangle(Img_coord_double_t *e0, Img_coord_double_t *e1, 
                     Img_coord_double_t *e2, Img_coord_double_t *d)
/* 
!C******************************************************************************

!Description: 'PointInTriangle' determines if a point lies within a triangle 
 and if it is, returns the location within the triangle.

!Input Parameters:
 e0             point to test
 e1             trinagle vertex 1
 e2             triangle vertex 2

!Output Parameters:
 d              if point is in the triangle, the location within the triangle, 
                otherwise, (0.0, 0.0); the location is a function of the 
                two vertexes:
                  'e0 = (d.l * e1) + (d.s * e2)'
 (returns)      flag indicating whether the point is in the triangle:
                  'true' = in the triangle
                  'false' = not in triangle

!Team Unique Header:

 ! Design Notes:
   1. The third triangle vertex (not given) is located at the 
      origin, (0.0, 0.0).
   2. Reference: Dr. Doob's Journal, August 2000, pp. 32-36.
   3. Notation:
       a. '(x, y)' in the journal correspond to '(l, s)' in this function.
       b. 'd' in the journal corresponds to 'r' in this funciton.
   4. A value on the edge of the triangle is considered "inside" the triangle.
   5. A point is concidered outside degenerative triangles with zero area
      (within 'EPS_TRIANGLE').
!END****************************************************************************
*/
{
  double r;
  double u, v;

  d->s = d->l = 0.0;

  if (e1->l <  EPS_TRIANGLE  &&  
      e1->l > -EPS_TRIANGLE) {
    if (e2->l <  EPS_TRIANGLE  &&  
        e2->l > -EPS_TRIANGLE) {
       return false;
    }
    u = e0->l / e2->l;
    if (u < 0.0  ||  u > 1.0) {
       return false;
    }
    if (e1->s <  EPS_TRIANGLE  &&  
        e1->s > -EPS_TRIANGLE) {
       return false;
    }
    v = (e0->s - (e2->s * u)) / e1->s;
    if (v < 0.0) {
       return false;
    }
  } else {
    r = (e2->s * e1->l) - (e2->l * e1->s);
    if (r <  EPS_TRIANGLE  &&  
        r > -EPS_TRIANGLE) {
       return false;
     }
    u = ((e0->s * e1->l) - (e0->l * e1->s)) / r;
    if (u < 0.0  ||  u > 1.0) {
       return false;
     }
    v = (e0->l - (e2->l * u)) / e1->l;
    if (v < 0.0) {
       return false;
    }
  }

  if ((u + v) > 1.0) {
     return false;
  }

  d->s = u;
  d->l = v;
  return true;
}


bool ProcessScan (Scan_t*        this, 
                  Kernel_t*      kernel, 
                  Patches_t*     patches, 
                  int            nl,
                  Kernel_type_t  kernel_type)
/* 
!C******************************************************************************

!Description: 'ProcessScan' processes a scan of input data and updates all of
 the output patches the scan overlaps.
 
!Input Parameters:
 this           'scan' data structure; the following fields are input:
                  isin_type, size, extra_before, extra_after, buf, isin_buf
 kernel         'kernel' data structure; the following fields are input:
                  before, after, delta_inv, l, s, 
 patches        'patches' data structure; the following fields are input:
                  size, loc, (loc_p)->u.pntr, (mem_p)->sum, (mem_p)->weight,
                  nmem, nmem_alloc, nmem_max, nnull, null_list, nused, 
                  used_list, (loc_p)->status, (mem_p)->prev, (mem_p)->next
 nl             number of lines to process

!Output Parameters:
 patches        'patches' data structure; the following fields are modified:
                  (mem_p)->sum, (mem_p)->weight, (mem_p)->ntouch, 
                  nmem, nnull, null_list, nused, used_list, mem[*], 
                  (mem_p)->prev, (mem_p)->next, (mem_p)->loc
                  (loc_p)->status, (loc_p)->u.pntr
 (returns)      status:
                  'true' = okay
                  'false' = error return

!Team Unique Header:

 ! Design Notes:
   1. An error status is returned when:
       a. there is a memory allocation error.
       b. there is an error initializing a new patch.
   2. Error messages are handled with the 'LOG_RETURN_ERROR' macro.
   3. 'SetupScan', 'SetupKernel' and 'SetupPatches' must be called before 
      this routine is called.

!END****************************************************************************
*/
{
    double *ds;
    int il_in1, il_in2;
    int is_in1, is_in2;
    int il_in, is_in;
    int il_area1, il_area2;
    int il_kernel1, is_kernel1;
    Img_coord_double_t *p0, *p1, *p2, *p3;
    double d;
    int il_out1, il_out2;
    int is_out1, is_out2;
    int is_area1, is_area2;
    int il_area, is_area;
    Img_coord_double_t e0_ul, e1_ul, e2_ul;
    Img_coord_double_t e0_lr, e1_lr, e2_lr;
    Img_coord_double_t del;
    int il_patch, is_patch;
    int il_rel, is_rel;
    int il_out, is_out;
    Patches_loc_t *loc_p;
    Patches_mem_t *mem_p;
    double *sum_p, *weight_p, *nn_wt_p;
    int il_delta, is_delta;
    int il_kernel, is_kernel;
    Img_coord_double_t w;
    double w1;
    Scan_buf_t *buf_p;
    double del_s1;
    int is_extra, is_area1a, is_area2a;
    int fill_count;
    int half_kernel_ttl;
    bool fill;

/* #define DEBUG */
#ifdef DEBUG
    printf("==================================================\n\n" );
    printf("initial: patches: %ld in mem  %ld used  %ld null\n", 
           (long)patches->nmem, (long)patches->nused, (long)patches->nnull);
#endif

    int do_old_nn = 0;
    char* chk_old_nn;

    if ((chk_old_nn = getenv("OLDNN")))
    {
       do_old_nn = (! strcmp(chk_old_nn, "DO_OLDNN"));
    }


    /*
    -------------------------------------------------------
    Allocate memory for cummulative delta-sample for 
    special input ISIN case
    -------------------------------------------------------*/
    ds = (double*)NULL;

    if (this->isin_type != SPACE_NOT_ISIN) 
    {
        ds = (double*)calloc((size_t)this->size.l, sizeof(double));

        if (ds == (double*)NULL) 
        {
            LOG_RETURN_ERROR("allocating cummulative delta-sample array", 
                             "ProcessScan", false);
        }
    }


    /*
    -------------------------------------------------------
    First line/sample and last line/sample in scan
    -------------------------------------------------------*/
    il_in1 = this->extra_before.l;
    il_in2 = this->extra_before.l + nl;
    is_in1 = this->extra_before.s;
    is_in2 = this->size.s - this->extra_after.s;

    /*
    -------------------------------------------------------
    For each line in extended scan
    -------------------------------------------------------*/
    for (il_in = 0; il_in < (this->size.l - 1); il_in++) 
    {
        /*
        -------------------------------------------------------
        Determine line extent in input and kernel start
        -------------------------------------------------------*/
        il_area1 = il_in - kernel->before.l;

        if (il_area1 < il_in1) 
            il_area1 = il_in1;

        il_area2 = il_in + kernel->after.l + 1;

        if (il_area2 > il_in2) 
            il_area2 = il_in2;

        il_kernel1 = (il_area1 - il_in) + kernel->before.l;

        /*
        -------------------------------------------------------
        For each sample in extended scan
        -------------------------------------------------------*/
        for (is_in = 0; is_in < (this->size.s - 1); is_in++) 
        {
            /*
            -------------------------------------------------------
            Get location in output space of the input point and 
            three others; For input ISIN case, lower two are 
            virtural
            -------------------------------------------------------*/
            p0 = &this->buf[il_in][is_in    ].img;
            p1 = &this->buf[il_in][is_in + 1].img;

            if (this->isin_type == SPACE_NOT_ISIN) 
            {
                p2 = &this->buf[il_in + 1][is_in + 1].img;
                p3 = &this->buf[il_in + 1][is_in    ].img;
            } 
            else 
            {
                p2 = &this->isin_buf[il_in][is_in + 1].vir_img;
                p3 = &this->isin_buf[il_in][is_in    ].vir_img;
            }

            /*
            -------------------------------------------------------
            Determine the maximum extent in ouput space of the 
            four points

            Minimum output line
            -------------------------------------------------------*/
            if (p0->is_fill || p2->is_fill || p1->is_fill || p3->is_fill) 
            {
                  continue;
            }
       
            d = p0->l;
            if (d > p1->l) 
                d = p1->l;

            if (d > p2->l) 
                d = p2->l;

            if (d > p3->l) 
                d = p3->l;

            il_out1 = (int)d;

            if (il_out1 >= patches->size.l) 
            {
               continue; 
            }

            if (d < 0.0) 
                il_out1 = 0;

            /*
            -------------------------------------------------------
            Maximum output line plus one
            -------------------------------------------------------*/
            d = p0->l;

            if (d < p1->l) 
                d = p1->l;

            if (d < p2->l) 
                d = p2->l;

            if (d < p3->l) 
                d = p3->l;

            if (d < 0.0) 
            {
               continue;
            }

            il_out2 = 1 + (int)d;

            if (il_out2 > patches->size.l) 
                il_out2 = patches->size.l;

            if (il_out1 >= il_out2) 
            {
              continue; 
            }

            /*
            -------------------------------------------------------
            Minimum output sample
            -------------------------------------------------------*/
            d = p0->s;

            if (d > p1->s) 
                d = p1->s;

            if (d > p2->s) 
                d = p2->s;

            if (d > p3->s) 
                d = p3->s;

            is_out1 = (int)d;

            if (is_out1 >= patches->size.s) 
            {
              continue; 
            }

            if (d < 0.0) 
                is_out1 = 0;

            /*
            -------------------------------------------------------
            Maximum output sample plus one
            -------------------------------------------------------*/
            d = p0->s;

            if (d < p1->s) 
                d = p1->s;

            if (d < p2->s) 
                d = p2->s;

            if (d < p3->s) 
                d = p3->s;

            if (d < 0.0) 
            {
              continue;
            }

            is_out2 = 1 + (int)d;
            if (is_out2 > patches->size.s) 
                is_out2 = patches->size.s;

            if (is_out1 >= is_out2) 
            {
                continue; 
            }

            /*
            -------------------------------------------------------
            Determine sample extent in input and kernel start
            -------------------------------------------------------*/
            is_area1 = is_in - kernel->before.s;

            if (is_area1 < is_in1) 
                is_area1 = is_in1;

            is_area2 = is_in + kernel->after.s + 1;

            if (is_area2 > is_in2) 
                is_area2 = is_in2;

            /*
            -------------------------------------------------------
            Cummulative delta-sample for special input ISIN case 
            -------------------------------------------------------*/
            if (this->isin_type != SPACE_NOT_ISIN) 
            {
                ds[il_in] = 0.0;

                for (il_area = il_in - 1; il_area >= il_area1; il_area--) 
                   ds[il_area] = ds[il_area + 1] - this->isin_buf[il_area][is_in].ds;

                for (il_area = il_in + 1; il_area < il_area2; il_area++) 
                   ds[il_area] = ds[il_area - 1] + this->isin_buf[il_area - 1][is_in].ds;
            }

            /*
            -------------------------------------------------------
            For each potential output pixel

            Set up sides of upper left and lower right triangles
            -------------------------------------------------------*/
            e1_ul.l = p3->l - p0->l;
            e1_ul.s = p3->s - p0->s;
            e2_ul.l = p1->l - p0->l;
            e2_ul.s = p1->s - p0->s;
            
            e1_lr.l = p1->l - p2->l;
            e1_lr.s = p1->s - p2->s;
            e2_lr.l = p3->l - p2->l;
            e2_lr.s = p3->s - p2->s;

            /*
            -------------------------------------------------------
            Loop through the output lines
            -------------------------------------------------------*/
            for (il_out = il_out1; il_out < il_out2; il_out++) 
            {
                il_patch = il_out / NLINE_PATCH;
                il_rel   = il_out % NLINE_PATCH;

                e0_ul.l = (double)il_out - p0->l;
                e0_lr.l = (double)il_out - p2->l;

                /*
                -------------------------------------------------------
                Loop through the output samples
                -------------------------------------------------------*/
                for (is_out = is_out1; is_out < is_out2; is_out++) 
                {
                    /*
                    -------------------------------------------------------
                    Determine valid output pixels that are inside 
                    triangle(s) and calculate offsets
                    -------------------------------------------------------*/

                    /*
                    -------------------------------------------------------
                    Check the upper left triangle
                    -------------------------------------------------------*/
                    e0_ul.s = (double)is_out - p0->s;

                    if (!PointInTriangle(&e0_ul, &e1_ul, &e2_ul, &del)) 
                    {
                        /*
                        -------------------------------------------------------
                        No intersection, so check the lower right triangle
                        -------------------------------------------------------*/
                        e0_lr.s = (double)is_out - p2->s;

                        if (!PointInTriangle(&e0_lr, &e1_lr, &e2_lr, &del)) 
                        {
                            continue;
                        }

                        if (del.l == 0.0  ||  del.s == 0.0) 
                        {
#ifdef DEBUG_ZEROS
                            printf("zero case: del.l %lf  del.s %lf", del.l, del.s);
                            printf("  il_out %d  is_out %d\n", il_out, is_out);
#endif
                            continue;
                        }

                        del.l = (double)1.0 - del.l;
                        del.s = (double)1.0 - del.s;
                    }

                    /*
                    -------------------------------------------------------
                    For this valid output pixel
                    -------------------------------------------------------*/
                    is_patch = is_out / NSAMPLE_PATCH;
                    is_rel   = is_out % NSAMPLE_PATCH;

                    loc_p    = &patches->loc[il_patch][is_patch];

                    if (loc_p->status != PATCH_IN_MEM) 
                    {
                        if (!InitPatchInMem(patches, il_patch, is_patch)) 
                        {
                            if (ds == (double *)NULL) 
                                free(ds);

                            LOG_RETURN_ERROR("initializing patch in memory",
                                             "ProcessScan", false);
                        } 
                    }

                    mem_p    = loc_p->u.pntr;
                    sum_p    = &mem_p->sum[il_rel][is_rel];
                    weight_p = &mem_p->weight[il_rel][is_rel];
                    nn_wt_p  = &mem_p->nn_wt[il_rel][is_rel];

                    /*
                    -------------------------------------------------------
                    Weight actual area pixels and sum weights
                    -------------------------------------------------------*/
                    il_delta = (int)((del.l * kernel->delta_inv.l) + (double)0.5);

                    if (this->isin_type == SPACE_NOT_ISIN) 
                    {
                        /*
                        -------------------------------------------------------
                        Normal case
                        -------------------------------------------------------*/
                        is_kernel1 = (is_area1 - is_in) + kernel->before.s;
                        is_delta   = (int)((del.s * kernel->delta_inv.s) + (double)0.5);

                        /*
                        -------------------------------------------------------
                        Set the fill count to 0 and determine how many pixels 
                        are in this kernel. Also determine half of the kernel 
                        total. 
                        
                        NOTE: il_area2 and is_area2 already have one extra 
                              for the 'for' loop, so don't add one for their 
                              total
                        -------------------------------------------------------*/
                        fill_count = 0;
                        fill       = false;

                        if (kernel_type == NN)
                            half_kernel_ttl = 1;
                        else 
                        if (kernel_type == BL)
                            half_kernel_ttl = 2;
                        else 
                        if (kernel_type == CC)
                            half_kernel_ttl = 8;
                        else
                            half_kernel_ttl = 0;  /* invalid option */

                        /*
                        -------------------------------------------------------
                        Loop through the lines in the kernel
                        -------------------------------------------------------*/
                        il_kernel = il_kernel1;

                        for (il_area = il_area1; il_area < il_area2; il_area++) 
                        {
                            w.l = kernel->l[il_delta][il_kernel++];

                            /*
                            -------------------------------------------------------
                            Loop through the samples in the kernel
                            -------------------------------------------------------*/
                            is_kernel = is_kernel1;
                            buf_p     = &this->buf[il_area][is_area1];
               
                            for (is_area = is_area1; is_area < is_area2; is_area++) 
                            {
                                w.s = kernel->s[is_delta][is_kernel++];
                                w1  = w.l * w.s;

                                /*
                                -------------------------------------------------------
                                If the pixel is a background fill value or if the 
                                weight value is 0.0 (NN sets up a 2x2 array, but half 
                                of the weights are 0s so we will skip the weight 
                                values of 0.0)
                                -------------------------------------------------------*/
                                if (((double)buf_p->v == (double)patches->fill_value) && 
                                    (w1 != 0.0)) 
                                {
                                    /*
                                    -------------------------------------------------------
                                    Otherwise use the pixel as normal in the resampling
                                    process
                                    -------------------------------------------------------*/
                                    fill_count++;

                                    /*
                                    -------------------------------------------------------
                                    If the number of fill values is >= 50% of the pixels
                                    in the kernel then assign a fill value to this pixel
                                    -------------------------------------------------------*/
                                    if (fill_count >= half_kernel_ttl) 
                                    {
                                        *sum_p    = patches->fill_value;
                                        *weight_p = MIN_WEIGHT * 0.5; /* < min weight means fill */
                                        fill      = true;

                                        break;
                                    }
                                    else 
                                    {
                                        /*
                                        -------------------------------------------------------
                                        Move to the next pixel in the buffer
                                        -------------------------------------------------------*/
                                        buf_p++;
                                    }
                                } 
                                else 
                                {
                                    /*
                                    ------------------------------------------------
                                    If this is nearest neighbor then just keep the 
                                    pixel that's nearest to the output pixel, and 
                                    thus has the greatest weight.
                                    ------------------------------------------------*/
                                    if ((kernel_type == NN) && (! do_old_nn))
                                    {
                                        if (w1 > *nn_wt_p)
                                        {
                                            *sum_p    = buf_p->v;
                                            *weight_p = 1;
                                            *nn_wt_p  = w1;
                                        }
                                    }
                                    else
                                    {
                                        *sum_p    += buf_p->v * w1;
                                        *weight_p += w1;
                                    }

                                    buf_p++;
                                }

                            } /* for (is_area ... */


                            /*
                            -------------------------------------------------------
                            If this is a fill pixel then break out of the loop
                            -------------------------------------------------------*/
                            if (fill) 
                            {
                                break;
                            }

                        } /* for (il_area ... */
                    } 
                    else 
                    {
                        /*
                        -------------------------------------------------------
                        Special handling for input ISIN case
                        -------------------------------------------------------*/
                        il_kernel = il_kernel1;

                        for (il_area = il_area1; il_area < il_area2; il_area++) 
                        {
                            w.l = kernel->l[il_delta][il_kernel++];

                            del_s1   = del.s + ds[il_area];
                            is_extra = (int)del_s1;

                            if (is_extra > del_s1) 
                                is_extra--;
                            
                            is_delta = (int)(((del_s1 - (double)is_extra) * 
                                               kernel->delta_inv.s) + (double)0.5);

                            is_area1a = is_area1 + is_extra;

                            if (is_area1a <  is_in1) 
                                is_area1a = is_in1;

                            if (is_area1a >= is_in2) 
                                is_area1a = is_in2 - 1;
                            
                            is_area2a = is_area2 + is_extra;

                            if (is_area2a <= is_in1) 
                                is_area2a = is_in1 + 1;

                            if (is_area2a >  is_in2) 
                                is_area2a = is_in2;
                            
                            is_kernel = (is_area1a - (is_in + is_extra)) + kernel->before.s;

                            buf_p = &this->buf[il_area][is_area1a];

                            for (is_area = is_area1a; is_area < is_area2a; is_area++) 
                            {
                                w.s = kernel->s[is_delta][is_kernel++];
                                w1  = w.l * w.s;

                                *sum_p    += buf_p->v * w1;
                                *weight_p += w1;

                                buf_p++;

                            } /* for (is_area ... */

                        } /* for (il_area ... */

                    }

                    /*
                    -------------------------------------------------------
                    Patch has been touched
                    -------------------------------------------------------*/
                    mem_p->ntouch = NSCAN_TOUCH;

                } /* for (is_out ... */

            } /* for (il_out ... */

        } /* for (is_in ... */

    } /* for (il_in ... */

    
    /*
    -------------------------------------------------------
    Free delta-sample array
    -------------------------------------------------------*/
    if (this->isin_type != SPACE_NOT_ISIN) 
        free(ds);

    return true;

} /* ProcessScan */


