/*
!C****************************************************************************

!File: geoloc.c
  
!Description: Functions for reading data from the geolocation data file for 
 swath input data, and for generating geolocation data for grid input data.

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
      phone: 301-614-5508               Lanham, MD 20770  
  
 ! Design Notes:
   1. The following public functions handle the geolocation data:

	OpenGeolocSwath - Open a geolocation swath data file for input.
	SetupGeolocGrid - Setup an input grid space.
	CloseGeoloc - Close the input geolocation file.
	FreeGeoloc - Free the 'geoloc' data structure memory.
	GetGeolocSwath - Read a scan of geolocation data and remap it to 
	  the output product space.
	CloseInput - Close the input file.
	FreeOutput - Free the 'input' data structure memory.

   2. Either 'OpenGeolocSwath' or 'SetupGeolocGrid' must be called before any 
      of the other routines.
   3. The 'OpenGeolocSwath', 'GetGeolocSwath' and 'CloseGeoloc' routines must 
      be used together and can not be used with 'SetupGeolocGrid'.
   4. The 'SetupGeolocGrid' can not be used with 'OpenGeolocSwath', 
      'GetGeolocSwath' and 'CloseGeoloc'.
   5. 'FreeGeoloc' should be used to free the 'geoloc' data structure.
   6. The only input geolocation file type supported is HDF.
   7. The input geolocation SDSs are named 'Latitude' and 'Longitude'.
   8. An input '_FillValue' is required for the geolocation file.

!END****************************************************************************
*/

#include <stdlib.h>
#include "geoloc.h"
#include "input.h"
#include "myerror.h"
#include "mystring.h"
#include "const.h"
#include "hdf.h"
#include "mfhdf.h"

/* Constants */

#define GEOLOC_LAT_SDS "Latitude"
#define GEOLOC_LON_SDS "Longitude"
#define FILL_ATTR_NAME "_FillValue"

Img_coord_double_t band_offset_gen[NBAND_OFFSET_GEN] = {
  {0.0, 0.0, false}, {0.0, 0.0, false}, {0.0, 0.0, false}, {0.0, 0.0, false}, 
  {0.0, 0.0, false}, {0.0, 0.0, false}, {0.0, 0.0, false}, {0.0, 0.0, false},
  {0.0, 0.0, false}, {0.0, 0.0, false}, {0.0, 0.0, false} 
};


