#!BPY

"""
Name: 'Quake3 (.md3)...'
Blender: 242
Group: 'Import'
Tooltip: 'Import from Quake3 file format. (.md3)'
"""

__author__ = "PhaethonH, Bob Holcomb, Robert (Tr3B) Beckebans"
__url__ = ("http://xreal.sourceforge.net")
__version__ = "0.6 2006-11-12"

__bpydoc__ = """\
This script imports a Quake 3 file (MD3), textures, 
and animations into Blender for editing. Loader is based on MD3 loader
from www.gametutorials.com-Thanks DigiBen! and the
md3 blender loader by PhaethonH <phaethon@linux.ucla.edu>

Supported:<br>
	Surfaces and Materials

Missing:<br>
    Animations, Tag rotations

Known issues:<br>
    None

Notes:<br>
    TODO
"""

import sys, os, os.path, struct, string, math

import Blender, Blender.Scene
from Blender import *

import types

import textwrap

import logging 
reload(logging)

import md3
from md3 import *

import q_math
from q_math import *

# our own logger class. it works just the same as a normal logger except
# all info messages get show. 
class Logger(logging.Logger):
	def __init__(self, name,level = logging.NOTSET):
		logging.Logger.__init__(self, name, level)
		
		self.has_warnings = False
		self.has_errors = False
		self.has_critical = False
	
	def info(self, msg, *args, **kwargs):
		apply(self._log,(logging.INFO, msg, args), kwargs)
		
	def warning(self, msg, *args, **kwargs):
		logging.Logger.warning(self, msg, *args, **kwargs)
		self.has_warnings = True
	
	def error(self, msg, *args, **kwargs):
		logging.Logger.error(self, msg, *args, **kwargs)
		self.has_errors = True
		
	def critical(self, msg, *args, **kwargs):
		logging.Logger.critical(self, msg, *args, **kwargs)
		self.has_errors = True
		
# should be able to make this print to stdout in realtime and save MESSAGES
# as well. perhaps also have a log to file option
class LogHandler(logging.StreamHandler):
	def __init__(self):
		logging.StreamHandler.__init__(self, sys.stdout)
		
		if "md3_import_log" not in Blender.Text.Get():
			self.outtext = Blender.Text.New("md3_import_log")
		else:
			self.outtext = Blender.Text.Get('md3_import_log')
			self.outtext.clear()
			
		self.lastmsg = ''
		
	def emit(self, record):
		# print to stdout and  to a new blender text object
		msg = self.format(record)
		
		if msg == self.lastmsg:
			return
		
		self.lastmsg = msg
		self.outtext.write("%s\n" %msg)
		
		logging.StreamHandler.emit(self, record)

logging.setLoggerClass(Logger)
log = logging.getLogger('md3_import')

handler = LogHandler()
formatter = logging.Formatter('%(levelname)s %(message)s')
handler.setFormatter(formatter)

log.addHandler(handler)
# set this to minimum output level. eg. logging.DEBUG, logging.WARNING, logging.ERROR
# logging.CRITICAL. logging.INFO will make little difference as these always get 
# output'd
log.setLevel(logging.WARNING)


class BlenderGui:
	def __init__(self):
		text = """A log has been written to a blender text window. Change this window type to 
a text window and you will be able to select the file md3_import_log."""

		text = textwrap.wrap(text,40)
		text += ['']
		
		if log.has_critical:
			text += ['There were critical errors!!!!']
			
		elif log.has_errors:
			text += ['There were errors!']
			
		elif log.has_warnings:
			text += ['There were warnings']
			
		# add any more text before here
		text.reverse()
		
		self.msg = text
		
		Blender.Draw.Register(self.gui, self.event, self.button_event)
		
	def gui(self,):
		quitbutton = Blender.Draw.Button("Exit", 1, 0, 0, 100, 20, "Close Window")
		
		y = 35
		
		for line in self.msg:
			BGL.glRasterPos2i(10,y)
			Blender.Draw.Text(line)
			y+=15
			
	def event(self,evt, val):
		if evt == Blender.Draw.ESCKEY:
			Blender.Draw.Exit()
			return
	
	def button_event(self,evt):
		if evt == 1:
			Blender.Draw.Exit()
			return

