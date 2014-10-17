# Loading OSM and LIDar to PostGIS 

This article is partialy based on [Yuriy’s Czoli](https://github.com/YKCzoli) article [‘Processing LiDAR to extract building heights’](https://gist.github.com/YKCzoli/3605e014b8ed09a571e5).

## Install PostGIS

### On Mac OSX

- Install Postgres, PostGIS and [OSM-PostGIS](http://wiki.openstreetmap.org/wiki/Osm2pgsql#Binary_Installer) tools 

```
brew install postgres postgis 
brew install protobuf-c
brew install osm2pgsql --with-protobuf-c
```

## Steps

Start postgres.

	postgres -D /usr/local/var/postgres  

Create a new database.

	createdb testDataBase

Enter to our database.

	psql testDataBase

Inside type
	CREATE extension hstore;
	\q

Download the [OSM Metro Extracts data in `OSM XML` format](https://mapzen.com/metro-extracts) and the style file [```osm2pgsql.sty```](https://github.com/mapzen/vector-datasource/blob/master/osm2pgsql.style) 

Then on the same that of your downloaded files type:

	osm2pgsql -s -E 3857 -d testDataBase --hstore -S osm2pgsql.style.txt data-file.osm.bz2   

If you want o upload also a LAS file, first translate the LIDar data to [```epsg:3857```](http://epsg.io/3857) projection using [```las2SM.py```](https://github.com/tangrams/LIDar-tools/blob/master/las2SM.py)

	las2SM.py data.las SMdata.las

And port it to XYZ format

	las2txt -i SMdata.las —parse xyz —delimiter “ “ -o SMdata.xyz
	
Enter to our database.

	psql testDataBase 

Activate the postgis extenstion (for spatial objects and functionality).

	CREATE EXTENSION postgis;

Create a table to hold our LIDar dataset. This creates the infrastructure to hold our x,y,z data. We are naming the table ‘elevation’ because that is primarily what we are interested with in regards to the lidar data.

	CREATE TABLE elevation (x double precision, y double precision, z double precision);

Now we will add a geometry column. PostGIS documentation suggests to [add a geometry column](http://postgis.refractions.net/docs/AddGeometryColumn.html) after table creation. If you create the geometry column in the initial table set up, you will have to manually register the geometry in the database. Lets avoid this and follow the documentation. ‘the_geom’ refers to the name of the column, and 32610 is the SRID. This specific SRID is UTM zone 10N. UTM zones are great for dealing with a small area and measurements, which we will be doing. Check out this [esri blog](http://blogs.esri.com/esri/arcgis/2010/03/05/measuring-distances-and-areas-when-your-map-uses-the-mercator-projection/) for a short summary.

	SELECT AddGeometryColumn ('elevation','the_geom',3857,'POINT',2);

Copy the data from our text file to our database. NOTE: if this does not work, re-type the single quotes inside of the terminal.

	copy elevation(x,y,z) FROM ``~/your/path/SMdata.xyz’ DELIMITERS ' ';

Lets update the elevation table with the ‘geometry from text’ function. This will parse through the table we just copied and pull the coordinates from the x y values. This will locate the points on the planar dimension of our data. This is using the table we just copied in from the previous step. Again using the utm 10n SRID. This will take a couple of minutes.

	UPDATE elevation SET the_geom = ST_GeomFromText('POINT(' || x || ' ' || y || ')',3857);

Now that we have our spatial data loaded, we definitely want to create a spatial index. Spatial indices create general envelopes around our geometries, and will allow for quicker query and function abilities later on. Read more about them [here](http://revenant.ca/www/postgis/workshop/indexing.html).

	CREATE INDEX elev_gix ON elevation USING GIST (the_geom);

We will switch our focus over to our shapefile for a second. Let’s exit our database.

	\q
	 