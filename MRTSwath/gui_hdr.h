#ifndef GUI_WRITE_HDR
#define GUI_WRITE_HDR

typedef enum {FAILURE = 0, SUCCESS = 1} Status_t;

#define MIN_LS_DIM_SIZE 250

/* BandType struct to describe one image band */
typedef struct
{
    /* band name */
    char *name;

    /* image dimensions */
    int nlines, nsamples;

    /* data types (byte, int, etc.) */
    int datatype;

    /* rank for 2-D and 3-D data sets */
    int rank;
}
BandType;

typedef struct
{
    /* number of bands in input image */
    int nbands;

    /* array[nbands] of info about each input band */
    BandType *bandinfo;

    /* input image size (raster corner points - lat/long in decimal degrees
       in input space) */
    double input_image_ll[4][2];
} SwathDescriptor;

/* Functions */
Status_t ReadHDFFile
(
    char *hdfname,          /* I:   HDF filename */
    SwathDescriptor *P	    /* I/O: HDF swath file info */
);

Status_t WriteHDRFile
(
    char *hdrname,          /* I: HDR filename */
    SwathDescriptor *P	    /* I: HDF swath file info */
);

char *SpaceToUnderscore (char *str);
#endif
