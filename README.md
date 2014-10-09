# LIDar Tools

## cropTile

Python script to crop a Tile from LAS LIDar data and project to [WGS 84 / Pseudo-Mercator -- Spherical Mercator, Google Maps, OpenStreetMap, Bing, ArcGIS, ESRI](http://epsg.io/3857)

Use:

	python cropTile.py [lidar-data].las [xCord] [yCord] [zoomLevel]

	python cropTile.py 98522082.las 19298 24633 16

### Dependences

* [libLAS](http://www.liblas.org/)
* [Proj4](http://trac.osgeo.org/proj/)

#### On MacOS

```
brew install liblas 
pip install liblas pyproj 
```