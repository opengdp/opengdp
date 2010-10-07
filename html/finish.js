var mapPanel;

Ext.onReady(function() {

  
 
/*******************************************************************************
  map panel
*******************************************************************************/

  map.addControl(new OpenLayers.Control.Permalink());
 
  if (!map.getCenter() || map.getCenter().lon == 0) {
    map.setCenter(new OpenLayers.LonLat(@mapcenter@), @mapzoom@);
  }

  mapPanel = new GeoExt.MapPanel({
    title: "@fullproject@",
    //renderTo: "mappanel",
    border: true,
    region: "center",
    map: map,
    center: [map.getCenter().Lon, map.getCenter().Lat],
    zoom: map.getZoom(),
    extent: map.getExtent(),
    stateId: "map"
  });
  
/*******************************************************************************
  tree panel
*******************************************************************************/
    
  var tree = new Ext.tree.TreePanel(
    {
      border: true,
      region: "west",
      title: "Layers",
      width: 250,
      split: true,
      collapsible: true,
      collapseMode: "mini",
      autoScroll: true,
      root: layerRoot,
      listeners: {
        'checkchange': function(node, checked) {
          if(checked) {
            map.addLayer(node.layer);
          } else {
            map.removeLayer(node.layer, TRUE);
          }
        }
      }

    }
  );

  
/*******************************************************************************
  viewport
*******************************************************************************/

  new Ext.Viewport({
    layout: "fit",
    hideBorders: true,
    items: {
      layout: "border",
      deferredRender: false,
      items: [mapPanel, tree/*, {
        contentEl: "desc",
        region: "east",
        bodyStyle: {"padding": "5px"},
        collapsible: true,
        collapseMode: "mini",
        split: true,
        width: 200,
        title: "Description"
      }*/]
    }
  });
});
