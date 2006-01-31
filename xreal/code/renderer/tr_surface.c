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
// tr_surf.c
#include "tr_local.h"

/*

  THIS ENTIRE FILE IS BACK END

backEnd.currentEntity will be valid.

Tess_Begin has already been called for the surface's shader.

The modelview matrix will be set.

It is safe to actually issue drawing commands here if you don't want to
use the shader system.
*/


//============================================================================


/*
==============
RB_CheckOverflow
==============
*/
void RB_CheckOverflow(int verts, int indexes)
{
	if(tess.numVertexes + verts < SHADER_MAX_VERTEXES && tess.numIndexes + indexes < SHADER_MAX_INDEXES)
	{
		return;
	}

	RB_EndSurface();

	if(verts >= SHADER_MAX_VERTEXES)
	{
		ri.Error(ERR_DROP, "RB_CheckOverflow: verts > MAX (%d > %d)", verts, SHADER_MAX_VERTEXES);
	}
	if(indexes >= SHADER_MAX_INDEXES)
	{
		ri.Error(ERR_DROP, "RB_CheckOverflow: indices > MAX (%d > %d)", indexes, SHADER_MAX_INDEXES);
	}

	RB_BeginSurface(tess.surfaceShader, tess.lightShader, tess.fogNum, tess.skipTangentSpaces);
}


/*
==============
RB_AddQuadStampExt
==============
*/
void RB_AddQuadStampExt(vec3_t origin, vec3_t left, vec3_t up, byte * color, float s1, float t1, float s2, float t2)
{
	vec3_t          normal;
	int             ndx;

	RB_CHECKOVERFLOW(4, 6);

	ndx = tess.numVertexes;

	// triangle indexes for a simple quad
	tess.indexes[tess.numIndexes] = ndx;
	tess.indexes[tess.numIndexes + 1] = ndx + 1;
	tess.indexes[tess.numIndexes + 2] = ndx + 3;

	tess.indexes[tess.numIndexes + 3] = ndx + 3;
	tess.indexes[tess.numIndexes + 4] = ndx + 1;
	tess.indexes[tess.numIndexes + 5] = ndx + 2;

	tess.xyz[ndx][0] = origin[0] + left[0] + up[0];
	tess.xyz[ndx][1] = origin[1] + left[1] + up[1];
	tess.xyz[ndx][2] = origin[2] + left[2] + up[2];

	tess.xyz[ndx + 1][0] = origin[0] - left[0] + up[0];
	tess.xyz[ndx + 1][1] = origin[1] - left[1] + up[1];
	tess.xyz[ndx + 1][2] = origin[2] - left[2] + up[2];

	tess.xyz[ndx + 2][0] = origin[0] - left[0] - up[0];
	tess.xyz[ndx + 2][1] = origin[1] - left[1] - up[1];
	tess.xyz[ndx + 2][2] = origin[2] - left[2] - up[2];

	tess.xyz[ndx + 3][0] = origin[0] + left[0] - up[0];
	tess.xyz[ndx + 3][1] = origin[1] + left[1] - up[1];
	tess.xyz[ndx + 3][2] = origin[2] + left[2] - up[2];


	// constant normal all the way around
	VectorSubtract(vec3_origin, backEnd.viewParms.or.axis[0], normal);

	tess.normals[ndx][0] = tess.normals[ndx + 1][0] = tess.normals[ndx + 2][0] = tess.normals[ndx + 3][0] =
		normal[0];
	tess.normals[ndx][1] = tess.normals[ndx + 1][1] = tess.normals[ndx + 2][1] = tess.normals[ndx + 3][1] =
		normal[1];
	tess.normals[ndx][2] = tess.normals[ndx + 1][2] = tess.normals[ndx + 2][2] = tess.normals[ndx + 3][2] =
		normal[2];

	// standard square texture coordinates
	tess.texCoords[ndx][0][0] = tess.texCoords[ndx][1][0] = s1;
	tess.texCoords[ndx][0][1] = tess.texCoords[ndx][1][1] = t1;

	tess.texCoords[ndx + 1][0][0] = tess.texCoords[ndx + 1][1][0] = s2;
	tess.texCoords[ndx + 1][0][1] = tess.texCoords[ndx + 1][1][1] = t1;

	tess.texCoords[ndx + 2][0][0] = tess.texCoords[ndx + 2][1][0] = s2;
	tess.texCoords[ndx + 2][0][1] = tess.texCoords[ndx + 2][1][1] = t2;

	tess.texCoords[ndx + 3][0][0] = tess.texCoords[ndx + 3][1][0] = s1;
	tess.texCoords[ndx + 3][0][1] = tess.texCoords[ndx + 3][1][1] = t2;

	// constant color all the way around
	// should this be identity and let the shader specify from entity?
	*(unsigned int *)&tess.vertexColors[ndx] =
		*(unsigned int *)&tess.vertexColors[ndx + 1] =
		*(unsigned int *)&tess.vertexColors[ndx + 2] =
		*(unsigned int *)&tess.vertexColors[ndx + 3] = *(unsigned int *)color;


	tess.numVertexes += 4;
	tess.numIndexes += 6;
}

/*
==============
RB_AddQuadStamp
==============
*/
void RB_AddQuadStamp(vec3_t origin, vec3_t left, vec3_t up, byte * color)
{
	RB_AddQuadStampExt(origin, left, up, color, 0, 0, 1, 1);
}

/*
==============
RB_SurfaceSprite
==============
*/
static void RB_SurfaceSprite(void)
{
	vec3_t          left, up;
	float           radius;

	// calculate the xyz locations for the four corners
	radius = backEnd.currentEntity->e.radius;
	if(backEnd.currentEntity->e.rotation == 0)
	{
		VectorScale(backEnd.viewParms.or.axis[1], radius, left);
		VectorScale(backEnd.viewParms.or.axis[2], radius, up);
	}
	else
	{
		float           s, c;
		float           ang;

		ang = M_PI * backEnd.currentEntity->e.rotation / 180;
		s = sin(ang);
		c = cos(ang);

		VectorScale(backEnd.viewParms.or.axis[1], c * radius, left);
		VectorMA(left, -s * radius, backEnd.viewParms.or.axis[2], left);

		VectorScale(backEnd.viewParms.or.axis[2], c * radius, up);
		VectorMA(up, s * radius, backEnd.viewParms.or.axis[1], up);
	}
	if(backEnd.viewParms.isMirror)
	{
		VectorSubtract(vec3_origin, left, left);
	}

	RB_AddQuadStamp(backEnd.currentEntity->e.origin, left, up, backEnd.currentEntity->e.shaderRGBA);
}


/*
=============
RB_SurfacePolychain
=============
*/
void RB_SurfacePolychain(srfPoly_t * p)
{
	int             i;
	int             numv;

	RB_CHECKOVERFLOW(p->numVerts, 3 * (p->numVerts - 2));

	// fan triangles into the tess array
	numv = tess.numVertexes;
	for(i = 0; i < p->numVerts; i++)
	{
		VectorCopy(p->verts[i].xyz, tess.xyz[numv]);
		tess.texCoords[numv][0][0] = p->verts[i].st[0];
		tess.texCoords[numv][0][1] = p->verts[i].st[1];
		*(int *)&tess.vertexColors[numv] = *(int *)p->verts[i].modulate;

		numv++;
	}

	// generate fan indexes into the tess array
	for(i = 0; i < p->numVerts - 2; i++)
	{
		tess.indexes[tess.numIndexes + 0] = tess.numVertexes;
		tess.indexes[tess.numIndexes + 1] = tess.numVertexes + i + 1;
		tess.indexes[tess.numIndexes + 2] = tess.numVertexes + i + 2;
		tess.numIndexes += 3;
	}

	tess.numVertexes = numv;
}


