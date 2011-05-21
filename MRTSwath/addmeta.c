/************************************************************************

FILE: addmeta.c

PURPOSE:  Attach metadata from input hdf file to output hdf file.

HISTORY:
Version    Date     Programmer      Code     Reason
-------    ----     ----------      ----     ------
1.0        UK       Doug Ilg                 Original Development 
           01/04    Gail Schmidt             Modified for use with MRTSwath

HARDWARE AND/OR SOFTWARE LIMITATIONS:
    None

PROJECT: MRTSwath

NOTES:   This program is an adaptation of metadmp.c, developed by Doug
Ilg (Doug.Ilg@gsfc.nasa.gov).

*************************************************************************/
#include "addmeta.h"
#include "myerror.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*************************************************************************

MODULE: AppendMetadata

PURPOSE: 
    This module allows user-defined core, archive, and structural
    metadata from the original (input) HDF file to be appended to the new
    (output) HDF file.  Specifically, the metadata from the old file is
    first extracted and then added to the new file as a string of characters
    assigned to a file-level attribute tag.  The user-defined metadata fields
    are assigned names of "OldCoreMetadata", "OldArchiveMetadata", and
    "OldStructMetadata." These metadata fields can be extracted using the
    SDfindattr and SDattrinfo commands from the HDF library.
 

RETURN VALUE:
Type = bool
Value    	Description
-----		-----------
true            Successful completion
false           Error processing

HISTORY:
Version    Date     Programmer      Code     Reason
-------    ----     ----------      ----     ------
           01/04    Gail Schmidt             Original Development based on
                                             Doug Ilg's metadmp.c program.

NOTES:

**************************************************************************/

bool AppendMetadata
(
    Param_t *param,         /* I: session parameter information */
    char *new_hdf_file,     /* I: HDF file we want to append metadata to */
    char *old_hdf_file,     /* I: HDF file we want to get metadata from */
    int proc_sds            /* I: current SDS being processed, for resolution
                                  info */
)

{
    intn trans_status; /* used to capture return val frm subfunct */
    int32 old_sd_id;   /* holds the id# for the old HDF file */
    int32 new_sd_id;   /* holds the id# for the new HDF file */ 

    /* Open the new HDF file for writing */
    new_sd_id = SDstart( new_hdf_file, DFACC_WRITE );
    if ( new_sd_id == -1 )
    {
        /* NOTE: We won't flag this as an error, since in some cases the
           resolution file may not exist.  For example, if a MOD02HKM is
           specified and the Latitude or Longitude data is specified, then
           that data is at a different resolution (1000m) than the rest of
           the image SDS data (500m).  The software will think that there
           should be a 1000m product, however the Latitude and Longitude
           data didn't actually get processed .... since it is float data.
           So, AppendMetadata will flag an error since that HDF file will
           not really exist. */
        return false;
/*        LOG_RETURN_ERROR( "unable to open new HDF file", "AppendMetadata",
                            false);
*/
    }

    /* Open the old HDF file for reading */
    old_sd_id = SDstart( old_hdf_file, DFACC_READ );
    if ( old_sd_id == -1 )
    {
        SDend( new_sd_id );
        LOG_RETURN_ERROR("unable to open old HDF file","AppendMetadata",false);
    }

    /* transfer attributes from old HDF file to new HDF file, only for the
       SDSs of the same resolution as the current SDS */
    trans_status = TransferAttributes( param, old_sd_id, new_sd_id, proc_sds );
    if ( trans_status == -1 )
        LOG_WARNING( "unable to transfer attributes from old HDF file to new "
                 "HDF file", "AppendMetadata" );

    /* transfer metadata from old HDF file to new HDF file */
    trans_status = TransferMetadata( old_sd_id, new_sd_id );
    if ( trans_status == -1 )
        LOG_WARNING( "unable to append metadata from old HDF file to new "
                 "HDF file", "AppendMetadata" );

    /* close the HDF files */
    SDend( old_sd_id);
    SDend( new_sd_id );

    /* return successful value */
    return( true );
}

/*************************************************************************

Module: TransferAttributes

Author: GLS 01/09/04  Original development

Transfer HDF SDS attributes from input to output, pretty much intact.
Global attributes are handled elsewhere.

*************************************************************************/

