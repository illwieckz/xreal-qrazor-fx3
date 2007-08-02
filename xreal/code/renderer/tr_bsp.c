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
// tr_map.c

#include "tr_local.h"

/*

Loads and prepares a map file for scene rendering.

A single entry point:

void RE_LoadWorldMap( const char *name );

*/

static world_t  s_worldData;
static int      s_lightCount;
static growList_t s_interactions;
static int      s_lightIndexes[SHADER_MAX_INDEXES];
static int      s_numLightIndexes;
static int      s_shadowIndexes[SHADER_MAX_INDEXES];
static int      s_numShadowIndexes;
static cplane_t s_shadowPlanes[SHADER_MAX_TRIANGLES * 4];
static int      s_numShadowPlanes;
static byte    *fileBase;

int             c_redundantInteractions;

//===============================================================================

void HSVtoRGB(float h, float s, float v, float rgb[3])
{
	int             i;
	float           f;
	float           p, q, t;

	h *= 5;

	i = floor(h);
	f = h - i;

	p = v * (1 - s);
	q = v * (1 - s * f);
	t = v * (1 - s * (1 - f));

	switch (i)
	{
		case 0:
			rgb[0] = v;
			rgb[1] = t;
			rgb[2] = p;
			break;
		case 1:
			rgb[0] = q;
			rgb[1] = v;
			rgb[2] = p;
			break;
		case 2:
			rgb[0] = p;
			rgb[1] = v;
			rgb[2] = t;
			break;
		case 3:
			rgb[0] = p;
			rgb[1] = q;
			rgb[2] = v;
			break;
		case 4:
			rgb[0] = t;
			rgb[1] = p;
			rgb[2] = v;
			break;
		case 5:
			rgb[0] = v;
			rgb[1] = p;
			rgb[2] = q;
			break;
	}
}

/*
===============
R_ColorShiftLightingBytes
===============
*/
static void R_ColorShiftLightingBytes(byte in[4], byte out[4])
{
	int             shift, r, g, b;

	// shift the color data based on overbright range
	shift = r_mapOverBrightBits->integer - tr.overbrightBits;

	// shift the data based on overbright range
	r = in[0] << shift;
	g = in[1] << shift;
	b = in[2] << shift;

	// normalize by color instead of saturating to white
	if((r | g | b) > 255)
	{
		int             max;

		max = r > g ? r : g;
		max = max > b ? max : b;
		r = r * 255 / max;
		g = g * 255 / max;
		b = b * 255 / max;
	}

	out[0] = r;
	out[1] = g;
	out[2] = b;
	out[3] = in[3];
}


/*
=================
RE_SetWorldVisData

This is called by the clipmodel subsystem so we can share the 1.8 megs of
space in big maps...
=================
*/
void RE_SetWorldVisData(const byte * vis)
{
	tr.externalVisData = vis;
}

/*
=================
R_LoadVisibility
=================
*/
static void R_LoadVisibility(lump_t * l)
{
	int             len;
	byte           *buf;

	ri.Printf(PRINT_ALL, "...loading visibility\n");

	len = (s_worldData.numClusters + 63) & ~63;
	s_worldData.novis = ri.Hunk_Alloc(len, h_low);
	Com_Memset(s_worldData.novis, 0xff, len);

	len = l->filelen;
	if(!len)
	{
		return;
	}
	buf = fileBase + l->fileofs;

	s_worldData.numClusters = LittleLong(((int *)buf)[0]);
	s_worldData.clusterBytes = LittleLong(((int *)buf)[1]);

	// CM_Load should have given us the vis data to share, so
	// we don't need to allocate another copy
	if(tr.externalVisData)
	{
		s_worldData.vis = tr.externalVisData;
	}
	else
	{
		byte           *dest;

		dest = ri.Hunk_Alloc(len - 8, h_low);
		Com_Memcpy(dest, buf + 8, len - 8);
		s_worldData.vis = dest;
	}
}

//===============================================================================


/*
===============
ShaderForShaderNum
===============
*/
static shader_t *ShaderForShaderNum(int shaderNum)
{
	shader_t       *shader;
	dshader_t      *dsh;

	shaderNum = LittleLong(shaderNum);
	if(shaderNum < 0 || shaderNum >= s_worldData.numShaders)
	{
		ri.Error(ERR_DROP, "ShaderForShaderNum: bad num %i", shaderNum);
	}
	dsh = &s_worldData.shaders[shaderNum];

//  ri.Printf(PRINT_ALL, "ShaderForShaderNum: '%s'\n", dsh->shader);

	shader = R_FindShader(dsh->shader, SHADER_3D_STATIC, qtrue);

	// if the shader had errors, just use default shader
	if(shader->defaultShader)
	{
//      ri.Printf(PRINT_ALL, "failed\n");
		return tr.defaultShader;
	}

//  ri.Printf(PRINT_ALL, "success\n");
	return shader;
}

/*
===============
ParseFace
===============
*/
static void ParseFace(dsurface_t * ds, drawVert_t * verts, bspSurface_t * surf, int *indexes)
{
	int             i, j;
	srfSurfaceFace_t *cv;
	srfTriangle_t  *tri;
	int             numVerts, numTriangles;

	// get shader value
	surf->shader = ShaderForShaderNum(ds->shaderNum);
	if(r_singleShader->integer && !surf->shader->isSky)
	{
		surf->shader = tr.defaultShader;
	}

	numVerts = LittleLong(ds->numVerts);
	if(numVerts > MAX_FACE_POINTS)
	{
		ri.Printf(PRINT_WARNING, "WARNING: MAX_FACE_POINTS exceeded: %i\n", numVerts);
		numVerts = MAX_FACE_POINTS;
		surf->shader = tr.defaultShader;
	}
	numTriangles = LittleLong(ds->numIndexes) / 3;

	cv = ri.Hunk_Alloc(sizeof(*cv), h_low);
	cv->surfaceType = SF_FACE;

	cv->numTriangles = numTriangles;
	cv->triangles = ri.Hunk_Alloc(numTriangles * sizeof(cv->triangles[0]), h_low);

	cv->numVerts = numVerts;
	cv->verts = ri.Hunk_Alloc(numVerts * sizeof(cv->verts[0]), h_low);

	surf->data = (surfaceType_t *) cv;

	// copy vertexes
	ClearBounds(cv->bounds[0], cv->bounds[1]);
	verts += LittleLong(ds->firstVert);
	for(i = 0; i < numVerts; i++)
	{
		for(j = 0; j < 3; j++)
		{
			cv->verts[i].xyz[j] = LittleFloat(verts[i].xyz[j]);
			cv->verts[i].normal[j] = LittleFloat(verts[i].normal[j]);
		}
		AddPointToBounds(cv->verts[i].xyz, cv->bounds[0], cv->bounds[1]);
		for(j = 0; j < 2; j++)
		{
			cv->verts[i].st[j] = LittleFloat(verts[i].st[j]);
		}

		R_ColorShiftLightingBytes(verts[i].color, cv->verts[i].color);
	}

	// copy triangles
	indexes += LittleLong(ds->firstIndex);
	for(i = 0, tri = cv->triangles; i < numTriangles; i++, tri++)
	{
		for(j = 0; j < 3; j++)
		{
			tri->indexes[j] = LittleLong(indexes[i * 3 + j]);

			if(tri->indexes[j] < 0 || tri->indexes[j] >= numVerts)
			{
				ri.Error(ERR_DROP, "Bad index in face surface");
			}
		}
	}

	R_CalcSurfaceTriangleNeighbors(numTriangles, cv->triangles);
	R_CalcSurfaceTrianglePlanes(numTriangles, cv->triangles, cv->verts);

	// take the plane information from the lightmap vector
	for(i = 0; i < 3; i++)
	{
		cv->plane.normal[i] = LittleFloat(ds->lightmapVecs[2][i]);
	}
	cv->plane.dist = DotProduct(cv->verts[0].xyz, cv->plane.normal);
	SetPlaneSignbits(&cv->plane);
	cv->plane.type = PlaneTypeForNormal(cv->plane.normal);

	surf->data = (surfaceType_t *) cv;

	// Tr3B - calc tangent spaces
	{
		float          *v;
		const float    *v0, *v1, *v2;
		const float    *t0, *t1, *t2;
		vec3_t          tangent;
		vec3_t          binormal;
		vec3_t          normal;

		for(i = 0; i < numVerts; i++)
		{
			VectorClear(cv->verts[i].tangent);
			VectorClear(cv->verts[i].binormal);
			VectorClear(cv->verts[i].normal);
		}

		for(i = 0, tri = cv->triangles; i < numTriangles; i++, tri++)
		{
			v0 = cv->verts[tri->indexes[0]].xyz;
			v1 = cv->verts[tri->indexes[1]].xyz;
			v2 = cv->verts[tri->indexes[2]].xyz;

			t0 = cv->verts[tri->indexes[0]].st;
			t1 = cv->verts[tri->indexes[1]].st;
			t2 = cv->verts[tri->indexes[2]].st;

			R_CalcTangentSpace(tangent, binormal, normal, v0, v1, v2, t0, t1, t2);

			for(j = 0; j < 3; j++)
			{
				v = cv->verts[tri->indexes[j]].tangent;
				VectorAdd(v, tangent, v);
				v = cv->verts[tri->indexes[j]].binormal;
				VectorAdd(v, binormal, v);
				v = cv->verts[tri->indexes[j]].normal;
				VectorAdd(v, normal, v);
			}
		}

		for(i = 0; i < numVerts; i++)
		{
			VectorNormalize(cv->verts[i].tangent);
			VectorNormalize(cv->verts[i].binormal);
			VectorNormalize(cv->verts[i].normal);
		}
	}
}


/*
===============
ParseMesh
===============
*/
static void ParseMesh(dsurface_t * ds, drawVert_t * verts, bspSurface_t * surf)
{
	srfGridMesh_t  *grid;
	int             i, j;
	int             width, height, numPoints;
	srfVert_t       points[MAX_PATCH_SIZE * MAX_PATCH_SIZE];
	vec3_t          bounds[2];
	vec3_t          tmpVec;
	static surfaceType_t skipData = SF_SKIP;

	// get shader value
	surf->shader = ShaderForShaderNum(ds->shaderNum);
	if(r_singleShader->integer && !surf->shader->isSky)
	{
		surf->shader = tr.defaultShader;
	}

	// we may have a nodraw surface, because they might still need to
	// be around for movement clipping
	if(s_worldData.shaders[LittleLong(ds->shaderNum)].surfaceFlags & SURF_NODRAW)
	{
		surf->data = &skipData;
		return;
	}

	width = LittleLong(ds->patchWidth);
	height = LittleLong(ds->patchHeight);

	verts += LittleLong(ds->firstVert);
	numPoints = width * height;
	for(i = 0; i < numPoints; i++)
	{
		for(j = 0; j < 3; j++)
		{
			points[i].xyz[j] = LittleFloat(verts[i].xyz[j]);
			points[i].normal[j] = LittleFloat(verts[i].normal[j]);
		}
		for(j = 0; j < 2; j++)
		{
			points[i].st[j] = LittleFloat(verts[i].st[j]);
		}
		R_ColorShiftLightingBytes(verts[i].color, points[i].color);
	}

	// pre-tesseleate
	grid = R_SubdividePatchToGrid(width, height, points);
	surf->data = (surfaceType_t *) grid;

	// copy the level of detail origin, which is the center
	// of the group of all curves that must subdivide the same
	// to avoid cracking
	for(i = 0; i < 3; i++)
	{
		bounds[0][i] = LittleFloat(ds->lightmapVecs[0][i]);
		bounds[1][i] = LittleFloat(ds->lightmapVecs[1][i]);
	}
	VectorAdd(bounds[0], bounds[1], bounds[1]);
	VectorScale(bounds[1], 0.5f, grid->lodOrigin);
	VectorSubtract(bounds[0], grid->lodOrigin, tmpVec);
	grid->lodRadius = VectorLength(tmpVec);
}



/*
===============
ParseTriSurf
===============
*/
static void ParseTriSurf(dsurface_t * ds, drawVert_t * verts, bspSurface_t * surf, int *indexes)
{
	srfTriangles_t *cv;
	srfTriangle_t  *tri;
	int             i, j;
	int             numVerts, numTriangles;

	// get shader
	surf->shader = ShaderForShaderNum(ds->shaderNum);
	if(r_singleShader->integer && !surf->shader->isSky)
	{
		surf->shader = tr.defaultShader;
	}

	numVerts = LittleLong(ds->numVerts);
	numTriangles = LittleLong(ds->numIndexes) / 3;

	cv = ri.Hunk_Alloc(sizeof(*cv), h_low);
	cv->surfaceType = SF_TRIANGLES;

	cv->numTriangles = numTriangles;
	cv->triangles = ri.Hunk_Alloc(numTriangles * sizeof(cv->triangles[0]), h_low);

	cv->numVerts = numVerts;
	cv->verts = ri.Hunk_Alloc(numVerts * sizeof(cv->verts[0]), h_low);

	surf->data = (surfaceType_t *) cv;

	// copy vertexes
	verts += LittleLong(ds->firstVert);
	for(i = 0; i < numVerts; i++)
	{
		for(j = 0; j < 3; j++)
		{
			cv->verts[i].xyz[j] = LittleFloat(verts[i].xyz[j]);
			cv->verts[i].normal[j] = LittleFloat(verts[i].normal[j]);
		}

		for(j = 0; j < 2; j++)
		{
			cv->verts[i].st[j] = LittleFloat(verts[i].st[j]);
		}

		R_ColorShiftLightingBytes(verts[i].color, cv->verts[i].color);
	}

	// copy triangles
	indexes += LittleLong(ds->firstIndex);
	for(i = 0, tri = cv->triangles; i < numTriangles; i++, tri++)
	{
		for(j = 0; j < 3; j++)
		{
			tri->indexes[j] = LittleLong(indexes[i * 3 + j]);

			if(tri->indexes[j] < 0 || tri->indexes[j] >= numVerts)
			{
				ri.Error(ERR_DROP, "Bad index in face surface");
			}
		}
	}

	// calc bounding box
	// HACK: don't loop only through the vertices because they can contain bad data with .lwo models ...
	ClearBounds(cv->bounds[0], cv->bounds[1]);
	for(i = 0, tri = cv->triangles; i < numTriangles; i++, tri++)
	{
		AddPointToBounds(cv->verts[tri->indexes[0]].xyz, cv->bounds[0], cv->bounds[1]);
		AddPointToBounds(cv->verts[tri->indexes[1]].xyz, cv->bounds[0], cv->bounds[1]);
		AddPointToBounds(cv->verts[tri->indexes[2]].xyz, cv->bounds[0], cv->bounds[1]);
	}

	R_CalcSurfaceTriangleNeighbors(numTriangles, cv->triangles);
	R_CalcSurfaceTrianglePlanes(numTriangles, cv->triangles, cv->verts);

	// Tr3B - calc tangent spaces
	{
		float          *v;
		const float    *v0, *v1, *v2;
		const float    *t0, *t1, *t2;
		vec3_t          tangent;
		vec3_t          binormal;
		vec3_t          normal;

		for(i = 0; i < numVerts; i++)
		{
			VectorClear(cv->verts[i].tangent);
			VectorClear(cv->verts[i].binormal);
			VectorClear(cv->verts[i].normal);
		}

		for(i = 0, tri = cv->triangles; i < numTriangles; i++, tri++)
		{
			v0 = cv->verts[tri->indexes[0]].xyz;
			v1 = cv->verts[tri->indexes[1]].xyz;
			v2 = cv->verts[tri->indexes[2]].xyz;

			t0 = cv->verts[tri->indexes[0]].st;
			t1 = cv->verts[tri->indexes[1]].st;
			t2 = cv->verts[tri->indexes[2]].st;

			R_CalcTangentSpace(tangent, binormal, normal, v0, v1, v2, t0, t1, t2);

			for(j = 0; j < 3; j++)
			{
				v = cv->verts[tri->indexes[j]].tangent;
				VectorAdd(v, tangent, v);
				v = cv->verts[tri->indexes[j]].binormal;
				VectorAdd(v, binormal, v);
				v = cv->verts[tri->indexes[j]].normal;
				VectorAdd(v, normal, v);
			}
		}

		for(i = 0; i < numVerts; i++)
		{
			VectorNormalize(cv->verts[i].tangent);
			VectorNormalize(cv->verts[i].binormal);
			VectorNormalize(cv->verts[i].normal);
		}

		// do another extra smoothing for normals to avoid flat shading
		for(i = 0; i < numVerts; i++)
		{
			for(j = 0; j < numVerts; j++)
			{
				if(i == j)
					continue;

				if(R_CompareVert(&cv->verts[i], &cv->verts[j], qfalse))
				{
					VectorAdd(cv->verts[i].normal, cv->verts[j].normal, cv->verts[i].normal);
				}
			}

			VectorNormalize(cv->verts[i].normal);
		}
	}
}

/*
===============
ParseFlare
===============
*/
static void ParseFlare(dsurface_t * ds, drawVert_t * verts, bspSurface_t * surf, int *indexes)
{
	srfFlare_t     *flare;
	int             i;

	// get shader
	surf->shader = ShaderForShaderNum(ds->shaderNum);
	if(r_singleShader->integer && !surf->shader->isSky)
	{
		surf->shader = tr.defaultShader;
	}

	flare = ri.Hunk_Alloc(sizeof(*flare), h_low);
	flare->surfaceType = SF_FLARE;

	surf->data = (surfaceType_t *) flare;

	for(i = 0; i < 3; i++)
	{
		flare->origin[i] = LittleFloat(ds->lightmapOrigin[i]);
		flare->color[i] = LittleFloat(ds->lightmapVecs[0][i]);
		flare->normal[i] = LittleFloat(ds->lightmapVecs[2][i]);
	}
}


