/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "tr_local.h"

/*

This file does all of the processing necessary to turn a raw grid of points
read from the map file into a srfGridMesh_t ready for rendering.

The level of detail solution is direction independent, based only on subdivided
distance from the true curve.

Only a single entry point:

srfGridMesh_t *R_SubdividePatchToGrid( int width, int height,
								srfVert_t points[MAX_PATCH_SIZE*MAX_PATCH_SIZE] ) {

*/


/*
============
LerpDrawVert
============
*/
static void LerpSurfaceVert(srfVert_t * a, srfVert_t * b, srfVert_t * out)
{
	out->xyz[0] = 0.5f * (a->xyz[0] + b->xyz[0]);
	out->xyz[1] = 0.5f * (a->xyz[1] + b->xyz[1]);
	out->xyz[2] = 0.5f * (a->xyz[2] + b->xyz[2]);

	out->st[0] = 0.5f * (a->st[0] + b->st[0]);
	out->st[1] = 0.5f * (a->st[1] + b->st[1]);

	out->lightmap[0] = 0.5f * (a->lightmap[0] + b->lightmap[0]);
	out->lightmap[1] = 0.5f * (a->lightmap[1] + b->lightmap[1]);

	out->color[0] = (a->color[0] + b->color[0]) >> 1;
	out->color[1] = (a->color[1] + b->color[1]) >> 1;
	out->color[2] = (a->color[2] + b->color[2]) >> 1;
	out->color[3] = (a->color[3] + b->color[3]) >> 1;
}

/*
============
Transpose
============
*/
static void Transpose(int width, int height, srfVert_t ctrl[MAX_GRID_SIZE][MAX_GRID_SIZE])
{
	int             i, j;
	srfVert_t       temp;

	if(width > height)
	{
		for(i = 0; i < height; i++)
		{
			for(j = i + 1; j < width; j++)
			{
				if(j < height)
				{
					// swap the value
					temp = ctrl[j][i];
					ctrl[j][i] = ctrl[i][j];
					ctrl[i][j] = temp;
				}
				else
				{
					// just copy
					ctrl[j][i] = ctrl[i][j];
				}
			}
		}
	}
	else
	{
		for(i = 0; i < width; i++)
		{
			for(j = i + 1; j < height; j++)
			{
				if(j < width)
				{
					// swap the value
					temp = ctrl[i][j];
					ctrl[i][j] = ctrl[j][i];
					ctrl[j][i] = temp;
				}
				else
				{
					// just copy
					ctrl[i][j] = ctrl[j][i];
				}
			}
		}
	}

}


/*
=================
MakeMeshNormals

Handles all the complicated wrapping and degenerate cases
=================
*/
static void MakeMeshNormals(int width, int height, srfVert_t ctrl[MAX_GRID_SIZE][MAX_GRID_SIZE])
{
	int             i, j, k, dist;
	vec3_t          normal;
	vec3_t          sum;
	int             count;
	vec3_t          base;
	vec3_t          delta;
	int             x, y;
	srfVert_t      *dv;
	vec3_t          around[8], temp;
	qboolean        good[8];
	qboolean        wrapWidth, wrapHeight;
	float           len;
	static int      neighbors[8][2] = {
		{0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}
	};

	wrapWidth = qfalse;
	for(i = 0; i < height; i++)
	{
		VectorSubtract(ctrl[i][0].xyz, ctrl[i][width - 1].xyz, delta);
		len = VectorLengthSquared(delta);
		if(len > 1.0)
		{
			break;
		}
	}
	if(i == height)
	{
		wrapWidth = qtrue;
	}

	wrapHeight = qfalse;
	for(i = 0; i < width; i++)
	{
		VectorSubtract(ctrl[0][i].xyz, ctrl[height - 1][i].xyz, delta);
		len = VectorLengthSquared(delta);
		if(len > 1.0)
		{
			break;
		}
	}
	if(i == width)
	{
		wrapHeight = qtrue;
	}


	for(i = 0; i < width; i++)
	{
		for(j = 0; j < height; j++)
		{
			count = 0;
			dv = &ctrl[j][i];
			VectorCopy(dv->xyz, base);
			for(k = 0; k < 8; k++)
			{
				VectorClear(around[k]);
				good[k] = qfalse;

				for(dist = 1; dist <= 3; dist++)
				{
					x = i + neighbors[k][0] * dist;
					y = j + neighbors[k][1] * dist;
					if(wrapWidth)
					{
						if(x < 0)
						{
							x = width - 1 + x;
						}
						else if(x >= width)
						{
							x = 1 + x - width;
						}
					}
					if(wrapHeight)
					{
						if(y < 0)
						{
							y = height - 1 + y;
						}
						else if(y >= height)
						{
							y = 1 + y - height;
						}
					}

					if(x < 0 || x >= width || y < 0 || y >= height)
					{
						break;	// edge of patch
					}
					VectorSubtract(ctrl[y][x].xyz, base, temp);
					if(VectorNormalize2(temp, temp) == 0)
					{
						continue;	// degenerate edge, get more dist
					}
					else
					{
						good[k] = qtrue;
						VectorCopy(temp, around[k]);
						break;	// good edge
					}
				}
			}

			VectorClear(sum);
			for(k = 0; k < 8; k++)
			{
				if(!good[k] || !good[(k + 1) & 7])
				{
					continue;	// didn't get two points
				}
				CrossProduct(around[(k + 1) & 7], around[k], normal);
				if(VectorNormalize2(normal, normal) == 0)
				{
					continue;
				}
				VectorAdd(normal, sum, sum);
				count++;
			}
			if(count == 0)
			{
//printf("bad normal\n");
				count = 1;
			}
			VectorNormalize2(sum, dv->normal);
		}
	}
}


