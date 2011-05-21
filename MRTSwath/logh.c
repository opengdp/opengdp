/******************************************************************************

FILE:  logh.c

PURPOSE:  Session logging to a file

HISTORY:
Version  Date   Programmer       Code  Reason
-------  -----  ---------------  ----  -------------------------------------
         04/04  Gail Schmidt           Original Development

HARDWARE AND/OR SOFTWARE LIMITATIONS:  
  None

PROJECT:    MODIS Swath Reprojection Tool

NOTES:

******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__CYGWIN__) || defined(WIN32)
#include <getopt.h>             /* getopt  prototype */
#else
#include <unistd.h>             /* getopt  prototype */
#endif
#ifdef WIN32
#include <io.h>                 /* _mktemp_s */
#endif
#include "logh.h"

/* NOTE:
**   templogname is a global variable used in this function and C_TRANS.C.
**   In C_TRANS.C it is used by the init() function which is found in
**   Modis/geolib/gctp/report.c
 */

char templogname[50];			/* temporary log file name */
static bool loginitialized = false;	/* init flag */

/* Create a global variable for the log filename. */
static char LOG_FILENAME[256] = "mrtswath.log";

/******************************************************************************

MODULE:  InitLogHandler

PURPOSE:  Create a temporary log file.

RETURN VALUE:
Type = boolean
Value           Description
-----           -----------
TRUE            Success
FALSE           Failure to write

HISTORY:
Version  Date   Programmer       Code  Reason
-------  -----  ---------------  ----  -------------------------------------
         04/04   Gail Schmidt           Original Development

NOTES:
  Creating a temporary log file and then appending it to the mrtswath
  log file. This should avoid the possibility that two processes might
  write to the log file at the same time.

******************************************************************************/
bool InitLogHandler (void)
{
    FILE *templog = NULL;    /* filename of the temporary log file */

    /* if it is already open */
    if ( loginitialized )
	return ( false );

    /* create a temporary name */
    strcpy(templogname, "tmpXXXXXX");
#ifdef WIN32
    if (_mktemp_s(templogname, strlen(templogname)+1) != 0)
#else
    if( mkstemp(templogname) == -1 )
#endif
    {
	fprintf( stdout, "Error: %s : %s\n", "InitLogHandler",
	    "cannot generate unique templogname with mkstemp()" );
        fflush( stdout );
	return ( false );
    }

    /* open a temporary log file in the current directory */
    templog = fopen( templogname, "w" );
    if ( !templog )
    {
	fprintf( stdout, "Error: %s : %s\n", "InitLogHandler",
	    ERRORMSG_LOGFILE_OPEN );
        fflush( stdout );
	return ( false );
    }

    /* let someone else open and close */
    fclose( templog );
    loginitialized = true;

    return ( true );
}

/******************************************************************************

MODULE:  CloseLogHandler

PURPOSE:  Copy the templog file to LOG_FILENAME

RETURN VALUE:
Type = boolean
Value           Description
-----           -----------
TRUE            Success
FALSE           Failure to write

HISTORY:
Version  Date   Programmer       Code  Reason
-------  -----  ---------------  ----  -------------------------------------
         04/04   Gail Schmidt           Original Development
 
NOTES:

******************************************************************************/
bool CloseLogHandler (void)
{
    FILE *templog = NULL,	/* file ptrs to input temp and output files */
         *logfile = NULL;
    char buffer[256];		/* string buffer for reading/writing */

    /* if no log initialized */
    if ( !loginitialized )
	return ( false );

    /* it is now closed */
    loginitialized = false;

    /* open resampler's log file */
    logfile = fopen( LOG_FILENAME, "a" );
    if ( !logfile )
    {
	fprintf( stdout, "Error: %s : %s\n", "CloseLogHandler",
	    ERRORMSG_LOGFILE_OPEN );
        fflush( stdout );
	return ( false );
    }

    /* open our temp log file */
    templog = fopen( templogname, "r" );
    if ( !templog )
    {
	fprintf( stdout, "Error: %s : %s\n", "CloseLogHandler",
	    ERRORMSG_LOGFILE_OPEN );
        fflush( stdout );
	fclose( logfile );
	return ( false );
    }

    /* append the contents of the temp log */
    while ( fgets( buffer, 254, templog ) )
	fputs( buffer, logfile );

    fclose( logfile );
    fclose( templog );
    remove( templogname );

    return ( true );
}

/******************************************************************************

MODULE:  LogHandler

PURPOSE:  Formats and prints messages to a logfile

RETURN VALUE:
Type = boolean
Value           Description
-----           -----------
TRUE            Success
FALSE           Failure to write

HISTORY:
Version  Date   Programmer       Code  Reason
-------  -----  ---------------  ----  -------------------------------------
         04/04   Gail Schmidt          Original Development

NOTES:

******************************************************************************/
bool LogHandler
(
    char *message	/* I:  message to be written to the log */
)

{
    FILE *logfile = NULL;

    logfile = fopen( templogname, "a" );
    if ( !logfile )
    {
	/*
	 * rather than create a potential loop by calling the ErrorHandler,
	 * just print a message and return
	 */

	fprintf( stdout, "Error: %s : %s\n", "LogHandler",
          ERRORMSG_LOGFILE_OPEN );
        fflush( stdout );
	return ( false );
    }

    fprintf( logfile, "%s", message );
    fclose( logfile );

    return ( true );
}