/*
=================
R_MergedWidthPoints

returns true if there are grid points merged on a width edge
=================
*/
int R_MergedWidthPoints(srfGridMesh_t * grid, int offset)
{
	int             i, j;

	for(i = 1; i < grid->width - 1; i++)
	{
		for(j = i + 1; j < grid->width - 1; j++)
		{
			if(fabs(grid->verts[i + offset].xyz[0] - grid->verts[j + offset].xyz[0]) > .1)
				continue;
			if(fabs(grid->verts[i + offset].xyz[1] - grid->verts[j + offset].xyz[1]) > .1)
				continue;
			if(fabs(grid->verts[i + offset].xyz[2] - grid->verts[j + offset].xyz[2]) > .1)
				continue;
			return qtrue;
		}
	}
	return qfalse;
}

/*
=================
R_MergedHeightPoints

returns true if there are grid points merged on a height edge
=================
*/
int R_MergedHeightPoints(srfGridMesh_t * grid, int offset)
{
	int             i, j;

	for(i = 1; i < grid->height - 1; i++)
	{
		for(j = i + 1; j < grid->height - 1; j++)
		{
			if(fabs(grid->verts[grid->width * i + offset].xyz[0] - grid->verts[grid->width * j + offset].xyz[0]) > .1)
				continue;
			if(fabs(grid->verts[grid->width * i + offset].xyz[1] - grid->verts[grid->width * j + offset].xyz[1]) > .1)
				continue;
			if(fabs(grid->verts[grid->width * i + offset].xyz[2] - grid->verts[grid->width * j + offset].xyz[2]) > .1)
				continue;
			return qtrue;
		}
	}
	return qfalse;
}

/*
=================
R_FixSharedVertexLodError_r

NOTE: never sync LoD through grid edges with merged points!

FIXME: write generalized version that also avoids cracks between a patch and one that meets half way?
=================
*/
void R_FixSharedVertexLodError_r(int start, srfGridMesh_t * grid1)
{
	int             j, k, l, m, n, offset1, offset2, touch;
	srfGridMesh_t  *grid2;

	for(j = start; j < s_worldData.numsurfaces; j++)
	{
		//
		grid2 = (srfGridMesh_t *) s_worldData.surfaces[j].data;
		// if this surface is not a grid
		if(grid2->surfaceType != SF_GRID)
			continue;
		// if the LOD errors are already fixed for this patch
		if(grid2->lodFixed == 2)
			continue;
		// grids in the same LOD group should have the exact same lod radius
		if(grid1->lodRadius != grid2->lodRadius)
			continue;
		// grids in the same LOD group should have the exact same lod origin
		if(grid1->lodOrigin[0] != grid2->lodOrigin[0])
			continue;
		if(grid1->lodOrigin[1] != grid2->lodOrigin[1])
			continue;
		if(grid1->lodOrigin[2] != grid2->lodOrigin[2])
			continue;
		//
		touch = qfalse;
		for(n = 0; n < 2; n++)
		{
			//
			if(n)
				offset1 = (grid1->height - 1) * grid1->width;
			else
				offset1 = 0;
			if(R_MergedWidthPoints(grid1, offset1))
				continue;
			for(k = 1; k < grid1->width - 1; k++)
			{
				for(m = 0; m < 2; m++)
				{

					if(m)
						offset2 = (grid2->height - 1) * grid2->width;
					else
						offset2 = 0;
					if(R_MergedWidthPoints(grid2, offset2))
						continue;
					for(l = 1; l < grid2->width - 1; l++)
					{
						//
						if(fabs(grid1->verts[k + offset1].xyz[0] - grid2->verts[l + offset2].xyz[0]) > .1)
							continue;
						if(fabs(grid1->verts[k + offset1].xyz[1] - grid2->verts[l + offset2].xyz[1]) > .1)
							continue;
						if(fabs(grid1->verts[k + offset1].xyz[2] - grid2->verts[l + offset2].xyz[2]) > .1)
							continue;
						// ok the points are equal and should have the same lod error
						grid2->widthLodError[l] = grid1->widthLodError[k];
						touch = qtrue;
					}
				}
				for(m = 0; m < 2; m++)
				{

					if(m)
						offset2 = grid2->width - 1;
					else
						offset2 = 0;
					if(R_MergedHeightPoints(grid2, offset2))
						continue;
					for(l = 1; l < grid2->height - 1; l++)
					{
						//
						if(fabs(grid1->verts[k + offset1].xyz[0] - grid2->verts[grid2->width * l + offset2].xyz[0]) > .1)
							continue;
						if(fabs(grid1->verts[k + offset1].xyz[1] - grid2->verts[grid2->width * l + offset2].xyz[1]) > .1)
							continue;
						if(fabs(grid1->verts[k + offset1].xyz[2] - grid2->verts[grid2->width * l + offset2].xyz[2]) > .1)
							continue;
						// ok the points are equal and should have the same lod error
						grid2->heightLodError[l] = grid1->widthLodError[k];
						touch = qtrue;
					}
				}
			}
		}
		for(n = 0; n < 2; n++)
		{
			//
			if(n)
				offset1 = grid1->width - 1;
			else
				offset1 = 0;
			if(R_MergedHeightPoints(grid1, offset1))
				continue;
			for(k = 1; k < grid1->height - 1; k++)
			{
				for(m = 0; m < 2; m++)
				{

					if(m)
						offset2 = (grid2->height - 1) * grid2->width;
					else
						offset2 = 0;
					if(R_MergedWidthPoints(grid2, offset2))
						continue;
					for(l = 1; l < grid2->width - 1; l++)
					{
						//
						if(fabs(grid1->verts[grid1->width * k + offset1].xyz[0] - grid2->verts[l + offset2].xyz[0]) > .1)
							continue;
						if(fabs(grid1->verts[grid1->width * k + offset1].xyz[1] - grid2->verts[l + offset2].xyz[1]) > .1)
							continue;
						if(fabs(grid1->verts[grid1->width * k + offset1].xyz[2] - grid2->verts[l + offset2].xyz[2]) > .1)
							continue;
						// ok the points are equal and should have the same lod error
						grid2->widthLodError[l] = grid1->heightLodError[k];
						touch = qtrue;
					}
				}
				for(m = 0; m < 2; m++)
				{

					if(m)
						offset2 = grid2->width - 1;
					else
						offset2 = 0;
					if(R_MergedHeightPoints(grid2, offset2))
						continue;
					for(l = 1; l < grid2->height - 1; l++)
					{
						//
						if(fabs
						   (grid1->verts[grid1->width * k + offset1].xyz[0] -
							grid2->verts[grid2->width * l + offset2].xyz[0]) > .1)
							continue;
						if(fabs
						   (grid1->verts[grid1->width * k + offset1].xyz[1] -
							grid2->verts[grid2->width * l + offset2].xyz[1]) > .1)
							continue;
						if(fabs
						   (grid1->verts[grid1->width * k + offset1].xyz[2] -
							grid2->verts[grid2->width * l + offset2].xyz[2]) > .1)
							continue;
						// ok the points are equal and should have the same lod error
						grid2->heightLodError[l] = grid1->heightLodError[k];
						touch = qtrue;
					}
				}
			}
		}
		if(touch)
		{
			grid2->lodFixed = 2;
			R_FixSharedVertexLodError_r(start, grid2);
			//NOTE: this would be correct but makes things really slow
			//grid2->lodFixed = 1;
		}
	}
}

/*
=================
R_FixSharedVertexLodError

This function assumes that all patches in one group are nicely stitched together for the highest LoD.
If this is not the case this function will still do its job but won't fix the highest LoD cracks.
=================
*/
void R_FixSharedVertexLodError(void)
{
	int             i;
	srfGridMesh_t  *grid1;

	for(i = 0; i < s_worldData.numsurfaces; i++)
	{
		//
		grid1 = (srfGridMesh_t *) s_worldData.surfaces[i].data;
		// if this surface is not a grid
		if(grid1->surfaceType != SF_GRID)
			continue;
		//
		if(grid1->lodFixed)
			continue;
		//
		grid1->lodFixed = 2;
		// recursively fix other patches in the same LOD group
		R_FixSharedVertexLodError_r(i + 1, grid1);
	}
}


/*
===============
R_StitchPatches
===============
*/
int R_StitchPatches(int grid1num, int grid2num)
{
	float          *v1, *v2;
	srfGridMesh_t  *grid1, *grid2;
	int             k, l, m, n, offset1, offset2, row, column;

	grid1 = (srfGridMesh_t *) s_worldData.surfaces[grid1num].data;
	grid2 = (srfGridMesh_t *) s_worldData.surfaces[grid2num].data;
	for(n = 0; n < 2; n++)
	{
		//
		if(n)
			offset1 = (grid1->height - 1) * grid1->width;
		else
			offset1 = 0;
		if(R_MergedWidthPoints(grid1, offset1))
			continue;
		for(k = 0; k < grid1->width - 2; k += 2)
		{

			for(m = 0; m < 2; m++)
			{

				if(grid2->width >= MAX_GRID_SIZE)
					break;
				if(m)
					offset2 = (grid2->height - 1) * grid2->width;
				else
					offset2 = 0;
				for(l = 0; l < grid2->width - 1; l++)
				{
					//
					v1 = grid1->verts[k + offset1].xyz;
					v2 = grid2->verts[l + offset2].xyz;
					if(fabs(v1[0] - v2[0]) > .1)
						continue;
					if(fabs(v1[1] - v2[1]) > .1)
						continue;
					if(fabs(v1[2] - v2[2]) > .1)
						continue;

					v1 = grid1->verts[k + 2 + offset1].xyz;
					v2 = grid2->verts[l + 1 + offset2].xyz;
					if(fabs(v1[0] - v2[0]) > .1)
						continue;
					if(fabs(v1[1] - v2[1]) > .1)
						continue;
					if(fabs(v1[2] - v2[2]) > .1)
						continue;
					//
					v1 = grid2->verts[l + offset2].xyz;
					v2 = grid2->verts[l + 1 + offset2].xyz;
					if(fabs(v1[0] - v2[0]) < .01 && fabs(v1[1] - v2[1]) < .01 && fabs(v1[2] - v2[2]) < .01)
						continue;
					//
					//ri.Printf( PRINT_ALL, "found highest LoD crack between two patches\n" );
					// insert column into grid2 right after after column l
					if(m)
						row = grid2->height - 1;
					else
						row = 0;
					grid2 = R_GridInsertColumn(grid2, l + 1, row, grid1->verts[k + 1 + offset1].xyz, grid1->widthLodError[k + 1]);
					grid2->lodStitched = qfalse;
					s_worldData.surfaces[grid2num].data = (void *)grid2;
					return qtrue;
				}
			}
			for(m = 0; m < 2; m++)
			{

				if(grid2->height >= MAX_GRID_SIZE)
					break;
				if(m)
					offset2 = grid2->width - 1;
				else
					offset2 = 0;
				for(l = 0; l < grid2->height - 1; l++)
				{
					//
					v1 = grid1->verts[k + offset1].xyz;
					v2 = grid2->verts[grid2->width * l + offset2].xyz;
					if(fabs(v1[0] - v2[0]) > .1)
						continue;
					if(fabs(v1[1] - v2[1]) > .1)
						continue;
					if(fabs(v1[2] - v2[2]) > .1)
						continue;

					v1 = grid1->verts[k + 2 + offset1].xyz;
					v2 = grid2->verts[grid2->width * (l + 1) + offset2].xyz;
					if(fabs(v1[0] - v2[0]) > .1)
						continue;
					if(fabs(v1[1] - v2[1]) > .1)
						continue;
					if(fabs(v1[2] - v2[2]) > .1)
						continue;
					//
					v1 = grid2->verts[grid2->width * l + offset2].xyz;
					v2 = grid2->verts[grid2->width * (l + 1) + offset2].xyz;
					if(fabs(v1[0] - v2[0]) < .01 && fabs(v1[1] - v2[1]) < .01 && fabs(v1[2] - v2[2]) < .01)
						continue;
					//
					//ri.Printf( PRINT_ALL, "found highest LoD crack between two patches\n" );
					// insert row into grid2 right after after row l
					if(m)
						column = grid2->width - 1;
					else
						column = 0;
					grid2 = R_GridInsertRow(grid2, l + 1, column, grid1->verts[k + 1 + offset1].xyz, grid1->widthLodError[k + 1]);
					grid2->lodStitched = qfalse;
					s_worldData.surfaces[grid2num].data = (void *)grid2;
					return qtrue;
				}
			}
		}
	}
	for(n = 0; n < 2; n++)
	{
		//
		if(n)
			offset1 = grid1->width - 1;
		else
			offset1 = 0;
		if(R_MergedHeightPoints(grid1, offset1))
			continue;
		for(k = 0; k < grid1->height - 2; k += 2)
		{
			for(m = 0; m < 2; m++)
			{

				if(grid2->width >= MAX_GRID_SIZE)
					break;
				if(m)
					offset2 = (grid2->height - 1) * grid2->width;
				else
					offset2 = 0;
				for(l = 0; l < grid2->width - 1; l++)
				{
					//
					v1 = grid1->verts[grid1->width * k + offset1].xyz;
					v2 = grid2->verts[l + offset2].xyz;
					if(fabs(v1[0] - v2[0]) > .1)
						continue;
					if(fabs(v1[1] - v2[1]) > .1)
						continue;
					if(fabs(v1[2] - v2[2]) > .1)
						continue;

					v1 = grid1->verts[grid1->width * (k + 2) + offset1].xyz;
					v2 = grid2->verts[l + 1 + offset2].xyz;
					if(fabs(v1[0] - v2[0]) > .1)
						continue;
					if(fabs(v1[1] - v2[1]) > .1)
						continue;
					if(fabs(v1[2] - v2[2]) > .1)
						continue;
					//
					v1 = grid2->verts[l + offset2].xyz;
					v2 = grid2->verts[(l + 1) + offset2].xyz;
					if(fabs(v1[0] - v2[0]) < .01 && fabs(v1[1] - v2[1]) < .01 && fabs(v1[2] - v2[2]) < .01)
						continue;
					//
					//ri.Printf( PRINT_ALL, "found highest LoD crack between two patches\n" );
					// insert column into grid2 right after after column l
					if(m)
						row = grid2->height - 1;
					else
						row = 0;
					grid2 = R_GridInsertColumn(grid2, l + 1, row,
											   grid1->verts[grid1->width * (k + 1) + offset1].xyz, grid1->heightLodError[k + 1]);
					grid2->lodStitched = qfalse;
					s_worldData.surfaces[grid2num].data = (void *)grid2;
					return qtrue;
				}
			}
			for(m = 0; m < 2; m++)
			{

				if(grid2->height >= MAX_GRID_SIZE)
					break;
				if(m)
					offset2 = grid2->width - 1;
				else
					offset2 = 0;
				for(l = 0; l < grid2->height - 1; l++)
				{
					//
					v1 = grid1->verts[grid1->width * k + offset1].xyz;
					v2 = grid2->verts[grid2->width * l + offset2].xyz;
					if(fabs(v1[0] - v2[0]) > .1)
						continue;
					if(fabs(v1[1] - v2[1]) > .1)
						continue;
					if(fabs(v1[2] - v2[2]) > .1)
						continue;

					v1 = grid1->verts[grid1->width * (k + 2) + offset1].xyz;
					v2 = grid2->verts[grid2->width * (l + 1) + offset2].xyz;
					if(fabs(v1[0] - v2[0]) > .1)
						continue;
					if(fabs(v1[1] - v2[1]) > .1)
						continue;
					if(fabs(v1[2] - v2[2]) > .1)
						continue;
					//
					v1 = grid2->verts[grid2->width * l + offset2].xyz;
					v2 = grid2->verts[grid2->width * (l + 1) + offset2].xyz;
					if(fabs(v1[0] - v2[0]) < .01 && fabs(v1[1] - v2[1]) < .01 && fabs(v1[2] - v2[2]) < .01)
						continue;
					//
					//ri.Printf( PRINT_ALL, "found highest LoD crack between two patches\n" );
					// insert row into grid2 right after after row l
					if(m)
						column = grid2->width - 1;
					else
						column = 0;
					grid2 = R_GridInsertRow(grid2, l + 1, column,
											grid1->verts[grid1->width * (k + 1) + offset1].xyz, grid1->heightLodError[k + 1]);
					grid2->lodStitched = qfalse;
					s_worldData.surfaces[grid2num].data = (void *)grid2;
					return qtrue;
				}
			}
		}
	}
	for(n = 0; n < 2; n++)
	{
		//
		if(n)
			offset1 = (grid1->height - 1) * grid1->width;
		else
			offset1 = 0;
		if(R_MergedWidthPoints(grid1, offset1))
			continue;
		for(k = grid1->width - 1; k > 1; k -= 2)
		{

			for(m = 0; m < 2; m++)
			{

				if(grid2->width >= MAX_GRID_SIZE)
					break;
				if(m)
					offset2 = (grid2->height - 1) * grid2->width;
				else
					offset2 = 0;
				for(l = 0; l < grid2->width - 1; l++)
				{
					//
					v1 = grid1->verts[k + offset1].xyz;
					v2 = grid2->verts[l + offset2].xyz;
					if(fabs(v1[0] - v2[0]) > .1)
						continue;
					if(fabs(v1[1] - v2[1]) > .1)
						continue;
					if(fabs(v1[2] - v2[2]) > .1)
						continue;

					v1 = grid1->verts[k - 2 + offset1].xyz;
					v2 = grid2->verts[l + 1 + offset2].xyz;
					if(fabs(v1[0] - v2[0]) > .1)
						continue;
					if(fabs(v1[1] - v2[1]) > .1)
						continue;
					if(fabs(v1[2] - v2[2]) > .1)
						continue;
					//
					v1 = grid2->verts[l + offset2].xyz;
					v2 = grid2->verts[(l + 1) + offset2].xyz;
					if(fabs(v1[0] - v2[0]) < .01 && fabs(v1[1] - v2[1]) < .01 && fabs(v1[2] - v2[2]) < .01)
						continue;
					//
					//ri.Printf( PRINT_ALL, "found highest LoD crack between two patches\n" );
					// insert column into grid2 right after after column l
					if(m)
						row = grid2->height - 1;
					else
						row = 0;
					grid2 = R_GridInsertColumn(grid2, l + 1, row, grid1->verts[k - 1 + offset1].xyz, grid1->widthLodError[k + 1]);
					grid2->lodStitched = qfalse;
					s_worldData.surfaces[grid2num].data = (void *)grid2;
					return qtrue;
				}
			}
			for(m = 0; m < 2; m++)
			{

				if(grid2->height >= MAX_GRID_SIZE)
					break;
				if(m)
					offset2 = grid2->width - 1;
				else
					offset2 = 0;
				for(l = 0; l < grid2->height - 1; l++)
				{
					//
					v1 = grid1->verts[k + offset1].xyz;
					v2 = grid2->verts[grid2->width * l + offset2].xyz;
					if(fabs(v1[0] - v2[0]) > .1)
						continue;
					if(fabs(v1[1] - v2[1]) > .1)
						continue;
					if(fabs(v1[2] - v2[2]) > .1)
						continue;

					v1 = grid1->verts[k - 2 + offset1].xyz;
					v2 = grid2->verts[grid2->width * (l + 1) + offset2].xyz;
					if(fabs(v1[0] - v2[0]) > .1)
						continue;
					if(fabs(v1[1] - v2[1]) > .1)
						continue;
					if(fabs(v1[2] - v2[2]) > .1)
						continue;
					//
					v1 = grid2->verts[grid2->width * l + offset2].xyz;
					v2 = grid2->verts[grid2->width * (l + 1) + offset2].xyz;
					if(fabs(v1[0] - v2[0]) < .01 && fabs(v1[1] - v2[1]) < .01 && fabs(v1[2] - v2[2]) < .01)
						continue;
					//
					//ri.Printf( PRINT_ALL, "found highest LoD crack between two patches\n" );
					// insert row into grid2 right after after row l
					if(m)
						column = grid2->width - 1;
					else
						column = 0;
					grid2 = R_GridInsertRow(grid2, l + 1, column, grid1->verts[k - 1 + offset1].xyz, grid1->widthLodError[k + 1]);
					if(!grid2)
						break;
					grid2->lodStitched = qfalse;
					s_worldData.surfaces[grid2num].data = (void *)grid2;
					return qtrue;
				}
			}
		}
	}
	for(n = 0; n < 2; n++)
	{
		//
		if(n)
			offset1 = grid1->width - 1;
		else
			offset1 = 0;
		if(R_MergedHeightPoints(grid1, offset1))
			continue;
		for(k = grid1->height - 1; k > 1; k -= 2)
		{
			for(m = 0; m < 2; m++)
			{

				if(grid2->width >= MAX_GRID_SIZE)
					break;
				if(m)
					offset2 = (grid2->height - 1) * grid2->width;
				else
					offset2 = 0;
				for(l = 0; l < grid2->width - 1; l++)
				{
					//
					v1 = grid1->verts[grid1->width * k + offset1].xyz;
					v2 = grid2->verts[l + offset2].xyz;
					if(fabs(v1[0] - v2[0]) > .1)
						continue;
					if(fabs(v1[1] - v2[1]) > .1)
						continue;
					if(fabs(v1[2] - v2[2]) > .1)
						continue;

					v1 = grid1->verts[grid1->width * (k - 2) + offset1].xyz;
					v2 = grid2->verts[l + 1 + offset2].xyz;
					if(fabs(v1[0] - v2[0]) > .1)
						continue;
					if(fabs(v1[1] - v2[1]) > .1)
						continue;
					if(fabs(v1[2] - v2[2]) > .1)
						continue;
					//
					v1 = grid2->verts[l + offset2].xyz;
					v2 = grid2->verts[(l + 1) + offset2].xyz;
					if(fabs(v1[0] - v2[0]) < .01 && fabs(v1[1] - v2[1]) < .01 && fabs(v1[2] - v2[2]) < .01)
						continue;
					//
					//ri.Printf( PRINT_ALL, "found highest LoD crack between two patches\n" );
					// insert column into grid2 right after after column l
					if(m)
						row = grid2->height - 1;
					else
						row = 0;
					grid2 = R_GridInsertColumn(grid2, l + 1, row,
											   grid1->verts[grid1->width * (k - 1) + offset1].xyz, grid1->heightLodError[k + 1]);
					grid2->lodStitched = qfalse;
					s_worldData.surfaces[grid2num].data = (void *)grid2;
					return qtrue;
				}
			}
			for(m = 0; m < 2; m++)
			{

				if(grid2->height >= MAX_GRID_SIZE)
					break;
				if(m)
					offset2 = grid2->width - 1;
				else
					offset2 = 0;
				for(l = 0; l < grid2->height - 1; l++)
				{
					//
					v1 = grid1->verts[grid1->width * k + offset1].xyz;
					v2 = grid2->verts[grid2->width * l + offset2].xyz;
					if(fabs(v1[0] - v2[0]) > .1)
						continue;
					if(fabs(v1[1] - v2[1]) > .1)
						continue;
					if(fabs(v1[2] - v2[2]) > .1)
						continue;

					v1 = grid1->verts[grid1->width * (k - 2) + offset1].xyz;
					v2 = grid2->verts[grid2->width * (l + 1) + offset2].xyz;
					if(fabs(v1[0] - v2[0]) > .1)
						continue;
					if(fabs(v1[1] - v2[1]) > .1)
						continue;
					if(fabs(v1[2] - v2[2]) > .1)
						continue;
					//
					v1 = grid2->verts[grid2->width * l + offset2].xyz;
					v2 = grid2->verts[grid2->width * (l + 1) + offset2].xyz;
					if(fabs(v1[0] - v2[0]) < .01 && fabs(v1[1] - v2[1]) < .01 && fabs(v1[2] - v2[2]) < .01)
						continue;
					//
					//ri.Printf( PRINT_ALL, "found highest LoD crack between two patches\n" );
					// insert row into grid2 right after after row l
					if(m)
						column = grid2->width - 1;
					else
						column = 0;
					grid2 = R_GridInsertRow(grid2, l + 1, column,
											grid1->verts[grid1->width * (k - 1) + offset1].xyz, grid1->heightLodError[k + 1]);
					grid2->lodStitched = qfalse;
					s_worldData.surfaces[grid2num].data = (void *)grid2;
					return qtrue;
				}
			}
		}
	}
	return qfalse;
}

