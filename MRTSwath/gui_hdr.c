/******************************************************************************

FUNCTION:  gui_hdr.c

PURPOSE:  Read the HDF file and provide metadata information to an HDR file
          for ingest by the GUI

HISTORY:
Version  Date   Programmer       Code  Reason
-------  -----  ---------------  ----  -------------------------------------
         03/04  Gail Schmidt           Original Development

HARDWARE AND/OR SOFTWARE LIMITATIONS:  
  None

PROJECT:    MODIS Swath Reprojection Tool

NOTES:

******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "myhdf.h"
#include "gui_hdr.h"
#include "geoloc.h"
#include "myendian.h"
#include "myisoc.h"

/******************************************************************************

FUNCTION:  ReadHDFFile

PURPOSE:  Finds the existing SDSs in the input file and grabs the band
          information for each SDS.

HISTORY:
Version  Date   Programmer       Code  Reason
-------  -----  ---------------  ----  -------------------------------------
         03/04  Gail Schmidt           Original Development

HARDWARE AND/OR SOFTWARE LIMITATIONS:  
  None

PROJECT:    MODIS Swath Reprojection Tool

NOTES:

******************************************************************************/
Status_t ReadHDFFile
(
    char *hdfname,          /* I:   HDF filename */
    SwathDescriptor *this   /* I/O: HDF swath file info */
)

{
    int i, j;
    int sds_nbands;              /* number of bands in this SDS */
    int nlines, nsamps;          /* number of lines and samples in SDS */
    int ires;                    /* resolution (1=1km, 2=500m, 4=250m) */
    int32 sd_fid, sds_id;        /* file id and sds id */
    int32 nsds;                  /* number of SDSs in the file */
    int32 nattr;                 /* number of attributes for the SDS */
    int32 data_type;             /* data type of the SDS */
    char sds_name[MAX_NC_NAME];  /* SDS name */
    char tmpstr[256];            /* temporary string for the SDS_band name */
    int32 rank;                  /* rank of the SDS */
    int32 dims[MYHDF_MAX_RANK];  /* dimensions of the SDS */
    Geo_coord_t ul_corner;       /* UL lat/long */
    Geo_coord_t lr_corner;       /* LR lat/long */

    /* Read the BOUNDING coords for the corner points */
    if (!ReadBoundCoords (hdfname, &ul_corner, &lr_corner))
    {
/*        printf("Error reading BOUNDING COORDS from metadata. "
          "Therefore, in order to process this data, the output spatial "
          "subsetting will need to be specified.\n");
*/
        ul_corner.lat = 0.0;
        ul_corner.lon = 0.0;
        lr_corner.lat = 0.0;
        lr_corner.lon = 0.0;
    }

    /* Write the lat/long values for each corner point */
    /* UL */
    this->input_image_ll[0][0] = ul_corner.lat;
    this->input_image_ll[0][1] = ul_corner.lon;

    /* UR */
    this->input_image_ll[1][0] = ul_corner.lat;
    this->input_image_ll[1][1] = lr_corner.lon;

    /* LL */
    this->input_image_ll[2][0] = lr_corner.lat;
    this->input_image_ll[2][1] = ul_corner.lon;

    /* LR */
    this->input_image_ll[3][0] = lr_corner.lat;
    this->input_image_ll[3][1] = lr_corner.lon;

    /* Open file for SD read access */
    sd_fid = SDstart (hdfname, DFACC_RDONLY);
    if (sd_fid == HDF_ERROR)
    {
        printf ("Error: Unable to open %s\n", hdfname);
        return FAILURE;
    }

    /* Get the list of swaths */
    SDfileinfo (sd_fid, &nsds, &nattr);

    /* Allocate space for 500 bands */
    this->bandinfo = calloc (500, sizeof (BandType));
    if (this->bandinfo == NULL)
    {
        printf ("Error: Allocating space for 500 bands in SwathDescriptor\n");
        return FAILURE;
    }

    /* Loop through the SDSs to get SDS names and info */
    this->nbands = 0;
    for (i = 0; i < nsds; i++)
    {
        /* grab the SDS */
        sds_id = SDselect (sd_fid, i);
        if (sds_id == HDF_ERROR)
        {
            printf ("Error selecting input SDS %d\n", i);
            return FAILURE;
        }

        /* get information for the current SDS */
        if (SDgetinfo (sds_id, sds_name, &rank, dims, &data_type, &nattr) ==
            HDF_ERROR)
        {
            printf ("Error getting SDS information %d\n", i);
            return FAILURE;
        }
/*	printf ("sds_name is %s, rank is %d, dims are %d/%d\n", sds_name,
          rank, dims[0], dims[1]);
	printf ("MIN_LS_DIM_SIZE is %d\n", MIN_LS_DIM_SIZE);
	printf ("NFRAME_1KM_MODIS is %d\n", NFRAME_1KM_MODIS);
*/

        /* determine the band names, number of lines, number of samples,
           and data types for each SDS/band combo */
        if (rank == 1)
        {
            /* don't process this SDS */
        }
        else if (rank == 2)
        {
            /* if the rank is 2 then there is only one band to process */

            /* determine the resolution of the product using the number of
               samples and the nominal scan size */
            ires = (int)((dims[1] / (double) NFRAME_1KM_MODIS) + 0.5);

            /* if the data type is not char8, int8, uint8, int16, or uint16,
               then it won't be processed by the resampler so don't add it
               to the list of bands */
            if (data_type == DFNT_CHAR8 || data_type == DFNT_INT8 ||
                data_type == DFNT_UINT8 || data_type == DFNT_INT16 ||
                data_type == DFNT_UINT16 || data_type == DFNT_INT32 ||
                data_type == DFNT_UINT32 )
            {
                /* if the lines and samples aren't less than MIN_LS_DIM_SIZE
                   then don't process the SDS. if the ires is not 1 (1km),
                   2 (500m), or 4 (250m) then don't process the SDS. */
                if (dims[0] >= MIN_LS_DIM_SIZE &&
                    dims[1] >= MIN_LS_DIM_SIZE &&
                    (ires == 1 || ires == 2 || ires == 4))
                {
                    this->bandinfo[this->nbands].name = strdup (sds_name);
                    this->bandinfo[this->nbands].datatype = data_type;
                    this->bandinfo[this->nbands].nlines = dims[0];
                    this->bandinfo[this->nbands].nsamples = dims[1];
                    (this->nbands)++;
                }
            }
        }
        else if (rank == 3)
        {
            /* 3D product - use the first dim value for the bands, second,
               for lines and third for samples */
            sds_nbands = (int) dims[0];
            nlines = (int) dims[1];
            nsamps = (int) dims[2];

            /* determine the resolution of the product using the number of
               samples and the nominal scan size */
            ires = (int)((nsamps / (double) NFRAME_1KM_MODIS) + 0.5);

            /* if the data type is not char8, int8, uint8, int16, or uint16,
               then it won't be processed by the resampler so don't add it
               to the list of bands */
            if (data_type == DFNT_CHAR8 || data_type == DFNT_INT8 ||
                data_type == DFNT_UINT8 || data_type == DFNT_INT16 ||
                data_type == DFNT_UINT16 || data_type == DFNT_INT32 ||
                data_type == DFNT_UINT32 )
            {
                /* if the smallest dimension is larger than MAX_VAR_DIMS then
                   don't process the SDS. if the lines and samples are less
                   than MIN_LS_DIM_SIZE then don't process the SDS. if the
                   ires is not 1 (1km), 2 (500m), or 4 (250m) then don't
                   process the SDS. */
                if (sds_nbands <= MAX_VAR_DIMS &&
                    nlines >= MIN_LS_DIM_SIZE &&
                    nsamps >= MIN_LS_DIM_SIZE &&
                    (ires == 1 || ires == 2 || ires == 4))
                {
                    /* loop through the bands and determine the SDS/band name
                       and other information */
                    for (j = 0; j < sds_nbands; j++)
                    {
                        sprintf (tmpstr, "%s_b%d", sds_name, j);
                        this->bandinfo[this->nbands].name = strdup (tmpstr);
                        this->bandinfo[this->nbands].datatype = data_type;
                        this->bandinfo[this->nbands].nlines = nlines;
                        this->bandinfo[this->nbands].nsamples = nsamps;
                        (this->nbands)++;
                    }
                }
            }
        }
        else
        {
            /* only process 2D and 3D products */
            printf ("Warning: This software only support 2D and 3D SDSs\n");
        }

        /* close the SDS */
        SDendaccess (sds_id);
    }

    /* Close the HDF-EOS file */
    SDend (sd_fid);

    return SUCCESS;
}

