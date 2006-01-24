import sys, struct, string, math
from types import *

import os
from os import path

import q_shared
from q_shared import *

import q_math
from q_math import *

GLM_IDENT = "2LGM"
GLM_VERSION = 6
GLM_MAX_QPATH = 64
GLM_MD3_STYLE_TAGS = 1

class glmBoneRef:
	index = 0

	binaryFormat = "<i"

	def __init__(self):
		self.index = 0
		
	def getSize(self):
		return struct.calcsize(self.binaryFormat)

	def load(self, file):
		tmpData = file.read(struct.calcsize(self.binaryFormat))
		data = struct.unpack(self.binaryFormat, tmpData)
		self.index = data[0]
		return self

	def save(self, file):
		tmpData = [0] * 1
		tmpData[0] = self.index
		data = struct.pack(self.binaryFormat,tmpData[0])
		file.write(data)
		#print "wrote GLM boneref structure: ",data

	def dump(self):
		print "GLM BoneRefs"
		print "Indices: ", self.index
		print ""

class glmVert:
	xyz = [0, 0, 0]
	normal = [0, 0, 0]
	weightsAndIndexes = 0
	boneWeightings = [0, 0, 0, 0]
	binaryFormat = "<ffffffiBBBB"
	
	def __init__(self):
		self.xyz = [0, 0, 0]
		self.normal = [0, 0, 0]
		
	def getSize(self):
		return struct.calcsize(self.binaryFormat)
	
	def load(self, file):
		tmpData = file.read(struct.calcsize(self.binaryFormat))
		data = struct.unpack(self.binaryFormat, tmpData)
		self.normal[0] = data[0]
		self.normal[1] = data[1]
		self.normal[2] = data[2]
		self.xyz[0] = data[3]
		self.xyz[1] = data[4]
		self.xyz[2] = data[5]
		self.weightsAndIndexes = data[6]
		self.boneWeightings[0] = data[7]
		self.boneWeightings[1] = data[8]
		self.boneWeightings[2] = data[9]
		self.boneWeightings[3] = data[10]
		return self
		
	def save(self, file):
		tmpData = [0] * 11
		tmpData[0] = self.normal[0]
		tmpData[1] = self.normal[1]
		tmpData[2] = self.normal[2]
		tmpData[3] = self.xyz[0]
		tmpData[4] = self.xyz[1]
		tmpData[5] = self.xyz[2]
		tmpData[6] = self.weightsAndIndexes
		tmpData[7] = self.boneWeightings[0]
		tmpData[8] = self.boneWeightings[1]
		tmpData[9] = self.boneWeightings[2]
		tmpData[10] = self.boneWeightings[3]
		data = struct.pack(self.binaryFormat, tmpData[0], tmpData[1], tmpData[2], tmpData[3], tmpData[4], tmpData[5], tmpData[6], tmpData[7], tmpData[8], tmpData[9], tmpData[10])
		file.write(data)
		#print "wrote GLM Vertex: ", data
	
	def dump(self):
		print "GLM  Vertex"
		print "X: ", self.xyz[0]
		print "Y: ", self.xyz[1]
		print "Z: ", self.xyz[2]
		print "Normal X: ", self.normal[0]
		print "Normal Y: ", self.normal[1]
		print "Normal Z: ", self.normal[2]
		print ""
		
class glmTexCoord:
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
		#print "wrote GLM texture coordinate structure: ", data

	def dump(self):
		print "GLM Texture Coordinates"
		print "U: ", self.u
		print "V: ", self.v
		print ""

class glmTriangle:
	ind = [0, 0, 0]
	
	binaryFormat="<3i"
	
	def __init__(self):
		self.ind = [0, 0, 0]
		
	def getSize(self):
		return struct.calcsize(self.binaryFormat)

	def load(self, file):
		tmpData = file.read(struct.calcsize(self.binaryFormat))
		data = struct.unpack(self.binaryFormat, tmpData)
		self.ind[0] = data[0]
		self.ind[1] = data[2]
		self.ind[2] = data[1]
		return self

	def save(self, file):
		tmpData = [0] * 3
		tmpData[0] = self.ind[0]
		tmpData[1] = self.ind[2] # reverse
		tmpData[2] = self.ind[1] # reverse
		data = struct.pack(self.binaryFormat,tmpData[0], tmpData[1], tmpData[2])
		file.write(data)
		#print "wrote GLM face structure: ",data

	def dump(self):
		print "GLM Triangle"
		print "Index 0: ", self.ind[0]
		print "Index 1: ", self.ind[1]
		print "Index 2: ", self.ind[2]
		print ""

