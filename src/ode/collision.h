/*************************************************************************
 *                                                                       *
 * Open Dynamics Engine, Copyright (C) 2001-2003 Russell L. Smith.       *
 * All rights reserved.  Email: russ@q12.org   Web: www.q12.org          *
 *                                                                       *
 * This library is free software; you can redistribute it and/or         *
 * modify it under the terms of EITHER:                                  *
 *   (1) The GNU Lesser General Public License as published by the Free  *
 *       Software Foundation; either version 2.1 of the License, or (at  *
 *       your option) any later version. The text of the GNU Lesser      *
 *       General Public License is included with this library in the     *
 *       file LICENSE.TXT.                                               *
 *   (2) The BSD-style license that is included with this library in     *
 *       the file LICENSE-BSD.TXT.                                       *
 *                                                                       *
 * This library is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files    *
 * LICENSE.TXT and LICENSE-BSD.TXT for more details.                     *
 *                                                                       *
 *************************************************************************/

#ifndef _ODE_COLLISION_H_
#define _ODE_COLLISION_H_

#include "common.h"
#include "collision_space.h"
#include "contact.h"

/* ************************************************************************ */
/* general functions */

void		dGeomDestroy(dGeomID);
void		dGeomSetData(dGeomID, void *);
void*		dGeomGetData(dGeomID);
void		dGeomSetBody(dGeomID, dBodyID);
dBodyID		dGeomGetBody(dGeomID);
void		dGeomSetPosition(dGeomID, vec_t x, vec_t y, vec_t z);
void		dGeomSetRotation(dGeomID, const matrix_c &R);
void		dGeomSetQuaternion(dGeomID, const quaternion_c &q);
const		vec_t* dGeomGetPosition(dGeomID);
//const		vec_t* dGeomGetRotation(dGeomID);
const quaternion_c&	dGeomGetQuaternion(dGeomID);
void		dGeomGetAABB(dGeomID, vec_t aabb[6]);
int		dGeomIsSpace(dGeomID);
dSpaceID	dGeomGetSpace(dGeomID);
int		dGeomGetClass(dGeomID);
void		dGeomSetCategoryBits(dGeomID, unsigned long bits);
void		dGeomSetCollideBits(dGeomID, unsigned long bits);
unsigned long	dGeomGetCategoryBits(dGeomID);
unsigned long	dGeomGetCollideBits(dGeomID);
void		dGeomEnable(dGeomID);
void		dGeomDisable(dGeomID);
int		dGeomIsEnabled(dGeomID);

/* ************************************************************************ */
/* collision detection */

int		dCollide(dGeomID o1, dGeomID o2, int flags, std::vector<dContact> &contacts);

void		dSpaceCollide(dSpaceID space, void *data, dNearCallback *callback);
void		dSpaceCollide2(dGeomID o1, dGeomID o2, void *data, dNearCallback *callback);

/* ************************************************************************ */
/* standard classes */

/* the maximum number of user classes that are supported */
//enum {
//  dMaxUserClasses = 4
//};

/* class numbers - each geometry object needs a unique number */
enum {
  dSphereClass = 0,
  dBoxClass,
  dCCylinderClass,
  dCylinderClass,
  dPlaneClass,
  dRayClass,
  dBSPClass,
  dGeomTransformClass,
#ifdef dTRIMESH_ENABLED
  dTriMeshClass,
#endif

  dFirstSpaceClass,
  dSimpleSpaceClass = dFirstSpaceClass,
  dHashSpaceClass,
  dQuadTreeSpaceClass,
  dLastSpaceClass = dQuadTreeSpaceClass,

//  dFirstUserClass,
//  dLastUserClass = dFirstUserClass + dMaxUserClasses - 1,
  dGeomNumClasses
};


dGeomID dCreateSphere (dSpaceID space, vec_t radius);
void dGeomSphereSetRadius (dGeomID sphere, vec_t radius);
vec_t dGeomSphereGetRadius (dGeomID sphere);
vec_t dGeomSpherePointDepth (dGeomID sphere, vec_t x, vec_t y, vec_t z);

dGeomID dCreateBox (dSpaceID space, vec_t lx, vec_t ly, vec_t lz);
void dGeomBoxSetLengths (dGeomID box, vec_t lx, vec_t ly, vec_t lz);
void dGeomBoxGetLengths (dGeomID box, dVector3 result);
vec_t dGeomBoxPointDepth (dGeomID box, vec_t x, vec_t y, vec_t z);

