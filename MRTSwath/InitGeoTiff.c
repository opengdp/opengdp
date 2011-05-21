#include "GeoS2G.h"
#include "hntdefs.h"
#include "space.h"
#include "myerror.h"
#include <string.h>
#include <stdlib.h>
#include "const.h"

static void SetGeoTIFFDatum (GeoTIFFFD *geotiff, Space_def_t *outproj, char *citation);
static void SetGeoTIFFSphere (GeoTIFFFD *geotiff, Space_def_t *outproj,
                       char *citation);

/* External arrays */

extern Proj_sphere_t Proj_sphere[PROJ_NSPHERE];
extern Proj_type_t Proj_type[PROJ_NPROJ];

/******************************************************************************

MODULE:  OpenGeoTIFFFile

PURPOSE: Set GeoTIFF tags and initialize TIFF output info

RETURN VALUE:
Type = bool
Value           Description
-----           -----------
true            Success
false           Failure

HISTORY:
Version  Date   Programmer       Code  Reason
-------  -----  ---------------  ----  -------------------------------------
         12/02  Gail Schmidt           Original Development
         11/03  Gail Schmidt           Ellipse-based projection have a datum
                                       of WGS84. Sphere-based projections
                                       don't have datums.
         12/03  Gail Schmidt           Added error checking
         04/04  Gail Schmidt           Since the user is providing an
                                       ellipsoid (or the first two projection
                                       parameters to specify the axis of the
                                       ellipsoid), it is technically incorrect
                                       to tag a datum to the data unless the
                                       ellipsoid is WGS84, which lines up with
                                       the WGS84 datum axis values.  UTM can
                                       also be tagged with WGS84 datum.
         05/04  Gail Schmidt           Add support for CHAR8 data types.
         06/04  Gail Schmidt           Modified the output corner point to
                                       be the center of the pixel rather than
                                       the extent of the pixel
         07/04  Gail Schmidt           Modified the GeoTiff name to remove any
                                       '/'s in the SDSname
         12/04  Gail Schmidt           Originally used PixelIsArea tag for
                                       the center of the pixel instead of
                                       PixelIsPoint. This has been changed.

NOTES:

******************************************************************************/
bool OpenGeoTIFFFile (Param_t *ParamList, GeoTIFFFD *MasterGeoMem)
{

  /* Variable Declarations */
  GeoTIFFFD *geotiff;
  
  double tiepoints[6], pixelscale[3];
  int LinearValue = Linear_Meter;  /* Linear_Meter defined in libgeotiff.a */
  int Set, Zone, i, j;    
  char filename[1024];
  char citation[256], NorS;       
  char errstr[256];
  char sdsname[256];               /* SDS string without '/'s */

  int UTMWGS84_ZoneCodes[2][60] = { /* zone code for UTM WGS84 projections */
        {PCS_WGS84_UTM_zone_1N,
         PCS_WGS84_UTM_zone_2N,
         PCS_WGS84_UTM_zone_3N,
         PCS_WGS84_UTM_zone_4N,
         PCS_WGS84_UTM_zone_5N,
         PCS_WGS84_UTM_zone_6N,
         PCS_WGS84_UTM_zone_7N,
         PCS_WGS84_UTM_zone_8N,
         PCS_WGS84_UTM_zone_9N,
         PCS_WGS84_UTM_zone_10N,
         PCS_WGS84_UTM_zone_11N,
         PCS_WGS84_UTM_zone_12N,
         PCS_WGS84_UTM_zone_13N,
         PCS_WGS84_UTM_zone_14N,
         PCS_WGS84_UTM_zone_15N,
         PCS_WGS84_UTM_zone_16N,
         PCS_WGS84_UTM_zone_17N,
         PCS_WGS84_UTM_zone_18N,
         PCS_WGS84_UTM_zone_19N,
         PCS_WGS84_UTM_zone_20N,
         PCS_WGS84_UTM_zone_21N,
         PCS_WGS84_UTM_zone_22N,
         PCS_WGS84_UTM_zone_23N,
         PCS_WGS84_UTM_zone_24N,
         PCS_WGS84_UTM_zone_25N,
         PCS_WGS84_UTM_zone_26N,
         PCS_WGS84_UTM_zone_27N,
         PCS_WGS84_UTM_zone_28N,
         PCS_WGS84_UTM_zone_29N,
         PCS_WGS84_UTM_zone_30N,
         PCS_WGS84_UTM_zone_31N,
         PCS_WGS84_UTM_zone_32N,
         PCS_WGS84_UTM_zone_33N,
         PCS_WGS84_UTM_zone_34N,
         PCS_WGS84_UTM_zone_35N,
         PCS_WGS84_UTM_zone_36N,
         PCS_WGS84_UTM_zone_37N,
         PCS_WGS84_UTM_zone_38N,
         PCS_WGS84_UTM_zone_39N,
         PCS_WGS84_UTM_zone_40N,
         PCS_WGS84_UTM_zone_41N,
         PCS_WGS84_UTM_zone_42N,
         PCS_WGS84_UTM_zone_43N,
         PCS_WGS84_UTM_zone_44N,
         PCS_WGS84_UTM_zone_45N,
         PCS_WGS84_UTM_zone_46N,
         PCS_WGS84_UTM_zone_47N,
         PCS_WGS84_UTM_zone_48N,
         PCS_WGS84_UTM_zone_49N,
         PCS_WGS84_UTM_zone_50N,
         PCS_WGS84_UTM_zone_51N,
         PCS_WGS84_UTM_zone_52N,
         PCS_WGS84_UTM_zone_53N,
         PCS_WGS84_UTM_zone_54N,
         PCS_WGS84_UTM_zone_55N,
         PCS_WGS84_UTM_zone_56N,
         PCS_WGS84_UTM_zone_57N,
         PCS_WGS84_UTM_zone_58N,
         PCS_WGS84_UTM_zone_59N,
         PCS_WGS84_UTM_zone_60N},
        {PCS_WGS84_UTM_zone_1S,
         PCS_WGS84_UTM_zone_2S,
         PCS_WGS84_UTM_zone_3S,
         PCS_WGS84_UTM_zone_4S,
         PCS_WGS84_UTM_zone_5S,
         PCS_WGS84_UTM_zone_6S,
         PCS_WGS84_UTM_zone_7S,
         PCS_WGS84_UTM_zone_8S,
         PCS_WGS84_UTM_zone_9S,
         PCS_WGS84_UTM_zone_10S,
         PCS_WGS84_UTM_zone_11S,
         PCS_WGS84_UTM_zone_12S,
         PCS_WGS84_UTM_zone_13S,
         PCS_WGS84_UTM_zone_14S,
         PCS_WGS84_UTM_zone_15S,
         PCS_WGS84_UTM_zone_16S,
         PCS_WGS84_UTM_zone_17S,
         PCS_WGS84_UTM_zone_18S,
         PCS_WGS84_UTM_zone_19S,
         PCS_WGS84_UTM_zone_20S,
         PCS_WGS84_UTM_zone_21S,
         PCS_WGS84_UTM_zone_22S,
         PCS_WGS84_UTM_zone_23S,
         PCS_WGS84_UTM_zone_24S,
         PCS_WGS84_UTM_zone_25S,
         PCS_WGS84_UTM_zone_26S,
         PCS_WGS84_UTM_zone_27S,
         PCS_WGS84_UTM_zone_28S,
         PCS_WGS84_UTM_zone_29S,
         PCS_WGS84_UTM_zone_30S,
         PCS_WGS84_UTM_zone_31S,
         PCS_WGS84_UTM_zone_32S,
         PCS_WGS84_UTM_zone_33S,
         PCS_WGS84_UTM_zone_34S,
         PCS_WGS84_UTM_zone_35S,
         PCS_WGS84_UTM_zone_36S,
         PCS_WGS84_UTM_zone_37S,
         PCS_WGS84_UTM_zone_38S,
         PCS_WGS84_UTM_zone_39S,
         PCS_WGS84_UTM_zone_40S,
         PCS_WGS84_UTM_zone_41S,
         PCS_WGS84_UTM_zone_42S,
         PCS_WGS84_UTM_zone_43S,
         PCS_WGS84_UTM_zone_44S,
         PCS_WGS84_UTM_zone_45S,
         PCS_WGS84_UTM_zone_46S,
         PCS_WGS84_UTM_zone_47S,
         PCS_WGS84_UTM_zone_48S,
         PCS_WGS84_UTM_zone_49S,
         PCS_WGS84_UTM_zone_50S,
         PCS_WGS84_UTM_zone_51S,
         PCS_WGS84_UTM_zone_52S,
         PCS_WGS84_UTM_zone_53S,
         PCS_WGS84_UTM_zone_54S,
         PCS_WGS84_UTM_zone_55S,
         PCS_WGS84_UTM_zone_56S,
         PCS_WGS84_UTM_zone_57S,
         PCS_WGS84_UTM_zone_58S,
         PCS_WGS84_UTM_zone_59S,
         PCS_WGS84_UTM_zone_60S}
    };     

  /** End Variable Declarations **/
  
  if( MasterGeoMem == NULL ) {
    sprintf(errstr, "internal GeoTiff error, MasterGeoMem not allocated");
    LOG_RETURN_ERROR(errstr, "OpenGeoTIFFFile", false);
  }

  /** Making the tag assignment easier **/
  geotiff = MasterGeoMem;
  strcpy(citation,"");

  /** Open the File for writing **/
  /** Copy the SDS name and remove any '/'s in the SDSname */
  j = 0;
  for (i = 0; i < (int)strlen(ParamList->output_sds_name); i++)
  {
      if (ParamList->output_sds_name[i] != '/' &&
          ParamList->output_sds_name[i] != '\\')
      {
          sdsname[j++] = ParamList->output_sds_name[i];
      }
      sdsname[j] = '\0';
  }

  /** Remove any spaces from the name **/
  i = 0;
  while (sdsname[i])
  {
    if(isspace(sdsname[i]))
      sdsname[i] = '_';  
    i++;
  }

  /** First make a file name with SDS name attached **/
  sprintf(filename, "%s_%s.tif", ParamList->output_file_name, sdsname);

  /** Open the GeoTiff File **/
  geotiff->tif  = XTIFFOpen(filename, "w");
  if (!geotiff->tif)
  {
    sprintf(errstr, "error opening GeoTiff file: %s", filename);
    LOG_RETURN_ERROR(errstr, "OpenGeoTIFFFile", false);
  }

  /** set up initial GeoTIFF info **/
  geotiff->gtif = GTIFNew(geotiff->tif);    
  if (!geotiff->tif)
  {
    sprintf(errstr, "error setting up GeoTiff file: %s", filename);
    XTIFFClose(geotiff->tif);
    LOG_RETURN_ERROR(errstr, "OpenGeoTIFFFile", false);
  }

  /** Set Width in samples **/
  TIFFSetField(geotiff->tif, TIFFTAG_IMAGEWIDTH, 
    ParamList->output_space_def.img_size.s);

  /** Set Length in Lines **/
  TIFFSetField(geotiff->tif,TIFFTAG_IMAGELENGTH,
    ParamList->output_space_def.img_size.l);  

  TIFFSetField( geotiff->tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE );
  TIFFSetField( geotiff->tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK );
  TIFFSetField( geotiff->tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG );
  TIFFSetField( geotiff->tif, TIFFTAG_SAMPLESPERPIXEL, 1 );
  TIFFSetField( geotiff->tif, TIFFTAG_ROWSPERSTRIP, 1L );
  TIFFSetField( geotiff->tif, TIFFTAG_SOFTWARE, "MRTSwath" );

  /** Set the Output Datatype **/
  switch(ParamList->output_data_type)
  {
    case DFNT_INT8:
    {
      TIFFSetField( geotiff->tif, TIFFTAG_BITSPERSAMPLE, 8 );
      TIFFSetField( geotiff->tif, TIFFTAG_SAMPLEFORMAT, 2 );
      break;
    }
    case DFNT_UINT8:
    case DFNT_CHAR8:
    {
      TIFFSetField( geotiff->tif, TIFFTAG_BITSPERSAMPLE, 8 );
      TIFFSetField( geotiff->tif, TIFFTAG_SAMPLEFORMAT, 1 );
      break;
    }
    case DFNT_INT16:
    {
      TIFFSetField( geotiff->tif, TIFFTAG_BITSPERSAMPLE, 16 );
      TIFFSetField( geotiff->tif, TIFFTAG_SAMPLEFORMAT, 2 );
      break;
    }
    case DFNT_UINT16:
    {
      TIFFSetField( geotiff->tif, TIFFTAG_BITSPERSAMPLE, 16 );
      TIFFSetField( geotiff->tif, TIFFTAG_SAMPLEFORMAT, 1 );
      break;
    }
    case DFNT_INT32:
    {
      TIFFSetField( geotiff->tif, TIFFTAG_BITSPERSAMPLE, 32 );
      TIFFSetField( geotiff->tif, TIFFTAG_SAMPLEFORMAT, 2 );
      break;
    }
    case DFNT_UINT32:
    {
      TIFFSetField( geotiff->tif, TIFFTAG_BITSPERSAMPLE, 32 );
      TIFFSetField( geotiff->tif, TIFFTAG_SAMPLEFORMAT, 1 );
      break;
    }
    case DFNT_FLOAT32:
    {
      TIFFSetField( geotiff->tif, TIFFTAG_BITSPERSAMPLE, 32 );
      TIFFSetField( geotiff->tif, TIFFTAG_SAMPLEFORMAT, 3 );
      break;
    }
    default:
    {
      sprintf(errstr, "error in output data type -- Unknown %d\n",
        ParamList->output_data_type);     
      LOG_RETURN_ERROR(errstr, "OpenGeoTIFFFile", false);
    }
  }

  /** UL Corner                                                  **/
  /** NOTE: According to note in Geotiff Source code (tifinit.c) **/
  /** quoting Goetiff docs. only need the UL corner specified    **/
  /* Since we are using RasterPixelIsPoint for the RasterTypeGeoKey, the
     UL corner point needs to be the center of the pixel */
  tiepoints[0] = tiepoints[1] = tiepoints[2] = tiepoints[5] = 0.0;
  if (ParamList->output_space_def.proj_num == PROJ_GEO)
  {
    /* If this is Geographic then coords are in radians. Convert to
       degrees before output. */
    tiepoints[3] = ParamList->output_space_def.ul_corner.x * DEG +
      0.5 * ParamList->output_space_def.pixel_size * DEG;
    tiepoints[4] = ParamList->output_space_def.ul_corner.y * DEG -
      0.5 * ParamList->output_space_def.pixel_size * DEG;
  }
  else
  {
    tiepoints[3] = ParamList->output_space_def.ul_corner.x +
      0.5 * ParamList->output_space_def.pixel_size;
    tiepoints[4] = ParamList->output_space_def.ul_corner.y -
      0.5 * ParamList->output_space_def.pixel_size;
  }
  TIFFSetField( geotiff->tif, TIFFTAG_GEOTIEPOINTS, 6, tiepoints );

#ifdef DEBUG
  printf ("Output UL projection coords: %f %f\n",
      ParamList->output_space_def.ul_corner.x,
      ParamList->output_space_def.ul_corner.y);
#endif

  /** Set Pixel Size (output in degrees, internally this is stored in
      Radians) **/
  if (ParamList->output_space_def.proj_num == PROJ_GEO)
  {
      pixelscale[0] = pixelscale[1] =
          ParamList->output_space_def.pixel_size * DEG;
  }
  else
  {
      pixelscale[0] = pixelscale[1] = ParamList->output_space_def.pixel_size;
  }
  pixelscale[2] = 0.0f;
   
  TIFFSetField( geotiff->tif, TIFFTAG_GEOPIXELSCALE, 3, pixelscale );

  /** Units                                         **/
  /** All are METERs except for GEO which is DEGREES **/

  /** Set the rest of the Geo Keys **/
  /** Use orig_proj_param coordinates since they are in decimal degrees
      and not DMS **/
  switch (ParamList->output_space_def.proj_num)
  {
    case PROJ_ALBERS:
    { /* datum */
      GTIFKeySet(geotiff->gtif, ProjCoordTransGeoKey, TYPE_SHORT, 1,
                 CT_AlbersEqualArea);
      GTIFKeySet(geotiff->gtif, GTModelTypeGeoKey, TYPE_SHORT, 1,
                 ModelTypeProjected);
      GTIFKeySet(geotiff->gtif, GTRasterTypeGeoKey, TYPE_SHORT, 1,
                 RasterPixelIsPoint);

      strcpy( citation, "AEA        " );
      SetGeoTIFFDatum( geotiff, &(ParamList->output_space_def), citation );

      GTIFKeySet(geotiff->gtif, GTCitationGeoKey, TYPE_ASCII, 0,
                 citation);
      GTIFKeySet(geotiff->gtif, GeogLinearUnitsGeoKey, TYPE_SHORT, 1,
                 LinearValue);
      GTIFKeySet(geotiff->gtif, GeogAngularUnitsGeoKey, TYPE_SHORT,
                 1, Angular_Degree);
      GTIFKeySet(geotiff->gtif, ProjectedCSTypeGeoKey, TYPE_SHORT, 1,
                 KvUserDefined);
      GTIFKeySet(geotiff->gtif, ProjectionGeoKey, TYPE_SHORT, 1,
                 KvUserDefined);
      GTIFKeySet(geotiff->gtif, ProjLinearUnitsGeoKey, TYPE_SHORT, 1,
                 LinearValue);
      GTIFKeySet(geotiff->gtif, ProjStdParallel1GeoKey, TYPE_DOUBLE,
                 1, ParamList->output_space_def.orig_proj_param[2]);
      GTIFKeySet(geotiff->gtif, ProjStdParallel2GeoKey, TYPE_DOUBLE,
                 1, ParamList->output_space_def.orig_proj_param[3]);
      GTIFKeySet(geotiff->gtif, ProjNatOriginLongGeoKey, TYPE_DOUBLE,
                 1, ParamList->output_space_def.orig_proj_param[4]);
      GTIFKeySet(geotiff->gtif, ProjNatOriginLatGeoKey, TYPE_DOUBLE,
                 1, ParamList->output_space_def.orig_proj_param[5]);
      GTIFKeySet(geotiff->gtif, ProjFalseEastingGeoKey, TYPE_DOUBLE,
                 1, ParamList->output_space_def.orig_proj_param[6]);
      GTIFKeySet(geotiff->gtif, ProjFalseNorthingGeoKey, TYPE_DOUBLE,
                 1, ParamList->output_space_def.orig_proj_param[7]);
      GTIFKeySet(geotiff->gtif, ProjFalseOriginLongGeoKey,
                 TYPE_DOUBLE, 1, (double)0.0);
      GTIFKeySet(geotiff->gtif, ProjFalseOriginLatGeoKey,
                 TYPE_DOUBLE, 1, (double)0.0);
      break;
    }
    case PROJ_EQRECT:
    { /* datum */
      GTIFKeySet( geotiff->gtif, ProjCoordTransGeoKey, TYPE_SHORT, 1,
                  CT_Equirectangular );
      GTIFKeySet( geotiff->gtif, GTModelTypeGeoKey, TYPE_SHORT, 1,
                  ModelTypeProjected );
      GTIFKeySet( geotiff->gtif, GTRasterTypeGeoKey, TYPE_SHORT, 1,
                  RasterPixelIsPoint );

      strcpy( citation, "EQRECT     " );
      SetGeoTIFFDatum( geotiff, &(ParamList->output_space_def), citation );

      GTIFKeySet(geotiff->gtif, GTCitationGeoKey, TYPE_ASCII, 0,
                  citation);
      GTIFKeySet(geotiff->gtif, GeogLinearUnitsGeoKey, TYPE_SHORT,
                  1, LinearValue);
      GTIFKeySet(geotiff->gtif, GeogAngularUnitsGeoKey, TYPE_SHORT,
                  1, Angular_Degree);
      GTIFKeySet(geotiff->gtif, ProjectedCSTypeGeoKey, TYPE_SHORT,
                  1, KvUserDefined);
      GTIFKeySet(geotiff->gtif, ProjLinearUnitsGeoKey, TYPE_SHORT,
                  1, LinearValue);
      GTIFKeySet(geotiff->gtif, ProjNatOriginLongGeoKey, TYPE_DOUBLE,
                  1, ParamList->output_space_def.orig_proj_param[4]);
      GTIFKeySet(geotiff->gtif, ProjNatOriginLatGeoKey, TYPE_DOUBLE,
                  1, ParamList->output_space_def.orig_proj_param[5]);
      GTIFKeySet( geotiff->gtif, ProjFalseEastingGeoKey, TYPE_DOUBLE,
                  1, ParamList->output_space_def.orig_proj_param[6] );
      GTIFKeySet( geotiff->gtif, ProjFalseNorthingGeoKey, TYPE_DOUBLE,
                  1, ParamList->output_space_def.orig_proj_param[7] );
      break;
    }
    case PROJ_GEO:
    { /* datum */
      GTIFKeySet( geotiff->gtif, GTModelTypeGeoKey, TYPE_SHORT, 1,
                  ModelTypeGeographic );
      GTIFKeySet( geotiff->gtif, GTRasterTypeGeoKey, TYPE_SHORT, 1,
	          RasterPixelIsPoint );
      GTIFKeySet( geotiff->gtif, GeogAngularUnitsGeoKey,
                  TYPE_SHORT, 1, Angular_Degree );

      strcpy( citation, "Geographic (Longitude, Latitude) " );
      SetGeoTIFFDatum( geotiff, &(ParamList->output_space_def), citation );

      GTIFKeySet( geotiff->gtif, GTCitationGeoKey, TYPE_ASCII, 1,
                  citation );
      break;
    }
    case PROJ_ISINUS:
    { /* no datum */
      GTIFKeySet( geotiff->gtif, GTModelTypeGeoKey, TYPE_SHORT, 1,
                  ModelTypeProjected );
      GTIFKeySet( geotiff->gtif, GTRasterTypeGeoKey, TYPE_SHORT, 1,
                  RasterPixelIsPoint );
      GTIFKeySet( geotiff->gtif, ProjCoordTransGeoKey, TYPE_SHORT, 1,
                  KvUserDefined );			
      GTIFKeySet( geotiff->gtif, ProjectionGeoKey, TYPE_SHORT, 1,
                  KvUserDefined );
      GTIFKeySet( geotiff->gtif, ProjectedCSTypeGeoKey, TYPE_SHORT, 1,
                  KvUserDefined );

      strcpy( citation, "Integerized Sinusoidal " );
      SetGeoTIFFSphere( geotiff, &(ParamList->output_space_def), citation );

      GTIFKeySet( geotiff->gtif, PCSCitationGeoKey, TYPE_ASCII, 1,
                   citation );
						
      GTIFKeySet( geotiff->gtif, GeogLinearUnitsGeoKey,
                  TYPE_SHORT, 1, LinearValue );
      GTIFKeySet( geotiff->gtif, GeogAngularUnitsGeoKey,
                  TYPE_SHORT, 1, Angular_Degree );
      GTIFKeySet( geotiff->gtif, ProjCenterLongGeoKey, TYPE_DOUBLE,
                  1, ParamList->output_space_def.orig_proj_param[4] );

      /* Set the false easting and false northing
      ---------------------------------------- */
      GTIFKeySet( geotiff->gtif, ProjFalseEastingGeoKey, TYPE_DOUBLE,
                  1, ParamList->output_space_def.orig_proj_param[6]);
      GTIFKeySet( geotiff->gtif, ProjFalseNorthingGeoKey, TYPE_DOUBLE,
                  1, ParamList->output_space_def.orig_proj_param[7]);
      break;
    }
    case PROJ_LAMAZ:
    { /* no datum */
      GTIFKeySet( geotiff->gtif, ProjCoordTransGeoKey, TYPE_SHORT, 1,
                  CT_LambertAzimEqualArea );
      GTIFKeySet( geotiff->gtif, GTModelTypeGeoKey, TYPE_SHORT, 1,
                  ModelTypeProjected );
      GTIFKeySet( geotiff->gtif, GTRasterTypeGeoKey, TYPE_SHORT, 1,
                  RasterPixelIsPoint );

      strcpy( citation, "LAEA       " );
      SetGeoTIFFSphere( geotiff, &(ParamList->output_space_def), citation );

      GTIFKeySet(geotiff->gtif, GTCitationGeoKey, TYPE_ASCII, 0,
                 citation);
      GTIFKeySet(geotiff->gtif, GeogLinearUnitsGeoKey, TYPE_SHORT,
                 1, LinearValue);
      GTIFKeySet(geotiff->gtif, GeogAngularUnitsGeoKey, TYPE_SHORT,
                 1, Angular_Degree);
      GTIFKeySet(geotiff->gtif, ProjectedCSTypeGeoKey, TYPE_SHORT,
                 1, KvUserDefined);
      GTIFKeySet(geotiff->gtif, ProjLinearUnitsGeoKey, TYPE_SHORT,
                 1, LinearValue);
      GTIFKeySet( geotiff->gtif, ProjCenterLongGeoKey, TYPE_DOUBLE,
                  1, ParamList->output_space_def.orig_proj_param[4] );
      GTIFKeySet( geotiff->gtif, ProjCenterLatGeoKey, TYPE_DOUBLE,
                  1, ParamList->output_space_def.orig_proj_param[5] );
      GTIFKeySet( geotiff->gtif, ProjFalseEastingGeoKey, TYPE_DOUBLE,
                  1, ParamList->output_space_def.orig_proj_param[6] );
      GTIFKeySet( geotiff->gtif, ProjFalseNorthingGeoKey, TYPE_DOUBLE,
                  1, ParamList->output_space_def.orig_proj_param[7] );

      break;
    }   
    case PROJ_MERCAT:
    { /* datum */
      GTIFKeySet(geotiff->gtif, ProjCoordTransGeoKey, TYPE_SHORT, 1,
                 CT_Mercator);
      GTIFKeySet(geotiff->gtif, GTModelTypeGeoKey, TYPE_SHORT, 1,
                 ModelTypeProjected);
      GTIFKeySet(geotiff->gtif, GTRasterTypeGeoKey, TYPE_SHORT, 1,
                 RasterPixelIsPoint );

      strcpy( citation, "MERCATOR   " );
      SetGeoTIFFDatum( geotiff, &(ParamList->output_space_def), citation );

      GTIFKeySet(geotiff->gtif, GTCitationGeoKey, TYPE_ASCII, 0,
                 citation);
      GTIFKeySet(geotiff->gtif, GeogLinearUnitsGeoKey, TYPE_SHORT,
                 1, LinearValue);
      GTIFKeySet(geotiff->gtif, GeogAngularUnitsGeoKey, TYPE_SHORT,
                 1, Angular_Degree);
      GTIFKeySet(geotiff->gtif, ProjectedCSTypeGeoKey, TYPE_SHORT,
                 1, KvUserDefined);
      GTIFKeySet(geotiff->gtif, ProjectionGeoKey, TYPE_SHORT, 1,
                 KvUserDefined);
      GTIFKeySet(geotiff->gtif, ProjLinearUnitsGeoKey, TYPE_SHORT, 1,
                 LinearValue);
      GTIFKeySet(geotiff->gtif, ProjNatOriginLongGeoKey, TYPE_DOUBLE,
                 1, ParamList->output_space_def.orig_proj_param[4]);
      GTIFKeySet(geotiff->gtif, ProjNatOriginLatGeoKey, TYPE_DOUBLE,
                 1, ParamList->output_space_def.orig_proj_param[5]);
      GTIFKeySet(geotiff->gtif, ProjFalseEastingGeoKey, TYPE_DOUBLE,
                 1, ParamList->output_space_def.orig_proj_param[6]);
      GTIFKeySet(geotiff->gtif, ProjFalseNorthingGeoKey, TYPE_DOUBLE,
                 1, ParamList->output_space_def.orig_proj_param[7]);
      break;
    }
    case PROJ_TM:
    { /* datum */
      GTIFKeySet(geotiff->gtif, ProjCoordTransGeoKey, TYPE_SHORT, 1,
                 CT_TransverseMercator);
      GTIFKeySet(geotiff->gtif, GTModelTypeGeoKey, TYPE_SHORT, 1,
                 ModelTypeProjected);
      GTIFKeySet(geotiff->gtif, GTRasterTypeGeoKey, TYPE_SHORT, 1,
                 RasterPixelIsPoint );

      strcpy( citation, "TM         " );
      SetGeoTIFFDatum( geotiff, &(ParamList->output_space_def), citation );

      GTIFKeySet(geotiff->gtif, GTCitationGeoKey, TYPE_ASCII, 0,
                 citation);
      GTIFKeySet(geotiff->gtif, GeogLinearUnitsGeoKey, TYPE_SHORT,
                 1, LinearValue);
      GTIFKeySet(geotiff->gtif, GeogAngularUnitsGeoKey, TYPE_SHORT,
                 1, Angular_Degree);
      GTIFKeySet(geotiff->gtif, ProjectedCSTypeGeoKey, TYPE_SHORT,
                 1, KvUserDefined);
      GTIFKeySet(geotiff->gtif, ProjectionGeoKey, TYPE_SHORT, 1,
                 KvUserDefined);
      GTIFKeySet(geotiff->gtif, ProjLinearUnitsGeoKey, TYPE_SHORT,
                 1, LinearValue);

      GTIFKeySet(geotiff->gtif, ProjCenterLongGeoKey, TYPE_DOUBLE,
                 1, ParamList->output_space_def.orig_proj_param[4]);
      GTIFKeySet(geotiff->gtif, ProjNatOriginLatGeoKey, TYPE_DOUBLE,
                 1, ParamList->output_space_def.orig_proj_param[5]);
      GTIFKeySet(geotiff->gtif, ProjFalseEastingGeoKey, TYPE_DOUBLE,
                 1, ParamList->output_space_def.orig_proj_param[6]);
      GTIFKeySet(geotiff->gtif, ProjFalseNorthingGeoKey, TYPE_DOUBLE,
                 1, ParamList->output_space_def.orig_proj_param[7]);
      GTIFKeySet(geotiff->gtif, ProjScaleAtNatOriginGeoKey,TYPE_DOUBLE, 
                 1, ParamList->output_space_def.orig_proj_param[2]);
      break;
    }
    case PROJ_UTM:
    { /* datum */
      if (ParamList->output_space_def.zone < 0)     /* South */
      {
        NorS = 'S';
        Set  = 1;
        Zone = abs (ParamList->output_space_def.zone);
      }
      else
      {
        NorS = 'N';
        Set  = 0;
        Zone = ParamList->output_space_def.zone;
      }

      (void) sprintf( citation,
                      "UTM Zone %d %c with WGS84", Zone, NorS);
      Zone -= 1; /* zero base */

      GTIFKeySet (geotiff->gtif, GTModelTypeGeoKey,
                  TYPE_SHORT, 1, ModelTypeProjected);
      GTIFKeySet (geotiff->gtif, GTRasterTypeGeoKey,
                  TYPE_SHORT, 1, RasterPixelIsPoint);
      GTIFKeySet (geotiff->gtif, GTCitationGeoKey,
                  TYPE_ASCII, 0, citation);
      GTIFKeySet (geotiff->gtif, GeogLinearUnitsGeoKey,
                  TYPE_SHORT, 1, LinearValue);
      GTIFKeySet (geotiff->gtif, GeogAngularUnitsGeoKey,
                  TYPE_SHORT, 1, Angular_Degree);
      GTIFKeySet (geotiff->gtif, ProjectedCSTypeGeoKey,
                  TYPE_SHORT, 1, UTMWGS84_ZoneCodes[Set][Zone]);
      break;
    }
    case PROJ_HAMMER:
    { /* no datum */
      GTIFKeySet(geotiff->gtif, ProjCoordTransGeoKey, TYPE_SHORT, 1,32);
      GTIFKeySet(geotiff->gtif, GTModelTypeGeoKey, TYPE_SHORT, 1,
                 ModelTypeProjected );
      GTIFKeySet(geotiff->gtif, GTRasterTypeGeoKey, TYPE_SHORT, 1,
                 RasterPixelIsPoint);

      strcpy( citation, "HAMMER     " );
      SetGeoTIFFSphere( geotiff, &(ParamList->output_space_def), citation );

      GTIFKeySet(geotiff->gtif, GTCitationGeoKey, TYPE_ASCII, 0,
                 citation);
      GTIFKeySet(geotiff->gtif, GeogLinearUnitsGeoKey, TYPE_SHORT, 1,
                 LinearValue);
      GTIFKeySet(geotiff->gtif, GeogAngularUnitsGeoKey, TYPE_SHORT,
                 1, Angular_Degree);
      GTIFKeySet(geotiff->gtif, ProjectedCSTypeGeoKey, TYPE_SHORT, 1,
                 KvUserDefined);
      GTIFKeySet(geotiff->gtif, ProjNatOriginLongGeoKey, TYPE_DOUBLE,
                 1, ParamList->output_space_def.orig_proj_param[4]);
      GTIFKeySet(geotiff->gtif, ProjLinearUnitsGeoKey, TYPE_SHORT, 1,
                 LinearValue);
      GTIFKeySet(geotiff->gtif, ProjFalseEastingGeoKey, TYPE_DOUBLE,
                 1, ParamList->output_space_def.orig_proj_param[6]);
      GTIFKeySet(geotiff->gtif, ProjFalseNorthingGeoKey, TYPE_DOUBLE, 
                 1, ParamList->output_space_def.orig_proj_param[7]);
      break;
    }
    case PROJ_GOODE:
    { /* no datum */
      GTIFKeySet(geotiff->gtif, ProjCoordTransGeoKey, TYPE_SHORT, 1, 29); 
      GTIFKeySet(geotiff->gtif, GTModelTypeGeoKey, TYPE_SHORT, 1,
                 ModelTypeProjected );
      GTIFKeySet(geotiff->gtif, GTRasterTypeGeoKey, TYPE_SHORT, 1,
                 RasterPixelIsPoint );

      strcpy( citation, "IGH        " );
      SetGeoTIFFSphere( geotiff, &(ParamList->output_space_def), citation );

      GTIFKeySet(geotiff->gtif, GTCitationGeoKey, TYPE_ASCII, 0,
                 citation);
      GTIFKeySet(geotiff->gtif, GeogLinearUnitsGeoKey, TYPE_SHORT, 1,
                 LinearValue);
      GTIFKeySet(geotiff->gtif, GeogAngularUnitsGeoKey, TYPE_SHORT,
                 1, Angular_Degree);
      GTIFKeySet(geotiff->gtif, ProjectedCSTypeGeoKey, TYPE_SHORT, 1,
                 KvUserDefined);
      break;
    }
    case PROJ_LAMCC:
    { /* datum */
      GTIFKeySet(geotiff->gtif, ProjCoordTransGeoKey, TYPE_SHORT, 1,
                 CT_LambertConfConic_2SP);
      GTIFKeySet(geotiff->gtif, GTModelTypeGeoKey, TYPE_SHORT, 1,
                 ModelTypeProjected);
      GTIFKeySet(geotiff->gtif, GTRasterTypeGeoKey, TYPE_SHORT, 1,
                 RasterPixelIsPoint );

      strcpy( citation, "LCC        " );
      SetGeoTIFFDatum( geotiff, &(ParamList->output_space_def), citation );

      GTIFKeySet(geotiff->gtif, GTCitationGeoKey, TYPE_ASCII, 0,
                 citation);
      GTIFKeySet(geotiff->gtif, GeogLinearUnitsGeoKey, TYPE_SHORT, 1,
                 LinearValue);
      GTIFKeySet(geotiff->gtif, GeogAngularUnitsGeoKey, TYPE_SHORT,
                 1, Angular_Degree);
      GTIFKeySet(geotiff->gtif, ProjectedCSTypeGeoKey, TYPE_SHORT, 1,
                 KvUserDefined);
      GTIFKeySet(geotiff->gtif, ProjectionGeoKey, TYPE_SHORT, 1,
                 KvUserDefined);
      GTIFKeySet(geotiff->gtif, ProjLinearUnitsGeoKey, TYPE_SHORT, 1,
                 LinearValue);
      GTIFKeySet(geotiff->gtif, ProjStdParallel1GeoKey, TYPE_DOUBLE,
                 1, ParamList->output_space_def.orig_proj_param[2]);
      GTIFKeySet(geotiff->gtif, ProjStdParallel2GeoKey, TYPE_DOUBLE,
                 1, ParamList->output_space_def.orig_proj_param[3]);
      GTIFKeySet(geotiff->gtif, ProjNatOriginLongGeoKey, TYPE_DOUBLE,
                 1, ParamList->output_space_def.orig_proj_param[4]);
      GTIFKeySet(geotiff->gtif, ProjNatOriginLatGeoKey, TYPE_DOUBLE,
                 1, ParamList->output_space_def.orig_proj_param[5]);
      GTIFKeySet(geotiff->gtif, ProjFalseEastingGeoKey, TYPE_DOUBLE,
                 1, ParamList->output_space_def.orig_proj_param[6]);
      GTIFKeySet(geotiff->gtif, ProjFalseNorthingGeoKey, TYPE_DOUBLE,
                 1, ParamList->output_space_def.orig_proj_param[7]);
      GTIFKeySet(geotiff->gtif, ProjFalseOriginLongGeoKey,
                 TYPE_DOUBLE, 1, (double)0.0);
      GTIFKeySet(geotiff->gtif, ProjFalseOriginLatGeoKey,
                 TYPE_DOUBLE, 1, (double)0.0);
      break;
    }
    case PROJ_MOLL:
    { /* no datum */
      GTIFKeySet(geotiff->gtif, ProjCoordTransGeoKey, TYPE_SHORT, 1,30);
      GTIFKeySet(geotiff->gtif, GTModelTypeGeoKey, TYPE_SHORT, 1,
                 ModelTypeProjected );
      GTIFKeySet(geotiff->gtif, GTRasterTypeGeoKey, TYPE_SHORT, 1,
                 RasterPixelIsPoint );

      strcpy( citation, "MOLLWEIDE  " );
      SetGeoTIFFSphere( geotiff, &(ParamList->output_space_def), citation );

      GTIFKeySet(geotiff->gtif, GTCitationGeoKey, TYPE_ASCII, 0,
                 citation);
      GTIFKeySet(geotiff->gtif, GeogLinearUnitsGeoKey, TYPE_SHORT, 1,
                 LinearValue);
      GTIFKeySet(geotiff->gtif, GeogAngularUnitsGeoKey, TYPE_SHORT,
                 1, Angular_Degree);
      GTIFKeySet(geotiff->gtif, ProjectedCSTypeGeoKey, TYPE_SHORT, 1,
                 KvUserDefined);
      GTIFKeySet(geotiff->gtif, ProjNatOriginLongGeoKey, TYPE_DOUBLE,
                 1, ParamList->output_space_def.orig_proj_param[4]);
      GTIFKeySet(geotiff->gtif, ProjLinearUnitsGeoKey, TYPE_SHORT, 1,
                 LinearValue);
      GTIFKeySet(geotiff->gtif, ProjFalseEastingGeoKey, TYPE_DOUBLE,
                 1, ParamList->output_space_def.orig_proj_param[6]);
      GTIFKeySet(geotiff->gtif, ProjFalseNorthingGeoKey, TYPE_DOUBLE, 
                 1, ParamList->output_space_def.orig_proj_param[7]);    
      break;
    }
    case PROJ_PS:
    { /* datum */
      GTIFKeySet(geotiff->gtif, ProjCoordTransGeoKey, TYPE_SHORT, 1,
                 CT_PolarStereographic);
      GTIFKeySet(geotiff->gtif, GTModelTypeGeoKey, TYPE_SHORT, 1,
                 ModelTypeProjected);
      GTIFKeySet(geotiff->gtif, GTRasterTypeGeoKey, TYPE_SHORT, 1,
                 RasterPixelIsPoint );

      strcpy( citation, "PS         " );
      SetGeoTIFFDatum( geotiff, &(ParamList->output_space_def), citation );

      GTIFKeySet(geotiff->gtif, GTCitationGeoKey, TYPE_ASCII, 0,
                 citation);
      GTIFKeySet(geotiff->gtif, GeogLinearUnitsGeoKey, TYPE_SHORT,
                 1, LinearValue);
      GTIFKeySet(geotiff->gtif, GeogAngularUnitsGeoKey, TYPE_SHORT,
                 1, Angular_Degree);
      GTIFKeySet(geotiff->gtif, ProjectedCSTypeGeoKey, TYPE_SHORT,
                 1, KvUserDefined);
      GTIFKeySet(geotiff->gtif, ProjectionGeoKey, TYPE_SHORT, 1,
                 KvUserDefined);
      GTIFKeySet(geotiff->gtif, ProjLinearUnitsGeoKey, TYPE_SHORT, 1,
                 LinearValue);
      GTIFKeySet(geotiff->gtif, ProjStraightVertPoleLongGeoKey,TYPE_DOUBLE, 
                 1, ParamList->output_space_def.orig_proj_param[4]);
      GTIFKeySet(geotiff->gtif, ProjNatOriginLatGeoKey, TYPE_DOUBLE,
                 1, ParamList->output_space_def.orig_proj_param[5]);
      GTIFKeySet(geotiff->gtif, ProjFalseEastingGeoKey, TYPE_DOUBLE,
                 1, ParamList->output_space_def.orig_proj_param[6]);
      GTIFKeySet(geotiff->gtif, ProjFalseNorthingGeoKey, TYPE_DOUBLE,
                 1, ParamList->output_space_def.orig_proj_param[7]);
      break;
    }
    case PROJ_SNSOID:
    { /* no datum */
      GTIFKeySet(geotiff->gtif, ProjCoordTransGeoKey, TYPE_SHORT, 1,
                 CT_Sinusoidal);
      GTIFKeySet(geotiff->gtif, GTModelTypeGeoKey, TYPE_SHORT, 1,
                 ModelTypeProjected );
      GTIFKeySet(geotiff->gtif, GTRasterTypeGeoKey, TYPE_SHORT, 1,
                 RasterPixelIsPoint );

      strcpy( citation, "SINUSOIDAL " );
      SetGeoTIFFSphere( geotiff, &(ParamList->output_space_def), citation );

      GTIFKeySet(geotiff->gtif, GTCitationGeoKey, TYPE_ASCII, 0,
                 citation);
      GTIFKeySet(geotiff->gtif, GeogLinearUnitsGeoKey, TYPE_SHORT, 1,
                 LinearValue);
      GTIFKeySet(geotiff->gtif, GeogAngularUnitsGeoKey, TYPE_SHORT,
                 1, Angular_Degree);
      GTIFKeySet(geotiff->gtif, ProjectedCSTypeGeoKey, TYPE_SHORT, 1,
                 KvUserDefined);
      GTIFKeySet(geotiff->gtif, ProjNatOriginLongGeoKey, TYPE_DOUBLE,
                 1, ParamList->output_space_def.orig_proj_param[4]);
      GTIFKeySet(geotiff->gtif, ProjLinearUnitsGeoKey, TYPE_SHORT, 1,
                 LinearValue);
      GTIFKeySet(geotiff->gtif, ProjFalseEastingGeoKey, TYPE_DOUBLE,
                 1, ParamList->output_space_def.orig_proj_param[6]);
      GTIFKeySet(geotiff->gtif, ProjFalseNorthingGeoKey, TYPE_DOUBLE, 
                 1, ParamList->output_space_def.orig_proj_param[7]);
      break;
    }
    default:
    {
      sprintf(errstr, "unsupported projection type %d for GeoTiff output",
        ParamList->output_space_def.proj_num);
      LOG_RETURN_ERROR(errstr, "OpenGeoTIFFFile", false);
    }

  } /* end switch(ParamList->output_space_defs.proj_num) */
      
  return true;
}


