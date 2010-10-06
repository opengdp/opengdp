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

dsname="ASTER"
baseurl="@edcftp@/SATELLITE/ASTER/"
basedir="@basedir@"
indir="${basedir}/source/${dsname}/"
outdir="${basedir}/done/${dsname}/"
mapfile="@mapfile@"

tmp="@tmp@"

mapserverpath="@mapserverpath@"

##### setup proccess management #####

((limit=@limit@))

source "@scriptdir@/dwh-generic.bash"

dofunc="ASTER_dofile"

doovr="no"

fetchpattern="AST_L1AE*tif.zip"

datefunc="ASTER_dodate"

###############################################################################
# function to get a ts from a lftp command
###############################################################################

function ASTER_dodate {
    sed 's:.*/AST_L1AE_[0-9]*_\([0-9]\{4\}\)\([0-9]\{4\}\)[0-9]\{6\}_.*:\2\1:'
}

#################################################################################################
# function to proccess a file
#################################################################################################

function ASTER_dofile {
    
    myline=$1
    zipfile="${myline##*/}"

    tifs="${zipfile%_*}"

    dir="${zipfile:13:4}${zipfile:17:4}"
    ts="${zipfile:17:4}${zipfile:13:4}"

    if echo "$myline" | grep -e "^get" > /dev/null
    then

        tmpdir=$(mktemp -d -p "$tmp" "${dsname}XXXXXXXXXX")
        
        lftp -e "$myline ; exit"
        
	    if ! [ -d "$outdir/${ts}" ]
        then
            mkdir -p "$outdir/${ts}"
        fi

	    unzip "${indir}/${dir}/${zipfile}" "${tifs}_b*.tif" -d "$tmpdir"

        gdalbuildvrt -srcnodata 0 -separate "${tmpdir}/${tifs}.vrt" "${tmpdir}/${tifs}_b3N.tif" "${tmpdir}/${tifs}_b2.tif" "${tmpdir}/${tifs}_b1.tif"
        
        gdalwarp -t_srs EPSG:4326 "${tmpdir}/${tifs}.vrt" "${tmpdir}/${tifs}.tif"

        gdaladdo -r average "${tmpdir}/${tifs}.tif" 2 4 8 16 32
      
        mv "${tmpdir}/${tifs}.tif" "$outdir/${ts}/${tifs}.tif"
	    
	    rm -rf "${tmpdir}"

        gdaltindex "${outdir}/${dsname}${ts}.shp" "$outdir/${ts}/${tifs}.tif"

    fi
    echo >&3
}

main









