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

dsname="AERIAL_NASA_AVIRIS"
baseurl="@edcftp@/AERIAL_NASA_AVIRIS/"
basedir="@basedir@"
indir="${basedir}/source/${dsname}/"
outdir="${basedir}/done/${dsname}/"
mapfile="@mapfile@"


tmp="@tmp@"

mapserverpath="@mapserverpath@"

##### setup proccess management #####

((limit=@limit@))

source "@scriptdir@/dwh-generic.bash"

dofunc="AERIAL_NASA_AVIRIS_dofile"

###############################################################################
# function to proccess a file
###############################################################################

function AERIAL_NASA_AVIRIS_dofile {
    
    myline=$1
    zipfile="${myline##*/}"

    tif="${zipfile%.*}.tif"

    ts="${zipfile:25:8}"

    
    if echo "$myline" | grep -e "^get" > /dev/null
    then

        tmpdir=$(mktemp -d -p "$tmp" "${dsname}XXXXXXXXXX")
  
        lftp -e "$(echo "$myline" | sed "s:get -O [/_.A-Za-z0-9]*:get -O ${tmpdir}:") ; exit"

        if ! [ -d "$outdir/${ts}" ]
        then
            mkdir -p "$outdir/${ts}"
        fi

        unzip "${tmpdir}/${zipfile}" "$tif" -d "$tmpdir"

        gdal_translate -ot byte -a_nodata 0 -scale 0 32767 "${tmpdir}/${tif}" "${tmpdir}/translated_${tif}"
        
        if ! gdalinfo "${tmpdir}/translated_${tif}" | grep 'AUTHORITY[[]"EPSG","4326"[]][]]'
        then
            gdalwarp -t_srs EPSG:4326 "${tmpdir}/translated_${tif}" "${tmpdir}/warped_${tif}"
            nearblack -co TILED=YES -of GTiff "${tmpdir}/warped_${tif}" -o "${tmpdir}/nearblack_${tif}"
        else
            nearblack -co TILED=YES -of GTiff "${tmpdir}/translated_${tif}" -o "${tmpdir}/nearblack_${tif}"
        fi

        gdaladdo -r average "${tmpdir}/nearblack_${tif}" 2 4 8 16 32
        
        tiffset -s 306 "${ts:0:4}:${ts:4:2}:${ts:6:2} 12:00:00" "${tmpdir}/nearblack_${tif}"
        
        mv "${tmpdir}/nearblack_${tif}" "$outdir/${ts}/${tif}"
        mv "${tmpdir}/${zipfile}" "$indir"

        rm -rf "${tmpdir}"

        gdaltindex "${outdir}/${dsname}${ts}.shp" "${outdir}/${ts}/${tif}"
        
        

    fi

    echo >&3
}

main