/*
============
InvertCtrl
============
*/
static void InvertCtrl(int width, int height, srfVert_t ctrl[MAX_GRID_SIZE][MAX_GRID_SIZE])
{
	int             i, j;
	srfVert_t       temp;

	for(i = 0; i < height; i++)
	{
		for(j = 0; j < width / 2; j++)
		{
			temp = ctrl[i][j];
			ctrl[i][j] = ctrl[i][width - 1 - j];
			ctrl[i][width - 1 - j] = temp;
		}
	}
}


/*
=================
InvertErrorTable
=================
*/
static void InvertErrorTable(float errorTable[2][MAX_GRID_SIZE], int width, int height)
{
	int             i;
	float           copy[2][MAX_GRID_SIZE];

	Com_Memcpy(copy, errorTable, sizeof(copy));

	for(i = 0; i < width; i++)
	{
		errorTable[1][i] = copy[0][i];	//[width-1-i];
	}

	for(i = 0; i < height; i++)
	{
		errorTable[0][i] = copy[1][height - 1 - i];
	}

}

/*
==================
PutPointsOnCurve
==================
*/
static void PutPointsOnCurve(srfVert_t ctrl[MAX_GRID_SIZE][MAX_GRID_SIZE], int width, int height)
{
	int             i, j;
	srfVert_t       prev, next;

	for(i = 0; i < width; i++)
	{
		for(j = 1; j < height; j += 2)
		{
			LerpSurfaceVert(&ctrl[j][i], &ctrl[j + 1][i], &prev);
			LerpSurfaceVert(&ctrl[j][i], &ctrl[j - 1][i], &next);
			LerpSurfaceVert(&prev, &next, &ctrl[j][i]);
		}
	}


	for(j = 0; j < height; j++)
	{
		for(i = 1; i < width; i += 2)
		{
			LerpSurfaceVert(&ctrl[j][i], &ctrl[j][i + 1], &prev);
			LerpSurfaceVert(&ctrl[j][i], &ctrl[j][i - 1], &next);
			LerpSurfaceVert(&prev, &next, &ctrl[j][i]);
		}
	}
}

