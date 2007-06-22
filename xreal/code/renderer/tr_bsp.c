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
static byte    *fileBase;

int             c_culledFaceTriangles;
int             c_culledTriTriangles;
int             c_culledGridTriangles;

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
static void ParseFace(dsurface_t * ds, drawVert_t * verts, msurface_t * surf, int *indexes)
{
	int             i, j;
	srfSurfaceFace_t *cv;
	srfTriangle_t  *tri;
	int             numVerts, numTriangles;

	// get fog volume
	surf->fogIndex = LittleLong(ds->fogNum) + 1;

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
static void ParseMesh(dsurface_t * ds, drawVert_t * verts, msurface_t * surf)
{
	srfGridMesh_t  *grid;
	int             i, j;
	int             width, height, numPoints;
	MAC_STATIC srfVert_t points[MAX_PATCH_SIZE * MAX_PATCH_SIZE];
	vec3_t          bounds[2];
	vec3_t          tmpVec;
	static surfaceType_t skipData = SF_SKIP;

	// get fog volume
	surf->fogIndex = LittleLong(ds->fogNum) + 1;

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
static void ParseTriSurf(dsurface_t * ds, drawVert_t * verts, msurface_t * surf, int *indexes)
{
	srfTriangles_t *cv;
	srfTriangle_t  *tri;
	int             i, j;
	int             numVerts, numTriangles;

	// get fog volume
	surf->fogIndex = LittleLong(ds->fogNum) + 1;

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
static void ParseFlare(dsurface_t * ds, drawVert_t * verts, msurface_t * surf, int *indexes)
{
	srfFlare_t     *flare;
	int             i;

	// get fog volume
	surf->fogIndex = LittleLong(ds->fogNum) + 1;

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
R_CreateVBOs
===============
*/
static void R_CreateVBOs()
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

	msurface_t     *surface;
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

				// set up inversed colors array
				cv->ofsColorsInversed = dataOfs;
				for(i = 0; i < cv->numVerts; i++)
				{
					color4ub_t            colorInv;
					
					colorInv[0] = 255 - cv->verts[i].color[0];
					colorInv[1] = 255 - cv->verts[i].color[1];
					colorInv[2] = 255 - cv->verts[i].color[2];
					colorInv[3] = 255 - cv->verts[i].color[3];

					memcpy(data + dataOfs, colorInv, sizeof(color4ub_t));
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

				// set up inversed colors array
				cv->ofsColorsInversed = dataOfs;
				for(i = 0; i < cv->numVerts; i++)
				{
					color4ub_t            colorInv;
					
					colorInv[0] = 255 - cv->verts[i].color[0];
					colorInv[1] = 255 - cv->verts[i].color[1];
					colorInv[2] = 255 - cv->verts[i].color[2];
					colorInv[3] = 255 - cv->verts[i].color[3];

					memcpy(data + dataOfs, colorInv, sizeof(color4ub_t));
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

				// set up inversed colors array
				cv->ofsColorsInversed = dataOfs;
				for(i = 0; i < cv->numVerts; i++)
				{
					color4ub_t            colorInv;
					
					colorInv[0] = 255 - cv->verts[i].color[0];
					colorInv[1] = 255 - cv->verts[i].color[1];
					colorInv[2] = 255 - cv->verts[i].color[2];
					colorInv[3] = 255 - cv->verts[i].color[3];

					memcpy(data + dataOfs, colorInv, sizeof(color4ub_t));
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
	msurface_t     *out;
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

	R_CreateVBOs();
}



/*
=================
R_LoadSubmodels
=================
*/
static void R_LoadSubmodels(lump_t * l)
{
	dmodel_t       *in;
	bmodel_t       *out;
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
static void R_SetParent(mnode_t * node, mnode_t * parent)
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
	mnode_t        *out;
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
	msurface_t    **out;

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
=================
R_LoadFogs

=================
*/
static void R_LoadFogs(lump_t * l, lump_t * brushesLump, lump_t * sidesLump)
{
	int             i;
	fog_t          *out;
	dfog_t         *fogs;
	dbrush_t       *brushes, *brush;
	dbrushside_t   *sides;
	int             count, brushesCount, sidesCount;
	int             sideNum;
	int             planeNum;
	shader_t       *shader;
	float           d;
	int             firstSide;

	ri.Printf(PRINT_ALL, "...loading fogs\n");

	fogs = (void *)(fileBase + l->fileofs);
	if(l->filelen % sizeof(*fogs))
	{
		ri.Error(ERR_DROP, "LoadMap: funny lump size in %s", s_worldData.name);
	}
	count = l->filelen / sizeof(*fogs);

	// create fog strucutres for them
	s_worldData.numfogs = count + 1;
	s_worldData.fogs = ri.Hunk_Alloc(s_worldData.numfogs * sizeof(*out), h_low);
	out = s_worldData.fogs + 1;

	if(!count)
	{
		return;
	}

	brushes = (void *)(fileBase + brushesLump->fileofs);
	if(brushesLump->filelen % sizeof(*brushes))
	{
		ri.Error(ERR_DROP, "LoadMap: funny lump size in %s", s_worldData.name);
	}
	brushesCount = brushesLump->filelen / sizeof(*brushes);

	sides = (void *)(fileBase + sidesLump->fileofs);
	if(sidesLump->filelen % sizeof(*sides))
	{
		ri.Error(ERR_DROP, "LoadMap: funny lump size in %s", s_worldData.name);
	}
	sidesCount = sidesLump->filelen / sizeof(*sides);

	for(i = 0; i < count; i++, fogs++)
	{
		out->originalBrushNumber = LittleLong(fogs->brushNum);

		if((unsigned)out->originalBrushNumber >= brushesCount)
		{
			ri.Error(ERR_DROP, "fog brushNumber out of range");
		}
		brush = brushes + out->originalBrushNumber;

		firstSide = LittleLong(brush->firstSide);

		if((unsigned)firstSide > sidesCount - 6)
		{
			ri.Error(ERR_DROP, "fog brush sideNumber out of range");
		}

		// brushes are always sorted with the axial sides first
		sideNum = firstSide + 0;
		planeNum = LittleLong(sides[sideNum].planeNum);
		out->bounds[0][0] = -s_worldData.planes[planeNum].dist;

		sideNum = firstSide + 1;
		planeNum = LittleLong(sides[sideNum].planeNum);
		out->bounds[1][0] = s_worldData.planes[planeNum].dist;

		sideNum = firstSide + 2;
		planeNum = LittleLong(sides[sideNum].planeNum);
		out->bounds[0][1] = -s_worldData.planes[planeNum].dist;

		sideNum = firstSide + 3;
		planeNum = LittleLong(sides[sideNum].planeNum);
		out->bounds[1][1] = s_worldData.planes[planeNum].dist;

		sideNum = firstSide + 4;
		planeNum = LittleLong(sides[sideNum].planeNum);
		out->bounds[0][2] = -s_worldData.planes[planeNum].dist;

		sideNum = firstSide + 5;
		planeNum = LittleLong(sides[sideNum].planeNum);
		out->bounds[1][2] = s_worldData.planes[planeNum].dist;

		// get information from the shader for fog parameters
		shader = R_FindShader(fogs->shader, SHADER_3D_DYNAMIC, qtrue);

		out->parms = shader->fogParms;

		out->colorInt = ColorBytes4(shader->fogParms.color[0] * tr.identityLight,
									shader->fogParms.color[1] * tr.identityLight,
									shader->fogParms.color[2] * tr.identityLight, 1.0);

		d = shader->fogParms.depthForOpaque < 1 ? 1 : shader->fogParms.depthForOpaque;
		out->tcScale = 1.0f / (d * 8);

		// set the gradient vector
		sideNum = LittleLong(fogs->visibleSide);

		if(sideNum == -1)
		{
			out->hasSurface = qfalse;
		}
		else
		{
			out->hasSurface = qtrue;
			planeNum = LittleLong(sides[firstSide + sideNum].planeNum);
			VectorSubtract(vec3_origin, s_worldData.planes[planeNum].normal, out->surface);
			out->surface[3] = -s_worldData.planes[planeNum].dist;
		}

		out++;
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
	strcpy(w->entityString, (char *)(fileBase + l->fileofs));
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

		if(!Q_stricmp(keyname, "classname") && Q_stricmp(value, "worldspawn"))
		{
			ri.Printf(PRINT_WARNING, "WARNING: expected worldspawn found '%s'\n", value);
			continue;
		}
	}

//  ri.Printf(PRINT_ALL, "-----------\n%s\n----------\n", p);

#if 1
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
				continue;
			}

			Q_strncpyz(value, token, sizeof(value));

			// check if this entity is a light
			if(!Q_stricmp(keyname, "classname") && !Q_stricmp(value, "light"))
			{
				isLight = qtrue;
				continue;
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
		light->l.color[0] = 1;
		light->l.color[1] = 1;
		light->l.color[2] = 1;

		light->l.radius[0] = 300;
		light->l.radius[1] = 300;
		light->l.radius[2] = 300;
		
		light->l.fovX = 90;
		light->l.fovY = 90;
		light->l.distance = 300;

		AxisClear(light->l.axis);
		light->l.nonNormalizedAxes = qfalse;

		light->isStatic = qtrue;
		light->additive = qtrue;
	}
#endif

#if 1
	// parse lights
	p = pOld;
	numEntities = 1;
	light = s_worldData.lights;

	while(1)
	{
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
				
				MatrixToVectorsFLU(rotation, light->l.axis[0], light->l.axis[1], light->l.axis[2]);
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

		if(isLight)
		{
			if((numOmniLights + numProjLights) < s_worldData.numLights);
			{
				light++;

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
			}
		}

		numEntities++;
	}
#endif

	/*
	if((numOmniLights + numProjLights) != s_worldData.numLights)
	{
		ri.Error(ERR_DROP, "counted %i lights and parsed %i lights", s_worldData.numLights, (numOmniLights + numProjLights));
	}
	*/

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
static void R_PrecacheInteraction(trRefLight_t * light, msurface_t * surface)
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
}

/*
Triangle-box collider by Alen Ladavac and Vedran Klanac.
Ported to ODE by Oskari Nyman.
Ported to Q3A by Robert Beckebans.
*/

// global collider data
static vec3_t   vBestNormal;
static vec_t    fBestDepth;
static int      iBestAxis = 0;
static int      iExitAxis = 0;
static vec3_t   vE0, vE1, vE2, vN;

// test normal of mesh face as separating axis for intersection
static qboolean _cldTestNormal(vec_t fp0, vec_t fR, vec3_t vNormal, int iAxis)
{
	// calculate overlapping interval of box and triangle
	vec_t           fDepth = fR + fp0;
	vec_t           fLength;

	// if we do not overlap
	if(fDepth < 0)
	{
		// do nothing
		return qfalse;
	}

	// calculate normal's length
	fLength = VectorLength(vNormal);

	// if long enough
	if(fLength > 0.0f)
	{

		vec_t           fOneOverLength = 1.0f / fLength;

		// normalize depth
		fDepth = fDepth * fOneOverLength;

		// get minimum depth
		if(fDepth < fBestDepth)
		{
			vBestNormal[0] = -vNormal[0] * fOneOverLength;
			vBestNormal[1] = -vNormal[1] * fOneOverLength;
			vBestNormal[2] = -vNormal[2] * fOneOverLength;
			iBestAxis = iAxis;
			//dAASSERT(fDepth>=0);
			fBestDepth = fDepth;
		}

	}

	return qtrue;
}

// test box axis as separating axis
static qboolean _cldTestFace(vec_t fp0, vec_t fp1, vec_t fp2, vec_t fR, vec_t fD, vec3_t vNormal, int iAxis)
{
	vec_t           fMin, fMax;
	vec_t           fDepthMin;
	vec_t           fDepthMax;
	vec_t           fDepth;

	// find min of triangle interval 
	if(fp0 < fp1)
	{
		if(fp0 < fp2)
		{
			fMin = fp0;
		}
		else
		{
			fMin = fp2;
		}
	}
	else
	{
		if(fp1 < fp2)
		{
			fMin = fp1;
		}
		else
		{
			fMin = fp2;
		}
	}

	// find max of triangle interval 
	if(fp0 > fp1)
	{
		if(fp0 > fp2)
		{
			fMax = fp0;
		}
		else
		{
			fMax = fp2;
		}
	}
	else
	{
		if(fp1 > fp2)
		{
			fMax = fp1;
		}
		else
		{
			fMax = fp2;
		}
	}

	// calculate minimum and maximum depth
	fDepthMin = fR - fMin;
	fDepthMax = fMax + fR;

	// if we dont't have overlapping interval
	if(fDepthMin < 0 || fDepthMax < 0)
	{
		// do nothing
		return qfalse;
	}

	fDepth = 0;

	// if greater depth is on negative side 
	if(fDepthMin > fDepthMax)
	{
		// use smaller depth (one from positive side)
		fDepth = fDepthMax;
		// flip normal direction
		vNormal[0] = -vNormal[0];
		vNormal[1] = -vNormal[1];
		vNormal[2] = -vNormal[2];
		fD = -fD;
		// if greater depth is on positive side 
	}
	else
	{
		// use smaller depth (one from negative side)
		fDepth = fDepthMin;
	}


	// if lower depth than best found so far 
	if(fDepth < fBestDepth)
	{
		// remember current axis as best axis
		vBestNormal[0] = vNormal[0];
		vBestNormal[1] = vNormal[1];
		vBestNormal[2] = vNormal[2];
		iBestAxis = iAxis;
		//dAASSERT(fDepth>=0);
		fBestDepth = fDepth;
	}

	return qtrue;
}

// test cross products of box axis and triangle edges as separating axis
static qboolean _cldTestEdge(vec_t fp0, vec_t fp1, vec_t fR, vec_t fD, vec3_t vNormal, int iAxis)
{
	vec_t           fMin, fMax;
	vec_t           fDepthMin;
	vec_t           fDepthMax;
	vec_t           fDepth;
	vec_t           fLength;

	// calculate min and max interval values  
	if(fp0 < fp1)
	{
		fMin = fp0;
		fMax = fp1;
	}
	else
	{
		fMin = fp1;
		fMax = fp0;
	}

	// check if we overlapp
	fDepthMin = fR - fMin;
	fDepthMax = fMax + fR;

	// if we don't overlapp
	if(fDepthMin < 0 || fDepthMax < 0)
	{
		// do nothing
		return qfalse;
	}

	// if greater depth is on negative side 
	if(fDepthMin > fDepthMax)
	{
		// use smaller depth (one from positive side)
		fDepth = fDepthMax;
		// flip normal direction
		vNormal[0] = -vNormal[0];
		vNormal[1] = -vNormal[1];
		vNormal[2] = -vNormal[2];
		fD = -fD;
		// if greater depth is on positive side 
	}
	else
	{
		// use smaller depth (one from negative side)
		fDepth = fDepthMin;
	}

	// calculate normal's length
	fLength = VectorLength(vNormal);

	// if long enough
	if(fLength > 0.0f)
	{
		// normalize depth
		vec_t           fOneOverLength = 1.0f / fLength;

		fDepth = fDepth * fOneOverLength;
		fD *= fOneOverLength;

		// if lower depth than best found so far (favor face over edges)
		if(fDepth * 1.5f < fBestDepth)
		{
			// remember current axis as best axis
			vBestNormal[0] = vNormal[0] * fOneOverLength;
			vBestNormal[1] = vNormal[1] * fOneOverLength;
			vBestNormal[2] = vNormal[2] * fOneOverLength;
			iBestAxis = iAxis;
			//dAASSERT(fDepth>=0);
			fBestDepth = fDepth;
		}
	}

	return qtrue;
}

static qboolean _cldTestSeparatingAxes(trRefLight_t * light, const vec3_t v0, const vec3_t v1, const vec3_t v2)
{
	vec3_t          vA0, vA1, vA2;
	vec_t           fa0, fa1, fa2;
	vec3_t          vD;
	vec_t           fNLen;
	vec3_t          vL;
	vec_t           fp0, fp1, fp2, fR, fD;

	// reset best axis
	iBestAxis = 0;
	iExitAxis = -1;
	fBestDepth = FLT_MAX;

	// calculate edges
	VectorSubtract(v1, v0, vE0);
	VectorSubtract(v2, v0, vE1);
	VectorSubtract(vE1, vE0, vE2);

	// calculate poly normal
	CrossProduct(vE0, vE1, vN);

	// extract box axes as vectors
	VectorCopy(light->l.axis[0], vA0);
	VectorCopy(light->l.axis[1], vA1);
	VectorCopy(light->l.axis[2], vA2);

	// box halfsizes
	fa0 = light->l.radius[0];
	fa1 = light->l.radius[1];
	fa2 = light->l.radius[2];

	// calculate relative position between box and triangle
	VectorSubtract(v0, light->l.origin, vD);

	// calculate length of face normal
	fNLen = VectorLength(vN);

	//
	// test separating axes for intersection
	//

	// Axis 1 - Triangle Normal
	VectorCopy(vN, vL);
	fp0 = DotProduct(vL, vD);
	fp1 = fp0;
	fp2 = fp0;
	fR = fa0 * Q_fabs(DotProduct(vN, vA0)) + fa1 * Q_fabs(DotProduct(vN, vA1)) + fa2 * Q_fabs(DotProduct(vN, vA2));

	if(!_cldTestNormal(fp0, fR, vL, 1))
	{
		iExitAxis = 1;
		return qfalse;
	}

	//
	// test faces
	//

	// Axis 2 - Box X-Axis
	VectorCopy(vA0, vL);
	fD = DotProduct(vL, vN) / fNLen;
	fp0 = DotProduct(vL, vD);
	fp1 = fp0 + DotProduct(vA0, vE0);
	fp2 = fp0 + DotProduct(vA0, vE1);
	fR = fa0;

	if(!_cldTestFace(fp0, fp1, fp2, fR, fD, vL, 2))
	{
		iExitAxis = 2;
		return qfalse;
	}

	// Axis 3 - Box Y-Axis
	VectorCopy(vA1, vL);
	fD = DotProduct(vL, vN) / fNLen;
	fp0 = DotProduct(vL, vD);
	fp1 = fp0 + DotProduct(vA1, vE0);
	fp2 = fp0 + DotProduct(vA1, vE1);
	fR = fa1;

	if(!_cldTestFace(fp0, fp1, fp2, fR, fD, vL, 3))
	{
		iExitAxis = 3;
		return qfalse;
	}

	// Axis 4 - Box Z-Axis
	VectorCopy(vA2, vL);
	fD = DotProduct(vL, vN) / fNLen;
	fp0 = DotProduct(vL, vD);
	fp1 = fp0 + DotProduct(vA2, vE0);
	fp2 = fp0 + DotProduct(vA2, vE1);
	fR = fa2;

	if(!_cldTestFace(fp0, fp1, fp2, fR, fD, vL, 4))
	{
		iExitAxis = 4;
		return qfalse;
	}

	//
	// test edges
	//

	// Axis 5 - Box X-Axis cross Edge0
	CrossProduct(vA0, vE0, vL);
	fD = DotProduct(vL, vN) / fNLen;
	fp0 = DotProduct(vL, vD);
	fp1 = fp0;
	fp2 = fp0 + DotProduct(vA0, vN);
	fR = fa1 * Q_fabs(DotProduct(vA2, vE0)) + fa2 * Q_fabs(DotProduct(vA1, vE0));

	if(!_cldTestEdge(fp1, fp2, fR, fD, vL, 5))
	{
		iExitAxis = 5;
		return qfalse;
	}

	// Axis 6 - Box X-Axis cross Edge1
	CrossProduct(vA0, vE1, vL);
	fD = DotProduct(vL, vN) / fNLen;
	fp0 = DotProduct(vL, vD);
	fp1 = fp0 - DotProduct(vA0, vN);
	fp2 = fp0;
	fR = fa1 * Q_fabs(DotProduct(vA2, vE1)) + fa2 * Q_fabs(DotProduct(vA1, vE1));

	if(!_cldTestEdge(fp0, fp1, fR, fD, vL, 6))
	{
		iExitAxis = 6;
		return qfalse;
	}

	// Axis 7 - Box X-Axis cross Edge2
	CrossProduct(vA0, vE2, vL);
	fD = DotProduct(vL, vN) / fNLen;
	fp0 = DotProduct(vL, vD);
	fp1 = fp0 - DotProduct(vA0, vN);
	fp2 = fp0 - DotProduct(vA0, vN);
	fR = fa1 * Q_fabs(DotProduct(vA2, vE2)) + fa2 * Q_fabs(DotProduct(vA1, vE2));

	if(!_cldTestEdge(fp0, fp1, fR, fD, vL, 7))
	{
		iExitAxis = 7;
		return qfalse;
	}

	// Axis 8 - Box Y-Axis cross Edge0
	CrossProduct(vA1, vE0, vL);
	fD = DotProduct(vL, vN) / fNLen;
	fp0 = DotProduct(vL, vD);
	fp1 = fp0;
	fp2 = fp0 + DotProduct(vA1, vN);
	fR = fa0 * Q_fabs(DotProduct(vA2, vE0)) + fa2 * Q_fabs(DotProduct(vA0, vE0));

	if(!_cldTestEdge(fp0, fp2, fR, fD, vL, 8))
	{
		iExitAxis = 8;
		return qfalse;
	}

	// Axis 9 - Box Y-Axis cross Edge1
	CrossProduct(vA1, vE1, vL);
	fD = DotProduct(vL, vN) / fNLen;
	fp0 = DotProduct(vL, vD);
	fp1 = fp0 - DotProduct(vA1, vN);
	fp2 = fp0;
	fR = fa0 * Q_fabs(DotProduct(vA2, vE1)) + fa2 * Q_fabs(DotProduct(vA0, vE1));

	if(!_cldTestEdge(fp0, fp1, fR, fD, vL, 9))
	{
		iExitAxis = 9;
		return qfalse;
	}

	// Axis 10 - Box Y-Axis cross Edge2
	CrossProduct(vA1, vE2, vL);
	fD = DotProduct(vL, vN) / fNLen;
	fp0 = DotProduct(vL, vD);
	fp1 = fp0 - DotProduct(vA1, vN);
	fp2 = fp0 - DotProduct(vA1, vN);
	fR = fa0 * Q_fabs(DotProduct(vA2, vE2)) + fa2 * Q_fabs(DotProduct(vA0, vE2));

	if(!_cldTestEdge(fp0, fp1, fR, fD, vL, 10))
	{
		iExitAxis = 10;
		return qfalse;
	}

	// Axis 11 - Box Z-Axis cross Edge0
	CrossProduct(vA2, vE0, vL);
	fD = DotProduct(vL, vN) / fNLen;
	fp0 = DotProduct(vL, vD);
	fp1 = fp0;
	fp2 = fp0 + DotProduct(vA2, vN);
	fR = fa0 * Q_fabs(DotProduct(vA1, vE0)) + fa1 * Q_fabs(DotProduct(vA0, vE0));

	if(!_cldTestEdge(fp0, fp2, fR, fD, vL, 11))
	{
		iExitAxis = 11;
		return qfalse;
	}

	// Axis 12 - Box Z-Axis cross Edge1
	CrossProduct(vA2, vE1, vL);
	fD = DotProduct(vL, vN) / fNLen;
	fp0 = DotProduct(vL, vD);
	fp1 = fp0 - DotProduct(vA2, vN);
	fp2 = fp0;
	fR = fa0 * Q_fabs(DotProduct(vA1, vE1)) + fa1 * Q_fabs(DotProduct(vA0, vE1));

	if(!_cldTestEdge(fp0, fp1, fR, fD, vL, 12))
	{
		iExitAxis = 12;
		return qfalse;
	}

	// Axis 13 - Box Z-Axis cross Edge2
	CrossProduct(vA2, vE2, vL);
	fD = DotProduct(vL, vN) / fNLen;
	fp0 = DotProduct(vL, vD);
	fp1 = fp0 - DotProduct(vA2, vN);
	fp2 = fp0 - DotProduct(vA2, vN);
	fR = fa0 * Q_fabs(DotProduct(vA1, vE2)) + fa1 * Q_fabs(DotProduct(vA0, vE2));

	if(!_cldTestEdge(fp0, fp1, fR, fD, vL, 13))
	{
		iExitAxis = 13;
		return qfalse;
	}

	return qtrue;
}

// test one mesh triangle on intersection with given box
static qboolean _cldTestOneTriangle(trRefLight_t * light, const vec3_t v0, const vec3_t v1, const vec3_t v2)
{
	// do intersection test and find best separating axis
	if(!_cldTestSeparatingAxes(light, v0, v1, v2))
	{
		// if not found do nothing
		return qfalse;
	}

	return qtrue;

	/*
	   // if best separation axis is not found
	   if ( iBestAxis == 0 ) {
	   // this should not happen (we should already exit in that case)
	   //dMessage (0, "best separation axis not found");
	   // do nothing
	   return;
	   }

	   _cldClipping(v0, v1, v2);
	 */
}

static int R_BuildShadowVolume(int numTriangles, const srfTriangle_t * triangles, int numVerts, int indexes[SHADER_MAX_INDEXES])
{
	int             i;
	int				numIndexes;
	const srfTriangle_t  *tri;

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

static qboolean R_PrecacheFaceInteraction(srfSurfaceFace_t * cv, shader_t * shader, trRefLight_t * light)
{
	int             i;
	srfTriangle_t  *tri;
	int             numIndexes;
	int            *indexes;

	// check if bounds intersect
	if(!BoundsIntersect(light->worldBounds[0], light->worldBounds[1], cv->bounds[0], cv->bounds[1]))
	{
		return qfalse;
	}

	if(r_precacheLightIndexes->integer)
	{
		// build a list of triangles that need light
		Com_Memset(&sh, 0, sizeof(shadowState_t));

		numIndexes = 0;
		indexes = s_lightIndexes;

		for(i = 0, tri = cv->triangles; i < cv->numTriangles; i++, tri++)
		{
			vec3_t          verts[3];

			vec4_t          plane;
			float           d;

			// assume the triangle is ok and visible
			sh.facing[i] = qtrue;

			VectorCopy(cv->verts[tri->indexes[0]].xyz, verts[0]);
			VectorCopy(cv->verts[tri->indexes[1]].xyz, verts[1]);
			VectorCopy(cv->verts[tri->indexes[2]].xyz, verts[2]);

			/*
			   VectorSubtract(verts[1], verts[0], d1);
			   VectorSubtract(verts[2], verts[0], d2);

			   CrossProduct(d1, d2, plane);
			   plane[3] = DotProduct(plane, verts[0]);

			   d = DotProduct(plane, light->origin) - plane[3];
			   if(d > 0)
			   {
			   sh.facing[i] = qtrue;
			   }
			   else
			   {
			   sh.facing[i] = qfalse;
			   }
			 */

			if(PlaneFromPoints(plane, verts[0], verts[1], verts[2], qtrue))
			{
				sh.degenerated[i] = qfalse;
				
				// check if light origin is behind triangle
				d = DotProduct(plane, light->origin) - plane[3];

				if(d <= 0 || shader->cullType == CT_BACK_SIDED)
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

			// check with ODE's triangle<->OBB collider for an intersection
			if(light->l.rlType == RL_OMNI && !sh.degenerated && !_cldTestOneTriangle(light, verts[0], verts[1], verts[2]))
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
			else
			{
				c_culledFaceTriangles++;
			}
		}

		if(numIndexes == 0)
		{
			return qfalse;
		}

		s_numLightIndexes = numIndexes;

		if(r_shadows->integer == 3 && r_precacheShadowIndexes->integer)
		{
			if(r_shadows->integer != 3 || (sh.numFacing * (6 + 2) * 3) >= SHADER_MAX_INDEXES)
			{
				return qtrue;
			}

			s_numShadowIndexes = R_BuildShadowVolume(cv->numTriangles, cv->triangles, cv->numVerts, s_shadowIndexes);
		}

	}

	return qtrue;
}


static int R_PrecacheGridInteraction(srfGridMesh_t * cv, shader_t * shader, trRefLight_t * light)
{
	int             i;
	srfTriangle_t  *tri;
	int             numIndexes;
	int            *indexes;

	// check if bounds intersect
	if(!BoundsIntersect(light->worldBounds[0], light->worldBounds[1], cv->meshBounds[0], cv->meshBounds[1]))
	{
		return qfalse;
	}

	if(r_precacheLightIndexes->integer)
	{
		// build a list of triangles that need light
		Com_Memset(&sh, 0, sizeof(shadowState_t));

		numIndexes = 0;
		indexes = s_lightIndexes;

		for(i = 0, tri = cv->triangles; i < cv->numTriangles; i++, tri++)
		{
			vec3_t          verts[3];
			vec4_t          plane;
			float           d;

			sh.facing[i] = qtrue;

			VectorCopy(cv->verts[tri->indexes[0]].xyz, verts[0]);
			VectorCopy(cv->verts[tri->indexes[1]].xyz, verts[1]);
			VectorCopy(cv->verts[tri->indexes[2]].xyz, verts[2]);

			if(PlaneFromPoints(plane, verts[0], verts[1], verts[2], qtrue))
			{
				sh.degenerated[i] = qfalse;
				
				// check if light origin is behind triangle
				d = DotProduct(plane, light->origin) - plane[3];

				if(d <= 0 || shader->cullType == CT_BACK_SIDED)
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

			// check with ODE's triangle<->OBB collider for an intersection
			if(light->l.rlType == RL_OMNI && !sh.degenerated && !_cldTestOneTriangle(light, verts[0], verts[1], verts[2]))
			{
				sh.facing[i] = qfalse;
			}

			if(numIndexes >= SHADER_MAX_INDEXES)
			{
				ri.Error(ERR_DROP, "R_PrecacheGridInteraction: indices > MAX (%d > %d)", numIndexes, SHADER_MAX_INDEXES);
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
			else
			{
				c_culledGridTriangles++;
			}
		}

		if(numIndexes == 0)
		{
			return qfalse;
		}

		s_numLightIndexes = numIndexes;

		if(r_shadows->integer == 3 && r_precacheShadowIndexes->integer)
		{
			if(r_shadows->integer != 3 || (sh.numFacing * (6 + 2) * 3) >= SHADER_MAX_INDEXES)
			{
				return qtrue;
			}

			s_numShadowIndexes = R_BuildShadowVolume(cv->numTriangles, cv->triangles, cv->numVerts, s_shadowIndexes);
		}
	}

	return qtrue;
}


static int R_PrecacheTrisurfInteraction(srfTriangles_t * cv, shader_t * shader, trRefLight_t * light)
{
	int             i;
	srfTriangle_t  *tri;
	int             numIndexes;
	int            *indexes;

	// check if bounds intersect
	if(!BoundsIntersect(light->worldBounds[0], light->worldBounds[1], cv->bounds[0], cv->bounds[1]))
	{
		return qfalse;
	}

	if(r_precacheLightIndexes->integer)
	{
		// build a list of triangles that need light
		Com_Memset(&sh, 0, sizeof(shadowState_t));

		numIndexes = 0;
		indexes = s_lightIndexes;

		for(i = 0, tri = cv->triangles; i < cv->numTriangles; i++, tri++)
		{
			vec3_t          verts[3];
			vec4_t          plane;
			float           d;

			sh.facing[i] = qtrue;

			VectorCopy(cv->verts[tri->indexes[0]].xyz, verts[0]);
			VectorCopy(cv->verts[tri->indexes[1]].xyz, verts[1]);
			VectorCopy(cv->verts[tri->indexes[2]].xyz, verts[2]);

			if(PlaneFromPoints(plane, verts[0], verts[1], verts[2], qtrue))
			{
				sh.degenerated[i] = qfalse;
				
				// check if light origin is behind triangle
				d = DotProduct(plane, light->origin) - plane[3];

				if(d <= 0 || shader->cullType == CT_BACK_SIDED)
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

			// check with ODE's triangle<->OBB collider for an intersection
			if(light->l.rlType == RL_OMNI && !sh.degenerated && !_cldTestOneTriangle(light, verts[0], verts[1], verts[2]))
			{
				sh.facing[i] = qfalse;
			}

			if(numIndexes >= SHADER_MAX_INDEXES)
			{
				ri.Error(ERR_DROP, "R_PrecacheTrisurfInteraction: indices > MAX (%d > %d)", numIndexes, SHADER_MAX_INDEXES);
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
			else
			{
				c_culledTriTriangles++;
			}
		}

		if(numIndexes == 0)
		{
			return qfalse;
		}

		s_numLightIndexes = numIndexes;

		if(r_shadows->integer == 3 && r_precacheShadowIndexes->integer)
		{
			if(r_shadows->integer != 3 || (sh.numFacing * (6 + 2) * 3) >= SHADER_MAX_INDEXES)
			{
				return qtrue;
			}

			s_numShadowIndexes = R_BuildShadowVolume(cv->numTriangles, cv->triangles, cv->numVerts, s_shadowIndexes);
		}
	}

	return qtrue;
}


/*
======================
R_PrecacheInteractionSurface
======================
*/
static void R_PrecacheInteractionSurface(msurface_t * surf, trRefLight_t * light)
{
	qboolean        intersects;

	if(surf->lightCount == s_lightCount)
	{
		return;					// already checked this surface
	}
	surf->lightCount = s_lightCount;

	// skip all surfaces that don't matter for lighting only pass
	if(surf->shader->surfaceFlags & (SURF_NODLIGHT | SURF_SKY))
		return;

	s_numLightIndexes = 0;
	s_numShadowIndexes = 0;

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
static void R_RecursivePrecacheInteractionNode(mnode_t * node, trRefLight_t * light)
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
		msurface_t     *surf, **mark;

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
static void R_RecursiveAddInteractionNode(mnode_t * node, trRefLight_t * light, int *numLeafs, qboolean onlyCount)
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
		if(!onlyCount)
		{
			// assign leave and increase leave counter
			light->leafs[*numLeafs] = node;
		}

		*numLeafs = *numLeafs + 1;
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
=============
R_PrecacheInteractions
=============
*/
void R_PrecacheInteractions()
{
	int             i;
	trRefLight_t   *light;
	int             numLeafs;
	interactionCache_t *iaCache;
	msurface_t     *surface;
	vec3_t          localBounds[2];
	int             startTime, endTime;
		
	startTime = ri.Milliseconds();

	s_lightCount = 0;
	Com_InitGrowList(&s_interactions, 100);

	c_culledFaceTriangles = 0;
	c_culledGridTriangles = 0;
	c_culledTriTriangles = 0;

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
		MatrixSetupTransform(light->transformMatrix, light->l.axis[0], light->l.axis[1], light->l.axis[2], light->l.origin);

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

		/*
		switch (light->l.rlType)
		{
			case RL_OMNI:
				break;

			case RL_PROJ:
				// FIXME support these in the future
				break;

			case RL_DIRECT:
				break;

			default:
				break;
		}
		*/

		// perform culling and add all the potentially visible surfaces
		s_lightCount++;
		R_RecursivePrecacheInteractionNode(s_worldData.nodes, light);

		// count number of leafs that touch this light
		s_lightCount++;
		numLeafs = 0;
		R_RecursiveAddInteractionNode(s_worldData.nodes, light, &numLeafs, qtrue);
		//ri.Printf(PRINT_ALL, "light %i touched %i leaves\n", i, numLeafs);

		// create storage room for them
		light->leafs = (struct mnode_s **)ri.Hunk_Alloc(numLeafs * sizeof(*light->leafs), h_low);
		light->numLeafs = numLeafs;

		// fill storage with them
		s_lightCount++;
		numLeafs = 0;
		R_RecursiveAddInteractionNode(s_worldData.nodes, light, &numLeafs, qfalse);

		// calculate pyramid bits for each interaction in omni-directional lights
		if(light->l.rlType == RL_OMNI)
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

					grid = (srfGridMesh_t *) surface;

					VectorCopy(grid->meshBounds[0], localBounds[0]);
					VectorCopy(grid->meshBounds[1], localBounds[1]);
				}
				else if(*surface->data == SF_TRIANGLES)
				{
					srfTriangles_t *tri;

					tri = (srfTriangles_t *) surface;

					VectorCopy(tri->bounds[0], localBounds[0]);
					VectorCopy(tri->bounds[1], localBounds[1]);
				}
				else
				{
					iaCache->cubeSideBits = CUBESIDE_CLIPALL;
					continue;
				}

				iaCache->cubeSideBits = R_CalcLightCubeSideBits(light, NULL, localBounds);
			}
		}
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
	ri.Printf(PRINT_ALL, "%i planar surface triangles culled\n", c_culledFaceTriangles);
	ri.Printf(PRINT_ALL, "%i bezier surface triangles culled\n", c_culledGridTriangles);
	ri.Printf(PRINT_ALL, "%i abitrary surface triangles culled\n", c_culledTriTriangles);

	if(r_shadows->integer == 4)
	{
		// only interesting for omni-directional shadow mapping
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
	R_LoadFogs(&header->lumps[LUMP_FOGS], &header->lumps[LUMP_BRUSHES], &header->lumps[LUMP_BRUSHSIDES]);
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
	//		  (s_worldData.dataSize % (1024 * 1024)) * 100 / (1024 * 1024));

	// only set tr.world now that we know the entire level has loaded properly
	tr.world = &s_worldData;

	ri.FS_FreeFile(buffer);
}
