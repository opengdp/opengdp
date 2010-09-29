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

baseurl="http://edcftp.cr.usgs.gov/pub/data/disaster/201004_Oilspill_GulfOfMexico/data/ASTER/"
basedir="/storage/data/deephorizon/"
indir="${basedir}/source/ASTER/"
outdir="${basedir}/done/ASTER/"

if ! [ -d "$indir" ]
then
    mkdir -p "$indir"
fi

if ! [ -d "$outdir" ]
then
    mkdir -p "$outdir"
fi


cd "$indir"

((doing=0))
((limit=7))

function dofile {
    ((doing +=1))

    myline=$1
    zipfile="${myline##*/}"

    tif="${zipfile%.*}"
    tif="${tif%_*}_b1.tif"

    lftp -e "$myline ; exit"

    if ! unzip "$zipfile" "$tif"
    then
        return
    fi

    gdaladdo -r average "$tif" 2 4 8 16 32

    mv "$tif" "$outdir"

    gdaltindex "${outdir}/ASTER.shp" "${outdir}/${tif}"

    ((doing -= 1))
}

##### make the map file #####

cat > "${outdir}/ASTER.map" << EOF
 LAYER
    NAME 'ASTER'
    TYPE RASTER
    STATUS ON
    DUMP TRUE
    PROJECTION
     'proj=longlat'
     'ellps=WGS84'
     'datum=WGS84'
     'no_defs'
     ''
    END
    METADATA
      'wms_title'        'ASTER'
      'wms_srs'          'EPSG:4326'
      'wms_extent'	 '-100.0  17.0 -74.0  33.0'
    END
    OFFSITE 0 0 0
    TILEINDEX 'done/ASTER/ASTER.shp'
  END

EOF

##### get a list of new files #####

lftp "$baseurl" -e "mirror -I AST_L1BE_003_*_tif.zip --script=mirror.lftp ; exit"

##### loop over the list #####

cat mirror.lftp |\
 while read line ;
 do
    
    if [ $doing -lt $limit ]
    then
        dofile "$line" &
        ((doing +=1))
    else
        wait
        ((doing=0))
        dofile "$line" &
        ((doing +=1))
    fi

 done



    
    






