######################################################
# MD5 Importer
# By:  Bob Holcomb, Thomas "der_ton" Hutter
# Date: 2006-02-20
# Ver: 0.5
######################################################
# This script imports a MD5mesh and MD5anim file
# into blender for editing.
######################################################

# todo:
# more options on anim import (startframe, keyframe-frequency, ...)

# Version History:
# 0.31 (2004-10-27): Bones that are oriented in the direction of a child are extended so that their end touches the origin of the child bone (IK rigging is easier this way)
# 0.4  (2006-01-09): Updated to Blender2.40, added mesh parenting to armature
# 0.5  (2006-02-20): Updated to Blender2.41, added animation import

######################################################
# Importing modules
######################################################

import Blender
from Blender import NMesh, Scene, Object, Draw, Image, Material
from Blender.BGL import *
from Blender.Draw import *
from Blender.Window import *
from Blender.Image import *
from Blender.Material import *

import sys, struct, string
from types import *

import os
from os import path

import math
from math import *

# HACK -- it seems that some Blender versions don't define sys.argv,
# which may crash Python if a warning occurs.
if not hasattr(sys, "argv"): sys.argv = ["???"]



######################################################
# Vector, Quaterion, Matrix math stuff-taken from
# Jiba's blender2cal3d script
######################################################


def quaternion2matrix(q):
  xx = q[0] * q[0]
  yy = q[1] * q[1]
  zz = q[2] * q[2]
  xy = q[0] * q[1]
  xz = q[0] * q[2]
  yz = q[1] * q[2]
  wx = q[3] * q[0]
  wy = q[3] * q[1]
  wz = q[3] * q[2]
  return [[1.0 - 2.0 * (yy + zz),       2.0 * (xy + wz),       2.0 * (xz - wy), 0.0],
          [      2.0 * (xy - wz), 1.0 - 2.0 * (xx + zz),       2.0 * (yz + wx), 0.0],
          [      2.0 * (xz + wy),       2.0 * (yz - wx), 1.0 - 2.0 * (xx + yy), 0.0],
          [0.0                  , 0.0                  , 0.0                  , 1.0]]

def matrix2quaternion(m):
  s = math.sqrt(abs(m[0][0] + m[1][1] + m[2][2] + m[3][3]))
  if s == 0.0:
    x = abs(m[2][1] - m[1][2])
    y = abs(m[0][2] - m[2][0])
    z = abs(m[1][0] - m[0][1])
    if   (x >= y) and (x >= z): return 1.0, 0.0, 0.0, 0.0
    elif (y >= x) and (y >= z): return 0.0, 1.0, 0.0, 0.0
    else:                       return 0.0, 0.0, 1.0, 0.0
  return quaternion_normalize([
    -(m[2][1] - m[1][2]) / (2.0 * s),
    -(m[0][2] - m[2][0]) / (2.0 * s),
    -(m[1][0] - m[0][1]) / (2.0 * s),
    0.5 * s,
    ])

def quaternion_normalize(q):
  l = math.sqrt(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3])
  return q[0] / l, q[1] / l, q[2] / l, q[3] / l

def quaternion_multiply(q1, q2):
  r = [
    q2[3] * q1[0] + q2[0] * q1[3] + q2[1] * q1[2] - q2[2] * q1[1],
    q2[3] * q1[1] + q2[1] * q1[3] + q2[2] * q1[0] - q2[0] * q1[2],
    q2[3] * q1[2] + q2[2] * q1[3] + q2[0] * q1[1] - q2[1] * q1[0],
    q2[3] * q1[3] - q2[0] * q1[0] - q2[1] * q1[1] - q2[2] * q1[2],
    ]
  d = math.sqrt(r[0] * r[0] + r[1] * r[1] + r[2] * r[2] + r[3] * r[3])
  r[0] /= d
  r[1] /= d
  r[2] /= d
  r[3] /= d
  return r

def matrix_translate(m, v):
  m[3][0] += v[0]
  m[3][1] += v[1]
  m[3][2] += v[2]
  return m

def matrix_multiply(b, a):
  return [ [
    a[0][0] * b[0][0] + a[0][1] * b[1][0] + a[0][2] * b[2][0],
    a[0][0] * b[0][1] + a[0][1] * b[1][1] + a[0][2] * b[2][1],
    a[0][0] * b[0][2] + a[0][1] * b[1][2] + a[0][2] * b[2][2],
    0.0,
    ], [
    a[1][0] * b[0][0] + a[1][1] * b[1][0] + a[1][2] * b[2][0],
    a[1][0] * b[0][1] + a[1][1] * b[1][1] + a[1][2] * b[2][1],
    a[1][0] * b[0][2] + a[1][1] * b[1][2] + a[1][2] * b[2][2],
    0.0,
    ], [
    a[2][0] * b[0][0] + a[2][1] * b[1][0] + a[2][2] * b[2][0],
    a[2][0] * b[0][1] + a[2][1] * b[1][1] + a[2][2] * b[2][1],
    a[2][0] * b[0][2] + a[2][1] * b[1][2] + a[2][2] * b[2][2],
     0.0,
    ], [
    a[3][0] * b[0][0] + a[3][1] * b[1][0] + a[3][2] * b[2][0] + b[3][0],
    a[3][0] * b[0][1] + a[3][1] * b[1][1] + a[3][2] * b[2][1] + b[3][1],
    a[3][0] * b[0][2] + a[3][1] * b[1][2] + a[3][2] * b[2][2] + b[3][2],
    1.0,
    ] ]