/*
===============
R_TryStitchPatch

This function will try to stitch patches in the same LoD group together for the highest LoD.

Only single missing vertice cracks will be fixed.

Vertices will be joined at the patch side a crack is first found, at the other side
of the patch (on the same row or column) the vertices will not be joined and cracks
might still appear at that side.
===============
*/
int R_TryStitchingPatch(int grid1num)
{
	int             j, numstitches;
	srfGridMesh_t  *grid1, *grid2;

	numstitches = 0;
	grid1 = (srfGridMesh_t *) s_worldData.surfaces[grid1num].data;
	for(j = 0; j < s_worldData.numsurfaces; j++)
	{
		//
		grid2 = (srfGridMesh_t *) s_worldData.surfaces[j].data;
		// if this surface is not a grid
		if(grid2->surfaceType != SF_GRID)
			continue;
		// grids in the same LOD group should have the exact same lod radius
		if(grid1->lodRadius != grid2->lodRadius)
			continue;
		// grids in the same LOD group should have the exact same lod origin
		if(grid1->lodOrigin[0] != grid2->lodOrigin[0])
			continue;
		if(grid1->lodOrigin[1] != grid2->lodOrigin[1])
			continue;
		if(grid1->lodOrigin[2] != grid2->lodOrigin[2])
			continue;
		//
		while(R_StitchPatches(grid1num, j))
		{
			numstitches++;
		}
	}
	return numstitches;
}

/*
===============
R_StitchAllPatches
===============
*/
void R_StitchAllPatches(void)
{
	int             i, stitched, numstitches;
	srfGridMesh_t  *grid1;

	ri.Printf(PRINT_ALL, "...stitching LoD cracks\n");

	numstitches = 0;
	do
	{
		stitched = qfalse;
		for(i = 0; i < s_worldData.numsurfaces; i++)
		{
			//
			grid1 = (srfGridMesh_t *) s_worldData.surfaces[i].data;
			// if this surface is not a grid
			if(grid1->surfaceType != SF_GRID)
				continue;
			//
			if(grid1->lodStitched)
				continue;
			//
			grid1->lodStitched = qtrue;
			stitched = qtrue;
			//
			numstitches += R_TryStitchingPatch(i);
		}
	} while(stitched);
	ri.Printf(PRINT_ALL, "stitched %d LoD cracks\n", numstitches);
}

/*
===============
R_MovePatchSurfacesToHunk
===============
*/
void R_MovePatchSurfacesToHunk(void)
{
	int             i, size;
	srfGridMesh_t  *grid, *hunkgrid;

	for(i = 0; i < s_worldData.numsurfaces; i++)
	{
		//
		grid = (srfGridMesh_t *) s_worldData.surfaces[i].data;

		// if this surface is not a grid
		if(grid->surfaceType != SF_GRID)
			continue;
		//
		size = sizeof(*grid);
		hunkgrid = ri.Hunk_Alloc(size, h_low);
		Com_Memcpy(hunkgrid, grid, size);

		hunkgrid->widthLodError = ri.Hunk_Alloc(grid->width * 4, h_low);
		Com_Memcpy(hunkgrid->widthLodError, grid->widthLodError, grid->width * 4);

		hunkgrid->heightLodError = ri.Hunk_Alloc(grid->height * 4, h_low);
		Com_Memcpy(hunkgrid->heightLodError, grid->heightLodError, grid->height * 4);

		hunkgrid->numTriangles = grid->numTriangles;
		hunkgrid->triangles = ri.Hunk_Alloc(grid->numTriangles * sizeof(srfTriangle_t), h_low);
		Com_Memcpy(hunkgrid->triangles, grid->triangles, grid->numTriangles * sizeof(srfTriangle_t));

		hunkgrid->numVerts = grid->numVerts;
		hunkgrid->verts = ri.Hunk_Alloc(grid->numVerts * sizeof(srfVert_t), h_low);
		Com_Memcpy(hunkgrid->verts, grid->verts, grid->numVerts * sizeof(srfVert_t));

		R_FreeSurfaceGridMesh(grid);

		s_worldData.surfaces[i].data = (void *)hunkgrid;
	}
}

/*
===============
R_CreateWorldVBOs
===============
*/
static void R_CreateWorldVBOs()
{
	int             i, j, k;

	int             vertexesNum;
	byte           *data;
	int             dataSize;
	int             dataOfs;

	int             indexesNum;
	byte           *indexes;
	int             indexesSize;
	int             indexesOfs;

	bspSurface_t   *surface;
	vec4_t          tmp;


	if(!glConfig.vertexBufferObjectAvailable)
	{
		return;
	}

	ri.Printf(PRINT_ALL, "...calculating world VBOs\n");

	// count vertices and indices
	vertexesNum = 0;
	indexesNum = 0;

	for(k = 0, surface = &s_worldData.surfaces[0]; k < s_worldData.numsurfaces; k++, surface++)
	{
		if(*surface->data == SF_FACE)
		{
			srfSurfaceFace_t *cv = (srfSurfaceFace_t *) surface->data;

			vertexesNum += cv->numVerts;
			indexesNum += cv->numTriangles * 3;
		}
		else if(*surface->data == SF_GRID)
		{
			srfGridMesh_t  *grid = (srfGridMesh_t *) surface->data;

			vertexesNum += grid->numVerts;
			indexesNum += grid->numTriangles * 3;
		}
		else if(*surface->data == SF_TRIANGLES)
		{
			srfTriangles_t *tri = (srfTriangles_t *) surface->data;

			vertexesNum += tri->numVerts;
			indexesNum += tri->numTriangles * 3;
		}
	}

	// create VBOs
	qglGenBuffersARB(1, &s_worldData.vertsVBO);
	qglGenBuffersARB(1, &s_worldData.indexesVBO);

	dataSize = vertexesNum * (sizeof(vec4_t) * 6 + sizeof(color4ub_t));
	data = ri.Hunk_AllocateTempMemory(dataSize);
	dataOfs = 0;

	indexesSize = indexesNum * sizeof(int);
	indexes = ri.Hunk_AllocateTempMemory(indexesSize);
	indexesOfs = 0;

	for(k = 0, surface = &s_worldData.surfaces[0]; k < s_worldData.numsurfaces; k++, surface++)
	{
		if(*surface->data == SF_FACE)
		{
			srfSurfaceFace_t *cv = (srfSurfaceFace_t *) surface->data;

			if(cv->numVerts)
			{
				cv->vertsVBO = s_worldData.vertsVBO;

				// set up xyz array
				cv->ofsXYZ = dataOfs;
				for(i = 0; i < cv->numVerts; i++)
				{
					for(j = 0; j < 3; j++)
					{
						tmp[j] = cv->verts[i].xyz[j];
					}
					tmp[3] = 1;

					memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
					dataOfs += sizeof(vec4_t);
				}

				// set up texcoords array
				cv->ofsTexCoords = dataOfs;
				for(i = 0; i < cv->numVerts; i++)
				{
					for(j = 0; j < 2; j++)
					{
						tmp[j] = cv->verts[i].st[j];
					}
					tmp[2] = 0;
					tmp[3] = 1;

					memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
					dataOfs += sizeof(vec4_t);
				}

				// set up tangents array
				cv->ofsTangents = dataOfs;
				for(i = 0; i < cv->numVerts; i++)
				{
					for(j = 0; j < 3; j++)
					{
						tmp[j] = cv->verts[i].tangent[j];
					}
					tmp[3] = 1;

					memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
					dataOfs += sizeof(vec4_t);
				}

				// set up binormals array
				cv->ofsBinormals = dataOfs;
				for(i = 0; i < cv->numVerts; i++)
				{
					for(j = 0; j < 3; j++)
					{
						tmp[j] = cv->verts[i].binormal[j];
					}
					tmp[3] = 1;

					memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
					dataOfs += sizeof(vec4_t);
				}

				// set up normals array
				cv->ofsNormals = dataOfs;
				for(i = 0; i < cv->numVerts; i++)
				{
					for(j = 0; j < 3; j++)
					{
						tmp[j] = cv->verts[i].normal[j];
					}
					tmp[3] = 1;

					memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
					dataOfs += sizeof(vec4_t);
				}

				// set up colors array
				cv->ofsColors = dataOfs;
				for(i = 0; i < cv->numVerts; i++)
				{
					memcpy(data + dataOfs, cv->verts[i].color, sizeof(color4ub_t));
					dataOfs += sizeof(color4ub_t);
				}
			}
			else
			{
				cv->vertsVBO = 0;
			}

			if(cv->numTriangles)
			{
				srfTriangle_t  *tri;

				cv->indexesVBO = s_worldData.indexesVBO;

				// set up triangle indices
				cv->ofsIndexes = indexesOfs;
				for(i = 0, tri = cv->triangles; i < cv->numTriangles; i++, tri++)
				{
					memcpy(indexes + indexesOfs, tri->indexes, sizeof(tri->indexes));
					indexesOfs += sizeof(tri->indexes);
				}
			}
			else
			{
				cv->indexesVBO = 0;
			}
		}
		else if(*surface->data == SF_GRID)
		{
			srfGridMesh_t  *cv = (srfGridMesh_t *) surface->data;

			if(cv->numVerts)
			{
				cv->vertsVBO = s_worldData.vertsVBO;

				// set up xyz array
				cv->ofsXYZ = dataOfs;
				for(i = 0; i < cv->numVerts; i++)
				{
					for(j = 0; j < 3; j++)
					{
						tmp[j] = cv->verts[i].xyz[j];
					}
					tmp[3] = 1;

					memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
					dataOfs += sizeof(vec4_t);
				}

				// set up texcoords array
				cv->ofsTexCoords = dataOfs;
				for(i = 0; i < cv->numVerts; i++)
				{
					for(j = 0; j < 2; j++)
					{
						tmp[j] = cv->verts[i].st[j];
					}
					tmp[2] = 0;
					tmp[3] = 1;

					memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
					dataOfs += sizeof(vec4_t);
				}

				// set up tangents array
				cv->ofsTangents = dataOfs;
				for(i = 0; i < cv->numVerts; i++)
				{
					for(j = 0; j < 3; j++)
					{
						tmp[j] = cv->verts[i].tangent[j];
					}
					tmp[3] = 1;

					memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
					dataOfs += sizeof(vec4_t);
				}

				// set up binormals array
				cv->ofsBinormals = dataOfs;
				for(i = 0; i < cv->numVerts; i++)
				{
					for(j = 0; j < 3; j++)
					{
						tmp[j] = cv->verts[i].binormal[j];
					}
					tmp[3] = 1;

					memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
					dataOfs += sizeof(vec4_t);
				}

				// set up normals array
				cv->ofsNormals = dataOfs;
				for(i = 0; i < cv->numVerts; i++)
				{
					for(j = 0; j < 3; j++)
					{
						tmp[j] = cv->verts[i].normal[j];
					}
					tmp[3] = 1;

					memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
					dataOfs += sizeof(vec4_t);
				}

				// set up colors array
				cv->ofsColors = dataOfs;
				for(i = 0; i < cv->numVerts; i++)
				{
					memcpy(data + dataOfs, cv->verts[i].color, sizeof(color4ub_t));
					dataOfs += sizeof(color4ub_t);
				}
			}
			else
			{
				cv->vertsVBO = 0;
			}

			if(cv->numTriangles)
			{
				srfTriangle_t  *tri;

				cv->indexesVBO = s_worldData.indexesVBO;

				// set up triangle indices
				cv->ofsIndexes = indexesOfs;
				for(i = 0, tri = cv->triangles; i < cv->numTriangles; i++, tri++)
				{
					memcpy(indexes + indexesOfs, tri->indexes, sizeof(tri->indexes));
					indexesOfs += sizeof(tri->indexes);
				}
			}
			else
			{
				cv->indexesVBO = 0;
			}
		}
		else if(*surface->data == SF_TRIANGLES)
		{
			srfTriangles_t *cv = (srfTriangles_t *) surface->data;

			if(cv->numVerts)
			{
				cv->vertsVBO = s_worldData.vertsVBO;

				// set up xyz array
				cv->ofsXYZ = dataOfs;
				for(i = 0; i < cv->numVerts; i++)
				{
					for(j = 0; j < 3; j++)
					{
						tmp[j] = cv->verts[i].xyz[j];
					}
					tmp[3] = 1;

					memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
					dataOfs += sizeof(vec4_t);
				}

				// set up texcoords array
				cv->ofsTexCoords = dataOfs;
				for(i = 0; i < cv->numVerts; i++)
				{
					for(j = 0; j < 2; j++)
					{
						tmp[j] = cv->verts[i].st[j];
					}
					tmp[2] = 0;
					tmp[3] = 1;

					memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
					dataOfs += sizeof(vec4_t);
				}

				// set up tangents array
				cv->ofsTangents = dataOfs;
				for(i = 0; i < cv->numVerts; i++)
				{
					for(j = 0; j < 3; j++)
					{
						tmp[j] = cv->verts[i].tangent[j];
					}
					tmp[3] = 1;

					memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
					dataOfs += sizeof(vec4_t);
				}

				// set up binormals array
				cv->ofsBinormals = dataOfs;
				for(i = 0; i < cv->numVerts; i++)
				{
					for(j = 0; j < 3; j++)
					{
						tmp[j] = cv->verts[i].binormal[j];
					}
					tmp[3] = 1;

					memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
					dataOfs += sizeof(vec4_t);
				}

				// set up normals array
				cv->ofsNormals = dataOfs;
				for(i = 0; i < cv->numVerts; i++)
				{
					for(j = 0; j < 3; j++)
					{
						tmp[j] = cv->verts[i].normal[j];
					}
					tmp[3] = 1;

					memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
					dataOfs += sizeof(vec4_t);
				}

				// set up colors array
				cv->ofsColors = dataOfs;
				for(i = 0; i < cv->numVerts; i++)
				{
					memcpy(data + dataOfs, cv->verts[i].color, sizeof(color4ub_t));
					dataOfs += sizeof(color4ub_t);
				}
			}
			else
			{
				cv->vertsVBO = 0;
			}

			if(cv->numTriangles)
			{
				srfTriangle_t  *tri;

				cv->indexesVBO = s_worldData.indexesVBO;

				// set up triangle indices
				cv->ofsIndexes = indexesOfs;
				for(i = 0, tri = cv->triangles; i < cv->numTriangles; i++, tri++)
				{
					memcpy(indexes + indexesOfs, tri->indexes, sizeof(tri->indexes));
					indexesOfs += sizeof(tri->indexes);
				}
			}
			else
			{
				cv->indexesVBO = 0;
			}
		}
	}

	qglBindBufferARB(GL_ARRAY_BUFFER_ARB, s_worldData.vertsVBO);
	qglBufferDataARB(GL_ARRAY_BUFFER_ARB, dataSize, data, GL_STATIC_DRAW_ARB);

	qglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, s_worldData.indexesVBO);
	qglBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, indexesSize, indexes, GL_STATIC_DRAW_ARB);

	qglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
	qglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

	ri.Hunk_FreeTempMemory(indexes);
	ri.Hunk_FreeTempMemory(data);

	// megs
	ri.Printf(PRINT_ALL, "world data VBO size: %d.%02d MB\n", dataSize / (1024 * 1024),
			  (dataSize % (1024 * 1024)) * 100 / (1024 * 1024));
	ri.Printf(PRINT_ALL, "world tris VBO size: %d.%02d MB\n", indexesSize / (1024 * 1024),
			  (indexesSize % (1024 * 1024)) * 100 / (1024 * 1024));
}

