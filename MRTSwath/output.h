/*
!C****************************************************************************

!File: output.h

!Description: Header file for output.c - see output.c for more information.

!Revision History:
 Revision 1.0 2001/05/08
 Robert Wolfe
 Original Version.

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
   1. Structure is declared for the 'output' data type.
  
!END****************************************************************************
*/

#ifndef OUTPUT_H
#define OUTPUT_H

#include "resamp.h"
#include "bool.h"
#include "space.h"
#include "geoloc.h"
#include "input.h"
#include "myhdf.h"
#include "mystring.h"

/* Structure for the 'output' data type */

typedef struct {
  char *file_name;      /* Output file name */
  bool open;            /* Flag to indicate whether output file is open 
                           for access; 'true' = open, 'false' = not open */
  int32 sds_file_id;    /* SDS file id */
  int output_dt_size;   /* Output data type size */
  Myhdf_sds_t sds;      /* SDS data structure */
  Img_coord_int_t size; /* Output image size */
} Output_t;

/* Prototypes */

bool CreateOutput(char *file_name);
Output_t *OutputFile(char *file_name, char *sds_name, 
                     int output_data_type, Space_def_t *space_def);
bool CloseOutput(Output_t *this);
bool FreeOutput(Output_t *this);
int WriteMeta (char *output_filename, Space_def_t *output_space_def);
bool PutMetadata(Output_t *this, Geoloc_t *geoloc, Input_t *input);
bool WriteOutput(Output_t *this, int iline, void *buf);

#endif
