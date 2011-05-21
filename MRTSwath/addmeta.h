/*
!C****************************************************************************

!File: addmeta.h

!Description: Header file for addmeta.c - see addmeta.c for more information.

!Revision History:
 Revision 2.0 2004/01/09
 Gail Schmidt
 Original Version.

!END****************************************************************************
*/

#ifndef ADDMETA_H
#define ADDMETA_H

#include "hdf.h"
#include "mfhdf.h"
#include "param.h"

/* Prototypes */

bool AppendMetadata( Param_t *param, char *new_hdf_file, char *old_hdf_file,
     int proc_sds );
bool TransferAttributes( Param_t *param, int32 old_fid, int32 new_fid,
     int proc_sds );
bool TransferMetadata( int32 old_fid, int32 new_fid );
bool TransferAttr( int32 fid_old, int32 fid_new, char *attr );

#endif
