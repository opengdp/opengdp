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

#################################################################################################
# function to proccess a file
#################################################################################################

function dofile {
    
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

        if ! gdalinfo "${tmpdir}/${tif}" | grep 'AUTHORITY[[]"EPSG","4326"[]][]]'
        then
            gdalwarp -t_srs EPSG:4326 "${tmpdir}/${tif}" "${tmpdir}/warped_${tif}"
            nearblack -co TILED=YES -of GTiff "${tmpdir}/warped_${tif}" -o "${tmpdir}/nearblack_${tif}"
        else
            nearblack -co TILED=YES -of GTiff "${tmpdir}/${tif}" -o "${tmpdir}/nearblack_${tif}"
        fi

        gdaladdo -r average "${tmpdir}/nearblack_${tif}" 2 4 8 16 32
        
        mv "${tmpdir}/nearblack_${tif}" "$outdir/${ts}/${tif}"
        mv "${tmpdir}/${zipfile}" "$indir"

        rm -rf "${tmpdir}/"

        gdaltindex "${outdir}/${dsname}${ts}.shp" "${outdir}/${ts}/${tif}"
        
        

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

    ##### get a list of new files #####

    lftp "$baseurl" -e "mirror --script=${mirrorfile} ; exit"

    lines=$(wc -l "${mirrorfile}" | cut -d " " -f 1)
    ((donelines=0))
    started=$(date +%s)

    ##### loop over the list #####

    while read line ;
    do

        if echo "$line" | grep -e "^mkdir" > /dev/null
        then
            lftp -e "$line ; exit"
            ((donelines +=1))
            continue
        fi

        if [ $doing -lt $limit ]
        then
        	dofile "$line" > /dev/null 2> /dev/null &
         	((doing +=1))
        else
            read <&3
            ((doing -=1))
            ((donelines +=1))
            percdone=$(echo "scale = 6; $donelines / $lines" | bc)
            now=$(date +%s)
            ((elap = now - started))
            compsec=$(echo "scale=0; $elap / $percdone" | bc)
            ((compsec +=  started))
            printf "\r%1.2f complete. EST. finish at %s" $percdone "$(date -d "@${compsec}")"
        	dofile "$line" > /dev/null 2> /dev/null &
         	((doing +=1))
        fi

    done < "${mirrorfile}"

    wait
    echo

    ##### loop over each date of data we got #####

    grep "${mirrorfile}" -e "^get" |\
     sed 's:.*/AE00N[0-9]\{2\}_[a-zA-Z0-9]\{10\}_[0-9]\{6\}\([0-9]\{8\}\).*:\1:' |\
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
        
        makeoverview "$ts" "$extent"
        
     done 
}

