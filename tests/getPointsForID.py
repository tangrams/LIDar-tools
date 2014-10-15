import psycopg2
import sys

DATABASE = 'test2'
BUFFER_POLY = 5

def extractPolygon(osm_id,outfile):
	connection = psycopg2.connect(database = DATABASE)
	cursor = connection.cursor();

	centroid = (0,0)
	cursor.execute('SELECT ST_X(ST_Centroid(way)), ST_Y(ST_Centroid(way)) FROM planet_osm_polygon WHERE osm_id ='+osm_id)
	centroid = cursor.fetchone()

	cursor.execute('SELECT * FROM elevation INNER JOIN planet_osm_polygon AS polys ON ST_Intersects(elevation.the_geom, ST_Buffer(polys.way,5) ) WHERE polys.osm_id = ' + osm_id)
	# cursor.execute('SELECT * FROM (SELECT * FROM (SELECT * FROM elevation INNER JOIN planet_osm_polygon AS polys ON elevation.the_geom && polys.way WHERE polys.osm_id = '+osm_id+') AS in_bbox WHERE ST_Intersects(the_geom, ST_Buffer(way, '+str(BUFFER_POLY)+'))) AS in_poly')

	# Clean the out put file
	open(outfile, 'w').close()

	outFile = open(outfile, 'w')
	for row in cursor:
		newLine = '%.2f %.2f %.2f' % (row[0]-centroid[0], row[1]-centroid[1], row[2])
		outFile.write(newLine+'\n')
	outFile.close()

if __name__ == '__main__':
	assert len(sys.argv) == 3, 'Usage: python getPointsForID.py osm_id_number outfile.xyz'
	osm_id = sys.argv[1]
	outfile = sys.argv[2]
	extractPolygon(osm_id,outfile)