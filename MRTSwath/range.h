/*
!C****************************************************************************

!File: range.h

!Description: Header file for declaring range of HDF data types.

!Revision History:
 Revision 1.0 2001/05/08
 Robert Wolfe
 Original Version.

 Revision 1.5 2002/12/02
 Gail Schmidt
 Added support for INT8 data types.


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
      phone: 301-614-5508               Lanham, MD 20706  

!END****************************************************************************
*/

#ifndef RANGE_H
#define RANGE_H

/* possible ranges for data types */

#define RANGE_CHAR8H   (        127  )
#define RANGE_CHAR8L   (       -128  )
#define RANGE_UINT8H   (        255  )
#define RANGE_UINT8L   (          0  )
#define RANGE_INT8H    (        127  )
#define RANGE_INT8L    (       -128  )
#define RANGE_INT16H   (      32767  )
#define RANGE_INT16L   (     -32768  )
#define RANGE_UINT16H  (      65535u )
#define RANGE_UINT16L  (          0u )
#define RANGE_INT32H   ( 2147483647ll )
#define RANGE_INT32L   (-2147483647ll )
#define RANGE_UINT32H  ( 4294967295ul)
#define RANGE_UINT32L  (          0ul)
#define RANGE_FLOATH   (    1.0e+37  )
#define RANGE_FLOATL   (   -1.0e+37  )

#endif
