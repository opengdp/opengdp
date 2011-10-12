/******************************************************************************
 Program to destripe modis data
 
 Copyright(C) 2004, University of Wisconsin-Madison MODIS Group
 Created by Liam.Gumley@ssec.wisc.edu

   Developers:
   ----------
   Nazmi Saleous (adapted code to 1KM input)
   MODIS Land Science Team           
   Raytheon Systems 
   NASA's GSFC Code 923 
   Greenbelt, MD 20771
   phone: 301-614-6647
   nazmi.saleous@gsfc.nasa.gov      

   Liam Gumley (developer of original MOD_PRDS)
   CIMSS/SSEC
   1225 W. Dayton St.
   Madison WI 53706
   (608) 265-5358
   Liam.Gumley@ssec.wisc.edu

   SDST Contacts:
   -------------
   Carol Davidson	301-352-2159	cdavidson@saicmodis.com
   Gwyn Fireman		301-352-2118    gfireman@saicmodis.com
   SAIC-GSO MODIS SCIENCE DATA SUPPORT OFFICE
   7501 Forbes Boulevard, Suite 103, Seabrook, MD 20706
   Fax: 301-352-0143


 ported to C by Brian Case
 rush@winkey.org

******************************************************************************/

#include <stdio.h>
#include "hdf.h"
#include "mfhdf.h"
#include "df.h"
#include "modis_edf_destripe.h"

/******************************************************************************
    constants
******************************************************************************/

#define LINES_PER_SCAN_1km 10
#define NBANDS_1km 36

#define LINES_PER_SCAN_500m 20
#define NBANDS_500m 7

#define terrafile1k "MOD021KM_destripe_config.dat"
#define aquafile1k "MYD021KM_destripe_config.dat"
#define terrafile500 "MOD02HKM_destripe_config.dat"
#define aquafile500 "MYD02HKM_destripe_config.dat"

#define TESTBANDS1k "EV_1KM_Emissive"
#define TESTBANDS500 "EV_500_RefSB"

#define rcsid "$Id: hdf_destripe_new.f90,v 1.8 2004/06/24 14:25:27 gumley Exp $"


/******************************************************************************
 macros to print hdf errors
******************************************************************************/

#define HDFERROR(x) {\
  fprintf(stderr, "ERROR:\n      file: %s\n      line: %d\n      function:  %s\n",\
	        __FILE__, __LINE__, (x));\
  HEprint(stderr, 0);\
}

#define HDFWARN(x) {\
  fprintf(stderr, "WARNING:\n      file: %s\n      line: %d\n      function:  %s\n",\
	        __FILE__, __LINE__, (x));\
  HEprint(stderr, 0);\
}

/****************************************************************************//**

Macro to Get index within a SDS for a given band in a MODIS 1KM radiance
HDF file

@param band     the band number to look up 1 - 36

@return         the hdf sds_index

EV_250_Aggr1km_RefSB and
EV_250_RefSB contains bands    1,2

EV_500_Aggr1km_RefSB and
EV_500_RefSB contains bands    3,4,5,6,7

EV_1KM_RefSB contains bands    8,9,10,11,12,13lo,13hi,14lo,14hi,15,16,17,18,19,26

EV_1KM_Emissive contains bands 20,21,22,23,24,25,27,28,29,30,31,32,33,34,35,36

******************************************************************************/

#define get_band_index(band) band_index[band - 1]


static int band_index[36] = {
  0, 1, 
  0, 1, 2, 3, 4,
  0, 1, 2, 3, 4, 5, 7, 9, 10, 11, 12, 13,
  0, 1, 2, 3, 4, 5, 14, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};


/****************************************************************************//**

Macro to to emulate a 2d array with a 1d array

@param array    the array to operate on
@param x        the x coordinate in the array
@param y        the y coordinate in the array
@param width    the width of the array on the x axis

@return         the contents of the array

note: this can be used on the left or right side of assignment

******************************************************************************/

