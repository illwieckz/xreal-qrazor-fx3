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
#include "cm_trisoup.h"


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
static trianglePlane_t planes[SHADER_MAX_TRIANGLES];

static int      numTriangles;
static cTriangle_t triangles[SHADER_MAX_TRIANGLES];

#define	NORMAL_EPSILON		0.0001
#define	DIST_EPSILON		0.02
#define	PLANE_TRI_EPSILON	0.1

/*
==================
CM_PlaneEqual
==================
*/
static int CM_PlaneEqual(trianglePlane_t * p, float plane[4], int *flipped)
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
	
//	Com_Printf("CM_EdgePlaneNum: %i %i\n", tri, edgeType);

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
static void CM_SetBorderInward(cTriangle_t * triangle, cTriangleSoup_t * triSoup, int trianglePlanes[SHADER_MAX_TRIANGLES],
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

	for(k = 0; k < triangle->numBorders; k++)
	{
		int             front, back;

		front = 0;
		back = 0;

		for(l = 0; l < numPoints; l++)
		{
			int             side;

			side = CM_PointOnPlaneSide(points[l], triangle->borderPlanes[k]);
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
			triangle->borderInward[k] = qtrue;
		}
		else if(back && !front)
		{
			triangle->borderInward[k] = qfalse;
		}
		else if(!front && !back)
		{
			// flat side border
			triangle->borderPlanes[k] = -1;
		}
		else
		{
			// bisecting side border
			Com_DPrintf("WARNING: CM_SetBorderInward: mixed plane sides\n");
			triangle->borderInward[k] = qfalse;
		}
	}
}

/*
==================
CM_ValidateFacet

If the facet isn't bounded by its borders, we screwed up.
==================
*/
static qboolean CM_ValidateTriangle(cTriangle_t * triangle)
{
	float           plane[4];
	int             j;
	winding_t      *w;
	vec3_t          bounds[2];

	if(triangle->surfacePlane == -1)
	{
		return qfalse;
	}

	VectorCopy4(planes[triangle->surfacePlane].plane, plane);
	w = BaseWindingForPlane(plane, plane[3]);
	for(j = 0; j < triangle->numBorders && w; j++)
	{
		if(triangle->borderPlanes[j] == -1)
		{
			FreeWinding(w);
			return qfalse;
		}
		VectorCopy4(planes[triangle->borderPlanes[j]].plane, plane);
		if(!triangle->borderInward[j])
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
		if(bounds[1][j] - bounds[0][j] > MAX_MAP_BOUNDS)
		{
			return qfalse;		// we must be missing a plane
		}
		if(bounds[0][j] >= MAX_MAP_BOUNDS)
		{
			return qfalse;
		}
		if(bounds[1][j] <= -MAX_MAP_BOUNDS)
		{
			return qfalse;
		}
	}
	return qtrue;				// winding is fine
}

/*
==================
CM_AddTriangleBevels
==================
*/
static void CM_AddTriangleBevels(cTriangle_t * triangle)
{

	int             i, j, k, l;
	int             axis, dir, order, flipped;
	float           plane[4], d, newplane[4];
	winding_t      *w, *w2;
	vec3_t          mins, maxs, vec, vec2;

	VectorCopy4(planes[triangle->surfacePlane].plane, plane);

	w = BaseWindingForPlane(plane, plane[3]);
	for(j = 0; j < triangle->numBorders && w; j++)
	{
		if(triangle->borderPlanes[j] == triangle->surfacePlane)
			continue;

		VectorCopy4(planes[triangle->borderPlanes[j]].plane, plane);

		if(!triangle->borderInward[j])
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
			if(CM_PlaneEqual(&planes[triangle->surfacePlane], plane, &flipped))
			{
				continue;
			}
			// see if the plane is allready present
			for(i = 0; i < triangle->numBorders; i++)
			{
				if(CM_PlaneEqual(&planes[triangle->borderPlanes[i]], plane, &flipped))
					break;
			}

			if(i == triangle->numBorders)
			{
				if(triangle->numBorders > MAX_TRIANGLE_BEVELS)
					Com_Printf("ERROR: CM_AddTriangleBevels: too many bevels\n");
				triangle->borderPlanes[triangle->numBorders] = CM_FindPlane2(plane, &flipped);
				triangle->borderNoAdjust[triangle->numBorders] = 0;
				triangle->borderInward[triangle->numBorders] = flipped;
				triangle->numBorders++;
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
				if(CM_PlaneEqual(&planes[triangle->surfacePlane], plane, &flipped))
				{
					continue;
				}
				// see if the plane is allready present
				for(i = 0; i < triangle->numBorders; i++)
				{
					if(CM_PlaneEqual(&planes[triangle->borderPlanes[i]], plane, &flipped))
					{
						break;
					}
				}

				if(i == triangle->numBorders)
				{
					if(triangle->numBorders > MAX_TRIANGLE_BEVELS)
						Com_Printf("ERROR: too many bevels\n");
					triangle->borderPlanes[triangle->numBorders] = CM_FindPlane2(plane, &flipped);

					for(k = 0; k < triangle->numBorders; k++)
					{
						if(triangle->borderPlanes[triangle->numBorders] == triangle->borderPlanes[k])
							Com_Printf("WARNING: bevel plane already used\n");
					}

					triangle->borderNoAdjust[triangle->numBorders] = 0;
					triangle->borderInward[triangle->numBorders] = flipped;
					//
					w2 = CopyWinding(w);
					VectorCopy4(planes[triangle->borderPlanes[triangle->numBorders]].plane, newplane);
					if(!triangle->borderInward[triangle->numBorders])
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
					triangle->numBorders++;
					//already got a bevel
//                  break;
				}
			}
		}
	}
	FreeWinding(w);

