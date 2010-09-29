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

dsname="modis"

baseurl="http://edcftp.cr.usgs.gov/pub/data/disaster/201004_Oilspill_GulfOfMexico/data/MODIS/DIRECT_BROADCAST_CSR_UTEXAS_DEEPWATER/Deepwater/MODIS/"
basedir="/storage/data/deephorizon/"
indir="${basedir}/source/${dsname}/"
outdir="${basedir}/done/${dsname}/"

glob="*16.gulf-geo.*"

#################################################################################################
# function to proccess a file
#################################################################################################

function dofile {

    myline=$1
    tif="${myline##*/}"

    ts="${tif:3:13}"
    
    item="${tif:17:1}"
    
    tifbase=$(sed 's/\([at]1.[0-9]\{8\}.[0-9]\{4\}\).*/\1/')
    qtif="${tifbase}.Q16.gulf-geo.250m.tif"
    htif="${tifbase}.H16.gulf-geo.500m.tif"
    otif="${tifbase}.O16.gulf-geo.1km.tif"
    qtfw="${tifbase}.Q16.gulf-geo.250m.tfw"
    htfw="${tifbase}.H16.gulf-geo.500m.tfw"
    otfw="${tifbase}.O16.gulf-geo.1km.tfw"

    scale="0 32767"
    
    if [ -f "$qtif" ] && [ -f "$htif" ] && [ -f "$otif" ] && [ -f "$qtfw" ] && [ -f "$htfw" ] && [ -f "$otfw" ]
    then
    
        ((band=1))
        
        tmpdir=$(mktemp -d -p /tmp "${dsname}XXXXXXXXXX")
        
        for (( lband = 1 ; lband <= 2 ; lband++ && band++ ));
        do$scale
            gdal_translate -ot Byte -scale $scale -b $lband "$qtif" "${tmpdir}/$tifbase.$(printf %02d $band).tif"
        done
    
        for (( lband = 1 ; lband <= 5 ; lband++ && band++ ))
        do
            gdal_translate -ot Byte -scale $scale -b $lband "$htif" "${tmpdir}/$tifbase.$(printf %02d $band).tif"
        done
        
        for (( lband = 1 ; lband <= 16 ; lband++ && band++ ))
        do
            gdal_translate -ot Byte -scale $scale -b $lband "$otif" "${tmpdir}/$tifbase.$(printf %02d $band).tif"
        done
        
        gdalbuildvrt -resolution highest -separate "${tmpdir}/$tifbase.vrt" "${tmpdir}/$tifbase."[0-9][0-9].tif
    
        gdal_translate "${tmpdir}/$tifbase.vrt" "${tmpdir}/$tifbase.tif"
    
        gdaladdo -r average "${tmpdir}/$tifbase.tif" 2 4 8 16 32 64
        
        mv "${tmpdir}/$tifbase.tif" "${outdir}/"

        tileindex 
 
    
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

lftp "$baseurl" -e "mirror -I "$glob" --script=mirror.lftp --no-recursion ; exit"
lftp "${baseurl}Archive/" -e "mirror -I "$glob" --script=mirror2.lftp --no-recursion ; exit"

##### go ahead and just get all the files now #####

lftp "$baseurl" -e "mirror -I "$glob" --no-recursion --parallel=7 ; exit"
lftp "${baseurl}Archive/" -e "mirror -I "$glob" --no-recursion --parallel=7 ; exit"

##### loop over the list #####

(cat mirror.lftp ; cat mirror2.lftp) |\
 sed "s:/Archive::"
 while read line ;
 do
    
    if [[ "${line##*.}" != "tif" ]]
    then
        continue
    fi

    if ! echo "$myline" | grep -e "^get" > /dev/null
    then
        continue
    fi


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

