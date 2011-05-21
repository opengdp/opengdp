/*
!C****************************************************************************

!File: input.h

!Description: Header file for 'input.c' - see 'input.c' for more information.

!Revision History:
 Revision 1.0 2001/05/08
 Robert Wolfe
 Original Version.

 Revision 1.5 2002/12/02
 Gail Schmidt
 Added support for INT8 data types.

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
   1. Structure is declared for the 'input' data type.
  
!END****************************************************************************
*/

#ifndef INPUT_H
#define INPUT_H

#include "resamp.h"
#include "param.h"
#include "bool.h"
#include "myhdf.h"

/* Structure for the 'input' data type */

typedef struct {
  Img_coord_int_t dim;  /* SDS line and sample dimension locations */
  int extra_dim[MYHDF_MAX_RANK]; /* SDS dimensions other than line and sample */
  char *file_name;      /* Input file name */
  int32 sds_file_id;    /* SDS file id */
  bool open;            /* Flag to indicate whether input file is open 
                           for access; 'true' = open, 'false' = not open */
  Myhdf_sds_t sds;      /* SDS data structure */
  Img_coord_int_t size; /* Input file size */
  Img_coord_int_t scan_size; /* Input scan size */
  int ires;             /* Input resolution (relative to 1 km input); 
                           1 = 1 km; 2 = 500 m; 4 = 250 m */
  int iband;            /* Band number for application of band offset */
  int nscan;            /* Number of input scans */
  int data_type_size;   /* Input data type size (bytes) */
  union {               /* Input data buffer (one line of data) */
    void *val_void;
    char8 *val_char8;
    uint8 *val_uint8;
    int8 *val_int8;
    int16 *val_int16;
    uint16 *val_uint16;    
    int32 *val_int32;
    uint32 *val_uint32;    
  } buf;  
  int fill_value;       /* Fill value for the current SDS (even floats will
                           be an "int" value) */
} Input_t;

/* Prototypes */

Input_t *OpenInput(char *file_name, char *sds_name, 
                   int iband, int rank, int *dim, char *errstr);
bool CloseInput(Input_t *this);
bool FreeInput(Input_t *this);
bool FindInputDim(int rank, int *param_dim, Myhdf_dim_t *sds_dim,
                  int *extra_dim, Img_coord_int_t *dim, char *errstr);

#endif