/*
=============
RB_SurfaceTriangles
=============
*/
void RB_SurfaceTriangles(srfTriangles_t * srf)
{
	int             i;
	srfVert_t      *dv;
	float          *xyz, *tangent, *binormal, *normal, *texCoords;
	byte           *color;

	RB_CHECKOVERFLOW(srf->numVerts, srf->numIndexes);

	for(i = 0; i < srf->numIndexes; i += 3)
	{
		tess.indexes[tess.numIndexes + i + 0] = tess.numVertexes + srf->indexes[i + 0];
		tess.indexes[tess.numIndexes + i + 1] = tess.numVertexes + srf->indexes[i + 1];
		tess.indexes[tess.numIndexes + i + 2] = tess.numVertexes + srf->indexes[i + 2];
	}
	tess.numIndexes += srf->numIndexes;

	dv = srf->verts;
	xyz = tess.xyz[tess.numVertexes];
	tangent = tess.tangents[tess.numVertexes];
	binormal = tess.binormals[tess.numVertexes];
	normal = tess.normals[tess.numVertexes];
	texCoords = tess.texCoords[tess.numVertexes][0];
	color = tess.vertexColors[tess.numVertexes];

	for(i = 0; i < srf->numVerts;
		i++, dv++, xyz += 4, tangent += 4, binormal += 4, normal += 4, texCoords += 4, color += 4)
	{
		xyz[0] = dv->xyz[0];
		xyz[1] = dv->xyz[1];
		xyz[2] = dv->xyz[2];

		if(!tess.skipTangentSpaces)
		{
			tangent[0] = dv->tangent[0];
			tangent[1] = dv->tangent[1];
			tangent[2] = dv->tangent[2];
		
			binormal[0] = dv->binormal[0];
			binormal[1] = dv->binormal[1];
			binormal[2] = dv->binormal[2];
		
			normal[0] = dv->normal[0];
			normal[1] = dv->normal[1];
			normal[2] = dv->normal[2];
		}

		texCoords[0] = dv->st[0];
		texCoords[1] = dv->st[1];

		texCoords[2] = dv->lightmap[0];
		texCoords[3] = dv->lightmap[1];

		*(int *)color = *(int *)dv->color;
	}

	tess.numVertexes += srf->numVerts;
}



/*
==============
RB_SurfaceBeam
==============
*/
void RB_SurfaceBeam(void)
{
#define NUM_BEAM_SEGS 6
	refExtEntity_t *e;
	int             i;
	vec3_t          perpvec;
	vec3_t          direction, normalized_direction;
	vec3_t          start_points[NUM_BEAM_SEGS], end_points[NUM_BEAM_SEGS];
	vec3_t          oldorigin, origin;

	e = &backEnd.currentEntity->e;

	oldorigin[0] = e->oldorigin[0];
	oldorigin[1] = e->oldorigin[1];
	oldorigin[2] = e->oldorigin[2];

	origin[0] = e->origin[0];
	origin[1] = e->origin[1];
	origin[2] = e->origin[2];

	normalized_direction[0] = direction[0] = oldorigin[0] - origin[0];
	normalized_direction[1] = direction[1] = oldorigin[1] - origin[1];
	normalized_direction[2] = direction[2] = oldorigin[2] - origin[2];

	if(VectorNormalize(normalized_direction) == 0)
		return;

	PerpendicularVector(perpvec, normalized_direction);

	VectorScale(perpvec, 4, perpvec);

	for(i = 0; i < NUM_BEAM_SEGS; i++)
	{
		RotatePointAroundVector(start_points[i], normalized_direction, perpvec, (360.0 / NUM_BEAM_SEGS) * i);
//      VectorAdd( start_points[i], origin, start_points[i] );
		VectorAdd(start_points[i], direction, end_points[i]);
	}

	GL_Program(0);
	GL_SelectTexture(0);
	GL_Bind(tr.whiteImage);

	GL_State(GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE);

	qglColor3f(1, 0, 0);

	qglBegin(GL_TRIANGLE_STRIP);
	for(i = 0; i <= NUM_BEAM_SEGS; i++)
	{
		qglVertex3fv(start_points[i % NUM_BEAM_SEGS]);
		qglVertex3fv(end_points[i % NUM_BEAM_SEGS]);
	}
	qglEnd();
}

//================================================================================

static void DoRailCore(const vec3_t start, const vec3_t end, const vec3_t up, float len, float spanWidth)
{
	float           spanWidth2;
	int             vbase;
	float           t = len / 256.0f;

	vbase = tess.numVertexes;

	spanWidth2 = -spanWidth;

	// FIXME: use quad stamp?
	VectorMA(start, spanWidth, up, tess.xyz[tess.numVertexes]);
	tess.texCoords[tess.numVertexes][0][0] = 0;
	tess.texCoords[tess.numVertexes][0][1] = 0;
	tess.vertexColors[tess.numVertexes][0] = backEnd.currentEntity->e.shaderRGBA[0] * 0.25;
	tess.vertexColors[tess.numVertexes][1] = backEnd.currentEntity->e.shaderRGBA[1] * 0.25;
	tess.vertexColors[tess.numVertexes][2] = backEnd.currentEntity->e.shaderRGBA[2] * 0.25;
	tess.numVertexes++;

	VectorMA(start, spanWidth2, up, tess.xyz[tess.numVertexes]);
	tess.texCoords[tess.numVertexes][0][0] = 0;
	tess.texCoords[tess.numVertexes][0][1] = 1;
	tess.vertexColors[tess.numVertexes][0] = backEnd.currentEntity->e.shaderRGBA[0];
	tess.vertexColors[tess.numVertexes][1] = backEnd.currentEntity->e.shaderRGBA[1];
	tess.vertexColors[tess.numVertexes][2] = backEnd.currentEntity->e.shaderRGBA[2];
	tess.numVertexes++;

	VectorMA(end, spanWidth, up, tess.xyz[tess.numVertexes]);

	tess.texCoords[tess.numVertexes][0][0] = t;
	tess.texCoords[tess.numVertexes][0][1] = 0;
	tess.vertexColors[tess.numVertexes][0] = backEnd.currentEntity->e.shaderRGBA[0];
	tess.vertexColors[tess.numVertexes][1] = backEnd.currentEntity->e.shaderRGBA[1];
	tess.vertexColors[tess.numVertexes][2] = backEnd.currentEntity->e.shaderRGBA[2];
	tess.numVertexes++;

	VectorMA(end, spanWidth2, up, tess.xyz[tess.numVertexes]);
	tess.texCoords[tess.numVertexes][0][0] = t;
	tess.texCoords[tess.numVertexes][0][1] = 1;
	tess.vertexColors[tess.numVertexes][0] = backEnd.currentEntity->e.shaderRGBA[0];
	tess.vertexColors[tess.numVertexes][1] = backEnd.currentEntity->e.shaderRGBA[1];
	tess.vertexColors[tess.numVertexes][2] = backEnd.currentEntity->e.shaderRGBA[2];
	tess.numVertexes++;

	tess.indexes[tess.numIndexes++] = vbase;
	tess.indexes[tess.numIndexes++] = vbase + 1;
	tess.indexes[tess.numIndexes++] = vbase + 2;

	tess.indexes[tess.numIndexes++] = vbase + 2;
	tess.indexes[tess.numIndexes++] = vbase + 1;
	tess.indexes[tess.numIndexes++] = vbase + 3;
}

