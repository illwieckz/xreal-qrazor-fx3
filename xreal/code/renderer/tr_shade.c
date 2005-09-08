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
// tr_shade.c

#include "tr_local.h"

/*

  THIS ENTIRE FILE IS BACK END

  This file deals with applying shaders to surface data in the tess struct.
*/

enum
{
	ATTR_INDEX_TEXCOORD0 = 8,
	ATTR_INDEX_TEXCOORD1 = 9,
	ATTR_INDEX_TANGENT = 10,
	ATTR_INDEX_BINORMAL = 11,
//  ATTR_INDEX_NORMAL       = 2,
	ATTR_INDEX_DELUXEL = 12,
//  ATTR_INDEX_COLOR        = 3
};

enum
{
	ATTR_VERTEX = (1 << 0),
	ATTR_TEXCOORD0 = (1 << 1),
	ATTR_TEXCOORD1 = (1 << 2),
	ATTR_TANGENT = (1 << 3),
	ATTR_BINORMAL = (1 << 4),
	ATTR_NORMAL = (1 << 5),
	ATTR_DELUXEL = (1 << 6),
	ATTR_COLOR = (1 << 7)
};

static char    *RB_PrintInfoLog(GLhandleARB object)
{
	static char     msg[4096 * 2];
	int             max_length = 0;

	qglGetObjectParameterivARB(object, GL_OBJECT_INFO_LOG_LENGTH_ARB, &max_length);

	if(max_length >= (int)sizeof(msg))
	{
		ri.Error(ERR_DROP, "RB_PrintInfoLog: max length >= sizeof(msg)");
		return NULL;
	}

	qglGetInfoLogARB(object, max_length, &max_length, msg);

	return msg;
}

static void RB_LoadGPUShader(GLhandleARB program, const char *name, GLenum shaderType)
{

	char            filename[MAX_QPATH];
	GLcharARB      *buffer = NULL;
	int             size;
	GLint           compiled;
	GLhandleARB     shader;

	if(shaderType == GL_VERTEX_SHADER_ARB)
	{
		Com_sprintf(filename, sizeof(filename), "glsl/%s_vp.glsl", name);
	}
	else
	{
		Com_sprintf(filename, sizeof(filename), "glsl/%s_fp.glsl", name);
	}

	ri.Printf(PRINT_ALL, "...loading '%s'\n", filename);
	size = ri.FS_ReadFile(filename, (void **)&buffer);
	if(!buffer)
	{
		ri.Error(ERR_DROP, "Couldn't load %s", filename);
	}

	shader = qglCreateShaderObjectARB(shaderType);
	qglShaderSourceARB(shader, 1, (const GLcharARB **)&buffer, &size);

	// compile shader
	qglCompileShaderARB(shader);

	// check if shader compiled
	qglGetObjectParameterivARB(shader, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);
	if(!compiled)
	{
		ri.Error(ERR_DROP, "Couldn't compile %s", RB_PrintInfoLog(shader));
		ri.FS_FreeFile(buffer);
		return;
	}

	// attach shader to program
	qglAttachObjectARB(program, shader);

	// delete shader, no longer needed
	qglDeleteObjectARB(shader);

	ri.FS_FreeFile(buffer);
}

static void RB_LinkProgram(GLhandleARB program)
{
	GLint           linked;

	qglLinkProgramARB(program);

	qglGetObjectParameterivARB(program, GL_OBJECT_LINK_STATUS_ARB, &linked);
	if(!linked)
	{
		ri.Error(ERR_DROP, "%s\nshaders failed to link", RB_PrintInfoLog(program));
	}
}

static void RB_ValidateProgram(GLhandleARB program)
{

	GLint           validated;

	qglValidateProgramARB(program);

	qglGetObjectParameterivARB(program, GL_OBJECT_VALIDATE_STATUS_ARB, &validated);
	if(!validated)
	{
		ri.Error(ERR_DROP, "%s\nshaders failed to validate", RB_PrintInfoLog(program));
	}
}

static void RB_InitGPUShader(shaderProgram_t * program, const char *name, int attribs)
{

	program->program = qglCreateProgramObjectARB();
	program->attribs = attribs;

	RB_LoadGPUShader(program->program, name, GL_VERTEX_SHADER_ARB);
	RB_LoadGPUShader(program->program, name, GL_FRAGMENT_SHADER_ARB);

//  if( attribs & ATTRVERTEX )
//      qglBindAttribLocationARB( program->program, ATTR_INDEX_VERTEX, "attr_Vertex");

	if(attribs & ATTR_TEXCOORD0)
		qglBindAttribLocationARB(program->program, ATTR_INDEX_TEXCOORD0, "attr_TexCoord0");

	if(attribs & ATTR_TEXCOORD1)
		qglBindAttribLocationARB(program->program, ATTR_INDEX_TEXCOORD1, "attr_TexCoord1");

	if(attribs & ATTR_TANGENT)
		qglBindAttribLocationARB(program->program, ATTR_INDEX_TANGENT, "attr_Tangent");

	if(attribs & ATTR_BINORMAL)
		qglBindAttribLocationARB(program->program, ATTR_INDEX_BINORMAL, "attr_Binormal");

//  if( attribs & ATTR_NORMAL )
//      qglBindAttribLocationARB( program->program, ATTR_INDEX_NORMAL, "attr_Color");

	if(attribs & ATTR_DELUXEL)
		qglBindAttribLocationARB(program->program, ATTR_INDEX_DELUXEL, "attr_Deluxel");

//  if( attribs & ATTR_COLOR )
//      qglBindAttribLocationARB( program->program, ATTR_INDEX_COLOR, "attr_Color");

	RB_LinkProgram(program->program);
	RB_ValidateProgram(program->program);
}

void RB_InitGPUShaders(void)
{

	ri.Printf(PRINT_ALL, "------- RB_InitGPUShaders -------\n");

	if(!glConfig2.shadingLanguage100Available)
		return;

	//
	// simple directional lighting ( Q3A style )
	//
	RB_InitGPUShader(&tr.lightShader_D_direct,
					 "lighting_D_direct", ATTR_VERTEX | ATTR_TEXCOORD0 | ATTR_NORMAL);

	tr.lightShader_D_direct.u_DiffuseMap =
		qglGetUniformLocationARB(tr.lightShader_D_direct.program, "u_DiffuseMap");
	tr.lightShader_D_direct.u_AmbientColor =
		qglGetUniformLocationARB(tr.lightShader_D_direct.program, "u_AmbientColor");
	tr.lightShader_D_direct.u_LightDir =
		qglGetUniformLocationARB(tr.lightShader_D_direct.program, "u_LightDir");
	tr.lightShader_D_direct.u_LightColor =
		qglGetUniformLocationARB(tr.lightShader_D_direct.program, "u_LightColor");

	qglUseProgramObjectARB(tr.lightShader_D_direct.program);
	qglUniform1iARB(tr.lightShader_D_direct.u_DiffuseMap, 0);
	qglUseProgramObjectARB(0);

	//
	// simple directional bump mapping
	//
	RB_InitGPUShader(&tr.lightShader_DB_direct,
					 "lighting_DB_direct",
					 ATTR_VERTEX | ATTR_TEXCOORD0 | ATTR_TANGENT | ATTR_BINORMAL | ATTR_NORMAL);

	tr.lightShader_DB_direct.u_DiffuseMap =
		qglGetUniformLocationARB(tr.lightShader_DB_direct.program, "u_DiffuseMap");
	tr.lightShader_DB_direct.u_NormalMap =
		qglGetUniformLocationARB(tr.lightShader_DB_direct.program, "u_NormalMap");
	tr.lightShader_DB_direct.u_AmbientColor =
		qglGetUniformLocationARB(tr.lightShader_DB_direct.program, "u_AmbientColor");
	tr.lightShader_DB_direct.u_LightDir =
		qglGetUniformLocationARB(tr.lightShader_DB_direct.program, "u_LightDir");
	tr.lightShader_DB_direct.u_LightColor =
		qglGetUniformLocationARB(tr.lightShader_DB_direct.program, "u_LightColor");
//	tr.lightShader_DB_direct.u_BumpScale =
//		qglGetUniformLocationARB(tr.lightShader_DB_direct.program, "u_BumpScale");

	qglUseProgramObjectARB(tr.lightShader_DB_direct.program);
	qglUniform1iARB(tr.lightShader_DB_direct.u_DiffuseMap, 0);
	qglUniform1iARB(tr.lightShader_DB_direct.u_NormalMap, 1);
	qglUseProgramObjectARB(0);
	
	//
	// simple directional specular bump mapping
	//
	RB_InitGPUShader(&tr.lightShader_DBS_direct,
					 "lighting_DBS_direct",
					 ATTR_VERTEX | ATTR_TEXCOORD0 | ATTR_TANGENT | ATTR_BINORMAL | ATTR_NORMAL);

	tr.lightShader_DBS_direct.u_DiffuseMap =
		qglGetUniformLocationARB(tr.lightShader_DBS_direct.program, "u_DiffuseMap");
	tr.lightShader_DBS_direct.u_NormalMap =
		qglGetUniformLocationARB(tr.lightShader_DBS_direct.program, "u_NormalMap");
	tr.lightShader_DBS_direct.u_SpecularMap =
		qglGetUniformLocationARB(tr.lightShader_DBS_direct.program, "u_SpecularMap");
	tr.lightShader_DBS_direct.u_ViewOrigin =
		qglGetUniformLocationARB(tr.lightShader_DBS_direct.program, "u_ViewOrigin");
	tr.lightShader_DBS_direct.u_AmbientColor =
		qglGetUniformLocationARB(tr.lightShader_DBS_direct.program, "u_AmbientColor");
	tr.lightShader_DBS_direct.u_LightDir =
		qglGetUniformLocationARB(tr.lightShader_DBS_direct.program, "u_LightDir");
	tr.lightShader_DBS_direct.u_LightColor =
		qglGetUniformLocationARB(tr.lightShader_DBS_direct.program, "u_LightColor");
//	tr.lightShader_DBS_direct.u_BumpScale =
//		qglGetUniformLocationARB(tr.lightShader_DBS_direct.program, "u_BumpScale");
	tr.lightShader_DBS_direct.u_SpecularExponent =
		qglGetUniformLocationARB(tr.lightShader_DBS_direct.program, "u_SpecularExponent");

	qglUseProgramObjectARB(tr.lightShader_DBS_direct.program);
	qglUniform1iARB(tr.lightShader_DBS_direct.u_DiffuseMap, 0);
	qglUniform1iARB(tr.lightShader_DBS_direct.u_NormalMap, 1);
	qglUniform1iARB(tr.lightShader_DBS_direct.u_SpecularMap, 2);
	qglUseProgramObjectARB(0);
}

