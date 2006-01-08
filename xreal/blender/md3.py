import sys, struct, string, math
from types import *

import os
from os import path

GAMEDIR = '/opt/xreal/base/'

MD3_IDENT = "IDP3"
MD3_VERSION = 15
MD3_MAX_QPATH = 64
MD3_MAX_TAGS = 16
MD3_MAX_SURFACES = 32
MD3_MAX_FRAMES = 1024
MD3_MAX_SHADERS = 256
MD3_MAX_VERTICES = 4096
MD3_MAX_TRIANGLES = 8192
MD3_XYZ_SCALE = (1.0 / 64.0)
MD3_BLENDER_SCALE = (1.0 / 1.0)


def asciiz(s):
	n = 0
	while(ord(s[n]) != 0):
		n = n + 1
	return s[0:n]

# strips the slashes from the back of a string
def stripPath(path):
	for c in range(len(path), 0, -1):
		if path[c-1] == "/" or path[c-1] == "\\":
			path = path[c:]
			break
	return path
	
# strips the model from path
def stripModel(path):
	for c in range(len(path), 0, -1):
		if path[c-1] == "/" or path[c-1] == "\\":
			path = path[:c]
			break
	return path

# strips file type extension
def stripExtension(name):
	if name.find('.') != -1:
		name = name[:name.find('.')]
	return name
	
# strips gamedir
def stripGamePath(name):
	if name[0:len(GAMEDIR)] == GAMEDIR:
		name = name[len(GAMEDIR):len(name)]
	return name



class md3Vert:
	xyz = []
	normal = 0
	binaryFormat = "<3hh"
	
	def __init__(self):
		self.xyz = [0, 0, 0]
		self.normal = 0
		
	def getSize(self):
		return struct.calcsize(self.binaryFormat)
	
	# copied from PhaethonH <phaethon@linux.ucla.edu> md3.py
	def decode(self, latlng):
		lat = (latlng >> 8) & 0xFF;
		lng = (latlng) & 0xFF;
		lat *= math.pi / 128;
		lng *= math.pi / 128;
		x = math.cos(lat) * math.sin(lng)
		y = math.sin(lat) * math.sin(lng)
		z =                 math.cos(lng)
		retval = [ x, y, z ]
		return retval
	
	# copied from PhaethonH <phaethon@linux.ucla.edu> md3.py
	def encode(self, normal):
		x, y, z = normal
		
		# normalise
		l = math.sqrt((x*x) + (y*y) + (z*z))
		if l == 0:
			return 0
		x = x/l
		y = y/l
		z = z/l
		
		if (x == 0.0) & (y == 0.0) :
			if z > 0.0:
				return 0
			else:
				return (128 << 8)
		
		# encode a normal vector into a 16-bit latitude-longitude value
		#lng = math.acos(z)
		#lat = math.acos(x / math.sin(lng))
		#retval = ((lat & 0xFF) << 8) | (lng & 0xFF)
		lng = math.acos(z) * 255 / (2 * math.pi)
		lat = math.atan2(y, x) * 255 / (2 * math.pi)
		retval = ((int(lat) & 0xFF) << 8) | (int(lng) & 0xFF)
		return retval
	
	def load(self, file):
		tmpData = file.read(struct.calcsize(self.binaryFormat))
		data = struct.unpack(self.binaryFormat, tmpData)
		self.xyz[0] = data[0] * MD3_XYZ_SCALE
		self.xyz[1] = data[1] * MD3_XYZ_SCALE
		self.xyz[2] = data[2] * MD3_XYZ_SCALE
		self.normal = data[3]
		return self
		
	def save(self, file):
		tmpData = [0] * 4
		tmpData[0] = self.xyz[0] / MD3_XYZ_SCALE
		tmpData[1] = self.xyz[1] / MD3_XYZ_SCALE
		tmpData[2] = self.xyz[2] / MD3_XYZ_SCALE
		tmpData[3] = self.normal
		data = struct.pack(self.binaryFormat, tmpData[0], tmpData[1], tmpData[2], tmpData[3])
		file.write(data)
		#print "Wrote MD3 Vertex: ", data
	
	def dump(self):
		print "MD3 Vertex"
		print "X: ", self.xyz[0]
		print "Y: ", self.xyz[1]
		print "Z: ", self.xyz[2]
		print "Normal: ", self.normal
		print ""
		
