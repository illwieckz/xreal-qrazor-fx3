/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006 Robert Beckebans <trebor_7@users.sourceforge.net>

This file is part of XreaL source code.

XreaL source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

XreaL source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XreaL source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "cm_local.h"


/*
=================
CM_SignbitsForNormal
=================
*/
static int CM_SignbitsForNormal(vec3_t normal)
{
	int             bits, j;

	bits = 0;
	for(j = 0; j < 3; j++)
	{
		if(normal[j] < 0)
		{
			bits |= 1 << j;
		}
	}
	return bits;
}

/*
=====================
CM_PlaneFromPoints

Returns false if the triangle is degenrate.
The normal will point out of the clock for clockwise ordered points
=====================
*/
static qboolean CM_PlaneFromPoints(vec4_t plane, vec3_t a, vec3_t b, vec3_t c)
{
	vec3_t          d1, d2;

	VectorSubtract(b, a, d1);
	VectorSubtract(c, a, d2);
	CrossProduct(d2, d1, plane);
	if(VectorNormalize(plane) == 0)
	{
		return qfalse;
	}

	plane[3] = DotProduct(a, plane);
	return qtrue;
}

/*
================================================================================

PATCH COLLIDE GENERATION

================================================================================
*/

static int      numPlanes;
static cPlane_t planes[SHADER_MAX_TRIANGLES];

static int      numFacets;
static cFacet_t facets[SHADER_MAX_TRIANGLES];

#define	NORMAL_EPSILON		0.0001
#define	DIST_EPSILON		0.02
#define	PLANE_TRI_EPSILON	0.1

/*
==================
CM_PlaneEqual
==================
*/
static int CM_PlaneEqual(cPlane_t * p, float plane[4], int *flipped)
{
	float           invplane[4];

	if(fabs(p->plane[0] - plane[0]) < NORMAL_EPSILON
	   && fabs(p->plane[1] - plane[1]) < NORMAL_EPSILON
	   && fabs(p->plane[2] - plane[2]) < NORMAL_EPSILON && fabs(p->plane[3] - plane[3]) < DIST_EPSILON)
	{
		*flipped = qfalse;
		return qtrue;
	}

	VectorNegate(plane, invplane);
	invplane[3] = -plane[3];

	if(fabs(p->plane[0] - invplane[0]) < NORMAL_EPSILON
	   && fabs(p->plane[1] - invplane[1]) < NORMAL_EPSILON
	   && fabs(p->plane[2] - invplane[2]) < NORMAL_EPSILON && fabs(p->plane[3] - invplane[3]) < DIST_EPSILON)
	{
		*flipped = qtrue;
		return qtrue;
	}

	return qfalse;
}

/*
==================
CM_SnapVector
==================
*/
static void CM_SnapVector(vec3_t normal)
{
	int             i;

	for(i = 0; i < 3; i++)
	{
		if(fabs(normal[i] - 1) < NORMAL_EPSILON)
		{
			VectorClear(normal);
			normal[i] = 1;
			break;
		}
		if(fabs(normal[i] - -1) < NORMAL_EPSILON)
		{
			VectorClear(normal);
			normal[i] = -1;
			break;
		}
	}
}

/*
==================
CM_FindPlane2
==================
*/
static int CM_FindPlane2(float plane[4], int *flipped)
{
	int             i;

	// see if the points are close enough to an existing plane
	for(i = 0; i < numPlanes; i++)
	{
		if(CM_PlaneEqual(&planes[i], plane, flipped))
			return i;
	}

	// add a new plane
	if(numPlanes == SHADER_MAX_TRIANGLES)
	{
		Com_Error(ERR_DROP, "CM_FindPlane2: SHADER_MAX_TRIANGLES");
	}

	VectorCopy4(plane, planes[numPlanes].plane);
	planes[numPlanes].signbits = CM_SignbitsForNormal(plane);

	numPlanes++;

	*flipped = qfalse;

	return numPlanes - 1;
}