/******************************************************************************

MODULE:  SetGeoTIFFDatum

PURPOSE:  Set GeoTIFF tags for the datum to be WGS84, only if the output
          ellipsoid or projection parameters represent the WGS84 ellipsoid.

RETURN VALUE:
Type = none

HISTORY:
Version  Date   Programmer       Code  Reason
-------  -----  ---------------  ----  -------------------------------------
         11/03  Gail Schmidt           Original Development

NOTES:

******************************************************************************/
void SetGeoTIFFDatum
(
    GeoTIFFFD *geotiff,                 /* I: GeoTIFF descriptor */
    Space_def_t *outproj,               /* I:  projection info */
    char *citation                      /* I/O: Add datum citation */
)

{
    if( geotiff == NULL ) {
       return;
    }

    /* If WGS84 ellipsoid, then we can tag the WGS84 datum, otherwise no
       datum can be specified. */
    if (outproj->sphere == 8 /* WGS84 */ ||
        (outproj->proj_param[0] == 6378137.0 &&
         outproj->proj_param[1] == 6356752.31414))
    {
        strcat( citation, "WGS 1984");
        GTIFKeySet( geotiff->gtif, GeogGeodeticDatumGeoKey, TYPE_SHORT,
                    1, Datum_WGS84 );
        GTIFKeySet( geotiff->gtif, GeographicTypeGeoKey, TYPE_SHORT, 1,
                    GCS_WGS_84 );
    }
    else
    {
        SetGeoTIFFSphere( geotiff, outproj, citation );
    }
}