class glmTag:
	name = ""
	origin = []
	axis = []
	v1 = []
	v2 = []
	v3 = []
	
	#binaryFormat="<%ds3f9f" % MAX_QPATH
	
	def __init__(self):
		self.name = ""
		self.origin = [0, 0, 0]
		self.axis = [0, 0, 0, 0, 0, 0, 0, 0, 0]
		self.v1 = [0, 0, 0]
		self.v2 = [0, 0, 0]
		self.v3 = [0, 0, 0]
		
	#def getSize(self):
	#	return struct.calcsize(self.binaryFormat)

	def fromSurf( self, name, vert0, vert1, vert2 ):
		#vec0 = [0, 0, 0]
		#vec1 = [0, 0, 0]
		#vec2 = [0, 0, 0]
		vec0 = VectorInverse( VectorSubtract( vert1, vert2 ) )
		vec1 = VectorSubtract( vert0, vert2 )
		vec2 = CrossProduct( vec0, vec1 )
		self.origin = [vert2[0], vert2[1], vert2[2]]
		self.axis = [vec0[0], vec0[1], vec1[2], vec1[0], vec1[1], vec1[2], vec2[0], vec2[1], vec2[2]]
		self.name = name
		#vec1[0] = (tag_src[0][0] - tag_src[2][0]);
		#vec1[1] = (tag_src[0][1] - tag_src[2][1]);
		#vec1[2] = (tag_src[0][2] - tag_src[2][2]);
		#vec0[0] = -(tag_src[1][0] - tag_src[2][0]);
		#vec0[1] = -(tag_src[1][1] - tag_src[2][1]);
		#vec0[2] = -(tag_src[1][2] - tag_src[2][2]);
		
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
		#print "wrote GLM Tag structure: ",data
		
	def dump(self):
		print "GLM Tag"
		print "Name: ", self.name
		print "Origin: ", self.origin
		print "Axis: ", self.axis
		print ""