void RB_ShutdownGPUShaders(void)
{
	ri.Printf(PRINT_ALL, "------- RB_ShutdownGPUShaders -------\n");

	if(!glConfig2.shadingLanguage100Available)
		return;

	if(tr.lightShader_D_direct.program)
	{
		qglDeleteObjectARB(tr.lightShader_D_direct.program);
		tr.lightShader_D_direct.program = 0;
	}

	if(tr.lightShader_DB_direct.program)
	{
		qglDeleteObjectARB(tr.lightShader_DB_direct.program);
		tr.lightShader_DB_direct.program = 0;
	}
	
	if(tr.lightShader_DBS_direct.program)
	{
		qglDeleteObjectARB(tr.lightShader_DBS_direct.program);
		tr.lightShader_DBS_direct.program = 0;
	}
}

static void RB_EnableVertexAttribs(int attribs)
{

//	if(attribs & ATTR_VERTEX)
//		qglEnableClientState(GL_VERTEX_ARRAY);

	if(attribs & ATTR_TEXCOORD0)
		qglEnableVertexAttribArrayARB(ATTR_INDEX_TEXCOORD0);

	if(attribs & ATTR_TEXCOORD1)
		qglEnableVertexAttribArrayARB(ATTR_INDEX_TEXCOORD1);

	if(attribs & ATTR_TANGENT)
		qglEnableVertexAttribArrayARB(ATTR_INDEX_TANGENT);

	if(attribs & ATTR_BINORMAL)
		qglEnableVertexAttribArrayARB(ATTR_INDEX_BINORMAL);

	if(attribs & ATTR_NORMAL)
		qglEnableClientState(GL_NORMAL_ARRAY);

	if(attribs & ATTR_DELUXEL)
		qglEnableVertexAttribArrayARB(ATTR_INDEX_DELUXEL);

	if(attribs & ATTR_COLOR)
		qglEnableClientState(GL_COLOR_ARRAY);
}

void RB_DisableVertexAttribs(int attribs)
{

//	if(attribs & ATTR_VERTEX)
//		qglDisableClientState(GL_VERTEX_ARRAY);

	if(attribs & ATTR_TEXCOORD0)
		qglDisableVertexAttribArrayARB(ATTR_INDEX_TEXCOORD0);

	if(attribs & ATTR_TEXCOORD1)
		qglDisableVertexAttribArrayARB(ATTR_INDEX_TEXCOORD1);

	if(attribs & ATTR_TANGENT)
		qglDisableVertexAttribArrayARB(ATTR_INDEX_TANGENT);

	if(attribs & ATTR_BINORMAL)
		qglDisableVertexAttribArrayARB(ATTR_INDEX_BINORMAL);

	if(attribs & ATTR_NORMAL)
		qglDisableClientState(GL_NORMAL_ARRAY);

	if(attribs & ATTR_DELUXEL)
		qglDisableVertexAttribArrayARB(ATTR_INDEX_DELUXEL);

	if(attribs & ATTR_COLOR)
		qglDisableClientState(GL_COLOR_ARRAY);
}


static void RB_SetVertexAttribs(int attribs)
{
	/*
	   if( glConfig2.vertexBufferObjectAvailable && input->vbo_array_buffer ) {
	   TODO
	   }
	   else 


	   if(gl_config.arb_vertex_buffer_object) {
	   gl_state.current_vbo_array_buffer = 0;
	   gl_state.current_vbo_vertexes_ofs = 0;

	   qglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	   }
	 */

//	if(attribs & ATTR_VERTEX)
//		qglVertexPointer(3, GL_FLOAT, 16, tess.xyz);

	if(attribs & ATTR_TEXCOORD0)
		qglVertexAttribPointerARB(ATTR_INDEX_TEXCOORD0, 2, GL_FLOAT, 0, 0, tess.svars.texCoords[0]);

	if(attribs & ATTR_TEXCOORD1)
		qglVertexAttribPointerARB(ATTR_INDEX_TEXCOORD0, 2, GL_FLOAT, 0, 0, tess.svars.texCoords[1]);

	if(attribs & ATTR_TANGENT)
		qglVertexAttribPointerARB(ATTR_INDEX_TANGENT, 3, GL_FLOAT, 0, 16, tess.tangents);

	if(attribs & ATTR_BINORMAL)
		qglVertexAttribPointerARB(ATTR_INDEX_BINORMAL, 3, GL_FLOAT, 0, 16, tess.binormals);

	if(attribs & ATTR_NORMAL)
		qglNormalPointer(GL_FLOAT, 16, tess.normals);

	if(attribs & ATTR_DELUXEL)
		qglVertexAttribPointerARB(ATTR_INDEX_DELUXEL, 3, GL_FLOAT, 0, 16, tess.deluxels);

	if(attribs & ATTR_COLOR)
		qglColorPointer(4, GL_UNSIGNED_BYTE, 0, tess.svars.colors);
}


/*
================
R_ArrayElementDiscrete

This is just for OpenGL conformance testing, it should never be the fastest
================
*/
static void APIENTRY R_ArrayElementDiscrete(GLint index)
{
	qglColor4ubv(tess.svars.colors[index]);
	if(glState.currenttmu)
	{
		qglMultiTexCoord2fARB(0, tess.svars.texCoords[0][index][0], tess.svars.texCoords[0][index][1]);
		qglMultiTexCoord2fARB(1, tess.svars.texCoords[1][index][0], tess.svars.texCoords[1][index][1]);
	}
	else
	{
		qglTexCoord2fv(tess.svars.texCoords[0][index]);
	}
	qglVertex3fv(tess.xyz[index]);
}


/*
===================
R_DrawStripElements

===================
*/
static int      c_vertexes;		// for seeing how long our average strips are
static int      c_begins;
static void R_DrawStripElements(int numIndexes, const glIndex_t * indexes, void (APIENTRY * element) (GLint))
{
	int             i;
	int             last[3] = { -1, -1, -1 };
	qboolean        even;

	c_begins++;

	if(numIndexes <= 0)
	{
		return;
	}

	qglBegin(GL_TRIANGLE_STRIP);

	// prime the strip
	element(indexes[0]);
	element(indexes[1]);
	element(indexes[2]);
	c_vertexes += 3;

	last[0] = indexes[0];
	last[1] = indexes[1];
	last[2] = indexes[2];

	even = qfalse;

	for(i = 3; i < numIndexes; i += 3)
	{
		// odd numbered triangle in potential strip
		if(!even)
		{
			// check previous triangle to see if we're continuing a strip
			if((indexes[i + 0] == last[2]) && (indexes[i + 1] == last[1]))
			{
				element(indexes[i + 2]);
				c_vertexes++;
				assert(indexes[i + 2] < tess.numVertexes);
				even = qtrue;
			}
			// otherwise we're done with this strip so finish it and start
			// a new one
			else
			{
				qglEnd();

				qglBegin(GL_TRIANGLE_STRIP);
				c_begins++;

				element(indexes[i + 0]);
				element(indexes[i + 1]);
				element(indexes[i + 2]);

				c_vertexes += 3;

				even = qfalse;
			}
		}
		else
		{
			// check previous triangle to see if we're continuing a strip
			if((last[2] == indexes[i + 1]) && (last[0] == indexes[i + 0]))
			{
				element(indexes[i + 2]);
				c_vertexes++;

				even = qfalse;
			}
			// otherwise we're done with this strip so finish it and start
			// a new one
			else
			{
				qglEnd();

				qglBegin(GL_TRIANGLE_STRIP);
				c_begins++;

				element(indexes[i + 0]);
				element(indexes[i + 1]);
				element(indexes[i + 2]);
				c_vertexes += 3;

				even = qfalse;
			}
		}

		// cache the last three vertices
		last[0] = indexes[i + 0];
		last[1] = indexes[i + 1];
		last[2] = indexes[i + 2];
	}

	qglEnd();
}




