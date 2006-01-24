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

  for a projection shadow:

  point[x] += light vector * ( z - shadow plane )
  point[y] +=
  point[z] = shadow plane

  1 0 light[x] / light[z]

*/

typedef struct
{
	int             i2;
	int             facing;
} edgeDef_t;

#define	MAX_EDGE_DEFS	32

static edgeDef_t edgeDefs[SHADER_MAX_VERTEXES][MAX_EDGE_DEFS];
static int      numEdgeDefs[SHADER_MAX_VERTEXES];
static int      facing[SHADER_MAX_INDEXES / 3];

void R_AddEdgeDef(int i1, int i2, int facing)
{
	int             c;

	c = numEdgeDefs[i1];
	if(c == MAX_EDGE_DEFS)
	{
		return;					// overflow
	}
	edgeDefs[i1][c].i2 = i2;
	edgeDefs[i1][c].facing = facing;

	numEdgeDefs[i1]++;
}

void R_RenderShadowEdges(void)
{
	int             i;

#if 0
	int             numTris;

	// dumb way -- render every triangle's edges
	numTris = tess.numIndexes / 3;

	for(i = 0; i < numTris; i++)
	{
		int             i1, i2, i3;

		if(!facing[i])
		{
			continue;
		}

		i1 = tess.indexes[i * 3 + 0];
		i2 = tess.indexes[i * 3 + 1];
		i3 = tess.indexes[i * 3 + 2];

		qglBegin(GL_TRIANGLE_STRIP);
		qglVertex3fv(tess.xyz[i1]);
		qglVertex3fv(tess.xyz[i1 + tess.numVertexes]);
		qglVertex3fv(tess.xyz[i2]);
		qglVertex3fv(tess.xyz[i2 + tess.numVertexes]);
		qglVertex3fv(tess.xyz[i3]);
		qglVertex3fv(tess.xyz[i3 + tess.numVertexes]);
		qglVertex3fv(tess.xyz[i1]);
		qglVertex3fv(tess.xyz[i1 + tess.numVertexes]);
		qglEnd();
		
		backEnd.pc.c_shadowVertexes += 8;
		backEnd.pc.c_shadowIndexes += 8;
		backEnd.pc.c_totalIndexes += 8;
	}
#else
	int             c, c2;
	int             j, k;
	int             i2;
	int             c_edges, c_rejected;
	int             hit[2];

	// an edge is NOT a silhouette edge if its face doesn't face the light,
	// or if it has a reverse paired edge that also faces the light.
	// A well behaved polyhedron would have exactly two faces for each edge,
	// but lots of models have dangling edges or overfanned edges
	c_edges = 0;
	c_rejected = 0;

	for(i = 0; i < tess.numVertexes; i++)
	{
		c = numEdgeDefs[i];
		for(j = 0; j < c; j++)
		{
			if(!edgeDefs[i][j].facing)
			{
				continue;
			}

			hit[0] = 0;
			hit[1] = 0;

			i2 = edgeDefs[i][j].i2;
			c2 = numEdgeDefs[i2];
			for(k = 0; k < c2; k++)
			{
				if(edgeDefs[i2][k].i2 == i)
				{
					hit[edgeDefs[i2][k].facing]++;
				}
			}

			// if it doesn't share the edge with another front facing
			// triangle, it is a sil edge
			if(hit[1] == 0)
			{
				qglBegin(GL_TRIANGLE_STRIP);
				qglVertex3fv(tess.xyz[i]);
				qglVertex4fv(tess.xyz[i + tess.numVertexes]);
				qglVertex3fv(tess.xyz[i2]);
				qglVertex4fv(tess.xyz[i2 + tess.numVertexes]);
				qglEnd();
				c_edges++;
				
				backEnd.pc.c_shadowVertexes += 4;
				backEnd.pc.c_shadowIndexes += 4;
				backEnd.pc.c_totalIndexes += 4;
			}
			else
			{
				c_rejected++;
			}
		}
	}
#endif
}

