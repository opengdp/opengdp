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
        
        read w s e n < <(getextent $(echo "$layer" | sed 's:.*_\([0-9]*\):\1:'))
        read gw gs junk < <(gdaltransform -s_srs EPSG:4326 -t_srs EPSG:900913 <<< "$w $s")
        read ge gn junk < <(gdaltransform -s_srs EPSG:4326 -t_srs EPSG:900913 <<< "$e $n")
        
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
      'myExtent': new OpenLayers.Bounds(${gw}, ${gs}, ${ge}, ${gn})
    }
  );

  ${dsname}_layers.push( ${layer} );

EOF
    done

    cat << EOF
    
  map.addLayers(${dsname}_layers);
  
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
      expanded: false,
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
# function to create a js with the layers for openlayers and a geoext folder for tilecache
###############################################################################

function dogeoext_tc {

(
    cat << EOF

Ext.onReady(function() {

  var ${dsname}_tc_layers = [];

EOF

    for map in $(find $outdir -name "*.map" | sort )
    do
        if [[ "$doovr" == "yes" ]]
        then
            layer=$(grep "$map" -e GROUP | cut -d "'" -f 2 | uniq )
        else
            layer=$(grep "$map" -e NAME  | cut -d "'" -f 2 | uniq )
        fi
        
        read w s e n < <(getextent $(echo "$layer" | sed 's:.*_\([0-9]*\):\1:'))
        read gw gs junk < <(gdaltransform -s_srs EPSG:4326 -t_srs EPSG:900913 <<< "$w $s")
        read ge gn junk < <(gdaltransform -s_srs EPSG:4326 -t_srs EPSG:900913 <<< "$e $n")
        
        cat << EOF

  ${layer}_tc = new OpenLayers.Layer.WMS(
    "${layer}",
    "$urlcgibin_tc",
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

  ${dsname}_tc_layers.push( ${layer}_tc );

EOF
    done

    cat << EOF
  
  map.addLayers(${dsname}_tc_layers);

  ${dsname}_tc_store = new GeoExt.data.LayerStore(
    {
      initDir: 0,
      layers: ${dsname}_tc_layers
    }
  );

  ${dsname}_tc_list = new GeoExt.tree.OverlayLayerContainer(
    {
      text: '${dsname}',
      layerStore: ${dsname}_tc_store,
      leaf: false,
      nodeType: "gx_overlaylayercontainer",
      expanded: false,
      applyLoader: false
    }
  );

  layerRoot.appendChild(${dsname}_tc_list);

});

EOF


) > ${htmlbase}/${dsname}_tc.js

#    ##### make sure the js is loaded by index.html
#    
#    if ! grep "${htmlbase}/index.html" -e "${dsname}_tc.js" > /dev/null
#    then
#        linenum=$(cat "${htmlbase}/index.html" |\
#                   grep -n -e "finish.js" |\
#                   tail -n 1 |\
#                   cut -d ":" -f 1
#                 )
#
#        ed -s "${htmlbase}/index.html" << EOF
#${linenum}-1a
#        <script type="text/javascript" src="${dsname}_tc.js"></script>
#.
#w
#EOF
#    fi

}
