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

/*

standard ODE geometry primitives: public API and pairwise collision functions.

the rule is that only the low level primitive collision functions should set
dContactGeom::g1 and dContactGeom::g2.

*/

#include "common.h"
#include "collision.h"
#include "matrix.h"
#include "rotation.h"
#include "odemath.h"
#include "collision_kernel.h"
#include "collision_std.h"
#include "collision_util.h"

#ifdef _MSC_VER
#pragma warning(disable:4291)  // for VC++, no complaints about "no matching operator delete found"
#endif

//****************************************************************************
// the basic geometry objects

struct dxSphere : public dxGeom {
  vec_t radius;		// sphere radius
  dxSphere (dSpaceID space, vec_t _radius);
  void computeAABB();
};


struct dxBox : public dxGeom {
  dVector3 side;	// side lengths (x,y,z)
  dxBox (dSpaceID space, vec_t lx, vec_t ly, vec_t lz);
  void computeAABB();
};


struct dxCCylinder : public dxGeom {
  vec_t radius,lz;	// radius, length along z axis
  dxCCylinder (dSpaceID space, vec_t _radius, vec_t _length);
  void computeAABB();
};


struct dxPlane : public dxGeom {
  vec_t p[4];
  dxPlane (dSpaceID space, vec_t a, vec_t b, vec_t c, vec_t d);
  void computeAABB();
};


struct dxRay : public dxGeom {
  vec_t length;
  dxRay (dSpaceID space, vec_t _length);
  void computeAABB();
};


struct dxBSP : public dxGeom {
  struct dBSPNode {
    vec_t* plane;		// split plane
    int	children[2];		// negative numbers are leafs
  };
  
  struct dBSPLeaf {
    int brushes_first;
    int brushes_num;
    
    int patches_first;
    int patches_num;
  };
  
  struct dBSPBrush {
    int sides_first;
    int sides_num;
    
    int checkcount;
  };
  
  struct dBSPBrushSide {
    vec_t* plane;
  };

  dVector3		side;	// side lengths (x,y,z) for AABB
  dArray<dBSPNode>	nodes;
  dArray<dBSPLeaf>	leafs;
  dArray<dVector4>	planes;
  dArray<dBSPBrush>	brushes;
  dArray<dBSPBrushSide> brushsides;
  dArray<int>		leafbrushes;
  int 			checkcount;
  
  dxBSP (dSpaceID space);
  void computeAABB();
};


//****************************************************************************
// sphere public API

dxSphere::dxSphere (dSpaceID space, vec_t _radius) : dxGeom (space,1)
{
  dAASSERT (_radius > 0);
  type = dSphereClass;
  radius = _radius;
}


void dxSphere::computeAABB()
{
  aabb[0] = pos[0] - radius;
  aabb[1] = pos[0] + radius;
  aabb[2] = pos[1] - radius;
  aabb[3] = pos[1] + radius;
  aabb[4] = pos[2] - radius;
  aabb[5] = pos[2] + radius;
}


dGeomID dCreateSphere (dSpaceID space, vec_t radius)
{
  return new dxSphere (space,radius);
}


void dGeomSphereSetRadius (dGeomID g, vec_t radius)
{
  dUASSERT (g && g->type == dSphereClass,"argument not a sphere");
  dAASSERT (radius > 0);
  dxSphere *s = (dxSphere*) g;
  s->radius = radius;
  dGeomMoved (g);
}


vec_t dGeomSphereGetRadius (dGeomID g)
{
  dUASSERT (g && g->type == dSphereClass,"argument not a sphere");
  dxSphere *s = (dxSphere*) g;
  return s->radius;
}


vec_t dGeomSpherePointDepth (dGeomID g, vec_t x, vec_t y, vec_t z)
{
  dUASSERT (g && g->type == dSphereClass,"argument not a sphere");
  dxSphere *s = (dxSphere*) g;
  return s->radius - X_sqrt((x-s->pos[0])*(x-s->pos[0]) +
			    (y-s->pos[1])*(y-s->pos[1]) +
			    (z-s->pos[2])*(z-s->pos[2]));
}

//****************************************************************************
// box public API

dxBox::dxBox (dSpaceID space, vec_t lx, vec_t ly, vec_t lz) : dxGeom (space,1)
{
  dAASSERT (lx >= 0 && ly >= 0 && lz >= 0);
  type = dBoxClass;
  side[0] = lx;
  side[1] = ly;
  side[2] = lz;
}


void dxBox::computeAABB()
{
	vec_t xrange = REAL(0.5) * (X_fabs((*R)[0][0] * side[0]) + X_fabs((*R)[0][1] * side[1]) + X_fabs((*R)[0][2] * side[2]));
	vec_t yrange = REAL(0.5) * (X_fabs((*R)[1][0] * side[0]) + X_fabs((*R)[1][1] * side[1]) + X_fabs((*R)[1][2] * side[2]));
	vec_t zrange = REAL(0.5) * (X_fabs((*R)[2][0] * side[0]) + X_fabs((*R)[2][1] * side[1]) + X_fabs((*R)[2][2] * side[2]));
  
	aabb[0] = pos[0] - xrange;
	aabb[1] = pos[0] + xrange;
	aabb[2] = pos[1] - yrange;
	aabb[3] = pos[1] + yrange;
	aabb[4] = pos[2] - zrange;
	aabb[5] = pos[2] + zrange;
}


dGeomID dCreateBox (dSpaceID space, vec_t lx, vec_t ly, vec_t lz)
{
  return new dxBox (space,lx,ly,lz);
}


void dGeomBoxSetLengths (dGeomID g, vec_t lx, vec_t ly, vec_t lz)
{
  dUASSERT (g && g->type == dBoxClass,"argument not a box");
  dAASSERT (lx > 0 && ly > 0 && lz > 0);
  dxBox *b = (dxBox*) g;
  b->side[0] = lx;
  b->side[1] = ly;
  b->side[2] = lz;
  dGeomMoved (g);
}


void dGeomBoxGetLengths (dGeomID g, dVector3 result)
{
  dUASSERT (g && g->type == dBoxClass,"argument not a box");
  dxBox *b = (dxBox*) g;
  result[0] = b->side[0];
  result[1] = b->side[1];
  result[2] = b->side[2];
}


vec_t dGeomBoxPointDepth (dGeomID g, vec_t x, vec_t y, vec_t z)
{
  dUASSERT (g && g->type == dBoxClass,"argument not a box");
  dxBox *b = (dxBox*) g;

  // Set p = (x,y,z) relative to box center
  //
  // This will be (0,0,0) if the point is at (side[0]/2,side[1]/2,side[2]/2)

  dVector3 p,q;

  p[0] = x - b->pos[0];
  p[1] = y - b->pos[1];
  p[2] = z - b->pos[2];

  // Rotate p into box's coordinate frame, so we can
  // treat the OBB as an AABB

  dMULTIPLY1_331 (q, (vec_t*)*b->R, p);

  // Record distance from point to each successive box side, and see
  // if the point is inside all six sides

  vec_t dist[6];
  int   i;

  bool inside = true;

  for (i=0; i < 3; i++) {
    vec_t side = b->side[i] * REAL(0.5);

    dist[i  ] = side - q[i];
    dist[i+3] = side + q[i];

    if ((dist[i] < 0) || (dist[i+3] < 0)) {
      inside = false;
    }
  }

  // If point is inside the box, the depth is the smallest positive distance
  // to any side

  if (inside) {
    vec_t smallest_dist = (vec_t) (unsigned) -1;

    for (i=0; i < 6; i++) {
      if (dist[i] < smallest_dist) smallest_dist = dist[i];
    }

    return smallest_dist;
  }

  // Otherwise, if point is outside the box, the depth is the largest
  // distance to any side.  This is an approximation to the 'proper'
  // solution (the proper solution may be larger in some cases).

  vec_t largest_dist = 0;

  for (i=0; i < 6; i++) {
    if (dist[i] > largest_dist) largest_dist = dist[i];
  }

  return -largest_dist;
}

//****************************************************************************
// capped cylinder public API

dxCCylinder::dxCCylinder (dSpaceID space, vec_t _radius, vec_t _length) :
  dxGeom (space,1)
{
  dAASSERT (_radius > 0 && _length > 0);
  type = dCCylinderClass;
  radius = _radius;
  lz = _length;
}


void dxCCylinder::computeAABB()
{
  vec_t xrange = X_fabs((*R)[0][2] * lz) * REAL(0.5) + radius;
  vec_t yrange = X_fabs((*R)[1][2] * lz) * REAL(0.5) + radius;
  vec_t zrange = X_fabs((*R)[2][2] * lz) * REAL(0.5) + radius;
  
  aabb[0] = pos[0] - xrange;
  aabb[1] = pos[0] + xrange;
  aabb[2] = pos[1] - yrange;
  aabb[3] = pos[1] + yrange;
  aabb[4] = pos[2] - zrange;
  aabb[5] = pos[2] + zrange;
}


dGeomID dCreateCCylinder (dSpaceID space, vec_t radius, vec_t length)
{
  return new dxCCylinder (space,radius,length);
}


void dGeomCCylinderSetParams (dGeomID g, vec_t radius, vec_t length)
{
  dUASSERT (g && g->type == dCCylinderClass,"argument not a ccylinder");
  dAASSERT (radius > 0 && length > 0);
  dxCCylinder *c = (dxCCylinder*) g;
  c->radius = radius;
  c->lz = length;
  dGeomMoved (g);
}


void dGeomCCylinderGetParams (dGeomID g, vec_t *radius, vec_t *length)
{
  dUASSERT (g && g->type == dCCylinderClass, "argument not a ccylinder");
  dxCCylinder *c = (dxCCylinder*) g;
  *radius = c->radius;
  *length = c->lz;
}


vec_t	dGeomCCylinderPointDepth(dGeomID g, vec_t x, vec_t y, vec_t z)
{
	dUASSERT(g && g->type == dCCylinderClass, "argument not a ccylinder");
	dxCCylinder *c = (dxCCylinder*)g;
	
	dVector3 a;
	a[0] = x - c->pos[0];
	a[1] = y - c->pos[1];
	a[2] = z - c->pos[2];
	
	vec_t beta = dDOT14(a, c->R[0][2]);
	vec_t lz2 = c->lz*REAL(0.5);
	
	if(beta < -lz2)
		beta = -lz2;
	
	else if(beta > lz2)
		beta = lz2;
	
	a[0] = c->pos[0] + beta * *c->R[0][2];
	a[1] = c->pos[1] + beta * *c->R[1][2];
	a[2] = c->pos[2] + beta * *c->R[2][2];
	
	return c->radius - X_sqrt((x-a[0])*(x-a[0]) + (y-a[1])*(y-a[1]) + (z-a[2])*(z-a[2]));
}

//****************************************************************************
// plane public API