Geoloc_t *OpenGeolocSwath(char *file_name)
/* 
!C******************************************************************************

!Description: 'OpenGeolocSwath' sets up the 'geoloc' data structure and opens 
 the input geolocation file for read access.
 
!Input Parameters:
 file_name      geolocation file name

!Output Parameters:
 (returns)      'geoloc' data structure or NULL when an error occurs

!Team Unique Header:

 ! Design Notes:
   1. When 'OpenGeolocSwath' returns, the file is open for HDF access and the 
      SDS is open for access.
   2. An error status is returned when:
       a. duplicating strings is not successful
       b. errors occur when opening the input HDF file
       c. errors occur when opening the SDSs for read access
       d. errors occur when reading SDS dimensions or attributes
       e. the ranks of the SDSs are not 2
       f. the number of lines is not an integral multiple of the size of 
          a MODIS 1km scan ('NDET_1KM_MODIS')
       g. the dimensions of the latitude and longitude arrays don't match
       h. memory allocation is not successful
   3. Error messages are handled with the 'LOG_RETURN_ERROR' macro.
   4. 'FreeGeoloc' should be called to deallocate memory used by the 
      'geoloc' data structures.
   5. 'CloseGeoloc' should be called after all of the data is written and 
      before the 'geoloc' data structure memory is released.

!END****************************************************************************
*/
{
  Geoloc_t *this;
  Myhdf_sds_t *sds;
  int i, ir, ib, ib1;
  int ir1;
  size_t n;
  Img_coord_double_t *img_p;
  char *error_string = (char *)NULL;
  double fill[MYHDF_MAX_NATTR_VAL];
  Myhdf_attr_t attr;

  /* Create the Geoloc data structure */

  this = (Geoloc_t *)malloc(sizeof(Geoloc_t));
  if (this == (Geoloc_t *)NULL) 
    LOG_RETURN_ERROR("allocating Geoloc structure", "OpenGeolocSwath", 
                 (Geoloc_t *)NULL);

  /* Populate the data structure */

  this->geoloc_type = SWATH_GEOLOC;

  this->file_name = DupString(file_name);
  if (this->file_name == (char *)NULL) {
    free(this);
    LOG_RETURN_ERROR("duplicating file name", "OpenGeolocSwath",
                          (Geoloc_t *)NULL);
  }

  this->sds_lat.name = DupString(GEOLOC_LAT_SDS);
  if (this->sds_lat.name == (char *)NULL) {
    free(this->file_name);
    free(this);
    LOG_RETURN_ERROR("duplicating sds name", "OpenGeolocSwath",
                          (Geoloc_t *)NULL);
  }

  this->sds_lon.name = DupString(GEOLOC_LON_SDS);
  if (this->sds_lon.name == (char *)NULL) {
    free(this->sds_lat.name);
    free(this->file_name);
    free(this);
    LOG_RETURN_ERROR("duplicating sds name", "OpenGeolocSwath",
                          (Geoloc_t *)NULL);
  }

  /* Set up the band offsets */

  for (ib = 0; ib < NBAND_MODIS; ib++)
    this->band_offset[ib].l = this->band_offset[ib].s = 0.0;
  for (ib = 0; ib < NBAND_OFFSET_GEN; ib++) {
    ib1 = ib + NBAND_MODIS;
    this->band_offset[ib1].l = band_offset_gen[ib].l;
    this->band_offset[ib1].s = band_offset_gen[ib].s;
  }

  /* Open file for SD access */

  this->sds_file_id = SDstart((char *)file_name, DFACC_RDONLY);
  if (this->sds_file_id == HDF_ERROR) {
    free(this->sds_lat.name);
    free(this->sds_lon.name);
    free(this->file_name);
    free(this);  
    LOG_RETURN_ERROR("opening geolocation file", "OpenGeolocSwath", 
                 (Geoloc_t *)NULL); 
  }
  this->open = true;

  /* Open the latitude and longitude SDSs */

  for (i = 0; i < 2; i++) {
    sds = (i == 1)  ?  &this->sds_lat  :  &this->sds_lon;

    /* Get SDS information and start SDS access */

    if (!GetSDSInfo(this->sds_file_id, sds)) {
      SDend(this->sds_file_id);
      free(this->sds_lat.name);
      free(this->sds_lon.name);
      free(this->file_name);
      free(this);
      LOG_RETURN_ERROR("getting sds info", "OpenGeolocSwath",
                            (Geoloc_t *)NULL);
    }

    /* Check rank and type */

    if (sds->rank != 2) error_string = "invalid rank";
    else if (sds->type != DFNT_FLOAT32) error_string = "invalid type";

    /* Get dimensions */

    if (error_string == (char *)NULL) {
      for (ir = 0; ir < sds->rank; ir++) {
        if (!GetSDSDimInfo(sds->id, &sds->dim[ir], ir)) {
          for (ir1 = 0; ir1 < ir; ir1++) free(sds->dim[ir1].name);
          error_string = "getting dimension";
        }
      }
    }

    /* Get fill value */

    if (error_string == (char *)NULL) {
      attr.name = FILL_ATTR_NAME;
      if (!GetAttrDouble(sds->id, &attr, fill))
        error_string = "getting fill value";
      else {
        if (i == 1) this->lat_fill = (float32)fill[0];
        else this->lon_fill = (float32)fill[0];
      }
    }
    
    if (error_string != (char *)NULL) {
      SDendaccess(sds->id);
      if (i > 0) {
        sds = &this->sds_lat;
        for (ir = 0; ir < sds->rank; ir++) free(sds->dim[ir].name);
        SDendaccess(sds->id);
      }
      SDend(this->sds_file_id);
      free(this->sds_lat.name);
      free(this->sds_lon.name);
      free(this->file_name);
      free(this);
      LOG_RETURN_ERROR(error_string, "OpenGeolocSwath", (Geoloc_t *)NULL);
    }

  }

  /* Check dimensions */

  this->scan_size.l = NDET_1KM_MODIS;
  this->scan_size.s = this->sds_lat.dim[1].nval;
  this->scan_size_geo.l = this->scan_size.l;
  this->scan_size_geo.s = this->scan_size.s;
  this->size.l = this->sds_lat.dim[0].nval;
  this->size.s = this->sds_lat.dim[1].nval;
  this->nscan = this->size.l / this->scan_size.l;

  if ((this->nscan * this->scan_size.l) != this->size.l) 
    error_string = "not an integral number of scans";
  else if (this->size.l != this->sds_lon.dim[0].nval)
    error_string = "number of lines don't match";
  else if (this->size.s != this->sds_lon.dim[1].nval)
    error_string = "number of samples don't match";

  /* Allocate buffers */

  this->n_nest = -1;

  this->img = (Img_coord_double_t **)NULL;
  this->geo = (Geo_coord_t **)NULL;
  this->lat_buf = (float32 *)NULL;
  this->lon_buf = (float32 *)NULL;

  if (error_string == (char *)NULL) {
    this->img = (Img_coord_double_t **)calloc((size_t)this->scan_size.l, 
                                             sizeof(Img_coord_double_t *));
    if (this->img == (Img_coord_double_t **)NULL)
      error_string = "allocating image location buffer array";
  }

  if (error_string == (char *)NULL) {
    n = (size_t)(this->scan_size.l * this->scan_size.s);
    img_p = (Img_coord_double_t *)calloc(n, sizeof(Img_coord_double_t));
    if (img_p == (Img_coord_double_t *)NULL) 
      error_string = "allocating image location buffer";
  }

  if (error_string == (char *)NULL) {
    for (i = 0; i < this->scan_size.l; i++) {
      this->img[i] = img_p;
      img_p += this->scan_size.s;
    }

    this->lat_buf = (float32 *)calloc(this->scan_size.s, sizeof(float32));
    if (this->lat_buf == (float32 *)NULL) 
      error_string = "allocating latitude buffer";
  }

  if (error_string == (char *)NULL) {
    this->lon_buf = (float32 *)calloc(this->scan_size.s, sizeof(float32));
    if (this->lon_buf == (float32 *)NULL) 
      error_string = "allocating longitude buffer";
  }

  if (error_string != (char *)NULL) {
    if (this->lon_buf != (float32 *)NULL) free(this->lon_buf);
    if (this->lat_buf != (float32 *)NULL) free(this->lat_buf);
    if (this->img != (Img_coord_double_t **)NULL) {
      if (this->img[0] != (Img_coord_double_t *)NULL) free(this->img[0]);
      free(this->img);
    }
    for (i = 0; i < 2; i++) {
      sds = (i == 1) ? &this->sds_lat : &this->sds_lon;
      for (ir = 0; ir < sds->rank; ir++) free(sds->dim[ir].name);
      SDendaccess(sds->id);
    }
    SDend(this->sds_file_id);
    free(this->sds_lat.name);
    free(this->sds_lon.name);
    free(this->file_name);
    free(this);
    LOG_RETURN_ERROR(error_string, "OpenGeolocSwath", (Geoloc_t *)NULL);
  }

  return this;
}