bool TransferAttributes( Param_t *param, int32 old_fid, int32 new_fid,
  int proc_sds )
{
    int j = 0;
    int curr_band, curr_sds;
    int32 old_sds, new_sds;
    int32 sds_index;
    int32 nattr, attr_index;
    int32 data_type, n_values;
    char attr_name[1024], sds_name[1024], *buffer = NULL;
    int32 rank;
    int32 dims[10];

    /* loop through the remaining SDS's and add them only if their output
       resolution is the same as the current SDSs resolution */
    for ( curr_sds = proc_sds; curr_sds < param->num_input_sds; curr_sds++ )
    {
        /* if this SDS is not the same resolution as proc_sds, then do not
           output metadata attributes for the SDS */
        if ( param->output_pixel_size[curr_sds] !=
             param->output_pixel_size[proc_sds] )
            continue;

        /* get the SDS index */
        sds_index = SDnametoindex( old_fid,
            param->input_sds_name_list[curr_sds] );

        /* open old SDS */
        old_sds = SDselect( old_fid, sds_index );

        /* get various SDS info */
        SDgetinfo( old_sds, sds_name, &rank, dims, &data_type, &nattr );

        /* loop through the bands in this SDS, writing the same info
           to each band that was processed */
        for ( curr_band = 0; curr_band < param->input_sds_nbands[curr_sds];
              curr_band++ )
        {
            /* if this band wasn't processed, skip to the next band */
            if (param->input_sds_bands[curr_sds][curr_band] == 0)
                continue;

            /* open new SDS, for this SDS/band combination */
            new_sds = SDselect( new_fid, j++ );

            /* for each SDS attribute */
            for ( attr_index = 0; attr_index < nattr; attr_index++ )
            {
                /* get SDS attribute info */
                SDattrinfo( old_sds, attr_index, attr_name, &data_type,
                    &n_values );

                /* allocate memory */
                switch (data_type)
                {
                    case 6:
                    case 26:
                    case 27:
                        buffer = calloc(n_values, 8);
                        break;

                    case 5:
                    case 24:
                    case 25:
                        buffer = calloc(n_values, 4);
                        break;

                    case 22:
                    case 23:
                        buffer = calloc(n_values, 2);
                        break;

                    default:
                        buffer = calloc(n_values, 1);
                        break;
                }

                /* make sure the buffer was allocated */
                if ( buffer == NULL )
                    LOG_RETURN_ERROR( "unable to allocate space for buffer",
                        "TransferAttributes", false);

                /* get SDS attribute values */
                if ( SDreadattr( old_sds, attr_index, buffer ) == -1 )
                    LOG_RETURN_ERROR("unable to find attribute in old HDF file",
                        "TransferAttributes", false);

                /* write SDS attribute to output file */
                if (SDsetattr( new_sds, attr_name, data_type, n_values,
                    buffer ) == -1 )
                    LOG_RETURN_ERROR( "unable to write attribute to new HDF "
                        "file", "TransferAttributes", false);
            } /* for attr_index */

            /* free memory */
            free(buffer);

            /* close current SDS */
            SDendaccess( new_sds );
        } /* for curr_band */

        /* close current SDS */
        SDendaccess( old_sds );
    } /* for curr_sds */

    return true;
}

/*************************************************************************

MODULE: TransferMetadata

PURPOSE: 
    This module sends requests to TransferAttr to append user-defined core,
    archive, and structural metadata to the new HDF file from the old HDF
    file.
 

RETURN VALUE:
Type = bool
Value    	Description
-----		-----------
true            Successful completion
false           Error processing

HISTORY:
Version    Date     Programmer      Code     Reason
-------    ----     ----------      ----     ------
           02/01    John Rishea              Original Development based on
                                             Doug Ilg's metadmp.c program.

NOTES:

**************************************************************************/

bool TransferMetadata
(
    int32 old_fid,	/* I: SDS file id # for the old HDF file */ 
    int32 new_fid	/* I: SDS file id # for the new HDF file */ 
)

