# LIDar Tools

## cropTile

Python script to crop a Tile from LAS LIDar data and project to [Spherical Mercator (epsg:3857)](http://epsg.io/3857)

Use:

	python cropTile.py [lidar-data].las [xCord] [yCord] [zoomLevel]

	python cropTile.py 98522082.las 19298 24633 16


## las2ply.py 

Python script to export .las/.las into .ply files

# Dependences

* [libLAS](http://www.liblas.org/)
* [Proj4](http://trac.osgeo.org/proj/)

# Install On MacOS

```
brew install laszip liblas
pip install liblas pyproj
```
