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
# function to process a tar file
###############################################################################

function dotar {
    local zipfile="$1"
    local tmpdir="$2"
    local ts="$3"
    local origdir="$4"
    local f
    
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

        tar -xf "${origdir}/${zipfile}" -C "$tmpdir" "$f" > /dev/null 2> /dev/null
        
        ##### try to unzip a world file if its there #####
        
        tar -xf "${origdir}/${zipfile}" -C "$tmpdir" "${imgdir}${imgbase}*.??w" > /dev/null 2> /dev/null
        
        ##### try to unzip a aux file if its there #####
        
        tar -xf "${origdir}/${zipfile}" -C "$tmpdir" "${imgdir}${imgbase}*.aux" > /dev/null 2> /dev/null
        
        doimg "$f" "$tmpdir" "$ts" "$(gdalinfo "${tmpdir}/${f}")" "no"
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
    local f

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

        
        tar -xzf "${origdir}/${zipfile}" -C "$tmpdir" "$f" > /dev/null 2> /dev/null
        
        ##### try to unzip a world file if its there #####
        
        tar -xzf "${origdir}/${zipfile}" -C "$tmpdir" "${imgdir}${imgbase}*.??w" > /dev/null 2> /dev/null
        
        ##### try to unzip a aux file if its there #####
        
        tar -xzf "${origdir}/${zipfile}" -C "$tmpdir" "${imgdir}${imgbase}*.aux" > /dev/null 2> /dev/null
        
        doimg "$f" "$tmpdir" "$ts" "$(gdalinfo "${tmpdir}/${f}")" "no"
        
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
    local f

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

        tar -xjf "${origdir}/${zipfile}" -C "$tmpdir" "$f" > /dev/null 2> /dev/null
        
        ##### try to unzip a world file if its there #####
        
        tar -xjf "${origdir}/${zipfile}" -C "$tmpdir" "${imgdir}${imgbase}*.??w" > /dev/null 2> /dev/null
        
        ##### try to unzip a aux file if its there #####
        
        tar -xjf "${origdir}/${zipfile}" -C "$tmpdir" "${imgdir}${imgbase}*.aux" > /dev/null 2> /dev/null
        
        doimg "$f" "$tmpdir" "$ts" "$(gdalinfo "${tmpdir}/${f}")" "no"
        
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
    local f
    
    
    for f in $(zipinfo -1 "${origdir}/${zipfile}" "$extglob")
    do
        if ! grep '\\' <<< "$f" > /dev/null
        then
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
    
            unzip "${origdir}/${zipfile}" "$f" -d "$tmpdir" #> /dev/null 2> /dev/null
            
            ##### try to unzip a world file if its there #####
            
            unzip "${origdir}/${zipfile}" "${imgdir}${imgbase}*.??w" -d "$tmpdir" > /dev/null 2> /dev/null
            
            ##### try to unzip a aux file if its there #####
            
            unzip "${origdir}/${zipfile}" "${imgdir}${imgbase}*.aux" "${imgdir}${imgbase}*.aux.xml" -d "$tmpdir" > /dev/null 2> /dev/null

        else
            local imgfile="${f##*\\}"
            local imgbase="${imgfile%.*}"
            local imgext="${imgfile##*.}"

            local imgdir="${f%\\*}"
            if [[ "$imgdir" = "$imgfile" ]]
            then
                local imgdir=""
            else
                local imgdir="${imgdir}\\"
            fi

            unzip "${origdir}/${zipfile}" "*${imgfile}" -d "$tmpdir" #> /dev/null 2> /dev/null
            
            ##### try to unzip a world file if its there #####
            
            unzip "${origdir}/${zipfile}" "*${imgbase}*.??w" -d "$tmpdir" > /dev/null 2> /dev/null
            
            ##### try to unzip a aux file if its there #####
            
            unzip "${origdir}/${zipfile}" "*${imgbase}*.aux" "*${imgbase}*.aux.xml" -d "$tmpdir" > /dev/null 2> /dev/null

            f="${f/\\//}"

        fi

        
        doimg "$f" "$tmpdir" "$ts" "$(gdalinfo "${tmpdir}/${f}")" "no"

        
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
    local f
    
    local zipbase="${zipfile##*/}"
    local zipbase="${zipbase%.*}"
    
    for f in $(zipinfo -1 "${origdir}/${zipfile}" "$baseglob.kml")
    do

        ##### extract the kml #####
        
        unzip "${origdir}/${zipfile}" "$f" -d "$tmpdir"
        
        ##### find and extract the corisponding img #####
        
        local img=$(grep '<GroundOverlay>' -A12 "${tmpdir}/$f" |\
                    grep href | sed -r 's|.*<href>(.*)</href>.*|\1|')
        
        unzip "${origdir}/${zipfile}" "$img" -d "$tmpdir"
        
        local imgfile="${img##*/}"
        local imgbase="${imgfile%.*}"
        local imgext="${imgfile##*.}"
        
        local imgdir="${img%/*}"
        if [[ "$imgdir" = "$imgfile" ]]
        then
            local imgdir=""
        else
            local imgdir="${imgdir}/"
        fi

        ##### get the corner quords #####
        
        read n s e w < <(grep '<GroundOverlay>' -A12 "${tmpdir}/$f" |\
                          grep north -A3 |\
                          sed 's:<[/a-z]*>::g' |\
                          tr "\n" " ")
        
        ##### create a vrt with the proj #####
        
        gdal_translate -a_srs EPSG:4326 \
                       -a_ullr $w $n $e $s \
                       -of VRT -mask none \
                       "${tmpdir}/${img}" \
                       "${tmpdir}/${imgdir}/${zipbase}_${imgbase}.vrt"
                       
        ##### proccess #####
        
        doimg "${imgdir}/${zipbase}_${imgbase}.vrt" "$tmpdir" "$ts" \
              "$(gdalinfo "${tmpdir}/${imgdir}/${zipbase}_${imgbase}.vrt")" \
              "no"
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
        local ext="$(tr [A-Z] [a-z] <<< "$ext")"
        #local ext="${ext,,*}"
        
        if echo "$myline" | grep -e "$sourcedir" > /dev/null
        then
            local dir="$(echo "$myline" | sed "s|.*$sourcedir\(.*\) $url.*|\1|")/"
        else
            local dir=""
        fi
         
        local ts=$(${datefunc} <<< "$myline")
        
        if [ -n "$DEBUG_dofile" ]
        then
            printf " myline=%s\n sourcedir=%s\n file=%s\n base=%s\n ext=%s\n dir=%s\n ts=%s\n" \
                    "$myline" \
                    "$sourcedir "\
                    "$file" \
                    "$base" \
                    "$ext" \
                    "$dir" \
                    "$ts"
            echo >&3
            return
        fi

        local tmpdir=$(mktemp -d -p "$tmp" "${dsname}XXXXXXXXXX")
        
        
        if [[ "$DWH_REBUILD" == "rebuild" ]]
        then
            local origdir="${indir/%\//}.old/${dir}"
        else
            lftp -e "$(echo "$myline" | sed "s:get \([-]. \)\{1,\}[-/_.A-Za-z0-9]*:get \1${tmpdir}:g") ; exit" > /dev/null 2> /dev/null
            local origdir="$tmpdir"
        fi
        

        if ! [ -d "$outdir/${ts}" ]
        then
            mkdir -p "$outdir/${ts}"
        fi
        
        case "$ext" in
        
            *tar)
                dotar "${file}" "$tmpdir" "$ts" "$origdir"
                ;;
                
            *tar.gz)
                dotargz "${file}" "$tmpdir" "$ts" "$origdir"
                ;;
            
            *tgz)
                dotargz "${file}" "$tmpdir" "$ts" "$origdir"
                ;;
            
            *tar.bz2)
                dotarbz2 "${file}" "$tmpdir" "$ts" "$origdir"
                ;;
            
            *zip)
                dozip "${file}" "$tmpdir" "$ts" "$origdir"
                ;;
            
            *kmz)
                dokmz "${file}" "$tmpdir" "$ts" "$origdir"
                ;;

#fixme this does not take into account world files that may be there too
            
            *)
                if gdalinfo "${origdir}/${file}" > /dev/null
                then
                    

                    ##### hack to make rebuild work here #####
                    
                    if [[ "$DWH_REBUILD" == "rebuild" ]]
                    then
                        ln -s "${origdir}/${file}" "${tmpdir}/${file}"
                    fi

                    doimg "${file}" \
                          "$tmpdir" \
                          "$ts" \
                          "$(gdalinfo ${origdir}/$file)" \
                          "yes"
                fi

            esac
        
        if [[ "$DWH_REBUILD" != "rebuild" ]]
        then
            mv "${tmpdir}/${file}" "${indir}/${dir}/${file}"
        fi

        rm -rf "${tmpdir}"
    
    fi    

    echo >&3
    
}

