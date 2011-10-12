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
#include "interp.h"
#include "hdf.h"


/******************************************************************************
    constants
******************************************************************************/

#define MaxValSize 32768

#define MaxVal 32767

/******************************************************************************
 prototypes
******************************************************************************/

size_t modis_edf_valid (size_t n, int16 *image);

int modis_edf_median (size_t n, int16 *image);

int create_edf (int32 nPixel, int32 nScan, int32 stripsize, int32 nDet, int16 *image, double *edf);

void create_lut (int32 nDet, double *edf, int32 ref_ind, int32 *lut);

int apply_lut ( int32 nPixel, int32 nScan, int32 stripsize, int32 nDet, int32 ref_ind,
                 int32 *lut, int16 *image, int16 *destripe);

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

Destripe one band of MODIS 1KM scaled integer data using the EDF
algorithm of Weinreb et. al.

@param nPixel       number of pixels (width)
@param nScan        number of scans in the image
@param stripsize    size of a modis strip (10 for 1km, 20 for 500m)
@param ref          referance detector
@param mir          Pointer to the array of mirror side data
@param image        pointer to the image
@param destripe     pointer to the image to store the output in

@return 0 on success, -1 on failure

******************************************************************************/

int modis_edf_destripe( int32 nPixel, int32 nScan, int32 stripsize,
                        int32 ref, int32 *mir,
                        int16 *image, int16 *destripe)
{

    int32 nDet = stripsize * 2;
    
    /***** Get the number of valid values in the input image *****/

    size_t nValid = modis_edf_valid(nScan * stripsize * nPixel, image);

    if (nValid < stripsize * nPixel)
        return -1;
    
    /***** Create the EDF for each detector *****/
 
    double *edf = calloc( MaxValSize * nDet,  sizeof(double) );
    if ( !edf ) {
        return -1;
    }
    
    if ( 0 > create_edf (nPixel, nScan, stripsize, nDet, image, edf) ) {
        free (edf);
        return -1;
    }

    /***** Get index for reference detector on mirror side zero *****/
    
    int ref_ind = ref;
    if (mir[ref] == 1)
        ref_ind = ref + stripsize;

    /***** Create the lookup table (LUT) which maps each detector to the reference detector *****/
    
    int32 *lut = malloc( MaxValSize * nDet * sizeof(int32) );
    if ( !lut ) {
        free (edf);
        return -1;
    }
    
    create_lut (nDet, edf, ref_ind, lut);
    free (edf);

    /***** Set the output image equal to the input image *****/

    memcpy(destripe, image, nScan * stripsize * nPixel * sizeof(int16));

    /***** Apply the LUT to all detectors except the reference detector *****/

    if ( 0 > apply_lut ( nPixel, nScan, stripsize, nDet, ref_ind,
                lut, image, destripe))
    {
        free(lut);
        return -1;
    }
    
    free (lut);
    
    /***** Replace bad destriped values *****/
    int i;
    for (i = 0; i < nScan * stripsize * nPixel ; i++) { 
        if (destripe[i] < 0 || destripe[i] > MaxVal)
            destripe[i] = image[i];
    }

    /***** Set median value of destriped image to median value of original image *****/

    int median_old, median_new, median_del;
    
    median_old = modis_edf_median(nScan * stripsize * nPixel, image);
    median_new = modis_edf_median(nScan * stripsize * nPixel, destripe);
    
    median_del = median_new - median_old;
    if (median_del != 0) {
        for (i = 0; i < nScan * stripsize * nPixel ; i++) {
            if (destripe[i] >= 0 || destripe[i] <= MaxVal)
                destripe[i] = destripe[i] - median_del;
        }
    }


    return 0;
}

/****************************************************************************//**

Compute the median of an array of MODIS 1KM scaled integers.

@param n        Number of pixels in the image
@param image    image to compute the median from

@return median value

******************************************************************************/

int modis_edf_median (size_t n, int16 *image) {

    int hist[32767];
    int count;

    /***** Compute histogram *****/

    memset(hist, 0, MaxVal * sizeof(int));

    size_t i;
    for ( i = 0; i < n; i++ ) {
        count = image[i];
        if (count >= 0 && count <= MaxVal)
            (hist[count])++;
    }

    /***** Compute cumulative sum, and return when median is found *****/
    
    int sum = 0;
    int median = 0;
    for ( i = 0; i <= MaxVal; i++ ) {
        sum += hist[i];
        median = i;
        if (sum >= (n / 2))
            break;
    }

    return median;
}


/****************************************************************************//**

Count the number of valid values in an array of MODIS 1KM scaled integers.

@param n        Number of pixels in the image
@param image    image to compute the valid values from

@return Number of valid values in the range 0 to 32767 (MaxVal) inclusive

******************************************************************************/

size_t modis_edf_valid (size_t n, int16 *image) {

    size_t sum = 0;

    /***** Compute number of valid values *****/
    size_t i;
    for ( i = 0; i < n ; i++) {
        if (image[i] >= 0 && image[i] <= MaxVal)
            sum++;
    }

    return sum;
}

/****************************************************************************//**

Compute row indices for a detector

@param nPixel       number of pixels (width)
@param nScan        number of scans in the image
@param stripsize    size of a modis strip (10 for 1km, 20 for 500m)
@param nDet         number of detectors (stripsize * 2)
@param iDet         the detector to calc indices for
@param row          pointer row index

@return the number of locations in the row index

******************************************************************************/

