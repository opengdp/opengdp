/*
!C****************************************************************************

!File: patches.c
  
!Description: Functions for storing patches of intermediate output image in 
 memory or in a temporary output file.

!Revision History:
 Revision 1.0 2001/05/08
 Robert Wolfe
 Original Version.

 Revision 1.5 2002/12/02
 Gail Schmidt
 Added support for INT8 data type. Also, char8 needs to be referenced to
 char8 and not int8.

 Revision 1.5 2002/12/03
 Gail Schmidt
 Patches will now be stored in the input data type. The conversion to
 the output data type (if different from the input data type) will attempt
 to retain more information by using the input data type range and the
 output data type range to determine a starting value and slope for the
 conversion.

 Revision 1.6 2003/10/03
 Gail Schmidt
 Fixed a bug that caused problems with HDF_FMT only in the .hdf output
 files. The BOTH .hdf output files were fine.

 Revision 2.0 2003/11/11
 Gail Schmidt
 Support output to raw binary.

 Revision 2.0a 2004/08/21
 Gail Schmidt
 Pass the fill value in after reading from the input SDS.

 Revision 2.0b 2008/12/01
 Maverick Merrirr

 Merged fixes from mrtswath_strip.  Primarily, the "Modified code to fill in
 any holes in the output product when using NN as the processing kernel." and
 the "Added support for INT32 and UINT32." fixes made by Gail Schmidt.

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
   1. The following public functions handle the image patches:

       SetupPatches - Set up the 'patches' data structure.
       FreePatchesInMem - Free allocated space for patches in memory.
       FreePatches - Free 'patches' data structure.
       InitPatchInMem - Initialize a patch in memory.
       UntouchPatches - Mark all patches as eligible to be written to 
         temporary disk file.
       TossPatches - Write all eligible (complete) patches to temporary disk 
         file.
       UnscramblePatches - Read patches from temporary disk file and write them
         to output product HDF file.

   2. The following internal functions are also used to handle the patches:

       CreatePatches - Create (allocate) more patches in memory.
       ConvertToChar8 - Convert a float to a HDF CHAR8 data type.
       ConvertToUint8 - Convert a float to a HDF UINT8 data type.
       ConvertToInt8 - Convert a float to a HDF INT8 data type.
       ConvertToInt16 - Convert a float to a HDF INT16 data type.
       ConvertToUint16 - Convert a float to a HDF UINT16 data type.
       ConvertToInt32 - Convert a float to a HDF INT32 data type.
       ConvertToUint32 - Convert a float to a HDF UINT32 data type.

   3. Each image patch is initially uninitialized. Once initialized, it is
      either in memory or on disk.
   4. 'SetupPatches' must be called before any of the other routines.  
   5. 'FreePatchesInMem' must be called before 'FreePatches'.
   6. 'FreePatches' should be used to free the 'patches' data structure.

!END****************************************************************************
*/

#include <stdlib.h>
#include <math.h>
#include "myerror.h"
#include "patches.h" 
#include "mystring.h"
#include "range.h"
#include "param.h"
#include "rb.h"
#include "geowrpr.h"
#include <sys/types.h>
#ifdef __CYGWIN__
#include <getopt.h>             /* getopt  prototype */
#else
#ifndef WIN32
#include <unistd.h>             /* getopt  prototype */
#endif
#endif
#include "winpid.h"

/* Macros */

#define MAX2(a, b) (((a) >= (b)) ? (a) : (b))

/* Constants */

#define NPATCH_MEM_INIT (4)  /* Initial number of sets of patches in memory */
#define NPATCH_MEM_MAX (60)  /* Maximum number of sets of patches in memory */
#define MIN_WEIGHT (0.10)    /* Minimum weight for a valid output pixel */

/* #define DEBUG_ZEROS */

/* Functions */

bool CreatePatches(Patches_t *this)
/* 
!C******************************************************************************

!Description: 'CreatePatches' creates (allocates) more patches in memory.
 
!Input Parameters:
 this           patches structure; the following fields are input:
                  nmem, nmem_alloc, nmem_max, null_list, nnull, 
		  (mem_p)->prev, (mem_p)->next

!Output Parameters:
 this           patches structure; the following fields are modified:
                  nmem, nnull, null_list, mem[*], (mem_p)->ntouch,
		  (mem_p)->prev, (mem_p)->next, (mem_p)->loc, 
		  (mem_p)->sum, (mem_p)->weight, (mem_p)->nn_wt
 (returns)      status:
                  'true' = okay
		  'false' = error return

!Team Unique Header:

 ! Design Notes:
   1. An error status is returned when:
       a. the maximum allowable patches in memory is exceeded
       b. memory allocation is not successful
   2. Error messages are handled with the 'LOG_RETURN_ERROR' macro.
   3. 'SetupPatches' must be called before this routine.
   4. 'FreePatchesInMem' should be called to deallocate the memory.

!END****************************************************************************
*/
{
  Patches_mem_t *mem_p;
  long ip;
  long nmem1, nmem2;
  Patches_mem_t *prev_mem_p, *next_mem_p;
  double *sum_p, *weight_p, *nn_wt_p;
  size_t n;
  int il;

  /* Check to see if we are being ask to allocate too many patches */

  nmem1 = this->nmem; 
  nmem2 = nmem1 + this->nmem_alloc;
  if (nmem2 >= this->nmem_max)
    LOG_RETURN_ERROR("exceeded maximum allowable patches in memory",
    "CreatePatches", false);

  /* Allocate the patches */

  mem_p = (Patches_mem_t *)calloc((size_t)this->nmem_alloc, 
                                  sizeof(Patches_mem_t));
  if (mem_p == (Patches_mem_t *)NULL)
    LOG_RETURN_ERROR("allocating patches memory array", "CreatePatches", 
                 false);

  /* Find the last patch at the end of the null list */

  prev_mem_p = (Patches_mem_t *)NULL;
  next_mem_p = this->null_list;
  while (next_mem_p != (Patches_mem_t *)NULL) {
    prev_mem_p = next_mem_p;
    next_mem_p = next_mem_p->next;
  }

  /* Initialize all new patches to null and add to null list */

  n = (size_t)(NLINE_PATCH * NSAMPLE_PATCH);

  for (ip = nmem1; ip < nmem2; ip++) {
    this->mem[ip] = mem_p;
    mem_p->ntouch = -1;
    mem_p->loc.l = -1;
    mem_p->loc.s = -1;

    mem_p->prev = prev_mem_p;
    if (mem_p->prev == (Patches_mem_t *)NULL)
      this->null_list = mem_p;
    else
      prev_mem_p->next = mem_p;

    mem_p->next = (Patches_mem_t *)NULL;

    this->nmem++;
    this->nnull++;

    /* Allocate and initialize sum and weight arrays */

    sum_p = (double *)calloc(n, sizeof(double));
    if (sum_p == (double *)NULL)
      LOG_RETURN_ERROR("allocating memory for a patch's sum", 
                   "CreatePatches", false);

    weight_p = (double *)calloc(n, sizeof(double));
    if (weight_p == (double *)NULL) {
      free(sum_p);
      LOG_RETURN_ERROR("allocating memory for a patch's weights", 
                   "CreatePatches", false);
    }

    nn_wt_p = (double *)calloc(n, sizeof(double));
    if (nn_wt_p == (double *)NULL) 
    {
      free(sum_p);
      free(weight_p);
      LOG_RETURN_ERROR("allocating memory for a patch's nearest neighbor weights", 
                   "CreatePatches", false);
    }

    for (il = 0; il < NLINE_PATCH; il++) {
       mem_p->sum[il]    = sum_p;    sum_p    += NSAMPLE_PATCH;
       mem_p->weight[il] = weight_p; weight_p += NSAMPLE_PATCH;
       mem_p->nn_wt[il]  = nn_wt_p;  nn_wt_p  += NSAMPLE_PATCH;
    }

    prev_mem_p = mem_p;
    mem_p++;
  }

  return true;
}


Patches_t *SetupPatches(Img_coord_int_t *img_size, char *file_name,
                        int32 input_data_type, int input_fill_value)
