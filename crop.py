from liblas import file
from liblas import srs
from liblas import header
from pyproj import Proj
from pyproj import transform
from math import *
import sys

#	http://svn.openstreetmap.org/applications/routing/pyroute/tilenames.py
#
def numTiles(z):
  return(pow(2,z))

def latEdges(y,z):
  n = numTiles(z)
  unit = 1 / n
  relY1 = y * unit
  relY2 = relY1 + unit
  lat1 = mercatorToLat(pi * (1 - 2 * relY1))
  lat2 = mercatorToLat(pi * (1 - 2 * relY2))
  return(lat1,lat2)

def lonEdges(x,z):
  n = numTiles(z)
  unit = 360 / n
  lon1 = -180 + x * unit
  lon2 = lon1 + unit
  return(lon1,lon2)
  
def tileEdges(x,y,z):
  lat1,lat2 = latEdges(y,z)
  lon1,lon2 = lonEdges(x,z)
  return((lat2, lon1, lat1, lon2)) # S,W,N,E

def mercatorToLat(mercatorY):
  return(degrees(atan(sinh(mercatorY))))

# WGS 84 / Pseudo-Mercator -- Spherical Mercator, Google Maps, OpenStreetMap, Bing, ArcGIS, ESRI
targetProjection = srs.SRS()
targetProjection.set_userinput('epsg:3857') # Ending projection ( http://epsg.io/3857 
# print targetProjection.proj4

def print_stuff(inputFile,x, y, z):
  # Choose a tile to extract
  # bBox = tileEdges(19299,24631,16) # Empire State
  # bBox = tileEdges(19296,24633,16) 
  bBox = tileEdges(x, y, z) 
  # print bBox

  # Project the coorners to Spherical Mercator
  latLonProj = Proj(init='epsg:4326') # LAT/LON
  sphericalMercatorProj = Proj(init='epsg:3857') # Spherical Mercator

  south, north = transform(latLonProj,sphericalMercatorProj,bBox[0],bBox[2])
  west, east = transform(latLonProj,sphericalMercatorProj,bBox[1],bBox[3])
  # print "S/N ", south, north
  # print "W/E ", west, east

  minY, minX = min(south,north), min(west,east)
  maxY, maxX = max(south,north), max(west,east)
  # print 'Min %f %f ' % (minX, minY)
  # print 'Max %f %f ' % (maxX, maxY)

  # Open the LAS file and reproject to Pseudo-Mercator
  f = file.File(inputFile, header=None, mode='r', in_srs=None , out_srs=targetProjection)

  # filter the outside the bBox (projected)
  for p in f:
    if minX <= p.x <= maxX and minY <= p.y <= maxY:
      print '%f,%f,%f' % (p.x, p.y, p.z)


if __name__ == '__main__':
  assert len(sys.argv) == 5, 'Usage: python crop.py inFile x y z'
  inputFile = sys.argv[1]
  x, y, z = map(float, sys.argv[2:])
  print_stuff(inputFile,x, y, z)
