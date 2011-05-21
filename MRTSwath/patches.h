/*
!C****************************************************************************

!File: patches.h

!Description: Header file for patches.c - see patches.c for more information.

!Revision History:
 Revision 1.0 2001/05/08
 Robert Wolfe
 Original Version.

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
      phone: 301-614-5508               Lanham, MD 20706  

 ! Design Notes:
   1. Structures are declared for the Patches_mem, Patches_loc and Patches 
      data types.
   2. The data type Patch_status is defined.
   3. The number of samples and lines in a patch are 'NLINE_PATCH' and 
      'NSAMPLE_PATCH', respectively.
  
!END****************************************************************************
*/

#ifndef PATCHES_H
#define PATCHES_H

#include <stdio.h>
#include "resamp.h"
#include "output.h"
#include "bool.h"
#include "geowrpr.h"

/* Constants */

#define NLINE_PATCH (32)  /* Number of lines in a patch */
#define NSAMPLE_PATCH (32)  /* Number of samples per line in patch */

/* Data type for patch status */

typedef enum {
  PATCH_NULL,           /* Null (uninitialized) patch */
  PATCH_IN_MEM,         /* Patch in memory */
  PATCH_ON_DISK         /* Patch on disk */
} Patch_status_t;

/* Structure for patches in memory */

typedef struct Patches_mem_s {
  int ntouch;           /* Flag indicating that patch was touched in the last 
                           'ntouch' iterations; a value of -1 indicates the 
			   patch has not been touched recently and is eligable
			   to be written to disk */
  struct Patches_mem_s *prev;  /* Pointer to previous patch */
  struct Patches_mem_s *next;  /* Pointer to next patch */
  Img_coord_int_t loc;  /* Patch location (patch coordinates) */
  double *sum[NLINE_PATCH];  /* Array containing sum of weighted values 
                               for each output pixel in patch */
  double *weight[NLINE_PATCH];  /* Array containing sum of weights 
                                  for each output pixel in patch */

  double* nn_wt[NLINE_PATCH]; /* Array containing the weight of the 
                                 nearest neighbor for each output pixel in patch */

} Patches_mem_t;

/* Structure for patch location, either on disk or in memory */

typedef struct {
  Patch_status_t status;
  union {
    long loc;           /* Start byte on disk */
    Patches_mem_t *pntr;  /* Pointer in memory */
  } u;
} Patches_loc_t;

/* Structure for each patch */

typedef struct {
  char *file_name;      /* Temporary patch file name */
  FILE *file;           /* File I/O data structure */
  Img_coord_int_t size;  /* Output product image size */
  Img_coord_int_t npatch;  /* Number of patches in each dimension */
  long nmem;            /* Number of patches allocated in memory */
  long nmem_max;        /* Maximum number of patches in memory threshold */
  long nmem_alloc;      /* Number of patches to allocate when more are needed */
  long nused;           /* Number of patches in memory that are being used */
  long nnull;           /* Number of patches in memory that are null
                           (uninitialized) */
  int32 data_type;      /* Input product data type */
  double fill_value;    /* Output product fill value */
  size_t data_type_size;  /* Size of input product data type (bytes) */
  size_t patch_size;    /* Size of a patch (bytes) */
  long file_size;       /* Current temporary patch file size (bytes) */
  Patches_mem_t *used_list; /* Head of list of patches being used */
  Patches_mem_t *null_list; /* Head of list of null patches */
  union {               /* Output buffer (for each output data type) */
    void *val_void[NLINE_PATCH];
    char8 *val_char8[NLINE_PATCH];
    uint8 *val_uint8[NLINE_PATCH];
    int8 *val_int8[NLINE_PATCH];
    int16 *val_int16[NLINE_PATCH];
    uint16 *val_uint16[NLINE_PATCH];
    int32 *val_int32[NLINE_PATCH];
    uint32 *val_uint32[NLINE_PATCH];
  } buf;
  Patches_loc_t **loc;  /* Array containing location of each patch */
  Patches_mem_t **mem;  /* Array containing pointer to patch in memory
                           for each patch location */
} Patches_t;

/* Prototypes */

Patches_t *SetupPatches(Img_coord_int_t *img_size, char *file_name,
                        int32 input_data_type, int input_fill_value);
bool FreePatchesInMem(Patches_t *this);
bool FreePatches(Patches_t *this);
bool InitPatchInMem(Patches_t *this, int il_patch, int is_patch);
bool UntouchPatches(Patches_t *this);
bool TossPatches(Patches_t *this, int32 output_data_type);
bool UnscramblePatches(Patches_t *this, Output_t *output,
     Output_file_format_t output_format, FILE_ID *GeoTiffFile,
     FILE *rbfile, int32 output_data_type, Kernel_type_t kernel_type);
bool FillOutput(void *void_buf[NLINE_PATCH], int nlines, int nsamps,
     int32 output_data_type, double fill_value, double slope,
     bool same_data_type);
int FindMedian(int *buf, int nvals);

#endif
