/*************************************************************************
 *                                                                       *
 * Open Dynamics Engine, Copyright (C) 2001,2002 Russell L. Smith.       *
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

/*

the standard ODE geometry primitives.

*/

#ifndef ODE_COLLISION_STD_H
#define ODE_COLLISION_STD_H

#include "common.h"
#include "collision_kernel.h"


// primitive collision functions - these have the dColliderFn interface, i.e.
// the same interface as dCollide(). the first and second geom arguments must
// have the specified types.


int	dCollideSphereSphere(dGeomID o1, dGeomID o2, int flags, std::vector<dContact> &contacts);
int	dCollideSphereBox(dGeomID o1, dGeomID o2, int flags, std::vector<dContact> &contacts);
int	dCollideSpherePlane(dGeomID o1, dGeomID o2, int flags, std::vector<dContact> &contacts);

int	dCollideBoxBox(dGeomID o1, dGeomID o2, int flags, std::vector<dContact> &contacts);
int	dCollideBoxPlane(dGeomID o1, dGeomID o2, int flags, std::vector<dContact> &contacts);

int	dCollideCCylinderSphere(dGeomID o1, dGeomID o2, int flags, std::vector<dContact> &contacts);
int	dCollideCCylinderBox(dGeomID o1, dGeomID o2, int flags, std::vector<dContact> &contacts);
int	dCollideCCylinderCCylinder(dGeomID o1, dGeomID o2, int flags, std::vector<dContact> &contacts);
int	dCollideCCylinderPlane(dGeomID o1, dGeomID o2, int flags, std::vector<dContact> &contacts);

int	dCollideRaySphere(dGeomID o1, dGeomID o2, int flags, std::vector<dContact> &contacts);
int	dCollideRayBox(dGeomID o1, dGeomID o2, int flags, std::vector<dContact> &contacts);
int	dCollideRayCCylinder(dGeomID o1, dGeomID o2, int flags, std::vector<dContact> &contacts);
int	dCollideRayPlane(dGeomID o1, dGeomID o2, int flags, std::vector<dContact> &contacts);
int	dCollideRayBrush(dGeomID o1, dGeomID o2, int flags, std::vector<dContact> &contacts);

int	dCollideBSPSphere(dGeomID o1, dGeomID o2, int flags, std::vector<dContact> &contacts);
int	dCollideBSPBox(dGeomID o1, dGeomID o2, int flags, std::vector<dContact> &contacts);
int	dCollideBSPCCylinder(dGeomID o1, dGeomID o2, int flags, std::vector<dContact> &contacts);
int	dCollideBSPCylinder(dGeomID o1, dGeomID o2, int flags, std::vector<dContact> &contacts);
int	dCollideBSPPlane(dGeomID o1, dGeomID o2, int flags, std::vector<dContact> &contacts);
int	dCollideBSPRay(dGeomID o1, dGeomID o2, int flags, std::vector<dContact> &contacts);

#endif

