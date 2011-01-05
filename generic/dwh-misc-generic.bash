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
# to remove dupe /'s from a path
###############################################################################

function fixpath {

    sed -r 's|[/]+|/|g' <<<"$1"
}

###############################################################################
# function to get the extent of the ds
###############################################################################

function getextent {
    ts="$1"

    ogrinfo -so -al "${outdir}/${dsname}${ts}.shp" |\
     grep Extent: |\
     sed -e 's/) - (/ /' -e 's/Extent: (//' -e 's/,//g' -e 's/)//'
}

###############################################################################
# min a b
###############################################################################

function min {

    if [[ $(bc <<< "scale = 16; $1-$2") = -* ]]
    then
        echo "$1"
    else
        echo "$2";
    fi

}

###############################################################################
# max a b
###############################################################################

function max {

    if [[ $(bc <<< "scale = 16; $1-$2") = -* ]]
    then
        echo "$2"
    else
        echo "$1";
    fi

}

###############################################################################
# function to get a ts from a lftp command
###############################################################################

function dodate {
    sed 's:.*/AE00N[0-9]\{2\}_[a-zA-Z0-9]\{10\}_[0-9]\{6\}\([0-9]\{8\}\).*:\1:'
}

###############################################################################
# function to get the a list of new files
###############################################################################

function getlist {
    local mirrorfile="$1"
    local patern="$2"

    ##### not as multiple fetch #####
    
    if [ "${#multifetchpattern[@]}" == "0" ]
    then
        lftp "$baseurl" -e "mirror --script=${mirrorfile} -I "$patern"; exit"

    ##### a multiple fetch #####

    else
        
        ##### go though the paterns #####

        patern=""
        local nump=0
        for p in "${multifetchpattern[@]}"
        do
            ((nump++))
            
            ##### create a group of -I switches for lftps mirror #####

            patern="$patern -I $p"
        done
        
        ##### keep track of the last patern so we can make sure #####
        ##### its the last file fetched. this file will be the  #####
        ##### one gdal operattes on. we need the jpeg not the   #####
        ##### world or aux                                      #####

        local lastp="$p"
        echo lastp = "$lastp"

        ##### get the list of files to fetch #####

        lftp "$baseurl" -e "mirror --script=${mirrorfile} $patern; exit"
        
        ##### put all the mkdir commands at the top of the file #####

        grep "${mirrorfile}" -e "^mkdir" > "${mirrorfile}.new"
        
        ##### parse the get commands to make each group a single line #####
        
        local done=no
        while true
        do
            local lastline=""
            
            ##### loop number of patern times and read #####

            for ((i = 0 ; i < nump ; i++))
            do
                if ! read line 
                then
                    done=yes
                    break
                fi

                if ! [[ "$line" == $lastp ]]
                then
                    echo -n "${line};"
                else
                    lastline="$line"
                fi
            done

            if [ -n "$lastline" ]
            then
                echo "${lastline}"
            fi
        
            if [[ "$done" == "yes" ]]
            then
                break
            fi

        done < <( grep "${mirrorfile}" -e "^get" ) >> "${mirrorfile}.new"

        mv "${mirrorfile}" "${mirrorfile}.old"
        mv "${mirrorfile}.new" "${mirrorfile}"
    fi

}


###############################################################################
# function to get the a list of new files
###############################################################################


function float_cmp {

    local cond=0
    if [[ $# -gt 0 ]]
    then
        cond=$(echo "$*" | bc -q 2>/dev/null)
        if [[ -z "$cond" ]]
        then
            cond=0
        fi
        if [[ "$cond" != 0  &&  "$cond" != 1 ]]
        then
            cond=0
        fi
    fi
    local res=$((cond == 0))
    return $res
}


function dotc_old {

(
    for map in $(find $outdir -name "*.map" | sort )
    do
        if [[ "$doovr" == "yes" ]]
        then
            layer=$(grep "$map" -e GROUP | cut -d "'" -f 2 | uniq )
        else
            layer=$(grep "$map" -e NAME  | cut -d "'" -f 2 | uniq )
        fi
        
        cat << EOF

[${layer}]
type=WMS
url=${urlcgibin}?
layers=${layer}
extension=png
tms_type=google
levels=24
spherical_mercator=true

EOF

    done
) > ${basedir}/${dsname}.tilecache.conf
}

