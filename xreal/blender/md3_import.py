#!BPY

""" Registration info for Blender menus:
Name: 'Quake3 (.md3)'
Blender: 237
Group: 'Import'
Tip: 'Import from Quake3 file format. (.md3)'
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
MD3_MAX_QPATH=64
MD3_MAX_TAGS=16
MD3_MAX_SURFACES=32
MD3_MAX_FRAMES=1024
MD3_MAX_SHADERS=256
MD3_MAX_VERTICES=4096
MD3_MAX_TRIANGLES=8192
MD3_XYZ_SCALE=(1.0/64.0)


class vert:
	x=0.0
	y=0.0
	z=0.0
	
	def __init__(self):
		self.x=0.0
		self.y=0.0
		self.z=0.0

class md3_vert:
	co=[]
	normal=0
	binary_format="<3hh"
	
	def __init__(self):
		self.co=[0]*3
		self.normal=0
	
	#copied from PhaethonH <phaethon@linux.ucla.edu> md3.py
	def decode (self, latlng):
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
		lat *= math.pi/128;
		lng *= math.pi/128;
		x = math.cos(lat) * math.sin(lng)
		y = math.sin(lat) * math.sin(lng)
		z =                 math.cos(lng)
		retval = [ x, y, z ]
		return retval
	
	#copied from PhaethonH <phaethon@linux.ucla.edu> md3.py
	def encode (self, normal):
		#Encode a normal vector into a 16-bit latitude-longitude value.
		x, y, z = normal
		lng = math.acos(z)
		lat = math.acos(x / math.sin(lng))
		retval = ((lat & 0xFF) << 8) | (lng & 0xFF)
		return retval
	
	def load(self,file):
		temp_data=file.read(struct.calcsize(self.binary_format))
		data=struct.unpack(self.binary_format, temp_data)
		self.co[0]=data[0]
		self.co[1]=data[1]
		self.co[2]=data[2]
		self.normal=data[3]
		return self
		
	def save(self, file):
		temp_data=[0]*4
		temp_data[0]=self.co[0]
		temp_data[1]=self.co[1]
		temp_data[2]=self.co[2]
		temp_data[3]=self.normal
		data=struct.pack(self.binary_format, temp_data[0], temp_data[1], temp_data[2], temp_data[3])
		file.write(data)
		#print "Wrote MD3 Vertex: ", data
	
	def dump(self):
		print "MD3 Vertex Structure"
		print "X: ", self.co[0]
		print "Y: ", self.co[1]
		print "Z: ", self.co[2]
		print "Normal: ", self.normal
		print ""

class md3_tex_coord:
	u=0.0
	v=0.0

	binary_format="<2f"

	def __init__(self):
		self.u=0.0
		self.v=0.0

	def load (self, file):
		temp_data=file.read(struct.calcsize(self.binary_format))
		data=struct.unpack(self.binary_format, temp_data)
		self.u=data[0]
		self.v=data[1]
		return self

	def save(self, file):
		temp_data=[0]*2
		temp_data[0]=self.u
		temp_data[1]=self.v
		data=struct.pack(self.binary_format, temp_data[0], temp_data[1])
		file.write(data)
		#print "wrote MD3 texture coordinate structure: ",data

	def dump (self):
		print "MD3 Texture Coordinate Structure"
		print "texture coordinate u: ",self.u
		print "texture coordinate v: ",self.v
		print ""
		

class md3_triangle:
	vertex_index=[]

	binary_format="<3i"

	def __init__(self):
		self.vertex_index = [ 0, 0, 0 ]

	def load (self, file):
		temp_data=file.read(struct.calcsize(self.binary_format))
		data=struct.unpack(self.binary_format, temp_data)
		self.vertex_index[0]=data[0]
		self.vertex_index[1]=data[1]
		self.vertex_index[2]=data[2]
		return self

	def save(self, file):
		temp_data=[0]*3
		temp_data[0]=self.vertex_index[0]
		temp_data[1]=self.vertex_index[1]
		temp_data[2]=self.vertex_index[2]
		data=struct.pack(self.binary_format,temp_data[0],temp_data[1],temp_data[2])
		file.write(data)
		#print "wrote MD3 face structure: ",data

	def dump (self):
		print "MD3 Triangle Structure"
		print "vertex index: ", self.vertex_index[0]
		print "vertex index: ", self.vertex_index[1]
		print "vertex index: ", self.vertex_index[2]
		print ""


class md3_shader:
	name=""
	index=0
	
	binary_format="<%dsi" % MD3_MAX_QPATH

	def __init__(self):
		self.name=""
		self.index=0

	def load (self, file):
		temp_data=file.read(struct.calcsize(self.binary_format))
		data=struct.unpack(self.binary_format, temp_data)
		self.name=asciiz(data[0])
		self.index=data[1]
		return self

	def save(self, file):
		temp_data[0]=self.name
		temp_data[1]=self.index
		data=struct.pack(self.binary_format, temp_data[0], temp_data[1])
		file.write(data)
		#print "wrote MD3 shader structure: ",data

	def dump (self):
		print "MD3 Shader"
		print "shader name: ",self.name
		print "shader index: ",self.index
		print ""


class md3_surface:
	ident=""
	name=""
	flags=0
	num_frames=0
	num_shaders=0
	num_verts=0
	num_tris=0
	offset_tris=0
	offset_shaders=0
	offset_uv=0
	offset_verts=0
	offset_end=0
	shaders=[]
	tris=[]
	uv=[]
	verts=[]
	
	binary_format="<4s%ds10i" % MD3_MAX_QPATH  #1 int, name, then 10 ints
	
	def __init__(self):
		self.ident=""
		self.name=""
		self.flags=0
		self.num_frames=0
		self.num_shaders=0
		self.num_verts=0
		self.num_tris=0
		self.offset_tris=0
		self.offset_shaders=0
		self.offset_uv=0
		self.offset_verts=0
		self.offset_end
		self.shaders=[]
		self.tris=[]
		self.uv=[]
		self.verts=[]
		
	def load (self, file):
		#where are we in the file (for calculating real offsets)
		offset_begin=file.tell()
		temp_data=file.read(struct.calcsize(self.binary_format))
		data=struct.unpack(self.binary_format, temp_data)
		self.ident=data[0]
		self.name=asciiz(data[1])
		self.flags=data[2]
		self.num_frames=data[3]
		self.num_shaders=data[4]
		self.num_verts=data[5]
		self.num_tris=data[6]
		self.offset_tris=data[7]
		self.offset_shaders=data[8]
		self.offset_uv=data[9]
		self.offset_verts=data[10]
		self.offset_end=data[11]
		
		#load the tri info
		file.seek(offset_begin+self.offset_tris,0)
		for i in range(0, self.num_tris):
			self.tris.append(md3_triangle())
			self.tris[i].load(file)
			#self.tris[i].dump()
		
		#load the shader info
		file.seek(offset_begin+self.offset_shaders,0)
		for i in range(0, self.num_shaders):
			self.shaders.append(md3_shader())
			self.shaders[i].load(file)
			#self.shaders[i].dump()
			
		#load the uv info
		file.seek(offset_begin+self.offset_uv,0)
		for i in range(0, self.num_verts):
			self.uv.append(md3_tex_coord())
			self.uv[i].load(file)
			#self.uv[i].dump()
			
		#load the verts info
		file.seek(offset_begin+self.offset_verts,0)
		for i in range(0,self.num_frames):
			for j in range(0, self.num_verts):
				self.verts.append(md3_vert())
				#i*self.num_verts+j=where in the surface vertex list the vert position for this frame is
				self.verts[(i*self.num_verts)+j].load(file)
				#self.verts[j].dump()
			
		#go to the end of this structure
		file.seek(offset_begin+self.offset_end, 0)
			
		return self
	
	def save(self, file):
		temp_data=[0]*12
		temp_data[0]=self.ident
		temp_data[1]=self.name
		temp_data[2]=self.flags
		temp_data[3]=self.num_frames
		temp_data[4]=self.num_shaders
		temp_data[5]=self.num_verts
		temp_data[6]=self.num_tris
		temp_data[7]=self.offset_tris
		temp_data[8]=self.offset_shaders
		temp_data[9]=self.offset_uv
		temp_data[10]=self.offset_verts
		temp_data[11]=self.offset_end
		data=struct.pack(self.binary_format, temp_data[0],temp_data[1],temp_data[2],temp_data[3],temp_data[4],temp_data[5],temp_data[6],temp_data[7],temp_data[8],temp_data[9],temp_data[10],temp_data[11])
		file.write(data)

		#write the tri data
		for i in range(0,self.num_tris):
			self.tris[i].save(file)

		#save the shader coordinates
		for i in range(0, self.num_shaders):
			self.shaders[i].save(file)

		#save the uv info
		for i in range(0, self.num_uv):
			self.uv[i].save(file)

		#save the verts
		for i in range(0, self.num_verts):
			self.verts[i].save(file)

	def dump (self):
		print "MD3 Surface"
		print "Ident: ", self.ident
		print "Name: ", self.name
		print "Flags: ", self.flags
		print "Number of Frames: ", self.num_frames
		print "Number of Shaders: ", self.num_shaders
		print "Number of Verts: ", self.num_verts
		print "Number of Triangles: ", self.num_tris
		print "Offset to Triangles: ",self.offset_tris
		print "Offset to Shaders: ",self.offset_shaders
		print "Offset to UV: ", self.offset_uv
		print "Offset to Verts: ", self.offset_verts
		print "Offset to end: ", self.offset_end
		print ""
		

class md3_tag:
	name=""
	origin=[]
	axis=[]
	
	binary_format="<%ds3f9f" % MD3_MAX_QPATH
	
	def __init__(self):
		self.name=""
		self.origin=vert()
		self.axis=[vert()]*3
		
	def load(self, file):
		temp_data=file.read(struct.calcsize(self.binary_format))
		data=struct.unpack(self.binary_format, temp_data)
		self.name=asciiz(data[0])
		self.origin.x=data[1]
		self.origin.y=data[2]
		self.origin.z=data[3]
		self.axis[0].x=data[4]
		self.axis[0].y=data[5]
		self.axis[0].z=data[6]
		self.axis[1].x=data[7]
		self.axis[1].y=data[8]
		self.axis[1].z=data[9]
		self.axis[2].x=data[10]
		self.axis[2].y=data[11]
		self.axis[2].z=data[12]
		return self
		
	def save(self, file):
		temp_data=[0]*13
		temp_data[0]=self.name
		temp_data[1]=float(self.origin.x)
		temp_data[2]=float(self.origin.y)
		temp_data[3]=float(self.origin.z)
		temp_data[4]=float(self.axis[0].x)
		temp_data[5]=float(self.axis[0].y)
		temp_data[6]=float(self.axis[0].z)
		temp_data[7]=float(self.axis[1].x)
		temp_data[8]=float(self.axis[1].y)
		temp_data[9]=float(self.axis[1].z)
		temp_data[10]=float(self.axis[2].x)
		temp_data[11]=float(self.axis[2].y)
		temp_data[12]=float(self.axis[2].z)
		data=struct.pack(self.binary_format, temp_data[0],temp_data[1],temp_data[2],temp_data[3],temp_data[4],temp_data[5],temp_data[6], temp_data[7], temp_data[8], temp_data[9], temp_data[10], temp_data[11], temp_data[12])
		file.write(data)
		#print "wrote MD3 Tag structure: ",data
		
	def dump(self):
		print "MD3 Tag"
		print "Name: ", self.name
		print "Origin: ",self.origin
		print "Axis: ",self.axis
		print ""
	
class md3_frame:
	min_bounds=0
	max_bounds=0
	local_origin=0
	radius=0.0
	name=""
	
	binary_format="<3f3f3ff16s"
	
	def __init__(self):
		self.min_bounds=vert()
		self.max_bounds=vert()
		self.local_origin=vert()
		self.radius=0.0
		self.name=""

	def load (self, file):
		temp_data=file.read(struct.calcsize(self.binary_format))
		data=struct.unpack(self.binary_format, temp_data)
		self.min_bounds.x=data[0]
		self.min_bounds.y=data[1]
		self.min_bounds.z=data[2]
		self.max_bounds.x=data[3]
		self.max_bounds.y=data[4]
		self.max_bounds.z=data[5]
		self.local_origin.x=data[6]
		self.local_origin.y=data[7]
		self.local_origin.z=data[8]
		self.radius=data[9]
		self.name=asciiz(data[10])
		return self

	def save(self, file):
		temp_data=[0]*11
		temp_data[0]=self.min_bounds.x
		temp_data[1]=self.min_bounds.y
		temp_data[2]=self.min_bounds.z
		temp_data[3]=self.max_bounds.x
		temp_data[4]=self.max_bounds.y
		temp_data[5]=self.max_bounds.z
		temp_data[6]=self.local_origin.x
		temp_data[7]=self.local_origin.y
		temp_data[8]=self.local_origin.z
		temp_data[9]=self.radius
		temp_data[10]=self.name
		data=struct.pack(self.binary_format, temp_data[0],temp_data[1],temp_data[2],temp_data[3],temp_data[4],temp_data[5],temp_data[6],temp_data[7], temp_data[8], temp_data[9], temp_data[10])
		file.write(data)
		#print "wrote MD3 frame structure: ",data

	def dump (self):
		print "MD3 Frame"
		print "Min Bounds: ",self.min_bounds
		print "Max Bounds: ",self.max_bounds
		print "Local Origin: ",self.local_origin
		print "Radius: ",self.radius
		print "Name: ",self.name
		print ""

class md3_obj:
	#Header Structure
	ident=""			#iThis is used to identify the file (must be IDP3)
	version=0			#iThe version number of the file (Must be 15)
	name=""
	flags=0
	num_frames=0
	num_tags=0
	num_surfaces=0
	num_skins=0		#don't think this is used, but here anyways
	offset_frames=0
	offset_tags=0
	offset_surfaces=0
	offset_end=0
	frames=[]
	tags=[]
	surfaces=[]

	binary_format="<4si%ds9i" % MD3_MAX_QPATH  #little-endian (<), 17 integers (17i)

	def __init__ (self):
		self.ident=0
		self.version=0
		self.name=""
		self.flags=0
		self.num_frames=0
		self.num_tags=0
		self.num_surfaces=0
		self.num_skins=0
		self.offset_frames=0
		self.offset_tags=0
		self.offset_surfaces=0
		self.offset_end=0
		self.frames=[]
		self.tags=[]
		self.surfaces=[]


	def load (self, file):
		temp_data = file.read(struct.calcsize(self.binary_format))
		data = struct.unpack(self.binary_format, temp_data)

		self.ident=data[0]
		self.version=data[1]

		if (self.ident!="IDP3" or self.version!=15):
			print "Not a valid MD3 file"
			print "Ident=", self.ident
			print "Version=",self.version
			Exit()

		self.name=asciiz(data[2])
		self.flags=data[3]
		self.num_frames=data[4]
		self.num_tags=data[5]
		self.num_surfaces=data[6]
		self.num_skins=data[7]
		self.offset_frames=data[8]
		self.offset_tags=data[9]
		self.offset_surfaces=data[10]
		self.offset_end=data[11]

		#load the frame info
		file.seek(self.offset_frames,0)
		for i in range(0, self.num_frames):
			self.frames.append(md3_frame())
			self.frames[i].load(file)
			#self.frames[i].dump()
		
		#load the tags info
		file.seek(self.offset_tags,0)
		for i in range(0, self.num_tags):
			self.tags.append(md3_tag())
			self.tags[i].load(file)
			#self.tags[i].dump()
		
		#load the surface info
		file.seek(self.offset_surfaces,0)
		for i in range(0, self.num_surfaces):
			self.surfaces.append(md3_surface())
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
		print "Number of Frames: ",self.num_frames
		print "Number of Tags: ", self.num_tags
		print "Number of Surfaces: ", self.num_surfaces
		print "Number of Skins: ", self.num_skins
		print "Offset Frames: ", self.offset_frames
		print "Offset Tags: ", self.offset_tags
		print "Offset Surfaces: ", self.offset_surfaces
		print "Offset end: ", self.offset_end
		print ""

######################################################
# Import functions
######################################################
def animate_md3(surface, mesh_obj):
	######### Animate the verts through keyframe animation
	mesh=mesh_obj.getData()
	
	for i in range(0, surface.num_frames):
		#update the vertices
		for j in range(0,surface.num_verts):
			#i*sufrace.num_verts+j=where in the surface vertex list the vert position for this frame is
			x=surface.verts[(i*surface.num_verts)+j].co[0]*(MD3_XYZ_SCALE) #*g_scale.val
			y=surface.verts[(i*surface.num_verts)+j].co[1]*(MD3_XYZ_SCALE) #*g_scale.val
			z=surface.verts[(i*surface.num_verts)+j].co[2]*(MD3_XYZ_SCALE) #*g_scale.val

			#put the vertex in the right spot
			mesh.verts[j].co[0]=x
			mesh.verts[j].co[1]=y
			mesh.verts[j].co[2]=z

		mesh.update()
		NMesh.PutRaw(mesh, mesh_obj.name)
		#absolute works too, but I want to get these into NLA actions
		mesh.insertKey(i,"relative")

def load_shaders(md3, shader_name):
	pass
	
def load_md3 (md3_filename):
	#read the file in
	file=open(md3_filename,"rb")
	md3=md3_obj()
	md3.load(file)
	md3.dump()
	file.close()
	
	for surface in md3.surfaces:
		######## Creates a new mesh
		mesh = NMesh.New(surface.name)
		uv_coord=[]
		uv_list=[]

		######### Make the verts
		for i in range (0,surface.num_verts):
			x = surface.verts[i].co[0] * (MD3_XYZ_SCALE) #*g_scale.val
			y = surface.verts[i].co[1] * (MD3_XYZ_SCALE) #*g_scale.val
			z = surface.verts[i].co[2] * (MD3_XYZ_SCALE) #*g_scale.val
			vertex=NMesh.Vert(x, y, z)
			mesh.verts.append(vertex)
	
		######## Make the UV list
		mesh.hasFaceUV(1)  #turn on face UV coordinates for this mesh
		for tex_coord in surface.uv:
			u=tex_coord.u
			v=tex_coord.v
			#for some reason quake3 texture maps are upside down, flip that
			uv_coord=(u,1-v)
			uv_list.append(uv_coord)
	
		######### Make the faces
		for triangle in surface.tris:
			face = NMesh.Face()
			#draw the triangles in reverse order so they show up
			face.v.append(mesh.verts[triangle.vertex_index[0]])
			face.v.append(mesh.verts[triangle.vertex_index[2]])
			face.v.append(mesh.verts[triangle.vertex_index[1]])
			#append the list of UV
			#ditto in reverse order with the texture verts
			face.uv.append(uv_list[triangle.vertex_index[0]])
			face.uv.append(uv_list[triangle.vertex_index[2]])
			face.uv.append(uv_list[triangle.vertex_index[1]])
	
			mesh.faces.append(face)
	
		mesh_obj=NMesh.PutRaw(mesh)
		mesh_obj.name=surface.name
	
		animate_md3(surface, mesh_obj)
	
	#locate the Object containing the mesh at the cursor location
	cursor_pos = Blender.Window.GetCursorPos()
	mesh_obj.setLocation(float(cursor_pos[0]),float(cursor_pos[1]),float(cursor_pos[2]))
	
Blender.Window.FileSelector(load_md3, 'Import Quake3 MD3')