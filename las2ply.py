from liblas import file
from liblas import srs
import sys

def line_prepend(filename,line):
    with open(filename,'r+') as f:
        content = f.read()
        f.seek(0,0)
        f.write(line.rstrip('\r\n') + '\n' + content)

def las2ply(inputFile,outputFile):

	# Open the LAS file and reproject to Pseudo-Mercator
	# WGS 84 / Pseudo-Mercator -- Spherical Mercator, Google Maps, OpenStreetMap, Bing, ArcGIS, ESRI
	targetProjection = srs.SRS()
	targetProjection.set_userinput('epsg:3857') # Ending projection ( http://epsg.io/3857 )
	inFile = file.File(inputFile, header=None, mode='r', in_srs=None , out_srs=targetProjection)

	# Clear file
	open(outputFile, 'w').close()

	plyHeader = '''ply
format ascii 1.0
element vertex '''+str(inHeader.count)+'''
property float x
property float y
property float z
property uchar red
property uchar green
property uchar blue
end_header
'''
	newLine = ''
	newFile = open(outputFile, 'w')
	for p in inFile:
		newline = '%.2f %.2f %.2f %i %i %i' % (p.x, p.y, p.z, p.intensity, p.intensity, p.intensity)
		newFile.write(newline + "\n")
	newFile.close()

	line_prepend(outputFile, plyHeader)

if __name__ == '__main__':
	assert len(sys.argv) == 3, 'Usage: python las2ply.py in.las out.ply'
	inputFile = sys.argv[1]
	outputFile = sys.argv[2]
	las2ply(inputFile,outputFile)