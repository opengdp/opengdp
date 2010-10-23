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
urlbase="@urlbase@"
htmlbase="@htmlbase@"


export GDAL_CACHEMAX=500


###############################################################################
# function to proccess a single file
###############################################################################

function dosubimg {
    local img="$1"
    local tmpdir="$2"
    local ts="$3"
    local info="$4"
    
    local imgfile="${img##*/}"
    local imgbase="${imgfile%.*}"
    local imgext="${imgfile##*.}"
    local imgextlower="$(echo $imgext | tr [A-Z] [a-z])"
    
    local imgdir="${img%/*}"
    if [[ "$imgdir" = "$imgfile" ]]
    then
        local imgdir=""
    else
        local imgdir="${imgdir}/"
    fi

        
    ##### test if the image is in a lossy format #####
    
    if grep -e "COMPRESSION=.*JPEG" <<< "$info" > /dev/null || \
       [[ "$imgextlower" == "sid" ]]
       [[ "$imgextlower" == "pdf" ]]
    then
        local islossy=true
    fi
    
    ##### test the projection ####
       
    if ! echo $info | grep 'GEOGCS\["WGS 84", DATUM\["WGS_1984", SPHEROID\["WGS 84",6378137,298.257223563, AUTHORITY\["EPSG","7030"\]\], AUTHORITY\["EPSG","6326"\]\], PRIMEM\["Greenwich",0\], UNIT\["degree",0.0174532925199433\], AUTHORITY\["EPSG","4326"\]\]' > /dev/null
    then
        echo "needs warped"
        ##### does the image not have an alpha band? #####
        
        if ! echo "$info" | grep 'ColorInterp=Alpha' > /dev/null
        then
            echo "needs warped needs alpha"
            
            #####  create a mask with the nearblack method #####
            
            if [[ "$islossy" == "true" ]]
            then
                gdalmask -nearblack -internal "${tmpdir}/${img}" \
                         "${tmpdir}/prewarp_${imgbase}.tif" > /dev/null
            else
                gdalmask -nearblack -near 0 -internal "${tmpdir}/${img}" \
                         "${tmpdir}/prewarp_${imgbase}.tif" > /dev/null
            fi
            rm "${tmpdir}/${img}"
                 
            ##### needs warped #####
        
            gdalwarp -co TILED=YES \
                     -dstalpha \
                     -t_srs EPSG:4326 \
                     "${tmpdir}/prewarp_${imgbase}.tif" \
                     "${tmpdir}/warped_${imgbase}.tif" > /dev/null
            
            #####  create a mask and compress #####
            
            gdalmask -co TILED=YES -co COMPRESS=JPEG -co PHOTOMETRIC=YCBCR \
                     -nearblack -near 0 -internal \
                     "${tmpdir}/warped_${imgbase}.tif" \
                     "${tmpdir}/final_${imgbase}.tif" > /dev/null
            rm "${tmpdir}/warped_${imgbase}.tif"

        ##### since it has a alpha band already skip the nearblack #####
        
        else
            echo "needs warped has alpha"
        
            ##### needs warped #####
        
            gdalwarp -co TILED=YES \
                     -t_srs EPSG:4326 \
                     "${tmpdir}/${img}" \
                     "${tmpdir}/warped_${imgbase}.tif"  > /dev/null
            rm "${tmpdir}/${img}"
            
            #####  create a mask and compress #####
            
            gdalmask -co TILED=YES -co COMPRESS=JPEG -co PHOTOMETRIC=YCBCR \
                     -nearblack -near 0 -internal \
                     "${tmpdir}/warped_${imgbase}.tif" \
                     "${tmpdir}/final_${imgbase}.tif"  > /dev/null
            rm "${tmpdir}/warped_${imgbase}.tif"
            
        fi
            
    ##### already the right proj #####
      
    else
        echo "no warp"
        
        ##### if the source is anything but a tif or does #####
        ##### not have a alpha band we need to copy       #####
        
        if ! echo "$info" | grep 'ColorInterp=Alpha' > /dev/null ||
           [[ "${imgextlower}" != "tif" ]]
        then
            
            echo "no warp no alpha"
            
            #####  create a mask and compress #####
            if [[ "$islossy" == "true" ]]
            then
                gdalmask -co TILED=YES -co COMPRESS=JPEG -co PHOTOMETRIC=YCBCR \
                         -nearblack -internal \
                         "${tmpdir}/${img}" \
                         "${tmpdir}/final_${imgbase}.tif" > /dev/null
            else
                gdalmask -co TILED=YES -co COMPRESS=JPEG -co PHOTOMETRIC=YCBCR \
                         -nearblack -near 0 -internal \
                         "${tmpdir}/${img}" \
                         "${tmpdir}/final_${imgbase}.tif" > /dev/null
            fi
            rm "${tmpdir}/${img}"
            
      
        else
           echo "no warp has alpha"

            gdalmask -co TILED=YES -co COMPRESS=JPEG -co PHOTOMETRIC=YCBCR \
                     -internal \
                     "${tmpdir}/${img}" \
                     "${tmpdir}/final_${imgbase}.tif" > /dev/null
            rm "${tmpdir}/${img}"
            
        fi

    fi
        
    ##### get the xy size in pixels #####
        
    read x y < <(echo "$info" | grep -e "Size is" | sed 's/Size is \([0-9]*\), \([0-9]*\)/\1 \2/')
    
    ##### add overviews #####
    
    ((o=2))
    ovrs=""
    while (( o <= x / 64 )) || (( o <= y / 64 ))
    do
        ovrs="$ovrs $o"
        ((o*=2))
    done
    
    gdaladdo -r average "${tmpdir}/final_${imgbase}.tif" $ovrs  > /dev/null
        
    ##### add a timestamp for indexers #####
                
    tiffset -s 306 \
                "${ts:0:4}:${ts:4:2}:${ts:6:2} 12:00:00" \
                "${tmpdir}/final_${imgbase}.tif"  > /dev/null
        
    ##### move the output to the outdir #####
        
    mv "${tmpdir}/final_${imgbase}.tif" "$outdir/${ts}/${imgbase}.tif"
        
    ##### add the file to the tile index #####
    
    ##### lock! #####
    
    lock="${outdir}/${dsname}${ts}.shp"
    lock="${lock//\//.}"
    
    while ! mkdir "${lock}"
	do
		sleep 1
	done
    
    gdaltindex "${outdir}/${dsname}${ts}.shp" "${outdir}/${ts}/${imgbase}.tif"  > /dev/null
    
    ##### unlock #####
    
    rmdir "${lock}"
    
    if [[ "$haveomar" == "yes" ]]
    then
        curl --data "filename=${outdir}/${ts}/${imgbase}.tif" \
             "${urlbase}/omar/dataManager/addRaster"
    fi
    
}