#define get2darray(array, x, y, width)    array[ (x) + (y) * (width) ]

/******************************************************************************
 prototypes;
 
******************************************************************************/

int get_band_config(char *pathname, int32 stripsize, int nBands, int *band_data, char *header);

int get_modis_mirror( char *in_file, int32 *mirror_side );

int32 get_sds_index (int32 hdfid, int band, int bDo1k);

int setdims (int32 sds_id, int32 sub_sds, int32 *start, int32 *stride, int32 *edge);

void rep_bad_det( int band, int32 nPixel, int32 nScan, int32 stripsize,
                  int *band_data, int16 *image);



void usage(char *app) {

    fprintf (stderr, "Usage:\n");
    fprintf (stderr, "    %s <infile.hdf> <-terra | -aqua> <-1km | -500m>\n", app);

    exit (EXIT_FAILURE);

}

/****************************************************************************//**

main to Destripe a MODIS L1B 1KM HDF file.

@param argc     2
@param argv     appname <infile.hdf> <-terra | -aqua> <-1km | -500m>\n""

@return EXIT_SUCCESS or EXIT_FAILURE

NOTE: The input file is ireversibly modified

fixme: all arguments but the input file can be derived from the input file.
ASSOCIATEDPLATFORMSHORTNAME seems to contain Aqua or Terra,
and 1km of 500m can be derived by testing for sds "EV_1KM_Emissive" or 
"EV_500_RefSB"

******************************************************************************/