static void DoRailDiscs(int numSegs, const vec3_t start, const vec3_t dir, const vec3_t right,
						const vec3_t up)
{
	int             i;
	vec3_t          pos[4];
	vec3_t          v;
	int             spanWidth = r_railWidth->integer;
	float           c, s;
	float           scale;

	if(numSegs > 1)
		numSegs--;
	if(!numSegs)
		return;

	scale = 0.25;

	for(i = 0; i < 4; i++)
	{
		c = cos(DEG2RAD(45 + i * 90));
		s = sin(DEG2RAD(45 + i * 90));
		v[0] = (right[0] * c + up[0] * s) * scale * spanWidth;
		v[1] = (right[1] * c + up[1] * s) * scale * spanWidth;
		v[2] = (right[2] * c + up[2] * s) * scale * spanWidth;
		VectorAdd(start, v, pos[i]);

		if(numSegs > 1)
		{
			// offset by 1 segment if we're doing a long distance shot
			VectorAdd(pos[i], dir, pos[i]);
		}
	}

	for(i = 0; i < numSegs; i++)
	{
		int             j;

		RB_CHECKOVERFLOW(4, 6);

		for(j = 0; j < 4; j++)
		{
			VectorCopy(pos[j], tess.xyz[tess.numVertexes]);
			tess.texCoords[tess.numVertexes][0][0] = (j < 2);
			tess.texCoords[tess.numVertexes][0][1] = (j && j != 3);
			tess.vertexColors[tess.numVertexes][0] = backEnd.currentEntity->e.shaderRGBA[0];
			tess.vertexColors[tess.numVertexes][1] = backEnd.currentEntity->e.shaderRGBA[1];
			tess.vertexColors[tess.numVertexes][2] = backEnd.currentEntity->e.shaderRGBA[2];
			tess.numVertexes++;

			VectorAdd(pos[j], dir, pos[j]);
		}

		tess.indexes[tess.numIndexes++] = tess.numVertexes - 4 + 0;
		tess.indexes[tess.numIndexes++] = tess.numVertexes - 4 + 1;
		tess.indexes[tess.numIndexes++] = tess.numVertexes - 4 + 3;
		tess.indexes[tess.numIndexes++] = tess.numVertexes - 4 + 3;
		tess.indexes[tess.numIndexes++] = tess.numVertexes - 4 + 1;
		tess.indexes[tess.numIndexes++] = tess.numVertexes - 4 + 2;
	}
}

/*
** RB_SurfaceRailRinges
*/
void RB_SurfaceRailRings(void)
{
	refExtEntity_t *e;
	int             numSegs;
	int             len;
	vec3_t          vec;
	vec3_t          right, up;
	vec3_t          start, end;

	e = &backEnd.currentEntity->e;

	VectorCopy(e->oldorigin, start);
	VectorCopy(e->origin, end);

	// compute variables
	VectorSubtract(end, start, vec);
	len = VectorNormalize(vec);
	MakeNormalVectors(vec, right, up);
	numSegs = (len) / r_railSegmentLength->value;
	if(numSegs <= 0)
	{
		numSegs = 1;
	}

	VectorScale(vec, r_railSegmentLength->value, vec);

	DoRailDiscs(numSegs, start, vec, right, up);
}

/*
** RB_SurfaceRailCore
*/
void RB_SurfaceRailCore(void)
{
	refExtEntity_t *e;
	int             len;
	vec3_t          right;
	vec3_t          vec;
	vec3_t          start, end;
	vec3_t          v1, v2;

	e = &backEnd.currentEntity->e;

	VectorCopy(e->oldorigin, start);
	VectorCopy(e->origin, end);

	VectorSubtract(end, start, vec);
	len = VectorNormalize(vec);

	// compute side vector
	VectorSubtract(start, backEnd.viewParms.or.origin, v1);
	VectorNormalize(v1);
	VectorSubtract(end, backEnd.viewParms.or.origin, v2);
	VectorNormalize(v2);
	CrossProduct(v1, v2, right);
	VectorNormalize(right);

	DoRailCore(start, end, right, len, r_railCoreWidth->integer);
}

/*
** RB_SurfaceLightningBolt
*/
void RB_SurfaceLightningBolt(void)
{
	refExtEntity_t *e;
	int             len;
	vec3_t          right;
	vec3_t          vec;
	vec3_t          start, end;
	vec3_t          v1, v2;
	int             i;

	e = &backEnd.currentEntity->e;

	VectorCopy(e->oldorigin, end);
	VectorCopy(e->origin, start);

	// compute variables
	VectorSubtract(end, start, vec);
	len = VectorNormalize(vec);

	// compute side vector
	VectorSubtract(start, backEnd.viewParms.or.origin, v1);
	VectorNormalize(v1);
	VectorSubtract(end, backEnd.viewParms.or.origin, v2);
	VectorNormalize(v2);
	CrossProduct(v1, v2, right);
	VectorNormalize(right);

	for(i = 0; i < 4; i++)
	{
		vec3_t          temp;

		DoRailCore(start, end, right, len, 8);
		RotatePointAroundVector(temp, vec, right, 45);
		VectorCopy(temp, right);
	}
}

/*
** VectorArrayNormalize
*
* The inputs to this routing seem to always be close to length = 1.0 (about 0.6 to 2.0)
* This means that we don't have to worry about zero length or enormously long vectors.
*/
static void VectorArrayNormalize(vec4_t * normals, unsigned int count)
{
//    assert(count);

#if idppc
	{
		register float  half = 0.5;
		register float  one = 1.0;
		float          *components = (float *)normals;

		// Vanilla PPC code, but since PPC has a reciprocal square root estimate instruction,
		// runs *much* faster than calling sqrt().  We'll use a single Newton-Raphson
		// refinement step to get a little more precision.  This seems to yeild results
		// that are correct to 3 decimal places and usually correct to at least 4 (sometimes 5).
		// (That is, for the given input range of about 0.6 to 2.0).
		do
		{
			float           x, y, z;
			float           B, y0, y1;

			x = components[0];
			y = components[1];
			z = components[2];
			components += 4;
			B = x * x + y * y + z * z;

#ifdef __GNUC__
		  asm("frsqrte %0,%1": "=f"(y0):"f"(B));
#else
			y0 = __frsqrte(B);
#endif
			y1 = y0 + half * y0 * (one - B * y0 * y0);

			x = x * y1;
			y = y * y1;
			components[-4] = x;
			z = z * y1;
			components[-3] = y;
			components[-2] = z;
		} while(count--);
	}
#else							// No assembly version for this architecture, or C_ONLY defined
	// given the input, it's safe to call VectorNormalizeFast
	while(count--)
	{
		VectorNormalize(normals[0]);
		normals++;
	}
#endif

}



