#!BPY

""" Registration info for Blender menus:
Name: 'Quake3 (.md3)'
Blender: 237
Group: 'Import'
Tooltip: 'Import from Quake3 file format. (.md3)'
"""

__author__ = "Bob Holcomb, Robert (Tr3B) Beckebans"
__url__ = ("http://xreal.sourceforge.net")
__version__ = "0.3 2004-10-21"

__bpydoc__ = """\
This script imports a Quake 3 file (MD3), textures, 
and animations into Blender for editing. Loader is based on MD3 loader
from www.gametutorials.com-Thanks DigiBen! and the
md3 blender loader by PhaethonH <phaethon@linux.ucla.edu>

Supported:<br>
    TODO

Missing:<br>
    TODO

Known issues:<br>
    None.

Notes:<br>
    TODO
"""

import Blender
from Blender import NMesh, Object
from Blender.BGL import *
from Blender.Draw import *
from Blender.Window import *
from Blender.Image import *

import sys, struct, string
from types import *

import os
from os import path



def asciiz (s):
  n = 0
  while (ord(s[n]) != 0):
    n = n + 1
  return s[0:n]


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


class vert:
	x = 0.0
	y = 0.0
	z = 0.0
	
	def __init__(self):
		self.x = 0.0
		self.y = 0.0
		self.z = 0.0

class md3Vert:
	xyz = []
	normal = 0
	binaryFormat = "<3hh"
	
	def __init__(self):
		self.xyz = [ 0, 0, 0 ]
		self.normal = 0
	
	#copied from PhaethonH <phaethon@linux.ucla.edu> md3.py
	def decode(self, latlng):
		#Decode 16-bit latitude-longitude value into a normal vector.
		#Code ripped from q3toosl/q3map/misc_model.c:
		#            // decode the lat/lng normal to a 3 float normal
		#            lat = ( xyz->normal >> 8 ) & 0xff;
		#            lng = ( xyz->normal & 0xff );
		#            lat *= Q_PI/128;
		#            lng *= Q_PI/128;
		#
		#            temp[0] = cos(lat) * sin(lng);
		#            temp[1] = sin(lat) * sin(lng);
		#            temp[2] = cos(lng);
		lat = (latlng >> 8) & 0xFF;
		lng = (latlng) & 0xFF;
		lat *= math.pi / 128;
		lng *= math.pi / 128;
		x = math.cos(lat) * math.sin(lng)
		y = math.sin(lat) * math.sin(lng)
		z =                 math.cos(lng)
		retval = [ x, y, z ]
		return retval
	
	#copied from PhaethonH <phaethon@linux.ucla.edu> md3.py
	def encode(self, normal):
		#Encode a normal vector into a 16-bit latitude-longitude value.
		x, y, z = normal
		lng = math.acos(z)
		lat = math.acos(x / math.sin(lng))
		retval = ((lat & 0xFF) << 8) | (lng & 0xFF)
		return retval
	
	def load(self, file):
		tmpData = file.read(struct.calcsize(self.binaryFormat))
		data = struct.unpack(self.binaryFormat, tmpData)
		self.xyz[0] = data[0]
		self.xyz[1] = data[1]
		self.xyz[2] = data[2]
		self.normal = data[3]
		return self
		
	def save(self, file):
		tmpData = [0]*4
		tmpData[0] = self.xyz[0]
		tmpData[1] = self.xyz[1]
		tmpData[2] = self.xyz[2]
		tmpData[3] = self.normal
		data = struct.pack(self.binaryFormat, tmpData[0], tmpData[1], tmpData[2], tmpData[3])
		file.write(data)
		#print "Wrote MD3 Vertex: ", data
	
	def dump(self):
		print "MD3 Vertex Structure"
		print "X: ", self.xyz[0]
		print "Y: ", self.xyz[1]
		print "Z: ", self.xyz[2]
		print "Normal: ", self.normal
		print ""

class md3TexCoord:
	u=0.0
	v=0.0

	binaryFormat="<2f"

	def __init__(self):
		self.u=0.0
		self.v=0.0

	def load(self, file):
		tmpData = file.read(struct.calcsize(self.binaryFormat))
		data = struct.unpack(self.binaryFormat, tmpData)
		self.u = data[0]
		self.v = data[1]
		return self

	def save(self, file):
		tmpData = [0]*2
		tmpData[0] = self.u
		tmpData[1] = self.v
		data = struct.pack(self.binaryFormat, tmpData[0], tmpData[1])
		file.write(data)
		#print "wrote MD3 texture coordinate structure: ",data

	def dump(self):
		print "MD3 Texture Coordinate Structure"
		print "texture coordinate u: ", self.u
		print "texture coordinate v: ", self.v
		print ""
		