int main (int argc, char *argv[]) {


    /***** read args *****/

    int bDo1k = 0;
    int bDo500 = 0;
    int bDoterra = 0;
    int bDoaqua = 0;
    char *in_file = NULL;
    
    if ( argc < 3)
        usage(argv[0]);

    int arg;

    for (arg = 1; arg < argc; arg++) {
        
        if      ( ! strcasecmp(argv[arg], "-1km") )
            bDo1k = 1;
        else if ( ! strcasecmp(argv[arg], "-500m") )
            bDo500 = 1;
        else if ( ! strcasecmp(argv[arg], "-terra") )
            bDoterra = 1;
        else if ( ! strcasecmp(argv[arg], "-aqua") )
            bDoaqua = 1;
        else if ( *(argv[arg]) == '-' )
            fprintf(stderr, "Warning: unrecognised switch %s\n", argv[arg] );
        else
            in_file = argv[arg];
    }

    if ( !(bDo1k ^ bDo500) || !(bDoterra ^ bDoaqua) || !in_file )
        usage(argv[0]);
    
    char *configfile;
    int32 stripsize;
    int nBands;
    char *testbands;
    
    if ( bDo1k ) {
        stripsize = LINES_PER_SCAN_1km;
        nBands = NBANDS_1km;
        testbands = TESTBANDS1k;
        if ( bDoterra )
            configfile = terrafile1k;
        else
            configfile = aquafile1k;
    }
    else {
        stripsize=LINES_PER_SCAN_500m;
        nBands = NBANDS_500m;
        testbands = TESTBANDS500;
        if ( bDoterra )
            configfile = terrafile500;
        else
            configfile = aquafile500;
    }

    /***** open the hdf file *****/
    
    int32 hdfid, sds_id, attid;
    
    hdfid = SDstart(in_file, DFACC_RDWR);
    if (hdfid == FAIL) {
        fprintf(stderr, "Error opening MODIS L1B 1KM input file %s\n", in_file);
        HDFERROR("");
        exit(EXIT_FAILURE);
    }


    /***** Select the 1KM emissive band SDS *****/

    sds_id = SDselect(hdfid, SDnametoindex(hdfid, testbands ));

    if (sds_id == FAIL) {
        fprintf(stderr, "MODIS %s were not found in input file.\n", testbands);
        exit(EXIT_FAILURE);
    }
    
    /***** Check if the file is already destriped *****/
    
    attid = SDfindattr(hdfid, "UW_DESTRIPE");

    if (attid != FAIL) {
        fprintf(stderr, "MODIS input file is already destriped.\n");
        exit(EXIT_FAILURE);
    }
        
    int32 mystart[MAX_VAR_DIMS] = {0};
    int32 mystride[MAX_VAR_DIMS] = {0};
    int32 myedge[MAX_VAR_DIMS] = {0};
        
    setdims (sds_id, 0, mystart, mystride, myedge);

    /***** Get the number of pixels, lines, and scans *****/
    
    int32 nPixel = myedge[2];
    int32 num_lines  = myedge[1];
    int32 nScan  = num_lines / stripsize;
    printf("imagesize %i\n", nPixel * num_lines );
    printf( "nPixel %i num_lines %i nScan %i\n", nPixel, num_lines, nScan );

    if (nScan <= 1) {
        fprintf(stderr, "Number of MODIS L1B scans is <= 1.\n");
        exit(EXIT_FAILURE);
    }

  
    /***** Allocate runtime arrays *****/

    int16 *destripe;
    int16 *buffer;
    
    if (! (destripe = malloc(nPixel * num_lines * sizeof (int16)))) {
        fprintf(stderr, "Error allocating memory for destripe.\n");
        exit(EXIT_FAILURE);
    } 
    
    if (! (buffer = malloc(nPixel * num_lines * sizeof (int16)))) {
        fprintf(stderr, "Error allocating memory for buffer.\n");
        free(destripe);
        exit(EXIT_FAILURE);
    } 
    
    /***** Get mirror side data *****/

    int32 *mirror_side;
        
    if (! (mirror_side = malloc(nScan * sizeof (int32)))) {
        fprintf(stderr, "Error allocating memory for mirror_side.\n");
        free(destripe);
        free(buffer);
        exit(EXIT_FAILURE);
    } 
    
    if ( 0 > get_modis_mirror(in_file, mirror_side) ) {
        fprintf(stderr, "Error reading MODIS L1B mirror side data.\n");
        free(destripe);
        free(buffer);
        free(mirror_side);
        exit(EXIT_FAILURE);
    }
    
    /***** Get MODIS destriping band configuration *****/
    
    char header[132];

    int *band_data = calloc ( (stripsize + 2) * nBands, sizeof(int));
    if (!band_data ) {
        fprintf(stderr,"Error alocateing memmory for destriping configuration data.\n");
        free(destripe);
        free(buffer);
        free(mirror_side);
        exit(EXIT_FAILURE);
    }

    size_t need = snprintf (NULL, 0, "%s/%s", MOD_PRDS_DATA_DIR, configfile);
    char *config = malloc ((need + 1) * sizeof (char));
    if (!config) {
        fprintf(stderr,"Error alocateing memmory for destriping configuration filename.\n");
        free(destripe);
        free(buffer);
        free(mirror_side);
        free(band_data);
        exit(EXIT_FAILURE);
    }
    sprintf (config, "%s/%s", MOD_PRDS_DATA_DIR, configfile);

    if ( 0 > get_band_config(config, stripsize, nBands, band_data, header) ) {
        fprintf(stderr,"Error reading MODIS destriping configuration file.\n");
        free(destripe);
        free(buffer);
        free(mirror_side);
        free(band_data);
        free(config);
        exit(EXIT_FAILURE);
    }

    free(config);
    
    int band_loop, band;

    int32 ref_det;
    int errflag = 0;
    int det_index, min_diff, i, det_diff, rep_index;

     
    /***** Loop over each band to be destriped *****/
    
    for ( band_loop = 1 ; band_loop <= nBands ; band_loop++) {

        /***** Get band number *****/

        band = get2darray( band_data, 0, band_loop - 1, (stripsize + 2));
        if (band == 0)
            continue;

        /***** get the sds index for the band *****/
        
        int32 sds_index = get_sds_index (hdfid, band, bDo1k);
        if (sds_index == FAIL)
            continue;
        
        /***** get the sds_id for the band *****/

        sds_id = SDselect(hdfid, sds_index);
        if (sds_id == FAIL) {
            fprintf(stderr, "Warning: failed to find the sds_id for band %i \n",
                    band);
            HDFWARN("");
            continue;
        }
        
        /***** Get band index in SDS array for this band *****/

        //start[0] = get_band_index(band);

        int32 start[MAX_VAR_DIMS] = {0};
        int32 stride[MAX_VAR_DIMS] = {0};
        int32 edge[MAX_VAR_DIMS] = {0};
        
        setdims (sds_id, get_band_index(band), start, stride, edge);

        /***** Read the input image for this band *****/
        
        intn retn;
        retn = SDreaddata(sds_id, start, stride, edge, (VOIDP)buffer);
        
        if (retn == FAIL) {
            fprintf(stderr,
                    "Warning: failed to read data for band %i sds index %i sds dim %i\n",
                    band, sds_index, start[2] );
            HDFWARN("");
            SDendaccess(sds_id);
            continue;
        }
        
        /***** Get reference detector for this band *****/
        
        ref_det = get2darray( band_data, 1, band_loop - 1, (stripsize + 2));
          
        /***** Compute destriped image *****/
        
        errflag = modis_edf_destripe( nPixel, nScan, stripsize,
                            ref_det, mirror_side,
                            buffer, destripe);


        if (errflag != 0) {
            fprintf(stderr, "Could not destripe band %d\n", band);
            SDendaccess(sds_id);
            continue;
        }

        /***** Replace bad detectors with nearest good neighbor *****/

        rep_bad_det( band, nPixel, nScan, stripsize, band_data, destripe);
            
        /***** write the destriped image for this band *****/
            
        SDwritedata(sds_id, start, NULL, edge, destripe);
            
        SDendaccess(sds_id);
    }
    
    /***** Write a new global attribute to show this file is destriped *****/
    
    SDsetattr(hdfid, "UW_DESTRIPE", DFNT_CHAR8, strlen(rcsid), rcsid);

    /***** Write a new global attribute to record destriping configuration *****/
    
    SDsetattr(hdfid, "UW_DESTRIPE_CONFIG", DFNT_CHAR8, strlen(header), header);

    /***** Close the input file *****/
    
    SDend(hdfid);

    /***** Deallocate the input and destriped image arrays *****/
    
    //free(image);
    free(destripe);
    free(buffer);
    free(mirror_side);
    free(band_data);
        
    /***** Print progress message to logfile *****/
    
    //write(errtext, '(''Successfully destriped MODIS L1B 1KM file: '', a)') in_file
    //call message(pgename, errtext, 0, 3)

    /***** Return exit code zero to shell *****/
    

    return (EXIT_SUCCESS);
}