Geoloc_t *SetupGeolocGrid(Space_def_t *space_def, Input_t *input, 
                          Kernel_t *kernel)
/* 
!C******************************************************************************

!Description: 'SetupGeolocGrid' sets up the 'geoloc' data structure for an 
 input grid.
 
!Input Parameters:
 space_def      input grid space definition; the following fields are input:
                  proj_num, proj_param[*], pixel_size, 
                  ul_corner, ul_corner_set, img_size, zone, sphere, zone_set, 
		  isin_type
 input		'input' data structure; the following fields are input:
                  size, scan_size, nscan
 kernel         'kernel' data structure; the following fields are input:
                  after, before

!Output Parameters:
 (returns)      'geoloc' data structure or NULL when an error occurs

!Team Unique Header:

 ! Design Notes:
   1. An error status is returned when:
       a. memory allocation is not successful
   2. Error messages are handled with the 'LOG_RETURN_ERROR' macro.
   3. 'FreeGeoloc' should be called to deallocate memory used by the 
      'geoloc' data structures.

!END****************************************************************************
*/
{
  Geoloc_t *this;
  int ip, i, ir, ib, ib1;
  size_t n;
  Myhdf_sds_t *sds;
  char *error_string = (char *)NULL;
  Geo_coord_t *geo_p = NULL;
  int in;
  Geo_coord_t *geo_isin_nest_p[SPACE_MAX_NEST];

  /* Create the Geoloc data structure */

  this = (Geoloc_t *)malloc(sizeof(Geoloc_t));
  if (this == (Geoloc_t *)NULL) 
    LOG_RETURN_ERROR("allocating Geoloc structure", "SetupGeolocSwath", 
                 (Geoloc_t *)NULL);

  /* Populate the data structure */

  this->geoloc_type = GRID_GEOLOC;
  this->file_name = (char *)NULL;
  this->size.l = input->size.l;
  this->size.s = input->size.s;
  this->scan_size.l = input->scan_size.l;
  this->scan_size.s = input->scan_size.s;

  this->scan_size_geo.l = input->scan_size.l + 
                          kernel->before.l + kernel->after.l + 1;
  this->scan_size_geo.s = input->scan_size.s + 
                          kernel->before.s + kernel->after.s + 1;
  if (space_def->isin_type != SPACE_NOT_ISIN) this->scan_size_geo.l++;

  this->nscan = input->nscan;
  this->open = false;

  this->space_def.proj_num = space_def->proj_num;
  for (ip = 0; ip < NPROJ_PARAM; ip++)
    this->space_def.proj_param[ip] = space_def->proj_param[ip];
  this->space_def.pixel_size = space_def->pixel_size;
  this->space_def.ul_corner.x = space_def->ul_corner.x;
  this->space_def.ul_corner.y = space_def->ul_corner.y;
  this->space_def.ul_corner_set = space_def->ul_corner_set;
  this->space_def.img_size.l = input->size.l;
  this->space_def.img_size.s = input->size.s;
  this->space_def.zone = space_def->zone;
  this->space_def.sphere = space_def->sphere;
  this->space_def.zone_set = space_def->zone_set;
  this->space_def.isin_type = space_def->isin_type;

  this->sds_file_id = -1;
  this->sds_lat.name = (char *)NULL;
  this->sds_lon.name = (char *)NULL;
  for (i = 0; i < 2; i++) {
    sds = (i == 1) ? &this->sds_lat : &this->sds_lon;
    sds->rank = 1;
    for (ir = 0; ir < sds->rank; ir++)
      sds->dim[ir].name = (char *)NULL;
  }

  /* Set up the band offsets */

  for (ib = 0; ib < NBAND_MODIS; ib++)
    this->band_offset[ib].l = this->band_offset[ib].s = 0.0;
  for (ib = 0; ib < NBAND_OFFSET_GEN; ib++) {
    ib1 = ib + NBAND_MODIS;
    this->band_offset[ib1].l = this->band_offset[ib1].s = 0.0;
  }
  this->lat_fill = -1.0;
  this->lon_fill = -1.0;

  /* Determine nesting array size for special nested ISIN cases */

  switch (space_def->isin_type) {
    case SPACE_ISIN_NEST_2: this->n_nest = 2; break;
    case SPACE_ISIN_NEST_4: this->n_nest = 4; break;
    default: this->n_nest = -1;
  }

  /* Allocate buffers */

  this->img = (Img_coord_double_t **)NULL;
  this->geo = (Geo_coord_t **)NULL;
  for (in = 0; in < 4; in++) 
    this->geo_isin_nest[in] = (Geo_coord_t **)NULL;

  if (error_string == (char *)NULL) {
    this->geo = (Geo_coord_t **)calloc((size_t)this->scan_size_geo.l, 
                                        sizeof(Geo_coord_t *));
    if (this->geo == (Geo_coord_t **)NULL)
      error_string = "allocating geo location buffer array";
  }

  n = (size_t)(this->scan_size_geo.l * this->scan_size_geo.s);
  if (error_string == (char *)NULL) {
    geo_p = (Geo_coord_t *)calloc(n, sizeof(Geo_coord_t));
    if (geo_p == (Geo_coord_t *)NULL) 
      error_string = "allocating geo location buffer";
  }

  /* Allocate extra buffers for speical nested ISIN cases */

  for (in = 0; in < this->n_nest; in++) {
    if (error_string == (char *)NULL) {
      this->geo_isin_nest[in] = 
        (Geo_coord_t **)calloc((size_t)this->scan_size_geo.l, 
                               sizeof(Geo_coord_t *));
      if (this->geo_isin_nest[in] == (Geo_coord_t **)NULL)
        error_string = "allocating geo location (isin_nest) buffer array";
    }

    if (error_string == (char *)NULL) {
      geo_isin_nest_p[in] = (Geo_coord_t *)calloc(n, sizeof(Geo_coord_t));
      if (geo_isin_nest_p[in] == (Geo_coord_t *)NULL) 
        error_string = "allocating geo location (isin_nest) buffer";
    }
  }

  if (error_string == (char *)NULL) {
    for (i = 0; i < this->scan_size_geo.l; i++) {
      this->geo[i] = geo_p;
      geo_p += this->scan_size_geo.s;
    }
    for (in = 0; in < this->n_nest; in++) {
      for (i = 0; i < this->scan_size_geo.l; i++) {
        this->geo_isin_nest[in][i] = geo_isin_nest_p[in];
        geo_isin_nest_p[in] += this->scan_size_geo.s;
      }
    }
  }

  /* Handle any memory allocation errors */

  if (error_string != (char *)NULL) {
    if (this->geo != (Geo_coord_t **)NULL) {
      if (this->geo[0] != (Geo_coord_t *)NULL) free(this->geo[0]);
      free(this->geo);
    }
    for (in = 0; in < this->n_nest; in++) {
      if (this->geo_isin_nest[in] != (Geo_coord_t **)NULL) {
        if (this->geo_isin_nest[in][0] != (Geo_coord_t *)NULL) 
	  free(this->geo_isin_nest[in][0]);
        free(this->geo_isin_nest[in]);
      }
    }
    
    free(this);
    LOG_RETURN_ERROR(error_string, "SetupGeolocGrid", (Geoloc_t *)NULL);
  }

  this->lat_buf = (float32 *)NULL;
  this->lon_buf = (float32 *)NULL;

  return this;
}