/*
** LerpMeshVertexes
*/
static void LerpMeshVertexes(md3Surface_t * surf, float backlerp)
{
	short          *oldXyz, *newXyz, *oldNormals, *newNormals;
	float          *outXyz, *outNormal;
	float           oldXyzScale, newXyzScale;
	float           oldNormalScale, newNormalScale;
	int             vertNum;
	int             numVerts;

	outXyz = tess.xyz[tess.numVertexes];
	outNormal = tess.normals[tess.numVertexes];

	newXyz = (short *)((byte *) surf + surf->ofsXyzNormals)
		+ (backEnd.currentEntity->e.frame * surf->numVerts * 4);
	newNormals = newXyz + 3;

	newXyzScale = MD3_XYZ_SCALE * (1.0 - backlerp);
	newNormalScale = 1.0 - backlerp;

	numVerts = surf->numVerts;

	if(backlerp == 0)
	{
#if idppc_altivec
		unsigned        lat, lng;
		vector signed short newNormalsVec0;
		vector signed short newNormalsVec1;
		vector signed int newNormalsIntVec;
		vector float    newNormalsFloatVec;
		vector float    newXyzScaleVec;
		vector unsigned char newNormalsLoadPermute;
		vector unsigned char newNormalsStorePermute;
		vector float    zero;

		newNormalsStorePermute = vec_lvsl(0, (float *)&newXyzScaleVec);
		newXyzScaleVec = *(vector float *)&newXyzScale;
		newXyzScaleVec = vec_perm(newXyzScaleVec, newXyzScaleVec, newNormalsStorePermute);
		newXyzScaleVec = vec_splat(newXyzScaleVec, 0);
		newNormalsLoadPermute = vec_lvsl(0, newXyz);
		newNormalsStorePermute = vec_lvsr(0, outXyz);
		zero = (vector float)vec_splat_s8(0);
		//
		// just copy the vertexes
		//
		for(vertNum = 0; vertNum < numVerts; vertNum++,
			newXyz += 4, newNormals += 4, outXyz += 4, outNormal += 4)
		{
			newNormalsLoadPermute = vec_lvsl(0, newXyz);
			newNormalsStorePermute = vec_lvsr(0, outXyz);
			newNormalsVec0 = vec_ld(0, newXyz);
			newNormalsVec1 = vec_ld(16, newXyz);
			newNormalsVec0 = vec_perm(newNormalsVec0, newNormalsVec1, newNormalsLoadPermute);
			newNormalsIntVec = vec_unpackh(newNormalsVec0);
			newNormalsFloatVec = vec_ctf(newNormalsIntVec, 0);
			newNormalsFloatVec = vec_madd(newNormalsFloatVec, newXyzScaleVec, zero);
			newNormalsFloatVec = vec_perm(newNormalsFloatVec, newNormalsFloatVec, newNormalsStorePermute);
			//outXyz[0] = newXyz[0] * newXyzScale;
			//outXyz[1] = newXyz[1] * newXyzScale;
			//outXyz[2] = newXyz[2] * newXyzScale;

			lat = (newNormals[0] >> 8) & 0xff;
			lng = (newNormals[0] & 0xff);
			lat *= (FUNCTABLE_SIZE / 256);
			lng *= (FUNCTABLE_SIZE / 256);

			// decode X as cos( lat ) * sin( long )
			// decode Y as sin( lat ) * sin( long )
			// decode Z as cos( long )

			outNormal[0] = tr.sinTable[(lat + (FUNCTABLE_SIZE / 4)) & FUNCTABLE_MASK] * tr.sinTable[lng];
			outNormal[1] = tr.sinTable[lat] * tr.sinTable[lng];
			outNormal[2] = tr.sinTable[(lng + (FUNCTABLE_SIZE / 4)) & FUNCTABLE_MASK];

			vec_ste(newNormalsFloatVec, 0, outXyz);
			vec_ste(newNormalsFloatVec, 4, outXyz);
			vec_ste(newNormalsFloatVec, 8, outXyz);
		}

#else
		//
		// just copy the vertexes
		//
		for(vertNum = 0; vertNum < numVerts; vertNum++,
			newXyz += 4, newNormals += 4, outXyz += 4, outNormal += 4)
		{

			outXyz[0] = newXyz[0] * newXyzScale;
			outXyz[1] = newXyz[1] * newXyzScale;
			outXyz[2] = newXyz[2] * newXyzScale;

			/* Tr3B - well will calculate the normals anyway for tangent space creation, so save speed
			   lat = ( newNormals[0] >> 8 ) & 0xff;
			   lng = ( newNormals[0] & 0xff );
			   lat *= (FUNCTABLE_SIZE/256);
			   lng *= (FUNCTABLE_SIZE/256);

			   // decode X as cos( lat ) * sin( long )
			   // decode Y as sin( lat ) * sin( long )
			   // decode Z as cos( long )

			   outNormal[0] = tr.sinTable[(lat+(FUNCTABLE_SIZE/4))&FUNCTABLE_MASK] * tr.sinTable[lng];
			   outNormal[1] = tr.sinTable[lat] * tr.sinTable[lng];
			   outNormal[2] = tr.sinTable[(lng+(FUNCTABLE_SIZE/4))&FUNCTABLE_MASK];
			 */
		}
#endif
	}
	else
	{
		//
		// interpolate and copy the vertex and normal
		//
		oldXyz = (short *)((byte *) surf + surf->ofsXyzNormals)
			+ (backEnd.currentEntity->e.oldframe * surf->numVerts * 4);
		oldNormals = oldXyz + 3;

		oldXyzScale = MD3_XYZ_SCALE * backlerp;
		oldNormalScale = backlerp;

		for(vertNum = 0; vertNum < numVerts; vertNum++,
			oldXyz += 4, newXyz += 4, oldNormals += 4, newNormals += 4, outXyz += 4, outNormal += 4)
		{
			//vec3_t uncompressedOldNormal, uncompressedNewNormal;

			// interpolate the xyz
			outXyz[0] = oldXyz[0] * oldXyzScale + newXyz[0] * newXyzScale;
			outXyz[1] = oldXyz[1] * oldXyzScale + newXyz[1] * newXyzScale;
			outXyz[2] = oldXyz[2] * oldXyzScale + newXyz[2] * newXyzScale;

			/* Tr3B - well will calculate the normals anyway for tangent space creation, so save speed
			   // FIXME: interpolate lat/long instead?
			   lat = ( newNormals[0] >> 8 ) & 0xff;
			   lng = ( newNormals[0] & 0xff );
			   lat *= 4;
			   lng *= 4;
			   uncompressedNewNormal[0] = tr.sinTable[(lat+(FUNCTABLE_SIZE/4))&FUNCTABLE_MASK] * tr.sinTable[lng];
			   uncompressedNewNormal[1] = tr.sinTable[lat] * tr.sinTable[lng];
			   uncompressedNewNormal[2] = tr.sinTable[(lng+(FUNCTABLE_SIZE/4))&FUNCTABLE_MASK];

			   lat = ( oldNormals[0] >> 8 ) & 0xff;
			   lng = ( oldNormals[0] & 0xff );
			   lat *= 4;
			   lng *= 4;

			   uncompressedOldNormal[0] = tr.sinTable[(lat+(FUNCTABLE_SIZE/4))&FUNCTABLE_MASK] * tr.sinTable[lng];
			   uncompressedOldNormal[1] = tr.sinTable[lat] * tr.sinTable[lng];
			   uncompressedOldNormal[2] = tr.sinTable[(lng+(FUNCTABLE_SIZE/4))&FUNCTABLE_MASK];

			   outNormal[0] = uncompressedOldNormal[0] * oldNormalScale + uncompressedNewNormal[0] * newNormalScale;
			   outNormal[1] = uncompressedOldNormal[1] * oldNormalScale + uncompressedNewNormal[1] * newNormalScale;
			   outNormal[2] = uncompressedOldNormal[2] * oldNormalScale + uncompressedNewNormal[2] * newNormalScale;

			   //           VectorNormalize (outNormal);
			 */
		}
//      VectorArrayNormalize((vec4_t *)tess.normals[tess.numVertexes], numVerts);
	}
}

