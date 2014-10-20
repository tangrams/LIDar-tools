from liblas import file
from liblas import srs
from liblas import header
from liblas import schema
from math import *
import sys

def toSphericalMercator(inputFile,outputFile):
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
	outf = file.File(outputFile, mode='w', header=outh)
	
	# filter the outside the bBox (projected)
	for p in inf:
		p.z = p.z * scaleZ
		outf.write(p)
	outf.close();

if __name__ == '__main__':
	assert len(sys.argv) == 3, 'Usage: python las2SM.py in.las out.las'
	inputFile = sys.argv[1]
	outputFile = sys.argv[2]
	toSphericalMercator(inputFile,outputFile)