class md3TexCoord:
	u = 0.0
	v = 0.0

	binaryFormat = "<2f"

	def __init__(self):
		self.u = 0.0
		self.v = 0.0
		
	def getSize(self):
		return struct.calcsize(self.binaryFormat)

	def load(self, file):
		tmpData = file.read(struct.calcsize(self.binaryFormat))
		data = struct.unpack(self.binaryFormat, tmpData)
		# for some reason quake3 texture maps are upside down, flip that
		self.u = data[0]
		self.v = 1.0 - data[1]
		return self

	def save(self, file):
		tmpData = [0] * 2
		tmpData[0] = self.u
		tmpData[1] = 1.0 - self.v
		data = struct.pack(self.binaryFormat, tmpData[0], tmpData[1])
		file.write(data)
		#print "wrote MD3 texture coordinate structure: ", data

	def dump(self):
		print "MD3 Texture Coordinates"
		print "U: ", self.u
		print "V: ", self.v
		print ""
		

class md3Triangle:
	indexes = []

	binaryFormat = "<3i"

	def __init__(self):
		self.indexes = [ 0, 0, 0 ]
		
	def getSize(self):
		return struct.calcsize(self.binaryFormat)

	def load(self, file):
		tmpData = file.read(struct.calcsize(self.binaryFormat))
		data = struct.unpack(self.binaryFormat, tmpData)
		self.indexes[0] = data[0]
		self.indexes[1] = data[2] # reverse
		self.indexes[2] = data[1] # reverse
		return self

	def save(self, file):
		tmpData = [0] * 3
		tmpData[0] = self.indexes[0]
		tmpData[1] = self.indexes[2] # reverse
		tmpData[2] = self.indexes[1] # reverse
		data = struct.pack(self.binaryFormat,tmpData[0], tmpData[1], tmpData[2])
		file.write(data)
		#print "wrote MD3 face structure: ",data

	def dump(self):
		print "MD3 Triangle"
		print "Indices: ", self.indexes
		print ""


class md3Shader:
	name = ""
	index = 0
	
	binaryFormat = "<%dsi" % MD3_MAX_QPATH

	def __init__(self):
		self.name = ""
		self.index = 0
		
	def getSize(self):
		return struct.calcsize(self.binaryFormat)

	def load(self, file):
		tmpData = file.read(struct.calcsize(self.binaryFormat))
		data = struct.unpack(self.binaryFormat, tmpData)
		self.name = asciiz(data[0])
		self.index = data[1]
		return self

	def save(self, file):
		tmpData = [0] * 2
		tmpData[0] = self.name
		tmpData[1] = self.index
		data = struct.pack(self.binaryFormat, tmpData[0], tmpData[1])
		file.write(data)
		#print "wrote MD3 shader structure: ",data

	def dump (self):
		print "MD3 Shader"
		print "Name: ", self.name
		print "Index: ", self.index
		print ""


