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

dsname="LANDSAT_TM_USGS"
baseurl="http://edcftp.cr.usgs.gov/pub/data/disaster/201004_Oilspill_GulfOfMexico/data/LANDSAT_TM_USGS/"
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

    tifs="${zipfile%_*}"

    ts="${zipfile:10:8}"

    lftp -e "$myline ; exit"

    if echo "$myline" | grep -e "^get" > /dev/null
    then

        if ! [ -d "$outdir/${ts}" ]
        then
            mkdir -p "$outdir/${ts}"
        fi

        tmpdir=$(mktemp -d -p /tmp "${dsname}XXXXXXXXXX")

	unzip "$zipfile" "${tifs}*.TIF" -d "$tmpdir"

        gdalbuildvrt -separate "${tmpdir}/${tifs}.vrt" "${tmpdir}/${tifs}"*.TIF
        
        gdalwarp -t_srs EPSG:4326 "${tmpdir}/${tifs}.vrt" "${tmpdir}/${tifs}.tif"

        gdaladdo -r average "${tmpdir}/${tifs}.tif" 2 4 8 16 32
      
        mv "${tmpdir}/${tifs}.tif" "$outdir/${ts}/${tifs}.tif"
	
	rm -rf "${tmpdir}/"

        gdaltindex "${outdir}/${dsname}${ts}.shp" "$outdir/${ts}/${tifs}.tif"

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
    NAME '${dsname}_rgb_${ts}'
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
      'wms_title'        '${dsname}_rgb_${ts}'
      'wms_srs'          'EPSG:900913 EPSG:4326'
      'wms_extent'	 '$extent'
    END
    OFFSITE 0 0 0
    TILEINDEX '${outdir}/${dsname}${ts}.shp'
    PROCESSING "BANDS=3,2,1"

 END

 LAYER
    NAME '${dsname}_ir1_${ts}'
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
      'wms_title'        '${dsname}_ir1_${ts}'
      'wms_srs'          'EPSG:900913 EPSG:4326'
      'wms_extent'	 '$extent'
    END
    OFFSITE 0 0 0
    TILEINDEX '${outdir}/${dsname}${ts}.shp'
    PROCESSING "BANDS=4"

 END

 LAYER
    NAME '${dsname}_ir2_${ts}'
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
      'wms_title'        '${dsname}_ir2_${ts}'
      'wms_srs'          'EPSG:900913 EPSG:4326'
      'wms_extent'	 '$extent'
    END
    OFFSITE 0 0 0
    TILEINDEX '${outdir}/${dsname}${ts}.shp'
    PROCESSING "BANDS=5"

 END

 LAYER
    NAME '${dsname}_ir3_${ts}'
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
      'wms_title'        '${dsname}_ir3_${ts}'
      'wms_srs'          'EPSG:900913 EPSG:4326'
      'wms_extent'	 '$extent'
    END
    OFFSITE 0 0 0
    TILEINDEX '${outdir}/${dsname}${ts}.shp'
    PROCESSING "BANDS=6"

  END

  LAYER
    NAME '${dsname}_ir321_${ts}'
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
      'wms_title'        '${dsname}_ir321_${ts}'
      'wms_srs'          'EPSG:900913 EPSG:4326'
      'wms_extent'	 '$extent'
    END
    OFFSITE 0 0 0
    TILEINDEX '${outdir}/${dsname}${ts}.shp'
    PROCESSING "BANDS=6,5,4"

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

lftp "$baseurl" -e "mirror --script=mirror.lftp -I "LS*.zip"; exit"

##### loop over the list #####

cat mirror.lftp |\
 while read line
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
 sed 's:.*/LS[0-9]\{8\}\([0-9]\{8\}\).*:\1:' |\
 sort |\
 uniq |\
 while read ts
 do

    ##### get the extent of the ds #####
    
    extent=$(getextent "$ts")

    ##### rewrite the map file #####

    writemap "$ts" "$extent"

    ##### add an include line in the main mapfile #####
    
    addinclude "$ts"

    ##### create a single file for larger area overviews #####
    
    #makeoverview "$ts" "$extent"
    
 done 

