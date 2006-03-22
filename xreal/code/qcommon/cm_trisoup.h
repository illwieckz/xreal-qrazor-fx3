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

//#define   CULL_BBOX

typedef struct
{
	float           plane[4];
	int             signbits;	// signx + (signy<<1) + (signz<<2), used as lookup during collision
} trianglePlane_t;

typedef struct
{
	int             surfacePlane;
	int             numBorders;	// 3 or four + 6 axial bevels + 4 or 3 * 4 edge bevels
	int             borderPlanes[4 + 6 + 16];
	int             borderInward[4 + 6 + 16];
	qboolean        borderNoAdjust[4 + 6 + 16];
} cTriangle_t;

typedef struct triSoupCollide_s
{
	vec3_t          bounds[2];
	
	int             numPlanes;	// surface planes plus edge planes
	trianglePlane_t *planes;
	
	int             numTriangles;
	cTriangle_t    *triangles;
} triSoupCollide_t;

typedef struct
{
	int             numTriangles;
	vec3_t          points[SHADER_MAX_TRIANGLES][3];
} cTriangleSoup_t;

struct triSoupCollide_s *CM_GenerateTriangleSoupCollide(int numVertexes, vec3_t * vertexes, int numIndexes, int * indexes);
