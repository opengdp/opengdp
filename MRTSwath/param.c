/*
!C****************************************************************************

!File: param.c
  
!Description: Functions for accepting parameters from the command line or 
 a file.

!Revision History:
 Revision 1.0 2000/11/07
 Robert Wolfe
 Original Version.

 Revision 1.1 2000/12/13
 Sadashiva Devadiga
 Modified to accept parameters from command line or file.

 Revision 1.2 2001/05/08
 Sadashiva Devadiga
 Added checks for required parameters.

 Revision 1.3 2002/03/02
 Robert Wolfe
 Added special handling for input ISINUS case.

 Revision 1.4 2002/05/10
 Robert Wolfe
 Added separate output SDS name.

 Revision 1.5 06/02
 Gail Schmidt
 Changed to allow the user to specify input lat/long for the UL and LR
   output corners, rather than specifying UL in x/y output space.  Since
   the UL and LR are specified, the output_image_size is no longer needed.

 Revision 1.5 08/02
 Gail Schmidt
 Changed to allow the user to specify the lat/long values in the output
   projection parameters as decimal degrees.  This software will convert
   the decimal degrees to DMS.

 Revision 2.0 Nov-Dec/03
 Gail Schmidt
 Modified the software to process all the SDSs (of nominal MODIS scan size),
   if an SDS was not specified in the parameter file. And, support multiple
   SDSs for input by the user.
 Also, GRIDs will not be used by MRTSwath, so don't allow GRID processing.
 Modified the software to read the NSEW BOUNDING COORDS in the metadata,
   if the UL and LR corners were not specified.
 Default kernel_type is NN (instead of CC) to match the MRT.
 Support output to raw binary.
 Add support for multiple output pixel sizes. This also means multiple
   lines and samples.
 Added support to automatically determine the input resolution, and use
   that as the default output pixel size ... if not specified by the user.
 Allow the user to specify the UL/LR corner in line/sample units.

 Revision 2.0 May/04
 Modified ISINUS to be a value of 31 instead of 99 to match GCTP.

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

      Sadashiva Devadiga (Code 922)
      MODIS Land Team Support Group     SSAI
      devadiga@ltpmail.gsfc.nasa.gov    5900 Princess Garden Pkway, #300
      phone: 301-614-5549               Lanham, MD 20706
  
 ! Design Notes:
   1. The following public functions handle the input data:

	GetParam - Setup 'param' data structure and populate with user
	           parameters.
	FreeParam - Free the 'param' data structure memory.

   2. 'GetParam' must be called before 'FreeParam'.  
   3. 'FreeParam' should be used to free the 'param' data structure.

!END****************************************************************************
*/
#if !defined(__CYGWIN__) && !defined(__APPLE__) && !defined(WIN32)
#include <values.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "hlimits.h"         /* MAX_VAR_DIMS, MAX_NC_NAME */
#include "param.h"
#include "geoloc.h"
#include "parser.h"
#include "usage.h"
#include "myisoc.h"

#include "myproj.h"

#include "const.h"
#include "myerror.h"
#include "deg2dms.h"

/* External arrays */

extern Proj_sphere_t Proj_sphere[PROJ_NSPHERE];
extern Proj_type_t Proj_type[PROJ_NPROJ];

/* Functions */

