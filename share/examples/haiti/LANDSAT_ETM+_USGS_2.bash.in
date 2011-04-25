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

dsname="LANDSAT_ETM_USGS"
baseurl="http://edcftp.cr.usgs.gov/pub/data/disaster/201011_Hurricane_Tomas/data/LANDSAT_ETM+_USGS/"

tmp="/mnt/ram2/"

##### setup proccess management #####

((limit=8))

source "@GENERICDIR@/dwh-generic.bash"
source "./dwh-proj_def.bash"



doovr="no"

datefunc="LANDSAT_ETM_USGS_dodate"

###############################################################################
# function to get a ts from a lftp command
###############################################################################

function LANDSAT_ETM_USGS_dodate {
    sed 's:.*/LS[0-9]\{8\}\([0-9]\{8\}\).*:\1:'
}

#################################################################################################
# function to proccess a file
#################################################################################################

function dofile {
    myline=$1

    if echo "$myline" | grep -e "^get" > /dev/null
    then    
        local sourcedir=${indir//\/\///}
        local sourcedir=${sourcedir//\/\///}

        local file="${myline##*/}"
        local base="${file%.*}"
        local ext="${file#*.}"
        local ext="$(tr [A-Z] [a-z] <<< "$ext")"
        #local ext="${ext,,*}"
        
        if echo "$myline" | grep -e "$sourcedir" > /dev/null
        then
            local dir="$(echo "$myline" | sed "s|.*$sourcedir\(.*\) $url.*|\1|")/"
        else
            local dir=""
        fi
         
        local ts=$(${datefunc} <<< "$myline")
        
        #printf " myline=%s\n sourcedir=%s\n file=%s\n base=%s\n ext=%s\n dir=%s\n ts=%s\n" \
        #        "$myline" \
        #        "$sourcedir "\
        #        "$file" \
        #        "$base" \
        #        "$ext" \
        #        "$dir" \
        #        "$ts"
        #echo >&3
        #return

        local tmpdir=$(mktemp -d -p "$tmp" "${dsname}XXXXXXXXXX")
        
        
        if [[ "$DWH_REBUILD" == "rebuild" ]]
        then
            local origdir="${indir/%\//}.old"
        else
            lftp -e "set cmd:set-term-status 0 ; $(echo "$myline" | sed "s:get -O [-/_.A-Za-z0-9]*:get -O ${tmpdir}:") ; exit" > /dev/null 2> /dev/null
            local origdir="$tmpdir"
        fi

        if ! [ -d "$outdir/${ts}" ]
        then
            mkdir -p "$outdir/${ts}"
        fi

        unzip "${origdir}/${file}" "*_B0[123].TIF" -d "$tmpdir"

        gdalbuildvrt -srcnodata 0 -separate "${tmpdir}/${base}.vrt" ${tmpdir}/*_B03.TIF ${tmpdir}/*_B02.TIF ${tmpdir}/*_B01.TIF
        
        gdal_fillnodata.py -md 12 -b 1 "${tmpdir}/${base}.vrt" "${tmpdir}/${base}_B01_filled.tif"
        gdal_fillnodata.py -md 12 -b 2 "${tmpdir}/${base}.vrt" "${tmpdir}/${base}_B02_filled.tif"
        gdal_fillnodata.py -md 12 -b 3 "${tmpdir}/${base}.vrt" "${tmpdir}/${base}_B03_filled.tif"
        
        gdalbuildvrt -srcnodata 0 -separate "${tmpdir}/${base}_filled.vrt" "${tmpdir}/${base}_B01_filled.tif" "${tmpdir}/${base}_B02_filled.tif" "${tmpdir}/${base}_B03_filled.tif"
        
         doimg "${base}_filled.vrt" "$tmpdir" "$ts" "$(gdalinfo "${tmpdir}/${base}_filled.vrt")"

        if [[ "$DWH_REBUILD" != "rebuild" ]]
        then
            mv "${tmpdir}/${file}" "${indir}/${dir}/${file}"
        fi
                
        rm -rf "${tmpdir}"

    fi
    echo >&3
}


main "$@"

