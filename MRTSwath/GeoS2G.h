#ifndef GEOTIFFDEF
#define GEOTIFFDEF

/** Include Files **/
/*** Include files from GeoTiff & Swath2Grid ***/
/* Geotiff */
#include "geotiffio.h"
#include "xtiffio.h"

/* Swath2Grid */
#include "param.h"
#include "myproj.h"

#include <ctype.h>

/** GeoTIFF FilePointer Mem **/
typedef struct {
  TIFF *tif;
  GTIF *gtif;
} GeoTIFFFD;

/** Function Prototypes **/
#ifdef __cplusplus
extern "C" {
#endif

bool OpenGeoTIFFFile (Param_t *ParamList, GeoTIFFFD *MasterGeoMem);
int CloseGeoTIFFFile(GeoTIFFFD *geotiff);

#ifdef __cplusplus
}
#endif

#endif
