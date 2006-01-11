import sys, struct, string, math
from types import *

import os
from os import path

import q_math
from q_math import *

import q_shared
from q_shared import *

MDS_IDENT = "MDSW"
MDS_VERSION = 4

class mdsWeight:
	boneIndex = 0
	weight = 0
	xyz = [0, 0, 0]
	
	binaryFormat = "<if3f"
	
	def __init__(self):
		self.boneIndex = 0
		self.weight = 0
		self.xyz = [0, 0, 0]
		
	def getSize(self):
		return struct.calcsize(self.binaryFormat)
	
	def load(self, file):
		tmpData = file.read(struct.calcsize(self.binaryFormat))
		data = struct.unpack(self.binaryFormat, tmpData)
		self.boneIndex = data[0]
		self.weight = data[1]
		self.xyz[0] = data[2]
		self.xyz[1] = data[3]
		self.xyz[2] = data[4]
		return self
		
	def save(self, file):
		tmpData = [0] * 5
		tmpData[0] = self.boneIndex
		tmpData[1] = self.weight
		tmpData[2] = self.xyz[0]
		tmpData[3] = self.xyz[1]
		tmpData[4] = self.xyz[2]
		data = struct.pack(self.binaryFormat, tmpData[0], tmpData[1], tmpData[2], tmpData[3], tmpData[4])
		file.write(data)
		#print "Wrote MDS Weight: ", data
	
	def dump(self):
		print "mdsWeight"
		print "boneIndex: ", self.boneIndex
		print "weight: ", self.weight
		print "xyz: ", self.xyz
		print ""

class mdsVert:
	normal = [0, 0, 0]
	st = [0, 0]
	numWeights = 0
	fixedParent = 0
	fixedDist = 0
	weights = []
	
	binaryFormat = "<3f2f"
	
	def __init__(self):
		self.normal = [0, 0, 0]
		self.st = [0, 0]
		self.numWeights = 0
		self.fixedParent = 0
		self.fixedDist = 0
		self.weights = []
		
	def getSize(self):
		return struct.calcsize(self.binaryFormat)
	
	def load(self, file):
		tmpData = file.read(struct.calcsize(self.binaryFormat))
		data = struct.unpack(self.binaryFormat, tmpData)
		self.normal[0] = data[0]
		self.normal[1] = data[1]
		self.normal[2] = data[2]
		self.st[0] = data[3]
		self.st[1] = data[4]
		self.numWeights = data[5]
		self.fixedParent = data[6]
		self.fixedDist = data[7]
		
		# load the vertex weight info
		ofs = struct.calcsize(self.binaryFormat)
		file.seek(ofs, 0)
		for i in range(0, self.numWeights):
			self.weights.append(mdsWeight())
			self.weights[i].load(file)
			self.weights[i].dump()
		return self
	
	def dump(self):
		print "mdsVertex"
		print "normal: ", self.normal
		print "st: ", self.st
		print "numWeights: ", self.numWeights
		print "fixedParent: ", self.fixedParent
		print "fixedDist: ", self.fixedDist
		print ""

class mdsTriangle:
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
		print "mdsTriangle"
		print "indexes: ", self.indexes
		print ""
		
class mdsBone:
	name = ""
	parentIndex = 0
	torsoWeight = 0
	parentDist = 0
	flags = 0
	
	binaryFormat = "<%dsi2fi" % MAX_QPATH
	
	def __init__(self):
		self.name = ""
		self.parentIndex = 0
		self.torsoWeight = 0
		self.parentDist = 0
		self.flags = 0
		
	def load(self, file):
		tmpData = file.read(struct.calcsize(self.binaryFormat))
		data = struct.unpack(self.binaryFormat, tmpData)
		self.name = asciiz(data[0])
		self.parentIndex = data[1]
		self.torsoWeight = data[2]
		self.parentDist = data[3]
		self.flags = data[4]
		return self

	def save(self, file):
		tmpData = [0] * 5
		tmpData[0] = self.name
		tmpData[1] = self.parentIndex
		tmpData[2] = self.torsoWeight
		tmpData[3] = self.parentDist
		tmpData[4] = self.flags
		data = struct.pack(self.binaryFormat,tmpData[0], tmpData[1], tmpData[2], tmpData[3], tmpData[4])
		file.write(data)
		#print "wrote MDS bone structure: ", data

	def dump(self):
		print "mdsBone"
		print "name: ", self.name
		print "parentIndex: ", self.parentIndex
		print "torsoWeight: ", self.torsoWeight
		print "parentDist: ", self.parentDist
		print "flags: ", self.flags
		print ""


class mdsFrameBone:
	angles = []
	offsetAngles = []
	
	binaryFormat = "<4h2h"
	
	def __init__(self):
		self.angles = [0, 0, 0]
		self.offsetAngles = [0, 0]
		
	def load(self, file):
		tmpData = file.read(struct.calcsize(self.binaryFormat))
		data = struct.unpack(self.binaryFormat, tmpData)
		self.angles[0] = SHORT2ANGLE(data[0])
		self.angles[1] = SHORT2ANGLE(data[1])
		self.angles[2] = SHORT2ANGLE(data[2])
		self.offsetAngles[0] = SHORT2ANGLE(data[4])
		self.offsetAngles[1] = SHORT2ANGLE(data[5])
		return self

	def save(self, file):
		tmpData = [0] * 6
		tmpData[0] = ANGLE2SHORT(self.angles[0])
		tmpData[1] = ANGLE2SHORT(self.angles[1])
		tmpData[2] = ANGLE2SHORT(self.angles[2])
		tmpData[3] = 0
		tmpData[4] = ANGLE2SHORT(self.offsetAngles[0])
		tmpData[5] = ANGLE2SHORT(self.offsetAngles[1])
		data = struct.pack(self.binaryFormat,tmpData[0], tmpData[1], tmpData[2], tmpData[3], tmpData[4], tmpData[5])
		file.write(data)
		#print "wrote compressed MDS bone structure: ", data

	def dump(self):
		print "mdsFrameBone"
		print "angles: ", self.angles
		print "offsetAngles: ", self.offsetAngles
		print ""