bool CloseGeoloc(Geoloc_t *this)
/* 
!C******************************************************************************

!Description: 'CloseGeoloc' ends SDS access and closes the input geolocation
 file.
 
!Input Parameters:
 this           'geoloc' data structure; the following fields are input:
                   open, sds_lat.id, sds_lon.id, sds_file_id

!Output Parameters:
 this           'geoloc' data structure; the following fields are modified:
                   open
 (returns)      status:
                  'true' = okay
		  'false' = error return

!Team Unique Header:

 ! Design Notes:
   1. An error status is returned when:
       a. the file is not open for access
       b. an error occurs when closing access to the SDS.
   2. Error messages are handled with the 'LOG_RETURN_ERROR' macro.
   3. 'OpenGeolocSwath' must be called before this routine is called.
   4. 'FreeGeoloc' should be called to deallocate memory used by the 
      'geoloc' data structure.

!END****************************************************************************
*/
{
  Myhdf_sds_t *sds;
  int i;

  if (!this->open)
    LOG_RETURN_ERROR("file not open", "CloseGeoloc", false);

  for (i = 0; i < 2; i++) {
    sds = (i == 1) ? &this->sds_lat :  &this->sds_lon;
    if (SDendaccess(sds->id) == HDF_ERROR) 
      LOG_RETURN_ERROR("ending sds access", "CloseGeoloc", false);
  }

  SDend(this->sds_file_id);
  this->open = false;

  return true;
}