/*
==================
R_DrawElements

Optionally performs our own glDrawElements that looks for strip conditions
instead of using the single glDrawElements call that may be inefficient
without compiled vertex arrays.
==================
*/
static void R_DrawElements(int numIndexes, const glIndex_t * indexes)
{
	int             primitives;

	primitives = r_primitives->integer;

	// default is to use triangles if compiled vertex arrays are present
	if(primitives == 0)
	{
		if(qglLockArraysEXT)
		{
			primitives = 2;
		}
		else
		{
			primitives = 1;
		}
	}


	if(primitives == 2)
	{
		qglDrawElements(GL_TRIANGLES, numIndexes, GL_INDEX_TYPE, indexes);
		return;
	}

	if(primitives == 1)
	{
		R_DrawStripElements(numIndexes, indexes, qglArrayElement);
		return;
	}

	if(primitives == 3)
	{
		R_DrawStripElements(numIndexes, indexes, R_ArrayElementDiscrete);
		return;
	}

	// anything else will cause no drawing
}



/*
=============================================================

SURFACE SHADERS

=============================================================
*/

shaderCommands_t tess;
//static qboolean setArraysOnce;


/*
=================
R_BindAnimatedImage

=================
*/
static void R_BindAnimatedImage(textureBundle_t * bundle)
{
	int             index;

	if(bundle->isVideoMap)
	{
		ri.CIN_RunCinematic(bundle->videoMapHandle);
		ri.CIN_UploadCinematic(bundle->videoMapHandle);
		return;
	}

	if(bundle->numImageAnimations <= 1)
	{
		GL_Bind(bundle->image[0]);
		return;
	}

	// it is necessary to do this messy calc to make sure animations line up
	// exactly with waveforms of the same frequency
	index = myftol(tess.shaderTime * bundle->imageAnimationSpeed * FUNCTABLE_SIZE);
	index >>= FUNCTABLE_SIZE2;

	if(index < 0)
	{
		index = 0;				// may happen with shader time offsets
	}
	index %= bundle->numImageAnimations;

	GL_Bind(bundle->image[index]);
}


/*
================
DrawTris

Draws triangle outlines for debugging
================
*/
static void DrawTris(shaderCommands_t * input)
{
	GL_Bind(tr.whiteImage);
	qglColor3f(1, 1, 1);

	GL_State(GLS_POLYMODE_LINE | GLS_DEPTHMASK_TRUE);
	qglDepthRange(0, 0);

	qglDisableClientState(GL_COLOR_ARRAY);
	qglDisableClientState(GL_TEXTURE_COORD_ARRAY);

	qglVertexPointer(3, GL_FLOAT, 16, input->xyz);	// padded for SIMD

	if(qglLockArraysEXT)
	{
		qglLockArraysEXT(0, input->numVertexes);
		GLimp_LogComment("glLockArraysEXT\n");
	}

	R_DrawElements(input->numIndexes, input->indexes);

	if(qglUnlockArraysEXT)
	{
		qglUnlockArraysEXT();
		GLimp_LogComment("glUnlockArraysEXT\n");
	}
	qglDepthRange(0, 1);
}


/*
================
DrawTangentSpaces

Draws vertex tangent spaces for debugging
================
*/
static void DrawTangentSpaces(shaderCommands_t * input)
{
	int             i;
	vec3_t          temp;

	GL_Bind(tr.whiteImage);
	qglDepthRange(0, 0);		// never occluded
	GL_State(GLS_POLYMODE_LINE | GLS_DEPTHMASK_TRUE);

	qglBegin(GL_LINES);
	for(i = 0; i < input->numVertexes; i++)
	{
		qglColor3f(1, 0, 0);
		qglVertex3fv(input->xyz[i]);
		VectorMA(input->xyz[i], 2, input->tangents[i], temp);
		qglVertex3fv(temp);

		qglColor3f(0, 1, 0);
		qglVertex3fv(input->xyz[i]);
		VectorMA(input->xyz[i], 2, input->binormals[i], temp);
		qglVertex3fv(temp);

		qglColor3f(0, 0, 1);
		qglVertex3fv(input->xyz[i]);
		VectorMA(input->xyz[i], 2, input->normals[i], temp);
		qglVertex3fv(temp);
	}
	qglEnd();

	qglDepthRange(0, 1);
}


/*
================
DrawDeluxels

Draws vertex deluxels for debugging
================
*/
static void DrawDeluxels(shaderCommands_t * input)
{
	int             i;
	vec3_t          temp;

	GL_Bind(tr.whiteImage);
	qglDepthRange(0, 0);		// never occluded
	GL_State(GLS_POLYMODE_LINE | GLS_DEPTHMASK_TRUE);

	qglBegin(GL_LINES);
	for(i = 0; i < input->numVertexes; i++)
	{
		qglColor3f(1, 1, 1);
		qglVertex3fv(input->xyz[i]);

		VectorMA(input->xyz[i], 2, input->deluxels[i], temp);
		qglColor3f(1, 1, 0);
		qglVertex3fv(temp);
	}
	qglEnd();

	qglDepthRange(0, 1);
}

/*
================
DrawNormals

Draws vertex normals for debugging
================
*/
static void DrawNormals(shaderCommands_t * input)
{
	int             i;
	vec3_t          temp;

	GL_Bind(tr.whiteImage);
	qglColor3f(1, 1, 1);
	qglDepthRange(0, 0);		// never occluded
	GL_State(GLS_POLYMODE_LINE | GLS_DEPTHMASK_TRUE);

	qglBegin(GL_LINES);
	for(i = 0; i < input->numVertexes; i++)
	{
		qglVertex3fv(input->xyz[i]);
		VectorMA(input->xyz[i], 2, input->normals[i], temp);
		qglVertex3fv(temp);
	}
	qglEnd();

	qglDepthRange(0, 1);
}

/*
==============
RB_BeginSurface

We must set some things up before beginning any tesselation,
because a surface may be forced to perform a RB_End due
to overflow.
==============
*/
void RB_BeginSurface(shader_t * shader, int fogNum)
{
	shader_t       *state = (shader->remappedShader) ? shader->remappedShader : shader;

	tess.numIndexes = 0;
	tess.numVertexes = 0;
	tess.shader = state;
	tess.fogNum = fogNum;
	tess.dlightBits = 0;		// will be OR'd in by surface functions
	tess.xstages = state->stages;
	tess.numPasses = state->numUnfoggedPasses;
	tess.currentStageIteratorFunc = state->optimalStageIteratorFunc;
//	tess.currentStageIteratorFunc = state->isSky ? RB_StageIteratorSky : RB_StageIteratorGeneric;

	tess.shaderTime = backEnd.refdef.floatTime - tess.shader->timeOffset;
	if(tess.shader->clampTime && tess.shaderTime >= tess.shader->clampTime)
	{
		tess.shaderTime = tess.shader->clampTime;
	}
}


static void DrawSingleTextured(int stage)
{
	shaderStage_t  *pStage;

	pStage = tess.xstages[stage];
	
	qglEnableClientState(GL_TEXTURE_COORD_ARRAY);
	qglTexCoordPointer(2, GL_FLOAT, 0, tess.svars.texCoords[0]);
	
	qglEnableClientState(GL_COLOR_ARRAY);
	qglColorPointer(4, GL_UNSIGNED_BYTE, 0, tess.svars.colors);
	
	GL_State(pStage->stateBits);

	R_BindAnimatedImage(&pStage->bundle[0]);

	R_DrawElements(tess.numIndexes, tess.indexes);
	
	qglDisableClientState(GL_TEXTURE_COORD_ARRAY);
	qglDisableClientState(GL_COLOR_ARRAY);
}