###############################################################################
# function to bust a larger image into chunks and proccess
###############################################################################

function doimg {
    local img="$1"
    local tmpdir="$2"
    local ts="$3"
    local info="$4"
    
    local imgfile="${img##*/}"
    local imgbase="${imgfile%.*}"
    local imgext="${imgfile##*.}"
    local imgdir="${img%/*}"
    
    local imgdir="${img%/*}"
    if [[ "$imgdir" = "$imgfile" ]]
    then
        local imgdir=""
    else
        local imgdir="${imgdir}/"
    fi

        
    #printf " img=%s\n tmpdir=%s\n ts=%s\n imgfile=%s\n imgbase=%s\n imgext=%s\n imgdir=%s\n" \
    #        "$img" \
    #        "$tmpdir "\
    #        "$ts" \
    #        "$imgfile" \
    #        "$imgbase" \
    #        "$imgext" \
    #        "$imgdir"
    #return

    
    ##### get the xy size in pixels #####
        
    read x y < <(echo "$info" | grep -e "Size is" | sed 's/Size is \([0-9]*\), \([0-9]*\)/\1 \2/')
    
    ##### is the img too big? #####
    
    ((cutat=16384))
    
    
    if [[ $x -gt $cutat ]] || [[ $y -gt $cutat ]]
    then
        
        ##### loop over x #####
        
        ((xsize = $cutat))
        for ((xoff = 0; xoff < x; xoff += $cutat))
        do
            
            ##### set the x size of the sub img #####
            
            if ((xoff + xsize >= x))
            then
                ((xsize = x - xoff))
            fi
                
            ##### loop over y #####
            
            ((ysize = $cutat))
            for ((yoff = 0; yoff < y; yoff += $cutat))
            do
                
                ##### set the y size of the sub img #####
            
                if ((yoff + ysize >= y))
                then
                    ((ysize = y - yoff))
                fi
                
                ##### translate #####
                
                gdal_translate -srcwin $xoff $yoff $xsize $ysize \
                               "${tmpdir}/${img}"\
                               "${tmpdir}/${imgdir}${imgbase}_${xoff}_${yoff}.vrt"
        
                dosubimg "${imgdir}/${imgbase}_${xoff}_${yoff}.vrt" \
                         "$tmpdir" "$ts" \
                         "$(gdalinfo "${tmpdir}/${imgdir}${imgbase}_${xoff}_${yoff}.vrt")"

            done
        done
    
        ##### rm the original image to free up ramdisk as fast as posible #####
    
        rm "${tmpdir}/${img}"
    
    ##### its not too big do the image as is #####
    
    else
    
        dosubimg "${img}" \
                 "$tmpdir" "$ts" \
                 "$info"
    
    fi

}

     
###############################################################################
# function to process a tar file
###############################################################################

