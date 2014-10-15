import psycopg2
import requests
from pyproj import Proj
from pyproj import transform

# Conect To the PostGIS
connection = psycopg2.connect(database = 'test2')
cursor = connection.cursor();


# Get a Vector Tile
tileX, tileY, tileZ = '19298', '24633', '16'
jsonUrl = "http://vector.mapzen.com/osm/all/"+tileZ+"/"+tileX+"/"+tileY+".json"
jsonData = requests.get(jsonUrl)
# jsonData = json.loads(response.read().decode("utf-8"))

madisonPark = jsonData.json()['landuse']['features'][0]['geometry']['coordinates'][0]
p1 = Proj(init='epsg:4326')	# From Lat/Lon
p2 = Proj(init='epsg:3857')	# To Spherical Mercator

points = "ST_Polygon(ST_GeomFromText('LINESTRING("
for point in madisonPark:
	points += str("%.2f %.2f," % transform(p1,p2,point[0],point[1]))

points = points[:-1]
points += ")'), 3857)"
# print points


# cursor.execute('SELECT * FROM elevation limit 5')
cursor.execute('SELECT * FROM elevation where ST_Intersects(the_geom, '+points+' )')

count  = 0
outFile = open('out.txt', 'w')
for row in cursor:
	newLine = '%.2f %.2f %.2f' % (row[0], row[1], row[2])
	outFile.write(newLine+'\n')
	count += 1
	# print row[0], row[1], row[2] 
outFile.close()
print count