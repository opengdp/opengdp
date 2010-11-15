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
# function to add internal overviews to a file down to  64x64
###############################################################################

function addo {
    local file="$1"
    local x
    local y
    local o
    local ovrs
    
    read x y < <( gdalinfo "${file}" |\
                           grep -e "Size is" |\
                           sed 's/Size is \([0-9]*\), \([0-9]*\)/\1 \2/')
    
    ##### add overviews #####
    
    ((o=2))
    ovrs=""
    while [[ 64 -lt $(bc <<<"$x / $o") ]] && [[ 64 -lt $(bc <<<"$y / $o") ]]
    do
        ovrs="$ovrs $o"
        ((o*=2))
    done
    
    gdaladdo -clean -r average "$file" $ovrs > /dev/null
    
}
###############################################################################
# function to add internal overviews to a file down to  64x64
###############################################################################

function readdo2 {
    local file="$1"
    local x
    local y
    local o
    local ovrs

    read x y < <( gdalinfo "${file}" |\
                           grep -e "Size is" |\
                           sed 's/Size is \([0-9]*\), \([0-9]*\)/\1 \2/')
    
    ##### add overviews #####
    
    ((o=2))
    ovrs=""
    while [[ 64 -lt $(bc <<<"$x / $o") ]] && [[ 64 -lt $(bc <<<"$y / $o") ]]
    do
        ovrs="$ovrs $o"
        ((o*=2))
    done
    
    gdaladdo -clean -r average "$file" $ovrs > /dev/null
    
    echo >&3
}

###############################################################################
# function to rebuild internal overviews
###############################################################################

function readdo {
    
    local tmpdir=$(mktemp -d -p "$tmp" "${dsname}XXXXXXXXXX")
            
    find "${outdir}" -name "*.tif" > "${tmpdir}/tiflist.txt"
 
    mainloop "${tmpdir}/tiflist.txt" readdo2
    
    rm -rf "${tmpdir}"
    
}





