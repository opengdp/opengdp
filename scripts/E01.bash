#!/bin/bash
# Copyright (c) 2010, Brian Case
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

dsname="EO1"
baseurl="http://edcftp.cr.usgs.gov/pub/data/disaster/201004_Oilspill_GulfOfMexico/data/SATELLITE/EO1/"
basedir="/storage/data/deephorizon/"
indir="${basedir}/source/${dsname}/"
outdir="${basedir}/done/${dsname}/"
mapfile="${basedir}/deephorizon.map"

tmp=/mnt/ram2/

mapserverpath="/usr/local/src/mapserver/mapserver"

##### setup proccess management #####

((limit=1))

source dwh-generic.bash

dofunc="EO1_dofile"

doovr="no"

fetchpattern="EO1A*.tgz"

datefunc="EO1_dodate"

###############################################################################
# function to get a ts from a lftp command
###############################################################################

function EO1_dodate {
     
     while read line
     do
        ts="$(echo "$line" | sed 's:.*/EO1.......\(....\)\(...\).*:\1 \2:')"
        date -d "jan 1 $ts days - 1 day" "+%Y%m%d"
     done
}

#################################################################################################
# function to proccess a file
#################################################################################################

function EO1_dofile {
    
    myline=$1
    zipfile="${myline##*/}"

    tifs="${zipfile%.*}"
    
    y="${zipfile:10:4}"
    d="${zipfile:14:3}"
    ts=$(date -d "jan 1 $y $d days - 1 day" "+%Y%m%d")
    

    if echo "$myline" | grep -e "^get" > /dev/null
    then

        tmpdir=$(mktemp -d -p "$tmp" "${dsname}XXXXXXXXXX")
        
        lftp -e "$(echo "$myline" | sed "s: -O [/_.A-Za-z0-9]*: -O ${tmpdir}:") ; exit"
        
	    if ! [ -d "$outdir/${ts}" ]
        then
            mkdir -p "$outdir/${ts}"
        fi

	    tar -xzf "${tmpdir}/$zipfile" -C "$tmpdir"

        gdalbuildvrt -srcnodata 0 -separate "${tmpdir}/${tifs}.vrt" "${tmpdir}/${tifs}/${tifs}_B05_L1T.TIF" "${tmpdir}/${tifs}/${tifs}_B04_L1T.TIF" "${tmpdir}/${tifs}/${tifs}_B03_L1T.TIF"
        
        gdal_translate -ot Byte -scale "${tmpdir}/${tifs}.vrt" "${tmpdir}/${tifs}_byte.tif"
        gdalwarp -t_srs EPSG:4326 "${tmpdir}/${tifs}_byte.tif" "${tmpdir}/${tifs}.tif"

        gdaladdo -r average "${tmpdir}/${tifs}.tif" 2 4 8 16 32
      
        mv "${tmpdir}/${tifs}.tif" "$outdir/${ts}/${tifs}.tif"
	    mv "${tmpdir}/${zipfile}" "$indir"
	    
	    rm -rf "${tmpdir}/"

        gdaltindex "${outdir}/${dsname}${ts}.shp" "$outdir/${ts}/${tifs}.tif"

    fi
    echo >&3
}

main


