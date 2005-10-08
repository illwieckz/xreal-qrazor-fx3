#!BPY

"""
Name: 'Quake3 (.md3)...'
Blender: 237
Group: 'Export'
Tooltip: 'Export to Quake3 file format. (.md3)'
"""
__author__ = "PhaethonH, Bob Holcomb, Damien McGinnes, Robert (Tr3B) Beckebans"
__url__ = ("http://xreal.sourceforge.net")
__version__ = "0.2 2005-10-7"

__bpydoc__ = """\
This script exports a Quake3 file (MD3).

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
from Blender import *
from Blender import NMesh, Object, Material, Image, Texture

import sys, struct, string, math
from types import *

import os
from os import path

import md3
from md3 import *


NUM_FRAMES = 1
EXPORT_ALL = 0   # export only selected objs, or all?

def applyTransform(vert, matrix):
	vertCopy = Mathutils.CopyVec(vert)
	vertCopy.resize4D()
	return Mathutils.VecMultMat(vertCopy, matrix)


def updateFrameData(v, f):
	for i in range(0, 3):
		f.mins[i] = min(v[i], f.mins[i])
	for i in range(0, 3):
		f.maxs[i] = max(v[i], f.mins[i])


def processSurface(mesh_obj, md3):
	# because md3 doesnt suppoort faceUVs like blender, we need to duplicate
	# any vertex that has multiple uv coords

	vertDict = {} 
	indexDict = {} # maps a vertex index to the revised index after duplicating to account for uv
	vertList = [] # list of vertices ordered by revised index
	numVerts = 0
	uvList = [] # list of tex coords ordered by revised index
	faceList = [] # list of faces (they index into vertList)
	numFaces = 0

	Blender.Set("curframe", 1)

	#get access to the mesh data (as at frame #1)
	mesh = NMesh.GetRawFromObject(mesh_obj.name)	
	matrix = mesh_obj.getMatrix('worldspace')

	surf = md3Surface()
	surf.numFrames = md3.numFrames
	surf.name = mesh_obj.getName()
	surf.ident = MD3_IDENT

	#process each face in the mesh
	for face in mesh.faces:
		tris_in_this_face = []  #to handle quads and up...
		
		# this makes a list of indices for each tri in this face. a quad will be [[0,1,1],[0,2,3]]
		for vi in range(1, len(face.v)-1):
			tris_in_this_face.append([0, vi, vi + 1])
		
		#loop across each tri in the face, then each vertex in the tri
		for this_tri in tris_in_this_face:
			numFaces += 1
			tri = md3Triangle()
			tri_ind = 0
			for i in this_tri:
				#get the vertex index, coords and uv coords
				index = face.v[i].index
				v = face.v[i].co
				if mesh.hasFaceUV():
					uv = face.uv[i]
				elif mesh.hasVertexUV():
					uv = (face.v[i].uvco[0], face.v[i].uvco[1])
				else:
					uv = (0.0, 0.0) # handle case with no tex coords	

				
				if vertDict.has_key((index, uv)):
					# if we've seen this exact vertex before, simply add it
					# to the tris list of vertex indices
					tri.indexes[tri_ind] = vertDict[(index, uv)]
				else:
					# havent seen this tri before 
					# (or its uv coord is different, so we need to duplicate it)
					
					vertDict[(index, uv)] = numVerts
					
					# put the uv coord into the list
					# (uv coord are directly related to each vertex)
					tex = md3TexCoord()
					tex.u = uv[0]
					tex.v = uv[1]
					uvList.append(tex)

					tri.indexes[tri_ind] = numVerts

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
						#this is a new one
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

	# now vertices are stored as frames -
	# all vertices for frame 1, all vertices for frame 2...., all vertices for frame n
	# so we need to iterate across blender's frames, and copy out each vertex
	for	frameNum in range(1, md3.numFrames + 1):
		Blender.Set("curframe", frameNum)

		m = NMesh.GetRawFromObject(mesh_obj.name)

		vlist = [0] * numVerts
		for vertex in m.verts:
			try:
				vindices = indexDict[vertex.index]
			except:
				print "warning found a vertex in %s that is not part of a face" % mesh_obj.name
				continue

			vTx = applyTransform(vertex.co, matrix)
			nTx = applyTransform(vertex.no, matrix)
			updateFrameData(vTx, md3.frames[frameNum - 1])
			vert = md3Vert()
			vert.xyz = vTx[0:3]
			vert.normal = vert.encode(nTx[0:3])
			for ind in vindices:  #apply the position to all the duplicated vertices
				vlist[ind] = vert

		for vl in vlist:
			surf.verts.append(vl)

	surf.dump()
	md3.surfaces.append(surf)
	md3.numSurfaces += 1


def saveModel(filename):
	if(filename.find('.md3', -4) <= 0):
		filename += '.md3'
	print "Exporting MD3 format to ", filename
	
	md3 = md3Object()
	md3.ident = MD3_IDENT
	md3.version = MD3_VERSION

	tagList = []

	Blender.Set("curframe", 1)

	# create a bunch of blank frames, they'll be filled in by 'processSurface'
	md3.numFrames = NUM_FRAMES
	for i in range(1, NUM_FRAMES + 1):
		fr = md3Frame()
		fr.name = "FRAME" + str(i)
		md3.frames.append(fr)

	# do we export all objects or just the selected ones?
	if EXPORT_ALL:
		objlist = Blender.Object.Get()
	else:
		objlist = Blender.Object.GetSelected()


	# process each object for the export
	for obj in objlist:
		# check if it's a mesh object
		if obj.getType() == "Mesh":
			print "processing surface", obj.name
			if len(md3.surfaces) == MD3_MAX_SURFACES:
				print "hit md3 limit (%i) for number of surfaces, skipping" % MD3_MAX_SURFACES , obj.getName()
			else:
				processSurface(obj, md3)
		elif obj.getType() == "Empty":   #for tags, we just put em in a list so we can process them all together
			if obj.name[0:4] == "tag_":
				print "processing tag", obj.name
				tagList.append(obj)
				md3.numTags += 1
		else:
			print "Skipping non mesh object", obj.name

	# work out the transforms for the tags for each frame of the export
	for fr in range(1, NUM_FRAMES + 1):
		Blender.Set("curframe", fr)
		for tag in tagList:
			t = md3Tag()
			matrix = tag.getMatrix('worldspace')
			for i in range(0, 3):
				t.origin[0] = matrix[3][0]
				t.origin[1] = matrix[3][1]
				t.origin[2] = matrix[3][2]
				
				t.axis[0] = matrix[0][0]
				t.axis[1] = matrix[0][1]
				t.axis[2] = matrix[0][2]
				
				t.axis[3] = matrix[1][0]
				t.axis[4] = matrix[1][1]
				t.axis[5] = matrix[1][2]
				
				t.axis[6] = matrix[2][0]
				t.axis[7] = matrix[2][1]
				t.axis[8] = matrix[2][2]
			t.name = obj.name[4:]
			md3.tags.append(t)

	# export!
	file = open(filename, "wb")
	md3.save(file)
	file.close()
	md3.dump()

Blender.Window.FileSelector(saveModel, "Export Quake3 MD3")