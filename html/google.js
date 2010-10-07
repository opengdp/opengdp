


Ext.onReady(function() {

  var google_layers = [];
  
  var google_terrain = new OpenLayers.Layer.Google(
    "Google Terrain",
    {
      type: G_PHYSICAL_MAP,
     'sphericalMercator': true
    },
    {
      displayInLayerSwitcher: true
    }

  );

  google_layers.push( google_terrain );
  
  /*var google_streets = new OpenLayers.Layer.Google(
    "Google Streets",
    {
      'sphericalMercator': true
    },
    {
      displayInLayerSwitcher: true
    }
  );
  
  google_layers.push( google_streets );
  
  var google_physical = new OpenLayers.Layer.Google(
    "Google Physical",
    {
      type: G_PHYSICAL_MAP}
    );

  google_layers.push( google_physical );
  
  var google_hybrid = new OpenLayers.Layer.Google(
    "Google Hybrid",
    {
      type: G_HYBRID_MAP,
      'sphericalMercator': true
    },
    {
      displayInLayerSwitcher: true
    }
  );
  
  google_layers.push( google_hybrid );

  var google_sat = new OpenLayers.Layer.Google(
    "Google Satellite",
    {
      type: G_SATELLITE_MAP,
      numZoomLevels: 22
    }
  );

  google_layers.push( google_sat );
  */

  map.addLayers(google_layers);

  /*var google_store = new GeoExt.data.LayerStore(
    {
      initDir: 0,
      layers: google_layers
    }
  );


  var BASE_list = new GeoExt.tree.BaseLayerContainer(
    {
      text: "Base Layers",
      layerStore: google_store,
      map: map,
      draggable:false,
      expanded: true
    }
  );

  layerRoot.appendChild(BASE_list);

*/
});