#ifndef BSPC
	//add opposite plane
	triangle->borderPlanes[triangle->numBorders] = triangle->surfacePlane;
	triangle->borderNoAdjust[triangle->numBorders] = 0;
	triangle->borderInward[triangle->numBorders] = qtrue;
	triangle->numBorders++;
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
CM_TriangleSoupCollideFromTriangleSoup
==================
*/
static void CM_TriangleSoupCollideFromTriangleSoup(cTriangleSoup_t * triSoup, triSoupCollide_t * tc)
{
	int             i;
	float          *p1, *p2, *p3;
	static int      trianglePlanes[SHADER_MAX_TRIANGLES];
	cTriangle_t    *triangle;
	int             borders[4];
	int             noAdjust[4];

	numPlanes = 0;
	numTriangles = 0;

	// find the planes for each triangle of the grid
	for(i = 0; i < triSoup->numTriangles; i++)
	{
		p1 = triSoup->points[i][0];
		p2 = triSoup->points[i][1];
		p3 = triSoup->points[i][2];
		trianglePlanes[i] = CM_FindPlane(p1, p2, p3);
		
		//Com_Printf("trianglePlane[%i] = %i\n", i, trianglePlanes[i]);
	}
	
//	Com_Printf("base triangle planes calculated\n");

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

		if(numTriangles == SHADER_MAX_TRIANGLES)
		{
			Com_Error(ERR_DROP, "CM_TriangleSoupCollideFromTriangleSoup: SHADER_MAX_TRIANGLES");
		}
		triangle = &triangles[numTriangles];
		Com_Memset(triangle, 0, sizeof(*triangle));


		// two seperate triangles
		triangle->surfacePlane = trianglePlanes[i];
		triangle->numBorders = 3;
		triangle->borderPlanes[0] = borders[EN_FIRST];
		triangle->borderNoAdjust[0] = noAdjust[EN_FIRST];
		triangle->borderPlanes[1] = borders[EN_SECOND];
		triangle->borderNoAdjust[1] = noAdjust[EN_SECOND];
		triangle->borderPlanes[2] = borders[EN_THIRD];
		triangle->borderNoAdjust[2] = noAdjust[EN_THIRD];
		
		CM_SetBorderInward(triangle, triSoup, trianglePlanes, i, 0);
		
		if(CM_ValidateTriangle(triangle))
		{
			CM_AddTriangleBevels(triangle);
			numTriangles++;
		}
	}


	// copy the results out
	tc->numPlanes = numPlanes;
	tc->planes = Hunk_Alloc(numPlanes * sizeof(*tc->planes), h_high);
	Com_Memcpy(tc->planes, planes, numPlanes * sizeof(*tc->planes));
	
	tc->numTriangles = numTriangles;
	tc->triangles = Hunk_Alloc(numTriangles * sizeof(*tc->triangles), h_high);
	Com_Memcpy(tc->triangles, triangles, numTriangles * sizeof(*tc->triangles));
}


