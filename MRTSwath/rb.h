/*
!C****************************************************************************

!File: rb.h

!Description: Header file for raw binary .c files (write_rb.c, write_hdr.c)

!Revision History:
 Revision 2.0 2003/11/13
 Gail Schmidt
 Original version.

!Team Unique Header:

 ! References and Credits:

  ! Developers:
      Gail Schmidt
      SAIC - USGS EROS Data Center

!END*****************************************************************************/

#ifndef RB_H
#define RB_H

int WriteHeaderFile (Param_t *ParamList, Patches_t *PatchesList);
bool RBWriteScanLine (FILE *rbfile, Output_t *output, int line_num, void *buf);

#endif
