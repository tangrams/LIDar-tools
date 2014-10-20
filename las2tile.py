from liblas import file
from liblas import srs
from liblas import header
from liblas import schema
from math import *
import sys

def metersForTile(x,y,z):
    half_circumference = 20037508.342789244
    metersX = x * half_circumference * 2.0 / pow(2.0, z) - half_circumference
    metersY = (y * half_circumference * 2.0 / pow(2.0, z)) * -1.0 + half_circumference
    return metersX,metersY 
	
def tileEdges(x,y,z):
	x1, y1 = metersForTile(x,y,z)
	x2, y2 = metersForTile(x+1,y+1,z) 

	south, north = min(y1,y2), max(y1,y2)
	west, east = min(x1,x2), max(x1,x2)

	return((south,west,north,east)) # S,W,N,E

def cropTile(inputFile,x,y,z):

	# Get the edges of a tile
	south,west,north,east = tileEdges(x, y, z) 
	scaleZ = 1.0

	# Open the LAS file and reproject to Pseudo-Mercator
	# WGS 84 / Pseudo-Mercator -- Spherical Mercator, Google Maps, OpenStreetMap, Bing, ArcGIS, ESRI
	targetProjection = srs.SRS()
	targetProjection.set_userinput('epsg:3857') # Ending projection ( http://epsg.io/3857 )
	inf = file.File(inputFile, header=None, mode='r', in_srs=None , out_srs=targetProjection)
	inh = inf.header

	# If the height is in US Feets scale them into meters
	if (inh.srs.proj4.find('+units=us-ft')):
		scaleZ = 0.3048006096012192

	# Create the out put file. 
	outh = header.Header()
	outh.dataformat_id = 1
	outh.scale = [0.01,0.01,0.01]
	outh.offset = [0.0,0.0,-0.0]
	outsrs = srs.SRS()
	outsrs.set_userinput('epsg:3857')
	outh.srs = outsrs
	outh.schema = inh.schema

	# Open file
	outf = file.File(str(int(x))+'-'+str(int(y))+'-'+str(int(z))+'.las', mode='w', header=outh)
	
	# filter the outside the bBox (projected)
	for p in inf:
		if west <= p.x <= east and south <= p.y <= north:
			p.z = p.z * scaleZ
			outf.write(p)
			print '%.2f,%.2f,%.2f' % (p.x, p.y, p.z)

	outf.close();

if __name__ == '__main__':
	assert len(sys.argv) == 5, 'Usage: python las2tile.py inFile x y z'
	inputFile = sys.argv[1]
	x, y, z = map(float, sys.argv[2:])
	cropTile(inputFile,x, y, z)