dGeomID dCreatePlane (dSpaceID space, vec_t a, vec_t b, vec_t c, vec_t d);
void dGeomPlaneSetParams (dGeomID plane, vec_t a, vec_t b, vec_t c, vec_t d);
void dGeomPlaneGetParams (dGeomID plane, dVector4 result);
vec_t dGeomPlanePointDepth (dGeomID plane, vec_t x, vec_t y, vec_t z);

dGeomID dCreateCCylinder (dSpaceID space, vec_t radius, vec_t length);
void dGeomCCylinderSetParams (dGeomID ccylinder, vec_t radius, vec_t length);
void dGeomCCylinderGetParams (dGeomID ccylinder, vec_t *radius, vec_t *length);
vec_t dGeomCCylinderPointDepth (dGeomID ccylinder, vec_t x, vec_t y, vec_t z);

dGeomID dCreateRay (dSpaceID space, vec_t length);
void dGeomRaySetLength (dGeomID ray, vec_t length);
vec_t dGeomRayGetLength (dGeomID ray);
void dGeomRaySet (dGeomID ray, vec_t px, vec_t py, vec_t pz,
		  vec_t dx, vec_t dy, vec_t dz);
void dGeomRayGet (dGeomID ray, dVector3 start, dVector3 dir);

/*
 * Set/get ray flags that influence ray collision detection.
 * These flags are currently only noticed by the trimesh collider, because
 * they can make a major differences there.
 */
void dGeomRaySetParams (dGeomID g, int FirstContact, int BackfaceCull);
void dGeomRayGetParams (dGeomID g, int *FirstContact, int *BackfaceCull);
void dGeomRaySetClosestHit (dGeomID g, int closestHit);
int dGeomRayGetClosestHit (dGeomID g);

dGeomID		dCreateBSP(dSpaceID space);
void		dGeomBSPSetLengths(dGeomID box, vec_t lx, vec_t ly, vec_t lz);
void		dGeomBSPAddPlane(dGeomID g, vec_t a, vec_t b, vec_t c, vec_t d);
//void		dGeomBSPAddBrush(dGeomID g, int sides_first, int sides_num);
//void		dGeomBSPAddBrushSide(dGeomID g, int plane_num);
//void		dGeomBSPAddLeafBrush(dGeomID g, int num);

void		dGeomBSPAddNode(dGeomID g, int plane_num, int child0, int child1);
void		dGeomBSPAddLeaf(dGeomID g, int surfaces_first, int surfaces_num, int brushes_first, int brushes_num, int cluster, int area);

void		dGeomBSPAddSurface(dGeomID g, int face_type, int shader_num, const std::vector<vec3_c> &vertexes, const std::vector<vec3_c> &normals, const std::vector<index_t> &indexes);
void		dGeomBSPAddLeafSurface(dGeomID g, int num);

#ifdef dTRIMESH_ENABLED
#include "collision_trimesh.h"
#endif

dGeomID dCreateGeomTransform (dSpaceID space);
void dGeomTransformSetGeom (dGeomID g, dGeomID obj);
dGeomID dGeomTransformGetGeom (dGeomID g);
void dGeomTransformSetCleanup (dGeomID g, int mode);
int dGeomTransformGetCleanup (dGeomID g);
void dGeomTransformSetInfo (dGeomID g, int mode);
int dGeomTransformGetInfo (dGeomID g);

/* ************************************************************************ */
/* utility functions */

void dClosestLineSegmentPoints (const dVector3 a1, const dVector3 a2,
				const dVector3 b1, const dVector3 b2,
				dVector3 cp1, dVector3 cp2);

int dBoxTouchesBox (const dVector3 _p1, const matrix_c &R1,
		    const dVector3 side1, const dVector3 _p2,
		    const matrix_c &R2, const dVector3 side2);

void dInfiniteAABB (dGeomID geom, vec_t aabb[6]);
void dCloseODE(void);

/* ************************************************************************ */
/* custom classes */

//typedef void dGetAABBFn (dGeomID, vec_t aabb[6]);
typedef int dColliderFn(dGeomID o1, dGeomID o2, int flags, std::vector<dContact> &contacts);
//typedef dColliderFn * dGetColliderFnFn (int num);
//typedef void dGeomDtorFn (dGeomID o);
//typedef int dAABBTestFn (dGeomID o1, dGeomID o2, vec_t aabb[6]);

/* ************************************************************************ */

#endif
