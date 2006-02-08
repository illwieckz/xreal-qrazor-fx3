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

static edge_t   edges[SHADER_MAX_VERTEXES][MAX_EDGES];
static int      numEdges[SHADER_MAX_VERTEXES];
static qboolean facing[SHADER_MAX_INDEXES / 3];

static void R_AddEdgeDef(int i1, int i2, qboolean facing)
{
	int             c;

	c = numEdges[i1];
	if(c == MAX_EDGES)
	{
		return;					// overflow
	}
	edges[i1][c].i2 = i2;
	edges[i1][c].facing = facing;

	numEdges[i1]++;
}

static void R_RenderShadowEdges(void)
{
	if(tess.numShadowIndexes)
	{
		int             i;
		int             i1, i2;
		float          *v;
		
		for(i = 0; i < tess.numShadowIndexes; i += 2)
		{
			i1 = tess.shadowIndexes[i];
			i2 = tess.shadowIndexes[i + 1];
			
			qglBegin(GL_TRIANGLE_STRIP);
			
			v = tess.xyz[i1];
			qglVertex3fv(v);
			qglVertex4f(v[0], v[1], v[2], 0);
						
			v = tess.xyz[i2];
			qglVertex3fv(v);
			qglVertex4f(v[0], v[1], v[2], 0);
						
			qglEnd();
		}
	}
	else
	{
		int             i;
		int             c, c2;
		int             j, k;
		int             i2;
		int             c_edges, c_rejected;
		int             hit[2];
		float          *v;
	
		// an edge is NOT a silhouette edge if its face doesn't face the light,
		// or if it has a reverse paired edge that also faces the light.
		// A well behaved polyhedron would have exactly two faces for each edge,
		// but lots of models have dangling edges or overfanned edges
		c_edges = 0;
		c_rejected = 0;
	
		for(i = 0; i < tess.numVertexes; i++)
		{
			c = numEdges[i];
			for(j = 0; j < c; j++)
			{
				if(!edges[i][j].facing)
				{
					continue;
				}
	
				hit[0] = 0;
				hit[1] = 0;
	
				i2 = edges[i][j].i2;
				c2 = numEdges[i2];
				for(k = 0; k < c2; k++)
				{
					if(edges[i2][k].i2 == i)
					{
						hit[edges[i2][k].facing]++;
					}
				}
	
				// if it doesn't share the edge with another front facing
				// triangle, it is a sil edge
				if(hit[1] == 0)
				{
					qglBegin(GL_TRIANGLE_STRIP);
					
					v = tess.xyz[i];
					qglVertex3fv(v);
					qglVertex4f(v[0], v[1], v[2], 0);
					
					v = tess.xyz[i2];
					qglVertex3fv(v);
					qglVertex4f(v[0], v[1], v[2], 0);
					
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
	}
}

static void R_RenderShadowCaps(qboolean front)
{
	if(tess.numLightIndexes)
	{
		/*
		if(front)
		{
			R_DrawElements(tess.numLightIndexes, tess.lightIndexes);
			
			backEnd.pc.c_shadowVertexes += tess.numLightIndexes / 6;
			backEnd.pc.c_shadowIndexes += tess.numLightIndexes / 6;
			backEnd.pc.c_totalIndexes += tess.numLightIndexes / 6;
		}
		else
		*/
		{
			int             i;
			int             numTris;
			float          *v;
	
			numTris = tess.numLightIndexes / 3;
	
			for(i = 0; i < numTris; i++)
			{
				int             i1, i2, i3;
	
				i1 = tess.lightIndexes[i * 3 + 0];
				i2 = tess.lightIndexes[i * 3 + 1];
				i3 = tess.lightIndexes[i * 3 + 2];
	
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
				
					v = tess.xyz[i3];
					qglVertex4f(v[0], v[1], v[2], 0);
				
					v = tess.xyz[i2];
					qglVertex4f(v[0], v[1], v[2], 0);
				
					v = tess.xyz[i1];
					qglVertex4f(v[0], v[1], v[2], 0);
				
					qglEnd();
				}
			
				backEnd.pc.c_shadowVertexes += 3;
				backEnd.pc.c_shadowIndexes += 3;
				backEnd.pc.c_totalIndexes += 3;
			}
		}
	}
	else
	{
		
		int             i;
		int             numTris;
		float          *v;
	
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
				
				v = tess.xyz[i3];
				qglVertex4f(v[0], v[1], v[2], 0);
				
				v = tess.xyz[i2];
				qglVertex4f(v[0], v[1], v[2], 0);
				
				v = tess.xyz[i1];
				qglVertex4f(v[0], v[1], v[2], 0);
				
				qglEnd();
			}
			
			backEnd.pc.c_shadowVertexes += 3;
			backEnd.pc.c_shadowIndexes += 3;
			backEnd.pc.c_totalIndexes += 3;
		}
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
	if(!tess.numShadowIndexes)
	{
		int             i;
		int             numTris;
		vec3_t          lightOrigin;
	
		VectorCopy(backEnd.currentLight->transformed, lightOrigin);
	
		// decide which triangles face the light
		Com_Memset(numEdges, 0, 4 * tess.numVertexes);
	
		numTris = tess.numIndexes / 3;
		for(i = 0; i < numTris; i++)
		{
			int             i1, i2, i3;
			float          *v1, *v2, *v3;
			vec3_t          d1, d2;
			float           d;
			vec4_t          plane;
	
			i1 = tess.indexes[i * 3 + 0];
			i2 = tess.indexes[i * 3 + 1];
			i3 = tess.indexes[i * 3 + 2];
	
			v1 = tess.xyz[i1];
			v2 = tess.xyz[i2];
			v3 = tess.xyz[i3];
			
			if(PlaneFromPoints(plane, v1, v2, v3, qfalse))
			{
				d = DotProduct(plane, lightOrigin) - plane[3];
				if(d > 0)
				{
					facing[i] = qtrue;
				}
				else
				{
					facing[i] = qfalse;
				}
			}
			else
			{
				// FIXME triangle is degenerated!
				facing[i] = qfalse;
			}
	
			// create the edges
			R_AddEdgeDef(i1, i2, facing[i]);
			R_AddEdgeDef(i2, i3, facing[i]);
			R_AddEdgeDef(i3, i1, facing[i]);
		}
	}

	if(r_showShadowVolumes->integer)
	{
		qglColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		qglDisable(GL_CULL_FACE);
		qglDisable(GL_STENCIL_TEST);
		//qglDisable(GL_DEPTH_TEST);
		
		qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		qglColor4f(1.0f, 1.0f, 1.0f, 0.07f);
		R_RenderShadowCaps(qfalse);
		R_RenderShadowCaps(qtrue);
			
		qglColor4f(1.0f, 1.0f, 0.7f, 0.15f);
		R_RenderShadowEdges();

		qglColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		qglEnable(GL_CULL_FACE);
		qglEnable(GL_STENCIL_TEST);
		//qglEnable(GL_DEPTH_TEST);
	}
	else
	{
		if(backEnd.currentEntity->needZFail)
		{
			// mirrors have the culling order reversed
			if(backEnd.viewParms.isMirror)
				qglFrontFace(GL_CW);
			
			if(qglActiveStencilFaceEXT && glConfig2.stencilWrapAvailable)
			{
				// render both sides at once
				qglDisable(GL_CULL_FACE);
				
				qglEnable(GL_STENCIL_TEST_TWO_SIDE_EXT);
				
				qglActiveStencilFaceEXT(GL_BACK);
				qglStencilOp(GL_KEEP, GL_DECR_WRAP_EXT, GL_KEEP);
				qglStencilMask(~0);
				qglStencilFunc(GL_ALWAYS, 0, ~0);

				qglActiveStencilFaceEXT(GL_FRONT);
				qglStencilOp(GL_KEEP, GL_INCR_WRAP_EXT, GL_KEEP);
				qglStencilMask(~0);
				qglStencilFunc(GL_ALWAYS, 0, ~0);
				
				R_RenderShadowEdges();
				R_RenderShadowCaps(qfalse);
				R_RenderShadowCaps(qtrue);
				
				qglDisable(GL_STENCIL_TEST_TWO_SIDE_EXT);
				
				qglEnable(GL_CULL_FACE);
			}
			else
			{
			
				// draw only the front faces of the shadow volume
				qglCullFace(GL_FRONT);
		
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
				qglCullFace(GL_BACK);
		
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
			
			if(backEnd.viewParms.isMirror)
				qglFrontFace(GL_CCW);
		}
		else
		{
			// Tr3B - zpass rendering is cheaper because we can skip the lightcap and darkcap
			// see GPU Gems1 9.5.4
			
			// mirrors have the culling order reversed
			if(backEnd.viewParms.isMirror)
				qglFrontFace(GL_CW);
			
			if(qglActiveStencilFaceEXT && glConfig2.stencilWrapAvailable)
			{
				// render both sides at once
				qglDisable(GL_CULL_FACE);
				
				qglEnable(GL_STENCIL_TEST_TWO_SIDE_EXT);

				qglActiveStencilFaceEXT(GL_BACK);
				qglStencilOp(GL_KEEP, GL_KEEP, GL_INCR_WRAP_EXT);
				qglStencilMask(~0);
				qglStencilFunc(GL_ALWAYS, 0, ~0);

				qglActiveStencilFaceEXT(GL_FRONT);
				qglStencilOp(GL_KEEP, GL_KEEP, GL_DECR_WRAP_EXT);
				qglStencilMask(~0);
				qglStencilFunc(GL_ALWAYS, 0, ~0);
				
				R_RenderShadowEdges();
				
				qglDisable(GL_STENCIL_TEST_TWO_SIDE_EXT);
				
				qglEnable(GL_CULL_FACE);
			}
			else
			{
			
				// draw only the back faces of the shadow volume
				qglCullFace(GL_BACK);
					
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
				qglCullFace(GL_FRONT);
		
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
			
			if(backEnd.viewParms.isMirror)
				qglFrontFace(GL_CCW);
		}
	}
	
	// clear shader so we can tell we don't have any unclosed surfaces
	tess.numIndexes = 0;
	tess.numLightIndexes = 0;
	tess.numShadowIndexes = 0;
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