/*
=============
RB_SurfaceMD3
=============
*/
void RB_SurfaceMD3(md3Surface_t * surface)
{
	int             j;
	float           backlerp;
	int            *triangles;
	float          *texCoords;
	int             numVertexes;
	int             numIndexes;

	if(backEnd.currentEntity->e.oldframe == backEnd.currentEntity->e.frame)
	{
		backlerp = 0;
	}
	else
	{
		backlerp = backEnd.currentEntity->e.backlerp;
	}

	RB_CHECKOVERFLOW(surface->numVerts, surface->numTriangles * 3);

	LerpMeshVertexes(surface, backlerp);

	triangles = (int *)((byte *) surface + surface->ofsTriangles);
	numIndexes = surface->numTriangles * 3;
	for(j = 0; j < numIndexes; j++)
	{
		tess.indexes[tess.numIndexes + j] = tess.numVertexes + triangles[j];
	}

	texCoords = (float *)((byte *) surface + surface->ofsSt);

	numVertexes = surface->numVerts;
	for(j = 0; j < numVertexes; j++)
	{
		tess.texCoords[tess.numVertexes + j][0][0] = texCoords[j * 2 + 0];
		tess.texCoords[tess.numVertexes + j][0][1] = texCoords[j * 2 + 1];
		// FIXME: fill in lightmapST for completeness?
	}

	// Tr3B - calc tangent spaces
	if(!tess.skipTangentSpaces)
	{
		int             i;
		vec3_t          faceNormal;
		float          *v;
		const float    *v0, *v1, *v2;
		const float    *t0, *t1, *t2;
		vec3_t          tangent;
		vec3_t          binormal;
		vec3_t          normal;
		int            *indices;

		for(i = 0; i < numVertexes; i++)
		{
			VectorClear(tess.tangents[tess.numVertexes + i]);
			VectorClear(tess.binormals[tess.numVertexes + i]);
			VectorClear(tess.normals[tess.numVertexes + i]);
		}

		for(i = 0, indices = tess.indexes + tess.numIndexes; i < numIndexes; i += 3, indices += 3)
		{
			v0 = tess.xyz[indices[0]];
			v1 = tess.xyz[indices[1]];
			v2 = tess.xyz[indices[2]];

			t0 = tess.texCoords[indices[0]][0];
			t1 = tess.texCoords[indices[1]][0];
			t2 = tess.texCoords[indices[2]][0];

			R_CalcNormalForTriangle(faceNormal, v0, v1, v2);
			R_CalcTangentSpace(tangent, binormal, normal, v0, v1, v2, t0, t1, t2, faceNormal);

			for(j = 0; j < 3; j++)
			{
				v = tess.tangents[indices[j]];
				VectorAdd(v, tangent, v);
				v = tess.binormals[indices[j]];
				VectorAdd(v, binormal, v);
				v = tess.normals[indices[j]];
				VectorAdd(v, normal, v);
			}
		}

		VectorArrayNormalize((vec4_t *) tess.tangents[tess.numVertexes], numVertexes);
		VectorArrayNormalize((vec4_t *) tess.binormals[tess.numVertexes], numVertexes);
		VectorArrayNormalize((vec4_t *) tess.normals[tess.numVertexes], numVertexes);
	}

	tess.numIndexes += numIndexes;
	tess.numVertexes += surface->numVerts;
}


/*
==============
RB_SurfaceMDS
==============
*/
void RB_SurfaceMDS(mdsSurface_t * surface)
{
	int             i, j, k;
	float           frontlerp, backlerp;
	int            *triangles;
	int             numIndexes = 0;
	int             numVertexes;
	mdsVertex_t    *v;
	vec_t          *boneMat;
	matrix_t		boneMats[MDS_MAX_BONES];
	matrix_t		boneOrigins[MDS_MAX_BONES];
	vec_t          *boneOrigin;
	mdsBone_t      *bone;
//	mdsBone_t      *parent;
	vec_t          *parentMat;
	vec_t          *parentOrigin;
	mdsBoneFrame_t *boneFrame;
	mdsBoneFrame_t *oldBoneFrame;
	mdsHeader_t    *header;
	mdsFrame_t     *frame;
	mdsFrame_t     *oldFrame;
	int             frameSize;
	vec3_t			angles;
	matrix_t		boneRotationMat;
	matrix_t		boneTranslationMat;
//	matrix_t		boneTransformMat;
	matrix_t		offsetRotationMat;
	vec3_t			offsetVec, offsetVec2;
	vec3_t			forward, right, up;


	if(backEnd.currentEntity->e.oldframe == backEnd.currentEntity->e.frame)
	{
		backlerp = 0;
		frontlerp = 1;
	}
	else
	{
		backlerp = backEnd.currentEntity->e.backlerp;
		frontlerp = 1.0f - backlerp;
	}
	header = (mdsHeader_t *) ((byte *) surface + surface->ofsHeader);

	frameSize = (int)(&((mdsFrame_t *) 0)->bones[header->numBones]);

	frame = (mdsFrame_t *) ((byte *) header + header->ofsFrames + backEnd.currentEntity->e.frame * frameSize);
	oldFrame = (mdsFrame_t *) ((byte *) header + header->ofsFrames + backEnd.currentEntity->e.oldframe * frameSize);

	if(!r_showSkeleton->integer)
	{
		RB_CheckOverflow(surface->numVerts, surface->numTriangles * 3);
	
		triangles = (int *)((byte *) surface + surface->ofsTriangles);
		numIndexes = surface->numTriangles * 3;
		for(j = 0; j < numIndexes; j++)
		{
			tess.indexes[tess.numIndexes + j] = tess.numVertexes + triangles[j];
		}
	}
	else
	{
		GL_Program(0);
		GL_SelectTexture(0);
		GL_Bind(tr.whiteImage);
		qglLineWidth(3);
		qglBegin(GL_LINES);
	}

	// lerp all the needed bones
	bone = (mdsBone_t *) ((byte *) header + header->ofsBones);
	for(i = 0; i < header->numBones; i++, bone++)
	{
		boneMat = &boneMats[i][0];
		boneOrigin = &boneOrigins[i][0];
		
		//ri.Printf(PRINT_ALL, "RB_SurfaceMDS: parentIndex %i\n", bone->parentIndex);
		
		// TODO lerp angles between boneFrame and oldBoneFrame
		boneFrame = &frame->bones[i];
		oldBoneFrame = &oldFrame->bones[i];
		
		// construct bone rotation matrix
		angles[PITCH] = SHORT2ANGLE(boneFrame->angles[0]);
		angles[YAW] = SHORT2ANGLE(boneFrame->angles[1]);
		angles[ROLL] = SHORT2ANGLE(boneFrame->angles[2]);
		MatrixFromAngles(boneRotationMat, angles[PITCH], angles[YAW], angles[ROLL]);
		AngleVectors(angles, forward, right, up);
		
		//if(i == 0)
		//	ri.Printf(PRINT_ALL, "RB_SurfaceMDS: angles %i %i %i\n", (int)angles[PITCH], (int)angles[YAW], (int)angles[ROLL]);
		
		// construct bone offset rotation matrix
		angles[PITCH] = SHORT2ANGLE(boneFrame->ofsAngles[0]);
		angles[YAW] = SHORT2ANGLE(boneFrame->ofsAngles[1]);
		angles[ROLL] = 0;
		MatrixFromAngles(offsetRotationMat, angles[PITCH], angles[YAW], angles[ROLL]);
		
		// construct bone offset
		VectorSet(offsetVec, bone->parentDist, 0, 0);
		MatrixTransformNormal(offsetRotationMat, offsetVec, offsetVec2);
		
		if(bone->parentIndex < 0)
		{
			boneOrigin[0] = frame->parentOffset[0] + offsetVec2[0];
			boneOrigin[1] = frame->parentOffset[1] + offsetVec2[1];
			boneOrigin[2] = frame->parentOffset[2] + offsetVec2[2];
			
			//boneOrigin[0] = offsetVec2[0];
			//boneOrigin[1] = offsetVec2[1];
			//boneOrigin[2] = offsetVec2[2];
			
			MatrixSetupTranslation(boneTranslationMat, boneOrigin[0], boneOrigin[1], boneOrigin[2]);
			MatrixMultiply(boneTranslationMat, boneRotationMat, boneMat);
			//MatrixMultiply(boneRotationMat, boneTranslationMat, boneMat);
			
			if(r_showSkeleton->integer)
			{
				// draw bone
				qglColor3f(1, 1, 1);
				qglVertex3f(0, 0, 0);
				qglVertex3fv(boneOrigin);
				
				// draw bone axis
				qglColor3f(1, 0, 0);
				qglVertex3f(0, 0, 0);
				qglVertex3fv(forward);
				qglVertex3fv(forward);
				qglVertex3fv(boneOrigin);
				
				qglColor3f(0, 1, 0);
				qglVertex3f(0, 0, 0);
				qglVertex3fv(right);
				qglVertex3fv(right);
				qglVertex3fv(boneOrigin);
				
				qglColor3f(0, 0, 1);
				qglVertex3f(0, 0, 0);
				qglVertex3fv(up);
				qglVertex3fv(up);
				qglVertex3fv(boneOrigin);
			}
		}
		else
		{
			//parent = (mdsBone_t *) ((byte *) header + header->ofsBones + bone->parentIndex);
			parentMat = &boneMats[bone->parentIndex][0];
			parentOrigin = &boneOrigins[bone->parentIndex][0];
			
			boneOrigin[0] = parentOrigin[0] + offsetVec2[0];
			boneOrigin[1] = parentOrigin[1] + offsetVec2[1];
			boneOrigin[2] = parentOrigin[2] + offsetVec2[2];
			
			MatrixSetupTranslation(boneTranslationMat, boneOrigin[0], boneOrigin[1], boneOrigin[2]);
			MatrixMultiply(boneTranslationMat, boneRotationMat, boneMat);
			//MatrixMultiply(boneRotationMat, boneTranslationMat, boneMat);
			
			if(r_showSkeleton->integer)
			{
				// draw bone
				qglColor3f(1, 1, 1);
				qglVertex3fv(parentOrigin);
				//qglVertex3f(parentMat[12], parentMat[13], parentMat[14]);
				qglVertex3fv(boneOrigin);
				//qglVertex3f(boneMat[12], boneMat[13], boneMat[14]);
				
				// draw bone axis
				qglColor3f(1, 0, 0);
				qglVertex3fv(parentOrigin);
				VectorAdd(parentOrigin, forward, forward);
				qglVertex3fv(forward);
				qglVertex3fv(forward);
				qglVertex3fv(boneOrigin);
				
				qglColor3f(0, 1, 0);
				qglVertex3fv(parentOrigin);
				VectorAdd(parentOrigin, right, right);
				qglVertex3fv(right);
				qglVertex3fv(right);
				qglVertex3fv(boneOrigin);
				
				qglColor3f(0, 0, 1);
				qglVertex3fv(parentOrigin);
				VectorAdd(parentOrigin, up, up);
				qglVertex3fv(up);
				qglVertex3fv(up);
				qglVertex3fv(boneOrigin);
			}
		}
	}
	
	if(r_showSkeleton->integer)
	{
		qglEnd();
		qglLineWidth(1);
	}

	// deform the vertexes by the lerped bones
	if(!r_showSkeleton->integer)
	{
		numVertexes = surface->numVerts;
		v = (mdsVertex_t *) ((byte *) surface + surface->ofsVerts);
		for(j = 0; j < numVertexes; j++)
		{
			vec3_t          tmpVert;
			mdsWeight_t    *w;
	
			VectorClear(tmpVert);
			
			w = v->weights;
			for(k = 0; k < v->numWeights; k++, w++)
			{
				boneMat = &boneMats[w->boneIndex][0];
				boneOrigin = &boneOrigins[w->boneIndex][0];
	
				tmpVert[0] += ((DotProduct(boneMat + 0, w->offset) + boneMat[12]) * w->boneWeight);
				tmpVert[1] += ((DotProduct(boneMat + 4, w->offset) + boneMat[13]) * w->boneWeight);
				tmpVert[2] += ((DotProduct(boneMat + 8, w->offset) + boneMat[14]) * w->boneWeight);
			}
	
			tess.xyz[tess.numVertexes + j][0] = tmpVert[0];
			tess.xyz[tess.numVertexes + j][1] = tmpVert[1];
			tess.xyz[tess.numVertexes + j][2] = tmpVert[2];
			
			tess.texCoords[tess.numVertexes + j][0][0] = v->texCoords[0];
			tess.texCoords[tess.numVertexes + j][0][1] = v->texCoords[1];
	
			v = (mdsVertex_t *) &v->weights[v->numWeights];
		}
		
		// calc tangent spaces
		if(!tess.skipTangentSpaces)
		{
			int             i;
			vec3_t          faceNormal;
			float          *v;
			const float    *v0, *v1, *v2;
			const float    *t0, *t1, *t2;
			vec3_t          tangent;
			vec3_t          binormal;
			vec3_t          normal;
			int            *indices;
		
			for(i = 0; i < numVertexes; i++)
			{
				VectorClear(tess.tangents[tess.numVertexes + i]);
				VectorClear(tess.binormals[tess.numVertexes + i]);
				VectorClear(tess.normals[tess.numVertexes + i]);
			}
		
			for(i = 0, indices = tess.indexes + tess.numIndexes; i < numIndexes; i += 3, indices += 3)
			{
				v0 = tess.xyz[indices[0]];
				v1 = tess.xyz[indices[1]];
				v2 = tess.xyz[indices[2]];
		
				t0 = tess.texCoords[indices[0]][0];
				t1 = tess.texCoords[indices[1]][0];
				t2 = tess.texCoords[indices[2]][0];
		
				R_CalcNormalForTriangle(faceNormal, v0, v1, v2);
				R_CalcTangentSpace(tangent, binormal, normal, v0, v1, v2, t0, t1, t2, faceNormal);
		
				for(j = 0; j < 3; j++)
				{
					v = tess.tangents[indices[j]];
					VectorAdd(v, tangent, v);
					v = tess.binormals[indices[j]];
					VectorAdd(v, binormal, v);
					v = tess.normals[indices[j]];
					VectorAdd(v, normal, v);
				}
			}
		
			VectorArrayNormalize((vec4_t *) tess.tangents[tess.numVertexes], numVertexes);
			VectorArrayNormalize((vec4_t *) tess.binormals[tess.numVertexes], numVertexes);
			VectorArrayNormalize((vec4_t *) tess.normals[tess.numVertexes], numVertexes);
		}
	
		tess.numIndexes += numIndexes;
		tess.numVertexes += numVertexes;
	}
}