def matrix_invert(m):
  det = (m[0][0] * (m[1][1] * m[2][2] - m[2][1] * m[1][2])
       - m[1][0] * (m[0][1] * m[2][2] - m[2][1] * m[0][2])
       + m[2][0] * (m[0][1] * m[1][2] - m[1][1] * m[0][2]))
  if det == 0.0: return None
  det = 1.0 / det
  r = [ [
      det * (m[1][1] * m[2][2] - m[2][1] * m[1][2]),
    - det * (m[0][1] * m[2][2] - m[2][1] * m[0][2]),
      det * (m[0][1] * m[1][2] - m[1][1] * m[0][2]),
      0.0,
    ], [
    - det * (m[1][0] * m[2][2] - m[2][0] * m[1][2]),
      det * (m[0][0] * m[2][2] - m[2][0] * m[0][2]),
    - det * (m[0][0] * m[1][2] - m[1][0] * m[0][2]),
      0.0
    ], [
      det * (m[1][0] * m[2][1] - m[2][0] * m[1][1]),
    - det * (m[0][0] * m[2][1] - m[2][0] * m[0][1]),
      det * (m[0][0] * m[1][1] - m[1][0] * m[0][1]),
      0.0,
    ] ]
  r.append([
    -(m[3][0] * r[0][0] + m[3][1] * r[1][0] + m[3][2] * r[2][0]),
    -(m[3][0] * r[0][1] + m[3][1] * r[1][1] + m[3][2] * r[2][1]),
    -(m[3][0] * r[0][2] + m[3][1] * r[1][2] + m[3][2] * r[2][2]),
    1.0,
    ])
  return r

def matrix_rotate_x(angle):
  cos = math.cos(angle)
  sin = math.sin(angle)
  return [
    [1.0,  0.0, 0.0, 0.0],
    [0.0,  cos, sin, 0.0],
    [0.0, -sin, cos, 0.0],
    [0.0,  0.0, 0.0, 1.0],
    ]

def matrix_rotate_y(angle):
  cos = math.cos(angle)
  sin = math.sin(angle)
  return [
    [cos, 0.0, -sin, 0.0],
    [0.0, 1.0,  0.0, 0.0],
    [sin, 0.0,  cos, 0.0],
    [0.0, 0.0,  0.0, 1.0],
    ]

def matrix_rotate_z(angle):
  cos = math.cos(angle)
  sin = math.sin(angle)
  return [
    [ cos, sin, 0.0, 0.0],
    [-sin, cos, 0.0, 0.0],
    [ 0.0, 0.0, 1.0, 0.0],
    [ 0.0, 0.0, 0.0, 1.0],
    ]

def matrix_rotate(axis, angle):
  vx  = axis[0]
  vy  = axis[1]
  vz  = axis[2]
  vx2 = vx * vx
  vy2 = vy * vy
  vz2 = vz * vz
  cos = math.cos(angle)
  sin = math.sin(angle)
  co1 = 1.0 - cos
  return [
    [vx2 * co1 + cos,          vx * vy * co1 + vz * sin, vz * vx * co1 - vy * sin, 0.0],
    [vx * vy * co1 - vz * sin, vy2 * co1 + cos,          vy * vz * co1 + vx * sin, 0.0],
    [vz * vx * co1 + vy * sin, vy * vz * co1 - vx * sin, vz2 * co1 + cos,          0.0],
    [0.0, 0.0, 0.0, 1.0],
    ]

def matrix_scale(fx, fy, fz):
  return [
    [ fx, 0.0, 0.0, 0.0],
    [0.0,  fy, 0.0, 0.0],
    [0.0, 0.0,  fz, 0.0],
    [0.0, 0.0, 0.0, 1.0],
    ]

def point_by_matrix(p, m):
  return [p[0] * m[0][0] + p[1] * m[1][0] + p[2] * m[2][0] + m[3][0],
          p[0] * m[0][1] + p[1] * m[1][1] + p[2] * m[2][1] + m[3][1],
          p[0] * m[0][2] + p[1] * m[1][2] + p[2] * m[2][2] + m[3][2]]

def point_distance(p1, p2):
  return math.sqrt((p2[0] - p1[0]) ** 2 + (p2[1] - p1[1]) ** 2 + (p2[2] - p1[2]) ** 2)

def vector_by_matrix(p, m):
  return [p[0] * m[0][0] + p[1] * m[1][0] + p[2] * m[2][0],
          p[0] * m[0][1] + p[1] * m[1][1] + p[2] * m[2][1],
          p[0] * m[0][2] + p[1] * m[1][2] + p[2] * m[2][2]]