/******************************************************************************

MODULE:  SetGeoTIFFSphere

PURPOSE:  Set GeoTIFF tags for the sphere-based projection (no datum)

RETURN VALUE:
Type = none

HISTORY:
Version  Date   Programmer       Code  Reason
-------  -----  ---------------  ----  -------------------------------------
         11/03  Gail Schmidt           Original Development

NOTES:

******************************************************************************/
void SetGeoTIFFSphere
(
    GeoTIFFFD *geotiff,                 /* I:  GeoTIFF descriptor */
    Space_def_t *outproj,               /* I:  projection info */
    char *citation                      /* I/O:  add datum citation */
)

{
    char tmpstr[256];
    double proj_param[2];   /* local copy of first two projection params */


    /* copy the first two projection parameters */
    proj_param[0] = outproj->orig_proj_param[0];
    proj_param[1] = outproj->orig_proj_param[1];

    /* if the first two projection parameters weren't specified, then an
       ellipsoid was specified. determine the semi-major and semi-minor axis
       for the ellipsoid. */
    if (proj_param[0] < 1.0 && proj_param[1] < 1.0)
    {
        proj_param[0] = Proj_sphere[outproj->sphere].major_axis;
        proj_param[1] = Proj_sphere[outproj->sphere].minor_axis;
    }

    /* use the projection parameters for the semi-major and semi-minor axes */
    sprintf( tmpstr, "No Datum. Semi-major axis: %f, "
             "Semi-minor axis: %f", proj_param[0], proj_param[1] );
    strcat( citation, tmpstr );
    GTIFKeySet( geotiff->gtif, GeogGeodeticDatumGeoKey,
                TYPE_SHORT, 1, KvUserDefined );
    GTIFKeySet( geotiff->gtif, GeographicTypeGeoKey,
                TYPE_SHORT, 1, KvUserDefined );

    GTIFKeySet( geotiff->gtif, GeogSemiMajorAxisGeoKey,
                TYPE_DOUBLE, 1, proj_param[0] );
    if ( proj_param[1] != 0.0 )
        GTIFKeySet( geotiff->gtif, GeogSemiMinorAxisGeoKey,
                    TYPE_DOUBLE, 1, proj_param[1] );
    else
        /* If this is a sphere, use the radius for the semi-minor
           value as well. */
        GTIFKeySet( geotiff->gtif, GeogSemiMinorAxisGeoKey,
                    TYPE_DOUBLE, 1, proj_param[0] );
}

/******************************************************************************

MODULE:  CloseGeoTIFFFile

PURPOSE:  Close a TIFF

RETURN VALUE:
Type = int
Value           Description
-----           -----------
TRUE            Success  (always)
FALSE           Failure

HISTORY:
Version  Date   Programmer       Code  Reason
-------  -----  ---------------  ----  -------------------------------------
         06/00  Rob Burrell            Original Development
         01/01  John Rishea            Standardized formatting

NOTES:

******************************************************************************/
int CloseGeoTIFFFile(GeoTIFFFD *geotiff)
{
  GTIFWriteKeys(geotiff->gtif);
  GTIFFree(geotiff->gtif);
  XTIFFClose(geotiff->tif);

  return 1;
}