/*
==============
RB_SurfaceMD5
==============
*/
void RB_SurfaceMD5(md5Surface_t * surface)
{
	int             i, j, k;
	float           frontlerp, backlerp;
	int            *indexes;
	int             numIndexes = 0;
	int             numVertexes;
	md5Model_t     *model;
	md5Vertex_t    *v;
	md5Bone_t      *bone;
	vec3_t			offsetVec;

	if(backEnd.currentEntity->e.oldframe == backEnd.currentEntity->e.frame)
	{
		backlerp = 0;
		frontlerp = 1;
	}
	else
	{
		backlerp = backEnd.currentEntity->e.backlerp;
		frontlerp = 1.0f - backlerp;
	}
	
	model = surface->model;

//	frameSize = (int)(&((mdsFrame_t *) 0)->bones[header->numBones]);

//	frame = (mdsFrame_t *) ((byte *) header + header->ofsFrames + backEnd.currentEntity->e.frame * frameSize);
//	oldFrame = (mdsFrame_t *) ((byte *) header + header->ofsFrames + backEnd.currentEntity->e.oldframe * frameSize);

	RB_CheckOverflow(surface->numVerts, surface->numIndexes);
	
	indexes = surface->indexes;
	numIndexes = surface->numIndexes;
	for(j = 0; j < numIndexes; j++)
	{
		tess.indexes[tess.numIndexes + j] = tess.numVertexes + indexes[j];
	}

	// deform the vertexes by the lerped bones
	numVertexes = surface->numVerts;
	for(j = 0, v = surface->verts; j < numVertexes; j++, v++)
	{
		vec3_t          tmpVert;
		md5Weight_t    *w;

		VectorClear(tmpVert);
		
		for(k = 0, w = v->weights[0]; k < v->numWeights; k++, w++)
		{
			bone = &model->bones[w->boneIndex];
			
			if(backEnd.currentEntity->e.renderfx & RF_SKELETON)
			{
				MatrixTransformPoint(backEnd.currentEntity->e.bones[w->boneIndex].transform, w->offset, offsetVec);
			}
			else
			{
				MatrixTransformPoint(bone->transform, w->offset, offsetVec);
			}
			VectorMA(tmpVert, w->boneWeight, offsetVec, tmpVert);
		}

		tess.xyz[tess.numVertexes + j][0] = tmpVert[0];
		tess.xyz[tess.numVertexes + j][1] = tmpVert[1];
		tess.xyz[tess.numVertexes + j][2] = tmpVert[2];
		
		tess.texCoords[tess.numVertexes + j][0][0] = v->texCoords[0];
		tess.texCoords[tess.numVertexes + j][0][1] = v->texCoords[1];
	}
	
	// calc tangent spaces
	if(!tess.skipTangentSpaces)
	{
		int             i;
		vec3_t          faceNormal;
		float          *v;
		const float    *v0, *v1, *v2;
		const float    *t0, *t1, *t2;
		vec3_t          tangent;
		vec3_t          binormal;
		vec3_t          normal;
		int            *indices;
	
		for(i = 0; i < numVertexes; i++)
		{
			VectorClear(tess.tangents[tess.numVertexes + i]);
			VectorClear(tess.binormals[tess.numVertexes + i]);
			VectorClear(tess.normals[tess.numVertexes + i]);
		}
	
		for(i = 0, indices = tess.indexes + tess.numIndexes; i < numIndexes; i += 3, indices += 3)
		{
			v0 = tess.xyz[indices[0]];
			v1 = tess.xyz[indices[1]];
			v2 = tess.xyz[indices[2]];
	
			t0 = tess.texCoords[indices[0]][0];
			t1 = tess.texCoords[indices[1]][0];
			t2 = tess.texCoords[indices[2]][0];
	
			R_CalcNormalForTriangle(faceNormal, v0, v1, v2);
			R_CalcTangentSpace(tangent, binormal, normal, v0, v1, v2, t0, t1, t2, faceNormal);
	
			for(j = 0; j < 3; j++)
			{
				v = tess.tangents[indices[j]];
				VectorAdd(v, tangent, v);
				v = tess.binormals[indices[j]];
				VectorAdd(v, binormal, v);
				v = tess.normals[indices[j]];
				VectorAdd(v, normal, v);
			}
		}
	
		VectorArrayNormalize((vec4_t *) tess.tangents[tess.numVertexes], numVertexes);
		VectorArrayNormalize((vec4_t *) tess.binormals[tess.numVertexes], numVertexes);
		VectorArrayNormalize((vec4_t *) tess.normals[tess.numVertexes], numVertexes);
	}

	tess.numIndexes += numIndexes;
	tess.numVertexes += numVertexes;
}

