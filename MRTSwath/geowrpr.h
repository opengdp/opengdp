#ifndef GEOWRPR_H
#define GEOWRPR_H

/* The reason for this wrapper is because there were some conflicts between
 * the HDF and geoTIFF header files.  This is a simple attempt to
 * seperate out the geoTIFF headers from the HDF headers.  A better solution
 * could be provided, but time was limited.
 */

typedef enum { FILE_NO_FILETYPE,
               FILE_HDF_FILETYPE,
               FILE_GEOTIFF_FILETYPE,
               FILE_RB_FILETYPE } FileType_t;

#define FILE_ERROR_ALLOC  1
#define FILE_ERROR_LEN 1023

typedef struct {
   FileType_t ftype;
   int error;
   char error_msg[FILE_ERROR_LEN+1];
   void *fptr;
} FILE_ID;

/*
 * For some reason, maybe due to a bug, when using the Microsoft's compiler,
 * when compiled as a .c file, it complains about a simple cast and stops
 * with an error.  When compiled as a .cpp file, it compiles fine.  On unix
 * systems then, we use filegeo.c.  For Windows, we use filegeo.cpp.  They
 * should be exactly the same in content and they both share this header file
 * and thus the extern "C" defines... to tell C++ not to mangle the function
 * names.  The "bug" seems to be with Visual C/C++ 2008.
 */

#ifdef __cplusplus
extern "C" {
#endif

FILE_ID *new_FILE_ID();
void     delete_FILE_ID(FILE_ID **fid);

FILE_ID *Open_GEOTIFF( void *ParamList );
int GEOTIFF_WriteScanline(FILE_ID *, void *data, void *flag, void *sample);
void Close_GEOTIFF( FILE_ID *fid );

#ifdef __cplusplus
}
#endif

#endif

