#!BPY

"""
Name: 'Ghoul2 (.glm)...'
Blender: 240
Group: 'Export'
Tooltip: 'Export to Ghoul2 file format. (.glm)'
"""
__author__ = "PhaethonH, Bob Holcomb, Robert (Tr3B) Beckebans, Wudan"
__url__ = ("http://forums.mt-wudan.com/viewforum.php?f=10")
__version__ = "0.4 2005-10-7"

__bpydoc__ = """\
This script exports a Ghoul2 mesh file (GLM).  Script is based on the
md3 blender script suite by Robert (Tr3B) Beckebans, which is based on the
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

import sys, struct, string, math
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


NUM_FRAMES = 1
EXPORT_ALL = 1   # export only selected objs, or all?

def applyTransform(vert, matrix):
	#vertCopy = Mathutils.CopyVec(vert)
	#vertCopy.resize4D()
	#return Mathutils.VecMultMat(vertCopy, matrix)
	return vert * matrix


def updateFrameBounds(v, f):
	for i in range(0, 3):
		f.mins[i] = min(v[i], f.mins[i])
	for i in range(0, 3):
		f.maxs[i] = max(v[i], f.maxs[i])


def updateFrameRadius(f):
	f.radius = RadiusFromBounds(f.mins, f.maxs)

def processSurfHier(glm):
	for i in range(0, glm.numSurfaces):
		glm.surfHier.append(glmSurfHier())
		glm.surfHier[i].name = glm.lods[0].surfaces[i].name
		if( glm.surfHier[i].name[0] == '*' ):
			glm.surfHier[i].flags = 1
		else:
			glm.surfHier[i].flags = 0
		glm.surfHier[i].shader = "[nomaterial]"
		glm.surfHier[i].number = 0
		if( i == 0 ):
			glm.surfHier[i].parentIndex = -1
			glm.surfHier[i].numChildren = glm.numSurfaces-1
			for j in range(i+1, glm.numSurfaces):
				glm.surfHier[i].childIndexes.append(glmBoneRef())
				glm.surfHier[i].childIndexes[i].index = j
		else:
			glm.surfHier[i].numChildren = 0
			glm.surfHier[i].parentIndex = 0
		glm.surfHier[i].dump()

def processSurface(blenderObject, glm, pathName, modelName):
	# because glm doesnt suppoort faceUVs like blender, we need to duplicate
	# any vertex that has multiple uv coords

	vertDict = {}
	indexDict = {} # maps a vertex index to the revised index after duplicating to account for uv
	vertList = [] # list of vertices ordered by revised index
	numVerts = 0
	uvList = [] # list of tex coords ordered by revised index
	faceList = [] # list of faces (they index into vertList)
	numFaces = 0
	
	# get access to the mesh data (as at frame #1)
	mesh = NMesh.GetRawFromObject(blenderObject.name)
	matrix = blenderObject.getMatrix('worldspace')

	surf = glmSurface()
	surf.name = blenderObject.getName()
	surf.ident = GLM_IDENT
	
	# process each face in the mesh
	for face in mesh.faces:
		tris_in_this_face = []  #to handle quads and up...
		
		# this makes a list of indices for each tri in this face. a quad will be [[0,1,1],[0,2,3]]
		for vi in range(1, len(face.v)-1):
			tris_in_this_face.append([0, vi, vi + 1])
		
		# loop across each tri in the face, then each vertex in the tri
		for this_tri in tris_in_this_face:
			numFaces += 1
			tri = glmTriangle()
			tri_ind = 0
			for i in this_tri:
				# get the vertex index, coords and uv coords
				index = face.v[i].index
				v = face.v[i].co
				uv = (0.0, 0.0) # handle case with no tex coords
				#if mesh.hasFaceUV():
				#	uv = face.uv[i]
				#elif mesh.hasVertexUV():
				#	uv = (face.v[i].uvco[0], face.v[i].uvco[1])
				#else:
				#	uv = (0.0, 0.0) # handle case with no tex coords
				
				if vertDict.has_key((index, uv)):
					# if we've seen this exact vertex before, simply add it
					# to the tris list of vertex indices
					tri.ind[tri_ind] = vertDict[(index, uv)]
				else:
					# havent seen this tri before 
					# (or its uv coord is different, so we need to duplicate it)
					
					vertDict[(index, uv)] = numVerts
					
					# put the uv coord into the list
					# (uv coord are directly related to each vertex)
					tex = glmTexCoord()
					tex.u = uv[0]
					tex.v = uv[1]
					uvList.append(tex)

					tri.ind[tri_ind] = numVerts

					# now because we have created a new index, 
					# we need a way to link it to the index that
					# blender returns for NMVert.index
					if indexDict.has_key(index):
						# already there - each of the entries against 
						# this key represents  the same vertex with a
						# different uv value
						ilist = indexDict[index]
						ilist.append(numVerts)
						indexDict[index] = ilist
					else:
						# this is a new one
						indexDict[index] = [numVerts]

					numVerts += 1
				tri_ind +=1
			faceList.append(tri)

	# we're done with faces and uv coords
	for t in uvList:
		surf.uv.append(t)

	for f in faceList:
		surf.triangles.append(f)
	
	surf.numTriangles = len(faceList)
	surf.numVerts = numVerts
	
	m = NMesh.GetRawFromObject(blenderObject.name)
	vlist = [0] * numVerts
	for vertex in m.verts:
		try:
			vindices = indexDict[vertex.index]
		except:
			print "warning found a vertex in %s that is not part of a face" % blenderObject.name
			continue
		
		vert = glmVert()
		vert.normal = vertex.no[0:3]
		vert.xyz = vertex.co[0:3]
		for ind in vindices:  # apply the position to all the duplicated vertices
			vlist[ind] = vert

	for vl in vlist:
		surf.verts.append(vl)

	surf.dump()
	glm.lods[0].surfaces.append(surf)
	glm.numSurfaces += 1


def saveModel(fileName):
	lodnum = 0
	if(fileName.find('.glm', -4) <= 0):
		fileName += '.glm'
	print "Exporting GLM format to ", fileName
	
	pathName = stripGamePath(stripModel(fileName))
	print "shader path name ", pathName
	
	modelName = stripExtension(stripPath(fileName))
	print "model name ", modelName
	
	glm = glmObject()
	glm.ident = GLM_IDENT
	glm.version = GLM_VERSION
	
	# do we export all objects or just the selected ones?
	if EXPORT_ALL:
		objlist = Blender.Object.Get()
	else:
		objlist = Blender.Object.GetSelected()
	
	glm.lods.append(glmLod())
	glm.numLODs = 1 #- must have 1
	# process each object for the export
	for obj in objlist:
		# check if it's a mesh object
		if obj.getType() == "Mesh":
			print "processing surface", obj.name
			processSurface(obj, glm, pathName, modelName)
		else:
			print "skipping object", obj.name
	
	processSurfHier(glm)
	# load the surf ofs info
	for i in range(0, glm.numLODs):
		for j in range(0, glm.numSurfaces):
			glm.lods[i].ofsSurfs.append(glmBoneRef())
			glm.lods[i].ofsSurfs[j].index = 0
	
	ofsBegin = glm.ofsLODs
	for i in range(0, glm.numLODs):
		for j in range(0, glm.numSurfaces):
			glm.lods[i].getSize(glm.numSurfaces)
			glm.lods[i].surfaces[j].ofsHeader = -(ofsBegin + glm.lods[i].ofsSurfs[j].index)
	
	#glm.dump()

	# export!
	file = open(fileName, "wb")
	glm.save(file)
	file.close()
	glm.dump()

Blender.Window.FileSelector(saveModel, "Export Ghoul2 GLM")