class mdsFrame:
	mins = 0
	maxs = 0
	localOrigin = 0
	radius = 0.0
	parentOffset = 0
	bones = []
	
	binaryFormat="<3f3f3ff3f"
	
	def __init__(self):
		self.mins = [0, 0, 0]
		self.maxs = [0, 0, 0]
		self.localOrigin = [0, 0, 0]
		self.radius = 0.0
		self.parentOffset = [0, 0, 0]
		self.bones = []
		
	def getSize(self):
		return struct.calcsize(self.binaryFormat)

	def load(self, file, numBones):
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
		self.parentOffset[0] = data[10]
		self.parentOffset[1] = data[11]
		self.parentOffset[2] = data[12]
		
		# load the compressed frame bones
		for i in range(0, numBones):
			self.bones.append(mdsFrameBone())
			self.bones[i].load(file)
			#self.bones[i].dump()
		return self

	"""
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
	"""
	
	def dump(self):
		print "mdsFrame"
		print "mins: ", self.mins
		print "maxs: ", self.maxs
		print "localOrigin: ", self.localOrigin
		print "radius: ", self.radius
		print "parentOffset: ", self.parentOffset
		print ""


class mdsObject:
	# header structure
	ident = ""			# this is used to identify the file (must be MDSW)
	version = 0			# the version number of the file (Must be 4)
	name = ""
	lodScale = 0
	lodBias = 0
	numFrames = 0
	numBones = 0
	ofsFrames = 0
	ofsBones = 0
	torsoParent = 0
	numSurfaces = 0
	ofsSurfaces = 0
	numTags = 0
	ofsTags = 0
	ofsEnd = 0
	frames = []
	bones = []
	tags = []
	surfaces = []

	binaryFormat="<4si%ds2f10i" % MAX_QPATH  # little-endian (<)

	def __init__(self):
		self.ident = 0
		self.version = 0
		self.name = ""
		self.lodScale = 0
		self.lodBias = 0
		self.numFrames = 0
		self.numBones = 0
		self.ofsFrames = 0
		self.ofsBones = 0
		self.torsoParent = 0
		self.numSurfaces = 0
		self.ofsSurfaces = 0
		self.numTags = 0
		self.ofsTags = 0
		self.ofsEnd = 0
		self.frames = []
		self.bones = []
		self.tags = []
		self.surfaces = []

	"""
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
	"""

	def load(self, file):
		tmpData = file.read(struct.calcsize(self.binaryFormat))
		data = struct.unpack(self.binaryFormat, tmpData)

		self.ident = data[0]
		self.version = data[1]

		if(self.ident != MDS_IDENT or self.version != MDS_VERSION):
			print "Not a valid MDS file"
			print "Ident = ", self.ident
			print "Version = ",self.version
			Exit()

		self.name = asciiz(data[2])
		self.lodScale = data[3]
		self.lodBias = data[4]
		self.numFrames = data[5]
		self.numBones = data[6]
		self.ofsFrames = data[7]
		self.ofsBones = data[8]
		self.torsoParent = data[9]
		self.numSurfaces = data[10]
		self.ofsSurfaces = data[11]
		self.numTags = data[12]
		self.ofsTags = data[13]
		self.ofsEnd = data[14]
		
		# load the bones
		file.seek(self.ofsBones, 0)
		for i in range(0, self.numBones):
			self.bones.append(mdsBone())
			self.bones[i].load(file)
			self.bones[i].dump()

		# load the frames
		file.seek(self.ofsFrames, 0)
		for i in range(0, self.numFrames):
			self.frames.append(mdsFrame())
			self.frames[i].load(file, self.numBones)
			#self.frames[i].dump()
		
		# load the tags info
		#file.seek(self.ofsTags, 0)
		#for i in range(0, self.numTags):
		#	self.tags.append(md3Tag())
		#	self.tags[i].load(file)
		#	self.tags[i].dump()
		
		# load the surface info
		#file.seek(self.ofsSurfaces, 0)
		#for i in range(0, self.numSurfaces):
		#	self.surfaces.append(md3Surface())
		#	self.surfaces[i].load(file)
		#	self.surfaces[i].dump()
		return self

	"""
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

		data=struct.pack(self.binaryFormat, tmpData[0],tmpData[1],tmpData[2],tmpData[3],tmpData[4],tmpData[5],tmpData[6],tmpData[7], tmpData[8], tmpData[9], tmpData[10], tmpData[11])
		file.write(data)

		for f in self.frames:
			f.save(file)
			
		for t in self.tags:
			t.save(file)
			
		for s in self.surfaces:
			s.save(file)
	"""

	def dump(self):
		print "Header Information"
		print "ident: ", self.ident
		print "version: ", self.version
		print "name: ", self.name
		print "lodScale: ", self.lodScale
		print "lodBias: ", self.lodBias
		print "numFrames: ",self.numFrames
		print "numBones: ", self.numBones
		print "ofsFrames: ", self.ofsFrames
		print "ofsBones: ", self.ofsBones
		print "torsoParent: ", self.torsoParent
		print "numSurfaces: ", self.numSurfaces
		print "ofsSurfaces: ", self.ofsSurfaces
		print "numTags: ", self.numTags
		print "ofsTags: ", self.ofsTags
		print "ofsEnd: ", self.ofsEnd
		print ""