class md3Surface:
	ident = ""
	name = ""
	flags = 0
	numFrames = 0
	numShaders = 0
	numVerts = 0
	numTriangles = 0
	ofsTriangles = 0
	ofsShaders = 0
	ofsUV = 0
	ofsVerts = 0
	ofsEnd = 0
	shaders = []
	triangles = []
	uv = []
	verts = []
	
	binaryFormat = "<4s%ds10i" % MD3_MAX_QPATH  # 1 int, name, then 10 ints
	
	def __init__(self):
		self.ident = ""
		self.name = ""
		self.flags = 0
		self.numFrames = 0
		self.numShaders = 0
		self.numVerts = 0
		self.numTriangles = 0
		self.ofsTriangles = 0
		self.ofsShaders = 0
		self.ofsUV = 0
		self.ofsVerts = 0
		self.ofsEnd
		self.shaders = []
		self.triangles = []
		self.uv = []
		self.verts = []
		
	def getSize(self):
		sz = struct.calcsize(self.binaryFormat)
		self.ofsTriangles = sz
		for t in self.triangles:
			sz += t.getSize()
		self.ofsShaders = sz
		for s in self.shaders:
			sz += s.getSize()
		self.ofsUV = sz
		for u in self.uv:
			sz += u.getSize()
		self.ofsVerts = sz
		for v in self.verts:
			sz += v.getSize()
		self.ofsEnd = sz
		return self.ofsEnd
		
	def load(self, file):
		# where are we in the file (for calculating real offsets)
		ofsBegin = file.tell()
		tmpData = file.read(struct.calcsize(self.binaryFormat))
		data = struct.unpack(self.binaryFormat, tmpData)
		self.ident = data[0]
		self.name = asciiz(data[1])
		self.flags = data[2]
		self.numFrames = data[3]
		self.numShaders = data[4]
		self.numVerts = data[5]
		self.numTriangles = data[6]
		self.ofsTriangles = data[7]
		self.ofsShaders = data[8]
		self.ofsUV = data[9]
		self.ofsVerts = data[10]
		self.ofsEnd = data[11]
		
		# load the tri info
		file.seek(ofsBegin + self.ofsTriangles, 0)
		for i in range(0, self.numTriangles):
			self.triangles.append(md3Triangle())
			self.triangles[i].load(file)
			#self.triangles[i].dump()
		
		# load the shader info
		file.seek(ofsBegin + self.ofsShaders, 0)
		for i in range(0, self.numShaders):
			self.shaders.append(md3Shader())
			self.shaders[i].load(file)
			self.shaders[i].dump()
			
		# load the uv info
		file.seek(ofsBegin + self.ofsUV, 0)
		for i in range(0, self.numVerts):
			self.uv.append(md3TexCoord())
			self.uv[i].load(file)
			#self.uv[i].dump()
			
		# load the verts info
		file.seek(ofsBegin + self.ofsVerts, 0)
		for i in range(0, self.numFrames):
			for j in range(0, self.numVerts):
				self.verts.append(md3Vert())
				#i*self.numVerts+j=where in the surface vertex list the vert position for this frame is
				self.verts[(i * self.numVerts) + j].load(file)
				#self.verts[j].dump()
			
		# go to the end of this structure
		file.seek(ofsBegin+self.ofsEnd, 0)
			
		return self
	
	def save(self, file):
		self.getSize()
		tmpData = [0] * 12
		tmpData[0] = self.ident
		tmpData[1] = self.name
		tmpData[2] = self.flags
		tmpData[3] = self.numFrames
		tmpData[4] = self.numShaders
		tmpData[5] = self.numVerts
		tmpData[6] = self.numTriangles
		tmpData[7] = self.ofsTriangles
		tmpData[8] = self.ofsShaders
		tmpData[9] = self.ofsUV
		tmpData[10] = self.ofsVerts
		tmpData[11] = self.ofsEnd
		data = struct.pack(self.binaryFormat, tmpData[0],tmpData[1],tmpData[2],tmpData[3],tmpData[4],tmpData[5],tmpData[6],tmpData[7],tmpData[8],tmpData[9],tmpData[10],tmpData[11])
		file.write(data)

		# write the tri data
		for t in self.triangles:
			t.save(file)

		# save the shader coordinates
		for s in self.shaders:
			s.save(file)

		#save the uv info
		for u in self.uv:
			u.save(file)

		#save the verts
		for v in self.verts:
			v.save(file)

	def dump(self):
		print "MD3 Surface"
		print "Ident: ", self.ident
		print "Name: ", self.name
		print "Flags: ", self.flags
		print "Number of Frames: ", self.numFrames
		print "Number of Shaders: ", self.numShaders
		print "Number of Verts: ", self.numVerts
		print "Number of Triangles: ", self.numTriangles
		print "Offset to Triangles: ", self.ofsTriangles
		print "Offset to Shaders: ", self.ofsShaders
		print "Offset to UV: ", self.ofsUV
		print "Offset to Verts: ", self.ofsVerts
		print "Offset to end: ", self.ofsEnd
		print ""
		