def vector_length(v):
  return math.sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2])

def vector_normalize(v):
  l = math.sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2])
  try:
    return v[0] / l, v[1] / l, v[2] / l
  except:
    return 1, 0, 0

def vector_dotproduct(v1, v2):
  return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]

def vector_crossproduct(v1, v2):
  return [
    v1[1] * v2[2] - v1[2] * v2[1],
    v1[2] * v2[0] - v1[0] * v2[2],
    v1[0] * v2[1] - v1[1] * v2[0],
    ]

def vector_angle(v1, v2):
  s = vector_length(v1) * vector_length(v2)
  f = vector_dotproduct(v1, v2) / s
  if f >=  1.0: return 0.0
  if f <= -1.0: return math.pi / 2.0
  return math.atan(-f / math.sqrt(1.0 - f * f)) + math.pi / 2.0


######################################################
# MD5 Data structures
######################################################


class md5_vert:
	vert_index=0
	co=[]
	uvco=[]
	blend_index=0
	blend_count=0

	def __init__(self):
		self.vert_index=0
		self.co=[0.0]*3
		self.uvco=[0.0]*2
		self.blend_index=0
		self.blend_count=0

	def dump(self):
		print "vert index: ", self.vert_index
		print "co: ", self.co
		print "couv: ", self.couv
		print "blend index: ", self.blend_index
		print "belnd count: ", self.blend_count

class md5_weight:
	weight_index=0
	bone_index=0
	bias=0.0
	weights=[]

	def __init__(self):
		self.weight_index=0
		self.bone_index=0
		self.bias=0.0
		self.weights=[0.0]*3

	def dump(self):
		print "weight index: ", self.weight_index
		print "bone index: ", self.bone_index
		print "bias: ", self.bias
		print "weighst: ", self.weights

class md5_bone:
	bone_index=0
	name=""
	bindpos=[]
	bindmat=[]
	parent=""
	parent_index=0
	blenderbone=None
	roll=0

	def __init__(self):
		self.bone_index=0
		self.name=""
		self.bindpos=[0.0]*3
		self.bindmat=[None]*3  #is this how you initilize a 2d-array
		for i in range(3): self.bindmat[i] = [0.0]*3
		self.parent=""
		self.parent_index=0
		self.blenderbone=None

	def dump(self):
		print "bone index: ", self.bone_index
		print "name: ", self.name
		print "bind position: ", self.bindpos
		print "bind translation matrix: ", self.bindmat
		print "parent: ", self.parent
		print "parent index: ", self.parent_index
		print "blenderbone: ", self.blenderbone


class md5_tri:
	tri_index=0
	vert_index=[]

	def __init__(self):
		self.tri_index=0;
		self.vert_index=[0]*3

	def dump(self):
		print "tri index: ", self.tri_index
		print "vert index: ", self.vert_index

class md5_mesh:
	mesh_index=0
	verts=[]
	tris=[]
	weights=[]
	shader=""

	def __init__(self):
		self.mesh_index=0
		self.verts=[]
		self.tris=[]
		self.weights=[]
		self.shader=""

	def dump(self):
		print "mesh index: ", self.mesh_index
		print "verts: ", self.verts
		print "tris: ", self.tris
		print "weights: ", self.weights
		print "shader: ", self.shader

######################################################
# IMPORT
######################################################