function dotar {
    local zipfile="$1"
    local tmpdir="$2"
    local ts="$3"
    local origdir="$4"
   
    
    for f in $(tar -tf "${origdir}/${zipfile}" --wildcards "$extglob")
    do
        local imgfile="${f##*/}"
        local imgbase="${imgfile%.*}"
        local imgext="${imgfile##*.}"
        
        local imgdir="${f%/*}"
        if [[ "$imgdir" = "$imgfile" ]]
        then
            local imgdir=""
        else
            local imgdir="${imgdir}/"
        fi

        tar -xf "${origdir}/${zipfile}" -C "$tmpdir" "$f"
        
        ##### try to unzip a world file if its there #####
        
        tar -xf "${origdir}/${zipfile}" -C "$tmpdir" "${imgdir}${imgbase}.??w"
        
        local info=$(gdalinfo "${tmpdir}/${f}")
        doimg "$f" "$tmpdir" "$ts" "$info"
    done
    
}

###############################################################################
# function to process a tar.gz file
###############################################################################

function dotargz {
    local zipfile="$1"
    local tmpdir="$2"
    local ts="$3"
    local origdir="$4"
    

    for f in $(tar -tzf "${origdir}/${zipfile}" --wildcards "$extglob")
    do
        local imgfile="${f##*/}"
        local imgbase="${imgfile%.*}"
        local imgext="${imgfile##*.}"
        
        local imgdir="${f%/*}"
        if [[ "$imgdir" = "$imgfile" ]]
        then
            local imgdir=""
        else
            local imgdir="${imgdir}/"
        fi

        
        tar -xzf "${origdir}/${zipfile}" -C "$tmpdir" "$f"
        
        ##### try to unzip a world file if its there #####
        
        tar -xzf "${origdir}/${zipfile}" -C "$tmpdir" "${imgdir}${imgbase}.??w"
        
        local info=$(gdalinfo "${tmpdir}/${f}")
        doimg "$f" "$tmpdir" "$ts" "$info"
        
    done
}

###############################################################################
# function to process a tar.gz file
###############################################################################

function dotarbz2 {
    local zipfile="$1"
    local tmpdir="$2"
    local ts="$3"
    local origdir="$4"
    

    for f in $(tar -tjf "${origdir}/${zipfile}" --wildcards "$extglob")
    do
        local imgfile="${f##*/}"
        local imgbase="${imgfile%.*}"
        local imgext="${imgfile##*.}"
        
        local imgdir="${f%/*}"
        if [[ "$imgdir" = "$imgfile" ]]
        then
            local imgdir=""
        else
            local imgdir="${imgdir}/"
        fi

        tar -xjf "${origdir}/${zipfile}" -C "$tmpdir" "$f"
        
        ##### try to unzip a world file if its there #####
        
        tar -xjf "${origdir}/${zipfile}" -C "$tmpdir" "${imgdir}${imgbase}.??w"
        
        local info=$(gdalinfo "${tmpdir}/${f}")
        doimg "$f" "$tmpdir" "$ts" "$info"
        
    done
}

###############################################################################
# function to process a zip file
###############################################################################

