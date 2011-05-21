/*
!C****************************************************************************

!File: myhdf.h

!Description: Header file for myhdf.c - see myhdf.c for more information.

!Revision History:
 Revision 1.0 2001/05/08
 Robert Wolfe
 Original Version.

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
   1. Structures for Attributes, SDSs and SDS dimensions are declared.
   2. A maximum SDS rank allowed is no greater than 'MYHDF_MAX_RANK'.
   3. The maximum number of attribute values allowed is no greater than 
      'MYHDF_MAX_NATTR_VAL'.
  
!END****************************************************************************
*/

#ifndef MYHDF_H
#define MYHDF_H

#include "hdf.h"
#include "mfhdf.h"
#include "bool.h"
#include "space.h"
#include "common.h"

#define MYHDF_MAX_NATTR_VAL (4)	/* maximum number of attribute values expected */
#define HDF_ERROR (-1)

/* Structure to store information about the HDF SDS */

typedef struct {		 /* information about each dimension */
  int32 nval, id;	 /*    number of values and id */ 
  int32 type, nattr;	 /*    HDF data type and number of attributes */
  char *name;		 /*    dimension name */
} Myhdf_dim_t;

typedef struct {
  int32 index, id, rank; /* index, id and rank */
  int32 type, nattr;	 /* HDF data type and number of attributes */
  Myhdf_dim_t dim[MYHDF_MAX_RANK];  /* information about each dimension */
  char *name;		 /* SDS name */
} Myhdf_sds_t;

/* Structure to store information about the HDF attribute */

typedef struct {
  int32 id, type, nval;	/* id, data type and number of values */
  char *name;		/* attribute name */
} Myhdf_attr_t;

/* Prototypes */

bool GetSDSInfo(int32 sds_file_id, Myhdf_sds_t *sds);
bool GetSDSDimInfo(int32 sds_id, Myhdf_dim_t *dim, int irank);
bool PutSDSInfo(int32 sds_file_id, Myhdf_sds_t *sds);
bool PutSDSDimInfo(int32 sds_id, Myhdf_dim_t *dim, int irank);
bool GetAttrDouble(int32 sds_id, Myhdf_attr_t *attr, double *val);
bool ReadBoundCoords(char *infile, Geo_coord_t *ul_corner,
     Geo_coord_t *lr_corner);
bool ReadMetadata(int32 sd_id, char *attr, double bound_coords[],
     bool *coords_present);
bool DetermineResolution(Myhdf_sds_t *sds, Img_coord_int_t *ls_dim, int *ires);
bool DeterminePixelSize(char *geoloc_file_name, int num_input_sds,
  int ires[MAX_SDS_DIMS], int out_proj_num,
  double output_pixel_size[MAX_SDS_DIMS]);

#endif