class glmSurface:
	ident = ""
	thisIndex = 0
	ofsHeader = 0
	numVerts = 0
	ofsVerts = 0
	numTriangles = 0
	ofsTriangles = 0
	numBoneRefs = 0
	ofsBoneRefs = 0
	ofsEnd = 0
	name = ""
	shader = ""
	ofsUV = 0
	triangles = []
	uv = []
	verts = []
	bonerefs = []
	
	binaryFormat = "<4s9i" # 1 int, name, then 10 ints
	
	def __init__(self):
		self.ident = ""
		self.thisIndex = 0
		self.numVerts = 0
		self.numTriangles = 0
		self.ofsTriangles = 0
		self.numBoneRefs = 0
		self.ofsBoneRefs = 0
		self.ofsUV = 0
		self.ofsVerts = 0
		self.ofsEnd
		self.bonerefs = []
		self.triangles = []
		self.uv = []
		self.verts = []
		
	def getSize(self):
		sz = struct.calcsize(self.binaryFormat)
		self.numVerts = 0
		self.numTriangles = 0
		self.numBoneRefs = 0
		#first, bone refs
		self.ofsBoneRefs = sz
		for b in self.bonerefs:
			self.numBoneRefs += 1
			sz += b.getSize()
		self.ofsTriangles = sz
		for t in self.triangles:
			self.numTriangles += 1
			sz += t.getSize()
		self.ofsVerts = sz
		for v in self.verts:
			self.numVerts += 1
			sz += v.getSize()
		self.ofsUV = sz
		for u in self.uv:
			sz += u.getSize()
		self.ofsEnd = sz
		return self.ofsEnd
		
	def load(self, file):
		# where are we in the file (for calculating real offsets)
		ofsBegin = file.tell()
		tmpData = file.read(struct.calcsize(self.binaryFormat))
		data = struct.unpack(self.binaryFormat, tmpData)
		self.ident = data[0]
		self.thisIndex = data[1]
		self.ofsHeader = data[2]
		self.numVerts = data[3]
		self.ofsVerts = data[4]
		self.numTriangles = data[5]
		self.ofsTriangles = data[6]
		self.numBoneRefs = data[7]
		self.ofsBoneRefs = data[8]
		self.ofsEnd = data[9]
		self.ofsUV = self.ofsVerts + (self.numVerts * 32)
		#return self
		
		# load the boneref info
		file.seek(ofsBegin + self.ofsBoneRefs, 0)
		for i in range(0, self.numBoneRefs ):
			self.bonerefs.append(glmBoneRef())
			self.bonerefs[i].load(file)
			#self.bonerefs[i].dump()

		# load the tri info
		file.seek(ofsBegin + self.ofsTriangles)
		for i in range(0, self.numTriangles):
			self.triangles.append(glmTriangle())
			self.triangles[i].load(file)
			#self.triangles[i].dump()
			
		file.seek(ofsBegin + self.ofsVerts, 0)
		# load the verts info
		for i in range(0, self.numVerts):
			self.verts.append(glmVert())
			self.verts[i].load(file)
			#self.verts[i].dump()

		# load the uv info
		file.seek(ofsBegin + self.ofsUV, 0)
		for i in range(0, self.numVerts):
			self.uv.append(glmTexCoord())
			self.uv[i].load(file)
			#self.uv[i].dump()
			
		# go to the end of this structure
		file.seek(ofsBegin+self.ofsEnd, 0)
		
		return self
	
	def save(self, file):
		self.getSize()
		tmpData = [0] * 10
		
		tmpData[0] = self.ident
		tmpData[1] = self.thisIndex
		tmpData[2] = self.ofsHeader
		tmpData[3] = self.numVerts
		tmpData[4] = self.ofsVerts
		tmpData[5] = self.numTriangles
		tmpData[6] = self.ofsTriangles
		tmpData[7] = self.numBoneRefs
		tmpData[8] = self.ofsBoneRefs
		tmpData[9] = self.ofsEnd
		data = struct.pack(self.binaryFormat, tmpData[0],tmpData[1],tmpData[2],tmpData[3],tmpData[4],tmpData[5],tmpData[6],tmpData[7],tmpData[8],tmpData[9])
		#print "wrote GLM surface structure: ",data
		file.write(data)
		
		# write the boneref info
		for i in range(0, self.numBoneRefs ):
			self.bonerefs[i].save(file)

		# write the tri data
		for t in self.triangles:
			t.save(file)
		
		#save the verts
		for v in self.verts:
			v.save(file)
		
		self.ofsUV = self.ofsVerts + (self.numVerts * 32)

		#save the uv info
		for u in self.uv:
			u.save(file)

	def dump(self):
		print "GLM Surface"
		print "Ident: ", self.ident
		print "ofsHeader: ", self.ofsHeader
		print "Index: ", self.thisIndex 
		print "numVerts: ", self.numVerts
		print "ofsVerts: ", self.ofsVerts
		print "numTriangles: ", self.numTriangles
		print "ofsTriangles: ", self.ofsTriangles
		print "numBoneRefs: ", self.numBoneRefs
		print "ofsBoneRefs: ", self.ofsBoneRefs
		print "Offset to end: ", self.ofsEnd
		print "Offset to UV: ", self.ofsUV
		print ""
		#for i in range(0, self.numTriangles):
		#	print "idx 0", self.triangles[i].ind[0]
		#	print "idx 1", self.triangles[i].ind[1]
		#	print "idx 2", self.triangles[i].ind[2]
		for i in range(0, self.numVerts):
			print "vert", i,
			print "u", self.uv[i].u,
			print "v", self.uv[i].v
		print ""