/*
==================
CM_FindPlane
==================
*/
static int CM_FindPlane(float *p1, float *p2, float *p3)
{
	float           plane[4];
	int             i;
	float           d;

	if(!CM_PlaneFromPoints(plane, p1, p2, p3))
	{
		return -1;
	}

	// see if the points are close enough to an existing plane
	for(i = 0; i < numPlanes; i++)
	{
		if(DotProduct(plane, planes[i].plane) < 0)
		{
			continue;			// allow backwards planes?
		}

		d = DotProduct(p1, planes[i].plane) - planes[i].plane[3];
		if(d < -PLANE_TRI_EPSILON || d > PLANE_TRI_EPSILON)
		{
			continue;
		}

		d = DotProduct(p2, planes[i].plane) - planes[i].plane[3];
		if(d < -PLANE_TRI_EPSILON || d > PLANE_TRI_EPSILON)
		{
			continue;
		}

		d = DotProduct(p3, planes[i].plane) - planes[i].plane[3];
		if(d < -PLANE_TRI_EPSILON || d > PLANE_TRI_EPSILON)
		{
			continue;
		}

		// found it
		return i;
	}

	// add a new plane
	if(numPlanes == SHADER_MAX_TRIANGLES)
	{
		Com_Error(ERR_DROP, "CM_FindPlane: SHADER_MAX_TRIANGLES");
	}

	VectorCopy4(plane, planes[numPlanes].plane);
	planes[numPlanes].signbits = CM_SignbitsForNormal(plane);

	numPlanes++;

	return numPlanes - 1;
}

/*
==================
CM_PointOnPlaneSide
==================
*/
static int CM_PointOnPlaneSide(float *p, int planeNum)
{
	float          *plane;
	float           d;

	if(planeNum == -1)
	{
		return SIDE_ON;
	}
	plane = planes[planeNum].plane;

	d = DotProduct(p, plane) - plane[3];

	if(d > PLANE_TRI_EPSILON)
	{
		return SIDE_FRONT;
	}

	if(d < -PLANE_TRI_EPSILON)
	{
		return SIDE_BACK;
	}

	return SIDE_ON;
}

/*
==================
CM_TrianglePlane
==================
*/
static int CM_TrianglePlane(int trianglePlanes[SHADER_MAX_TRIANGLES], int tri)
{
	int             p;

	p = trianglePlanes[tri];
	if(p != -1)
	{
		//Com_Printf("CM_TrianglePlane %i %i\n", tri, p);
		return p;
	}

	// should never happen
	Com_Printf("WARNING: CM_TrianglePlane unresolvable\n");
	return -1;
}

/*
==================
CM_EdgePlaneNum
==================
*/
static int CM_EdgePlaneNum(cTriangleSoup_t * triSoup, int trianglePlanes[SHADER_MAX_TRIANGLES], int tri, int edgeType)
{
	float          *p1, *p2;
	vec3_t          up;
	int             p;

//  Com_Printf("CM_EdgePlaneNum: %i %i\n", tri, edgeType);

	switch (edgeType)
	{
		case 0:
			p1 = triSoup->points[tri][0];
			p2 = triSoup->points[tri][1];
			p = CM_TrianglePlane(trianglePlanes, tri);
			if(p == -1)
				break;
			VectorMA(p1, 4, planes[p].plane, up);
			return CM_FindPlane(p1, p2, up);

		case 1:
			p1 = triSoup->points[tri][1];
			p2 = triSoup->points[tri][2];
			p = CM_TrianglePlane(trianglePlanes, tri);
			if(p == -1)
				break;
			VectorMA(p1, 4, planes[p].plane, up);
			return CM_FindPlane(p1, p2, up);

		case 2:
			p1 = triSoup->points[tri][2];
			p2 = triSoup->points[tri][0];
			p = CM_TrianglePlane(trianglePlanes, tri);
			if(p == -1)
				break;
			VectorMA(p1, 4, planes[p].plane, up);
			return CM_FindPlane(p2, p1, up);

		default:
			Com_Error(ERR_DROP, "CM_EdgePlaneNum: bad edgeType=%i", edgeType);

	}

	return -1;
}

/*
===================
CM_SetBorderInward
===================
*/
static void CM_SetBorderInward(cFacet_t * facet, cTriangleSoup_t * triSoup, int trianglePlanes[SHADER_MAX_TRIANGLES],
							   int i, int which)
{
	int             k, l;
	float          *points[4];
	int             numPoints;

	switch (which)
	{
		case 0:
			points[0] = triSoup->points[i][0];
			points[1] = triSoup->points[i][1];
			points[2] = triSoup->points[i][2];
			numPoints = 3;
			break;
		case 1:
			points[0] = triSoup->points[i][2];
			points[1] = triSoup->points[i][1];
			points[2] = triSoup->points[i][0];
			numPoints = 3;
			break;
		default:
			Com_Error(ERR_FATAL, "CM_SetBorderInward: bad parameter %i", which);
			numPoints = 0;
			break;
	}

	for(k = 0; k < facet->numBorders; k++)
	{
		int             front, back;

		front = 0;
		back = 0;

		for(l = 0; l < numPoints; l++)
		{
			int             side;

			side = CM_PointOnPlaneSide(points[l], facet->borderPlanes[k]);
			if(side == SIDE_FRONT)
			{
				front++;
			}
			if(side == SIDE_BACK)
			{
				back++;
			}
		}

		if(front && !back)
		{
			facet->borderInward[k] = qtrue;
		}
		else if(back && !front)
		{
			facet->borderInward[k] = qfalse;
		}
		else if(!front && !back)
		{
			// flat side border
			facet->borderPlanes[k] = -1;
		}
		else
		{
			// bisecting side border
			Com_DPrintf("WARNING: CM_SetBorderInward: mixed plane sides\n");
			facet->borderInward[k] = qfalse;
		}
	}
}