/*
===============
R_LoadSurfaces
===============
*/
static void R_LoadSurfaces(lump_t * surfs, lump_t * verts, lump_t * indexLump)
{
	dsurface_t     *in;
	bspSurface_t   *out;
	drawVert_t     *dv;
	int            *indexes;
	int             count;
	int             numFaces, numMeshes, numTriSurfs, numFlares;
	int             i;

	ri.Printf(PRINT_ALL, "...loading surfaces\n");

	numFaces = 0;
	numMeshes = 0;
	numTriSurfs = 0;
	numFlares = 0;

	in = (void *)(fileBase + surfs->fileofs);
	if(surfs->filelen % sizeof(*in))
		ri.Error(ERR_DROP, "LoadMap: funny lump size in %s", s_worldData.name);
	count = surfs->filelen / sizeof(*in);

	dv = (void *)(fileBase + verts->fileofs);
	if(verts->filelen % sizeof(*dv))
		ri.Error(ERR_DROP, "LoadMap: funny lump size in %s", s_worldData.name);

	indexes = (void *)(fileBase + indexLump->fileofs);
	if(indexLump->filelen % sizeof(*indexes))
		ri.Error(ERR_DROP, "LoadMap: funny lump size in %s", s_worldData.name);

	out = ri.Hunk_Alloc(count * sizeof(*out), h_low);

	s_worldData.surfaces = out;
	s_worldData.numsurfaces = count;

	for(i = 0; i < count; i++, in++, out++)
	{
		switch (LittleLong(in->surfaceType))
		{
			case MST_PATCH:
				ParseMesh(in, dv, out);
				numMeshes++;
				break;
			case MST_TRIANGLE_SOUP:
				ParseTriSurf(in, dv, out, indexes);
				numTriSurfs++;
				break;
			case MST_PLANAR:
				ParseFace(in, dv, out, indexes);
				numFaces++;
				break;
			case MST_FLARE:
				ParseFlare(in, dv, out, indexes);
				numFlares++;
				break;
			default:
				ri.Error(ERR_DROP, "Bad surfaceType");
		}
	}

	ri.Printf(PRINT_ALL, "...loaded %d faces, %i meshes, %i trisurfs, %i flares\n", numFaces, numMeshes, numTriSurfs, numFlares);

	if(r_stitchCurves->integer)
	{
		R_StitchAllPatches();
	}

	R_FixSharedVertexLodError();

	if(r_stitchCurves->integer)
	{
		R_MovePatchSurfacesToHunk();
	}

	R_CreateWorldVBOs();
}



/*
=================
R_LoadSubmodels
=================
*/
static void R_LoadSubmodels(lump_t * l)
{
	dmodel_t       *in;
	bspModel_t     *out;
	int             i, j, count;

	ri.Printf(PRINT_ALL, "...loading submodels\n");

	in = (void *)(fileBase + l->fileofs);
	if(l->filelen % sizeof(*in))
		ri.Error(ERR_DROP, "LoadMap: funny lump size in %s", s_worldData.name);
	count = l->filelen / sizeof(*in);

	s_worldData.bmodels = out = ri.Hunk_Alloc(count * sizeof(*out), h_low);

	for(i = 0; i < count; i++, in++, out++)
	{
		model_t        *model;

		model = R_AllocModel();

		assert(model != NULL);	// this should never happen

		model->type = MOD_BRUSH;
		model->bmodel = out;
		Com_sprintf(model->name, sizeof(model->name), "*%d", i);

		for(j = 0; j < 3; j++)
		{
			out->bounds[0][j] = LittleFloat(in->mins[j]);
			out->bounds[1][j] = LittleFloat(in->maxs[j]);
		}

		out->firstSurface = s_worldData.surfaces + LittleLong(in->firstSurface);
		out->numSurfaces = LittleLong(in->numSurfaces);
	}
}



//==================================================================

/*
=================
R_SetParent
=================
*/
static void R_SetParent(bspNode_t * node, bspNode_t * parent)
{
	node->parent = parent;
	if(node->contents != -1)
		return;
	R_SetParent(node->children[0], node);
	R_SetParent(node->children[1], node);
}

/*
=================
R_LoadNodesAndLeafs
=================
*/
static void R_LoadNodesAndLeafs(lump_t * nodeLump, lump_t * leafLump)
{
	int             i, j, p;
	dnode_t        *in;
	dleaf_t        *inLeaf;
	bspNode_t      *out;
	int             numNodes, numLeafs;

	ri.Printf(PRINT_ALL, "...loading nodes and leaves\n");

	in = (void *)(fileBase + nodeLump->fileofs);
	if(nodeLump->filelen % sizeof(dnode_t) || leafLump->filelen % sizeof(dleaf_t))
	{
		ri.Error(ERR_DROP, "LoadMap: funny lump size in %s", s_worldData.name);
	}
	numNodes = nodeLump->filelen / sizeof(dnode_t);
	numLeafs = leafLump->filelen / sizeof(dleaf_t);

	out = ri.Hunk_Alloc((numNodes + numLeafs) * sizeof(*out), h_low);

	s_worldData.nodes = out;
	s_worldData.numnodes = numNodes + numLeafs;
	s_worldData.numDecisionNodes = numNodes;

	// load nodes
	for(i = 0; i < numNodes; i++, in++, out++)
	{
		for(j = 0; j < 3; j++)
		{
			out->mins[j] = LittleLong(in->mins[j]);
			out->maxs[j] = LittleLong(in->maxs[j]);
		}

		p = LittleLong(in->planeNum);
		out->plane = s_worldData.planes + p;

		out->contents = CONTENTS_NODE;	// differentiate from leafs

		for(j = 0; j < 2; j++)
		{
			p = LittleLong(in->children[j]);
			if(p >= 0)
				out->children[j] = s_worldData.nodes + p;
			else
				out->children[j] = s_worldData.nodes + numNodes + (-1 - p);
		}
	}

	// load leafs
	inLeaf = (void *)(fileBase + leafLump->fileofs);
	for(i = 0; i < numLeafs; i++, inLeaf++, out++)
	{
		for(j = 0; j < 3; j++)
		{
			out->mins[j] = LittleLong(inLeaf->mins[j]);
			out->maxs[j] = LittleLong(inLeaf->maxs[j]);
		}

		out->cluster = LittleLong(inLeaf->cluster);
		out->area = LittleLong(inLeaf->area);

		if(out->cluster >= s_worldData.numClusters)
		{
			s_worldData.numClusters = out->cluster + 1;
		}

		out->firstmarksurface = s_worldData.marksurfaces + LittleLong(inLeaf->firstLeafSurface);
		out->nummarksurfaces = LittleLong(inLeaf->numLeafSurfaces);
	}

	// chain decendants
	R_SetParent(s_worldData.nodes, NULL);
}

//=============================================================================

/*
=================
R_LoadShaders
=================
*/
static void R_LoadShaders(lump_t * l)
{
	int             i, count;
	dshader_t      *in, *out;

	ri.Printf(PRINT_ALL, "...loading shaders\n");

	in = (void *)(fileBase + l->fileofs);
	if(l->filelen % sizeof(*in))
		ri.Error(ERR_DROP, "LoadMap: funny lump size in %s", s_worldData.name);
	count = l->filelen / sizeof(*in);
	out = ri.Hunk_Alloc(count * sizeof(*out), h_low);

	s_worldData.shaders = out;
	s_worldData.numShaders = count;

	Com_Memcpy(out, in, count * sizeof(*out));

	for(i = 0; i < count; i++)
	{
		out[i].surfaceFlags = LittleLong(out[i].surfaceFlags);
		out[i].contentFlags = LittleLong(out[i].contentFlags);
	}
}


/*
=================
R_LoadMarksurfaces
=================
*/
static void R_LoadMarksurfaces(lump_t * l)
{
	int             i, j, count;
	int            *in;
	bspSurface_t  **out;

	ri.Printf(PRINT_ALL, "...loading mark surfaces\n");

	in = (void *)(fileBase + l->fileofs);
	if(l->filelen % sizeof(*in))
		ri.Error(ERR_DROP, "LoadMap: funny lump size in %s", s_worldData.name);
	count = l->filelen / sizeof(*in);
	out = ri.Hunk_Alloc(count * sizeof(*out), h_low);

	s_worldData.marksurfaces = out;
	s_worldData.nummarksurfaces = count;

	for(i = 0; i < count; i++)
	{
		j = LittleLong(in[i]);
		out[i] = s_worldData.surfaces + j;
	}
}


/*
=================
R_LoadPlanes
=================
*/
static void R_LoadPlanes(lump_t * l)
{
	int             i, j;
	cplane_t       *out;
	dplane_t       *in;
	int             count;
	int             bits;

	ri.Printf(PRINT_ALL, "...loading planes\n");

	in = (void *)(fileBase + l->fileofs);
	if(l->filelen % sizeof(*in))
		ri.Error(ERR_DROP, "LoadMap: funny lump size in %s", s_worldData.name);
	count = l->filelen / sizeof(*in);
	out = ri.Hunk_Alloc(count * 2 * sizeof(*out), h_low);

	s_worldData.planes = out;
	s_worldData.numplanes = count;

	for(i = 0; i < count; i++, in++, out++)
	{
		bits = 0;
		for(j = 0; j < 3; j++)
		{
			out->normal[j] = LittleFloat(in->normal[j]);
			if(out->normal[j] < 0)
			{
				bits |= 1 << j;
			}
		}

		out->dist = LittleFloat(in->dist);
		out->type = PlaneTypeForNormal(out->normal);
		out->signbits = bits;
	}
}


/*
================
R_LoadLightGrid
================
*/
void R_LoadLightGrid(lump_t * l)
{
	int             i;
	vec3_t          maxs;
	int             numGridPoints;
	world_t        *w;
	float          *wMins, *wMaxs;

	ri.Printf(PRINT_ALL, "...loading light grid\n");

	w = &s_worldData;

	w->lightGridInverseSize[0] = 1.0f / w->lightGridSize[0];
	w->lightGridInverseSize[1] = 1.0f / w->lightGridSize[1];
	w->lightGridInverseSize[2] = 1.0f / w->lightGridSize[2];

	wMins = w->bmodels[0].bounds[0];
	wMaxs = w->bmodels[0].bounds[1];

	for(i = 0; i < 3; i++)
	{
		w->lightGridOrigin[i] = w->lightGridSize[i] * ceil(wMins[i] / w->lightGridSize[i]);
		maxs[i] = w->lightGridSize[i] * floor(wMaxs[i] / w->lightGridSize[i]);
		w->lightGridBounds[i] = (maxs[i] - w->lightGridOrigin[i]) / w->lightGridSize[i] + 1;
	}

	numGridPoints = w->lightGridBounds[0] * w->lightGridBounds[1] * w->lightGridBounds[2];

	if(l->filelen != numGridPoints * 8)
	{
		ri.Printf(PRINT_WARNING, "WARNING: light grid mismatch\n");
		w->lightGridData = NULL;
		return;
	}

	w->lightGridData = ri.Hunk_Alloc(l->filelen, h_low);
	Com_Memcpy(w->lightGridData, (void *)(fileBase + l->fileofs), l->filelen);

	// deal with overbright bits
	for(i = 0; i < numGridPoints; i++)
	{
		R_ColorShiftLightingBytes(&w->lightGridData[i * 8], &w->lightGridData[i * 8]);
		R_ColorShiftLightingBytes(&w->lightGridData[i * 8 + 3], &w->lightGridData[i * 8 + 3]);
	}
}

