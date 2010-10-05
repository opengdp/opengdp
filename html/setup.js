layers = [];

layerRoot = {};
map = {};

Ext.onReady(function() {

  map = new OpenLayers.Map(
    {
       allOverlays: false
    }); 

  layerRoot = new Ext.tree.TreeNode({
    text: "Layers",
    expanded: true
  });

});