/*
===================
DrawMultiTextured

output = t0 * t1 or t0 + t1

t0 = most upstream according to spec
t1 = most downstream according to spec
===================
*/
static void DrawMultiTextured(int stage)
{
	shaderStage_t  *pStage;

	pStage = tess.xstages[stage];

	qglEnableClientState(GL_TEXTURE_COORD_ARRAY);
	qglTexCoordPointer(2, GL_FLOAT, 0, tess.svars.texCoords[0]);
	
	qglEnableClientState(GL_COLOR_ARRAY);
	qglColorPointer(4, GL_UNSIGNED_BYTE, 0, tess.svars.colors);
	
	GL_State(pStage->stateBits);

	// this is an ugly hack to work around a GeForce driver
	// bug with multitexture and clip planes
	if(backEnd.viewParms.isPortal)
	{
		qglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// base
	GL_SelectTexture(0);
	R_BindAnimatedImage(&pStage->bundle[0]);

	// lightmap/secondary pass
	GL_SelectTexture(1);
	qglEnable(GL_TEXTURE_2D);
	
	qglEnableClientState(GL_TEXTURE_COORD_ARRAY);
	qglTexCoordPointer(2, GL_FLOAT, 0, tess.svars.texCoords[1]);

	if(r_lightmap->integer)
	{
		GL_TexEnv(GL_REPLACE);
	}
	else
	{
		GL_TexEnv(tess.shader->multitextureEnv);
	}

	R_BindAnimatedImage(&pStage->bundle[1]);

	R_DrawElements(tess.numIndexes, tess.indexes);

	// disable texturing on TEXTURE1, then select TEXTURE0
	qglDisableClientState(GL_TEXTURE_COORD_ARRAY);
	qglDisable(GL_TEXTURE_2D);

	GL_SelectTexture(0);
	
	qglDisableClientState(GL_TEXTURE_COORD_ARRAY);
	qglDisableClientState(GL_COLOR_ARRAY);
}

static void DrawPerPixelLit_D(int stage)
{
	shaderStage_t  *pStage;

	vec4_t          ambientLight;
	vec3_t          lightDir;
	vec4_t          directedLight;
	trRefEntity_t  *ent = backEnd.currentEntity;
	
	pStage = tess.xstages[stage];
	
	GL_State(pStage->stateBits);

	// enable shader, set arrays
	qglUseProgramObjectARB(tr.lightShader_D_direct.program);
	
	RB_EnableVertexAttribs(tr.lightShader_D_direct.attribs);
	RB_SetVertexAttribs(tr.lightShader_D_direct.attribs);

	// set uniforms
	VectorScale(ent->ambientLight, (1.0 / 255.0) * 0.3, ambientLight);
	ClampColor(ambientLight);
	VectorScale(ent->directedLight, 1.0 / 255.0, directedLight);
	ClampColor(directedLight);
	VectorCopy(ent->lightDir, lightDir);
	
	qglUniform3fARB(tr.lightShader_D_direct.u_AmbientColor, ambientLight[0], ambientLight[1],
					ambientLight[2]);
	qglUniform3fARB(tr.lightShader_D_direct.u_LightDir, lightDir[0], lightDir[1], lightDir[2]);
	qglUniform3fARB(tr.lightShader_D_direct.u_LightColor, directedLight[0], directedLight[1],
					directedLight[2]);


	// bind TMU
	GL_Bind(pStage->bundle[TB_DIFFUSEMAP].image[0]);
	
	R_DrawElements(tess.numIndexes, tess.indexes);

	// disable GPU shader
	RB_DisableVertexAttribs(tr.lightShader_D_direct.attribs);
	qglUseProgramObjectARB(0);
}


static void DrawPerPixelLit_DB(int stage)
{
	shaderStage_t  *pStage;

	vec4_t          ambientLight;
	vec3_t          lightDir;
	vec4_t          directedLight;
	trRefEntity_t  *ent = backEnd.currentEntity;
	
	pStage = tess.xstages[stage];

	// enable shader, set arrays
	qglUseProgramObjectARB(tr.lightShader_DB_direct.program);
	
	RB_EnableVertexAttribs(tr.lightShader_DB_direct.attribs);
	RB_SetVertexAttribs(tr.lightShader_DB_direct.attribs);
	
	GL_State(pStage->stateBits);

	// set uniforms
	VectorScale(ent->ambientLight, (1.0 / 255.0) * 0.3, ambientLight);
	ClampColor(ambientLight);
	VectorScale(ent->directedLight, 1.0 / 255.0, directedLight);
	ClampColor(directedLight);
	VectorCopy(ent->lightDir, lightDir);
	
	qglUniform3fARB(tr.lightShader_DB_direct.u_AmbientColor, ambientLight[0], ambientLight[1],
					ambientLight[2]);
	qglUniform3fARB(tr.lightShader_DB_direct.u_LightDir, lightDir[0], lightDir[1], lightDir[2]);
	qglUniform3fARB(tr.lightShader_DB_direct.u_LightColor, directedLight[0], directedLight[1],
					directedLight[2]);


	// bind diffusemap
	GL_SelectTexture(0);
	GL_Bind(pStage->bundle[TB_DIFFUSEMAP].image[0]);
	
	// bind normalmap
	GL_SelectTexture(1);
	GL_Bind(pStage->bundle[TB_NORMALMAP].image[0]);
	
	R_DrawElements(tess.numIndexes, tess.indexes);

	// clean up
	GL_SelectTexture(0);
	RB_DisableVertexAttribs(tr.lightShader_DB_direct.attribs);
	qglUseProgramObjectARB(0);
}

static void DrawPerPixelLit_DBS(int stage)
{
	//TODO
}


/*
===================
ProjectDlightTexture

Perform dynamic lighting with another rendering pass
===================
*/
static void ProjectDlightTexture(void)
{
#if 0
	int             i, l;

#if idppc_altivec
	vec_t           origin0, origin1, origin2;
	float           texCoords0, texCoords1;
	vector float    floatColorVec0, floatColorVec1;
	vector float    modulateVec, colorVec, zero;
	vector short    colorShort;
	vector signed int colorInt;
	vector unsigned char floatColorVecPerm, modulatePerm, colorChar;
	vector unsigned char vSel = (vector unsigned char)(0x00, 0x00, 0x00, 0xff,
													   0x00, 0x00, 0x00, 0xff,
													   0x00, 0x00, 0x00, 0xff,
													   0x00, 0x00, 0x00, 0xff);
#else
	vec3_t          origin;
#endif
	float          *texCoords;
	byte           *colors;
	byte            clipBits[SHADER_MAX_VERTEXES];
	MAC_STATIC float texCoordsArray[SHADER_MAX_VERTEXES][2];
	byte            colorArray[SHADER_MAX_VERTEXES][4];
	unsigned        hitIndexes[SHADER_MAX_INDEXES];
	int             numIndexes;
	float           scale;
	float           radius;
	vec3_t          floatColor;
	float           modulate;

	if(!backEnd.refdef.numDlights)
	{
		return;
	}

#if idppc_altivec
	// There has to be a better way to do this so that floatColor 
	// and/or modulate are already 16-byte aligned.
	floatColorVecPerm = vec_lvsl(0, (float *)floatColor);
	modulatePerm = vec_lvsl(0, (float *)&modulate);
	modulatePerm = (vector unsigned char)vec_splat((vector unsigned int)modulatePerm, 0);
	zero = (vector float)vec_splat_s8(0);
#endif

	for(l = 0; l < backEnd.refdef.numDlights; l++)
	{
		trRefDlight_t       *dl;

		if(!(tess.dlightBits & (1 << l)))
		{
			continue;			// this surface definately doesn't have any of this light
		}
		texCoords = texCoordsArray[0];
		colors = colorArray[0];

		dl = &backEnd.refdef.dlights[l];
#if idppc_altivec
		origin0 = dl->transformed[0];
		origin1 = dl->transformed[1];
		origin2 = dl->transformed[2];
#else
		VectorCopy(dl->transformed, origin);
#endif
		radius = dl->l.radius;
		scale = 1.0f / radius;

		floatColor[0] = dl->l.color[0] * 255.0f;
		floatColor[1] = dl->l.color[1] * 255.0f;
		floatColor[2] = dl->l.color[2] * 255.0f;
#if idppc_altivec
		floatColorVec0 = vec_ld(0, floatColor);
		floatColorVec1 = vec_ld(11, floatColor);
		floatColorVec0 = vec_perm(floatColorVec0, floatColorVec0, floatColorVecPerm);
#endif
		for(i = 0; i < tess.numVertexes; i++, texCoords += 2, colors += 4)
		{
#if idppc_altivec
			vec_t           dist0, dist1, dist2;
#else
			vec3_t          dist;
#endif
			int             clip;

			backEnd.pc.c_dlightVertexes++;

#if idppc_altivec
			//VectorSubtract( origin, tess.xyz[i], dist );
			dist0 = origin0 - tess.xyz[i][0];
			dist1 = origin1 - tess.xyz[i][1];
			dist2 = origin2 - tess.xyz[i][2];
			texCoords0 = 0.5f + dist0 * scale;
			texCoords1 = 0.5f + dist1 * scale;

			clip = 0;
			if(texCoords0 < 0.0f)
			{
				clip |= 1;
			}
			else if(texCoords0 > 1.0f)
			{
				clip |= 2;
			}
			if(texCoords1 < 0.0f)
			{
				clip |= 4;
			}
			else if(texCoords1 > 1.0f)
			{
				clip |= 8;
			}
			texCoords[0] = texCoords0;
			texCoords[1] = texCoords1;

			// modulate the strength based on the height and color
			if(dist2 > radius)
			{
				clip |= 16;
				modulate = 0.0f;
			}
			else if(dist2 < -radius)
			{
				clip |= 32;
				modulate = 0.0f;
			}
			else
			{
				dist2 = Q_fabs(dist2);
				if(dist2 < radius * 0.5f)
				{
					modulate = 1.0f;
				}
				else
				{
					modulate = 2.0f * (radius - dist2) * scale;
				}
			}
			clipBits[i] = clip;

			modulateVec = vec_ld(0, (float *)&modulate);
			modulateVec = vec_perm(modulateVec, modulateVec, modulatePerm);
			colorVec = vec_madd(floatColorVec0, modulateVec, zero);
			colorInt = vec_cts(colorVec, 0);	// RGBx
			colorShort = vec_pack(colorInt, colorInt);	// RGBxRGBx
			colorChar = vec_packsu(colorShort, colorShort);	// RGBxRGBxRGBxRGBx
			colorChar = vec_sel(colorChar, vSel, vSel);	// RGBARGBARGBARGBA replace alpha with 255
			vec_ste((vector unsigned int)colorChar, 0, (unsigned int *)colors);	// store color
#else
			VectorSubtract(origin, tess.xyz[i], dist);
			texCoords[0] = 0.5f + dist[0] * scale;
			texCoords[1] = 0.5f + dist[1] * scale;

			clip = 0;
			if(texCoords[0] < 0.0f)
			{
				clip |= 1;
			}
			else if(texCoords[0] > 1.0f)
			{
				clip |= 2;
			}
			if(texCoords[1] < 0.0f)
			{
				clip |= 4;
			}
			else if(texCoords[1] > 1.0f)
			{
				clip |= 8;
			}
			// modulate the strength based on the height and color
			if(dist[2] > radius)
			{
				clip |= 16;
				modulate = 0.0f;
			}
			else if(dist[2] < -radius)
			{
				clip |= 32;
				modulate = 0.0f;
			}
			else
			{
				dist[2] = Q_fabs(dist[2]);
				if(dist[2] < radius * 0.5f)
				{
					modulate = 1.0f;
				}
				else
				{
					modulate = 2.0f * (radius - dist[2]) * scale;
				}
			}
			clipBits[i] = clip;

			colors[0] = myftol(floatColor[0] * modulate);
			colors[1] = myftol(floatColor[1] * modulate);
			colors[2] = myftol(floatColor[2] * modulate);
			colors[3] = 255;
#endif
		}

		// build a list of triangles that need light
		numIndexes = 0;
		for(i = 0; i < tess.numIndexes; i += 3)
		{
			int             a, b, c;

			a = tess.indexes[i];
			b = tess.indexes[i + 1];
			c = tess.indexes[i + 2];
			if(clipBits[a] & clipBits[b] & clipBits[c])
			{
				continue;		// not lighted
			}
			hitIndexes[numIndexes] = a;
			hitIndexes[numIndexes + 1] = b;
			hitIndexes[numIndexes + 2] = c;
			numIndexes += 3;
		}

		if(!numIndexes)
		{
			continue;
		}

		qglEnableClientState(GL_TEXTURE_COORD_ARRAY);
		qglTexCoordPointer(2, GL_FLOAT, 0, texCoordsArray[0]);

		qglEnableClientState(GL_COLOR_ARRAY);
		qglColorPointer(4, GL_UNSIGNED_BYTE, 0, colorArray);

		GL_Bind(tr.dlightImage);
		// include GLS_DEPTHFUNC_EQUAL so alpha tested surfaces don't add light
		// where they aren't rendered
		if(dl->additive)
		{
			GL_State(GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE | GLS_DEPTHFUNC_EQUAL);
		}
		else
		{
			GL_State(GLS_SRCBLEND_DST_COLOR | GLS_DSTBLEND_ONE | GLS_DEPTHFUNC_EQUAL);
		}
		R_DrawElements(numIndexes, hitIndexes);
		backEnd.pc.c_totalIndexes += numIndexes;
		backEnd.pc.c_dlightIndexes += numIndexes;
	}
#endif
}



/*
===================
RB_FogPass

Blends a fog texture on top of everything else
===================
*/
static void RB_FogPass(void)
{
	fog_t          *fog;
	int             i;

	qglEnableClientState(GL_COLOR_ARRAY);
	qglColorPointer(4, GL_UNSIGNED_BYTE, 0, tess.svars.colors);

	qglEnableClientState(GL_TEXTURE_COORD_ARRAY);
	qglTexCoordPointer(2, GL_FLOAT, 0, tess.svars.texCoords[0]);

	fog = tr.world->fogs + tess.fogNum;

	for(i = 0; i < tess.numVertexes; i++)
	{
		*(int *)&tess.svars.colors[i] = fog->colorInt;
	}

	RB_CalcFogTexCoords((float *)tess.svars.texCoords[0]);

	GL_Bind(tr.fogImage);

	if(tess.shader->fogPass == FP_EQUAL)
	{
		GL_State(GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA | GLS_DEPTHFUNC_EQUAL);
	}
	else
	{
		GL_State(GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);
	}

	R_DrawElements(tess.numIndexes, tess.indexes);
}


/*
===============
ComputeColors
===============
*/
static void ComputeColors(shaderStage_t * pStage)
{
	int             i;

	//
	// rgbGen
	//
	switch (pStage->rgbGen)
	{
		case CGEN_IDENTITY:
			Com_Memset(tess.svars.colors, 0xff, tess.numVertexes * 4);
			break;
		default:
		case CGEN_IDENTITY_LIGHTING:
			Com_Memset(tess.svars.colors, tr.identityLightByte, tess.numVertexes * 4);
			break;
		case CGEN_LIGHTING_DIFFUSE:
			RB_CalcDiffuseColor((unsigned char *)tess.svars.colors);
			break;
		case CGEN_EXACT_VERTEX:
			Com_Memcpy(tess.svars.colors, tess.vertexColors, tess.numVertexes * sizeof(tess.vertexColors[0]));
			break;
		case CGEN_CONST:
			for(i = 0; i < tess.numVertexes; i++)
			{
				*(int *)tess.svars.colors[i] = *(int *)pStage->constantColor;
			}
			break;
		case CGEN_VERTEX:
			if(tr.identityLight == 1)
			{
				Com_Memcpy(tess.svars.colors, tess.vertexColors,
						   tess.numVertexes * sizeof(tess.vertexColors[0]));
			}
			else
			{
				for(i = 0; i < tess.numVertexes; i++)
				{
					tess.svars.colors[i][0] = tess.vertexColors[i][0] * tr.identityLight;
					tess.svars.colors[i][1] = tess.vertexColors[i][1] * tr.identityLight;
					tess.svars.colors[i][2] = tess.vertexColors[i][2] * tr.identityLight;
					tess.svars.colors[i][3] = tess.vertexColors[i][3];
				}
			}
			break;
		case CGEN_ONE_MINUS_VERTEX:
			if(tr.identityLight == 1)
			{
				for(i = 0; i < tess.numVertexes; i++)
				{
					tess.svars.colors[i][0] = 255 - tess.vertexColors[i][0];
					tess.svars.colors[i][1] = 255 - tess.vertexColors[i][1];
					tess.svars.colors[i][2] = 255 - tess.vertexColors[i][2];
				}
			}
			else
			{
				for(i = 0; i < tess.numVertexes; i++)
				{
					tess.svars.colors[i][0] = (255 - tess.vertexColors[i][0]) * tr.identityLight;
					tess.svars.colors[i][1] = (255 - tess.vertexColors[i][1]) * tr.identityLight;
					tess.svars.colors[i][2] = (255 - tess.vertexColors[i][2]) * tr.identityLight;
				}
			}
			break;
		case CGEN_FOG:
		{
			fog_t          *fog;

			fog = tr.world->fogs + tess.fogNum;

			for(i = 0; i < tess.numVertexes; i++)
			{
				*(int *)&tess.svars.colors[i] = fog->colorInt;
			}
		}
			break;
		case CGEN_WAVEFORM:
			RB_CalcWaveColor(&pStage->rgbWave, (unsigned char *)tess.svars.colors);
			break;
		case CGEN_ENTITY:
			RB_CalcColorFromEntity((unsigned char *)tess.svars.colors);
			break;
		case CGEN_ONE_MINUS_ENTITY:
			RB_CalcColorFromOneMinusEntity((unsigned char *)tess.svars.colors);
			break;
	}

	//
	// alphaGen
	//
	switch (pStage->alphaGen)
	{
		case AGEN_SKIP:
			break;
		case AGEN_IDENTITY:
			if(pStage->rgbGen != CGEN_IDENTITY)
			{
				if((pStage->rgbGen == CGEN_VERTEX && tr.identityLight != 1) || pStage->rgbGen != CGEN_VERTEX)
				{
					for(i = 0; i < tess.numVertexes; i++)
					{
						tess.svars.colors[i][3] = 0xff;
					}
				}
			}
			break;
		case AGEN_CONST:
			if(pStage->rgbGen != CGEN_CONST)
			{
				for(i = 0; i < tess.numVertexes; i++)
				{
					tess.svars.colors[i][3] = pStage->constantColor[3];
				}
			}
			break;
		case AGEN_WAVEFORM:
			RB_CalcWaveAlpha(&pStage->alphaWave, (unsigned char *)tess.svars.colors);
			break;
		case AGEN_LIGHTING_SPECULAR:
			RB_CalcSpecularAlpha((unsigned char *)tess.svars.colors);
			break;
		case AGEN_ENTITY:
			RB_CalcAlphaFromEntity((unsigned char *)tess.svars.colors);
			break;
		case AGEN_ONE_MINUS_ENTITY:
			RB_CalcAlphaFromOneMinusEntity((unsigned char *)tess.svars.colors);
			break;
		case AGEN_VERTEX:
			if(pStage->rgbGen != CGEN_VERTEX)
			{
				for(i = 0; i < tess.numVertexes; i++)
				{
					tess.svars.colors[i][3] = tess.vertexColors[i][3];
				}
			}
			break;
		case AGEN_ONE_MINUS_VERTEX:
			for(i = 0; i < tess.numVertexes; i++)
			{
				tess.svars.colors[i][3] = 255 - tess.vertexColors[i][3];
			}
			break;
		case AGEN_PORTAL:
		{
			unsigned char   alpha;

			for(i = 0; i < tess.numVertexes; i++)
			{
				float           len;
				vec3_t          v;

				VectorSubtract(tess.xyz[i], backEnd.viewParms.or.origin, v);
				len = VectorLength(v);

				len /= tess.shader->portalRange;

				if(len < 0)
				{
					alpha = 0;
				}
				else if(len > 1)
				{
					alpha = 0xff;
				}
				else
				{
					alpha = len * 0xff;
				}

				tess.svars.colors[i][3] = alpha;
			}
		}
			break;
	}

	//
	// fog adjustment for colors to fade out as fog increases
	//
	if(tess.fogNum)
	{
		switch (pStage->adjustColorsForFog)
		{
			case ACFF_MODULATE_RGB:
				RB_CalcModulateColorsByFog((unsigned char *)tess.svars.colors);
				break;
			case ACFF_MODULATE_ALPHA:
				RB_CalcModulateAlphasByFog((unsigned char *)tess.svars.colors);
				break;
			case ACFF_MODULATE_RGBA:
				RB_CalcModulateRGBAsByFog((unsigned char *)tess.svars.colors);
				break;
			case ACFF_NONE:
				break;
		}
	}
}


/*
===============
ComputeTexCoords
===============
*/
static void ComputeTexCoords(shaderStage_t * pStage)
{
	int             i;
	int             b;

	for(b = 0; b < MAX_TEXTURE_BUNDLES; b++)
	{
		int             tm;

		//
		// generate the texture coordinates
		//
		switch(pStage->bundle[b].tcGen)
		{
			case TCGEN_SKIP:
				break;
			case TCGEN_IDENTITY:
				Com_Memset(tess.svars.texCoords[b], 0, sizeof(float) * 2 * tess.numVertexes);
				break;
			case TCGEN_TEXTURE:
				for(i = 0; i < tess.numVertexes; i++)
				{
					tess.svars.texCoords[b][i][0] = tess.texCoords[i][0][0];
					tess.svars.texCoords[b][i][1] = tess.texCoords[i][0][1];
				}
				break;
			case TCGEN_LIGHTMAP:
				for(i = 0; i < tess.numVertexes; i++)
				{
					tess.svars.texCoords[b][i][0] = tess.texCoords[i][1][0];
					tess.svars.texCoords[b][i][1] = tess.texCoords[i][1][1];
				}
				break;
			case TCGEN_VECTOR:
				for(i = 0; i < tess.numVertexes; i++)
				{
					tess.svars.texCoords[b][i][0] =
						DotProduct(tess.xyz[i], pStage->bundle[b].tcGenVectors[0]);
					tess.svars.texCoords[b][i][1] =
						DotProduct(tess.xyz[i], pStage->bundle[b].tcGenVectors[1]);
				}
				break;
			case TCGEN_FOG:
				RB_CalcFogTexCoords((float *)tess.svars.texCoords[b]);
				break;
			case TCGEN_ENVIRONMENT_MAPPED:
				RB_CalcEnvironmentTexCoords((float *)tess.svars.texCoords[b]);
				break;
			case TCGEN_BAD:
				return;
		}

		//
		// alter texture coordinates
		//
		for(tm = 0; tm < pStage->bundle[b].numTexMods; tm++)
		{
			// Tr3B - for multiple images per shader stage
			if(pStage->bundle[b].tcGen == TCGEN_SKIP)
				continue;
			
			switch(pStage->bundle[b].texMods[tm].type)
			{
				case TMOD_NONE:
					tm = TR_MAX_TEXMODS;	// break out of for loop
					break;

				case TMOD_TURBULENT:
					RB_CalcTurbulentTexCoords(&pStage->bundle[b].texMods[tm].wave,
											  (float *)tess.svars.texCoords[b]);
					break;

				case TMOD_ENTITY_TRANSLATE:
					RB_CalcScrollTexCoords(backEnd.currentEntity->e.shaderTexCoord,
										   (float *)tess.svars.texCoords[b]);
					break;

				case TMOD_SCROLL:
					RB_CalcScrollTexCoords(pStage->bundle[b].texMods[tm].scroll,
										   (float *)tess.svars.texCoords[b]);
					break;

				case TMOD_SCALE:
					RB_CalcScaleTexCoords(pStage->bundle[b].texMods[tm].scale,
										  (float *)tess.svars.texCoords[b]);
					break;

				case TMOD_STRETCH:
					RB_CalcStretchTexCoords(&pStage->bundle[b].texMods[tm].wave,
											(float *)tess.svars.texCoords[b]);
					break;

				case TMOD_TRANSFORM:
					RB_CalcTransformTexCoords(&pStage->bundle[b].texMods[tm],
											  (float *)tess.svars.texCoords[b]);
					break;

				case TMOD_ROTATE:
					RB_CalcRotateTexCoords(pStage->bundle[b].texMods[tm].rotateSpeed,
										   (float *)tess.svars.texCoords[b]);
					break;

				default:
					ri.Error(ERR_DROP, "ERROR: unknown texmod '%d' in shader '%s'\n",
							 pStage->bundle[b].texMods[tm].type, tess.shader->name);
					break;
			}
		}
	}
}


/*
** RB_IterateStagesGeneric
*/
static void RB_IterateStagesGeneric()
{
	int             stage;

	for(stage = 0; stage < MAX_SHADER_STAGES; stage++)
	{
		shaderStage_t  *pStage = tess.xstages[stage];

		if(!pStage)
		{
			break;
		}

		ComputeColors(pStage);
		ComputeTexCoords(pStage);

		switch(pStage->type)
		{
			case ST_COLOR:
			default:
			{
				if(pStage->bundle[1].image[0] != 0)
				{
					DrawMultiTextured(stage);
				}
				else
				{
					DrawSingleTextured(stage);
				}
				break;
			}
			
			case ST_LIGHTING:
			{
				if(glConfig2.shadingLanguage100Available)
				{
					if(pStage->bundle[TB_NORMALMAP].isNormalMap && r_bumpMapping->integer)
					{
						if(pStage->bundle[TB_SPECULARMAP].isSpecularMap && r_specular->integer)
						{
							DrawPerPixelLit_DBS(stage);
						}
						else
						{
							DrawPerPixelLit_DB(stage);
						}
					}
					else
					{
						DrawPerPixelLit_D(stage);
					}
				}
				else
				{
					DrawSingleTextured(stage);
				}
				
				break;
			}
			
			/*
			case ST_HEATHAZE:
			{
				//TODO
				break;
			}
			*/
		}
		
		// allow skipping out to show just lightmaps during development
		if(r_lightmap->integer && (pStage->bundle[0].isLightmap || pStage->bundle[1].isLightmap))
		{
			break;
		}
	}
}


/*
** RB_StageIteratorGeneric
*/
void RB_StageIteratorGeneric(void)
{
	RB_DeformTessGeometry();

	// log this call
	if(r_logFile->integer)
	{
		// don't just call LogComment, or we will get
		// a call to va() every frame!
		GLimp_LogComment(va("--- RB_StageIteratorGeneric( %s ) ---\n", tess.shader->name));
	}

	// set face culling appropriately
	GL_Cull(tess.shader->cullType);

	// set polygon offset if necessary
	if(tess.shader->polygonOffset)
	{
		qglEnable(GL_POLYGON_OFFSET_FILL);
		qglPolygonOffset(r_offsetFactor->value, r_offsetUnits->value);
	}

	// lock XYZ
	qglVertexPointer(3, GL_FLOAT, 16, tess.xyz);	// padded for SIMD
	if(qglLockArraysEXT)
	{
		qglLockArraysEXT(0, tess.numVertexes);
		GLimp_LogComment("glLockArraysEXT\n");
	}

	// call shader function
	RB_IterateStagesGeneric();

	// now do any dynamic lighting needed
	if(tess.dlightBits && tess.shader->sort <= SS_OPAQUE && !(tess.shader->surfaceFlags & (SURF_NODLIGHT | SURF_SKY)))
	{
		ProjectDlightTexture();
	}
	
	// now do fog
	if(tess.fogNum && tess.shader->fogPass)
	{
		RB_FogPass();
	}

	// unlock arrays
	if(qglUnlockArraysEXT)
	{
		qglUnlockArraysEXT();
		GLimp_LogComment("glUnlockArraysEXT\n");
	}

	// reset polygon offset
	if(tess.shader->polygonOffset)
	{
		qglDisable(GL_POLYGON_OFFSET_FILL);
	}
}


/*
** RB_StageIteratorVertexLitTexture
*/
void RB_StageIteratorVertexLitTexture(void)
{
	shaderCommands_t *input;
	shader_t       *shader;

	input = &tess;

	shader = input->shader;

	//
	// compute colors
	//
	RB_CalcDiffuseColor((unsigned char *)tess.svars.colors);

	//
	// log this call
	//
	if(r_logFile->integer)
	{
		// don't just call LogComment, or we will get
		// a call to va() every frame!
		GLimp_LogComment(va("--- RB_StageIteratorVertexLitTexturedUnfogged( %s ) ---\n", tess.shader->name));
	}

	//
	// set face culling appropriately
	//
	GL_Cull(input->shader->cullType);

	//
	// set arrays and lock
	//
	qglEnableClientState(GL_COLOR_ARRAY);
	qglEnableClientState(GL_TEXTURE_COORD_ARRAY);

	qglColorPointer(4, GL_UNSIGNED_BYTE, 0, tess.svars.colors);
	qglTexCoordPointer(2, GL_FLOAT, 16, tess.texCoords[0][0]);
	qglVertexPointer(3, GL_FLOAT, 16, input->xyz);

	if(qglLockArraysEXT)
	{
		qglLockArraysEXT(0, input->numVertexes);
		GLimp_LogComment("glLockArraysEXT\n");
	}

	//
	// call special shade routine
	//
	R_BindAnimatedImage(&tess.xstages[0]->bundle[0]);
	GL_State(tess.xstages[0]->stateBits);
	R_DrawElements(input->numIndexes, input->indexes);

	// 
	// now do any dynamic lighting needed
	//
	if(tess.dlightBits && tess.shader->sort <= SS_OPAQUE)
	{
		ProjectDlightTexture();
	}

	//
	// now do fog
	//
	if(tess.fogNum && tess.shader->fogPass)
	{
		RB_FogPass();
	}

	// 
	// unlock arrays
	//
	if(qglUnlockArraysEXT)
	{
		qglUnlockArraysEXT();
		GLimp_LogComment("glUnlockArraysEXT\n");
	}
}

void RB_StageIteratorPerPixelLit_D(void)
{

	vec4_t          ambientLight;
	vec3_t          lightDir;
	vec4_t          directedLight;
	trRefEntity_t  *ent = backEnd.currentEntity;

	// compute colors
//  RB_CalcDiffuseColor( ( unsigned char * ) tess.svars.colors );

	// compute deluxels
//  RB_CalcDeluxels();

	// log this call
	if(r_logFile->integer)
	{
		// don't just call LogComment, or we will get
		// a call to va() every frame!
		GLimp_LogComment(va("--- RB_StageIteratorPerPixelLit_D( %s ) ---\n", tess.shader->name));
	}

	// set face culling appropriately
	GL_Cull(tess.shader->cullType);

	// enable shader, set arrays and lock
	qglUseProgramObjectARB(tr.lightShader_D_direct.program);
	RB_EnableVertexAttribs(tr.lightShader_D_direct.attribs);
	RB_SetVertexAttribs(tr.lightShader_D_direct.attribs);

	// set uniforms
	VectorScale(ent->ambientLight, (1.0 / 255.0) * 0.3, ambientLight);
	ClampColor(ambientLight);
	VectorScale(ent->directedLight, 1.0 / 255.0, directedLight);
	ClampColor(directedLight);
	VectorCopy(ent->lightDir, lightDir);
	
	qglUniform3fARB(tr.lightShader_D_direct.u_AmbientColor, ambientLight[0], ambientLight[1],
					ambientLight[2]);
	qglUniform3fARB(tr.lightShader_D_direct.u_LightDir, lightDir[0], lightDir[1], lightDir[2]);
	qglUniform3fARB(tr.lightShader_D_direct.u_LightColor, directedLight[0], directedLight[1],
					directedLight[2]);

	/*
	if(qglLockArraysEXT)
	{
		qglLockArraysEXT(0, tess.numVertexes);
		GLimp_LogComment("glLockArraysEXT\n");
	}
	*/

	// call special shade routine
	R_BindAnimatedImage(&tess.xstages[0]->bundle[TB_DIFFUSEMAP]);
	GL_State(tess.xstages[0]->stateBits);
	R_DrawElements(tess.numIndexes, tess.indexes);

	// disable GPU shader
	qglUseProgramObjectARB(0);
	RB_DisableVertexAttribs(tr.lightShader_D_direct.attribs);

	// now do any dynamic lighting needed
//  if ( tess.dlightBits && tess.shader->sort <= SS_OPAQUE ) {
//      ProjectDlightTexture();
//  }

	// now do fog
	if(tess.fogNum && tess.shader->fogPass)
	{
		RB_FogPass();
	}

	// unlock arrays
	/*
	if(qglUnlockArraysEXT)
	{
		qglUnlockArraysEXT();
		GLimp_LogComment("glUnlockArraysEXT\n");
	}
	*/
}


void RB_StageIteratorPerPixelLit_DB(void)
{

	vec4_t          ambientLight;
	vec3_t          lightDir;
	vec4_t          directedLight;
	trRefEntity_t  *ent = backEnd.currentEntity;

	// compute colors
//  RB_CalcDiffuseColor( ( unsigned char * ) tess.svars.colors );

	// compute deluxels
//  RB_CalcDeluxels();

	// log this call
	if(r_logFile->integer)
	{
		// don't just call LogComment, or we will get
		// a call to va() every frame!
		GLimp_LogComment(va("--- RB_StageIteratorPerPixelLit_DB( %s ) ---\n", tess.shader->name));
	}

	// set face culling appropriately
	GL_Cull(tess.shader->cullType);

	// reset state
	GL_State(GLS_DEFAULT);

	// enable shader, set arrays and lock
	qglUseProgramObjectARB(tr.lightShader_DB_direct.program);
	RB_EnableVertexAttribs(tr.lightShader_DB_direct.attribs);
	RB_SetVertexAttribs(tr.lightShader_DB_direct.attribs);

	// set uniforms
	VectorScale(ent->ambientLight, (1.0 / 255.0) * 0.3, ambientLight);
	ClampColor(ambientLight);
	VectorScale(ent->directedLight, 1.0 / 255.0, directedLight);
	ClampColor(directedLight);
	VectorCopy(ent->lightDir, lightDir);
	
	qglUniform3fARB(tr.lightShader_DB_direct.u_AmbientColor, ambientLight[0], ambientLight[1],
					ambientLight[2]);
	qglUniform3fARB(tr.lightShader_DB_direct.u_LightDir, lightDir[0], lightDir[1], lightDir[2]);
	qglUniform3fARB(tr.lightShader_DB_direct.u_LightColor, directedLight[0], directedLight[1],
					directedLight[2]);
//	qglUniform1fARB(tr.lightShader_DB_direct.u_BumpScale, 1.0);

	/*
	if(qglLockArraysEXT)
	{
		qglLockArraysEXT(0, tess.numVertexes);
		GLimp_LogComment("glLockArraysEXT\n");
	}
	*/

	// call special shade routine
	GL_SelectTexture(0);
	R_BindAnimatedImage(&tess.xstages[0]->bundle[TB_DIFFUSEMAP]);

	GL_SelectTexture(1);
	R_BindAnimatedImage(&tess.xstages[0]->bundle[TB_NORMALMAP]);

	R_DrawElements(tess.numIndexes, tess.indexes);

	// disable GPU shader
	qglUseProgramObjectARB(0);

	// switch back to default TMU
	GL_SelectTexture(0);

	RB_DisableVertexAttribs(tr.lightShader_DB_direct.attribs);


	// now do any dynamic lighting needed
//  if ( tess.dlightBits && tess.shader->sort <= SS_OPAQUE ) {
//      ProjectDlightTexture();
//  }

	// now do fog
	if(tess.fogNum && tess.shader->fogPass)
	{
		RB_FogPass();
	}

	// unlock arrays
	/*
	if(qglUnlockArraysEXT)
	{
		qglUnlockArraysEXT();
		GLimp_LogComment("glUnlockArraysEXT\n");
	}
	*/
}

void RB_StageIteratorPerPixelLit_DBS(void)
{

	vec3_t			viewOrigin;
	vec4_t          ambientLight;
	vec3_t          lightDir;
	vec4_t          directedLight;
	trRefEntity_t  *ent = backEnd.currentEntity;

	// compute colors
//  RB_CalcDiffuseColor( ( unsigned char * ) tess.svars.colors );

	// compute deluxels
//  RB_CalcDeluxels();

	// log this call
	if(r_logFile->integer)
	{
		// don't just call LogComment, or we will get
		// a call to va() every frame!
		GLimp_LogComment(va("--- RB_StageIteratorPerPixelLit_DBS( %s ) ---\n", tess.shader->name));
	}

	// set face culling appropriately
	GL_Cull(tess.shader->cullType);

	// reset state
	GL_State(GLS_DEFAULT);

	// enable shader, set arrays and lock
	qglUseProgramObjectARB(tr.lightShader_DBS_direct.program);
	RB_EnableVertexAttribs(tr.lightShader_DBS_direct.attribs);
	RB_SetVertexAttribs(tr.lightShader_DBS_direct.attribs);

	// set uniforms
	VectorCopy(backEnd.or.viewOrigin, viewOrigin);
	VectorScale(ent->ambientLight, (1.0 / 255.0) * 0.3, ambientLight);
	ClampColor(ambientLight);
	VectorScale(ent->directedLight, 1.0 / 255.0, directedLight);
	ClampColor(directedLight);
	VectorCopy(ent->lightDir, lightDir);
	
	qglUniform3fARB(tr.lightShader_DBS_direct.u_AmbientColor, ambientLight[0], ambientLight[1],
					ambientLight[2]);
	qglUniform3fARB(tr.lightShader_DBS_direct.u_ViewOrigin, viewOrigin[0], viewOrigin[1], viewOrigin[2]);
	qglUniform3fARB(tr.lightShader_DBS_direct.u_LightDir, lightDir[0], lightDir[1], lightDir[2]);
	qglUniform3fARB(tr.lightShader_DBS_direct.u_LightColor, directedLight[0], directedLight[1],
					directedLight[2]);				
//	qglUniform1fARB(tr.lightShader_DBS_direct.u_BumpScale, 1.0);
	qglUniform1fARB(tr.lightShader_DBS_direct.u_SpecularExponent, 32.0);

	/*
	if(qglLockArraysEXT)
	{
		qglLockArraysEXT(0, tess.numVertexes);
		GLimp_LogComment("glLockArraysEXT\n");
	}
	*/

	// call special shade routine
	GL_SelectTexture(0);
	R_BindAnimatedImage(&tess.xstages[0]->bundle[TB_DIFFUSEMAP]);

	GL_SelectTexture(1);
	R_BindAnimatedImage(&tess.xstages[0]->bundle[TB_NORMALMAP]);
	
	GL_SelectTexture(2);
	R_BindAnimatedImage(&tess.xstages[0]->bundle[TB_SPECULARMAP]);

	R_DrawElements(tess.numIndexes, tess.indexes);

	// disable GPU shader
	qglUseProgramObjectARB(0);

	// switch back to default TMU
	GL_SelectTexture(0);

	RB_DisableVertexAttribs(tr.lightShader_DB_direct.attribs);

	// now do any dynamic lighting needed
//  if ( tess.dlightBits && tess.shader->sort <= SS_OPAQUE ) {
//      ProjectDlightTexture();
//  }

	// now do fog
	if(tess.fogNum && tess.shader->fogPass)
	{
		RB_FogPass();
	}

	// unlock arrays
	/*
	if(qglUnlockArraysEXT)
	{
		qglUnlockArraysEXT();
		GLimp_LogComment("glUnlockArraysEXT\n");
	}
	*/
}

//#define   REPLACE_MODE

void RB_StageIteratorLightmappedMultitexture(void)
{
	shaderCommands_t *input;

	input = &tess;

	//
	// log this call
	//
	if(r_logFile->integer)
	{
		// don't just call LogComment, or we will get
		// a call to va() every frame!
		GLimp_LogComment(va("--- RB_StageIteratorLightmappedMultitexture( %s ) ---\n", tess.shader->name));
	}

	//
	// set face culling appropriately
	//
	GL_Cull(input->shader->cullType);

	//
	// set color, pointers, and lock
	//
	GL_State(GLS_DEFAULT);
	qglVertexPointer(3, GL_FLOAT, 16, input->xyz);

#ifdef REPLACE_MODE
	qglDisableClientState(GL_COLOR_ARRAY);
	qglColor3f(1, 1, 1);
	qglShadeModel(GL_FLAT);
#else
	qglEnableClientState(GL_COLOR_ARRAY);
	qglColorPointer(4, GL_UNSIGNED_BYTE, 0, tess.constantColor255);
#endif

	//
	// select base stage
	//
	GL_SelectTexture(0);

	qglEnableClientState(GL_TEXTURE_COORD_ARRAY);
	R_BindAnimatedImage(&tess.xstages[0]->bundle[0]);
	qglTexCoordPointer(2, GL_FLOAT, 16, tess.texCoords[0][0]);

	//
	// configure second stage
	//
	GL_SelectTexture(1);
	qglEnable(GL_TEXTURE_2D);
	if(r_lightmap->integer)
	{
		GL_TexEnv(GL_REPLACE);
	}
	else
	{
		GL_TexEnv(GL_MODULATE);
	}
	R_BindAnimatedImage(&tess.xstages[0]->bundle[1]);
	qglEnableClientState(GL_TEXTURE_COORD_ARRAY);
	qglTexCoordPointer(2, GL_FLOAT, 16, tess.texCoords[0][1]);

	//
	// lock arrays
	//
	if(qglLockArraysEXT)
	{
		qglLockArraysEXT(0, input->numVertexes);
		GLimp_LogComment("glLockArraysEXT\n");
	}

	R_DrawElements(input->numIndexes, input->indexes);

	//
	// disable texturing on TEXTURE1, then select TEXTURE0
	//
	qglDisable(GL_TEXTURE_2D);
	qglDisableClientState(GL_TEXTURE_COORD_ARRAY);

	GL_SelectTexture(0);
#ifdef REPLACE_MODE
	GL_TexEnv(GL_MODULATE);
	qglShadeModel(GL_SMOOTH);
#endif

	// 
	// now do any dynamic lighting needed
	//
	if(tess.dlightBits && tess.shader->sort <= SS_OPAQUE)
	{
		ProjectDlightTexture();
	}

	//
	// now do fog
	//
	if(tess.fogNum && tess.shader->fogPass)
	{
		RB_FogPass();
	}

	//
	// unlock arrays
	//
	if(qglUnlockArraysEXT)
	{
		qglUnlockArraysEXT();
		GLimp_LogComment("glUnlockArraysEXT\n");
	}
}

/*
** RB_EndSurface
*/
void RB_EndSurface(void)
{
	shaderCommands_t *input;

	input = &tess;

	if(input->numIndexes == 0)
	{
		return;
	}

	if(input->indexes[SHADER_MAX_INDEXES - 1] != 0)
	{
		ri.Error(ERR_DROP, "RB_EndSurface() - SHADER_MAX_INDEXES hit");
	}
	if(input->xyz[SHADER_MAX_VERTEXES - 1][0] != 0)
	{
		ri.Error(ERR_DROP, "RB_EndSurface() - SHADER_MAX_VERTEXES hit");
	}

	if(tess.shader == tr.shadowShader)
	{
		RB_ShadowTessEnd();
		return;
	}

	// for debugging of sort order issues, stop rendering after a given sort value
	if(r_debugSort->integer && r_debugSort->integer < tess.shader->sort)
	{
		return;
	}

	// update performance counters
	backEnd.pc.c_shaders++;
	backEnd.pc.c_vertexes += tess.numVertexes;
	backEnd.pc.c_indexes += tess.numIndexes;
	backEnd.pc.c_totalIndexes += tess.numIndexes * tess.numPasses;

	// call off to shader specific tess end function
	tess.currentStageIteratorFunc();

	// draw debugging stuff
	if(r_showtris->integer)
	{
		DrawTris(input);
	}
	if(r_shownormals->integer)
	{
		DrawNormals(input);
	}
	if(r_showTangentSpaces->integer)
	{
		DrawTangentSpaces(input);
	}
	if(r_showDeluxels->integer)
	{
		DrawDeluxels(input);
	}

	// clear shader so we can tell we don't have any unclosed surfaces
	tess.numIndexes = 0;

	GLimp_LogComment("----------\n");
}

void RB_EndSurfaceZFill(void)
{
	shaderCommands_t *input;

	input = &tess;

	if(input->numIndexes == 0)
	{
		return;
	}

	if(input->indexes[SHADER_MAX_INDEXES - 1] != 0)
	{
		ri.Error(ERR_DROP, "RB_EndSurface() - SHADER_MAX_INDEXES hit");
	}
	if(input->xyz[SHADER_MAX_VERTEXES - 1][0] != 0)
	{
		ri.Error(ERR_DROP, "RB_EndSurface() - SHADER_MAX_VERTEXES hit");
	}

	// update performance counters
	backEnd.pc.c_shaders++;
	backEnd.pc.c_vertexes += tess.numVertexes;
	backEnd.pc.c_indexes += tess.numIndexes;
	backEnd.pc.c_totalIndexes += tess.numIndexes * tess.numPasses;

	// call off to shader specific tess end function
	tess.currentStageIteratorFunc();

	// clear shader so we can tell we don't have any unclosed surfaces
	tess.numIndexes = 0;

	GLimp_LogComment("----------\n");
}
