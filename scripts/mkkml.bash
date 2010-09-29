cat > /var/www/html/deephorizon.kml << EOF
<?xml version="1.0" encoding="UTF-8"?>
<kml xmlns="http://www.opengis.net/kml/2.2">
<Document>
EOF

for map in $(find done/ -name "*.map" | grep AERIAL)
do
    layer=$(grep "$map" -e GROUP | cut -d "'" -f 2 | uniq )

    cat >> /var/www/html/deephorizon.kml << EOF
  
  <Folder>
  <name>${layer}</name>
    <visibility>0</visibility>
    <open>1</open>
    <Style>
      <ListStyle>
        <listItemType>radioFolder</listItemType>
        <bgColor>00ffffff</bgColor>
        <maxSnippetLines>2</maxSnippetLines>
      </ListStyle>
    </Style>

    <Folder>
      <name>off</name>
      <visibility>0</visibility>
    </Folder>

    <GroundOverlay>
      <name>on</name>
      <visibility>0</visibility>
      <Icon>
        <href>http://hurakan.ucsd.edu/cgi-bin/deephorizon_wms?SERVICE=WMS&amp;VERSION=1.1.1&amp;REQUEST=GetMap&amp;SRS=EPSG:4326&amp;LAYERS=${layer}&amp;FORMAT=image/png</href>
        <viewRefreshMode>onStop</viewRefreshMode>
        <viewFormat>BBOX=[bboxWest],[bboxSouth],[bboxEast],[bboxNorth]&amp;WIDTH=[horizPixels]&amp;HEIGHT=[vertPixels]</viewFormat>
        <viewBoundScale>1</viewBoundScale>
      </Icon>
      <LatLonBox></LatLonBox>
    </GroundOverlay>
  </Folder>

EOF

done

for map in $(find done/ -name "*.map" | grep -e "LANDSAT\|EO1")
do
    for layer in $(grep "$map" -e NAME | cut -d "'" -f 2 | uniq )
    do

        cat >> /var/www/html/deephorizon.kml << EOF
  
  <Folder>
  <name>${layer}</name>
    <visibility>0</visibility>
    <open>1</open>
    <Style>
      <ListStyle>
        <listItemType>radioFolder</listItemType>
        <bgColor>00ffffff</bgColor>
        <maxSnippetLines>2</maxSnippetLines>
      </ListStyle>
    </Style>

    <Folder>
      <name>off</name>
      <visibility>0</visibility>
    </Folder>

    <GroundOverlay>
      <name>on</name>
      <visibility>0</visibility>
      <Icon>
        <href>http://hurakan.ucsd.edu/cgi-bin/deephorizon_wms?SERVICE=WMS&amp;VERSION=1.1.1&amp;REQUEST=GetMap&amp;SRS=EPSG:4326&amp;LAYERS=${layer}&amp;FORMAT=image/png</href>
        <viewRefreshMode>onStop</viewRefreshMode>
        <viewFormat>BBOX=[bboxWest],[bboxSouth],[bboxEast],[bboxNorth]&amp;WIDTH=[horizPixels]&amp;HEIGHT=[vertPixels]</viewFormat>
        <viewBoundScale>1</viewBoundScale>
      </Icon>
      <LatLonBox></LatLonBox>
    </GroundOverlay>
  </Folder>

EOF
    done
done
cat >> /var/www/html/deephorizon.kml << EOF
</Document>
</kml>
EOF
