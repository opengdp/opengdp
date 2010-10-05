var mapPanel;

Ext.onReady(function() {

  
/*******************************************************************************
  map panel
*******************************************************************************/
 
  mapPanel = new GeoExt.MapPanel({
    title: "Hurakan Weather",
    //renderTo: "mappanel",
    border: true,
    region: "center",
    map: map,
    center: [-100, 35],
    zoom: 4,
    stateId: "map"
  });

  map.addControl(new OpenLayers.Control.Permalink());
  
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