/*
=================
R_CreateSurfaceGridMesh
=================
*/
srfGridMesh_t  *R_CreateSurfaceGridMesh(int width, int height,
										srfVert_t ctrl[MAX_GRID_SIZE][MAX_GRID_SIZE],
										float errorTable[2][MAX_GRID_SIZE])
{
	int             i, j, size;
	srfVert_t      *vert;
	vec3_t          tmpVec;
	srfGridMesh_t  *grid;

	// copy the results out to a grid
	size = (width * height - 1) * sizeof(srfVert_t) + sizeof(*grid);

#ifdef PATCH_STITCHING
	grid = /*ri.Hunk_Alloc */ ri.Malloc(size);
	Com_Memset(grid, 0, size);

	grid->widthLodError = /*ri.Hunk_Alloc */ ri.Malloc(width * 4);
	Com_Memcpy(grid->widthLodError, errorTable[0], width * 4);

	grid->heightLodError = /*ri.Hunk_Alloc */ ri.Malloc(height * 4);
	Com_Memcpy(grid->heightLodError, errorTable[1], height * 4);
#else
	grid = ri.Hunk_Alloc(size);
	Com_Memset(grid, 0, size);

	grid->widthLodError = ri.Hunk_Alloc(width * 4);
	Com_Memcpy(grid->widthLodError, errorTable[0], width * 4);

	grid->heightLodError = ri.Hunk_Alloc(height * 4);
	Com_Memcpy(grid->heightLodError, errorTable[1], height * 4);
#endif

	grid->width = width;
	grid->height = height;
	grid->surfaceType = SF_GRID;
	ClearBounds(grid->meshBounds[0], grid->meshBounds[1]);
	for(i = 0; i < width; i++)
	{
		for(j = 0; j < height; j++)
		{
			vert = &grid->verts[j * width + i];
			*vert = ctrl[j][i];
			AddPointToBounds(vert->xyz, grid->meshBounds[0], grid->meshBounds[1]);
		}
	}

	// compute local origin and bounds
	VectorAdd(grid->meshBounds[0], grid->meshBounds[1], grid->localOrigin);
	VectorScale(grid->localOrigin, 0.5f, grid->localOrigin);
	VectorSubtract(grid->meshBounds[0], grid->localOrigin, tmpVec);
	grid->meshRadius = VectorLength(tmpVec);

	VectorCopy(grid->localOrigin, grid->lodOrigin);
	grid->lodRadius = grid->meshRadius;
	//
	return grid;
}

/*
=================
R_FreeSurfaceGridMesh
=================
*/
void R_FreeSurfaceGridMesh(srfGridMesh_t * grid)
{
	ri.Free(grid->widthLodError);
	ri.Free(grid->heightLodError);
	ri.Free(grid);
}

/*
=================
R_SubdividePatchToGrid
=================
*/
srfGridMesh_t  *R_SubdividePatchToGrid(int width, int height, srfVert_t points[MAX_PATCH_SIZE * MAX_PATCH_SIZE])
{
	int             i, j, k, l;
	srfVert_t       prev, next, mid;
	float           len, maxLen;
	int             dir;
	int             t;
	MAC_STATIC srfVert_t ctrl[MAX_GRID_SIZE][MAX_GRID_SIZE];
	float           errorTable[2][MAX_GRID_SIZE];

	for(i = 0; i < width; i++)
	{
		for(j = 0; j < height; j++)
		{
			ctrl[j][i] = points[j * width + i];
		}
	}

	for(dir = 0; dir < 2; dir++)
	{

		for(j = 0; j < MAX_GRID_SIZE; j++)
		{
			errorTable[dir][j] = 0;
		}

		// horizontal subdivisions
		for(j = 0; j + 2 < width; j += 2)
		{
			// check subdivided midpoints against control points

			// FIXME: also check midpoints of adjacent patches against the control points
			// this would basically stitch all patches in the same LOD group together.

			maxLen = 0;
			for(i = 0; i < height; i++)
			{
				vec3_t          midxyz;
				vec3_t          midxyz2;
				vec3_t          dir;
				vec3_t          projected;
				float           d;

				// calculate the point on the curve
				for(l = 0; l < 3; l++)
				{
					midxyz[l] = (ctrl[i][j].xyz[l] + ctrl[i][j + 1].xyz[l] * 2
								 + ctrl[i][j + 2].xyz[l]) * 0.25f;
				}

				// see how far off the line it is
				// using dist-from-line will not account for internal
				// texture warping, but it gives a lot less polygons than
				// dist-from-midpoint
				VectorSubtract(midxyz, ctrl[i][j].xyz, midxyz);
				VectorSubtract(ctrl[i][j + 2].xyz, ctrl[i][j].xyz, dir);
				VectorNormalize(dir);

				d = DotProduct(midxyz, dir);
				VectorScale(dir, d, projected);
				VectorSubtract(midxyz, projected, midxyz2);
				len = VectorLengthSquared(midxyz2);	// we will do the sqrt later
				if(len > maxLen)
				{
					maxLen = len;
				}
			}

			maxLen = sqrt(maxLen);

			// if all the points are on the lines, remove the entire columns
			if(maxLen < 0.1f)
			{
				errorTable[dir][j + 1] = 999;
				continue;
			}

			// see if we want to insert subdivided columns
			if(width + 2 > MAX_GRID_SIZE)
			{
				errorTable[dir][j + 1] = 1.0f / maxLen;
				continue;		// can't subdivide any more
			}

			if(maxLen <= r_subdivisions->value)
			{
				errorTable[dir][j + 1] = 1.0f / maxLen;
				continue;		// didn't need subdivision
			}

			errorTable[dir][j + 2] = 1.0f / maxLen;

			// insert two columns and replace the peak
			width += 2;
			for(i = 0; i < height; i++)
			{
				LerpSurfaceVert(&ctrl[i][j], &ctrl[i][j + 1], &prev);
				LerpSurfaceVert(&ctrl[i][j + 1], &ctrl[i][j + 2], &next);
				LerpSurfaceVert(&prev, &next, &mid);

				for(k = width - 1; k > j + 3; k--)
				{
					ctrl[i][k] = ctrl[i][k - 2];
				}
				ctrl[i][j + 1] = prev;
				ctrl[i][j + 2] = mid;
				ctrl[i][j + 3] = next;
			}

			// back up and recheck this set again, it may need more subdivision
			j -= 2;

		}

		Transpose(width, height, ctrl);
		t = width;
		width = height;
		height = t;
	}


	// put all the aproximating points on the curve
	PutPointsOnCurve(ctrl, width, height);

	// cull out any rows or columns that are colinear
	for(i = 1; i < width - 1; i++)
	{
		if(errorTable[0][i] != 999)
		{
			continue;
		}
		for(j = i + 1; j < width; j++)
		{
			for(k = 0; k < height; k++)
			{
				ctrl[k][j - 1] = ctrl[k][j];
			}
			errorTable[0][j - 1] = errorTable[0][j];
		}
		width--;
	}

	for(i = 1; i < height - 1; i++)
	{
		if(errorTable[1][i] != 999)
		{
			continue;
		}
		for(j = i + 1; j < height; j++)
		{
			for(k = 0; k < width; k++)
			{
				ctrl[j - 1][k] = ctrl[j][k];
			}
			errorTable[1][j - 1] = errorTable[1][j];
		}
		height--;
	}

#if 1
	// flip for longest tristrips as an optimization
	// the results should be visually identical with or
	// without this step
	if(height > width)
	{
		Transpose(width, height, ctrl);
		InvertErrorTable(errorTable, width, height);
		t = width;
		width = height;
		height = t;
		InvertCtrl(width, height, ctrl);
	}
#endif

	// calculate normals
	MakeMeshNormals(width, height, ctrl);

	return R_CreateSurfaceGridMesh(width, height, ctrl, errorTable);
}