/*
==============
RB_SurfaceFace
==============
*/
void RB_SurfaceFace(srfSurfaceFace_t * surf)
{
	int             i;
	unsigned       *indices, *tessIndexes;
	float          *v;
	int             ndx;
	int             Bob;
	int             numPoints;

	RB_CHECKOVERFLOW(surf->numPoints, surf->numIndices);

	indices = (unsigned *)(((char *)surf) + surf->ofsIndices);

	Bob = tess.numVertexes;
	tessIndexes = tess.indexes + tess.numIndexes;
	for(i = surf->numIndices - 1; i >= 0; i--)
	{
		tessIndexes[i] = indices[i] + Bob;
	}

	v = surf->points[0];

	ndx = tess.numVertexes;

	numPoints = surf->numPoints;

	for(i = 0, v = surf->points[0], ndx = tess.numVertexes; i < numPoints; i++, v += VERTEXSIZE, ndx++)
	{
		VectorCopy(v, tess.xyz[ndx]);
		tess.texCoords[ndx][0][0] = v[3];
		tess.texCoords[ndx][0][1] = v[4];
		tess.texCoords[ndx][1][0] = v[5];
		tess.texCoords[ndx][1][1] = v[6];
		*(unsigned int *)&tess.vertexColors[ndx] = *(unsigned int *)&v[7];
	}

	if(!tess.skipTangentSpaces)
	{
		for(i = 0, v = surf->points[0], ndx = tess.numVertexes; i < numPoints; i++, v += VERTEXSIZE, ndx++)
		{
			VectorCopy(&v[8], tess.tangents[ndx]);
		}
	
		for(i = 0, v = surf->points[0], ndx = tess.numVertexes; i < numPoints; i++, v += VERTEXSIZE, ndx++)
		{
			VectorCopy(&v[11], tess.binormals[ndx]);
		}
#if 1
		for(i = 0, v = surf->points[0], ndx = tess.numVertexes; i < numPoints; i++, v += VERTEXSIZE, ndx++)
		{
			VectorCopy(&v[14], tess.normals[ndx]);
		}
#else
		for(i = 0, ndx = tess.numVertexes; i < numPoints; i++, ndx++)
		{
			VectorCopy(surf->plane.normal, tess.normals[ndx]);
		}
#endif
	}

	tess.numIndexes += surf->numIndices;
	tess.numVertexes += surf->numPoints;
}


static float LodErrorForVolume(vec3_t local, float radius)
{
	vec3_t          world;
	float           d;

	// never let it go negative
	if(r_lodCurveError->value < 0)
	{
		return 0;
	}

	world[0] = local[0] * backEnd.or.axis[0][0] + local[1] * backEnd.or.axis[1][0] +
		local[2] * backEnd.or.axis[2][0] + backEnd.or.origin[0];
	world[1] = local[0] * backEnd.or.axis[0][1] + local[1] * backEnd.or.axis[1][1] +
		local[2] * backEnd.or.axis[2][1] + backEnd.or.origin[1];
	world[2] = local[0] * backEnd.or.axis[0][2] + local[1] * backEnd.or.axis[1][2] +
		local[2] * backEnd.or.axis[2][2] + backEnd.or.origin[2];

	VectorSubtract(world, backEnd.viewParms.or.origin, world);
	d = DotProduct(world, backEnd.viewParms.or.axis[0]);

	if(d < 0)
	{
		d = -d;
	}
	d -= radius;
	if(d < 1)
	{
		d = 1;
	}

	return r_lodCurveError->value / d;
}