def Import(fileName):
	log.info("Starting ...")
	
	log.info("Importing MD3 model: %s", fileName)
	
	pathName = StripGamePath(StripModel(fileName))
	log.info("Shader path name: %s", pathName)
	
	modelName = StripExtension(StripPath(fileName))
	log.info("Model name: %s", modelName)
	
	# read the file in
	file = open(fileName,"rb")
	md3 = md3Object()
	md3.Load(file, log)
	md3.Dump(log)
	file.close()
	
	scene = Scene.getCurrent()
	
	for k in range(0, md3.numSurfaces):
		surface = md3.surfaces[k]
		
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
		mesh = meshObject.getData()
		
		for i in range(0, surface.numFrames):
			
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
			NMesh.PutRaw(mesh, meshObject.name)
			#absolute works too, but I want to get these into NLA actions
			mesh.insertKey(i,"relative")
		"""
		
		# create materials for surface
		for i in range(0, surface.numShaders):
			
			# create new material if necessary
			matName = StripExtension(StripPath(surface.shaders[i].name))
			if matName == "" :
				matName = "no_texture"
			
			try:
				mat = Material.Get(matName)
			except:
				log.info("Creating new material: %s", matName)
				mat = Material.New(matName)
			
				# create new texture
				texture = Texture.New(matName)
				texture.setType('Image')
			
				# try .tga by default
				imageName = StripExtension(GAMEDIR + surface.shaders[i].name) + '.tga'
				try:
					image = Image.Load(imageName)
				
					texture.image = image
				except:
					try:
						imageName = StripExtension(imageName) + '.png'
						image = Image.Load(imageName)
					
						texture.image = image
					except:
						try:
							imageName = StripExtension(imageName) + '.jpg'
							image = Image.Load(imageName)
						
							texture.image = image
						except:
							log.warning("Unable to load image: %s", imageName)
 				
				# texture to material
				mat.setTexture(0, texture, Texture.TexCo.UV, Texture.MapTo.COL)
	
			# append material to the mesh's list of materials
			mesh.materials.append(mat)
			mesh.update()
	
	# create tags
	tags = []
	scn = Scene.GetCurrent()
	for i in range(0, md3.numTags):
		tag = md3.tags[i]
		
		# this should be an Empty object
		blenderTag = Object.New("Empty", tag.name);
		# set ipo
		ipo = Ipo.New('Object', tag.name + "_ipo")
		locX = ipo.addCurve('LocX')
		locY = ipo.addCurve('LocY')
		locZ = ipo.addCurve('LocZ')
		rotX = ipo.addCurve('RotX')
		rotY = ipo.addCurve('RotY')
		rotZ = ipo.addCurve('RotZ')
		locX.setInterpolation('Constant')
		locY.setInterpolation('Constant')
		locZ.setInterpolation('Constant')
		rotX.setInterpolation('Constant')
		rotY.setInterpolation('Constant')
		rotZ.setInterpolation('Constant')
		#locX.addBezier((tag.origin[1],tag.origin[2]))
		#locY.addBezier((tag.origin[0],tag.origin[2]))
		#locZ.addBezier((tag.origin[0],tag.origin[1]))
		#set ipo for tag
		blenderTag.setIpo(ipo)
		tags.append(blenderTag)
		scene.link(blenderTag)
		blenderTag.setLocation(tag.origin)
 
	# FIXME this imports only the baseframe tags
	for i in range(0, md3.numFrames):
 
		for j in range(0, md3.numTags):
			tag = md3.tags[i * md3.numTags + j]
			#Blender.Set("curframe", i)
			#tagName = tag.name# + '_' + str(i)
			#blenderTag = Object.New("Empty", tagName);
			#tags.append(blenderTag)
			#scene.link(blenderTag)
			#blenderTag = tags[j]
			blenderTag = Object.Get(tag.name)
			ipo = Ipo.Get(tag.name + "_ipo")
			locX = ipo.getCurve('LocX')
			locY = ipo.getCurve('LocY')
			locZ = ipo.getCurve('LocZ')
			rotX = ipo.getCurve('RotX')
			rotY = ipo.getCurve('RotY')
			rotZ = ipo.getCurve('RotZ')
 
			# Note: Quake3 uses left-hand geometry
			forward = [tag.axis[0], tag.axis[1], tag.axis[2]]
			left = [tag.axis[3], tag.axis[4], tag.axis[5]]
			up = [tag.axis[6], tag.axis[7], tag.axis[8]]
		
			transform = MatrixSetupTransform(forward, left, up, tag.origin)
			transform2 = Blender.Mathutils.Matrix(transform[0], transform[1], transform[2], transform[3])
			#rotation = Blender.Mathutils.Matrix(forward, left, up)
			#blenderTag.setMatrix(transform2)
			#print "org: ", tag.origin
			locX.addBezier((i,tag.origin[0]))
			locY.addBezier((i,tag.origin[1]))
			locZ.addBezier((i,tag.origin[2]))
			#blenderTag.setLocation(tag.origin)
			#blenderTag.insertKey(i,"relative")
 
	# locate the Object containing the mesh at the cursor location
	if md3.numSurfaces:
		cursorPos = Blender.Window.GetCursorPos()
		meshObject.setLocation(float(cursorPos[0]), float(cursorPos[1]), float(cursorPos[2]))
	
	# not really necessary, but I like playing with the frame counter
	#Blender.Set("staframe", 1)
	#Blender.Set("curframe", md3.numFrames)
	#Blender.Set("endframe", md3.numFrames)

def FileSelectorCallback(fileName):
	Import(fileName)
	
	BlenderGui()

Blender.Window.FileSelector(FileSelectorCallback, 'Import Quake3 MD3')