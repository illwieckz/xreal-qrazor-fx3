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

some useful collision utility stuff.

*/

#ifndef ODE_COLLISION_UTIL_H
#define ODE_COLLISION_UTIL_H

#include "common.h"
#include "contact.h"


// returns true if contact was added
bool	dAddContact(const dContact c, std::vector<dContact> &contacts);

// given a vector p get barycentric coordinates and distance to p
void	dGetContactData(const vec3_c& p, const vec3_c &v0,  const vec3_c &edge0, const vec3_c &edge1, vec_t &dist, float &u, float &v);

// returns true if p is inside the triangle vertices
bool	dPointInTriangle(const vec3_c &p, const vec3_c &v0, const vec3_c &v1, const vec3_c &v2, const vec3_c &normal, bool cw);

// given a pointer `p' to a dContactGeom, return the dContactGeom at
// p + skip bytes.
//#define CONTACT(p,skip) ((dContactGeom*) (((char*)p) + (skip)))


// if the spheres (p1,r1) and (p2,r2) collide, set the contact `c' and
// return true, else return false.
bool	dCollideSpheres(const vec3_c & p1, vec_t r1, const vec3_c & p2, vec_t r2, dContactGeom &c);


// given two lines
//    qa = pa + alpha* ua
//    qb = pb + beta * ub
// where pa,pb are two points, ua,ub are two unit length vectors, and alpha,
// beta go from [-inf,inf], return alpha and beta such that qa and qb are
// as close as possible

void dLineClosestApproach (const dVector3 pa, const dVector3 ua,
			   const dVector3 pb, const dVector3 ub,
			   vec_t *alpha, vec_t *beta);


// given a line segment p1-p2 and a box (center 'c', rotation 'R', side length
// vector 'side'), compute the points of closest approach between the box
// and the line. return these points in 'lret' (the point on the line) and
// 'bret' (the point on the box). if the line actually penetrates the box
// then the solution is not unique, but only one solution will be returned.
// in this case the solution points will coincide.

void dClosestLineBoxPoints (const dVector3 p1, const dVector3 p2,
			    const dVector3 c, const matrix_c &R,
			    const dVector3 side,
			    dVector3 lret, dVector3 bret);

#endif
