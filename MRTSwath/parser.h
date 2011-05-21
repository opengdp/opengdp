/*
!C****************************************************************************

!File: parser.h

!Description: Header file for 'parser.c' - see 'param.c' for more information.

!Revision History:
 Revision 1.0 2000/12/13
 Sadashiva Devadiga
 Original version.

 Revision 1.1 2001/05/08
 Sadashiva Devadiga
 Cleanup and .

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

      Sadashiva Devadiga (Code 922)
      MODIS Land Team Support Group     SSAI
      devadiga@ltpmail.gsfc.nasa.gov    5900 Princess Garden Pkway, #300
      phone: 301-614-5549               Lanham, MD 20706

!END****************************************************************************
*/

#ifndef PARSER_H
#define PARSER_H

#include "param.h"

/* Constants */

#define MAX_NUM_PARAM (20)	/* maximum number of parameters in a list */
#define MAX_STR_LEN (255)	/* maximum length of string parameters */
#define MAX_SDS_STR_LEN (5000)	/* maximum length of SDS string parameters */

/* Prototypes */

bool ReadCmdLine(int argc, const char **argv, Param_t *this);
bool NeedHelp(int argc, const char **argv);
bool update_sds_info(int sdsnum, Param_t *this);

#endif
