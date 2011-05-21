/*
!C****************************************************************************

!File: error.c
  
!Description: Function for handling errors and informational messages.

!Revision History:
 Revision 1.0 2001/05/08
 Robert Wolfe
 Original Version.

 Revision 2.0 2004/04/22
 Gail Schmidt
 Modified to log errors to a log file.
 Added a routine for informational messages.

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
   1. See 'myerror.h' for information on the 'LOG_ERROR' and 'LOG_ERROR_RETURN'
      macros that automatically populate the source code file name, line number
      and exit flag.  

!END****************************************************************************
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "myerror.h"

void LogError(const char *message, const char *module, 
           const char *source, long line, bool done)
/* 
!C******************************************************************************

!Description: 'Error' writes an error message to 'stdout' and to the log file,
  and optionally exits the program with a 'EXIT_FAILURE' status.
 
!Input Parameters:
 message        error message
 module         calling module name
 source         source code file name containing the calling module
 line           line number in the source code file
 done           flag indicating if program is to exit with a failure status;
                'true' = exit, 'false' = return

!Output Parameters: (none)
 (returns)      (void)

!Team Unique Header:

 ! Design Notes:
   1. If the 'errno' flag is set, the 'perror' function is first called to 
      print any i/o related errors.
  
   2. The error message is written to 'stdout'.

   3. The module name, source code name and line number are included in the 
      error message.

!END****************************************************************************
*/
{
  char errmsg[M_ERRMSG_LEN+1];   /* error message string */

  if (errno) perror(" i/o error ");
  
  /* Put together the error message */
  sprintf(errmsg, "error: [%s, %s:%ld] : %s\n", module, source, line, message);

  /* Print the message to stdout */
  fprintf(stdout, " %s", errmsg);
  fflush(stdout);

  /* Dump the message to a log file */
  LogHandler (errmsg);

  /* Terminate if specified by the calling routine */
  if (done)
  {
    fprintf (stdout, "Terminating application ...\n");
    CloseLogHandler();
    exit (EXIT_FAILURE);
  }
  else
  {
    return;
  }
}

void LogInfomsg(const char *message)
/* 
!C******************************************************************************

!Description: 'Infomsg' writes an informational message to 'stdout' and to
  the logfile, and optionally exits the program with a 'EXIT_FAILURE' status.
 
!Input Parameters:
 message        informational message

!Output Parameters: (none)
 (returns)      (void)

!Team Unique Header:

! Design Notes:

!END****************************************************************************
*/
{
  char msg[M_MSG_LEN+1];

  /* Print the message to stdout */
  fprintf(stdout, "%s", message);
  fflush(stdout);

  /* Dump the message to a log file */
  strcpy (msg, message);
  LogHandler (msg);

  return;
}