/*
=============
RB_SurfaceGrid

Just copy the grid of points and triangulate
=============
*/
void RB_SurfaceGrid(srfGridMesh_t * cv)
{
	int             i, j;
	float          *xyz;
	float          *texCoords;
	float          *tangent;
	float          *binormal;
	float          *normal;
	unsigned char  *color;
	srfVert_t      *dv;
	int             rows, irows, vrows;
	int             used;
	int             widthTable[MAX_GRID_SIZE];
	int             heightTable[MAX_GRID_SIZE];
	float           lodError;
	int             lodWidth, lodHeight;
	int             numIndexes;
	int             numVertexes;

	// determine the allowable discrepance
	lodError = LodErrorForVolume(cv->lodOrigin, cv->lodRadius);

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
			vrows = (SHADER_MAX_VERTEXES - tess.numVertexes) / lodWidth;
			irows = (SHADER_MAX_INDEXES - tess.numIndexes) / (lodWidth * 6);

			// if we don't have enough space for at least one strip, flush the buffer
			if(vrows < 2 || irows < 1)
			{
				RB_EndSurface();
				RB_BeginSurface(tess.surfaceShader, tess.lightShader, tess.fogNum, tess.skipTangentSpaces);
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

		numVertexes = tess.numVertexes;

		xyz = tess.xyz[numVertexes];
		tangent = tess.tangents[numVertexes];
		binormal = tess.binormals[numVertexes];
		normal = tess.normals[numVertexes];
		texCoords = tess.texCoords[numVertexes][0];
		color = (unsigned char *)&tess.vertexColors[numVertexes];

		for(i = 0; i < rows; i++)
		{
			for(j = 0; j < lodWidth; j++)
			{
				dv = cv->verts + heightTable[used + i] * cv->width + widthTable[j];

				xyz[0] = dv->xyz[0];
				xyz[1] = dv->xyz[1];
				xyz[2] = dv->xyz[2];
				texCoords[0] = dv->st[0];
				texCoords[1] = dv->st[1];
				texCoords[2] = dv->lightmap[0];
				texCoords[3] = dv->lightmap[1];
				
				normal[0] = dv->normal[0];
				normal[1] = dv->normal[1];
				normal[2] = dv->normal[2];

				*(unsigned int *)color = *(unsigned int *)dv->color;
				xyz += 4;
				tangent += 4;
				binormal += 4;
				normal += 4;
				texCoords += 4;
				color += 4;
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
					v1 = tess.numVertexes + i * lodWidth + j + 1;
					v2 = v1 - 1;
					v3 = v2 + lodWidth;
					v4 = v3 + 1;

					tess.indexes[tess.numIndexes + numIndexes] = v2;
					tess.indexes[tess.numIndexes + numIndexes + 1] = v3;
					tess.indexes[tess.numIndexes + numIndexes + 2] = v1;

					tess.indexes[tess.numIndexes + numIndexes + 3] = v1;
					tess.indexes[tess.numIndexes + numIndexes + 4] = v3;
					tess.indexes[tess.numIndexes + numIndexes + 5] = v4;
					numIndexes += 6;
				}
			}
		}

		// Tr3B - calc tangent spaces
		if(!tess.skipTangentSpaces)
		{
			//vec3_t          faceNormal;
			float          *v;
			const float    *v0, *v1, *v2;
			const float    *t0, *t1, *t2;
			vec3_t          tangent;
			vec3_t          binormal;
			vec3_t          normal;
			int            *indices;

			for(i = 0; i < (rows * lodWidth); i++)
			{
				VectorClear(tess.tangents[tess.numVertexes + i]);
				VectorClear(tess.binormals[tess.numVertexes + i]);
				VectorClear(tess.normals[tess.numVertexes + i]);
			}

			for(i = 0, indices = tess.indexes + tess.numIndexes; i < numIndexes; i += 3, indices += 3)
			{
				v0 = tess.xyz[indices[0]];
				v1 = tess.xyz[indices[1]];
				v2 = tess.xyz[indices[2]];

				t0 = tess.texCoords[indices[0]][0];
				t1 = tess.texCoords[indices[1]][0];
				t2 = tess.texCoords[indices[2]][0];

				/*
				// compute the face normal based on vertex normals
				//VectorClear( faceNormal );
				//VectorAdd( faceNormal, tess.normals[indices[0]], faceNormal );
				//VectorAdd( faceNormal, tess.normals[indices[1]], faceNormal );
				//VectorAdd( faceNormal, tess.normals[indices[2]], faceNormal );

				VectorNormalize(faceNormal);
				*/

				R_CalcNormalForTriangle(normal, v0, v1, v2);
				R_CalcTangentsForTriangle(tangent, binormal, v0, v1, v2, t0, t1, t2);

				for(j = 0; j < 3; j++)
				{
					v = tess.tangents[indices[j]];
					VectorAdd(v, tangent, v);
					v = tess.binormals[indices[j]];
					VectorAdd(v, binormal, v);
					v = tess.normals[indices[j]];
					VectorAdd(v, normal, v);
				}
			}

			VectorArrayNormalize((vec4_t *) tess.tangents[tess.numVertexes], rows * lodWidth);
			VectorArrayNormalize((vec4_t *) tess.binormals[tess.numVertexes], rows * lodWidth);
			VectorArrayNormalize((vec4_t *) tess.normals[tess.numVertexes], rows * lodWidth);
		}

		tess.numIndexes += numIndexes;
		tess.numVertexes += rows * lodWidth;

		used += rows - 1;
	}
}


/*
===========================================================================

NULL MODEL

===========================================================================
*/

/*
===================
RB_SurfaceAxis

Draws x/y/z lines from the origin for orientation debugging
===================
*/
void RB_SurfaceAxis(void)
{
	GL_Program(0);
	GL_SelectTexture(0);
	GL_Bind(tr.whiteImage);
	qglLineWidth(3);
	qglBegin(GL_LINES);
	qglColor3f(1, 0, 0);
	qglVertex3f(0, 0, 0);
	qglVertex3f(16, 0, 0);
	qglColor3f(0, 1, 0);
	qglVertex3f(0, 0, 0);
	qglVertex3f(0, 16, 0);
	qglColor3f(0, 0, 1);
	qglVertex3f(0, 0, 0);
	qglVertex3f(0, 0, 16);
	qglEnd();
	qglLineWidth(1);
}

//===========================================================================

/*
====================
RB_SurfaceEntity

Entities that have a single procedurally generated surface
====================
*/
void RB_SurfaceEntity(surfaceType_t * surfType)
{
	switch (backEnd.currentEntity->e.reType)
	{
		case RT_SPRITE:
			RB_SurfaceSprite();
			break;
		case RT_BEAM:
			RB_SurfaceBeam();
			break;
		case RT_RAIL_CORE:
			RB_SurfaceRailCore();
			break;
		case RT_RAIL_RINGS:
			RB_SurfaceRailRings();
			break;
		case RT_LIGHTNING:
			RB_SurfaceLightningBolt();
			break;
		default:
			RB_SurfaceAxis();
			break;
	}
	return;
}

void RB_SurfaceBad(surfaceType_t * surfType)
{
	ri.Printf(PRINT_ALL, "Bad surface tesselated.\n");
}

#if 0

void RB_SurfaceFlare(srfFlare_t * surf)
{
	vec3_t          left, up;
	float           radius;
	byte            color[4];
	vec3_t          dir;
	vec3_t          origin;
	float           d;

	// calculate the xyz locations for the four corners
	radius = 30;
	VectorScale(backEnd.viewParms.or.axis[1], radius, left);
	VectorScale(backEnd.viewParms.or.axis[2], radius, up);
	if(backEnd.viewParms.isMirror)
	{
		VectorSubtract(vec3_origin, left, left);
	}

	color[0] = color[1] = color[2] = color[3] = 255;

	VectorMA(surf->origin, 3, surf->normal, origin);
	VectorSubtract(origin, backEnd.viewParms.or.origin, dir);
	VectorNormalize(dir);
	VectorMA(origin, r_ignore->value, dir, origin);

	d = -DotProduct(dir, surf->normal);
	if(d < 0)
	{
		return;
	}
#if 0
	color[0] *= d;
	color[1] *= d;
	color[2] *= d;
#endif

	RB_AddQuadStamp(origin, left, up, color);
}

#else

void RB_SurfaceFlare(srfFlare_t * surf)
{
#if 0
	vec3_t          left, up;
	byte            color[4];

	color[0] = surf->color[0] * 255;
	color[1] = surf->color[1] * 255;
	color[2] = surf->color[2] * 255;
	color[3] = 255;

	VectorClear(left);
	VectorClear(up);

	left[0] = r_ignore->value;

	up[1] = r_ignore->value;

	RB_AddQuadStampExt(surf->origin, left, up, color, 0, 0, 1, 1);
#endif
}

#endif



void RB_SurfaceDisplayList(srfDisplayList_t * surf)
{
	// all apropriate state must be set in RB_BeginSurface
	// this isn't implemented yet...
	qglCallList(surf->listNum);
}

void RB_SurfaceSkip(void *surf)
{
}


void	(*rb_surfaceTable[SF_NUM_SURFACE_TYPES]) (void *) =
{
		(void (*)(void *))RB_SurfaceBad,	// SF_BAD, 
		(void (*)(void *))RB_SurfaceSkip,	// SF_SKIP, 
		(void (*)(void *))RB_SurfaceFace,	// SF_FACE,
		(void (*)(void *))RB_SurfaceGrid,	// SF_GRID,
		(void (*)(void *))RB_SurfaceTriangles,	// SF_TRIANGLES,
		(void (*)(void *))RB_SurfacePolychain,	// SF_POLY,
		(void (*)(void *))RB_SurfaceMD3,	// SF_MD3,
		(void (*)(void *))RB_SurfaceMD4,	// SF_MD4,
		(void (*)(void *))RB_SurfaceMDS,	// SF_MDS,
		(void (*)(void *))RB_SurfaceMD5,	// SF_MD5,
		(void (*)(void *))RB_SurfaceFlare,	// SF_FLARE,
		(void (*)(void *))RB_SurfaceEntity,	// SF_ENTITY
		(void (*)(void *))RB_SurfaceDisplayList	// SF_DISPLAY_LIST
};
