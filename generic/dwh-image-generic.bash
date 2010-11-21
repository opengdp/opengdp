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

###############################################################################
# function to proccess a single file
###############################################################################

function dosubimg {
    local img="$1"
    local tmpdir="$2"
    local ts="$3"
    local info="$4"
    local islossy="$5"
    local isoriginal="$6"

    local imgfile="${img##*/}"
    local imgbase="${imgfile%.*}"
    local imgext="${imgfile##*.}"
    local imgextlower="$(tr [A-Z] [a-z] <<< "$imgext")"
    
    local imgdir="${img%/*}"
    if [[ "$imgdir" = "$imgfile" ]]
    then
        local imgdir=""
    else
        local imgdir="${imgdir}/"
    fi
    
    ##### if we need a debug do it and return #####

    if [ -n "$DEBUG_dosubimg" ]
    then            
        printf " img=%s\n tmpdir=%s\n ts=%s\n imgfile=%s\n imgbase=%s\n imgext=%s\n imgdir=%s\n islossy=%s\n isoriginal=%s\n " \
              "$img" \
              "$tmpdir "\
              "$ts" \
              "$imgfile" \
              "$imgbase" \
              "$imgext" \
              "$imgdir" \
              "$islossy" \
              "$isoriginal"
        return
    fi

    ###### RAMDISK? #####

    if [ -n "$ramdisk" ]
    then
        local tmpram=$(mktemp -d -p "${ramdisk}" "${dsname}XXXXXXXXXX")
    else
        local tmpram=$(mktemp -d -p "${tmpdir}" "${dsname}XXXXXXXXXX")
    fi

    
    ##### test the projection ####
       
    if ! echo $info | grep 'GEOGCS."WGS 84", DATUM."WGS_1984", SPHEROID."WGS 84",6378137,298.257223563, AUTHORITY."EPSG","7030".., AUTHORITY."EPSG","6326".., PRIMEM.."Greenwich",0.*., UNIT."degree",0.0174532925199433., AUTHORITY."EPSG","4326"..' > /dev/null
    then

        ##### does the image not have an alpha band? #####
        
        if ! echo "$info" | grep 'ColorInterp=Alpha' > /dev/null
        then
            
            #####  create a mask with the nearblack method #####
            
            if [[ "$islossy" == "true" ]]
            then
                if [[ "$nearwhite" == "true" ]]
                then
                    nearblack -co TILED=YES -setmask -white -of GTiff "${tmpdir}/${img}" \
                              -o "${tmpram}/prewarp_${imgbase}.tif" > /dev/null
                else
                    nearblack -co TILED=YES -setmask -of GTiff "${tmpdir}/${img}" \
                             -o "${tmpram}/prewarp_${imgbase}.tif" > /dev/null
                fi
            else
                if [[ "$nearwhite" == "true" ]]
                then
                    nearblack -co TILED=YES -setmask -near 0 -white -of GTiff "${tmpdir}/${img}" \
                             -o "${tmpram}/prewarp_${imgbase}.tif" > /dev/null
                else
                    nearblack -co TILED=YES -setmask -near 0 -of GTiff "${tmpdir}/${img}" \
                             -o "${tmpram}/prewarp_${imgbase}.tif" > /dev/null
                fi
            fi
            
            if [[ "$isoriginal" == "no" ]]
            then
                rm "${tmpdir}/${img}"
            fi

            ##### needs warped #####
        
            gdalwarp -co TILED=YES \
                     -dstalpha \
                     -t_srs EPSG:4326 \
                     "${tmpram}/prewarp_${imgbase}.tif" \
                     "${tmpram}/warped_${imgbase}.tif" > /dev/null
            
            #####  create a mask and compress #####
            
            rm "${tmpram}/prewarp_${imgbase}.tif"

            gdal_translate -co TILED=YES -co COMPRESS=JPEG -co PHOTOMETRIC=YCBCR \
                     -b 1 -b 2 -b 3 -mask 4 \
                     "${tmpram}/warped_${imgbase}.tif" \
                     "${tmpram}/final_${imgbase}.tif" > /dev/null
            rm "${tmpram}/warped_${imgbase}.tif"

        ##### since it has a alpha band already skip the nearblack #####
        
        else
            echo "needs warped has alpha"
        
            ##### needs warped #####
        
            gdalwarp -co TILED=YES \
                     -t_srs EPSG:4326 \
                     "${tmpdir}/${img}" \
                     "${tmpram}/warped_${imgbase}.tif"  > /dev/null
            
            if [[ "$isoriginal" == "no" ]]
            then
                rm "${tmpdir}/${img}"
            fi

            #####  create a mask and compress #####
            
            gdal_translate -co TILED=YES -co COMPRESS=JPEG -co PHOTOMETRIC=YCBCR \
                     -b 1 -b 2 -b 3 -mask 4 \
                     "${tmpram}/warped_${imgbase}.tif" \
                     "${tmpram}/final_${imgbase}.tif"  > /dev/null

            rm "${tmpram}/warped_${imgbase}.tif"
            
        fi
            
    ##### already the right proj #####
      
    else
        
        ##### if the source is anything but a tif or does #####
        ##### not have a alpha band we need to copy       #####
        
        if ! echo "$info" | grep 'ColorInterp=Alpha' > /dev/null ||
           [[ "${imgextlower}" != "tif" ]]
        then
            
            #####  create a mask and compress #####

            if [[ "$islossy" == "true" ]]
            then
                if [[ "$nearwhite" == "true" ]]
                then
                    nearblack -co TILED=YES -setmask -white -of GTiff \
                             "${tmpdir}/${img}" \
                             -o "${tmpram}/masked_${imgbase}.tif" > /dev/null
                else
                    nearblack -co TILED=YES -setmask -of GTiff \
                             "${tmpdir}/${img}" \
                             -o "${tmpram}/masked_${imgbase}.tif" > /dev/null
                fi
            else
                if [[ "$nearwhite" == "true" ]]
                then
                    nearblack -co TILED=YES -setmask -white -of GTiff \
                             -near 0 \
                             "${tmpdir}/${img}" \
                             -o "${tmpram}/masked_${imgbase}.tif" > /dev/null
                else
                    nearblack -co TILED=YES -setmask -of GTiff \
                             -near 0 \
                             "${tmpdir}/${img}" \
                             -o "${tmpram}/masked_${imgbase}.tif" > /dev/null
                fi
            fi

            if [[ "$isoriginal" == "no" ]]
            then
                rm "${tmpdir}/${img}"
            fi

            ##### translate to compress #####
            
            gdal_translate -co TILED=YES -co COMPRESS=JPEG -co PHOTOMETRIC=YCBCR \
                "${tmpram}/masked_${imgbase}.tif" \
                "${tmpram}/final_${imgbase}.tif" > /dev/null
            
            rm "${tmpram}/masked_${imgbase}.tif"

        else
           echo "no warp has alpha"

            gdal_translate -co TILED=YES -co COMPRESS=JPEG -co PHOTOMETRIC=YCBCR \
                     -b 1 -b 2 -b 3 -mask 4 \
                     "${tmpdir}/${img}" \
                     "${tmpram}/final_${imgbase}.tif" > /dev/null
            
            if [[ "$isoriginal" == "no" ]]
            then
                rm "${tmpdir}/${img}"
            fi

            
        fi

    fi
        
    ##### add overviews #####
    
    addo "${tmpram}/final_${imgbase}.tif"
    
    ##### add a timestamp for indexers #####
    
    tiffset -s 306 \
                "${ts:0:4}:${ts:4:2}:${ts:6:2} 12:00:00" \
                "${tmpdir}/final_${imgbase}.tif" > /dev/null 2> /dev/null
        
    ##### move the output to the outdir #####
    
    mv "${tmpram}/final_${imgbase}.tif" "$outdir/${ts}/${imgbase}.tif"
    
    rm -rf "${tmpram}"
    ##### add the file to the tile index #####
    
    ##### lock! #####
    
    local lock="${outdir}/${dsname}${ts}.shp"
    lock="${lock//\//.}"
    
    while ! mkdir "${lock}" 2> /dev/null
	do
		sleep 1
	done
    
    ##### make the tileindex in a subshell so we can cd with no adverse effect #####
    ##### this costs like 2s of system time per 4000 calls #####

    (
        cd ${outdir}
        gdaltindex "${dsname}${ts}.shp" "${ts}/${imgbase}.tif"  > /dev/null
    )
    
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
    local isoriginal="$5"
    
    
    local imgfile="${img##*/}"
    local imgbase="${imgfile%.*}"
    local imgext="${imgfile##*.}"
    local imgextlower="$(tr [A-Z] [a-z] <<< "$imgext")"
    local imgdir="${img%/*}"
    
    local imgdir="${img%/*}"
    if [[ "$imgdir" = "$imgfile" ]]
    then
        local imgdir=""
    else
        local imgdir="${imgdir}/"
    fi
    
    ##### double check the info #####
    
    if [ -n "$info" ]
    then
        info="$(gdalinfo "${tmpdir}/${img}")"
    fi
   
    ##### if we need a debug do it and return #####

    if [ -n "$DEBUG_doimg" ]
    then
        printf " img=%s\n tmpdir=%s\n ts=%s\n imgfile=%s\n imgbase=%s\n imgext=%s\n imgdir=%s \n" \
               "$img" \
               "$tmpdir "\
               "$ts" \
               "$imgfile" \
               "$imgbase" \
               "$imgext" \
               "$imgdir"
       return
    fi

    ##### test if the image is in a lossy format #####
    
    if grep -e "COMPRESSION=.*JPEG" <<< "$info" > /dev/null || \
       [[ "$imgextlower" == "sid" ]] || [[ "$imgextlower" == "pdf" ]]
    then
        local islossy=true
    fi

    ##### check if the image needs scaled #####
    
    if ! echo "$info" | grep -e Band.1.*Type=Byte > /dev/null && ! [ -n "$rescale" ]
    then
        
        type="$(echo "$info" | grep -e "Band 1 " | sed 's|.*Type=\([a-zA-Z0-9]*\),.*|\1|')"
        case "$type" in

            UInt16)
                local rescale="-32768 32767"
            ;;
            Int16)
                local rescale="0 65535"
            ;;
            UInt32)
                local rescale="-2147483,648 2147483647"
            ;;
            Int32)
                local rescale="0 4294967295"
            ;;

            ##### all other types we just let gdal scale with minmax #####

        esac
        
        local doscale=TRUE
    fi

    if [[ "$doscale" == "TRUE" ]] || [ -n "$rescale" ]
    then

        gdal_translate -of VRT -ot byte -scale $scale \
                       "${tmpdir}/${img}"\
                       "${tmpdir}/${imgdir}${imgbase}_scaled.vrt" > /dev/null


        img="${imgdir}${imgbase}_scaled.vrt"
        imgfile="${img##*/}"
        imgbase="${imgfile%.*}"
        imgext="${imgfile##*.}"
        
    fi
        
    ##### get the xy size in pixels #####
    
    local x
    local y

    read x y < <(echo "$info" | grep -e "Size is"  | sed 's/Size is \([0-9]*\), \([0-9]*\)/\1 \2/' )

    
    ##### is the img too big? #####
    
    ((cutat=16384))
    
    
    if [[ $x -gt $cutat ]] || [[ $y -gt $cutat ]]
    then
        
        ##### loop over x #####

        local xsize
        local xoff

        ((xsize = $cutat))
        for ((xoff = 0; xoff < x; xoff += $cutat))
        do
            
            ##### set the x size of the sub img #####
            
            if ((xoff + xsize >= x))
            then
                ((xsize = x - xoff))
            fi
                
            ##### loop over y #####
            
            local ysize
            local yoff
            
            ((ysize = $cutat))
            for ((yoff = 0; yoff < y; yoff += $cutat))
            do
                
                ##### set the y size of the sub img #####
            
                if ((yoff + ysize >= y))
                then
                    ((ysize = y - yoff))
                fi

                local myislossy="$islossy"