def load_md5(md5_filename):

        file=open(md5_filename,"r")
        lines=file.readlines()
        file.close()

        md5_model=[]
        md5_bones=[]

        num_lines=len(lines)

        mesh_counter=0
        for line_counter in range(0,num_lines):
            current_line=lines[line_counter]
            words=current_line.split()

            if words and words[0]=="numJoints":
                #print "found a bunch of bones"
                num_bones=int(words[1])
                print "num_bones: ", num_bones
            elif words and words[0]=="joints":
                for bone_counter in range(0,num_bones):
                    #make a new bone
                    md5_bones.append(md5_bone())
		    #next line
                    line_counter+=1
                    current_line=lines[line_counter]
                    words=current_line.split()
                    #skip over blank lines
                    while not words:
                        line_counter+=1
                        current_line=lines[line_counter]
                        words=current_line.split()

                    md5_bones[bone_counter].bone_index=bone_counter
                    #get rid of the quotes on either side
                    temp_name=str(words[0])
                    temp_name=temp_name[1:-1]
                    md5_bones[bone_counter].name=temp_name
                    #print "found a bone: ", md5_bones[bone_counter].name
                    md5_bones[bone_counter].parent_index = int(words[1])
                    if md5_bones[bone_counter].parent_index>=0:
                        md5_bones[bone_counter].parent = md5_bones[md5_bones[bone_counter].parent_index].name
                    md5_bones[bone_counter].bindpos[0]=float(words[3])
                    md5_bones[bone_counter].bindpos[1]=float(words[4])
                    md5_bones[bone_counter].bindpos[2]=float(words[5])
                    qx = float(words[8])
                    qy = float(words[9])
                    qz = float(words[10])
                    qw = 1 - qx*qx - qy*qy - qz*qz
                    if qw<0:
                        qw=0
                    else:
                        qw = -sqrt(qw)
                    md5_bones[bone_counter].bindmat = quaternion2matrix([qx,qy,qz,qw])



            elif words and words[0]=="numMeshes":
                num_meshes=int(words[1])
                print "num_meshes: ", num_meshes
            elif words and words[0]=="mesh":
                #create a new mesh and name it
                md5_model.append(md5_mesh())
                md5_model[mesh_counter].mesh_index = mesh_counter
                while (not words or (words and words[0]!="}")):
                        line_counter+=1
                        current_line=lines[line_counter]
                        words=current_line.split()
                        if words and words[0]=="shader":
                            #print "found a shader"
                            temp_name=str(words[1])
                            temp_name=temp_name[1:-1]
                            md5_model[mesh_counter].shader=temp_name
                        if words and words[0]=="vert":
			    #print "found a vert"
                            md5_model[mesh_counter].verts.append(md5_vert())
                            vert_counter=len(md5_model[mesh_counter].verts)-1
			    #load it with the raw data
                            md5_model[mesh_counter].verts[vert_counter].vert_index=int(words[1])
                            md5_model[mesh_counter].verts[vert_counter].uvco[0]=float(words[3])
                            md5_model[mesh_counter].verts[vert_counter].uvco[1]=(1-float(words[4]))
                            md5_model[mesh_counter].verts[vert_counter].blend_index=int(words[6])
                            md5_model[mesh_counter].verts[vert_counter].blend_count=int(words[7])
                        if words and words[0]=="tri":
                            #print "found a tri"
                            md5_model[mesh_counter].tris.append(md5_tri())
                            tri_counter=len(md5_model[mesh_counter].tris)-1
                            #load it with raw data
                            md5_model[mesh_counter].tris[tri_counter].tri_index=int(words[1])
                            md5_model[mesh_counter].tris[tri_counter].vert_index[0]=int(words[2])
                            md5_model[mesh_counter].tris[tri_counter].vert_index[1]=int(words[3])
                            md5_model[mesh_counter].tris[tri_counter].vert_index[2]=int(words[4])
                        if words and words[0]=="weight":
                            #print "found a weight"
                            md5_model[mesh_counter].weights.append(md5_weight())
                            weight_counter=len(md5_model[mesh_counter].weights)-1
                            #load it with raw data
                            md5_model[mesh_counter].weights[weight_counter].weight_index=int(words[1])
                            md5_model[mesh_counter].weights[weight_counter].bone_index=int(words[2])
                            md5_model[mesh_counter].weights[weight_counter].bias=float(words[3])
                            md5_model[mesh_counter].weights[weight_counter].weights[0]=float(words[5])
                            md5_model[mesh_counter].weights[weight_counter].weights[1]=float(words[6])
                            md5_model[mesh_counter].weights[weight_counter].weights[2]=float(words[7])
                            #md5_model[mesh_counter].weights[weight_counter].dump()
                #print "end of this mesh structure"
                mesh_counter += 1


	#figure out the base pose for each vertex from the weights
        for mesh in md5_model:
                #print "updating vertex info for mesh: ", mesh.mesh_index
                for vert_counter in range(0, len(mesh.verts)):
                        blend_index=mesh.verts[vert_counter].blend_index
                        for blend_counter in range(0, mesh.verts[vert_counter].blend_count):
                                #get the current weight info
                                w=mesh.weights[blend_index+blend_counter]
                                #print "w: "
                                #w.dump()
                                #the bone that the current weight is refering to
                                b=md5_bones[w.bone_index]
                                #print "b: "
                                #b.dump()
                                #a position is the weight position * bone transform matrix (or position)
                                pos=[0.0]*3
                                #print "pos: ", pos
                                pos= vector_by_matrix(w.weights, b.bindmat)
                                #print "pos: ", pos
                                #print "w.bias: ", w.bias
                                #print "b.bindpos: ", b.bindpos
                                pos=((pos[0]+b.bindpos[0])*w.bias, (pos[1]+b.bindpos[1])*w.bias, (pos[2]+b.bindpos[2])*w.bias)
                                #print "pos: ", pos
                                #vertex position is sum of all weight info adjusted for bias
                                mesh.verts[vert_counter].co[0]+=pos[0]
                                mesh.verts[vert_counter].co[1]+=pos[1]
                                mesh.verts[vert_counter].co[2]+=pos[2]

	#build the armature in blender
        translationtable = string.maketrans("\\", "/")
        tempstring = string.translate(md5_filename, translationtable)
        lindex = string.rfind(tempstring, "/")
        rindex = string.rfind(tempstring, ".")
        if lindex==-1: lindex=0
        tempstring = string.rstrip(tempstring, ".md5mesh")
        tempstring = tempstring[lindex+1:len(tempstring)]
        armObj = Object.New('Armature', tempstring)
        armData = Blender.Armature.Armature("MD5_ARM") 
        armData.drawAxes = True 
        armObj.link(armData) 
        scene = Blender.Scene.getCurrent() 
        scene.link(armObj) 
        armData.makeEditable() 
        
        for bone in md5_bones: 
            bone.blenderbone = Blender.Armature.Editbone() 
            headData = Blender.Mathutils.Vector(bone.bindpos[0]*scale, bone.bindpos[1]*scale, bone.bindpos[2]*scale) 
            bone.blenderbone.head = headData 
            tailData = Blender.Mathutils.Vector(bone.bindpos[0]*scale+bonesize*scale*bone.bindmat[1][0], bone.bindpos[1]*scale+bonesize*scale*bone.bindmat[1][1], bone.bindpos[2]*scale+bonesize*scale*bone.bindmat[1][2]) 
            bone.blenderbone.tail = tailData 
            if bone.parent != "": 
                bone.blenderbone.parent = md5_bones[bone.parent_index].blenderbone 

            #rotate the bones correctly through this HACK (yeah it's ridiculous ;) 
            #could probably be optimized a bit... 
            boneisaligned=False 
            for i in range(0, 359): 
                boneisaligned=False 
                m = Blender.Mathutils.Matrix(bone.blenderbone.matrix) 
                mb = bone.bindmat 
                cos = vector_dotproduct(vector_normalize(m[0]), vector_normalize(mb[0])) 
                if cos > 0.9999: 
                    boneisaligned=True 
                    #print m[0], mb[0] 
                    break 
                bone.blenderbone.roll = i 
            if not boneisaligned: 
                print "Eeek!! ", bone.name, boneisaligned 
            armData.bones[bone.name]=bone.blenderbone 
        armData.update() 
        armObj.makeDisplayList() 
        scene.update(); 
        Blender.Window.RedrawAll()


	#dump the meshes into blender
        for mesh in md5_model:
                print "adding mesh ", mesh.mesh_index, " to blender"
                #print "it has ", len(mesh.verts), "verts"
                blender_mesh=NMesh.New()
                for vert in mesh.verts:
                        v=NMesh.Vert(vert.co[0]*scale, vert.co[1]*scale, vert.co[2]*scale)
                        #add the uv coords to the vertex
                        v.uvco[0]=vert.uvco[0]
                        v.uvco[1]=vert.uvco[1]
                        #add the vertex to the blender mesh
                        blender_mesh.verts.append(v)

                if(mesh.shader!=""):
                        path=mesh.shader.split("\\")
                        path=""+path[len(path)-1] #we should use the Doom3 basepath here
                        if os.path.isfile(path):
                                print "shader: ", path
                                mesh_image=Blender.Image.Load(path)
                                print "loaded :", mesh_image
                        else:
                                print "couldn't find image for: ", mesh.shader
                                mesh_image=None
                else:
                        mesh_image=None

                for tri in mesh.tris:
                        f=NMesh.Face()
                        #tell it which blender verts to use for faces
                        f.v.append(blender_mesh.verts[tri.vert_index[0]])
                        f.v.append(blender_mesh.verts[tri.vert_index[2]])
                        f.v.append(blender_mesh.verts[tri.vert_index[1]])
                        f.uv.append((blender_mesh.verts[tri.vert_index[0]].uvco[0],blender_mesh.verts[tri.vert_index[0]].uvco[1]))
                        f.uv.append((blender_mesh.verts[tri.vert_index[2]].uvco[0],blender_mesh.verts[tri.vert_index[2]].uvco[1]))
                        f.uv.append((blender_mesh.verts[tri.vert_index[1]].uvco[0],blender_mesh.verts[tri.vert_index[1]].uvco[1]))
                        f.smooth = 1 # smooth the face, since md5 has smoothing for all faces. There's no such thing as smoothgroups in an md5.
                        if mesh_image!=None:
                                f.image=mesh_image
                        #add the vace
                        blender_mesh.faces.append(f)
                #blender_mesh.hasVertexUV(1)


		#build the vertex groups from the bone names
		#loop through the verts and if they are influced by a bone, then they are a member of that vertex group
		#percentage by weight bias

		#put the object into blender
                if 0: #mesh.shader!="":  # NMesh.PutRaw() will fail if the given name is already used for another mesh...
                    translationtable = string.maketrans("\\", "/")
                    tempstring = string.translate(mesh.shader, translationtable)
                    lindex = string.rfind(tempstring, "/")
                    tempstring = string.rstrip(tempstring, ".tga")
                    if lindex==-1: lindex=0
                    tempstring = tempstring[lindex+1:len(tempstring)]
                    mesh_obj=NMesh.PutRaw(blender_mesh, tempstring)
                else:
                    mesh_obj=NMesh.PutRaw(blender_mesh)
		# this line would put it to the cursor location
		#cursor_pos=Blender.Window.GetCursorPos()
		#mesh_obj.setLocation(float(cursor_pos[0]),float(cursor_pos[1]),float(cursor_pos[2]))

                blender_mesh = mesh_obj.getData() # this seems unnecessary but the Blender documentation recommends it
                if mesh.shader!="":
                    translationtable = string.maketrans("\\", "/")
                    tempstring = string.translate(mesh.shader, translationtable)
                    lindex = string.rfind(tempstring, "/")
                    tempstring = string.rstrip(tempstring, ".tga")
                    if lindex==-1: lindex=0
                    tempstring = tempstring[lindex+1:len(tempstring)]
                    mesh_obj.setName(tempstring)

                #vertgroup_created is an array that stores for each bone if the vertex group was already created.
                # it's used to speed up the weight creation. The alternative would have been to use NMesh.getVertGroupNames()
                vertgroup_created=[]
                for b in md5_bones:
                    vertgroup_created.append(0)

                for vert in mesh.verts:
                    weight_index=vert.blend_index
                    for weight_counter in range(vert.blend_count):
                        #get the current weight info
                        w=mesh.weights[weight_index+weight_counter]
                        #check if vertex group was already created
                        if vertgroup_created[w.bone_index]==0:
                            vertgroup_created[w.bone_index]=1
                            blender_mesh.addVertGroup(md5_bones[w.bone_index].name)
                        #assign the weight for this vertex
                        blender_mesh.assignVertsToGroup(md5_bones[w.bone_index].name, [vert.vert_index], w.bias, 'replace')

                armObj.makeParentDeform([mesh_obj], 0, 0)
        return armObj

