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

dsname="AERIAL_NASA"
baseurl="http://edcftp.cr.usgs.gov/pub/data/disaster/201004_Oilspill_GulfOfMexico/data/AERIAL_NASA/"
basedir="/storage/data/deephorizon/"
indir="${basedir}/source/${dsname}/"
outdir="${basedir}/done/${dsname}/"
mapfile="${basedir}/deephorizon.map"

#################################################################################################
# function to proccess a file
#################################################################################################

function dofile {
    
    myline=$1
    zipfile="${myline##*/}"

    tif="${zipfile%.*}.tif"

    ts="${zipfile:25:8}"

    lftp -e "$myline ; exit"

    if echo "$myline" | grep -e "^get" > /dev/null
    then

        if ! [ -d "$outdir/${ts}" ]
        then
            mkdir -p "$outdir/${ts}"
        fi

        tmpdir=$(mktemp -d -p /tmp "${dsname}XXXXXXXXXX")
	unzip "$zipfile" "$tif" -d "$tmpdir"

        gdal_translate -co TILED=YES "${tmpdir}/$tif" "${tmpdir}/warped_${tif}"
        gdaladdo -r average "${tmpdir}/warped_${tif}" 2 4 8 16 32

        mv "${tmpdir}/warped_${tif}" "$outdir/${ts}/${tif}"
        
        rm -rf "${tmpdir}/"

        gdaltindex "${outdir}/${dsname}${ts}.shp" "${outdir}/${ts}/${tif}"

    fi

}

#################################################################################################
# function to get the extent of the ds
#################################################################################################

function getextent {
    ts="$1"

    ogrinfo -so -al "${outdir}/${dsname}${ts}.shp" |\
     grep Extent: |\
     sed -e 's/) - (/ /' -e 's/Extent: (//' -e 's/,//' -e 's/)//'
}

#################################################################################################
# function to write out a map file
#################################################################################################

function writemap {
    ts="$1"
    extent="$2"

    cat > "${outdir}/${dsname}${ts}.map" << EOF

  LAYER
    NAME '${dsname}_${ts}_nominmax'
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
      'wms_title'        '${dsname}_${ts}_nominmax'
      'wms_srs'          'EPSG:900913 EPSG:4326'
      'wms_extent'	 '$extent'
    END
    OFFSITE 0 0 0
    TILEINDEX '${outdir}/${dsname}${ts}.shp'
    PROCESSING "SCALE=AUTO"

  END

 LAYER
    NAME '${dsname}_${ts}_hires'
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
      'wms_title'        '${dsname}_${ts}_hires'
      'wms_srs'          'EPSG:900913 EPSG:4326'
      'wms_extent'	 '$extent'
    END
    OFFSITE 0 0 0
    TILEINDEX '${outdir}/${dsname}${ts}.shp'
    GROUP '${dsname}_${ts}'
    MAXSCALEDENOM 50000
    PROCESSING "SCALE=AUTO"

  END

 LAYER
    NAME '${dsname}_${ts}_ovr'
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
      'wms_title'        '${dsname}_${ts}_ovr'
      'wms_srs'          'EPSG:900913 EPSG:4326'
      'wms_extent'	 '$extent'
    END
    OFFSITE 0 0 0
    DATA '${outdir}/${ts}/overview.tif'
    GROUP '${dsname}_${ts}'
    MINSCALEDENOM 50000
    PROCESSING "SCALE=AUTO"

  END

EOF

}

#################################################################################################
# function to add an include line in the main mapfile
#################################################################################################

function addinclude {
    ts="$1"

    if ! grep $mapfile -e "${outdir}/${dsname}${ts}.map" > /dev/null
    then
        linenum=$(cat "$mapfile" | grep -n -e "^[ ]*END[ ]*$" | tail -n 1 | cut -d ":" -f 1)

        ed -s "$mapfile" << EOF
${linenum}-1a
  INCLUDE '${outdir}/${dsname}${ts}.map'
.
w
EOF
    fi

}

#################################################################################################
# functiom to create a overview from the dataset
#################################################################################################

function makeoverview {
    ts="$1"
    extent="$2"

    /usr/local/src/mapserver-trunk/mapserver/shp2img -m "$mapfile" -l "${dsname}_${ts}_nominmax" \
     -o "${outdir}/${ts}/overview.tif" -s 12000 8000 -i image/tiff -e $extent

}


#################################################################################################
# main
#################################################################################################

##### make sure the base dirs exsist #####

if ! [ -d "$indir" ]
then
    mkdir -p "$indir"
fi

if ! [ -d "$outdir" ]
then
    mkdir -p "$outdir"
fi

##### cd to the in dir #####

cd "$indir"

##### setup proccess management #####

((doing=0))
((limit=7))

##### get a list of new files #####

lftp "$baseurl" -e "mirror --script=mirror.lftp ; exit"

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

wait

##### loop over each date of data we got #####

grep mirror.lftp -e "^get" |\
 sed 's:.*/AE00N[0-9]\{2\}_[a-zA-Z0-9]\{10\}_[0-9]\{6\}\([0-9]\{8\}\).*:\1:' |\
 sort |\
 uniq |\
 while read ts
 do
    echo $ts

    ##### get the extent of the ds #####
    
    extent=$(getextent "$ts")

    ##### rewrite the map file #####

    writemap "$ts" "$extent"

    ##### add an include line in the main mapfile #####
    
    addinclude "$ts"

    ##### create a single file for larger area overviews #####
    
    makeoverview "$ts" "$extent"
    
    echo $ts again
 done 

