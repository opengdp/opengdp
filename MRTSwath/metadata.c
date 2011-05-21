/************************************************************************

FILE: metadata.c

PURPOSE:  Attach metadata to the output hdf file.

HISTORY:
Version    Date     Programmer      Code     Reason
-------    ----     ----------      ----     ------
1.5        08/02    Gail Schmidt             Original Development 

*************************************************************************/

/*************************************************************************

!Description: 'WriteMeta' will write metadata information to the HDF file.
  The following values will be written as Global Attributes to the output
  HDF file:
  UL corner in output projection coordinates
  LR corner in output projection coordinates
  Number of lines, samples in the output image
  Pixel size of the output image
  Output projection type
  Output projection parameters
  Output sphere (semi-major and semi-minor axis)
  Output zone

!Input Parameters:
 output_filename   name of the output HDF file
 output_space_def  output grid space definition; the following fields are
                   input: pixel_size, ul_corner.x, ul_corner.y, lr_corner.x,
                   lr_corner.y, img_size.l, img_size.s, proj_num, zone,
                   sphere, proj_param[*]

!Output Parameters:
( returns)      an integer value (true, false) specifying no error or an error

HISTORY:
Version    Date     Programmer      Code     Reason
-------    ----     ----------      ----     ------
1.5        08/02    Gail Schmidt             Original Development

NOTES:

**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mfhdf.h"
#include "space.h"
#include "myproj.h"
#include "const.h"
#include "myerror.h"

/* External arrays */
extern Proj_type_t Proj_type[PROJ_NPROJ];

/************************************************************************/

int WriteMeta (char *output_filename, Space_def_t *output_space_def)
{
    int i;                  /* looping variable */
    intn status;            /* used to capture return values */
    int32 sd_id;            /* holds the id# for the output HDF file */ 
    float32 corner[2];      /* holds the corner point */
    float32 pixel_size;     /* holds the corner pixel size */
    int32 img_size[2];      /* holds the #line and #sample */
    int32 proj_zone;        /* holds the projection zone */
    int32 sphere;           /* holds the GCTP sphere number */
    char8 proj_name[256];   /* holds the projection name */
    float32 proj_parms[NPROJ_PARAM];
                            /* holds projection parameters */
    char msg[M_MSG_LEN+1];

    /* Open the output HDF file for writing */
    sd_id = SDstart (output_filename, DFACC_WRITE);
    if (sd_id == -1)
    {
	sprintf (msg, "Error: unable to open file %s \n", output_filename);
	LogInfomsg (msg);
	return (false);
    }

    /* UL corner in output projection coordinates */
    if (output_space_def->proj_num == PROJ_GEO)
    {
        /* If this is Geographic then coords are in radians. Convert to
           degrees before output. */
        corner[0] = (float32)(output_space_def->ul_corner.x * DEG);
        corner[1] = (float32)(output_space_def->ul_corner.y * DEG);
    }
    else
    {
        corner[0] = (float32) output_space_def->ul_corner.x;
        corner[1] = (float32) output_space_def->ul_corner.y;
    }
    status = SDsetattr (sd_id, "PROJ_UL_XY", DFNT_FLOAT32, 2, corner);
    if (status == -1)
    {
        sprintf (msg, "error outputting UL corner attribute\n");
	LogInfomsg (msg);
        return (false);
    }

    /* LR corner in output projection coordinates */
    if (output_space_def->proj_num == PROJ_GEO)
    {
        /* If this is Geographic then coords are in radians. Convert to
           degrees before output. */
        corner[0] = (float32)(output_space_def->lr_corner.x * DEG);
        corner[1] = (float32)(output_space_def->lr_corner.y * DEG);
    }
    else
    {
        corner[0] = (float32) output_space_def->lr_corner.x;
        corner[1] = (float32) output_space_def->lr_corner.y;
    }
    status = SDsetattr (sd_id, "PROJ_LR_XY", DFNT_FLOAT32, 2, corner);
    if (status == -1)
    {
        sprintf (msg, "error outputting LR corner attribute\n");
	LogInfomsg (msg);
        return (false);
    }

    /* Number of lines, samples in the output image */
    img_size[0] = (int32) output_space_def->img_size.l;
    img_size[1] = (int32) output_space_def->img_size.s;
    status = SDsetattr (sd_id, "IMAGE_SIZE_LS", DFNT_INT32, 2,
        img_size);
    if (status == -1)
    {
        sprintf (msg, "error outputting image size attribute\n");
	LogInfomsg (msg);
        return (false);
    }
    
    /* Pixel size of the output image (for Geographic output in degrees,
       internally this is stored as radians) */
    if (output_space_def->proj_num == PROJ_GEO)
        pixel_size = (float32)(output_space_def->pixel_size * DEG);
    else
        pixel_size = (float32) output_space_def->pixel_size;
    status = SDsetattr (sd_id, "PIXEL_SIZE", DFNT_FLOAT32, 1, &pixel_size);
    if (status == -1)
    {
        sprintf (msg, "error outputting pixel size attribute\n");
	LogInfomsg (msg);
        return (false);
    }

    /* Output projection type */
    strcpy ((char *) proj_name,
        Proj_type[output_space_def->proj_num].short_name);
    status = SDsetattr (sd_id, "PROJ_TYPE", DFNT_CHAR8,
        strlen ((char *) proj_name), proj_name);
    if (status == -1)
    {
        sprintf (msg, "error outputting projection name attribute\n");
	LogInfomsg (msg);
        return (false);
    }

    /* Output projection parameters */
    for (i = 0; i < NPROJ_PARAM; i++)
        proj_parms[i] = (float32) output_space_def->proj_param[i];
    status = SDsetattr (sd_id, "PROJ_PARAMS", DFNT_FLOAT32,
        NPROJ_PARAM, proj_parms);
    if (status == -1)
    {
        sprintf (msg, "error outputting projection parameters attribute\n");
	LogInfomsg (msg);
        return (false);
    }

    /* Output GCTP sphere number */
    sphere = (int32) output_space_def->sphere;
    status = SDsetattr (sd_id, "GCTP_SPHERE", DFNT_INT32, 1, &sphere);
    if (status == -1)
    {
        sprintf (msg, "error outputting GCTP sphere attribute\n");
	LogInfomsg (msg);
        return (false);
    }

    /* Output zone (only if this is UTM or State Plane) */
    if (output_space_def->zone_set &&
        (output_space_def->proj_num == PROJ_UTM ||
         output_space_def->proj_num == PROJ_SPCS))
    {
        proj_zone = (int32) output_space_def->zone;
        status = SDsetattr (sd_id, "PROJ_ZONE", DFNT_INT32, 1, &proj_zone);
        if (status == -1)
        {
            sprintf (msg, "error outputting projection zone attribute\n");
	    LogInfomsg (msg);
            return (false);
        }
    }

    /* close the output HDF file */
    status = SDend (sd_id);

    /* return the status value */
    return (true);
}
