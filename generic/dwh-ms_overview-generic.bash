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
# functiom to create a overview from the dataset
###############################################################################

function getoverviewscale {

(
cd "$outdir"
    local ts="$1"
    
    local blocksize=256

    local tmpdir=$(mktemp -d -p "$tmp" "${dsname}XXXXXXXXXX")

    local firstfile=$(ogrinfo -al "${outdir}/${dsname}${ts}.shp" |\
                       grep "  location (String) = " |\
                       head -n 1 |\
                       sed 's/  location (String) = //')
    
    GDAL_DISABLE_READDIR_ON_OPEN=TRUE gdalbuildvrt -overwrite \
                 "${tmpdir}/${dsname}${ts}.vrt" \
                 "${outdir}/${firstfile}" > /dev/null

    read xr yr < <(gdalinfo "${tmpdir}/${dsname}${ts}.vrt" |\
                    grep -e "Pixel Size" |\
                    sed 's/.*Pixel Size = ([-]*\([.0-9]*\),[-]*\([.0-9]*\)).*/\1 \2/')

    local orig=$(bc <<< "scale = 16; $(min $xr $yr) * 72 * 4374754")
    local scale=$(bc <<< "$orig + 150000")

    rm -rf ${tmpdir} > /dev/null

    echo ${scale%.*}
)
}

###############################################################################
# functiom to create a overview from the dataset
###############################################################################

function makeoverview {

(
cd "$outdir"
    local ts="$1"
    local extent="$2"
    
    local blocksize=256

    local tmpdir=$(mktemp -d -p "$tmp" "${dsname}XXXXXXXXXX")

    local firstfile=$(ogrinfo -al "${outdir}/${dsname}${ts}.shp" |\
                       grep "  location (String) = " |\
                       head -n 1 |\
                       sed 's/  location (String) = //')
    
    GDAL_DISABLE_READDIR_ON_OPEN=TRUE gdalbuildvrt -overwrite \
                 "${tmpdir}/${dsname}${ts}.vrt" \
                 "${outdir}/${firstfile}" > /dev/null

    read xr yr < <(gdalinfo "${tmpdir}/${dsname}${ts}.vrt" |\
                    grep -e "Pixel Size" |\
                    sed 's/.*Pixel Size = ([-]*\([.0-9]*\),[-]*\([.0-9]*\)).*/\1 \2/')

    local orig=$(bc <<< "scale = 16; $(min $xr $yr) * 72 * 4374754")
    local scale=$(bc <<< "$orig + 150000")

    ##### res = scale / (72 * 4374754) #####
    
    res=$(bc <<< "scale = 16; $scale / (72 * 4374754)")

    read ux ly lx uy <<< "$extent"

    x=$(bc <<< "scale=0; ($lx - $ux) / $res")
    y=$(bc <<< "scale=0; ($uy - $ly) / $res")

    
    cat > "${tmpdir}/${dsname}${ts}.xml" << EOF
<GDAL_WMS>
    <Service name="WMS">
        <ServerURL>${urlcgibin}?</ServerUrl>
        <ImageFormat>image/tiff</ImageFormat>
        <Transparent>TRUE</Transparent>
        <Layers>${dsname}_${ts}</Layers>
    </Service>
    
    <DataWindow>
        <UpperLeftX>${ux}</UpperLeftX>
        <UpperLeftY>${uy}</UpperLeftY>
        <LowerRightX>${lx}</LowerRightX>
        <LowerRightY>${ly}</LowerRightY>
        <SizeX>${x}</SizeX>
        <SizeY>${y}</SizeY>
    </DataWindow>
    <Projection>EPSG:4326</Projection>
    <BandsCount>4</BandsCount>
    <BlockSizeX>${blocksize}</BlockSizeX>
    <BlockSizeY>${blocksize}</BlockSizeY>
    <OverviewCount>0</OverviewCount>
    <MaxConnections>$((${limit} * 2))</MaxConnections>
    <Timeout>300</Timeout>
</GDAL_WMS>

EOF

    ##### translate #####

    gdal_translate -co TILED=YES -co BLOCKXSIZE=${blocksize} -co BLOCKYSIZE=${blocksize} \
                  -b 1 -b 2 -b 3 -b 4 \
                  "${tmpdir}/${dsname}${ts}.xml" \
                  "${tmpdir}/overview.tif" > /dev/null
    
    isoverview=true

    doimg "overview.tif" \
          "$tmpdir" \
          "$ts" \
          "$(gdalinfo "${tmpdir}/overview.tif")" \
          "yes"

    rm -rf ${tmpdir} > /dev/null

    unset isoverview

    echo ${scale%.*}
)

}