/*
===================
CM_GenerateTriangleSoupCollide

Creates an internal structure that will be used to perform
collision detection with a triangle soup mesh.

Points is packed as concatenated rows.
===================
*/
struct triSoupCollide_s *CM_GenerateTriangleSoupCollide(int numVertexes, vec3_t * vertexes, int numIndexes, int * indexes)
{
	triSoupCollide_t *tc;
	static cTriangleSoup_t triSoup;
	int             i, j;

	if(numVertexes <= 2 || !vertexes || numIndexes <= 2 || !indexes)
	{
		Com_Error(ERR_DROP, "CM_GenerateTriangleSoupCollide: bad parameters: (%i, %p, %i, %p)", numVertexes, vertexes, numIndexes, indexes);
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

	tc = Hunk_Alloc(sizeof(*tc), h_high);
	ClearBounds(tc->bounds[0], tc->bounds[1]);
	for(i = 0; i < triSoup.numTriangles; i++)
	{
		for(j = 0; j < 3; j++)
		{
			AddPointToBounds(triSoup.points[i][j], tc->bounds[0], tc->bounds[1]);
		}
	}

	// generate a bsp tree for the surface
	CM_TriangleSoupCollideFromTriangleSoup(&triSoup, tc);

	// expand by one unit for epsilon purposes
	tc->bounds[0][0] -= 1;
	tc->bounds[0][1] -= 1;
	tc->bounds[0][2] -= 1;

	tc->bounds[1][0] += 1;
	tc->bounds[1][1] += 1;
	tc->bounds[1][2] += 1;

	return tc;
}

/*
================================================================================

TRACE TESTING

================================================================================
*/

/*
====================
CM_TracePointThroughTriangleSoupCollide

  special case for point traces because the triangle soup collide "brushes" have no volume
====================
*/
void CM_TracePointThroughTriangleSoupCollide(traceWork_t * tw, const struct triSoupCollide_s *tc)
{
	static qboolean frontFacing[SHADER_MAX_TRIANGLES];
	static float    intersection[SHADER_MAX_TRIANGLES];
	float           intersect;
	const trianglePlane_t *planes;
	const cTriangle_t *triangle;
	int             i, j, k;
	float           offset;
	float           d1, d2;

#ifndef BSPC
	if(!cm_playerCurveClip->integer || !tw->isPoint)
	{
		return;
	}
#endif

	// determine the trace's relationship to all planes
	for(i = 0, planes = tc->planes; i < tc->numPlanes; i++, planes++)
	{
		offset = DotProduct(tw->offsets[planes->signbits], planes->plane);
		d1 = DotProduct(tw->start, planes->plane) - planes->plane[3] + offset;
		d2 = DotProduct(tw->end, planes->plane) - planes->plane[3] + offset;
		if(d1 <= 0)
		{
			frontFacing[i] = qfalse;
		}
		else
		{
			frontFacing[i] = qtrue;
		}
		if(d1 == d2)
		{
			intersection[i] = 99999;
		}
		else
		{
			intersection[i] = d1 / (d1 - d2);
			if(intersection[i] <= 0)
			{
				intersection[i] = 99999;
			}
		}
	}


	// see if any of the surface planes are intersected
	for(i = 0, triangle = tc->triangles; i < tc->numTriangles; i++, triangle++)
	{
		if(!frontFacing[triangle->surfacePlane])
		{
			continue;
		}
		intersect = intersection[triangle->surfacePlane];
		if(intersect < 0)
		{
			continue;			// surface is behind the starting point
		}
		if(intersect > tw->trace.fraction)
		{
			continue;			// already hit something closer
		}
		for(j = 0; j < triangle->numBorders; j++)
		{
			k = triangle->borderPlanes[j];
			if(frontFacing[k] ^ triangle->borderInward[j])
			{
				if(intersection[k] > intersect)
				{
					break;
				}
			}
			else
			{
				if(intersection[k] < intersect)
				{
					break;
				}
			}
		}
		if(j == triangle->numBorders)
		{
			// we hit this triangle
			planes = &tc->planes[triangle->surfacePlane];

			// calculate intersection with a slight pushoff
			offset = DotProduct(tw->offsets[planes->signbits], planes->plane);
			d1 = DotProduct(tw->start, planes->plane) - planes->plane[3] + offset;
			d2 = DotProduct(tw->end, planes->plane) - planes->plane[3] + offset;
			tw->trace.fraction = (d1 - SURFACE_CLIP_EPSILON) / (d1 - d2);

			if(tw->trace.fraction < 0)
			{
				tw->trace.fraction = 0;
			}

			VectorCopy(planes->plane, tw->trace.plane.normal);
			tw->trace.plane.dist = planes->plane[3];
		}
	}
}

/*
====================
CM_CheckTrianglePlane
====================
*/
static int CM_CheckTrianglePlane(float *plane, vec3_t start, vec3_t end, float *enterFrac, float *leaveFrac, int *hit)
{
	float           d1, d2, f;

	*hit = qfalse;

	d1 = DotProduct(start, plane) - plane[3];
	d2 = DotProduct(end, plane) - plane[3];

	// if completely in front of face, no intersection with the entire triangle
	if(d1 > 0 && (d2 >= SURFACE_CLIP_EPSILON || d2 >= d1))
	{
		return qfalse;
	}

	// if it doesn't cross the plane, the plane isn't relevent
	if(d1 <= 0 && d2 <= 0)
	{
		return qtrue;
	}

	// crosses face
	if(d1 > d2)
	{							// enter
		f = (d1 - SURFACE_CLIP_EPSILON) / (d1 - d2);
		if(f < 0)
		{
			f = 0;
		}
		//always favor previous plane hits and thus also the surface plane hit
		if(f > *enterFrac)
		{
			*enterFrac = f;
			*hit = qtrue;
		}
	}
	else
	{							// leave
		f = (d1 + SURFACE_CLIP_EPSILON) / (d1 - d2);
		if(f > 1)
		{
			f = 1;
		}
		if(f < *leaveFrac)
		{
			*leaveFrac = f;
		}
	}
	return qtrue;
}

/*
====================
CM_TraceThroughTriangleSoupCollide
====================
*/
void CM_TraceThroughTriangleSoupCollide(traceWork_t * tw, const struct triSoupCollide_s *tc)
{
	int             i, j, hit, hitnum;
	float           offset, enterFrac, leaveFrac, t;
	trianglePlane_t *planes;
	cTriangle_t    *triangle;
	float           plane[4] = {0, 0, 0, 0}, bestplane[4] =	{0, 0, 0, 0};
	vec3_t          startp, endp;

#ifndef BSPC
	if(!cm_noExtraAABBs->integer && !BoundsIntersect(tw->bounds[0], tw->bounds[1], tc->bounds[0], tc->bounds[1]))
		return;
#endif

	if(tw->isPoint)
	{
		CM_TracePointThroughTriangleSoupCollide(tw, tc);
		return;
	}

	for(i = 0, triangle = tc->triangles; i < tc->numTriangles; i++, triangle++)
	{
		enterFrac = -1.0;
		leaveFrac = 1.0;
		hitnum = -1;
		//
		planes = &tc->planes[triangle->surfacePlane];
		VectorCopy(planes->plane, plane);
		plane[3] = planes->plane[3];
		
		if(tw->sphere.use)
		{
			// adjust the plane distance apropriately for radius
			plane[3] += tw->sphere.radius;

			// find the closest point on the capsule to the plane
			t = DotProduct(plane, tw->sphere.offset);
			if(t > 0.0f)
			{
				VectorSubtract(tw->start, tw->sphere.offset, startp);
				VectorSubtract(tw->end, tw->sphere.offset, endp);
			}
			else
			{
				VectorAdd(tw->start, tw->sphere.offset, startp);
				VectorAdd(tw->end, tw->sphere.offset, endp);
			}
		}
		else
		{
			offset = DotProduct(tw->offsets[planes->signbits], plane);
			plane[3] -= offset;
			VectorCopy(tw->start, startp);
			VectorCopy(tw->end, endp);
		}

		if(!CM_CheckTrianglePlane(plane, startp, endp, &enterFrac, &leaveFrac, &hit))
		{
			continue;
		}
		
		if(hit)
		{
			VectorCopy4(plane, bestplane);
		}

		for(j = 0; j < triangle->numBorders; j++)
		{
			planes = &tc->planes[triangle->borderPlanes[j]];
			if(triangle->borderInward[j])
			{
				VectorNegate(planes->plane, plane);
				plane[3] = -planes->plane[3];
			}
			else
			{
				VectorCopy(planes->plane, plane);
				plane[3] = planes->plane[3];
			}
			
			if(tw->sphere.use)
			{
				// adjust the plane distance apropriately for radius
				plane[3] += tw->sphere.radius;

				// find the closest point on the capsule to the plane
				t = DotProduct(plane, tw->sphere.offset);
				if(t > 0.0f)
				{
					VectorSubtract(tw->start, tw->sphere.offset, startp);
					VectorSubtract(tw->end, tw->sphere.offset, endp);
				}
				else
				{
					VectorAdd(tw->start, tw->sphere.offset, startp);
					VectorAdd(tw->end, tw->sphere.offset, endp);
				}
			}
			else
			{
				// NOTE: this works even though the plane might be flipped because the bbox is centered
				offset = DotProduct(tw->offsets[planes->signbits], plane);
				plane[3] += fabs(offset);
				VectorCopy(tw->start, startp);
				VectorCopy(tw->end, endp);
			}

			if(!CM_CheckTrianglePlane(plane, startp, endp, &enterFrac, &leaveFrac, &hit))
			{
				break;
			}
			
			if(hit)
			{
				hitnum = j;
				VectorCopy4(plane, bestplane);
			}
		}
		
		if(j < triangle->numBorders)
			continue;
		
		//never clip against the back side
		if(hitnum == triangle->numBorders - 1)
			continue;

		if(enterFrac < leaveFrac && enterFrac >= 0)
		{
			if(enterFrac < tw->trace.fraction)
			{
				if(enterFrac < 0)
				{
					enterFrac = 0;
				}

				tw->trace.fraction = enterFrac;
				VectorCopy(bestplane, tw->trace.plane.normal);
				tw->trace.plane.dist = bestplane[3];
			}
		}
	}
}


/*
=======================================================================

POSITION TEST

=======================================================================
*/

/*
====================
CM_PositionTestInTriangleSoupCollide
====================
*/
qboolean CM_PositionTestInTriangleSoupCollide(traceWork_t * tw, const struct triSoupCollide_s *tc)
{
	int             i, j;
	float           offset, t;
	trianglePlane_t *planes;
	cTriangle_t    *triangle;
	float           plane[4];
	vec3_t          startp;

	if(tw->isPoint)
	{
		return qfalse;
	}
	
	//
	for(i = 0, triangle = tc->triangles; i < tc->numTriangles; i++, triangle++)
	{
		planes = &tc->planes[triangle->surfacePlane];
		VectorCopy(planes->plane, plane);
		plane[3] = planes->plane[3];
		if(tw->sphere.use)
		{
			// adjust the plane distance apropriately for radius
			plane[3] += tw->sphere.radius;

			// find the closest point on the capsule to the plane
			t = DotProduct(plane, tw->sphere.offset);
			if(t > 0)
			{
				VectorSubtract(tw->start, tw->sphere.offset, startp);
			}
			else
			{
				VectorAdd(tw->start, tw->sphere.offset, startp);
			}
		}
		else
		{
			offset = DotProduct(tw->offsets[planes->signbits], plane);
			plane[3] -= offset;
			VectorCopy(tw->start, startp);
		}

		if(DotProduct(plane, startp) - plane[3] > 0.0f)
		{
			continue;
		}

		for(j = 0; j < triangle->numBorders; j++)
		{
			planes = &tc->planes[triangle->borderPlanes[j]];
			if(triangle->borderInward[j])
			{
				VectorNegate(planes->plane, plane);
				plane[3] = -planes->plane[3];
			}
			else
			{
				VectorCopy(planes->plane, plane);
				plane[3] = planes->plane[3];
			}
			if(tw->sphere.use)
			{
				// adjust the plane distance apropriately for radius
				plane[3] += tw->sphere.radius;

				// find the closest point on the capsule to the plane
				t = DotProduct(plane, tw->sphere.offset);
				if(t > 0.0f)
				{
					VectorSubtract(tw->start, tw->sphere.offset, startp);
				}
				else
				{
					VectorAdd(tw->start, tw->sphere.offset, startp);
				}
			}
			else
			{
				// NOTE: this works even though the plane might be flipped because the bbox is centered
				offset = DotProduct(tw->offsets[planes->signbits], plane);
				plane[3] += fabs(offset);
				VectorCopy(tw->start, startp);
			}

			if(DotProduct(plane, startp) - plane[3] > 0.0f)
			{
				break;
			}
		}
		
		if(j < triangle->numBorders)
		{
			continue;
		}
		
		// inside this triangle
		return qtrue;
	}
	return qfalse;
}
