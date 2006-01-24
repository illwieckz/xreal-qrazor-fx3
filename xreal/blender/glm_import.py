#!BPY

"""
Name: 'Ghoul2 Mesh (.glm)...'
Blender: 240
Group: 'Import'
Tooltip: 'Import from Ghoul2 file format. (.glm)'
"""

__author__ = "PhaethonH, Bob Holcomb, Robert (Tr3B) Beckebans, Brad (Wudan) Newbold"
__url__ = ("http://forums.mt-wudan.com/viewforum.php?f=10")
__version__ = "0.4 2005-10-7"

__bpydoc__ = """\
This script imports a Ghoul2 mesh file (GLM), and textures 
into Blender for editing.  Loader is based on the
md3 blender loader by Robert (Tr3B) Beckebans, which
is based on the
md3 blender loader by PhaethonH <phaethon@linux.ucla.edu>

Supported:<br>
	Surfaces, and Materials.

Missing:<br>

Known issues:<br>
    None.

Notes:<br>
    TODO
"""

import sys, struct, string
from types import *

import os
from os import path

import Blender
from Blender import *

import glm
from glm import *

import q_math
from q_math import *

import q_shared
from q_shared import *

def loadModel(filename):
	tagct = 0
	# read the file in
	file = open(filename,"rb")
	glm = glmObject()
	glm.load(file)
	glm.dump()
	file.close()
	
	scene = Blender.Scene.getCurrent()

	for i in range(0, glm.numSurfaces ):
		glm.lods[0].surfaces[i].name = glm.surfHier[i].name
		glm.lods[0].surfaces[i].shader = glm.surfHier[i].shader
	
	for surface in glm.lods[0].surfaces:
		# create a new mesh
		if( surface.name[0] == '*' ):
			glm.lods[0].tags.append(glmTag())
			glm.lods[0].tags[tagct].fromSurf( surface.name, surface.verts[0].xyz, surface.verts[1].xyz, surface.verts[2].xyz)
			tagct += 1
		else:
			mesh = NMesh.New()
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
			mesh.hasVertexUV(1)  # turn on face UV coordinates for this mesh
			for tex_coord in surface.uv:
				u = tex_coord.u
				v = tex_coord.v
				uv = (u, v)
				uvList.append(uv)
		
			# make the faces
			for i in range(0, surface.numTriangles):
				face = NMesh.Face()
				
				face.v.append(mesh.verts[surface.triangles[i].ind[0]])
				face.v.append(mesh.verts[surface.triangles[i].ind[1]])
				face.v.append(mesh.verts[surface.triangles[i].ind[2]])
				face.uv.append(uvList[surface.triangles[i].ind[0]])
				face.uv.append(uvList[surface.triangles[i].ind[1]])
				face.uv.append(uvList[surface.triangles[i].ind[2]])
				mesh.faces.append(face)
		
			meshObject = NMesh.PutRaw(mesh)
			meshObject.name = ''+surface.name+''
	
			mesh = meshObject.getData()
			
			# create materials for surface
			# create new material if necessary
			matName = stripExtension(stripPath(surface.shader))
			
			try:
				mat = Material.Get(matName)
			except:
				print "creating new material", matName
				mat = Material.New(matName)
			
				# create new texture
				texture = Texture.New(matName)
				texture.setType('Image')
			
				# NOTE: change this to your installation directory
				imageName = stripExtension(GAMEDIR + surface.shader) + '.tga'
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
							
					print "Image from", imageName
					#print "loaded to obj", image.getName()
				
					# texture to material
					mat.setTexture(0, texture, Texture.TexCo.UV, Texture.MapTo.COL)
	
				# append material to the mesh's list of materials
				mesh.materials.append(mat)
				mesh.update()
			
			# locate the Object containing the mesh at the cursor location
			cursorPos = Blender.Window.GetCursorPos()
			meshObject.setLocation(float(cursorPos[0]), float(cursorPos[1]), float(cursorPos[2]))

	# create tags
	tags = []
	for tag in glm.lods[0].tags:
		blenderTag = Object.New("Empty", tag.name);
		tags.append(blenderTag)
		scene.link(blenderTag)
		forward = [tag.axis[0], tag.axis[1], tag.axis[2]]
		left = [tag.axis[3], tag.axis[4], tag.axis[5]]
		up = [tag.axis[6], tag.axis[7], tag.axis[8]]
		transform = MatrixSetupTransform(forward, left, up, tag.origin)
		matrix = Blender.Mathutils.Matrix(transform[0], transform[1], transform[2], transform[3])
		blenderTag.setMatrix(matrix)
		blenderTag.setLocation(tag.origin)

Blender.Window.FileSelector(loadModel, 'Import Ghoul2 GLM')