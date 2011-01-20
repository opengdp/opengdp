


Ext.onReady(function() {

  var base_layers = [];

  osm_google_tc = new OpenLayers.Layer.WMS(
    "Open Street Map",
    "http://cube.telascience.org/tilecache/tilecache.py",
    {
      layers: 'osm-google',
      format: 'image/png',
      transparency: 'TRUE',
    },
    {
      displayInLayerSwitcher: true,
      isBaseLayer: true,
      visibility: false
    }
  );

  base_layers.push(  osm_google_tc );
  
  NewWorld = new OpenLayers.Layer.WMS(
    "NewWorld",
    "http://cube.telascience.org/tilecache/tilecache.py",
    {
      layers: 'NewWorld_google',
      format: 'image/jpeg',
      transparency: 'TRUE',
    },
    {
      displayInLayerSwitcher: true,
      isBaseLayer: true,
      visibility: false
    }
  );
  
  base_layers.push( NewWorld );

  var OnEarth_PAN_321_20030801_tc = new OpenLayers.Layer.WMS(
    "OnEarth_PAN_321",
    "http://cube.telascience.org/tilecache/tilecache.py",
    {
      layers: 'OnEarth_PAN_321_20030801',
      format: 'image/png',
      transparency: 'TRUE',
    },
    {
      displayInLayerSwitcher: true,
      isBaseLayer: true,
      visibility: false
    }
  );

  base_layers.push( OnEarth_PAN_321_20030801_tc );


  NAIP_ALL_tc = new OpenLayers.Layer.WMS(
    "NAIP_ALL",
    "http://cube.telascience.org/tilecache/tilecache.py",
    {
      layers: 'NAIP_ALL',
      format: 'image/png',
      transparency: 'TRUE',
    },
    {
      displayInLayerSwitcher: true,
      isBaseLayer: true,
      visibility: false
    }
  );

  base_layers.push( NAIP_ALL_tc );

  var google_terrain = new OpenLayers.Layer.Google(
    "Google Terrain",
    {
      type: G_PHYSICAL_MAP,
     'sphericalMercator': true
    },
    {
      displayInLayerSwitcher: true,
      isBaseLayer: true,
      visibility: false
    }

  );
  
  base_layers.push( google_terrain );

  var google_streets = new OpenLayers.Layer.Google(
    "Google Streets",
    {
      'sphericalMercator': true
    },
    {
      displayInLayerSwitcher: true,
      isBaseLayer: true,
      visibility: false
    }
  );
  
  base_layers.push( google_streets );
  

  var google_hybrid = new OpenLayers.Layer.Google(
    "Google Hybrid",
    {
      type: G_HYBRID_MAP,
      'sphericalMercator': true
    },
    {
      displayInLayerSwitcher: true,
      isBaseLayer: true,
      visibility: false
    }
  );
  
  base_layers.push( google_hybrid );

  var google_sat = new OpenLayers.Layer.Google(
    "Google Satellite",
    {
      type: G_SATELLITE_MAP,
      'sphericalMercator': true
    },
    {
      displayInLayerSwitcher: true,
      isBaseLayer: true,
      visibility: false
    }
  );

  base_layers.push( google_sat );
  


  map.addLayers(base_layers);

 var BASE_store = new GeoExt.data.LayerStore(
    {
      initDir: 1,
      layers: base_layers
    }
  );


  var BASE_list = new GeoExt.tree.BaseLayerContainer(
    {
      text: "Base Layers",
      layerStore: BASE_store,
      map: map,
      draggable:false,
      expanded: true
    }
  );

  layerRoot.appendChild(BASE_list);


});
