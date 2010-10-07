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

urlcgibin="@urlcgibin@"
htmlbase="@htmlbase@"

###############################################################################
# function to proccess a file
###############################################################################

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

        rm -rf "${tmpdir}"

        gdaltindex "${outdir}/${dsname}${ts}.shp" "${outdir}/${ts}/${tif}"
        
        

    fi

    echo >&3
}


###############################################################################
# function to get the extent of the ds
###############################################################################

function getextent {
    ts="$1"

    ogrinfo -so -al "${outdir}/${dsname}${ts}.shp" |\
     grep Extent: |\
     sed -e 's/) - (/ /' -e 's/Extent: (//' -e 's/,//' -e 's/)//'
}

###############################################################################
# function to write out a map file
###############################################################################

function writemap {
    ts="$1"
    extent="$2"

    if [[ "$doovr" == "yes" ]]
    then
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

    else
        cat > "${outdir}/${dsname}${ts}.map" << EOF
        
  LAYER
    NAME '${dsname}_${ts}'
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
      'wms_title'        '${dsname}_${ts}'
      'wms_srs'          'EPSG:900913 EPSG:4326'
      'wms_extent'	 '$extent'
    END
    OFFSITE 0 0 0
    TILEINDEX '${outdir}/${dsname}${ts}.shp'


  END

EOF
    fi
    
    
}

###############################################################################
# function to add an include line in the main mapfile
###############################################################################

function addinclude {
    ts="$1"

    if ! grep $mapfile -e "${outdir}/${dsname}${ts}.map" > /dev/null
    then
        linenum=$(cat "$mapfile" |\
                   grep -n -e "^[ ]*END[ ]*$" |\
                   tail -n 1 |\
                   cut -d ":" -f 1
                 )

        ed -s "$mapfile" << EOF
${linenum}-1a
  INCLUDE '${outdir}/${dsname}${ts}.map'
.
w
EOF
    fi

}

###############################################################################
# functiom to create a overview from the dataset
###############################################################################

function makeoverview {
    ts="$1"
    extent="$2"

    ${mapserverpath}/shp2img -m "$mapfile" -l "${dsname}_${ts}_nominmax" \
     -o "${outdir}/${ts}/overview.tif" -s 12000 8000 -i image/tiff -e $extent

}

###############################################################################
# est completion time meter
###############################################################################

function comp_meter {

    started=$1
    lines=$2
    donelines=$3
    
    decdone=$(echo "scale = 6; $donelines / $lines" | bc)
    percdone=$(echo "scale = 0; $decdone * 100" | bc)
    elap=$(($(date +%s) - started))
    comp=$(echo "scale=0; $elap / $decdone" | bc)
    ((comp +=  started))
    
    printf "\r%3.0f%% complete. EST. finish at %s" $percdone "$(date -d "@${comp}")"
}

        	

###############################################################################
# multi proceessing loop
###############################################################################

function mainloop {
    mirrorfile="$1"
    dofunc="$2"
    
    ((doing=0))
    
    ##### open a fd to a named pipe #####

    mkfifo pipe; exec 3<>pipe
    
    ##### setup for the est completion time #####
    
    lines=$(wc -l "${mirrorfile}" | cut -d " " -f 1)
    ((donelines=0))
    started=$(date +%s)
    
    ##### loop over the list #####

    while read line ;
    do
        
        ##### if it is a mkdir command do it now #####
        
        if echo "$line" | grep -e "^mkdir" > /dev/null
        then
            lftp -e "$line ; exit"
            
            ((donelines++))
            
            continue
        fi
        
        ##### under the limit just start a job #####

        if [ $doing -lt $limit ]
        then
        	${dofunc} "$line"  &
         	((doing++))
         	
        ##### over the limit wait for a job to finish before starting #####
        
        else
            read <&3
            ((doing--))
            
            ((donelines++))
            comp_meter $started $lines $donelines
            
            ${dofunc} "$line"  &
         	((doing++))
        fi

    done < "${mirrorfile}"

    wait
    echo

}

###############################################################################
# function to create a js with the layers for openlayers and a geoext folder
###############################################################################


function dogeoext {

(
    cat << EOF

Ext.onReady(function() {

  var ${dsname}_layers = [];

EOF

    for map in $(find $outdir -name "*.map")
    do
        if [[ "$doovr" == "yes" ]]
        then
            layer=$(grep "$map" -e GROUP | cut -d "'" -f 2 | uniq )
        else
            layer=$(grep "$map" -e NAME  | cut -d "'" -f 2 | uniq )
        fi
        
        cat << EOF
    
  ${layer} = new OpenLayers.Layer.WMS(
    "${layer}",
    "$urlcgibin",
    {
      layers: '${layer}',
      format: 'image/png',
      transparency: 'TRUE',
    },
    {
      isBaseLayer: false,
      visibility: false,
    }
  );

  ${dsname}_layers.push( ${layer} );

EOF
    done

    cat << EOF
    
  ${dsname}_store = new GeoExt.data.LayerStore(
    {
      initDir: 0,
      layers: ${dsname}_layers
    }
  );

  ${dsname}_list = new GeoExt.tree.OverlayLayerContainer(
    {
      text: '${dsname}',
      layerStore: ${dsname}_store,
      leaf: false,
      nodeType: "gx_overlaylayercontainer",
      expanded: true,
      applyLoader: false
    }
  );

  layerRoot.appendChild(${dsname}_list);

});

EOF


) > ${htmlbase}/${dsname}.js

    ##### make sure the js is loaded by index.html
    
    if ! grep "${htmlbase}/index.html" -e "${dsname}.js" > /dev/null
    then
        linenum=$(cat "$mapfile" |\
                   grep -n -e "finish.js" |\
                   tail -n 1 |\
                   cut -d ":" -f 1
                 )

        ed -s "$mapfile" << EOF
${linenum}-1a
        <script type="text/javascript" src="${dsname}.js"></script>
.
w
EOF
    fi


}

