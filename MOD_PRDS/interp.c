
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

/****************************************************************************//**

Linearly interpolate an input array. Values outside the range of
  the input abscissae are obtained via extrapolation.

@param nold     Number of elements in the input arrays
@param xold     pointer to input array of Abscissa values (must be monotonic)
@param yold     pointer to input array of Values
@param nnew     Number of elements in the output arrays
@param xnew     pointer to output array  for Abscissa values
@param ynew     pointer to output array for Linearly interpolated values

@return nothing

fixme   get rid of the goto with a do while()

******************************************************************************/

void interp( int nold, double *xold, double *yold,
             int nnew, double *xnew, double *ynew )
{

    double slope, intercept;
    
    int lo = 0;
    int hi = 1;
    int init = 1;
                 
    int j;
    for (j = 0; j < nnew; j++) {

there:        
        /***** check if output point falls between current input points *****/

        if( xnew[j] > xold[hi] ) {
            if( hi < nold ) {
                lo++;
                hi++;
                init = 1;
                goto there;
            }
        }
                
        /***** compute slope and intercept only when necessary *****/

        if( init == 1 ) {
            slope = ( yold[hi] - yold[lo] ) / ( xold[hi] - xold[lo] );
            intercept = yold[lo] - slope * xold[lo];
            init = 0;
        }
        
        /***** compute output value *****/

        ynew[j] = slope * xnew[j] + intercept;

    }

    return;
}