/****************************************************************************//**

Get scan mirror data from a MODIS l1b HDF file.

@param in_file      Name of the MODIS l1b HDF file.
@param mirror_side  Pointer to the array to store the mirror side data in.

@return 0 on success, -1 on failure

mirror side values are 0 or 1

******************************************************************************/

int get_modis_mirror( char *in_file, int32 *mirror_side ) {

    /***** Local variables *****/
    int16 num_dds_block = 0;
    int32 file_id, rtn;
    int32 vdata_ref, vdata_id;
    
    int32 nScan, record_index;

    /***** Open the input HDF file for read only (DFACC_READ is defined in hdf.inc)  *****/

    file_id = Hopen(in_file, DFACC_READ, num_dds_block);
    if (file_id == -1)
        return -1;
      
    /***** Start vdata interface *****/
    
    rtn = Vstart(file_id);

    /***** Get reference number of vdata containing scan type and mirror side *****/
    
    vdata_ref = VSfind(file_id, "Level 1B Swath Metadata");
    if (vdata_ref == 0)
        return -2;

    /***** Attach to vdata *****/

    vdata_id = VSattach(file_id, vdata_ref, "r");

    /***** Get number of records (scans) *****/
    
    nScan = VSelts(vdata_id);
    
    /***** Read mirror side data *****/

    record_index = 0;
    rtn = VSseek(vdata_id, record_index);
    rtn = VSsetfields(vdata_id, "Mirror Side");
    rtn = VSread(vdata_id, (uint8 *)mirror_side, nScan, FULL_INTERLACE);
    
    /***** Detach from vdata *****/

    rtn = VSdetach(vdata_id);
      
    /***** End vdata interface *****/
    
    rtn = Vend(file_id);
      
    /***** Close the input file *****/

    rtn = Hclose(file_id);

    return 0;
}