static void make_sure_plane_normal_has_unit_length (dxPlane *g)
{
  vec_t l = g->p[0]*g->p[0] + g->p[1]*g->p[1] + g->p[2]*g->p[2];
  if (l > 0) {
    l = X_recipsqrt(l);
    g->p[0] *= l;
    g->p[1] *= l;
    g->p[2] *= l;
    g->p[3] *= l;
  }
  else {
    g->p[0] = 1;
    g->p[1] = 0;
    g->p[2] = 0;
    g->p[3] = 0;
  }
}


dxPlane::dxPlane (dSpaceID space, vec_t a, vec_t b, vec_t c, vec_t d) :
  dxGeom (space,0)
{
  type = dPlaneClass;
  p[0] = a;
  p[1] = b;
  p[2] = c;
  p[3] = d;
  make_sure_plane_normal_has_unit_length (this);
}


void dxPlane::computeAABB()
{
  // @@@ planes that have normal vectors aligned along an axis can use a
  // @@@ less comprehensive (half space) bounding box.
  aabb[0] = -X_infinity;
  aabb[1] = X_infinity;
  aabb[2] = -X_infinity;
  aabb[3] = X_infinity;
  aabb[4] = -X_infinity;
  aabb[5] = X_infinity;
}


dGeomID dCreatePlane (dSpaceID space,
		      vec_t a, vec_t b, vec_t c, vec_t d)
{
  return new dxPlane (space,a,b,c,d);
}


void dGeomPlaneSetParams (dGeomID g, vec_t a, vec_t b, vec_t c, vec_t d)
{
  dUASSERT (g && g->type == dPlaneClass,"argument not a plane");
  dxPlane *p = (dxPlane*) g;
  p->p[0] = a;
  p->p[1] = b;
  p->p[2] = c;
  p->p[3] = d;
  make_sure_plane_normal_has_unit_length (p);
  dGeomMoved (g);
}


void dGeomPlaneGetParams (dGeomID g, dVector4 result)
{
  dUASSERT (g && g->type == dPlaneClass,"argument not a plane");
  dxPlane *p = (dxPlane*) g;
  result[0] = p->p[0];
  result[1] = p->p[1];
  result[2] = p->p[2];
  result[3] = p->p[3];
}


vec_t dGeomPlanePointDepth (dGeomID g, vec_t x, vec_t y, vec_t z)
{
  dUASSERT (g && g->type == dPlaneClass,"argument not a plane");
  dxPlane *p = (dxPlane*) g;
  return p->p[3] - p->p[0]*x - p->p[1]*y - p->p[2]*z;
}

//****************************************************************************
// ray public API

dxRay::dxRay (dSpaceID space, vec_t _length) : dxGeom (space,1)
{
  type = dRayClass;
  length = _length;
}


void dxRay::computeAABB()
{
  dVector3 e;
  e[0] = pos[0] + *R[0][2]*length;
  e[1] = pos[1] + *R[1][2]*length;
  e[2] = pos[2] + *R[2][2]*length;

  if (pos[0] < e[0]){
    aabb[0] = pos[0];
    aabb[1] = e[0];
  }
  else{
    aabb[0] = e[0];
    aabb[1] = pos[0];
  }
  
  if (pos[1] < e[1]){
    aabb[2] = pos[1];
    aabb[3] = e[1];
  }
  else{
    aabb[2] = e[1];
    aabb[3] = pos[1];
  }

  if (pos[2] < e[2]){
    aabb[4] = pos[2];
    aabb[5] = e[2];
  }
  else{
    aabb[4] = e[2];
    aabb[5] = pos[2];
  }
}


dGeomID dCreateRay (dSpaceID space, vec_t length)
{
  return new dxRay (space,length);
}


void dGeomRaySetLength (dGeomID g, vec_t length)
{
  dUASSERT (g && g->type == dRayClass,"argument not a ray");
  dxRay *r = (dxRay*) g;
  r->length = length;
  dGeomMoved (g);
}


vec_t dGeomRayGetLength (dGeomID g)
{
  dUASSERT (g && g->type == dRayClass,"argument not a ray");
  dxRay *r = (dxRay*) g;
  return r->length;
}


void dGeomRaySet (dGeomID g, vec_t px, vec_t py, vec_t pz, vec_t dx, vec_t dy, vec_t dz)
{
	dUASSERT (g && g->type == dRayClass,"argument not a ray");
	matrix_c* rot = g->R;
	vec_t* pos = g->pos;
	dVector3 n;
	pos[0] = px;
	pos[1] = py;
	pos[2] = pz;
	
	n[0] = dx;
	n[1] = dy;
	n[2] = dz;
	dNormalize3(n);
	
	*rot[0][2] = n[0];
	*rot[1][2] = n[1];
	*rot[2][2] = n[2];
	
	dGeomMoved (g);
}


void	dGeomRayGet(dGeomID g, dVector3 start, dVector3 dir)
{
	dUASSERT (g && g->type == dRayClass,"argument not a ray");
	
	start[0] = g->pos[0];
	start[1] = g->pos[1];
	start[2] = g->pos[2];
	
	dir[0] = *g->R[0][2];
	dir[1] = *g->R[1][2];
	dir[2] = *g->R[2][2];
}


void dGeomRaySetParams (dxGeom *g, int FirstContact, int BackfaceCull)
{
  dUASSERT (g && g->type == dRayClass,"argument not a ray");

  if (FirstContact){
    g->gflags |= RAY_FIRSTCONTACT;
  }
  else g->gflags &= ~RAY_FIRSTCONTACT;

  if (BackfaceCull){
    g->gflags |= RAY_BACKFACECULL;
  }
  else g->gflags &= ~RAY_BACKFACECULL;
}


void dGeomRayGetParams (dxGeom *g, int *FirstContact, int *BackfaceCull)
{
  dUASSERT (g && g->type == dRayClass,"argument not a ray");

  (*FirstContact) = ((g->gflags & RAY_FIRSTCONTACT) != 0);
  (*BackfaceCull) = ((g->gflags & RAY_BACKFACECULL) != 0);
}


void dGeomRaySetClosestHit (dxGeom *g, int closestHit)
{
  dUASSERT (g && g->type == dRayClass,"argument not a ray");
  if (closestHit){
    g->gflags |= RAY_CLOSEST_HIT;
  }
  else g->gflags &= ~RAY_CLOSEST_HIT;
}


int dGeomRayGetClosestHit (dxGeom *g)
{
  dUASSERT (g && g->type == dRayClass,"argument not a ray");
  return ((g->gflags & RAY_CLOSEST_HIT) != 0);
}

//****************************************************************************
// BSP public API

#if 1
dxBSP::dxBSP (dSpaceID space) : dxGeom (space,1)
{
  dDEBUGMSG ("dxBSP");

  type = dBSPClass;
  side[0] = 0;//X_infinity;
  side[1] = 0;//X_infinity;
  side[2] = 0;//X_infinity;
  
  checkcount = 0;
}

void dxBSP::computeAABB()
{
	dDEBUGMSG ("");
	
	vec_t xrange = REAL(0.5) * (X_fabs((*R)[0][0] * side[0]) + X_fabs((*R)[0][1] * side[1]) + X_fabs((*R)[0][2] * side[2]));
	vec_t yrange = REAL(0.5) * (X_fabs((*R)[1][0] * side[0]) + X_fabs((*R)[1][1] * side[1]) + X_fabs((*R)[1][2] * side[2]));
	vec_t zrange = REAL(0.5) * (X_fabs((*R)[2][0] * side[0]) + X_fabs((*R)[2][1] * side[1]) + X_fabs((*R)[2][2] * side[2]));
	
	aabb[0] = pos[0] - xrange;
	aabb[1] = pos[0] + xrange;
	aabb[2] = pos[1] - yrange;
	aabb[3] = pos[1] + yrange;
	aabb[4] = pos[2] - zrange;
	aabb[5] = pos[2] + zrange;
}

dGeomID dCreateBSP (dSpaceID space)
{
  return new dxBSP (space);
}

void dGeomBSPSetLengths (dGeomID g, vec_t lx, vec_t ly, vec_t lz)
{
  dUASSERT (g && g->type == dBSPClass,"argument not a BSP");
  dAASSERT (lx > 0 && ly > 0 && lz > 0);
  dxBSP *bsp = (dxBSP*) g;
  bsp->side[0] = lx;
  bsp->side[1] = ly;
  bsp->side[2] = lz;
  dGeomMoved (g);
}

void dGeomBSPAddPlane (dGeomID g, vec_t a, vec_t b, vec_t c, vec_t d)
{
  dUASSERT (g && g->type == dBSPClass,"argument not a BSP");
  dxBSP *bsp = (dxBSP*) g;
  
  dVector4 p;
  p[0] = a;
  p[1] = b;
  p[2] = c;
  p[3] = d;
  
  vec_t l = p[0]*p[0] + p[1]*p[1] + p[2]*p[2];
  if (l > 0) {
    l = X_recipsqrt(l);
    p[0] *= l;
    p[1] *= l;
    p[2] *= l;
    p[3] *= l;
  }
  
  bsp->planes.push(p);
}

void dGeomBSPAddBrush (dGeomID g, int sides_first, int sides_num)
{
  dUASSERT (g && g->type == dBSPClass,"argument not a BSP");
  dxBSP *bsp = (dxBSP*) g;
  
  dxBSP::dBSPBrush b;
  b.sides_first = sides_first;
  b.sides_num = sides_num;
  
  bsp->brushes.push(b);
}

void dGeomBSPAddBrushSide (dGeomID g, int plane_num)
{
  dUASSERT (g && g->type == dBSPClass,"argument not a BSP");
  dxBSP *bsp = (dxBSP*) g;
  
  dxBSP::dBSPBrushSide s;
  s.plane = &bsp->planes[plane_num][0];
   
  bsp->brushsides.push(s);
}

void dGeomBSPAddNode (dGeomID g, int plane_num, int child0, int child1)
{
  dUASSERT (g && g->type == dBSPClass,"argument not a BSP");
  dxBSP *bsp = (dxBSP*) g;
  
  dxBSP::dBSPNode n;
  n.plane = &bsp->planes[plane_num][0];
  n.children[0] = child0;
  n.children[1] = child1;
  
  bsp->nodes.push(n);
}

void dGeomBSPAddLeaf (dGeomID g, int brushes_first, int brushes_num)
{
  dUASSERT (g && g->type == dBSPClass,"argument not a BSP");
  dxBSP *bsp = (dxBSP*) g;
  
  dxBSP::dBSPLeaf l;
  l.brushes_first = brushes_first;
  l.brushes_num = brushes_num;
  
  bsp->leafs.push(l);
}

void dGeomBSPAddLeafBrush (dGeomID g, int num)
{
  dUASSERT (g && g->type == dBSPClass,"argument not a BSP");
  dxBSP *bsp = (dxBSP*) g;
  
  bsp->leafbrushes.push(num);
}
#endif

//****************************************************************************
// box-box collision utility


