#include "const.h"
#include "output.h"
#include "myerror.h"

/******************************************************************************

MODULE:  RBWriteScanLine

PURPOSE:  Write a line of data to a raw binary file

RETURN VALUE:
Type = boolean
Value           Description
-----           -----------
true            Success
false           Failure

HISTORY:
Version  Date   Programmer       Code  Reason
-------  -----  ---------------  ----  -------------------------------------
         05/00  Rob Burrell            Original Development
         06/00  John Weiss             byte swapping for endian issues
         01/01  John Rishea            Standardized formatting
         01/01  John Rishea            Added check for memory allocation
         11/03  Gail Schmidt           Modified for use with MRTSwath
         01/04  Gail Schmidt           Don't do the byte swap, since we
                                       don't have to read the data back into
                                       MRTSwath

NOTES:

******************************************************************************/

bool RBWriteScanLine
(
    FILE *rbfile,             /* I: Raw binary file pointer */
    Output_t *output,         /* I: Output data information */
    int line_num,             /* I: Line number to be written */
    void *buf                 /* I: Buffer to be written */
)

{
    /* Check the parameters */
    if (!output->open)
        LOG_RETURN_ERROR("file not open", "RBWriteScanLine", false);
    if (line_num < 0  ||  line_num >= output->size.l)
        LOG_RETURN_ERROR("invalid line number", "RBWriteScanLine", false);

    /* write data as is for the current platform, no need to byte swap */
    if ( fwrite( buf, output->output_dt_size, output->size.s, rbfile )
        != (unsigned int) output->size.s )
    {
        /* oops, wrote wrong number of data items */
        LOG_RETURN_ERROR("wrote wrong number of data items", "RBWriteScanLine",
            false);
    }

    return ( true );
}


/******************************************************************************

MODULE:  RBWriteScanLineSwap

PURPOSE:  Write a line of data to a raw binary file

RETURN VALUE:
Type = boolean
Value           Description
-----           -----------
true            Success
false           Failure

HISTORY:
Version  Date   Programmer       Code  Reason
-------  -----  ---------------  ----  -------------------------------------
         05/00  Rob Burrell            Original Development
         06/00  John Weiss             byte swapping for endian issues
         01/01  John Rishea            Standardized formatting
         01/01  John Rishea            Added check for memory allocation 
         11/03  Gail Schmidt           Modified for use with MRTSwath

NOTES:
  1. Must handle byte order for multifile I/O. The raw binary data will
     always be written as big endian (even for Linux and Windows).

******************************************************************************/

bool RBWriteScanLineSwap
(
    FILE *rbfile,             /* I: Raw binary file pointer */
    Output_t *output,         /* I: Output data information */
    int line_num,             /* I: Line number to be written */
    void *buf                 /* I: Buffer to be written */
)

{
/*    int i;			 * index for byte swapping */
/*    int tmp;			 * temp value for byte swapping */
/*    unsigned char *val = NULL; * single byte for byte swapping */

    /* Check the parameters */
    if (!output->open)
        LOG_RETURN_ERROR("file not open", "RBWriteScanLine", false);
    if (line_num < 0  ||  line_num >= output->size.l)
        LOG_RETURN_ERROR("invalid line number", "RBWriteScanLine", false);

    /* NOTE: byte order is an issue for multifile I/O */
#if MRT_BYTE_ORDER == MRT_BIG_ENDIAN

    /* write big endian data, no need to byte swap */
    if ( fwrite( buf, output->output_dt_size, output->size.s, rbfile )
        != (size_t)output->size.s )
    {
        /* oops, wrote wrong number of data items */
        LOG_RETURN_ERROR("wrote wrong number of data items", "RBWriteScanLine",
            false);
    }

#elif MRT_BYTE_ORDER == MRT_LITTLE_ENDIAN

    /* have to byte swap int/float big endian data on little endian boxes */
    static unsigned char *bufswp = NULL;	/* pointer for swab */

    switch ( output->output_dt_size )
    {
            /* byte data? no need to byte swap */
        case 1:
            /* write row */
            if ( (int) fwrite( buf, output->output_dt_size, output->size.s,
                               rbfile ) != output->size.s )
            {
                /* oops, wrote wrong number of data items */
                LOG_RETURN_ERROR("wrote wrong number of data items",
                    "RBWriteScanLine", false);
            }
            break;

            /* 2-byte values: assume that swab is implemented efficiently */
        case 2:
            /* allocate memory for swap buffer */
            if ( bufswp == NULL )
                bufswp = calloc( output->size.s, output->size.s );

            /* if memory wasn't successfully allocated */
            if ( bufswp == NULL )
            {
                /* log the error */
                LOG_RETURN_ERROR("unable to allocate memory for byte swapping",
                    "RBWriteScanLine", false);
            }

            /* swap bytes: swab(src, dest, n) */
            swab( buf, bufswp, output->output_dt_size * output->size.s );

            /* copy back to row buffer: memcpy(dest, src, n) */
            memcpy( buf, bufswp, output->output_dt_size * output->size.s );

            /* write row */
            if ( (int) fwrite( buf, output->output_dt_size, output->size.s,
                               rbfile ) != output->size.s )
            {
                /* free allocated memory */
                free ( bufswp );

                /* oops, wrote wrong number of data items */
                LOG_RETURN_ERROR("wrote wrong number of data items",
                    "RBWriteScanLine", false);
            }

            /* free allocated memory */
            free ( bufswp );
            bufswp = NULL;

            break;

            /* 4-byte values: we better do this by hand (or use htonl() ?) */
        case 4:
            /* swap bytes */
            val = buf;
            for ( i = 0; i < output->size.s; i++, val += 4 )
            {
                tmp = val[0];
                val[0] = val[3];
                val[3] = tmp;
                tmp = val[1];
                val[1] = val[2];
                val[2] = tmp;
            }

            /* write row */
            if ( (int) fwrite( buf, output->output_dt_size, output->size.s,
                               rbfile ) != output->size.s )
            {
                /* oops, wrote wrong number of data items */
                LOG_RETURN_ERROR("wrote wrong number of data items",
                    "RBWriteScanLine", false);
            }
            break;
    }

#else

    LOG_RETURN_ERROR("cannot determine byte order", "RBWriteScanLine", false);

#endif

    return ( true );
}