/*
===============
R_GridInsertColumn
===============
*/
srfGridMesh_t  *R_GridInsertColumn(srfGridMesh_t * grid, int column, int row, vec3_t point,
								   float loderror)
{
	int             i, j;
	int             width, height, oldwidth;
	MAC_STATIC srfVert_t ctrl[MAX_GRID_SIZE][MAX_GRID_SIZE];
	float           errorTable[2][MAX_GRID_SIZE];
	float           lodRadius;
	vec3_t          lodOrigin;

	oldwidth = 0;
	width = grid->width + 1;
	if(width > MAX_GRID_SIZE)
		return NULL;
	height = grid->height;
	for(i = 0; i < width; i++)
	{
		if(i == column)
		{
			//insert new column
			for(j = 0; j < grid->height; j++)
			{
				LerpSurfaceVert(&grid->verts[j * grid->width + i - 1],
								&grid->verts[j * grid->width + i], &ctrl[j][i]);
				if(j == row)
					VectorCopy(point, ctrl[j][i].xyz);
			}
			errorTable[0][i] = loderror;
			continue;
		}
		errorTable[0][i] = grid->widthLodError[oldwidth];
		for(j = 0; j < grid->height; j++)
		{
			ctrl[j][i] = grid->verts[j * grid->width + oldwidth];
		}
		oldwidth++;
	}
	for(j = 0; j < grid->height; j++)
	{
		errorTable[1][j] = grid->heightLodError[j];
	}
	// put all the aproximating points on the curve
	//PutPointsOnCurve( ctrl, width, height );
	// calculate normals
	MakeMeshNormals(width, height, ctrl);

	VectorCopy(grid->lodOrigin, lodOrigin);
	lodRadius = grid->lodRadius;
	// free the old grid
	R_FreeSurfaceGridMesh(grid);
	// create a new grid
	grid = R_CreateSurfaceGridMesh(width, height, ctrl, errorTable);
	grid->lodRadius = lodRadius;
	VectorCopy(lodOrigin, grid->lodOrigin);
	return grid;
}