// find all the intersection points between the 2D rectangle with vertices
// at (+/-h[0],+/-h[1]) and the 2D quadrilateral with vertices (p[0],p[1]),
// (p[2],p[3]),(p[4],p[5]),(p[6],p[7]).
//
// the intersection points are returned as x,y pairs in the 'ret' array.
// the number of intersection points is returned by the function (this will
// be in the range 0 to 8).

static int intersectRectQuad (vec_t h[2], vec_t p[8], vec_t ret[16])
{
  // q (and r) contain nq (and nr) coordinate points for the current (and
  // chopped) polygons
  int nq=4,nr;
  vec_t buffer[16];
  vec_t *q = p;
  vec_t *r = ret;
  for (int dir=0; dir <= 1; dir++) {
    // direction notation: xy[0] = x axis, xy[1] = y axis
    for (int sign=-1; sign <= 1; sign += 2) {
      // chop q along the line xy[dir] = sign*h[dir]
      vec_t *pq = q;
      vec_t *pr = r;
      nr = 0;
      for (int i=nq; i > 0; i--) {
	// go through all points in q and all lines between adjacent points
	if (sign*pq[dir] < h[dir]) {
	  // this point is inside the chopping line
	  pr[0] = pq[0];
	  pr[1] = pq[1];
	  pr += 2;
	  nr++;
	  if (nr & 8) {
	    q = r;
	    goto done;
	  }
	}
	vec_t *nextq = (i > 1) ? pq+2 : q;
	if ((sign*pq[dir] < h[dir]) ^ (sign*nextq[dir] < h[dir])) {
	  // this line crosses the chopping line
	  pr[1-dir] = pq[1-dir] + (nextq[1-dir]-pq[1-dir]) /
	    (nextq[dir]-pq[dir]) * (sign*h[dir]-pq[dir]);
	  pr[dir] = sign*h[dir];
	  pr += 2;
	  nr++;
	  if (nr & 8) {
	    q = r;
	    goto done;
	  }
	}
	pq += 2;
      }
      q = r;
      r = (q==ret) ? buffer : ret;
      nq = nr;
    }
  }
 done:
  if (q != ret) memcpy (ret,q,nr*2*sizeof(vec_t));
  return nr;
}


// given n points in the plane (array p, of size 2*n), generate m points that
// best represent the whole set. the definition of 'best' here is not
// predetermined - the idea is to select points that give good box-box
// collision detection behavior. the chosen point indexes are returned in the
// array iret (of size m). 'i0' is always the first entry in the array.
// n must be in the range [1..8]. m must be in the range [1..n]. i0 must be
// in the range [0..n-1].

void cullPoints (int n, vec_t p[], int m, int i0, int iret[])
{
  // compute the centroid of the polygon in cx,cy
  int i,j;
  vec_t a,cx,cy,q;
  if (n==1) {
    cx = p[0];
    cy = p[1];
  }
  else if (n==2) {
    cx = REAL(0.5)*(p[0] + p[2]);
    cy = REAL(0.5)*(p[1] + p[3]);
  }
  else {
    a = 0;
    cx = 0;
    cy = 0;
    for (i=0; i<(n-1); i++) {
      q = p[i*2]*p[i*2+3] - p[i*2+2]*p[i*2+1];
      a += q;
      cx += q*(p[i*2]+p[i*2+2]);
      cy += q*(p[i*2+1]+p[i*2+3]);
    }
    q = p[n*2-2]*p[1] - p[0]*p[n*2-1];
    a = X_recip(REAL(3.0)*(a+q));
    cx = a*(cx + q*(p[n*2-2]+p[0]));
    cy = a*(cy + q*(p[n*2-1]+p[1]));
  }

  // compute the angle of each point w.r.t. the centroid
  vec_t A[8];
  for (i=0; i<n; i++) A[i] = X_atan2(p[i*2+1]-cy,p[i*2]-cx);

  // search for points that have angles closest to A[i0] + i*(2*pi/m).
  int avail[8];
  for (i=0; i<n; i++) avail[i] = 1;
  avail[i0] = 0;
  iret[0] = i0;
  iret++;
  for (j=1; j<m; j++) {
    a = vec_t(j)*(2*M_PI/m) + A[i0];
    if (a > M_PI) a -= 2*M_PI;
    vec_t maxdiff=1e9,diff;
#ifndef dNODEBUG
    *iret = i0;			// iret is not allowed to keep this value
#endif
    for (i=0; i<n; i++) {
      if (avail[i]) {
	diff = X_fabs (A[i]-a);
	if (diff > M_PI) diff = 2*M_PI - diff;
	if (diff < maxdiff) {
	  maxdiff = diff;
	  *iret = i;
	}
      }
    }
#ifndef dNODEBUG
    dIASSERT (*iret != i0);	// ensure iret got set
#endif
    avail[*iret] = 0;
    iret++;
  }
}


// given two boxes (p1,R1,side1) and (p2,R2,side2), collide them together and
// generate contact points. this returns 0 if there is no contact otherwise
// it returns the number of contacts generated.
// `normal' returns the contact normal.
// `depth' returns the maximum penetration depth along that normal.
// `return_code' returns a number indicating the type of contact that was
// detected:
//        1,2,3 = box 2 intersects with a face of box 1
//        4,5,6 = box 1 intersects with a face of box 2
//        7..15 = edge-edge contact
// `maxc' is the maximum number of contacts allowed to be generated, i.e.
// the size of the `contact' array.
// `contact' and `skip' are the contact array information provided to the
// collision functions. this function only fills in the position and depth
// fields.