Param_t *GetParam(int argc, const char **argv)
/* 
!C******************************************************************************

!Description: 'GetParam' sets up the 'param' data structure and populate with user
 parameters, either from the command line or from a parameter file.
 
!Input Parameters:
 argc           number of command line arguments
 argv           command line argument list

!Output Parameters:
 (returns)      'param' data structure or NULL when an error occurs

!Team Unique Header:

 ! Design Notes:
   1. An error status is returned when:
       a. memory allocation is not successful
       b. an error is returned from the ReadCmdLine function
       c. certain required parameters are invalid or not entered:
            input file name, output file name, geolocation file name,
	    SDS name, output space projection number, 
	    output space pixel size, output space upper left corner, 
	    output space image size, either output space sphere or projection
	    parameter number 0
	    output space zone number not given for UTM
   2. Error of type 'a' are handled with the 'LOG_RETURN_ERROR' macro and 
      the others are handled by writting the error messages to 'stderr' and 
      then printing the usage information.
   3. 'FreeParam' should be called to deallocate memory used by the 
      'param' data structures.

!END****************************************************************************
*/
{
  Param_t *this;
  Input_t *input = NULL;
  int i, j, ip, jp;
  char tmp_sds_name[MAX_STR_LEN];
  char errstr[M_MSG_LEN+1];            /* error string for OpenInput */
  char msg[M_MSG_LEN+1];
  char *extptr;                        /* ptr to the output file extension */
  double tmp_pixel_size;
  Geo_coord_t ul_corner;
  Geo_coord_t lr_corner;
  int copy_dim[MYHDF_MAX_RANK];

  /* Create the Param data structure */
  this = (Param_t *)malloc(sizeof(Param_t));
  if (this == (Param_t *)NULL)
    LOG_RETURN_ERROR("allocating Input structure", "GetParam",
                          (Param_t *)NULL);

  /* set default parameters */
  this->multires = false;
  this->input_file_name = (char *)NULL;
  this->output_file_name = (char *)NULL;
  this->geoloc_file_name = (char *)NULL;
  this->input_space_type = SWATH_SPACE;   /* Default is input swath */
  this->output_file_format = HDF_FMT;     /* Default is HDF output */

  this->num_input_sds = 0;                /* Default is no SDSs specified */
  this->output_space_def.pixel_size = -1.0;
  for (ip = 0; ip < MAX_SDS_DIMS; ip++)
  {
    this->input_sds_nbands[ip] = 0;
    this->output_pixel_size[ip] = -1.0;
    this->output_img_size[ip].l = -1;
    this->output_img_size[ip].s = -1;
    this->output_dt_arr[ip] = -1;
    this->ires[ip] = -1;
    this->fill_value[ip] = -1.0;

    for (jp = 0; jp < MAX_VAR_DIMS; jp++)
      this->input_sds_bands[ip][jp] = 0; /* Default is no bands processed */

    this->create_output[ip] = true;
    this->rank[ip] = 2;
    for (jp = 0; jp < MYHDF_MAX_RANK; jp++)
      this->dim[ip][jp] = 0;
    this->dim[ip][0] = -1;
    this->dim[ip][1] = -2;
  }

  this->input_sds_name = (char *)NULL;
  this->output_sds_name = (char *)NULL;
  this->iband = -1;
  this->kernel_type = NN;

  this->output_space_def.proj_num = -1;
  for (ip = 0; ip < NPROJ_PARAM; ip++)
    this->output_space_def.proj_param[ip] = 0.0;
  for (ip = 0; ip < NPROJ_PARAM; ip++)
    this->output_space_def.orig_proj_param[ip] = 0.0;
  this->output_space_def.ul_corner.x = -1.0;
  this->output_space_def.ul_corner.y = -1.0;
  this->output_space_def.ul_corner_geo.lat = -1.0;
  this->output_space_def.ul_corner_geo.lon = -1.0;
  this->output_space_def.ul_corner_set = false;
  this->output_space_def.lr_corner.x = -1.0;
  this->output_space_def.lr_corner.y = -1.0;
  this->output_space_def.lr_corner_geo.lat = -1.0;
  this->output_space_def.lr_corner_geo.lon = -1.0;
  this->output_space_def.lr_corner_set = false;
  this->output_space_def.img_size.l = -1;
  this->output_space_def.img_size.s = -1;
  this->output_space_def.zone = 0;
  this->output_space_def.zone_set = false;
  this->output_space_def.sphere = -1;
  this->output_space_def.isin_type = SPACE_NOT_ISIN;
  this->output_spatial_subset_type = LAT_LONG;

  /* Input space is not really used, since the MRTSwath will not
     work with Grids (only swath) */
  this->input_space_def.proj_num = -1;
  for (ip = 0; ip < NPROJ_PARAM; ip++)
    this->input_space_def.proj_param[ip] = 0.0;
  this->input_space_def.pixel_size = -1.0;
  this->input_space_def.ul_corner.x = -1.0;
  this->input_space_def.ul_corner.y = -1.0;
  this->input_space_def.ul_corner_set = false;
  this->input_space_def.img_size.l = -1;
  this->input_space_def.img_size.s = -1;
  this->input_space_def.zone = 0;
  this->input_space_def.zone_set = false;
  this->input_space_def.sphere = -1;
  this->input_space_def.isin_type = SPACE_NOT_ISIN;

  this->output_data_type = -1;
  this->patches_file_name = "patches.tmp";        

  /* Read the command-line and parameter file parameters */
  if (!ReadCmdLine(argc, argv, this)) {
    FreeParam(this);
    sprintf(msg, "%s\n", USAGE);
    LogInfomsg(msg);
    return (Param_t *)NULL; 
  }

  /* Check to see that all of the parameters are entered */
  if ((this->input_file_name == (char *)NULL)  ||  
      (strlen(this->input_file_name) < 1)) {
    sprintf(msg, "resamp: input file name not given\n");
    LogInfomsg(msg);
    FreeParam(this);
    sprintf(msg, "%s\n", USAGE);
    LogInfomsg(msg);
    return (Param_t *)NULL; 
  }

  /* Check the output filename */
  if ((this->output_file_name == (char *)NULL)  ||  
      (strlen(this->output_file_name) < 1)) {
    sprintf(msg, "resamp: output file name not given\n");
    LogInfomsg(msg);
    FreeParam(this);
    sprintf(msg, "%s\n", USAGE);
    LogInfomsg(msg);
    return (Param_t *)NULL; 
  }

  /* Check to see if a .hdf, .hdr. or .tif extension was provided in the
     filename. If so, remove it since the output file format will specify
     the output extension. */
  extptr = strstr (this->output_file_name, ".hdf");
  if (extptr)
    extptr[0] = '\0';

  extptr = strstr (this->output_file_name, ".HDF");
  if (extptr)
    extptr[0] = '\0';

  extptr = strstr (this->output_file_name, ".hdr");
  if (extptr)
    extptr[0] = '\0';

  extptr = strstr (this->output_file_name, ".HDR");
  if (extptr)
    extptr[0] = '\0';

  extptr = strstr (this->output_file_name, ".tif");
  if (extptr)
    extptr[0] = '\0';

  extptr = strstr (this->output_file_name, ".TIF");
  if (extptr)
    extptr[0] = '\0';

  /* Check the output file format */
  if ((this->output_file_format != HDF_FMT) && 
      (this->output_file_format != GEOTIFF_FMT) &&
      (this->output_file_format != RB_FMT) &&
      (this->output_file_format != BOTH)) {
    sprintf(msg, "resamp: unsupported output file format\n");    
    LogInfomsg(msg);
    FreeParam(this);
    sprintf(msg, "%s\n", USAGE);
    LogInfomsg(msg);
    return (Param_t *)NULL; 
  }

  if ((this->input_space_type == SWATH_SPACE)  &&
      ((this->geoloc_file_name == (char *)NULL)  ||  
       (strlen(this->input_file_name) < 1))) {
    sprintf(msg, "resamp: geolocation file name not given\n");
    LogInfomsg(msg);
    FreeParam(this);
    sprintf(msg, "%s\n", USAGE);
    LogInfomsg(msg);
    return (Param_t *)NULL; 
  }

  /* If no SDS names were specified then process all of them in the file,
     otherwise fill in the rest of the SDS information. */
  if (this->num_input_sds == 0) {
    /* Process all the SDS names, by default */
#ifdef DEBUG
    printf ("Reading default SDSs\n");
#endif
    this->num_input_sds = ReadSDS(this);
    if (this->num_input_sds == 0) {
      sprintf(msg, "resamp: error reading default SDS names\n");
      LogInfomsg(msg);
      FreeParam(this);
      sprintf(msg, "%s\n", USAGE);
      LogInfomsg(msg);
      return (Param_t *)NULL; 
    }
  }
  else {
    /* Read the SDSs and determine the number of bands in each */
    if (!SDSInfo(this)) {
      sprintf(msg, "resamp: error reading SDS information\n");
      LogInfomsg(msg);
      FreeParam(this);
      sprintf(msg, "%s\n", USAGE);
      LogInfomsg(msg);
      return (Param_t *)NULL; 
    }
  }

  /* Check output space definition */
  if (this->output_space_def.proj_num < 0) {
    sprintf(msg, "resamp: output space projection number not given\n");
    LogInfomsg(msg);
    FreeParam(this);
    sprintf(msg, "%s\n", USAGE);
    LogInfomsg(msg);
    return (Param_t *)NULL; 
  }

  /* Loop through all the SDSs and determine their resolution */
  for (i = 0; i < this->num_input_sds; i++)
  {
    /* Loop through all the bands in the current SDS until we find one
       that will be processed. Use that band to get the resolution of the
       SDS, since all the bands in the SDS will be the same resolution. */
    for (j = 0; j < this->input_sds_nbands[i]; j++)
    {
      /* Is this band one that should be processed? */
      if (!this->input_sds_bands[i][j])
        continue;

      /* Create the input_sds_name which is "SDSname, band" */
      if (this->input_sds_nbands[i] == 1)
      {
        /* 2D product so the band number is not needed */
        sprintf(tmp_sds_name, "%s", this->input_sds_name_list[i]);
      }
      else
      {
        /* 3D product so the band number is required */
        sprintf(tmp_sds_name, "%s, %d", this->input_sds_name_list[i], j);
      }

      this->input_sds_name = strdup (tmp_sds_name);
      if (this->input_sds_name == NULL) {
        sprintf(msg, "resamp: error creating input SDS band name");
	LogInfomsg(msg);
        FreeParam(this);
        return (Param_t *)NULL;
      }

#ifdef DEBUG
      printf ("Getting param %s ...\n", this->input_sds_name);
#endif

      /* Update the system to process the current SDS and band */
      if (!update_sds_info(i, this)) {
        sprintf(msg, "resamp: error updating SDS information");
	LogInfomsg(msg);
        FreeParam(this);
        return (Param_t *)NULL;
      }

      /* Make a copy of the dim parameters, since they get modified */
      for (ip = 0; ip < MYHDF_MAX_RANK; ip++) {
        copy_dim[ip] = this->dim[i][ip];
      }

      /* Open input file for the specified SDS and band */
      input = OpenInput(this->input_file_name, this->input_sds_name,
                        this->iband, this->rank[i], copy_dim, errstr);
      if (input == (Input_t *)NULL) {
        /* This is an invalid SDS for our processing so skip to the next
           SDS. We will only process SDSs that are at the 1km, 500m, or
           250m resolution (i.e. a factor of 1, 2, or 4 compared to the
           1km geolocation lat/long data). We also only process CHAR8,
           INT8, UINT8, INT16, and UINT16 data types. */
#ifdef DEBUG
        printf("%s\n", errstr);
        printf("%s %ld: param, not processing SDS/band\n\n", (__FILE__),
          (long)(__LINE__));
#endif
        break;
      }

      /* Determine the resolution of each of the input SDSs */
      if (!DetermineResolution(&input->sds, &input->dim, &this->ires[i])) {
        sprintf(msg, "resamp: error determining input resolution\n");
	LogInfomsg(msg);
        CloseInput(input);
        FreeInput(input);
        FreeParam(this);
        return (Param_t *)NULL; 
      }

      /* Close input file */
      if (!CloseInput(input)) {
        sprintf(msg, "resamp: closing input file");
	LogInfomsg(msg);
        FreeInput(input);
        FreeParam(this);
        return (Param_t *)NULL; 
      }

      /* Free the input structure */
      if (!FreeInput(input)) {
        sprintf(msg, "resamp: freeing input file stucture");
	LogInfomsg(msg);
        FreeParam(this);
        return (Param_t *)NULL;
      }

      /* We only need one band in this SDS, so break out of the loop */
      break;
    }  /* for (j = 0; j < this->input_sds_nbands[i]; j++) */
  }  /* for (i = 0; i < this->num_input_sds; i++) */

  /* Verify that at least one output pixel size value is defined */
  if (this->output_pixel_size[0] < 0.0) {
    /* No pixel size was specified, so try to determine the resolution of
       the input SDSs and use that for the pixel size. It is assumed that
       the input swath product will have the same resolution for all SDSs. */
    if (!DeterminePixelSize(this->geoloc_file_name, this->num_input_sds,
      this->ires, this->output_space_def.proj_num,
      this->output_pixel_size)) {
      sprintf(msg, "resamp: error determining output pixel size. "
        "Therefore, in order to process this data, the output pixel size "
        "must be specified.\n");
      LogInfomsg(msg);
      FreeParam(this);
      sprintf(msg, "%s\n", USAGE);
      LogInfomsg(msg);
      return (Param_t *)NULL; 
    }

    /* Set multires to FALSE since the output product will have the same
       resolution for all SDSs */
    this->multires = false;
  }

  /* If not enough pixel sizes were provided, then fill the pixel sizes
     in with the very last valid pixel size */
  for (ip = 0; ip < this->num_input_sds; ip++) {
    if (this->output_pixel_size[ip] < 0.0) {
      /* This is the first undefined pixel size, so grab the previous
         pixel size since it was the last valid pixel size value */
      tmp_pixel_size = this->output_pixel_size[ip-1];
      break;
    }
  }

  /* Fill in the rest of the values with the saved pixel size value */
  for (; ip < this->num_input_sds; ip++) {
    this->output_pixel_size[ip] = tmp_pixel_size;
  }

  /* Check the user-specified pixel sizes to see if the output product
     is multiple resolutions */
  for (ip = 0; ip < this->num_input_sds; ip++) {
    if (this->output_pixel_size[ip] != this->output_pixel_size[0])
      this->multires = true;
  }

  /* If the UL or LR corner was not specified, then use the bounding coords */
  if (!this->output_space_def.ul_corner_set ||
      !this->output_space_def.lr_corner_set) {
    /* Read the BOUNDING coords, by default */
    if (!ReadBoundCoords(this->input_file_name, &ul_corner, &lr_corner)) {
      sprintf(msg, "resamp: error reading BOUNDING COORDS from metadata. "
        "Therefore, in order to process this data, the output spatial "
        "subsetting will need to be specified.\n");
      LogInfomsg(msg);
      sprintf(msg, "%s\n", USAGE);
      LogInfomsg(msg);
      FreeParam(this);
      return (Param_t *)NULL; 
    }
    else {
      /* Store all initial corner points in the x/y corner structure.
         The call to convert_corners will handle moving to the lat/long
         structure location. */
      this->output_space_def.ul_corner_set = true;
      this->output_space_def.lr_corner_set = true;
      this->output_space_def.ul_corner.x = ul_corner.lon;
      this->output_space_def.ul_corner.y = ul_corner.lat;
      this->output_space_def.lr_corner.x = lr_corner.lon;
      this->output_space_def.lr_corner.y = lr_corner.lat;
      this->output_spatial_subset_type = LAT_LONG;
    }
  }

  if ((this->output_space_def.proj_param[0] <= 0.0) && 
      (this->output_space_def.sphere < 0)) {
    sprintf(msg, "resamp: either output space sphere or projection "
            "parameter number 0 must be given\n");
    LogInfomsg(msg);
    sprintf(msg, "%s\n", USAGE);
    LogInfomsg(msg);
    FreeParam(this);
    return (Param_t *)NULL; 
  }

  if ((this->output_space_def.proj_num == 1)  &&  /* UTM => proj_num = 1 */
       !this->output_space_def.zone_set) { 
    sprintf(msg, "resamp: output space zone number not given for UTM\n");
    LogInfomsg(msg);
    sprintf(msg, "%s\n", USAGE);
    LogInfomsg(msg);
    FreeParam(this);
    return (Param_t *)NULL; 
  }

  if (this->output_space_def.proj_num == 31) /* ISINUS => proj_num = 31 */
    this->output_space_def.isin_type= SPACE_ISIN_NEST_1;

  /* Copy the projection parameters to orig_proj_param to use the decimal
     degree values later (GeoTiff output) */
  for (i = 0; i < NPROJ_PARAM; i++) {
    this->output_space_def.orig_proj_param[i] =
      this->output_space_def.proj_param[i];
  }

  /* Convert the output projection parameter lat/long values from decimal
     degrees to DMS */
  if (!Deg2DMS (this->output_space_def.proj_num,
                this->output_space_def.proj_param)) {
    sprintf(msg, "resamp: error converting projection parameters from"
            "decimal degrees to DMS\n");
    LogInfomsg(msg);
    FreeParam(this);
    return (Param_t *)NULL; 
  }

  /* Use the UL and LR corner points to get the UL corner in output
     space and the number of lines/samples in the output image. This must
     be done for each SDS, since the pixel size might be different. */
  if (!ConvertCorners (this)) {
    sprintf(msg, "resamp: error determining UL and lines/samples from "
            "the input UL and LR corners\n");
    LogInfomsg(msg);
    FreeParam(this);
    return (Param_t *)NULL; 
  }

  /* Make sure the lat/long values are between +-180 and +-90 */
  if (this->output_space_def.ul_corner_geo.lat < -90.0 ||
      this->output_space_def.ul_corner_geo.lat > 90.0  ||
      this->output_space_def.lr_corner_geo.lat < -90.0 ||
      this->output_space_def.lr_corner_geo.lat > 90.0  ||
      this->output_space_def.ul_corner_geo.lon < -180.0 ||
      this->output_space_def.ul_corner_geo.lon > 180.0  ||
      this->output_space_def.lr_corner_geo.lon < -180.0 ||
      this->output_space_def.lr_corner_geo.lon > 180.0) {
    sprintf(msg, "resamp: invalid output lat/lon corners\n");
    LogInfomsg(msg);
    FreeParam(this);
    return (Param_t *)NULL; 
  }

  /* MRTSwath will only process swath data */
  if (this->input_space_type != SWATH_SPACE) {
    sprintf(msg, "resamp: grid or point data detected. MRTSwath will "
            "only process swath data\n");
    LogInfomsg(msg);
    FreeParam(this);
    return (Param_t *)NULL; 
  }

  return this;
}