###############################################################################
# function to get a ts from a lftp command
###############################################################################

function dodate {
    sed 's:.*/AE00N[0-9]\{2\}_[a-zA-Z0-9]\{10\}_[0-9]\{6\}\([0-9]\{8\}\).*:\1:'
}

###############################################################################
# function to add the data to the map file and create overviews
###############################################################################

function finishup {
    mirrorfile="$1"
    datefunc="$2"
    
    ##### loop over each date of data we got #####

    grep "${mirrorfile}" -e "^get" |\
     ${datefunc} |\
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

        if [[ "$doovr" == "yes" ]]
        then
        
            ##### create a single file for larger area overviews #####
        
            makeoverview "$ts" "$extent"
            
        fi
        
        #####  write out a js file for geoext #####
        
        dogeoext
     done 
}

###############################################################################
# function to get the a list of new files
###############################################################################

function getlist {
    mirrorfile="$1"
    patern="$2"

    lftp "$baseurl" -e "mirror --script=${mirrorfile} -I "$patern"; exit"
    
}

###############################################################################
# main
###############################################################################

function main {
    

    ##### SANITY CHECKS #####
    
    if ! [[ -n "${dsname}" ]]
    then
        echo "ERROR: var dsname not set"
        exit
    fi
    
    if ! [[ -n "${baseurl}" ]]
    then
        echo "ERROR: var baseurl not set"
        exit
    fi
    
    if ! [[ -n "${basedir}" ]]
    then
        echo "ERROR: var basedir not set"
        exit
    fi
    
    if ! [ -d "$basedir" ]
    then
        echo "ERROR: no such dir $basedir"
        exit
    fi
    
    if ! [ -w "$basedir" ]
    then
        echo "ERROR: no write access to $basedir"
        exit
    fi
    
    if ! [[ -n "${indir}" ]]
    then
        echo "ERROR: var indir not set"
        exit
    fi
    
    
    if ! [ -d "$indir" ]
    then
        if ! mkdir -p "$indir"
        then
            exit
        fi
    fi
    
    if ! [ -w "$indir" ]
    then
        echo "ERROR: no write access to $indir"
        exit
    fi
    
    if ! [[ -n "${outdir}" ]]
    then
        echo "ERROR: var outdir not set"
        exit
    fi
    
    if ! [ -d "$outdir" ]
    then
        if ! mkdir -p "$outdir"
        then
            exit
        fi
    fi
    
    if ! [ -w "$outdir" ]
    then
        echo "ERROR: no write access to $outdir"
        exit
    fi
    
    if ! [ -n "$tmp" ] ; then tmp="/tmp/" ; fi
    
    if ! [ -d "$tmp" ]
    then
        echo "ERROR: no such dir $tmp"
        exit
    fi
    
    if ! [ -w "$tmp" ]
    then
        echo "ERROR: no write access to $tmp"
        exit
    fi
    
    if ! [[ -n "${mapfile}" ]]
    then
        echo "ERROR: var mapfile not set"
        exit
    fi
    
    if ! [ -f "$mapfile" ]
    then
        echo "ERROR: no such file $mapfile"
        exit
    fi

    if ! [ -w "$mapfile" ]
    then
        echo "ERROR: no write access to $mapfile"
        exit
    fi
    
    if ! [ -n "$mapserverpath" ] ; then mapserverpath="/usr/local/src/mapserver/mapserver/" ; fi
    
    if ! [ -d "$mapserverpath" ]
    then
        echo "ERROR: no such dir $mapserverpath"
        exit
    fi
    
    if ! [ -x "${mapserverpath}/shp2img" ]
    then
        echo "ERROR: no executable ${mapserverpath}/shp2img"
        exit
    fi
    
    if ! [ -n "$doovr" ] ; then doovr="yes" ; fi

    
    ##### setup proccess management #####
    
    if ! [ -n "$limit" ] ; then limit="4" ; fi
    
    ##### cd to the in dir #####

    
    cd "$indir"

    host="$(hostname)"
    mirrorfile="$host.mirror.lftp"
    
    ##### get the list of new files to fetch #####
    
    if ! [ -n "$fetchpattern" ] ; then fetchpattern="*" ; fi

    if ! getlist "$mirrorfile" "$fetchpattern"
    then
        exit
    fi
    
    ##### loop over the commands in the mirrorfile #####
    
    if ! [ -n "$dofunc" ] ; then dofunc="dofile" ; fi
    
    if ! mainloop "$mirrorfile" "$dofunc"
    then
        exit
    fi

    ##### finish up, make overvies etc... #####
    
    if ! [ -n "$datefunc" ] ; then datefunc="dodate" ; fi

    if ! finishup "$mirrorfile" "$datefunc"
    then
        exit
    fi

    
}