int dBoxBox (const dVector3 p1, const matrix_c &R1,
	     const dVector3 side1, const dVector3 p2,
	     const matrix_c &R2, const dVector3 side2,
	     dVector3 normal, vec_t *depth, int *return_code,
	     int maxc, dContactGeom *contact, int skip)
{
  const vec_t fudge_factor = REAL(1.05);
  dVector3 p,pp,normalC;
  const vec_t *normalR = 0;
  vec_t A[3],B[3],R11,R12,R13,R21,R22,R23,R31,R32,R33,
    Q11,Q12,Q13,Q21,Q22,Q23,Q31,Q32,Q33,s,s2,l;
  int i,j,invert_normal,code;

  // get vector from centers of box 1 to box 2, relative to box 1
  p[0] = p2[0] - p1[0];
  p[1] = p2[1] - p1[1];
  p[2] = p2[2] - p1[2];
  
  dMULTIPLY1_331(pp, (vec_t*)R1, p);		// get pp = p relative to body 1

  // get side lengths / 2
  A[0] = side1[0]*REAL(0.5);
  A[1] = side1[1]*REAL(0.5);
  A[2] = side1[2]*REAL(0.5);
  B[0] = side2[0]*REAL(0.5);
  B[1] = side2[1]*REAL(0.5);
  B[2] = side2[2]*REAL(0.5);

  // Rij is R1'*R2, i.e. the relative rotation between R1 and R2
  R11 = dDOT44(R1+0,R2+0); R12 = dDOT44(R1+0,R2+1); R13 = dDOT44(R1+0,R2+2);
  R21 = dDOT44(R1+1,R2+0); R22 = dDOT44(R1+1,R2+1); R23 = dDOT44(R1+1,R2+2);
  R31 = dDOT44(R1+2,R2+0); R32 = dDOT44(R1+2,R2+1); R33 = dDOT44(R1+2,R2+2);

  Q11 = X_fabs(R11); Q12 = X_fabs(R12); Q13 = X_fabs(R13);
  Q21 = X_fabs(R21); Q22 = X_fabs(R22); Q23 = X_fabs(R23);
  Q31 = X_fabs(R31); Q32 = X_fabs(R32); Q33 = X_fabs(R33);

  // for all 15 possible separating axes:
  //   * see if the axis separates the boxes. if so, return 0.
  //   * find the depth of the penetration along the separating axis (s2)
  //   * if this is the largest depth so far, record it.
  // the normal vector will be set to the separating axis with the smallest
  // depth. note: normalR is set to point to a column of R1 or R2 if that is
  // the smallest depth normal so far. otherwise normalR is 0 and normalC is
  // set to a vector relative to body 1. invert_normal is 1 if the sign of
  // the normal should be flipped.

#define TST(expr1,expr2,norm,cc) \
  s2 = X_fabs(expr1) - (expr2); \
  if (s2 > 0) return 0; \
  if (s2 > s) { \
    s = s2; \
    normalR = norm; \
    invert_normal = ((expr1) < 0); \
    code = (cc); \
  }

  s = -X_infinity;
  invert_normal = 0;
  code = 0;

  // separating axis = u1,u2,u3
  TST (pp[0],(A[0] + B[0]*Q11 + B[1]*Q12 + B[2]*Q13),R1+0,1);
  TST (pp[1],(A[1] + B[0]*Q21 + B[1]*Q22 + B[2]*Q23),R1+1,2);
  TST (pp[2],(A[2] + B[0]*Q31 + B[1]*Q32 + B[2]*Q33),R1+2,3);

  // separating axis = v1,v2,v3
  TST (dDOT41(R2+0,p),(A[0]*Q11 + A[1]*Q21 + A[2]*Q31 + B[0]),R2+0,4);
  TST (dDOT41(R2+1,p),(A[0]*Q12 + A[1]*Q22 + A[2]*Q32 + B[1]),R2+1,5);
  TST (dDOT41(R2+2,p),(A[0]*Q13 + A[1]*Q23 + A[2]*Q33 + B[2]),R2+2,6);

  // note: cross product axes need to be scaled when s is computed.
  // normal (n1,n2,n3) is relative to box 1.
#undef TST
#define TST(expr1,expr2,n1,n2,n3,cc) \
  s2 = X_fabs(expr1) - (expr2); \
  if (s2 > 0) return 0; \
  l = X_sqrt ((n1)*(n1) + (n2)*(n2) + (n3)*(n3)); \
  if (l > 0) { \
    s2 /= l; \
    if (s2*fudge_factor > s) { \
      s = s2; \
      normalR = 0; \
      normalC[0] = (n1)/l; normalC[1] = (n2)/l; normalC[2] = (n3)/l; \
      invert_normal = ((expr1) < 0); \
      code = (cc); \
    } \
  }

  // separating axis = u1 x (v1,v2,v3)
  TST(pp[2]*R21-pp[1]*R31,(A[1]*Q31+A[2]*Q21+B[1]*Q13+B[2]*Q12),0,-R31,R21,7);
  TST(pp[2]*R22-pp[1]*R32,(A[1]*Q32+A[2]*Q22+B[0]*Q13+B[2]*Q11),0,-R32,R22,8);
  TST(pp[2]*R23-pp[1]*R33,(A[1]*Q33+A[2]*Q23+B[0]*Q12+B[1]*Q11),0,-R33,R23,9);

  // separating axis = u2 x (v1,v2,v3)
  TST(pp[0]*R31-pp[2]*R11,(A[0]*Q31+A[2]*Q11+B[1]*Q23+B[2]*Q22),R31,0,-R11,10);
  TST(pp[0]*R32-pp[2]*R12,(A[0]*Q32+A[2]*Q12+B[0]*Q23+B[2]*Q21),R32,0,-R12,11);
  TST(pp[0]*R33-pp[2]*R13,(A[0]*Q33+A[2]*Q13+B[0]*Q22+B[1]*Q21),R33,0,-R13,12);

  // separating axis = u3 x (v1,v2,v3)
  TST(pp[1]*R11-pp[0]*R21,(A[0]*Q21+A[1]*Q11+B[1]*Q33+B[2]*Q32),-R21,R11,0,13);
  TST(pp[1]*R12-pp[0]*R22,(A[0]*Q22+A[1]*Q12+B[0]*Q33+B[2]*Q31),-R22,R12,0,14);
  TST(pp[1]*R13-pp[0]*R23,(A[0]*Q23+A[1]*Q13+B[0]*Q32+B[1]*Q31),-R23,R13,0,15);

#undef TST

  if (!code) return 0;

  // if we get to this point, the boxes interpenetrate. compute the normal
  // in global coordinates.
  if (normalR) {
    normal[0] = normalR[0];
    normal[1] = normalR[4];
    normal[2] = normalR[8];
  }
  else {
    dMULTIPLY0_331 (normal, (vec_t*)R1, normalC);
  }
  if (invert_normal) {
    normal[0] = -normal[0];
    normal[1] = -normal[1];
    normal[2] = -normal[2];
  }
  *depth = -s;

  // compute contact point(s)

  if (code > 6) {
    // an edge from box 1 touches an edge from box 2.
    // find a point pa on the intersecting edge of box 1
    dVector3 pa;
    vec_t sign;
    for (i=0; i<3; i++) pa[i] = p1[i];
    for (j=0; j<3; j++) {
      sign = (dDOT14(normal,R1+j) > 0) ? REAL(1.0) : REAL(-1.0);
      for (i=0; i<3; i++) pa[i] += sign * A[j] * R1[i][j];
    }

    // find a point pb on the intersecting edge of box 2
    dVector3 pb;
    for (i=0; i<3; i++) pb[i] = p2[i];
    for (j=0; j<3; j++) {
      sign = (dDOT14(normal,R2+j) > 0) ? REAL(-1.0) : REAL(1.0);
      for (i=0; i<3; i++) pb[i] += sign * B[j] * R2[i][j];
    }

    vec_t alpha,beta;
    dVector3 ua,ub;
    for (i=0; i<3; i++) ua[i] = R1[i][((code)-7)/3];
    for (i=0; i<3; i++) ub[i] = R2[i][((code)-7)%3];

    dLineClosestApproach (pa,ua,pb,ub,&alpha,&beta);
    for (i=0; i<3; i++) pa[i] += ua[i]*alpha;
    for (i=0; i<3; i++) pb[i] += ub[i]*beta;

    for (i=0; i<3; i++) contact[0].pos[i] = REAL(0.5)*(pa[i]+pb[i]);
    contact[0].depth = *depth;
    *return_code = code;
    return 1;
  }

  // okay, we have a face-something intersection (because the separating
  // axis is perpendicular to a face). define face 'a' to be the reference
  // face (i.e. the normal vector is perpendicular to this) and face 'b' to be
  // the incident face (the closest face of the other box).

  const vec_t *Ra,*Rb,*pa,*pb,*Sa,*Sb;
  if (code <= 3) {
    Ra = R1;
    Rb = R2;
    pa = p1;
    pb = p2;
    Sa = A;
    Sb = B;
  }
  else {
    Ra = R2;
    Rb = R1;
    pa = p2;
    pb = p1;
    Sa = B;
    Sb = A;
  }

  // nr = normal vector of reference face dotted with axes of incident box.
  // anr = absolute values of nr.
  dVector3 normal2,nr,anr;
  if (code <= 3) {
    normal2[0] = normal[0];
    normal2[1] = normal[1];
    normal2[2] = normal[2];
  }
  else {
    normal2[0] = -normal[0];
    normal2[1] = -normal[1];
    normal2[2] = -normal[2];
  }
  dMULTIPLY1_331 (nr,Rb,normal2);
  anr[0] = X_fabs (nr[0]);
  anr[1] = X_fabs (nr[1]);
  anr[2] = X_fabs (nr[2]);

  // find the largest compontent of anr: this corresponds to the normal
  // for the indident face. the other axis numbers of the indicent face
  // are stored in a1,a2.
  int lanr,a1,a2;
  if (anr[1] > anr[0]) {
    if (anr[1] > anr[2]) {
      a1 = 0;
      lanr = 1;
      a2 = 2;
    }
    else {
      a1 = 0;
      a2 = 1;
      lanr = 2;
    }
  }
  else {
    if (anr[0] > anr[2]) {
      lanr = 0;
      a1 = 1;
      a2 = 2;
    }
    else {
      a1 = 0;
      a2 = 1;
      lanr = 2;
    }
  }

  // compute center point of incident face, in reference-face coordinates
  dVector3 center;
  if (nr[lanr] < 0) {
    for (i=0; i<3; i++) center[i] = pb[i] - pa[i] + Sb[lanr] * Rb[i*4+lanr];
  }
  else {
    for (i=0; i<3; i++) center[i] = pb[i] - pa[i] - Sb[lanr] * Rb[i*4+lanr];
  }

  // find the normal and non-normal axis numbers of the reference box
  int codeN,code1,code2;
  if (code <= 3) codeN = code-1; else codeN = code-4;
  if (codeN==0) {
    code1 = 1;
    code2 = 2;
  }
  else if (codeN==1) {
    code1 = 0;
    code2 = 2;
  }
  else {
    code1 = 0;
    code2 = 1;
  }

  // find the four corners of the incident face, in reference-face coordinates
  vec_t quad[8];	// 2D coordinate of incident face (x,y pairs)
  vec_t c1,c2,m11,m12,m21,m22;
  c1 = dDOT14 (center,Ra+code1);
  c2 = dDOT14 (center,Ra+code2);
  // optimize this? - we have already computed this data above, but it is not
  // stored in an easy-to-index format. for now it's quicker just to recompute
  // the four dot products.
  m11 = dDOT44 (Ra+code1,Rb+a1);
  m12 = dDOT44 (Ra+code1,Rb+a2);
  m21 = dDOT44 (Ra+code2,Rb+a1);
  m22 = dDOT44 (Ra+code2,Rb+a2);
  {
    vec_t k1 = m11*Sb[a1];
    vec_t k2 = m21*Sb[a1];
    vec_t k3 = m12*Sb[a2];
    vec_t k4 = m22*Sb[a2];
    quad[0] = c1 - k1 - k3;
    quad[1] = c2 - k2 - k4;
    quad[2] = c1 - k1 + k3;
    quad[3] = c2 - k2 + k4;
    quad[4] = c1 + k1 + k3;
    quad[5] = c2 + k2 + k4;
    quad[6] = c1 + k1 - k3;
    quad[7] = c2 + k2 - k4;
  }

  // find the size of the reference face
  vec_t rect[2];
  rect[0] = Sa[code1];
  rect[1] = Sa[code2];

  // intersect the incident and reference faces
  vec_t ret[16];
  int n = intersectRectQuad (rect,quad,ret);
  if (n < 1) return 0;		// this should never happen

  // convert the intersection points into reference-face coordinates,
  // and compute the contact position and depth for each point. only keep
  // those points that have a positive (penetrating) depth. delete points in
  // the 'ret' array as necessary so that 'point' and 'ret' correspond.
  vec_t point[3*8];		// penetrating contact points
  vec_t dep[8];			// depths for those points
  vec_t det1 = X_recip(m11*m22 - m12*m21);
  m11 *= det1;
  m12 *= det1;
  m21 *= det1;
  m22 *= det1;
  int cnum = 0;			// number of penetrating contact points found
  for (j=0; j < n; j++) {
    vec_t k1 =  m22*(ret[j*2]-c1) - m12*(ret[j*2+1]-c2);
    vec_t k2 = -m21*(ret[j*2]-c1) + m11*(ret[j*2+1]-c2);
    for (i=0; i<3; i++) point[cnum*3+i] =
			  center[i] + k1*Rb[i*4+a1] + k2*Rb[i*4+a2];
    dep[cnum] = Sa[codeN] - dDOT(normal2,point+cnum*3);
    if (dep[cnum] >= 0) {
      ret[cnum*2] = ret[j*2];
      ret[cnum*2+1] = ret[j*2+1];
      cnum++;
    }
  }
  if (cnum < 1) return 0;	// this should never happen

  // we can't generate more contacts than we actually have
  if (maxc > cnum) maxc = cnum;
  if (maxc < 1) maxc = 1;

  if (cnum <= maxc) {
    // we have less contacts than we need, so we use them all
    for (j=0; j < cnum; j++) {
      dContactGeom *con = CONTACT(contact,skip*j);
      for (i=0; i<3; i++) con->pos[i] = point[j*3+i] + pa[i];
      con->depth = dep[j];
    }
  }
  else {
    // we have more contacts than are wanted, some of them must be culled.
    // find the deepest point, it is always the first contact.
    int i1 = 0;
    vec_t maxdepth = dep[0];
    for (i=1; i<cnum; i++) {
      if (dep[i] > maxdepth) {
	maxdepth = dep[i];
	i1 = i;
      }
    }

    int iret[8];
    cullPoints (cnum,ret,maxc,i1,iret);

    for (j=0; j < maxc; j++) {
      dContactGeom *con = CONTACT(contact,skip*j);
      for (i=0; i<3; i++) con->pos[i] = point[iret[j]*3+i] + pa[i];
      con->depth = dep[iret[j]];
    }
    cnum = maxc;
  }

  *return_code = code;
  return cnum;
}

//****************************************************************************
// pairwise collision functions for standard geom types

int dCollideSphereSphere (dxGeom *o1, dxGeom *o2, int flags,
			  dContactGeom *contact, int skip)
{
  dIASSERT (skip >= (int)sizeof(dContactGeom));
  dIASSERT (o1->type == dSphereClass);
  dIASSERT (o2->type == dSphereClass);
  dxSphere *sphere1 = (dxSphere*) o1;
  dxSphere *sphere2 = (dxSphere*) o2;

  contact->g1 = o1;
  contact->g2 = o2;

  return dCollideSpheres (o1->pos,sphere1->radius,
			  o2->pos,sphere2->radius,contact);
}