{
    intn i;                /* used to loop through attrs */
    intn j;
    bool status;           /* receives result of processing
                              from transferattr function */     
    char attrname[256];    /* holds the file_name string */ 
    char *root = NULL;     /* will point to attr type */

    /* set up a loop to check for and transfer the three
       types of metadata -- structural, core, archive */
    for ( i = 0; i < 3; i++ )
    {
        /* set up root to hold correct string */ 
        switch( i )
        {
            case 0:
                root = "StructMetadata";
                break;

            case 1:
                root = "CoreMetadata";
                break;

            case 2:
                root = "ArchiveMetadata";
                break;
        }  

        /* First, try root name alone */
        status = TransferAttr( old_fid, new_fid, root );
        if ( !status )
        {   /* error occurred in TransferAttr call */
            /* try concatenating sequence numbers (0-9) */
            for ( j = 0; j <= 9; j++ )
            {
                sprintf( attrname, "%s.%d", root, j );
                status = TransferAttr( old_fid, new_fid, attrname );
                if ( status )
                {
                    /* successful return from TransferAttr */
                    break;
                }
            }
        }
    }

    return ( true );
}


/*************************************************************************

MODULE: TransferAttr

PURPOSE: 
    This module reads the user-defined core, archive, and structural metadata
    from the original (input) HDF file and writes it as new user-defined
    metadata in the new HDF file.  The new file-level attribute namefields in
    the output HDF file are of the form "Old+<original field name>" and can 
    be extracted from the output HDF file using the SDfindattr and SDattrinfo
    commands from the HDf library.

RETURN VALUE:
Type = bool
Value    	Description
-----		-----------
true            Successful completion
false           Error processing

HISTORY:
Version    Date     Programmer      Code     Reason
-------    ----     ----------      ----     ------
           01/04    Gail Schmidt             Original Development based on
                                             Doug Ilg's metadmp.c program.

NOTES:

**************************************************************************/
bool TransferAttr
(
    int32 fid_old,	/* I: SDS id # for the old HDF file */ 
    int32 fid_new,	/* I: SDS id # for the new HDF file */ 
    char *attr		/* I: Filename handle of the attribute to move */ 
)

{
    intn status;            /* this is the var that holds return val */
    int32 my_attr_index;    /* holds return val from SDfindattr */ 
    int32 data_type;	    /* holds attribute's data type */ 
    int32 n_values;         /* stores # of vals of the attribute */
    char *file_data = NULL;         /* char ptr used to allocate temp space
                                       during transfer of attribute info */
    char attr_name[MAX_NC_NAME];    /* holds attribute's name */
    char new_attr_name[MAX_NC_NAME + 3];    /* holds new attr name */

    /* look for attribute in the old HDF file */
    my_attr_index = SDfindattr( fid_old, attr );

    /* only proceed if the attribute was found */
    if ( my_attr_index == -1 )
        return( false );
 
    /* get size of old HDF file attribute  */
    status = SDattrinfo( fid_old, my_attr_index, attr_name, &data_type,
        &n_values );

    /* only proceed if successful read of attribute info */
    if ( status == -1 )
        return( false );

    /* attempt to allocate memory for old HDF file attribute contents (add
       one character for the end of string character) */
    file_data = ( char * ) calloc( n_values+1, sizeof(char) );
    if ( file_data == NULL )
        LOG_RETURN_ERROR( "unable to allocate memory for reading old HDF file "
            "metadata", "TransferAttr", false);

    /* read attribute from the old HDF file */
    status = SDreadattr( fid_old, my_attr_index, file_data );
    if(status == -1 )
    {
        free( file_data );
        return( false );
    }

    /* modify the attribute name from old HDF file prior to appending
       metadata to new HDF file; put prefix "Old" in front of old name */
    strcpy( new_attr_name, "Old" );
    strcat( new_attr_name, attr_name );
    new_attr_name[ strlen(new_attr_name) + 1 ] = '\0';

    /* attempt to write old metadata to output HDF file */
    status = SDsetattr( fid_new, new_attr_name, DFNT_CHAR8, strlen(file_data),
        (VOIDP)file_data );
 
    /* free dynamically allocated memory */
    free( file_data );

    return ( true );
}