class md5anim_bone:
    name = ""
    parent_index = 0
    flags = 0
    frameDataIndex = 0
    bindpos = []
    bindquat = []
    #bindmat = []
    posemat = None #armature-space pose matrix, needed to import animation
    restmat = None
    invrestmat = None
    
    def __init__(self):
        name = ""
        self.bindpos=[0.0]*3
        self.bindquat=[0.0]*4
        #self.bindmat=[None]*3  #is this how you initilize a 2d-array
        #for i in range(3): self.bindmat[i] = [0.0]*3
        self.parent_index = 0
        self.flags = 0
        self.frameDataIndex = 0
        self.restmat = None
        self.invrestmat = None
        self.posemat = None

        
class md5anim:
    num_bones = 0
    md5anim_bones = []
    frameRate = 24
    numFrames = 0
    numAnimatedComponents = 0
    baseframe = []
    framedata = []

    def __init__(self):
        num_bones = 0
        md5anim_bones = []
        baseframe = []
        framedata = []
        
    def load_md5anim(self, md5_filename):
        file=open(md5_filename,"r")
        lines=file.readlines()
        file.close()

        num_lines=len(lines)

        for line_counter in range(0,num_lines):
            current_line=lines[line_counter]
            words=current_line.split()

            if words and words[0]=="numJoints":
                self.num_bones=int(words[1])
                print "num_bones: ", self.num_bones
                
            elif words and words[0]=="numFrames":
                self.numFrames=int(words[1])
                print "num_frames: ", self.numFrames
                #fill framedata array with numframes empty arrays
                self.framedata = [[]]*self.numFrames
                
            elif words and words[0]=="frameRate":
                self.frameRate=int(words[1])
                print "frameRate: ", self.frameRate
                
            elif words and words[0]=="numAnimatedComponents":
                self.numAnimatedComponents=int(words[1])
                print "numAnimatedComponents: ", self.numAnimatedComponents
                
            elif words and words[0]=="hierarchy":
                for bone_counter in range(0,self.num_bones):
                    #make a new bone
                    self.md5anim_bones.append(md5anim_bone())
		    #next line
                    line_counter+=1
                    current_line=lines[line_counter]
                    words=current_line.split()
                    #skip over blank lines
                    while not words:
                        line_counter+=1
                        current_line=lines[line_counter]
                        words=current_line.split()

                    #self.md5anim_bones[bone_counter].bone_index=bone_counter
                    #get rid of the quotes on either side
                    temp_name=str(words[0])
                    temp_name=temp_name[1:-1]
                    self.md5anim_bones[bone_counter].name=temp_name
                    print "found bone: ", self.md5anim_bones[bone_counter].name
                    self.md5anim_bones[bone_counter].parent_index = int(words[1])
                    #if self.md5anim_bones[bone_counter].parent_index>=0:
                    #    self.md5anim_bones[bone_counter].parent = self.md5anim_bones[self.md5anim_bones[bone_counter].parent_index].name
                    self.md5anim_bones[bone_counter].flags = int(words[2])
                    self.md5anim_bones[bone_counter].frameDataIndex=int(words[3])


            elif words and words[0]=="baseframe":
                for bone_counter in range(0,self.num_bones):
                    line_counter+=1
                    current_line=lines[line_counter]
                    words=current_line.split()
                    #skip over blank lines
                    while not words:
                        line_counter+=1
                        current_line=lines[line_counter]
                        words=current_line.split()
                    self.md5anim_bones[bone_counter].bindpos[0]=float(words[1])
                    self.md5anim_bones[bone_counter].bindpos[1]=float(words[2])
                    self.md5anim_bones[bone_counter].bindpos[2]=float(words[3])
                    qx = float(words[6])
                    qy = float(words[7])
                    qz = float(words[8])
                    qw = 1 - qx*qx - qy*qy - qz*qz
                    if qw<0:
                        qw=0
                    else:
                        qw = -sqrt(qw)
                    self.md5anim_bones[bone_counter].bindquat = [qx,qy,qz,qw]

            elif words and words[0]=="frame":
                framenumber = int(words[1])
                self.framedata[framenumber]=[]
                line_counter+=1
                current_line=lines[line_counter]
                words=current_line.split()
                while words and not(words[0]=="frame" or words[0]=="}"):
                    for i in range(0, len(words)):
                        self.framedata[framenumber].append(float(words[i]))
                    line_counter+=1
                    current_line=lines[line_counter]
                    words=current_line.split()

    def apply(self, arm_obj, actionname):
      action = Blender.Armature.NLA.NewAction(actionname)
      action.setActive(arm_obj)
      thepose = arm_obj.getPose()
      for b in self.md5anim_bones:
        b.invrestmat = Blender.Mathutils.Matrix(arm_obj.getData().bones[b.name].matrix['ARMATURESPACE']).invert()
        b.restmat = Blender.Mathutils.Matrix(arm_obj.getData().bones[b.name].matrix['ARMATURESPACE'])
      for currntframe in range(1, self.numFrames+1):
        print "importing frame ", currntframe," of", self.numFrames
        Blender.Set("curframe", currntframe)
        for md5b in self.md5anim_bones:
          try:
            thebone = thepose.bones[md5b.name]
          except:
            print "could not find bone ", md5b.name, " in armature"
            continue
          (qx,qy,qz,qw) = md5b.bindquat
          lx,ly,lz = md5b.bindpos
          frameDataIndex = md5b.frameDataIndex
          if (md5b.flags & 1):
            lx = self.framedata[currntframe-1][frameDataIndex]
            frameDataIndex+=1
          if (md5b.flags & 2):
            ly = self.framedata[currntframe-1][frameDataIndex]
            frameDataIndex+=1          
          if (md5b.flags & 4):
            lz = self.framedata[currntframe-1][frameDataIndex]
            frameDataIndex+=1
          if (md5b.flags & 8):
            qx = self.framedata[currntframe-1][frameDataIndex]
            frameDataIndex+=1
          if (md5b.flags & 16):
            qy = self.framedata[currntframe-1][frameDataIndex]
            frameDataIndex+=1                     
          if (md5b.flags & 32):
            qz = self.framedata[currntframe-1][frameDataIndex]
          qw = 1 - qx*qx - qy*qy - qz*qz
          if qw<0:
            qw=0
          else:
            qw = -sqrt(qw)
          lmat = quaternion2matrix([qx,qy,qz,qw])
          lmat[3][0] = lx*scale
          lmat[3][1] = ly*scale
          lmat[3][2] = lz*scale
          lmat = Blender.Mathutils.Matrix(lmat[0], lmat[1], lmat[2], lmat[3])
          #if md5b.parent_index>=0:
          #  md5b.posemat = lmat*self.md5anim_bones[md5b.parent_index].posemat
          #else:
          #  md5b.posemat = lmat
          if md5b.parent_index>=0:
            thebone.localMatrix = Blender.Mathutils.Matrix(lmat) * (md5b.restmat * self.md5anim_bones[md5b.parent_index].invrestmat).invert()
          else:
            thebone.localMatrix = lmat * md5b.invrestmat
          thepose.update()
          thebone.insertKey(arm_obj, currntframe, [Blender.Object.Pose.ROT, Blender.Object.Pose.LOC])
          thepose.update()
      Blender.Set("curframe", 1)
      

