/*
!C****************************************************************************

!File: mystring.c
  
!Description: Functions for handling strings.

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
   1. The following functions handle strings:

       DupString - Duplicate a string.

!END****************************************************************************
*/

#include <stdlib.h>
#include <string.h>
#include "myerror.h"

char *DupString(char *string) 
/* 
!C******************************************************************************

!Description: 'DupString' duplicates a string.
 
!Input Parameters:
 string         string to duplicate

!Output Parameters:
 (returns)      duplicated string or 
                NULL when an error occurs

!Team Unique Header:

 ! Design Notes:
   1. An error status is returned when:
       a. a null string pointer is input
       b. the input string length is invalid (< 0)
       c. there is an erorr allocating memory for the string
       d. there is an error copying the string.
   2. Memory is allocated for the duplicated string.
   3. Error messages are handled with the 'LOG_RETURN_ERROR' macro.

!END****************************************************************************
*/
{
  int len;
  char *s;

  if (string == (char *)NULL) return ((char *)NULL);
  len = strlen(string);
  if (len < 0) 
    LOG_RETURN_ERROR("invalid string length", "DupString", (char *)NULL);

  s = (char *)calloc((len + 1), sizeof(char));
  if (s == (char *)NULL) 
    LOG_RETURN_ERROR("allocating memory", "DupString", (char *)NULL);
  if (strcpy(s, string) != s) {
    free(s);
    LOG_RETURN_ERROR("copying string", "DupString", (char *)NULL);
  }

  return (s);
}