/*
================
R_LoadEntities
================
*/
void R_LoadEntities(lump_t * l)
{
	int             i;
	char           *p, *pOld, *token, *s;
	char            keyname[MAX_TOKEN_CHARS];
	char            value[MAX_TOKEN_CHARS];
	world_t        *w;
	qboolean        isLight = qfalse;
	int             numEntities = 0;
	int             numLights = 0;
	int             numOmniLights = 0;
	int             numProjLights = 0;
	trRefLight_t   *light;

	ri.Printf(PRINT_ALL, "...loading entities\n");

	w = &s_worldData;
	w->lightGridSize[0] = 64;
	w->lightGridSize[1] = 64;
	w->lightGridSize[2] = 128;

	// store for reference by the cgame
	w->entityString = ri.Hunk_Alloc(l->filelen + 1, h_low);
	//strcpy(w->entityString, (char *)(fileBase + l->fileofs));
	Q_strncpyz(w->entityString, (char *)(fileBase + l->fileofs), l->filelen + 1);
	w->entityParsePoint = w->entityString;

#if 1
	p = w->entityString;
#else
	p = (char *)(fileBase + l->fileofs);
#endif

	// only parse the world spawn
	while(1)
	{
		// parse key
		token = Com_ParseExt(&p, qtrue);

		if(!*token)
		{
			ri.Printf(PRINT_WARNING, "WARNING: unexpected end of entities string while parsing worldspawn\n", token);
			break;
		}

		if(*token == '{')
		{
			continue;
		}

		if(*token == '}')
		{
			break;
		}

		Q_strncpyz(keyname, token, sizeof(keyname));

		// parse value
		token = Com_ParseExt(&p, qfalse);

		if(!*token)
		{
			continue;
		}

		Q_strncpyz(value, token, sizeof(value));

		// check for remapping of shaders for vertex lighting
		s = "vertexremapshader";
		if(!Q_strncmp(keyname, s, strlen(s)))
		{
			s = strchr(value, ';');
			if(!s)
			{
				ri.Printf(PRINT_WARNING, "WARNING: no semi colon in vertexshaderremap '%s'\n", value);
				break;
			}
			*s++ = 0;
			continue;
		}

		// check for remapping of shaders
		s = "remapshader";
		if(!Q_strncmp(keyname, s, strlen(s)))
		{
			s = strchr(value, ';');
			if(!s)
			{
				ri.Printf(PRINT_WARNING, "WARNING: no semi colon in shaderremap '%s'\n", value);
				break;
			}
			*s++ = 0;
			R_RemapShader(value, s, "0");
			continue;
		}

		// check for a different grid size
		if(!Q_stricmp(keyname, "gridsize"))
		{
			sscanf(value, "%f %f %f", &w->lightGridSize[0], &w->lightGridSize[1], &w->lightGridSize[2]);
			continue;
		}

		// check for ambient color
		else if(!Q_stricmp(keyname, "_color") || !Q_stricmp(keyname, "ambientColor"))
		{
			if(r_forceAmbient->value <= 0)
			{
				sscanf(value, "%f %f %f", &tr.worldEntity.ambientLight[0], &tr.worldEntity.ambientLight[1], &tr.worldEntity.ambientLight[2]);
				
				VectorCopy(tr.worldEntity.ambientLight, tr.worldEntity.ambientLight);
				VectorScale(tr.worldEntity.ambientLight, r_ambientScale->value, tr.worldEntity.ambientLight);
			}
		}

		if(!Q_stricmp(keyname, "classname") && Q_stricmp(value, "worldspawn"))
		{
			ri.Printf(PRINT_WARNING, "WARNING: expected worldspawn found '%s'\n", value);
			continue;
		}
	}

//  ri.Printf(PRINT_ALL, "-----------\n%s\n----------\n", p);

	pOld = p;
	numEntities = 1;			// parsed worldspawn so far

	// count lights
	while(1)
	{
		// parse {
		token = Com_ParseExt(&p, qtrue);

		if(!*token)
		{
			// end of entities string
			break;
		}

		if(*token != '{')
		{
			ri.Printf(PRINT_WARNING, "WARNING: expected { found '%s'\n", token);
			break;
		}

		// new entity
		isLight = qfalse;

		// parse epairs
		while(1)
		{
			// parse key
			token = Com_ParseExt(&p, qtrue);

			if(*token == '}')
			{
				break;
			}

			if(!*token)
			{
				ri.Printf(PRINT_WARNING, "WARNING: EOF without closing bracket\n");
				break;
			}

			Q_strncpyz(keyname, token, sizeof(keyname));

			// parse value
			token = Com_ParseExt(&p, qfalse);

			if(!*token)
			{
				ri.Printf(PRINT_WARNING, "WARNING: missing value for key '%s'\n", keyname);
				continue;
			}

			Q_strncpyz(value, token, sizeof(value));

			// check if this entity is a light
			if(!Q_stricmp(keyname, "classname") && !Q_stricmp(value, "light"))
			{
				isLight = qtrue;
			}
		}

		if(*token != '}')
		{
			ri.Printf(PRINT_WARNING, "WARNING: expected } found '%s'\n", token);
			break;
		}

		if(isLight)
		{
			numLights++;
		}

		numEntities++;
	}

	ri.Printf(PRINT_ALL, "%i total entities counted\n", numEntities);
	ri.Printf(PRINT_ALL, "%i total lights counted\n", numLights);

	s_worldData.numLights = numLights;
	s_worldData.lights = ri.Hunk_Alloc(s_worldData.numLights * sizeof(trRefLight_t), h_low);

	// basic light setup
	for(i = 0, light = s_worldData.lights; i < s_worldData.numLights; i++, light++)
	{
		QuatClear(light->l.rotation);

		light->l.color[0] = 1;
		light->l.color[1] = 1;
		light->l.color[2] = 1;

		light->l.radius[0] = 300;
		light->l.radius[1] = 300;
		light->l.radius[2] = 300;

		light->l.fovX = 90;
		light->l.fovY = 90;
		light->l.distance = 300;

		light->isStatic = qtrue;
		light->additive = qtrue;

		light->shadowLOD = 0;
	}

	// parse lights
	p = pOld;
	numEntities = 1;
	light = &s_worldData.lights[0];

	while(1)
	{
		// parse {
		token = Com_ParseExt(&p, qtrue);

		if(!*token)
		{
			// end of entities string
			break;
		}

		if(*token != '{')
		{
			ri.Printf(PRINT_WARNING, "WARNING: expected { found '%s'\n", token);
			break;
		}

		// new entity
		isLight = qfalse;

		// parse epairs
		while(1)
		{
			// parse key
			token = Com_ParseExt(&p, qtrue);

			if(*token == '}')
			{
				break;
			}

			if(!*token)
			{
				ri.Printf(PRINT_WARNING, "WARNING: EOF without closing bracket\n");
				break;
			}

			Q_strncpyz(keyname, token, sizeof(keyname));

			// parse value
			token = Com_ParseExt(&p, qfalse);

			if(!*token)
			{
				ri.Printf(PRINT_WARNING, "WARNING: missing value for key '%s'\n", keyname);
				continue;
			}

			Q_strncpyz(value, token, sizeof(value));

			// check if this entity is a light
			if(!Q_stricmp(keyname, "classname") && !Q_stricmp(value, "light"))
			{
				isLight = qtrue;
			}
			// check for origin
			else if(!Q_stricmp(keyname, "origin") || !Q_stricmp(keyname, "light_origin"))
			{
				sscanf(value, "%f %f %f", &light->l.origin[0], &light->l.origin[1], &light->l.origin[2]);
			}
			// check for center
			else if(!Q_stricmp(keyname, "light_center"))
			{
				sscanf(value, "%f %f %f", &light->l.center[0], &light->l.center[1], &light->l.center[2]);
			}
			// check for color
			else if(!Q_stricmp(keyname, "_color"))
			{
				sscanf(value, "%f %f %f", &light->l.color[0], &light->l.color[1], &light->l.color[2]);
			}
			// check for radius
			else if(!Q_stricmp(keyname, "light_radius"))
			{
				sscanf(value, "%f %f %f", &light->l.radius[0], &light->l.radius[1], &light->l.radius[2]);
			}
			// check for fovX
			else if(!Q_stricmp(keyname, "light_fovX"))
			{
				light->l.fovX = atof(value);
				light->l.rlType = RL_PROJ;
			}
			// check for fovY
			else if(!Q_stricmp(keyname, "light_fovY"))
			{
				light->l.fovY = atof(value);
				light->l.rlType = RL_PROJ;
			}
			// check for distance
			else if(!Q_stricmp(keyname, "light_distance"))
			{
				light->l.distance = atof(value);
				light->l.rlType = RL_PROJ;
			}
			// check for radius
			else if(!Q_stricmp(keyname, "light") || !Q_stricmp(keyname, "_light"))
			{
				vec_t           value2;

				value2 = atof(value);
				light->l.radius[0] = value2;
				light->l.radius[1] = value2;
				light->l.radius[2] = value2;
			}
			// check for light shader
			else if(!Q_stricmp(keyname, "texture"))
			{
				light->l.attenuationShader = RE_RegisterShaderLightAttenuation(value);
			}
			// check for rotation
			else if(!Q_stricmp(keyname, "rotation") || !Q_stricmp(keyname, "light_rotation"))
			{
				matrix_t        rotation;

				sscanf(value, "%f %f %f %f %f %f %f %f %f", &rotation[0], &rotation[1], &rotation[2],
					   &rotation[4], &rotation[5], &rotation[6], &rotation[8], &rotation[9], &rotation[10]);

				QuatFromMatrix(light->l.rotation, rotation);
			}
			// check if this light does not cast any shadows
			else if(!Q_stricmp(keyname, "noShadows") && !Q_stricmp(value, "1"))
			{
				light->l.noShadows = qtrue;
			}
		}

		if(*token != '}')
		{
			ri.Printf(PRINT_WARNING, "WARNING: expected } found '%s'\n", token);
			break;
		}

		if(!isLight)
		{
			// reset rotation because it may be set to the rotation of other entities
			QuatClear(light->l.rotation);
		}
		else
		{
			if((numOmniLights + numProjLights) < s_worldData.numLights);
			{
				switch (light->l.rlType)
				{
					case RL_OMNI:
						numOmniLights++;
						break;

					case RL_PROJ:
						numProjLights++;
						break;

					default:
						break;
				}

				light++;
			}
		}

		numEntities++;
	}

	if((numOmniLights + numProjLights) != s_worldData.numLights)
	{
		ri.Error(ERR_DROP, "counted %i lights and parsed %i lights", s_worldData.numLights, (numOmniLights + numProjLights));
	}

	ri.Printf(PRINT_ALL, "%i total entities parsed\n", numEntities);
	ri.Printf(PRINT_ALL, "%i total lights parsed\n", numOmniLights + numProjLights);
	ri.Printf(PRINT_ALL, "%i omni-directional lights parsed\n", numOmniLights);
	ri.Printf(PRINT_ALL, "%i projective lights parsed\n", numProjLights);
}


/*
=================
R_GetEntityToken
=================
*/
qboolean R_GetEntityToken(char *buffer, int size)
{
	const char     *s;

	s = Com_Parse(&s_worldData.entityParsePoint);
	Q_strncpyz(buffer, s, size);
	if(!s_worldData.entityParsePoint || !s[0])
	{
		s_worldData.entityParsePoint = s_worldData.entityString;
		return qfalse;
	}
	else
	{
		return qtrue;
	}
}


/*
=================
R_PrecacheInteraction
=================
*/
static void R_PrecacheInteraction(trRefLight_t * light, bspSurface_t * surface)
{
	interactionCache_t *iaCache;

	iaCache = ri.Hunk_Alloc(sizeof(*iaCache), h_low);
	Com_AddToGrowList(&s_interactions, iaCache);

	// connect to interaction grid
	if(!light->firstInteractionCache)
	{
		light->firstInteractionCache = iaCache;
	}

	if(light->lastInteractionCache)
	{
		light->lastInteractionCache->next = iaCache;
	}

	light->lastInteractionCache = iaCache;

	iaCache->next = NULL;
	iaCache->surface = surface;

	iaCache->redundant = qfalse;

	// copy triangle indices used for lighting
	if(s_numLightIndexes >= 3)
	{
		iaCache->numLightIndexes = s_numLightIndexes;
		iaCache->lightIndexes = ri.Hunk_Alloc(s_numLightIndexes * sizeof(int), h_low);
		Com_Memcpy(iaCache->lightIndexes, s_lightIndexes, s_numLightIndexes * sizeof(int));
	}
	else
	{
		iaCache->numLightIndexes = 0;
		iaCache->lightIndexes = NULL;
	}

	// copy triangle indices used for shadowing
	if(s_numShadowIndexes >= (6 + 2) * 3)
	{
		iaCache->numShadowIndexes = s_numShadowIndexes;
		iaCache->shadowIndexes = ri.Hunk_Alloc(s_numShadowIndexes * sizeof(int), h_low);
		Com_Memcpy(iaCache->shadowIndexes, s_shadowIndexes, s_numShadowIndexes * sizeof(int));
	}
	else
	{
		iaCache->numShadowIndexes = 0;
		iaCache->shadowIndexes = NULL;
	}

	// copy shadow frustum
	if(s_numShadowPlanes)
	{
		iaCache->numShadowPlanes = s_numShadowPlanes;
		iaCache->shadowPlanes = ri.Hunk_Alloc(s_numShadowPlanes * sizeof(cplane_t), h_low);
		Com_Memcpy(iaCache->shadowPlanes, s_shadowPlanes, s_numShadowPlanes * sizeof(cplane_t));
	}
	else
	{
		iaCache->numShadowPlanes = 0;
		iaCache->shadowPlanes = NULL;
	}
}

static int R_BuildShadowVolume(int numTriangles, const srfTriangle_t * triangles, int numVerts, int indexes[SHADER_MAX_INDEXES])
{
	int             i;
	int             numIndexes;
	const srfTriangle_t *tri;

	// calculate zfail shadow volume
	numIndexes = 0;

	// set up indices for silhouette edges
	for(i = 0, tri = triangles; i < numTriangles; i++, tri++)
	{
		if(!sh.facing[i])
		{
			continue;
		}

		if(tri->neighbors[0] < 0 || !sh.facing[tri->neighbors[0]])
		{
			indexes[numIndexes + 0] = tri->indexes[1];
			indexes[numIndexes + 1] = tri->indexes[0];
			indexes[numIndexes + 2] = tri->indexes[0] + numVerts;

			indexes[numIndexes + 3] = tri->indexes[1];
			indexes[numIndexes + 4] = tri->indexes[0] + numVerts;
			indexes[numIndexes + 5] = tri->indexes[1] + numVerts;

			numIndexes += 6;
		}

		if(tri->neighbors[1] < 0 || !sh.facing[tri->neighbors[1]])
		{
			indexes[numIndexes + 0] = tri->indexes[2];
			indexes[numIndexes + 1] = tri->indexes[1];
			indexes[numIndexes + 2] = tri->indexes[1] + numVerts;

			indexes[numIndexes + 3] = tri->indexes[2];
			indexes[numIndexes + 4] = tri->indexes[1] + numVerts;
			indexes[numIndexes + 5] = tri->indexes[2] + numVerts;

			numIndexes += 6;
		}

		if(tri->neighbors[2] < 0 || !sh.facing[tri->neighbors[2]])
		{
			indexes[numIndexes + 0] = tri->indexes[0];
			indexes[numIndexes + 1] = tri->indexes[2];
			indexes[numIndexes + 2] = tri->indexes[2] + numVerts;

			indexes[numIndexes + 3] = tri->indexes[0];
			indexes[numIndexes + 4] = tri->indexes[2] + numVerts;
			indexes[numIndexes + 5] = tri->indexes[0] + numVerts;

			numIndexes += 6;
		}
	}

	// set up indices for light and dark caps
	for(i = 0, tri = triangles; i < numTriangles; i++, tri++)
	{
		if(!sh.facing[i])
		{
			continue;
		}

		// light cap
		indexes[numIndexes + 0] = tri->indexes[0];
		indexes[numIndexes + 1] = tri->indexes[1];
		indexes[numIndexes + 2] = tri->indexes[2];

		// dark cap
		indexes[numIndexes + 3] = tri->indexes[2] + numVerts;
		indexes[numIndexes + 4] = tri->indexes[1] + numVerts;
		indexes[numIndexes + 5] = tri->indexes[0] + numVerts;

		numIndexes += 6;
	}

	return numIndexes;
}