/* 
!C******************************************************************************

!Description: 'SetupPatches' sets up the 'patches' data structure and temporary 
 disk file.
 
!Input Parameters:
 img_size       output image size
 file_name      temporary disk file name
 input_data_type  input HDF data type; data types currently supported are
                     CHAR8, UINT8, INT8, INT16, UINT16, UINT32 and INT32
 input_fill_value int fill value from the input SDS

!Output Parameters:
 (returns)      'patches' data structure or NULL when an error occurs

!Team Unique Header:

 ! Design Notes:
   1. An error status is returned when:
       a. the image dimensions are zero or negative
       b. an invalid output data type is passed
       c. memory allocation is not successful
       d. duplicating strings is not successful
       e. errors occur when opening the temporary disk file
   2. Error messages are handled with the 'LOG_RETURN_ERROR' macro.
   3. 'FreePatchesInMem' and 'FreePatches' should be called to deallocate 
      memory used by the 'patches' data structures.

!END****************************************************************************
*/
{
  Patches_t *this;
  Patches_loc_t *loc_p;
  size_t n;
  int il, is;
  long ip;
  char8 *val_char8_p;
  uint8 *val_uint8_p;
  int8 *val_int8_p;
  int16 *val_int16_p;
  uint16 *val_uint16_p;
  int32 *val_int32_p;
  uint32 *val_uint32_p;

  char *error_string = (char *)NULL;

  pid_t ThisPid;
  char CharThisPid[256];
  char FinalFileName[1024];

  /* Check some input parameters */

  if (img_size->l < 0)
     LOG_RETURN_ERROR("output number of lines is less than 1",
                           "SetupPatches", (Patches_t *)NULL);
  if (img_size->s < 0)
     LOG_RETURN_ERROR("output number of samples is less than 1",
                                    "SetupPatches", (Patches_t *)NULL);

  /* Create the patches data structure */

  this = (Patches_t *)malloc(sizeof(Patches_t));
  if (this == (Patches_t *)NULL) 
    LOG_RETURN_ERROR("allocating patches structure", "SetupPatches", 
                     (Patches_t *)NULL);

  /* Populate the data structure */
  this->size.l = img_size->l;
  this->size.s = img_size->s;
  this->npatch.l = ((img_size->l - 1) / NLINE_PATCH) + 1;
  this->npatch.s = ((img_size->s - 1) / NSAMPLE_PATCH) + 1;
  this->used_list = (Patches_mem_t *)NULL;
  this->null_list = (Patches_mem_t *)NULL;
  this->nmem = 0;
  this->nmem_alloc = MAX2(this->npatch.l, this->npatch.s);
  this->nmem_max = this->nmem_alloc * NPATCH_MEM_MAX;
  this->nused = 0;
  this->nnull = 0;
  this->data_type = input_data_type;
  this->fill_value = (float) input_fill_value;
#ifdef DEBUG_ZEROS
  this->fill_value = -1.0;
#endif
  this->file_size = 0;

  /* Open temporary file for write access */
  ThisPid = getpid();
  sprintf(CharThisPid,"%d",ThisPid);
  strcpy(FinalFileName,file_name);
  strcat(FinalFileName,CharThisPid);

  this->file_name = DupString(FinalFileName);
  if (this->file_name == (char *)NULL) {
    free(this);
    LOG_RETURN_ERROR("copying name of temporary file",
                 "SetupPatches", (Patches_t *)NULL);
  }
  this->file = fopen(this->file_name, "w+b");
  if (this->file == (FILE *)NULL) {
    free(this->file_name);
    free(this);
    LOG_RETURN_ERROR("opening temporary file", 
                 "SetupPatches", (Patches_t *)NULL);
  }

  /* Set up the patch i/o buffer */

  n = (size_t)(NLINE_PATCH * NSAMPLE_PATCH);
  switch (this->data_type) {
    case DFNT_CHAR8:
      this->data_type_size = sizeof(char8);
      val_char8_p = (char8 *)calloc(n, this->data_type_size);
      if (val_char8_p == (char8 *)NULL) 
        error_string = "allocating patches i/o buffer";
      for (il = 0; il < NLINE_PATCH; il++) {
        this->buf.val_char8[il] = val_char8_p;
	val_char8_p += NSAMPLE_PATCH;
      }
      break;
    case DFNT_UINT8:
      this->data_type_size = sizeof(uint8);
      val_uint8_p = (uint8 *)calloc(n, this->data_type_size);
      if (val_uint8_p == (uint8 *)NULL)  
        error_string = "allocating patches i/o buffer";
      for (il = 0; il < NLINE_PATCH; il++) {
        this->buf.val_uint8[il] = val_uint8_p;
	val_uint8_p += NSAMPLE_PATCH;
      }
      break;
    case DFNT_INT8:
      this->data_type_size = sizeof(int8);
      val_int8_p = (int8 *)calloc(n, this->data_type_size);
      if (val_int8_p == (int8 *)NULL)  
        error_string = "allocating patches i/o buffer";
      for (il = 0; il < NLINE_PATCH; il++) {
        this->buf.val_int8[il] = val_int8_p;
	val_int8_p += NSAMPLE_PATCH;
      }
      break;
    case DFNT_INT16:
      this->data_type_size = sizeof(int16);
      val_int16_p = (int16 *)calloc(n, this->data_type_size);
      if (val_int16_p == (int16 *)NULL) 
        error_string = "allocating patches i/o buffer";
      for (il = 0; il < NLINE_PATCH; il++) {
        this->buf.val_int16[il] = val_int16_p;
	val_int16_p += NSAMPLE_PATCH;
      }
      break;
    case DFNT_UINT16:
      this->data_type_size = sizeof(uint16);
      val_uint16_p = (uint16 *)calloc(n, this->data_type_size);
      if (val_uint16_p == (uint16 *)NULL) 
        error_string = "allocating patches i/o buffer";
      for (il = 0; il < NLINE_PATCH; il++) {
        this->buf.val_uint16[il] = val_uint16_p;
	val_uint16_p += NSAMPLE_PATCH;
      }
      break;
    case DFNT_UINT32:
      this->data_type_size = sizeof(uint32);
      val_uint32_p = (uint32 *)calloc(n, this->data_type_size);
      if (val_uint32_p == (uint32 *)NULL)
        error_string = "allocating patches i/o buffer";
      for (il = 0; il < NLINE_PATCH; il++) {
        this->buf.val_uint32[il] = val_uint32_p;
        val_uint32_p += NSAMPLE_PATCH;
      }
      break;
    case DFNT_INT32:
      this->data_type_size = sizeof(int32);
      val_int32_p = (int32 *)calloc(n, this->data_type_size);
      if (val_int32_p == (int32 *)NULL)
        error_string = "allocating patches i/o buffer";
      for (il = 0; il < NLINE_PATCH; il++) {
        this->buf.val_int32[il] = val_int32_p;
        val_int32_p += NSAMPLE_PATCH;
      }
      break;
    default:
      error_string = "invalid data type";
  }

  if (error_string != (char *)NULL) {
    fclose(this->file);
    free(this->file_name);
    free(this);
    LOG_RETURN_ERROR(error_string, "SetupPatches", (Patches_t *)NULL);
  }

  this->patch_size = n * this->data_type_size;

  /* Set up a two dimensional buffer for each patch's location and status */

  this->loc = (Patches_loc_t **)calloc((size_t)this->npatch.l, 
                                       sizeof(Patches_loc_t *));
  if (this->loc == (Patches_loc_t **)NULL) {
    free(this->buf.val_void[0]);
    fclose(this->file);
    free(this->file_name);
    free(this);
    LOG_RETURN_ERROR("allocating Patches location and statuts array", 
                 "SetupPatches", (Patches_t *)NULL);
  }

  n = (size_t)(this->npatch.s * this->npatch.l);
  loc_p = (Patches_loc_t *)calloc(n, sizeof(Patches_loc_t));
  if (loc_p == (Patches_loc_t *)NULL) {
    free(this->loc);
    free(this->buf.val_void[0]);
    fclose(this->file);
    free(this->file_name);
    free(this);
    LOG_RETURN_ERROR("allocating Patches location and status structure", 
                 "SetupPatches", (Patches_t *)NULL);
  }
  for (il = 0; il < this->npatch.l; il++) {
    this->loc[il] = loc_p;
    loc_p += this->npatch.s;
    for (is = 0; is < this->npatch.s; is++) {
      this->loc[il][is].status = PATCH_NULL;
      this->loc[il][is].u.pntr = (Patches_mem_t *)NULL;
    }
  }

  /* Set up an array in memory for the patches */

  this->mem = (Patches_mem_t **)calloc((size_t)this->nmem_max, 
                                       sizeof(Patches_mem_t *));
  if (this->mem == (Patches_mem_t **)NULL) {
    free(this->loc[0]);
    free(this->loc);
    free(this->buf.val_void[0]);
    fclose(this->file);
    free(this->file_name);
    free(this);
    LOG_RETURN_ERROR("allocating patches memory array (a)", "SetupPatches", 
                 (Patches_t *)NULL);
  }

  for (ip = 0; ip < NPATCH_MEM_INIT; ip++) {
    if (!CreatePatches(this)) {
      if (!FreePatchesInMem(this))
        LOG_RETURN_ERROR("freeing patches in memory", "SetupPatches", 
	             (Patches_t *)NULL);
      if (!FreePatches(this))
        LOG_RETURN_ERROR("freeing patches", "SetupPatches", (Patches_t *)NULL);
      LOG_RETURN_ERROR("creating patches", "SetupPatches", (Patches_t *)NULL);
    }
  }

  return this;
}


bool FreePatchesInMem(Patches_t *this)
/* 
!C******************************************************************************

!Description: 'FreePatchesInMem' frees allocated space for patches in memory.
 
!Input Parameters:
 this           'patches' data structure; the following fields are input:
                  mem[*], (mem_p)->sum[0], (mem_p)->weight[0], (mem_p)->nn_wt

!Output Parameters:
 this           'patches' data structure; the following fields are modified:
                  mem[*], (mem_p)->sum[0], (mem_p)->weight[0], (mem_p)->nn_wt
 (returns)      status:
                  'true' = okay (always returned)

!Team Unique Header:

 ! Design Notes:
   1. 'SetupPatches' must be called before this routine is called.
   2. An error status is never returned.

!END****************************************************************************
*/
{
  long ip;

  if (this != (Patches_t *)NULL) {
    if (this->mem != (Patches_mem_t **)NULL) {
      if (this->mem[0] != (Patches_mem_t *)NULL) {
        for (ip = 0; ip < this->nmem; ip++) 
	{
	    if (this->mem[ip]->sum[0] != (double*)NULL) 
	    {
		free(this->mem[ip]->sum[0]);
		this->mem[ip]->sum[0] = (double*)NULL;
	    }

	    if (this->mem[ip]->weight[0] != (double*)NULL) 
	    {
		free(this->mem[ip]->weight[0]);
		this->mem[ip]->weight[0] = (double*)NULL;
	    }

	    if (this->mem[ip]->nn_wt[0] != (double*)NULL) 
	    {
		free(this->mem[ip]->nn_wt[0]);
		this->mem[ip]->nn_wt[0] = (double*)NULL;
	    }
        }

        for (ip = 0; ip < this->nmem; ip += this->nmem_alloc) 
	{
	    free(this->mem[ip]);
	    this->mem[ip] = (Patches_mem_t *)NULL;
	}
      }
      free(this->mem);
      this->mem = (Patches_mem_t **)NULL;
    }
  }

  return true;
}

		       
bool FreePatches(Patches_t *this)
/* 
!C******************************************************************************

!Description: 'FreePatches' frees the 'patches' data structure memory.
 
!Input Parameters:
 this           'patches' data structure; the following fields are input:
                  loc, loc[0], buf.val_void[0], file_name, file

!Output Parameters:
 (returns)      status:
                  'true' = okay (always returned)

!Team Unique Header:

 ! Design Notes:
   1. 'SetupPatches' and 'FreePatchesInMem' must be called before this 
      routine is called.
   2. An error status is never returned.

!END****************************************************************************
*/
{
  if (this != (Patches_t *)NULL) {
    if (this->loc != (Patches_loc_t **)NULL) {
      if (this->loc[0] != (Patches_loc_t *)NULL) {
        free(this->loc[0]);
	this->loc[0] = (Patches_loc_t *)NULL;
      }
      free(this->loc);
      this->loc = (Patches_loc_t **)NULL;
    }
    if (this->buf.val_void[0] != NULL) {
      free(this->buf.val_void[0]);
      this->buf.val_void[0] = NULL;
    }
    if (this->file_name != (char *)NULL) {
      free(this->file_name);
      this->file_name = (char *)NULL;
    }
    if (this->file != (FILE *)NULL) fclose(this->file);
    free(this);
    this = (Patches_t *)NULL;
  }
  return true;
}


bool InitPatchInMem(Patches_t *this, int il_patch, int is_patch)
/* 
!C******************************************************************************

!Description: 'InitPatchInMem' initializes a patch in memory.
 
!Input Parameters:
 this           'patches' data structure; the following fields are input:
                  nmem, nmem_alloc, nmem_max, nnull, null_list,
                  nused, used_list, loc[il_patch][is_patch], 
		  (loc_p)->status, (mem_p)->prev, (mem_p)->next
 il_patch       line number of the patch to initialize
 is_patch       patch number of the patch to initialize

!Output Parameters:
 this           'patches' data structure; the following fields are modified:
                  nmem, nnull, null_list, nused, used_list, mem[*], 
		  (mem_p)->ntouch, (mem_p)->prev, (mem_p)->next, 
		  (mem_p)->loc, (mem_p)->sum, (mem_p)->weight, (mem_p)->nn_wt
		  loc[il_patch][is_patch], (loc_p)->status, (loc_p)->u.pntr
 (returns)      status:
                  'true' = okay
		  'false' = error return

!Team Unique Header:

 ! Design Notes:
   1. An error status is returned when:
       a. the patch is already in memory or on disk
       b. the patch status is invalid
       c. a new set of patches can not be created
       d. no null patches are left
       e. there are an invalid number (< 0) of null or used patches.
   2. 'SetupPatches' must be called before this routine is called.
   3. Error messages are handled with the 'LOG_RETURN_ERROR' macro.

!END****************************************************************************
*/
{
  Patches_loc_t *loc_p;
  Patches_mem_t *mem_p;
  Patches_mem_t *next_mem_p;
  int il, is;

  /* Check the patch */

  loc_p = &this->loc[il_patch][is_patch];

  if (loc_p->status != PATCH_NULL) {
    if (loc_p->status == PATCH_IN_MEM)
      LOG_RETURN_ERROR("patch already in memory", "InitPatchInMem", false);

    if (loc_p->status == PATCH_ON_DISK) {

/* #define DEBUG1 */
#ifdef DEBUG1
      int il1_deb, is1_deb;
      int il2_deb, is2_deb;
      int n = 4;
      char *stat[3] = {"null", "mem ", "disk"};

      il1_deb = il_patch - n;
      if (il1_deb < 0) il1_deb = 0;
      il2_deb = il_patch + n + 1;
      if (il2_deb > this->npatch.l) il2_deb = this->npatch.l;

      is1_deb = is_patch - n;
      if (is1_deb < 0) is1_deb = 0;
      is2_deb = is_patch + n + 1;
      if (is2_deb > this->npatch.s) is2_deb = this->npatch.s;

      printf("      ");
      for (is = is1_deb; is < is2_deb; is++) printf("  %4d", is);
      printf("\n");

      for (il = il1_deb; il < il2_deb; il++) {
        printf("  %4d", il);
        for (is = is1_deb; is < is2_deb; is++) {
	  if (il == il_patch  &&  is == is_patch) printf(" *");
	  else printf("  ");
	  loc_p = &this->loc[il][is];
	  printf("%s", stat[loc_p->status]);
        }
	printf("\n");
      }
#endif

      LOG_RETURN_ERROR("patch already on disk", "InitPatchInMem", false);
    }
    LOG_RETURN_ERROR("invalid patch status", "InitPatchInMem", false);
  }

  /* Get a patch from the head of the null list */

  mem_p = this->null_list;

  if (mem_p == (Patches_mem_t *)NULL) {
    if (!CreatePatches(this))
      LOG_RETURN_ERROR("can't create new patches","InitPatchInMem", false);
    mem_p = this->null_list;
    if (mem_p == (Patches_mem_t *)NULL)
      LOG_RETURN_ERROR("no null patches","InitPatchInMem", false);
  }

  this->nnull--;
  if (this->nnull < 0)
    LOG_RETURN_ERROR("invalid number of null patches", "InitPatchInMem", false);

  next_mem_p = mem_p->next;
  if (next_mem_p != (Patches_mem_t *)NULL)
    next_mem_p->prev = (Patches_mem_t *)NULL;
  this->null_list = next_mem_p;

  loc_p->status = PATCH_IN_MEM;
  loc_p->u.pntr = mem_p;

  /* Add patch to head of used list */

  this->nused++;
  if (this->nused > this->nmem)
    LOG_RETURN_ERROR("invalid number of used patches", "InitPatchInMem", false);

  next_mem_p = this->used_list;
  if (next_mem_p != (Patches_mem_t *)NULL)
    next_mem_p->prev = mem_p;
  this->used_list = mem_p;
  mem_p->prev = (Patches_mem_t *)NULL;
  mem_p->next = next_mem_p;

  /* Initialize patch */

  mem_p->ntouch = -1;
  mem_p->loc.l = il_patch;
  mem_p->loc.s = is_patch;
  for (il = 0; il < NLINE_PATCH; il++) {  
    for (is = 0; is < NSAMPLE_PATCH; is++) {  
      mem_p->sum[il][is]    = 0.0;
      mem_p->weight[il][is] = 0.0;
      mem_p->nn_wt[il][is]  = 0.0;
    }
  }

  return true;
}