/******************************************************************************

FUNCTION:  WriteHeaderFile

PURPOSE:  Write raw binary headers for reading by the GUI

HISTORY:
Version  Date   Programmer       Code  Reason
-------  -----  ---------------  ----  -------------------------------------
         03/04  Gail Schmidt           Original Development

HARDWARE AND/OR SOFTWARE LIMITATIONS:  
  None

PROJECT:    MODIS Swath Reprojection Tool

NOTES:

******************************************************************************/
Status_t WriteHDRFile
(
    char *hdrname,          /* I: HDR filename */
    SwathDescriptor *this   /* I: HDF swath file info */
)

{
    FILE *fp = NULL;            /* output file pointer */
    char str[256];              /* temp string */
    char endianstr[256];        /* temp endian string */
    int i;                      /* index */
    double west_coord;          /* west-bounding corner coordinate */
    double east_coord;          /* east-bounding corner coordinate */
    double north_coord;         /* north-bounding corner coordinate */
    double south_coord;         /* south-bounding corner coordinate */
    MrtSwathEndianness machineEndianness = GetMachineEndianness();

    strcpy( endianstr, "BYTE_ORDER = " );
    if( machineEndianness == MRTSWATH_BIG_ENDIAN ) {
       strcat( endianstr, "big_endian\n" );
    } else if ( machineEndianness == MRTSWATH_LITTLE_ENDIAN ) {
       strcat( endianstr, "little_endian\n" );
    } else {
       printf("Error: Unable to determine machine endianness");
       return FAILURE;
    }

    /* open header file for writing */
    fp = fopen (hdrname, "w");
    if (fp == NULL)
    {
        printf ("Error: Unable to open %s", hdrname);
	return FAILURE;
    }

    /* determine the bounding extents of the corner points */
    west_coord = this->input_image_ll[0][1];
    east_coord = this->input_image_ll[3][1];
    for (i = 0; i < 4; i++)
    {
        /* Find western boundary */
        if (this->input_image_ll[i][1] < west_coord)
            west_coord = this->input_image_ll[i][1];

        /* Find eastern boundary */
        if (this->input_image_ll[i][1] > east_coord)
            east_coord = this->input_image_ll[i][1];
    }

    north_coord = this->input_image_ll[0][0];
    south_coord = this->input_image_ll[3][0];
    for (i = 0; i < 4; i++)
    {
        /* Find southern boundary */
        if (this->input_image_ll[i][0] < south_coord)
            south_coord = this->input_image_ll[i][0];

        /* Find northern boundary */
        if (this->input_image_ll[i][0] > north_coord)
            north_coord = this->input_image_ll[i][0];
    }

    /* write spatial extents (lat/lon)
       UL_CORNER_LATLON = (ULlat ULlon)
       UR_CORNER_LATLON = (URlat URlon)
       LL_CORNER_LATLON = (LLlat LLlon)
       LR_CORNER_LATLON = (LRlat LRlon) */

    fprintf (fp, "\nUL_CORNER_LATLON = ( %.9f %.9f )\n",
        north_coord, west_coord);
/*        this->input_image_ll[0][0], this->input_image_ll[0][1]); */

    fprintf (fp, "UR_CORNER_LATLON = ( %.9f %.9f )\n",
        north_coord, east_coord);
/*        this->input_image_ll[1][0], this->input_image_ll[1][1]); */

    fprintf (fp, "LL_CORNER_LATLON = ( %.9f %.9f )\n",
        south_coord, west_coord);
/*        this->input_image_ll[2][0], this->input_image_ll[2][1]); */

    fprintf (fp, "LR_CORNER_LATLON = ( %.9f %.9f )\n",
        south_coord, east_coord);
/*        this->input_image_ll[3][0], this->input_image_ll[3][1]); */

    /* write total number of bands: NBANDS = n */
    fprintf (fp, "\nNBANDS = %i\n", this->nbands);

    /* write band names */
    fprintf (fp, "BANDNAMES = (");
    for (i = 0; i < this->nbands; i++)
    {
        strcpy( str, this->bandinfo[i].name);
        SpaceToUnderscore (str);
        fprintf (fp, " %s", str);
    }
    fprintf (fp, " )\n");

    /* write data type: DATA_TYPE = ... */
    fprintf (fp, "DATA_TYPE = (");
    for (i = 0; i < this->nbands; i++)
    {
        switch (this->bandinfo[i].datatype)
        {
            case DFNT_INT8:
                fprintf (fp, " INT8");
                break;
            case DFNT_UINT8:
                fprintf (fp, " UINT8");
                break;
            case DFNT_INT16:
                fprintf (fp, " INT16");
                break;
            case DFNT_UINT16:
                fprintf (fp, " UINT16");
                break;
            case DFNT_INT32:
                fprintf (fp, " INT32");
                break;
            case DFNT_UINT32:
                fprintf (fp, " UINT32");
                break;
            case DFNT_FLOAT32:
                fprintf (fp, " FLOAT32");
                break;
            default:
                printf ("Error: Bad data type\n");
                return FAILURE;
        }
    }
    fprintf (fp, " )\n");

    /* write NLINES field */
    fprintf (fp, "NLINES = (");
    for (i = 0; i < this->nbands; i++)
    {
        fprintf (fp, " %i", this->bandinfo[i].nlines);
    }
    fprintf (fp, " )\n");

    /* write NSAMPLES field */
    fprintf (fp, "NSAMPLES = (");
    for (i = 0; i < this->nbands; i++)
    {
        fprintf (fp, " %i", this->bandinfo[i].nsamples);
    }
    fprintf (fp, " )\n");

    /* write BYTE_ORDER field */
    fprintf( fp,  endianstr );

    /* finish up */
    fclose (fp);
    return SUCCESS;
}

/******************************************************************************

FUNCTION:  SpaceToUnderscore

PURPOSE:  Convert all white space characters in a string to underscores.

HISTORY:
Version  Date   Programmer       Code  Reason
-------  -----  ---------------  ----  -------------------------------------
         03/04  Gail Schmidt           Original Development

HARDWARE AND/OR SOFTWARE LIMITATIONS:  
  None

PROJECT:    MODIS Swath Reprojection Tool

NOTES:

******************************************************************************/
char *SpaceToUnderscore (char *str)
{
    char *s = str;

    while (*s)
    {
        if (isspace ((int) *s))
            *s = '_';
        s++;
    }

    return str;
}