/******************************************************************************

Read MODIS destriping band configuration file

@param pathname     name of thew config file to read the config from
@param stripsize    size of a modis strip (10 for 1km, 20 for 500m)
@param nBands       number of bands to read config for (36 for 1km, 7 for 500m)
@param band_data    pointer to the array to return the band data in
@param header       poiner to char array to return the header in

@return 0 on success, -1 on failure

config file shoule be all comma seperated ints.
band number, referance det, followed by stripsize number of 0 or 1 for each
detector (0 good, 1 bad).

fixme: this function could probably read the "Noisy Detector List" from the
HDF file. for instance band 5 ( the third 500m band ) seems to always have
detector 4 marked as noisy and it results in some dad stripes across the image.

******************************************************************************/

int get_band_config(char *pathname, int32 stripsize, int nBands, int *band_data, char *header) {

    /***** Local variables *****/
    
    char buf[100] = {};

    FILE *fp = NULL;

    /***** open the file *****/
     
    if (! (fp = fopen(pathname, "r") ))
        return -1;

    /***** init the band_data array *****/

    memset(band_data, 0, (stripsize + 2) * nBands * sizeof(int));

    /***** Read the header line *****/

    if (! fgets (header, 99, fp)) {
        fclose(fp);
        return -1;
    }
    *buf = 0;
    
    /***** Read all remaining lines from the input file *****/
    
    while (fgets (buf, 99, fp ) && *buf ) {
        
        int row;
        sscanf(buf, "%d", &row);
        if (row >= nBands)
            continue;
        row--;

        int used;
        char *tmp = buf;
        if ( 2 != sscanf( tmp, "%d, %d%n",
                          band_data + 0 + row * (stripsize + 2),
                          band_data + 1 + row * (stripsize + 2),
                          &used
                        ))
        {
            return -1;
        }
        tmp = tmp + used;

        int32 iDet;
        for ( iDet = 0; iDet < stripsize; iDet++) {
            if ( 1 != sscanf( tmp, ",%d%n",
                              band_data + 2 + iDet + row * (stripsize + 2),
                              &used
                            ))
            {
                return -1;
            }
            tmp = tmp + used;

        }
        
        *buf = '\0';
    }   


#ifdef DEBUG
    for (i = 0; i < nBands; i++) {
        int j;
        for (j = 0; j < (stripsize + 2); j++) {
            printf( "%i ", get2darray( band_data, j, i, (stripsize + 2)) );
        }
        printf("\n");
    }
#endif
    
    /***** Close input file *****/

    fclose(fp);

    return 0;
}


/****************************************************************************//**

get the hdf sds_index for a band

@param hdfid    hdf id to read data from
@param band     band number (1-36 for 1km, 1-7 for 500m)
@param bDo1k    0 for 500m, 1 for 1km

@return sds_index or FAIL

******************************************************************************/

