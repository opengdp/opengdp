/*
!C****************************************************************************

!File: const.h

!Description: Header file for declaring mathmatical constants.

!Revision History:
 Revision 1.0 1999/07/30
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
  
!END****************************************************************************
*/

#ifndef CONST_H
#define CONST_H

#include <math.h>

/*
 * PI, TWO_PI, and HALF_PI are also defined in gctp/cproj.h.  At this point,
 * they are basically the same, but GCTP defines HALF_PI as (PI * 0.5).
 */

#ifndef PI
#ifndef M_PI
#define PI (3.141592653589793238)
#else
#define PI (M_PI)
#endif
#endif

#ifndef TWO_PI
#define TWO_PI (2.0 * PI)
#endif
#ifndef HALF_PI
#define HALF_PI (PI / 2.0)
#endif

#define DEG (180.0 / PI)
#define RAD (PI / 180.0)

/* target platform-specific stuff */
#define MRT_DOS                 1       /* DJGPP */
#define MRT_WINDOWS             2       /* CYGWIN */
#define MRT_WIN98               3
#define MRT_WINNT               4
#define MRT_LINUX               5
#define MRT_SUN                 6
#define MRT_SOLARIS             7
#define MRT_SGI                 8
#define MRT_IRIX                9

#endif
