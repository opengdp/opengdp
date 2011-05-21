#include <ctype.h>
#include "patches.h"
#include "param.h"
#include "myerror.h"
#include "myendian.h"
#include "geo_space.h"
#include "const.h"
#include "myproj.h"

/* External arrays */

extern Proj_sphere_t Proj_sphere[PROJ_NSPHERE];
extern Proj_type_t Proj_type[PROJ_NPROJ];

/******************************************************************************

MODULE:  WriteHeaderFile

PURPOSE:  Write a raw binary header

RETURN VALUE:
Type = int
Value           Description
-----           -----------
(returns)       Status:
                  'true' = okay
                  'false' = error writing the raw binary header

HISTORY:
Version  Date   Programmer       Code  Reason
-------  -----  ---------------  ----  -------------------------------------
         05/00  John Weiss             Original Development
         06/00  Rob Burrell            Output info from ModisDescriptor
         06/00  Rob Burrell            ParamsDMS2Deg fix
         07/00  Rob Burrell            Error return values
         01/01  John Rishea            Standardized formatting
         02/01  John Weiss             Add UTM zone to header
         04/01  Rob Burrell            Added TM projection
         07/01  Gail Schmidt           UTM_ZONE and ELLIPSOID_CODE no longer
                                       are commented fields
         12/01  Gail Schmidt           ELLIPSOID_CODE is now DATUM for all
                                       projections
         07/03  Gail Schmidt           Added Equirectangular projection
         11/03  Gail Schmidt           Modified for use with MRTSwath --
                                       no datum conversions
         04/04  Gail Schmidt           Switched back to ELLIPSOID_CODE instead
                                       of DATUM since either a product has a
                                       datum or an ellipsoid, but not both

NOTES:

******************************************************************************/
int WriteHeaderFile
(
    Param_t *ParamList,      /* I: parameter information */
    Patches_t *PatchesList   /* I: patches information (fill value) */
)