int32 get_sds_index (int32 hdfid, int band, int bDo1k) {

    char *sdsname = NULL;

    if (bDo1k) {
        switch (band) {
            case 1:  case 2:
                sdsname="EV_250_Aggr1km_RefSB";
                break;
            
            case 3:  case 4:  case 5:  case 6:  case 7:
                sdsname="EV_500_Aggr1km_RefSB";
                break;
                
            case 8:  case 9:  case 10: case 11: case 12: case 13: case 14:
            case 15: case 16: case 17: case 18: case 19: case 26:
                sdsname="EV_1KM_RefSB";
                break;
                
            case 20: case 21: case 22: case 23: case 24: case 25: case 27:
            case 28: case 29: case 30: case 31: case 32: case 33: case 34:
            case 35: case 36:
                sdsname="EV_1KM_Emissive";
                break;
            
            default:
                fprintf (stderr, "Warning: Unknown band number %i\n", band );
                return FAIL;
                break;
        }
    }
    else {
        switch (band) {
            case 1:  case 2:
                sdsname="EV_250_Aggr500_RefSB";
                break;
            
            case 3:  case 4:  case 5:  case 6:  case 7:
                sdsname="EV_500_RefSB";
                break;
                
            default:
                fprintf (stderr, "Warning: Unknown band number %i\n", band );
                return FAIL;
                break;
        }
    }
    
    int32 sds_index = SDnametoindex(hdfid, sdsname);

    if (sds_index == FAIL)
        fprintf (stderr, "Warning: could not find index for band %i %s\n",
                 band, sdsname );

    return sds_index;
}

/****************************************************************************//**
set the start, edge, and stride arrays

@param sds_id   hdf sds_id
@param sub_sds  sds's band
@param start    Pointer to the start array
@param stride   Pointer to the stride array
@param edge     pointer to the edge array

@return 
******************************************************************************/
//#define DEBUG
int setdims (int32 sds_id, int32 sub_sds, int32 *start, int32 *stride, int32 *edge)
{
    char sds_name[100];
    int32 rank = 0, dimsizes[MAX_VAR_DIMS], data_type = 0, num_attrs = 0;
         
    if (FAIL == SDgetinfo(sds_id, sds_name, &rank, dimsizes, &data_type, &num_attrs) )
        return FAIL;
    
    SDendaccess(sds_id);

    int iDim;
    for (iDim = 0; iDim < rank; iDim++) {
        start[iDim] = 0;
        stride[iDim] = 1;
        edge[iDim] = dimsizes[iDim];
    }

    start[0] = sub_sds;
    edge[0] = 1;

    return 0;
}


/****************************************************************************//**

replace bad detectors with nearest good neighbor

@param band         band number (1-36 for 1km, 1-7 for 500m)
@param nPixel       number of pixels (width)
@param nScan        number of scans in the image
@param stripsize    size of a modis strip (10 for 1km, 20 for 500m)
@param band_data    pointer to the array of configuration band data
@param image        pointer to the image

@return nothing

******************************************************************************/

void rep_bad_det( int band, int32 nPixel, int32 nScan, int32 stripsize,
                  int *band_data, int16 *image)
{

    /***** Replace bad detectors with nearest good neighbor *****/
    int32 iDet;    
    for (iDet = 0; iDet < stripsize ; iDet++) {
        
        /***** Check if this detector should be replaced *****/

        if (get2darray( band_data, 2 + iDet, band - 1, (stripsize + 2)) == 1) {
            
            /***** Get the index of the nearest good detector *****/
            
            int min_diff = stripsize - 1;
            int det_diff;
            int rep_index;
            
            int32 iMyDet;
            for ( iMyDet = 0; iMyDet < stripsize ; iMyDet++) {
                det_diff = abs(iMyDet - iDet);
                if ( iMyDet != iDet && det_diff < min_diff &&
                     get2darray( band_data, 2 + iMyDet, band - 1, stripsize + 2)  == 0)
                {
                    rep_index = iMyDet;
                    min_diff = det_diff;
                }
            }

            /***** Replace the data for this detector *****/
            
            int32 iScan;
            for ( iScan = 0; iScan < nScan; iScan++) {
                int j;
                for (j =  0; j < nPixel; j++) {
                    
                    int32 badline =  iScan * stripsize + iDet;
                    int32 goodline = iScan * stripsize + rep_index;

                    get2darray( image, j, badline, nPixel) =
                        get2darray( image, j, goodline, nPixel);
                    
                }
            }
        }
    }
}