function dozip {
    local zipfile="$1"
    local tmpdir="$2"
    local ts="$3"
    local origdir="$4"
    

    for f in $(zipinfo -1 "${origdir}/${zipfile}" "$extglob")
    do
        local imgfile="${f##*/}"
        local imgbase="${imgfile%.*}"
        local imgext="${imgfile##*.}"

        local imgdir="${f%/*}"
        if [[ "$imgdir" = "$imgfile" ]]
        then
            local imgdir=""
        else
            local imgdir="${imgdir}/"
        fi  

        unzip "${origdir}/${zipfile}" "$f" -d "$tmpdir"
        
        ##### try to unzip a world file if its there #####
        
        unzip "${origdir}/${zipfile}" "${imgdir}${imgbase}.??w" -d "$tmpdir"
        
        local info=$(gdalinfo "${tmpdir}/${f}")
        doimg "$f" "$tmpdir" "$ts" "$info"
        
    done

}

###############################################################################
# function to process a kmz file
###############################################################################

function dokmz {
    local zipfile="$1"
    local tmpdir="$2"
    local ts="$3"
    local origdir="$4"

    for f in $(zipinfo -1 "${origdir}/${zipfile}" "$baseglob.kml")
    do
        
        ##### extract the kml #####
        
        unzip "${origdir}/${zipfile}" "$f" -d "$tmpdir"
        
        ##### find and extract the corisponding img #####
        
        local img=$(grep '<GroundOverlay>' -A12 "$kml" |\
                    grep href | sed -r 's|.*<href>(.*)</href>.*|\1|')
        
        unzip "${origdir}/${zipfile}" "$img" -d "$tmpdir"
        
        local imgfile="${img##*}"
        local imgbase="${imgfile%.*}"
        local imgext="${imgfile##*.}"
        
        local imgdir="${f%/*}"
        if [[ "$imgdir" = "$imgfile" ]]
        then
            local imgdir=""
        else
            local imgdir="${imgdir}/"
        fi

        ##### get the corner quords #####
        
        read n s e w < <(grep '<GroundOverlay>' -A12 "$kml" |\
                          grep north -A3 |\
                          sed 's:<[/a-z]*>::g' |\
                          tr "\n" " ")
        
        ##### get the xy size in pixels #####
        
        read x y < <(gdalinfo "${tmpdir}/$img" |\
                      grep -e "Size is" |\
                      sed 's/Size is \([0-9]*\), \([0-9]*\)/\1 \2/')
        
        ##### calc the pixel size #####
        
        xp=$(echo "scale = 16; ($e - $w) / $x" | bc)
        yp=$(echo "scale = 16; ($s - $n) / $y" | bc)
        
        ##### decide the world file ext #####
        
        case "$imgext" in
            jpg)
                world="jpw"
                ;;
            tif)
                world="tfw"
                ;;
            png)
                world="pgw"
                ;;
        esac

        ##### write out the world file #####
        
        cat > "${tmpdir}/${imgdir}/${imgbase}.$world" <<EOF
$xp
0.0000000000000000
0.0000000000000000
$yp
$w
$n
EOF

        ##### create a vrt with the proj #####
        
        gdal_translate -a_srs EPSG:4326 \
                       -of VRT \
                       "${tmpdir}/${img}" \
                       "${tmpdir}/${imgdir}/${imgbase}.vrt"
                       
        ##### proccess #####
        
        local info=$(gdalinfo "${tmpdir}/${imgdir}/${imgbase}.vrt")
        doimg "${tmpdir}/${imgdir}/${imgbase}.vrt" "$tmpdir" "$ts" "$info"
    done

}
        
###############################################################################
# function to proccess a file
###############################################################################