#armobj is either an armature object or None
def load_md5anim(md5anim_filename, armobj):
  theanim = md5anim()
  theanim.load_md5anim(md5anim_filename)
  if (armobj):
    obj = armobj
  else:
    obj = None
    for armobj in Blender.Object.Get():
      data = armobj.getData()
      if type(data) is Blender.Types.ArmatureType:
        obj = armobj
        break
  if obj:
    print "applying animation to armature: ", obj.getName()
    pth, actionname = os.path.split(md5anim_filename)
    theanim.apply(obj, actionname)
    scn = Blender.Scene.GetCurrent()
    context = scn.getRenderingContext()
    context.endFrame(theanim.numFrames+1)
  else:
    print "couldn't apply animation, no armature in the scene"    
  return

######################################################
# GUI STUFF
######################################################

draw_busy_screen = 0
EVENT_NOEVENT = 1
EVENT_IMPORT = 2
EVENT_QUIT = 3
EVENT_MESHFILENAME = 4
EVENT_ANIMFILENAME = 5
EVENT_MESHFILENAME_STRINGBUTTON = 6
EVENT_ANIMFILENAME_STRINGBUTTON = 7
md5mesh_filename = Blender.Draw.Create("")
md5anim_filename = Blender.Draw.Create("")


scale_slider = Blender.Draw.Create(1.0)
scale = 1.0
bonesize_slider = Blender.Draw.Create(3.0)
bonesize = 3.0

