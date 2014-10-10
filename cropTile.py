from liblas import file
from liblas import srs
from liblas import header
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
	# print 'S/N ', south , north
	# print 'W/E', west, east

	# Open the LAS file and reproject to Pseudo-Mercator
	# WGS 84 / Pseudo-Mercator -- Spherical Mercator, Google Maps, OpenStreetMap, Bing, ArcGIS, ESRI
	targetProjection = srs.SRS()
	targetProjection.set_userinput('epsg:3857') # Ending projection ( http://epsg.io/3857 )
	inf = file.File(inputFile, header=None, mode='r', in_srs=None , out_srs=targetProjection)

	# Create the out put file. 
	outf = file.File(str(int(x))+'-'+str(int(y))+'-'+str(int(z))+'.las', mode="w")

	# filter the outside the bBox (projected)
	for p in inf:
		if west <= p.x <= east and south <= p.y <= north:
			#	TODO: detect if Z is in feets (International/US) and scale it to meters
			#
			p.z = p.z * 0.3048006096012192
			outf.write(p)
			print '%f,%f,%f' % (p.x, p.y, p.z)

	outf.close();

if __name__ == '__main__':
	assert len(sys.argv) == 5, 'Usage: python cropTile.py inFile x y z'
	inputFile = sys.argv[1]
	x, y, z = map(float, sys.argv[2:])
	cropTile(inputFile,x, y, z)