bool UntouchPatches(Patches_t *this)
/* 
!C******************************************************************************

!Description: 'UntouchPatches' marks all patches as eligible to be written 
 to the temporary disk file.
 
!Input Parameters:
 this           'patches' data structure; the following fields are input:
                  used_list, (mem_p)->next

!Output Parameters:
 this           'patches' data structure; the following fields are modified:
                  (mem_p)->ntouch
 (returns)      status:
                  'true' = okay (always returned)

!Team Unique Header:

 ! Design Notes:
   1. 'SetupPatches' must be called before this routine is called.
   2. An error status is never returned.

!END****************************************************************************
*/
{
  Patches_mem_t *mem_p;

  /* Clear the touched flag in all the patches in memory */

  mem_p = this->used_list;
  while (mem_p != (Patches_mem_t *)NULL) {
    mem_p->ntouch = -1;
    mem_p = mem_p->next;
  }

  return true;
}


char8 ConvertToChar8(double v, double slope, bool same_data_type)
/* 
!C******************************************************************************

!Description: 'ConvertToChar8' converts a float to an HDF CHAR8 data type.
 
!Input Parameters:
 v              floating point number
 slope          slope of the conversion from one data type to another
                (input_max - input_min) / (output_max - output_min)
 same_data_type flag to specify if the input data type and output data type
                are the same

!Output Parameters:
 (returns)      input value converted to HDF CHAR8 data type

!Team Unique Header:

 ! Design Notes:
   1. The value is rounded to the nearest integer.
   2. If the value is outside of the range 'RANGE_CHAR8L' to 'RANGE_CHAR8H' a 
      value within the range is returned.

!END****************************************************************************
*/
{
  double out_value, vi;

  /* if the input and output data types are the same then use the data
     value as is */
  if (same_data_type)
    out_value = v;
  else
    /* output value is data type low range value + the slope * floating
       point number */
    out_value = (double)RANGE_CHAR8L + slope * v;

  /* vi = floor((double)(out_value + 0.5)); */
  vi = out_value < 0.0 ? out_value - 0.5 : out_value + 0.5;
  if (vi < (double)RANGE_CHAR8L) return((char8)RANGE_CHAR8L);
  else if (vi > (double)RANGE_CHAR8H) return((char8)RANGE_CHAR8H);
  else return((char8)vi);
}


uint8 ConvertToUint8(double v, double slope, bool same_data_type)
/* 
!C******************************************************************************

!Description: 'ConvertToUint8' converts a float to an HDF UINT8 data type.
 
!Input Parameters:
 v              floating point number
 slope          slope of the conversion from one data type to another
                (input_max - input_min) / (output_max - output_min)
 same_data_type flag to specify if the input data type and output data type
                are the same

!Output Parameters:
 (returns)      input value converted to HDF UINT8 data type

!Team Unique Header:

 ! Design Notes:
   1. The value is rounded to the nearest integer.
   2. If the value is outside of the range 'RANGE_UINT8L' to 'RANGE_UINT8H' a 
      value within the range is returned.

!END****************************************************************************
*/
{
  double out_value, vi;

  /* if the input and output data types are the same then use the data
     value as is */
  if (same_data_type)
    out_value = v;
  else
    /* output value is data type low range value + the slope * floating
       point number */
    out_value = (double)RANGE_UINT8L + slope * v;

  /* vi = floor((double)(out_value + 0.5)); */
  vi = out_value < 0.0 ? out_value - 0.5 : out_value + 0.5;
  if (vi < (double)RANGE_UINT8L) return((uint8)RANGE_UINT8L);
  else if (vi > (double)RANGE_UINT8H) return((uint8)RANGE_UINT8H);
  else return((uint8)vi);
}


int8 ConvertToInt8(double v, double slope, bool same_data_type)
/* 
!C******************************************************************************

!Description: 'ConvertToInt8' converts a float to an HDF INT8 data type.
 
!Input Parameters:
 v              floating point number
 slope          slope of the conversion from one data type to another
                (input_max - input_min) / (output_max - output_min)
 same_data_type flag to specify if the input data type and output data type
                are the same

!Output Parameters:
 (returns)      input value converted to HDF INT8 data type

!Team Unique Header:

 ! Design Notes:
   1. The value is rounded to the nearest integer.
   2. If the value is outside of the range 'RANGE_INT8L' to 'RANGE_INT8H' a 
      value within the range is returned.

!END****************************************************************************
*/
{
  double out_value, vi;

  /* if the input and output data types are the same then use the data
     value as is */
  if (same_data_type)
    out_value = v;
  else
    /* output value is data type low range value + the slope * floating
       point number */
    out_value = (double)RANGE_INT8L + slope * v;

  /* vi = floor((double)(out_value + 0.5)); */
  vi = out_value < 0.0 ? out_value - 0.5 : out_value + 0.5;
  if (vi < (double)RANGE_INT8L) return((int8)RANGE_INT8L);
  else if (vi > (double)RANGE_INT8H) return((int8)RANGE_INT8H);
  else return((int8)vi);
}


int16 ConvertToInt16(double v, double slope, bool same_data_type)
/* 
!C******************************************************************************

!Description: 'ConvertToInt16' converts a float to an HDF UINT16 data type.
 
!Input Parameters:
 v              floating point number
 slope          slope of the conversion from one data type to another
                (input_max - input_min) / (output_max - output_min)
 same_data_type flag to specify if the input data type and output data type
                are the same

!Output Parameters:
 (returns)      input value converted to HDF UINT16 data type

!Team Unique Header:

 ! Design Notes:
   1. The value is rounded to the nearest integer.
   2. If the value is outside of the range 'RANGE_UINT16L' to 'RANGE_UINT16H' a 
      value within the range is returned.

!END****************************************************************************
*/
{
  double out_value, vi;

  /* if the input and output data types are the same then use the data
     value as is */
  if (same_data_type)
    out_value = v;
  else
    /* output value is data type low range value + the slope * floating
       point number */
    out_value = (double)RANGE_INT16L + slope * v;

  /* vi = floor((double)(out_value + 0.5)); */
  vi = out_value < 0.0 ? out_value - 0.5 : out_value + 0.5;
  if (vi < (double)RANGE_INT16L) return((int16)RANGE_INT16L);
  else if (vi > (double)RANGE_INT16H) return((int16)RANGE_INT16H);
  else return((int16)vi);
}


uint16 ConvertToUint16(double v, double slope, bool same_data_type)
/* 
!C******************************************************************************

!Description: 'ConvertToUint16' converts a float to an HDF INT16 data type.
 
!Input Parameters:
 v              floating point number
 slope          slope of the conversion from one data type to another
                (input_max - input_min) / (output_max - output_min)
 same_data_type flag to specify if the input data type and output data type
                are the same

!Output Parameters:
 (returns)      input value converted to HDF UINT16 data type

!Team Unique Header:

 ! Design Notes:
   1. If the value is rounded to the nearest integer.
   2. If the value is outside of the range 'RANGE_UINT16L' to 'RANGE_UINT16H' a 
      value within the range is returned.

!END****************************************************************************
*/
{
  double out_value, vi;

  /* if the input and output data types are the same then use the data
     value as is */
  if (same_data_type)
    out_value = v;
  else
    /* output value is data type low range value + the slope * floating
       point number */
    out_value = (double)RANGE_UINT16L + slope * v;

  /* vi = floor((double)(out_value + 0.5)); */
  vi = out_value < 0.0 ? out_value - 0.5 : out_value + 0.5;
  if (vi < (double)RANGE_UINT16L) return((uint16)RANGE_UINT16L);
  else if (vi > (double)RANGE_UINT16H) return((uint16)RANGE_UINT16H);
  else return((uint16)vi);
}

uint32 ConvertToUint32(double v, double slope, bool same_data_type)
/* 
!C******************************************************************************

!Description: 'ConvertToUint32' converts a float to an HDF UINT32 data type.
 
!Input Parameters:
 v              floating point number
 slope          slope of the conversion from one data type to another
                (input_max - input_min) / (output_max - output_min)
 same_data_type flag to specify if the input data type and output data type
                are the same

!Output Parameters:
 (returns)      input value converted to HDF UINT32 data type

!Team Unique Header:

 ! Design Notes:
   1. If the value is rounded to the nearest integer.
   2. If the value is outside of the range 'RANGE_UINT32L' to 'RANGE_UINT32H' a
      value within the range is returned.

!END****************************************************************************
*/
{
  double out_value, vi;

  /* if the input and output data types are the same then use the data
     value as is */
  if (same_data_type)
    out_value = v;
  else
    /* output value is data type low range value + the slope * floating
       point number */
    out_value = (double)RANGE_UINT32L + slope * v;

  /* vi = floor((double)(out_value + 0.5)); */
  vi = out_value < 0.0 ? out_value - 0.5 : out_value + 0.5;
  if (vi < (double)RANGE_UINT32L) return((uint32)RANGE_UINT32L);
  else if (vi > (double)RANGE_UINT32H) return((uint32)RANGE_UINT32H);
  else return((uint32)vi);
}

int32 ConvertToInt32(double v, double slope, bool same_data_type)
/*
!C******************************************************************************

!Description: 'ConvertToInt32' converts a float to an HDF INT32 data type.

!Input Parameters:
 v              floating point number
 slope          slope of the conversion from one data type to another
                (input_max - input_min) / (output_max - output_min)
 same_data_type flag to specify if the input data type and output data type
                are the same

!Output Parameters:
 (returns)      input value converted to HDF INT32 data type

!Team Unique Header:

 ! Design Notes:
   1. If the value is rounded to the nearest integer.
   2. If the value is outside of the range 'RANGE_INT32L' to 'RANGE_INT32H' a
      value within the range is returned.

!END****************************************************************************
*/
{
  double out_value, vi;

  /* if the input and output data types are the same then use the data
     value as is */
  if (same_data_type)
    out_value = v;
  else
    /* output value is data type low range value + the slope * floating
       point number */
    out_value = (double)RANGE_INT32L + slope * v;

  /* vi = floor((double)(out_value + 0.5)); */
  vi = out_value < 0.0 ? out_value - 0.5 : out_value + 0.5;
  if (vi < (double)RANGE_INT32L) return((int32)RANGE_INT32L);
  else if (vi > (double)RANGE_INT32H) return((int32)RANGE_INT32H);
  else return((int32)vi);
}