{
    FILE *fpo = NULL;		/* output file pointer */
    char filename[1024],        /* name of header to be written to */
         sdsname[1024],         /* temporary sdsname */
         str[1024],             /* temporary string for writing */
         endianstr[256];        /* temporary string for writing endian */
    int i;   		        /* index */
    MrtSwathEndianness machineEndianness = GetMachineEndianness();


	/*******************************************************************/

    /* add the SDS band name and hdr extension to the filename */
    strcpy( sdsname, ParamList->output_sds_name );

    /* remove any spaces (from the SDS name) from the name */
    i = 0;
    while ( sdsname[i] )
    {
        if ( isspace( sdsname[i] ) )
            sdsname[i] = '_';
        i++;
    }

    sprintf( filename, "%s_%s.hdr", ParamList->output_file_name,
      sdsname );

    /* determine machine endiannesss */
    strcpy( endianstr, "BYTE_ORDER = " );
    if( machineEndianness == MRTSWATH_BIG_ENDIAN ) {
       strcat( endianstr, "big_endian\n" );
    } else if ( machineEndianness == MRTSWATH_LITTLE_ENDIAN ) {
       strcat( endianstr, "little_endian\n" );
    } else {
       LOG_RETURN_ERROR("Error: Unable to determine machine endianness",
                    "WriteHeaderFile", false);
    }

        /*******************************************************************/

    /* open header file for writing */
    fpo = fopen( filename, "w" );

    /* check if header file was successfully opened */
    if ( fpo == NULL )
    {
	sprintf( str, "unable to open %s", filename );
        LOG_RETURN_ERROR( str, "WriteHeaderFile", false );
    }

	/*******************************************************************/

    /* write output projection type: PROJECTION_TYPE = ... */
    fprintf( fpo, "\nPROJECTION_TYPE = " );

    switch ( ParamList->output_space_def.proj_num )
    {
	case PROJ_ISINUS:
	    fprintf( fpo, "INTEGERIZED_SINUSOIDAL\n" );
	    break;

	case PROJ_ALBERS:
	    fprintf( fpo, "ALBERS_EQUAL_AREA\n" );
	    break;

	case PROJ_EQRECT:
	    fprintf( fpo, "EQUIRECTANGULAR\n" );
	    break;

	case PROJ_GEO:
	    fprintf( fpo, "GEOGRAPHIC\n" );
	    break;

	case PROJ_HAMMER:
	    fprintf( fpo, "HAMMER\n" );
	    break;

	case PROJ_GOODE:
	    fprintf( fpo, "INTERRUPTED_GOODE_HOMOLOSINE\n" );
	    break;

	case PROJ_LAMAZ:
	    fprintf( fpo, "LAMBERT_AZIMUTHAL\n" );
	    break;

	case PROJ_LAMCC:
	    fprintf( fpo, "LAMBERT_CONFORMAL_CONIC\n" );
	    break;

	case PROJ_MERCAT:
	    fprintf( fpo, "MERCATOR\n" );
	    break;

	case PROJ_MOLL:
	    fprintf( fpo, "MOLLWEIDE\n" );
	    break;

	case PROJ_PS:
	    fprintf( fpo, "POLAR_STEREOGRAPHIC\n" );
	    break;

	case PROJ_SNSOID:
	    fprintf( fpo, "SINUSOIDAL\n" );
	    break;

	case PROJ_TM:
	    fprintf( fpo, "TRANSVERSE_MERCATOR\n" );
	    break;

	case PROJ_UTM:
	    fprintf( fpo, "UTM\n" );

            /* write UTM zone */
            fprintf( fpo, "\nUTM_ZONE = %i\n",
                ParamList->output_space_def.zone );
	    break;

	default:
            LOG_RETURN_ERROR( "bad output projection type", "WriteHeaderFile",
                false );
    }

	/*******************************************************************/

    /*
     * write 15 output projection parameters:
     *
     * PROJECTION_PARAMETERS = ( p1 p2 ... p15 )
     */

    fprintf( fpo, "\nPROJECTION_PARAMETERS = (" );

    for ( i = 0; i < NPROJ_PARAM; i++ )
    {
	if ( i % 3 == 0 )
	    fprintf( fpo, "\n" );

	fprintf( fpo, "%24.9f",
            ParamList->output_space_def.orig_proj_param[i] );
    }
    fprintf( fpo, " )\n" );

	/*******************************************************************/

    /*
     * write spatial extents (lat/lon)
     *
     * UL_CORNER_LATLON = ( ULlat ULlon )
     * UR_CORNER_LATLON = ( URlat URlon )
     * LL_CORNER_LATLON = ( LLlat LLlon )
     * LR_CORNER_LATLON = ( LRlat LRlon )
     */

    fprintf( fpo, "\nUL_CORNER_LATLON = ( %.9f %.9f )\n",
        ParamList->output_space_def.ul_corner_geo.lat * DEG,
        ParamList->output_space_def.ul_corner_geo.lon * DEG );

    fprintf( fpo, "UR_CORNER_LATLON = ( %.9f %.9f )\n",
        ParamList->output_space_def.ul_corner_geo.lat * DEG,
        ParamList->output_space_def.lr_corner_geo.lon * DEG );

    fprintf( fpo, "LL_CORNER_LATLON = ( %.9f %.9f )\n",
        ParamList->output_space_def.lr_corner_geo.lat * DEG,
        ParamList->output_space_def.ul_corner_geo.lon * DEG );

    fprintf( fpo, "LR_CORNER_LATLON = ( %.9f %.9f )\n",
        ParamList->output_space_def.lr_corner_geo.lat * DEG,
        ParamList->output_space_def.lr_corner_geo.lon * DEG );

	/*******************************************************************/

    /*
     * write spatial extents (projection coordinates)
     *
     * UL_CORNER_XY = ( ULlat ULlon )
     * UR_CORNER_XY = ( URlat URlon )
     * LL_CORNER_XY = ( LLlat LLlon )
     * LR_CORNER_XY = ( LRlat LRlon )
     */

    if ( ParamList->output_space_def.proj_num == PROJ_GEO ) {
       /* Geographic has values in radians so output in degrees */
       fprintf( fpo, "\n# UL_CORNER_XY = ( %.9f %.9f )\n",
           ParamList->output_space_def.ul_corner.x * DEG,
           ParamList->output_space_def.ul_corner.y * DEG );

       fprintf( fpo, "# UR_CORNER_XY = ( %.9f %.9f )\n",
           ParamList->output_space_def.lr_corner.x * DEG,
           ParamList->output_space_def.ul_corner.y * DEG );

       fprintf( fpo, "# LL_CORNER_XY = ( %.9f %.9f )\n",
           ParamList->output_space_def.ul_corner.x * DEG,
           ParamList->output_space_def.lr_corner.y * DEG );

       fprintf( fpo, "# LR_CORNER_XY = ( %.9f %.9f )\n",
           ParamList->output_space_def.lr_corner.x * DEG,
           ParamList->output_space_def.lr_corner.y * DEG );
    }
    else {
       fprintf( fpo, "\n# UL_CORNER_XY = ( %.9f %.9f )\n",
           ParamList->output_space_def.ul_corner.x,
           ParamList->output_space_def.ul_corner.y );

       fprintf( fpo, "# UR_CORNER_XY = ( %.9f %.9f )\n",
           ParamList->output_space_def.lr_corner.x,
           ParamList->output_space_def.ul_corner.y );

       fprintf( fpo, "# LL_CORNER_XY = ( %.9f %.9f )\n",
           ParamList->output_space_def.ul_corner.x,
           ParamList->output_space_def.lr_corner.y );

       fprintf( fpo, "# LR_CORNER_XY = ( %.9f %.9f )\n",
           ParamList->output_space_def.lr_corner.x,
           ParamList->output_space_def.lr_corner.y );
    }

	/*******************************************************************/

    /* write only one band at a time */

    fprintf( fpo, "\nNBANDS = 1\n" );

	/*******************************************************************/

    /* write band name */

    fprintf( fpo, "BANDNAMES = ( %s )\n", ParamList->output_sds_name );

	/*******************************************************************/

    /* write output data type: DATA_TYPE = ... */

    fprintf( fpo, "DATA_TYPE = (" );

    switch ( ParamList->output_data_type )
    {
	case DFNT_CHAR8:
	    fprintf( fpo, " CHAR8" );
	    break;
	case DFNT_INT8:
	    fprintf( fpo, " INT8" );
	    break;
	case DFNT_UINT8:
	    fprintf( fpo, " UINT8" );
	    break;
	case DFNT_INT16:
	    fprintf( fpo, " INT16" );
	    break;
	case DFNT_UINT16:
	    fprintf( fpo, " UINT16" );
	    break;
	case DFNT_INT32:
	    fprintf( fpo, " INT32" );
	    break;
	case DFNT_UINT32:
	    fprintf( fpo, " UINT32" );
	    break;
	case DFNT_FLOAT32:
	    fprintf( fpo, " FLOAT32" );
	    break;
	default:
            LOG_RETURN_ERROR("bad output data type", "WriteHeaderFile", false);
    }
    fprintf( fpo, " )\n" );

	/*******************************************************************/

    /* write NLINES field */
    fprintf( fpo, "NLINES = ( %i )\n",
        ParamList->output_space_def.img_size.l );

	/*******************************************************************/

    /* write NSAMPLES field */

    fprintf( fpo, "NSAMPLES = ( %i )\n",
        ParamList->output_space_def.img_size.s );

	/*******************************************************************/

    /* write PIXEL_SIZE field */

    if ( ParamList->output_space_def.proj_num == PROJ_GEO )
    {
        /* pixel size (output in degrees, internally this is stored in
           radians for Geographic) */
        fprintf( fpo, "PIXEL_SIZE = ( %.12f )\n",
            ParamList->output_space_def.pixel_size * DEG );
    }
    else
    {
        fprintf( fpo, "PIXEL_SIZE = ( %.12f )\n",
            ParamList->output_space_def.pixel_size );
    }

	/*******************************************************************/

    /* write BACKGROUND_FILL field */

    fprintf( fpo, "BACKGROUND_FILL = ( %f )\n", PatchesList->fill_value );

	/*******************************************************************/

    /* write the ellipsoid */
    if (ParamList->output_space_def.sphere < 0 ||
        ParamList->output_space_def.sphere >= PROJ_NSPHERE)
        fprintf( fpo, "\nELLIPSOID_CODE = No Ellipsoid\n" );
    else
        fprintf( fpo, "\nELLIPSOID_CODE = %s\n",
            Proj_sphere[ParamList->output_space_def.sphere].name );

	/*******************************************************************/

    /* write the datum value, if an ellipse-based projection then WGS84
       ellipsoids can be tagged with the WGS84 datum.  otherwise no datum
       will be used. */

    switch (ParamList->output_space_def.proj_num)
    {
        case PROJ_ALBERS:
        case PROJ_EQRECT:
        case PROJ_GEO:
        case PROJ_MERCAT:
        case PROJ_TM:
        case PROJ_UTM:
        case PROJ_LAMCC:
        case PROJ_PS:
            /* If WGS84 ellipsoid, then we can tag the WGS84 datum,
               otherwise no datum can be specified. */
            if (ParamList->output_space_def.sphere == 8 /* WGS84 */ ||
             (ParamList->output_space_def.orig_proj_param[0] == 6378137.0 &&
              ParamList->output_space_def.orig_proj_param[1] == 6356752.31414))
                fprintf( fpo, "\nDATUM = WGS84\n" );
            else
                fprintf( fpo, "\nDATUM = No Datum\n" );
            break;

        default:
            fprintf( fpo, "\nDATUM = No Datum\n" );
            break;
    }

	/*******************************************************************/

    /* write byte_order */
    fprintf( fpo, endianstr );

    /* finish up */

    fclose( fpo );
    return true;
}

