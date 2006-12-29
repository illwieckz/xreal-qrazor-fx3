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
#include "tr_local.h"

backEndData_t  *backEndData[SMP_FRAMES];
backEndState_t  backEnd;



void GL_Bind(image_t * image)
{
	int             texnum;

	if(!image)
	{
		ri.Printf(PRINT_WARNING, "GL_Bind: NULL image\n");
		texnum = tr.defaultImage->texnum;
	}
	else
	{
		if(r_logFile->integer)
		{
			// don't just call LogComment, or we will get a call to va() every frame!
			GLimp_LogComment(va("--- GL_Bind( %s ) ---\n", image->name));
		}

		texnum = image->texnum;
	}

	if(r_nobind->integer && tr.dlightImage)
	{							// performance evaluation option
		texnum = tr.dlightImage->texnum;
	}

	if(glState.currenttextures[glState.currenttmu] != texnum)
	{
		image->frameUsed = tr.frameCount;
		glState.currenttextures[glState.currenttmu] = texnum;
		qglBindTexture(image->type, texnum);
	}
}

void GL_TextureFilter(image_t * image, filterType_t filterType)
{
	if(!image)
	{
		ri.Printf(PRINT_WARNING, "GL_TextureFilter: NULL image\n");
	}
	else
	{
		if(r_logFile->integer)
		{
			// don't just call LogComment, or we will get a call to va() every frame!
			GLimp_LogComment(va("--- GL_TextureFilter( %s ) ---\n", image->name));
		}
	}

	if(image->filterType == filterType)
		return;

	// set filter type
	switch (image->filterType)
	{
			/*
			   case FT_DEFAULT:
			   qglTexParameterf(image->type, GL_TEXTURE_MIN_FILTER, gl_filter_min);
			   qglTexParameterf(image->type, GL_TEXTURE_MAG_FILTER, gl_filter_max);

			   // set texture anisotropy
			   if(glConfig.textureAnisotropyAvailable)
			   qglTexParameterf(image->type, GL_TEXTURE_MAX_ANISOTROPY_EXT, r_ext_texture_filter_anisotropic->value);
			   break;
			 */

		case FT_LINEAR:
			qglTexParameterf(image->type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			qglTexParameterf(image->type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			break;

		case FT_NEAREST:
			qglTexParameterf(image->type, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			qglTexParameterf(image->type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			break;

		default:
			break;
	}
}

void GL_Program(GLhandleARB program)
{
	if(glConfig.shadingLanguage100Available)
	{
		if(glState.currentProgram != program)
		{
			glState.currentProgram = program;
			qglUseProgramObjectARB(program);
		}
	}
}

/*
** GL_SelectTexture
*/
void GL_SelectTexture(int unit)
{
	if(glState.currenttmu == unit)
	{
		return;
	}

	if(unit >= 0 && unit <= 7)
	{
		qglActiveTextureARB(GL_TEXTURE0_ARB + unit);
		qglClientActiveTextureARB(GL_TEXTURE0_ARB + unit);

		if(r_logFile->integer)
		{
			GLimp_LogComment(va("glActiveTextureARB( GL_TEXTURE%i_ARB )\n", unit));
			GLimp_LogComment(va("glClientActiveTextureARB( GL_TEXTURE%i_ARB )\n", unit));
		}
	}
	else
	{
		ri.Error(ERR_DROP, "GL_SelectTexture: unit = %i", unit);
	}

	glState.currenttmu = unit;
}


/*
** GL_BindMultitexture
*/
void GL_BindMultitexture(image_t * image0, GLuint env0, image_t * image1, GLuint env1)
{
	int             texnum0, texnum1;

	texnum0 = image0->texnum;
	texnum1 = image1->texnum;

	if(r_nobind->integer && tr.dlightImage)
	{							// performance evaluation option
		texnum0 = texnum1 = tr.dlightImage->texnum;
	}

	if(glState.currenttextures[1] != texnum1)
	{
		GL_SelectTexture(1);
		image1->frameUsed = tr.frameCount;
		glState.currenttextures[1] = texnum1;
		qglBindTexture(GL_TEXTURE_2D, texnum1);
	}
	if(glState.currenttextures[0] != texnum0)
	{
		GL_SelectTexture(0);
		image0->frameUsed = tr.frameCount;
		glState.currenttextures[0] = texnum0;
		qglBindTexture(GL_TEXTURE_2D, texnum0);
	}
}


/*
** GL_Cull
*/
void GL_Cull(int cullType)
{
	if(glState.faceCulling == cullType)
	{
		return;
	}

	glState.faceCulling = cullType;

	if(cullType == CT_TWO_SIDED)
	{
		qglDisable(GL_CULL_FACE);
	}
	else
	{
		qglEnable(GL_CULL_FACE);

		if(cullType == CT_BACK_SIDED)
		{
			if(backEnd.viewParms.isMirror)
			{
				qglCullFace(GL_FRONT);
			}
			else
			{
				qglCullFace(GL_BACK);
			}
		}
		else
		{
			if(backEnd.viewParms.isMirror)
			{
				qglCullFace(GL_BACK);
			}
			else
			{
				qglCullFace(GL_FRONT);
			}
		}
	}
}

/*
** GL_TexEnv
*/
void GL_TexEnv(int env)
{
	if(env == glState.texEnv[glState.currenttmu])
	{
		return;
	}

	glState.texEnv[glState.currenttmu] = env;


	switch (env)
	{
		case GL_MODULATE:
			qglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			break;
		case GL_REPLACE:
			qglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			break;
		case GL_DECAL:
			qglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
			break;
		case GL_ADD:
			qglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
			break;
		default:
			ri.Error(ERR_DROP, "GL_TexEnv: invalid env '%d' passed\n", env);
			break;
	}
}

/*
** GL_State
**
** This routine is responsible for setting the most commonly changed state
** in Q3.
*/
void GL_State(unsigned long stateBits)
{
	unsigned long   diff = stateBits ^ glState.glStateBits;

	if(!diff)
	{
		return;
	}

	// check depthFunc bits
	if(diff & GLS_DEPTHFUNC_BITS)
	{
		switch (stateBits & GLS_DEPTHFUNC_BITS)
		{
			case 0:
				qglDepthFunc(GL_LEQUAL);
				break;
			case GLS_DEPTHFUNC_LESS:
				qglDepthFunc(GL_LESS);
				break;
			case GLS_DEPTHFUNC_EQUAL:
				qglDepthFunc(GL_EQUAL);
				break;
			default:
				assert(0);
				break;
		}
	}

	// check blend bits
	if(diff & (GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS))
	{
		GLenum          srcFactor, dstFactor;

		if(stateBits & (GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS))
		{
			switch (stateBits & GLS_SRCBLEND_BITS)
			{
				case GLS_SRCBLEND_ZERO:
					srcFactor = GL_ZERO;
					break;
				case GLS_SRCBLEND_ONE:
					srcFactor = GL_ONE;
					break;
				case GLS_SRCBLEND_DST_COLOR:
					srcFactor = GL_DST_COLOR;
					break;
				case GLS_SRCBLEND_ONE_MINUS_DST_COLOR:
					srcFactor = GL_ONE_MINUS_DST_COLOR;
					break;
				case GLS_SRCBLEND_SRC_ALPHA:
					srcFactor = GL_SRC_ALPHA;
					break;
				case GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA:
					srcFactor = GL_ONE_MINUS_SRC_ALPHA;
					break;
				case GLS_SRCBLEND_DST_ALPHA:
					srcFactor = GL_DST_ALPHA;
					break;
				case GLS_SRCBLEND_ONE_MINUS_DST_ALPHA:
					srcFactor = GL_ONE_MINUS_DST_ALPHA;
					break;
				case GLS_SRCBLEND_ALPHA_SATURATE:
					srcFactor = GL_SRC_ALPHA_SATURATE;
					break;
				default:
					srcFactor = GL_ONE;	// to get warning to shut up
					ri.Error(ERR_DROP, "GL_State: invalid src blend state bits\n");
					break;
			}

			switch (stateBits & GLS_DSTBLEND_BITS)
			{
				case GLS_DSTBLEND_ZERO:
					dstFactor = GL_ZERO;
					break;
				case GLS_DSTBLEND_ONE:
					dstFactor = GL_ONE;
					break;
				case GLS_DSTBLEND_SRC_COLOR:
					dstFactor = GL_SRC_COLOR;
					break;
				case GLS_DSTBLEND_ONE_MINUS_SRC_COLOR:
					dstFactor = GL_ONE_MINUS_SRC_COLOR;
					break;
				case GLS_DSTBLEND_SRC_ALPHA:
					dstFactor = GL_SRC_ALPHA;
					break;
				case GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA:
					dstFactor = GL_ONE_MINUS_SRC_ALPHA;
					break;
				case GLS_DSTBLEND_DST_ALPHA:
					dstFactor = GL_DST_ALPHA;
					break;
				case GLS_DSTBLEND_ONE_MINUS_DST_ALPHA:
					dstFactor = GL_ONE_MINUS_DST_ALPHA;
					break;
				default:
					dstFactor = GL_ONE;	// to get warning to shut up
					ri.Error(ERR_DROP, "GL_State: invalid dst blend state bits\n");
					break;
			}

			qglEnable(GL_BLEND);
			qglBlendFunc(srcFactor, dstFactor);
		}
		else
		{
			qglDisable(GL_BLEND);
		}
	}

	// check colormask
	if(diff & GLS_COLORMASK_BITS)
	{
		if(stateBits & GLS_COLORMASK_BITS)
		{
			qglColorMask((stateBits & GLS_REDMASK_FALSE) ? GL_FALSE : GL_TRUE,
						 (stateBits & GLS_GREENMASK_FALSE) ? GL_FALSE : GL_TRUE,
						 (stateBits & GLS_BLUEMASK_FALSE) ? GL_FALSE : GL_TRUE,
						 (stateBits & GLS_ALPHAMASK_FALSE) ? GL_FALSE : GL_TRUE);
		}
		else
		{
			qglColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		}
	}

	// check depthmask
	if(diff & GLS_DEPTHMASK_TRUE)
	{
		if(stateBits & GLS_DEPTHMASK_TRUE)
		{
			qglDepthMask(GL_TRUE);
		}
		else
		{
			qglDepthMask(GL_FALSE);
		}
	}

	// fill/line mode
	if(diff & GLS_POLYMODE_LINE)
	{
		if(stateBits & GLS_POLYMODE_LINE)
		{
			qglPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else
		{
			qglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}

	// depthtest
	if(diff & GLS_DEPTHTEST_DISABLE)
	{
		if(stateBits & GLS_DEPTHTEST_DISABLE)
		{
			qglDisable(GL_DEPTH_TEST);
		}
		else
		{
			qglEnable(GL_DEPTH_TEST);
		}
	}

	// alpha test
	if(diff & GLS_ATEST_BITS)
	{
		switch (stateBits & GLS_ATEST_BITS)
		{
			case 0:
				qglDisable(GL_ALPHA_TEST);
				break;
			case GLS_ATEST_GT_0:
				qglEnable(GL_ALPHA_TEST);
				qglAlphaFunc(GL_GREATER, 0.0f);
				break;
			case GLS_ATEST_LT_80:
				qglEnable(GL_ALPHA_TEST);
				qglAlphaFunc(GL_LESS, 0.5f);
				break;
			case GLS_ATEST_GE_80:
				qglEnable(GL_ALPHA_TEST);
				qglAlphaFunc(GL_GEQUAL, 0.5f);
				break;
			case GLS_ATEST_GT_CUSTOM:
				// FIXME
				qglEnable(GL_ALPHA_TEST);
				qglAlphaFunc(GL_GREATER, 0.5f);
				break;
			default:
				assert(0);
				break;
		}
	}

	// stenciltest
	if(diff & GLS_STENCILTEST_ENABLE)
	{
		if(stateBits & GLS_STENCILTEST_ENABLE)
		{
			qglEnable(GL_STENCIL_TEST);
		}
		else
		{
			qglDisable(GL_STENCIL_TEST);
		}
	}

	glState.glStateBits = stateBits;
}

void GL_ClientState(unsigned long stateBits)
{
	unsigned long   diff = stateBits ^ glState.glClientStateBits;

	if(!diff)
	{
		return;
	}

	/*
	   if(diff & GLCS_VERTEX)
	   {
	   if(stateBits & GLCS_VERTEX)
	   {
	   qglEnableClientState(GL_VERTEX_ARRAY);
	   }
	   else
	   {
	   qglDisableClientState(GL_VERTEX_ARRAY);
	   }
	   }
	 */

	if(diff & GLCS_TEXCOORD0)
	{
		if(stateBits & GLCS_TEXCOORD0)
		{
			qglEnableVertexAttribArrayARB(ATTR_INDEX_TEXCOORD0);
		}
		else
		{
			qglDisableVertexAttribArrayARB(ATTR_INDEX_TEXCOORD0);
		}
	}

	if(diff & GLCS_TEXCOORD1)
	{
		if(stateBits & GLCS_TEXCOORD1)
		{
			qglEnableVertexAttribArrayARB(ATTR_INDEX_TEXCOORD1);
		}
		else
		{
			qglDisableVertexAttribArrayARB(ATTR_INDEX_TEXCOORD1);
		}
	}

	if(diff & GLCS_TEXCOORD2)
	{
		if(stateBits & GLCS_TEXCOORD2)
		{
			qglEnableVertexAttribArrayARB(ATTR_INDEX_TEXCOORD2);
		}
		else
		{
			qglDisableVertexAttribArrayARB(ATTR_INDEX_TEXCOORD2);
		}
	}

	if(diff & GLCS_TEXCOORD3)
	{
		if(stateBits & GLCS_TEXCOORD3)
		{
			qglEnableVertexAttribArrayARB(ATTR_INDEX_TEXCOORD3);
		}
		else
		{
			qglDisableVertexAttribArrayARB(ATTR_INDEX_TEXCOORD3);
		}
	}

	if(diff & GLCS_TANGENT)
	{
		if(stateBits & GLCS_TANGENT)
		{
			qglEnableVertexAttribArrayARB(ATTR_INDEX_TANGENT);
		}
		else
		{
			qglDisableVertexAttribArrayARB(ATTR_INDEX_TANGENT);
		}
	}

	if(diff & GLCS_BINORMAL)
	{
		if(stateBits & GLCS_BINORMAL)
		{
			qglEnableVertexAttribArrayARB(ATTR_INDEX_BINORMAL);
		}
		else
		{
			qglDisableVertexAttribArrayARB(ATTR_INDEX_BINORMAL);
		}
	}

	if(diff & GLCS_NORMAL)
	{
		if(stateBits & GLCS_NORMAL)
		{
			qglEnableClientState(GL_NORMAL_ARRAY);
		}
		else
		{
			qglDisableClientState(GL_NORMAL_ARRAY);
		}
	}

	if(diff & GLCS_COLOR)
	{
		if(stateBits & GLCS_COLOR)
		{
			qglEnableClientState(GL_COLOR_ARRAY);
		}
		else
		{
			qglDisableClientState(GL_COLOR_ARRAY);
		}
	}

	glState.glClientStateBits = stateBits;
}


void GL_SetVertexAttribs()
{
	//static GLuint   oldVertexesVBO = 0;

	if(glConfig.vertexBufferObjectAvailable && tess.vertexesVBO)
	{
		/*
		   if(oldVertexesVBO == tess.vertexesVBO)
		   {
		   // no update needed so avoid expensive glVertexPointer call
		   return;
		   }

		   oldVertexesVBO = tess.vertexesVBO;
		 */

		//if(glState.glClientStateBits & GLCS_VERTEX)
		//  qglVertexPointer(3, GL_FLOAT, 16, BUFFER_OFFSET(tess.ofsXYZ));

		if(glState.glClientStateBits & GLCS_TEXCOORD0)
			qglVertexAttribPointerARB(ATTR_INDEX_TEXCOORD0, 4, GL_FLOAT, 0, 0, BUFFER_OFFSET(tess.ofsTexCoords));

		if(glState.glClientStateBits & GLCS_TEXCOORD1)
			qglVertexAttribPointerARB(ATTR_INDEX_TEXCOORD1, 4, GL_FLOAT, 0, 0, BUFFER_OFFSET(tess.ofsTexCoords));

		if(glState.glClientStateBits & GLCS_TEXCOORD2)
			qglVertexAttribPointerARB(ATTR_INDEX_TEXCOORD2, 4, GL_FLOAT, 0, 0, BUFFER_OFFSET(tess.ofsTexCoords));

		if(glState.glClientStateBits & GLCS_TEXCOORD3)
			qglVertexAttribPointerARB(ATTR_INDEX_TEXCOORD3, 4, GL_FLOAT, 0, 0, BUFFER_OFFSET(tess.ofsTexCoords2));

		if(glState.glClientStateBits & GLCS_TANGENT)
			qglVertexAttribPointerARB(ATTR_INDEX_TANGENT, 3, GL_FLOAT, 0, 16, BUFFER_OFFSET(tess.ofsTangents));

		if(glState.glClientStateBits & GLCS_BINORMAL)
			qglVertexAttribPointerARB(ATTR_INDEX_BINORMAL, 3, GL_FLOAT, 0, 16, BUFFER_OFFSET(tess.ofsBinormals));

		if(glState.glClientStateBits & GLCS_NORMAL)
			qglNormalPointer(GL_FLOAT, 16, BUFFER_OFFSET(tess.ofsNormals));

		if(glState.glClientStateBits & GLCS_COLOR)
			qglColorPointer(4, GL_UNSIGNED_BYTE, 0, BUFFER_OFFSET(tess.ofsColors));
	}
	else
	{
		//if(glState.glClientStateBits & GLCS_VERTEX)
		//  qglVertexPointer(4, GL_FLOAT, 0, tess.xyz);

		if(glState.glClientStateBits & GLCS_TEXCOORD0)
			qglVertexAttribPointerARB(ATTR_INDEX_TEXCOORD0, 2, GL_FLOAT, 0, 0, tess.svars.texCoords[TB_COLORMAP]);

		if(glState.glClientStateBits & GLCS_TEXCOORD1)
		{
			if(tess.svars.skipCoords[TB_NORMALMAP])
				qglVertexAttribPointerARB(ATTR_INDEX_TEXCOORD1, 2, GL_FLOAT, 0, 0, tess.svars.texCoords[TB_DIFFUSEMAP]);
			else
				qglVertexAttribPointerARB(ATTR_INDEX_TEXCOORD1, 2, GL_FLOAT, 0, 0, tess.svars.texCoords[TB_NORMALMAP]);
		}

		if(glState.glClientStateBits & GLCS_TEXCOORD2)
		{
			if(tess.svars.skipCoords[TB_SPECULARMAP])
				qglVertexAttribPointerARB(ATTR_INDEX_TEXCOORD2, 2, GL_FLOAT, 0, 0, tess.svars.texCoords[TB_DIFFUSEMAP]);
			else
				qglVertexAttribPointerARB(ATTR_INDEX_TEXCOORD2, 2, GL_FLOAT, 0, 0, tess.svars.texCoords[TB_SPECULARMAP]);
		}

		if(glState.glClientStateBits & GLCS_TEXCOORD3)
			qglVertexAttribPointerARB(ATTR_INDEX_TEXCOORD3, 2, GL_FLOAT, 0, 0, tess.svars.texCoords[TB_LIGHTMAP]);

		if(glState.glClientStateBits & GLCS_TANGENT)
			qglVertexAttribPointerARB(ATTR_INDEX_TANGENT, 3, GL_FLOAT, 0, 16, tess.tangents);

		if(glState.glClientStateBits & GLCS_BINORMAL)
			qglVertexAttribPointerARB(ATTR_INDEX_BINORMAL, 3, GL_FLOAT, 0, 16, tess.binormals);

		if(glState.glClientStateBits & GLCS_NORMAL)
			qglNormalPointer(GL_FLOAT, 16, tess.normals);

		if(glState.glClientStateBits & GLCS_COLOR)
			qglColorPointer(4, GL_UNSIGNED_BYTE, 0, tess.svars.colors);
	}
}


/*
================
RB_Hyperspace

A player has predicted a teleport, but hasn't arrived yet
================
*/
static void RB_Hyperspace(void)
{
	float           c;

	if(!backEnd.isHyperspace)
	{
		// do initialization shit
	}

	c = (backEnd.refdef.time & 255) / 255.0f;
	qglClearColor(c, c, c, 1);
	qglClear(GL_COLOR_BUFFER_BIT);

	backEnd.isHyperspace = qtrue;
}


static void SetViewportAndScissor(void)
{
#if 0
	matrix_t        projectionMatrix;

	// convert from our coordinate system (looking down X)
	// to OpenGL's coordinate system (looking down -Z)
	MatrixMultiply(backEnd.viewParms.projectionMatrix, quakeToOpenGLMatrix, projectionMatrix);

	qglMatrixMode(GL_PROJECTION);
	qglLoadMatrixf(projectionMatrix);
	qglMatrixMode(GL_MODELVIEW);
#else
	qglMatrixMode(GL_PROJECTION);
	qglLoadMatrixf(backEnd.viewParms.projectionMatrix);
	qglMatrixMode(GL_MODELVIEW);
#endif

	// set the window clipping
	qglViewport(backEnd.viewParms.viewportX, backEnd.viewParms.viewportY,
				backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportHeight);

	qglScissor(backEnd.viewParms.viewportX, backEnd.viewParms.viewportY,
			   backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportHeight);
}


/*
================
RB_SetGL2D
================
*/
static void RB_SetGL2D(void)
{
	GLimp_LogComment("--- RB_SetGL2D ---\n");

	// disable offscreen rendering
	if(glConfig.framebufferObjectAvailable)
	{
		R_BindNullFBO();
	}

	backEnd.projection2D = qtrue;

	// set 2D virtual screen size
	qglViewport(0, 0, glConfig.vidWidth, glConfig.vidHeight);
	qglScissor(0, 0, glConfig.vidWidth, glConfig.vidHeight);
	qglMatrixMode(GL_PROJECTION);
	qglLoadIdentity();
	qglOrtho(0, glConfig.vidWidth, glConfig.vidHeight, 0, 0, 1);
	qglMatrixMode(GL_MODELVIEW);
	qglLoadIdentity();

	GL_State(GLS_DEPTHTEST_DISABLE | GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);

	qglDisable(GL_CULL_FACE);
	qglDisable(GL_CLIP_PLANE0);

	// set time for 2D shaders
	backEnd.refdef.time = ri.Milliseconds();
	backEnd.refdef.floatTime = backEnd.refdef.time * 0.001f;
}

/*
=================
RB_BeginDrawingView

Any mirrored or portaled views have already been drawn, so prepare
to actually render the visible surfaces for this view
=================
*/
static void RB_BeginDrawingView(void)
{
	int             clearBits = 0;

	GLimp_LogComment("--- RB_BeginDrawingView ---\n");

	// sync with gl if needed
	if(r_finish->integer == 1 && !glState.finishCalled)
	{
		qglFinish();
		glState.finishCalled = qtrue;
	}
	if(r_finish->integer == 0)
	{
		glState.finishCalled = qtrue;
	}

	// disable offscreen rendering
	if(glConfig.framebufferObjectAvailable)
	{
		R_BindNullFBO();
	}

	// we will need to change the projection matrix before drawing
	// 2D images again
	backEnd.projection2D = qfalse;

	// set the modelview matrix for the viewer
	SetViewportAndScissor();

	// ensures that depth writes are enabled for the depth clear
	GL_State(GLS_DEFAULT);

	// clear relevant buffers
	clearBits = GL_DEPTH_BUFFER_BIT;

	if(r_measureOverdraw->integer || r_shadows->integer == 3)
	{
		clearBits |= GL_STENCIL_BUFFER_BIT;
	}
	if(r_fastsky->integer && !(backEnd.refdef.rdflags & RDF_NOWORLDMODEL))
	{
		clearBits |= GL_COLOR_BUFFER_BIT;	// FIXME: only if sky shaders have been used
#ifdef _DEBUG
		qglClearColor(0.0f, 0.0f, 1.0f, 1.0f);	// FIXME: get color of sky
#else
		qglClearColor(0.0f, 0.0f, 0.0f, 1.0f);	// FIXME: get color of sky
#endif
	}
	qglClear(clearBits);

	if((backEnd.refdef.rdflags & RDF_HYPERSPACE))
	{
		RB_Hyperspace();
		return;
	}
	else
	{
		backEnd.isHyperspace = qfalse;
	}

	glState.faceCulling = -1;	// force face culling to set next time

	// we will only draw a sun if there was sky rendered in this view
	backEnd.skyRenderedThisView = qfalse;

	// clip to the plane of the portal
	if(backEnd.viewParms.isPortal)
	{
		float           plane[4];
		double          plane2[4];

		plane[0] = backEnd.viewParms.portalPlane.normal[0];
		plane[1] = backEnd.viewParms.portalPlane.normal[1];
		plane[2] = backEnd.viewParms.portalPlane.normal[2];
		plane[3] = backEnd.viewParms.portalPlane.dist;

		plane2[0] = DotProduct(backEnd.viewParms.or.axis[0], plane);
		plane2[1] = DotProduct(backEnd.viewParms.or.axis[1], plane);
		plane2[2] = DotProduct(backEnd.viewParms.or.axis[2], plane);
		plane2[3] = DotProduct(plane, backEnd.viewParms.or.origin) - plane[3];

//      qglLoadIdentity();
		qglLoadMatrixf(quakeToOpenGLMatrix);
		qglClipPlane(GL_CLIP_PLANE0, plane2);
		qglEnable(GL_CLIP_PLANE0);
	}
	else
	{
		qglDisable(GL_CLIP_PLANE0);
	}

	// check for offscreen rendering
	/*
	   if(glConfig.shadingLanguage100Available && glConfig.framebufferObjectAvailable)
	   {
	   if(backEnd.viewParms.isPortal)
	   {
	   R_BindFBO(tr.portalRenderFBO);
	   }
	   else
	   {
	   R_BindFBO(tr.currentRenderFBO);
	   }

	   // clear relevant buffers
	   clearBits = GL_DEPTH_BUFFER_BIT;

	   // FIXME: GL_STENCIL_BUFFER_BIT with FBOs
	   if(r_measureOverdraw->integer || r_shadows->integer == 3)
	   {
	   clearBits |= GL_STENCIL_BUFFER_BIT;
	   }

	   if(r_fastsky->integer && !(backEnd.refdef.rdflags & RDF_NOWORLDMODEL))
	   {
	   clearBits |= GL_COLOR_BUFFER_BIT;    // FIXME: only if sky shaders have been used
	   }
	   qglClear(clearBits);
	   }
	 */

	GL_CheckErrors();
}

static void RB_RenderDrawSurfaces(float originalTime, drawSurf_t * drawSurfs, int numDrawSurfs, qboolean opaque)
{
	trRefEntity_t  *entity, *oldEntity;
	int             lightmapNum, oldLightmapNum;
	shader_t       *shader, *oldShader;
	int             fogNum, oldFogNum;
	qboolean        depthRange, oldDepthRange;
	int             i;
	drawSurf_t     *drawSurf;
	int             oldSort;

	GLimp_LogComment("--- RB_RenderDrawSurfaces ---\n");

	// draw everything
	oldEntity = NULL;
	oldShader = NULL;
	oldLightmapNum = -1;
	oldFogNum = -1;
	oldDepthRange = qfalse;
	oldSort = -1;
	depthRange = qfalse;

	for(i = 0, drawSurf = drawSurfs; i < numDrawSurfs; i++, drawSurf++)
	{
		// update locals
		entity = drawSurf->entity;
		shader = tr.sortedShaders[drawSurf->shaderNum];
		lightmapNum = drawSurf->lightmapNum;
		fogNum = drawSurf->fogNum;

		if(opaque)
		{
			// skip all translucent surfaces that don't matter for this pass
			if(shader->sort > SS_OPAQUE)
			{
				break;
			}
		}
		else
		{
			// skip all opaque surfaces that don't matter for this pass
			if(shader->sort <= SS_OPAQUE)
			{
				continue;
			}
		}

		if(entity == oldEntity && shader == oldShader && lightmapNum == oldLightmapNum && fogNum == oldFogNum)
		{
			// fast path, same as previous sort
			rb_surfaceTable[*drawSurf->surface] (drawSurf->surface, 0, NULL, 0, NULL);
			continue;
		}

		// change the tess parameters if needed
		// a "entityMergable" shader is a shader that can have surfaces from seperate
		// entities merged into a single batch, like smoke and blood puff sprites
		if(shader != oldShader || fogNum != oldFogNum || lightmapNum != oldLightmapNum ||
		   (entity != oldEntity && !shader->entityMergable))
		{
			if(oldShader != NULL)
			{
				Tess_End();
			}

			Tess_Begin(Tess_StageIteratorGeneric, shader, NULL, lightmapNum, fogNum, qfalse, qfalse);
			oldShader = shader;
			oldLightmapNum = lightmapNum;
			oldFogNum = fogNum;
		}

		// change the modelview matrix if needed
		if(entity != oldEntity)
		{
			depthRange = qfalse;

			if(entity != &tr.worldEntity)
			{
				backEnd.currentEntity = entity;
				backEnd.refdef.floatTime = originalTime - backEnd.currentEntity->e.shaderTime;

				// we have to reset the shaderTime as well otherwise image animations start
				// from the wrong frame
				tess.shaderTime = backEnd.refdef.floatTime - tess.surfaceShader->timeOffset;

				// set up the transformation matrix
				R_RotateEntityForViewParms(backEnd.currentEntity, &backEnd.viewParms, &backEnd.or);

				if(backEnd.currentEntity->e.renderfx & RF_DEPTHHACK)
				{
					// hack the depth range to prevent view model from poking into walls
					depthRange = qtrue;
				}
			}
			else
			{
				backEnd.currentEntity = &tr.worldEntity;
				backEnd.refdef.floatTime = originalTime;
				backEnd.or = backEnd.viewParms.world;

				// we have to reset the shaderTime as well otherwise image animations on
				// the world (like water) continue with the wrong frame
				tess.shaderTime = backEnd.refdef.floatTime - tess.surfaceShader->timeOffset;
			}

			qglLoadMatrixf(backEnd.or.modelViewMatrix);

			// change depthrange if needed
			if(oldDepthRange != depthRange)
			{
				if(depthRange)
				{
					qglDepthRange(0, 0.3);
				}
				else
				{
					qglDepthRange(0, 1);
				}
				oldDepthRange = depthRange;
			}

			oldEntity = entity;
		}

		// add the triangles for this surface
		rb_surfaceTable[*drawSurf->surface] (drawSurf->surface, 0, NULL, 0, NULL);
	}

	backEnd.refdef.floatTime = originalTime;

	// draw the contents of the last shader batch
	if(oldShader != NULL)
	{
		Tess_End();
	}

	// go back to the world modelview matrix
	qglLoadMatrixf(backEnd.viewParms.world.modelViewMatrix);
	if(depthRange)
	{
		qglDepthRange(0, 1);
	}

	GL_CheckErrors();
}

/*
=================
RB_RenderInteractions
=================
*/
static void RB_RenderInteractions(float originalTime, interaction_t * interactions, int numInteractions)
{
	shader_t       *shader, *oldShader;
	trRefEntity_t  *entity, *oldEntity;
	trRefLight_t   *light, *oldLight;
	interaction_t  *ia;
	qboolean        depthRange, oldDepthRange;
	int             iaCount;
	surfaceType_t  *surface;
	vec3_t          tmp;
	matrix_t        modelToLight;

	GLimp_LogComment("--- RB_RenderInteractions ---\n");

	// draw everything
	oldLight = NULL;
	oldEntity = NULL;
	oldShader = NULL;
	oldDepthRange = qfalse;
	depthRange = qfalse;

	// render interactions
	for(iaCount = 0, ia = &interactions[0]; iaCount < numInteractions;)
	{
		backEnd.currentLight = light = ia->light;
		backEnd.currentEntity = entity = ia->entity;
		surface = ia->surface;
		shader = ia->surfaceShader;

		/*
		   if(glConfig.occlusionQueryBits && !ia->occlusionQuerySamples)
		   {
		   // skip all interactions of this light because it failed the occlusion query
		   goto skipInteraction;
		   }
		 */

		if(!shader->interactLight)
		{
			// skip this interaction because the surface shader has no ability to interact with light
			// this will save texcoords and matrix calculations
			goto skipInteraction;
		}

		if(ia->type == IA_SHADOWONLY)
		{
			// skip this interaction because the interaction is meant for shadowing only
			goto skipInteraction;
		}

		if(light != oldLight)
		{
			GLimp_LogComment("----- Rendering new light -----\n");

			// set light scissor to reduce fillrate
			qglScissor(ia->scissorX, ia->scissorY, ia->scissorWidth, ia->scissorHeight);
		}

		// Tr3B: this should never happen in the first iteration
		if(light == oldLight && entity == oldEntity && shader == oldShader)
		{
			// fast path, same as previous
			rb_surfaceTable[*surface] (surface, ia->numLightIndexes, ia->lightIndexes, 0, NULL);
			goto nextInteraction;
		}

		// draw the contents of the last shader batch
		Tess_End();

		// begin a new batch
		Tess_Begin(Tess_StageIteratorLighting, shader, ia->lightShader, -1, 0, qfalse, qfalse);

		// change the modelview matrix if needed
		if(entity != oldEntity)
		{
			depthRange = qfalse;

			if(entity != &tr.worldEntity)
			{
				backEnd.refdef.floatTime = originalTime - backEnd.currentEntity->e.shaderTime;
				// we have to reset the shaderTime as well otherwise image animations start
				// from the wrong frame
				tess.shaderTime = backEnd.refdef.floatTime - tess.surfaceShader->timeOffset;

				// set up the transformation matrix
				R_RotateEntityForViewParms(backEnd.currentEntity, &backEnd.viewParms, &backEnd.or);

				if(backEnd.currentEntity->e.renderfx & RF_DEPTHHACK)
				{
					// hack the depth range to prevent view model from poking into walls
					depthRange = qtrue;
				}
			}
			else
			{
				backEnd.refdef.floatTime = originalTime;
				backEnd.or = backEnd.viewParms.world;
				// we have to reset the shaderTime as well otherwise image animations on
				// the world (like water) continue with the wrong frame
				tess.shaderTime = backEnd.refdef.floatTime - tess.surfaceShader->timeOffset;
			}

			qglLoadMatrixf(backEnd.or.modelViewMatrix);

			// change depthrange if needed
			if(oldDepthRange != depthRange)
			{
				if(depthRange)
				{
					qglDepthRange(0, 0.3);
				}
				else
				{
					qglDepthRange(0, 1);
				}
				oldDepthRange = depthRange;
			}
		}

		// change the attenuation matrix if needed
		if(light != oldLight || entity != oldEntity)
		{
			// transform light origin into model space for u_LightOrigin parameter
			if(entity != &tr.worldEntity)
			{
				VectorSubtract(light->origin, backEnd.or.origin, tmp);
				light->transformed[0] = DotProduct(tmp, backEnd.or.axis[0]);
				light->transformed[1] = DotProduct(tmp, backEnd.or.axis[1]);
				light->transformed[2] = DotProduct(tmp, backEnd.or.axis[2]);
			}
			else
			{
				VectorCopy(light->origin, light->transformed);
			}

			// build the attenuation matrix using the entity transform
			MatrixMultiply(light->viewMatrix, backEnd.or.transformMatrix, modelToLight);

			MatrixSetupTranslation(light->attenuationMatrix, 0.5, 0.5, 0.5);	// bias
			MatrixMultiplyScale(light->attenuationMatrix, 0.5, 0.5, 0.5);	// scale
			MatrixMultiply2(light->attenuationMatrix, light->projectionMatrix);	// light projection (frustum)
			MatrixMultiply2(light->attenuationMatrix, modelToLight);
		}

		// add the triangles for this surface
		rb_surfaceTable[*surface] (surface, ia->numLightIndexes, ia->lightIndexes, 0, NULL);

	  nextInteraction:

		// remember values
		oldLight = light;
		oldEntity = entity;
		oldShader = shader;

	  skipInteraction:
		if(!ia->next)
		{
			// draw the contents of the last shader batch
			Tess_End();

			if(iaCount < (numInteractions - 1))
			{
				// jump to next interaction and continue
				ia++;
				iaCount++;
			}
			else
			{
				// increase last time to leave for loop
				iaCount++;
			}

			// force updates
			oldLight = NULL;
			oldEntity = NULL;
			oldShader = NULL;
		}
		else
		{
			// just continue
			ia = ia->next;
			iaCount++;
		}
	}

	backEnd.refdef.floatTime = originalTime;

	// go back to the world modelview matrix
	qglLoadMatrixf(backEnd.viewParms.world.modelViewMatrix);
	if(depthRange)
	{
		qglDepthRange(0, 1);
	}

	// reset scissor
	qglScissor(backEnd.viewParms.viewportX, backEnd.viewParms.viewportY,
			   backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportHeight);

	GL_CheckErrors();
}


/*
=================
RB_RenderInteractionsStencilShadowed
=================
*/
static void RB_RenderInteractionsStencilShadowed(float originalTime, interaction_t * interactions, int numInteractions)
{
	shader_t       *shader, *oldShader;
	trRefEntity_t  *entity, *oldEntity;
	trRefLight_t   *light, *oldLight;
	interaction_t  *ia;
	int             iaCount;
	int             iaFirst;
	surfaceType_t  *surface;
	qboolean        depthRange, oldDepthRange;
	vec3_t          tmp;
	matrix_t        modelToLight;
	qboolean        drawShadows;

	if(glConfig.stencilBits < 4 || !glConfig.shadingLanguage100Available)
	{
		RB_RenderInteractions(originalTime, interactions, numInteractions);
		return;
	}

	GLimp_LogComment("--- RB_RenderInteractionsStencilShadowed ---\n");

	// draw everything
	oldLight = NULL;
	oldEntity = NULL;
	oldShader = NULL;
	oldDepthRange = qfalse;
	depthRange = qfalse;
	drawShadows = qtrue;

	/*
	   if(qglActiveStencilFaceEXT)
	   {
	   qglEnable(GL_STENCIL_TEST_TWO_SIDE_EXT);
	   }
	 */

	// render interactions
	for(iaCount = 0, iaFirst = 0, ia = &interactions[0]; iaCount < numInteractions;)
	{
		backEnd.currentLight = light = ia->light;
		backEnd.currentEntity = entity = ia->entity;
		surface = ia->surface;
		shader = ia->surfaceShader;

		// only iaCount == iaFirst if first iteration or counters were reset
		if(iaCount == iaFirst)
		{
			if(r_logFile->integer)
			{
				// don't just call LogComment, or we will get
				// a call to va() every frame!
				GLimp_LogComment(va("----- First Interaction: %i -----\n", iaCount));
			}

			if(drawShadows)
			{
				// set light scissor to reduce fillrate
				qglScissor(ia->scissorX, ia->scissorY, ia->scissorWidth, ia->scissorHeight);

				// set depth test to reduce fillrate
				if(qglDepthBoundsEXT)
				{
					if(!ia->noDepthBoundsTest)
					{
						qglEnable(GL_DEPTH_BOUNDS_TEST_EXT);
						qglDepthBoundsEXT(ia->depthNear, ia->depthFar);
					}
					else
					{
						qglDisable(GL_DEPTH_BOUNDS_TEST_EXT);
					}
				}

				if(!light->l.noShadows)
				{
					GLimp_LogComment("--- Rendering shadow volumes ---\n");

					// set the reference stencil value
					qglClearStencil(0);

					// reset stencil buffer
					qglClear(GL_STENCIL_BUFFER_BIT);

					// use less compare as depthfunc
					// don't write to the color buffer or depth buffer
					// enable stencil testing for this light
					GL_State(GLS_DEPTHFUNC_LESS | GLS_COLORMASK_BITS | GLS_STENCILTEST_ENABLE);

					qglStencilFunc(GL_ALWAYS, 0, 255);
					qglStencilMask(255);

					//qglEnable(GL_POLYGON_OFFSET_FILL);
					//qglPolygonOffset(r_shadowOffsetFactor->value, r_shadowOffsetUnits->value);

					// enable shadow volume extrusion shader
					GL_Program(tr.shadowExtrudeShader.program);
					GL_ClientState(tr.shadowExtrudeShader.attribs);
				}
			}
			else
			{
				GLimp_LogComment("--- Rendering lighting ---\n");

				if(!light->l.noShadows)
				{
					qglStencilFunc(GL_EQUAL, 0, 255);
				}
				else
				{
					// don't consider shadow volumes
					qglStencilFunc(GL_ALWAYS, 0, 255);
				}

				/*
				   if(qglActiveStencilFaceEXT)
				   {
				   qglActiveStencilFaceEXT(GL_BACK);
				   qglStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

				   qglActiveStencilFaceEXT(GL_FRONT);
				   qglStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
				   }
				   else
				 */
				{
					qglStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
				}

				//qglDisable(GL_POLYGON_OFFSET_FILL);

				// disable shadow volume extrusion shader
				GL_Program(0);
			}
		}

		if(drawShadows)
		{
			if(entity->e.renderfx & (RF_NOSHADOW | RF_DEPTHHACK))
			{
				goto skipInteraction;
			}

			if(shader->sort > SS_OPAQUE)
			{
				goto skipInteraction;
			}

			if(shader->noShadows || light->l.noShadows)
			{
				goto skipInteraction;
			}

			if(ia->type == IA_LIGHTONLY)
			{
				goto skipInteraction;
			}
		}
		else
		{
			if(!shader->interactLight)
			{
				goto skipInteraction;
			}

			if(ia->type == IA_SHADOWONLY)
			{
				goto skipInteraction;
			}
		}

		if(drawShadows)
		{
			if(light == oldLight && entity == oldEntity && shader == oldShader)
			{
				if(r_logFile->integer)
				{
					// don't just call LogComment, or we will get
					// a call to va() every frame!
					GLimp_LogComment(va("----- Batching Shadow Interaction: %i -----\n", iaCount));
				}

				// fast path, same as previous
				rb_surfaceTable[*surface] (surface, 0, NULL, ia->numShadowIndexes, ia->shadowIndexes);
				goto nextInteraction;
			}
			else
			{
				if(oldLight)
				{
					// draw the contents of the last shader batch
					Tess_End();
				}

				if(r_logFile->integer)
				{
					// don't just call LogComment, or we will get
					// a call to va() every frame!
					GLimp_LogComment(va("----- Beginning Shadow Interaction: %i -----\n", iaCount));
				}

				// we don't need tangent space calculations here
				Tess_Begin(Tess_StageIteratorStencilShadowVolume, shader, ia->lightShader, -1, 0, qtrue, qtrue);
			}
		}
		else
		{
			if(light == oldLight && entity == oldEntity && shader == oldShader)
			{
				if(r_logFile->integer)
				{
					// don't just call LogComment, or we will get
					// a call to va() every frame!
					GLimp_LogComment(va("----- Batching Light Interaction: %i -----\n", iaCount));
				}

				// fast path, same as previous
				rb_surfaceTable[*surface] (surface, ia->numLightIndexes, ia->lightIndexes, 0, NULL);
				goto nextInteraction;
			}
			else
			{
				if(oldLight)
				{
					// draw the contents of the last shader batch
					Tess_End();
				}

				if(r_logFile->integer)
				{
					// don't just call LogComment, or we will get
					// a call to va() every frame!
					GLimp_LogComment(va("----- Beginning Light Interaction: %i -----\n", iaCount));
				}

				// begin a new batch
				Tess_Begin(Tess_StageIteratorStencilLighting, shader, ia->lightShader, -1, 0, qfalse, qfalse);
			}
		}

		// change the modelview matrix if needed
		if(entity != oldEntity)
		{
			depthRange = qfalse;

			if(entity != &tr.worldEntity)
			{
				backEnd.refdef.floatTime = originalTime - backEnd.currentEntity->e.shaderTime;
				// we have to reset the shaderTime as well otherwise image animations start
				// from the wrong frame
				tess.shaderTime = backEnd.refdef.floatTime - tess.surfaceShader->timeOffset;

				// set up the transformation matrix
				R_RotateEntityForViewParms(backEnd.currentEntity, &backEnd.viewParms, &backEnd.or);

				if(backEnd.currentEntity->e.renderfx & RF_DEPTHHACK)
				{
					// hack the depth range to prevent view model from poking into walls
					depthRange = qtrue;
				}
			}
			else
			{
				backEnd.refdef.floatTime = originalTime;
				backEnd.or = backEnd.viewParms.world;
				// we have to reset the shaderTime as well otherwise image animations on
				// the world (like water) continue with the wrong frame
				tess.shaderTime = backEnd.refdef.floatTime - tess.surfaceShader->timeOffset;
			}

			qglLoadMatrixf(backEnd.or.modelViewMatrix);

			// change depthrange if needed
			if(oldDepthRange != depthRange)
			{
				if(depthRange)
				{
					qglDepthRange(0, 0.3);
				}
				else
				{
					qglDepthRange(0, 1);
				}
				oldDepthRange = depthRange;
			}
		}

		// change the attenuation matrix if needed
		if(light != oldLight || entity != oldEntity)
		{
			// transform light origin into model space for u_LightOrigin parameter
			if(entity != &tr.worldEntity)
			{
				VectorSubtract(light->origin, backEnd.or.origin, tmp);
				light->transformed[0] = DotProduct(tmp, backEnd.or.axis[0]);
				light->transformed[1] = DotProduct(tmp, backEnd.or.axis[1]);
				light->transformed[2] = DotProduct(tmp, backEnd.or.axis[2]);
			}
			else
			{
				VectorCopy(light->origin, light->transformed);
			}

			if(drawShadows)
			{
				// set uniform parameter u_LightOrigin for GLSL shader
				qglUniform3fARB(tr.shadowExtrudeShader.u_LightOrigin, light->transformed[0], light->transformed[1],
								light->transformed[2]);
			}

			// build the attenuation matrix using the entity transform          
			MatrixMultiply(light->viewMatrix, backEnd.or.transformMatrix, modelToLight);

			MatrixSetupTranslation(light->attenuationMatrix, 0.5, 0.5, 0.5);	// bias
			MatrixMultiplyScale(light->attenuationMatrix, 0.5, 0.5, 0.5);	// scale
			MatrixMultiply2(light->attenuationMatrix, light->projectionMatrix);	// light projection (frustum)
			MatrixMultiply2(light->attenuationMatrix, modelToLight);
		}

		if(drawShadows)
		{
			// add the triangles for this surface
			rb_surfaceTable[*surface] (surface, 0, NULL, ia->numShadowIndexes, ia->shadowIndexes);
		}
		else
		{
			// add the triangles for this surface
			rb_surfaceTable[*surface] (surface, ia->numLightIndexes, ia->lightIndexes, 0, NULL);
		}

	  nextInteraction:

		// remember values
		oldLight = light;
		oldEntity = entity;
		oldShader = shader;

	  skipInteraction:
		if(!ia->next)
		{
			// if ia->next does not point to any other interaction then
			// this is the last interaction of the current light

			if(r_logFile->integer)
			{
				// don't just call LogComment, or we will get
				// a call to va() every frame!
				GLimp_LogComment(va("----- Last Interaction: %i -----\n", iaCount));
			}

			// draw the contents of the last shader batch
			Tess_End();

			if(drawShadows)
			{
				// jump back to first interaction of this light and start lighting
				ia = &interactions[iaFirst];
				iaCount = iaFirst;
				drawShadows = qfalse;
			}
			else
			{
				if(iaCount < (numInteractions - 1))
				{
					// jump to next interaction and start shadowing
					ia++;
					iaCount++;
					iaFirst = iaCount;
					drawShadows = qtrue;
				}
				else
				{
					// increase last time to leave for loop
					iaCount++;
				}
			}

			// force updates
			oldLight = NULL;
			oldEntity = NULL;
			oldShader = NULL;
		}
		else
		{
			// just continue
			ia = ia->next;
			iaCount++;
		}
	}

	backEnd.refdef.floatTime = originalTime;

	// go back to the world modelview matrix
	qglLoadMatrixf(backEnd.viewParms.world.modelViewMatrix);
	if(depthRange)
	{
		qglDepthRange(0, 1);
	}

	// reset scissor clamping
	qglScissor(backEnd.viewParms.viewportX, backEnd.viewParms.viewportY,
			   backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportHeight);

	// reset depth clamping
	if(qglDepthBoundsEXT)
	{
		qglDisable(GL_DEPTH_BOUNDS_TEST_EXT);
	}

	/*
	   if(qglActiveStencilFaceEXT)
	   {
	   qglDisable(GL_STENCIL_TEST_TWO_SIDE_EXT);
	   }
	 */

	GL_CheckErrors();
}

/*
=================
RB_RenderInteractionsShadowMapped
=================
*/
static void RB_RenderInteractionsShadowMapped(float originalTime, interaction_t * interactions, int numInteractions)
{
	shader_t       *shader, *oldShader;
	trRefEntity_t  *entity, *oldEntity;
	trRefLight_t   *light, *oldLight;
	interaction_t  *ia;
	int             iaCount;
	int             iaFirst;
	surfaceType_t  *surface;
	qboolean        depthRange, oldDepthRange;
	vec3_t          tmp;
	matrix_t        modelToLight;
	qboolean        drawShadows;
	int             cubeSide;

	if(!glConfig.framebufferObjectAvailable || !glConfig.shadingLanguage100Available || !glConfig.textureFloatAvailable)
	{
		RB_RenderInteractions(originalTime, interactions, numInteractions);
		return;
	}

	GLimp_LogComment("--- RB_RenderInteractionsShadowMapped ---\n");

	// draw everything
	oldLight = NULL;
	oldEntity = NULL;
	oldShader = NULL;
	oldDepthRange = qfalse;
	depthRange = qfalse;
	drawShadows = qtrue;
	cubeSide = 0;

	// if we need to clear the FBO color buffers then it should be white
	qglClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	// render interactions
	for(iaCount = 0, iaFirst = 0, ia = &interactions[0]; iaCount < numInteractions;)
	{
		backEnd.currentLight = light = ia->light;
		backEnd.currentEntity = entity = ia->entity;
		surface = ia->surface;
		shader = ia->surfaceShader;

		// only iaCount == iaFirst if first iteration or counters were reset
		if(iaCount == iaFirst)
		{
			if(drawShadows)
			{
				// HACK: bring OpenGL into a safe state or strange FBO update problems will occur
				GL_Program(0);
				GL_State(GLS_DEFAULT);
				GL_ClientState(GLCS_VERTEX);

				GL_SelectTexture(0);
				GL_Bind(tr.whiteImage);

				/*
				   if(light->l.noShadows)
				   {
				   if(r_logFile->integer)
				   {
				   // don't just call LogComment, or we will get
				   // a call to va() every frame!
				   GLimp_LogComment(va("----- Skipping shadowCube side: %i -----\n", cubeSide));
				   }

				   goto skipInteraction;
				   }
				   else
				 */
				{
					R_BindFBO(tr.shadowMapFBO[light->shadowLOD]);

					switch (light->l.rlType)
					{
						case RL_OMNI:
						{
							float           xMin, xMax, yMin, yMax;
							float           width, height, depth;
							float           zNear, zFar;
							float           fovX, fovY;
							qboolean        flipX, flipY;
							float          *proj;
							vec3_t          angles;
							matrix_t        rotationMatrix, transformMatrix, viewMatrix;

							if(r_logFile->integer)
							{
								// don't just call LogComment, or we will get
								// a call to va() every frame!
								GLimp_LogComment(va("----- Rendering shadowCube side: %i -----\n", cubeSide));
							}

							R_AttachFBOTexture2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + cubeSide,
												 tr.shadowCubeFBOImage[light->shadowLOD]->texnum, 0);
							if(!r_ignoreGLErrors->integer)
							{
								R_CheckFBO(tr.shadowMapFBO[light->shadowLOD]);
							}

							// set the window clipping
							qglViewport(0, 0, shadowMapResolutions[light->shadowLOD], shadowMapResolutions[light->shadowLOD]);
							qglScissor(0, 0, shadowMapResolutions[light->shadowLOD], shadowMapResolutions[light->shadowLOD]);

							qglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

							switch (cubeSide)
							{
								case 0:
								{
									// view parameters
									VectorSet(angles, 0, 0, 90);

									// projection parameters
									flipX = qfalse;
									flipY = qfalse;
									break;
								}

								case 1:
								{
									VectorSet(angles, 0, 180, 90);
									flipX = qtrue;
									flipY = qtrue;
									break;
								}

								case 2:
								{
									VectorSet(angles, 0, 90, 0);
									flipX = qfalse;
									flipY = qfalse;
									break;
								}

								case 3:
								{
									VectorSet(angles, 0, -90, 0);
									flipX = qtrue;
									flipY = qtrue;
									break;
								}

								case 4:
								{
									VectorSet(angles, -90, 90, 0);
									flipX = qfalse;
									flipY = qfalse;
									break;
								}

								case 5:
								{
									VectorSet(angles, 90, 90, 0);
									flipX = qtrue;
									flipY = qtrue;
									break;
								}

								default:
								{
									// shut up compiler
									VectorSet(angles, 0, 0, 0);
									flipX = qfalse;
									flipY = qfalse;
									break;
								}
							}

							// Quake -> OpenGL view matrix from light perspective
							MatrixFromAngles(rotationMatrix, angles[PITCH], angles[YAW], angles[ROLL]);
							MatrixSetupTransformFromRotation(transformMatrix, rotationMatrix, light->origin);
							MatrixAffineInverse(transformMatrix, viewMatrix);

							// convert from our coordinate system (looking down X)
							// to OpenGL's coordinate system (looking down -Z)
							MatrixMultiply(quakeToOpenGLMatrix, viewMatrix, light->viewMatrix);

							// OpenGL projection matrix
							fovX = 90;
							fovY =
								R_CalcFov(fovX, shadowMapResolutions[light->shadowLOD], shadowMapResolutions[light->shadowLOD]);

							zNear = 1.0;
							zFar = light->sphereRadius;

							if(!flipX)
							{
								xMax = zNear * tan(fovX * M_PI / 360.0f);
								xMin = -xMax;
							}
							else
							{
								xMin = zNear * tan(fovX * M_PI / 360.0f);
								xMax = -xMin;
							}

							if(!flipY)
							{
								yMax = zNear * tan(fovY * M_PI / 360.0f);
								yMin = -yMax;
							}
							else
							{
								yMin = zNear * tan(fovY * M_PI / 360.0f);
								yMax = -yMin;
							}

							width = xMax - xMin;
							height = yMax - yMin;
							depth = zFar - zNear;

							proj = light->projectionMatrix;
							proj[0] = (2 * zNear) / width;
							proj[4] = 0;
							proj[8] = (xMax + xMin) / width;
							proj[12] = 0;
							proj[1] = 0;
							proj[5] = (2 * zNear) / height;
							proj[9] = (yMax + yMin) / height;
							proj[13] = 0;
							proj[2] = 0;
							proj[6] = 0;
							proj[10] = -(zFar + zNear) / depth;
							proj[14] = -(2 * zFar * zNear) / depth;
							proj[3] = 0;
							proj[7] = 0;
							proj[11] = -1;
							proj[15] = 0;

							qglMatrixMode(GL_PROJECTION);
							qglLoadMatrixf(light->projectionMatrix);
							qglMatrixMode(GL_MODELVIEW);
							break;
						}

						case RL_PROJ:
						{
							GLimp_LogComment("--- Rendering projective shadowMap ---\n");

							R_AttachFBOTexture2D(GL_TEXTURE_2D, tr.shadowMapFBOImage[light->shadowLOD]->texnum, 0);
							if(!r_ignoreGLErrors->integer)
							{
								R_CheckFBO(tr.shadowMapFBO[light->shadowLOD]);
							}

							// set the window clipping
							qglViewport(0, 0, shadowMapResolutions[light->shadowLOD], shadowMapResolutions[light->shadowLOD]);
							qglScissor(0, 0, shadowMapResolutions[light->shadowLOD], shadowMapResolutions[light->shadowLOD]);

							qglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

							qglMatrixMode(GL_PROJECTION);
							qglLoadMatrixf(light->projectionMatrix);
							qglMatrixMode(GL_MODELVIEW);
							break;
						}

						case RL_DIRECT:
						{
							GLimp_LogComment("--- Rendering directional shadowMap ---\n");

							// TODO
							break;
						}

						default:
							break;
					}
				}

				if(r_logFile->integer)
				{
					// don't just call LogComment, or we will get
					// a call to va() every frame!
					GLimp_LogComment(va("----- First Shadow Interaction: %i -----\n", iaCount));
				}
			}
			else
			{
				GLimp_LogComment("--- Rendering lighting ---\n");

				if(r_logFile->integer)
				{
					// don't just call LogComment, or we will get
					// a call to va() every frame!
					GLimp_LogComment(va("----- First Light Interaction: %i -----\n", iaCount));
				}

				R_BindNullFBO();

				// set the window clipping
				qglViewport(backEnd.viewParms.viewportX, backEnd.viewParms.viewportY,
							backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportHeight);

				qglScissor(backEnd.viewParms.viewportX, backEnd.viewParms.viewportY,
						   backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportHeight);

				// restore camera matrices
				qglMatrixMode(GL_PROJECTION);
				qglLoadMatrixf(backEnd.viewParms.projectionMatrix);
				qglMatrixMode(GL_MODELVIEW);

				qglLoadMatrixf(backEnd.or.modelViewMatrix);

				// reset light view and projection matrices
				switch (light->l.rlType)
				{
					case RL_OMNI:
					{
						MatrixAffineInverse(light->transformMatrix, light->viewMatrix);
						MatrixSetupScale(light->projectionMatrix, 1.0 / light->l.radius[0], 1.0 / light->l.radius[1],
										 1.0 / light->l.radius[2]);
						break;
					}

					default:
						break;
				}

				// show shadowRender for debugging
				/*
				   switch (light->l.rlType)
				   {
				   case RL_PROJ:
				   {
				   float           x, y, w, h;

				   // enable shader, set arrays
				   GL_Program(tr.genericSingleShader.program);
				   GL_State(GLS_DEPTHTEST_DISABLE);
				   GL_ClientState(tr.genericSingleShader.attribs);
				   //GL_SetVertexAttribs();
				   GL_Cull(CT_TWO_SIDED);

				   qglColor3f(1, 1, 1);

				   // bind u_ColorMap
				   GL_SelectTexture(0);
				   GL_Bind(tr.shadowMapFBOImage);

				   // set 2D virtual screen size
				   qglPushMatrix();
				   qglLoadIdentity();
				   qglMatrixMode(GL_PROJECTION);
				   qglPushMatrix();
				   qglLoadIdentity();
				   qglOrtho(backEnd.viewParms.viewportX,
				   backEnd.viewParms.viewportX + backEnd.viewParms.viewportWidth,
				   backEnd.viewParms.viewportY, backEnd.viewParms.viewportY + backEnd.viewParms.viewportHeight, -99999, 99999);

				   x = 0;
				   y = 0;
				   w = backEnd.viewParms.viewportWidth / 3;
				   h = backEnd.viewParms.viewportHeight / 3;

				   qglBegin(GL_QUADS);
				   qglTexCoord2f(0, 0);
				   qglVertex2f(x, y);
				   qglTexCoord2f(1, 0);
				   qglVertex2f(x + w, y);
				   qglTexCoord2f(1, 1);
				   qglVertex2f(x + w, y + h);
				   qglTexCoord2f(0, 1);
				   qglVertex2f(x, y + h);
				   qglEnd();

				   qglPopMatrix();
				   qglMatrixMode(GL_MODELVIEW);
				   qglPopMatrix();
				   break;
				   }

				   default:
				   break;
				   }
				 */
			}
		}						// end if(iaCount == iaFirst)

		if(drawShadows)
		{
			if(entity->e.renderfx & (RF_NOSHADOW | RF_DEPTHHACK))
			{
				goto skipInteraction;
			}

			if(shader->sort > SS_OPAQUE)
			{
				goto skipInteraction;
			}

			if(shader->noShadows || light->l.noShadows)
			{
				goto skipInteraction;
			}

			if(ia->type == IA_LIGHTONLY)
			{
				goto skipInteraction;
			}

			if(light->l.rlType == RL_OMNI && !(ia->cubeSideBits & (1 << cubeSide)))
			{
				goto skipInteraction;
			}

			switch (light->l.rlType)
			{
				case RL_OMNI:
				case RL_PROJ:
				{
					if(light == oldLight && entity == oldEntity && shader == oldShader)
					{
						if(r_logFile->integer)
						{
							// don't just call LogComment, or we will get
							// a call to va() every frame!
							GLimp_LogComment(va("----- Batching Shadow Interaction: %i -----\n", iaCount));
						}

						// fast path, same as previous
						rb_surfaceTable[*surface] (surface, ia->numLightIndexes, ia->lightIndexes, 0, NULL);
						goto nextInteraction;
					}
					else
					{
						if(oldLight)
						{
							// draw the contents of the last shader batch
							Tess_End();
						}

						if(r_logFile->integer)
						{
							// don't just call LogComment, or we will get
							// a call to va() every frame!
							GLimp_LogComment(va("----- Beginning Shadow Interaction: %i -----\n", iaCount));
						}

						// we don't need tangent space calculations here
						Tess_Begin(Tess_StageIteratorShadowFill, shader, ia->lightShader, -1, 0, qtrue, qfalse);
					}
					break;
				}

				default:
					break;
			}
		}
		else
		{
			if(!shader->interactLight)
			{
				goto skipInteraction;
			}

			if(ia->type == IA_SHADOWONLY)
			{
				goto skipInteraction;
			}

			if(light == oldLight && entity == oldEntity && shader == oldShader)
			{
				if(r_logFile->integer)
				{
					// don't just call LogComment, or we will get
					// a call to va() every frame!
					GLimp_LogComment(va("----- Batching Light Interaction: %i -----\n", iaCount));
				}

				// fast path, same as previous
				rb_surfaceTable[*surface] (surface, ia->numLightIndexes, ia->lightIndexes, 0, NULL);
				goto nextInteraction;
			}
			else
			{
				if(oldLight)
				{
					// draw the contents of the last shader batch
					Tess_End();
				}

				if(r_logFile->integer)
				{
					// don't just call LogComment, or we will get
					// a call to va() every frame!
					GLimp_LogComment(va("----- Beginning Light Interaction: %i -----\n", iaCount));
				}

				// begin a new batch
				Tess_Begin(Tess_StageIteratorLighting, shader, ia->lightShader, -1, 0, qfalse, qfalse);
			}
		}

		// change the modelview matrix if needed
		if(entity != oldEntity)
		{
			depthRange = qfalse;

			if(entity != &tr.worldEntity)
			{
				backEnd.refdef.floatTime = originalTime - entity->e.shaderTime;
				// we have to reset the shaderTime as well otherwise image animations start
				// from the wrong frame
				tess.shaderTime = backEnd.refdef.floatTime - tess.surfaceShader->timeOffset;

				// set up the transformation matrix
				if(drawShadows)
				{
					R_RotateEntityForLight(entity, light, &backEnd.or);
				}
				else
				{
					R_RotateEntityForViewParms(entity, &backEnd.viewParms, &backEnd.or);
				}

				if(entity->e.renderfx & RF_DEPTHHACK)
				{
					// hack the depth range to prevent view model from poking into walls
					depthRange = qtrue;
				}
			}
			else
			{
				backEnd.refdef.floatTime = originalTime;
				// we have to reset the shaderTime as well otherwise image animations on
				// the world (like water) continue with the wrong frame
				tess.shaderTime = backEnd.refdef.floatTime - tess.surfaceShader->timeOffset;

				// set up the transformation matrix
				if(drawShadows)
				{
					Com_Memset(&backEnd.or, 0, sizeof(backEnd.or));

					backEnd.or.axis[0][0] = 1;
					backEnd.or.axis[1][1] = 1;
					backEnd.or.axis[2][2] = 1;
					VectorCopy(light->l.origin, backEnd.or.viewOrigin);

					MatrixIdentity(backEnd.or.transformMatrix);
					//MatrixAffineInverse(backEnd.or.transformMatrix, backEnd.or.viewMatrix);
					MatrixMultiply(light->viewMatrix, backEnd.or.transformMatrix, backEnd.or.viewMatrix);
					MatrixCopy(backEnd.or.viewMatrix, backEnd.or.modelViewMatrix);
				}
				else
				{
					// transform by the camera placement
					backEnd.or = backEnd.viewParms.world;
				}
			}

			qglLoadMatrixf(backEnd.or.modelViewMatrix);

			// change depthrange if needed
			if(oldDepthRange != depthRange)
			{
				if(depthRange)
				{
					qglDepthRange(0, 0.3);
				}
				else
				{
					qglDepthRange(0, 1);
				}
				oldDepthRange = depthRange;
			}
		}

		// change the attenuation matrix if needed
		if(light != oldLight || entity != oldEntity)
		{
			// transform light origin into model space for u_LightOrigin parameter
			if(entity != &tr.worldEntity)
			{
				VectorSubtract(light->origin, backEnd.or.origin, tmp);
				light->transformed[0] = DotProduct(tmp, backEnd.or.axis[0]);
				light->transformed[1] = DotProduct(tmp, backEnd.or.axis[1]);
				light->transformed[2] = DotProduct(tmp, backEnd.or.axis[2]);
			}
			else
			{
				VectorCopy(light->origin, light->transformed);
			}

			MatrixMultiply(light->viewMatrix, backEnd.or.transformMatrix, modelToLight);

			// build the attenuation matrix using the entity transform
			switch (light->l.rlType)
			{
				case RL_PROJ:
				{
					MatrixSetupTranslation(light->attenuationMatrix, 0.5, 0.5, 0.0);	// bias
					MatrixMultiplyScale(light->attenuationMatrix, 0.5, 0.5, 1.0);	// scale
					break;
				}

				case RL_OMNI:
				default:
				{
					MatrixSetupTranslation(light->attenuationMatrix, 0.5, 0.5, 0.5);	// bias
					MatrixMultiplyScale(light->attenuationMatrix, 0.5, 0.5, 0.5);	// scale
					break;
				}
			}
			MatrixMultiply2(light->attenuationMatrix, light->projectionMatrix);
			MatrixMultiply2(light->attenuationMatrix, modelToLight);
		}

		if(drawShadows)
		{
			switch (light->l.rlType)
			{
				case RL_OMNI:
				case RL_PROJ:
				{
					// add the triangles for this surface
					rb_surfaceTable[*surface] (surface, ia->numLightIndexes, ia->lightIndexes, 0, NULL);
					break;
				}

				default:
					break;
			}
		}
		else
		{
			// add the triangles for this surface
			rb_surfaceTable[*surface] (surface, ia->numLightIndexes, ia->lightIndexes, 0, NULL);
		}

	  nextInteraction:

		// remember values
		oldLight = light;
		oldEntity = entity;
		oldShader = shader;

	  skipInteraction:
		if(!ia->next)
		{
			// if ia->next does not point to any other interaction then
			// this is the last interaction of the current light

			if(r_logFile->integer)
			{
				// don't just call LogComment, or we will get
				// a call to va() every frame!
				GLimp_LogComment(va("----- Last Interaction: %i -----\n", iaCount));
			}

			// draw the contents of the last shader batch
			Tess_End();

			if(drawShadows)
			{
				switch (light->l.rlType)
				{
					case RL_OMNI:
					{
						if(cubeSide == 5)
						{
							cubeSide = 0;
							drawShadows = qfalse;
						}
						else
						{
							cubeSide++;
						}

						// jump back to first interaction of this light
						ia = &interactions[iaFirst];
						iaCount = iaFirst;
						break;
					}

					case RL_PROJ:
					{
						// jump back to first interaction of this light and start lighting
						ia = &interactions[iaFirst];
						iaCount = iaFirst;
						drawShadows = qfalse;
						break;
					}

					default:
						break;
				}
			}
			else
			{
				if(iaCount < (numInteractions - 1))
				{
					// jump to next interaction and start shadowing
					ia++;
					iaCount++;
					iaFirst = iaCount;
					drawShadows = qtrue;
				}
				else
				{
					// increase last time to leave for loop
					iaCount++;
				}
			}

			// force updates
			oldLight = NULL;
			oldEntity = NULL;
			oldShader = NULL;
		}
		else
		{
			// just continue
			ia = ia->next;
			iaCount++;
		}
	}

	backEnd.refdef.floatTime = originalTime;

	// go back to the world modelview matrix
	qglLoadMatrixf(backEnd.viewParms.world.modelViewMatrix);
	if(depthRange)
	{
		qglDepthRange(0, 1);
	}

	// reset scissor clamping
	qglScissor(backEnd.viewParms.viewportX, backEnd.viewParms.viewportY,
			   backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportHeight);

	// reset clear color
	qglClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	GL_CheckErrors();
}

static void RB_RenderDrawSurfacesIntoGeometricBuffer(float originalTime, drawSurf_t * drawSurfs, int numDrawSurfs)
{
	trRefEntity_t  *entity, *oldEntity;
	int             lightmapNum, oldLightmapNum;
	shader_t       *shader, *oldShader;
	int             fogNum, oldFogNum;
	qboolean        depthRange, oldDepthRange;
	int             i;
	drawSurf_t     *drawSurf;
	int             oldSort;

	GLimp_LogComment("--- RB_RenderDrawSurfacesIntoGeometricBuffer ---\n");

	// draw everything
	oldEntity = NULL;
	oldShader = NULL;
	oldLightmapNum = -1;
	oldFogNum = -1;
	oldDepthRange = qfalse;
	oldSort = -1;
	depthRange = qfalse;

	R_BindFBO(tr.deferredRenderFBO);
	qglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	GL_CheckErrors();

	for(i = 0, drawSurf = drawSurfs; i < numDrawSurfs; i++, drawSurf++)
	{
		// update locals
		entity = drawSurf->entity;
		shader = tr.sortedShaders[drawSurf->shaderNum];
		lightmapNum = drawSurf->lightmapNum;
		fogNum = drawSurf->fogNum;

		//if(opaque)
		{
			// skip all translucent surfaces that don't matter for this pass
			if(shader->sort > SS_OPAQUE)
			{
				break;
			}
		}
		/*
		   else
		   {
		   // skip all opaque surfaces that don't matter for this pass
		   if(shader->sort <= SS_OPAQUE)
		   {
		   continue;
		   }
		   }
		 */

		if(entity == oldEntity && shader == oldShader && lightmapNum == oldLightmapNum && fogNum == oldFogNum)
		{
			// fast path, same as previous sort
			rb_surfaceTable[*drawSurf->surface] (drawSurf->surface, 0, NULL, 0, NULL);
			continue;
		}

		// change the tess parameters if needed
		// a "entityMergable" shader is a shader that can have surfaces from seperate
		// entities merged into a single batch, like smoke and blood puff sprites
		if(shader != oldShader || fogNum != oldFogNum || lightmapNum != oldLightmapNum ||
		   (entity != oldEntity && !shader->entityMergable))
		{
			if(oldShader != NULL)
			{
				Tess_End();
			}

			Tess_Begin(Tess_StageIteratorGBuffer, shader, NULL, lightmapNum, fogNum, qfalse, qfalse);
			oldShader = shader;
			oldLightmapNum = lightmapNum;
			oldFogNum = fogNum;
		}

		// change the modelview matrix if needed
		if(entity != oldEntity)
		{
			depthRange = qfalse;

			if(entity != &tr.worldEntity)
			{
				backEnd.currentEntity = entity;
				backEnd.refdef.floatTime = originalTime - backEnd.currentEntity->e.shaderTime;

				// we have to reset the shaderTime as well otherwise image animations start
				// from the wrong frame
				tess.shaderTime = backEnd.refdef.floatTime - tess.surfaceShader->timeOffset;

				// set up the transformation matrix
				R_RotateEntityForViewParms(backEnd.currentEntity, &backEnd.viewParms, &backEnd.or);

				if(backEnd.currentEntity->e.renderfx & RF_DEPTHHACK)
				{
					// hack the depth range to prevent view model from poking into walls
					depthRange = qtrue;
				}
			}
			else
			{
				backEnd.currentEntity = &tr.worldEntity;
				backEnd.refdef.floatTime = originalTime;
				backEnd.or = backEnd.viewParms.world;

				// we have to reset the shaderTime as well otherwise image animations on
				// the world (like water) continue with the wrong frame
				tess.shaderTime = backEnd.refdef.floatTime - tess.surfaceShader->timeOffset;
			}

			qglLoadMatrixf(backEnd.or.modelViewMatrix);

			// change depthrange if needed
			if(oldDepthRange != depthRange)
			{
				if(depthRange)
				{
					qglDepthRange(0, 0.3);
				}
				else
				{
					qglDepthRange(0, 1);
				}
				oldDepthRange = depthRange;
			}

			oldEntity = entity;
		}

		// add the triangles for this surface
		rb_surfaceTable[*drawSurf->surface] (drawSurf->surface, 0, NULL, 0, NULL);
	}

	backEnd.refdef.floatTime = originalTime;

	// draw the contents of the last shader batch
	if(oldShader != NULL)
	{
		Tess_End();
	}

	// go back to the world modelview matrix
	qglLoadMatrixf(backEnd.viewParms.world.modelViewMatrix);
	if(depthRange)
	{
		qglDepthRange(0, 1);
	}

	// disable offscreen rendering
	R_BindNullFBO();

	GL_CheckErrors();
}

void RB_RenderInteractionsDeferred(interaction_t * interactions, int numInteractions)
{
	interaction_t  *ia;
	int             iaCount;
	trRefLight_t   *light, *oldLight = NULL;
	shader_t       *lightShader;
	shaderStage_t  *attenuationXYStage;
	shaderStage_t  *attenuationZStage;
	int             j;
	float           fbufWidthScale, fbufHeightScale;
	float           npotWidthScale, npotHeightScale;
	vec3_t          viewOrigin;
	vec3_t          lightOrigin;
	vec4_t          lightColor;

	GLimp_LogComment("--- RB_RenderInteractionsDeferred ---\n");

	R_BindNullFBO();

	// update uniforms
	VectorCopy(backEnd.viewParms.or.origin, viewOrigin);
	
	fbufWidthScale = Q_recip((float)glConfig.vidWidth);
	fbufHeightScale = Q_recip((float)glConfig.vidHeight);
	npotWidthScale = (float)glConfig.vidWidth / (float)NearestPowerOfTwo(glConfig.vidWidth);
	npotHeightScale = (float)glConfig.vidHeight / (float)NearestPowerOfTwo(glConfig.vidHeight);

	// set 2D virtual screen size
	qglPushMatrix();
	qglLoadIdentity();
	qglMatrixMode(GL_PROJECTION);
	qglPushMatrix();
	qglLoadIdentity();
	qglOrtho(backEnd.viewParms.viewportX,
			 backEnd.viewParms.viewportX + backEnd.viewParms.viewportWidth,
			 backEnd.viewParms.viewportY, backEnd.viewParms.viewportY + backEnd.viewParms.viewportHeight, -99999, 99999);

	// loop trough all light interactions and render the light quad for each last interaction
	for(iaCount = 0, ia = &interactions[0]; iaCount < numInteractions;)
	{
		backEnd.currentLight = light = ia->light;

		if(light != oldLight)
		{
			// set light scissor to reduce fillrate
			qglScissor(ia->scissorX, ia->scissorY, ia->scissorWidth, ia->scissorHeight);
			
			// build the attenuation matrix using the entity transform
			MatrixSetupTranslation(light->attenuationMatrix, 0.5, 0.5, 0.5);	// bias
			MatrixMultiplyScale(light->attenuationMatrix, 0.5, 0.5, 0.5);		// scale
			MatrixMultiply2(light->attenuationMatrix, light->projectionMatrix);	// light projection (frustum)
			MatrixMultiply2(light->attenuationMatrix, light->viewMatrix);
		}

		if(!ia->next)
		{
			// last interaction of current light
			lightShader = ia->lightShader;
			attenuationZStage = lightShader->stages[0];

			for(j = 1; j < MAX_SHADER_STAGES; j++)
			{
				attenuationXYStage = lightShader->stages[j];

				if(!attenuationXYStage)
				{
					break;
				}

				if(attenuationXYStage->type != ST_ATTENUATIONMAP_XY)
				{
					continue;
				}

				if(!RB_EvalExpression(&attenuationXYStage->ifExp, 1.0))
				{
					continue;
				}

				Tess_ComputeColor(attenuationXYStage);
				R_ComputeFinalAttenuation(attenuationXYStage, light);

				if(light->l.rlType == RL_OMNI)
				{
					// enable shader, set arrays
					GL_Program(tr.deferredLightingShader_DBS_omni.program);
					
					// set OpenGL state for additive lighting
					GL_State(GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE | GLS_DEPTHTEST_DISABLE);
					GL_ClientState(tr.deferredLightingShader_DBS_omni.attribs);
					
					//GL_SetVertexAttribs();
					GL_Cull(CT_TWO_SIDED);

					// set uniforms
					VectorCopy(light->origin, lightOrigin);
					VectorCopy(tess.svars.color, lightColor);

					qglUniform3fARB(tr.deferredLightingShader_DBS_omni.u_ViewOrigin, viewOrigin[0], viewOrigin[1], viewOrigin[2]);
					qglUniform3fARB(tr.deferredLightingShader_DBS_omni.u_LightOrigin, lightOrigin[0], lightOrigin[1], lightOrigin[2]);
					qglUniform3fARB(tr.deferredLightingShader_DBS_omni.u_LightColor, lightColor[0], lightColor[1], lightColor[2]);
					qglUniform1fARB(tr.deferredLightingShader_DBS_omni.u_LightRadius, light->sphereRadius);
					qglUniform1fARB(tr.deferredLightingShader_DBS_omni.u_LightScale, r_lightScale->value);
					qglUniformMatrix4fvARB(tr.deferredLightingShader_DBS_omni.u_LightAttenuationMatrix, 1, GL_FALSE, light->attenuationMatrix2);
					qglUniform2fARB(tr.deferredLightingShader_DBS_omni.u_FBufScale, fbufWidthScale, fbufHeightScale);
					qglUniform2fARB(tr.deferredLightingShader_DBS_omni.u_NPOTScale, npotWidthScale, npotHeightScale);

					// bind u_DiffuseMap
					GL_SelectTexture(0);
					GL_Bind(tr.deferredDiffuseFBOImage);
					//GL_TextureFilter(tr.deferredDiffuseFBOImage, FT_NEAREST);

					// bind u_NormalMap
					GL_SelectTexture(1);
					GL_Bind(tr.deferredNormalFBOImage);
					//GL_TextureFilter(tr.deferredNormalFBOImage, FT_NEAREST);
					
					// bind u_SpecularMap
					GL_SelectTexture(2);
					GL_Bind(tr.deferredSpecularFBOImage);
					//GL_TextureFilter(tr.deferredSpecularFBOImage, FT_NEAREST);

					// bind u_PositionMap
					GL_SelectTexture(3);
					GL_Bind(tr.deferredPositionFBOImage);
					//GL_TextureFilter(tr.deferredPointFBOImage, FT_NEAREST);

					// bind u_AttenuationMapXY
					GL_SelectTexture(4);
					BindAnimatedImage(&attenuationXYStage->bundle[TB_COLORMAP]);

					// bind u_AttenuationMapZ
					GL_SelectTexture(5);
					BindAnimatedImage(&attenuationZStage->bundle[TB_COLORMAP]);
				}
				else if(light->l.rlType == RL_PROJ)
				{
					// TODO
					// Render_lighting_D_proj(diffuseStage, attenuationXYStage, attenuationZStage, light);
				}
				else
				{
					// TODO
				}


				// draw lighting
#if 0
				qglBegin(GL_QUADS);
				qglVertex2f(ia->scissorX, ia->scissorY);
				qglVertex2f(ia->scissorX + ia->scissorWidth - 1, ia->scissorY);
				qglVertex2f(ia->scissorX + ia->scissorWidth - 1, ia->scissorY + ia->scissorHeight - 1);
				qglVertex2f(ia->scissorX, ia->scissorY + ia->scissorHeight - 1);
				qglEnd();
#else
				qglBegin(GL_QUADS);
				qglVertex2f(backEnd.viewParms.viewportX, backEnd.viewParms.viewportY);
				qglVertex2f(backEnd.viewParms.viewportX + backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportY);
				qglVertex2f(backEnd.viewParms.viewportX + backEnd.viewParms.viewportWidth,
							backEnd.viewParms.viewportY + backEnd.viewParms.viewportHeight);
				qglVertex2f(backEnd.viewParms.viewportX, backEnd.viewParms.viewportY + backEnd.viewParms.viewportHeight);
				qglEnd();
#endif
			}

			if(iaCount < (numInteractions - 1))
			{
				// jump to next interaction and continue
				ia++;
				iaCount++;
			}
			else
			{
				// increase last time to leave for loop
				iaCount++;
			}
		}
		else
		{
			// just continue
			ia = ia->next;
			iaCount++;
		}

		oldLight = light;
	}

	qglPopMatrix();
	qglMatrixMode(GL_MODELVIEW);
	qglPopMatrix();

	// go back to the world modelview matrix
	backEnd.or = backEnd.viewParms.world;
	qglLoadMatrixf(backEnd.viewParms.world.modelViewMatrix);

	// reset scissor
	qglScissor(backEnd.viewParms.viewportX, backEnd.viewParms.viewportY,
			   backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportHeight);

	GL_CheckErrors();
}

void RB_RenderGeometricBuffer()
{
	float           fbufWidthScale, fbufHeightScale;
	float           npotWidthScale, npotHeightScale;

	GLimp_LogComment("--- RB_RenderGeometricBuffer ---\n");

	R_BindNullFBO();

	// enable shader, set arrays
	GL_Program(tr.screenShader.program);
	GL_State(GLS_DEPTHTEST_DISABLE);
	GL_ClientState(tr.screenShader.attribs);
	//GL_SetVertexAttribs();
	GL_Cull(CT_TWO_SIDED);

	// set uniforms
	fbufWidthScale = Q_recip((float)glConfig.vidWidth);
	fbufHeightScale = Q_recip((float)glConfig.vidHeight);
	npotWidthScale = (float)glConfig.vidWidth / (float)NearestPowerOfTwo(glConfig.vidWidth);
	npotHeightScale = (float)glConfig.vidHeight / (float)NearestPowerOfTwo(glConfig.vidHeight);

	qglUniform2fARB(tr.screenShader.u_FBufScale, fbufWidthScale, fbufHeightScale);
	qglUniform2fARB(tr.screenShader.u_NPOTScale, npotWidthScale, npotHeightScale);

	// bind colorMap
	GL_SelectTexture(0);
	GL_Bind(tr.deferredNormalFBOImage);
	//GL_TextureFilter(tr.deferredNormalFBOImage, FT_NEAREST);

	// set 2D virtual screen size
	qglPushMatrix();
	qglLoadIdentity();
	qglMatrixMode(GL_PROJECTION);
	qglPushMatrix();
	qglLoadIdentity();
	qglOrtho(backEnd.viewParms.viewportX,
			 backEnd.viewParms.viewportX + backEnd.viewParms.viewportWidth,
			 backEnd.viewParms.viewportY, backEnd.viewParms.viewportY + backEnd.viewParms.viewportHeight, -99999, 99999);

	qglBegin(GL_QUADS);
	qglVertex2f(backEnd.viewParms.viewportX, backEnd.viewParms.viewportY);
	qglVertex2f(backEnd.viewParms.viewportX + backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportY);
	qglVertex2f(backEnd.viewParms.viewportX + backEnd.viewParms.viewportWidth,
				backEnd.viewParms.viewportY + backEnd.viewParms.viewportHeight);
	qglVertex2f(backEnd.viewParms.viewportX, backEnd.viewParms.viewportY + backEnd.viewParms.viewportHeight);
	qglEnd();

	qglPopMatrix();
	qglMatrixMode(GL_MODELVIEW);
	qglPopMatrix();
}

void RB_RenderOcclusionQueries(interaction_t * interactions, int numInteractions)
{
	GLimp_LogComment("--- RB_RenderOcclusionQueries ---\n");

	if(glConfig.occlusionQueryBits)
	{
		int             i;
		interaction_t  *ia;
		int             iaCount;
		int             iaFirst;
		trRefLight_t   *light, *oldLight;
		int             ocCount;
		GLint           ocSamples = 0;
		qboolean        queryObjects;
		GLint           available;

		qglColor4f(1.0f, 0.0f, 0.0f, 0.05f);

		GL_Program(0);
		GL_Cull(CT_TWO_SIDED);
		GL_SelectTexture(0);
		qglDisable(GL_TEXTURE_2D);

		// don't write to the color buffer or depth buffer
		if(r_showOcclusionQueries->integer)
		{
			GL_State(GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE);
		}
		else
		{
			GL_State(GLS_COLORMASK_BITS);
		}

		// loop trough all light interactions and render the light OBB for each last interaction
		ocCount = -1;
		for(iaCount = 0, ia = &interactions[0]; iaCount < numInteractions;)
		{
			backEnd.currentLight = light = ia->light;

			if(!ia->next)
			{
				ocCount++;

				// last interaction of current light
				if(ocCount < (MAX_OCCLUSION_QUERIES - 1) && !R_LightIntersectsPoint(light, backEnd.viewParms.or.origin))
				{
					R_RotateLightForViewParms(light, &backEnd.viewParms, &backEnd.or);
					qglLoadMatrixf(backEnd.or.modelViewMatrix);

					// begin the occlusion query
					qglBeginQueryARB(GL_SAMPLES_PASSED, tr.occlusionQueryObjects[ocCount]);

					qglBegin(GL_QUADS);

					qglVertex3f(light->localBounds[0][0], light->localBounds[0][1], light->localBounds[0][2]);
					qglVertex3f(light->localBounds[0][0], light->localBounds[1][1], light->localBounds[0][2]);
					qglVertex3f(light->localBounds[0][0], light->localBounds[1][1], light->localBounds[1][2]);
					qglVertex3f(light->localBounds[0][0], light->localBounds[0][1], light->localBounds[1][2]);

					qglVertex3f(light->localBounds[1][0], light->localBounds[0][1], light->localBounds[0][2]);
					qglVertex3f(light->localBounds[1][0], light->localBounds[1][1], light->localBounds[0][2]);
					qglVertex3f(light->localBounds[1][0], light->localBounds[1][1], light->localBounds[1][2]);
					qglVertex3f(light->localBounds[1][0], light->localBounds[0][1], light->localBounds[1][2]);

					qglVertex3f(light->localBounds[0][0], light->localBounds[0][1], light->localBounds[1][2]);
					qglVertex3f(light->localBounds[0][0], light->localBounds[1][1], light->localBounds[1][2]);
					qglVertex3f(light->localBounds[1][0], light->localBounds[1][1], light->localBounds[1][2]);
					qglVertex3f(light->localBounds[1][0], light->localBounds[0][1], light->localBounds[1][2]);

					qglVertex3f(light->localBounds[0][0], light->localBounds[0][1], light->localBounds[0][2]);
					qglVertex3f(light->localBounds[0][0], light->localBounds[1][1], light->localBounds[0][2]);
					qglVertex3f(light->localBounds[1][0], light->localBounds[1][1], light->localBounds[0][2]);
					qglVertex3f(light->localBounds[1][0], light->localBounds[0][1], light->localBounds[0][2]);

					qglVertex3f(light->localBounds[0][0], light->localBounds[0][1], light->localBounds[0][2]);
					qglVertex3f(light->localBounds[0][0], light->localBounds[0][1], light->localBounds[1][2]);
					qglVertex3f(light->localBounds[1][0], light->localBounds[0][1], light->localBounds[1][2]);
					qglVertex3f(light->localBounds[1][0], light->localBounds[0][1], light->localBounds[0][2]);

					qglVertex3f(light->localBounds[0][0], light->localBounds[1][1], light->localBounds[0][2]);
					qglVertex3f(light->localBounds[0][0], light->localBounds[1][1], light->localBounds[1][2]);
					qglVertex3f(light->localBounds[1][0], light->localBounds[1][1], light->localBounds[1][2]);
					qglVertex3f(light->localBounds[1][0], light->localBounds[1][1], light->localBounds[0][2]);

					qglEnd();

					// end the query
					// don't read back immediately so that we give the query time to be ready
					qglEndQueryARB(GL_SAMPLES_PASSED);

					backEnd.pc.c_occlusionQueries++;
				}

				if(iaCount < (numInteractions - 1))
				{
					// jump to next interaction and continue
					ia++;
					iaCount++;
				}
				else
				{
					// increase last time to leave for loop
					iaCount++;
				}
			}
			else
			{
				// just continue
				ia = ia->next;
				iaCount++;
			}
		}

		// go back to the world modelview matrix
		backEnd.or = backEnd.viewParms.world;
		qglLoadMatrixf(backEnd.viewParms.world.modelViewMatrix);

		if(!ocCount)
		{
			qglEnable(GL_TEXTURE_2D);
			return;
		}

		qglFinish();

		// do other work until "most" of the queries are back, to avoid
		// wasting time spinning
#if 1
		i = (int)(ocCount * 3 / 4);	// instead of N-1, to prevent the GPU from going idle
		do
		{
			i++;

			//if(i >= ocCount)
			//  i = (int)(ocCount * 3 / 4);

			qglGetQueryObjectivARB(tr.occlusionQueryObjects[i], GL_QUERY_RESULT_AVAILABLE_ARB, &available);
		} while(!available && i < ocCount);
#endif

		// reenable writes to depth and color buffers
		GL_State(GLS_DEPTHMASK_TRUE);
		qglEnable(GL_TEXTURE_2D);

		// loop trough all light interactions and fetch results for each last interaction
		// then copy result to all other interactions that belong to the same light
		ocCount = -1;
		iaFirst = 0;
		queryObjects = qtrue;
		oldLight = NULL;
		for(iaCount = 0, ia = &interactions[0]; iaCount < numInteractions;)
		{
			backEnd.currentLight = light = ia->light;

			if(light != oldLight)
			{
				iaFirst = iaCount;
			}

			if(!queryObjects)
			{
				ia->occlusionQuerySamples = ocSamples;
			}

			if(!ia->next)
			{
				if(queryObjects)
				{
					ocCount++;

					if(ocCount < (MAX_OCCLUSION_QUERIES - 1) && !R_LightIntersectsPoint(light, backEnd.viewParms.or.origin))
					{
#if 1
						qglGetQueryObjectivARB(tr.occlusionQueryObjects[ocCount], GL_QUERY_RESULT_AVAILABLE_ARB, &available);
						if(available)
#else
						if(1)
#endif
						{
							backEnd.pc.c_occlusionQueriesAvailable++;

							// get the object and store it in the occlusion bits for the light
							qglGetQueryObjectivARB(tr.occlusionQueryObjects[ocCount], GL_QUERY_RESULT, &ocSamples);

							if(ocSamples <= 0)
							{
								backEnd.pc.c_occlusionQueriesCulled++;
							}
						}
						else
						{
							ocSamples = 1;
						}
					}
					else
					{
						ocSamples = 1;
					}

					// jump back to first interaction of this light copy query result
					ia = &interactions[iaFirst];
					iaCount = iaFirst;
					queryObjects = qfalse;
				}
				else
				{
					if(iaCount < (numInteractions - 1))
					{
						// jump to next interaction and start querying
						ia++;
						iaCount++;
						queryObjects = qtrue;
					}
					else
					{
						// increase last time to leave for loop
						iaCount++;
					}
				}
			}
			else
			{
				// just continue
				ia = ia->next;
				iaCount++;
			}

			oldLight = light;
		}
	}
#if 0
	// Tr3B - try to cull light interactions manually with stencil overdraw test
	else
	{
		interaction_t  *ia;
		int             iaCount;
		int             iaFirst = 0;
		trRefLight_t   *light, *oldLight;
		int             i;
		long            sum = 0;
		unsigned char  *stencilReadback;
		qboolean        calcSum;

		qglColor4f(1.0f, 0.0f, 0.0f, 0.05f);

		GL_Program(0);
		GL_Cull(CT_TWO_SIDED);
		GL_SelectTexture(0);
		GL_Bind(tr.whiteImage);

		stencilReadback = ri.Hunk_AllocateTempMemory(glConfig.vidWidth * glConfig.vidHeight);

		// don't write to the color buffer or depth buffer
		if(r_showOcclusionQueries->integer)
		{
			GL_State(GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE);
		}
		else
		{
			GL_State(GLS_COLORMASK_BITS | GLS_STENCILTEST_ENABLE);
		}

		oldLight = NULL;
		calcSum = qtrue;

		// loop trough all light interactions and render the light OBB for each last interaction
		for(iaCount = 0, ia = &interactions[0]; iaCount < numInteractions;)
		{
			backEnd.currentLight = light = ia->light;

			if(light != oldLight)
			{
				iaFirst = iaCount;
			}

			if(!calcSum)
			{
				ia->occlusionQuerySamples = sum;
			}

			if(!ia->next)
			{
				// last interaction of current light
				if(calcSum)
				{
					if(!R_LightIntersectsPoint(light, backEnd.viewParms.or.origin))
					{
						// clear stencil buffer
						qglClear(GL_STENCIL_BUFFER_BIT);

						// set the reference stencil value
						//qglClearStencil(0U);
						qglStencilMask(~0);
						qglStencilFunc(GL_ALWAYS, 0, ~0);
						qglStencilOp(GL_KEEP, GL_INCR, GL_INCR);

						R_RotateLightForViewParms(light, &backEnd.viewParms, &backEnd.or);
						qglLoadMatrixf(backEnd.or.modelViewMatrix);

						qglBegin(GL_QUADS);

						qglVertex3f(light->localBounds[0][0], light->localBounds[0][1], light->localBounds[0][2]);
						qglVertex3f(light->localBounds[0][0], light->localBounds[1][1], light->localBounds[0][2]);
						qglVertex3f(light->localBounds[0][0], light->localBounds[1][1], light->localBounds[1][2]);
						qglVertex3f(light->localBounds[0][0], light->localBounds[0][1], light->localBounds[1][2]);

						qglVertex3f(light->localBounds[1][0], light->localBounds[0][1], light->localBounds[0][2]);
						qglVertex3f(light->localBounds[1][0], light->localBounds[1][1], light->localBounds[0][2]);
						qglVertex3f(light->localBounds[1][0], light->localBounds[1][1], light->localBounds[1][2]);
						qglVertex3f(light->localBounds[1][0], light->localBounds[0][1], light->localBounds[1][2]);

						qglVertex3f(light->localBounds[0][0], light->localBounds[0][1], light->localBounds[1][2]);
						qglVertex3f(light->localBounds[0][0], light->localBounds[1][1], light->localBounds[1][2]);
						qglVertex3f(light->localBounds[1][0], light->localBounds[1][1], light->localBounds[1][2]);
						qglVertex3f(light->localBounds[1][0], light->localBounds[0][1], light->localBounds[1][2]);

						qglVertex3f(light->localBounds[0][0], light->localBounds[0][1], light->localBounds[0][2]);
						qglVertex3f(light->localBounds[0][0], light->localBounds[1][1], light->localBounds[0][2]);
						qglVertex3f(light->localBounds[1][0], light->localBounds[1][1], light->localBounds[0][2]);
						qglVertex3f(light->localBounds[1][0], light->localBounds[0][1], light->localBounds[0][2]);

						qglVertex3f(light->localBounds[0][0], light->localBounds[0][1], light->localBounds[0][2]);
						qglVertex3f(light->localBounds[0][0], light->localBounds[0][1], light->localBounds[1][2]);
						qglVertex3f(light->localBounds[1][0], light->localBounds[0][1], light->localBounds[1][2]);
						qglVertex3f(light->localBounds[1][0], light->localBounds[0][1], light->localBounds[0][2]);

						qglVertex3f(light->localBounds[0][0], light->localBounds[1][1], light->localBounds[0][2]);
						qglVertex3f(light->localBounds[0][0], light->localBounds[1][1], light->localBounds[1][2]);
						qglVertex3f(light->localBounds[1][0], light->localBounds[1][1], light->localBounds[1][2]);
						qglVertex3f(light->localBounds[1][0], light->localBounds[1][1], light->localBounds[0][2]);

						qglEnd();

						backEnd.pc.c_occlusionQueries++;
						backEnd.pc.c_occlusionQueriesAvailable++;

#if 1
						qglReadPixels(0, 0, glConfig.vidWidth, glConfig.vidHeight, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE,
									  stencilReadback);

						for(i = 0, sum = 0; i < glConfig.vidWidth * glConfig.vidHeight; i++)
						{
							sum += stencilReadback[i];
						}
#else
						// only consider the 2D light scissor of current light
						qglReadPixels(ia->scissorX, ia->scissorY, ia->scissorWidth, ia->scissorHeight, GL_STENCIL_INDEX,
									  GL_UNSIGNED_BYTE, stencilReadback);

						for(i = 0, sum = 0; i < ia->scissorWidth * ia->scissorHeight; i++)
						{
							sum += stencilReadback[i];
						}
#endif

						if(!sum)
						{
							backEnd.pc.c_occlusionQueriesCulled++;
						}
					}
					else
					{
						sum = 1;
					}

					// jump back to first interaction of this light copy sum to all interactions
					ia = &interactions[iaFirst];
					iaCount = iaFirst;
					calcSum = qfalse;
				}
				else
				{
					if(iaCount < (numInteractions - 1))
					{
						// jump to next interaction and continue
						ia++;
						iaCount++;
						calcSum = qtrue;
					}
					else
					{
						// increase last time to leave for loop
						iaCount++;
					}
				}
			}
			else
			{
				// just continue
				ia = ia->next;
				iaCount++;
			}

			oldLight = light;
		}

		// go back to the world modelview matrix
		backEnd.or = backEnd.viewParms.world;
		qglLoadMatrixf(backEnd.viewParms.world.modelViewMatrix);

		// reenable writes to depth and color buffers
		GL_State(GLS_DEPTHMASK_TRUE);

		ri.Hunk_FreeTempMemory(stencilReadback);
	}
#endif

	GL_CheckErrors();
}

static void RB_RenderDebugUtils(interaction_t * interactions, int numInteractions)
{
	GLimp_LogComment("--- RB_RenderDebugUtils ---\n");

	if(r_showLightTransforms->integer)
	{
		int             i;
		trRefLight_t   *light;
		vec3_t          forward, left, up;
		vec3_t          tmp;

		if(r_dynamicLighting->integer)
		{
			GL_Program(0);
			GL_State(0);
			GL_SelectTexture(0);
			GL_Bind(tr.whiteImage);

			light = backEnd.refdef.lights;
			for(i = 0; i < backEnd.refdef.numLights; i++, light++)
			{
				// set up the transformation matrix
				R_RotateLightForViewParms(light, &backEnd.viewParms, &backEnd.or);
				qglLoadMatrixf(backEnd.or.modelViewMatrix);

				MatrixToVectorsFLU(matrixIdentity, forward, left, up);
				VectorMA(vec3_origin, 16, forward, forward);
				VectorMA(vec3_origin, 16, left, left);
				VectorMA(vec3_origin, 16, up, up);

				// draw axis
				//qglLineWidth(3);
				qglBegin(GL_LINES);

				qglColor4fv(colorRed);
				qglVertex3fv(vec3_origin);
				qglVertex3fv(forward);

				qglColor4fv(colorGreen);
				qglVertex3fv(vec3_origin);
				qglVertex3fv(left);

				qglColor4fv(colorBlue);
				qglVertex3fv(vec3_origin);
				qglVertex3fv(up);

				qglColor4fv(colorYellow);
				qglVertex3fv(vec3_origin);
				VectorSubtract(light->origin, backEnd.or.origin, tmp);
				light->transformed[0] = DotProduct(tmp, backEnd.or.axis[0]);
				light->transformed[1] = DotProduct(tmp, backEnd.or.axis[1]);
				light->transformed[2] = DotProduct(tmp, backEnd.or.axis[2]);
				qglVertex3fv(light->transformed);

				qglColor4fv(colorMagenta);
				qglVertex3fv(vec3_origin);
				qglVertex3fv(light->l.target);

				qglColor4fv(colorCyan);
				qglVertex3fv(vec3_origin);
				qglVertex3fv(light->l.right);

				qglColor4fv(colorWhite);
				qglVertex3fv(vec3_origin);
				qglVertex3fv(light->l.up);

				qglColor4fv(colorMdGrey);
				qglVertex3fv(vec3_origin);
				VectorAdd(light->l.target, light->l.up, tmp);
				qglVertex3fv(tmp);

				qglEnd();
				//qglLineWidth(1);

				if(light->shadowLOD == 0)
				{
					R_DebugBoundingBox(vec3_origin, light->localBounds[0], light->localBounds[1], colorRed);
				}
				else if(light->shadowLOD == 1)
				{
					R_DebugBoundingBox(vec3_origin, light->localBounds[0], light->localBounds[1], colorGreen);
				}
				else if(light->shadowLOD == 2)
				{
					R_DebugBoundingBox(vec3_origin, light->localBounds[0], light->localBounds[1], colorBlue);
				}
				else
				{
					R_DebugBoundingBox(vec3_origin, light->localBounds[0], light->localBounds[1], colorMdGrey);
				}

				/*
				   // go back to the world modelview matrix
				   backEnd.or = backEnd.viewParms.world;
				   qglLoadMatrixf(backEnd.viewParms.world.modelViewMatrix);

				   R_DebugBoundingBox(vec3_origin, light->worldBounds[0], light->worldBounds[1], colorWhite);
				 */
			}
		}

		if(!(backEnd.refdef.rdflags & RDF_NOWORLDMODEL))
		{
			GL_Program(0);
			GL_State(0);
			GL_SelectTexture(0);
			GL_Bind(tr.whiteImage);

			for(i = 0; i < tr.world->numLights; i++)
			{
				light = &tr.world->lights[i];

				// set up the transformation matrix
				R_RotateLightForViewParms(light, &backEnd.viewParms, &backEnd.or);
				qglLoadMatrixf(backEnd.or.modelViewMatrix);

				MatrixToVectorsFLU(matrixIdentity, forward, left, up);
				VectorMA(vec3_origin, 16, forward, forward);
				VectorMA(vec3_origin, 16, left, left);
				VectorMA(vec3_origin, 16, up, up);

				// draw axis
				//qglLineWidth(3);
				qglBegin(GL_LINES);

				qglColor4fv(colorRed);
				qglVertex3fv(vec3_origin);
				qglVertex3fv(forward);

				qglColor4fv(colorGreen);
				qglVertex3fv(vec3_origin);
				qglVertex3fv(left);

				qglColor4fv(colorBlue);
				qglVertex3fv(vec3_origin);
				qglVertex3fv(up);

				qglColor4fv(colorYellow);
				qglVertex3fv(vec3_origin);
				VectorSubtract(light->origin, backEnd.or.origin, tmp);
				light->transformed[0] = DotProduct(tmp, backEnd.or.axis[0]);
				light->transformed[1] = DotProduct(tmp, backEnd.or.axis[1]);
				light->transformed[2] = DotProduct(tmp, backEnd.or.axis[2]);
				qglVertex3fv(light->transformed);

				qglColor4fv(colorMagenta);
				qglVertex3fv(vec3_origin);
				qglVertex3fv(light->l.target);

				qglColor4fv(colorCyan);
				qglVertex3fv(vec3_origin);
				qglVertex3fv(light->l.right);

				qglColor4fv(colorWhite);
				qglVertex3fv(vec3_origin);
				qglVertex3fv(light->l.up);

				qglColor4fv(colorMdGrey);
				qglVertex3fv(vec3_origin);
				VectorAdd(light->l.target, light->l.up, tmp);
				qglVertex3fv(tmp);

				qglEnd();
				//qglLineWidth(1);

				if(light->shadowLOD == 0)
				{
					R_DebugBoundingBox(vec3_origin, light->localBounds[0], light->localBounds[1], colorRed);
				}
				else if(light->shadowLOD == 1)
				{
					R_DebugBoundingBox(vec3_origin, light->localBounds[0], light->localBounds[1], colorGreen);
				}
				else if(light->shadowLOD == 2)
				{
					R_DebugBoundingBox(vec3_origin, light->localBounds[0], light->localBounds[1], colorBlue);
				}
				else
				{
					R_DebugBoundingBox(vec3_origin, light->localBounds[0], light->localBounds[1], colorMdGrey);
				}

				/*
				   // go back to the world modelview matrix
				   backEnd.or = backEnd.viewParms.world;
				   qglLoadMatrixf(backEnd.viewParms.world.modelViewMatrix);

				   R_DebugBoundingBox(vec3_origin, light->worldBounds[0], light->worldBounds[1], colorYellow);
				 */
			}
		}
	}

	if(r_showLightInteractions->integer)
	{
		interaction_t  *ia;
		int             iaCount;
		trRefEntity_t  *entity;
		surfaceType_t  *surface;

		GL_Program(0);
		GL_State(0);
		GL_SelectTexture(0);
		GL_Bind(tr.whiteImage);

		for(iaCount = 0, ia = &interactions[0]; iaCount < numInteractions;)
		{
			backEnd.currentEntity = entity = ia->entity;
			surface = ia->surface;

			R_RotateEntityForViewParms(entity, &backEnd.viewParms, &backEnd.or);
			qglLoadMatrixf(backEnd.or.modelViewMatrix);

			if(*surface == SF_FACE)
			{
				srfSurfaceFace_t *face;

				face = (srfSurfaceFace_t *) surface;
				R_DebugBoundingBox(vec3_origin, face->bounds[0], face->bounds[1], colorYellow);
			}
			else if(*surface == SF_GRID)
			{
				srfGridMesh_t  *grid;

				grid = (srfGridMesh_t *) surface;
				R_DebugBoundingBox(vec3_origin, grid->meshBounds[0], grid->meshBounds[1], colorMagenta);
			}
			else if(*surface == SF_TRIANGLES)
			{
				srfTriangles_t *tri;

				tri = (srfTriangles_t *) surface;
				R_DebugBoundingBox(vec3_origin, tri->bounds[0], tri->bounds[1], colorCyan);
			}
			else if(*surface == SF_MDX)
			{
				R_DebugBoundingBox(vec3_origin, entity->localBounds[0], entity->localBounds[1], colorMdGrey);
			}

			if(!ia->next)
			{
				if(iaCount < (numInteractions - 1))
				{
					// jump to next interaction and continue
					ia++;
					iaCount++;
				}
				else
				{
					// increase last time to leave for loop
					iaCount++;
				}
			}
			else
			{
				// just continue
				ia = ia->next;
				iaCount++;
			}
		}

		// go back to the world modelview matrix
		backEnd.or = backEnd.viewParms.world;
		qglLoadMatrixf(backEnd.viewParms.world.modelViewMatrix);
	}

	if(r_showEntityTransforms->integer)
	{
		trRefEntity_t  *ent;
		int             i;

		GL_Program(0);
		GL_State(0);
		GL_SelectTexture(0);
		GL_Bind(tr.whiteImage);

		ent = backEnd.refdef.entities;
		for(i = 0; i < backEnd.refdef.numEntities; i++, ent++)
		{
			if((ent->e.renderfx & RF_THIRD_PERSON) && !backEnd.viewParms.isPortal)
				continue;

			// set up the transformation matrix
			R_RotateEntityForViewParms(ent, &backEnd.viewParms, &backEnd.or);
			qglLoadMatrixf(backEnd.or.modelViewMatrix);

			R_DebugAxis(vec3_origin, matrixIdentity);
			R_DebugBoundingBox(vec3_origin, ent->localBounds[0], ent->localBounds[1], colorMagenta);

			// go back to the world modelview matrix
			backEnd.or = backEnd.viewParms.world;
			qglLoadMatrixf(backEnd.viewParms.world.modelViewMatrix);

			R_DebugBoundingBox(vec3_origin, ent->worldBounds[0], ent->worldBounds[1], colorCyan);
		}
	}

	if(r_showLightScissors->integer)
	{
		interaction_t  *ia;
		int             iaCount;

		GL_Program(0);
		GL_SelectTexture(0);
		GL_Bind(tr.whiteImage);
		GL_State(GLS_POLYMODE_LINE | GLS_DEPTHTEST_DISABLE);
		GL_Cull(CT_TWO_SIDED);

		// set 2D virtual screen size
		qglPushMatrix();
		qglLoadIdentity();
		qglMatrixMode(GL_PROJECTION);
		qglPushMatrix();
		qglLoadIdentity();
		qglOrtho(backEnd.viewParms.viewportX,
				 backEnd.viewParms.viewportX + backEnd.viewParms.viewportWidth,
				 backEnd.viewParms.viewportY, backEnd.viewParms.viewportY + backEnd.viewParms.viewportHeight, -99999, 99999);

		for(iaCount = 0, ia = &interactions[0]; iaCount < numInteractions;)
		{
			if(qglDepthBoundsEXT)
			{
				if(ia->noDepthBoundsTest)
				{
					qglColor4fv(colorRed);
				}
				else
				{
					qglColor4fv(colorGreen);
				}

				qglBegin(GL_QUADS);
				qglVertex2f(ia->scissorX, ia->scissorY);
				qglVertex2f(ia->scissorX + ia->scissorWidth - 1, ia->scissorY);
				qglVertex2f(ia->scissorX + ia->scissorWidth - 1, ia->scissorY + ia->scissorHeight - 1);
				qglVertex2f(ia->scissorX, ia->scissorY + ia->scissorHeight - 1);
				qglEnd();
			}
			else
			{
				qglBegin(GL_QUADS);
				qglColor4fv(colorRed);
				qglVertex2f(ia->scissorX, ia->scissorY);
				qglColor4fv(colorGreen);
				qglVertex2f(ia->scissorX + ia->scissorWidth - 1, ia->scissorY);
				qglColor4fv(colorBlue);
				qglVertex2f(ia->scissorX + ia->scissorWidth - 1, ia->scissorY + ia->scissorHeight - 1);
				qglColor4fv(colorWhite);
				qglVertex2f(ia->scissorX, ia->scissorY + ia->scissorHeight - 1);
				qglEnd();
			}

			if(!ia->next)
			{
				if(iaCount < (numInteractions - 1))
				{
					// jump to next interaction and continue
					ia++;
					iaCount++;
				}
				else
				{
					// increase last time to leave for loop
					iaCount++;
				}
			}
			else
			{
				// just continue
				ia = ia->next;
				iaCount++;
			}
		}

		qglPopMatrix();
		qglMatrixMode(GL_MODELVIEW);
		qglPopMatrix();
	}

	GL_CheckErrors();
}



/*
==================
RB_RenderDrawSurfList
==================
*/
static void RB_RenderDrawSurfList(drawSurf_t * drawSurfs, int numDrawSurfs, interaction_t * interactions, int numInteractions)
{
	float           originalTime;

	if(r_logFile->integer)
	{
		// don't just call LogComment, or we will get a call to va() every frame!
		GLimp_LogComment(va("--- RB_RenderDrawSurfList( %i surfaces, %i interactions ) ---\n", numDrawSurfs, numInteractions));
	}

	GL_CheckErrors();

	// save original time for entity shader offsets
	originalTime = backEnd.refdef.floatTime;

	// clear the z buffer, set the modelview, etc
	RB_BeginDrawingView();

	backEnd.pc.c_surfaces += numDrawSurfs;

	if(r_deferredShading->integer && glConfig.framebufferObjectAvailable && glConfig.shadingLanguage100Available &&
	   glConfig.textureFloatAvailable)
	{
		//RB_RenderDrawSurfaces(originalTime, drawSurfs, numDrawSurfs, qtrue);
		
		RB_RenderDrawSurfacesIntoGeometricBuffer(originalTime, drawSurfs, numDrawSurfs);
#if 0
		RB_RenderGeometricBuffer();
#endif
		RB_RenderInteractionsDeferred(interactions, numInteractions);
		
		//RB_RenderDrawSurfaces(originalTime, drawSurfs, numDrawSurfs, qfalse);
	}
	else
	{
		// draw everything that is opaque
		RB_RenderDrawSurfaces(originalTime, drawSurfs, numDrawSurfs, qtrue);

#if 0
		// try to cull lights using occlusion queries
		RB_RenderOcclusionQueries(interactions, numInteractions);
#endif

		if(r_shadows->integer == 4)
		{
			// render dynamic shadowing and lighting using shadow mapping
			RB_RenderInteractionsShadowMapped(originalTime, interactions, numInteractions);
		}
		else if(r_shadows->integer == 3)
		{
			// render dynamic shadowing and lighting using stencil shadow volumes
			RB_RenderInteractionsStencilShadowed(originalTime, interactions, numInteractions);
		}
		else
		{
			// render dynamic lighting
			RB_RenderInteractions(originalTime, interactions, numInteractions);
		}

		// draw everything that is translucent
		RB_RenderDrawSurfaces(originalTime, drawSurfs, numDrawSurfs, qfalse);
	}

#if 0
	// add the sun flare
	RB_DrawSun();
#endif

#if 0
	// add light flares on lights that aren't obscured
	RB_RenderFlares();
#endif

	// render debug information
	RB_RenderDebugUtils(interactions, numInteractions);

	GL_CheckErrors();
}


/*
============================================================================

RENDER BACK END THREAD FUNCTIONS

============================================================================
*/


/*
=============
RE_StretchRaw

FIXME: not exactly backend
Stretches a raw 32 bit power of 2 bitmap image over the given screen rectangle.
Used for cinematics.
=============
*/
void RE_StretchRaw(int x, int y, int w, int h, int cols, int rows, const byte * data, int client, qboolean dirty)
{
	int             i, j;
	int             start, end;

	if(!tr.registered)
	{
		return;
	}
	R_SyncRenderThread();

	// we definately want to sync every frame for the cinematics
	qglFinish();

	start = end = 0;
	if(r_speeds->integer)
	{
		start = ri.Milliseconds();
	}

	// make sure rows and cols are powers of 2
	for(i = 0; (1 << i) < cols; i++)
	{
	}
	for(j = 0; (1 << j) < rows; j++)
	{
	}
	if((1 << i) != cols || (1 << j) != rows)
	{
		ri.Error(ERR_DROP, "Draw_StretchRaw: size not a power of 2: %i by %i", cols, rows);
	}

	GL_SelectTexture(0);
	GL_Bind(tr.scratchImage[client]);

	// if the scratchImage isn't in the format we want, specify it as a new texture
	if(cols != tr.scratchImage[client]->width || rows != tr.scratchImage[client]->height)
	{
		tr.scratchImage[client]->width = tr.scratchImage[client]->uploadWidth = cols;
		tr.scratchImage[client]->height = tr.scratchImage[client]->uploadHeight = rows;
		qglTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, cols, rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	}
	else
	{
		if(dirty)
		{
			// otherwise, just subimage upload it so that drivers can tell we are going to be changing
			// it and don't try and do a texture compression
			qglTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, cols, rows, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
	}

	if(r_speeds->integer)
	{
		end = ri.Milliseconds();
		ri.Printf(PRINT_ALL, "qglTexSubImage2D %i, %i: %i msec\n", cols, rows, end - start);
	}

	RB_SetGL2D();

	qglColor3f(tr.identityLight, tr.identityLight, tr.identityLight);

	qglBegin(GL_QUADS);
	qglTexCoord2f(0.5f / cols, 0.5f / rows);
	qglVertex2f(x, y);
	qglTexCoord2f((cols - 0.5f) / cols, 0.5f / rows);
	qglVertex2f(x + w, y);
	qglTexCoord2f((cols - 0.5f) / cols, (rows - 0.5f) / rows);
	qglVertex2f(x + w, y + h);
	qglTexCoord2f(0.5f / cols, (rows - 0.5f) / rows);
	qglVertex2f(x, y + h);
	qglEnd();
}

void RE_UploadCinematic(int w, int h, int cols, int rows, const byte * data, int client, qboolean dirty)
{
	GL_Bind(tr.scratchImage[client]);

	// if the scratchImage isn't in the format we want, specify it as a new texture
	if(cols != tr.scratchImage[client]->width || rows != tr.scratchImage[client]->height)
	{
		tr.scratchImage[client]->width = tr.scratchImage[client]->uploadWidth = cols;
		tr.scratchImage[client]->height = tr.scratchImage[client]->uploadHeight = rows;
		qglTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, cols, rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	}
	else
	{
		if(dirty)
		{
			// otherwise, just subimage upload it so that drivers can tell we are going to be changing
			// it and don't try and do a texture compression
			qglTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, cols, rows, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
	}
}


/*
=============
RB_SetColor
=============
*/
const void     *RB_SetColor(const void *data)
{
	const setColorCommand_t *cmd;

	GLimp_LogComment("--- RB_SetColor ---\n");

	cmd = (const setColorCommand_t *)data;

	backEnd.color2D[0] = cmd->color[0] * 255;
	backEnd.color2D[1] = cmd->color[1] * 255;
	backEnd.color2D[2] = cmd->color[2] * 255;
	backEnd.color2D[3] = cmd->color[3] * 255;

	return (const void *)(cmd + 1);
}

/*
=============
RB_StretchPic
=============
*/
const void     *RB_StretchPic(const void *data)
{
	const stretchPicCommand_t *cmd;
	shader_t       *shader;
	int             numVerts, numIndexes;

	GLimp_LogComment("--- RB_StretchPic ---\n");

	cmd = (const stretchPicCommand_t *)data;

	if(!backEnd.projection2D)
	{
		RB_SetGL2D();
	}

	shader = cmd->shader;
	if(shader != tess.surfaceShader)
	{
		if(tess.numIndexes)
		{
			Tess_End();
		}
		backEnd.currentEntity = &backEnd.entity2D;
		Tess_Begin(Tess_StageIteratorGeneric, shader, NULL, -1, 0, qfalse, qfalse);
	}

	Tess_CheckOverflow(4, 6);
	numVerts = tess.numVertexes;
	numIndexes = tess.numIndexes;

	tess.numVertexes += 4;
	tess.numIndexes += 6;

	tess.indexes[numIndexes] = numVerts + 3;
	tess.indexes[numIndexes + 1] = numVerts + 0;
	tess.indexes[numIndexes + 2] = numVerts + 2;
	tess.indexes[numIndexes + 3] = numVerts + 2;
	tess.indexes[numIndexes + 4] = numVerts + 0;
	tess.indexes[numIndexes + 5] = numVerts + 1;

	*(int *)tess.colors[numVerts] =
		*(int *)tess.colors[numVerts + 1] =
		*(int *)tess.colors[numVerts + 2] = *(int *)tess.colors[numVerts + 3] = *(int *)backEnd.color2D;

	tess.xyz[numVerts][0] = cmd->x;
	tess.xyz[numVerts][1] = cmd->y;
	tess.xyz[numVerts][2] = 0;
	tess.xyz[numVerts][3] = 1;

	tess.texCoords[numVerts][0][0] = cmd->s1;
	tess.texCoords[numVerts][0][1] = cmd->t1;

	tess.xyz[numVerts + 1][0] = cmd->x + cmd->w;
	tess.xyz[numVerts + 1][1] = cmd->y;
	tess.xyz[numVerts + 1][2] = 0;
	tess.xyz[numVerts + 1][3] = 1;

	tess.texCoords[numVerts + 1][0][0] = cmd->s2;
	tess.texCoords[numVerts + 1][0][1] = cmd->t1;

	tess.xyz[numVerts + 2][0] = cmd->x + cmd->w;
	tess.xyz[numVerts + 2][1] = cmd->y + cmd->h;
	tess.xyz[numVerts + 2][2] = 0;
	tess.xyz[numVerts + 2][3] = 1;

	tess.texCoords[numVerts + 2][0][0] = cmd->s2;
	tess.texCoords[numVerts + 2][0][1] = cmd->t2;

	tess.xyz[numVerts + 3][0] = cmd->x;
	tess.xyz[numVerts + 3][1] = cmd->y + cmd->h;
	tess.xyz[numVerts + 3][2] = 0;
	tess.xyz[numVerts + 3][3] = 1;

	tess.texCoords[numVerts + 3][0][0] = cmd->s1;
	tess.texCoords[numVerts + 3][0][1] = cmd->t2;

	return (const void *)(cmd + 1);
}


/*
=============
RB_DrawSurfs
=============
*/
const void     *RB_DrawSurfs(const void *data)
{
	const drawSurfsCommand_t *cmd;

	GLimp_LogComment("--- RB_DrawSurfs ---\n");

	// finish any 2D drawing if needed
	if(tess.numIndexes)
	{
		Tess_End();
	}

	cmd = (const drawSurfsCommand_t *)data;

	backEnd.refdef = cmd->refdef;
	backEnd.viewParms = cmd->viewParms;

	RB_RenderDrawSurfList(cmd->drawSurfs, cmd->numDrawSurfs, cmd->interactions, cmd->numInteractions);

	return (const void *)(cmd + 1);
}


/*
=============
RB_DrawBuffer
=============
*/
const void     *RB_DrawBuffer(const void *data)
{
	const drawBufferCommand_t *cmd;

	GLimp_LogComment("--- RB_DrawBuffer ---\n");

	cmd = (const drawBufferCommand_t *)data;

	qglDrawBuffer(cmd->buffer);

	// clear screen for debugging
	if(r_clear->integer)
	{
//      qglClearColor(1, 0, 0.5, 1);
		qglClearColor(0, 0, 0, 1);
		qglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	return (const void *)(cmd + 1);
}

/*
===============
RB_ShowImages

Draw all the images to the screen, on top of whatever
was there.  This is used to test for texture thrashing.

Also called by RE_EndRegistration
===============
*/
void RB_ShowImages(void)
{
	int             i;
	image_t        *image;
	float           x, y, w, h;
	int             start, end;

	GLimp_LogComment("--- RB_ShowImages ---\n");

	if(!backEnd.projection2D)
	{
		RB_SetGL2D();
	}

	qglClear(GL_COLOR_BUFFER_BIT);

	qglFinish();

	GL_SelectTexture(0);

	start = ri.Milliseconds();

	for(i = 0; i < tr.numImages; i++)
	{
		image = tr.images[i];

		if(image->bits & (IF_RGBA16F | IF_ALPHA32F | IF_RGBA32F))
		{
			// don't render float textures using FFP
			continue;
		}

		w = glConfig.vidWidth / 20;
		h = glConfig.vidHeight / 15;
		x = i % 20 * w;
		y = i / 20 * h;

		// show in proportional size in mode 2
		if(r_showImages->integer == 2)
		{
			w *= image->uploadWidth / 512.0f;
			h *= image->uploadHeight / 512.0f;
		}

		GL_Bind(image);
		qglBegin(GL_QUADS);
		qglTexCoord2f(0, 0);
		qglVertex2f(x, y);
		qglTexCoord2f(1, 0);
		qglVertex2f(x + w, y);
		qglTexCoord2f(1, 1);
		qglVertex2f(x + w, y + h);
		qglTexCoord2f(0, 1);
		qglVertex2f(x, y + h);
		qglEnd();
	}

	qglFinish();

	end = ri.Milliseconds();
	ri.Printf(PRINT_ALL, "%i msec to draw all images\n", end - start);

}

/*
=============
RB_SwapBuffers
=============
*/
const void     *RB_SwapBuffers(const void *data)
{
	const swapBuffersCommand_t *cmd;

	// finish any 2D drawing if needed
	if(tess.numIndexes)
	{
		Tess_End();
	}

	// texture swapping test
	if(r_showImages->integer)
	{
		RB_ShowImages();
	}

	cmd = (const swapBuffersCommand_t *)data;

	// we measure overdraw by reading back the stencil buffer and
	// counting up the number of increments that have happened
	if(r_measureOverdraw->integer)
	{
		int             i;
		long            sum = 0;
		unsigned char  *stencilReadback;

		stencilReadback = ri.Hunk_AllocateTempMemory(glConfig.vidWidth * glConfig.vidHeight);
		qglReadPixels(0, 0, glConfig.vidWidth, glConfig.vidHeight, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, stencilReadback);

		for(i = 0; i < glConfig.vidWidth * glConfig.vidHeight; i++)
		{
			sum += stencilReadback[i];
		}

		backEnd.pc.c_overDraw += sum;
		ri.Hunk_FreeTempMemory(stencilReadback);
	}


	if(!glState.finishCalled)
	{
		qglFinish();
	}

	GLimp_LogComment("***************** RB_SwapBuffers *****************\n\n\n");

	GLimp_EndFrame();

	backEnd.projection2D = qfalse;

	return (const void *)(cmd + 1);
}

/*
====================
RB_ExecuteRenderCommands

This function will be called synchronously if running without
smp extensions, or asynchronously by another thread.
====================
*/
void RB_ExecuteRenderCommands(const void *data)
{
	int             t1, t2;

	GLimp_LogComment("--- RB_ExecuteRenderCommands ---\n");

	t1 = ri.Milliseconds();

	if(!r_smp->integer || data == backEndData[0]->commands.cmds)
	{
		backEnd.smpFrame = 0;
	}
	else
	{
		backEnd.smpFrame = 1;
	}

	while(1)
	{
		switch (*(const int *)data)
		{
			case RC_SET_COLOR:
				data = RB_SetColor(data);
				break;
			case RC_STRETCH_PIC:
				data = RB_StretchPic(data);
				break;
			case RC_DRAW_SURFS:
				data = RB_DrawSurfs(data);
				break;
			case RC_DRAW_BUFFER:
				data = RB_DrawBuffer(data);
				break;
			case RC_SWAP_BUFFERS:
				data = RB_SwapBuffers(data);
				break;
			case RC_SCREENSHOT:
				data = RB_TakeScreenshotCmd(data);
				break;
			case RC_VIDEOFRAME:
				data = RB_TakeVideoFrameCmd(data);
				break;

			case RC_END_OF_LIST:
			default:
				// stop rendering on this thread
				t2 = ri.Milliseconds();
				backEnd.pc.msec = t2 - t1;
				return;
		}
	}
}


/*
================
RB_RenderThread
================
*/
void RB_RenderThread(void)
{
	const void     *data;

	// wait for either a rendering command or a quit command
	while(1)
	{
		// sleep until we have work to do
		data = GLimp_RendererSleep();

		if(!data)
		{
			return;				// all done, renderer is shutting down
		}

		renderThreadActive = qtrue;

		RB_ExecuteRenderCommands(data);

		renderThreadActive = qfalse;
	}
}