######################################################
# Callbacks for Window functions
######################################################
def md5meshname_callback(filename):
  global md5mesh_filename
  md5mesh_filename.val=filename

def md5animname_callback(filename):
  global md5anim_filename
  md5anim_filename.val=filename

def md5camanimname_callback(filename):
  global md5camanim_filename
  md5camanim_filename.val=filename
  
######################################################
# GUI Functions
######################################################
def handle_event(evt, val):
  if evt == Blender.Draw.ESCKEY:
    Blender.Draw.Exit()
    return

def handle_button_event(evt):
  global EVENT_NOEVENT, EVENT_IMPORT, EVENT_QUIT, EVENT_MESHFILENAME, EVENT_ANIMFILENAME, EVENT_MESHFILENAME_STRINGBUTTON, EVENT_ANIMFILENAME_STRINGBUTTON
  global draw_busy_screen, md5mesh_filename, md5anim_filename, scale_slider, scale, bonesize_slider, bonesize
  if evt == EVENT_IMPORT:
    scale = scale_slider.val
    bonesize = bonesize_slider.val
    draw_busy_screen = 1
    Blender.Draw.Draw()
    if len(md5mesh_filename.val)>0:
      armObj = load_md5(md5mesh_filename.val)
      if len(md5anim_filename.val)>0:
        load_md5anim(md5anim_filename.val, armObj) #load anim onto newly imported skel
    else:
      if len(md5anim_filename.val)>0:
        armObj = Blender.Scene.GetCurrent().getActiveObject()
        if (armObj):
          data = armObj.getData()
          if not (type(data) is Blender.Types.ArmatureType):
            armObj = None
        load_md5anim(md5anim_filename.val, armObj)
          

    draw_busy_screen = 0
    Blender.Draw.Redraw(1)
    return
  if evt == EVENT_QUIT:
    Blender.Draw.Exit()
  if evt == EVENT_MESHFILENAME:
    Blender.Window.FileSelector(md5meshname_callback, "Select md5mesh file...")
    Blender.Draw.Redraw(1)
  if evt == EVENT_ANIMFILENAME:
    Blender.Window.FileSelector(md5animname_callback, "Select md5anim file...")
    Blender.Draw.Redraw(1)

