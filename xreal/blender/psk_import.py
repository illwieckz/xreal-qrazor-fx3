#!BPY

"""
Name: 'Unreal Skeletal Mesh (.psk)'
Blender: 233a
Group: 'Import'
Tooltip: 'Unreal Skeletal Mesh Import (*.psk)' 
"""

##Unreal Tournament PSK file to Blender mesh converter V1.0
##
## * Imports a *psk file to a new mesh
## * Smoothing groups are shown as different material colors, faces with no smoothing will be white
## * Bone influences are shown as vertex colors
## **  This version DOES NOT import armature
## 
## *Tested with Blender V2.33a
##
##Author: D.M. Sturgeon (camg188 at the elYsium forum)

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

##USAGE#############################################
#Set the input file name in the "infile" variable.
#Run the script (alt-p). 
#Select an imput psk file. The script may pause a minute or two while it is working.
#UT2004 includes psk files on the play disc in:
#  D:\Extras\SkinArt\Meshes
#-A new mesh will be created.  It will be very large in Blender units.
#Set to "Object Mode".
#View as "Solid" to see faces and smoothing groups
#View as "Shaded" to see the smoothing groups clearly
#Set to "Vertex Paint", the "v" key, to view bone influences
#
#Armature bone names are output to the log file (C:\psk2blendermesh.log)
#
##TO DO ############################################
##Include armature importation to make this a true importer
## - Anyone who is good with Quat,Loc to Head,Tail,Roll coversion
##   could do it.
####################################################

import Blender
import os
import sys
import string
from string import *
from Blender import NMesh, Material, Window
from struct import *

