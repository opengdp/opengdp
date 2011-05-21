/*
!C****************************************************************************

!File: kernel.h

!Description: Header file for 'kernel.c' - see 'kernel.c' for more information.

!Revision History:
 Revision 1.0 2001/05/08
 Robert Wolfe
 Original Version.

 Revision 2.0 2003/10/15
 Gail Schmidt
 Removed user-defined kernels since they are not used.

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
   1. Structures are declared for the 'kernel' and 'kernel_type' data types.
   2. Only separable kernels (two 1D) are currently supported. True 2D kernels
      are not supported.
  
!END****************************************************************************
*/

#ifndef KERNEL_H
#define KERNEL_H

#include "resamp.h"
#include "bool.h"

#define MAX_KERNEL_SIZE (32)
#define MAX_NDELTA (128)

/* Structure for the 'kernel_type' data type */

typedef enum {
  NN,                   /* Nearest Neighbor */
  BL,                   /* Bi-linear */
  CC                    /* Cubic Convolution */
} Kernel_type_t;

/* Structure for the 'kernel' data type */

typedef struct {
  Kernel_type_t kernel_type; /* Kernel type (see 'kernel_type')*/
  Img_coord_int_t size;  /* Kernel size */
  Img_coord_int_t before;  /* Number of whole pixels before the kernel reference
                           pixel */
  Img_coord_int_t after;  /* Number of while pixels after the kernel reference
                           pixel */
  Img_coord_int_t delta_size;  /* Number of elements between whole pixel
                           locations (including boundaries) */
  Img_coord_double_t delta_inv;  /* Inverse of distance (delta) between kernel 
                           elements */
  Img_coord_double_t delta;  /* Distance (delta) between kernel elements */
  double *l[MAX_NDELTA + 1];  /* Kernel in the line direction */
  double *s[MAX_NDELTA + 1];  /* Kernel in the sample direction */
} Kernel_t;

/* Prototypes */

Kernel_t *GenKernel(Kernel_type_t kernel_type);
bool FreeKernel(Kernel_t *kernel);

#endif
