# LIDar Tools

### las2SM.py
Project LAS/LAZ LIDar data to [Spherical Mercator (epsg:3857)](http://epsg.io/3857)

Example:
	
	python las2SM.py data.las SMdata.las

### las2tile.py

Crop a tile from LAS LIDar data and project to [Spherical Mercator (epsg:3857)](http://epsg.io/3857)

Example:

	python las2tile.py data.las 19298 24633 16


### las2ply.py 

Export .las/.las into .ply files

Example:

	python las2ply.py 19298-24633-16.las 19298-24633-16.ply


### getPointsForID.py

Get the 3D points inside a OSM polygon through PostGIS using the [OSM ID](http://www.openstreetmap.org/way/264768896). For these you previously need to [load your LIDar and OSM data to your PostGIS server](recipes/postgisOSM-LAS.md)

Example:

	python getPointsForID.py 264768896 outfile.xyz 

## Dependences

* [libLAS](http://www.liblas.org/)
* [Proj4](http://trac.osgeo.org/proj/)

## Installation

#### On Mac OSX

```
brew install laszip
brew install liblas
```

#### On Linux 

Follow [this tutorial](http://scigeo.org/articles/howto-install-latest-geospatial-software-on-linux.html#liblas).

### Then

```
pip install liblas 
pip install pyproj
pip install PPyGIS
```

# Tutorials 

- [Install Python on MacOs](https://gist.github.com/patriciogonzalezvivo/77da993b14a48753efda)
- [Load LIDar and OSM data to your PostGIS server](https://gist.github.com/patriciogonzalezvivo/229c5cd4001c2ed45ec6)