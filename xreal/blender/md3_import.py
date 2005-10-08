#!BPY

"""
Name: 'Quake3 (.md3)...'
Blender: 237
Group: 'Import'
Tooltip: 'Import from Quake3 file format. (.md3)'
"""

__author__ = "PhaethonH, Bob Holcomb, Robert (Tr3B) Beckebans"
__url__ = ("http://xreal.sourceforge.net")
__version__ = "0.4 2005-10-7"

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
from Blender import NMesh, Object, Material, Image, Texture

import sys, struct, string
from types import *

import os
from os import path

import md3
from md3 import *

# strips the slashes from the back of a string
def stripPath(path):
	for CH in range(len(path), 0, -1):
		if path[CH-1] == "/" or path[CH-1] == "\\":
			path = path[CH:]
			break
	return path

# strips file type extension
def stripExtension(name):
	return name[:name.find('.')]

def animateMesh(surface, meshObject):
	# animate the verts through keyframe animation
	mesh = meshObject.getData()
	
	for i in range(0, surface.numFrames):
		# update the vertices
		for j in range(0,surface.numVerts):
			# i*sufrace.numVerts+j=where in the surface vertex list the vert position for this frame is
			x = surface.verts[(i * surface.numVerts) + j].xyz[0] * (MD3_XYZ_SCALE) * (MD3_BLENDER_SCALE)
			y = surface.verts[(i * surface.numVerts) + j].xyz[1] * (MD3_XYZ_SCALE) * (MD3_BLENDER_SCALE)
			z = surface.verts[(i * surface.numVerts) + j].xyz[2] * (MD3_XYZ_SCALE) * (MD3_BLENDER_SCALE)

			# put the vertex in the right spot
			mesh.verts[j].co[0] = x
			mesh.verts[j].co[1] = y
			mesh.verts[j].co[2] = z

		mesh.update()
		NMesh.PutRaw(mesh, meshObject.name)
		# absolute works too, but I want to get these into NLA actions
		# mesh.insertKey(i, "relative")
		
		# absolute keys, need to figure out how to get them working around the 100 frame limitation
		mesh.insertKey(i, "absolute")
		
		# hack to evenly space out the vertex keyframes on the IPO chart
		if i == 1:
			# after an IPO curve is created, make it a strait line so it 
			# doesn't peak out inserted frames position at 100
			# get the IPO for the model, it's ugly, but it works
			ob = Blender.Ipo.Get("KeyIpo")
			
			# get the curve for the IPO, again ugly
			ipos = ob.getCurves()
			
			# make the first (and only) curve extrapolated
			for this_ipo in ipos:
				this_ipo.setExtrapolation("Extrapolation")
				
				# recalculate it
				this_ipo.Recalc()

		# not really necissary, but I like playing with the frame counter
		Blender.Set("curframe", i)

def skinMesh(surface, meshObject):
	mesh = meshObject.getData()
	
	for i in range(0, surface.numShaders):
		# create new material
		matName = stripExtension(stripPath(surface.shaders[i].name))
		mat = Material.New(matName)
		
		# create new texture
		texture = Texture.New(matName)
		texture.setType('Image')
		
		# NOTE: change this to your installation directory
		try:
			# try .tga by default
			imageName = stripExtension('/opt/xreal/base/' + surface.shaders[i].name) + '.tga'
			image = Image.Load(imageName)
			
			# assign image to texture
			texture.image = image
		except:
			try:
				imageName = stripExtension(imageName) + '.jpg'
				image = Image.Load(imageName)
				
				# assign image to texture
				texture.image = image
			except:
				print "unable to load image ", imageName
			
		#print "Image from", image.getFilename()
		#print "loaded to obj", image.getName()
		
		# texture to material
		mat.setTexture(0, texture, Texture.TexCo.UV, Texture.MapTo.COL)

		# and append it to the mesh's list of mats
   		mesh.materials.append(mat)
		
		mesh.update()
	
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
			x = surface.verts[i].xyz[0] * (MD3_XYZ_SCALE) * (MD3_BLENDER_SCALE)
			y = surface.verts[i].xyz[1] * (MD3_XYZ_SCALE) * (MD3_BLENDER_SCALE)
			z = surface.verts[i].xyz[2] * (MD3_XYZ_SCALE) * (MD3_BLENDER_SCALE)
			vertex = NMesh.Vert(x, y, z)
			mesh.verts.append(vertex)
	
		# make the UV list
		mesh.hasFaceUV(1)  #turn on face UV coordinates for this mesh
		for tex_coord in surface.uv:
			u = tex_coord.u
			v = tex_coord.v
			# for some reason quake3 texture maps are upside down, flip that
			uv_coord = (u, 1-v)
			uv_list.append(uv_coord)
	
		# make the faces
		for triangle in surface.triangles:
			face = NMesh.Face()
			# draw the triangles in reverse order so they show up
			face.v.append(mesh.verts[triangle.indexes[0]])
			face.v.append(mesh.verts[triangle.indexes[2]])
			face.v.append(mesh.verts[triangle.indexes[1]])
			# append the list of UV
			# ditto in reverse order with the texture verts
			face.uv.append(uv_list[triangle.indexes[0]])
			face.uv.append(uv_list[triangle.indexes[2]])
			face.uv.append(uv_list[triangle.indexes[1]])
	
			mesh.faces.append(face)
	
		meshObject = NMesh.PutRaw(mesh)
		meshObject.name = surface.name
	
		animateMesh(surface, meshObject)
		skinMesh(surface, meshObject)
	
	# locate the Object containing the mesh at the cursor location
	cursor_pos = Blender.Window.GetCursorPos()
	meshObject.setLocation(float(cursor_pos[0]), float(cursor_pos[1]), float(cursor_pos[2]))
	
Blender.Window.FileSelector(loadModel, 'Import Quake3 MD3')