/*
==================
CM_ValidateFacet

If the facet isn't bounded by its borders, we screwed up.
==================
*/
static qboolean CM_ValidateFacet(cFacet_t * facet)
{
	float           plane[4];
	int             j;
	winding_t      *w;
	vec3_t          bounds[2];

	if(facet->surfacePlane == -1)
	{
		return qfalse;
	}

	VectorCopy4(planes[facet->surfacePlane].plane, plane);
	w = BaseWindingForPlane(plane, plane[3]);
	for(j = 0; j < facet->numBorders && w; j++)
	{
		if(facet->borderPlanes[j] == -1)
		{
			FreeWinding(w);
			return qfalse;
		}
		VectorCopy4(planes[facet->borderPlanes[j]].plane, plane);
		if(!facet->borderInward[j])
		{
			VectorSubtract(vec3_origin, plane, plane);
			plane[3] = -plane[3];
		}
		ChopWindingInPlace(&w, plane, plane[3], 0.1f);
	}

	if(!w)
	{
		return qfalse;			// winding was completely chopped away
	}

	// see if the facet is unreasonably large
	WindingBounds(w, bounds[0], bounds[1]);
	FreeWinding(w);

	for(j = 0; j < 3; j++)
	{
		if(bounds[1][j] - bounds[0][j] > MAX_WORLD_COORD)
		{
			return qfalse;		// we must be missing a plane
		}
		if(bounds[0][j] >= MAX_WORLD_COORD)
		{
			return qfalse;
		}
		if(bounds[1][j] <= MIN_WORLD_COORD)
		{
			return qfalse;
		}
	}
	return qtrue;				// winding is fine
}

