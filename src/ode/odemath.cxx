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

#include "common.h"
#include "odemath.h"

// get some math functions under windows
#ifdef WIN32
#include <float.h>
#ifndef CYGWIN			// added by andy for cygwin
#define copysign(a,b) ((vec_t)_copysign(a,b))
#endif				// added by andy for cygwin
#endif


// this may be called for vectors `a' with extremely small magnitude, for
// example the result of a cross product on two nearly perpendicular vectors.
// we must be robust to these small vectors. to prevent numerical error,
// first find the component a[i] with the largest magnitude and then scale
// all the components by 1/a[i]. then we can compute the length of `a' and
// scale the components by 1/l. this has been verified to work with vectors
// containing the smallest representable numbers.

void dNormalize3 (dVector3 a)
{
  vec_t a0,a1,a2,aa0,aa1,aa2,l;
  dAASSERT (a);
  a0 = a[0];
  a1 = a[1];
  a2 = a[2];
  aa0 = X_fabs(a0);
  aa1 = X_fabs(a1);
  aa2 = X_fabs(a2);
  if (aa1 > aa0) {
    if (aa2 > aa1) {
      goto aa2_largest;
    }
    else {		// aa1 is largest
      a0 /= aa1;
      a2 /= aa1;
      l = X_recipsqrt (a0*a0 + a2*a2 + 1);
      a[0] = a0*l;
      a[1] = X_copysign(l,a1);
      a[2] = a2*l;
    }
  }
  else {
    if (aa2 > aa0) {
      aa2_largest:	// aa2 is largest
      a0 /= aa2;
      a1 /= aa2;
      l = X_recipsqrt (a0*a0 + a1*a1 + 1);
      a[0] = a0*l;
      a[1] = a1*l;
      a[2] = X_copysign(l,a2);
    }
    else {		// aa0 is largest
      if (aa0 <= 0) {
	// dDEBUGMSG ("vector has zero size"); ... this messace is annoying
	a[0] = 1;	// if all a's are zero, this is where we'll end up.
	a[1] = 0;	// return a default unit length vector.
	a[2] = 0;
	return;
      }
      a1 /= aa0;
      a2 /= aa0;
      l = X_recipsqrt (a1*a1 + a2*a2 + 1);
      a[0] = X_copysign(l,a0);
      a[1] = a1*l;
      a[2] = a2*l;
    }
  }
}


/* OLD VERSION */
/*
void dNormalize3 (dVector3 a)
{
  dASSERT (a);
  vec_t l = dDOT(a,a);
  if (l > 0) {
    l = dRecipSqrt(l);
    a[0] *= l;
    a[1] *= l;
    a[2] *= l;
  }
  else {
    a[0] = 1;
    a[1] = 0;
    a[2] = 0;
  }
}
*/


void dNormalize4 (dVector4 a)
{
  dAASSERT (a);
  vec_t l = dDOT(a,a)+a[3]*a[3];
  if (l > 0) {
    l = X_recipsqrt(l);
    a[0] *= l;
    a[1] *= l;
    a[2] *= l;
    a[3] *= l;
  }
  else {
    // dDEBUGMSG ("vector has zero size");
    a[0] = 1;
    a[1] = 0;
    a[2] = 0;
    a[3] = 0;
  }
}


void dPlaneSpace (const dVector3 n, dVector3 p, dVector3 q)
{
  dAASSERT (n && p && q);
  if (X_fabs(n[2]) > M_SQRT1_2) {
    // choose p in y-z plane
    vec_t a = n[1]*n[1] + n[2]*n[2];
    vec_t k = X_recipsqrt (a);
    p[0] = 0;
    p[1] = -n[2]*k;
    p[2] = n[1]*k;
    // set q = n x p
    q[0] = a*k;
    q[1] = -n[0]*p[2];
    q[2] = n[0]*p[1];
  }
  else {
    // choose p in x-y plane
    vec_t a = n[0]*n[0] + n[1]*n[1];
    vec_t k = X_recipsqrt (a);
    p[0] = -n[1]*k;
    p[1] = n[0]*k;
    p[2] = 0;
    // set q = n x p
    q[0] = -n[2]*p[1];
    q[1] = n[2]*p[0];
    q[2] = a*k;
  }
}


vec_t dPlaneDistance (const dVector4 p, const dVector3 v)
{
  dAASSERT (p && v);
  vec_t d = dDOT (p, v) + p[3];
  return d;
}

int dVertexOnPlaneSide (const dVector4 p, const dVector3 v)
{
  dAASSERT (p && v);
  vec_t d = dPlaneDistance (p, v);
  if (d >= 0) {
    return dPlaneSideFront;
  }
  else {
    return dPlaneSideBack;
  }
}

/*
 * given an axis aligned bounding box tell us on which side of the plane it is
 */
int dBoxOnPlaneSide (const dVector4 p, const vec_t aabb[6])
{
  dAASSERT (p && aabb);

  dVector3 corners[2];
  
  corners[0][0] = (p[0] < 0) ? aabb[0] : aabb[1];
  corners[0][1] = (p[1] < 0) ? aabb[2] : aabb[3];
  corners[0][2] = (p[2] < 0) ? aabb[4] : aabb[5];
  
  corners[1][0] = (p[0] < 0) ? aabb[1] : aabb[0];
  corners[1][1] = (p[1] < 0) ? aabb[3] : aabb[2];
  corners[1][2] = (p[2] < 0) ? aabb[5] : aabb[4];
  
  vec_t dist1 = dPlaneDistance (p, corners[0]);
  vec_t dist2 = dPlaneDistance (p, corners[1]);
  
  bool front = (dist1 >= 0);
  
  if(dist2 < 0) {
    if(front) {
      return dPlaneSideCross;
    }
    else {
      return dPlaneSideBack;
    }
  }
  else {
    return dPlaneSideFront;
  }
}