Param_t *CopyParam(Param_t *param)
/* 
!C******************************************************************************

!Description: 'CopyParam' sets up the 'param' data structure and populates
 it with the values in hte input 'param' data structure.
 
!Input Parameters:
 param          valid 'param' data structure

!Output Parameters:
 (returns)      copy of input 'param' data structure or NULL if an error occurs

!Team Unique Header:

 ! Design Notes:
   1. An error status is returned when:
       a. memory allocation is not successful
   2. Error of type 'a' are handled with the 'LOG_RETURN_ERROR' macro and 
      the others are handled by writting the error messages to 'stderr' and 
      then printing the usage information.
   3. 'FreeParam' should be called to deallocate memory used by the 
      'param' data structures.

!END****************************************************************************
*/
{
  Param_t *this;
  int ip, jp;

  /* create the Param data structure */

  this = (Param_t *)malloc(sizeof(Param_t));
  if (this == (Param_t *)NULL)
    LOG_RETURN_ERROR("allocating Input structure", "CopyParam",
                          (Param_t *)NULL);

  /* copy the parameters */

  this->input_file_name = strdup(param->input_file_name);
  this->output_file_name = strdup(param->output_file_name);
  this->geoloc_file_name = strdup(param->geoloc_file_name);
  this->output_file_format = param->output_file_format;
  this->input_space_type = param->input_space_type;
  this->num_input_sds = param->num_input_sds;
  this->multires = param->multires;

  for (ip = 0; ip < this->num_input_sds; ip++)
  {
    if (param->input_sds_name_list[ip] != NULL)
      strcpy (this->input_sds_name_list[ip], param->input_sds_name_list[ip]);

    this->input_sds_nbands[ip] = param->input_sds_nbands[ip];
    for (jp = 0; jp < this->input_sds_nbands[ip]; jp++)
      this->input_sds_bands[ip][jp] = param->input_sds_bands[ip][jp];

    this->rank[ip] = param->rank[ip];
    for (jp = 0; jp < MYHDF_MAX_RANK; jp++)
      this->dim[ip][jp] = param->dim[ip][jp];

    this->output_pixel_size[ip] = param->output_pixel_size[ip];
    this->output_img_size[ip] = param->output_img_size[ip];
    this->output_dt_arr[ip] = param->output_dt_arr[ip];
    this->ires[ip] = param->ires[ip];
    this->fill_value[ip] = param->fill_value[ip];
    this->create_output[ip] = param->create_output[ip];
  }

  if (param->input_sds_name != NULL)
    this->input_sds_name = strdup(param->input_sds_name);
  else
    this->input_sds_name = (char *)NULL;

  if (param->output_sds_name != NULL)
    this->output_sds_name = strdup(param->output_sds_name);
  else
    this->output_sds_name = (char *)NULL;

  this->iband = param->iband;
  this->kernel_type = param->kernel_type;

  /* Space_def_t doesn't contain any pointers, so its ok to make an
     exact copy */
  this->output_space_def = param->output_space_def;
  this->input_space_def = param->input_space_def;

  this->output_spatial_subset_type = param->output_spatial_subset_type;
  this->output_data_type = param->output_data_type;
  this->patches_file_name = strdup(param->patches_file_name);

  return this;
}