class glmLod:
	ofsEnd = 0
	ofsSurfs = []
	surfaces = []
	tags = []
	
	binaryFormat = "<i"
	
	def __init__(self):
		self.ofsEnd
		self.ofsSurfs = []
		self.surfaces = []

	def getSize(self, numSurfs):
		sz = struct.calcsize(self.binaryFormat)
		for b in self.ofsSurfs:
			sz += b.getSize()
		for i in range(0, numSurfs):
			self.ofsSurfs[i].index = sz - 4
			sz += self.surfaces[i].getSize()
		#for b in self.surfaces:
		#	sz += b.getSize()
		self.ofsEnd = sz
		return self.ofsEnd
		
	def load(self, numSurfs, file):
		snum = 0
		# where are we in the file (for calculating real offsets)
		ofsBegin = file.tell()
		tmpData = file.read(struct.calcsize(self.binaryFormat))
		data = struct.unpack(self.binaryFormat, tmpData)
		self.ofsEnd = data[0]
		
		# load the surf ofs info
		for i in range(0, numSurfs):
			file.seek(ofsBegin + 4 + (4 * i), 0)
			self.ofsSurfs.append(glmBoneRef())
			self.ofsSurfs[i].load(file)
			self.ofsSurfs[i].index += 4
			#self.ofsSurfs[i].dump()

		# load the surfs info
		for i in range(0, numSurfs):
			file.seek(ofsBegin + self.ofsSurfs[i].index, 0)
			self.surfaces.append(glmSurface())
			self.surfaces[i].load(file)
			self.surfaces[i].dump()

		file.seek(ofsBegin+self.ofsEnd, 0)
		return self
	
	def save(self, numSurfs, file):
		self.getSize(numSurfs)
		tmpData = [0] * 1
		
		tmpData[0] = self.ofsEnd
		
		data = struct.pack(self.binaryFormat, tmpData[0])
		#print "wrote GLM lod structure: ",data
		file.write(data)
		
		for i in range(0, numSurfs):
			self.ofsSurfs[i].save(file)
		
		for i in range(0, numSurfs):
			self.surfaces[i].save(file)

	def dump(self):
		print "GLM LOD - next", self.ofsEnd
		for v in self.ofsSurfs:
			print "Surf ofs", v.index
		print ""

class glmSurfHier:
	name = ""
	flags = 0
	shader = ""
	shaderIndex = 0
	parentIndex = 0
	numChildren = 0
	childIndexes = []
	ofsEnd = 0
	bonerefs = []
	
	binaryFormat = "<64si64s3i"# little-endian (<), 17 integers (17i)
	
	def __init__(self):
		self.name = ""
		self.flags = 0
		self.shader = ""
		self.number = 0
		self.parentIndex = 0
		self.numChildren = 0
		self.childIndexes = []
		self.ofsEnd = 0
		
	def getSize(self):
		sz = struct.calcsize(self.binaryFormat)
		sz += 4 * self.numChildren
		self.ofsEnd = sz
		#print "GetSize SurfHier: ", self.ofsEnd
		return self.ofsEnd
		
	def load(self, file):
		# where are we in the file (for calculating real offsets)
		ofsBegin = file.tell()
		tmpData = file.read(struct.calcsize(self.binaryFormat))
		data = struct.unpack(self.binaryFormat, tmpData)

		self.name = asciiz(data[0])
		self.flags = data[1]
		self.shader = asciiz(data[2])
		self.number = data[3]
		self.parentIndex = data[4]
		self.numChildren = data[5]
		self.getSize()
		
		# load the child info
		file.seek(ofsBegin + struct.calcsize(self.binaryFormat), 0)
		for i in range(0, self.numChildren ):
			self.childIndexes.append(glmBoneRef())
			self.childIndexes[i].load(file)
			#self.bonerefs[i].dump()
			
		# go to the end of this structure
		file.seek(ofsBegin+self.ofsEnd, 0)
		
		return self
	
	def save(self, file):
		self.getSize()
		tmpData = [0] * 6
		
		tmpData[0] = self.name
		tmpData[1] = self.flags
		tmpData[2] = self.shader
		tmpData[3] = self.number
		tmpData[4] = self.parentIndex
		tmpData[5] = self.numChildren
		data = struct.pack(self.binaryFormat, tmpData[0],tmpData[1],tmpData[2],tmpData[3],tmpData[4],tmpData[5])
		#print "wrote GLM surfhier structure: ",data
		file.write(data)

		# write the child indexes data
		for t in self.childIndexes:
			t.save(file)

	def dump(self):
		self.getSize()
		print "GLM Surface"
		print "Name: ", self.name
		print "Flags: ", self.flags
		print "Shader: ", self.shader
		print "Shader: ", self.shaderIndex
		print "Parent: ", self.parentIndex
		print "Children: ", self.numChildren
		print "Offset to end: ", self.ofsEnd
		print ""