function dofile {
    myline=$1

    if echo "$myline" | grep -e "^get" > /dev/null
    then    
        local sourcedir=${indir//\/\///}
        local sourcedir=${sourcedir//\/\///}
        
        local file="${myline##*/}"
        local base="${file%.*}"
        local ext="${file#*.}"
        local ext="$(echo $ext | tr [A-Z] [a-z])"
        #local ext="${ext,,*}"
        
        if echo "$myline" | grep -e "$sourcedir" > /dev/null
        then
            local dir="$(echo "$myline" | sed "s|.*$sourcedir\(.*\) $url.*|\1|")/"
        else
            local dir=""
        fi
         
        ts=$(echo "$myline" | dodate)
        
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
            lftp -e "$(echo "$myline" | sed "s:get -O [/_.A-Za-z0-9]*:get -O ${tmpdir}:") ; exit"
            local origdir="$tmpdir"
        fi

        if ! [ -d "$outdir/${ts}" ]
        then
            mkdir -p "$outdir/${ts}"
        fi
        
        case "$ext" in
        
            *tar)
                dotar "${dir}${file}" "$tmpdir" "$ts" "$origdir"
                ;;
                
            *tar.gz)
                dotargz "${dir}${file}" "$tmpdir" "$ts" "$origdir"
                ;;
            
            *tgz)
                dotargz "${dir}${file}" "$tmpdir" "$ts" "$origdir"
                ;;
            
            *tar.bz2)
                dotarbz2 "${dir}${file}" "$tmpdir" "$ts" "$origdir"
                ;;
            
            *zip)
                dozip "${dir}${file}" "$tmpdir" "$ts" "$origdir"
                ;;
            
            *kmz)
                dokmz "${dir}${file}" "$tmpdir" "$ts" "$origdir"
                ;;

#fixme this does not take into account world files that may be there too
            
            *)
                if gdalinfo "${$origdir}/${dir}/${file}"
                then
                    doimg "{$origdir}/${dir}/${file}"
                          "$tmpdir"
                          "$ts"
                          $(gdalinfo "${origdir}/${dir}/${file}")
                fi

            esac
        
        if [[ "$DWH_REBUILD" != "rebuild" ]]
        then
            mv "${tmpdir}/${dir}/${file}" "${indir}/${dir}/${file}"
        fi

        rm -rf "${tmpdir}"
    
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
    local ts="$1"
    local extent="$2"
    local scale="$3"

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

    TILEINDEX '${outdir}/${dsname}${ts}.shp'
    GROUP '${dsname}_${ts}'
    MAXSCALEDENOM $scale


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

    DATA '${outdir}/${ts}/overview.tif'
    GROUP '${dsname}_${ts}'
    MINSCALEDENOM $scale


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

function min {

    if [[ $(bc <<< "scale = 16; $1-$2") = -* ]]
    then
        echo "$1"
    else
        echo "$2";
    fi

}


      
###############################################################################
# functiom to create a overview from the dataset
###############################################################################

