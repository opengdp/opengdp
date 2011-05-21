/*
!C****************************************************************************

!File: kernel.c
  
!Description: Functions generating or reading the resampling kernel.

!Revision History:
 Revision 1.0 2001/05/08
 Robert Wolfe
 Original Version.

 Revision 2.0 2003/10/15
 Gail Schmidt
 Removed the user kernel definition, since it isn't used.

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
   1. The following public functions handle the resampling kernels:

	GenKernel - Generate a resampling kernel.
	FreeKernel - Free the 'kernel' data structure memory.

   2. 'GenKernel' must be called before 'FreeKernel'.  
   3. 'FreeKernel' should be called to free the 'kernel' data structure 
      memory.

!END****************************************************************************
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "kernel.h"
#include "myerror.h"

#include <string.h>

/* Constants */

#define ALPHA_FACTOR (-0.5) /* Alpha factor for the cubic convolution kenel */
#define NDELTA (64)  /* Default 'ndelta' value for generated kernels */

/* Functions */

Kernel_t *GenKernel(Kernel_type_t kernel_type)
/* 
!C******************************************************************************

!Description: 'GenKernel' sets up the 'kernel' data structure and populates it 
 for a specific resampling kernel.
 
!Input Parameters:
 kernel_type    kernel type; either 'NN' (nearest neighbor), 'BL' (bi-linear) 
                or 'CC' (cubic convolution)

!Output Parameters:
 (returns)      'kernel' data structure or NULL when an error occurs

!Team Unique Header:

 ! Design Notes:
   1. An error status is returned when:
       a. memory allocation is not successful
   2. Error messages are handled with the 'LOG_RETURN_ERROR' macro.
   3. 'FreeKernel' should be called to deallocate memory used by the 
      'kernel' data structures.

!END****************************************************************************
*/
{
  Kernel_t *this;
  double w, x, a, x2, x3;
  int id, id1;
  double *l_p, *s_p;
  size_t n;

  int   do_old_nn = 0;
  char* chk_old_nn;

  /* Check the kernel types */

  if (kernel_type != NN  &&  
      kernel_type != BL  &&  
      kernel_type != CC)
    LOG_RETURN_ERROR("invalid kernel type", "GenKernel", (Kernel_t *)NULL);

  /* Create the kernel data structure */

  this = (Kernel_t *)malloc(sizeof(Kernel_t));
  if (this == (Kernel_t *)NULL) 
    LOG_RETURN_ERROR("allocating kernel structure", 
                 "GenKernel", (Kernel_t *)NULL);

  /* Set up the size for the different kernels */

  this->delta_size.l = this->delta_size.s = NDELTA + 1;
  this->delta_inv.l  = this->delta_inv.s  = (double)NDELTA;
  this->delta.l      = this->delta.s      = 1.0 / this->delta_inv.s;

  switch (kernel_type) {
    case NN:
    case BL:
      this->size.l   = this->size.s   = 2;
      this->before.l = this->before.s = 0;
      this->after.l  = this->after.s  = 1;
      break;

    case CC:
      this->size.l = this->size.s = 4;
      this->before.l = this->before.s = 1;
      this->after.l = this->after.s = 2;
      break;
  }

  /* Allocate space for each kernel */

  n = (size_t)(this->size.l * this->delta_size.l);
  l_p = (double *)calloc(n, sizeof(double));
  if (l_p == (double *)NULL) {
    free(this);
    LOG_RETURN_ERROR("allocating kernel buffer (l)", 
                 "GenKernel", (Kernel_t *)NULL);
  }
  for (id = 0; id < this->delta_size.l; id++) {
    this->l[id] = l_p;
    l_p += this->size.l;
  }

  s_p = (double *)calloc(n, sizeof(double));
  if (s_p == (double *)NULL) {
    free(this->l[0]);
    free(this);
    LOG_RETURN_ERROR("allocating kernel buffer (s)", 
                 "GenKernel", (Kernel_t *)NULL);
  }
  for (id = 0; id < this->delta_size.s; id++) {
    this->s[id] = s_p;
    s_p += this->size.s;
  }

  switch (kernel_type) {
    case NN:
      if ((chk_old_nn = getenv("OLDNN"))) 
      {
	 do_old_nn = (! strcmp(chk_old_nn, "DO_OLDNN"));
      }

      if (do_old_nn)
      {
	  for (id = 0; id < this->delta_size.l; id++) {
	    x = (double)id * this->delta.l;
	    if (x <= 0.5) {
	      this->l[id][0] = this->s[id][0] = 1.0;
	      this->l[id][1] = this->s[id][1] = 0.0;
	    } else {
	      this->l[id][0] = this->s[id][0] = 0.0;
	      this->l[id][1] = this->s[id][1] = 1.0;
	    }
	  }
      }
      else
      {
	  for (id = 0; id < this->delta_size.l; id++) {
	    x = (double)id * this->delta.l;

	    this->l[id][0] = this->s[id][0] = (double)1.0 - x;
	    this->l[id][1] = this->s[id][1] = x;
	  }
      }
      break;

    case BL:
      for (id = 0; id < this->delta_size.l; id++) {
        x = (double)id * this->delta.l;
	this->l[id][0] = this->s[id][0] = (double)1.0 - x;
	this->l[id][1] = this->s[id][1] = x;
      }
      break;

    case CC:
      a = ALPHA_FACTOR;
      for (id = 0; id < this->delta_size.l; id++) {
        id1 = this->delta_size.l - 1 - id;

        x = (double)id * this->delta.l;
        x2 = x * x;
        x3 = x2 * x;
	w = (double)(((a + 2.0) * x3) - ((a + 3.0) * x2) + 1.0);
	this->l[id][1] = this->s[id][1] = w;
	this->l[id1][2] = this->s[id1][2] = w;

        x += 1.0;
        x2 = x * x;
        x3 = x2 * x;
	w = (double)((a * x3) - (5.0 * a * x2) + (8.0 * a * x) - (4.0 * a));
	this->l[id][0] = this->s[id][0] = w;
	this->l[id1][3] = this->s[id1][3] = w;
      }
      break;
  }
  
/* #define DEBUG */
#ifdef DEBUG
  {
    int i = 0;
    int ik;
    printf("  i  ik id1  id       x       w\n");
    for (ik = 0; ik < this->size.l; ik++) {
      for (id1 = (this->delta_size.l - 1); id1 > -1; id1--) {
        id = this->delta_size.l - id1 - 1;
        x = fabs(((double)id * this->delta.l) + 
	                (double)(ik - this->before.l));
	printf("%3.3d  %2.2d  %2.2d  %2.2d  %6.3lf  %6.3lf\n", 
	       i, ik, id1, id, (double)x, (double)this->l[id1][ik]);
	i++;
      }
    }
    printf("\n");
  }
#endif

  return this;
}

bool FreeKernel(Kernel_t *this)
/* 
!C******************************************************************************

!Description: 'FreeKernel' frees the 'kernel' data structure memory.
 
!Input Parameters:
 this           'kernel' data structure; the following fields are input:
                   s, l

!Output Parameters:
 (returns)      status:
                  'true' = okay (always returned)

!Team Unique Header:

 ! Design Notes:
   1. Either 'GetUserKernel' or 'GenKernel' must be called before this routine 
      is called.
   2. An error status is never returned.

!END****************************************************************************
*/
{
  if (this != (Kernel_t *)NULL) {
    if (this->l[0] != (double *)NULL) free(this->l[0]);
    if (this->s[0] != (double *)NULL) free(this->s[0]);
    free(this);
  }

  return true;
}