#####################################################
def pskimport(infile):
  print "Importing file: ", infile
  pskfile = file(infile,'rb')
  logf = file('C:\\psk2blendermesh.log','w')
  #
  Tmsh = NMesh.GetRaw()
  objName = infile.split('\\')[-1].split('.')[0]
  Tmsh.name = objName
  print >> logf, "New Mesh = ",Tmsh.name
  
  #read general header
  indata = unpack('20s3i',pskfile.read(32))
  #not using the general header at this time
  
  #read the PNTS0000 header
  indata = unpack('20s3i',pskfile.read(32))
  recCount = indata[3]
  print >> logf, "Nbr of PNTS0000 records: ", recCount
  counter = 0
  while counter < recCount:
    counter = counter + 1
    indata = unpack('3f',pskfile.read(12))
    Tmsh.verts.append(NMesh.Vert(indata[0],indata[1],indata[2]))
  
  #read the VTXW0000 header
  indata = unpack('20s3i',pskfile.read(32))
  recCount = indata[3]
  print >> logf, "Nbr of VTXW0000 records: ", recCount
  counter = 0
  UVCoords = []
  #UVCoords record format = [index to PNTS, U coord, v coord]
  #We need to temporarily store this info to combine it with face data
  # from the next section of the psk file.
  while counter < recCount:
    counter = counter + 1
    indata = unpack('hhffhh',pskfile.read(16))
    UVCoords.append([indata[0],indata[2],indata[3]])
  
  #read the FACE0000 header
  indata = unpack('20s3i',pskfile.read(32))
  recCount = indata[3]
  print >> logf,"Nbr of FACE0000 records: ", recCount
  
  #PSK FACE0000 fields: WdgIdx1|WdgIdx2|WdgIdx3|MatIdx|AuxMatIdx|SmthGrp
  #associate MatIdx to an image, associate SmthGrp to a material
  SGlist = []
  counter = 0
  while counter < recCount:
    counter = counter + 1
    indata = unpack('hhhbbi',pskfile.read(12))
    #the psk values are: nWdgIdx1|WdgIdx2|WdgIdx3|MatIdx|AuxMatIdx|SmthGrp
    #indata[0] = index of UVCoords
    #UVCoords[indata[0]]=[index to PNTS, U coord, v coord]
    #UVCoords[indata[0]][0] = index to PNTS
    PNTSA = UVCoords[indata[0]][0]
    PNTSB = UVCoords[indata[1]][0]
    PNTSC = UVCoords[indata[2]][0]
    Tmsh.faces.append(NMesh.Face([Tmsh.verts[PNTSA],Tmsh.verts[PNTSB],Tmsh.verts[PNTSC]]))
    #get the uv coords - UVCoords[indata[0]][1] = u0, UVCoords[indata[0]][2] = v0, UVCoords[indata[1]][1] = v1, etc
    u0 = UVCoords[indata[0]][1]
    v0 = UVCoords[indata[0]][2]
    u1 = UVCoords[indata[1]][1]
    v1 = UVCoords[indata[1]][2]
    u2 = UVCoords[indata[2]][1]
    v2 = UVCoords[indata[2]][2]
    #update the uv var of the last item in the Tmsh.faces list
    # which is the face just added above
    Tmsh.faces[-1].uv = [(u0,v0),(u1,v1),(u2,v2)]
    #collect a list of the smoothing groups
    if SGlist.count(indata[5]) == 0:
      SGlist.append(indata[5])
    #assign a material index to the face
    Tmsh.faces[-1].materialIndex = SGlist.index(indata[5])
    
  print >> logf, "Using Materials to represent PSK Smoothing Groups..."
  #create a material for each SmthGrp
  for x in range(len(SGlist)):
    MatName = "SmthGrp"+str(SGlist[x])
    if SGlist[x] == 0:
      MatName = 'SmthGrpNone'
    newMat = Material.New(MatName)
    newMat.mode |= Material.Modes.SHADELESS
    #change the overall darkness of each material in a range between 0.1 and 0.9
    tmpVal = ((float(x)+1.0)/(len(SGlist))*0.7)+0.1
    #set no smthgrp to light gray
    if SGlist[x] == 0:  tmpVal = 0.9
    newMat.R = tmpVal
    newMat.G = tmpVal
    newMat.B = tmpVal
    #Change the color of each material slightly
    if SGlist[x] != 0:
      if x % 3 == 0:
        if newMat.R < 0.5: newMat.R += 0.25
        else: newMat.R -= 0.25
      if x % 3 == 1:
        if newMat.G < 0.5: newMat.G += 0.25
        else: newMat.G -= 0.25
      if x % 3 == 2:
        if newMat.B < 0.5: newMat.B += 0.25
        else: newMat.B -= 0.25
    #Add the material to the mesh
    Tmsh.materials.append(newMat)
  
  ##
  #read the MATT0000 header
  indata = unpack('20s3i',pskfile.read(32))
  recCount = indata[3]
  print >> logf,"Nbr of MATT0000 records: ", recCount
  print >> logf," - Not importing any material data now. PSKs are texture wrapped!"
  counter = 0
  while counter < recCount:
    counter = counter + 1
    indata = unpack('64s6i',pskfile.read(88))
  
  ##
  #read the REFSKEL0 header
  indata = unpack('20s3i',pskfile.read(32))
  recCount = indata[3]
  print >> logf, "Nbr of REFSKEL0 records: ", recCount
  #REFSKEL0 fields - Name|Flgs|NumChld|PrntIdx|Qw|Qx|Qy|Qz|LocX|LocY|LocZ|Lngth|XSize|YSize|ZSize
  Bns = []
  counter = 0
  while counter < recCount:
    counter = counter + 1
    indata = unpack('64s3i11f',pskfile.read(120))
    #get the bone names
    bnstr = rstrip(indata[0][:30])
    Bns.append(bnstr)
    print >> logf, ("idx = %i; name = %s" % (counter,bnstr))
  
  ##TODO - Add import of Armature
  
  ##
  #print "Using VertexColors to represent PSK Vertex Influences..."
  #create a color for each bone
  VtxCol = []
  for x in range(len(Bns)):
    #change the overall darkness of each material in a range between 0.1 and 0.9
    tmpVal = ((float(x)+1.0)/(len(Bns))*0.7)+0.1
    tmpVal = int(tmpVal * 256)
    tmpCol = [tmpVal,tmpVal,tmpVal,0]
    #Change the color of each material slightly
    if x % 3 == 0:
      if tmpCol[0] < 128: tmpCol[0] += 60
      else: tmpCol[0] -= 60
    if x % 3 == 1:
      if tmpCol[1] < 128: tmpCol[1] += 60
      else: tmpCol[1] -= 60
    if x % 3 == 2:
      if tmpCol[2] < 128: tmpCol[2] += 60
      else: tmpCol[2] -= 60
    #Add the material to the mesh
    VtxCol.append(tmpCol)
  
  #read the RAWW0000 header
  indata = unpack('20s3i',pskfile.read(32))
  recCount = indata[3]
  print >> logf, "Nbr of RAWW0000 records: ", recCount
  #RAWW0000 fields: Weight|PntIdx|BoneIdx
  RWghts = []
  counter = 0
  while counter < recCount:
    counter = counter + 1
    indata = unpack('fii',pskfile.read(12))
    RWghts.append([indata[1],indata[2],indata[0]])
  #RWghts fields = PntIdx|BoneIdx|Weight
  RWghts.sort()
  print >> logf, "len(RWghts)=",len(RWghts)
  
  Tmsh.update()
  
  ##set the Vertex Colors of the faces
  ##face.v[n] = RWghts[0]
  ##RWghts[1] = index of VtxCol
  for x in range(len(Tmsh.faces)):
    for y in range(len(Tmsh.faces[x].v)):
      #find v in RWghts[n][0]
      findVal = Tmsh.faces[x].v[y].index
      n = 0
      while findVal != RWghts[n][0]:
        n = n + 1
      TmpCol = VtxCol[RWghts[n][1]]
      #check if a vertex has more than one influence
      if n != len(RWghts)-1:
        if RWghts[n][0] == RWghts[n+1][0]:
          #if there is more than one influence, use the one with the greater influence
          #for simplicity only 2 influences are checked, 2nd and 3rd influences are usually very small
          if RWghts[n][2] < RWghts[n+1][2]:
            TmpCol = VtxCol[RWghts[n+1][1]]
      Tmsh.faces[x].col.append(NMesh.Col(TmpCol[0],TmpCol[1],TmpCol[2],0))
  ##
  
  pskfile.close()
  logf.close()
  
  Tmsh.update()
  NMesh.PutRaw(Tmsh,objName)
  Blender.Window.RedrawAll()
  
  print "PSK2Blender completed"
##End of def pskimport#########################

def getInputFilename(filename):
  checktype = filename.split('\\')[-1].split('.')[1]
  if checktype.upper() != 'PSK':
    print "  Selected file = ",filename
    raise IOError, "The selected input file is not a *.psk file"
  pskimport(filename)


Blender.Window.FileSelector(getInputFilename, 'SELECT A PSK FILE')