function makeoverview {
    local ts="$1"
    local extent="$2"
    
    local tmpdir=$(mktemp -d -p "$tmp" "${dsname}XXXXXXXXXX")

    gdalbuildvrt -overwrite \
                 "${tmpdir}/${dsname}${ts}.vrt" \
                 "${outdir}/${dsname}${ts}.shp" > /dev/null

    read xr yr < <(gdalinfo "${tmpdir}/${dsname}${ts}.vrt" |\
                    grep -e "Pixel Size" |\
                    sed 's/.*Pixel Size = ([-]*\([.0-9]*\),[-]*\([.0-9]*\)).*/\1 \2/')

    local orig=$(bc <<< "scale = 16; $(min $xr $yr) * 72 * 4374754")
    local scale=$(bc <<< "$orig + 30000")

    ##### res = scale / (72 * 4374754) #####
    
    res=$(bc <<< "scale = 16; $scale / (72 * 4374754)")
    
    gdalbuildvrt -overwrite -tr $res $res \
                 "${tmpdir}/${dsname}${ts}_2.vrt" \
                 "${outdir}/${dsname}${ts}.shp" > /dev/null
    
    read x y < <( gdalinfo "${tmpdir}/${dsname}${ts}_2.vrt" |\
                   grep -e "Size is" |\
                   sed 's/Size is \([0-9]*\), \([0-9]*\)/\1 \2/')

    ${mapserverpath}/shp2img -m "$mapfile" -l "${dsname}_${ts}_nominmax" \
     -o "${tmpdir}/${dsname}${ts}.tif" -s $x $y -i image/tiff -e $extent > /dev/null


    gdalmask -internal -co TILED=YES -co COMPRESS=JPEG -co PHOTOMETRIC=YCBCR \
             "${tmpdir}/${dsname}${ts}.tif" \
             "${outdir}/${ts}/overview.tif" > /dev/null
    
    ##### add overviews #####
    
    ((o=2))
    ovrs=""
    while (( o <= x / 64 )) || (( o <= y / 64 ))
    do
        ovrs="$ovrs $o"
        ((o*=2))
    done
    
    gdaladdo -r average "${outdir}/${ts}/overview.tif" $ovrs  > /dev/null
    
    rm -rf ${tmpdir} > /dev/null

    echo ${scale%.*}
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

    for map in $(find $outdir -name "*.map" | sort )
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
      visibility: false
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
        linenum=$(cat "${htmlbase}/index.html" |\
                   grep -n -e "finish.js" |\
                   tail -n 1 |\
                   cut -d ":" -f 1
                 )

        ed -s "${htmlbase}/index.html" << EOF
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

    if [[ "$DWH_REOVER" != "reover" ]]
    then
        grep "${mirrorfile}" -e "^get" |\
         ${datefunc}
    else
        find "${outdir}" -type d -name "[0-9]*" |\
        sed 's:.*/::g'
    fi |\
     sort |\
     uniq |\
     while read ts
     do
        
        ##### write the map file #####

        writemap "$ts" "$extent" "50000"

        ##### add an include line in the main mapfile #####
        
        addinclude "$ts"

        ##### get the extent of the ds #####
        
        extent=$(getextent "$ts")

        if [[ "$doovr" == "yes" ]]
        then
        
            ##### create a single file for larger area overviews #####
        
            scale=$(makeoverview "$ts" "$extent")
            
        fi

        ##### rewrite the map file with the correct scale #####

        writemap "$ts" "$extent" "$scale"

        ##### add an include line in the main mapfile #####
        
        addinclude "$ts"

        
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
    

    ##### dsname #####
    
    if ! [[ -n "${dsname}" ]]
    then
        echo "ERROR: var dsname not set"
        exit
    fi
    
    ##### fetch url #####
    
    if ! [[ -n "${baseurl}" ]]
    then
        echo "ERROR: var baseurl not set"
        exit
    fi
    
    ##### basedir #####
    
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
    
    ##### indir #####
    
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
    
    ##### outdir #####
    
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
    
    ##### tmp dir #####
    
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
    
    ##### mapfile #####
    
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
    
    ##### mapserverpath #####
    
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
    
    ##### do ovrview default #####
    
    if ! [ -n "$doovr" ] ; then doovr="yes" ; fi

    ##### fetch pattern default #####
    
    if ! [ -n "$fetchpattern" ] ; then fetchpattern="*" ; fi
    
    ##### unzip untar... pattern default #####
    
    if ! [ -n "$extglob" ] ; then extglob="*.tif" ; fi
    
    ##### unzip kmz pattern default #####
    
    if ! [ -n "$baseglob" ] ; then baseglob="tile-*" ; fi
    
    ##### proccess limit default #####
    
    if ! [ -n "$limit" ] ; then limit="4" ; fi
    
    if ! [ -n "$datefunc" ] ; then datefunc="dodate" ; fi

    ##### cd to the in dir #####

    cd "$indir"

    ##### file name for the mirror file #####
    
    host="$(hostname)"
    mirrorfile="$host.mirror.lftp"

    ##### check if called for a rebuild #####

    if [[ "$DWH_REBUILD" == "rebuild" ]]
    then
        echo "rebuilding"

        if ! mv "$indir" "${indir/%\//}.old"
        then
            exit
        fi

        if ! mv "$outdir" "${outdir/%\//}.old"
        then
            exit
        fi

        if ! mkdir -p "$indir"
        then
            exit
        fi
        
        cd "$indir"

        baseurl="file://${indir/%\//}.old"

    fi
    
    if [[ "$DWH_REOVER" != "reover" ]]
    then
    
        ##### get the list of new files to fetch #####
        
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

    
    else
    
        ##### finish up, make overvies etc... #####
    
        if ! finishup "$mirrorfile" "$datefunc"
        then
            exit
        fi
    fi


    if [[ "$DWH_REBUILD" == "rebuild" ]]
    then
        echo "rebuilding"
        rm -r "$indir"

        mv "${indir/%\//}.old" "$indir"
    
    fi

}

