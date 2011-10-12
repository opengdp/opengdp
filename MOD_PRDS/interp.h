/******************************************************************************
 Program to destripe modis data
 
 Copyright(C) 2004, University of Wisconsin-Madison MODIS Group
 Created by Liam.Gumley@ssec.wisc.edu
     
 ported to C by Brian Case

******************************************************************************/

#ifndef _INTERP_H
#define _INTERP_H

void interp( int nold, double *xold, double *yold,
             int nnew, double *xnew, double *ynew );

#endif