/*
==================
CM_AddFacetBevels
==================
*/
static void CM_AddFacetBevels(cFacet_t * facet)
{

	int             i, j, k, l;
	int             axis, dir, order, flipped;
	float           plane[4], d, newplane[4];
	winding_t      *w, *w2;
	vec3_t          mins, maxs, vec, vec2;

	VectorCopy4(planes[facet->surfacePlane].plane, plane);

	w = BaseWindingForPlane(plane, plane[3]);
	for(j = 0; j < facet->numBorders && w; j++)
	{
		if(facet->borderPlanes[j] == facet->surfacePlane)
			continue;
		VectorCopy4(planes[facet->borderPlanes[j]].plane, plane);

		if(!facet->borderInward[j])
		{
			VectorSubtract(vec3_origin, plane, plane);
			plane[3] = -plane[3];
		}

		ChopWindingInPlace(&w, plane, plane[3], 0.1f);
	}
	if(!w)
	{
		return;
	}

	WindingBounds(w, mins, maxs);

	// add the axial planes
	order = 0;
	for(axis = 0; axis < 3; axis++)
	{
		for(dir = -1; dir <= 1; dir += 2, order++)
		{
			VectorClear(plane);
			plane[axis] = dir;
			if(dir == 1)
			{
				plane[3] = maxs[axis];
			}
			else
			{
				plane[3] = -mins[axis];
			}
			//if it's the surface plane
			if(CM_PlaneEqual(&planes[facet->surfacePlane], plane, &flipped))
			{
				continue;
			}
			// see if the plane is allready present
			for(i = 0; i < facet->numBorders; i++)
			{
				if(CM_PlaneEqual(&planes[facet->borderPlanes[i]], plane, &flipped))
					break;
			}

			if(i == facet->numBorders)
			{
				if(facet->numBorders > MAX_FACET_BEVELS)
					Com_Printf("ERROR: too many bevels\n");
				facet->borderPlanes[facet->numBorders] = CM_FindPlane2(plane, &flipped);
				facet->borderNoAdjust[facet->numBorders] = 0;
				facet->borderInward[facet->numBorders] = flipped;
				facet->numBorders++;
			}
		}
	}
	//
	// add the edge bevels
	//
	// test the non-axial plane edges
	for(j = 0; j < w->numpoints; j++)
	{
		k = (j + 1) % w->numpoints;
		VectorSubtract(w->p[j], w->p[k], vec);
		//if it's a degenerate edge
		if(VectorNormalize(vec) < 0.5)
			continue;
		CM_SnapVector(vec);
		for(k = 0; k < 3; k++)
			if(vec[k] == -1 || vec[k] == 1)
				break;			// axial
		if(k < 3)
			continue;			// only test non-axial edges

		// try the six possible slanted axials from this edge
		for(axis = 0; axis < 3; axis++)
		{
			for(dir = -1; dir <= 1; dir += 2)
			{
				// construct a plane
				VectorClear(vec2);
				vec2[axis] = dir;
				CrossProduct(vec, vec2, plane);
				if(VectorNormalize(plane) < 0.5)
					continue;
				plane[3] = DotProduct(w->p[j], plane);

				// if all the points of the facet winding are
				// behind this plane, it is a proper edge bevel
				for(l = 0; l < w->numpoints; l++)
				{
					d = DotProduct(w->p[l], plane) - plane[3];
					if(d > 0.1)
						break;	// point in front
				}
				if(l < w->numpoints)
					continue;

				//if it's the surface plane
				if(CM_PlaneEqual(&planes[facet->surfacePlane], plane, &flipped))
				{
					continue;
				}
				// see if the plane is allready present
				for(i = 0; i < facet->numBorders; i++)
				{
					if(CM_PlaneEqual(&planes[facet->borderPlanes[i]], plane, &flipped))
					{
						break;
					}
				}

				if(i == facet->numBorders)
				{
					if(facet->numBorders > MAX_FACET_BEVELS)
						Com_Printf("ERROR: too many bevels\n");
					facet->borderPlanes[facet->numBorders] = CM_FindPlane2(plane, &flipped);

					for(k = 0; k < facet->numBorders; k++)
					{
						if(facet->borderPlanes[facet->numBorders] == facet->borderPlanes[k])
							Com_Printf("WARNING: bevel plane already used\n");
					}

					facet->borderNoAdjust[facet->numBorders] = 0;
					facet->borderInward[facet->numBorders] = flipped;
					//
					w2 = CopyWinding(w);
					VectorCopy4(planes[facet->borderPlanes[facet->numBorders]].plane, newplane);
					if(!facet->borderInward[facet->numBorders])
					{
						VectorNegate(newplane, newplane);
						newplane[3] = -newplane[3];
					}
					ChopWindingInPlace(&w2, newplane, newplane[3], 0.1f);
					if(!w2)
					{
						Com_DPrintf("WARNING: CM_AddFacetBevels... invalid bevel\n");
						continue;
					}
					else
					{
						FreeWinding(w2);
					}
					//
					facet->numBorders++;
					//already got a bevel
//                  break;
				}
			}
		}
	}
	FreeWinding(w);

#ifndef BSPC
	//add opposite plane
	facet->borderPlanes[facet->numBorders] = facet->surfacePlane;
	facet->borderNoAdjust[facet->numBorders] = 0;
	facet->borderInward[facet->numBorders] = qtrue;
	facet->numBorders++;
#endif							//BSPC

}

typedef enum
{
	EN_FIRST,
	EN_SECOND,
	EN_THIRD
} edgeName_t;