class glmObject:
	# header structure
	ident = ""			# this is used to identify the file (must be IDP3)
	version = 0			# the version number of the file (Must be 15)
	name = ""
	animName = ""
	number = 0
	numBones = 0
	numLODs = 0
	ofsLODs = 0
	numSurfaces = 0
	ofsSurfaceHierarchy = 0
	ofsEnd = 0
	lods = []
	surfHier = []

	binaryFormat="<4si64s64s7i"# little-endian (<), 17 integers (17i)

	def __init__(self):
		self.ident = 0
		self.version = 0
		self.name = ""
		self.animName = ""
		self.numBones = 0
		self.numLODs = 0
		self.ofsLODs = 0
		self.numSurfaces = 0
		self.ofsSurfHierarchy = 0
		self.ofsEnd = 0
		self.surfHier = []
		self.lods = []

	def getSize(self):
		sz = struct.calcsize(self.binaryFormat)
		self.ofsSurfHierarchy = sz
		for h in self.surfHier:
			sz += h.getSize()
		self.ofsLODs = sz
		for l in self.lods:
			sz += l.getSize(self.numSurfaces)
		self.ofsEnd = sz
		return self.ofsEnd

	def load(self, file):
		tmpData = file.read(struct.calcsize(self.binaryFormat))
		ofsBegin = 0
		data = struct.unpack(self.binaryFormat, tmpData)

		self.ident = data[0]
		self.version = data[1]

		if(self.ident != "2LGM" or self.version != 6):
			print "Not a valid GLM file"
			print "Ident=", self.ident
			print "Version=",self.version
			Exit()
		
		self.name = data[2]
		self.animName = data[3]
		self.number = data[4]
		self.numBones = data[5]
		self.numLODs = data[6]
		self.ofsLODs = data[7]
		self.numSurfaces = data[8]
		self.ofsSurfHierarchy = data[9]
		self.ofsEnd = data[10]
		
		# load the surface info
		file.seek(ofsBegin + self.ofsSurfHierarchy, 0)
		for i in range(0, self.numSurfaces):
			self.surfHier.append(glmSurfHier())
			self.surfHier[i].load(file)
			self.surfHier[i].dump()
		
		file.seek(ofsBegin + self.ofsLODs, 0)
		for i in range(0, self.numLODs):
			self.lods.append(glmLod())
			self.lods[i].load(self.numSurfaces, file)
			self.lods[i].dump()
        
		return self

	def save(self, file):
		self.getSize()
		tmpData = [0] * 11
		tmpData[0] = self.ident
		tmpData[1] = self.version
		tmpData[2] = self.name
		tmpData[3] = self.animName
		tmpData[4] = self.number
		tmpData[5] = self.numBones
		tmpData[6] = self.numLODs
		tmpData[7] = self.ofsLODs
		tmpData[8] = self.numSurfaces
		tmpData[9] = self.ofsSurfHierarchy
		tmpData[10] = self.ofsEnd
		
		#binaryFormat="<4si64s64s7i"# little-endian (<), 17 integers (17i)
		ident_f="<4si64s64siiiiiii"
		
		#data = struct.pack(ident_f, tmpData[0], tmpData[1], tmpData[2], tmpData[3])
		data = struct.pack(ident_f, tmpData[0],tmpData[1],tmpData[2],tmpData[3],tmpData[4],tmpData[5],tmpData[6],tmpData[7], tmpData[8], tmpData[9], tmpData[10])
		#print "wrote GLM object structure: ",data
		file.write(data)

		for h in self.surfHier:
			h.save(file)

		for l in self.lods:
			l.save(self.numSurfaces, file)
			#l.save(file, self.numSurfaces)

	def dump(self):
		return
		print "Header Information"
		print "ident: ", self.ident
		print "version: ", self.version
		print "Name: ", self.name
		print "AnimName: ", self.animName
		print "NumBones: ", self.numBones
		print "NumLODs: ", self.numLODs
		print "ofsLODs: ", self.ofsLODs
		print "NumSurfs: ", self.numSurfaces
		print "ofsSurfH: ", self.ofsSurfHierarchy
		print "ofsEnd: ", self.ofsEnd
		print ""
