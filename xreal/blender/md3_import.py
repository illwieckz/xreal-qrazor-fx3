#!BPY

"""
Name: 'Quake3 (.md3)...'
Blender: 240
Group: 'Import'
Tooltip: 'Import from Quake3 file format. (.md3)'
"""

__author__ = "PhaethonH, Bob Holcomb, Robert (Tr3B) Beckebans"
__url__ = ("http://xreal.sourceforge.net")
__version__ = "0.5 2006-01-08"

__bpydoc__ = """\
This script imports a Quake 3 file (MD3), textures, 
and animations into Blender for editing. Loader is based on MD3 loader
from www.gametutorials.com-Thanks DigiBen! and the
md3 blender loader by PhaethonH <phaethon@linux.ucla.edu>

Supported:<br>
	Surfaces and Materials

Missing:<br>
    Animations

Known issues:<br>
    None

Notes:<br>
    TODO
"""

import sys, struct, string
from types import *

import os
from os import path

import Blender
from Blender import *

import md3
from md3 import *

import q_math
from q_math import *

#import q_shared
#from q_shared import *

def loadModel(filename):
	# read the file in
	file = open(filename,"rb")
	md3 = md3Object()
	md3.load(file)
	md3.dump()
	file.close()
	
	scene = Scene.getCurrent()
	
	for surface in md3.surfaces:
		# create a new mesh
		mesh = NMesh.New(surface.name)
		uv = []
		uvList = []

		# make the verts
		for i in range(0, surface.numVerts):
			x = surface.verts[i].xyz[0]
			y = surface.verts[i].xyz[1]
			z = surface.verts[i].xyz[2]
			vertex = NMesh.Vert(x, y, z)
			mesh.verts.append(vertex)
	
		# make the UV list
		mesh.hasFaceUV(1)  # turn on face UV coordinates for this mesh
		for tex_coord in surface.uv:
			u = tex_coord.u
			v = tex_coord.v
			uv = (u, v)
			uvList.append(uv)
	
		# make the faces
		for triangle in surface.triangles:
			face = NMesh.Face()
			
			face.v.append(mesh.verts[triangle.indexes[0]])
			face.v.append(mesh.verts[triangle.indexes[1]])
			face.v.append(mesh.verts[triangle.indexes[2]])
			face.uv.append(uvList[triangle.indexes[0]])
			face.uv.append(uvList[triangle.indexes[1]])
			face.uv.append(uvList[triangle.indexes[2]])
			mesh.faces.append(face)
	
		meshObject = NMesh.PutRaw(mesh)
		meshObject.name = surface.name
		
		"""
		# animate the verts through keyframe animation
		#mesh = meshObject.getData()
		
		#for i in range(0, surface.numFrames):
			
			# absolute works too, but I want to get these into NLA actions
			#mesh.insertKey(i + 1, "relative")
			
			# absolute keys, need to figure out how to get them working around the 100 frame limitation
			#mesh.insertKey(i + 1, "absolute")
			
			#key = mesh.getKey()
			#print "key: ", key
		
			#keyBlocks = key.getBlocks()
			#for keyBlock in keyBlocks:
			#keyBlock.name = 
			#print "keyblock: ", keyBlock.name
			
			# update the vertices
			for j in range(0, surface.numVerts):
				# i*sufrace.numVerts+j=where in the surface vertex list the vert position for this frame is
				x = surface.verts[(i * surface.numVerts) + j].xyz[0]
				y = surface.verts[(i * surface.numVerts) + j].xyz[1]
				z = surface.verts[(i * surface.numVerts) + j].xyz[2]
	
				# put the vertex in the right spot
				mesh.verts[j].co[0] = x
				mesh.verts[j].co[1] = y
				mesh.verts[j].co[2] = z
			
			mesh.update()
			
			#NMesh.PutRaw(mesh, meshObject.name)
		"""
		
		# create materials for surface
		for i in range(0, surface.numShaders):
			
			# create new material if necessary
			matName = stripExtension(stripPath(surface.shaders[i].name))
			
			try:
				mat = Material.Get(matName)
			except:
				print "creating new material", matName
				mat = Material.New(matName)
			
				# create new texture
				texture = Texture.New(matName)
				texture.setType('Image')
			
				# try .tga by default
				imageName = stripExtension(GAMEDIR + surface.shaders[i].name) + '.tga'
				try:
					image = Image.Load(imageName)
				
					texture.image = image
				except:
					try:
						imageName = stripExtension(imageName) + '.png'
						image = Image.Load(imageName)
					
						texture.image = image
					except:
						try:
							imageName = stripExtension(imageName) + '.jpg'
							image = Image.Load(imageName)
						
							texture.image = image
						except:
							print "unable to load image ", imageName
				
				# texture to material
				mat.setTexture(0, texture, Texture.TexCo.UV, Texture.MapTo.COL)
	
			# append material to the mesh's list of materials
			mesh.materials.append(mat)
			mesh.update()
	
	# create tags
	tags = []
	for i in range(0, md3.numTags):
		tag = md3.tags[i]
		
		# this should be an Empty object
		blenderTag = Object.New("Empty", tag.name);
		tags.append(blenderTag)
		scene.link(blenderTag)

	# FIXME this imports only the baseframe tags
	for i in range(0, 1): #surface.numFrames):
		#Blender.Set("curframe", i + 1)
		
		for j in range(0, md3.numTags):
			tag = md3.tags[i * md3.numTags + j]
			
			#tagName = tag.name + '_' + str(i + 1)
			#blenderTag = Object.New("Empty", tagName);
			#scene.link(blenderTag)
			blenderTag = tags[j]
		
			# Note: Quake3 uses left-hand geometry
			forward = [tag.axis[0], tag.axis[1], tag.axis[2]]
			left = [tag.axis[3], tag.axis[4], tag.axis[5]]
			up = [tag.axis[6], tag.axis[7], tag.axis[8]]
		
			transform = MatrixSetupTransform(forward, left, up, tag.origin)
			transform2 = Blender.Mathutils.Matrix(transform[0], transform[1], transform[2], transform[3])
			#rotation = Blender.Mathutils.Matrix(forward, left, up)
			blenderTag.setMatrix(transform2)
			
			blenderTag.setLocation(tag.origin)
	
	# locate the Object containing the mesh at the cursor location
	cursorPos = Blender.Window.GetCursorPos()
	meshObject.setLocation(float(cursorPos[0]), float(cursorPos[1]), float(cursorPos[2]))
	
	# not really necessary, but I like playing with the frame counter
	#Blender.Set("staframe", 1)
	#Blender.Set("curframe", md3.numFrames)
	#Blender.Set("endframe", md3.numFrames)
	
Blender.Window.FileSelector(loadModel, 'Import Quake3 MD3')