/*
===============
R_GridInsertRow
===============
*/
srfGridMesh_t  *R_GridInsertRow(srfGridMesh_t * grid, int row, int column, vec3_t point,
								float loderror)
{
	int             i, j;
	int             width, height, oldheight;
	MAC_STATIC srfVert_t ctrl[MAX_GRID_SIZE][MAX_GRID_SIZE];
	float           errorTable[2][MAX_GRID_SIZE];
	float           lodRadius;
	vec3_t          lodOrigin;

	oldheight = 0;
	width = grid->width;
	height = grid->height + 1;
	if(height > MAX_GRID_SIZE)
		return NULL;
	for(i = 0; i < height; i++)
	{
		if(i == row)
		{
			//insert new row
			for(j = 0; j < grid->width; j++)
			{
				LerpSurfaceVert(&grid->verts[(i - 1) * grid->width + j],
								&grid->verts[i * grid->width + j], &ctrl[i][j]);
				if(j == column)
					VectorCopy(point, ctrl[i][j].xyz);
			}
			errorTable[1][i] = loderror;
			continue;
		}
		errorTable[1][i] = grid->heightLodError[oldheight];
		for(j = 0; j < grid->width; j++)
		{
			ctrl[i][j] = grid->verts[oldheight * grid->width + j];
		}
		oldheight++;
	}
	for(j = 0; j < grid->width; j++)
	{
		errorTable[0][j] = grid->widthLodError[j];
	}
	// put all the aproximating points on the curve
	//PutPointsOnCurve( ctrl, width, height );
	// calculate normals
	MakeMeshNormals(width, height, ctrl);

	VectorCopy(grid->lodOrigin, lodOrigin);
	lodRadius = grid->lodRadius;
	// free the old grid
	R_FreeSurfaceGridMesh(grid);
	// create a new grid
	grid = R_CreateSurfaceGridMesh(width, height, ctrl, errorTable);
	grid->lodRadius = lodRadius;
	VectorCopy(lodOrigin, grid->lodOrigin);
	return grid;
}


