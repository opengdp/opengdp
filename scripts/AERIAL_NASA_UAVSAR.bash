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

dsname="AERIAL_NASA_UAVSAR"
baseurl="http://edcftp.cr.usgs.gov/pub/data/disaster/201004_Oilspill_GulfOfMexico/data/AERIAL/NASA_UAVSAR/"
basedir="/storage/data/deephorizon/"
indir="${basedir}/source/${dsname}/"
outdir="${basedir}/done/${dsname}/"
mapfile="${basedir}/deephorizon.map"

tmp=/mnt/ram2/

mapserverpath="/usr/local/src/mapserver/mapserver"

##### setup proccess management #####

((limit=1))

source dwh-generic.bash

dofunc="AERIAL_NASA_UAVSAR_dofile"

doovr="no"

fetchpattern="*.kmz"

datefunc="AERIAL_NASA_UAVSAR_dodate"


###############################################################################
# function to get a ts from a lftp command
###############################################################################

function AERIAL_NASA_UAVSAR_dodate {
    sed -r 's:.*/[a-zA-Z0-9]{6}_[0-9]{5}_[0-9]{5}_[0-9]{3}_([0-9]{6})_.*:20\1:'
}


###############################################################################
# function to proccess a file
###############################################################################

function AERIAL_NASA_UAVSAR_dofile {
    
    myline=$1
    zipfile="${myline##*/}"

    zipbase="${zipfile%.*}"

    ts="20${zipfile:23:6}"

    if echo "$myline" | grep -e "^get" > /dev/null
    then
        path="$(echo "$myline" | sed "s:get -O \([/_.A-Za-z0-9]*\) .*:\1:")"

        tmpdir=$(mktemp -d -p "$tmp" "${dsname}XXXXXXXXXX")
	
        lftp -e "$myline ; exit"

        if ! [ -d "$outdir/${ts}" ]
        then
            mkdir -p "$outdir/${ts}"
        fi

        unzip "${path}/${zipfile}" -d "$tmpdir"
	
        for kml in $(find "$tmpdir/" -iname "tile*.kml")
        do

    	    img=$(grep '<GroundOverlay>' -A12 "$kml" | grep href | sed -r 's|.*<href>(.*)</href>.*|\1|')

            read n s e w < <(grep '<GroundOverlay>' -A12 "$kml" | grep north -A3 | sed 's:<[/a-z]*>::g' | tr "\n" " ")

            read x y < <(gdalinfo "${tmpdir}/$img" | grep -e "Size is" | sed 's/Size is \([0-9]*\), \([0-9]*\)/\1 \2/')
            
            xp=$(echo "scale = 16; ($e - $w) / $x" | bc)
            yp=$(echo "scale = 16; ($s - $n) / $y" | bc)
            
   	        base="${img%.*}"
            tilename="${base##*/}"
            ext="${img##*.}"
            
            case "$ext" in
                jpg)
                    world="${base}.jpw"
                    ;;
                tif)
                    world="${base}.tfw"
                    ;;
                png)
                    world="${base}.pgw"
                    ;;
                esac

            cat > "${tmpdir}/$world" <<EOF
$xp
0.0000000000000000
0.0000000000000000
$yp
$w
$n
EOF
            gdal_translate -ot Byte -scale -b 1 -b 2 -b 3 -of GTiff -co "TILED=YES" "${tmpdir}/$img" "${tmpdir}/${zipbase}_${tilename}.tif"

            gdaladdo -r average "${tmpdir}/${zipbase}_${tilename}.tif" 2 4 8 16
        
            mv "${tmpdir}/${zipbase}_${tilename}.tif" "$outdir/${ts}/${zipbase}_${tilename}.tif"
            
            gdaltindex "${outdir}/${dsname}${ts}.shp" "${outdir}/${ts}/${zipbase}_${tilename}.tif"

        done

        rm -rf "${tmpdir}"

    fi

    echo >&3
}



main

