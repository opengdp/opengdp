#include "geowrpr.h"
#include "GeoS2G.h"
#include "param.h"
#include <tiffio.h>
#include <stdlib.h>
#include <string.h>

FILE_ID *Open_GEOTIFF( void *params ) {
   FILE_ID *fid = new_FILE_ID();

   if( ! fid )
      return fid;

   fid->ftype = FILE_GEOTIFF_FILETYPE;
   fid->error = 0;
   fid->error_msg[0] = 0;
   fid->fptr = malloc( sizeof(GeoTIFFFD) );
   if( ! fid->fptr ) {
      fid->error = FILE_ERROR_ALLOC;
      strcpy( fid->error_msg, "allocating GeoTiff structure" );
      return fid;
   }

   GeoTIFFFD *gfid = (GeoTIFFFD *)fid->fptr;
   Param_t *ParamList = (Param_t *)params;
   if( !OpenGeoTIFFFile( ParamList, gfid ) ) {
      fid->error = 2;
      strcpy( fid->error_msg, "opening and initializing GeoTiff file" );
   }

   return fid;
}

void Close_GEOTIFF( FILE_ID *fid ) {
   if( fid && fid->ftype == FILE_GEOTIFF_FILETYPE ) {
      GeoTIFFFD *gfid = (GeoTIFFFD *)fid->fptr;
      CloseGeoTIFFFile(gfid);
      if( fid->fptr )
         free( fid->fptr );
      delete_FILE_ID(&fid);
   }
}

int GEOTIFF_WriteScanline(FILE_ID *fid, void *data, void *flag, void *sample) {
   int result = 0;
   if( fid && fid->ftype == FILE_GEOTIFF_FILETYPE ) {
      GeoTIFFFD *gfid = (GeoTIFFFD *)fid->fptr;
      uint32 *iflag = (uint32 *)flag;
      uint16 *isample = (uint16 *)sample;

      result = TIFFWriteScanline(gfid->tif, data, *iflag, *isample);
   }
   return result;
}


