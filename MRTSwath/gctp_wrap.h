/******************************************************************************
 *
 * Project:  Hierarchical Data Format Release 4 (HDF4)
 * Purpose:  This is the wrapper code to use OGR Coordinate Transformation
 *           services instead of GCTP library
 * Author:   Andrey Kiselev, dron@ak4719.spb.edu
 *
 ******************************************************************************
 * Copyright (c) 2004, Andrey Kiselev <dron@ak4719.spb.edu>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ****************************************************************************/


#include "ogr_srs_api.h"

/*
   0 = Geographic
   1 = Universal Transverse Mercator (UTM)
   2 = State Plane Coordinates
   3 = Albers Conical Equal Area
   4 = Lambert Conformal Conic
   5 = Mercator
   6 = Polar Stereographic
   7 = Polyconic
   8 = Equidistant Conic
   9 = Transverse Mercator
  10 = Stereographic
  11 = Lambert Azimuthal Equal Area
  12 = Azimuthal Equidistant
  13 = Gnomonic
  14 = Orthographic
  15 = General Vertical Near-Side Perspective
  16 = Sinusiodal
  17 = Equirectangular
  18 = Miller Cylindrical
  19 = Van der Grinten
  20 = (Hotine) Oblique Mercator 
  21 = Robinson
  22 = Space Oblique Mercator (SOM)
  23 = Alaska Conformal
  24 = Interrupted Goode Homolosine 
  25 = Mollweide
  26 = Interrupted Mollweide
  27 = Hammer
  28 = Wagner IV
  29 = Wagner VII
  30 = Oblated Equal Area
  99 = User defined
*/

#define GEO 0
#define UTM 1
#define SPCS 2
#define ALBERS 3
#define LAMCC 4
#define MERCAT 5
#define PS 6
#define POLYC 7
#define EQUIDC 8
#define TM 9
#define STEREO 10
#define LAMAZ 11
#define AZMEQD 12
#define GNOMON 13
#define ORTHO 14
#define GVNSP 15
#define SNSOID 16
#define EQRECT 17
#define MILLER 18
#define VGRINT 19
#define HOM 20
#define ROBIN 21
#define SOM 22
#define ALASKA 23
#define GOOD 24
#define MOLL 25
#define IMOLL 26
#define HAMMER 27
#define WAGIV 28
#define WAGVII 29
#define OBEQA 30
#define USDEF 99

#define GCTP_OK	0
#define GCTP_ERROR  -1
#define GCTP_IN_BREAK -2
#define GCTP_ERANGE -9

int32 osr_for(
double lon,			/* (I) Longitude 		*/
double lat,			/* (I) Latitude 		*/
double *x,			/* (O) X projection coordinate 	*/
double *y)			/* (O) Y projection coordinate 	*/
;

void for_init(
int32 outsys,		/* output system code				*/
int32 outzone,		/* output zone number				*/
float64 *outparm,	/* output array of projection parameters	*/
int32 outdatum,		/* output datum					*/
char *fn27,		/* NAD 1927 parameter file			*/
char *fn83,		/* NAD 1983 parameter file			*/
int32 *iflg,		/* status flag					*/
int32 (*for_trans)(double, double, double *, double *))
                        /* forward function pointer			*/
;

int32 osr_inv(
double x,			/* (O) X projection coordinate 	*/
double y,			/* (O) Y projection coordinate 	*/
double *lon,			/* (I) Longitude 		*/
double *lat)			/* (I) Latitude 		*/
;

void inv_init(
int32 insys,		/* input system code				*/
int32 inzone,		/* input zone number				*/
float64 *inparm,	/* input array of projection parameters         */
int32 indatum,	        /* input datum code			        */
char *fn27,		/* NAD 1927 parameter file			*/
char *fn83,		/* NAD 1983 parameter file			*/
int32 *iflg,		/* status flag					*/
int32 (*inv_trans)(double, double, double*, double*))	
                        /* inverse function pointer			*/
;