static int R_BuildShadowPlanes(int numTriangles, const srfTriangle_t * triangles, int numVerts, srfVert_t * verts,
							   cplane_t shadowPlanes[SHADER_MAX_TRIANGLES], trRefLight_t * light)
{
	int             i;
	int             numShadowPlanes;
	const srfTriangle_t *tri;
	vec3_t          pos[3];

//  vec3_t          lightDir;
	vec4_t          plane;

	if(r_noShadowFrustums->integer)
	{
		return 0;
	}

	// calculate shadow frustum
	numShadowPlanes = 0;

	// set up indices for silhouette edges
	for(i = 0, tri = triangles; i < numTriangles; i++, tri++)
	{
		if(!sh.facing[i])
		{
			continue;
		}

		if(tri->neighbors[0] < 0 || !sh.facing[tri->neighbors[0]])
		{
			//indexes[numIndexes + 0] = tri->indexes[1];
			//indexes[numIndexes + 1] = tri->indexes[0];
			//indexes[numIndexes + 2] = tri->indexes[0] + numVerts;

			VectorCopy(verts[tri->indexes[1]].xyz, pos[0]);
			VectorCopy(verts[tri->indexes[0]].xyz, pos[1]);
			VectorCopy(light->origin, pos[2]);

			// extrude the infinite one
			//VectorSubtract(verts[tri->indexes[0]].xyz, light->origin, lightDir);
			//VectorAdd(verts[tri->indexes[0]].xyz, lightDir, pos[2]);
			//VectorNormalize(lightDir);
			//VectorMA(verts[tri->indexes[0]].xyz, 9999, lightDir, pos[2]);

			if(PlaneFromPoints(plane, pos[0], pos[1], pos[2], qtrue))
			{
				shadowPlanes[numShadowPlanes].normal[0] = plane[0];
				shadowPlanes[numShadowPlanes].normal[1] = plane[1];
				shadowPlanes[numShadowPlanes].normal[2] = plane[2];
				shadowPlanes[numShadowPlanes].dist = plane[3];

				numShadowPlanes++;
			}
			else
			{
				return 0;
			}
		}

		if(tri->neighbors[1] < 0 || !sh.facing[tri->neighbors[1]])
		{
			//indexes[numIndexes + 0] = tri->indexes[2];
			//indexes[numIndexes + 1] = tri->indexes[1];
			//indexes[numIndexes + 2] = tri->indexes[1] + numVerts;

			VectorCopy(verts[tri->indexes[2]].xyz, pos[0]);
			VectorCopy(verts[tri->indexes[1]].xyz, pos[1]);
			VectorCopy(light->origin, pos[2]);

			// extrude the infinite one
			//VectorSubtract(verts[tri->indexes[1]].xyz, light->origin, lightDir);
			//VectorNormalize(lightDir);
			//VectorMA(verts[tri->indexes[1]].xyz, 9999, lightDir, pos[2]);

			if(PlaneFromPoints(plane, pos[0], pos[1], pos[2], qtrue))
			{
				shadowPlanes[numShadowPlanes].normal[0] = plane[0];
				shadowPlanes[numShadowPlanes].normal[1] = plane[1];
				shadowPlanes[numShadowPlanes].normal[2] = plane[2];
				shadowPlanes[numShadowPlanes].dist = plane[3];

				numShadowPlanes++;
			}
			else
			{
				return 0;
			}
		}

		if(tri->neighbors[2] < 0 || !sh.facing[tri->neighbors[2]])
		{
			//indexes[numIndexes + 0] = tri->indexes[0];
			//indexes[numIndexes + 1] = tri->indexes[2];
			//indexes[numIndexes + 2] = tri->indexes[2] + numVerts;

			VectorCopy(verts[tri->indexes[0]].xyz, pos[0]);
			VectorCopy(verts[tri->indexes[2]].xyz, pos[1]);
			VectorCopy(light->origin, pos[2]);

			// extrude the infinite one
			//VectorSubtract(verts[tri->indexes[2]].xyz, light->origin, lightDir);
			//VectorNormalize(lightDir);
			//VectorMA(verts[tri->indexes[2]].xyz, 9999, lightDir, pos[2]);

			if(PlaneFromPoints(plane, pos[0], pos[1], pos[2], qtrue))
			{
				shadowPlanes[numShadowPlanes].normal[0] = plane[0];
				shadowPlanes[numShadowPlanes].normal[1] = plane[1];
				shadowPlanes[numShadowPlanes].normal[2] = plane[2];
				shadowPlanes[numShadowPlanes].dist = plane[3];

				numShadowPlanes++;
			}
			else
			{
				return 0;
			}
		}
	}

	// set up indices for light and dark caps
	for(i = 0, tri = triangles; i < numTriangles; i++, tri++)
	{
		if(!sh.facing[i])
		{
			continue;
		}

		// light cap
		//indexes[numIndexes + 0] = tri->indexes[0];
		//indexes[numIndexes + 1] = tri->indexes[1];
		//indexes[numIndexes + 2] = tri->indexes[2];

		VectorCopy(verts[tri->indexes[0]].xyz, pos[0]);
		VectorCopy(verts[tri->indexes[1]].xyz, pos[1]);
		VectorCopy(verts[tri->indexes[2]].xyz, pos[2]);

		if(PlaneFromPoints(plane, pos[0], pos[1], pos[2], qfalse))
		{
			shadowPlanes[numShadowPlanes].normal[0] = plane[0];
			shadowPlanes[numShadowPlanes].normal[1] = plane[1];
			shadowPlanes[numShadowPlanes].normal[2] = plane[2];
			shadowPlanes[numShadowPlanes].dist = plane[3];

			numShadowPlanes++;
		}
		else
		{
			return 0;
		}
	}


	for(i = 0; i < numShadowPlanes; i++)
	{
		//vec_t           length, ilength;

		shadowPlanes[i].type = PLANE_NON_AXIAL;
		/*
		   // normalize plane
		   length = VectorLength(shadowPlanes[i].normal);
		   if(length)
		   {
		   ilength = 1.0 / length;
		   light->frustum[i].normal[0] *= ilength;
		   light->frustum[i].normal[1] *= ilength;
		   light->frustum[i].normal[2] *= ilength;
		   light->frustum[i].dist *= ilength;
		   }
		 */

		SetPlaneSignbits(&shadowPlanes[i]);
	}

	return numShadowPlanes;
}

static int R_BuildLightIndexes(int numTriangles, const srfTriangle_t * triangles, srfVert_t * verts,
							   int indexes[SHADER_MAX_INDEXES], trRefLight_t * light)
{
	int             i;
	const srfTriangle_t *tri;
	int             numIndexes;

	// build a list of triangles that need light
	Com_Memset(&sh, 0, sizeof(shadowState_t));

	numIndexes = 0;

	for(i = 0, tri = triangles; i < numTriangles; i++, tri++)
	{
		vec3_t          pos[3];

		vec4_t          plane;
		float           d;

		// assume the triangle is ok and visible
		sh.facing[i] = qtrue;

		VectorCopy(verts[tri->indexes[0]].xyz, pos[0]);
		VectorCopy(verts[tri->indexes[1]].xyz, pos[1]);
		VectorCopy(verts[tri->indexes[2]].xyz, pos[2]);

		if(PlaneFromPoints(plane, pos[0], pos[1], pos[2], qtrue))
		{
			sh.degenerated[i] = qfalse;

			// check if light origin is behind triangle
			d = DotProduct(plane, light->origin) - plane[3];

			if(d <= 0 /* || shader->cullType == CT_BACK_SIDED */ )
			{
				sh.facing[i] = qfalse;
			}
		}
		else
		{
			sh.numDegenerated++;
			sh.degenerated[i] = qtrue;

			sh.facing[i] = qtrue;
		}

		if(R_CullLightTriangle(light, pos) == CULL_OUT)
		{
			sh.facing[i] = qfalse;
		}

		if(numIndexes >= SHADER_MAX_INDEXES)
		{
			ri.Error(ERR_DROP, "R_PrecacheFaceInteraction: indices > MAX (%d > %d)", numIndexes, SHADER_MAX_INDEXES);
		}

		// create triangle indices
		if(sh.facing[i])
		{
			indexes[numIndexes + 0] = tri->indexes[0];
			indexes[numIndexes + 1] = tri->indexes[1];
			indexes[numIndexes + 2] = tri->indexes[2];
			numIndexes += 3;

			sh.numFacing++;
		}
	}

	return numIndexes;
}

static qboolean R_PrecacheFaceInteraction(srfSurfaceFace_t * cv, shader_t * shader, trRefLight_t * light)
{
	int             numIndexes;

	// check if bounds intersect
	if(!BoundsIntersect(light->worldBounds[0], light->worldBounds[1], cv->bounds[0], cv->bounds[1]))
	{
		return qfalse;
	}

	if(r_precacheLightIndexes->integer)
	{
		numIndexes = R_BuildLightIndexes(cv->numTriangles, cv->triangles, cv->verts, s_lightIndexes, light);

		if(numIndexes == 0)
		{
			return qfalse;
		}

		s_numLightIndexes = numIndexes;

		if(r_shadows->integer == 3 && r_precacheShadowIndexes->integer)
		{
			if((sh.numFacing * (6 + 2) * 3) >= SHADER_MAX_INDEXES)
			{
				return qtrue;
			}

			s_numShadowIndexes = R_BuildShadowVolume(cv->numTriangles, cv->triangles, cv->numVerts, s_shadowIndexes);
		}

		if(r_shadows->integer >= 3)
		{
			s_numShadowPlanes =
				R_BuildShadowPlanes(cv->numTriangles, cv->triangles, cv->numVerts, cv->verts, s_shadowPlanes, light);
		}
	}

	return qtrue;
}


static int R_PrecacheGridInteraction(srfGridMesh_t * cv, shader_t * shader, trRefLight_t * light)
{
	int             numIndexes;

	// check if bounds intersect
	if(!BoundsIntersect(light->worldBounds[0], light->worldBounds[1], cv->meshBounds[0], cv->meshBounds[1]))
	{
		return qfalse;
	}

	if(r_precacheLightIndexes->integer)
	{
		numIndexes = R_BuildLightIndexes(cv->numTriangles, cv->triangles, cv->verts, s_lightIndexes, light);

		if(numIndexes == 0)
		{
			return qfalse;
		}

		s_numLightIndexes = numIndexes;

		if(r_shadows->integer == 3 && r_precacheShadowIndexes->integer)
		{
			if((sh.numFacing * (6 + 2) * 3) >= SHADER_MAX_INDEXES)
			{
				return qtrue;
			}

			s_numShadowIndexes = R_BuildShadowVolume(cv->numTriangles, cv->triangles, cv->numVerts, s_shadowIndexes);
		}

		if(r_shadows->integer >= 3)
		{
			s_numShadowPlanes =
				R_BuildShadowPlanes(cv->numTriangles, cv->triangles, cv->numVerts, cv->verts, s_shadowPlanes, light);
		}
	}

	return qtrue;
}


static int R_PrecacheTrisurfInteraction(srfTriangles_t * cv, shader_t * shader, trRefLight_t * light)
{
	int             numIndexes;

	// check if bounds intersect
	if(!BoundsIntersect(light->worldBounds[0], light->worldBounds[1], cv->bounds[0], cv->bounds[1]))
	{
		return qfalse;
	}

	if(r_precacheLightIndexes->integer)
	{
		numIndexes = R_BuildLightIndexes(cv->numTriangles, cv->triangles, cv->verts, s_lightIndexes, light);

		if(numIndexes == 0)
		{
			return qfalse;
		}

		s_numLightIndexes = numIndexes;

		if(r_shadows->integer == 3 && r_precacheShadowIndexes->integer)
		{
			if((sh.numFacing * (6 + 2) * 3) >= SHADER_MAX_INDEXES)
			{
				return qtrue;
			}

			s_numShadowIndexes = R_BuildShadowVolume(cv->numTriangles, cv->triangles, cv->numVerts, s_shadowIndexes);
		}

		if(r_shadows->integer >= 3)
		{
			s_numShadowPlanes =
				R_BuildShadowPlanes(cv->numTriangles, cv->triangles, cv->numVerts, cv->verts, s_shadowPlanes, light);
		}
	}

	return qtrue;
}


/*
======================
R_PrecacheInteractionSurface
======================
*/
static void R_PrecacheInteractionSurface(bspSurface_t * surf, trRefLight_t * light)
{
	qboolean        intersects;

	if(surf->lightCount == s_lightCount)
	{
		return;					// already checked this surface
	}
	surf->lightCount = s_lightCount;

	// skip all surfaces that don't matter for lighting only pass
	if(surf->shader->isSky || (!surf->shader->interactLight && surf->shader->noShadows))
		return;

	s_numLightIndexes = 0;
	s_numShadowIndexes = 0;
	s_numShadowPlanes = 0;

	if(*surf->data == SF_FACE)
	{
		intersects = R_PrecacheFaceInteraction((srfSurfaceFace_t *) surf->data, surf->shader, light);
	}
	else if(*surf->data == SF_GRID)
	{
		intersects = R_PrecacheGridInteraction((srfGridMesh_t *) surf->data, surf->shader, light);
	}
	else if(*surf->data == SF_TRIANGLES)
	{
		intersects = R_PrecacheTrisurfInteraction((srfTriangles_t *) surf->data, surf->shader, light);
	}
	else
	{
		intersects = qfalse;
	}

	if(intersects)
	{
		R_PrecacheInteraction(light, surf);
	}
}

/*
================
R_RecursivePrecacheInteractionNode
================
*/
static void R_RecursivePrecacheInteractionNode(bspNode_t * node, trRefLight_t * light)
{
	int             r;

	// light already hit node
	if(node->lightCount == s_lightCount)
	{
		return;
	}
	node->lightCount = s_lightCount;

	if(node->contents != -1)
	{
		// leaf node, so add mark surfaces
		int             c;
		bspSurface_t   *surf, **mark;

		// add the individual surfaces
		mark = node->firstmarksurface;
		c = node->nummarksurfaces;
		while(c--)
		{
			// the surface may have already been added if it
			// spans multiple leafs
			surf = *mark;
			R_PrecacheInteractionSurface(surf, light);
			mark++;
		}

		return;
	}

	// node is just a decision point, so go down both sides
	// since we don't care about sort orders, just go positive to negative
	r = BoxOnPlaneSide(light->worldBounds[0], light->worldBounds[1], node->plane);

	switch (r)
	{
		case 1:
			R_RecursivePrecacheInteractionNode(node->children[0], light);
			break;

		case 2:
			R_RecursivePrecacheInteractionNode(node->children[1], light);
			break;

		case 3:
		default:
			// recurse down the children, front side first
			R_RecursivePrecacheInteractionNode(node->children[0], light);
			R_RecursivePrecacheInteractionNode(node->children[1], light);
			break;
	}
}

/*
================
R_RecursiveAddInteractionNode
================
*/
static void R_RecursiveAddInteractionNode(bspNode_t * node, trRefLight_t * light, int *numLeafs, qboolean onlyCount)
{
	int             r;

	// light already hit node
	if(node->lightCount == s_lightCount)
	{
		return;
	}
	node->lightCount = s_lightCount;

	if(node->contents != -1)
	{
		vec3_t          worldBounds[2];

		VectorCopy(node->mins, worldBounds[0]);
		VectorCopy(node->maxs, worldBounds[1]);

		if(R_CullLightWorldBounds(light, worldBounds) != CULL_OUT)
		{
			if(!onlyCount)
			{
				// assign leave and increase leave counter
				light->leafs[*numLeafs] = node;
			}

			*numLeafs = *numLeafs + 1;
		}
		return;
	}

	// node is just a decision point, so go down both sides
	// since we don't care about sort orders, just go positive to negative
	r = BoxOnPlaneSide(light->worldBounds[0], light->worldBounds[1], node->plane);

	switch (r)
	{
		case 1:
			R_RecursiveAddInteractionNode(node->children[0], light, numLeafs, onlyCount);
			break;

		case 2:
			R_RecursiveAddInteractionNode(node->children[1], light, numLeafs, onlyCount);
			break;

		case 3:
		default:
			// recurse down the children, front side first
			R_RecursiveAddInteractionNode(node->children[0], light, numLeafs, onlyCount);
			R_RecursiveAddInteractionNode(node->children[1], light, numLeafs, onlyCount);
			break;
	}
}

/*
=================
R_ShadowFrustumCullWorldBounds

Returns CULL_IN, CULL_CLIP, or CULL_OUT
=================
*/
int R_ShadowFrustumCullWorldBounds(int numShadowPlanes, cplane_t * shadowPlanes, vec3_t worldBounds[2])
{
	int             i;
	cplane_t       *plane;
	qboolean        anyClip;
	int             r;

	if(!numShadowPlanes)
		return CULL_CLIP;

	// check against frustum planes
	anyClip = qfalse;
	for(i = 0; i < numShadowPlanes; i++)
	{
		plane = &shadowPlanes[i];

		r = BoxOnPlaneSide(worldBounds[0], worldBounds[1], plane);

		if(r == 2)
		{
			// completely outside frustum
			return CULL_OUT;
		}
		if(r == 3)
		{
			anyClip = qtrue;
		}
	}

	if(!anyClip)
	{
		// completely inside frustum
		return CULL_IN;
	}

	// partially clipped
	return CULL_CLIP;
}

/*
=============
R_KillRedundantInteractions
=============
*/
static void R_KillRedundantInteractions(trRefLight_t * light)
{
	interactionCache_t *iaCache, *iaCache2;
	bspSurface_t   *surface;
	vec3_t          localBounds[2];

	if(r_shadows->integer <= 2)
		return;

	if(!light->firstInteractionCache)
	{
		// this light has no interactions precached
		return;
	}

	if(light->l.noShadows)
	{
		// actually noShadows lights are quite bad concerning this optimization
		return;
	}

	for(iaCache = light->firstInteractionCache; iaCache; iaCache = iaCache->next)
	{
		surface = iaCache->surface;

		if(surface->shader->sort > SS_OPAQUE)
			continue;

		if(surface->shader->noShadows)
			continue;

		// HACK: allow fancy alphatest shadows with shadow mapping
		if(r_shadows->integer == 4 && surface->shader->alphaTest)
			continue;

		for(iaCache2 = light->firstInteractionCache; iaCache2; iaCache2 = iaCache2->next)
		{
			if(iaCache == iaCache2)
			{
				// don't check the surface of the current interaction with its shadow frustum
				continue;
			}

			surface = iaCache2->surface;

			if(*surface->data == SF_FACE)
			{
				srfSurfaceFace_t *face;

				face = (srfSurfaceFace_t *) surface->data;

				VectorCopy(face->bounds[0], localBounds[0]);
				VectorCopy(face->bounds[1], localBounds[1]);
			}
			else if(*surface->data == SF_GRID)
			{
				srfGridMesh_t  *grid;

				grid = (srfGridMesh_t *) surface->data;

				VectorCopy(grid->meshBounds[0], localBounds[0]);
				VectorCopy(grid->meshBounds[1], localBounds[1]);
			}
			else if(*surface->data == SF_TRIANGLES)
			{
				srfTriangles_t *tri;

				tri = (srfTriangles_t *) surface->data;

				VectorCopy(tri->bounds[0], localBounds[0]);
				VectorCopy(tri->bounds[1], localBounds[1]);
			}
			else
			{
				iaCache2->redundant = qfalse;
				continue;
			}

			if(R_ShadowFrustumCullWorldBounds(iaCache->numShadowPlanes, iaCache->shadowPlanes, localBounds) == CULL_IN)
			{
				iaCache2->redundant = qtrue;
				c_redundantInteractions++;
			}
		}

		if(iaCache->redundant)
		{
			c_redundantInteractions++;
		}
	}
}