bool TossPatches(Patches_t *this, int32 output_data_type)
/* 
!C******************************************************************************

!Description: 'TossPatches' writes all eligible (complete) patches to the 
 temporary disk file.
 
!Input Parameters:
 this           'patches' data structure; the following fields are input:
                  used_list, (mem_p)->ntouch, (mem_p)->next, 
		  (mem_p)->loc, (loc_p)->status, data_type, fill_value, 
		  (mem_p)->weight[*][*], (mem_p)->sum[*][*], 
		  (mem_p)->prev, nused, used_list, 
		  nnull, null_list, patch_size, file, file_size
 output_data_type data type of output image

!Output Parameters:
 this           'patches' data structure; the following fields are modified:
                  (mem_p)->ntouch, buf, (loc_p)->u.loc, 
		  (loc_p)->status, (mem_p)->prev, (mem_p)->next, nused, 
		  used_list, nnull, null_list, (mem_p)->loc, patch_size, 
		  file, file_size
 (returns)      status:
                  'true' = okay
		  'false' = error return

!Team Unique Header:

 ! Design Notes:
   1. The 'ntouch' value is decremented by one if the patch is not yet 
      eligible to be written to disk.
   2. An error status is returned when:
       a. a patch is both still in memory and on disk
       b. the patch status is null or invalid
       c. the output data type is invalid
       d. an I/O error occurs when writing the patch
       e. there are an invalid number (< 0) of null or used patches.
   3. 'SetupPatches' must be called before this routine is called.
   4. Error messages are handled with the 'LOG_RETURN_ERROR' macro.

!END****************************************************************************
*/
{
  Patches_mem_t *mem_p;
  Patches_mem_t *next_null_mem_p, *next_used_mem_p, *prev_mem_p;
  Patches_loc_t *loc_p;
  int il_patch, is_patch;
  char8 fill_char8 = 0;
  uint8 fill_uint8 = 0;
  int8 fill_int8 = 0;
  int16 fill_int16 = 0;
  uint16 fill_uint16 = 0;
  int32 fill_int32 = 0;
  uint32 fill_uint32 = 0;
  int il, is;
  double w;
  bool same_data_type;
  double slope;
  int32 output_diff = 0, input_diff = 0;

  /* For each non-null patch in memory */

  mem_p = this->used_list;
  while (mem_p != (Patches_mem_t *)NULL) {

    /* If recently touched, go on to the next patch */

    if (mem_p->ntouch >= 0) {
      mem_p->ntouch--;
      mem_p = mem_p->next;
      continue;
    }

    /* Check the patch status */

    il_patch = mem_p->loc.l;
    is_patch = mem_p->loc.s;
    loc_p = &this->loc[il_patch][is_patch];

    if (loc_p->status != PATCH_IN_MEM) {
      if (loc_p->status == PATCH_ON_DISK)
        LOG_RETURN_ERROR("patch already on disk", "TossPatches", false);
      if (loc_p->status == PATCH_NULL)
        LOG_RETURN_ERROR("patch is null", "TossPatches", false);
      LOG_RETURN_ERROR("invalid patch status", "TossPatches", false);
    }

    /* Normalize each pixel in the patch and convert to output type */

/* #define DEBUG2 */
#ifdef DEBUG2
    {
      double v;

      printf( "loc_p = loc[%d][%d]\n\n", il_patch, is_patch );
      printf(" w  ");
      for (is = 0; is < NSAMPLE_PATCH; is++) printf("%5d", is);
      printf("\n");
      for (il = 0; il < NLINE_PATCH; il++) {
        printf("%2ld  ", (long)il);
        for (is = 0; is < NSAMPLE_PATCH; is++)
          printf(" %4.2f", (double)mem_p->weight[il][is]);
        printf("\n");
      }
      printf("\n");

      printf(" v  ");
      for (is = 0; is < NSAMPLE_PATCH; is++) printf("%5d", is);
      printf("\n");
      for (il = 0; il < NLINE_PATCH; il++) {
        printf("%2ld  ", (long)il);
        for (is = 0; is < NSAMPLE_PATCH; is++) {
          w = mem_p->weight[il][is];
          v = (w > 0.0) ? (mem_p->sum[il][is] / w) : 0.0;
  	  v *= 0.001;
          printf(" %4.2f", (double)v);
        }
        printf("\n");
      }
      printf("\n");
    }
#endif

    /* are we dealing with the same data type for input and output? */
    same_data_type = (bool) (this->data_type == output_data_type);

    /* determine the slope between the input data type and output data type */
    if (same_data_type)
    {
      /* slope is 1.0 */
      slope = 1.0;
    }
    else
    {
      /* determine the difference between the input image's high and low
         range values */
      switch (this->data_type)
      {
        case DFNT_CHAR8:
          input_diff = RANGE_CHAR8H - RANGE_CHAR8L;
          break;
        case DFNT_UINT8:
          input_diff = RANGE_UINT8H - RANGE_UINT8L;
          break;
        case DFNT_INT8:
          input_diff = RANGE_INT8H - RANGE_INT8L;
          break;
        case DFNT_UINT16:
          input_diff = RANGE_UINT16H - RANGE_UINT16L;
          break;
        case DFNT_INT16:
          input_diff = RANGE_INT16H - RANGE_INT16L;
          break;
        case DFNT_UINT32:
          input_diff = RANGE_UINT32H - RANGE_UINT32L;
          break;
        case DFNT_INT32:
          input_diff = RANGE_INT32H - RANGE_INT32L;
          break;
      }

      /* determine the difference between the output image's high and low
         range values */
      switch (output_data_type)
      {
        case DFNT_CHAR8:
          output_diff = RANGE_CHAR8H - RANGE_CHAR8L;
          break;
        case DFNT_UINT8:
          output_diff = RANGE_UINT8H - RANGE_UINT8L;
          break;
        case DFNT_INT8:
          output_diff = RANGE_INT8H - RANGE_INT8L;
          break;
        case DFNT_UINT16:
          output_diff = RANGE_UINT16H - RANGE_UINT16L;
          break;
        case DFNT_INT16:
          output_diff = RANGE_INT16H - RANGE_INT16L;
          break;
        case DFNT_UINT32:
          output_diff = RANGE_UINT32H - RANGE_UINT32L;
          break;
        case DFNT_INT32:
          output_diff = RANGE_INT32H - RANGE_INT32L;
          break;
      }

      /* determine the slope */
      if (input_diff != 0)
        slope = (double) output_diff / (double) input_diff;
      else
        slope = 1.0;
    }

    switch (this->data_type) {
      case DFNT_CHAR8:
        fill_char8 = ConvertToChar8(this->fill_value, slope, same_data_type);
        for (il = 0; il < NLINE_PATCH; il++) {
          for (is = 0; is < NSAMPLE_PATCH; is++) {
            w = mem_p->weight[il][is];
            this->buf.val_char8[il][is] = (w > MIN_WEIGHT) ? 
	      ConvertToChar8(mem_p->sum[il][is] / w, slope, same_data_type) :
                fill_char8;
	  }
        }
        break;
      case DFNT_UINT8:
        fill_uint8 = ConvertToUint8(this->fill_value, slope, same_data_type);
        for (il = 0; il < NLINE_PATCH; il++) {
          for (is = 0; is < NSAMPLE_PATCH; is++) {
            w = mem_p->weight[il][is];
            this->buf.val_uint8[il][is] = (w > MIN_WEIGHT) ? 
	      ConvertToUint8(mem_p->sum[il][is] / w, slope, same_data_type) :
                fill_uint8;
	  }
        }
        break;
      case DFNT_INT8:
        fill_int8 = ConvertToInt8(this->fill_value, slope, same_data_type);
        for (il = 0; il < NLINE_PATCH; il++) {
          for (is = 0; is < NSAMPLE_PATCH; is++) {
            w = mem_p->weight[il][is];
            this->buf.val_int8[il][is] = (w > MIN_WEIGHT) ? 
	      ConvertToInt8(mem_p->sum[il][is] / w, slope, same_data_type) :
                fill_int8;
	  }
        }
        break;
      case DFNT_INT16:
        fill_int16 = ConvertToInt16(this->fill_value, slope, same_data_type);
        for (il = 0; il < NLINE_PATCH; il++) {
          for (is = 0; is < NSAMPLE_PATCH; is++) {
            w = mem_p->weight[il][is];
            this->buf.val_int16[il][is] = (w > MIN_WEIGHT) ? 
	      ConvertToInt16(mem_p->sum[il][is] / w, slope, same_data_type) :
                fill_int16;
	  }
        }
        break;
      case DFNT_UINT16:
        fill_uint16 = ConvertToUint16(this->fill_value, slope, same_data_type);
        for (il = 0; il < NLINE_PATCH; il++) {
          for (is = 0; is < NSAMPLE_PATCH; is++) {
            w = mem_p->weight[il][is];
            this->buf.val_uint16[il][is] = (w > MIN_WEIGHT) ? 
              ConvertToUint16(mem_p->sum[il][is] / w, slope, same_data_type) :
                fill_uint16;
	  }
        }
        break;
      case DFNT_INT32:
        fill_int32 = ConvertToInt32(this->fill_value, slope, same_data_type);
        for (il = 0; il < NLINE_PATCH; il++) {
          for (is = 0; is < NSAMPLE_PATCH; is++) {
            w = mem_p->weight[il][is];
            this->buf.val_int32[il][is] = (w > MIN_WEIGHT) ?
              ConvertToInt32(mem_p->sum[il][is] / w, slope, same_data_type) :
                fill_int32;
          }
        }
        break;
      case DFNT_UINT32:
        fill_uint32 = ConvertToUint32(this->fill_value, slope, same_data_type);
        for (il = 0; il < NLINE_PATCH; il++) {
          for (is = 0; is < NSAMPLE_PATCH; is++) {
            w = mem_p->weight[il][is];
            this->buf.val_uint32[il][is] = (w > MIN_WEIGHT) ?
              ConvertToUint32(mem_p->sum[il][is] / w, slope, same_data_type) :
                fill_uint32;
          }
        }
        break;
      default:
        LOG_RETURN_ERROR("invalid data type", "TossPatches", false);
    }

    /* Write patch to disk and update location and status */

    loc_p->u.loc = this->file_size;
    loc_p->status = PATCH_ON_DISK;

    if (fwrite(this->buf.val_void[0], this->patch_size, 1, this->file) != 1)
      LOG_RETURN_ERROR("writing patch to disk", "TossPatches", false);
    this->file_size += this->patch_size;

    /* Remove patch from used list */

    next_used_mem_p = mem_p->next;
    prev_mem_p = mem_p->prev;
    if (next_used_mem_p != (Patches_mem_t *)NULL)
      next_used_mem_p->prev = prev_mem_p;
    if (prev_mem_p == (Patches_mem_t *)NULL)
      this->used_list = next_used_mem_p;
    else
      prev_mem_p->next = next_used_mem_p;

    this->nused--;
    if (this->nused < 0)
      LOG_RETURN_ERROR("invalid number of used patches", "TossPatches", false);

    /* Add patch to head of null list */

    next_null_mem_p = this->null_list;
    if (next_null_mem_p != (Patches_mem_t *)NULL)
      next_null_mem_p->prev = mem_p;
    this->null_list = mem_p;

    mem_p->ntouch = -1;
    mem_p->loc.l = -1;
    mem_p->loc.s = -1;
    mem_p->next = next_null_mem_p;
    mem_p->prev = (Patches_mem_t *)NULL;

    this->nnull++;
    if (this->nnull > this->nmem)
      LOG_RETURN_ERROR("invalid number of null patches", "TossPatches", false);

    mem_p = next_used_mem_p;
  }

  return true;
}


bool UnscramblePatches(Patches_t *this, Output_t *output,
                       Output_file_format_t output_format,
                       FILE_ID *GeoTiffFile, FILE *rbfile,
                       int32 output_data_type,
                       Kernel_type_t kernel_type)
