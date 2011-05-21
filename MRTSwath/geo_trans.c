/*
!C****************************************************************************

!File: geo_trans.c

!Description: Functions for mapping from geographic image space to latitude
 and longitude and visa versa. GCTP does not provide forward and inverse
 geographic mapping functions and instead handles these in the gctp.c call
 itself.

!Developers:
 Gail Schmidt
 SAIC / USGS EROS Data Center
 Rapid City, SD 57701
 gschmidt@usgs.gov

!END*****************************************************************************/

#include "cproj.h"

/* 
!C******************************************************************************

!Description: 'geofor' handles the geographic forward mapping.
  
!Input Parameters:
 lon          longitude in radians
 lat          latitude in radians

!Output Parameters:
 x            geographic x value
 y            geographic y value
 (returns)    0 to match a succussful return value in GCTP (OK in crpoj.h)

!END*****************************************************************************/

long geofor
(
  double lon,           /* (I) Longitude                */
  double lat,           /* (I) Latitude                 */
  double *x,            /* (O) X projection coordinate  */
  double *y             /* (O) Y projection coordinate  */
)
{
  *x = lon;
  *y = lat;
  return GCTP_OK;      /* successful return to match GCTP OK return */
}


/* 
!C******************************************************************************

!Description: 'geoinv' handles the geographic inverse mapping.
  
!Input Parameters:
 x            geographic x value
 y            geographic y value

!Output Parameters:
 lon          longitude in radians
 lat          latitude in radians
 (returns)    0 to match a succussful return value in GCTP (OK in crpoj.h)

!END*****************************************************************************/

long geoinv
(
  double x,            /* (I) X projection coordinate  */
  double y,            /* (I) Y projection coordinate  */
  double *lon,         /* (O) Longitude                */
  double *lat          /* (O) Latitude                 */
) 
{       
  *lon = x;
  *lat = y;
  return GCTP_OK;      /* successful return to match GCTP OK return */
} 