/*
=================
InteractionCacheCompare
compare function for qsort()
=================
*/
static int InteractionCacheCompare(const void *a, const void *b)
{
	interactionCache_t *aa, *bb;

	aa = *(interactionCache_t **) a;
	bb = *(interactionCache_t **) b;

	// shader first
	if(aa->surface->shader < bb->surface->shader)
		return -1;

	else if(aa->surface->shader > bb->surface->shader)
		return 1;

	return 0;
}

/*
===============
R_CreateVBOLightMeshes
===============
*/
static void R_CreateVBOLightMeshes(trRefLight_t * light)
{
#if 1
	int             i, j, k, l;

	int             vertexesNum;
	byte           *data;
	int             dataSize;
	int             dataOfs;

	int             indexesNum;
	byte           *indexes;
	int             indexesSize;
	int             indexesOfs;

	interactionCache_t *iaCache, *iaCache2;
	interactionCache_t **iaCachesSorted;
	int             numCaches;

	shader_t       *shader, *oldShader;

	bspSurface_t   *surface;
	vec4_t          tmp;
	int             index;

	srfVBOLightMesh_t *lightSurf;

	if(!glConfig.vertexBufferObjectAvailable)
		return;

	if(!r_vboLighting->integer)
		return;

	if(!light->firstInteractionCache)
	{
		// this light has no interactions precached
		return;
	}

	// count number of interaction caches
	numCaches = 0;
	for(iaCache = light->firstInteractionCache; iaCache; iaCache = iaCache->next)
	{
		if(iaCache->redundant)
			continue;

		if(!iaCache->numLightIndexes)
			continue;

		surface = iaCache->surface;

		if(!surface->shader->interactLight)
			continue;

		numCaches++;
	}

	// build interaction caches list
	iaCachesSorted = ri.Hunk_AllocateTempMemory(numCaches * sizeof(iaCachesSorted[0]));

	numCaches = 0;
	for(iaCache = light->firstInteractionCache; iaCache; iaCache = iaCache->next)
	{
		if(iaCache->redundant)
			continue;

		if(!iaCache->numLightIndexes)
			continue;

		surface = iaCache->surface;

		if(!surface->shader->interactLight)
			continue;

		iaCachesSorted[numCaches] = iaCache;
		numCaches++;
	}


	// sort interaction caches by shader
	qsort(iaCachesSorted, numCaches, sizeof(iaCachesSorted), InteractionCacheCompare);

	// create a VBO for each shader
	shader = oldShader = NULL;

	for(k = 0; k < numCaches; k++)
	{
		iaCache = iaCachesSorted[k];

		shader = iaCache->surface->shader;

		if(shader != oldShader)
		{
			oldShader = shader;

			// count vertices and indices
			vertexesNum = 0;
			indexesNum = 0;

			for(l = k; l < numCaches; l++)
			{
				iaCache2 = iaCachesSorted[l];

				surface = iaCache2->surface;

				if(surface->shader != shader)
					continue;

				indexesNum += iaCache2->numLightIndexes;

				if(*surface->data == SF_FACE)
				{
					srfSurfaceFace_t *face = (srfSurfaceFace_t *) surface->data;

					if(face->numVerts)
						vertexesNum += face->numVerts;
				}
				else if(*surface->data == SF_GRID)
				{
					srfGridMesh_t  *grid = (srfGridMesh_t *) surface->data;

					if(grid->numVerts)
						vertexesNum += grid->numVerts;
				}
				else if(*surface->data == SF_TRIANGLES)
				{
					srfTriangles_t *tri = (srfTriangles_t *) surface->data;

					if(tri->numVerts)
						vertexesNum += tri->numVerts;
				}
			}

			if(!vertexesNum || !indexesNum)
				return;

			//ri.Printf(PRINT_ALL, "...calculating light mesh VBOs ( %s, %i verts %i tris )\n", shader->name, vertexesNum, indexesNum / 3);

			// create surface
			lightSurf = ri.Hunk_Alloc(sizeof(*lightSurf), h_low);
			lightSurf->surfaceType = SF_VBO_LIGHT_MESH;
			lightSurf->numIndexes = indexesNum;
			lightSurf->numVerts = vertexesNum;

			// create VBOs
			qglGenBuffersARB(1, &lightSurf->vertsVBO);
			qglGenBuffersARB(1, &lightSurf->indexesVBO);

			dataSize = vertexesNum * (sizeof(vec4_t) * 6 + sizeof(color4ub_t));
			data = ri.Hunk_AllocateTempMemory(dataSize);
			dataOfs = 0;
			vertexesNum = 0;

			indexesSize = indexesNum * sizeof(int);
			indexes = ri.Hunk_AllocateTempMemory(indexesSize);
			indexesOfs = 0;
			indexesNum = 0;

			ClearBounds(lightSurf->bounds[0], lightSurf->bounds[1]);

			// build triangle indices
			for(l = k; l < numCaches; l++)
			{
				iaCache2 = iaCachesSorted[l];

				surface = iaCache2->surface;

				if(surface->shader != shader)
					continue;

				// set up triangle indices
				for(i = 0; i < iaCache2->numLightIndexes; i++)
				{
					index = vertexesNum + iaCache2->lightIndexes[i];

					memcpy(indexes + indexesOfs, &index, sizeof(int));
					indexesOfs += sizeof(int);
				}

				if(*surface->data == SF_FACE)
				{
					srfSurfaceFace_t *face = (srfSurfaceFace_t *) surface->data;

					if(face->numVerts)
						vertexesNum += face->numVerts;
				}
				else if(*surface->data == SF_GRID)
				{
					srfGridMesh_t  *grid = (srfGridMesh_t *) surface->data;

					if(grid->numVerts)
						vertexesNum += grid->numVerts;
				}
				else if(*surface->data == SF_TRIANGLES)
				{
					srfTriangles_t *tri = (srfTriangles_t *) surface->data;

					if(tri->numVerts)
						vertexesNum += tri->numVerts;
				}

				indexesNum += iaCache2->numLightIndexes;
			}

			// feed vertex XYZ
			for(l = k; l < numCaches; l++)
			{
				iaCache2 = iaCachesSorted[l];

				surface = iaCache2->surface;

				if(surface->shader != shader)
					continue;

				if(*surface->data == SF_FACE)
				{
					srfSurfaceFace_t *cv = (srfSurfaceFace_t *) surface->data;

					if(cv->numVerts)
					{
						// set up xyz array
						for(i = 0; i < cv->numVerts; i++)
						{
							for(j = 0; j < 3; j++)
							{
								tmp[j] = cv->verts[i].xyz[j];
							}
							tmp[3] = 1;

							memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
							dataOfs += sizeof(vec4_t);

							AddPointToBounds(cv->verts[i].xyz, lightSurf->bounds[0], lightSurf->bounds[1]);
						}

						vertexesNum += cv->numVerts;
					}
				}
				else if(*surface->data == SF_GRID)
				{
					srfGridMesh_t  *cv = (srfGridMesh_t *) surface->data;

					if(cv->numVerts)
					{
						// set up xyz array
						for(i = 0; i < cv->numVerts; i++)
						{
							for(j = 0; j < 3; j++)
							{
								tmp[j] = cv->verts[i].xyz[j];
							}
							tmp[3] = 1;

							memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
							dataOfs += sizeof(vec4_t);

							AddPointToBounds(cv->verts[i].xyz, lightSurf->bounds[0], lightSurf->bounds[1]);
						}

						vertexesNum += cv->numVerts;
					}
				}
				else if(*surface->data == SF_TRIANGLES)
				{
					srfTriangles_t *cv = (srfTriangles_t *) surface->data;

					if(cv->numVerts)
					{
						// set up xyz array
						for(i = 0; i < cv->numVerts; i++)
						{
							for(j = 0; j < 3; j++)
							{
								tmp[j] = cv->verts[i].xyz[j];
							}
							tmp[3] = 1;

							memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
							dataOfs += sizeof(vec4_t);

							AddPointToBounds(cv->verts[i].xyz, lightSurf->bounds[0], lightSurf->bounds[1]);
						}

						vertexesNum += cv->numVerts;
					}
				}
			}

			// feed vertex texcoords
			lightSurf->ofsTexCoords = dataOfs;
			for(l = k; l < numCaches; l++)
			{
				iaCache2 = iaCachesSorted[l];

				surface = iaCache2->surface;

				if(surface->shader != shader)
					continue;

				if(*surface->data == SF_FACE)
				{
					srfSurfaceFace_t *cv = (srfSurfaceFace_t *) surface->data;

					if(cv->numVerts)
					{
						for(i = 0; i < cv->numVerts; i++)
						{
							for(j = 0; j < 2; j++)
							{
								tmp[j] = cv->verts[i].st[j];
							}
							tmp[2] = 0;
							tmp[3] = 1;

							memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
							dataOfs += sizeof(vec4_t);
						}
					}
				}
				else if(*surface->data == SF_GRID)
				{
					srfGridMesh_t  *cv = (srfGridMesh_t *) surface->data;

					if(cv->numVerts)
					{
						for(i = 0; i < cv->numVerts; i++)
						{
							for(j = 0; j < 2; j++)
							{
								tmp[j] = cv->verts[i].st[j];
							}
							tmp[2] = 0;
							tmp[3] = 1;

							memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
							dataOfs += sizeof(vec4_t);
						}
					}
				}
				else if(*surface->data == SF_TRIANGLES)
				{
					srfTriangles_t *cv = (srfTriangles_t *) surface->data;

					if(cv->numVerts)
					{
						for(i = 0; i < cv->numVerts; i++)
						{
							for(j = 0; j < 2; j++)
							{
								tmp[j] = cv->verts[i].st[j];
							}
							tmp[2] = 0;
							tmp[3] = 1;

							memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
							dataOfs += sizeof(vec4_t);
						}
					}
				}
			}

			// feed vertex tangents
			lightSurf->ofsTangents = dataOfs;
			for(l = k; l < numCaches; l++)
			{
				iaCache2 = iaCachesSorted[l];

				surface = iaCache2->surface;

				if(surface->shader != shader)
					continue;

				if(*surface->data == SF_FACE)
				{
					srfSurfaceFace_t *cv = (srfSurfaceFace_t *) surface->data;

					if(cv->numVerts)
					{
						for(i = 0; i < cv->numVerts; i++)
						{
							for(j = 0; j < 3; j++)
							{
								tmp[j] = cv->verts[i].tangent[j];
							}
							tmp[3] = 1;

							memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
							dataOfs += sizeof(vec4_t);
						}
					}
				}
				else if(*surface->data == SF_GRID)
				{
					srfGridMesh_t  *cv = (srfGridMesh_t *) surface->data;

					if(cv->numVerts)
					{
						for(i = 0; i < cv->numVerts; i++)
						{
							for(j = 0; j < 3; j++)
							{
								tmp[j] = cv->verts[i].tangent[j];
							}
							tmp[3] = 1;

							memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
							dataOfs += sizeof(vec4_t);
						}
					}
				}
				else if(*surface->data == SF_TRIANGLES)
				{
					srfTriangles_t *cv = (srfTriangles_t *) surface->data;

					if(cv->numVerts)
					{
						for(i = 0; i < cv->numVerts; i++)
						{
							for(j = 0; j < 3; j++)
							{
								tmp[j] = cv->verts[i].tangent[j];
							}
							tmp[3] = 1;

							memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
							dataOfs += sizeof(vec4_t);
						}
					}
				}
			}

			// feed vertex binormals
			lightSurf->ofsBinormals = dataOfs;
			for(l = k; l < numCaches; l++)
			{
				iaCache2 = iaCachesSorted[l];

				surface = iaCache2->surface;

				if(surface->shader != shader)
					continue;

				if(*surface->data == SF_FACE)
				{
					srfSurfaceFace_t *cv = (srfSurfaceFace_t *) surface->data;

					if(cv->numVerts)
					{
						for(i = 0; i < cv->numVerts; i++)
						{
							for(j = 0; j < 3; j++)
							{
								tmp[j] = cv->verts[i].binormal[j];
							}
							tmp[3] = 1;

							memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
							dataOfs += sizeof(vec4_t);
						}
					}
				}
				else if(*surface->data == SF_GRID)
				{
					srfGridMesh_t  *cv = (srfGridMesh_t *) surface->data;

					for(i = 0; i < cv->numVerts; i++)
					{
						for(j = 0; j < 3; j++)
						{
							tmp[j] = cv->verts[i].binormal[j];
						}
						tmp[3] = 1;

						memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
						dataOfs += sizeof(vec4_t);
					}
				}
				else if(*surface->data == SF_TRIANGLES)
				{
					srfTriangles_t *cv = (srfTriangles_t *) surface->data;

					for(i = 0; i < cv->numVerts; i++)
					{
						for(j = 0; j < 3; j++)
						{
							tmp[j] = cv->verts[i].binormal[j];
						}
						tmp[3] = 1;

						memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
						dataOfs += sizeof(vec4_t);
					}
				}
			}

			// feed vertex normals
			lightSurf->ofsNormals = dataOfs;
			for(l = k; l < numCaches; l++)
			{
				iaCache2 = iaCachesSorted[l];

				surface = iaCache2->surface;

				if(surface->shader != shader)
					continue;

				if(*surface->data == SF_FACE)
				{
					srfSurfaceFace_t *cv = (srfSurfaceFace_t *) surface->data;

					if(cv->numVerts)
					{
						for(i = 0; i < cv->numVerts; i++)
						{
							for(j = 0; j < 3; j++)
							{
								tmp[j] = cv->verts[i].normal[j];
							}
							tmp[3] = 1;

							memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
							dataOfs += sizeof(vec4_t);
						}
					}
				}
				else if(*surface->data == SF_GRID)
				{
					srfGridMesh_t  *cv = (srfGridMesh_t *) surface->data;

					for(i = 0; i < cv->numVerts; i++)
					{
						for(j = 0; j < 3; j++)
						{
							tmp[j] = cv->verts[i].normal[j];
						}
						tmp[3] = 1;

						memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
						dataOfs += sizeof(vec4_t);
					}
				}
				else if(*surface->data == SF_TRIANGLES)
				{
					srfTriangles_t *cv = (srfTriangles_t *) surface->data;

					for(i = 0; i < cv->numVerts; i++)
					{
						for(j = 0; j < 3; j++)
						{
							tmp[j] = cv->verts[i].normal[j];
						}
						tmp[3] = 1;

						memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
						dataOfs += sizeof(vec4_t);
					}
				}
			}

			// feed vertex colors
			lightSurf->ofsColors = dataOfs;
			for(l = k; l < numCaches; l++)
			{
				iaCache2 = iaCachesSorted[l];

				surface = iaCache2->surface;

				if(surface->shader != shader)
					continue;

				if(*surface->data == SF_FACE)
				{
					srfSurfaceFace_t *cv = (srfSurfaceFace_t *) surface->data;

					for(i = 0; i < cv->numVerts; i++)
					{
						memcpy(data + dataOfs, cv->verts[i].color, sizeof(color4ub_t));
						dataOfs += sizeof(color4ub_t);
					}
				}
				else if(*surface->data == SF_GRID)
				{
					srfGridMesh_t  *cv = (srfGridMesh_t *) surface->data;

					for(i = 0; i < cv->numVerts; i++)
					{
						memcpy(data + dataOfs, cv->verts[i].color, sizeof(color4ub_t));
						dataOfs += sizeof(color4ub_t);
					}
				}
				else if(*surface->data == SF_TRIANGLES)
				{
					srfTriangles_t *cv = (srfTriangles_t *) surface->data;

					for(i = 0; i < cv->numVerts; i++)
					{
						memcpy(data + dataOfs, cv->verts[i].color, sizeof(color4ub_t));
						dataOfs += sizeof(color4ub_t);
					}
				}
			}

			qglBindBufferARB(GL_ARRAY_BUFFER_ARB, lightSurf->vertsVBO);
			qglBufferDataARB(GL_ARRAY_BUFFER_ARB, dataSize, data, GL_STATIC_DRAW_ARB);

			qglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, lightSurf->indexesVBO);
			qglBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, indexesSize, indexes, GL_STATIC_DRAW_ARB);

			qglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
			qglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

			ri.Hunk_FreeTempMemory(indexes);
			ri.Hunk_FreeTempMemory(data);

			// add everything needed to the current iaCache
			iaCache->shader = (struct shader_s *)shader;
			iaCache->vboLightMesh = (struct srfVBOLightMesh_s *)lightSurf;

			// megs
			/*
			   ri.Printf(PRINT_ALL, "light mesh data VBO size: %d.%02d MB\n", dataSize / (1024 * 1024),
			   (dataSize % (1024 * 1024)) * 100 / (1024 * 1024));
			   ri.Printf(PRINT_ALL, "light mesh tris VBO size: %d.%02d MB\n", indexesSize / (1024 * 1024),
			   (indexesSize % (1024 * 1024)) * 100 / (1024 * 1024));
			 */
		}
	}

	ri.Hunk_FreeTempMemory(iaCachesSorted);
#endif
}