/*
=============
R_CalcTangentSpacesOnGrid
=============
*/
/*
void R_CalcTangentSpacesOnGrid(srfGridMesh_t * cv)
{
	int             i, j;
	float          *xyz;
	float          *texCoord;
	float          *tangent;
	float          *binormal;
	float          *normal;
	srfVert_t      *dv;
	int             rows, irows, vrows;
	int             used;
	int             widthTable[MAX_GRID_SIZE];
	int             heightTable[MAX_GRID_SIZE];
	float           lodError;
	int             lodWidth, lodHeight;
	int             numIndexes;
	int             numVertexes;
	
	glIndex_t       indexes[SHADER_MAX_INDEXES];
	vec4_t          vertexes[SHADER_MAX_VERTEXES];
	vec4_t          tangents[SHADER_MAX_VERTEXES];
	vec4_t          binormals[SHADER_MAX_VERTEXES];
	vec4_t          normals[SHADER_MAX_VERTEXES];
	vec2_t          texCoords[SHADER_MAX_VERTEXES];
	
	numVertexes = 0;
	numIndexes = 0;

	// determine the allowable discrepance
	lodError = r_lodCurveError->value; //LodErrorForVolume(cv->lodOrigin, cv->lodRadius);

	// determine which rows and columns of the subdivision
	// we are actually going to use
	widthTable[0] = 0;
	lodWidth = 1;
	for(i = 1; i < cv->width - 1; i++)
	{
		if(cv->widthLodError[i] <= lodError)
		{
			widthTable[lodWidth] = i;
			lodWidth++;
		}
	}
	widthTable[lodWidth] = cv->width - 1;
	lodWidth++;

	heightTable[0] = 0;
	lodHeight = 1;
	for(i = 1; i < cv->height - 1; i++)
	{
		if(cv->heightLodError[i] <= lodError)
		{
			heightTable[lodHeight] = i;
			lodHeight++;
		}
	}
	heightTable[lodHeight] = cv->height - 1;
	lodHeight++;


	// very large grids may have more points or indexes than can be fit
	// in the tess structure, so we may have to issue it in multiple passes

	used = 0;
	rows = 0;
	while(used < lodHeight - 1)
	{
		// see how many rows of both verts and indexes we can add without overflowing
		do
		{
			vrows = (SHADER_MAX_VERTEXES - numVertexes) / lodWidth;
			irows = (SHADER_MAX_INDEXES - numIndexes) / (lodWidth * 6);

			// if we don't have enough space for at least one strip, flush the buffer
			if(vrows < 2 || irows < 1)
			{
				ri.Printf(PRINT_WARNING, "failed on precalculations of tangent spaces for grid surface\n");
				numIndexes = 0;
				numVertexes = 0;
			}
			else
			{
				break;
			}
		} while(1);

		rows = irows;
		if(vrows < irows + 1)
		{
			rows = vrows - 1;
		}
		if(used + rows > lodHeight)
		{
			rows = lodHeight - used;
		}

		// copy necessary information to calculate the tangent spaces
		xyz = vertexes[0];
		texCoord = texCoords[0];
		
		for(i = 0; i < rows; i++)
		{
			for(j = 0; j < lodWidth; j++)
			{
				dv = cv->verts + heightTable[used + i] * cv->width + widthTable[j];

				xyz[0] = dv->xyz[0];
				xyz[1] = dv->xyz[1];
				xyz[2] = dv->xyz[2];
				
				texCoord[0] = dv->st[0];
				texCoord[1] = dv->st[1];

				xyz += 4;
				texCoord += 4;
			}
		}

		// add the indexes
		{
			int             w, h;

			h = rows - 1;
			w = lodWidth - 1;
			numIndexes = 0;
			for(i = 0; i < h; i++)
			{
				for(j = 0; j < w; j++)
				{
					int             v1, v2, v3, v4;

					// vertex order to be reckognized as tristrips
					v1 = i * lodWidth + j + 1;
					v2 = v1 - 1;
					v3 = v2 + lodWidth;
					v4 = v3 + 1;

					indexes[numIndexes] = v2;
					indexes[numIndexes + 1] = v3;
					indexes[numIndexes + 2] = v1;

					indexes[numIndexes + 3] = v1;
					indexes[numIndexes + 4] = v3;
					indexes[numIndexes + 5] = v4;
					numIndexes += 6;
				}
			}
		}

		// calc tangent spaces
		{
			float          *v;
			const float    *v0, *v1, *v2;
			const float    *t0, *t1, *t2;
			vec3_t          tangent;
			vec3_t          binormal;
			vec3_t          normal;
			int            *indices;

			for(i = 0; i < (rows * lodWidth); i++)
			{
				VectorClear(tangents[numVertexes + i]);
				VectorClear(binormals[numVertexes + i]);
				VectorClear(normals[numVertexes + i]);
			}

			for(i = 0, indices = indexes + numIndexes; i < numIndexes; i += 3, indices += 3)
			{
				v0 = vertexes[indices[0]];
				v1 = vertexes[indices[1]];
				v2 = vertexes[indices[2]];

				t0 = texCoords[indices[0]];
				t1 = texCoords[indices[1]];
				t2 = texCoords[indices[2]];

				R_CalcNormalForTriangle(normal, v0, v1, v2);
				R_CalcTangentsForTriangle(tangent, binormal, v0, v1, v2, t0, t1, t2);

				for(j = 0; j < 3; j++)
				{
					v = tangents[indices[j]];
					VectorAdd(v, tangent, v);
					v = binormals[indices[j]];
					VectorAdd(v, binormal, v);
					v = normals[indices[j]];
					VectorAdd(v, normal, v);
				}
			}

			for(i = 0; i < (rows * lodWidth); i++)
			{
				VectorNormalize(tangents[numVertexes + i]);
				VectorNormalize(binormals[numVertexes + i]);
				VectorNormalize(normals[numVertexes + i]);
			}
		}
		
		// copy tangents, binormals and normals back
		tangent = tangents[0];
		binormal = binormals[0];
		normal = normals[0];
		
		for(i = 0; i < rows; i++)
		{
			for(j = 0; j < lodWidth; j++)
			{
				dv = cv->verts + heightTable[used + i] * cv->width + widthTable[j];
				
				dv->tangent[0] = tangent[0];
				dv->tangent[1] = tangent[1];
				dv->tangent[2] = tangent[2];
				
				dv->binormal[0] = binormal[0];
				dv->binormal[1] = binormal[1];
				dv->binormal[2] = binormal[2];
				
				dv->normal[0] = normal[0];
				dv->normal[1] = normal[1];
				dv->normal[2] = normal[2];
				
				tangent += 4;
				binormal += 4;
				normal += 4;
			}
		}

		numIndexes += numIndexes;
		numVertexes += rows * lodWidth;

		used += rows - 1;
	}
}
*/
