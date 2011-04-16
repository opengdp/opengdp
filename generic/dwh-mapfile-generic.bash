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

function writemap_nominmax {
    local ts="$1"
    local extent="$2"
    
    cat > "${outdir}/${dsname}${ts}.map" << EOF
        
  LAYER
    NAME '${dsname}_${ts}_nominmax'
    TYPE RASTER
    STATUS ON
    DUMP TRUE
    PROJECTION
     'init=epsg:4326'
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

    PROCESSING "RESAMPLE=AVERAGE"
  END
EOF

}

###############################################################################
# function to write out a map file
###############################################################################

function writemap_withover {
    local ts="$1"
    local extent="$2"
    local scale="$3"
    
    cat > "${outdir}/${dsname}${ts}.map" << EOF
        
  LAYER
    NAME '${dsname}_${ts}_hires'
    TYPE RASTER
    STATUS ON
    DUMP TRUE
    PROJECTION
     'init=epsg:4326'
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
    $offsite
    PROCESSING "RESAMPLE=AVERAGE"
  END

  LAYER
    NAME '${dsname}_${ts}_ovr'
    TYPE RASTER
    STATUS ON
    DUMP TRUE
    PROJECTION
     'init=epsg:4326'
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

    TILEINDEX "${outdir}/overview_${dsname}${ts}.shp"
    GROUP '${dsname}_${ts}'
    MINSCALEDENOM $scale
    $offsite
    PROCESSING "RESAMPLE=AVERAGE"
  END

EOF

    
}

###############################################################################
# function to write out a map file without a overview layer
###############################################################################

function writemap_noover {
    local ts="$1"
    local extent="$2"
    
    cat > "${outdir}/${dsname}${ts}.map" << EOF
        
  LAYER
    NAME '${dsname}_${ts}'
    TYPE RASTER
    STATUS ON
    DUMP TRUE
    PROJECTION
     'init=epsg:4326'
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
    
    TILEINDEX '${outdir}/${dsname}${ts}.shp'
    $offsite
    PROCESSING "RESAMPLE=AVERAGE"
  END

EOF
  
    
}

function writemap_NewWorld {
    local ts="$1"
    local extent="$2"
    
    cat > "${outdir}/NewWorld_${dsname}${ts}.map" << EOF

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
    
    TILEINDEX '${outdir}/${dsname}${ts}.shp'

    GROUP 'NewWorld'
    MAXSCALEDENOM $NewWorld_MAXSCALEDENOM

    PROCESSING "RESAMPLE=AVERAGE"

  END

EOF


    
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
# function to add an include line in the NewWorld mapfile
###############################################################################

function addinclude_NewWorld {
    ts="$1"

    if ! grep "$NewWorld_mapfile" -e "${outdir}/${dsname}${ts}.map" > /dev/null
    then
        echo "  INCLUDE '${outdir}/NewWorld_${dsname}${ts}.map'" >> "$NewWorld_mapfile"
    fi

}

###############################################################################
# function to add the main mapfile if its missing
###############################################################################

function write_main_map {


cat > "$mapfile" << EOF

MAP
  NAME '${project}'
  # Map image size

  SIZE 512 512
  UNITS dd

########  WEST       SOUTH     EAST      NORTH

  EXTENT -180.0  -90.0 180.0  90.0
  
  PROJECTION
    'proj=longlat'
    'ellps=WGS84'
    'datum=WGS84'
    'no_defs'
    ''
  END

  # Background color for the map canvas -- change as desired
#  IMAGECOLOR 0 0 0
  IMAGEQUALITY 95
  IMAGETYPE png24

#  OUTPUTFORMAT
#    NAME png
#    DRIVER "GD/PNG"
#    TRANSPARENT ON
#    MIMETYPE "image/png"
#    IMAGEMODE PC256
#    EXTENSION "png"
#  END  

  OUTPUTFORMAT
    NAME jpeg
    DRIVER 'GD/JPEG'
    MIMETYPE 'image/jpeg'
    IMAGEMODE RGB
    FORMATOPTION  QUALITY=90
    EXTENSION 'jpg'
  END

  OUTPUTFORMAT
    NAME GTiff
    DRIVER "GDAL/GTiff"
    MIMETYPE "image/tiff"
    IMAGEMODE RGBA
    FORMATOPTION  TILED=YES
    EXTENSION "tif"
    TRANSPARENT ON
  END

  OUTPUTFORMAT
    NAME PNG24
    DRIVER "GDAL/PNG"
    MIMETYPE "image/png"
    EXTENSION PNG
    IMAGEMODE RGBA
    TRANSPARENT ON
  END

  # Legend
  LEGEND
      IMAGECOLOR 255 255 255
    STATUS ON
    KEYSIZE 18 12
    LABEL
      TYPE BITMAP
      SIZE MEDIUM
      COLOR 0 0 0
    END
  END

  # Web interface definition. Only the template parameter
  # is required to display a map. See MapServer documentation
  WEB
    # Set IMAGEPATH to the path where MapServer should
    # write its output.
    IMAGEPATH '/tmp/'

    # Set IMAGEURL to the url that points to IMAGEPATH
    # as defined in your web server configuration
    IMAGEURL '/tmp/'

    # WMS server settings
    METADATA
      'wms_title'           '${project}'
      'wms_onlineresource'  '${urlcgibin}?SERVICE=WMS'
      'wms_srs'             'EPSG:900913 EPSG:4326'
    END
#   LOG '/var/www/html/mapserver.log'
    #Scale range at which web interface will operate
    # Template and header/footer settings
    # Only the template parameter is required to display a map. See MapServer documentation
  END



END

EOF

}