void R_RenderShadowCaps(qboolean front)
{
	int             i;
	int             numTris;

	numTris = tess.numIndexes / 3;

	for(i = 0; i < numTris; i++)
	{
		int             i1, i2, i3;

		if(!facing[i])
		{
			continue;
		}

		i1 = tess.indexes[i * 3 + 0];
		i2 = tess.indexes[i * 3 + 1];
		i3 = tess.indexes[i * 3 + 2];

		if(front)
		{
			qglBegin(GL_TRIANGLES);
			qglVertex3fv(tess.xyz[i1]);
			qglVertex3fv(tess.xyz[i2]);
			qglVertex3fv(tess.xyz[i3]);
			qglEnd();
		}
		else
		{
			qglBegin(GL_TRIANGLES);
			qglVertex4fv(tess.xyz[i3 + tess.numVertexes]);
			qglVertex4fv(tess.xyz[i2 + tess.numVertexes]);
			qglVertex4fv(tess.xyz[i1 + tess.numVertexes]);
			qglEnd();
		}
		
		backEnd.pc.c_shadowVertexes += 3;
		backEnd.pc.c_shadowIndexes += 3;
		backEnd.pc.c_totalIndexes += 3;
	}
}

/*
=================
RB_ShadowTessEnd

triangleFromEdge[ v1 ][ v2 ]


  set triangle from edge( v1, v2, tri )
  if ( facing[ triangleFromEdge[ v1 ][ v2 ] ] && !facing[ triangleFromEdge[ v2 ][ v1 ] ) {
  }
=================
*/
void RB_ShadowTessEnd(void)
{
	int             i;
	int             numTris;
	vec3_t          lightDir;

	// we can only do this if we have enough space in the vertex buffers
	if(tess.numVertexes >= SHADER_MAX_VERTEXES / 2)
	{
		return;
	}

	if(glConfig.stencilBits < 4)
	{
		return;
	}

	VectorCopy(backEnd.currentEntity->lightDir, lightDir);

	// project vertexes away from light direction
	for(i = 0; i < tess.numVertexes; i++)
	{
		VectorMA(tess.xyz[i], -512, lightDir, tess.xyz[i + tess.numVertexes]);

		// set w component to zero to work at infinity
		tess.xyz[i + tess.numVertexes][3] = 0;
	}

	// decide which triangles face the light
	Com_Memset(numEdgeDefs, 0, 4 * tess.numVertexes);

	numTris = tess.numIndexes / 3;
	for(i = 0; i < numTris; i++)
	{
		int             i1, i2, i3;
		vec3_t          d1, d2, normal;
		float          *v1, *v2, *v3;
		float           d;

		i1 = tess.indexes[i * 3 + 0];
		i2 = tess.indexes[i * 3 + 1];
		i3 = tess.indexes[i * 3 + 2];

		v1 = tess.xyz[i1];
		v2 = tess.xyz[i2];
		v3 = tess.xyz[i3];

		VectorSubtract(v2, v1, d1);
		VectorSubtract(v3, v1, d2);
		CrossProduct(d1, d2, normal);

		d = DotProduct(normal, lightDir);
		if(d > 0)
		{
			facing[i] = 1;
		}
		else
		{
			facing[i] = 0;
		}

		// create the edges
		R_AddEdgeDef(i1, i2, facing[i]);
		R_AddEdgeDef(i2, i3, facing[i]);
		R_AddEdgeDef(i3, i1, facing[i]);
	}

	// draw the silhouette edges
	GL_Program(0);
	GL_SelectTexture(0);
	GL_Bind(tr.whiteImage);

	if(r_showShadowVolumes->integer)
	{
		GL_State(GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);

		if(!backEnd.viewParms.isMirror)
		{
			qglColor4f(1.0f, 1.0f, 1.0f, 0.07f);
			R_RenderShadowCaps(qfalse);
			R_RenderShadowCaps(qtrue);
			
			qglColor4f(1.0f, 1.0f, 0.7f, 0.15f);
			R_RenderShadowEdges();
		}
	}
	else
	{
		if(backEnd.currentEntity->needZFail)
		{
			qglEnable(GL_CULL_FACE);
			GL_State(GLS_SRCBLEND_ONE | GLS_DSTBLEND_ZERO);
			qglColor3f(0.2f, 0.2f, 0.2f);

			// store current OpenGL state 
			qglPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_POLYGON_BIT | GL_STENCIL_BUFFER_BIT);

			// don't write to the color buffer or depth buffer
			qglColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
			qglDepthMask(GL_FALSE);

			qglEnable(GL_STENCIL_TEST);

			// set the reference stencil value
			qglStencilFunc(GL_ALWAYS, 0, ~0);

			// mirrors have the culling order reversed
			if(backEnd.viewParms.isMirror)
			{
				// draw only the back faces of the shadow volume
				qglCullFace(GL_BACK);

				// increment the stencil value on ZFail
				qglStencilOp(GL_KEEP, GL_INCR, GL_KEEP);

				R_RenderShadowCaps(qfalse);
				R_RenderShadowEdges();
				R_RenderShadowCaps(qtrue);

				// draw only the front faces of the shadow volume
				qglCullFace(GL_FRONT);

				// decrement the stencil value on ZFail
				qglStencilOp(GL_KEEP, GL_DECR, GL_KEEP);

				R_RenderShadowCaps(qfalse);
				R_RenderShadowEdges();
				R_RenderShadowCaps(qtrue);
			}
			else
			{
				// draw only the front faces of the shadow volume
				qglCullFace(GL_FRONT);

				// increment the stencil value on ZFail
				qglStencilOp(GL_KEEP, GL_INCR, GL_KEEP);

				R_RenderShadowCaps(qfalse);
				R_RenderShadowEdges();
				R_RenderShadowCaps(qtrue);

				// draw only the back faces of the shadow volume
				qglCullFace(GL_BACK);

				// decrement the stencil value on ZFail
				qglStencilOp(GL_KEEP, GL_DECR, GL_KEEP);

				R_RenderShadowCaps(qfalse);
				R_RenderShadowEdges();
				R_RenderShadowCaps(qtrue);
			}

			// reenable writing to the color buffer and depth buffer
			qglColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			qglDepthMask(GL_TRUE);

			// restore OpenGL state
			qglPopAttrib();
		}
		else
		{
			qglEnable(GL_CULL_FACE);
			GL_State(GLS_SRCBLEND_ONE | GLS_DSTBLEND_ZERO);
			qglColor3f(0.2f, 0.2f, 0.2f);

			// don't write to the color buffer
			qglColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

			qglEnable(GL_STENCIL_TEST);

			// set the reference stencil value
			qglStencilFunc(GL_ALWAYS, 0, ~0);

			// mirrors have the culling order reversed
			if(backEnd.viewParms.isMirror)
			{
				// draw only the front faces of the shadow volume
				qglCullFace(GL_FRONT);

				// increment the stencil value on ZPass
				qglStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

				R_RenderShadowEdges();

				// draw only the back faces of the shadow volume
				qglCullFace(GL_BACK);

				// decrement the stencil value on ZPass
				qglStencilOp(GL_KEEP, GL_KEEP, GL_DECR);

				R_RenderShadowEdges();
			}
			else
			{
				// draw only the back faces of the shadow volume
				qglCullFace(GL_BACK);

				// increment the stencil value on ZPass
				qglStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

				R_RenderShadowEdges();

				// draw only the front faces of the shadow volume
				qglCullFace(GL_FRONT);

				// decrement the stencil value on ZPass
				qglStencilOp(GL_KEEP, GL_KEEP, GL_DECR);

				R_RenderShadowEdges();
			}

			// reenable writing to the color buffer
			qglColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		}
	}
}


