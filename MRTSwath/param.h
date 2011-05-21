/*
!C****************************************************************************

!File: param.h

!Description: Header file for 'param.c' - see 'param.c' for more information.

!Revision History:
 Revision 1.0 2000/11/07
 Robert Wolfe
 Original Version.

 Revision 1.1 2000/12/13
 Sadashiva Devadiga
 Modified to accept parameters from command line or file.

 Revision 1.2 2001/05/08
 Sadashiva Devadiga
 Cleanup.

 Revision 1.3 2002/05/10
 Robert Wolfe
 Added separate output SDS name.

 Revision 2.0 2003/11
 Gail Schmidt
 Added support for output to raw binary, in addition to Geotiff and HDF.
 Added support for multiple pixel sizes.

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

      Sadashiva Devadiga (Code 922)
      MODIS Land Team Support Group     SSAI
      devadiga@ltpmail.gsfc.nasa.gov    5900 Princess Garden Pkway, #300
      phone: 301-614-5549               Lanham, MD 20706

 ! Design Notes:
   1. Structures are declared for the 'input_space_type' and 'param' data types.
   2. The acronym SDS stands for Science Data Set.
  
!END****************************************************************************
*/

#ifndef PARAM_H
#define PARAM_H

#include <hdf.h>
#include "hlimits.h"
#include "kernel.h"
#include "space.h"

/* Input space type definition */
/* NOTE: GRID_SPACE is not suppoted in MRTSwath */

typedef enum {
  SWATH_SPACE,          /* Swath (L1/L2) input space */
  /* NOTE: GRID_SPACE is not suppoted in MRTSwath */
  GRID_SPACE            /* Grid (L2G/L3/L4) input space */
} Input_space_type_t;

typedef enum {
  HDF_FMT,                 /* HDF file format output */
  GEOTIFF_FMT,             /* Geotiff file format output */
  RB_FMT,                  /* Raw binary file format output */
  BOTH                     /* Both Geotiff & HDF file formats output */
} Output_file_format_t;

typedef enum {
  LAT_LONG,                /* UL/LR corners are in lat/long */
  PROJ_COORDS,             /* UL/LR corners are in projection coords */
  LINE_SAMPLE              /* UL/LR corners are specified by starting
                              line/sample and number of lines/samples */
} Output_spatial_subset_t;

/* Parameter data structure type definition */

typedef struct {
  bool multires;           /* Did the user specify multiple resolutions for
                              the output product? */
  char *input_file_name;   /* Name of the input image HDF file */
  char *output_file_name;  /* Name of the output image HDF file */
  char *geoloc_file_name;  /* Name of the input geolocation HDF file */
  Output_file_format_t output_file_format; /* Output file format (see above) */
  Input_space_type_t input_space_type;     /* Input space type (see above) */
  int num_input_sds;       /* Number of input SDSs to be processed */
  int ires[MAX_SDS_DIMS];  /* Input resolution (relative to 1 km input) for
                              each specified SDS: 1 = 1 km; 2 = 500 m;
                              4 = 250 m */
  char input_sds_name_list[MAX_SDS_DIMS][MAX_NC_NAME]; /* Input list of HDF SDS
                             names to process (actual number of SDSs is
                             num_input_sds) */
  int input_sds_nbands[MAX_SDS_DIMS];  /* Number of bands in the input SDSs */
  int input_sds_bands[MAX_SDS_DIMS][MAX_VAR_DIMS];  /* Array of 0s and 1s
                             specifying which bands in the SDS should be
                             processed (actual number of bands in this SDS
                             is input_sds_nbands) */
  char *input_sds_name;   /* Current input HDF SDS name */
  char *output_sds_name;  /* Output HDF SDS name */
  int iband;              /* Input band number */
  int rank[MAX_SDS_DIMS]; /* Rank (number of dimensions) for each SDS */
  int dim[MAX_SDS_DIMS][MYHDF_MAX_RANK];/* Dimension flags for each SDS;
                             the line and sample dimensions are indicated by
                             a -1 and -2 in this array, respectively; the
                             index in the other dimensions are indicated by a
                             value of zero or greater */
  Kernel_type_t kernel_type;    /* Input kernel type (see 'kernel.h') */
  Space_def_t input_space_def;  /* Input space map projection information */
  Space_def_t output_space_def; /* Output space map projection information */
  Output_spatial_subset_t output_spatial_subset_type;  /* Output spatial
                            subset type - default is lat/long (see above) */
  int output_data_type;  /* Output data type (-1 indicates same
                            as input type) */
  int output_dt_arr[MAX_SDS_DIMS];  /* Output data type storage for output
                                       to the metadata file, one for each SDS */
  double fill_value[MAX_SDS_DIMS];  /* Background fill val, one for each SDS */
  bool create_output[MAX_SDS_DIMS]; /* Create output flag ('true' = create
                                       output, 'false' = don't create output)
                                       one for each SDS */
  char *patches_file_name;  /* Patches file name; this is an intermediate file
                            that can be deleted after the program exits */
  double output_pixel_size[MAX_SDS_DIMS]; /* Output pixel size (meters,
                                         degrees for GEO) one for each SDS */
  Img_coord_int_t output_img_size[MAX_SDS_DIMS]; /* Output image size
                                         (lines, samples) one for each SDS */
} Param_t;

/* Prototypes */

int ConvertCorners(Param_t *param);
Param_t *GetParam(int argc, const char **argv);
bool FreeParam(Param_t *param);
int ReadSDS(Param_t *param);
bool SDSInfo(Param_t *param);
Param_t *CopyParam(Param_t *param);
void PrintParam(Param_t *param);

#endif
