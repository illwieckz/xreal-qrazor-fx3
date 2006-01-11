#!BPY

"""
Name: 'RTCW (.mds)...'
Blender: 240
Group: 'Import'
Tooltip: 'Import from RTCW file format. (.mds)'
"""

__author__ = "Robert (Tr3B) Beckebans"
__url__ = ("http://xreal.sourceforge.net")
__version__ = "0.1 2006-01-08"

__bpydoc__ = """\
This script imports a Return to Castle Wolfenstein file (MDS).

Supported:<br>
	This script imports the skeleton and creates an armature object of it.

Missing:<br>
    TODO

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
#from Blender import *

import mds
from mds import *

import q_math
from q_math import *

import q_shared
from q_shared import *

def loadModel(filename):
	# read the file in
	file = open(filename,"rb")
	mds = mdsObject()
	mds.load(file)
	mds.dump()
	file.close()
	
	# build the armature in blender
	armObj = Blender.Object.New('Armature', "bones")
	
	armData = Blender.Armature.Armature("MDS_ARM")
	#armData.drawType = Blender.Armature.STICK
	armData.drawAxes = True
	
	armObj.link(armData)
	scene = Blender.Scene.getCurrent()
	scene.link(armObj)
	
	armData.makeEditable()

	# create bones
	for i in range(0, mds.numBones):
		armBone = Blender.Armature.Editbone()
		armBone.name = mds.bones[i].name
		armData.bones[mds.bones[i].name] = armBone
	
	for i in range(0, 1): #mds.numFrames):
		frame = mds.frames[i]
		
		#Blender.Set("curframe", i + 1)
		
		#context = scene.getRenderingContext()
		#context.currentFrame(i + 1)
		
		#action = Blender.Armature.NLA.NewAction()
 		#action.setActive(armObj)
		
		for j in range(0, mds.numBones):
			bone = mds.bones[j]
			frameBone = mds.frames[i].bones[j]
			
			armBone = armData.bones[mds.bones[j].name]
			
			# create bone rotation matrix
			pitch = frameBone.angles[0]
			yaw = frameBone.angles[1]
			roll = frameBone.angles[2]
			#if j == 0:
			#	print "framebone angles:", pitch, yaw, roll
			boneRotation = MatrixFromAngles(pitch, yaw, roll)
			
			# rotate offset vector
			pitch = frameBone.offsetAngles[0]
			yaw = frameBone.offsetAngles[1]
			offsetRotation = MatrixFromAngles(pitch, yaw, 0.0)
			
			offsetVec = [bone.parentDist, 0, 0]
			offsetVec = MatrixTransformNormal(offsetRotation, offsetVec)
			
			if bone.parentIndex == -1:
				tail = VectorAdd(frame.parentOffset, offsetVec)
				
				armBone.head = Blender.Mathutils.Vector(0, 0, 0)
				armBone.tail = Blender.Mathutils.Vector(tail[0], tail[1], tail[2])
			else:
				armParent = armData.bones[mds.bones[mds.bones[j].parentIndex].name]
				armBone.parent = armParent
				
				armBone.head = armParent.tail
				armBone.tail = armParent.tail
				armBone.tail.x += offsetVec[0]
				armBone.tail.y += offsetVec[1]
				armBone.tail.z += offsetVec[2]
				
			#armBone.options = [Armature.CONNECTED, Armature.HINGE]
			#armBone.setPose([ROT,LOC,SIZE])
			
	"""
	for i in range(0, 32): #mds.numFrames):
		frame = mds.frames[i]
		
		Blender.Set("curframe", i + 1)
		
		#context = scene.getRenderingContext()
		#context.currentFrame(i + 1)
		
		action = Blender.Armature.NLA.NewAction()
 		action.setActive(armObj)
		
		for j in range(0, mds.numBones):
			armBone = armData.bones[mds.bones[j].name]
			armBone.setPose([ROT,LOC,SIZE])
	"""
		
	armData.update()
	armObj.makeDisplayList()
	Blender.Window.RedrawAll()
	
Blender.Window.FileSelector(loadModel, 'Import RTCW MDS')