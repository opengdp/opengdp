#if !defined(__CYGWIN__) && !defined(__APPLE__) && !defined(WIN32)
#include <values.h>
#endif
#include "myproj.h"
#include "myerror.h"
#include "deg2dms.h"

/******************************************************************************

MODULE:  Deg2DMS

PURPOSE:  Convert parameters from decimal degrees to DMS

RETURN VALUE:
Type = int
Value           Description
-----           -----------
TRUE            success
FALSE           failure

HISTORY:
Version  Date   Programmer       Code  Reason
-------  -----  ---------------  ----  -------------------------------------
         08/02  Gail Schmidt           Original Development

NOTES:

******************************************************************************/
int Deg2DMS
( 
    int projection_type,             /* I: see myproj_const.h for a list
                                            of projection numbers */
    double *projection_parameters    /* I/O: projection parameters */
)
{
    int status = false;              /* error status */
    double temp1, temp2, temp3, temp4, temp5;
                                     /* local temp storage for conversion */

    switch ( projection_type )
    {
        case PROJ_UTM:
            if ( degdms( &projection_parameters[0], &temp1, "DEG", "LON" )
                     != ERR_RESP &&
                 degdms( &projection_parameters[1], &temp2, "DEG", "LAT" )
                     != ERR_RESP )
            {
                projection_parameters[0] = temp1;
                projection_parameters[1] = temp2;
                status = true;
            }
            break;

        case PROJ_GEO:
        case PROJ_SPCS:
        case PROJ_IMOLL:
        case PROJ_ALASKA:
        case PROJ_GOODE:
            status = true;
            /* nothing to do */
            break;

        case PROJ_ISINUS:
        case PROJ_HAMMER:
        case PROJ_MOLL:
        case PROJ_SNSOID:
        case PROJ_MILLER:
        case PROJ_ROBIN:
        case PROJ_WAGIV:
        case PROJ_WAGVII:
            if ( degdms( &projection_parameters[4], &temp1, "DEG", "LON" )
                    != ERR_RESP )
            {
                projection_parameters[4] = temp1;
                status = true;
            }
            break;

        case PROJ_PS:
        case PROJ_LAMAZ:
        case PROJ_STEREO:
        case PROJ_MERCAT:
        case PROJ_POLYC:
        case PROJ_AZMEQD:
        case PROJ_GNOMON:
        case PROJ_ORTHO:
        case PROJ_GVNSP:
        case PROJ_EQRECT:
        case PROJ_VGRINT:
            if ( degdms( &projection_parameters[4], &temp1, "DEG", "LON" )
                    != ERR_RESP &&
                 degdms( &projection_parameters[5], &temp2, "DEG", "LAT" )
                    != ERR_RESP )
            {
                projection_parameters[4] = temp1;
                projection_parameters[5] = temp2;
                status = true;
            }
            break;

        case PROJ_ALBERS:
        case PROJ_LAMCC:
            if ( degdms( &projection_parameters[2], &temp1, "DEG", "LAT" )
                    != ERR_RESP &&
                 degdms( &projection_parameters[3], &temp2, "DEG", "LAT" )
                    != ERR_RESP &&
                 degdms( &projection_parameters[4], &temp3, "DEG", "LON" )
                    != ERR_RESP &&
                 degdms( &projection_parameters[5], &temp4, "DEG", "LAT" )
                    != ERR_RESP )
            {
                projection_parameters[2] = temp1;
                projection_parameters[3] = temp2;
                projection_parameters[4] = temp3;
                projection_parameters[5] = temp4;
                status = true;
            }
            break;

        case PROJ_TM:
            if ( degdms( &projection_parameters[4], &temp1, "DEG", "LON" )
                    != ERR_RESP &&
                 degdms( &projection_parameters[5], &temp2, "DEG", "LAT" )
                    != ERR_RESP )
            {
                projection_parameters[4] = temp1;
                projection_parameters[5] = temp2;
                status = true;
            }
            break;

        case PROJ_EQUIDC: /* Equidistant Conic A */
            if ( degdms( &projection_parameters[2], &temp1, "DEG", "LAT" )
                    != ERR_RESP &&
                 degdms( &projection_parameters[4], &temp2, "DEG", "LON" )
                    != ERR_RESP &&
                 degdms( &projection_parameters[5], &temp3, "DEG", "LAT" )
                    != ERR_RESP )
            {
                projection_parameters[2] = temp1;
                projection_parameters[4] = temp2;
                projection_parameters[5] = temp3;
                status = true;
            }
            break;

        case PROJ_HOM: /* Hotin Oblique Mercator A */
            if ( degdms( &projection_parameters[5], &temp1, "DEG", "LAT" )
                    != ERR_RESP &&
                 degdms( &projection_parameters[8], &temp2, "DEG", "LON" )
                    != ERR_RESP &&
                 degdms( &projection_parameters[9], &temp3, "DEG", "LAT" )
                    != ERR_RESP &&
                 degdms( &projection_parameters[10], &temp4, "DEG", "LON" )
                    != ERR_RESP &&
                 degdms( &projection_parameters[11], &temp5, "DEG", "LAT" )
                    != ERR_RESP )
            {
                projection_parameters[5] = temp1;
                projection_parameters[8] = temp2;
                projection_parameters[9] = temp3;
                projection_parameters[10] = temp4;
                projection_parameters[11] = temp5;
                status = true;
            }
            break;

        case PROJ_SOM: /* SOM A */
            if ( degdms( &projection_parameters[3], &temp1, "DEG", "LAT" )
                    != ERR_RESP &&
                 degdms( &projection_parameters[4], &temp2, "DEG", "LON" )
                    != ERR_RESP )
            {
                projection_parameters[3] = temp1;
                projection_parameters[4] = temp2;
                status = true;
            }
            break;

        case PROJ_OBEQA:
            if ( degdms( &projection_parameters[4], &temp1, "DEG", "LON" )
                    != ERR_RESP &&
                 degdms( &projection_parameters[5], &temp2, "DEG", "LAT" )
                    != ERR_RESP &&
                 degdms( &projection_parameters[8], &temp3, "DEG", "LAT" )
                    != ERR_RESP )
            {
                projection_parameters[4] = temp1;
                projection_parameters[5] = temp2;
                projection_parameters[8] = temp3;
                status = true;
            }
            break;

        default:
            LOG_RETURN_ERROR("bad projection type", "Deg2DMS", false);
    }

    return (status);
}