/* 
!C******************************************************************************

!Description: 'UnscramblePatches' reads patches from temporary disk file and 
 write them to the output product file.
 
!Input Parameters:
 this           'patches' data structure; the following fields are input:
                  data_type, npatch, loc, (loc_p)->status, (loc_p)->u.loc, 
		  file, patch_size
 output         output data structure; the following fields are input:
                  size, buf, open, sds.id
 output_format  output data format (HDF_FMT, GEOTIFF_FMT, RB_FMT, BOTH)
 GeoTiffFile    output Geotiff file information
 rbfile         raw binary file pointer
 output_data_type output data type, patches are stored in the input data
                    type
 kernel_type    NN, Bilinear, CC kernel to be used for resampling process

!Output Parameters:
 this           'patches' data structure; the following fields are modified:
                  file
 output         'output' data structure; no fields are modified:
 (returns)      status:
                  'true' = okay
		  'false' = error return

!Team Unique Header:

 ! Design Notes:
   1. All data must be written from memory to disk before this routine is 
      called.  See the 'UntouchPatches' and 'TossPatches' routines.
   2. The 'this->data_type' value should be the data type for the 'patches'.
      The output_data_type should be the data type for the 'output' data
      structures.
   3. A buffer is created with 'NLINE_PATCH' lines, each line having the number 
      of samples per line in the output image.  Then for each set of output
      lines all of the corresponding input patches are read from the temporary 
      file and copied into the output line buffer.  Then the lines in the output
      buffer are written to an HDF file.
   4. An error status is returned when:
       a. a patch is still in memory
       b. the patch status is invalid
       c. the output data type is invalid
       d. an I/O error occurs when reading the patch
       * e. there are an invalid number (< 0) of null or used patches.
   5. 'SetupPatches' must be called before this routine is called.
   6. Error messages are handled with the 'LOG_RETURN_ERROR' macro.

!END****************************************************************************
*/
{
  union {
    void *val_void[NLINE_PATCH];
    char8 *val_char8[NLINE_PATCH];
    uint8 *val_uint8[NLINE_PATCH];
    int8 *val_int8[NLINE_PATCH];
    int16 *val_int16[NLINE_PATCH];
    uint16 *val_uint16[NLINE_PATCH];    
    int32 *val_int32[NLINE_PATCH];
    uint32 *val_uint32[NLINE_PATCH];
  } buf;
  size_t n;
  char8 *val_char8_p;
  uint8 *val_uint8_p;
  int8 *val_int8_p;
  int16 *val_int16_p;
  uint16 *val_uint16_p;
  int32 *val_int32_p;
  uint32 *val_uint32_p;
  char8 fill_char8 = 0;
  uint8 fill_uint8 = 0;
  int8 fill_int8 = 0;
  int16 fill_int16 = 0;
  uint16 fill_uint16 = 0;
  int32 fill_int32 = 0;
  uint32 fill_uint32 = 0;
  int il, is;
  int il_patch, is_patch;
  int il1, il2;
  int is1, is2;
  Patches_loc_t *loc_p;
  int il_rel, is_rel;
  bool same_data_type;
  double slope;
  int32 output_diff = 0, input_diff = 0;
#ifdef DEBUG_ZEROS
  int *zero_p;
  int *zeros[3];
  int *zero_chk;
  int nzero;
  int ib;
  int il_print;
  long nzero_tot;
#endif

  /* Are we dealing with the same data type for input and output? */

  same_data_type = (bool) (this->data_type == output_data_type);

  /* Determine the slope between the input data type and output data type */

  if (same_data_type)
  {
    /* Slope is 1.0 */
    slope = 1.0;
  }
  else
  {
    /* Determine the difference between the input image's high and low
       range values */
    switch (this->data_type)
    {
      case DFNT_CHAR8:
        input_diff = RANGE_CHAR8H - RANGE_CHAR8L;
        break;
      case DFNT_UINT8:
        input_diff = RANGE_UINT8H - RANGE_UINT8L;
        break;
      case DFNT_INT8:
        input_diff = RANGE_INT8H - RANGE_INT8L;
        break;
      case DFNT_UINT16:
        input_diff = RANGE_UINT16H - RANGE_UINT16L;
        break;
      case DFNT_INT16:
        input_diff = RANGE_INT16H - RANGE_INT16L;
        break;
      case DFNT_UINT32:
        input_diff = RANGE_UINT32H - RANGE_UINT32L;
        break;
      case DFNT_INT32:
        input_diff = RANGE_INT32H - RANGE_INT32L;
        break;
    }

    /* Determine the difference between the output image's high and low
       range values */
    switch (output_data_type)
    {
      case DFNT_CHAR8:
        output_diff = RANGE_CHAR8H - RANGE_CHAR8L;
        break;
      case DFNT_UINT8:
        output_diff = RANGE_UINT8H - RANGE_UINT8L;
        break;
      case DFNT_INT8:
        output_diff = RANGE_INT8H - RANGE_INT8L;
        break;
      case DFNT_UINT16:
        output_diff = RANGE_UINT16H - RANGE_UINT16L;
        break;
      case DFNT_INT16:
        output_diff = RANGE_INT16H - RANGE_INT16L;
        break;
      case DFNT_UINT32:
        output_diff = RANGE_UINT32H - RANGE_UINT32L;
        break;
      case DFNT_INT32:
        output_diff = RANGE_INT32H - RANGE_INT32L;
        break;
    }

    /* Determine the slope */
    if (input_diff != 0)
      slope = (double) output_diff / (double) input_diff;
    else
      slope = 1.0;
  }

  /* Allocate space for lines of output product */

  n = (size_t)(NLINE_PATCH * output->size.s);
  switch (output_data_type) {
    case DFNT_CHAR8:
      fill_char8 = ConvertToChar8(this->fill_value, slope, same_data_type);
      val_char8_p = (char8 *)calloc(n, sizeof(char8));
      if (val_char8_p == (char8 *)NULL)
        LOG_RETURN_ERROR("allocating output product i/o buffer", 
                     "UnscramblePatches", false);
      for (il = 0; il < NLINE_PATCH; il++) {
        buf.val_char8[il] = val_char8_p;
	val_char8_p += output->size.s;
      }
      break;
    case DFNT_UINT8:
      fill_uint8 = ConvertToUint8(this->fill_value, slope, same_data_type);
      val_uint8_p = (uint8 *)calloc(n, sizeof(uint8));
      if (val_uint8_p == (uint8 *)NULL)
        LOG_RETURN_ERROR("allocating output product i/o buffer", 
                     "UnscramblePatches", false);
      for (il = 0; il < NLINE_PATCH; il++) {
        buf.val_uint8[il] = val_uint8_p;
	val_uint8_p += output->size.s;
      }
      break;
    case DFNT_INT8:
      fill_int8 = ConvertToInt8(this->fill_value, slope, same_data_type);
      val_int8_p = (int8 *)calloc(n, sizeof(int8));
      if (val_int8_p == (int8 *)NULL)
        LOG_RETURN_ERROR("allocating output product i/o buffer", 
                     "UnscramblePatches", false);
      for (il = 0; il < NLINE_PATCH; il++) {
        buf.val_int8[il] = val_int8_p;
	val_int8_p += output->size.s;
      }
      break;
    case DFNT_INT16:
      fill_int16 = ConvertToInt16(this->fill_value, slope, same_data_type);
      val_int16_p = (int16 *)calloc(n, sizeof(int16));
      if (val_int16_p == (int16 *)NULL)
        LOG_RETURN_ERROR("allocating output product i/o buffer", 
                     "UnscramblePatches", false);
      for (il = 0; il < NLINE_PATCH; il++) {
        buf.val_int16[il] = val_int16_p;
	val_int16_p += output->size.s;
      }
      break;
    case DFNT_UINT16:
      fill_uint16 = ConvertToUint16(this->fill_value, slope, same_data_type);
      val_uint16_p = (uint16 *)calloc(n, sizeof(uint16));
      if (val_uint16_p == (uint16 *)NULL)
        LOG_RETURN_ERROR("allocating output product i/o buffer", 
                     "UnscramblePatches", false);
      for (il = 0; il < NLINE_PATCH; il++) {
        buf.val_uint16[il] = val_uint16_p;
	val_uint16_p += output->size.s;
      }
      break;
    case DFNT_INT32:
      fill_int32 = ConvertToInt32(this->fill_value, slope, same_data_type);
      val_int32_p = (int32 *)calloc(n, sizeof(int32));
      if (val_int32_p == (int32 *)NULL)
        LOG_RETURN_ERROR("allocating output product i/o buffer", 
                     "UnscramblePatches", false);
      for (il = 0; il < NLINE_PATCH; il++) {
        buf.val_int32[il] = val_int32_p;
        val_int32_p += output->size.s;
      }
      break;
    case DFNT_UINT32:
      fill_uint32 = ConvertToUint32(this->fill_value, slope, same_data_type);
      val_uint32_p = (uint32 *)calloc(n, sizeof(uint32));
      if (val_uint32_p == (uint32 *)NULL)
        LOG_RETURN_ERROR("allocating output product i/o buffer",
                     "UnscramblePatches", false);
      for (il = 0; il < NLINE_PATCH; il++) {
        buf.val_uint32[il] = val_uint32_p;
        val_uint32_p += output->size.s;
      }
      break;
    default:
      LOG_RETURN_ERROR("invalid data type (a)", "UnscramblePatches", false);
  }
#ifdef DEBUG_ZEROS
  printf("checking for isolated zeros\n");
  if (this->data_type != DFNT_INT16) 
    ERROR("debuging only set up to handle INT16 data type",  
          "UnscramblePatches");
  zero_p = (int *)calloc((4 * output->size.s), sizeof(int));
  if (zero_p == (int *)NULL) {
    free(zero_p);
    LOG_RETURN_ERROR("allocating zeros buffer", "UnscramblePatches", false);
  }
  zero_chk = zero_p;
  for (is = 0; is < output->size.s; is++) zero_chk[is] = 1;
  for (ib = 0; ib < 3; ib++) {
    zero_p += output->size.s;
    zeros[ib] = zero_p;
    for (is = 0; is < output->size.s; is++) zeros[ib][is] = 3;
    il_print = -1;
  }
  nzero_tot = 0;
#endif

  /* For each output row of patches */

  for (il_patch = 0; il_patch < this->npatch.l; il_patch++) {
    il1 = il_patch * NLINE_PATCH;
    il2 = il1 + NLINE_PATCH;
    if (il2 > output->size.l) il2 = output->size.l;

    /* For each patch in the row */

    for (is_patch = 0; is_patch < this->npatch.s; is_patch++) {
      is1 = is_patch * NSAMPLE_PATCH;
      is2 = is1 + NSAMPLE_PATCH;
      if (is2 > output->size.s) is2 = output->size.s;

      loc_p = &this->loc[il_patch][is_patch];

      if (loc_p->status != PATCH_NULL  &&  
          loc_p->status != PATCH_ON_DISK) {
        free(buf.val_void[0]);
	if (loc_p->status == PATCH_IN_MEM)
          LOG_RETURN_ERROR("patch still in memory", "UnscramblePatches", false);
        LOG_RETURN_ERROR("invalid patch status", "UnscramblePatches", false);
      }

      /* If not fill, get the patch and store in the lines, otherwise 
         fill the lines */

      if (loc_p->status == PATCH_ON_DISK) {
      
        /* Get the patch from disk */

        if (fseek(this->file, loc_p->u.loc, SEEK_SET)) {
          free(buf.val_void[0]);
	  LOG_RETURN_ERROR("seeking patch on disk",  "UnscramblePatches",false);
	}
        if (fread(this->buf.val_void[0], this->patch_size, 1, 
	          this->file) != 1) {
          free(buf.val_void[0]);
          LOG_RETURN_ERROR("reading patch from disk","UnscramblePatches",false);
	}

	/* Store the patch in the output buffer */

        switch (output_data_type)
        {
          case DFNT_CHAR8:
	    il_rel = 0;
            for (il = il1; il < il2; il++)
	    {
	      is_rel = 0;
              for (is = is1; is < is2; is++)
	      {
                switch (this->data_type)
                {
                  case DFNT_CHAR8:
                    buf.val_char8[il_rel][is] = 
                      this->buf.val_char8[il_rel][is_rel++];
                    break;
                  case DFNT_UINT8:
                    buf.val_char8[il_rel][is] = 
                      this->buf.val_uint8[il_rel][is_rel++];
                    break;
                  case DFNT_INT8:
                    buf.val_char8[il_rel][is] = 
                      this->buf.val_int8[il_rel][is_rel++];
                    break;
                  case DFNT_UINT16:
                    buf.val_char8[il_rel][is] = (char8)
                      this->buf.val_uint16[il_rel][is_rel++];
                    break;
                  case DFNT_INT16:
                    buf.val_char8[il_rel][is] = (char8)
                      this->buf.val_int16[il_rel][is_rel++];
                    break;
                  case DFNT_UINT32:
                    buf.val_char8[il_rel][is] = (char8)
                      this->buf.val_uint32[il_rel][is_rel++];
                    break;
                  case DFNT_INT32:
                    buf.val_char8[il_rel][is] = (char8)
                      this->buf.val_int32[il_rel][is_rel++];
                    break;
                }
	      }
	      il_rel++;
	    }
            break;
          case DFNT_UINT8:
	    il_rel = 0;
            for (il = il1; il < il2; il++)
	    {
	      is_rel = 0;
              for (is = is1; is < is2; is++)
	      {
                switch (this->data_type)
                {
                  case DFNT_CHAR8:
                    buf.val_uint8[il_rel][is] = 
                      this->buf.val_char8[il_rel][is_rel++];
                    break;
                  case DFNT_UINT8:
                    buf.val_uint8[il_rel][is] = 
                      this->buf.val_uint8[il_rel][is_rel++];
                    break;
                  case DFNT_INT8:
                    buf.val_uint8[il_rel][is] = 
                      this->buf.val_int8[il_rel][is_rel++];
                    break;
                  case DFNT_UINT16:
                    buf.val_uint8[il_rel][is] = (uint8)
                      this->buf.val_uint16[il_rel][is_rel++];
                    break;
                  case DFNT_INT16:
                    buf.val_uint8[il_rel][is] = (uint8)
                      this->buf.val_int16[il_rel][is_rel++];
                    break;
                  case DFNT_UINT32:
                    buf.val_uint8[il_rel][is] = (uint8)
                      this->buf.val_uint32[il_rel][is_rel++];
                    break;
                  case DFNT_INT32:
                    buf.val_uint8[il_rel][is] = (uint8)
                      this->buf.val_int32[il_rel][is_rel++];
                    break;
                }
	      }
	      il_rel++;
	    }
            break;
          case DFNT_INT8:
	    il_rel = 0;
            for (il = il1; il < il2; il++)
	    {
	      is_rel = 0;
              for (is = is1; is < is2; is++)
	      {
                switch (this->data_type)
                {
                  case DFNT_CHAR8:
                    buf.val_int8[il_rel][is] = 
                      this->buf.val_char8[il_rel][is_rel++];
                    break;
                  case DFNT_UINT8:
                    buf.val_int8[il_rel][is] = 
                      this->buf.val_uint8[il_rel][is_rel++];
                    break;
                  case DFNT_INT8:
                    buf.val_int8[il_rel][is] = 
                      this->buf.val_int8[il_rel][is_rel++];
                    break;
                  case DFNT_UINT16:
                    buf.val_int8[il_rel][is] = (int8)
                      this->buf.val_uint16[il_rel][is_rel++];
                    break;
                  case DFNT_INT16:
                    buf.val_int8[il_rel][is] = (int8)
                      this->buf.val_int16[il_rel][is_rel++];
                    break;
                  case DFNT_UINT32:
                    buf.val_int8[il_rel][is] = (int8)
                      this->buf.val_uint32[il_rel][is_rel++];
                    break;
                  case DFNT_INT32:
                    buf.val_int8[il_rel][is] = (int8)
                      this->buf.val_int32[il_rel][is_rel++];
                    break;
                }
	      }
	      il_rel++;
	    }
            break;
          case DFNT_UINT16:
	    il_rel = 0;
            for (il = il1; il < il2; il++)
	    {
	      is_rel = 0;
              for (is = is1; is < is2; is++)
	      {
                switch (this->data_type)
                {
                  case DFNT_CHAR8:
                    buf.val_uint16[il_rel][is] = 
                      this->buf.val_char8[il_rel][is_rel++];
                    break;
                  case DFNT_UINT8:
                    buf.val_uint16[il_rel][is] = 
                      this->buf.val_uint8[il_rel][is_rel++];
                    break;
                  case DFNT_INT8:
                    buf.val_uint16[il_rel][is] = 
                      this->buf.val_int8[il_rel][is_rel++];
                    break;
                  case DFNT_UINT16:
                    buf.val_uint16[il_rel][is] = 
                      this->buf.val_uint16[il_rel][is_rel++];
                    break;
                  case DFNT_INT16:
                    buf.val_uint16[il_rel][is] = 
                      this->buf.val_int16[il_rel][is_rel++];
                    break;
                  case DFNT_UINT32:
                    buf.val_uint16[il_rel][is] = (uint16)
                      this->buf.val_uint32[il_rel][is_rel++];
                    break;
                  case DFNT_INT32:
                    buf.val_uint16[il_rel][is] = (uint16)
                      this->buf.val_int32[il_rel][is_rel++];
                    break;
                }
	      }
	      il_rel++;
	    }
            break;
          case DFNT_INT16:
	    il_rel = 0;
            for (il = il1; il < il2; il++)
	    {
	      is_rel = 0;
              for (is = is1; is < is2; is++)
	      {
                switch (this->data_type)
                {
                  case DFNT_CHAR8:
                    buf.val_int16[il_rel][is] = 
                      this->buf.val_char8[il_rel][is_rel++];
                    break;
                  case DFNT_UINT8:
                    buf.val_int16[il_rel][is] = 
                      this->buf.val_uint8[il_rel][is_rel++];
                    break;
                  case DFNT_INT8:
                    buf.val_int16[il_rel][is] = 
                      this->buf.val_int8[il_rel][is_rel++];
                    break;
                  case DFNT_UINT16:
                    buf.val_int16[il_rel][is] = 
                      this->buf.val_uint16[il_rel][is_rel++];
                    break;
                  case DFNT_INT16:
                    buf.val_int16[il_rel][is] = 
                      this->buf.val_int16[il_rel][is_rel++];
                    break;
                  case DFNT_UINT32:
                    buf.val_int16[il_rel][is] = (int16)
                      this->buf.val_uint32[il_rel][is_rel++];
                    break;
                  case DFNT_INT32:
                    buf.val_int16[il_rel][is] = (int16)
                      this->buf.val_int32[il_rel][is_rel++];
                    break;
                }
	      }
	      il_rel++;
	    }
            break;
          case DFNT_UINT32:
            il_rel = 0;
            for (il = il1; il < il2; il++)
            {
              is_rel = 0;
              for (is = is1; is < is2; is++)
              {
                switch (this->data_type)
                {
                  case DFNT_CHAR8:
                    buf.val_uint32[il_rel][is] =
                      this->buf.val_char8[il_rel][is_rel++];
                    break;
                  case DFNT_UINT8:
                    buf.val_uint32[il_rel][is] =
                      this->buf.val_uint8[il_rel][is_rel++];
                    break;
                  case DFNT_INT8:
                    buf.val_uint32[il_rel][is] =
                      this->buf.val_int8[il_rel][is_rel++];
                    break;
                  case DFNT_UINT16:
                    buf.val_uint32[il_rel][is] =
                      this->buf.val_uint16[il_rel][is_rel++];
                    break;
                  case DFNT_INT16:
                    buf.val_uint32[il_rel][is] =
                      this->buf.val_int16[il_rel][is_rel++];
                    break;
                  case DFNT_UINT32:
                    buf.val_uint32[il_rel][is] =
                      this->buf.val_uint32[il_rel][is_rel++];
                    break;
                  case DFNT_INT32:
                    buf.val_uint32[il_rel][is] =
                      this->buf.val_int32[il_rel][is_rel++];
                    break;
                }
              }
              il_rel++;
            }
            break;
          case DFNT_INT32:
            il_rel = 0;
            for (il = il1; il < il2; il++)
            {
              is_rel = 0;
              for (is = is1; is < is2; is++)
              {
                switch (this->data_type)
                {
                  case DFNT_CHAR8:
                    buf.val_int32[il_rel][is] =
                      this->buf.val_char8[il_rel][is_rel++];
                    break;
                  case DFNT_UINT8:
                    buf.val_int32[il_rel][is] =
                      this->buf.val_uint8[il_rel][is_rel++];
                    break;
                  case DFNT_INT8:
                    buf.val_int32[il_rel][is] =
                      this->buf.val_int8[il_rel][is_rel++];
                    break;
                  case DFNT_UINT16:
                    buf.val_int32[il_rel][is] =
                      this->buf.val_uint16[il_rel][is_rel++];
                    break;
                  case DFNT_INT16:
                    buf.val_int32[il_rel][is] =
                      this->buf.val_int16[il_rel][is_rel++];
                    break;
                  case DFNT_UINT32:
                    buf.val_int32[il_rel][is] =
                      this->buf.val_uint32[il_rel][is_rel++];
                    break;
                  case DFNT_INT32:
                    buf.val_int32[il_rel][is] =
                      this->buf.val_int32[il_rel][is_rel++];
                    break;
                }
              }
              il_rel++;
            }
            break;
          default:
            free(buf.val_void[0]);
            LOG_RETURN_ERROR("invalid data type (b)","UnscramblePatches",false);
        }

      } else {

	/* Patch is null so put fill in the line */

        switch (output_data_type) {
          case DFNT_CHAR8:
	    il_rel = 0;
            for (il = il1; il < il2; il++) {
              for (is = is1; is < is2; is++)
                buf.val_char8[il_rel][is] = fill_char8;
	      il_rel++;
	    }
            break;
          case DFNT_UINT8:
	    il_rel = 0;
            for (il = il1; il < il2; il++) {
              for (is = is1; is < is2; is++)
                buf.val_uint8[il_rel][is] = fill_uint8;
	      il_rel++;
	    }
            break;
          case DFNT_INT8:
	    il_rel = 0;
            for (il = il1; il < il2; il++) {
              for (is = is1; is < is2; is++)
                buf.val_int8[il_rel][is] = fill_int8;
	      il_rel++;
	    }
            break;
          case DFNT_INT16:
	    il_rel = 0;
            for (il = il1; il < il2; il++) {
              for (is = is1; is < is2; is++)
                buf.val_int16[il_rel][is] = fill_int16;
	      il_rel++;
	    }
            break;
          case DFNT_UINT16:
	    il_rel = 0;
            for (il = il1; il < il2; il++) {
              for (is = is1; is < is2; is++)
                buf.val_uint16[il_rel][is] = fill_uint16;
	      il_rel++;
	    }
            break;
          case DFNT_INT32:
            il_rel = 0;
            for (il = il1; il < il2; il++) {
              for (is = is1; is < is2; is++)
                buf.val_int32[il_rel][is] = fill_int32;
              il_rel++;
            }
            break;
          case DFNT_UINT32:
            il_rel = 0;
            for (il = il1; il < il2; il++) {
              for (is = is1; is < is2; is++)
                buf.val_uint32[il_rel][is] = fill_uint32;
              il_rel++;
            }
            break;
          default:
            free(buf.val_void[0]);
            LOG_RETURN_ERROR("invalid data type (c)","UnscramblePatches",false);
        }
      }
    } /* for (is_patch ... */

    /* If using NN kernel, fill any unfilled gaps.  There are cases where
       pixels don't get filled (mainly down the middle of the image) during
       the NN resampling process.  We want to use a quick, brute-force method
       to fill those values in the output image. */

    if(kernel_type == NN)
    {
      if (!FillOutput(buf.val_void, NLINE_PATCH, output->size.s,
          output_data_type, this->fill_value, slope, same_data_type))
      {
        LOG_RETURN_ERROR("filling gaps in output file", "UnscramblePatches",
          false);
      }
    }

    /* Write the lines to disk */

    il_rel = 0;
    for (il = il1; il < il2; il++) 
    {
      /* Output can be HDF, GeoTiff, raw binary, or both HDF and GeoTiff */
      if(output_format == HDF_FMT || output_format == BOTH)
      {
        if (!WriteOutput(output, il, buf.val_void[il_rel]))  
        {
          free(buf.val_void[0]);
          LOG_RETURN_ERROR("writing output file", "UnscramblePatches", false);
        }

        /* Don't increment il_rel if we still need to output to GeoTiff */
        if(output_format == HDF_FMT)
          il_rel++;
      }

      if(output_format == GEOTIFF_FMT || output_format == BOTH)
      {
        uint16 zero = 0;
        GEOTIFF_WriteScanline( GeoTiffFile, buf.val_void[il_rel++], &il, &zero);
        /* TIFFWriteScanline(GeoTiffFile->tif, buf.val_void[il_rel++], il, 0);
         */
      }

      if(output_format == RB_FMT)
      {
        RBWriteScanLine(rbfile, output, il, buf.val_void[il_rel++]);
      }
    } /* for (il ... */

#ifdef DEBUG_ZEROS
    il_rel = 0;
    if (il_print > 0) printf("\n");
    il_print = -1;
    for (il = il1; il < il2; il++) {
      for (is = 0; is < output->size.s; is++) {
	zeros[0][is] = zeros[1][is];
	zeros[1][is] = zeros[2][is];
	zeros[2][is] = 0;

	if (buf.val_int16[il_rel][is] == fill_int16) zeros[2][is]++;
	if (is <= 0) zeros[2][is]++;
	else if (buf.val_int16[il_rel][is - 1] == fill_int16) zeros[2][is]++;
	if (is >= (output->size.s - 1)) zeros[2][is]++;
	else if (buf.val_int16[il_rel][is + 1] == fill_int16) zeros[2][is]++;

        nzero = zeros[0][is] + zeros[1][is] + zeros[2][is];
	if (nzero == 1  &&  zero_chk[is] > 0) {
          nzero_tot++;
	  if (il == il_print) printf(", %d", is);
	  else {
	    if (il_print > 0) printf("\n");
  	    printf("zero: il %d  is: %d", (il - 1), is);
	    il_print = il;
	  }
	}
	zero_chk[is] = (buf.val_int16[il_rel][is] == fill_int16) ? 1 : 0;
      }
      il_rel++;
    }
#endif

  } /* for (il_patch ... */

#ifdef DEBUG_ZEROS
  if (il_print > 0) printf("\n");
  printf("total number of zeros %d\n", nzero_tot);
  free(zero_chk);
#endif

  /* Free the buffer */

  free(buf.val_void[0]);

  return true;
}