bool FreeParam(Param_t *this)
/* 
!C******************************************************************************

!Description: 'FreeParam' frees the 'param' data structure memory.
 
!Input Parameters:
 this           'param' data structure; the following fields are input:
                   input_file_name, output_file_name, geoloc_file_name, 
		   input_sds_name, output_sds_name

!Output Parameters:
 (returns)      status:
                  'true' = okay (always returned)

!Team Unique Header:

 ! Design Notes:
   1. 'GetParam' must be called before this routine is called.
   2. An error status is never returned.

!END****************************************************************************
*/
{
  if (this != (Param_t *)NULL) {
    if (this->input_file_name  != (char *)NULL) free(this->input_file_name);
    if (this->output_file_name != (char *)NULL) free(this->output_file_name);
    if (this->geoloc_file_name != (char *)NULL) free(this->geoloc_file_name);
    if (this->input_sds_name   != (char *)NULL) free(this->input_sds_name);
    if (this->output_sds_name  != (char *)NULL) free(this->output_sds_name);
    free(this);
  }
  return true;
}                            


int ReadSDS(Param_t *this)
/*
!C******************************************************************************
!Description: 'ReadSDS' reads the input file for available swath SDSs.

!Input Parameters:
 this           'input' data structure; the following fields are input:
                   input_file_name

!Output Parameters:
 this           'input' data structure; the following fields are modified:
                   input_sds_name_list, input_sds_nbands, input_sds_bands

 (returns)      int: number of SDSs

!Team Unique Header:

 ! Design Notes:
   1. The file is open for HDF access but closed before returning from the
      routine.
   2. An error status is returned when:
       a. duplicating strings is not successful
       b. errors occur when opening the input HDF file
       c. memory allocation is not successful
   3. Error messages are handled with the 'LOG_RETURN_ERROR' macro.

!END*****************************************************************************/
{
  int i, j;
  int32 sd_fid, sds_id;        /* file id and sds id */
  int32 nsds;                  /* number of SDSs in the file */
  int32 nattr;                 /* number of attributes for the SDS */
  int32 data_type;             /* data type of the SDS */
  char sds_name[MAX_NC_NAME];  /* SDS name */
  int32 rank;                  /* rank of the SDS */
  int32 dims[MYHDF_MAX_RANK];  /* dimensions of the SDS */

  /* Open file for SD read access */

  sd_fid = SDstart(this->input_file_name, DFACC_RDONLY);
  if (sd_fid == HDF_ERROR)
    LOG_RETURN_ERROR("opening input file", "ReadSDS", 0);

  /* Get the list of swaths */

  SDfileinfo(sd_fid, &nsds, &nattr);
#ifdef DEBUG
  printf("Number of SDSs: %i\n", (int) nsds);
#endif

  /* Loop through the SDSs to get SDS names and info */

  for (i = 0; i < nsds; i++)
  {
    /* grab the SDS */
    sds_id = SDselect(sd_fid, i);
    if (sds_id == HDF_ERROR)
      LOG_RETURN_ERROR("selecting input SDS", "ReadSDS", 0);

    /* get information for the current SDS */
    if (SDgetinfo(sds_id, sds_name, &rank, dims, &data_type, &nattr) ==
        HDF_ERROR)
      LOG_RETURN_ERROR("getting SDS information", "ReadSDS", 0);

    /* process the swath SDSs */
    strcpy(this->input_sds_name_list[i], sds_name);
#ifdef DEBUG
    printf("\nSDgetinfo: sds %i, %s, rank = %i, dims =", i,
      sds_name, (int) rank);
    for (j = 0; j < rank; j++)
      printf(" %i", (int) dims[j]);
    printf("\n");
#endif

    /* if the rank is 2 then there is only one band to process, otherwise
       the first dimension value contains the number of bands in the SDS */
    if (rank == 1)
      this->input_sds_nbands[i] = 0;
    else if (rank == 2)
      this->input_sds_nbands[i] = 1;
    else
    {
      /* 3D product. Use the smallest dim value for the bands. */
      this->input_sds_nbands[i] = (int)dims[0];
      for (j = 0; j < rank; j++)
      {
        /* find the smallest dimension */
        if ((int)dims[j] < this->input_sds_nbands[i])
          this->input_sds_nbands[i] = (int)dims[j];

        /* if the smallest dimension is larger than MAX_VAR_DIMS then set
           it to MAX_VAR_DIMS. Most likely it won't be processed anyhow. */
        if (this->input_sds_nbands[i] > MAX_VAR_DIMS)
          this->input_sds_nbands[i] = MAX_VAR_DIMS;
      }
    }

    if (rank > MYHDF_MAX_RANK) {
      SDendaccess(sds_id);
      LOG_RETURN_ERROR("sds rank too large", "ReadSDS", false);
    }

    /* process all the bands in this SDS */
    for (j = 0; j < this->input_sds_nbands[i]; j++)
      this->input_sds_bands[i][j] = 1;

    /* close the SDS */
    SDendaccess(sds_id);
  }

  /* Close the HDF-EOS file */

  SDend(sd_fid);

  return (int)nsds;
}