class md3Triangle:
	indexes=[]

	binaryFormat="<3i"

	def __init__(self):
		self.indexes = [ 0, 0, 0 ]

	def load (self, file):
		tmpData = file.read(struct.calcsize(self.binaryFormat))
		data = struct.unpack(self.binaryFormat, tmpData)
		self.indexes[0] = data[0]
		self.indexes[1] = data[1]
		self.indexes[2] = data[2]
		return self

	def save(self, file):
		tmpData = [0]*3
		tmpData[0] = self.indexes[0]
		tmpData[1] = self.indexes[1]
		tmpData[2] = self.indexes[2]
		data = struct.pack(self.binaryFormat,tmpData[0], tmpData[1], tmpData[2])
		file.write(data)
		#print "wrote MD3 face structure: ",data

	def dump (self):
		print "MD3 Triangle Structure"
		print "vertex index: ", self.indexes[0]
		print "vertex index: ", self.indexes[1]
		print "vertex index: ", self.indexes[2]
		print ""


class md3Shader:
	name = ""
	index = 0
	
	binaryFormat = "<%dsi" % MD3_MAX_QPATH

	def __init__(self):
		self.name = ""
		self.index = 0

	def load (self, file):
		tmpData = file.read(struct.calcsize(self.binaryFormat))
		data = struct.unpack(self.binaryFormat, tmpData)
		self.name = asciiz(data[0])
		self.index = data[1]
		return self

	def save(self, file):
		tmpData[0] = self.name
		tmpData[1] = self.index
		data = struct.pack(self.binaryFormat, tmpData[0], tmpData[1])
		file.write(data)
		#print "wrote MD3 shader structure: ",data

	def dump (self):
		print "MD3 Shader"
		print "shader name: ", self.name
		print "shader index: ", self.index
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
	
	binaryFormat = "<4s%ds10i" % MD3_MAX_QPATH  #1 int, name, then 10 ints
	
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
		
	def load (self, file):
		#where are we in the file (for calculating real offsets)
		offset_begin = file.tell()
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
		
		#load the tri info
		file.seek(offset_begin+self.ofsTriangles, 0)
		for i in range(0, self.numTriangles):
			self.triangles.append(md3Triangle())
			self.triangles[i].load(file)
			#self.triangles[i].dump()
		
		#load the shader info
		file.seek(offset_begin+self.ofsShaders, 0)
		for i in range(0, self.numShaders):
			self.shaders.append(md3Shader())
			self.shaders[i].load(file)
			self.shaders[i].dump()
			
		#load the uv info
		file.seek(offset_begin+self.ofsUV, 0)
		for i in range(0, self.numVerts):
			self.uv.append(md3TexCoord())
			self.uv[i].load(file)
			#self.uv[i].dump()
			
		#load the verts info
		file.seek(offset_begin+self.ofsVerts, 0)
		for i in range(0,self.numFrames):
			for j in range(0, self.numVerts):
				self.verts.append(md3Vert())
				#i*self.numVerts+j=where in the surface vertex list the vert position for this frame is
				self.verts[(i*self.numVerts)+j].load(file)
				#self.verts[j].dump()
			
		#go to the end of this structure
		file.seek(offset_begin+self.ofsEnd, 0)
			
		return self
	
	def save(self, file):
		tmpData = [0]*12
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

		#write the tri data
		for i in range(0,self.numTriangles):
			self.triangles[i].save(file)

		#save the shader coordinates
		for i in range(0, self.numShaders):
			self.shaders[i].save(file)

		#save the uv info
		for i in range(0, self.num_uv):
			self.uv[i].save(file)

		#save the verts
		for i in range(0, self.numVerts):
			self.verts[i].save(file)

	def dump (self):
		print "MD3 Surface"
		print "Ident: ", self.ident
		print "Name: ", self.name
		print "Flags: ", self.flags
		print "Number of Frames: ", self.numFrames
		print "Number of Shaders: ", self.numShaders
		print "Number of Verts: ", self.numVerts
		print "Number of Triangles: ", self.numTriangles
		print "Offset to Triangles: ",self.ofsTriangles
		print "Offset to Shaders: ",self.ofsShaders
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
		self.origin = vert()
		self.axis = [vert()]*3
		
	def load(self, file):
		tmpData = file.read(struct.calcsize(self.binaryFormat))
		data = struct.unpack(self.binaryFormat, tmpData)
		self.name = asciiz(data[0])
		self.origin.x = data[1]
		self.origin.y = data[2]
		self.origin.z = data[3]
		self.axis[0].x = data[4]
		self.axis[0].y = data[5]
		self.axis[0].z = data[6]
		self.axis[1].x = data[7]
		self.axis[1].y = data[8]
		self.axis[1].z = data[9]
		self.axis[2].x = data[10]
		self.axis[2].y = data[11]
		self.axis[2].z = data[12]
		return self
		
	def save(self, file):
		tmpData = [0]*13
		tmpData[0] = self.name
		tmpData[1] = float(self.origin.x)
		tmpData[2] = float(self.origin.y)
		tmpData[3] = float(self.origin.z)
		tmpData[4] = float(self.axis[0].x)
		tmpData[5] = float(self.axis[0].y)
		tmpData[6] = float(self.axis[0].z)
		tmpData[7] = float(self.axis[1].x)
		tmpData[8] = float(self.axis[1].y)
		tmpData[9] = float(self.axis[1].z)
		tmpData[10] = float(self.axis[2].x)
		tmpData[11] = float(self.axis[2].y)
		tmpData[12] = float(self.axis[2].z)
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
	min_bounds = 0
	max_bounds = 0
	local_origin = 0
	radius = 0.0
	name = ""
	
	binaryFormat="<3f3f3ff16s"
	
	def __init__(self):
		self.min_bounds = vert()
		self.max_bounds = vert()
		self.local_origin = vert()
		self.radius = 0.0
		self.name = ""

	def load (self, file):
		tmpData = file.read(struct.calcsize(self.binaryFormat))
		data = struct.unpack(self.binaryFormat, tmpData)
		self.min_bounds.x = data[0]
		self.min_bounds.y = data[1]
		self.min_bounds.z = data[2]
		self.max_bounds.x = data[3]
		self.max_bounds.y = data[4]
		self.max_bounds.z = data[5]
		self.local_origin.x = data[6]
		self.local_origin.y = data[7]
		self.local_origin.z = data[8]
		self.radius = data[9]
		self.name = asciiz(data[10])
		return self

	def save(self, file):
		tmpData = [0]*11
		tmpData[0] = self.min_bounds.x
		tmpData[1] = self.min_bounds.y
		tmpData[2] = self.min_bounds.z
		tmpData[3] = self.max_bounds.x
		tmpData[4] = self.max_bounds.y
		tmpData[5] = self.max_bounds.z
		tmpData[6] = self.local_origin.x
		tmpData[7] = self.local_origin.y
		tmpData[8] = self.local_origin.z
		tmpData[9] = self.radius
		tmpData[10] = self.name
		data = struct.pack(self.binaryFormat, tmpData[0],tmpData[1],tmpData[2],tmpData[3],tmpData[4],tmpData[5],tmpData[6],tmpData[7], tmpData[8], tmpData[9], tmpData[10])
		file.write(data)
		#print "wrote MD3 frame structure: ",data

	def dump(self):
		print "MD3 Frame"
		print "Min Bounds: ", self.min_bounds
		print "Max Bounds: ", self.max_bounds
		print "Local Origin: ", self.local_origin
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
	numSkins = 0		#don't think this is used, but here anyways
	ofsFrames = 0
	ofsTags = 0
	ofsSurfaces = 0
	ofsEnd = 0
	frames = []
	tags = []
	surfaces = []

	binaryFormat="<4si%ds9i" % MD3_MAX_QPATH  #little-endian (<), 17 integers (17i)

	def __init__(self):
		self.ident=0
		self.version=0
		self.name=""
		self.flags=0
		self.numFrames=0
		self.numTags=0
		self.numSurfaces=0
		self.numSkins=0
		self.ofsFrames=0
		self.ofsTags=0
		self.ofsSurfaces=0
		self.ofsEnd=0
		self.frames=[]
		self.tags=[]
		self.surfaces=[]


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
		for i in range(0, self.numTags):
			self.tags.append(md3Tag())
			self.tags[i].load(file)
			self.tags[i].dump()
		
		# load the surface info
		file.seek(self.ofsSurfaces, 0)
		for i in range(0, self.numSurfaces):
			self.surfaces.append(md3Surface())
			self.surfaces[i].load(file)
			self.surfaces[i].dump()
		return self

	def save(self, file):
		pass

	def dump (self):
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