#                ##### make sure if were in the center of the img we turn off islossy #####
#                
#                if [[ "$islossy" == "yes" ]] && \
#                   ((yoff > 0 )) && (( ysize + yoff < y )) && \
#                   ((xoff > 0 )) && (( xsize + xoff < x ))
#                then
#                    myislossy="no"
#                fi
                
                if [ -n "$ramdisk" ]
                then
                    local tmpram=$(mktemp -d -p "${ramdisk}" "${dsname}XXXXXXXXXX")
                else
                    local tmpram=$(mktemp -d -p "${tmpdir}" "${dsname}XXXXXXXXXX")
                fi

                ##### translate #####
                
                if [[ "$imgextlower" == "sid" ]]
                then
                    mrsiddecode -ulxy $xoff $yoff -wh $xsize $ysize  -s 0 \
                                -i "${tmpdir}/${img}" \
                                -o "${tmpram}/${imgdir}${imgbase}_${xoff}_${yoff}.tif" > /dev/null

                    dosubimg "${imgdir}/${imgbase}_${xoff}_${yoff}.tif" \
                             "$tmpram" "$ts" \
                             "$(gdalinfo "${tmpram}/${imgdir}${imgbase}_${xoff}_${yoff}.tif")" \
                             "$myislossy" "no"

                else
                    gdal_translate -of VRT -srcwin $xoff $yoff $xsize $ysize \
                                   "${tmpdir}/${img}"\
                                   "${tmpram}/${imgdir}${imgbase}_${xoff}_${yoff}.vrt" > /dev/null
            
                    dosubimg "${imgdir}/${imgbase}_${xoff}_${yoff}.vrt" \
                             "$tmpram" "$ts" \
                             "$(gdalinfo "${tmpram}/${imgdir}${imgbase}_${xoff}_${yoff}.vrt")" \
                             "$myislossy" "no"
                fi
                
                rm -rf "$tmpram"

            done
        done
    
    ##### its not too big do the image as is #####
    
    else
    
        dosubimg "${img}" "$tmpdir" "$ts" \
                 "$info" "$islossy" "$isoriginal"
    fi

}

###############################################################################
# function rebuild the tile indexes for a ds
###############################################################################

function rebuildtindexs {

    ##### remove the old tindexs #####
    
    for shp in $(find ${outdir} -name "${dsname}[0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9].shp")
    do
        local base="${shp%.*}"
        for ext in shp dbf prj shx
        do
            rm "${base}.${ext}"
        done
    done
    
    ##### make the ne tindexs #####
    
    for img in $(find ${outdir} -iname "*.tif")
    do
        local imgfile="${img##*/}"
        local dir="${img%/*}"
        local ts="${dir: -8}"
        if [[ "$imgfile" != overview* ]]
        then
            
            ##### make the tileindex in a subshell so we can cd with no adverse effect #####
            ##### this costs like 2s of system time per 4000 calls #####

           (
               cd ${outdir}
               gdaltindex "${dsname}${ts}.shp" "${ts}/${imgfile}"  > /dev/null
           )
        fi
    done
}
