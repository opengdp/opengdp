/*
!C****************************************************************************

!File: myerror.h
  
!Description: Header file for error.c - see error.c for more information. 

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
   1. These macros automatically generates the source file name, line number 
      and return status parameters for the 'Error' function.

        'LOG_ERROR' exits with a fatal error
        'LOG_ERROR_RETURN' returns to the calling program with a status of
	               'status'

!END****************************************************************************
*/

#ifndef MRTSWATH_ERROR_H
#define MRTSWATH_ERROR_H

#include "logh.h"

#define ERR_RESP -1
#define OK_RESP   0

#define LOG_ERROR(message, module) \
  LogError((message), (module), (__FILE__), (long)(__LINE__), true)

#define LOG_WARNING(message, module) \
  LogError((message), (module), (__FILE__), (long)(__LINE__), false)


#define LOG_RETURN_ERROR(message, module, status) \
   {LogError((message), (module), (__FILE__), (long)(__LINE__), false); \
   return (status);}


void LogError(const char *message, const char *module, 
              const char *source, long line, bool done);

void LogInfomsg(const char *message);


#endif