bool FreeGeoloc(Geoloc_t *this)
/* 
!C******************************************************************************

!Description: 'FreeGeoloc' frees the 'geoloc' data structure memory.
 
!Input Parameters:
 this           'geoloc' data structure; the following fields are input:
                   geoloc_type, lon_buf, lat_buf, img, geo, 
		   sds_lat, sds_lon, (sds_t)->id, (sds_t)->dim[*].name,
		   (sds_t)->rank, (sds_t)->name, file_name

!Output Parameters:
 this           'geoloc' data structure; the following fields are modified:
                   lon_buf, lat_buf, img, geo, sds_lat, sds_lon, 
		   (sds_t)->dim[*].name, (sds_t)->name, file_name
 (returns)      status:
                  'true' = okay (always returned)

!Team Unique Header:

 ! Design Notes:
   1. Either 'OpenGeolocSwath' or 'SetupGeolocGrid' must be called before 
      this routine is called.
   2. If 'OpenGeolocSwath' is called, then 'CloseGeoloc' must also be called 
      before this routine is called.
   3. An error status is never returned.

!END****************************************************************************
*/
{
  Myhdf_sds_t *sds;
  int i, ir;
  int in;

  if (this != (Geoloc_t *)NULL) {

    if (this->geoloc_type == SWATH_GEOLOC) {
      if (this->lon_buf != (float32 *)NULL) free(this->lon_buf);
      if (this->lat_buf != (float32 *)NULL) free(this->lat_buf);
      if (this->img != (Img_coord_double_t **)NULL) {
        if (this->img[0] != (Img_coord_double_t *)NULL) free(this->img[0]);
        free(this->img);
      }
    }

    if (this->geoloc_type == GRID_GEOLOC) {
      if (this->geo != (Geo_coord_t **)NULL) {
        if (this->geo[0] != (Geo_coord_t *)NULL) free(this->geo[0]);
        free(this->geo);
      }
      for (in = 0; in < this->n_nest; in++) {
        if (this->geo_isin_nest[in] != (Geo_coord_t **)NULL) {
          if (this->geo_isin_nest[in][0] != (Geo_coord_t *)NULL) 
  	    free(this->geo_isin_nest[in][0]);
          free(this->geo_isin_nest[in]);
        }
      }
    }

    if (this->geoloc_type == SWATH_GEOLOC) {
      for (i = 0; i < 2; i++) {
        sds = (i == 1) ? &this->sds_lat : &this->sds_lon;
        for (ir = 0; ir < sds->rank; ir++) {
          if (sds->dim[ir].name != (char *)NULL) free(sds->dim[ir].name);
        }
      }
      if (this->sds_lat.name != (char *)NULL) free(this->sds_lat.name);
      if (this->sds_lon.name != (char *)NULL) free(this->sds_lon.name);
      if (this->file_name != (char *)NULL) free(this->file_name);
    }
    free(this);

  }

  return true;
}