/*
===============
R_CreateVBOShadowVolume
Go through all static interactions of this light and create a new VBO shadow volume surface,
so we can render all static shadows of this light using a single glDrawElements call
without any renderer backend batching
===============
*/
static void R_CreateVBOShadowVolume(trRefLight_t * light)
{
	int             i, j;

	int             vertexesNum;
	byte           *data;
	int             dataSize;
	int             dataOfs;

	int             indexesNum;
	byte           *indexes;
	int             indexesSize;
	int             indexesOfs;

	interactionCache_t *iaCache;
	bspSurface_t   *surface;
	vec4_t          tmp;
	int             index;

	srfVBOShadowVolume_t *shadowSurf;

	if(r_shadows->integer <= 2)
		return;

	if(!glConfig.vertexBufferObjectAvailable)
		return;

	if(!r_vboShadows->integer)
		return;

	if(!light->firstInteractionCache)
	{
		// this light has no interactions precached
		return;
	}

	if(light->l.noShadows)
	{
		// actually noShadows lights are quite bad concerning this optimization
		return;
	}

	// count vertices and indices
	vertexesNum = 0;
	indexesNum = 0;

	for(iaCache = light->firstInteractionCache; iaCache; iaCache = iaCache->next)
	{
		if(iaCache->redundant)
			continue;

		surface = iaCache->surface;

		if(surface->shader->sort > SS_OPAQUE)
			continue;

		if(surface->shader->noShadows)
			continue;

		if(!iaCache->numShadowIndexes)
			continue;

		indexesNum += iaCache->numShadowIndexes;

		if(*surface->data == SF_FACE)
		{
			srfSurfaceFace_t *face = (srfSurfaceFace_t *) surface->data;

			if(face->numVerts)
				vertexesNum += face->numVerts * 2;
		}
		else if(*surface->data == SF_GRID)
		{
			srfGridMesh_t  *grid = (srfGridMesh_t *) surface->data;

			if(grid->numVerts)
				vertexesNum += grid->numVerts * 2;
		}
		else if(*surface->data == SF_TRIANGLES)
		{
			srfTriangles_t *tri = (srfTriangles_t *) surface->data;

			if(tri->numVerts)
				vertexesNum += tri->numVerts * 2;
		}
	}

	if(!vertexesNum || !indexesNum)
		return;

	//ri.Printf(PRINT_ALL, "...calculating world VBOs %i verts %i tris\n", vertexesNum, indexesNum / 3);

	// create surface
	shadowSurf = ri.Hunk_Alloc(sizeof(*shadowSurf), h_low);
	shadowSurf->surfaceType = SF_VBO_SHADOW_VOLUME;
	shadowSurf->numIndexes = indexesNum;
	shadowSurf->numVerts = vertexesNum;

	// create VBOs
	qglGenBuffersARB(1, &shadowSurf->vertsVBO);
	qglGenBuffersARB(1, &shadowSurf->indexesVBO);

	dataSize = vertexesNum * (sizeof(vec4_t) * 2);
	data = ri.Hunk_AllocateTempMemory(dataSize);
	dataOfs = 0;
	vertexesNum = 0;

	indexesSize = indexesNum * sizeof(int);
	indexes = ri.Hunk_AllocateTempMemory(indexesSize);
	indexesOfs = 0;
	indexesNum = 0;

	for(iaCache = light->firstInteractionCache; iaCache; iaCache = iaCache->next)
	{
		if(iaCache->redundant)
			continue;

		surface = iaCache->surface;

		if(surface->shader->sort > SS_OPAQUE)
			continue;

		if(surface->shader->noShadows)
			continue;

		if(!iaCache->numShadowIndexes)
			continue;

		// set the interaction to lightonly because we will render the shadows
		// using the new srfVBOShadowVolume
		iaCache->type = IA_LIGHTONLY;

		// set up triangle indices
		for(i = 0; i < iaCache->numShadowIndexes; i++)
		{
			index = vertexesNum + iaCache->shadowIndexes[i];

			memcpy(indexes + indexesOfs, &index, sizeof(int));
			indexesOfs += sizeof(int);
		}
		indexesNum += iaCache->numShadowIndexes;

		if(*surface->data == SF_FACE)
		{
			srfSurfaceFace_t *cv = (srfSurfaceFace_t *) surface->data;

			if(cv->numVerts)
			{
				// set up xyz array
				for(i = 0; i < cv->numVerts; i++)
				{
					for(j = 0; j < 3; j++)
					{
						tmp[j] = cv->verts[i].xyz[j];
					}
					tmp[3] = 1;

					memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
					dataOfs += sizeof(vec4_t);
				}

				for(i = 0; i < cv->numVerts; i++)
				{
					for(j = 0; j < 3; j++)
					{
						tmp[j] = cv->verts[i].xyz[j];
					}
					tmp[3] = 0;

					memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
					dataOfs += sizeof(vec4_t);
				}

				vertexesNum += cv->numVerts * 2;
			}
		}
		else if(*surface->data == SF_GRID)
		{
			srfGridMesh_t  *cv = (srfGridMesh_t *) surface->data;

			if(cv->numVerts)
			{
				// set up xyz array
				for(i = 0; i < cv->numVerts; i++)
				{
					for(j = 0; j < 3; j++)
					{
						tmp[j] = cv->verts[i].xyz[j];
					}
					tmp[3] = 1;

					memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
					dataOfs += sizeof(vec4_t);
				}

				for(i = 0; i < cv->numVerts; i++)
				{
					for(j = 0; j < 3; j++)
					{
						tmp[j] = cv->verts[i].xyz[j];
					}
					tmp[3] = 0;

					memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
					dataOfs += sizeof(vec4_t);
				}

				vertexesNum += cv->numVerts * 2;
			}
		}
		else if(*surface->data == SF_TRIANGLES)
		{
			srfTriangles_t *cv = (srfTriangles_t *) surface->data;

			if(cv->numVerts)
			{
				// set up xyz array
				for(i = 0; i < cv->numVerts; i++)
				{
					for(j = 0; j < 3; j++)
					{
						tmp[j] = cv->verts[i].xyz[j];
					}
					tmp[3] = 1;

					memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
					dataOfs += sizeof(vec4_t);
				}

				for(i = 0; i < cv->numVerts; i++)
				{
					for(j = 0; j < 3; j++)
					{
						tmp[j] = cv->verts[i].xyz[j];
					}
					tmp[3] = 0;

					memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
					dataOfs += sizeof(vec4_t);
				}

				vertexesNum += cv->numVerts * 2;
			}
		}
	}

	qglBindBufferARB(GL_ARRAY_BUFFER_ARB, shadowSurf->vertsVBO);
	qglBufferDataARB(GL_ARRAY_BUFFER_ARB, dataSize, data, GL_STATIC_DRAW_ARB);

	qglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, shadowSurf->indexesVBO);
	qglBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, indexesSize, indexes, GL_STATIC_DRAW_ARB);

	qglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
	qglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

	ri.Hunk_FreeTempMemory(indexes);
	ri.Hunk_FreeTempMemory(data);

	iaCache = light->firstInteractionCache;
	iaCache->vboShadowVolume = (struct srfVBOShadowVolume_s *)shadowSurf;

#if 0
	// megs
	ri.Printf(PRINT_ALL, "shadow volume data VBO size: %d.%02d MB\n", dataSize / (1024 * 1024),
			  (dataSize % (1024 * 1024)) * 100 / (1024 * 1024));
	ri.Printf(PRINT_ALL, "shadow volume tris VBO size: %d.%02d MB\n", indexesSize / (1024 * 1024),
			  (indexesSize % (1024 * 1024)) * 100 / (1024 * 1024));
#endif
}

static void CalcLightCubeSideBits(trRefLight_t * light)
{
	interactionCache_t *iaCache;
	bspSurface_t   *surface;
	vec3_t          localBounds[2];

	if(r_shadows->integer <= 2)
		return;

	if(!light->firstInteractionCache)
	{
		// this light has no interactions precached
		return;
	}

	if(light->l.noShadows)
	{
		// actually noShadows lights are quite bad concerning this optimization
		return;
	}

	if(light->l.rlType != RL_OMNI)
		return;

	if(glConfig.vertexBufferObjectAvailable && r_vboLighting->integer)
	{
		srfVBOLightMesh_t *srf;
		shader_t       *shader;

		for(iaCache = light->firstInteractionCache; iaCache; iaCache = iaCache->next)
		{
			if(iaCache->redundant)
				continue;

			if(!iaCache->vboLightMesh)
				continue;

			srf = iaCache->vboLightMesh;
			shader = iaCache->shader;

			VectorCopy(srf->bounds[0], localBounds[0]);
			VectorCopy(srf->bounds[1], localBounds[1]);

			light->shadowLOD = 0;	// important for R_CalcLightCubeSideBits
			iaCache->cubeSideBits = R_CalcLightCubeSideBits(light, localBounds);
		}
	}
	else
	{
		for(iaCache = light->firstInteractionCache; iaCache; iaCache = iaCache->next)
		{
			surface = iaCache->surface;

			if(*surface->data == SF_FACE)
			{
				srfSurfaceFace_t *face;

				face = (srfSurfaceFace_t *) surface->data;

				VectorCopy(face->bounds[0], localBounds[0]);
				VectorCopy(face->bounds[1], localBounds[1]);
			}
			else if(*surface->data == SF_GRID)
			{
				srfGridMesh_t  *grid;

				grid = (srfGridMesh_t *) surface->data;

				VectorCopy(grid->meshBounds[0], localBounds[0]);
				VectorCopy(grid->meshBounds[1], localBounds[1]);
			}
			else if(*surface->data == SF_TRIANGLES)
			{
				srfTriangles_t *tri;

				tri = (srfTriangles_t *) surface->data;

				VectorCopy(tri->bounds[0], localBounds[0]);
				VectorCopy(tri->bounds[1], localBounds[1]);
			}
			else
			{
				iaCache->cubeSideBits = CUBESIDE_CLIPALL;
				continue;
			}

			light->shadowLOD = 0;	// important for R_CalcLightCubeSideBits
			iaCache->cubeSideBits = R_CalcLightCubeSideBits(light, localBounds);
		}
	}
}

/*
=============
R_PrecacheInteractions
=============
*/
void R_PrecacheInteractions()
{
	int             i;
	trRefLight_t   *light;
	int             numLeafs;
	int             startTime, endTime;

	startTime = ri.Milliseconds();

	s_lightCount = 0;
	Com_InitGrowList(&s_interactions, 100);

	c_redundantInteractions = 0;
	ri.Printf(PRINT_ALL, "...precaching %i lights\n", s_worldData.numLights);

	for(i = 0; i < s_worldData.numLights; i++)
	{
		light = &s_worldData.lights[i];

#if 0
		ri.Printf(PRINT_ALL, "light %i: origin(%i %i %i) radius(%i %i %i) color(%f %f %f)\n",
				  i,
				  (int)light->l.origin[0], (int)light->l.origin[1], (int)light->l.origin[2],
				  (int)light->l.radius[0], (int)light->l.radius[1], (int)light->l.radius[2],
				  light->l.color[0], light->l.color[1], light->l.color[2]);
#endif

		// set up light transform matrix
		MatrixSetupTransformFromQuat(light->transformMatrix, light->l.rotation, light->l.origin);

		// set up light origin for lighting and shadowing
		R_SetupLightOrigin(light);

		// calc local bounds for culling
		R_SetupLightLocalBounds(light);

		// setup world bounds for intersection tests
		R_SetupLightWorldBounds(light);

		// setup frustum planes for intersection tests
		R_SetupLightFrustum(light);

		// set up model to light view matrix
		R_SetupLightView(light);

		// set up projection
		R_SetupLightProjection(light);

		// setup interactions
		light->firstInteractionCache = NULL;
		light->lastInteractionCache = NULL;


		// perform culling and add all the potentially visible surfaces
		s_lightCount++;
		R_RecursivePrecacheInteractionNode(s_worldData.nodes, light);

		// count number of leafs that touch this light
		s_lightCount++;
		numLeafs = 0;
		R_RecursiveAddInteractionNode(s_worldData.nodes, light, &numLeafs, qtrue);
		//ri.Printf(PRINT_ALL, "light %i touched %i leaves\n", i, numLeafs);

		// create storage room for them
		light->leafs = (struct bspNode_s **)ri.Hunk_Alloc(numLeafs * sizeof(*light->leafs), h_low);
		light->numLeafs = numLeafs;

		// fill storage with them
		s_lightCount++;
		numLeafs = 0;
		R_RecursiveAddInteractionNode(s_worldData.nodes, light, &numLeafs, qfalse);

#if 0
		// Tr3b: this can cause really bad shadow problems :/

		// check if interactions are inside shadows of other interactions
		R_KillRedundantInteractions(light);
#endif

		// create a static VBO surface for each light geometry batch
		R_CreateVBOLightMeshes(light);

		// create a static VBO surface of all shadow volumes
		R_CreateVBOShadowVolume(light);

		// calculate pyramid bits for each interaction in omni-directional lights
		CalcLightCubeSideBits(light);

	}

	// move interactions grow list to hunk
	s_worldData.numInteractions = s_interactions.currentElements;
	s_worldData.interactions = ri.Hunk_Alloc(s_worldData.numInteractions * sizeof(*s_worldData.interactions), h_low);

	for(i = 0; i < s_worldData.numInteractions; i++)
	{
		s_worldData.interactions[i] = (interactionCache_t *) Com_GrowListElement(&s_interactions, i);
	}

	Com_DestroyGrowList(&s_interactions);


	ri.Printf(PRINT_ALL, "%i interactions precached\n", s_worldData.numInteractions);
	ri.Printf(PRINT_ALL, "%i interactions were hidden in shadows\n", c_redundantInteractions);

	if(r_shadows->integer == 4)
	{
		// only interesting for omni-directional shadow mapping
		ri.Printf(PRINT_ALL, "%i omni pyramid tests\n", tr.pc.c_pyramidTests);
		ri.Printf(PRINT_ALL, "%i omni pyramid surfaces visible\n", tr.pc.c_pyramid_cull_ent_in);
		ri.Printf(PRINT_ALL, "%i omni pyramid surfaces clipped\n", tr.pc.c_pyramid_cull_ent_clip);
		ri.Printf(PRINT_ALL, "%i omni pyramid surfaces culled\n", tr.pc.c_pyramid_cull_ent_out);
	}

	endTime = ri.Milliseconds();

	ri.Printf(PRINT_ALL, "lights precaching time = %5.2f seconds\n", (endTime - startTime) / 1000.0);
}

/*
=================
RE_LoadWorldMap

Called directly from cgame
=================
*/
void RE_LoadWorldMap(const char *name)
{
	int             i;
	dheader_t      *header;
	byte           *buffer;
	byte           *startMarker;

	if(tr.worldMapLoaded)
	{
		ri.Error(ERR_DROP, "ERROR: attempted to redundantly load world map\n");
	}

	ri.Printf(PRINT_ALL, "----- RE_LoadWorldMap( %s ) -----\n", name);

	// set default sun direction to be used if it isn't
	// overridden by a shader
	tr.sunDirection[0] = 0.45f;
	tr.sunDirection[1] = 0.3f;
	tr.sunDirection[2] = 0.9f;

	VectorNormalize(tr.sunDirection);

	// set default ambient color
	tr.worldEntity.ambientLight[0] = r_forceAmbient->value;
	tr.worldEntity.ambientLight[1] = r_forceAmbient->value;
	tr.worldEntity.ambientLight[2] = r_forceAmbient->value;

	tr.worldMapLoaded = qtrue;

	// load it
	ri.FS_ReadFile(name, (void **)&buffer);
	if(!buffer)
	{
		ri.Error(ERR_DROP, "RE_LoadWorldMap: %s not found", name);
	}

	// clear tr.world so if the level fails to load, the next
	// try will not look at the partially loaded version
	tr.world = NULL;

	Com_Memset(&s_worldData, 0, sizeof(s_worldData));
	Q_strncpyz(s_worldData.name, name, sizeof(s_worldData.name));

	Q_strncpyz(s_worldData.baseName, Com_SkipPath(s_worldData.name), sizeof(s_worldData.name));
	Com_StripExtension(s_worldData.baseName, s_worldData.baseName, sizeof(s_worldData.baseName));

	startMarker = ri.Hunk_Alloc(0, h_low);

	header = (dheader_t *) buffer;
	fileBase = (byte *) header;

	i = LittleLong(header->version);
	if(i != BSP_VERSION)
	{
		ri.Error(ERR_DROP, "RE_LoadWorldMap: %s has wrong version number (%i should be %i)", name, i, BSP_VERSION);
	}

	// swap all the lumps
	for(i = 0; i < sizeof(dheader_t) / 4; i++)
	{
		((int *)header)[i] = LittleLong(((int *)header)[i]);
	}

	// load into heap
	R_LoadEntities(&header->lumps[LUMP_ENTITIES]);
	R_LoadShaders(&header->lumps[LUMP_SHADERS]);
	R_LoadPlanes(&header->lumps[LUMP_PLANES]);
	R_LoadSurfaces(&header->lumps[LUMP_SURFACES], &header->lumps[LUMP_DRAWVERTS], &header->lumps[LUMP_DRAWINDEXES]);
	R_LoadMarksurfaces(&header->lumps[LUMP_LEAFSURFACES]);
	R_LoadNodesAndLeafs(&header->lumps[LUMP_NODES], &header->lumps[LUMP_LEAFS]);
	R_LoadSubmodels(&header->lumps[LUMP_MODELS]);
	R_LoadVisibility(&header->lumps[LUMP_VISIBILITY]);
	R_LoadLightGrid(&header->lumps[LUMP_LIGHTGRID]);

	// we precache interactions between lights and surfaces
	// to reduce the polygon count
	R_PrecacheInteractions();

	s_worldData.dataSize = (byte *) ri.Hunk_Alloc(0, h_low) - startMarker;

	//ri.Printf(PRINT_ALL, "total world data size: %d.%02d MB\n", s_worldData.dataSize / (1024 * 1024),
	//        (s_worldData.dataSize % (1024 * 1024)) * 100 / (1024 * 1024));

	// only set tr.world now that we know the entire level has loaded properly
	tr.world = &s_worldData;

	ri.FS_FreeFile(buffer);
}