int calc_row( int32 nPixel, int32 nScan, int32 stripsize, int32 nDet, int32 iDet, int32 *row)
{
    /***** Compute row indices for this detector *****/

    int32 nLoc = nScan / 2 - 1;
    
    int32 iLoc;
    for (iLoc = 0; iLoc <= nLoc; iLoc++) {
        row[iLoc] = iLoc * nDet + iDet;
    }
    
    /***** Handle an odd number of scans *****/
    
    if (nScan % 2 == 1) {
        nLoc = nScan / 2;
        
        if (iDet < stripsize)
            row[nLoc] = row[nLoc - 1] + nDet;
        
        else
            row[nLoc] = row[nLoc - 1] - nDet;\
    }

    return nLoc;
}

/****************************************************************************//**

Create the EDF for each detector

@param nPixel       number of pixels (width)
@param nScan        number of scans in the image
@param stripsize    size of a modis strip (10 for 1km, 20 for 500m)
@param nDet         number of detectors (stripsize * 2)
@param image        pointer to the image to read the data from
@param edf          pointer to the edf to store the output in

@return 0 on success, -1 on failure

******************************************************************************/

int create_edf (int32 nPixel, int32 nScan, int32 stripsize, int32 nDet, int16 *image, double *edf) {

    int32 *row = malloc( (nScan / 2 + 1) * sizeof(int32) );
    if ( !row )
        return -1;
    
    int32 iDet;
    for (iDet = 0; iDet < nDet; iDet++) {

        /***** Compute row indices for this detector *****/
        
        int32 nLoc = calc_row( nPixel, nScan, stripsize, nDet, iDet, row);
    
        /***** Compute the histogram for this detector *****/
        
        int hist[MaxValSize];
        memset(hist, 0, MaxValSize * sizeof(int));
        int ngood = 0;
        
        int32 iLoc;
        for ( iLoc = 0; iLoc <= nLoc; iLoc++ ) {
            int32 iPix;
            for ( iPix = 0; iPix < nPixel; iPix++) {
                int16 count = 0;
                count = get2darray(image, iPix, row[iLoc], nPixel);
                if (count >= 0 && count <= MaxVal) {
                    (hist[count])++;
                    ngood++;
                }
            }
        }
        
        /***** Compute the EDF for this detector *****/
        
        int sum = 0;
        int i;
        for ( i = 0; i <= MaxVal; i++) {
            sum += hist[i];
            get2darray( edf, i, iDet, MaxValSize) = (double)sum / (double) ngood;
        }
    }
    
    free (row);
    
    return 0;
}

/****************************************************************************//**

Create the lookup table which maps each detector to the reference detector

@param edf          pointer to the edf array
@param ref_ind      referance detector
@param lut          pointer to the lut array to store the output in

@return nothing

******************************************************************************/

void create_lut (int32 nDet, double *edf, int32 ref_ind, int32 *lut) {

    double xtab[MaxValSize], ytab[MaxValSize];
    double xint[MaxValSize], yint[MaxValSize];

    int i;
    for (i = 0; i < MaxValSize; i++) {
        xtab[i] = get2darray( edf, i, ref_ind, MaxValSize);
        ytab[i] = i;
    }

    int n = MaxValSize;
    int32 iDet;
    for (iDet = 0; iDet < nDet; iDet++) {
        if (iDet != ref_ind) {
            for (i = 0; i < MaxValSize; i++)
                xint[i] = get2darray(edf, i, iDet, MaxValSize);
            
            interp(n, xtab, ytab, n, xint, yint);

            for (i = 0; i < MaxValSize; i++)
                get2darray( lut, i, iDet, MaxValSize) = (int)(yint[i] + .5);
        }
    }
}

/****************************************************************************//**

Apply the LUT to all detectors except the reference detector

@param nPixel       number of pixels (width)
@param nScan        number of scans in the image
@param stripsize    size of a modis strip (10 for 1km, 20 for 500m)
@param nDet         number of detectors (stripsize * 2)
@param ref_ind      referance detector
@param lut          pointer to the lut array
@param image        pointer to the image
@param destripe     pointer to the image to store the output in

@return 0 on success, -1 on failure

******************************************************************************/

int apply_lut ( int32 nPixel, int32 nScan, int32 stripsize, int32 nDet, int32 ref_ind,
                 int32 *lut, int16 *image, int16 *destripe)
{
    
    int32 *row = malloc( (nScan / 2 + 1) * sizeof(int32) );
    if ( !row )
        return -1;
    
    int32 iDet;
    for (iDet = 0; iDet < nDet; iDet++) {

        if (iDet != ref_ind) {

            /***** Compute row indices for this detector *****/

            int32 nLoc = calc_row( nPixel, nScan, stripsize, nDet, iDet, row);
            
            /***** Apply the LUT to valid input values *****/
            int32 iLoc;
            for ( iLoc = 0; iLoc <= nLoc; iLoc++ ) {

                int32 iPix;
                for (iPix = 0; iPix < nPixel; iPix++) {
                    int16 pix = get2darray( image, iPix, row[iLoc], nPixel);

                    if (pix >= 0 && pix <= MaxVal) {
                        get2darray( destripe, iPix, row[iLoc], nPixel) = 
                            get2darray( lut, pix, iDet, MaxValSize );
                    }
                }
            }
        }
    }

    free (row);
    
    return 0;
}