def show_gui():
  global EVENT_NOEVENT, EVENT_IMPORT, EVENT_QUIT, EVENT_MESHFILENAME, EVENT_ANIMFILENAME, EVENT_MESHFILENAME_STRINGBUTTON, EVENT_ANIMFILENAME_STRINGBUTTON
  global draw_busy_screen, md5mesh_filename, md5anim_filename, scale_slider, bonesize_slider
  global startframe_slider, endframe_slider
  button_width = 240
  browsebutton_width = 60
  button_height = 25
  if draw_busy_screen == 1:
    Blender.BGL.glClearColor(0.3,0.3,0.3,1.0)
    Blender.BGL.glClear(Blender.BGL.GL_COLOR_BUFFER_BIT)
    Blender.BGL.glColor3f(1,1,1)
    Blender.BGL.glRasterPos2i(20,25)
    Blender.Draw.Text("Please wait...")
    return
  Blender.BGL.glClearColor(0.6,0.6,0.6,1.0)
  Blender.BGL.glClear(Blender.BGL.GL_COLOR_BUFFER_BIT)
  Blender.Draw.Button("Import!", EVENT_IMPORT, 20, 2*button_height, button_width, button_height, "Start the MD5-import")
  Blender.Draw.Button("Quit", EVENT_QUIT, 20, button_height, button_width, button_height, "Quit this script")
  Blender.Draw.Button("Browse...", EVENT_MESHFILENAME, 21+button_width-browsebutton_width, 4*button_height, browsebutton_width, button_height, "Specify md5mesh-file")
  Blender.Draw.Button("Browse...", EVENT_ANIMFILENAME, 21+button_width-browsebutton_width, 3*button_height, browsebutton_width, button_height, "Specify md5anim-file")
  md5mesh_filename = Blender.Draw.String("MD5Mesh file:", EVENT_MESHFILENAME_STRINGBUTTON, 20, 4*button_height, button_width-browsebutton_width, button_height, md5mesh_filename.val, 255, "MD5Mesh-File to import")
  md5anim_filename = Blender.Draw.String("MD5Anim file:", EVENT_ANIMFILENAME_STRINGBUTTON, 20, 3*button_height, button_width-browsebutton_width, button_height, md5anim_filename.val, 255, "MD5Anim-File to import")
  scale_slider = Blender.Draw.Slider("Scale:", EVENT_NOEVENT, 20, 6*button_height, button_width, button_height, scale_slider.val, 0.01, 10.0, 0, "Adjust the size factor of the imported object")
  bonesize_slider = Blender.Draw.Slider("Bonesize:", EVENT_NOEVENT, 20, 7*button_height, button_width, button_height, bonesize_slider.val, 0.01, 10.0, 0, "Adjust the size (length) of the bones")
  

Blender.Draw.Register (show_gui, handle_event, handle_button_event)