/*
=================
RB_ShadowTessEnd2

triangleFromEdge[ v1 ][ v2 ]


  set triangle from edge( v1, v2, tri )
  if ( facing[ triangleFromEdge[ v1 ][ v2 ] ] && !facing[ triangleFromEdge[ v2 ][ v1 ] ) {
  }
=================
*/
void RB_ShadowTessEnd2(void)
{
	int             i;
	int             numTris;
	vec3_t          lightDir;

	// we can only do this if we have enough space in the vertex buffers
	if(tess.numVertexes >= SHADER_MAX_VERTEXES / 2)
	{
		return;
	}

	if(glConfig.stencilBits < 4)
	{
		return;
	}

	VectorCopy(backEnd.currentLight->transformed, lightDir);
	VectorNormalize(lightDir);

	// project vertexes away from light direction
	for(i = 0; i < tess.numVertexes; i++)
	{
		VectorSubtract(tess.xyz[i], backEnd.currentLight->transformed, tess.xyz[i + tess.numVertexes]);

		// set w component to zero to work at infinity
		tess.xyz[i + tess.numVertexes][3] = 0;
	}

	// decide which triangles face the light
	Com_Memset(numEdgeDefs, 0, 4 * tess.numVertexes);

	numTris = tess.numIndexes / 3;
	for(i = 0; i < numTris; i++)
	{
		int             i1, i2, i3;
		vec3_t          d1, d2, normal;
		float          *v1, *v2, *v3;
		float           d;

		i1 = tess.indexes[i * 3 + 0];
		i2 = tess.indexes[i * 3 + 1];
		i3 = tess.indexes[i * 3 + 2];

		v1 = tess.xyz[i1];
		v2 = tess.xyz[i2];
		v3 = tess.xyz[i3];

		VectorSubtract(v2, v1, d1);
		VectorSubtract(v3, v1, d2);
		CrossProduct(d1, d2, normal);

		d = DotProduct(normal, lightDir);
		if(d > 0)
		{
			facing[i] = 1;
		}
		else
		{
			facing[i] = 0;
		}

		// create the edges
		R_AddEdgeDef(i1, i2, facing[i]);
		R_AddEdgeDef(i2, i3, facing[i]);
		R_AddEdgeDef(i3, i1, facing[i]);
	}

	// draw the silhouette edges
	GL_Program(0);
	GL_SelectTexture(0);
	GL_Bind(tr.whiteImage);

	if(r_showShadowVolumes->integer)
	{
		qglColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		qglDisable(GL_STENCIL_TEST);
		//qglDisable(GL_DEPTH_TEST);
		
		qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		if(!backEnd.viewParms.isMirror)
		{
			qglCullFace(GL_FRONT);

			qglColor4f(1.0f, 1.0f, 1.0f, 0.07f);
			R_RenderShadowCaps(qfalse);
			R_RenderShadowCaps(qtrue);
			
			qglColor4f(1.0f, 1.0f, 0.7f, 0.15f);
			R_RenderShadowEdges();
		}

		qglColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		qglEnable(GL_STENCIL_TEST);
		//qglEnable(GL_DEPTH_TEST);
	}
	else
	{
		if(backEnd.currentEntity->needZFail)
		{
			// draw only the front faces of the shadow volume
			if(!backEnd.viewParms.isMirror)
			{
				qglCullFace(GL_FRONT);
			}
			else
			{
				// mirrors have the culling order reversed
				qglCullFace(GL_BACK);
			}
	
			// increment the stencil value on zfail
			if(glConfig2.stencilWrapAvailable)
			{
				qglStencilOp(GL_KEEP, GL_INCR_WRAP_EXT, GL_KEEP);
			}
			else
			{
				qglStencilOp(GL_KEEP, GL_INCR, GL_KEEP);
			}
	
			R_RenderShadowEdges();
			R_RenderShadowCaps(qfalse);
			R_RenderShadowCaps(qtrue);
	
			// draw only the back faces of the shadow volume
			if(!backEnd.viewParms.isMirror)
			{
				qglCullFace(GL_BACK);
			}
			else
			{
				qglCullFace(GL_FRONT);
			}
	
			// decrement the stencil value on zfail
			if(glConfig2.stencilWrapAvailable)
			{
				qglStencilOp(GL_KEEP, GL_DECR_WRAP_EXT, GL_KEEP);
			}
			else
			{
				qglStencilOp(GL_KEEP, GL_DECR, GL_KEEP);
			}
	
			R_RenderShadowEdges();
			R_RenderShadowCaps(qfalse);
			R_RenderShadowCaps(qtrue);
		}
		else
		{
			// Tr3B - zpass rendering is cheaper because we can skip the lightcap and darkcap
			// see GPU Gems1 9.5.4
			
			// draw only the back faces of the shadow volume
			if(!backEnd.viewParms.isMirror)
			{
				qglCullFace(GL_BACK);
			}
			else
			{
				// mirrors have the culling order reversed
				qglCullFace(GL_FRONT);
			}
				
			// increment the stencil value on zpass
			if(glConfig2.stencilWrapAvailable)
			{
				qglStencilOp(GL_KEEP, GL_KEEP, GL_INCR_WRAP_EXT);
			}
			else
			{
				qglStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
			}
				
			R_RenderShadowEdges();
	
			// draw only the front faces of the shadow volume
			if(!backEnd.viewParms.isMirror)
			{
				qglCullFace(GL_FRONT);
			}
			else
			{
				qglCullFace(GL_BACK);
			}
	
			// decrement the stencil value on zpass
			if(glConfig2.stencilWrapAvailable)
			{
				qglStencilOp(GL_KEEP, GL_KEEP, GL_DECR_WRAP_EXT);
			}
			else
			{
				qglStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
			}
	
			R_RenderShadowEdges();
		}
	}
}