def animateModel(surface, mesh_obj):
	# animate the verts through keyframe animation
	mesh = mesh_obj.getData()
	
	for i in range(0, surface.numFrames):
		# update the vertices
		for j in range(0,surface.numVerts):
			# i*sufrace.numVerts+j=where in the surface vertex list the vert position for this frame is
			x = surface.verts[(i * surface.numVerts) + j].xyz[0] * (MD3_XYZ_SCALE) #*g_scale.val
			y = surface.verts[(i * surface.numVerts) + j].xyz[1] * (MD3_XYZ_SCALE) #*g_scale.val
			z = surface.verts[(i * surface.numVerts) + j].xyz[2] * (MD3_XYZ_SCALE) #*g_scale.val

			# put the vertex in the right spot
			mesh.verts[j].co[0] = y
			mesh.verts[j].co[1] =-x
			mesh.verts[j].co[2] = z

		mesh.update()
		NMesh.PutRaw(mesh, mesh_obj.name)
		# absolute works too, but I want to get these into NLA actions
		# mesh.insertKey(i, "relative")
		
		# absolute keys, need to figure out how to get them working around the 100 frame limitation
		mesh.insertKey(i, "absolute")
		
		#hack to evenly space out the vertex keyframes on the IPO chart	
		#it does-Happy Dance!
		if i == 1:
			#after an IPO curve is created, make it a strait line so it 
			#doesn't peak out inserted frames position at 100
			#get the IPO for the model, it's ugly, but it works
			ob = Blender.Ipo.Get("KeyIpo")
			
			#get the curve for the IPO, again ugly
			ipos = ob.getCurves()
			
			#make the first (and only) curve extrapolated
			for this_ipo in ipos:
				this_ipo.setExtrapolation("Extrapolation")
				
				#recalculate it
				this_ipo.Recalc()

		#not really necissary, but I like playing with the frame counter
		Blender.Set("curframe", i)

