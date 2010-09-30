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
basedir="/data/testdwh/"
indir="${basedir}/source/${dsname}/"
outdir="${basedir}/done/${dsname}/"
mapfile="${basedir}/deephorizon.map"

#tmp=/mnt/ram2/
tmp=/data/testdwh/
mapserverpath="/usr/local/src/mapserver/"

##### setup proccess management #####

((doing=0))
((limit=16))


#################################################################################################
# function to proccess a file
#################################################################################################

function dofile {
    
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

            gdaladdo -r average "${tmpdir}/${zipbase}_${tilename}.tif" 2 4 8 16 32
        
            mv "${tmpdir}/${zipbase}_${tilename}.tif" "$outdir/${ts}/${zipbase}_${tilename}.tif"
            
            gdaltindex "${outdir}/${dsname}${ts}.shp" "${outdir}/${ts}/${zipbase}_${tilename}.tif"

        done

        rm -rf "${tmpdir}/"

    fi

    echo >&3
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

    ${mapserverpath}/shp2img -m "$mapfile" -l "${dsname}_${ts}_nominmax" \
     -o "${outdir}/${ts}/overview.tif" -s 12000 8000 -i image/tiff -e $extent

}

#################################################################################################
# main
#################################################################################################

function main {

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

    #open a fd to a named pipe

    mkfifo pipe; exec 3<>pipe

    host="$(hostname)"
    mirrorfile="$host.mirror.lftp"

    ##### get a list of new files #####

    lftp "$baseurl" -e "mirror --script=${mirrorfile} -I *.kmz ; exit"

    ##### loop over the list #####

    while read line ;
    do

        if echo "$line" | grep -e "^mkdir" > /dev/null
        then
            lftp -e "$line ; exit"
            continue
        fi

        if [ $doing -lt $limit ]
        then
        	dofile "$line" &
         	((doing +=1))
        else
            read <&3
            ((doing -=1))
        	dofile "$line" &
         	((doing +=1))
        fi

    done < "${mirrorfile}"

    wait

 ##### loop over each date of data we got #####

    grep "${mirrorfile}" -e "^get" |\
     sed -r 's:.*/[a-zA-Z0-9]{6}_[0-9]{5}_[0-9]{5}_[0-9]{3}_([0-9]{6})_.*:20\1:' |\
     sort |\
     uniq |\
     while read ts
     do

        for t in "${outdir}/${ts}/"*
        do
            if [[ "$t" != "${outdir}/${ts}/overview.tif" ]]
            then
                gdaltindex "${outdir}/${dsname}${ts}.shp"  "$t"
            fi
        done

        ##### get the extent of the ds #####
        
        extent=$(getextent "$ts")

        ##### rewrite the map file #####

        writemap "$ts" "$extent"

        ##### add an include line in the main mapfile #####
        
        addinclude "$ts"

        ##### create a single file for larger area overviews #####
        
        makeoverview "$ts" "$extent"

     done 

}


main

