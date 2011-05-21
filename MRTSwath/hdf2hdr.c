/*
!C****************************************************************************

!File: hdf2hdr.c
  
!Description: Writes HDF info to a header format readable by the MRTSwath GUI.

!Revision History:
 Revision 1.0 2004/03/19
 Gail Schmidt
 Original Version.

!Team Unique Header:
  This software was developed by the USGS EROS Data Center DAAC Team.

 ! References and Credits:
  ! Developers:
      Gail Schmidt
      EROS Data Center DAAC             SAIC
      MODIS Reprojection Tool           501 East St. Joesph Street
      gschmidt@usgs.gov                 Rapid City, SD 57701
  
 ! Design Notes:

!END****************************************************************************
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gui_hdr.h"
#include "myisoc.h"

int main (int argc, const char **argv) 
{
    char *hdfname = NULL;          /* input hdf filename */
    char *hdrname = "TmpHdr.hdr";  /* output hdr filename */
    SwathDescriptor desc;          /* structure holding the HDF swath info */

    /* Check the arguments */
    if (argc != 2)
    {
        printf ("Usage: %s hdfname\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* Get the input HDF filename from the command line */
    hdfname = strdup (argv[1]);

    /* Read the input HDF file */
    if (ReadHDFFile (hdfname, &desc) != SUCCESS)
    {
        printf ("%s: Error reading the HDF file, %s\n", argv[0], hdfname);
        return EXIT_FAILURE;
    }
    free (hdfname);

    /* Write the HDF information to the HDR file */
    if (WriteHDRFile (hdrname, &desc) != SUCCESS)
    {
        printf ("%s: Error writing the HDR file information to %s\n", argv[0],
            hdrname);
        return EXIT_FAILURE;
    }

    /* Print the HDR name to stdout for the GUI to grab */
    printf ("%s\n", hdrname);

    return EXIT_SUCCESS;
}