def loadShaders(md3, shader_name):
	pass
	
def loadModel(filename):
	# read the file in
	file = open(filename,"rb")
	md3 = md3Object()
	md3.load(file)
	md3.dump()
	file.close()
	
	for surface in md3.surfaces:
		# create a new mesh
		mesh = NMesh.New(surface.name)
		uv_coord = []
		uv_list = []

		# make the verts
		for i in range (0, surface.numVerts):
			x = surface.verts[i].xyz[0] * (MD3_XYZ_SCALE) #*g_scale.val
			y = surface.verts[i].xyz[1] * (MD3_XYZ_SCALE) #*g_scale.val
			z = surface.verts[i].xyz[2] * (MD3_XYZ_SCALE) #*g_scale.val
			vertex = NMesh.Vert(y, -x, z)
			mesh.verts.append(vertex)
	
		# mke the UV list
		mesh.hasFaceUV(1)  #turn on face UV coordinates for this mesh
		for tex_coord in surface.uv:
			u = tex_coord.u
			v = tex_coord.v
			#for some reason quake3 texture maps are upside down, flip that
			uv_coord = (u, 1-v)
			uv_list.append(uv_coord)
	
		# make the faces
		for triangle in surface.triangles:
			face = NMesh.Face()
			#draw the triangles in reverse order so they show up
			face.v.append(mesh.verts[triangle.indexes[0]])
			face.v.append(mesh.verts[triangle.indexes[2]])
			face.v.append(mesh.verts[triangle.indexes[1]])
			#append the list of UV
			#ditto in reverse order with the texture verts
			face.uv.append(uv_list[triangle.indexes[0]])
			face.uv.append(uv_list[triangle.indexes[2]])
			face.uv.append(uv_list[triangle.indexes[1]])
	
			mesh.faces.append(face)
	
		mesh_obj = NMesh.PutRaw(mesh)
		mesh_obj.name = surface.name
	
		animateModel(surface, mesh_obj)
	
	# locate the Object containing the mesh at the cursor location
	cursor_pos = Blender.Window.GetCursorPos()
	mesh_obj.setLocation(float(cursor_pos[0]), float(cursor_pos[1]), float(cursor_pos[2]))
	
Blender.Window.FileSelector(loadModel, 'Import Quake3 MD3')