bool FillOutput(void *void_buf[NLINE_PATCH], int nlines, int nsamps,
     int32 output_data_type, double fill_value, double slope,
     bool same_data_type)
/* 
!C******************************************************************************

!Description: 'FillOutput' fills any fill pixels that aren't part of the
 actual background fill area.  This is specifically for NN processing, since
 it tends to leave holes in the output product.  The goal is to keep the
 same exact pixel values in the output product, thus an average or weighting
 of surrounding pixels is not a valid option.
 
!Input Parameters:
 void_buf         void buffer containing the data being processed
 nlines           number of lines in void buffer
 nsamps           number of samples in void buffer
 output_data_type output data type, patches are stored in the input data
                    type
 fill_value       value that has been used to fill the output image
 slope            slope for input data type to output data type conversion
 same_data_type   are the input and output data types the same?

!Output Parameters:
 val_void         fill pixels are filled in the void buffer
 (returns)        status:
                    'true' = okay
		    'false' = error return

!Team Unique Header:

!END****************************************************************************
*/
{
  char8 *val_char8_p[NLINE_PATCH];
  uint8 *val_uint8_p[NLINE_PATCH];
  int8 *val_int8_p[NLINE_PATCH];
  uint16 *val_uint16_p[NLINE_PATCH];
  int16 *val_int16_p[NLINE_PATCH];
  uint32 *val_uint32_p[NLINE_PATCH];
  int32 *val_int32_p[NLINE_PATCH];
  char8 fill_char8 = 0;
  uint8 fill_uint8 = 0;
  int8 fill_int8 = 0;
  uint16 fill_uint16 = 0;
  int16 fill_int16 = 0;
  uint32 fill_uint32 = 0;
  int32 fill_int32 = 0;
  int i, il, is;           /* looping variables for lines and samples */
  int neighbor_array[8];   /* eight neighbor pixels to use for filling */
  int neighbor_count;      /* count of the actual number of valid neighbors */
  int median;              /* median value of value in the neighbor array */

  /* Determine the fill value used for the output data */
  switch (output_data_type) {
    case DFNT_CHAR8:
      fill_char8 = ConvertToChar8(fill_value, slope, same_data_type);
      for (i = 0; i < nlines; i++)
        val_char8_p[i] = void_buf[i];
      break;
    case DFNT_UINT8:
      fill_uint8 = ConvertToUint8(fill_value, slope, same_data_type);
      for (i = 0; i < nlines; i++)
        val_uint8_p[i] = void_buf[i];
      break;
    case DFNT_INT8:
      fill_int8 = ConvertToInt8(fill_value, slope, same_data_type);
      for (i = 0; i < nlines; i++)
        val_int8_p[i] = void_buf[i];
      break;
    case DFNT_UINT16:
      fill_uint16 = ConvertToUint16(fill_value, slope, same_data_type);
      for (i = 0; i < nlines; i++)
        val_uint16_p[i] = void_buf[i];
      break;
    case DFNT_INT16:
      fill_int16 = ConvertToInt16(fill_value, slope, same_data_type);
      for (i = 0; i < nlines; i++)
        val_int16_p[i] = void_buf[i];
      break;
    case DFNT_UINT32:
      fill_uint32 = ConvertToUint32(fill_value, slope, same_data_type);
      for (i = 0; i < nlines; i++)
        val_uint32_p[i] = void_buf[i];
      break;
    case DFNT_INT32:
      fill_int32 = ConvertToInt32(fill_value, slope, same_data_type);
      for (i = 0; i < nlines; i++)
        val_int32_p[i] = void_buf[i];
      break;
    default:
      LOG_RETURN_ERROR("invalid data type (a)", "FillOutput", false);
  }

  /* Loop through the pixels in the buffer looking for fill pixels.  Only fill
     pixels with top and bottom neighbors being non-fill pixels will be
     processed */
  for (il = 0; il < nlines; il++)
  {
    for (is = 0; is < nsamps; is++)
    {
      /* Determine if the current pixel is a fill pixel and one we want to
         process (i.e. all surrounding pixels except possibly the left and
         right pixels are non-fill pixels) */
      switch (output_data_type) {
        case DFNT_CHAR8:
          if (val_char8_p[il][is] == fill_char8)
          {
            /* Check the surrounding neighbors.  If any of the top or bottom
               are fill then we won't fill this pixel. Fill pixels to the left
               and right are ok and even likely. */
            /* Top row */
            if (il-1 >= 0 && is-1 >= 0 &&
                val_char8_p[il-1][is-1] == fill_char8)
              break;
            if (il-1 >= 0 && val_char8_p[il-1][is] == fill_char8)
              break;
            if (il-1 >= 0 && is+1 < nsamps &&
                val_char8_p[il-1][is+1] == fill_char8)
              break;

            /* Bottom row */
            if (il+1 < nlines && is-1 >= 0 &&
                val_char8_p[il+1][is-1] == fill_char8)
              break;
            if (il+1 < nlines && val_char8_p[il+1][is] == fill_char8)
              break;
            if (il+1 < nlines && is+1 < nsamps &&
                val_char8_p[il+1][is+1] == fill_char8)
              break;

            /* All top and bottom neighbor pixels must be non-fill, so we'll
               grab the non-fill neighbor pixels to use for filling */
            neighbor_count = 0;
            if (il-1 >= 0 && is-1 >= 0)  /* ul */
              neighbor_array[neighbor_count++] = val_char8_p[il-1][is-1];
            if (il-1 >= 0)  /* top */
              neighbor_array[neighbor_count++] = val_char8_p[il-1][is];
            if (il-1 >= 0 && is+1 < nsamps)  /* ur */
              neighbor_array[neighbor_count++] = val_char8_p[il-1][is+1];
            if (is-1 >= 0 && val_char8_p[il][is-1] != fill_char8)  /* left */
              neighbor_array[neighbor_count++] = val_char8_p[il][is-1];
            if (is+1 < nsamps && val_char8_p[il][is+1] != fill_char8)/* right */
              neighbor_array[neighbor_count++] = val_char8_p[il][is+1];
            if (il+1 < nlines && is-1 >= 0)  /* ll */
              neighbor_array[neighbor_count++] = val_char8_p[il+1][is-1];
            if (il+1 < nlines)  /* bottom */
              neighbor_array[neighbor_count++] = val_char8_p[il+1][is];
            if (il+1 < nlines && is+1 < nsamps)  /* lr */
              neighbor_array[neighbor_count++] = val_char8_p[il+1][is+1];

            /* Find the median */
            median = FindMedian (neighbor_array, neighbor_count);

            /* Fill the current pixel with the median value */
            val_char8_p[il][is] = (char8) median;
          }
          break;

        case DFNT_UINT8:
          if (val_uint8_p[il][is] == fill_uint8)
          {
            /* Check the surrounding neighbors.  If any of the top or bottom
               are fill then we won't fill this pixel. Fill pixels to the left
               and right are ok and even likely. */
            /* Top row */
            if (il-1 >= 0 && is-1 >= 0 &&
                val_uint8_p[il-1][is-1] == fill_uint8)
              break;
            if (il-1 >= 0 && val_uint8_p[il-1][is] == fill_uint8)
              break;
            if (il-1 >= 0 && is+1 < nsamps &&
                val_uint8_p[il-1][is+1] == fill_uint8)
              break;

            /* Bottom row */
            if (il+1 < nlines && is-1 >= 0 &&
                val_uint8_p[il+1][is-1] == fill_uint8)
              break;
            if (il+1 < nlines && val_uint8_p[il+1][is] == fill_uint8)
              break;
            if (il+1 < nlines && is+1 < nsamps &&
                val_uint8_p[il+1][is+1] == fill_uint8)
              break;

            /* All top and bottom neighbor pixels must be non-fill, so we'll
               grab the non-fill neighbor pixels to use for filling */
            neighbor_count = 0;
            if (il-1 >= 0 && is-1 >= 0)  /* ul */
              neighbor_array[neighbor_count++] = val_uint8_p[il-1][is-1];
            if (il-1 >= 0)  /* top */
              neighbor_array[neighbor_count++] = val_uint8_p[il-1][is];
            if (il-1 >= 0 && is+1 < nsamps)  /* ur */
              neighbor_array[neighbor_count++] = val_uint8_p[il-1][is+1];
            if (is-1 >= 0 && val_uint8_p[il][is-1] != fill_uint8)  /* left */
              neighbor_array[neighbor_count++] = val_uint8_p[il][is-1];
            if (is+1 < nsamps && val_uint8_p[il][is+1] != fill_uint8)/* right */
              neighbor_array[neighbor_count++] = val_uint8_p[il][is+1];
            if (il+1 < nlines && is-1 >= 0)  /* ll */
              neighbor_array[neighbor_count++] = val_uint8_p[il+1][is-1];
            if (il+1 < nlines)  /* bottom */
              neighbor_array[neighbor_count++] = val_uint8_p[il+1][is];
            if (il+1 < nlines && is+1 < nsamps)  /* lr */
              neighbor_array[neighbor_count++] = val_uint8_p[il+1][is+1];

            /* Find the median */
            median = FindMedian (neighbor_array, neighbor_count);

            /* Fill the current pixel with the median value */
            val_uint8_p[il][is] = (uint8) median;
          }
          break;

        case DFNT_INT8:
          if (val_int8_p[il][is] == fill_int8)
          {
            /* Check the surrounding neighbors.  If any of the top or bottom
               are fill then we won't fill this pixel. Fill pixels to the left
               and right are ok and even likely. */
            /* Top row */
            if (il-1 >= 0 && is-1 >= 0 &&
                val_int8_p[il-1][is-1] == fill_int8)
              break;
            if (il-1 >= 0 && val_int8_p[il-1][is] == fill_int8)
              break;
            if (il-1 >= 0 && is+1 < nsamps &&
                val_int8_p[il-1][is+1] == fill_int8)
              break;

            /* Bottom row */
            if (il+1 < nlines && is-1 >= 0 &&
                val_int8_p[il+1][is-1] == fill_int8)
              break;
            if (il+1 < nlines && val_int8_p[il+1][is] == fill_int8)
              break;
            if (il+1 < nlines && is+1 < nsamps &&
                val_int8_p[il+1][is+1] == fill_int8)
              break;

            /* All top and bottom neighbor pixels must be non-fill, so we'll
               grab the non-fill neighbor pixels to use for filling */
            neighbor_count = 0;
            if (il-1 >= 0 && is-1 >= 0)  /* ul */
              neighbor_array[neighbor_count++] = val_int8_p[il-1][is-1];
            if (il-1 >= 0)  /* top */
              neighbor_array[neighbor_count++] = val_int8_p[il-1][is];
            if (il-1 >= 0 && is+1 < nsamps)  /* ur */
              neighbor_array[neighbor_count++] = val_int8_p[il-1][is+1];
            if (is-1 >= 0 && val_int8_p[il][is-1] != fill_int8)  /* left */
              neighbor_array[neighbor_count++] = val_int8_p[il][is-1];
            if (is+1 < nsamps && val_int8_p[il][is+1] != fill_int8)/* right */
              neighbor_array[neighbor_count++] = val_int8_p[il][is+1];
            if (il+1 < nlines && is-1 >= 0)  /* ll */
              neighbor_array[neighbor_count++] = val_int8_p[il+1][is-1];
            if (il+1 < nlines)  /* bottom */
              neighbor_array[neighbor_count++] = val_int8_p[il+1][is];
            if (il+1 < nlines && is+1 < nsamps)  /* lr */
              neighbor_array[neighbor_count++] = val_int8_p[il+1][is+1];

            /* Find the median */
            median = FindMedian (neighbor_array, neighbor_count);

            /* Fill the current pixel with the median value */
            val_int8_p[il][is] = (int8) median;
          }
          break;

        case DFNT_UINT16:
          if (val_uint16_p[il][is] == fill_uint16)
          {
            /* Check the surrounding neighbors.  If any of the top or bottom
               are fill then we won't fill this pixel. Fill pixels to the left
               and right are ok and even likely. */
            /* Top row */
            if (il-1 >= 0 && is-1 >= 0 &&
                val_uint16_p[il-1][is-1] == fill_uint16)
              break;
            if (il-1 >= 0 && val_uint16_p[il-1][is] == fill_uint16)
              break;
            if (il-1 >= 0 && is+1 < nsamps &&
                val_uint16_p[il-1][is+1] == fill_uint16)
              break;

            /* Bottom row */
            if (il+1 < nlines && is-1 >= 0 &&
                val_uint16_p[il+1][is-1] == fill_uint16)
              break;
            if (il+1 < nlines && val_uint16_p[il+1][is] == fill_uint16)
              break;
            if (il+1 < nlines && is+1 < nsamps &&
                val_uint16_p[il+1][is+1] == fill_uint16)
              break;

            /* All top and bottom neighbor pixels must be non-fill, so we'll
               grab the non-fill neighbor pixels to use for filling */
            neighbor_count = 0;
            if (il-1 >= 0 && is-1 >= 0)  /* ul */
              neighbor_array[neighbor_count++] = val_uint16_p[il-1][is-1];
            if (il-1 >= 0)  /* top */
              neighbor_array[neighbor_count++] = val_uint16_p[il-1][is];
            if (il-1 >= 0 && is+1 < nsamps)  /* ur */
              neighbor_array[neighbor_count++] = val_uint16_p[il-1][is+1];
            if (is-1 >= 0 && val_uint16_p[il][is-1] != fill_uint16)  /* left */
              neighbor_array[neighbor_count++] = val_uint16_p[il][is-1];
            if (is+1 < nsamps && val_uint16_p[il][is+1] != fill_uint16)/* right */
              neighbor_array[neighbor_count++] = val_uint16_p[il][is+1];
            if (il+1 < nlines && is-1 >= 0)  /* ll */
              neighbor_array[neighbor_count++] = val_uint16_p[il+1][is-1];
            if (il+1 < nlines)  /* bottom */
              neighbor_array[neighbor_count++] = val_uint16_p[il+1][is];
            if (il+1 < nlines && is+1 < nsamps)  /* lr */
              neighbor_array[neighbor_count++] = val_uint16_p[il+1][is+1];

            /* Find the median */
            median = FindMedian (neighbor_array, neighbor_count);

            /* Fill the current pixel with the median value */
            val_uint16_p[il][is] = (uint16) median;
          }
          break;

        case DFNT_INT16:
          if (val_int16_p[il][is] == fill_int16)
          {
            /* Check the surrounding neighbors.  If any of the top or bottom
               are fill then we won't fill this pixel. Fill pixels to the left
               and right are ok and even likely. */
            /* Top row */
            if (il-1 >= 0 && is-1 >= 0 &&
                val_int16_p[il-1][is-1] == fill_int16)
              break;
            if (il-1 >= 0 && val_int16_p[il-1][is] == fill_int16)
              break;
            if (il-1 >= 0 && is+1 < nsamps &&
                val_int16_p[il-1][is+1] == fill_int16)
              break;

            /* Bottom row */
            if (il+1 < nlines && is-1 >= 0 &&
                val_int16_p[il+1][is-1] == fill_int16)
              break;
            if (il+1 < nlines && val_int16_p[il+1][is] == fill_int16)
              break;
            if (il+1 < nlines && is+1 < nsamps &&
                val_int16_p[il+1][is+1] == fill_int16)
              break;

            /* All top and bottom neighbor pixels must be non-fill, so we'll
               grab the non-fill neighbor pixels to use for filling */
            neighbor_count = 0;
            if (il-1 >= 0 && is-1 >= 0)  /* ul */
              neighbor_array[neighbor_count++] = val_int16_p[il-1][is-1];
            if (il-1 >= 0)  /* top */
              neighbor_array[neighbor_count++] = val_int16_p[il-1][is];
            if (il-1 >= 0 && is+1 < nsamps)  /* ur */
              neighbor_array[neighbor_count++] = val_int16_p[il-1][is+1];
            if (is-1 >= 0 && val_int16_p[il][is-1] != fill_int16)  /* left */
              neighbor_array[neighbor_count++] = val_int16_p[il][is-1];
            if (is+1 < nsamps && val_int16_p[il][is+1] != fill_int16)/* right */
              neighbor_array[neighbor_count++] = val_int16_p[il][is+1];
            if (il+1 < nlines && is-1 >= 0)  /* ll */
              neighbor_array[neighbor_count++] = val_int16_p[il+1][is-1];
            if (il+1 < nlines)  /* bottom */
              neighbor_array[neighbor_count++] = val_int16_p[il+1][is];
            if (il+1 < nlines && is+1 < nsamps)  /* lr */
              neighbor_array[neighbor_count++] = val_int16_p[il+1][is+1];

            /* Find the median */
            median = FindMedian (neighbor_array, neighbor_count);

            /* Fill the current pixel with the median value */
            val_int16_p[il][is] = (int16) median;
          }
          break;

        case DFNT_UINT32:
          if (val_uint32_p[il][is] == fill_uint32)
          {
            /* Check the surrounding neighbors.  If any of the top or bottom
               are fill then we won't fill this pixel. Fill pixels to the left
               and right are ok and even likely. */
            /* Top row */
            if (il-1 >= 0 && is-1 >= 0 &&
                val_uint32_p[il-1][is-1] == fill_uint32)
              break;
            if (il-1 >= 0 && val_uint32_p[il-1][is] == fill_uint32)
              break;
            if (il-1 >= 0 && is+1 < nsamps &&
                val_uint32_p[il-1][is+1] == fill_uint32)
              break;

            /* Bottom row */
            if (il+1 < nlines && is-1 >= 0 &&
                val_uint32_p[il+1][is-1] == fill_uint32)
              break;
            if (il+1 < nlines && val_uint32_p[il+1][is] == fill_uint32)
              break;
            if (il+1 < nlines && is+1 < nsamps &&
                val_uint32_p[il+1][is+1] == fill_uint32)
              break;

            /* All top and bottom neighbor pixels must be non-fill, so we'll
               grab the non-fill neighbor pixels to use for filling */
            neighbor_count = 0;
            if (il-1 >= 0 && is-1 >= 0)  /* ul */
              neighbor_array[neighbor_count++] = val_uint32_p[il-1][is-1];
            if (il-1 >= 0)  /* top */
              neighbor_array[neighbor_count++] = val_uint32_p[il-1][is];
            if (il-1 >= 0 && is+1 < nsamps)  /* ur */
              neighbor_array[neighbor_count++] = val_uint32_p[il-1][is+1];
            if (is-1 >= 0 && val_uint32_p[il][is-1] != fill_uint32)  /* left */
              neighbor_array[neighbor_count++] = val_uint32_p[il][is-1];
            if (is+1 < nsamps && val_uint32_p[il][is+1] != fill_uint32)/* right */
              neighbor_array[neighbor_count++] = val_uint32_p[il][is+1];
            if (il+1 < nlines && is-1 >= 0)  /* ll */
              neighbor_array[neighbor_count++] = val_uint32_p[il+1][is-1];
            if (il+1 < nlines)  /* bottom */
              neighbor_array[neighbor_count++] = val_uint32_p[il+1][is];
            if (il+1 < nlines && is+1 < nsamps)  /* lr */
              neighbor_array[neighbor_count++] = val_uint32_p[il+1][is+1];

            /* Find the median */
            median = FindMedian (neighbor_array, neighbor_count);

            /* Fill the current pixel with the median value */
            val_uint32_p[il][is] = (uint32) median;
          }
          break;

        case DFNT_INT32:
          if (val_int32_p[il][is] == fill_int32)
          {
            /* Check the surrounding neighbors.  If any of the top or bottom
               are fill then we won't fill this pixel. Fill pixels to the left
               and right are ok and even likely. */
            /* Top row */
            if (il-1 >= 0 && is-1 >= 0 &&
                val_int32_p[il-1][is-1] == fill_int32)
              break;
            if (il-1 >= 0 && val_int32_p[il-1][is] == fill_int32)
              break;
            if (il-1 >= 0 && is+1 < nsamps &&
                val_int32_p[il-1][is+1] == fill_int32)
              break;

            /* Bottom row */
            if (il+1 < nlines && is-1 >= 0 &&
                val_int32_p[il+1][is-1] == fill_int32)
              break;
            if (il+1 < nlines && val_int32_p[il+1][is] == fill_int32)
              break;
            if (il+1 < nlines && is+1 < nsamps &&
                val_int32_p[il+1][is+1] == fill_int32)
              break;

            /* All top and bottom neighbor pixels must be non-fill, so we'll
               grab the non-fill neighbor pixels to use for filling */
            neighbor_count = 0;
            if (il-1 >= 0 && is-1 >= 0)  /* ul */
              neighbor_array[neighbor_count++] = val_int32_p[il-1][is-1];
            if (il-1 >= 0)  /* top */
              neighbor_array[neighbor_count++] = val_int32_p[il-1][is];
            if (il-1 >= 0 && is+1 < nsamps)  /* ur */
              neighbor_array[neighbor_count++] = val_int32_p[il-1][is+1];
            if (is-1 >= 0 && val_int32_p[il][is-1] != fill_int32)  /* left */
              neighbor_array[neighbor_count++] = val_int32_p[il][is-1];
            if (is+1 < nsamps && val_int32_p[il][is+1] != fill_int32)/* right */
              neighbor_array[neighbor_count++] = val_int32_p[il][is+1];
            if (il+1 < nlines && is-1 >= 0)  /* ll */
              neighbor_array[neighbor_count++] = val_int32_p[il+1][is-1];
            if (il+1 < nlines)  /* bottom */
              neighbor_array[neighbor_count++] = val_int32_p[il+1][is];
            if (il+1 < nlines && is+1 < nsamps)  /* lr */
              neighbor_array[neighbor_count++] = val_int32_p[il+1][is+1];

            /* Find the median */
            median = FindMedian (neighbor_array, neighbor_count);

            /* Fill the current pixel with the median value */
            val_int32_p[il][is] = (int32) median;
          }
          break;
      }
    }
  }

  return true;
}


int FindMedian(int *buf, int nvals)
/* 
!C******************************************************************************

!Description: 'FindMedian' determines the median value of all values in the
 buffer.
 
!Input Parameters:
 buf              buffer containing the data values
 nvals            number of values in buffer

!Output Parameters:
 buf              array is sorted, in place
 (returns)        median value of the array

!Team Unique Header:

!END****************************************************************************
*/

{
  int i, j;          /* looping variables */
  int temp;          /* temporary value for swapping */
  int medianbin;     /* location of median value in the array */

  /* Sort the array */
  for (i = nvals-1; i >= 0; i--)
  {
    for (j = 1; j <= i; j++)
    {
      if (buf[j-1] > buf[j])
      {
        temp = buf[j-1];
        buf[j-1] = buf[j];
        buf[j] = temp;
      }
    }
  }

  /* Find the median - point where we have accumulated half the valid pixels
     in the array */
  medianbin = (int)(nvals / 2.0 + 0.5) - 1;   /* make it 0-based */
  if (medianbin < 0)
    medianbin = 0;

  /* Return the median value */
  return buf[medianbin];
}