class md3Tag:
	name = ""
	origin = []
	axis = []
	
	binaryFormat="<%ds3f9f" % MD3_MAX_QPATH
	
	def __init__(self):
		self.name = ""
		self.origin = [0, 0, 0]
		self.axis = [0, 0, 0, 0, 0, 0, 0, 0, 0]
		
	def getSize(self):
		return struct.calcsize(self.binaryFormat)
		
	def load(self, file):
		tmpData = file.read(struct.calcsize(self.binaryFormat))
		data = struct.unpack(self.binaryFormat, tmpData)
		self.name = asciiz(data[0])
		self.origin[0] = data[1]
		self.origin[1] = data[2]
		self.origin[2] = data[3]
		self.axis[0] = data[4]
		self.axis[1] = data[5]
		self.axis[2] = data[6]
		self.axis[3] = data[7]
		self.axis[4] = data[8]
		self.axis[5] = data[9]
		self.axis[6] = data[10]
		self.axis[7] = data[11]
		self.axis[8] = data[12]
		return self
		
	def save(self, file):
		tmpData = [0] * 13
		tmpData[0] = self.name
		tmpData[1] = float(self.origin[0])
		tmpData[2] = float(self.origin[1])
		tmpData[3] = float(self.origin[2])
		tmpData[4] = float(self.axis[0])
		tmpData[5] = float(self.axis[1])
		tmpData[6] = float(self.axis[2])
		tmpData[7] = float(self.axis[3])
		tmpData[8] = float(self.axis[4])
		tmpData[9] = float(self.axis[5])
		tmpData[10] = float(self.axis[6])
		tmpData[11] = float(self.axis[7])
		tmpData[12] = float(self.axis[8])
		data = struct.pack(self.binaryFormat, tmpData[0],tmpData[1],tmpData[2],tmpData[3],tmpData[4],tmpData[5],tmpData[6], tmpData[7], tmpData[8], tmpData[9], tmpData[10], tmpData[11], tmpData[12])
		file.write(data)
		#print "wrote MD3 Tag structure: ",data
		
	def dump(self):
		print "MD3 Tag"
		print "Name: ", self.name
		print "Origin: ", self.origin
		print "Axis: ", self.axis
		print ""
	
class md3Frame:
	mins = 0
	maxs = 0
	localOrigin = 0
	radius = 0.0
	name = ""
	
	binaryFormat="<3f3f3ff16s"
	
	def __init__(self):
		self.mins = [0, 0, 0]
		self.maxs = [0, 0, 0]
		self.localOrigin = [0, 0, 0]
		self.radius = 0.0
		self.name = ""
		
	def getSize(self):
		return struct.calcsize(self.binaryFormat)

	def load(self, file):
		tmpData = file.read(struct.calcsize(self.binaryFormat))
		data = struct.unpack(self.binaryFormat, tmpData)
		self.mins[0] = data[0]
		self.mins[1] = data[1]
		self.mins[2] = data[2]
		self.maxs[0] = data[3]
		self.maxs[1] = data[4]
		self.maxs[2] = data[5]
		self.localOrigin[0] = data[6]
		self.localOrigin[1] = data[7]
		self.localOrigin[2] = data[8]
		self.radius = data[9]
		self.name = asciiz(data[10])
		return self

	def save(self, file):
		tmpData = [0] * 11
		tmpData[0] = self.mins[0]
		tmpData[1] = self.mins[1]
		tmpData[2] = self.mins[2]
		tmpData[3] = self.maxs[0]
		tmpData[4] = self.maxs[1]
		tmpData[5] = self.maxs[2]
		tmpData[6] = self.localOrigin[0]
		tmpData[7] = self.localOrigin[1]
		tmpData[8] = self.localOrigin[2]
		tmpData[9] = self.radius
		tmpData[10] = self.name
		data = struct.pack(self.binaryFormat, tmpData[0],tmpData[1],tmpData[2],tmpData[3],tmpData[4],tmpData[5],tmpData[6],tmpData[7], tmpData[8], tmpData[9], tmpData[10])
		file.write(data)
		#print "wrote MD3 frame structure: ",data

	def dump(self):
		print "MD3 Frame"
		print "Min Bounds: ", self.mins
		print "Max Bounds: ", self.maxs
		print "Local Origin: ", self.localOrigin
		print "Radius: ", self.radius
		print "Name: ", self.name
		print ""