bool SDSInfo(Param_t *this)
/*
!C******************************************************************************
!Description: 'SDSInfo' accesses the specified SDSs in the input file and
  grabs the band information for each SDS.

!Input Parameters:
 this           'input' data structure; the following fields are input:
                   input_file_name, input_sds_name_list, input_sds_nbands

!Output Parameters:
 this           'input' data structure; the following fields are modified:
                   input_sds_nbands, input_sds_bands

 (returns)      status:
                  'true' = okay
                  'false' = error return

!Team Unique Header:

 ! Design Notes:
   1. The file is open for HDF access but closed before returning from the
      routine.
   2. An error status is returned when:
       a. duplicating strings is not successful
       b. errors occur when opening the input HDF file
       c. memory allocation is not successful
   3. Error messages are handled with the 'LOG_RETURN_ERROR' macro.

!END*****************************************************************************/
{
  int i, j;
  int32 sd_fid, sds_id;        /* file id and sds id */
  int32 sds_index;             /* index of the current SDS */
  int32 nattr;                 /* number of attributes for the SDS */
  int32 data_type;             /* data type of the SDS */
  char sds_name[MAX_NC_NAME];  /* SDS name */
  int32 rank;                  /* rank of the SDS */
  int32 dims[MYHDF_MAX_RANK];  /* dimensions of the SDS */
  char errmsg[M_MSG_LEN+1];    /* error message */
  char tmpsds[MAX_STR_LEN];    /* temp copy of the SDS name in case we need
                                  to remove the '_'s from the SDS name */
  int curr_char;               /* current character in the SDS name */

  /* Open file for SD read access */
  sd_fid = SDstart(this->input_file_name, DFACC_RDONLY);
  if (sd_fid == HDF_ERROR)
    LOG_RETURN_ERROR("opening input file", "SDSInfo", false);

  /* loop through the SDSs */
  for (i = 0; i < this->num_input_sds; i++)
  {
    /* try to find the index for the current SDS name */
    sds_index = SDnametoindex(sd_fid, this->input_sds_name_list[i]);
    if (sds_index == HDF_ERROR) {
      /* if running the GUI, SDS names with blank spaces have '_'s placed
         for the blank spaces. replace the '_'s with blank spaces and then
         try to find the SDS. */
      strcpy(tmpsds, this->input_sds_name_list[i]);
      for (curr_char = 0; curr_char < (int) strlen (tmpsds) - 1; curr_char++)
      {
        /* if this character is an underscore then replace with a blank */
        if (tmpsds[curr_char] == '_')
          tmpsds[curr_char] = ' ';
      }

      /* try to find the index for the new SDS name */
      sds_index = SDnametoindex(sd_fid, tmpsds);
      if (sds_index == HDF_ERROR) {
        sprintf(errmsg, "couldn't get sds index for %s or %s",
          this->input_sds_name_list[i], tmpsds);
        LOG_RETURN_ERROR(errmsg, "SDSInfo", false);
      }

      /* copy the correct SDS name into the SDS name list */
      strcpy(this->input_sds_name_list[i], tmpsds);
    }

    /* get the current SDS */
    sds_id = SDselect(sd_fid, sds_index);
    if (sds_id == HDF_ERROR)
      LOG_RETURN_ERROR("getting sds id", "SDSInfo", false);

    /* get the current SDS information */
    if (SDgetinfo(sds_id, sds_name, &rank, dims, &data_type, &nattr) ==
        HDF_ERROR) {
      SDendaccess(sds_id);
      LOG_RETURN_ERROR("getting sds information", "SDSInfo", false);
    }

    if (rank > MYHDF_MAX_RANK) {
      SDendaccess(sds_id);
      LOG_RETURN_ERROR("sds rank too large", "SDSInfo", false);
    }

    /* if the user specified bands to be processed for the current
       SDS, then fill in the number of bands in the SDS */
    if (this->input_sds_nbands[i] != 0)
    {
       /* if the rank is 2 then there is only one band to process, otherwise
          the first dimension value contains the number of bands in the SDS */
       if (rank == 2)
         this->input_sds_nbands[i] = 1;
       else
         this->input_sds_nbands[i] = (int)dims[0];
    }

    /* otherwise process all the bands in the SDS by default */
    else
    {
       /* if the rank is 2 then there is only one band to process, otherwise
          the first dimension value contains the number of bands in the SDS */
       if (rank == 2)
         this->input_sds_nbands[i] = 1;
       else
         this->input_sds_nbands[i] = (int)dims[0];

       /* Process all the bands in this SDS */
       for (j = 0; j < this->input_sds_nbands[i]; j++)
         this->input_sds_bands[i][j] = 1;
    }

    /* close the SDS */
    SDendaccess(sds_id);
  }

  /* close the HDF-EOS file */
  SDend(sd_fid);

  return true;
}


