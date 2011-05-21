/* 
!C******************************************************************************

!Description: 'ConvertCorners' uses the output UL and LR lat/long corners to
 determine the UL corner in output space and the number of lines and samples
 in output space (if output spatial subset type is LAT_LONG). If the output
 spatial subset type is PROJ_COORDS, then the number of lines and samples
 are calculated from the UL and LR proj coords. If the output spatial subset
 type is LINE_SAMPLE, then the UL and LR corners are calculated given the
 UL and LR line/sample values in input space.
 
!Input Parameters:
 param             list of user parameters; the following fields are input:
                   output_spatial_subset_type

 output_space_def  output grid space definition; the following fields are
                   input: pixel_size, ul_corner.lat, ul_corner.lon,
                   lr_corner.lat, lr_corner.lon, proj_num, zone,
                   sphere, proj_param[*]
                   the following fields are output: ul_corner.x, ul_corner.y,
                   lr_corner.x, lr_corner.y, ul_corner_geo.x, ul_corner_geo.y,
                   lr_corner_geo.x, lr_corner_geo.y, img_size.l, img_size.s

!Output Parameters:
 (returns)      an integer value (true, false) specifying no error or an error

!Developers:
 Gail Schmidt
 SAIC / USGS EROS Data Center
 Rapid City, SD 57701
 gschmidt@usgs.gov
    
!Notes:

!END****************************************************************************
*/

#if !defined(__CYGWIN__) && !defined(__APPLE__) && !defined(WIN32)
#include <values.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "param.h"

#include "geoloc.h"
#include "parser.h"
#include "usage.h"

#include "myproj.h"

#include "myerror.h"
#include "mydtype.h"
#include "deg2dms.h"
#include "cproj.h"
#include "const.h"

/* Functions */

