# Extract serving one LIDar tile on PostGIS 

This article is based on [Yuriy’s Czoli](https://github.com/YKCzoli) article [‘Processing LiDAR to extract building heights’](https://gist.github.com/YKCzoli/3605e014b8ed09a571e5).

## Installation

### On Mac OSX

- Install Postgres

```brew install postgres```

- Install PostGIS

```brew install postgis```

## Steps

Crop the tile you want to work using ```cropTile.py```

	python cropTile.py data.las 19298 24633 16

Convert the .las file to a more useable format

	las2txt -i 19298-24633-16.las -o 19298-24633-16.txt

Start postgres.

	postgres -D /usr/local/var/postgres  

Create a new database.

	createdb testTile

Enter to our database.

	psql SFdowntown

Follow [Yuriy’s Tutorial](https://gist.github.com/YKCzoli/3605e014b8ed09a571e5) but using [```epsg:3857```](http://epsg.io/3857) projection.

_Activate the postgis extenstion (for spatial objects and functionality)._

	CREATE EXTENSION postgis;

_Create a table to hold our LIDar dataset. This creates the infrastructure to hold our x,y,z data. We are naming the table ‘elevation’ because that is primarily what we are interested with in regards to the lidar data._

	CREATE TABLE elevation (x double precision, y double precision, z double precision);

_Now we will add a geometry column. PostGIS documentation suggests to [add a geometry column](http://postgis.refractions.net/docs/AddGeometryColumn.html) after table creation. If you create the geometry column in the initial table set up, you will have to manually register the geometry in the database. Lets avoid this and follow the documentation. ‘the_geom’ refers to the name of the column, and 32610 is the SRID. This specific SRID is UTM zone 10N. UTM zones are great for dealing with a small area and measurements, which we will be doing. Check out this [esri blog](http://blogs.esri.com/esri/arcgis/2010/03/05/measuring-distances-and-areas-when-your-map-uses-the-mercator-projection/) for a short summary._

	SELECT AddGeometryColumn ('elevation','the_geom',3857,'POINT',2);

_Copy the data from our text file to our database. NOTE: if this does not work, re-type the single quotes inside of the terminal._

	copy elevation(x,y,z) FROM ``~/your/path/19298-24633-16.txt’ DELIMITERS ',';

_Lets update the elevation table with the ‘geometry from text’ function. This will parse through the table we just copied and pull the coordinates from the x y values. This will locate the points on the planar dimension of our data. This is using the table we just copied in from the previous step. Again using the utm 10n SRID. This will take a couple of minutes._

	UPDATE elevation SET the_geom = ST_GeomFromText('POINT(' || x || ' ' || y || ')',3857);

_Now that we have our spatial data loaded, we definitely want to create a spatial index. Spatial indices create general envelopes around our geometries, and will allow for quicker query and function abilities later on. Read more about them [here](http://revenant.ca/www/postgis/workshop/indexing.html)._

	CREATE INDEX elev_gix ON elevation USING GIST (the_geom);

_We will switch our focus over to our shapefile for a second. Let’s exit our database._

	\q
	 