/******************************************************************************

MODULE:  PrintParam

PURPOSE:  Print the user processing parameters

RETURN VALUE:
Type = none
Value           Description
-----           -----------

HISTORY:
Version  Date   Programmer       Code  Reason
-------  -----  ---------------  ----  -------------------------------------
         12/03  Gail Schmidt           Original Development

NOTES:

******************************************************************************/
void PrintParam
(
    Param_t *param        /* I: Parameter information */
)

{
    /* Strings to match the Kernel_type_t in kernel.h */
    static char *ResamplingTypeStrings[] =
    {
        "NN", "BI", "CC"
    };

    /* Strings to match the Output_file_format_t in param.h */
    static char *FileTypeStrings[] =
    {
        "HDF", "GEOTIFF", "RAW_BINARY", "HDF and GEOTIFF"
    };

    int i, j;
    int count;
    char tempstr[M_MSG_LEN+1],
	 msg[M_MSG_LEN+1];

    sprintf(msg, "\nGeneral processing info\n");
    LogInfomsg(msg);
    sprintf(msg, "-----------------------\n");
    LogInfomsg(msg);

    sprintf(msg, "input_filename:          %s\n", param->input_file_name);
    LogInfomsg(msg);
    sprintf(msg, "geoloc_filename:         %s\n", param->geoloc_file_name);
    LogInfomsg(msg);
    sprintf(msg, "output_filename:         %s\n", param->output_file_name);
    LogInfomsg(msg);
    sprintf(msg, "output_filetype:         %s\n",
        FileTypeStrings[(int)param->output_file_format]);
    LogInfomsg(msg);

    sprintf(msg, "output_projection_type:  %s\n",
        Proj_type[param->output_space_def.proj_num].name);
    LogInfomsg(msg);

    if (param->output_space_def.proj_num == PROJ_UTM)
    {
        sprintf(msg, "output_zone_code:        %d\n",
            param->output_space_def.zone);
        LogInfomsg(msg);
    }

    if (param->output_space_def.sphere < 0 ||
        param->output_space_def.sphere >= PROJ_NSPHERE)
    {
        sprintf(msg, "output_ellipsoid:        None\n");
        LogInfomsg(msg);
    }
    else
    {
        sprintf(msg, "output_ellipsoid:        %s\n",
            Proj_sphere[param->output_space_def.sphere].name);
        LogInfomsg(msg);
    }

    /* If this is an ellipse-based projection, then the output datum might be
       WGS84 */
    switch (param->output_space_def.proj_num)
    {
        case PROJ_ALBERS:
        case PROJ_EQRECT:
        case PROJ_GEO:
        case PROJ_MERCAT:
        case PROJ_TM:
        case PROJ_UTM:
        case PROJ_LAMCC:
        case PROJ_PS:
            /* If WGS84 ellipsoid, then we can tag the WGS84 datum,
               otherwise no datum can be specified. */
            if (param->output_space_def.sphere == 8 /* WGS84 */ ||
                (param->output_space_def.orig_proj_param[0] == 6378137.0 &&
                 param->output_space_def.orig_proj_param[1] == 6356752.31414))
            {
                sprintf(msg, "output_datum:            WGS84\n");
                LogInfomsg(msg);
            }
            else
            {
                sprintf(msg, "output_datum:            No Datum\n");
                LogInfomsg(msg);
            }
            break;

        default:
            sprintf(msg, "output_datum:            No Datum\n");
	    LogInfomsg(msg);
            break;
    }

    sprintf(msg, "resampling_type:         %s\n",
            ResamplingTypeStrings[param->kernel_type]);
    LogInfomsg(msg);

    strcpy(msg, "output projection parameters: ");
    for (i = 0; i < 15; i++)
    {
        sprintf(tempstr, "%.2f ", param->output_space_def.orig_proj_param[i]);
        strcat(msg, tempstr);
    }
    LogInfomsg(msg);
    sprintf(msg,
        "\n\n     SDS name                      #bands in SDS    #bands "
        "to process\n");
    LogInfomsg(msg);

    for (i = 0; i < param->num_input_sds; i++)
    {
        /* Determine how many bands in this SDS are to be processed */
        if (param->input_sds_nbands[i] == 1)
        {   /* SDS only has one band */
            count = 1;
        }
        else
        {   /* SDS has multiple bands */
            count = 0;
            for (j = 0; j < param->input_sds_nbands[i]; j++)
            {
                if (param->input_sds_bands[i][j] == 1)
                    count++;
            }
        }

        sprintf(msg, "%3i) %-32s %6i %15i\n", i+1,
            param->input_sds_name_list[i], param->input_sds_nbands[i], count);
	LogInfomsg(msg);
    }

    LogInfomsg("\n");
}