class md3Object:
	# header structure
	ident = ""			# this is used to identify the file (must be IDP3)
	version = 0			# the version number of the file (Must be 15)
	name = ""
	flags = 0
	numFrames = 0
	numTags = 0
	numSurfaces = 0
	numSkins = 0
	ofsFrames = 0
	ofsTags = 0
	ofsSurfaces = 0
	ofsEnd = 0
	frames = []
	tags = []
	surfaces = []

	binaryFormat="<4si%ds9i" % MD3_MAX_QPATH  # little-endian (<), 17 integers (17i)

	def __init__(self):
		self.ident = 0
		self.version = 0
		self.name = ""
		self.flags = 0
		self.numFrames = 0
		self.numTags = 0
		self.numSurfaces = 0
		self.numSkins = 0
		self.ofsFrames = 0
		self.ofsTags = 0
		self.ofsSurfaces = 0
		self.ofsEnd = 0
		self.frames = []
		self.tags = []
		self.surfaces = []

	def getSize(self):
		self.ofsFrames = struct.calcsize(self.binaryFormat)
		self.ofsTags = self.ofsFrames
		for f in self.frames:
			self.ofsTags += f.getSize()
		self.ofsSurfaces += self.ofsTags
		for t in self.tags:
			self.ofsSurfaces += t.getSize()
		self.ofsEnd = self.ofsSurfaces
		for s in self.surfaces:
			self.ofsEnd += s.getSize()
		return self.ofsEnd

	def load(self, file):
		tmpData = file.read(struct.calcsize(self.binaryFormat))
		data = struct.unpack(self.binaryFormat, tmpData)

		self.ident = data[0]
		self.version = data[1]

		if(self.ident != "IDP3" or self.version != 15):
			print "Not a valid MD3 file"
			print "Ident=", self.ident
			print "Version=",self.version
			Exit()

		self.name = asciiz(data[2])
		self.flags = data[3]
		self.numFrames = data[4]
		self.numTags = data[5]
		self.numSurfaces = data[6]
		self.numSkins = data[7]
		self.ofsFrames = data[8]
		self.ofsTags = data[9]
		self.ofsSurfaces = data[10]
		self.ofsEnd = data[11]

		# load the frame info
		file.seek(self.ofsFrames, 0)
		for i in range(0, self.numFrames):
			self.frames.append(md3Frame())
			self.frames[i].load(file)
			#self.frames[i].dump()
		
		# load the tags info
		file.seek(self.ofsTags, 0)
		for i in range(0, self.numFrames):
			for j in range(0, self.numTags):
				tag = md3Tag()
				tag.load(file)
				#tag.dump()
				self.tags.append(tag)
		
		# load the surface info
		file.seek(self.ofsSurfaces, 0)
		for i in range(0, self.numSurfaces):
			self.surfaces.append(md3Surface())
			self.surfaces[i].load(file)
			self.surfaces[i].dump()
		return self

	def save(self, file):
		self.getSize()
		tmpData = [0] * 12
		tmpData[0] = self.ident
		tmpData[1] = self.version
		tmpData[2] = self.name
		tmpData[3] = self.flags
		tmpData[4] = self.numFrames
		tmpData[5] = self.numTags
		tmpData[6] = self.numSurfaces
		tmpData[7] = self.numSkins
		tmpData[8] = self.ofsFrames
		tmpData[9] = self.ofsTags
		tmpData[10] = self.ofsSurfaces
		tmpData[11] = self.ofsEnd

		data = struct.pack(self.binaryFormat, tmpData[0],tmpData[1],tmpData[2],tmpData[3],tmpData[4],tmpData[5],tmpData[6],tmpData[7], tmpData[8], tmpData[9], tmpData[10], tmpData[11])
		file.write(data)

		for f in self.frames:
			f.save(file)
			
		for t in self.tags:
			t.save(file)
			
		for s in self.surfaces:
			s.save(file)

	def dump(self):
		print "Header Information"
		print "ident: ", self.ident
		print "version: ", self.version
		print "Name: ", self.name
		print "Flags: ", self.flags
		print "Number of Frames: ",self.numFrames
		print "Number of Tags: ", self.numTags
		print "Number of Surfaces: ", self.numSurfaces
		print "Number of Skins: ", self.numSkins
		print "Offset Frames: ", self.ofsFrames
		print "Offset Tags: ", self.ofsTags
		print "Offset Surfaces: ", self.ofsSurfaces
		print "Offset end: ", self.ofsEnd
		print ""