bool GetGeolocSwath(Geoloc_t *this, Space_t *space, int iscan)
/* 
!C******************************************************************************

!Description: 'GetGeolocSwath' reads a scan of geolocation data and remaps it 
 to the output product space.
 
!Input Parameters:
 this           'geoloc' data structure; the following fields are input:
                   open, nscan, scan_size.l, size.s, sds_lat.id,
		   sds_lon.id, lat_buf, lon_buf, img, lat_fill, 
		   lon_fill
 space          output grid space; the following field is input:
                   for_trans
 iscan          scan number

!Output Parameters:
 this           'geoloc' data structure; the following field is modified:
                  img
 (returns)      status:
                  'true' = okay
		  'false' = error return

!Team Unique Header:

 ! Design Notes:
   1. An error status is returned when:
       a. the file is not open for access
       b. the scan number is not in the valid range
       c. there is an error reading the SDSs
       d. there is an error converting to the output map projection coordinates.
   2. Error messages are handled with the 'LOG_RETURN_ERROR' macro.
   3. 'OpenGeolocSwath' must be called before this routine is called.

!END****************************************************************************
*/
{
  int32 start[MYHDF_MAX_RANK], nval[MYHDF_MAX_RANK];
  int il, is;
  int il_r;
  Geo_coord_t geo;
  Img_coord_double_t *img_p;

  if (!this->open)
    LOG_RETURN_ERROR("file not open", "GetGeolocSwath", false);

  if (iscan < 0  ||  iscan >= this->nscan)
    LOG_RETURN_ERROR("invalid scan number", "GetGeolocSwath", false);

  il = iscan * this->scan_size.l;
  for (il_r = 0; il_r < this->scan_size.l; il_r++) {

    start[0] = il; 
    start[1] = 0;

    nval[0] = 1;
    nval[1] = this->scan_size.s;

    if (SDreaddata(this->sds_lat.id, start, NULL, nval, 
                   this->lat_buf) == HDF_ERROR)
      LOG_RETURN_ERROR("reading latitude", "GetGeolocSwath", false);
    if (SDreaddata(this->sds_lon.id, start, NULL, nval, 
                   this->lon_buf) == HDF_ERROR)
      LOG_RETURN_ERROR("reading longitude", "GetGeolocSwath", false);

    img_p = this->img[il_r];
    for (is = 0; is < this->scan_size.s; is++) {
      img_p->is_fill = true;
      if (this->lat_buf[is] != this->lat_fill  && 
          this->lon_buf[is] != this->lon_fill) {
	geo.is_fill = false;
        geo.lat = this->lat_buf[is] * RAD;
        geo.lon = this->lon_buf[is] * RAD;
        if (!ToSpace(space, &geo, img_p))
          LOG_RETURN_ERROR("converting to output map coordinates", 
	               "GetGeolocSwath", false);
      }
      

/* #define DEBUG */
#ifdef DEBUG
      if (img_p->is_fill) 
        printf(" fill value at scan %d, line %d, sample %d\n", iscan, il_r, is);
#endif

      img_p++;
    }

    il++;
  }

  return true;
}