int ConvertCorners(Param_t *param)
{
  int i;
  int highest_ires;                        /* highest resolution of the SDSs */
  int ul_line, ul_samp, lr_line, lr_samp;  /* UL/LR line/sample values */
  double deg_inc;
  Map_coord_t ul, ur, ll, lr;
  double minx = MRT_FLOAT4_MAX,
         maxx = -MRT_FLOAT4_MAX,
         miny = MRT_FLOAT4_MAX,
         maxy = -MRT_FLOAT4_MAX;
  Space_t *out_space = NULL;
  Space_def_t *output_space_def = NULL;
  Geo_coord_t geo_coord_p;
  Map_coord_t output_coord_p;
  Geoloc_t *geoloc = NULL;             /* geolocation file */
  int32 start[MYHDF_MAX_RANK];         /* start reading at this location */
  int32 nval[MYHDF_MAX_RANK];          /* read this many values */

  /* Make the pointers cleaner */
  output_space_def = &(param->output_space_def);

  /* If the output spatial subset type is LINE_SAMPLE, then the UL and LR
     corner points have been provided as line/sample (in input space). First
     determine the lat/long UL/LR, then treat this as a LAT_LONG spatial
     subset type and determine the minimum bounding rectangle. */
  if (param->output_spatial_subset_type == LINE_SAMPLE)
  {
    /* Determine the highest resolution SDS that will be processed */
    highest_ires = -1;
    for (i = 0; i < param->num_input_sds; i++)
    {
      if (param->ires[i] > highest_ires)
        highest_ires = param->ires[i];
    }

    /* Determine the line/sample in the 1KM geolocation file for the
       specified line/sample in the highest resolution SDS */
    ul_samp = (int)output_space_def->ul_corner.x;
    ul_line = (int)output_space_def->ul_corner.y;
    lr_samp = (int)output_space_def->lr_corner.x;
    lr_line = (int)output_space_def->lr_corner.y;

    /* If the highest resolution is a 500m SDS, then divide the UL and
       LR line/sample values by 2.  If the highest resolution is a 250m
       SDS, then divide the UL/LR line/sample by 4.  This is their location
       in the 1KM geolocation file.  No modifications are needed if the
       highest resolution SDS is 1km. */
    if (highest_ires == 2)
    { /* 500m SDS */
      ul_samp /= 2;
      ul_line /= 2;
      lr_samp /= 2;
      lr_line /= 2;
    }
    else if (highest_ires == 4)
    { /* 250m SDS */
      ul_samp /= 4;
      ul_line /= 4;
      lr_samp /= 4;
      lr_line /= 4;
    }

    /* Open geoloc file */
    geoloc = OpenGeolocSwath(param->geoloc_file_name);
    if (geoloc == (Geoloc_t *)NULL)
      LOG_RETURN_ERROR("bad geolocation file", "ConvertCorners", false);

    /* Grab the UL longitude (0-based start values) */
    start[0] = ul_line - 1;
    start[1] = ul_samp - 1;
    nval[0] = 1;
    nval[1] = 1;
    if (SDreaddata(geoloc->sds_lon.id, start, NULL, nval,
      geoloc->lon_buf) == HDF_ERROR) {
      CloseGeoloc(geoloc);
      FreeGeoloc(geoloc);
      LOG_RETURN_ERROR("reading UL longitude", "ConvertCorners", false);
    }
    output_space_def->ul_corner.x = geoloc->lon_buf[0];

    /* Grab the UL latitude (0-based start values) */
    start[0] = ul_line - 1;
    start[1] = ul_samp - 1;
    nval[0] = 1;
    nval[1] = 1;
    if (SDreaddata(geoloc->sds_lat.id, start, NULL, nval,
      geoloc->lat_buf) == HDF_ERROR) {
      CloseGeoloc(geoloc);
      FreeGeoloc(geoloc);
      LOG_RETURN_ERROR("reading UL latitude", "ConvertCorners", false);
    }
    output_space_def->ul_corner.y = geoloc->lat_buf[0];

    /* Grab the LR longitude (0-based start values) */
    start[0] = lr_line - 1;
    start[1] = lr_samp - 1;
    nval[0] = 1;
    nval[1] = 1;
    if (SDreaddata(geoloc->sds_lon.id, start, NULL, nval,
      geoloc->lon_buf) == HDF_ERROR) {
      CloseGeoloc(geoloc);
      FreeGeoloc(geoloc);
      LOG_RETURN_ERROR("reading LR longitude", "ConvertCorners", false);
    }
    output_space_def->lr_corner.x = geoloc->lon_buf[0];

    /* Grab the LR latitude (0-based start values) */
    start[0] = lr_line - 1;
    start[1] = lr_samp - 1;
    nval[0] = 1;
    nval[1] = 1;
    if (SDreaddata(geoloc->sds_lat.id, start, NULL, nval,
      geoloc->lat_buf) == HDF_ERROR) {
      CloseGeoloc(geoloc);
      FreeGeoloc(geoloc);
      LOG_RETURN_ERROR("reading LR latitude", "ConvertCorners", false);
    }
    output_space_def->lr_corner.y = geoloc->lat_buf[0];

    /* Close geolocation file */
    if (!CloseGeoloc(geoloc)) {
      FreeGeoloc(geoloc);
      LOG_RETURN_ERROR("closing geolocation file", "ConvertCorners", false);
    }

    /* Free geolocation structure */
    if (!FreeGeoloc(geoloc))
      LOG_RETURN_ERROR("freeing geoloc file struct", "ConvertCorners", false);
  } /* if LINE_SAMPLE */

  /* If the output spatial subset type is LAT_LONG, use the lat/long corner
     points to get the UL corner in output space and the number of
     lines/samples in the output image. If the output spatial subset type
     is LINE_SAMPLE, then the corner points have been converted to lat/long.
     So handle them as LAT_LONG. */
  if (param->output_spatial_subset_type == LAT_LONG ||
      param->output_spatial_subset_type == LINE_SAMPLE)
  {
     /* Get the UR and LL lat/longs from the UL and LR lat/longs.
        Convert the corner points to RADIANS for the GCTP call. */
     output_space_def->ul_corner.x *= RAD;
     output_space_def->ul_corner.y *= RAD;
     output_space_def->lr_corner.x *= RAD;
     output_space_def->lr_corner.y *= RAD;
     ul.x = output_space_def->ul_corner.x;
     ul.y = output_space_def->ul_corner.y;
     lr.x = output_space_def->lr_corner.x;
     lr.y = output_space_def->lr_corner.y;
     ur.x = lr.x;
     ur.y = ul.y;
     ll.x = ul.x;
     ll.y = lr.y;

     /* Use the first pixel size for these calculations */
     if (output_space_def->proj_num == PROJ_GEO)
     {
       /* Convert Geographic pixel size from degrees to radians */
       output_space_def->pixel_size = param->output_pixel_size[0] * RAD;

       /* If the output projection is Geographic, then the pixel size needs
          to be in degrees. Verify that the pixel size is less than 1.0. */
       if (param->output_pixel_size[0] > 1.0)
       {
         LOG_RETURN_ERROR("for output to geographic the pixel size needs to be "
		      "in degrees", "ConvertCorners", false);
       }
     }
     else
     {
       output_space_def->pixel_size = param->output_pixel_size[0];

       /* If the output projection is non-Geographic, then the pixel size
          needs to be in meters. Verify that the pixel size is larger
          than 1.0. */
       if (param->output_pixel_size[0] < 1.0)
       {
         LOG_RETURN_ERROR("for output to non-geographic projections the pixel "
                      "size needs to be in meters", "ConvertCorners", false);
       }
     }

     /* Initialize the number of lines and samples to 1,1 just so SetupSpace
        won't complain.  These lines and samples are not used in for_trans. */
     output_space_def->img_size.s = 1;
     output_space_def->img_size.l = 1;

     /* Get the forward and reverse transformation functions */
     out_space = SetupSpace(output_space_def);
     if (out_space == (Space_t *)NULL)
         LOG_RETURN_ERROR("setting up output space", "ConvertCorners", false);

     /* UL */
#ifdef DEBUG
     printf ("Input UL lat/long (DEG): %f %f\n", ul.y*DEG, ul.x*DEG);
     printf ("Input UL lat/long (RAD): %f %f\n", ul.y, ul.x);
#endif
     geo_coord_p.lon = ul.x;
     geo_coord_p.lat = ul.y;
     geo_coord_p.is_fill = false;
     if (out_space->for_trans(geo_coord_p.lon, geo_coord_p.lat,
         &output_coord_p.x, &output_coord_p.y) != GCTP_OK) {
         FreeSpace(out_space);
         LOG_RETURN_ERROR("converting UL to output map coordinates",
                      "ConvertCorners", false);
     }
#ifdef DEBUG
     printf ("Output UL projection coords x/y: %f %f\n", output_coord_p.x,
         output_coord_p.y);
#endif

     if (output_coord_p.x < minx) minx = output_coord_p.x;
     if (output_coord_p.x > maxx) maxx = output_coord_p.x;
     if (output_coord_p.y < miny) miny = output_coord_p.y;
     if (output_coord_p.y > maxy) maxy = output_coord_p.y;

  /* UR */
#ifdef DEBUG
     printf ("Input UR lat/long (DEG): %f %f\n", ur.y*DEG, ur.x*DEG);
     printf ("Input UR lat/long (RAD): %f %f\n", ur.y, ur.x);
#endif
     geo_coord_p.lon = ur.x;
     geo_coord_p.lat = ur.y;
     geo_coord_p.is_fill = false;
     if (out_space->for_trans(geo_coord_p.lon, geo_coord_p.lat,
         &output_coord_p.x, &output_coord_p.y) != GCTP_OK) {
         FreeSpace(out_space);
         LOG_RETURN_ERROR("converting UR to output map coordinates",
                      "ConvertCorners", false);
     }
#ifdef DEBUG
     printf ("Output UR projection coords x/y: %f %f\n", output_coord_p.x,
         output_coord_p.y);
#endif

     if (output_coord_p.x < minx) minx = output_coord_p.x;
     if (output_coord_p.x > maxx) maxx = output_coord_p.x;
     if (output_coord_p.y < miny) miny = output_coord_p.y;
     if (output_coord_p.y > maxy) maxy = output_coord_p.y;

  /* LL */
#ifdef DEBUG
     printf ("Input LL lat/long (DEG): %f %f\n", ll.y*DEG, ll.x*DEG);
     printf ("Input LL lat/long (RAD): %f %f\n", ll.y, ll.x);
#endif
     geo_coord_p.lon = ll.x;
     geo_coord_p.lat = ll.y;
     geo_coord_p.is_fill = false;
     if (out_space->for_trans(geo_coord_p.lon, geo_coord_p.lat,
         &output_coord_p.x, &output_coord_p.y) != GCTP_OK) {
         FreeSpace(out_space);
         LOG_RETURN_ERROR("converting LL to output map coordinates",
                      "ConvertCorners", false);
     }
#ifdef DEBUG
     printf ("Output LL projection coords x/y: %f %f\n", output_coord_p.x,
         output_coord_p.y);
#endif

     if (output_coord_p.x < minx) minx = output_coord_p.x;
     if (output_coord_p.x > maxx) maxx = output_coord_p.x;
     if (output_coord_p.y < miny) miny = output_coord_p.y;
     if (output_coord_p.y > maxy) maxy = output_coord_p.y;

     /* LR */
#ifdef DEBUG
     printf ("Input LR lat/long (DEG): %f %f\n", lr.y*DEG, lr.x*DEG);
     printf ("Input LR lat/long (RAD): %f %f\n", lr.y, lr.x);
#endif
     geo_coord_p.lon = lr.x;
     geo_coord_p.lat = lr.y;
     geo_coord_p.is_fill = false;
     if (out_space->for_trans(geo_coord_p.lon, geo_coord_p.lat,
         &output_coord_p.x, &output_coord_p.y) != GCTP_OK) {
         FreeSpace(out_space);
         LOG_RETURN_ERROR("converting LR to output map coordinates",
                      "ConvertCorners", false);
     }
#ifdef DEBUG
     printf ("Output LR projection coords x/y: %f %f\n", output_coord_p.x,
         output_coord_p.y);
#endif

     if (output_coord_p.x < minx) minx = output_coord_p.x;
     if (output_coord_p.x > maxx) maxx = output_coord_p.x;
     if (output_coord_p.y < miny) miny = output_coord_p.y;
     if (output_coord_p.y > maxy) maxy = output_coord_p.y;

     /* Walk the boundary of the image looking for the min and max x/y coords.
        Check 5 points along each boundary. */
     /* Left */
     geo_coord_p.lon = ul.x;
     geo_coord_p.is_fill = false;
     deg_inc = (ul.y - ll.y) / 5.0;
     for (i = 0; i < 5; i++)
     {
       geo_coord_p.lat = ul.y - i * deg_inc;
       if (out_space->for_trans(geo_coord_p.lon, geo_coord_p.lat,
           &output_coord_p.x, &output_coord_p.y) != GCTP_OK) {
           FreeSpace(out_space);
           LOG_RETURN_ERROR("converting left side to output map coordinates",
                        "ConvertCorners", false);
       }

       if (output_coord_p.x < minx) minx = output_coord_p.x;
       if (output_coord_p.x > maxx) maxx = output_coord_p.x;
       if (output_coord_p.y < miny) miny = output_coord_p.y;
       if (output_coord_p.y > maxy) maxy = output_coord_p.y;
     }

     /* Right */
     geo_coord_p.lon = ur.x;
     geo_coord_p.is_fill = false;
     deg_inc = (ur.y - lr.y) / 5.0;
     for (i = 0; i < 5; i++)
     {
       geo_coord_p.lat = ur.y - i * deg_inc;
       if (out_space->for_trans(geo_coord_p.lon, geo_coord_p.lat,
           &output_coord_p.x, &output_coord_p.y) != GCTP_OK) {
           FreeSpace(out_space);
           LOG_RETURN_ERROR("converting right side to output map coordinates",
                        "ConvertCorners", false);
       }

       if (output_coord_p.x < minx) minx = output_coord_p.x;
       if (output_coord_p.x > maxx) maxx = output_coord_p.x;
       if (output_coord_p.y < miny) miny = output_coord_p.y;
       if (output_coord_p.y > maxy) maxy = output_coord_p.y;
     }

     /* Top */
     geo_coord_p.lat = ul.y;
     geo_coord_p.is_fill = false;
     deg_inc = (ul.x - ur.x) / 5.0;
     for (i = 0; i < 5; i++)
     {
       geo_coord_p.lon = ul.x - i * deg_inc;
       if (out_space->for_trans(geo_coord_p.lon, geo_coord_p.lat,
           &output_coord_p.x, &output_coord_p.y) != GCTP_OK) {
           FreeSpace(out_space);
           LOG_RETURN_ERROR("converting top side to output map coordinates",
                        "ConvertCorners", false);
       }

       if (output_coord_p.x < minx) minx = output_coord_p.x;
       if (output_coord_p.x > maxx) maxx = output_coord_p.x;
       if (output_coord_p.y < miny) miny = output_coord_p.y;
       if (output_coord_p.y > maxy) maxy = output_coord_p.y;
     }

     /* Bottom */
     geo_coord_p.lat = ll.y;
     geo_coord_p.is_fill = false;
     deg_inc = (ll.x - lr.x) / 5.0;
     for (i = 0; i < 5; i++)
     {
       geo_coord_p.lon = ll.x - i * deg_inc;
       if (out_space->for_trans(geo_coord_p.lon, geo_coord_p.lat,
           &output_coord_p.x, &output_coord_p.y) != GCTP_OK) {
           FreeSpace(out_space);
           LOG_RETURN_ERROR("converting bottom side to output map coordinates",
                        "ConvertCorners", false);
       }

       if (output_coord_p.x < minx) minx = output_coord_p.x;
       if (output_coord_p.x > maxx) maxx = output_coord_p.x;
       if (output_coord_p.y < miny) miny = output_coord_p.y;
       if (output_coord_p.y > maxy) maxy = output_coord_p.y;
     }

     /* The pixel size and number of lines/samples is specified for each
        SDS.  The overall UL and LR corners will be the same for each SDS,
        however the pixel size and number of lines/samples might be
        different. */
     for (i = 0; i < param->num_input_sds; i++)
     {
       if (output_space_def->proj_num == PROJ_GEO)
       {
         /* Convert Geographic pixel size from degrees to radians */
         param->output_pixel_size[i] *= RAD;
       }

       /* Calculate the number of output lines and samples */
       param->output_img_size[i].s =
         (int) ((maxx - minx) / param->output_pixel_size[i] + 0.5);
       param->output_img_size[i].l =
         (int) ((maxy - miny) / param->output_pixel_size[i] + 0.5);

#ifdef DEBUG
       if (output_space_def->proj_num == PROJ_GEO)
         printf ("Pixel size is %f\n", param->output_pixel_size[i]*DEG);
       else
         printf ("Pixel size is %f\n", param->output_pixel_size[i]);
       printf ("Output number of lines is %d\n",
         param->output_img_size[i].l);
       printf ("Output number of samples is %d\n",
         param->output_img_size[i].s);
#endif
     }

     /* Use the first pixel size and number of lines/samples for the
        rest of the calculations */
     output_space_def->pixel_size = param->output_pixel_size[0];
     output_space_def->img_size.l = param->output_img_size[0].l;
     output_space_def->img_size.s = param->output_img_size[0].s;

     /* Redefine the output coords to use the minimum bounding box in
        output projection coords. Also make the LR corner a factor of the
        number of lines and samples. */
     output_space_def->ul_corner.x = minx;
     output_space_def->ul_corner.y = maxy;
     output_space_def->lr_corner.x = output_space_def->ul_corner.x +
         output_space_def->img_size.s * output_space_def->pixel_size;
     output_space_def->lr_corner.y = output_space_def->ul_corner.y -
         output_space_def->img_size.l * output_space_def->pixel_size;

#ifdef DEBUG
     if (output_space_def->proj_num == PROJ_GEO)
     {
       printf ("Output UL projection coords: %f %f\n",
         output_space_def->ul_corner.x*DEG, output_space_def->ul_corner.y*DEG);
       printf ("Output LR projection coords: %f %f\n",
         output_space_def->lr_corner.x*DEG, output_space_def->lr_corner.y*DEG);
     }
     else
     {
       printf ("Output UL projection coords: %f %f\n",
         output_space_def->ul_corner.x, output_space_def->ul_corner.y);
       printf ("Output LR projection coords: %f %f\n",
         output_space_def->lr_corner.x, output_space_def->lr_corner.y);
     }
#endif
  }

  /* If the output spatial subset type is PROJ_COORDS, use the UL and LR
     projection coords to determine the number of lines and samples in the
     output image. Also determine the lat/long values. */
  else if (param->output_spatial_subset_type == PROJ_COORDS)
  {
     /* If the input was in projection coords and the projection is geographic,
        then the corners are in degrees.  We need to make sure they are in
        radians for the rest of the processing. */
     if (output_space_def->proj_num == PROJ_GEO)
     {
        output_space_def->ul_corner.x *= RAD;
        output_space_def->ul_corner.y *= RAD;
        output_space_def->lr_corner.x *= RAD;
        output_space_def->lr_corner.y *= RAD;
     }

     /* The pixel size and number of lines/samples is specified for each
        SDS.  The overall UL and LR corners will be the same for each SDS,
        however the pixel size and number of lines/samples might be
        different. */
     for (i = 0; i < param->num_input_sds; i++)
     {
       /* If the output projection is Geographic, then the pixel size needs
          to be in degrees. Verify that the pixel size is larger than 1.0. */
       if (output_space_def->proj_num == PROJ_GEO &&
           param->output_pixel_size[i] > 1.0)
       {
         FreeSpace(out_space);
         LOG_RETURN_ERROR("for output to geographic the pixel size needs to be "
                      "in degrees", "ConvertCorners", false);
       }
       else if (output_space_def->proj_num == PROJ_GEO)
       {
         /* Convert Geographic pixel size from degrees to radians */
         param->output_pixel_size[i] *= RAD;
       }

       /* Determine the number of lines and samples */
       param->output_img_size[i].l =
          (int) (((fabs (output_space_def->lr_corner.y -
          output_space_def->ul_corner.y)) / param->output_pixel_size[i]) +
          0.5);
       param->output_img_size[i].s =
          (int) (((fabs (output_space_def->lr_corner.x -
          output_space_def->ul_corner.x)) / param->output_pixel_size[i]) +
          0.5);
     }

     /* Use the first pixel size and number of lines/samples for the
        rest of the calculations */
     output_space_def->pixel_size = param->output_pixel_size[0];
     output_space_def->img_size.l = param->output_img_size[0].l;
     output_space_def->img_size.s = param->output_img_size[0].s;

     /* Recalculate the LR corner based on the number of lines and samples,
        since the projection coords might not have been an exact number of
        lines and samples. */
     output_space_def->lr_corner.x = output_space_def->ul_corner.x +
         output_space_def->img_size.s * output_space_def->pixel_size;
     output_space_def->lr_corner.y = output_space_def->ul_corner.y -
         output_space_def->img_size.l * output_space_def->pixel_size;

     /* Get the forward and reverse transformation functions */
     out_space = SetupSpace(output_space_def);
     if (out_space == (Space_t *)NULL)
         LOG_RETURN_ERROR("setting up output space", "ConvertCorners", false);
  }

  /* Determine/Redetermine the lat/long of the UL corner */
  ul.x = output_space_def->ul_corner.x;
  ul.y = output_space_def->ul_corner.y;
  if (out_space->inv_trans(ul.x, ul.y, &geo_coord_p.lon, &geo_coord_p.lat)
      != GCTP_OK) {
      FreeSpace(out_space);
      LOG_RETURN_ERROR("converting UL to output lat/long coordinates",
                   "ConvertCorners", false);
  }
  output_space_def->ul_corner_geo.lat = geo_coord_p.lat;
  output_space_def->ul_corner_geo.lon = geo_coord_p.lon;

  /* Determine the lat/long of the LR corner */
  lr.x = output_space_def->lr_corner.x;
  lr.y = output_space_def->lr_corner.y;
  if (out_space->inv_trans(lr.x, lr.y, &geo_coord_p.lon, &geo_coord_p.lat)
      != GCTP_OK) {
      FreeSpace(out_space);
      LOG_RETURN_ERROR("converting LR to output lat/long coordinates",
                   "ConvertCorners", false);
  }
  output_space_def->lr_corner_geo.lat = geo_coord_p.lat;
  output_space_def->lr_corner_geo.lon = geo_coord_p.lon;

  /* Free the output space */
  FreeSpace(out_space);

  return true;
}