/*
=================
RB_ShadowFinish

Darken everything that is is a shadow volume.
We have to delay this until everything has been shadowed,
because otherwise shadows from different body parts would
overlap and double darken.
=================
*/
void RB_ShadowFinish(void)
{
	if(r_shadows->integer != 2)
	{
		return;
	}
	if(glConfig.stencilBits < 4)
	{
		return;
	}

	GLimp_LogComment("--- RB_ShadowFinish ---\n");

	qglEnable(GL_STENCIL_TEST);
	qglStencilFunc(GL_NOTEQUAL, 0, 255);

	qglDisable(GL_CLIP_PLANE0);
	qglDisable(GL_CULL_FACE);

	GL_Program(0);
	GL_ClientState(GLCS_VERTEX);
	GL_State(GLS_DEPTHMASK_TRUE | GLS_SRCBLEND_DST_COLOR | GLS_DSTBLEND_ZERO);

	GL_SelectTexture(0);
	GL_Bind(tr.whiteImage);

	qglLoadIdentity();

	qglColor3f(0.6f, 0.6f, 0.6f);

	qglBegin(GL_QUADS);
	qglVertex3f(-100, 100, -10);
	qglVertex3f(100, 100, -10);
	qglVertex3f(100, -100, -10);
	qglVertex3f(-100, -100, -10);
	qglEnd();

	qglColor4f(1, 1, 1, 1);
	qglDisable(GL_STENCIL_TEST);
}


