var mapPanel;

Ext.onReady(function() {


/*******************************************************************************
  transparency slider
*******************************************************************************/

  openTransparencySlider = function(layer) {
    var title = 'Transparency - ' + layer.name;
    var transparency_window = new Ext.Window({
      title:title,
      layout:'fit',
      width:300,
      height:100,
      items:[new GeoExt.LayerOpacitySlider({
        layer: layer,
        aggressive: true, 
        width: 200,
        fieldLabel: 'opacity',
        plugins: new GeoExt.LayerOpacitySliderTip()
      })],
      buttons: [{
        text: 'Close',
        handler: function(){
          transparency_window.close();
        }
      }]
    });     
    transparency_window.show();
  };

/*******************************************************************************
   vector drawing layers
*******************************************************************************/

    var vector = new OpenLayers.Layer.Vector("vector");
    map.addLayer(vector);


/*******************************************************************************
   toolbar
*******************************************************************************/

  var maptbar = [
    "->",
    new GeoExt.Action({
      control: new OpenLayers.Control.Navigation(),
      map: map,
      toggleGroup: "edit",
      pressed: true,
      allowDepress: false,
      text: "Navigate"
    }),
    new GeoExt.Action({
        text: "Draw Poly",
        control: new OpenLayers.Control.DrawFeature(
            vector, OpenLayers.Handler.Polygon
        ),
        map: map,
        // button options
        toggleGroup: "edit",
        allowDepress: false,
        tooltip: "Draw Polygon",
        // check item options
        group: "draw"
    }),
    new GeoExt.Action({
        text: "Draw Line",
        control: new OpenLayers.Control.DrawFeature(
            vector, OpenLayers.Handler.Path
        ),
        map: map,
        // button options
        toggleGroup: "edit",
        allowDepress: false,
        tooltip: "Draw Linestring",
        // check item options
        group: "draw"
    }),
    new GeoExt.Action({
        text: "Draw Point",
        control: new OpenLayers.Control.DrawFeature(
            vector, OpenLayers.Handler.Point
        ),
        map: map,
        // button options
        toggleGroup: "edit",
        allowDepress: false,
        tooltip: "Draw Point",
        // check item options
        group: "draw"
    })
 ];


 
/*******************************************************************************
  map panel
*******************************************************************************/

  map.addControl(new OpenLayers.Control.Permalink());
 

  if (!map.getCenter() || map.getCenter().lon == 0) {
    var proj = new OpenLayers.Projection("EPSG:4326");
    map.setCenter(center.transform(proj, map.getProjectionObject()), zoom);
  }

  mapPanel = new GeoExt.MapPanel({
    title: "title",
    //renderTo: "mappanel",
    border: true,
    region: "center",
    map: map,
    center: [map.getCenter().Lon, map.getCenter().Lat],
    zoom: map.getZoom(),
    extent: map.getExtent(),
    stateId: "map",
    tbar: maptbar
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
        
        /***** checkchange *****/
        
        'checkchange': function(node, checked) {
          if (node.layer.isBaseLayer === false) {
            
            /***** dont add or remove from the map if perm is true *****/

            if (node.layer.perm === false) {
              if(checked) {
                map.addLayer(node.layer);
              } else {
                map.removeLayer(node.layer, 'TRUE');
              }
            }

          /***** set the zoom on the map when baselayer is changed *****/

          } else {
            node.layer.onMapResize();
            var center = map.getCenter();
    
            if (map.baseLayer != null && center != null) {
              var zoom = map.getZoom();
              map.zoom = null;
              map.setCenter(center, zoom);
            }
          }
        },
        
        /***** rightclick menu linstner *****/
        
        contextmenu: function(node, e) {
          if (node && node.layer) {
            node.select();
            if (node.layer.isBaseLayer) {
              var c = node.getOwnerTree().baseLayerContextMenu;
              c.showAt(e.getXY());
            } else if (node.layer instanceof OpenLayers.Layer.Vector) {
              //var c = node.getOwnerTree().vectorOverlayContextMenu;
              //c.showAt(e.getXY());
              1+1;
            } else {
              var c = node.getOwnerTree().rasterOverlayContextMenu;
              c.showAt(e.getXY());
            }
          }
        },
        scope: this
      },
      
      /***** rightclick menu *****/
      
      baseLayerContextMenu: new Ext.menu.Menu({
        items: [{
          text: "Adjust Transparency",
          iconCls: 'default-icon-menu',
          handler: function() {
            var node = tree.getSelectionModel().getSelectedNode();
            if(node && node.layer) {
              openTransparencySlider(node.layer);
            }
          },
          scope: this
        }]
      }),

      rasterOverlayContextMenu: new Ext.menu.Menu({
        items: [{
          text: "Adjust Transparency",
          iconCls: 'default-icon-menu',
          handler: function() {
            var node = tree.getSelectionModel().getSelectedNode();
            if(node && node.layer) {
              openTransparencySlider(node.layer);
            }
          },
        scope: this
        },
        {
          text: "Get Layer URL",
          iconCls: 'default-icon-menu',
          handler: function() {
            var node = tree.getSelectionModel().getSelectedNode();
            if(node && node.layer && node.layer.getFullRequestString) {
              Ext.MessageBox.alert(node.layer.name + ' - ' + node.layer.CLASS_NAME,
              node.layer.getFullRequestString());
            }
          },
          scope: this
        },
        {
          text: "Zoom to Layer Extent",
	      iconCls: "icon-zoom-visible",
          handler: function() {
	        var node = tree.getSelectionModel().getSelectedNode();
            if(node && node.layer) {
	          this.map.zoomToExtent(node.layer.myExtent);//restrictedExtent);
	        }
	      },
	      scope: this
        }
        ]
        
        
      })

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
