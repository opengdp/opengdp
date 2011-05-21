
#include "geowrpr.h"
#include <stdlib.h>

FILE_ID *new_FILE_ID(void) {

   FILE_ID *fid = (FILE_ID *)malloc( sizeof(FILE_ID) );

   if( fid == NULL )
      return fid;

   fid->ftype = FILE_NO_FILETYPE;
   fid->error = 0;
   fid->error_msg[0] = 0;
   fid->fptr = NULL;

   return fid;
}

void delete_FILE_ID(FILE_ID **fid) {
   if( fid && *fid ) {
      free( *fid );
      *fid = NULL;
   }
}