/*
=================
RB_ProjectionShadowDeform
=================
*/
void RB_ProjectionShadowDeform(void)
{
	float          *xyz;
	int             i;
	float           h;
	vec3_t          ground;
	vec3_t          light;
	float           groundDist;
	float           d;
	vec3_t          lightDir;

	xyz = (float *)tess.xyz;

	ground[0] = backEnd.or.axis[0][2];
	ground[1] = backEnd.or.axis[1][2];
	ground[2] = backEnd.or.axis[2][2];

	groundDist = backEnd.or.origin[2] - backEnd.currentEntity->e.shadowPlane;

	VectorCopy(backEnd.currentEntity->lightDir, lightDir);
	d = DotProduct(lightDir, ground);
	// don't let the shadows get too long or go negative
	if(d < 0.5)
	{
		VectorMA(lightDir, (0.5 - d), ground, lightDir);
		d = DotProduct(lightDir, ground);
	}
	d = 1.0 / d;

	light[0] = lightDir[0] * d;
	light[1] = lightDir[1] * d;
	light[2] = lightDir[2] * d;

	for(i = 0; i < tess.numVertexes; i++, xyz += 4)
	{
		h = DotProduct(xyz, ground) + groundDist;

		xyz[0] -= light[0] * h;
		xyz[1] -= light[1] * h;
		xyz[2] -= light[2] * h;
	}
}