int dCollideSphereBox (dxGeom *o1, dxGeom *o2, int flags,
		       dContactGeom *contact, int skip)
{
  // this is easy. get the sphere center `p' relative to the box, and then clip
  // that to the boundary of the box (call that point `q'). if q is on the
  // boundary of the box and |p-q| is <= sphere radius, they touch.
  // if q is inside the box, the sphere is inside the box, so set a contact
  // normal to push the sphere to the closest box face.

  dVector3 l,t,p,q,r;
  vec_t depth;
  int onborder = 0;

  dIASSERT (skip >= (int)sizeof(dContactGeom));
  dIASSERT (o1->type == dSphereClass);
  dIASSERT (o2->type == dBoxClass);
  dxSphere *sphere = (dxSphere*) o1;
  dxBox *box = (dxBox*) o2;

  contact->g1 = o1;
  contact->g2 = o2;

  p[0] = o1->pos[0] - o2->pos[0];
  p[1] = o1->pos[1] - o2->pos[1];
  p[2] = o1->pos[2] - o2->pos[2];

  l[0] = box->side[0]*REAL(0.5);
  t[0] = dDOT14(p, (vec_t*)o2->R);
  if (t[0] < -l[0]) { t[0] = -l[0]; onborder = 1; }
  if (t[0] >  l[0]) { t[0] =  l[0]; onborder = 1; }

  l[1] = box->side[1]*REAL(0.5);
  t[1] = dDOT14(p, (vec_t*)o2->R+1);
  if (t[1] < -l[1]) { t[1] = -l[1]; onborder = 1; }
  if (t[1] >  l[1]) { t[1] =  l[1]; onborder = 1; }

  t[2] = dDOT14(p, (vec_t*)o2->R+2);
  l[2] = box->side[2]*REAL(0.5);
  if (t[2] < -l[2]) { t[2] = -l[2]; onborder = 1; }
  if (t[2] >  l[2]) { t[2] =  l[2]; onborder = 1; }

  if (!onborder) {
    // sphere center inside box. find closest face to `t'
    vec_t min_distance = l[0] - X_fabs(t[0]);
    int mini = 0;
    for (int i=1; i<3; i++) {
      vec_t face_distance = l[i] - X_fabs(t[i]);
      if (face_distance < min_distance) {
	min_distance = face_distance;
	mini = i;
      }
    }
    // contact position = sphere center
    contact->pos[0] = o1->pos[0];
    contact->pos[1] = o1->pos[1];
    contact->pos[2] = o1->pos[2];
    // contact normal points to closest face
    dVector3 tmp;
    tmp[0] = 0;
    tmp[1] = 0;
    tmp[2] = 0;
    tmp[mini] = (t[mini] > 0) ? REAL(1.0) : REAL(-1.0);
    dMULTIPLY0_331(contact->normal, (vec_t*)o2->R, tmp);
    // contact depth = distance to wall along normal plus radius
    contact->depth = min_distance + sphere->radius;
    return 1;
  }

  t[3] = 0;			//@@@ hmmm
  dMULTIPLY0_331 (q, (vec_t*)o2->R, t);
  r[0] = p[0] - q[0];
  r[1] = p[1] - q[1];
  r[2] = p[2] - q[2];
  depth = sphere->radius - X_sqrt(dDOT(r,r));
  if (depth < 0) return 0;
  contact->pos[0] = q[0] + o2->pos[0];
  contact->pos[1] = q[1] + o2->pos[1];
  contact->pos[2] = q[2] + o2->pos[2];
  contact->normal[0] = r[0];
  contact->normal[1] = r[1];
  contact->normal[2] = r[2];
  dNormalize3 (contact->normal);
  contact->depth = depth;
  return 1;
}


int dCollideSpherePlane (dxGeom *o1, dxGeom *o2, int flags,
			 dContactGeom *contact, int skip)
{
  dIASSERT (skip >= (int)sizeof(dContactGeom));
  dIASSERT (o1->type == dSphereClass);
  dIASSERT (o2->type == dPlaneClass);
  dxSphere *sphere = (dxSphere*) o1;
  dxPlane *plane = (dxPlane*) o2;

  contact->g1 = o1;
  contact->g2 = o2;
  vec_t k = dDOT (o1->pos,plane->p);
  vec_t depth = plane->p[3] - k + sphere->radius;
  if (depth >= 0) {
    contact->normal[0] = plane->p[0];
    contact->normal[1] = plane->p[1];
    contact->normal[2] = plane->p[2];
    contact->pos[0] = o1->pos[0] - plane->p[0] * sphere->radius;
    contact->pos[1] = o1->pos[1] - plane->p[1] * sphere->radius;
    contact->pos[2] = o1->pos[2] - plane->p[2] * sphere->radius;
    contact->depth = depth;
    return 1;
  }
  else return 0;
}


int	dCollideBoxBox(dxGeom *o1, dxGeom *o2, int flags, dContactGeom *contact, int skip)
{
  dIASSERT (skip >= (int)sizeof(dContactGeom));
  dIASSERT (o1->type == dBoxClass);
  dIASSERT (o2->type == dBoxClass);
  dVector3 normal;
  vec_t depth;
  int code;
  dxBox *b1 = (dxBox*) o1;
  dxBox *b2 = (dxBox*) o2;
  int num = dBoxBox(o1->pos, *o1->R, b1->side, o2->pos, *o2->R, b2->side, normal, &depth, &code, flags & NUMC_MASK, contact, skip);
  
	for(int i=0; i<num; i++)
	{
		CONTACT(contact,i*skip)->normal[0] = -normal[0];
		CONTACT(contact,i*skip)->normal[1] = -normal[1];
		CONTACT(contact,i*skip)->normal[2] = -normal[2];
		CONTACT(contact,i*skip)->g1 = o1;
		CONTACT(contact,i*skip)->g2 = o2;
	}
	
	return num;
}


