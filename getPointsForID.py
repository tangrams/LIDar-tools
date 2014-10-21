import psycopg2
import sys

DATABASE = 'test2'
BUFFER_POLY = 5 # 0 == disable BUFFER (extend the radius of the search)
FILLING_DIST = 0 # 0 == disable filling 
CENTERED = True

def extractPolygon(osm_id,outfile):
	connection = psycopg2.connect(database = DATABASE)
	cursor = connection.cursor();

	# Default variables
	ground = 100000
	centroid = (0,0)
	ceiling = 0

	# Center the feature
	if(CENTERED):
		cursor.execute('SELECT ST_X(ST_Centroid(way)), ST_Y(ST_Centroid(way)) FROM planet_osm_polygon WHERE osm_id ='+osm_id)
		centroid = cursor.fetchone()

	# Get the OSM Polygon
	cursor.execute('SELECT ST_AsText( way ) FROM planet_osm_polygon WHERE osm_id ='+osm_id)
	polygonSTR = cursor.fetchone()[0][9:-2]
	polygonPts = []
	for point in polygonSTR.split(","):
		polygonPts.append([float(coor) for coor in point.split(" ")])

	cursor.execute('SELECT height FROM planet_osm_polygon WHERE osm_id ='+osm_id)
	ceiling = cursor.fetchone()[0]

	# Extend borders
	if(BUFFER_POLY == 0):
		cursor.execute('SELECT * FROM elevation INNER JOIN planet_osm_polygon AS polys ON ST_Intersects(elevation.the_geom, polys.way ) WHERE polys.osm_id = ' + osm_id)
	else:
		cursor.execute('SELECT * FROM (SELECT * FROM (SELECT * FROM elevation INNER JOIN planet_osm_polygon AS polys ON elevation.the_geom && polys.way WHERE polys.osm_id = '+osm_id+') AS in_bbox WHERE ST_Intersects(the_geom, ST_Buffer(way, '+str(BUFFER_POLY)+'))) AS in_poly')

	# Clean the out put file
	open(outfile, 'w').close()
	
	# 	Write output file
	outFile = open(outfile, 'w')
	for row in cursor:
		if (int(row[2]) < ground):
			ground = int(row[2])
		newLine = '%.2f %.2f %.2f' % (row[0]-centroid[0], row[1]-centroid[1], row[2])
		outFile.write(newLine+'\n')

	if (FILLING_DIST > 0):
		for z in xrange(int(ground),int(ceiling)):
			# print z
			for p in polygonPts:
				newLine = '%.2f %.2f %.2i' % (float(p[0])-float(centroid[0]), float(p[1])-float(centroid[1]), z+10.0)
				# print newLine
				outFile.write(newLine+'\n')

	outFile.close()

if __name__ == '__main__':
	total = len(sys.argv)
	assert total >= 3, 'Usage: python getPointsForID.py osm_id_number outfile.xyz [--center]'

	for arg in xrange(total):
		if( sys.argv[arg]=='--center'):
			CENTERED = True

	osm_id = sys.argv[1]
	outfile = sys.argv[2]
	extractPolygon(osm_id,outfile)