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
#include "../files.h"

#ifdef _MSC_VER
#pragma warning(disable:4291)  // for VC++, no complaints about "no matching operator delete found"
#endif

//****************************************************************************
// sphere public API

dxSphere::dxSphere (dSpaceID space, vec_t _radius) : dxGeom (space,1)
{
  dAASSERT (_radius > 0);
  type = dSphereClass;
  radius = _radius;
}


void	dxSphere::computeAABB()
{
//	Com_Printf("dxSphere::computeAABB:\n");
	
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
	dUASSERT(g && g->type == dSphereClass, "argument not a sphere");
	
	dxSphere *s = (dxSphere*)g;
	return s->radius - X_sqrt((x-s->pos[0])*(x-s->pos[0]) + (y-s->pos[1])*(y-s->pos[1]) + (z-s->pos[2])*(z-s->pos[2]));
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


dGeomID	dCreateBox(dSpaceID space, vec_t lx, vec_t ly, vec_t lz)
{
  return new dxBox(space,lx,ly,lz);
}


void	dGeomBoxSetLengths(dGeomID g, vec_t lx, vec_t ly, vec_t lz)
{
	dUASSERT(g && g->type == dBoxClass, "argument not a box");
	dAASSERT(lx > 0 && ly > 0 && lz > 0);
	dxBox *b = (dxBox*) g;
	b->side[0] = lx;
	b->side[1] = ly;
	b->side[2] = lz;
	dGeomMoved(g);
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
// plane public API

static void	make_sure_plane_normal_has_unit_length(dxPlane *g)
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

dxBSP::dxBSP(dSpaceID space) : dxGeom(space, 1)
{
	dDEBUGMSG("dxBSP");
	
	type = dBSPClass;
	side[0] = 0;//X_infinity;
	side[1] = 0;//X_infinity;
	side[2] = 0;//X_infinity;
	
	checkcount = 0;
}

void dxBSP::computeAABB()
{
	dDEBUGMSG("");
#if 0	
	vec_t xrange = REAL(0.5) * (X_fabs((*R)[0][0] * side[0]) + X_fabs((*R)[0][1] * side[1]) + X_fabs((*R)[0][2] * side[2]));
	vec_t yrange = REAL(0.5) * (X_fabs((*R)[1][0] * side[0]) + X_fabs((*R)[1][1] * side[1]) + X_fabs((*R)[1][2] * side[2]));
	vec_t zrange = REAL(0.5) * (X_fabs((*R)[2][0] * side[0]) + X_fabs((*R)[2][1] * side[1]) + X_fabs((*R)[2][2] * side[2]));
	
	aabb[0] = pos[0] - xrange;
	aabb[1] = pos[0] + xrange;
	aabb[2] = pos[1] - yrange;
	aabb[3] = pos[1] + yrange;
	aabb[4] = pos[2] - zrange;
	aabb[5] = pos[2] + zrange;
#else
	aabb[0] = -X_infinity;
	aabb[1] = X_infinity;
	aabb[2] = -X_infinity;
	aabb[3] = X_infinity;
	aabb[4] = -X_infinity;
	aabb[5] = X_infinity;
#endif
}

dGeomID	dCreateBSP(dSpaceID space)
{
	return new dxBSP(space);
}

void	dGeomBSPSetLengths(dGeomID g, vec_t lx, vec_t ly, vec_t lz)
{
	dUASSERT(g && g->type == dBSPClass, "argument not a BSP");
	dAASSERT(lx > 0 && ly > 0 && lz > 0);
	dxBSP *bsp = (dxBSP*)g;
	
	bsp->side[0] = lx;
	bsp->side[1] = ly;
	bsp->side[2] = lz;
	
	dGeomMoved(g);
}

void	dGeomBSPAddPlane(dGeomID g, vec_t a, vec_t b, vec_t c, vec_t d)
{
	dUASSERT(g && g->type == dBSPClass, "argument not a BSP");
	dxBSP *bsp = (dxBSP*)g;
	
	cplane_c p;
	p.set(a, b, c, d);
	
	bsp->planes.push_back(p);
}

/*
void	dGeomBSPAddBrush(dGeomID g, int sides_first, int sides_num)
{
	dUASSERT(g && g->type == dBSPClass, "argument not a BSP");
	dxBSP *bsp = (dxBSP*)g;
	
	dxBSP::dBSPBrush b;
	b.sides_first = sides_first;
	b.sides_num = sides_num;
	
	bsp->brushes.push(b);
}

void	dGeomBSPAddBrushSide(dGeomID g, int plane_num)
{
	dUASSERT(g && g->type == dBSPClass, "argument not a BSP");
	dxBSP *bsp = (dxBSP*)g;
	
	dxBSP::dBSPBrushSide s;
	s.plane = &bsp->planes[plane_num];
	
	bsp->brushsides.push(s);
}
*/

void	dGeomBSPAddNode(dGeomID g, int plane_num, int child0, int child1)
{
	dUASSERT(g && g->type == dBSPClass, "argument not a BSP");
	dxBSP *bsp = (dxBSP*)g;
	
	dxBSP::dBSPNode n;
	n.plane = &bsp->planes[plane_num];
	n.children[0] = child0;
	n.children[1] = child1;
	
	bsp->nodes.push_back(n);
}

void	dGeomBSPAddLeaf(dGeomID g, int surfaces_first, int surfaces_num, int brushes_first, int brushes_num, int cluster, int area)
{
	dUASSERT(g && g->type == dBSPClass, "argument not a BSP");
	dxBSP *bsp = (dxBSP*)g;
	
	dxBSP::dBSPLeaf l;
	l.surfaces_first = surfaces_first;
	l.surfaces_num = surfaces_num;
	
	l.brushes_first = brushes_first;
	l.brushes_num = brushes_num;
	
	l.cluster = cluster;
	l.area = area;
	
	bsp->leafs.push_back(l);
}

/*
void	dGeomBSPAddLeafBrush(dGeomID g, int num)
{
	dUASSERT(g && g->type == dBSPClass, "argument not a BSP");
	dxBSP *bsp = (dxBSP*)g;
	
	bsp->leafbrushes.push(num);
}
*/

void	dGeomBSPAddShader(dGeomID g, int flags, int contents)
{
	dUASSERT(g && g->type == dBSPClass, "argument not a BSP");
	dxBSP *bsp = (dxBSP*)g;
	
	dxBSP::dBSPShader shader;
	shader.flags = flags;
	shader.contents = contents;
	
	bsp->shaders.push_back(shader);
}

void	dGeomBSPAddSurface(dGeomID g, int face_type, int shader_num, const std::vector<vec3_c> &vertexes, const std::vector<index_t> &indexes)
{
	dUASSERT(g && g->type == dBSPClass, "argument not a BSP");
	dxBSP *bsp = (dxBSP*)g;
	
	dxBSP::dBSPSurface surf;
	surf.face_type = face_type;
	surf.shader_num = shader_num;
	surf.vertexes = vertexes;
	surf.indexes = indexes;
	std::reverse(surf.indexes.begin(), surf.indexes.end());	// change from CCW to CW order
	
	switch(surf.face_type)
	{
		case BSPST_PLANAR:
		{
			// try to compute only one plane for all triangles
		
			if(indexes.size() >= 3)
				surf.planes.resize(1);
				
			uint_t j=0;
			for(uint_t i=0; i<indexes.size(); i += 3, j++)
			{
				try
				{
					const vec3_c &v0 = vertexes.at(indexes[i+0]);
					const vec3_c &v1 = vertexes.at(indexes[i+1]);
					const vec3_c &v2 = vertexes.at(indexes[i+2]);
				
					if(surf.plane.fromThreePointForm(v0, v1, v2))
						break;
				}
				catch(...)
				{
					Com_Error(ERR_DROP, "dGeomBSPAddSurface: exception occured while calculating triangle planes");
				}	
			}
			
			if(j == indexes.size())
			{
				Com_Error(ERR_DROP, "dGeomBSPAddSurface: degenerated triangle");
			}
			break;
		}
		
		case BSPST_BEZIER:
		case BSPST_MESH:
		{
			// try to compute one plane for each triangle
		
			if(indexes.size() >= 3)
				surf.planes.resize(indexes.size()/3);
		
			for(uint_t i=0, j=0; i<indexes.size(); i += 3, j++)
			{
				try
				{
					const vec3_c &v0 = vertexes.at(indexes[i+0]);
					const vec3_c &v1 = vertexes.at(indexes[i+1]);
					const vec3_c &v2 = vertexes.at(indexes[i+2]);
					
					if(!(surf.planes[j].fromThreePointForm(v0, v1, v2)))
						Com_Error(ERR_DROP, "dGeomBSPAddSurface: degenerated triangle");
				}
				catch(...)
				{
					Com_Error(ERR_DROP, "dGeomBSPAddSurface: exception occured while calculating triangle planes");
				}
			}
			break;
		}
		
		default:
			break;
	}
	
	bsp->surfaces.push_back(surf);
}

void	dGeomBSPAddLeafSurface(dGeomID g, int num)
{
	dUASSERT(g && g->type == dBSPClass, "argument not a BSP");
	dxBSP *bsp = (dxBSP*)g;
	
	bsp->leafsurfaces.push_back(num);
}

//****************************************************************************
// box-box collision utility


// find all the intersection points between the 2D rectangle with vertices
// at (+/-h[0],+/-h[1]) and the 2D quadrilateral with vertices (p[0],p[1]),
// (p[2],p[3]),(p[4],p[5]),(p[6],p[7]).
//
// the intersection points are returned as x,y pairs in the 'ret' array.
// the number of intersection points is returned by the function (this will
// be in the range 0 to 8).

int intersectRectQuad (vec_t h[2], vec_t p[8], vec_t ret[16])
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
#if DEBUG
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
#if DEBUG
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
/*
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

    for (i=0; i<3; i++)
    	contact[0].pos[i] = REAL(0.5)*(pa[i]+pb[i]);
	
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
*/
//****************************************************************************
// pairwise collision functions for standard geom types

int	dCollideSphereSphere(dxGeom *o1, dxGeom *o2, int flags, std::vector<dContact> &contacts)
{
	dIASSERT(o1->type == dSphereClass);
	dIASSERT(o2->type == dSphereClass);
	dxSphere *sphere1 = (dxSphere*)o1;
	dxSphere *sphere2 = (dxSphere*)o2;
	
	dContact contact;
	
	contact.geom._g1 = o1;
	contact.geom._g2 = o2;
	
	if(dCollideSpheres(o1->pos, sphere1->radius, o2->pos, sphere2->radius, contact.geom))
	{
		if(dAddContact(contact, contacts))
			return 1;
	}
	
	return 0;
}


int	dCollideSphereBox (dxGeom *o1, dxGeom *o2, int flags, std::vector<dContact> &contacts)
{
  // this is easy. get the sphere center `p' relative to the box, and then clip
  // that to the boundary of the box (call that point `q'). if q is on the
  // boundary of the box and |p-q| is <= sphere radius, they touch.
  // if q is inside the box, the sphere is inside the box, so set a contact
  // normal to push the sphere to the closest box face.

	dIASSERT(o1->type == dSphereClass);
	dIASSERT(o2->type == dBoxClass);
#if 0
   dVector3 l,t,p,q,r;
  vec_t depth;
  int onborder = 0;
  
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
    dMULTIPLY0_331((vec_t*)contact->normal, (vec_t*)o2->R, tmp);
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
#else
	return 0;
#endif
}

int	dCollideSpherePlane(dxGeom *o1, dxGeom *o2, int flags, std::vector<dContact> &contacts)
{
	dIASSERT(o1->type == dSphereClass);
	dIASSERT(o2->type == dPlaneClass);
	
	dxSphere *sphere = (dxSphere*)o1;
	dxPlane *plane = (dxPlane*)o2;
	
	// get closest distance to sphere origin
	vec_t d = plane->p.distance(sphere->pos);
	
	// check if sphere center is in negative half-space of the plane
//	if(d < REAL(0.0))
//		return 0;

	// get closest point to sphere origin
	vec3_c c = vec3_c(sphere->pos) - (plane->p._normal * d);
		
	// get sphere depth using this closest point in the plane
	vec_t depth = sphere->radius - c.distance(sphere->pos);
		
	if(depth >= REAL(0.0))
	{
		if(dAddContact(dContact(dContactGeom(c, plane->p._normal, depth, o1, o2)), contacts));
			return 1;
	}

	return 0;
}


int	dCollideBoxBox(dxGeom *o1, dxGeom *o2, int flags, std::vector<dContact> &contacts)
{
	dIASSERT(o1->type == dBoxClass);
	dIASSERT(o2->type == dBoxClass);
	
	dxBox *b1 = (dxBox*) o1;
	dxBox *b2 = (dxBox*) o2;
	
#if 0
	const vec3_c& p1 = *b1->pos;
	const vec3_c& p2 = *b2->pos;

	// get side lengths / 2
	const vec3_c A
	(
		b1->side[0]*REAL(0.5),
		b1->side[1]*REAL(0.5),
		b1->side[2]*REAL(0.5)
	);
	
	const vec3_c B
	(
		b2->side[0]*REAL(0.5),
		b2->side[1]*REAL(0.5),
		b2->side[2]*REAL(0.5)
	);
	
	// create object space vertices of b1
	vec3_c V1[8] = 
	{
		( A[0],-A[1],-A[2]),	//(_maxs[0], _mins[1], _mins[2]),
		( A[0],-A[1], A[2]),	//(_maxs[0], _mins[1], _maxs[2]);
		(-A[0],-A[1], A[2]),	//(_mins[0], _mins[1], _maxs[2]);
		(-A[0],-A[1],-A[2]),	//(_mins[0], _mins[1], _mins[2]);
		
		( A[0], A[1],-A[2]),	//(_maxs[0], _maxs[1], _mins[2]);
		( A[0], A[1], A[2]),	//(_maxs[0], _maxs[1], _maxs[2]);
		(-A[0], A[1], A[2]),	//(_mins[0], _maxs[1], _maxs[2]);
		(-A[0], A[1],-A[2])	//(_mins[0], _maxs[1], _mins[2]);
	};
	
	// create object space vertices of b2
	vec3_c V2[8] = 
	{
		( B[0],-B[1],-B[2]),
		( B[0],-B[1], B[2]),
		(-B[0],-B[1], B[2]),
		(-B[0],-B[1],-B[2]),
		
		( B[0], B[1],-B[2]),
		( B[0], B[1], B[2]),
		(-B[0], B[1], B[2]),
		(-B[0], B[1],-B[2])
	};
	
	// transform vertices to world space
	for(int i=0; i<8; i++)
	{
		V1[i].rotate(*b1->R);
		V1[i] += p1;
				
		V2[i].rotate(*b2->R);
		V2[i] += p2;
	}
	
	int contacts_num = 0;
	for(int i=0; i<8; i++)
	{
		vec_t depth = dGeomBoxPointDepth(o2, V1[i][0], V1[i][1], V1[i][2]);
		
		if(depth >= REAL(0.0))
		{
			vec3_c normal = V1[i] - p2;
			normal.normalize();
		
			if(dAddContact(dContact(dContactGeom(V1[i], normal, depth, o1, o2)), contacts));
				contacts_num++;
		}
	}
	
	for(int i=0; i<8; i++)
	{
		vec_t depth = dGeomBoxPointDepth(o1, V2[i][0], V2[i][1], V2[i][2]);
		
		if(depth >= REAL(0.0))
		{
			vec3_c normal = V2[i] - p1;
			normal.normalize();
		
			if(dAddContact(dContact(dContactGeom(V2[i], normal, depth, o2, o1)), contacts));
				contacts_num++;
		}
	}
	
	return contacts_num;
#else

	const vec_t fudge_factor = REAL(1.05);
	dVector3 p, pp, normalC;
	const vec_t* normalR = 0;
	int contacts_num = 0;
	
	vec_t R11, R12, R13, R21, R22, R23, R31, R32, R33;
	vec_t Q11, Q12, Q13, Q21, Q22, Q23, Q31, Q32, Q33;
	vec_t s, s2, l;
	int i, j, invert_normal, code;
	
	const vec3_c& p1 = *b1->pos;
	const vec3_c& p2 = *b2->pos;
	
	const matrix_c& R1 = *b1->R;
	const matrix_c& R2 = *b2->R;
	
	// get vector from centers of box 1 to box 2, relative to box 1
	p[0] = b2->pos[0] - b1->pos[0];
	p[1] = b2->pos[1] - b1->pos[1];
	p[2] = b2->pos[2] - b1->pos[2];
	
	dMULTIPLY1_331(pp, (vec_t*)*o1->R, p);		// get pp = p relative to body 1
	
	// get side lengths / 2
	const vec3_c A
	(
		b1->side[0]*REAL(0.5),
		b1->side[1]*REAL(0.5),
		b1->side[2]*REAL(0.5)
	);
	
	const vec3_c B
	(
		b2->side[0]*REAL(0.5),
		b2->side[1]*REAL(0.5),
		b2->side[2]*REAL(0.5)
	);
	
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
	TST(pp[0],(A[0] + B[0]*Q11 + B[1]*Q12 + B[2]*Q13), R1+0, 1);
	TST(pp[1],(A[1] + B[0]*Q21 + B[1]*Q22 + B[2]*Q23), R1+1, 2);
	TST(pp[2],(A[2] + B[0]*Q31 + B[1]*Q32 + B[2]*Q33), R1+2, 3);
	
	// separating axis = v1,v2,v3
	TST(dDOT41(R2+0, p),(A[0]*Q11 + A[1]*Q21 + A[2]*Q31 + B[0]), R2+0, 4);
	TST(dDOT41(R2+1, p),(A[0]*Q12 + A[1]*Q22 + A[2]*Q32 + B[1]), R2+1, 5);
	TST(dDOT41(R2+2, p),(A[0]*Q13 + A[1]*Q23 + A[2]*Q33 + B[2]), R2+2, 6);
	
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
  
	// separating axis = u1 x (v1, v2, v3)
	TST(pp[2]*R21-pp[1]*R31, (A[1]*Q31+A[2]*Q21+B[1]*Q13+B[2]*Q12), 0, -R31, R21, 7);
	TST(pp[2]*R22-pp[1]*R32, (A[1]*Q32+A[2]*Q22+B[0]*Q13+B[2]*Q11), 0, -R32, R22, 8);
	TST(pp[2]*R23-pp[1]*R33, (A[1]*Q33+A[2]*Q23+B[0]*Q12+B[1]*Q11), 0, -R33, R23, 9);
	
	// separating axis = u2 x (v1,v2,v3)
	TST(pp[0]*R31-pp[2]*R11, (A[0]*Q31+A[2]*Q11+B[1]*Q23+B[2]*Q22), R31, 0, -R11, 10);
	TST(pp[0]*R32-pp[2]*R12, (A[0]*Q32+A[2]*Q12+B[0]*Q23+B[2]*Q21), R32, 0, -R12, 11);
	TST(pp[0]*R33-pp[2]*R13, (A[0]*Q33+A[2]*Q13+B[0]*Q22+B[1]*Q21), R33, 0, -R13, 12);
	
	// separating axis = u3 x (v1,v2,v3)
	TST(pp[1]*R11-pp[0]*R21, (A[0]*Q21+A[1]*Q11+B[1]*Q33+B[2]*Q32), -R21, R11, 0, 13);
	TST(pp[1]*R12-pp[0]*R22, (A[0]*Q22+A[1]*Q12+B[0]*Q33+B[2]*Q31), -R22, R12, 0, 14);
	TST(pp[1]*R13-pp[0]*R23, (A[0]*Q23+A[1]*Q13+B[0]*Q32+B[1]*Q31), -R23, R13, 0, 15);

#undef TST

	if(!code)
		return contacts_num;
		
	// if we get to this point, the boxes interpenetrate. compute the normal
	// in global coordinates.
	vec3_c normal;
	
	if(normalR)
	{
		normal[0] = normalR[0];
		normal[1] = normalR[4];
		normal[2] = normalR[8];
	}
	else
	{
		dMULTIPLY0_331((vec_t*)normal, (vec_t*)R1, normalC);
	}
	
	if(invert_normal)
		normal.negate();
	
	vec_t depth = -s;
	
	// compute contact point(s)
	if(code > 6)
	{
		// an edge from box 1 touches an edge from box 2.
		// find a point pa on the intersecting edge of box 1
		vec3_c pa(p1);
		vec_t sign;
			
		for(j=0; j<3; j++)
		{
			sign = (dDOT14(normal,R1+j) > 0) ? REAL(1.0) : REAL(-1.0);
			
			for(i=0; i<3; i++)
				pa[i] += sign * A[j] * R1[i][j];
		}
		
		// find a point pb on the intersecting edge of box 2
		vec3_c pb(p2);
		
		for(j=0; j<3; j++)
		{
			sign = (dDOT14(normal,R2+j) > 0) ? REAL(-1.0) : REAL(1.0);
			
			for(i=0; i<3; i++)
				pb[i] += sign * B[j] * R2[i][j];
		}
		
		vec_t alpha, beta;
		vec3_c ua, ub;
		
		for(i=0; i<3; i++)
			ua[i] = R1[i][((code)-7)/3];
		
		for(i=0; i<3; i++)
			ub[i] = R2[i][((code)-7)%3];
			
		dLineClosestApproach(pa, ua, pb, ub, &alpha, &beta);
		
		pa += (ua*alpha);
			
		pb += (ub*beta);
			
		vec3_c c = (pa+pb)*REAL(0.5);
			
		if(dAddContact(dContact(dContactGeom(c, -normal, depth, o1, o2)), contacts));
			contacts_num++;
			
		return contacts_num;
	}
	
	// okay, we have a face-something intersection (because the separating
	// axis is perpendicular to a face). define face 'a' to be the reference
	// face (i.e. the normal vector is perpendicular to this) and face 'b' to be
	// the incident face (the closest face of the other box).
	
	const vec_t *Ra, *Rb, *Sa, *Sb;
	vec3_c pa(false), pb(false);
	
	if(code <= 3)
	{
		Ra = R1;
		Rb = R2;
		pa = b1->pos;
		pb = b2->pos;
		Sa = A;
		Sb = B;
	}
	else
	{
		Ra = R2;
		Rb = R1;
		pa = b2->pos;
		pb = b1->pos;
		Sa = B;
		Sb = A;
	}
	
	// nr = normal vector of reference face dotted with axes of incident box.
	// anr = absolute values of nr.
	vec3_c normal2, nr, anr;
	if(code <= 3)
	{
		normal2 = normal;
	}
	else
	{
		normal2 = -normal;
	}
	
	dMULTIPLY1_331((vec_t*)nr, Rb, (vec_t*)normal2);
	
	anr[0] = X_fabs(nr[0]);
	anr[1] = X_fabs(nr[1]);
	anr[2] = X_fabs(nr[2]);
	
	// find the largest compontent of anr: this corresponds to the normal
	// for the indident face. the other axis numbers of the indicent face
	// are stored in a1,a2.
	
	int lanr, a1, a2;
	if(anr[1] > anr[0])
	{
		if(anr[1] > anr[2])
		{
			a1 = 0;
			lanr = 1;
			a2 = 2;
		}
		else
		{
			a1 = 0;
			a2 = 1;
			lanr = 2;
		}
	}
	else
	{
		if(anr[0] > anr[2])
		{
			lanr = 0;
			a1 = 1;
			a2 = 2;
		}
		else
		{
			a1 = 0;
			a2 = 1;
			lanr = 2;
		}
	}
	
	// compute center point of incident face, in reference-face coordinates
	vec3_c center;
	if(nr[lanr] < 0)
	{
		for(i=0; i<3; i++)
			center[i] = pb[i] - pa[i] + Sb[lanr] * Rb[i*4+lanr];
	}
	else
	{
		for(i=0; i<3; i++)
			center[i] = pb[i] - pa[i] - Sb[lanr] * Rb[i*4+lanr];
	}
	
	// find the normal and non-normal axis numbers of the reference box
	int codeN, code1, code2;
	if(code <= 3)
		codeN = code-1;
	else
		codeN = code-4;
	
	if(codeN==0)
	{
		code1 = 1;
		code2 = 2;
	}
	else if(codeN==1)
	{
		code1 = 0;
		code2 = 2;
	}
	else
	{
		code1 = 0;
		code2 = 1;
	}
	
	// find the four corners of the incident face, in reference-face coordinates
	vec_t quad[8];	// 2D coordinate of incident face (x,y pairs)
	vec_t c1, c2;
	vec_t m11, m12, m21, m22;
	c1 = dDOT14(center, Ra+code1);
	c2 = dDOT14(center, Ra+code2);
	 
	 // optimize this? - we have already computed this data above, but it is not
	 // stored in an easy-to-index format. for now it's quicker just to recompute
	 // the four dot products.
	 m11 = dDOT44(Ra+code1, Rb+a1);
	 m12 = dDOT44(Ra+code1, Rb+a2);
	 m21 = dDOT44(Ra+code2, Rb+a1);
	 m22 = dDOT44(Ra+code2, Rb+a2);
	 
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
	 
	 // find the size of the reference face
	 vec_t rect[2];
	 rect[0] = Sa[code1];
	 rect[1] = Sa[code2];
	 
	 // intersect the incident and reference faces
	 vec_t ret[16];
	 int n = intersectRectQuad(rect, quad, ret);
	 if(n < 1)
	 	return contacts_num;		// this should never happen
		
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
	int maxc = flags & NUMC_MASK;
	for(j=0; j < n; j++)
	{
		vec_t k1 =  m22*(ret[j*2]-c1) - m12*(ret[j*2+1]-c2);
		vec_t k2 = -m21*(ret[j*2]-c1) + m11*(ret[j*2+1]-c2);
		
		for(i=0; i<3; i++)
			point[cnum*3+i] = center[i] + k1*Rb[i*4+a1] + k2*Rb[i*4+a2];
			
		dep[cnum] = Sa[codeN] - dDOT(normal2,point+cnum*3);
		
		if(dep[cnum] >= 0)
		{
			ret[cnum*2] = ret[j*2];
			ret[cnum*2+1] = ret[j*2+1];
			cnum++;
		}
	}
	
	if(cnum < 1)
		return contacts_num;	// this should never happen
		
	// we can't generate more contacts than we actually have
	if(maxc > cnum)
		maxc = cnum;
	
	if(maxc < 1)
		maxc = 1;
		
	if(cnum <= maxc)
	{
		// we have less contacts than we need, so we use them all
		for(j=0; j < cnum; j++)
		{
			vec3_c c(false);
			for(i=0; i<3; i++)
				c[i] = point[j*3+i] + pa[i];
			
			if(dAddContact(dContact(dContactGeom(c, -normal, dep[j], o1, o2)), contacts))
				contacts_num++;
			
		}
	}
	else
	{
		// we have more contacts than are wanted, some of them must be culled.
		// find the deepest point, it is always the first contact.
		int i1 = 0;
		vec_t maxdepth = dep[0];
		
		for(i=1; i<cnum; i++)
		{
			if(dep[i] > maxdepth)
			{
				maxdepth = dep[i];
				i1 = i;
			}
		}
		
		int iret[8];
		cullPoints(cnum, ret, maxc, i1, iret);
		
		for(j=0; j < maxc; j++)
		{
			vec3_c c(false);
			for(i=0; i<3; i++)
				c[i] = point[iret[j]*3+i] + pa[i];
		
			if(dAddContact(dContact(dContactGeom(c, -normal, dep[iret[j]], o1, o2)), contacts))
				contacts_num++;
		}
	}
	
	return contacts_num;
#endif
}


int	dCollideBoxPlane(dxGeom *o1, dxGeom *o2, int flags, std::vector<dContact> &contacts)
{
	dIASSERT(o1->type == dBoxClass);
	dIASSERT(o2->type == dPlaneClass);
#if 1
	dxBox *box = (dxBox*)o1;
	dxPlane *plane = (dxPlane*)o2;

	int ret = 0;
	
	const matrix_c& R = *o1->R;		// rotation of box
	const vec3_c& n = plane->p._normal;	// normal vector
	
	// project sides lengths along normal vector, get absolute values
	vec3_c Q
	(
		dDOT14(n, R+0),
		dDOT14(n, R+1),
		dDOT14(n, R+2)
	);
	
	vec3_c A
	(
		box->side[0] * Q[0],
		box->side[1] * Q[1],
		box->side[2] * Q[2]
	);
	
	vec3_c B
	(
		X_fabs(A[0]),
		X_fabs(A[1]),
		X_fabs(A[2])
	);
	
	// early exit test
	vec_t depth = plane->p[3] + REAL(0.5)*(B[0]+B[1]+B[2]) - dDOT(n, o1->pos);
	if(depth < 0)
		return 0;
		
	// find number of contacts requested
	int maxc = flags & NUMC_MASK;
	if(maxc < 1)
		maxc = 1;
	
	if(maxc > 3)
		maxc = 3;	// no more than 3 contacts per box allowed
		
	// find deepest point
	vec3_c p1(o1->pos), p2(false), p3(false);

	if(A[0] > 0)
	{
		p1[0] -= REAL(0.5)*box->side[0] * R[0][0];
		p1[1] -= REAL(0.5)*box->side[0] * R[1][0];
		p1[2] -= REAL(0.5)*box->side[0] * R[2][0];
	}
	else
	{
		p1[0] += REAL(0.5)*box->side[0] * R[0][0];
		p1[1] += REAL(0.5)*box->side[0] * R[1][0];
		p1[2] += REAL(0.5)*box->side[0] * R[2][0];
	}
	
	if(A[1] > 0)
	{
		p1[0] -= REAL(0.5)*box->side[1] * R[0][1];
		p1[1] -= REAL(0.5)*box->side[1] * R[1][1];
		p1[2] -= REAL(0.5)*box->side[1] * R[2][1];
	}
	else
	{
		p1[0] += REAL(0.5)*box->side[1] * R[0][1];
		p1[1] += REAL(0.5)*box->side[1] * R[1][1];
		p1[2] += REAL(0.5)*box->side[1] * R[2][1];
	}
	
	if(A[2] > 0)
	{
		p1[0] -= REAL(0.5)*box->side[2] * R[0][2];
		p1[1] -= REAL(0.5)*box->side[2] * R[1][2];
		p1[2] -= REAL(0.5)*box->side[2] * R[2][2];
	}
	else
	{
		p1[0] += REAL(0.5)*box->side[2] * R[0][2];
		p1[1] += REAL(0.5)*box->side[2] * R[1][2];
		p1[2] += REAL(0.5)*box->side[2] * R[2][2];
	}

	// the deepest point is the first contact point
	if(dAddContact(dContact(dContactGeom(p1, n, depth, o1, o2)), contacts));
	{
		ret++;
		
		if(maxc == 1)
			return ret;
	}
		
	// get the second and third contact points by starting from `p' and going
	// along the two sides with the smallest projected length.	
	if(B[0] < B[1])
	{
		if(B[2] < B[0])
		{
			goto use_side_3;
		}
		else
		{
			//BAR(1, 0, 1);	// use side 1
			
			depth -= B[0];
			if(depth < 0)
				return ret;
				
			if(A[0] > 0)
			{
				p2[0] = p1[0] + box->side[0] * R[0][0];
				p2[1] = p1[1] + box->side[0] * R[1][0];
				p2[2] = p1[2] + box->side[0] * R[2][0];
			}
			else
			{
				p2[0] = p1[0] - box->side[0] * R[0][0];
				p2[1] = p1[1] - box->side[0] * R[1][0];
				p2[2] = p1[2] - box->side[0] * R[2][0];
			}
			
			if(dAddContact(dContact(dContactGeom(p2, n, depth, o1, o2)), contacts));
			{
				ret++;
				
				if(maxc == 2)
					return ret;
			}
			
			if(B[1] < B[2])
				goto contact2_2;
			else
				goto contact2_3;
		}
	}
	else
	{
		if(B[2] < B[1])
		{
			use_side_3:	// use side 3
			
			//BAR(1, 2, 3);
			
			depth -= B[2];
			if(depth < 0)
				return ret;
			
			if(A[2] > 0)
			{
				p2[0] = p1[0] + box->side[2] * R[0][2];
				p2[1] = p1[1] + box->side[2] * R[1][2];
				p2[2] = p1[2] + box->side[2] * R[2][2];
			}
			else
			{
				p2[0] = p1[0] - box->side[2] * R[0][2];
				p2[1] = p1[1] - box->side[2] * R[1][2];
				p2[2] = p1[2] - box->side[2] * R[2][2];
			}
			
			if(dAddContact(dContact(dContactGeom(p2, n, depth, o1, o2)), contacts));
			{
				ret++;
				
				if(maxc == 2)
					return ret;
			}
			
			if(B[0] < B[1])
				goto contact2_1;
			else
				goto contact2_2;
		}
		else
		{
			//BAR(1, 1, 2);	// use side 2
		
			depth -= B[1];
			if(depth < 0)
				return ret;
			
			if(A[1] > 0)
			{
				p2[0] = p1[0] + box->side[1] * R[0][1];
				p2[1] = p1[1] + box->side[1] * R[1][1];
				p2[2] = p1[2] + box->side[1] * R[2][1];
			}
			else
			{
				p2[0] = p1[0] - box->side[1] * R[0][1];
				p2[1] = p1[1] - box->side[1] * R[1][1];
				p2[2] = p1[2] - box->side[1] * R[2][1];
			}
			
			if(dAddContact(dContact(dContactGeom(p2, n, depth, o1, o2)), contacts));
			{
				ret++;
				
				if(maxc == 2)
					return ret;
			}
				
			if(B[0] < B[2])
				goto contact2_1;
			else
				goto contact2_3;
		}
	}
	
	contact2_1:
		//BAR(2,0,1);
		
		depth -= B[0];
		if(depth < 0)
			return ret;
			
		if(A[0] > 0)
		{
			p3[0] = p1[0] + box->side[0] * R[0][0];
			p3[1] = p1[1] + box->side[0] * R[1][0];
			p3[2] = p1[2] + box->side[0] * R[2][0];
		}
		else
		{
			p3[0] = p1[0] - box->side[0] * R[0][0];
			p3[1] = p1[1] - box->side[0] * R[1][0];
			p3[2] = p1[2] - box->side[0] * R[2][0];
		}
			
		if(dAddContact(dContact(dContactGeom(p3, n, depth, o1, o2)), contacts));
			ret++;	
		return ret;
		
	contact2_2:
		//BAR(2,1,2);
		
		depth -= B[1];
		if(depth < 0)
			return ret;
			
		if(A[1] > 0)
		{
			p3[0] = p1[0] + box->side[1] * R[0][1];
			p3[1] = p1[1] + box->side[1] * R[1][1];
			p3[2] = p1[2] + box->side[1] * R[2][1];
		}
		else
		{
			p3[0] = p1[0] - box->side[1] * R[0][1];
			p3[1] = p1[1] - box->side[1] * R[1][1];
			p3[2] = p1[2] - box->side[1] * R[2][1];
		}
			
		if(dAddContact(dContact(dContactGeom(p3, n, depth, o1, o2)), contacts));
			ret++;
		return ret;
		
	contact2_3:
		//BAR(2,2,3);
		
		depth -= B[2];
		if(depth < 0)
			return ret;
			
		if(A[2] > 0)
		{
			p3[0] = p1[0] + box->side[2] * R[0][2];
			p3[1] = p1[1] + box->side[2] * R[1][2];
			p3[2] = p1[2] + box->side[2] * R[2][2];
		}
		else
		{
			p3[0] = p1[0] - box->side[2] * R[0][2];
			p3[1] = p1[1] - box->side[2] * R[1][2];
			p3[2] = p1[2] - box->side[2] * R[2][2];
		}
			
		if(dAddContact(dContact(dContactGeom(p3, n, depth, o1, o2)), contacts));
			ret++;
		return ret;
#else
	return 0;
#endif
}


// if mode==1 then use the sphere exit contact, not the entry contact
/*
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
*/

int	dCollideRaySphere(dxGeom *o1, dxGeom *o2, int flags, std::vector<dContact> &contacts)
{
	dIASSERT (o1->type == dRayClass);
	dIASSERT (o2->type == dSphereClass);
#if 0
  dxRay *ray = (dxRay*) o1;
  dxSphere *sphere = (dxSphere*) o2;
  contact->g1 = ray;
  contact->g2 = sphere;
  return ray_sphere_helper (ray,sphere->pos,sphere->radius,contact,0);
#else
	return 0;
#endif
}


int	dCollideRayBox(dxGeom *o1, dxGeom *o2, int flags, std::vector<dContact> &contacts)
{
	dIASSERT (o1->type == dRayClass);
	dIASSERT (o2->type == dBoxClass);
#if 0
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
  if (alpha < 0 || alpha > ray->length)
  	return 0;
  contact->pos[0] = ray->pos[0] + alpha * *ray->R[0][2];
  contact->pos[1] = ray->pos[1] + alpha * *ray->R[1][2];
  contact->pos[2] = ray->pos[2] + alpha * *ray->R[2][2];
  contact->normal[0] = *box->R[0][n] * sign[n];
  contact->normal[1] = *box->R[1][n] * sign[n];
  contact->normal[2] = *box->R[2][n] * sign[n];
  contact->depth = alpha;
  return 1;
#else
	return 0;
#endif
}


int	dCollideRayPlane(dxGeom *o1, dxGeom *o2, int flags, std::vector<dContact> &contacts)
{
	dIASSERT(o1->type == dRayClass);
	dIASSERT(o2->type == dPlaneClass);
#if 0
  dxRay *ray = (dxRay*) o1;
  dxPlane *plane = (dxPlane*) o2;

  vec_t alpha = plane->p[3] - dDOT (plane->p._normal, ray->pos);
  // note: if alpha > 0 the starting point is below the plane
  vec_t nsign = (alpha > 0) ? REAL(-1.0) : REAL(1.0);
  vec_t k = dDOT14(plane->p._normal, &((*ray->R)[0][2]));
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
#else
	return 0;
#endif
}

int	dPointInLeaf(dxBSP *bsp, const vec3_c &p, int nodenum)
{
	dIASSERT(bsp->type == dBSPClass);

	if(nodenum < 0 || nodenum >= (int)bsp->nodes.size())
	{
		Com_Error(ERR_DROP, "dPointInLeaf: bad num %i", nodenum);
	}
	
	const dxBSP::dBSPNode& node = bsp->nodes[nodenum];

	plane_side_e side = node.plane->onSide(p);
	
	nodenum = node.children[side];
		
	if(nodenum < 0)
		return -1 -nodenum;
			
	if(nodenum == 0)
		return -1;
	
	return dPointInLeaf(bsp, p, nodenum);
}

void	dBoxLeafnums(dxBSP *bsp, const aabb_c &aabb, std::deque<int> &leafs, int nodenum)
{
	dIASSERT(bsp->type == dBSPClass);
#if 1
	if(nodenum < 0)
	{
		int leafnum = -1 -nodenum;
		
		if(leafnum < 0 || leafnum >= (int)bsp->leafs.size())
		{
			Com_Error(ERR_DROP, "dBoxLeafnums: bad leafnum %i", leafnum);
		}
		
		//std::vector<int>::iterator ir = find(areas.begin(), areas.end(), areanum);
		//if(ir == areas.end())
		//	areas.push_back(areanum);
		
		//Com_Printf("%i ", leafnum);
		
		leafs.push_back(leafnum);
		return;
	}
	
	const dxBSP::dBSPNode& node = bsp->nodes[nodenum];
		
	plane_side_e s = node.plane->onSide(aabb);
	
	switch(s)
	{
		case SIDE_FRONT:
		{
			dBoxLeafnums(bsp, aabb, leafs, node.children[0]);
			break;
		}
		
		case SIDE_BACK:
		{
			dBoxLeafnums(bsp, aabb, leafs, node.children[1]);
			break;
		}
		
		case SIDE_CROSS:
		{
			// go down both
			dBoxLeafnums(bsp, aabb, leafs, node.children[0]);
			dBoxLeafnums(bsp, aabb, leafs, node.children[1]);
			break;
		}
		
		default:
			break;
	}
#else
	while(nodenum >= 0)
	{
		const dxBSP::dBSPNode& node = bsp->nodes[nodenum];
		
		plane_side_e s = node.plane->onSide(aabb);
		
		if(s == SIDE_FRONT)
			nodenum = node.children[0];
		
		else if(s == SIDE_BACK)
			nodenum = node.children[1];
		
		else
		{	// go down both
			dBoxLeafnums(bsp, aabb, leafs, node.children[0]);
			
			nodenum = node.children[1];
		}
	}
	
//	Com_Printf("%i ", -1 -nodenum);
	leafs.push(-1 -nodenum);
#endif
}

static bool	_cldTestSeparatingAxes(const vec3_c vertexes[3])
{
	//TODO
	
	/*
	// reset best axis
	int axis_best = 0;
	int axis_exit = -1;
	vec_t best_depth = MAXVALUE;
	
	// calculate edges
	const vec3_c edge0 = vertexes[1] - vertexes[0];
	const vec3_c edge1 = vertexes[2] - vertexes[0];
	
	SUBTRACT(v1,v0,vE0);
	SUBTRACT(v2,v0,vE1);
	SUBTRACT(vE1,vE0,vE2);
	
	// calculate poly normal
	dCROSS(vN,=,vE0,vE1);
	
	// extract box axes as vectors
	vec3_c vA0,vA1,vA2;
	GETCOL(mHullBoxRot, 0, vA0);
	GETCOL(mHullBoxRot, 1, vA1);
	GETCOL(mHullBoxRot, 2, vA2);
	
	// box halfsizes
	dReal fa0 = vBoxHalfSize[0];
	dReal fa1 = vBoxHalfSize[1];
	dReal fa2 = vBoxHalfSize[2];

  // calculate relative position between box and triangle
  dVector3 vD;
  SUBTRACT(v0,vHullBoxPos,vD);

  // calculate length of face normal
  dReal fNLen = LENGTHOF( vN );

  dVector3 vL;
  dReal fp0, fp1, fp2, fR, fD;

  // Test separating axes for intersection
  // ************************************************
  // Axis 1 - Triangle Normal 
  SET(vL,vN);
  fp0  = dDOT(vL,vD);
  fp1  = fp0;
  fp2  = fp0;
  fR=fa0*dFabs( dDOT(vN,vA0) ) + fa1 * dFabs( dDOT(vN,vA1) ) + fa2 * dFabs( dDOT(vN,vA2) );


  if( !_cldTestNormal( fp0, fR, vL, 1) ) { 
    iExitAxis=1;
    return FALSE; 
  } 
 
  // ************************************************

  // Test Faces
  // ************************************************
  // Axis 2 - Box X-Axis
  SET(vL,vA0);
  fD  = dDOT(vL,vN)/fNLen;
  fp0 = dDOT(vL,vD);
  fp1 = fp0 + dDOT(vA0,vE0);
  fp2 = fp0 + dDOT(vA0,vE1);
  fR  = fa0;


  if( !_cldTestFace( fp0, fp1, fp2, fR, fD, vL, 2) ) { 
    iExitAxis=2;
    return FALSE; 
  }
  // ************************************************

  // ************************************************
  // Axis 3 - Box Y-Axis
  SET(vL,vA1);
  fD = dDOT(vL,vN)/fNLen;
  fp0 = dDOT(vL,vD);
  fp1 = fp0 + dDOT(vA1,vE0);
  fp2 = fp0 + dDOT(vA1,vE1);
  fR  = fa1;


  if( !_cldTestFace( fp0, fp1, fp2, fR, fD, vL, 3) ) { 
    iExitAxis=3;
    return FALSE; 
  }

  // ************************************************

  // ************************************************
  // Axis 4 - Box Z-Axis
  SET(vL,vA2);
  fD = dDOT(vL,vN)/fNLen;
  fp0 = dDOT(vL,vD);
  fp1 = fp0 + dDOT(vA2,vE0);
  fp2 = fp0 + dDOT(vA2,vE1);
  fR  = fa2;


  if( !_cldTestFace( fp0, fp1, fp2, fR, fD, vL, 4) ) { 
    iExitAxis=4;
    return FALSE; 
  }

  // ************************************************

  // Test Edges
  // ************************************************
  // Axis 5 - Box X-Axis cross Edge0
  dCROSS(vL,=,vA0,vE0);
  fD  = dDOT(vL,vN)/fNLen;
  fp0 = dDOT(vL,vD);
  fp1 = fp0;
  fp2 = fp0 + dDOT(vA0,vN);
  fR  = fa1 * dFabs(dDOT(vA2,vE0)) + fa2 * dFabs(dDOT(vA1,vE0));


  if( !_cldTestEdge( fp1, fp2, fR, fD, vL, 5) ) { 
    iExitAxis=5;
    return FALSE; 
  }
  // ************************************************

  // ************************************************
  // Axis 6 - Box X-Axis cross Edge1
  dCROSS(vL,=,vA0,vE1);
  fD  = dDOT(vL,vN)/fNLen;
  fp0 = dDOT(vL,vD);
  fp1 = fp0 - dDOT(vA0,vN);
  fp2 = fp0;
  fR  = fa1 * dFabs(dDOT(vA2,vE1)) + fa2 * dFabs(dDOT(vA1,vE1));


  if( !_cldTestEdge( fp0, fp1, fR, fD, vL, 6) ) { 
    iExitAxis=6;
    return FALSE; 
  }
  // ************************************************

  // ************************************************
  // Axis 7 - Box X-Axis cross Edge2
  dCROSS(vL,=,vA0,vE2);
  fD  = dDOT(vL,vN)/fNLen;
  fp0 = dDOT(vL,vD);
  fp1 = fp0 - dDOT(vA0,vN);
  fp2 = fp0 - dDOT(vA0,vN);
  fR  = fa1 * dFabs(dDOT(vA2,vE2)) + fa2 * dFabs(dDOT(vA1,vE2));


  if( !_cldTestEdge( fp0, fp1, fR, fD, vL, 7) ) { 
    iExitAxis=7;
    return FALSE; 
  }

  // ************************************************

  // ************************************************
  // Axis 8 - Box Y-Axis cross Edge0
  dCROSS(vL,=,vA1,vE0);
  fD  = dDOT(vL,vN)/fNLen;
  fp0 = dDOT(vL,vD);
  fp1 = fp0;
  fp2 = fp0 + dDOT(vA1,vN);
  fR  = fa0 * dFabs(dDOT(vA2,vE0)) + fa2 * dFabs(dDOT(vA0,vE0));


  if( !_cldTestEdge( fp0, fp2, fR, fD, vL, 8) ) { 
    iExitAxis=8;
    return FALSE; 
  }

  // ************************************************

  // ************************************************
  // Axis 9 - Box Y-Axis cross Edge1
  dCROSS(vL,=,vA1,vE1);
  fD  = dDOT(vL,vN)/fNLen;
  fp0 = dDOT(vL,vD);
  fp1 = fp0 - dDOT(vA1,vN);
  fp2 = fp0;
  fR  = fa0 * dFabs(dDOT(vA2,vE1)) + fa2 * dFabs(dDOT(vA0,vE1));


  if( !_cldTestEdge( fp0, fp1, fR, fD, vL, 9) ) { 
    iExitAxis=9;
    return FALSE; 
  }

  // ************************************************

  // ************************************************
  // Axis 10 - Box Y-Axis cross Edge2
  dCROSS(vL,=,vA1,vE2);
  fD  = dDOT(vL,vN)/fNLen;
  fp0 = dDOT(vL,vD);
  fp1 = fp0 - dDOT(vA1,vN);
  fp2 = fp0 - dDOT(vA1,vN);
  fR  = fa0 * dFabs(dDOT(vA2,vE2)) + fa2 * dFabs(dDOT(vA0,vE2));


  if( !_cldTestEdge( fp0, fp1, fR, fD, vL, 10) ) { 
    iExitAxis=10;
    return FALSE; 
  }

  // ************************************************

  // ************************************************
  // Axis 11 - Box Z-Axis cross Edge0
  dCROSS(vL,=,vA2,vE0);
  fD  = dDOT(vL,vN)/fNLen;
  fp0 = dDOT(vL,vD);
  fp1 = fp0;
  fp2 = fp0 + dDOT(vA2,vN);
  fR  = fa0 * dFabs(dDOT(vA1,vE0)) + fa1 * dFabs(dDOT(vA0,vE0));


  if( !_cldTestEdge( fp0, fp2, fR, fD, vL, 11) ) { 
    iExitAxis=11;
    return FALSE; 
  }
  // ************************************************

  // ************************************************
  // Axis 12 - Box Z-Axis cross Edge1
  dCROSS(vL,=,vA2,vE1);
  fD  = dDOT(vL,vN)/fNLen;
  fp0 = dDOT(vL,vD);
  fp1 = fp0 - dDOT(vA2,vN);
  fp2 = fp0;
  fR  = fa0 * dFabs(dDOT(vA1,vE1)) + fa1 * dFabs(dDOT(vA0,vE1));


  if( !_cldTestEdge( fp0, fp1, fR, fD, vL, 12) ) { 
    iExitAxis=12;
    return FALSE; 
  }
  // ************************************************

  // ************************************************
  // Axis 13 - Box Z-Axis cross Edge2
  dCROSS(vL,=,vA2,vE2);
  fD  = dDOT(vL,vN)/fNLen;
  fp0 = dDOT(vL,vD);
  fp1 = fp0 - dDOT(vA2,vN);
  fp2 = fp0 - dDOT(vA2,vN);
  fR  = fa0 * dFabs(dDOT(vA1,vE2)) + fa1 * dFabs(dDOT(vA0,vE2));


  if( !_cldTestEdge( fp0, fp1, fR, fD, vL, 13) ) { 
    iExitAxis=13;
    return FALSE; 
  }
 
  // ************************************************
  return TRUE; 
  
  */
  return false;
}

static int	dCollideBSPTriangleBox(dxGeom *o1, dxGeom *o2, int flags, std::vector<dContact> &contacts, const vec3_c vertexes[3], const cplane_c &p)
{
	dIASSERT(o1->type == dBSPClass);
	dIASSERT(o2->type == dBoxClass);

	// do intersection test and find best separating axis
	if(!_cldTestSeparatingAxes(vertexes))
	{
		// if not found do nothing
		return 0;
	}
	
	//TODO
	
	// if best separation axis is not found
	/*
	if(iBestAxis == 0)
	{
		// this should not happen (we should already exit in that case)
		//dMessage (0, "best separation axis not found");
		// do nothing
		return;
	}
	*/
	
	
	//_cldClipping(v0, v1, v2);
	
	//TODO
	return 0;
}


static bool	dCollideBSPTriangleSphere(dxGeom *o1, dxGeom *o2, int flags, std::vector<dContact> &contacts, const vec3_c vertexes[3], const cplane_c &p, bool planar, bool cw)
{
	dIASSERT(o1->type == dBSPClass);
	dIASSERT(o2->type == dSphereClass);
	
	dxSphere *sphere = (dxSphere*)o2;
	
	// check if sphere center is behind the triangle plane
#if 0
	if(planar)
	{
		if(p.distance(sphere->pos) <= -DIST_EPSILON)
			return false;
	}
	else
	{
		if(p.onSide(sphere->pos) == SIDE_BACK)
			return false;
	}
#endif

#if 0
	// get closest point to sphere origin
	//vec3_c C = p.closest(sphere->pos);

	/*
	vec3_c c(false), n(false);
	vec_t d = p.distance(sphere->pos);
	
	if(d > -DIST_EPSILON)
		c = vec3_c(sphere->pos) - (p._normal * d);
		
	else if(d < DIST_EPSILON)
		C = vec3_c(sphere->pos) + (p._normal * d);
	
	else
		return false;
	*/
	
	vec_t d = p.distance(sphere->pos);
	
	/*
	if(d < REAL(0.0))
	{
		return false;
	}
	else
	*/
	{
		vec3_c c = vec3_c(sphere->pos) - (p._normal * d);
	
		if(dPointInTriangle(c, vertexes[0], vertexes[1], vertexes[2], p._normal, cw))
		{
			// calculate penetration depth
			vec_t depth = sphere->radius - c.distance(sphere->pos);
		
			if(depth >= REAL(0.0))
			{
				if(dAddContact(dContact(dContactGeom(c, p._normal, depth, o1, o2)), contacts))
					return true;
			}
		}
	}
	
#elif 1
	const vec3_c edge0 = vertexes[1] - vertexes[0];
	const vec3_c edge1 = vertexes[2] - vertexes[0];

	// calculate distant from sphere origin to triangle and barycentric coords
	vec_t dist, u, v;
	dGetContactData(sphere->pos, vertexes[0], edge0, edge1, dist, u, v);
	
	// calculate the position of a vertex using its barycentric coords
	vec3_c C  = vertexes[0] + (edge0 * u) + (edge1 * v);
//	vec_t w = REAL(1.0) -u -v;
//	vec3_c C = (vertexes[0] * w) + (vertexes[1] * u) + (vertexes[2] * v);

	// calculate interpolated normal
//	vec3_c N1 = vertexes[0] + (edge0 * u) + (edge1 * v);
//	vec3_c N  = N1 + N2;
//	N.normalize();
	
	// calculate penetration depth
//	vec_t depth = sphere->radius - dist;
	vec_t depth = sphere->radius - C.distance(sphere->pos);
	
	if(depth >= REAL(0.0))
	{
		if(dAddContact(dContact(dContactGeom(C, p._normal, depth, o1, o2)), contacts))
			return true;
	}
#else

	// check if the sphere is on the side of an edge
	for(int i=0; i<3; i++)
	{
		// current vertex
		const vec3_c &Av = vertexes[i];
		
		const vec3_c &Bv = cw ?	vertexes[(i+1)%3] :	// next to A in clockwise order
					vertexes[(i+2)%3];	// next to A in counter clockwise order
			
		// current edge
		vec3_c E = Bv - Av;
		vec_t El = E.normalize();
		
		// create edge normal
		vec3_c En(false);
		En.crossProduct(p._normal, E);
		En.normalize();
		//En.negate();
		
		// create edge plane
		cplane_c Ep(En, En.dotProduct(Av));
		
		// distance between sphere and edge plane
		vec_t d = Ep.distance(sphere->pos, sphere->radius);
		
		// check if sphere is completely behind the edge plane
		if(d <= ((-sphere->radius*2)-DIST_EPSILON))
			continue;
			
		// check if sphere is too far away from edge plane
		if(d > REAL(0.0))	//DIST_EPSILON)//REAL(0.0))
			return false;
			
		// direction of sphere center from start point of edge
		vec3_c T  = vec3_c(sphere->pos) - Av;
				
		// distance of sphere center along the edge
		vec_t t = T.dotProduct(E);
			
		if(t <= REAL(0.0))
		{
			// sphere close to start point
			vec_t depth = sphere->radius - Av.distance(sphere->pos);
			
			if(depth >= REAL(0.0))
			{
				if(dAddContact(dContact(dContactGeom(Av, p._normal, depth, o1, o2)), contacts))
					return true;
			}
		}
		else if(t >= El)
		{
			// sphere close to end point of edge
			vec_t depth = sphere->radius - Bv.distance(sphere->pos);
			
			if(depth >= REAL(0.0))
			{
				if(dAddContact(dContact(dContactGeom(Bv, p._normal, depth, o1, o2)), contacts))
					return true;
			}
		}
		else
		{
			// spehre / edge collision (the sphere pos is 'inside' the edge length)
			
			// point of collision on the triangle edge 
			vec3_c C = Av + (E * t);
			
			vec_t depth = sphere->radius - C.distance(sphere->pos);
			
			if(depth >= REAL(0.0))
			{
				if(dAddContact(dContact(dContactGeom(C, p._normal, depth, o1, o2)), contacts))
					return true;
			}
		}
	}

	// the sphere pos is 'inside' the triangle edge planes, so do a sphere / plane collision
	
	// point of collision on the triangle plane 
	vec3_c C = p.closest(sphere->pos);
	
	//if(dPointInTriangle(C, vertexes[0], vertexes[1], vertexes[2], p._normal, cw))
	{
		vec_t depth = sphere->radius - C.distance(sphere->pos);
		
		if(depth >= REAL(0.0))
		{
			if(dAddContact(dContact(dContactGeom(C, p._normal, depth, o1, o2)), contacts))
				return true;
		}
	}
#endif
	return false;
}

static int	dCollideBSPSurfaceGeom(dxGeom *o1, dxGeom *o2, int flags, std::vector<dContact> &contacts, const dxBSP::dBSPSurface &surf)
{
	dIASSERT(o1->type == dBSPClass);
	dIASSERT(o2->type == dBoxClass || o2->type == dSphereClass);
	
	if(surf.indexes.empty())
		return 0;
		
	#if 1
	dxBSP *bsp = (dxBSP*)o1;
	const dxBSP::dBSPShader& shader = bsp->shaders[surf.shader_num];
	if(!(o2->collide_bits & shader.contents))
		return 0;
	#endif
		
	int contacts_num = 0;
	
	switch(surf.face_type)
	{
		case BSPST_PLANAR:
		{
			// for each triangle in the surface
			for(uint_t i=0; i<surf.indexes.size(); i += 3)
			{
				#if DEBUG
				try
				{
				#endif
				
				vec3_c vertexes[3] = 
				{
					surf.vertexes.at(surf.indexes[i+0]),
					surf.vertexes.at(surf.indexes[i+1]),
					surf.vertexes.at(surf.indexes[i+2])
				};
				
				if(o2->type == dBoxClass)
				{
					contacts_num += dCollideBSPTriangleBox(o1, o2, flags, contacts, vertexes, surf.plane);
				}
				else if(o2->type == dSphereClass)
				{	
					if(dCollideBSPTriangleSphere(o1, o2, flags, contacts, vertexes, surf.plane, false, true))
					{
						contacts_num++;
						break;
					}
				}
				
				#if DEBUG
				}
				catch(...)
				{
					Com_Error(ERR_DROP, "dCollideBSPSurfaceSphere: exception occured");
				}
				#endif
			}
			break;
		}
		
		case BSPST_BEZIER:
		case BSPST_MESH:
		{
			// for each triangle in the surface
			for(uint_t i=0, j=0; i<surf.indexes.size(); i += 3, j++)
			{
				#if DEBUG
				try
				{
				#endif
				
				vec3_c vertexes[3] = 
				{
					surf.vertexes.at(surf.indexes[i+0]),
					surf.vertexes.at(surf.indexes[i+1]),
					surf.vertexes.at(surf.indexes[i+2])
				};
				
				if(o2->type == dBoxClass)
				{
					contacts_num += dCollideBSPTriangleBox(o1, o2, flags, contacts, vertexes, surf.plane);
				}
				else if(o2->type == dSphereClass)
				{	
					if(dCollideBSPTriangleSphere(o1, o2, flags, contacts, vertexes, surf.plane, false, true))
						contacts_num++;
				}
				
				#if DEBUG
				}
				catch(...)
				{
					Com_Error(ERR_DROP, "dCollideBSPSurfaceSphere: exception occured");
				}
				#endif
			}
			break;
		}
		
		default:
			break;
	}
	
	return contacts_num;
}

static int	dCollideBSPLeafGeom(dxGeom *o1, dxGeom *o2, int flags, std::vector<dContact> &contacts, int leafnum)
{
	dIASSERT(o1->type == dBSPClass);
	dIASSERT(o2->type == dBoxClass || o2->type == dSphereClass);
	
	dxBSP *bsp = (dxBSP*)o1;
	
	const dxBSP::dBSPLeaf& leaf = bsp->leafs[leafnum];
	
	/*
	if(leaf.area < 0)
	{
		Com_Printf("dSphereInLeaf: leaf %i in bad area %i\n", leafnum, leaf.area);
		return 0;
	}
	*/
	
	//if(leaf.cluster == -1)
	//	return 0;
	
	//if(!(leaf.contents & trace_contents))
	//	return;

	int contacts_num = 0;
	for(int i=0; i<leaf.surfaces_num; i++)
	{
		dxBSP::dBSPSurface& surf = bsp->surfaces[bsp->leafsurfaces[leaf.surfaces_first + i]];
		
		if(surf.checkcount == bsp->checkcount)
			continue;	// already checked this surface in another leaf
			
		surf.checkcount = bsp->checkcount;
		
		//if(!(surf.contents & o2->trace_contents))
		//	continue;
		
		contacts_num += dCollideBSPSurfaceGeom(o1, o2, flags, contacts, surf);
	}
	
	return contacts_num;
}

static int	dCollideBSPGeom(dxGeom *o1, dxGeom *o2, int flags, std::vector<dContact> &contacts)
{
	dIASSERT(o1->type == dBSPClass);
	dIASSERT(o2->type == dBoxClass || o2->type == dSphereClass);
	
	dxBSP *bsp = (dxBSP*)o1;
	
	aabb_c aabb;
	aabb._mins[0] = o2->aabb[0] - 1.0f;
	aabb._maxs[0] = o2->aabb[1] + 1.0f;
	aabb._mins[1] = o2->aabb[2] - 1.0f;
	aabb._maxs[1] = o2->aabb[3] + 1.0f;
	aabb._mins[2] = o2->aabb[4] - 1.0f;
	aabb._maxs[2] = o2->aabb[5] + 1.0f;
	
	bsp->checkcount++;
	
	int contacts_num = 0;

#if 1
	//
	// test in all leafs the aabb touches
	//
	std::deque<int> leafs;
	dBoxLeafnums(bsp, aabb, leafs, 0);
	if(!leafs.size())
	{
		Com_Error(ERR_DROP, "dCollideBSPGeom: no BSP leaves touching");
		return 0;
	}
	else
	{
//		Com_Printf("dCollideBSPGeom: %i leaves\n", leafs.size());
	}

	for(std::deque<int>::const_iterator ir = leafs.begin(); ir != leafs.end(); ++ir)
	{
		contacts_num += dCollideBSPLeafGeom(o1, o2, flags, contacts, *ir);
	}
#else
	//
	// test only in single leaf
	//

	int leaf_num = dPointInLeaf(bsp, o2->pos, 0);
	
//	Com_Printf("dCollideBSPGeom: sphere in leaf %i, area %i, cluster %i\n", leaf_num, bsp->leafs[leaf_num].area, bsp->leafs[leaf_num].cluster);
	
	if(leaf_num != -1)
		contacts_num += dCollideBSPLeafGeom(o1, o2, flags, contacts, leaf_num);
#endif
	if(contacts_num)
	{
//		Com_Printf("dCollideBSPGeom: %i contacts\n", contacts_num);
	}
	
	return contacts_num;
}

int	dCollideBSPBox(dxGeom *o1, dxGeom *o2, int flags, std::vector<dContact> &contacts)
{
	dIASSERT(o1->type == dBSPClass);
	dIASSERT(o2->type == dBoxClass);
	
	return dCollideBSPGeom(o1, o2, flags, contacts);
}

int	dCollideBSPSphere(dxGeom *o1, dxGeom *o2, int flags, std::vector<dContact> &contacts)
{
	dIASSERT(o1->type == dBSPClass);
	dIASSERT(o2->type == dSphereClass);
	
	return dCollideBSPGeom(o1, o2, flags, contacts);
	
}

int	dCollideBSPRay(dxGeom *o1, dxGeom *o2, int flags, std::vector<dContact> &contacts)
{
	dIASSERT(o1->type == dBSPClass);
	dIASSERT(o2->type == dRayClass);
	dDEBUGMSG("");
	
	//TODO
	return 0;
}