int dCollideBoxPlane (dxGeom *o1, dxGeom *o2,
		      int flags, dContactGeom *contact, int skip)
{
  dIASSERT (skip >= (int)sizeof(dContactGeom));
  dIASSERT (o1->type == dBoxClass);
  dIASSERT (o2->type == dPlaneClass);
  dxBox *box = (dxBox*) o1;
  dxPlane *plane = (dxPlane*) o2;

  contact->g1 = o1;
  contact->g2 = o2;
  int ret = 0;

  //@@@ problem: using 4-vector (plane->p) as 3-vector (normal).
  const vec_t *R = (vec_t*)*o1->R;		// rotation of box
  const vec_t *n = plane->p;		// normal vector

  // project sides lengths along normal vector, get absolute values
  vec_t Q1 = dDOT14(n,R+0);
  vec_t Q2 = dDOT14(n,R+1);
  vec_t Q3 = dDOT14(n,R+2);
  vec_t A1 = box->side[0] * Q1;
  vec_t A2 = box->side[1] * Q2;
  vec_t A3 = box->side[2] * Q3;
  vec_t B1 = X_fabs(A1);
  vec_t B2 = X_fabs(A2);
  vec_t B3 = X_fabs(A3);

  // early exit test
  vec_t depth = plane->p[3] + REAL(0.5)*(B1+B2+B3) - dDOT(n,o1->pos);
  if (depth < 0) return 0;

  // find number of contacts requested
  int maxc = flags & NUMC_MASK;
  if (maxc < 1) maxc = 1;
  if (maxc > 3) maxc = 3;	// no more than 3 contacts per box allowed

  // find deepest point
  dVector3 p;
  p[0] = o1->pos[0];
  p[1] = o1->pos[1];
  p[2] = o1->pos[2];
#define FOO(i,op) \
  p[0] op REAL(0.5)*box->side[i] * R[0+i]; \
  p[1] op REAL(0.5)*box->side[i] * R[4+i]; \
  p[2] op REAL(0.5)*box->side[i] * R[8+i];
#define BAR(i,iinc) if (A ## iinc > 0) { FOO(i,-=) } else { FOO(i,+=) }
  BAR(0,1);
  BAR(1,2);
  BAR(2,3);
#undef FOO
#undef BAR

  // the deepest point is the first contact point
  contact->pos[0] = p[0];
  contact->pos[1] = p[1];
  contact->pos[2] = p[2];
  contact->normal[0] = n[0];
  contact->normal[1] = n[1];
  contact->normal[2] = n[2];
  contact->depth = depth;
  ret = 1;		// ret is number of contact points found so far
  if (maxc == 1) goto done;

  // get the second and third contact points by starting from `p' and going
  // along the two sides with the smallest projected length.

#define FOO(i,j,op) \
  CONTACT(contact,i*skip)->pos[0] = p[0] op box->side[j] * R[0+j]; \
  CONTACT(contact,i*skip)->pos[1] = p[1] op box->side[j] * R[4+j]; \
  CONTACT(contact,i*skip)->pos[2] = p[2] op box->side[j] * R[8+j];
#define BAR(ctact,side,sideinc) \
  depth -= B ## sideinc; \
  if (depth < 0) goto done; \
  if (A ## sideinc > 0) { FOO(ctact,side,+) } else { FOO(ctact,side,-) } \
  CONTACT(contact,ctact*skip)->depth = depth; \
  ret++;

  CONTACT(contact,skip)->normal[0] = n[0];
  CONTACT(contact,skip)->normal[1] = n[1];
  CONTACT(contact,skip)->normal[2] = n[2];
  if (maxc == 3) {
    CONTACT(contact,2*skip)->normal[0] = n[0];
    CONTACT(contact,2*skip)->normal[1] = n[1];
    CONTACT(contact,2*skip)->normal[2] = n[2];
  }

  if (B1 < B2) {
    if (B3 < B1) goto use_side_3; else {
      BAR(1,0,1);	// use side 1
      if (maxc == 2) goto done;
      if (B2 < B3) goto contact2_2; else goto contact2_3;
    }
  }
  else {
    if (B3 < B2) {
      use_side_3:	// use side 3
      BAR(1,2,3);
      if (maxc == 2) goto done;
      if (B1 < B2) goto contact2_1; else goto contact2_2;
    }
    else {
      BAR(1,1,2);	// use side 2
      if (maxc == 2) goto done;
      if (B1 < B3) goto contact2_1; else goto contact2_3;
    }
  }

  contact2_1: BAR(2,0,1); goto done;
  contact2_2: BAR(2,1,2); goto done;
  contact2_3: BAR(2,2,3); goto done;
#undef FOO
#undef BAR

 done:
  for (int i=0; i<ret; i++) {
    CONTACT(contact,i*skip)->g1 = o1;
    CONTACT(contact,i*skip)->g2 = o2;
  }
  return ret;
}


int dCollideCCylinderSphere (dxGeom *o1, dxGeom *o2, int flags,
			     dContactGeom *contact, int skip)
{
  dIASSERT (skip >= (int)sizeof(dContactGeom));
  dIASSERT (o1->type == dCCylinderClass);
  dIASSERT (o2->type == dSphereClass);
  dxCCylinder *ccyl = (dxCCylinder*) o1;
  dxSphere *sphere = (dxSphere*) o2;

  contact->g1 = o1;
  contact->g2 = o2;

  // find the point on the cylinder axis that is closest to the sphere
  vec_t alpha = 
    *o1->R[0][2] * (o2->pos[0] - o1->pos[0]) +
    *o1->R[1][2] * (o2->pos[1] - o1->pos[1]) +
    *o1->R[2][2] * (o2->pos[2] - o1->pos[2]);
  vec_t lz2 = ccyl->lz * REAL(0.5);
  if (alpha > lz2) alpha = lz2;
  if (alpha < -lz2) alpha = -lz2;

  // collide the spheres
  dVector3 p;
  p[0] = o1->pos[0] + alpha * *o1->R[0][2];
  p[1] = o1->pos[1] + alpha * *o1->R[1][2];
  p[2] = o1->pos[2] + alpha * *o1->R[2][2];
  
  return dCollideSpheres(p, ccyl->radius, o2->pos, sphere->radius, contact);
}


int dCollideCCylinderBox (dxGeom *o1, dxGeom *o2, int flags, dContactGeom *contact, int skip)
{
  dIASSERT (skip >= (int)sizeof(dContactGeom));
  dIASSERT (o1->type == dCCylinderClass);
  dIASSERT (o2->type == dBoxClass);
  dxCCylinder *cyl = (dxCCylinder*) o1;
  dxBox *box = (dxBox*) o2;

  contact->g1 = o1;
  contact->g2 = o2;

  // get p1,p2 = cylinder axis endpoints, get radius
  dVector3 p1,p2;
  vec_t clen = cyl->lz * REAL(0.5);
  
  p1[0] = o1->pos[0] + clen * *o1->R[0][2];
  p1[1] = o1->pos[1] + clen * *o1->R[1][2];
  p1[2] = o1->pos[2] + clen * *o1->R[2][2];
  
  p2[0] = o1->pos[0] - clen * *o1->R[0][2];
  p2[1] = o1->pos[1] - clen * *o1->R[1][2];
  p2[2] = o1->pos[2] - clen * *o1->R[2][2];
  
  vec_t radius = cyl->radius;

  // copy out box center, rotation matrix, and side array
  vec_t *c = o2->pos;
  const vec_t *side = box->side;

  // get the closest point between the cylinder axis and the box
  dVector3 pl,pb;
  dClosestLineBoxPoints(p1, p2, c, *o2->R, side, pl, pb);

  // generate contact point
  return dCollideSpheres (pl,radius,pb,0,contact);
}


int dCollideCCylinderCCylinder (dxGeom *o1, dxGeom *o2, int flags, dContactGeom *contact, int skip)
{
  int i;
  const vec_t tolerance = REAL(1e-5);

  dIASSERT (skip >= (int)sizeof(dContactGeom));
  dIASSERT (o1->type == dCCylinderClass);
  dIASSERT (o2->type == dCCylinderClass);
  dxCCylinder *cyl1 = (dxCCylinder*) o1;
  dxCCylinder *cyl2 = (dxCCylinder*) o2;

  contact->g1 = o1;
  contact->g2 = o2;

  // copy out some variables, for convenience
  vec_t lz1 = cyl1->lz * REAL(0.5);
  vec_t lz2 = cyl2->lz * REAL(0.5);
  vec_t *pos1 = o1->pos;
  vec_t *pos2 = o2->pos;
  vec_t axis1[3],axis2[3];
  
  axis1[0] = *o1->R[0][2];
  axis1[1] = *o1->R[1][2];
  axis1[2] = *o1->R[2][2];
  
  axis2[0] = *o2->R[0][2];
  axis2[1] = *o2->R[1][2];
  axis2[2] = *o2->R[2][2];

  // if the cylinder axes are close to parallel, we'll try to detect up to
  // two contact points along the body of the cylinder. if we can't find any
  // points then we'll fall back to the closest-points algorithm. note that
  // we are not treating this special case for reasons of degeneracy, but
  // because we want two contact points in some situations. the closet-points
  // algorithm is robust in all casts, but it can return only one contact.

  dVector3 sphere1,sphere2;
  vec_t a1a2 = dDOT (axis1,axis2);
  vec_t det = REAL(1.0)-a1a2*a1a2;
  if (det < tolerance) {
    // the cylinder axes (almost) parallel, so we will generate up to two
    // contacts. alpha1 and alpha2 (line position parameters) are related by:
    //       alpha2 =   alpha1 + (pos1-pos2)'*axis1   (if axis1==axis2)
    //    or alpha2 = -(alpha1 + (pos1-pos2)'*axis1)  (if axis1==-axis2)
    // first compute where the two cylinders overlap in alpha1 space:
    if (a1a2 < 0) {
      axis2[0] = -axis2[0];
      axis2[1] = -axis2[1];
      axis2[2] = -axis2[2];
    }
    vec_t q[3];
    for (i=0; i<3; i++) q[i] = pos1[i]-pos2[i];
    vec_t k = dDOT (axis1,q);
    vec_t a1lo = -lz1;
    vec_t a1hi = lz1;
    vec_t a2lo = -lz2 - k;
    vec_t a2hi = lz2 - k;
    vec_t lo = (a1lo > a2lo) ? a1lo : a2lo;
    vec_t hi = (a1hi < a2hi) ? a1hi : a2hi;
    if (lo <= hi) {
      int num_contacts = flags & NUMC_MASK;
      if (num_contacts >= 2 && lo < hi) {
	// generate up to two contacts. if one of those contacts is
	// not made, fall back on the one-contact strategy.
	for (i=0; i<3; i++) sphere1[i] = pos1[i] + lo*axis1[i];
	for (i=0; i<3; i++) sphere2[i] = pos2[i] + (lo+k)*axis2[i];
	int n1 = dCollideSpheres (sphere1,cyl1->radius,
				  sphere2,cyl2->radius,contact);
	if (n1) {
	  for (i=0; i<3; i++) sphere1[i] = pos1[i] + hi*axis1[i];
	  for (i=0; i<3; i++) sphere2[i] = pos2[i] + (hi+k)*axis2[i];
	  dContactGeom *c2 = CONTACT(contact,skip);
	  int n2 = dCollideSpheres (sphere1,cyl1->radius,
				    sphere2,cyl2->radius, c2);
	  if (n2) {
	    c2->g1 = o1;
	    c2->g2 = o2;
	    return 2;
	  }
	}
      }

      // just one contact to generate, so put it in the middle of
      // the range
      vec_t alpha1 = (lo + hi) * REAL(0.5);
      vec_t alpha2 = alpha1 + k;
      for (i=0; i<3; i++) sphere1[i] = pos1[i] + alpha1*axis1[i];
      for (i=0; i<3; i++) sphere2[i] = pos2[i] + alpha2*axis2[i];
      return dCollideSpheres (sphere1,cyl1->radius,
			      sphere2,cyl2->radius,contact);
    }
  }
	  
  // use the closest point algorithm
  dVector3 a1,a2,b1,b2;
  a1[0] = o1->pos[0] + axis1[0]*lz1;
  a1[1] = o1->pos[1] + axis1[1]*lz1;
  a1[2] = o1->pos[2] + axis1[2]*lz1;
  a2[0] = o1->pos[0] - axis1[0]*lz1;
  a2[1] = o1->pos[1] - axis1[1]*lz1;
  a2[2] = o1->pos[2] - axis1[2]*lz1;
  b1[0] = o2->pos[0] + axis2[0]*lz2;
  b1[1] = o2->pos[1] + axis2[1]*lz2;
  b1[2] = o2->pos[2] + axis2[2]*lz2;
  b2[0] = o2->pos[0] - axis2[0]*lz2;
  b2[1] = o2->pos[1] - axis2[1]*lz2;
  b2[2] = o2->pos[2] - axis2[2]*lz2;

  dClosestLineSegmentPoints (a1,a2,b1,b2,sphere1,sphere2);
  return dCollideSpheres (sphere1,cyl1->radius,sphere2,cyl2->radius,contact);
}


int dCollideCCylinderPlane (dxGeom *o1, dxGeom *o2, int flags,
			    dContactGeom *contact, int skip)
{
  dIASSERT (skip >= (int)sizeof(dContactGeom));
  dIASSERT (o1->type == dCCylinderClass);
  dIASSERT (o2->type == dPlaneClass);
  dxCCylinder *ccyl = (dxCCylinder*) o1;
  dxPlane *plane = (dxPlane*) o2;

  // collide the deepest capping sphere with the plane
  vec_t sign = (dDOT14(plane->p, &((*o1->R)[0][2])) > 0) ? REAL(-1.0) : REAL(1.0);
  dVector3 p;
  p[0] = o1->pos[0] + *o1->R[0][2] * ccyl->lz * REAL(0.5) * sign;
  p[1] = o1->pos[1] + *o1->R[1][2] * ccyl->lz * REAL(0.5) * sign;
  p[2] = o1->pos[2] + *o1->R[2][2] * ccyl->lz * REAL(0.5) * sign;

  vec_t k = dDOT (p,plane->p);
  vec_t depth = plane->p[3] - k + ccyl->radius;
  if (depth < 0) return 0;
  contact->normal[0] = plane->p[0];
  contact->normal[1] = plane->p[1];
  contact->normal[2] = plane->p[2];
  contact->pos[0] = p[0] - plane->p[0] * ccyl->radius;
  contact->pos[1] = p[1] - plane->p[1] * ccyl->radius;
  contact->pos[2] = p[2] - plane->p[2] * ccyl->radius;
  contact->depth = depth;

  int ncontacts = 1;
  if ((flags & NUMC_MASK) >= 2) {
    // collide the other capping sphere with the plane
    p[0] = o1->pos[0] - *o1->R[0][2]  * ccyl->lz * REAL(0.5) * sign;
    p[1] = o1->pos[1] - *o1->R[1][2]  * ccyl->lz * REAL(0.5) * sign;
    p[2] = o1->pos[2] - *o1->R[2][2] * ccyl->lz * REAL(0.5) * sign;

    k = dDOT (p,plane->p);
    depth = plane->p[3] - k + ccyl->radius;
    if (depth >= 0) {
      dContactGeom *c2 = CONTACT(contact,skip);
      c2->normal[0] = plane->p[0];
      c2->normal[1] = plane->p[1];
      c2->normal[2] = plane->p[2];
      c2->pos[0] = p[0] - plane->p[0] * ccyl->radius;
      c2->pos[1] = p[1] - plane->p[1] * ccyl->radius;
      c2->pos[2] = p[2] - plane->p[2] * ccyl->radius;
      c2->depth = depth;
      ncontacts = 2;
    }
  }

  for (int i=0; i < ncontacts; i++) {
    CONTACT(contact,i*skip)->g1 = o1;
    CONTACT(contact,i*skip)->g2 = o2;
  }
  return ncontacts;
}


// if mode==1 then use the sphere exit contact, not the entry contact

static int ray_sphere_helper (dxRay *ray, dVector3 sphere_pos, vec_t radius,
			      dContactGeom *contact, int mode)
{
  dVector3 q;
  q[0] = ray->pos[0] - sphere_pos[0];
  q[1] = ray->pos[1] - sphere_pos[1];
  q[2] = ray->pos[2] - sphere_pos[2];
  vec_t B = dDOT14(q, &((*ray->R)[0][2]));
  vec_t C = dDOT(q,q) - radius*radius;
  // note: if C <= 0 then the start of the ray is inside the sphere
  vec_t k = B*B - C;
  if (k < 0) return 0;
  k = X_sqrt(k);
  vec_t alpha;
  if (mode && C >= 0) {
    alpha = -B + k;
    if (alpha < 0) return 0;
  }
  else {
    alpha = -B - k;
    if (alpha < 0) {
      alpha = -B + k;
      if (alpha < 0) return 0;
    }
  }
  if (alpha > ray->length) return 0;
  contact->pos[0] = ray->pos[0] + alpha * *ray->R[0][2];
  contact->pos[1] = ray->pos[1] + alpha * *ray->R[1][2];
  contact->pos[2] = ray->pos[2] + alpha * *ray->R[2][2];
  vec_t nsign = (C < 0 || mode) ? REAL(-1.0) : REAL(1.0);
  contact->normal[0] = nsign*(contact->pos[0] - sphere_pos[0]);
  contact->normal[1] = nsign*(contact->pos[1] - sphere_pos[1]);
  contact->normal[2] = nsign*(contact->pos[2] - sphere_pos[2]);
  dNormalize3 (contact->normal);
  contact->depth = alpha;
  return 1;
}


int dCollideRaySphere (dxGeom *o1, dxGeom *o2, int flags,
		       dContactGeom *contact, int skip)
{
  dIASSERT (skip >= (int)sizeof(dContactGeom));
  dIASSERT (o1->type == dRayClass);
  dIASSERT (o2->type == dSphereClass);
  dxRay *ray = (dxRay*) o1;
  dxSphere *sphere = (dxSphere*) o2;
  contact->g1 = ray;
  contact->g2 = sphere;
  return ray_sphere_helper (ray,sphere->pos,sphere->radius,contact,0);
}


int dCollideRayBox (dxGeom *o1, dxGeom *o2, int flags,
		    dContactGeom *contact, int skip)
{
  dIASSERT (skip >= (int)sizeof(dContactGeom));
  dIASSERT (o1->type == dRayClass);
  dIASSERT (o2->type == dBoxClass);
  dxRay *ray = (dxRay*) o1;
  dxBox *box = (dxBox*) o2;

  contact->g1 = ray;
  contact->g2 = box;

  int i;

  // compute the start and delta of the ray relative to the box.
  // we will do all subsequent computations in this box-relative coordinate
  // system. we have to do a translation and rotation for each point.
  dVector3 tmp,s,v;
  tmp[0] = ray->pos[0] - box->pos[0];
  tmp[1] = ray->pos[1] - box->pos[1];
  tmp[2] = ray->pos[2] - box->pos[2];
  dMULTIPLY1_331 (s, (vec_t*)*box->R,tmp);
  tmp[0] = *ray->R[0][2];
  tmp[1] = *ray->R[1][2];
  tmp[2] = *ray->R[2][2];
  dMULTIPLY1_331 (v, (vec_t*)*box->R,tmp);

  // mirror the line so that v has all components >= 0
  dVector3 sign;
  for (i=0; i<3; i++) {
    if (v[i] < 0) {
      s[i] = -s[i];
      v[i] = -v[i];
      sign[i] = 1;
    }
    else sign[i] = -1;
  }

  // compute the half-sides of the box
  vec_t h[3];
  h[0] = REAL(0.5) * box->side[0];
  h[1] = REAL(0.5) * box->side[1];
  h[2] = REAL(0.5) * box->side[2];

  // do a few early exit tests
  if ((s[0] < -h[0] && v[0] <= 0) || s[0] >  h[0] ||
      (s[1] < -h[1] && v[1] <= 0) || s[1] >  h[1] ||
      (s[2] < -h[2] && v[2] <= 0) || s[2] >  h[2] ||
      (v[0] == 0 && v[1] == 0 && v[2] == 0)) {
    return 0;
  }

  // compute the t=[lo..hi] range for where s+v*t intersects the box
  vec_t lo = -X_infinity;
  vec_t hi = X_infinity;
  int nlo = 0, nhi = 0;
  for (i=0; i<3; i++) {
    if (v[i] != 0) {
      vec_t k = (-h[i] - s[i])/v[i];
      if (k > lo) {
	lo = k;
	nlo = i;
      }
      k = (h[i] - s[i])/v[i];
      if (k < hi) {
	hi = k;
	nhi = i;
      }
    }
  }

  // check if the ray intersects
  if (lo > hi) return 0;
  vec_t alpha;
  int n;
  if (lo >= 0) {
    alpha = lo;
    n = nlo;
  }
  else {
    alpha = hi;
    n = nhi;
  }
  if (alpha < 0 || alpha > ray->length) return 0;
  contact->pos[0] = ray->pos[0] + alpha * *ray->R[0][2];
  contact->pos[1] = ray->pos[1] + alpha * *ray->R[1][2];
  contact->pos[2] = ray->pos[2] + alpha * *ray->R[2][2];
  contact->normal[0] = *box->R[0][n] * sign[n];
  contact->normal[1] = *box->R[1][n] * sign[n];
  contact->normal[2] = *box->R[2][n] * sign[n];
  contact->depth = alpha;
  return 1;
}


int dCollideRayCCylinder (dxGeom *o1, dxGeom *o2,
			  int flags, dContactGeom *contact, int skip)
{
  dIASSERT (skip >= (int)sizeof(dContactGeom));
  dIASSERT (o1->type == dRayClass);
  dIASSERT (o2->type == dCCylinderClass);
  dxRay *ray = (dxRay*) o1;
  dxCCylinder *ccyl = (dxCCylinder*) o2;

  contact->g1 = ray;
  contact->g2 = ccyl;
  vec_t lz2 = ccyl->lz * REAL(0.5);

  // compute some useful info
  dVector3 cs,q,r;
  vec_t C,k;
  cs[0] = ray->pos[0] - ccyl->pos[0];
  cs[1] = ray->pos[1] - ccyl->pos[1];
  cs[2] = ray->pos[2] - ccyl->pos[2];
  k = dDOT41(&((*ccyl->R)[0][2]), cs);	// position of ray start along ccyl axis
  q[0] = k * *ccyl->R[0][2] - cs[0];
  q[1] = k * *ccyl->R[1][2] - cs[1];
  q[2] = k * *ccyl->R[2][2] - cs[2];
  C = dDOT(q,q) - ccyl->radius*ccyl->radius;
  // if C < 0 then ray start position within infinite extension of cylinder

  // see if ray start position is inside the capped cylinder
  int inside_ccyl = 0;
  if (C < 0) {
    if (k < -lz2) k = -lz2;
    else if (k > lz2) k = lz2;
    r[0] = ccyl->pos[0] + k * *ccyl->R[0][2];
    r[1] = ccyl->pos[1] + k * *ccyl->R[1][2];
    r[2] = ccyl->pos[2] + k * *ccyl->R[2][2];
    if ((ray->pos[0]-r[0])*(ray->pos[0]-r[0]) +
	(ray->pos[1]-r[1])*(ray->pos[1]-r[1]) +
	(ray->pos[2]-r[2])*(ray->pos[2]-r[2]) < ccyl->radius*ccyl->radius) {
      inside_ccyl = 1;
    }
  }

  // compute ray collision with infinite cylinder, except for the case where
  // the ray is outside the capped cylinder but within the infinite cylinder
  // (it that case the ray can only hit endcaps)
  if (!inside_ccyl && C < 0) {
    // set k to cap position to check
    if (k < 0) k = -lz2; else k = lz2;
  }
  else {
    vec_t uv = dDOT44(&((*ccyl->R)[0][2]), &((*ray->R[0][2])));
    r[0] = uv * *ccyl->R[0][2] - *ray->R[0][2];
    r[1] = uv * *ccyl->R[1][2] - *ray->R[1][2];
    r[2] = uv * *ccyl->R[2][2] - *ray->R[2][2];
    vec_t A = dDOT(r,r);
    vec_t B = 2*dDOT(q,r);
    k = B*B-4*A*C;
    if (k < 0) {
      // the ray does not intersect the infinite cylinder, but if the ray is
      // inside and parallel to the cylinder axis it may intersect the end
      // caps. set k to cap position to check.
      if (!inside_ccyl) return 0;
      if (uv < 0) k = -lz2; else k = lz2;
    }
    else {
      k = X_sqrt(k);
      A = X_recip (2*A);
      vec_t alpha = (-B-k)*A;
      if (alpha < 0) {
	alpha = (-B+k)*A;
	if (alpha < 0) return 0;
      }
      if (alpha > ray->length) return 0;

      // the ray intersects the infinite cylinder. check to see if the
      // intersection point is between the caps
      contact->pos[0] = ray->pos[0] + alpha * *ray->R[0][2];
      contact->pos[1] = ray->pos[1] + alpha * *ray->R[1][2];
      contact->pos[2] = ray->pos[2] + alpha * *ray->R[2][2];
      q[0] = contact->pos[0] - ccyl->pos[0];
      q[1] = contact->pos[1] - ccyl->pos[1];
      q[2] = contact->pos[2] - ccyl->pos[2];
      k = dDOT14(q, &((*ccyl->R)[0][2]));
      vec_t nsign = inside_ccyl ? REAL(-1.0) : REAL(1.0);
      if (k >= -lz2 && k <= lz2) {
	contact->normal[0] = nsign * (contact->pos[0] -
				      (ccyl->pos[0] + k * *ccyl->R[0][2]));
	contact->normal[1] = nsign * (contact->pos[1] -
				      (ccyl->pos[1] + k * *ccyl->R[1][2]));
	contact->normal[2] = nsign * (contact->pos[2] - 
				      (ccyl->pos[2] + k * *ccyl->R[2][2]));
	dNormalize3 (contact->normal);
	contact->depth = alpha;
	return 1;
      }

      // the infinite cylinder intersection point is not between the caps.
      // set k to cap position to check.
      if (k < 0) k = -lz2; else k = lz2;
    }
  }

  // check for ray intersection with the caps. k must indicate the cap
  // position to check
  q[0] = ccyl->pos[0] + k * *ccyl->R[0][2];
  q[1] = ccyl->pos[1] + k * *ccyl->R[1][2];
  q[2] = ccyl->pos[2] + k * *ccyl->R[2][2];
  return ray_sphere_helper (ray,q,ccyl->radius,contact, inside_ccyl);
}


int dCollideRayPlane (dxGeom *o1, dxGeom *o2, int flags,
		      dContactGeom *contact, int skip)
{
  dIASSERT (skip >= (int)sizeof(dContactGeom));
  dIASSERT (o1->type == dRayClass);
  dIASSERT (o2->type == dPlaneClass);
  dxRay *ray = (dxRay*) o1;
  dxPlane *plane = (dxPlane*) o2;

  vec_t alpha = plane->p[3] - dDOT (plane->p,ray->pos);
  // note: if alpha > 0 the starting point is below the plane
  vec_t nsign = (alpha > 0) ? REAL(-1.0) : REAL(1.0);
  vec_t k = dDOT14(plane->p, &((*ray->R)[0][2]));
  if (k==0) return 0;		// ray parallel to plane
  alpha /= k;
  if (alpha < 0 || alpha > ray->length) return 0;
  contact->pos[0] = ray->pos[0] + alpha * *ray->R[0][2];
  contact->pos[1] = ray->pos[1] + alpha * *ray->R[1][2];
  contact->pos[2] = ray->pos[2] + alpha * *ray->R[2][2];
  contact->normal[0] = nsign*plane->p[0];
  contact->normal[1] = nsign*plane->p[1];
  contact->normal[2] = nsign*plane->p[2];
  contact->depth = alpha;
  contact->g1 = ray;
  contact->g2 = plane;
  return 1;
}

#if 1
void dBoxLeafnums (dxBSP *bsp, const vec_t aabb[6], dArray<int> &leafs, int nodenum)
{
  dIASSERT (bsp->type == dBSPClass);

  while(nodenum >= 0) {
    const dxBSP::dBSPNode& node = bsp->nodes[nodenum];
    
    int s = dBoxOnPlaneSide(node.plane, aabb);

    if(s == dPlaneSideFront) {
      nodenum = node.children[0];
    }
    else if(s == dPlaneSideBack) {
      nodenum = node.children[1];
    }
    else {	// go down both
      dBoxLeafnums(bsp, aabb, leafs, node.children[0]);
      nodenum = node.children[1];
    }
  }
  
  leafs.push(-1 - nodenum);
}


int dBoxInBrush (dxGeom *o1, dxGeom *o2, int flags,
		      dContactGeom *contact, int skip, const dxBSP::dBSPBrush &brush)
{
  dIASSERT (skip >= (int)sizeof(dContactGeom));
  dIASSERT (o1->type == dBSPClass);
  dIASSERT (o2->type == dBoxClass);
  dxBSP *bsp = (dxBSP*) o1;
  dxBox *box = (dxBox*) o2;
  
  contact->g1 = o1;
  contact->g2 = o2;
  int ret = 0;

  //@@@ problem: using 4-vector (plane->p) as 3-vector (normal).
  const vec_t *R = (vec_t*)*o1->R;		// rotation of box
  
  // find number of contacts requested
  int maxc = flags & NUMC_MASK;
  if (maxc < 1) maxc = 1;
  if (maxc > 3) maxc = 3;	// no more than 3 contacts per box allowed
  
  for (int i=0; i<brush.sides_num; i++) {
    int contacts_num = 0;
    const vec_t *plane = bsp->brushsides[brush.sides_first + i].plane;
    const vec_t *n = plane;		// normal vector

    // project sides lengths along normal vector, get absolute values
    vec_t Q1 = dDOT14(n,R+0);
    vec_t Q2 = dDOT14(n,R+1);
    vec_t Q3 = dDOT14(n,R+2);
    vec_t A1 = box->side[0] * Q1;
    vec_t A2 = box->side[1] * Q2;
    vec_t A3 = box->side[2] * Q3;
    vec_t B1 = X_fabs(A1);
    vec_t B2 = X_fabs(A2);
    vec_t B3 = X_fabs(A3);
    
    // early exit test
    vec_t depth = plane[3] + REAL(0.5)*(B1+B2+B3) - dDOT(n,o1->pos);
    if (depth < 0) return 0;
    
    // find deepest point
    dVector3 p;
    p[0] = o1->pos[0];
    p[1] = o1->pos[1];
    p[2] = o1->pos[2];
#define FOO(i,op) \
  p[0] op REAL(0.5)*box->side[i] * R[0+i]; \
  p[1] op REAL(0.5)*box->side[i] * R[4+i]; \
  p[2] op REAL(0.5)*box->side[i] * R[8+i];
#define BAR(i,iinc) if (A ## iinc > 0) { FOO(i,-=) } else { FOO(i,+=) }
    BAR(0,1);
    BAR(1,2);
    BAR(2,3);
#undef FOO
#undef BAR

    // the deepest point is the first contact point
    contact->pos[0] = p[0];
    contact->pos[1] = p[1];
    contact->pos[2] = p[2];
    contact->normal[0] = n[0];
    contact->normal[1] = n[1];
    contact->normal[2] = n[2];
    contact->depth = depth;
    contacts_num = 1;		// ret is number of contact points found so far
    if (maxc == 1) goto done;
    
    // get the second and third contact points by starting from `p' and going
    // along the two sides with the smallest projected length.

#define FOO(i,j,op) \
  CONTACT(contact,i*skip)->pos[0] = p[0] op box->side[j] * R[0+j]; \
  CONTACT(contact,i*skip)->pos[1] = p[1] op box->side[j] * R[4+j]; \
  CONTACT(contact,i*skip)->pos[2] = p[2] op box->side[j] * R[8+j];
#define BAR(ctact,side,sideinc) \
  depth -= B ## sideinc; \
  if (depth < 0) goto done; \
  if (A ## sideinc > 0) { FOO(ctact,side,+) } else { FOO(ctact,side,-) } \
  CONTACT(contact,ctact*skip)->depth = depth; \
  ret++;

    CONTACT(contact,skip)->normal[0] = n[0];
    CONTACT(contact,skip)->normal[1] = n[1];
    CONTACT(contact,skip)->normal[2] = n[2];
    if (maxc == 3) {
      CONTACT(contact,2*skip)->normal[0] = n[0];
      CONTACT(contact,2*skip)->normal[1] = n[1];
      CONTACT(contact,2*skip)->normal[2] = n[2];
    }

    if (B1 < B2) {
      if (B3 < B1) goto use_side_3; else {
        BAR(1,0,1);	// use side 1
        if (maxc == 2) goto done;
        if (B2 < B3) goto contact2_2; else goto contact2_3;
      }
    }
    else {
      if (B3 < B2) {
        use_side_3:	// use side 3
        BAR(1,2,3);
        if (maxc == 2) goto done;
        if (B1 < B2) goto contact2_1; else goto contact2_2;
      }
      else {
        BAR(1,1,2);	// use side 2
        if (maxc == 2) goto done;
        if (B1 < B3) goto contact2_1; else goto contact2_3;
      }
    }

    contact2_1: BAR(2,0,1); goto done;
    contact2_2: BAR(2,1,2); goto done;
    contact2_3: BAR(2,2,3); goto done;
#undef FOO
#undef BAR

    done:
      ret += contacts_num;
  }
    
  for (int i=0; i<ret; i++) {
    CONTACT(contact,i*skip)->g1 = o1;
    CONTACT(contact,i*skip)->g2 = o2;
  }
    
  return ret;
}

int dBoxInLeaf (dxGeom *o1, dxGeom *o2, int flags,
		      dContactGeom *contact, int skip, int leafnum)
{
  dIASSERT (skip >= (int)sizeof(dContactGeom));
  dIASSERT (o1->type == dBSPClass);
  dIASSERT (o2->type == dBoxClass);
  dxBSP *bsp = (dxBSP*) o1;
  
  const dxBSP::dBSPLeaf& leaf = bsp->leafs[leafnum];

  //if(!(leaf.contents & trace_contents))
  //  return;
  
  int contacts_num = 0;
  for(int i=0; i<leaf.brushes_num; i++) {
    dxBSP::dBSPBrush& brush = bsp->brushes[bsp->leafbrushes[leaf.brushes_first + i]];
    
    if(brush.checkcount == bsp->checkcount)
      continue;	// already checked this brush in another leaf
      
    brush.checkcount = bsp->checkcount;
    
    //if(!(brush.contents & trace_contents))
      //continue;
      
    contacts_num += dBoxInBrush(o1, o2, flags, contact, skip, brush);
    //if(!trace_trace.fraction)
      //return;
  }
  
  if (!contacts_num) {
    dDEBUGMSG ("no contacts");
  }
  else {
    dDEBUGMSG ("contacts!!!");
  }
  
  return contacts_num;
}

int dCollideBSPSphere (dxGeom *o1, dxGeom *o2, int flags,
		      dContactGeom *contact, int skip)
{
  dIASSERT (skip >= (int)sizeof(dContactGeom));
  dIASSERT (o1->type == dBSPClass);
  dIASSERT (o2->type == dSphereClass);
  dDEBUGMSG ("");
  return 0;
}

int dCollideBSPBox (dxGeom *o1, dxGeom *o2, int flags,
		      dContactGeom *contact, int skip)
{
  dIASSERT (skip >= (int)sizeof(dContactGeom));
  dIASSERT (o1->type == dBSPClass);
  dIASSERT (o2->type == dBoxClass);
  dDEBUGMSG ("");
#if 0
  dxBSP *bsp = (dxBSP*) o1;
  dxBox *box = (dxBox*) o2;
  
  vec_t aabb[6];
  aabb[0] = box->aabb[0]- 1.0f;
  aabb[1] = box->aabb[1]+ 1.0f;
  aabb[2] = box->aabb[2]- 1.0f;
  aabb[3] = box->aabb[3]+ 1.0f;
  aabb[4] = box->aabb[4]- 1.0f;
  aabb[5] = box->aabb[5]+ 1.0f;
  
  bsp->checkcount++;
  
  dArray<int> leafs;
  dBoxLeafnums(bsp, aabb, leafs, 0);
  if (!leafs.size()) {
    dDEBUGMSG ("no BSP leaves hit");
    return 0;
  }
  
  int contacts_num = 0;
  for (int i=0; i<leafs.size(); i++) {
    contacts_num += dBoxInLeaf(o1, o2, flags, contact, skip, leafs[i]);
  }
  
  if (!contacts_num) {
    dDEBUGMSG ("no contacts");
  }
  else {
    dDEBUGMSG ("contacts!!!");
  }
  
  return contacts_num;
#else
  return 0;
#endif
}

int dCollideBSPCCylinder (dxGeom *o1, dxGeom *o2, int flags,
		      dContactGeom *contact, int skip)
{
  dIASSERT (skip >= (int)sizeof(dContactGeom));
  dIASSERT (o1->type == dBSPClass);
  dIASSERT (o2->type == dCCylinderClass);
  dDEBUGMSG ("");
  return 0;
}

int dCollideBSPCylinder (dxGeom *o1, dxGeom *o2, int flags,
		      dContactGeom *contact, int skip)
{
  dIASSERT (skip >= (int)sizeof(dContactGeom));
  dIASSERT (o1->type == dBSPClass);
  dIASSERT (o2->type == dCylinderClass);
  dDEBUGMSG ("");
  return 0;
}

int dCollideBSPPlane (dxGeom *o1, dxGeom *o2, int flags,
		      dContactGeom *contact, int skip)
{
  dIASSERT (skip >= (int)sizeof(dContactGeom));
  dIASSERT (o1->type == dBSPClass);
  dIASSERT (o2->type == dPlaneClass);
  dDEBUGMSG ("");
  return 0;
}

int dCollideBSPRay (dxGeom *o1, dxGeom *o2, int flags,
		      dContactGeom *contact, int skip)
{
  dIASSERT (skip >= (int)sizeof(dContactGeom));
  dIASSERT (o1->type == dBSPClass);
  dIASSERT (o2->type == dRayClass);
  dDEBUGMSG ("");
  return 0;
}
#endif

