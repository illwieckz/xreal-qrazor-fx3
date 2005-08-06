/// ============================================================================
/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2004 Robert Beckebans <trebor_7@users.sourceforge.net>
Please see the file "AUTHORS" for a list of contributors

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
/// ============================================================================
#ifndef CM_H
#define CM_H


/// includes ===================================================================
// system -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
#include "files.h"
// xreal --------------------------------------------------------------------
// shared -------------------------------------------------------------------
#include "x_shared.h"
//#include "x_ode.h"

//
// public interface
//
//d_bsp_c*		CM_BeginRegistration(const std::string &name, bool clientload, unsigned *checksum, dSpaceID space);
cmodel_c*		CM_BeginRegistration(const std::string &name, bool clientload, unsigned *checksum);
cmodel_c*		CM_RegisterModel(const std::string &name);
animation_c*		CM_RegisterAnimation(const std::string &name);
void			CM_EndRegistration();

cmodel_c*		CM_GetModelByName(const std::string &name);
cmodel_c*		CM_GetModelByNum(int num);

animation_c*		CM_GetAnimationByName(const std::string &name);
animation_c*		CM_GetAnimationByNum(int num);

//int			CM_LeafContents(int leafnum);
//int			CM_LeafCluster(int leafnum);
//int			CM_LeafArea(int leafnum);

int			CM_NumModels();

//const char*		CM_EntityString();

// creates a clipping hull for an arbitrary box
cmodel_c*		CM_ModelForBox(const aabb_c& aabb);

// returns an ORed contents mask
//int			CM_PointContents(const vec3_c &p, const cmodel_c* model);
//int			CM_TransformedPointContents(const vec3_c &p, int headnode, const vec3_c &origin, const quaternion_c &quat);

//trace_t			CM_BoxTrace(const vec3_c &start, const vec3_c &end, const aabb_c &bbox, const cmodel_c* model, int brushmask);

/*	
trace_t			CM_TransformedBoxTrace(const vec3_c &start, const vec3_c &end,
						const aabb_c &bbox,
						const cmodel_c*, int brushmask, 
						const vec3_c &origin, const quaternion_c &quat);
*/
//int			CM_PointLeafnum(const vec3_c &p);
//int			CM_PointAreanum(const vec3_c &p);

// call with topnode set to the headnode, returns with topnode
// set to the first node that splits the box
//int			CM_BoxLeafnums(const aabb_c &bbox, std::vector<int> &list, int max);

// returns NULL if bad cluster
//byte*			CM_ClusterPVS(int cluster);

//int			CM_GetClosestAreaPortal(const vec3_c &p);
//bool			CM_GetAreaPortalState(int portal);
//void			CM_SetAreaPortalState(int portal, bool open);
//bool			CM_AreasConnected(int area1, int area2);

void			CM_WriteAreaBits(boost::dynamic_bitset<byte> &bits, int area);
void			CM_MergeAreaBits(byte *buffer, int area);

//bool			CM_HeadnodeVisible(int headnode, byte *visbits);

#endif	// CM_H


