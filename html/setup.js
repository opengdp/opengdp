layers = [];

layerRoot = {};
map = {};

Ext.onReady(function() {

  map = new OpenLayers.Map(
    {
      'units' : "m",
      'maxResolution' : 156543.0339,
      'numZoomLevels' : 22,
      'projection' : new OpenLayers.Projection("EPSG:900913"),
      'displayProjection' : new OpenLayers.Projection("EPSG:4326"),
      'maxExtent' : new OpenLayers.Bounds(-20037508.34,-20037508.34, 20037508.34,20037508.34),
      'controls': [new OpenLayers.Control.Navigation(),
                   new OpenLayers.Control.PanZoomBar(),
                   new OpenLayers.Control.Attribution()],
      'theme': 'http://openlayers.org/dev/theme/default/style.css',
      allOverlays: false
    }
  );


  layerRoot = new Ext.tree.TreeNode({
    text: "Layers",
    expanded: true
  });

});

