# LIDar Tools

### cropTile

Python script to crop a Tile from LAS LIDar data and project to [Spherical Mercator (epsg:3857)](http://epsg.io/3857)

Example:

	python cropTile.py data.las 19298 24633 16


### las2ply.py 

Python script to export .las/.las into .ply files

Example:

	python las2ply.py 19298-24633-16.las 19298-24633-16.ply


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

Follow [this tutorial](http://scigeo.org/articles/howto-install-latest-geospatial-software-on-linux.html#liblas)

### Then

```
pip install liblas 
pip install pyproj
pip install requests
```

## Others useful libraries

- [PCL](http://www.pointclouds.org/news/2013/02/07/python-bindings-for-the-point-cloud-library/)

- [CGAL](http://cgal-python.gforge.inria.fr/)

- [PDAL](http://www.pdal.io/)

# Tutorials 

- [Install Python on MacOs](https://gist.github.com/patriciogonzalezvivo/77da993b14a48753efda)
- [Extract serving one LIDar tile on PostGIS](recipes/postgisTile.md)