/*
==================
CM_SurfaceCollideFromTriangleSoup
==================
*/
static void CM_SurfaceCollideFromTriangleSoup(cTriangleSoup_t * triSoup, cSurfaceCollide_t * sc)
{
	int             i;
	float          *p1, *p2, *p3;
	static int      trianglePlanes[SHADER_MAX_TRIANGLES];
	cFacet_t       *facet;
	int             borders[4];
	int             noAdjust[4];

	numPlanes = 0;
	numFacets = 0;

	// find the planes for each triangle of the grid
	for(i = 0; i < triSoup->numTriangles; i++)
	{
		p1 = triSoup->points[i][0];
		p2 = triSoup->points[i][1];
		p3 = triSoup->points[i][2];
		trianglePlanes[i] = CM_FindPlane(p1, p2, p3);

		//Com_Printf("trianglePlane[%i] = %i\n", i, trianglePlanes[i]);
	}

	// create the borders for each triangle
	for(i = 0; i < triSoup->numTriangles; i++)
	{
		borders[EN_FIRST] = -1;
		noAdjust[EN_FIRST] = (borders[EN_FIRST] == trianglePlanes[i]);
		if(borders[EN_FIRST] == -1 || noAdjust[EN_FIRST])
		{
			borders[EN_FIRST] = CM_EdgePlaneNum(triSoup, trianglePlanes, i, 0);
		}

		borders[EN_SECOND] = -1;
		noAdjust[EN_SECOND] = (borders[EN_SECOND] == trianglePlanes[i]);
		if(borders[EN_SECOND] == -1 || noAdjust[EN_SECOND])
		{
			borders[EN_SECOND] = CM_EdgePlaneNum(triSoup, trianglePlanes, i, 1);
		}

		borders[EN_THIRD] = -1;
		noAdjust[EN_THIRD] = (borders[EN_THIRD] == trianglePlanes[i]);
		if(borders[EN_THIRD] == -1 || noAdjust[EN_THIRD])
		{
			borders[EN_THIRD] = CM_EdgePlaneNum(triSoup, trianglePlanes, i, 2);
		}

		if(numFacets == SHADER_MAX_TRIANGLES)
		{
			Com_Error(ERR_DROP, "CM_TriangleSoupCollideFromTriangleSoup: SHADER_MAX_TRIANGLES");
		}

		facet = &facets[numFacets];
		Com_Memset(facet, 0, sizeof(*facet));


		// two seperate facets
		facet->surfacePlane = trianglePlanes[i];
		facet->numBorders = 3;
		facet->borderPlanes[0] = borders[EN_FIRST];
		facet->borderNoAdjust[0] = noAdjust[EN_FIRST];
		facet->borderPlanes[1] = borders[EN_SECOND];
		facet->borderNoAdjust[1] = noAdjust[EN_SECOND];
		facet->borderPlanes[2] = borders[EN_THIRD];
		facet->borderNoAdjust[2] = noAdjust[EN_THIRD];

		CM_SetBorderInward(facet, triSoup, trianglePlanes, i, 0);

		if(CM_ValidateFacet(facet))
		{
			CM_AddFacetBevels(facet);
			numFacets++;
		}
	}


	// copy the results out
	sc->numPlanes = numPlanes;
	sc->planes = Hunk_Alloc(numPlanes * sizeof(*sc->planes), h_high);
	Com_Memcpy(sc->planes, planes, numPlanes * sizeof(*sc->planes));

	sc->numFacets = numFacets;
	sc->facets = Hunk_Alloc(numFacets * sizeof(*sc->facets), h_high);
	Com_Memcpy(sc->facets, facets, numFacets * sizeof(*sc->facets));
}


/*
===================
CM_GenerateTriangleSoupCollide

Creates an internal structure that will be used to perform
collision detection with a triangle soup mesh.

Points is packed as concatenated rows.
===================
*/
cSurfaceCollide_t *CM_GenerateTriangleSoupCollide(int numVertexes, vec3_t * vertexes, int numIndexes, int *indexes)
{
	cSurfaceCollide_t *sc;
	static cTriangleSoup_t triSoup;
	int             i, j;

	if(numVertexes <= 2 || !vertexes || numIndexes <= 2 || !indexes)
	{
		Com_Error(ERR_DROP, "CM_GenerateTriangleSoupCollide: bad parameters: (%i, %p, %i, %p)", numVertexes, vertexes, numIndexes,
				  indexes);
	}

	if(numIndexes > SHADER_MAX_INDEXES)
	{
		Com_Error(ERR_DROP, "CM_GenerateTriangleSoupCollide: source is > SHADER_MAX_TRIANGLES");
	}

	// build a triangle soup
	triSoup.numTriangles = numIndexes / 3;
	for(i = 0; i < triSoup.numTriangles; i++)
	{
		for(j = 0; j < 3; j++)
		{
			VectorCopy(vertexes[indexes[i * 3 + j]], triSoup.points[i][j]);
		}
	}

	sc = Hunk_Alloc(sizeof(*sc), h_high);
	ClearBounds(sc->bounds[0], sc->bounds[1]);
	for(i = 0; i < triSoup.numTriangles; i++)
	{
		for(j = 0; j < 3; j++)
		{
			AddPointToBounds(triSoup.points[i][j], sc->bounds[0], sc->bounds[1]);
		}
	}

	// generate a bsp tree for the surface
	CM_SurfaceCollideFromTriangleSoup(&triSoup, sc);

	// expand by one unit for epsilon purposes
	sc->bounds[0][0] -= 1;
	sc->bounds[0][1] -= 1;
	sc->bounds[0][2] -= 1;

	sc->bounds[1][0] += 1;
	sc->bounds[1][1] += 1;
	sc->bounds[1][2] += 1;

	return sc;
}


