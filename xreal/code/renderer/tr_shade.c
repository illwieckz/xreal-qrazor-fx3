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
// tr_shade.c

#include "tr_local.h"

/*

  THIS ENTIRE FILE IS BACK END

  This file deals with applying shaders to surface data in the tess struct.
*/

static char    *GLSL_PrintInfoLog(GLhandleARB object)
{
	static char     msg[4096];
	int             maxLength = 0;

	qglGetObjectParameterivARB(object, GL_OBJECT_INFO_LOG_LENGTH_ARB, &maxLength);

	if(maxLength >= (int)sizeof(msg))
	{
		ri.Error(ERR_DROP, "RB_PrintInfoLog: max length >= sizeof(msg)");
		return NULL;
	}

	qglGetInfoLogARB(object, maxLength, &maxLength, msg);

	return msg;
}

char           *GLSL_PrintShaderSource(GLhandleARB object)
{
	static char     msg[4096];
	int             maxLength = 0;

	qglGetObjectParameterivARB(object, GL_OBJECT_SHADER_SOURCE_LENGTH_ARB, &maxLength);

	if(maxLength >= (int)sizeof(msg))
	{
		ri.Error(ERR_DROP, "RB_PrintShaderSource: max length >= sizeof(msg)");
		return NULL;
	}

	qglGetShaderSourceARB(object, maxLength, &maxLength, msg);

	return msg;
}

static void GLSL_LoadGPUShader(GLhandleARB program, const char *name, GLenum shaderType)
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

	{
		static GLcharARB bufferExtra[32000];
		int             sizeExtra;

		GLcharARB      *bufferFinal = NULL;
		int             sizeFinal;
		
		Com_Memset(bufferExtra, 0, sizeof(bufferExtra));

		// HACK: add ATI's GLSL quirks		
		if(glConfig.hardwareType == GLHW_ATI)
		{
			Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef ATI\n#define ATI\n#endif\n");
		}
		
		/*
		if(glConfig.drawBuffersAvailable && glConfig.maxDrawBuffers >= 4)
		{
			Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef GL_ARB_draw_buffers\n#define GL_ARB_draw_buffers 1\n#endif\n");
		}
		*/
		
		if(glConfig.textureFloatAvailable)
		{
			Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef GL_ARB_texture_float\n#define GL_ARB_texture_float 1\n#endif\n");
		}
		
		if(glConfig.framebufferObjectAvailable && r_shadows->integer == 4)
		{
			Q_strcat(bufferExtra, sizeof(bufferExtra), "#ifndef SHADOWMAPPING\n#define SHADOWMAPPING 1\n#endif\n");
		}

		sizeExtra = strlen(bufferExtra);
		sizeFinal = sizeExtra + size;
		
		//ri.Printf(PRINT_ALL, "GLSL extra: %s\n", bufferExtra);

		bufferFinal = ri.Hunk_AllocateTempMemory(size + sizeExtra);

		strcpy(bufferFinal, bufferExtra);
		Q_strcat(bufferFinal, sizeFinal, buffer);

		qglShaderSourceARB(shader, 1, (const GLcharARB **)&bufferFinal, &sizeFinal);

		ri.Hunk_FreeTempMemory(bufferFinal);
	}

	// compile shader
	qglCompileShaderARB(shader);

	// check if shader compiled
	qglGetObjectParameterivARB(shader, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);
	if(!compiled)
	{
		ri.Error(ERR_DROP, "Couldn't compile %s", GLSL_PrintInfoLog(shader));
		ri.FS_FreeFile(buffer);
		return;
	}
#ifdef _DEBUG
	ri.Printf(PRINT_ALL, "GLSL compile log:\n%s\n", GLSL_PrintInfoLog(shader));
#endif
//  ri.Printf(PRINT_ALL, "%s\n", GLSL_PrintShaderSource(shader));

	// attach shader to program
	qglAttachObjectARB(program, shader);

	// delete shader, no longer needed
	qglDeleteObjectARB(shader);

	ri.FS_FreeFile(buffer);
}

static void GLSL_LinkProgram(GLhandleARB program)
{
	GLint           linked;

	qglLinkProgramARB(program);

	qglGetObjectParameterivARB(program, GL_OBJECT_LINK_STATUS_ARB, &linked);
	if(!linked)
	{
		ri.Error(ERR_DROP, "%s\nshaders failed to link", GLSL_PrintInfoLog(program));
	}
}

static void GLSL_ValidateProgram(GLhandleARB program)
{
	GLint           validated;

	qglValidateProgramARB(program);

	qglGetObjectParameterivARB(program, GL_OBJECT_VALIDATE_STATUS_ARB, &validated);
	if(!validated)
	{
		ri.Error(ERR_DROP, "%s\nshaders failed to validate", GLSL_PrintInfoLog(program));
	}
}

static void GLSL_ShowProgramUniforms(GLhandleARB program)
{
	int             i, count, size, type;
	char            uniformName[1000];

	// install the executables in the program object as part of current state.
	qglUseProgramObjectARB(program);

	// check for GL Errors

	// query the number of active uniforms
	qglGetObjectParameterivARB(program, GL_OBJECT_ACTIVE_UNIFORMS_ARB, &count);

	// Loop over each of the active uniforms, and set their value
	for(i = 0; i < count; i++)
	{
		qglGetActiveUniformARB(program, i, sizeof(uniformName), NULL, &size, &type, uniformName);

		ri.Printf(PRINT_DEVELOPER, "active uniform: '%s'\n", uniformName);
	}

	qglUseProgramObjectARB(0);
}

static void GLSL_InitGPUShader(shaderProgram_t * program, const char *name, int attribs, qboolean fragmentShader)
{

	ri.Printf(PRINT_DEVELOPER, "------- GPU shader -------\n");

	program->program = qglCreateProgramObjectARB();
	program->attribs = attribs;

	GLSL_LoadGPUShader(program->program, name, GL_VERTEX_SHADER_ARB);

	if(fragmentShader)
		GLSL_LoadGPUShader(program->program, name, GL_FRAGMENT_SHADER_ARB);

//  if( attribs & ATTRVERTEX )
//      qglBindAttribLocationARB( program->program, ATTR_INDEX_VERTEX, "attr_Vertex");

	if(attribs & GLCS_TEXCOORD0)
		qglBindAttribLocationARB(program->program, ATTR_INDEX_TEXCOORD0, "attr_TexCoord0");

	if(attribs & GLCS_TEXCOORD1)
		qglBindAttribLocationARB(program->program, ATTR_INDEX_TEXCOORD1, "attr_TexCoord1");

	if(attribs & GLCS_TEXCOORD2)
		qglBindAttribLocationARB(program->program, ATTR_INDEX_TEXCOORD2, "attr_TexCoord2");

	if(attribs & GLCS_TEXCOORD3)
		qglBindAttribLocationARB(program->program, ATTR_INDEX_TEXCOORD3, "attr_TexCoord3");

	if(attribs & GLCS_TANGENT)
		qglBindAttribLocationARB(program->program, ATTR_INDEX_TANGENT, "attr_Tangent");

	if(attribs & GLCS_BINORMAL)
		qglBindAttribLocationARB(program->program, ATTR_INDEX_BINORMAL, "attr_Binormal");

//  if( attribs & GLCS_NORMAL )
//      qglBindAttribLocationARB( program->program, ATTR_INDEX_NORMAL, "attr_Normal");

//  if( attribs & GLCS_COLOR )
//      qglBindAttribLocationARB( program->program, ATTR_INDEX_COLOR, "attr_Color");

	GLSL_LinkProgram(program->program);
}

void GLSL_InitGPUShaders(void)
{
	int             startTime, endTime;
	
	ri.Printf(PRINT_ALL, "------- GLSL_InitGPUShaders -------\n");

	if(!glConfig.shadingLanguage100Available)
		return;
		
	startTime = ri.Milliseconds();

	//
	// single texture rendering
	//
	GLSL_InitGPUShader(&tr.genericSingleShader, "genericSingle", GLCS_VERTEX | GLCS_TEXCOORD0 | GLCS_COLOR, qtrue);

	tr.genericSingleShader.u_ColorMap = qglGetUniformLocationARB(tr.genericSingleShader.program, "u_ColorMap");

	qglUseProgramObjectARB(tr.genericSingleShader.program);
	qglUniform1iARB(tr.genericSingleShader.u_ColorMap, 0);
	qglUseProgramObjectARB(0);
	
	GLSL_ValidateProgram(tr.genericSingleShader.program);
	GLSL_ShowProgramUniforms(tr.genericSingleShader.program);
	
	//
	// simple texture rendering into gl_FragData[0] and everything blackened (good for MRT testing)
	//
	GLSL_InitGPUShader(&tr.genericSingleMRTShader, "genericSingleMRT", GLCS_VERTEX | GLCS_TEXCOORD0 | GLCS_COLOR, qtrue);

	tr.genericSingleMRTShader.u_ColorMap = qglGetUniformLocationARB(tr.genericSingleMRTShader.program, "u_ColorMap");
	tr.genericSingleMRTShader.u_Color = qglGetUniformLocationARB(tr.genericSingleMRTShader.program, "u_Color");

	qglUseProgramObjectARB(tr.genericSingleMRTShader.program);
	qglUniform1iARB(tr.genericSingleMRTShader.u_ColorMap, 0);
	qglUseProgramObjectARB(0);

	GLSL_ValidateProgram(tr.genericSingleMRTShader.program);
	GLSL_ShowProgramUniforms(tr.genericSingleMRTShader.program);

	//
	// black depth fill rendering with textures
	//
	GLSL_InitGPUShader(&tr.depthFillShader, "depthFill", GLCS_VERTEX | GLCS_TEXCOORD0, qtrue);

	tr.depthFillShader.u_ColorMap = qglGetUniformLocationARB(tr.depthFillShader.program, "u_ColorMap");

	qglUseProgramObjectARB(tr.depthFillShader.program);
	qglUniform1iARB(tr.depthFillShader.u_ColorMap, 0);
	qglUseProgramObjectARB(0);

	GLSL_ValidateProgram(tr.depthFillShader.program);
	GLSL_ShowProgramUniforms(tr.depthFillShader.program);
	
	//
	// colored depth test rendering with textures into gl_FragData[1]
	//
	GLSL_InitGPUShader(&tr.depthTestShader, "depthTest", GLCS_VERTEX | GLCS_TEXCOORD0, qtrue);

	tr.depthTestShader.u_ColorMap = qglGetUniformLocationARB(tr.depthTestShader.program, "u_ColorMap");
	tr.depthTestShader.u_CurrentMap = qglGetUniformLocationARB(tr.depthTestShader.program, "u_CurrentMap");
	tr.depthTestShader.u_FBufScale = qglGetUniformLocationARB(tr.depthTestShader.program, "u_FBufScale");
	tr.depthTestShader.u_NPOTScale = qglGetUniformLocationARB(tr.depthTestShader.program, "u_NPOTScale");

	qglUseProgramObjectARB(tr.depthTestShader.program);
	qglUniform1iARB(tr.depthTestShader.u_ColorMap, 0);
	qglUniform1iARB(tr.depthTestShader.u_CurrentMap, 1);
	qglUseProgramObjectARB(0);

	GLSL_ValidateProgram(tr.depthTestShader.program);
	GLSL_ShowProgramUniforms(tr.depthTestShader.program);
	
	//
	// shadow volume extrusion
	//
	GLSL_InitGPUShader(&tr.shadowExtrudeShader, "shadowExtrude", GLCS_VERTEX, qtrue);

	tr.shadowExtrudeShader.u_LightOrigin = qglGetUniformLocationARB(tr.shadowExtrudeShader.program, "u_LightOrigin");

	GLSL_ValidateProgram(tr.shadowExtrudeShader.program);
	GLSL_ShowProgramUniforms(tr.shadowExtrudeShader.program);
	
	//
	// shadow distance RGBA compression
	//
	GLSL_InitGPUShader(&tr.shadowFillShader, "shadowFill", GLCS_VERTEX | GLCS_TEXCOORD0, qtrue);

	tr.shadowFillShader.u_ColorMap = qglGetUniformLocationARB(tr.shadowFillShader.program, "u_ColorMap");
	tr.shadowFillShader.u_LightOrigin = qglGetUniformLocationARB(tr.shadowFillShader.program, "u_LightOrigin");
	tr.shadowFillShader.u_LightRadius = qglGetUniformLocationARB(tr.shadowFillShader.program, "u_LightRadius");
	tr.shadowFillShader.u_ModelMatrix = qglGetUniformLocationARB(tr.shadowFillShader.program, "u_ModelMatrix");

	qglUseProgramObjectARB(tr.shadowFillShader.program);
	qglUniform1iARB(tr.shadowFillShader.u_ColorMap, 0);
	qglUseProgramObjectARB(0);

	GLSL_ValidateProgram(tr.shadowFillShader.program);
	GLSL_ShowProgramUniforms(tr.shadowFillShader.program);

	//
	// directional lighting ( Q3A style )
	//
	GLSL_InitGPUShader(&tr.lightShader_D_direct, "lighting_D_direct", GLCS_VERTEX | GLCS_TEXCOORD0 | GLCS_NORMAL, qtrue);

	tr.lightShader_D_direct.u_DiffuseMap = qglGetUniformLocationARB(tr.lightShader_D_direct.program, "u_DiffuseMap");
	tr.lightShader_D_direct.u_AmbientColor = qglGetUniformLocationARB(tr.lightShader_D_direct.program, "u_AmbientColor");
	tr.lightShader_D_direct.u_LightDir = qglGetUniformLocationARB(tr.lightShader_D_direct.program, "u_LightDir");
	tr.lightShader_D_direct.u_LightColor = qglGetUniformLocationARB(tr.lightShader_D_direct.program, "u_LightColor");

	qglUseProgramObjectARB(tr.lightShader_D_direct.program);
	qglUniform1iARB(tr.lightShader_D_direct.u_DiffuseMap, 0);
	qglUseProgramObjectARB(0);

	GLSL_ValidateProgram(tr.lightShader_D_direct.program);
	GLSL_ShowProgramUniforms(tr.lightShader_D_direct.program);

	//
	// directional bump mapping
	//
	GLSL_InitGPUShader(&tr.lightShader_DB_direct,
					 "lighting_DB_direct",
					 GLCS_VERTEX | GLCS_TEXCOORD0 | GLCS_TEXCOORD1 | GLCS_TANGENT | GLCS_BINORMAL | GLCS_NORMAL, qtrue);

	tr.lightShader_DB_direct.u_DiffuseMap = qglGetUniformLocationARB(tr.lightShader_DB_direct.program, "u_DiffuseMap");
	tr.lightShader_DB_direct.u_NormalMap = qglGetUniformLocationARB(tr.lightShader_DB_direct.program, "u_NormalMap");
	tr.lightShader_DB_direct.u_AmbientColor = qglGetUniformLocationARB(tr.lightShader_DB_direct.program, "u_AmbientColor");
	tr.lightShader_DB_direct.u_LightDir = qglGetUniformLocationARB(tr.lightShader_DB_direct.program, "u_LightDir");
	tr.lightShader_DB_direct.u_LightColor = qglGetUniformLocationARB(tr.lightShader_DB_direct.program, "u_LightColor");
//  tr.lightShader_DB_direct.u_BumpScale =
//      qglGetUniformLocationARB(tr.lightShader_DB_direct.program, "u_BumpScale");

	qglUseProgramObjectARB(tr.lightShader_DB_direct.program);
	qglUniform1iARB(tr.lightShader_DB_direct.u_DiffuseMap, 0);
	qglUniform1iARB(tr.lightShader_DB_direct.u_NormalMap, 1);
	qglUseProgramObjectARB(0);

	GLSL_ValidateProgram(tr.lightShader_DB_direct.program);
	GLSL_ShowProgramUniforms(tr.lightShader_DB_direct.program);

	//
	// directional specular bump mapping
	//
	GLSL_InitGPUShader(&tr.lightShader_DBS_direct,
					 "lighting_DBS_direct",
					 GLCS_VERTEX | GLCS_TEXCOORD0 | GLCS_TEXCOORD1 | GLCS_TEXCOORD2 | GLCS_TANGENT | GLCS_BINORMAL | GLCS_NORMAL,
					 qtrue);

	tr.lightShader_DBS_direct.u_DiffuseMap = qglGetUniformLocationARB(tr.lightShader_DBS_direct.program, "u_DiffuseMap");
	tr.lightShader_DBS_direct.u_NormalMap = qglGetUniformLocationARB(tr.lightShader_DBS_direct.program, "u_NormalMap");
	tr.lightShader_DBS_direct.u_SpecularMap = qglGetUniformLocationARB(tr.lightShader_DBS_direct.program, "u_SpecularMap");
	tr.lightShader_DBS_direct.u_ViewOrigin = qglGetUniformLocationARB(tr.lightShader_DBS_direct.program, "u_ViewOrigin");
	tr.lightShader_DBS_direct.u_AmbientColor = qglGetUniformLocationARB(tr.lightShader_DBS_direct.program, "u_AmbientColor");
	tr.lightShader_DBS_direct.u_LightDir = qglGetUniformLocationARB(tr.lightShader_DBS_direct.program, "u_LightDir");
	tr.lightShader_DBS_direct.u_LightColor = qglGetUniformLocationARB(tr.lightShader_DBS_direct.program, "u_LightColor");
	tr.lightShader_DBS_direct.u_SpecularExponent =
		qglGetUniformLocationARB(tr.lightShader_DBS_direct.program, "u_SpecularExponent");

	qglUseProgramObjectARB(tr.lightShader_DBS_direct.program);
	qglUniform1iARB(tr.lightShader_DBS_direct.u_DiffuseMap, 0);
	qglUniform1iARB(tr.lightShader_DBS_direct.u_NormalMap, 1);
	qglUniform1iARB(tr.lightShader_DBS_direct.u_SpecularMap, 2);
	qglUseProgramObjectARB(0);

	GLSL_ValidateProgram(tr.lightShader_DBS_direct.program);
	GLSL_ShowProgramUniforms(tr.lightShader_DBS_direct.program);

	//
	// omni-directional lighting ( Doom3 style )
	//
	GLSL_InitGPUShader(&tr.lightShader_D_omni, "lighting_D_omni", GLCS_VERTEX | GLCS_TEXCOORD0 | GLCS_NORMAL, qtrue);

	tr.lightShader_D_omni.u_DiffuseMap = qglGetUniformLocationARB(tr.lightShader_D_omni.program, "u_DiffuseMap");
	tr.lightShader_D_omni.u_AttenuationMapXY = qglGetUniformLocationARB(tr.lightShader_D_omni.program, "u_AttenuationMapXY");
	tr.lightShader_D_omni.u_AttenuationMapZ = qglGetUniformLocationARB(tr.lightShader_D_omni.program, "u_AttenuationMapZ");
	tr.lightShader_D_omni.u_LightOrigin = qglGetUniformLocationARB(tr.lightShader_D_omni.program, "u_LightOrigin");
	tr.lightShader_D_omni.u_LightColor = qglGetUniformLocationARB(tr.lightShader_D_omni.program, "u_LightColor");
	tr.lightShader_D_omni.u_LightScale = qglGetUniformLocationARB(tr.lightShader_D_omni.program, "u_LightScale");

	qglUseProgramObjectARB(tr.lightShader_D_omni.program);
	qglUniform1iARB(tr.lightShader_D_omni.u_DiffuseMap, 0);
	qglUniform1iARB(tr.lightShader_D_omni.u_AttenuationMapXY, 1);
	qglUniform1iARB(tr.lightShader_D_omni.u_AttenuationMapZ, 2);
	qglUseProgramObjectARB(0);

	GLSL_ValidateProgram(tr.lightShader_D_omni.program);
	GLSL_ShowProgramUniforms(tr.lightShader_D_omni.program);

	//
	// omni-directional bump mapping ( Doom3 style )
	//
	GLSL_InitGPUShader(&tr.lightShader_DB_omni,
					 "lighting_DB_omni",
					 GLCS_VERTEX | GLCS_TEXCOORD0 | GLCS_TEXCOORD1 | GLCS_TANGENT | GLCS_BINORMAL | GLCS_NORMAL, qtrue);

	tr.lightShader_DB_omni.u_DiffuseMap = qglGetUniformLocationARB(tr.lightShader_DB_omni.program, "u_DiffuseMap");
	tr.lightShader_DB_omni.u_NormalMap = qglGetUniformLocationARB(tr.lightShader_DB_omni.program, "u_NormalMap");
	tr.lightShader_DB_omni.u_AttenuationMapXY = qglGetUniformLocationARB(tr.lightShader_DB_omni.program, "u_AttenuationMapXY");
	tr.lightShader_DB_omni.u_AttenuationMapZ = qglGetUniformLocationARB(tr.lightShader_DB_omni.program, "u_AttenuationMapZ");
	tr.lightShader_DB_omni.u_LightOrigin = qglGetUniformLocationARB(tr.lightShader_DB_omni.program, "u_LightOrigin");
	tr.lightShader_DB_omni.u_LightColor = qglGetUniformLocationARB(tr.lightShader_DB_omni.program, "u_LightColor");
	tr.lightShader_DB_omni.u_LightScale = qglGetUniformLocationARB(tr.lightShader_DB_omni.program, "u_LightScale");

	qglUseProgramObjectARB(tr.lightShader_DB_omni.program);
	qglUniform1iARB(tr.lightShader_DB_omni.u_DiffuseMap, 0);
	qglUniform1iARB(tr.lightShader_DB_omni.u_NormalMap, 1);
	qglUniform1iARB(tr.lightShader_DB_omni.u_AttenuationMapXY, 2);
	qglUniform1iARB(tr.lightShader_DB_omni.u_AttenuationMapZ, 3);
	qglUseProgramObjectARB(0);

	GLSL_ValidateProgram(tr.lightShader_DB_omni.program);
	GLSL_ShowProgramUniforms(tr.lightShader_DB_omni.program);

	//
	// omni-directional specular bump mapping ( Doom3 style )
	//
	GLSL_InitGPUShader(&tr.lightShader_DBS_omni,
					 "lighting_DBS_omni",
					 GLCS_VERTEX | GLCS_TEXCOORD0 | GLCS_TEXCOORD1 | GLCS_TEXCOORD2 | GLCS_TANGENT | GLCS_BINORMAL | GLCS_NORMAL,
					 qtrue);

	tr.lightShader_DBS_omni.u_DiffuseMap = qglGetUniformLocationARB(tr.lightShader_DBS_omni.program, "u_DiffuseMap");
	tr.lightShader_DBS_omni.u_NormalMap = qglGetUniformLocationARB(tr.lightShader_DBS_omni.program, "u_NormalMap");
	tr.lightShader_DBS_omni.u_SpecularMap = qglGetUniformLocationARB(tr.lightShader_DBS_omni.program, "u_SpecularMap");
	tr.lightShader_DBS_omni.u_AttenuationMapXY = qglGetUniformLocationARB(tr.lightShader_DBS_omni.program, "u_AttenuationMapXY");
	tr.lightShader_DBS_omni.u_AttenuationMapZ = qglGetUniformLocationARB(tr.lightShader_DBS_omni.program, "u_AttenuationMapZ");
	tr.lightShader_DBS_omni.u_ViewOrigin = qglGetUniformLocationARB(tr.lightShader_DBS_omni.program, "u_ViewOrigin");
	tr.lightShader_DBS_omni.u_LightOrigin = qglGetUniformLocationARB(tr.lightShader_DBS_omni.program, "u_LightOrigin");
	tr.lightShader_DBS_omni.u_LightColor = qglGetUniformLocationARB(tr.lightShader_DBS_omni.program, "u_LightColor");
	tr.lightShader_DBS_omni.u_LightScale = qglGetUniformLocationARB(tr.lightShader_DBS_omni.program, "u_LightScale");
	tr.lightShader_DBS_omni.u_SpecularExponent = qglGetUniformLocationARB(tr.lightShader_DBS_omni.program, "u_SpecularExponent");

	qglUseProgramObjectARB(tr.lightShader_DBS_omni.program);
	qglUniform1iARB(tr.lightShader_DBS_omni.u_DiffuseMap, 0);
	qglUniform1iARB(tr.lightShader_DBS_omni.u_NormalMap, 1);
	qglUniform1iARB(tr.lightShader_DBS_omni.u_SpecularMap, 2);
	qglUniform1iARB(tr.lightShader_DBS_omni.u_AttenuationMapXY, 3);
	qglUniform1iARB(tr.lightShader_DBS_omni.u_AttenuationMapZ, 4);
	qglUseProgramObjectARB(0);

	GLSL_ValidateProgram(tr.lightShader_DBS_omni.program);
	GLSL_ShowProgramUniforms(tr.lightShader_DBS_omni.program);

	//
	// omni-directional specular parallax bump mapping ( Doom3 style )
	//
	GLSL_InitGPUShader(&tr.lightShader_DBSP_omni,
					 "lighting_DBSP_omni",
					 GLCS_VERTEX | GLCS_TEXCOORD0 | GLCS_TEXCOORD1 | GLCS_TEXCOORD2 | GLCS_TANGENT | GLCS_BINORMAL | GLCS_NORMAL,
					 qtrue);

	tr.lightShader_DBSP_omni.u_DiffuseMap = qglGetUniformLocationARB(tr.lightShader_DBSP_omni.program, "u_DiffuseMap");
	tr.lightShader_DBSP_omni.u_NormalMap = qglGetUniformLocationARB(tr.lightShader_DBSP_omni.program, "u_NormalMap");
	tr.lightShader_DBSP_omni.u_SpecularMap = qglGetUniformLocationARB(tr.lightShader_DBSP_omni.program, "u_SpecularMap");
	tr.lightShader_DBSP_omni.u_AttenuationMapXY =
		qglGetUniformLocationARB(tr.lightShader_DBSP_omni.program, "u_AttenuationMapXY");
	tr.lightShader_DBSP_omni.u_AttenuationMapZ = qglGetUniformLocationARB(tr.lightShader_DBSP_omni.program, "u_AttenuationMapZ");
	tr.lightShader_DBSP_omni.u_ViewOrigin = qglGetUniformLocationARB(tr.lightShader_DBSP_omni.program, "u_ViewOrigin");
	tr.lightShader_DBSP_omni.u_LightOrigin = qglGetUniformLocationARB(tr.lightShader_DBSP_omni.program, "u_LightOrigin");
	tr.lightShader_DBSP_omni.u_LightColor = qglGetUniformLocationARB(tr.lightShader_DBSP_omni.program, "u_LightColor");
	tr.lightShader_DBSP_omni.u_LightScale = qglGetUniformLocationARB(tr.lightShader_DBSP_omni.program, "u_LightScale");
	tr.lightShader_DBSP_omni.u_SpecularExponent =
		qglGetUniformLocationARB(tr.lightShader_DBSP_omni.program, "u_SpecularExponent");
	tr.lightShader_DBSP_omni.u_HeightScale = qglGetUniformLocationARB(tr.lightShader_DBSP_omni.program, "u_HeightScale");
	tr.lightShader_DBSP_omni.u_HeightBias = qglGetUniformLocationARB(tr.lightShader_DBSP_omni.program, "u_HeightBias");

	qglUseProgramObjectARB(tr.lightShader_DBSP_omni.program);
	qglUniform1iARB(tr.lightShader_DBSP_omni.u_DiffuseMap, 0);
	qglUniform1iARB(tr.lightShader_DBSP_omni.u_NormalMap, 1);
	qglUniform1iARB(tr.lightShader_DBSP_omni.u_SpecularMap, 2);
	qglUniform1iARB(tr.lightShader_DBSP_omni.u_AttenuationMapXY, 3);
	qglUniform1iARB(tr.lightShader_DBSP_omni.u_AttenuationMapZ, 4);
	qglUseProgramObjectARB(0);

	GLSL_ValidateProgram(tr.lightShader_DBSP_omni.program);
	GLSL_ShowProgramUniforms(tr.lightShader_DBSP_omni.program);

	//
	// projective lighting ( Doom3 style )
	//
	GLSL_InitGPUShader(&tr.lightShader_D_proj, "lighting_D_proj", GLCS_VERTEX | GLCS_TEXCOORD0 | GLCS_NORMAL, qtrue);

	tr.lightShader_D_proj.u_DiffuseMap = qglGetUniformLocationARB(tr.lightShader_D_proj.program, "u_DiffuseMap");
	tr.lightShader_D_proj.u_AttenuationMapXY = qglGetUniformLocationARB(tr.lightShader_D_proj.program, "u_AttenuationMapXY");
	tr.lightShader_D_proj.u_AttenuationMapZ = qglGetUniformLocationARB(tr.lightShader_D_proj.program, "u_AttenuationMapZ");
	tr.lightShader_D_proj.u_ShadowMap = qglGetUniformLocationARB(tr.lightShader_D_proj.program, "u_ShadowMap");
	tr.lightShader_D_proj.u_LightOrigin = qglGetUniformLocationARB(tr.lightShader_D_proj.program, "u_LightOrigin");
	tr.lightShader_D_proj.u_LightColor = qglGetUniformLocationARB(tr.lightShader_D_proj.program, "u_LightColor");
	tr.lightShader_D_proj.u_LightRadius = qglGetUniformLocationARB(tr.lightShader_D_proj.program, "u_LightRadius");
	tr.lightShader_D_proj.u_LightScale = qglGetUniformLocationARB(tr.lightShader_D_proj.program, "u_LightScale");
	tr.lightShader_D_proj.u_ModelMatrix = qglGetUniformLocationARB(tr.lightShader_D_proj.program, "u_ModelMatrix");

	qglUseProgramObjectARB(tr.lightShader_D_proj.program);
	qglUniform1iARB(tr.lightShader_D_proj.u_DiffuseMap, 0);
	qglUniform1iARB(tr.lightShader_D_proj.u_AttenuationMapXY, 1);
	qglUniform1iARB(tr.lightShader_D_proj.u_AttenuationMapZ, 2);
	qglUniform1iARB(tr.lightShader_D_proj.u_ShadowMap, 3);
	qglUseProgramObjectARB(0);

	GLSL_ValidateProgram(tr.lightShader_D_proj.program);
	GLSL_ShowProgramUniforms(tr.lightShader_D_proj.program);

	//
	// radiosity lighting ( Q3A style )
	//
	GLSL_InitGPUShader(&tr.lightShader_D_radiosity,
					 "lighting_D_radiosity", GLCS_VERTEX | GLCS_TEXCOORD0 | GLCS_TEXCOORD3 | GLCS_NORMAL, qtrue);

	tr.lightShader_D_radiosity.u_DiffuseMap = qglGetUniformLocationARB(tr.lightShader_D_radiosity.program, "u_DiffuseMap");
	tr.lightShader_D_radiosity.u_LightMap = qglGetUniformLocationARB(tr.lightShader_D_radiosity.program, "u_LightMap");

	qglUseProgramObjectARB(tr.lightShader_D_radiosity.program);
	qglUniform1iARB(tr.lightShader_D_radiosity.u_DiffuseMap, 0);
	qglUniform1iARB(tr.lightShader_D_radiosity.u_LightMap, 1);
	qglUseProgramObjectARB(0);

	GLSL_ValidateProgram(tr.lightShader_D_radiosity.program);
	GLSL_ShowProgramUniforms(tr.lightShader_D_radiosity.program);

	//
	// radiosity bump mapping
	//
	GLSL_InitGPUShader(&tr.lightShader_DB_radiosity,
					 "lighting_DB_radiosity",
					 GLCS_VERTEX | GLCS_TEXCOORD0 | GLCS_TEXCOORD1 | GLCS_TEXCOORD3 | GLCS_TANGENT | GLCS_BINORMAL | GLCS_NORMAL,
					 qtrue);

	tr.lightShader_DB_radiosity.u_DiffuseMap = qglGetUniformLocationARB(tr.lightShader_DB_radiosity.program, "u_DiffuseMap");
	tr.lightShader_DB_radiosity.u_NormalMap = qglGetUniformLocationARB(tr.lightShader_DB_radiosity.program, "u_NormalMap");
	tr.lightShader_DB_radiosity.u_LightMap = qglGetUniformLocationARB(tr.lightShader_DB_radiosity.program, "u_LightMap");
	tr.lightShader_DB_radiosity.u_DeluxeMap = qglGetUniformLocationARB(tr.lightShader_DB_radiosity.program, "u_DeluxeMap");

	qglUseProgramObjectARB(tr.lightShader_DB_radiosity.program);
	qglUniform1iARB(tr.lightShader_DB_radiosity.u_DiffuseMap, 0);
	qglUniform1iARB(tr.lightShader_DB_radiosity.u_NormalMap, 1);
	qglUniform1iARB(tr.lightShader_DB_radiosity.u_LightMap, 2);
	qglUniform1iARB(tr.lightShader_DB_radiosity.u_DeluxeMap, 3);
	qglUseProgramObjectARB(0);

	GLSL_ValidateProgram(tr.lightShader_DB_radiosity.program);
	GLSL_ShowProgramUniforms(tr.lightShader_DB_radiosity.program);

	//
	// radiosity specular bump mapping
	//
	GLSL_InitGPUShader(&tr.lightShader_DBS_radiosity,
					 "lighting_DBS_radiosity",
					 GLCS_VERTEX | GLCS_TEXCOORD0 | GLCS_TEXCOORD1 | GLCS_TEXCOORD2 | GLCS_TEXCOORD3 | GLCS_TANGENT |
					 GLCS_BINORMAL | GLCS_NORMAL, qtrue);

	tr.lightShader_DBS_radiosity.u_DiffuseMap = qglGetUniformLocationARB(tr.lightShader_DBS_radiosity.program, "u_DiffuseMap");
	tr.lightShader_DBS_radiosity.u_NormalMap = qglGetUniformLocationARB(tr.lightShader_DBS_radiosity.program, "u_NormalMap");
	tr.lightShader_DBS_radiosity.u_SpecularMap = qglGetUniformLocationARB(tr.lightShader_DBS_radiosity.program, "u_SpecularMap");
	tr.lightShader_DBS_radiosity.u_LightMap = qglGetUniformLocationARB(tr.lightShader_DBS_radiosity.program, "u_LightMap");
	tr.lightShader_DBS_radiosity.u_DeluxeMap = qglGetUniformLocationARB(tr.lightShader_DBS_radiosity.program, "u_DeluxeMap");
	tr.lightShader_DBS_radiosity.u_ViewOrigin = qglGetUniformLocationARB(tr.lightShader_DBS_radiosity.program, "u_ViewOrigin");
	tr.lightShader_DBS_radiosity.u_SpecularExponent =
		qglGetUniformLocationARB(tr.lightShader_DBS_radiosity.program, "u_SpecularExponent");

	qglUseProgramObjectARB(tr.lightShader_DBS_radiosity.program);
	qglUniform1iARB(tr.lightShader_DBS_radiosity.u_DiffuseMap, 0);
	qglUniform1iARB(tr.lightShader_DBS_radiosity.u_NormalMap, 1);
	qglUniform1iARB(tr.lightShader_DBS_radiosity.u_SpecularMap, 2);
	qglUniform1iARB(tr.lightShader_DBS_radiosity.u_LightMap, 3);
	qglUniform1iARB(tr.lightShader_DBS_radiosity.u_DeluxeMap, 4);
	qglUseProgramObjectARB(0);

	GLSL_ValidateProgram(tr.lightShader_DBS_radiosity.program);
	GLSL_ShowProgramUniforms(tr.lightShader_DBS_radiosity.program);

	//
	// cubemap reflection for abitrary polygons
	//
	GLSL_InitGPUShader(&tr.reflectionShader_C, "reflection_C", GLCS_VERTEX | GLCS_NORMAL, qtrue);

	tr.reflectionShader_C.u_ColorMap = qglGetUniformLocationARB(tr.reflectionShader_C.program, "u_ColorMap");
	tr.reflectionShader_C.u_ViewOrigin = qglGetUniformLocationARB(tr.reflectionShader_C.program, "u_ViewOrigin");

	qglUseProgramObjectARB(tr.reflectionShader_C.program);
	qglUniform1iARB(tr.reflectionShader_C.u_ColorMap, 0);
	qglUseProgramObjectARB(0);

	GLSL_ValidateProgram(tr.reflectionShader_C.program);
	GLSL_ShowProgramUniforms(tr.reflectionShader_C.program);

	//
	// bumped cubemap reflection for abitrary polygons ( EMBM )
	//
	GLSL_InitGPUShader(&tr.reflectionShader_CB,
					 "reflection_CB", GLCS_VERTEX | GLCS_TEXCOORD1 | GLCS_TANGENT | GLCS_BINORMAL | GLCS_NORMAL, qtrue);

	tr.reflectionShader_CB.u_ColorMap = qglGetUniformLocationARB(tr.reflectionShader_CB.program, "u_ColorMap");
	tr.reflectionShader_CB.u_NormalMap = qglGetUniformLocationARB(tr.reflectionShader_CB.program, "u_NormalMap");
	tr.reflectionShader_CB.u_ViewOrigin = qglGetUniformLocationARB(tr.reflectionShader_CB.program, "u_ViewOrigin");
	tr.reflectionShader_CB.u_ModelMatrix = qglGetUniformLocationARB(tr.reflectionShader_CB.program, "u_ModelMatrix");

	qglUseProgramObjectARB(tr.reflectionShader_CB.program);
	qglUniform1iARB(tr.reflectionShader_CB.u_ColorMap, 0);
	qglUniform1iARB(tr.reflectionShader_CB.u_NormalMap, 1);
	qglUseProgramObjectARB(0);

	GLSL_ValidateProgram(tr.reflectionShader_CB.program);
	GLSL_ShowProgramUniforms(tr.reflectionShader_CB.program);

	//
	// cubemap refraction for abitrary polygons
	//
	GLSL_InitGPUShader(&tr.refractionShader_C, "refraction_C", GLCS_VERTEX | GLCS_NORMAL, qtrue);

	tr.reflectionShader_C.u_ColorMap = qglGetUniformLocationARB(tr.refractionShader_C.program, "u_ColorMap");
	tr.refractionShader_C.u_ViewOrigin = qglGetUniformLocationARB(tr.refractionShader_C.program, "u_ViewOrigin");
	tr.refractionShader_C.u_RefractionIndex = qglGetUniformLocationARB(tr.refractionShader_C.program, "u_RefractionIndex");
	tr.refractionShader_C.u_FresnelPower = qglGetUniformLocationARB(tr.refractionShader_C.program, "u_FresnelPower");
	tr.refractionShader_C.u_FresnelScale = qglGetUniformLocationARB(tr.refractionShader_C.program, "u_FresnelScale");
	tr.refractionShader_C.u_FresnelBias = qglGetUniformLocationARB(tr.refractionShader_C.program, "u_FresnelBias");

	qglUseProgramObjectARB(tr.refractionShader_C.program);
	qglUniform1iARB(tr.refractionShader_C.u_ColorMap, 0);
	qglUseProgramObjectARB(0);

	GLSL_ValidateProgram(tr.refractionShader_C.program);
	GLSL_ShowProgramUniforms(tr.refractionShader_C.program);

	//
	// cubemap dispersion for abitrary polygons
	//
	GLSL_InitGPUShader(&tr.dispersionShader_C, "dispersion_C", GLCS_VERTEX | GLCS_NORMAL, qtrue);

	tr.reflectionShader_C.u_ColorMap = qglGetUniformLocationARB(tr.dispersionShader_C.program, "u_ColorMap");
	tr.dispersionShader_C.u_ViewOrigin = qglGetUniformLocationARB(tr.dispersionShader_C.program, "u_ViewOrigin");
	tr.dispersionShader_C.u_EtaRatio = qglGetUniformLocationARB(tr.dispersionShader_C.program, "u_EtaRatio");
	tr.dispersionShader_C.u_FresnelPower = qglGetUniformLocationARB(tr.dispersionShader_C.program, "u_FresnelPower");
	tr.dispersionShader_C.u_FresnelScale = qglGetUniformLocationARB(tr.dispersionShader_C.program, "u_FresnelScale");
	tr.dispersionShader_C.u_FresnelBias = qglGetUniformLocationARB(tr.dispersionShader_C.program, "u_FresnelBias");

	qglUseProgramObjectARB(tr.dispersionShader_C.program);
	qglUniform1iARB(tr.dispersionShader_C.u_ColorMap, 0);
	qglUseProgramObjectARB(0);

	GLSL_ValidateProgram(tr.dispersionShader_C.program);
	GLSL_ShowProgramUniforms(tr.dispersionShader_C.program);

	//
	// skybox drawing for abitrary polygons
	//
	GLSL_InitGPUShader(&tr.skyBoxShader, "skybox", GLCS_VERTEX | GLCS_NORMAL, qtrue);

	tr.skyBoxShader.u_ColorMap = qglGetUniformLocationARB(tr.skyBoxShader.program, "u_ColorMap");
	tr.skyBoxShader.u_ViewOrigin = qglGetUniformLocationARB(tr.skyBoxShader.program, "u_ViewOrigin");

	qglUseProgramObjectARB(tr.skyBoxShader.program);
	qglUniform1iARB(tr.skyBoxShader.u_ColorMap, 0);
	qglUseProgramObjectARB(0);

	GLSL_ValidateProgram(tr.skyBoxShader.program);
	GLSL_ShowProgramUniforms(tr.skyBoxShader.program);

	//
	// heatHaze post process effect
	//
	GLSL_InitGPUShader(&tr.heatHazeShader, "heatHaze", GLCS_VERTEX | GLCS_TEXCOORD0, qtrue);

	if(glConfig.hardwareType == GLHW_ATI)
	{
		tr.heatHazeShader.u_ProjectionMatrixTranspose =
			qglGetUniformLocationARB(tr.heatHazeShader.program, "u_ProjectionMatrixTranspose");
	}
	tr.heatHazeShader.u_DeformMagnitude = qglGetUniformLocationARB(tr.heatHazeShader.program, "u_DeformMagnitude");
	tr.heatHazeShader.u_NormalMap = qglGetUniformLocationARB(tr.heatHazeShader.program, "u_NormalMap");
	tr.heatHazeShader.u_CurrentMap = qglGetUniformLocationARB(tr.heatHazeShader.program, "u_CurrentMap");
	tr.heatHazeShader.u_ContrastMap = qglGetUniformLocationARB(tr.heatHazeShader.program, "u_ContrastMap");
	tr.heatHazeShader.u_FBufScale = qglGetUniformLocationARB(tr.heatHazeShader.program, "u_FBufScale");
	tr.heatHazeShader.u_NPOTScale = qglGetUniformLocationARB(tr.heatHazeShader.program, "u_NPOTScale");

	qglUseProgramObjectARB(tr.heatHazeShader.program);
	qglUniform1iARB(tr.heatHazeShader.u_NormalMap, 0);
	qglUniform1iARB(tr.heatHazeShader.u_CurrentMap, 1);
	qglUniform1iARB(tr.heatHazeShader.u_ContrastMap, 2);
	qglUseProgramObjectARB(0);

	GLSL_ValidateProgram(tr.heatHazeShader.program);
	GLSL_ShowProgramUniforms(tr.heatHazeShader.program);

	//
	// bloom post process effect
	//
	GLSL_InitGPUShader(&tr.bloomShader, "bloom", GLCS_VERTEX, qtrue);

	tr.bloomShader.u_ColorMap = qglGetUniformLocationARB(tr.bloomShader.program, "u_ColorMap");
	tr.bloomShader.u_ContrastMap = qglGetUniformLocationARB(tr.bloomShader.program, "u_ContrastMap");
	tr.bloomShader.u_FBufScale = qglGetUniformLocationARB(tr.bloomShader.program, "u_FBufScale");
	tr.bloomShader.u_NPOTScale = qglGetUniformLocationARB(tr.bloomShader.program, "u_NPOTScale");
	tr.bloomShader.u_BlurMagnitude = qglGetUniformLocationARB(tr.bloomShader.program, "u_BlurMagnitude");

	qglUseProgramObjectARB(tr.bloomShader.program);
	qglUniform1iARB(tr.bloomShader.u_ColorMap, 0);
	qglUniform1iARB(tr.bloomShader.u_ContrastMap, 1);
	qglUseProgramObjectARB(0);

	GLSL_ValidateProgram(tr.bloomShader.program);
	GLSL_ShowProgramUniforms(tr.bloomShader.program);

	//
	// contrast post process effect
	//
	GLSL_InitGPUShader(&tr.contrastShader, "contrast", GLCS_VERTEX, qtrue);

	tr.contrastShader.u_ColorMap = qglGetUniformLocationARB(tr.contrastShader.program, "u_ColorMap");
	tr.contrastShader.u_FBufScale = qglGetUniformLocationARB(tr.contrastShader.program, "u_FBufScale");
	tr.contrastShader.u_NPOTScale = qglGetUniformLocationARB(tr.contrastShader.program, "u_NPOTScale");

	qglUseProgramObjectARB(tr.contrastShader.program);
	qglUniform1iARB(tr.contrastShader.u_ColorMap, 0);
	qglUseProgramObjectARB(0);

	GLSL_ValidateProgram(tr.contrastShader.program);
	GLSL_ShowProgramUniforms(tr.contrastShader.program);

	//
	// blurX post process effect
	//
	GLSL_InitGPUShader(&tr.blurXShader, "blurX", GLCS_VERTEX, qtrue);

	tr.blurXShader.u_ColorMap = qglGetUniformLocationARB(tr.blurXShader.program, "u_ColorMap");
	tr.blurXShader.u_FBufScale = qglGetUniformLocationARB(tr.blurXShader.program, "u_FBufScale");
	tr.blurXShader.u_NPOTScale = qglGetUniformLocationARB(tr.blurXShader.program, "u_NPOTScale");

	qglUseProgramObjectARB(tr.blurXShader.program);
	qglUniform1iARB(tr.blurXShader.u_ColorMap, 0);
	qglUseProgramObjectARB(0);

	GLSL_ValidateProgram(tr.blurXShader.program);
	GLSL_ShowProgramUniforms(tr.blurXShader.program);

	//
	// blurY post process effect
	//
	GLSL_InitGPUShader(&tr.blurYShader, "blurY", GLCS_VERTEX, qtrue);

	tr.blurYShader.u_ColorMap = qglGetUniformLocationARB(tr.blurYShader.program, "u_ColorMap");
	tr.blurYShader.u_FBufScale = qglGetUniformLocationARB(tr.blurYShader.program, "u_FBufScale");
	tr.blurYShader.u_NPOTScale = qglGetUniformLocationARB(tr.blurYShader.program, "u_NPOTScale");

	qglUseProgramObjectARB(tr.blurYShader.program);
	qglUniform1iARB(tr.blurYShader.u_ColorMap, 0);
	qglUseProgramObjectARB(0);

	GLSL_ValidateProgram(tr.blurYShader.program);
	GLSL_ShowProgramUniforms(tr.blurYShader.program);

	//
	// rotoscope post process effect
	//
	GLSL_InitGPUShader(&tr.rotoscopeShader, "rotoscope", GLCS_VERTEX | GLCS_TEXCOORD0, qtrue);

	tr.rotoscopeShader.u_ColorMap = qglGetUniformLocationARB(tr.rotoscopeShader.program, "u_ColorMap");
	tr.rotoscopeShader.u_FBufScale = qglGetUniformLocationARB(tr.rotoscopeShader.program, "u_FBufScale");
	tr.rotoscopeShader.u_NPOTScale = qglGetUniformLocationARB(tr.rotoscopeShader.program, "u_NPOTScale");
	tr.rotoscopeShader.u_BlurMagnitude = qglGetUniformLocationARB(tr.rotoscopeShader.program, "u_BlurMagnitude");

	qglUseProgramObjectARB(tr.rotoscopeShader.program);
	qglUniform1iARB(tr.rotoscopeShader.u_ColorMap, 0);
	qglUseProgramObjectARB(0);

	GLSL_ValidateProgram(tr.rotoscopeShader.program);
	GLSL_ShowProgramUniforms(tr.rotoscopeShader.program);
	
	//
	// screen post process effect
	//
	GLSL_InitGPUShader(&tr.screenShader, "screen", GLCS_VERTEX, qtrue);

	tr.screenShader.u_CurrentMap = qglGetUniformLocationARB(tr.screenShader.program, "u_CurrentMap");
	tr.screenShader.u_FBufScale = qglGetUniformLocationARB(tr.screenShader.program, "u_FBufScale");
	tr.screenShader.u_NPOTScale = qglGetUniformLocationARB(tr.screenShader.program, "u_NPOTScale");

	qglUseProgramObjectARB(tr.screenShader.program);
	qglUniform1iARB(tr.screenShader.u_CurrentMap, 0);
	qglUseProgramObjectARB(0);

	GLSL_ValidateProgram(tr.screenShader.program);
	GLSL_ShowProgramUniforms(tr.screenShader.program);
	
	endTime = ri.Milliseconds();
	
	ri.Printf(PRINT_ALL, "GLSL shaders load time = %5.2f seconds\n", (endTime - startTime) / 1000.0);
}

void GLSL_ShutdownGPUShaders(void)
{
	ri.Printf(PRINT_ALL, "------- GLSL_ShutdownGPUShaders -------\n");

	if(!glConfig.shadingLanguage100Available)
		return;

	if(tr.genericSingleShader.program)
	{
		qglDeleteObjectARB(tr.genericSingleShader.program);
		tr.genericSingleShader.program = 0;
	}

	if(tr.genericSingleMRTShader.program)
	{
		qglDeleteObjectARB(tr.genericSingleMRTShader.program);
		tr.genericSingleMRTShader.program = 0;
	}

	if(tr.depthFillShader.program)
	{
		qglDeleteObjectARB(tr.depthFillShader.program);
		tr.depthFillShader.program = 0;
	}
	
	if(tr.depthTestShader.program)
	{
		qglDeleteObjectARB(tr.depthTestShader.program);
		tr.depthTestShader.program = 0;
	}
	
	if(tr.shadowExtrudeShader.program)
	{
		qglDeleteObjectARB(tr.shadowExtrudeShader.program);
		tr.shadowExtrudeShader.program = 0;
	}
	
	if(tr.shadowFillShader.program)
	{
		qglDeleteObjectARB(tr.shadowFillShader.program);
		tr.shadowFillShader.program = 0;
	}

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

	if(tr.lightShader_D_omni.program)
	{
		qglDeleteObjectARB(tr.lightShader_D_omni.program);
		tr.lightShader_D_omni.program = 0;
	}

	if(tr.lightShader_DB_omni.program)
	{
		qglDeleteObjectARB(tr.lightShader_DB_omni.program);
		tr.lightShader_DB_omni.program = 0;
	}

	if(tr.lightShader_DBS_omni.program)
	{
		qglDeleteObjectARB(tr.lightShader_DBS_omni.program);
		tr.lightShader_DBS_omni.program = 0;
	}

	if(tr.lightShader_DBSP_omni.program)
	{
		qglDeleteObjectARB(tr.lightShader_DBSP_omni.program);
		tr.lightShader_DBSP_omni.program = 0;
	}

	if(tr.lightShader_D_proj.program)
	{
		qglDeleteObjectARB(tr.lightShader_D_proj.program);
		tr.lightShader_D_proj.program = 0;
	}

	if(tr.lightShader_D_radiosity.program)
	{
		qglDeleteObjectARB(tr.lightShader_D_radiosity.program);
		tr.lightShader_D_radiosity.program = 0;
	}

	if(tr.lightShader_DB_radiosity.program)
	{
		qglDeleteObjectARB(tr.lightShader_DB_radiosity.program);
		tr.lightShader_DB_radiosity.program = 0;
	}

	if(tr.lightShader_DBS_radiosity.program)
	{
		qglDeleteObjectARB(tr.lightShader_DBS_radiosity.program);
		tr.lightShader_DBS_radiosity.program = 0;
	}

	if(tr.reflectionShader_C.program)
	{
		qglDeleteObjectARB(tr.reflectionShader_C.program);
		tr.reflectionShader_C.program = 0;
	}

	if(tr.reflectionShader_CB.program)
	{
		qglDeleteObjectARB(tr.reflectionShader_CB.program);
		tr.reflectionShader_CB.program = 0;
	}

	if(tr.refractionShader_C.program)
	{
		qglDeleteObjectARB(tr.refractionShader_C.program);
		tr.refractionShader_C.program = 0;
	}

	if(tr.dispersionShader_C.program)
	{
		qglDeleteObjectARB(tr.dispersionShader_C.program);
		tr.dispersionShader_C.program = 0;
	}

	if(tr.skyBoxShader.program)
	{
		qglDeleteObjectARB(tr.skyBoxShader.program);
		tr.skyBoxShader.program = 0;
	}

	if(tr.heatHazeShader.program)
	{
		qglDeleteObjectARB(tr.heatHazeShader.program);
		tr.heatHazeShader.program = 0;
	}

	if(tr.bloomShader.program)
	{
		qglDeleteObjectARB(tr.bloomShader.program);
		tr.bloomShader.program = 0;
	}

	if(tr.contrastShader.program)
	{
		qglDeleteObjectARB(tr.contrastShader.program);
		tr.contrastShader.program = 0;
	}

	if(tr.blurXShader.program)
	{
		qglDeleteObjectARB(tr.blurXShader.program);
		tr.blurXShader.program = 0;
	}

	if(tr.blurYShader.program)
	{
		qglDeleteObjectARB(tr.blurYShader.program);
		tr.blurYShader.program = 0;
	}

	if(tr.rotoscopeShader.program)
	{
		qglDeleteObjectARB(tr.rotoscopeShader.program);
		tr.rotoscopeShader.program = 0;
	}
	
	if(tr.screenShader.program)
	{
		qglDeleteObjectARB(tr.screenShader.program);
		tr.screenShader.program = 0;
	}
	
	glState.currentProgram = 0;
	qglUseProgramObjectARB(0);
}

/*
==================
DrawElements
==================
*/
static void DrawElements()
{
	// move tess data through the GPU, finally
	if(glConfig.vertexBufferObjectAvailable && tess.indexesVBO)
	{
		//qglDrawRangeElementsEXT(GL_TRIANGLES, 0, tessmesh->vertexes.size(), mesh->indexes.size(), GL_UNSIGNED_INT, VBO_BUFFER_OFFSET(mesh->vbo_indexes_ofs));

		qglDrawElements(GL_TRIANGLES, tess.numIndexes, GL_INDEX_TYPE, BUFFER_OFFSET(tess.ofsIndexes));
		backEnd.pc.c_indexes += tess.numIndexes;
	}
#if 0
	else if(tess.numLightIndexes)
	{
		qglDrawElements(GL_TRIANGLES, tess.numLightIndexes, GL_INDEX_TYPE, tess.lightIndexes);
		backEnd.pc.c_indexes += tess.numLightIndexes;
	}
#endif
	else
	{
		qglDrawElements(GL_TRIANGLES, tess.numIndexes, GL_INDEX_TYPE, tess.indexes);
		backEnd.pc.c_indexes += tess.numIndexes;
	}

	// update performance counters
	backEnd.pc.c_drawElements++;
	backEnd.pc.c_vertexes += tess.numVertexes;

	if(tess.indexesVBO)
	{
		backEnd.pc.c_vboIndexes += tess.numIndexes;
	}

	if(tess.vertexesVBO)
	{
		backEnd.pc.c_vboVertexes += tess.numVertexes;
	}
}


/*
=============================================================

SURFACE SHADERS

=============================================================
*/

shaderCommands_t tess;


/*
=================
BindAnimatedImage
=================
*/
static void BindAnimatedImage(textureBundle_t * bundle)
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
=================
BindLightMap
=================
*/
static void BindLightMap()
{
	image_t        *lightmap;

	if(tess.lightmapNum >= 0 && tess.lightmapNum < tr.numLightmaps)
	{
		lightmap = tr.lightmaps[tess.lightmapNum];
	}
	else
	{
		lightmap = NULL;
	}

	if(!tr.numLightmaps || !lightmap)
	{
		GL_Bind(tr.whiteImage);
		return;
	}

	GL_Bind(lightmap);
}

/*
=================
BindDeluxeMap
=================
*/
static void BindDeluxeMap()
{
	image_t        *deluxemap;

	if(tess.lightmapNum >= 0 && tess.lightmapNum < tr.numLightmaps)
	{
		deluxemap = tr.lightmaps[tess.lightmapNum + 1];
	}
	else
	{
		deluxemap = NULL;
	}

	if(!tr.numLightmaps || !deluxemap)
	{
		GL_Bind(tr.flatImage);
		return;
	}

	GL_Bind(deluxemap);
}

/*
================
DrawTris

Draws triangle outlines for debugging
================
*/
static void DrawTris()
{
	GLimp_LogComment("--- DrawTris ---\n");
	
	if(tess.indexesVBO && tess.vertexesVBO)
	{
		qglColor3f(0, 0, 1);
	}
	else if(tess.indexesVBO)
	{
		qglColor3f(0, 1, 0);
	}
	else if(tess.vertexesVBO)
	{
		qglColor3f(1, 0, 0);
	}
	else
	{
		qglColor3f(1, 1, 1);
	}

	GL_Program(0);
	GL_SelectTexture(0);
	GL_Bind(tr.whiteImage);

	GL_State(GLS_POLYMODE_LINE | GLS_DEPTHMASK_TRUE);
	GL_ClientState(GLCS_VERTEX);
	qglDepthRange(0, 0);

	if(glConfig.vertexBufferObjectAvailable && tess.vertexesVBO)
	{
		qglVertexPointer(4, GL_FLOAT, 0, BUFFER_OFFSET(tess.ofsXYZ));
	}
	else
	{
		qglVertexPointer(4, GL_FLOAT, 0, tess.xyz);
	}

	if(qglLockArraysEXT)
	{
		qglLockArraysEXT(0, tess.numVertexes);
		GLimp_LogComment("glLockArraysEXT\n");
	}

	DrawElements();

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
static void DrawTangentSpaces()
{
	int             i;
	vec3_t          temp;
	
	GLimp_LogComment("--- DrawTangentSpaces ---\n");

	GL_Program(0);
	GL_SelectTexture(0);
	GL_Bind(tr.whiteImage);
	qglDepthRange(0, 0);		// never occluded
	GL_State(GLS_POLYMODE_LINE | GLS_DEPTHMASK_TRUE);

	qglBegin(GL_LINES);
	for(i = 0; i < tess.numVertexes; i++)
	{
		qglColor3f(1, 0, 0);
		qglVertex3fv(tess.xyz[i]);
		VectorMA(tess.xyz[i], 2, tess.tangents[i], temp);
		qglVertex3fv(temp);

		qglColor3f(0, 1, 0);
		qglVertex3fv(tess.xyz[i]);
		VectorMA(tess.xyz[i], 2, tess.binormals[i], temp);
		qglVertex3fv(temp);

		qglColor3f(0, 0, 1);
		qglVertex3fv(tess.xyz[i]);
		VectorMA(tess.xyz[i], 2, tess.normals[i], temp);
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
static void DrawNormals()
{
	int             i;
	vec3_t          temp;
	
	GLimp_LogComment("--- DrawNormals ---\n");

	GL_Program(0);
	GL_SelectTexture(0);
	GL_Bind(tr.whiteImage);
	qglColor3f(1, 1, 1);
	qglDepthRange(0, 0);		// never occluded
	GL_State(GLS_POLYMODE_LINE | GLS_DEPTHMASK_TRUE);

	qglBegin(GL_LINES);
	for(i = 0; i < tess.numVertexes; i++)
	{
		qglVertex3fv(tess.xyz[i]);
		VectorMA(tess.xyz[i], 2, tess.normals[i], temp);
		qglVertex3fv(temp);
	}
	qglEnd();

	qglDepthRange(0, 1);
}

/*
==============
Tess_Begin

We must set some things up before beginning any tesselation,
because a surface may be forced to perform a Tess_End due
to overflow.
==============
*/
// *INDENT-OFF*
void Tess_Begin(	 void (*stageIteratorFunc)(),
					 shader_t * surfaceShader, shader_t * lightShader,
					 int lightmapNum,
					 int fogNum,
					 qboolean skipTangentSpaces,
					 qboolean shadowVolume)
{
	shader_t       *state = (surfaceShader->remappedShader) ? surfaceShader->remappedShader : surfaceShader;
		
	tess.indexesVBO = 0;
	tess.vertexesVBO = 0;

	tess.numIndexes = 0;
	tess.numVertexes = 0;
	tess.surfaceShader = state;
	
	tess.surfaceStages = state->stages;
	tess.numSurfaceStages = state->numStages;
	
	tess.lightShader = lightShader;
	
	tess.stageIteratorFunc = stageIteratorFunc;
	if(!tess.stageIteratorFunc)
	{
		tess.stageIteratorFunc = Tess_StageIteratorGeneric;
	}
	if(tess.stageIteratorFunc == Tess_StageIteratorGeneric)
	{
		tess.stageIteratorFunc = state->isSky ? Tess_StageIteratorSky : Tess_StageIteratorGeneric;		
	}

	tess.shaderTime = backEnd.refdef.floatTime - tess.surfaceShader->timeOffset;
	if(tess.surfaceShader->clampTime && tess.shaderTime >= tess.surfaceShader->clampTime)
	{
		tess.shaderTime = tess.surfaceShader->clampTime;
	}
	
	tess.lightmapNum = lightmapNum;
	tess.fogNum = fogNum;
	
	tess.skipTangentSpaces = skipTangentSpaces;
	tess.shadowVolume = shadowVolume;
	
	if(r_logFile->integer)
	{
		// don't just call LogComment, or we will get
		// a call to va() every frame!
		GLimp_LogComment(va("--- Tess_Begin( %s, %s, %i, %i, %i, %i ) ---\n", tess.surfaceShader->name, tess.lightShader ? tess.lightShader->name : NULL, tess.lightmapNum, tess.fogNum, tess.skipTangentSpaces, tess.shadowVolume));
	}
}
// *INDENT-ON*

static void     ComputeColors(shaderStage_t * pStage);
static void     ComputeColor(shaderStage_t * pStage);

static void Render_genericSingle_FFP(int stage)
{
	shaderStage_t  *pStage;

	GLimp_LogComment("--- Render_genericSingle_FFP ---\n");

	pStage = tess.surfaceStages[stage];

	GL_Program(0);
	GL_State(pStage->stateBits);

	if(glConfig.vertexBufferObjectAvailable && tess.vertexesVBO)
	{
		ComputeColor(pStage);
		qglColor4fv(tess.svars.color);
		GL_ClientState(GLCS_VERTEX);
	}
	else
	{
		ComputeColors(pStage);
		GL_ClientState(GLCS_VERTEX | GLCS_COLOR);
	}

	GL_SetVertexAttribs();

	GL_SelectTexture(0);
//  qglEnable(GL_TEXTURE_2D);
	qglEnableClientState(GL_TEXTURE_COORD_ARRAY);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(tess.svars.texMatrices[TB_COLORMAP]);
	qglMatrixMode(GL_MODELVIEW);

	if(glConfig.vertexBufferObjectAvailable && tess.vertexesVBO)
	{
		qglTexCoordPointer(4, GL_FLOAT, 0, BUFFER_OFFSET(tess.ofsTexCoords));
	}
	else
	{
		qglTexCoordPointer(2, GL_FLOAT, 0, tess.svars.texCoords[TB_COLORMAP]);
	}

	BindAnimatedImage(&pStage->bundle[0]);

	DrawElements();

	qglDisableClientState(GL_TEXTURE_COORD_ARRAY);
//  qglDisable(GL_TEXTURE_2D);

	GL_CheckErrors();
}

#if 1
#define Render_genericSingle Render_genericSingle_FFP
#else
static void Render_genericSingle(int stage)
{
	shaderStage_t  *pStage;

	GLimp_LogComment("--- Render_genericSingle ---\n");

	pStage = tess.surfaceStages[stage];

	ComputeColors(pStage);

	GL_State(pStage->stateBits);

	// enable shader, set arrays
	if(glConfig.framebufferObjectAvailable && glState.currentFBO)
	{
		GL_Program(tr.genericSingleMRTShader.program);
		GL_ClientState(tr.genericSingleMRTShader.attribs);
		GL_SetVertexAttribs();
	}
	else
	{
		GL_Program(tr.genericSingleShader.program);
		GL_ClientState(tr.genericSingleShader.attribs);
		GL_SetVertexAttribs();
	}

	// bind u_ColorMap
	GL_SelectTexture(0);
	BindAnimatedImage(&pStage->bundle[TB_COLORMAP]);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(tess.svars.texMatrices[TB_COLORMAP]);
	qglMatrixMode(GL_MODELVIEW);

	DrawElements();
	
	GL_CheckErrors();
}
#endif

/*
static void Render_fboTest(int stage)
{
	shaderStage_t  *pStage;

	GLimp_LogComment("--- Render_fboTest ---\n");

	pStage = tess.surfaceStages[stage];

	if(glConfig.framebufferObjectAvailable && glConfig.textureNPOTAvailable)
	{
		static float	yawAngle = 0.0f;
		static float	pitchAngle = 0.0f;
		matrix_t		transformMatrix, modelViewMatrix;
		vec3_t          origin;
		
		R_BindFBO(tr.visibilityFBO);
		
		qglMatrixMode(GL_PROJECTION);
		qglLoadMatrixf(backEnd.viewParms.projectionMatrix);
		qglMatrixMode(GL_MODELVIEW);
		
		// set the window clipping
		qglViewport(0, 0, glConfig.vidWidth, glConfig.vidHeight);
		qglScissor(0, 0, glConfig.vidWidth, glConfig.vidHeight);
		
		qglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		
		// transform by the camera placement
		VectorCopy(backEnd.viewParms.or.origin, origin);
		VectorMA(origin, 32, backEnd.viewParms.or.axis[0], origin);
		
		MatrixSetupTransform(transformMatrix,
						 backEnd.viewParms.or.axis[0], backEnd.viewParms.or.axis[1], backEnd.viewParms.or.axis[2], origin);

		MatrixMultiply(backEnd.viewParms.world.viewMatrix, transformMatrix, modelViewMatrix);
		qglLoadMatrixf(modelViewMatrix);
		
		yawAngle += 0.3f;
		pitchAngle += 0.21f;
		qglRotatef(yawAngle, 0.0f, 1.0f, 0.0f);
		qglRotatef(pitchAngle, 1.0f, 0.0f, 0.0f);
		qglRotatef(55.0f, 1.0f, 0.0f, 1.0f);
		qglScalef(8.0f, 8.0f, 8.0f);
		
		GL_Program(0);
		//qglEnable(GL_DEPTH_TEST);
		GL_State(GLS_DEPTHTEST_DISABLE | GLS_DEPTHMASK_TRUE);
		GL_Cull(CT_TWO_SIDED);
		
		GL_SelectTexture(0);
		GL_Bind(tr.whiteImage);
		
		// draw cube
		qglBegin(GL_QUADS);
		
		qglColor3f(1.0f, 1.0f, 1.0f);
		qglVertex3f(-1.0f, -1.0f, -1.0f);
		qglVertex3f(-1.0f, 1.0f, -1.0f);
		qglVertex3f(-1.0f, 1.0f, 1.0f);
		qglVertex3f(-1.0f, -1.0f, 1.0f);

		qglColor3f(1.0f, 0.7f, 0.7f);
		qglVertex3f(1.0f, -1.0f, -1.0f);
		qglVertex3f(1.0f, 1.0f, -1.0f);
		qglVertex3f(1.0f, 1.0f, 1.0f);
		qglVertex3f(1.0f, -1.0f, 1.0f);

		qglColor3f(0.7f, 1.0f, 0.7f);
		qglVertex3f(-1.0f, -1.0f, 1.0f);
		qglVertex3f(-1.0f, 1.0f, 1.0f);
		qglVertex3f(1.0f, 1.0f, 1.0f);
		qglVertex3f(1.0f, -1.0f, 1.0f);

		qglColor3f(0.7f, 0.7f, 1.0f);
		qglVertex3f(-1.0f, -1.0f, -1.0f);
		qglVertex3f(-1.0f, 1.0f, -1.0f);
		qglVertex3f(1.0f, 1.0f, -1.0f);
		qglVertex3f(1.0f, -1.0f, -1.0f);

		qglColor3f(1.0f, 1.0f, 0.7f);
		qglVertex3f(-1.0f, -1.0f, -1.0f);
		qglVertex3f(-1.0f, -1.0f, 1.0f);
		qglVertex3f(1.0f, -1.0f, 1.0f);
		qglVertex3f(1.0f, -1.0f, -1.0f);

		qglColor3f(0.7f, 1.0f, 1.0f);
		qglVertex3f(-1.0f, 1.0f, -1.0f);
		qglVertex3f(-1.0f, 1.0f, 1.0f);
		qglVertex3f(1.0f, 1.0f, 1.0f);
		qglVertex3f(1.0f, 1.0f, -1.0f);

		qglEnd();
		
		qglLoadMatrixf(backEnd.or.modelViewMatrix);

		R_BindNullFBO();
		
		ComputeColors(pStage);
		GL_State(pStage->stateBits);

		// enable shader, set arrays

		// FIXME: VBO surfaces have only tess.svars.color
		GL_Program(tr.genericShader_single.program);
		GL_ClientState(tr.genericShader_single.attribs);
		GL_SetVertexAttribs();
		
		// bind FBO image
		GL_SelectTexture(0);
		GL_Bind(tr.visibilityFBOImage);
		qglMatrixMode(GL_TEXTURE);
		qglLoadMatrixf(tess.svars.texMatrices[TB_COLORMAP]);
		qglMatrixMode(GL_MODELVIEW);

		DrawElements();
	}
}
*/

static void Render_depthFill_FFP(int stage)
{
	shaderStage_t  *pStage;

	GLimp_LogComment("--- Render_depthFill_FFP ---\n");

	pStage = tess.surfaceStages[stage];

#if 1
	qglColor3f(0, 0, 0);
#else
	qglColor3f(1, 1, 1);
#endif

	GL_Program(0);
	GL_State(pStage->stateBits);
	GL_ClientState(GLCS_VERTEX);
	GL_SetVertexAttribs();

	GL_SelectTexture(0);

	if(pStage->stateBits & GLS_ATEST_BITS)
	{
		//qglEnable(GL_TEXTURE_2D);
		qglEnableClientState(GL_TEXTURE_COORD_ARRAY);
		qglMatrixMode(GL_TEXTURE);
		qglLoadMatrixf(tess.svars.texMatrices[TB_DIFFUSEMAP]);
		qglMatrixMode(GL_MODELVIEW);

		if(glConfig.vertexBufferObjectAvailable && tess.vertexesVBO)
		{
			qglTexCoordPointer(4, GL_FLOAT, 0, BUFFER_OFFSET(tess.ofsTexCoords));
		}
		else
		{
			qglTexCoordPointer(2, GL_FLOAT, 0, tess.svars.texCoords[TB_DIFFUSEMAP]);
		}

		GL_Bind(pStage->bundle[TB_DIFFUSEMAP].image[0]);
	}
	else
	{
		GL_Bind(tr.whiteImage);
	}

	DrawElements();

	if(pStage->stateBits & GLS_ATEST_BITS)
	{
		qglDisableClientState(GL_TEXTURE_COORD_ARRAY);
		//qglDisable(GL_TEXTURE_2D);
	}
	
	GL_CheckErrors();
}

#if 1
#define Render_depthFill Render_depthFill_FFP
#else
static void Render_depthFill(int stage)
{
	shaderStage_t  *pStage;

	GLimp_LogComment("--- Render_depthFill ---\n");

	pStage = tess.surfaceStages[stage];

	GL_State(pStage->stateBits);

	// enable shader, set arrays
	GL_Program(tr.depthFillShader.program);
	GL_ClientState(tr.depthFillShader.attribs);
	GL_SetVertexAttribs();

	// bind u_ColorMap
	GL_SelectTexture(0);

	if(pStage->stateBits & GLS_ATEST_BITS)
	{
		qglMatrixMode(GL_TEXTURE);
		qglLoadMatrixf(tess.svars.texMatrices[TB_COLORMAP]);
		qglMatrixMode(GL_MODELVIEW);

		GL_Bind(pStage->bundle[TB_COLORMAP].image[0]);
	}
	else
	{
		GL_Bind(tr.whiteImage);
	}

	DrawElements();
	
	GL_CheckErrors();
}
#endif

static void Render_shadowFill(int stage)
{
	shaderStage_t  *pStage;
	vec3_t			lightOrigin;
	unsigned        stateBits;

	GLimp_LogComment("--- Render_shadowFill ---\n");

	pStage = tess.surfaceStages[stage];

	// remove blend mode
	stateBits = pStage->stateBits;
	stateBits &= ~(GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS);
		
	GL_State(stateBits);

	// enable shader, set arrays
	GL_Program(tr.shadowFillShader.program);
	GL_ClientState(tr.shadowFillShader.attribs);
	GL_SetVertexAttribs();
	
	// set uniforms
	VectorCopy(backEnd.currentLight->origin, lightOrigin);	// in world space
	
	qglUniform3fARB(tr.shadowFillShader.u_LightOrigin, lightOrigin[0], lightOrigin[1], lightOrigin[2]);
	qglUniform1fARB(tr.shadowFillShader.u_LightRadius, backEnd.currentLight->sphereRadius);
	qglUniformMatrix4fvARB(tr.shadowFillShader.u_ModelMatrix, 1, GL_FALSE, backEnd.or.transformMatrix);

	// bind u_ColorMap
	GL_SelectTexture(0);

	if(stateBits & GLS_ATEST_BITS)
	{
		GL_Bind(pStage->bundle[TB_COLORMAP].image[0]);
		qglMatrixMode(GL_TEXTURE);
		qglLoadMatrixf(tess.svars.texMatrices[TB_COLORMAP]);
		qglMatrixMode(GL_MODELVIEW);
	}
	else
	{
		GL_Bind(tr.whiteImage);
	}

	DrawElements();
	
	GL_CheckErrors();
}

/*
===================
DrawMultiTextured

output = t0 * t1 or t0 + t1

t0 = most upstream according to spec
t1 = most downstream according to spec
===================
*/
static void Render_genericMulti_FFP(int stage)
{
	shaderStage_t  *pStage;

	GLimp_LogComment("--- Render_genericMulti_FFP ---\n");

	pStage = tess.surfaceStages[stage];

	ComputeColors(pStage);

	GL_Program(0);
	GL_State(pStage->stateBits);
	GL_ClientState(GLCS_VERTEX | GLCS_COLOR);
	GL_SetVertexAttribs();

	// base
	GL_SelectTexture(0);
//  qglEnable(GL_TEXTURE_2D);
	qglEnableClientState(GL_TEXTURE_COORD_ARRAY);
	qglTexCoordPointer(2, GL_FLOAT, 0, tess.svars.texCoords[TB_DIFFUSEMAP]);
	BindAnimatedImage(&pStage->bundle[0]);

	// lightmap/secondary pass
	GL_SelectTexture(1);
	qglEnable(GL_TEXTURE_2D);
	qglEnableClientState(GL_TEXTURE_COORD_ARRAY);
	qglTexCoordPointer(2, GL_FLOAT, 0, tess.svars.texCoords[1]);

	if(r_showLightMaps->integer)
	{
		GL_TexEnv(GL_REPLACE);
	}
	else
	{
		GL_TexEnv(tess.surfaceShader->collapseTextureEnv);
	}

	BindAnimatedImage(&pStage->bundle[1]);

	DrawElements();

	// clean up
	qglDisableClientState(GL_TEXTURE_COORD_ARRAY);
	qglDisable(GL_TEXTURE_2D);

	GL_SelectTexture(0);
	qglDisableClientState(GL_TEXTURE_COORD_ARRAY);
//  qglDisable(GL_TEXTURE_2D);

	GL_CheckErrors();
}

static void Render_lighting_D_direct(int stage)
{
	shaderStage_t  *pStage;

	vec4_t          ambientLight;
	vec3_t          lightDir;
	vec4_t          directedLight;
	trRefEntity_t  *ent = backEnd.currentEntity;

	GLimp_LogComment("--- Render_lighting_D_direct ---\n");

	pStage = tess.surfaceStages[stage];

	GL_State(pStage->stateBits);

	// enable shader, set arrays
	GL_Program(tr.lightShader_D_direct.program);
	GL_ClientState(tr.lightShader_D_direct.attribs);
	GL_SetVertexAttribs();

	// set uniforms
	VectorScale(ent->ambientLight, (1.0 / 255.0) * 0.3, ambientLight);
	ClampColor(ambientLight);
	VectorScale(ent->directedLight, 1.0 / 255.0, directedLight);
	ClampColor(directedLight);
	VectorCopy(ent->lightDir, lightDir);

	qglUniform3fARB(tr.lightShader_D_direct.u_AmbientColor, ambientLight[0], ambientLight[1], ambientLight[2]);
	qglUniform3fARB(tr.lightShader_D_direct.u_LightDir, lightDir[0], lightDir[1], lightDir[2]);
	qglUniform3fARB(tr.lightShader_D_direct.u_LightColor, directedLight[0], directedLight[1], directedLight[2]);

	// bind u_DiffuseMap
	GL_SelectTexture(0);
	GL_Bind(pStage->bundle[TB_DIFFUSEMAP].image[0]);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(tess.svars.texMatrices[TB_DIFFUSEMAP]);
	qglMatrixMode(GL_MODELVIEW);

	DrawElements();

	GL_CheckErrors();
}


static void Render_lighting_DB_direct(int stage)
{
	shaderStage_t  *pStage;

	vec4_t          ambientLight;
	vec3_t          lightDir;
	vec4_t          directedLight;
	trRefEntity_t  *ent = backEnd.currentEntity;

	GLimp_LogComment("--- Render_lighting_DB_direct ---\n");

	pStage = tess.surfaceStages[stage];

	GL_State(pStage->stateBits);

	// enable shader, set arrays
	GL_Program(tr.lightShader_DB_direct.program);
	GL_ClientState(tr.lightShader_DB_direct.attribs);
	GL_SetVertexAttribs();

	// set uniforms
	VectorScale(ent->ambientLight, (1.0 / 255.0) * 0.3, ambientLight);
	ClampColor(ambientLight);
	VectorScale(ent->directedLight, 1.0 / 255.0, directedLight);
	ClampColor(directedLight);
	VectorCopy(ent->lightDir, lightDir);

	qglUniform3fARB(tr.lightShader_DB_direct.u_AmbientColor, ambientLight[0], ambientLight[1], ambientLight[2]);
	qglUniform3fARB(tr.lightShader_DB_direct.u_LightDir, lightDir[0], lightDir[1], lightDir[2]);
	qglUniform3fARB(tr.lightShader_DB_direct.u_LightColor, directedLight[0], directedLight[1], directedLight[2]);


	// bind u_DiffuseMap
	GL_SelectTexture(0);
	GL_Bind(pStage->bundle[TB_DIFFUSEMAP].image[0]);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(tess.svars.texMatrices[TB_DIFFUSEMAP]);
	qglMatrixMode(GL_MODELVIEW);

	// bind u_NormalMap
	GL_SelectTexture(1);
	GL_Bind(pStage->bundle[TB_NORMALMAP].image[0]);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(tess.svars.texMatrices[TB_NORMALMAP]);
	qglMatrixMode(GL_MODELVIEW);

	DrawElements();

	GL_CheckErrors();
}

static void Render_lighting_DBS_direct(int stage)
{
	shaderStage_t  *pStage;

	vec3_t          viewOrigin;
	vec4_t          ambientLight;
	vec3_t          lightDir;
	vec4_t          directedLight;
	float           specularExponent;
	trRefEntity_t  *ent = backEnd.currentEntity;

	GLimp_LogComment("--- Render_lighting_DBS_direct ---\n");

	pStage = tess.surfaceStages[stage];

	GL_State(pStage->stateBits);

	// enable shader, set arrays
	GL_Program(tr.lightShader_DBS_direct.program);
	GL_ClientState(tr.lightShader_DBS_direct.attribs);
	GL_SetVertexAttribs();

	// set uniforms
	VectorCopy(backEnd.or.viewOrigin, viewOrigin);
	VectorScale(ent->ambientLight, (1.0 / 255.0) * 0.3, ambientLight);
	ClampColor(ambientLight);
	VectorScale(ent->directedLight, 1.0 / 255.0, directedLight);
	ClampColor(directedLight);
	VectorCopy(ent->lightDir, lightDir);
	specularExponent = RB_EvalExpression(&pStage->specularExponentExp, r_specularExponent->value);

	qglUniform3fARB(tr.lightShader_DBS_direct.u_AmbientColor, ambientLight[0], ambientLight[1], ambientLight[2]);
	qglUniform3fARB(tr.lightShader_DBS_direct.u_ViewOrigin, viewOrigin[0], viewOrigin[1], viewOrigin[2]);
	qglUniform3fARB(tr.lightShader_DBS_direct.u_LightDir, lightDir[0], lightDir[1], lightDir[2]);
	qglUniform3fARB(tr.lightShader_DBS_direct.u_LightColor, directedLight[0], directedLight[1], directedLight[2]);
	qglUniform1fARB(tr.lightShader_DBS_direct.u_SpecularExponent, specularExponent);

	// bind u_DiffuseMap
	GL_SelectTexture(0);
	GL_Bind(pStage->bundle[TB_DIFFUSEMAP].image[0]);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(tess.svars.texMatrices[TB_DIFFUSEMAP]);
	qglMatrixMode(GL_MODELVIEW);

	// bind u_NormalMap
	GL_SelectTexture(1);
	GL_Bind(pStage->bundle[TB_NORMALMAP].image[0]);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(tess.svars.texMatrices[TB_NORMALMAP]);
	qglMatrixMode(GL_MODELVIEW);

	// bind u_SpecularMap
	GL_SelectTexture(2);
	GL_Bind(pStage->bundle[TB_SPECULARMAP].image[0]);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(tess.svars.texMatrices[TB_SPECULARMAP]);
	qglMatrixMode(GL_MODELVIEW);

	DrawElements();

	GL_CheckErrors();
}

static void Render_lighting_D_omni(shaderStage_t * diffuseStage,
								   shaderStage_t * attenuationXYStage, shaderStage_t * attenuationZStage, trRefLight_t * light)
{
	vec3_t          lightOrigin;
	vec4_t          lightColor;

	GLimp_LogComment("--- Render_lighting_D_omni ---\n");

	// enable shader, set arrays
	GL_Program(tr.lightShader_D_omni.program);
	GL_ClientState(tr.lightShader_D_omni.attribs);
	GL_SetVertexAttribs();

	// set uniforms
	VectorCopy(light->transformed, lightOrigin);
	VectorCopy(tess.svars.color, lightColor);

	qglUniform3fARB(tr.lightShader_D_omni.u_LightOrigin, lightOrigin[0], lightOrigin[1], lightOrigin[2]);
	qglUniform3fARB(tr.lightShader_D_omni.u_LightColor, lightColor[0], lightColor[1], lightColor[2]);
	qglUniform1fARB(tr.lightShader_D_omni.u_LightScale, r_lightScale->value);

	// bind u_DiffuseMap
	GL_SelectTexture(0);
	GL_Bind(diffuseStage->bundle[TB_DIFFUSEMAP].image[0]);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(tess.svars.texMatrices[TB_DIFFUSEMAP]);
	qglMatrixMode(GL_MODELVIEW);

	// bind u_AttenuationMapXY
	GL_SelectTexture(1);
	BindAnimatedImage(&attenuationXYStage->bundle[TB_COLORMAP]);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(light->attenuationMatrix2);
	qglMatrixMode(GL_MODELVIEW);

	// bind u_AttenuationMapZ
	GL_SelectTexture(2);
	BindAnimatedImage(&attenuationZStage->bundle[TB_COLORMAP]);
	qglMatrixMode(GL_TEXTURE);
	qglLoadIdentity();
	qglMatrixMode(GL_MODELVIEW);

	DrawElements();

	// update performance counters
	backEnd.pc.c_dlightVertexes += tess.numVertexes;
	backEnd.pc.c_dlightIndexes += tess.numIndexes;

	GL_CheckErrors();
}

static void Render_lighting_DB_omni(shaderStage_t * diffuseStage,
									shaderStage_t * attenuationXYStage, shaderStage_t * attenuationZStage, trRefLight_t * light)
{
	vec3_t          lightOrigin;
	vec4_t          lightColor;

	GLimp_LogComment("--- Render_lighting_DB_omni ---\n");

	// enable shader, set arrays
	GL_Program(tr.lightShader_DB_omni.program);
	GL_ClientState(tr.lightShader_DB_omni.attribs);
	GL_SetVertexAttribs();

	// set uniforms
	VectorCopy(light->transformed, lightOrigin);
	VectorCopy(tess.svars.color, lightColor);

	qglUniform3fARB(tr.lightShader_DB_omni.u_LightOrigin, lightOrigin[0], lightOrigin[1], lightOrigin[2]);
	qglUniform3fARB(tr.lightShader_DB_omni.u_LightColor, lightColor[0], lightColor[1], lightColor[2]);
	qglUniform1fARB(tr.lightShader_DB_omni.u_LightScale, r_lightScale->value);

	// bind u_DiffuseMap
	GL_SelectTexture(0);
	GL_Bind(diffuseStage->bundle[TB_DIFFUSEMAP].image[0]);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(tess.svars.texMatrices[TB_DIFFUSEMAP]);
	qglMatrixMode(GL_MODELVIEW);

	// bind u_NormalMap
	GL_SelectTexture(1);
	GL_Bind(diffuseStage->bundle[TB_NORMALMAP].image[0]);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(tess.svars.texMatrices[TB_NORMALMAP]);
	qglMatrixMode(GL_MODELVIEW);

	// bind u_AttenuationMapXY
	GL_SelectTexture(2);
	BindAnimatedImage(&attenuationXYStage->bundle[TB_COLORMAP]);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(light->attenuationMatrix2);
	qglMatrixMode(GL_MODELVIEW);

	// bind u_AttenuationMapZ
	GL_SelectTexture(3);
	BindAnimatedImage(&attenuationZStage->bundle[TB_COLORMAP]);

	DrawElements();

	// update performance counters
	backEnd.pc.c_dlightVertexes += tess.numVertexes;
	backEnd.pc.c_dlightIndexes += tess.numIndexes;

	GL_CheckErrors();
}

static void Render_lighting_DBS_omni(shaderStage_t * diffuseStage,
									 shaderStage_t * attenuationXYStage,
									 shaderStage_t * attenuationZStage, trRefLight_t * light)
{
	vec3_t          viewOrigin;
	vec3_t          lightOrigin;
	vec4_t          lightColor;
	float           specularExponent;

	GLimp_LogComment("--- Render_lighting_DBS_omni ---\n");

	// enable shader, set arrays
	GL_Program(tr.lightShader_DBS_omni.program);
	GL_ClientState(tr.lightShader_DBS_omni.attribs);
	GL_SetVertexAttribs();

	// set uniforms
	VectorCopy(backEnd.or.viewOrigin, viewOrigin);
	VectorCopy(light->transformed, lightOrigin);
	VectorCopy(tess.svars.color, lightColor);
	specularExponent = RB_EvalExpression(&diffuseStage->specularExponentExp, r_specularExponent->value);

	qglUniform3fARB(tr.lightShader_DBS_omni.u_ViewOrigin, viewOrigin[0], viewOrigin[1], viewOrigin[2]);
	qglUniform3fARB(tr.lightShader_DBS_omni.u_LightOrigin, lightOrigin[0], lightOrigin[1], lightOrigin[2]);
	qglUniform3fARB(tr.lightShader_DBS_omni.u_LightColor, lightColor[0], lightColor[1], lightColor[2]);
	qglUniform1fARB(tr.lightShader_DBS_omni.u_LightScale, r_lightScale->value);
	qglUniform1fARB(tr.lightShader_DBS_omni.u_SpecularExponent, specularExponent);

	// bind u_DiffuseMap
	GL_SelectTexture(0);
	GL_Bind(diffuseStage->bundle[TB_DIFFUSEMAP].image[0]);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(tess.svars.texMatrices[TB_DIFFUSEMAP]);
	qglMatrixMode(GL_MODELVIEW);

	// bind u_NormalMap
	GL_SelectTexture(1);
	GL_Bind(diffuseStage->bundle[TB_NORMALMAP].image[0]);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(tess.svars.texMatrices[TB_NORMALMAP]);
	qglMatrixMode(GL_MODELVIEW);

	// bind u_SpecularMap
	GL_SelectTexture(2);
	GL_Bind(diffuseStage->bundle[TB_SPECULARMAP].image[0]);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(tess.svars.texMatrices[TB_SPECULARMAP]);
	qglMatrixMode(GL_MODELVIEW);

	// bind u_AttenuationMapXY
	GL_SelectTexture(3);
	BindAnimatedImage(&attenuationXYStage->bundle[TB_COLORMAP]);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(light->attenuationMatrix2);
	qglMatrixMode(GL_MODELVIEW);

	// bind u_AttenuationMapZ
	GL_SelectTexture(4);
	BindAnimatedImage(&attenuationZStage->bundle[TB_COLORMAP]);

	DrawElements();

	// update performance counters
	backEnd.pc.c_dlightVertexes += tess.numVertexes;
	backEnd.pc.c_dlightIndexes += tess.numIndexes;

	GL_CheckErrors();
}

static void Render_lighting_DBSP_omni(shaderStage_t * diffuseStage,
									  shaderStage_t * attenuationXYStage,
									  shaderStage_t * attenuationZStage, trRefLight_t * light)
{
	vec3_t          viewOrigin;
	vec3_t          lightOrigin;
	vec4_t          lightColor;
	float           specularExponent;

	GLimp_LogComment("--- Render_lighting_DBSP_omni ---\n");

	// enable shader, set arrays
	GL_Program(tr.lightShader_DBSP_omni.program);
	GL_ClientState(tr.lightShader_DBSP_omni.attribs);
	GL_SetVertexAttribs();

	// set uniforms
	VectorCopy(backEnd.or.viewOrigin, viewOrigin);
	VectorCopy(light->transformed, lightOrigin);
	VectorCopy(tess.svars.color, lightColor);
	specularExponent = RB_EvalExpression(&diffuseStage->specularExponentExp, r_specularExponent->value);

	qglUniform3fARB(tr.lightShader_DBSP_omni.u_ViewOrigin, viewOrigin[0], viewOrigin[1], viewOrigin[2]);
	qglUniform3fARB(tr.lightShader_DBSP_omni.u_LightOrigin, lightOrigin[0], lightOrigin[1], lightOrigin[2]);
	qglUniform3fARB(tr.lightShader_DBSP_omni.u_LightColor, lightColor[0], lightColor[1], lightColor[2]);
	qglUniform1fARB(tr.lightShader_DBSP_omni.u_LightScale, r_lightScale->value);
	qglUniform1fARB(tr.lightShader_DBSP_omni.u_SpecularExponent, specularExponent);
	qglUniform1fARB(tr.lightShader_DBSP_omni.u_HeightScale, RB_EvalExpression(&diffuseStage->heightScaleExp, 0.05f));
	qglUniform1fARB(tr.lightShader_DBSP_omni.u_HeightBias, RB_EvalExpression(&diffuseStage->heightBiasExp, 0.0f));

	// bind u_DiffuseMap
	GL_SelectTexture(0);
	GL_Bind(diffuseStage->bundle[TB_DIFFUSEMAP].image[0]);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(tess.svars.texMatrices[TB_DIFFUSEMAP]);
	qglMatrixMode(GL_MODELVIEW);

	// bind u_NormalMap
	GL_SelectTexture(1);
	GL_Bind(diffuseStage->bundle[TB_NORMALMAP].image[0]);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(tess.svars.texMatrices[TB_NORMALMAP]);
	qglMatrixMode(GL_MODELVIEW);

	// bind u_SpecularMap
	GL_SelectTexture(2);
	GL_Bind(diffuseStage->bundle[TB_SPECULARMAP].image[0]);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(tess.svars.texMatrices[TB_SPECULARMAP]);
	qglMatrixMode(GL_MODELVIEW);

	// bind u_AttenuationMapXY
	GL_SelectTexture(3);
	BindAnimatedImage(&attenuationXYStage->bundle[TB_COLORMAP]);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(light->attenuationMatrix2);
	qglMatrixMode(GL_MODELVIEW);

	// bind u_AttenuationMapZ
	GL_SelectTexture(4);
	BindAnimatedImage(&attenuationZStage->bundle[TB_COLORMAP]);

	DrawElements();

	// update performance counters
	backEnd.pc.c_dlightVertexes += tess.numVertexes;
	backEnd.pc.c_dlightIndexes += tess.numIndexes;

	GL_CheckErrors();
}

static void Render_lighting_D_proj(shaderStage_t * diffuseStage,
								   shaderStage_t * attenuationXYStage, shaderStage_t * attenuationZStage, trRefLight_t * light)
{
	vec3_t          lightOrigin;
	vec4_t          lightColor;

	GLimp_LogComment("--- Render_lighting_D_proj ---\n");

	// enable shader, set arrays
	GL_Program(tr.lightShader_D_proj.program);
	GL_ClientState(tr.lightShader_D_proj.attribs);
	GL_SetVertexAttribs();

	// set uniforms
	VectorCopy(light->origin, lightOrigin);
	VectorCopy(tess.svars.color, lightColor);

	qglUniform3fARB(tr.lightShader_D_proj.u_LightOrigin, lightOrigin[0], lightOrigin[1], lightOrigin[2]);
	qglUniform3fARB(tr.lightShader_D_proj.u_LightColor, lightColor[0], lightColor[1], lightColor[2]);
	qglUniform1fARB(tr.lightShader_D_proj.u_LightRadius, light->sphereRadius);
	qglUniform1fARB(tr.lightShader_D_proj.u_LightScale, r_lightScale->value);
	qglUniformMatrix4fvARB(tr.lightShader_D_proj.u_ModelMatrix, 1, GL_FALSE, backEnd.or.transformMatrix);

	// bind u_DiffuseMap
	GL_SelectTexture(0);
	GL_Bind(diffuseStage->bundle[TB_DIFFUSEMAP].image[0]);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(tess.svars.texMatrices[TB_DIFFUSEMAP]);
	qglMatrixMode(GL_MODELVIEW);

	// bind u_AttenuationMapXY
	GL_SelectTexture(1);
	BindAnimatedImage(&attenuationXYStage->bundle[TB_COLORMAP]);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(light->attenuationMatrix2);
	qglMatrixMode(GL_MODELVIEW);

	// bind u_AttenuationMapZ
	GL_SelectTexture(2);
	BindAnimatedImage(&attenuationZStage->bundle[TB_COLORMAP]);
	
	// bind u_ShadowMap
	GL_SelectTexture(3);
	GL_Bind(tr.shadowMapFBOImage);

	DrawElements();

	// update performance counters
	backEnd.pc.c_dlightVertexes += tess.numVertexes;
	backEnd.pc.c_dlightIndexes += tess.numIndexes;

	GL_CheckErrors();
}

static void Render_lightmap_FFP(int stage, int texCoordsIndex)
{
	shaderStage_t  *pStage;

	GLimp_LogComment("--- Render_lightmap_FFP ---\n");

	pStage = tess.surfaceStages[stage];

	GL_Program(0);

	qglColor4fv(colorWhite);

	if(r_showLightMaps->integer)
	{
		GL_State(pStage->stateBits & ~(GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS));
	}
	else
	{
		GL_State(pStage->stateBits);
	}

	GL_ClientState(GLCS_VERTEX);
	GL_SetVertexAttribs();

	GL_SelectTexture(0);
//  qglEnable(GL_TEXTURE_2D);
	qglEnableClientState(GL_TEXTURE_COORD_ARRAY);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(tess.svars.texMatrices[texCoordsIndex]);
	qglMatrixMode(GL_MODELVIEW);

	if(glConfig.vertexBufferObjectAvailable && tess.vertexesVBO)
	{
		if(texCoordsIndex == TB_LIGHTMAP)
			qglTexCoordPointer(4, GL_FLOAT, 0, BUFFER_OFFSET(tess.ofsTexCoords2));
		else
			qglTexCoordPointer(4, GL_FLOAT, 0, BUFFER_OFFSET(tess.ofsTexCoords));
	}
	else
	{
		qglTexCoordPointer(2, GL_FLOAT, 0, tess.svars.texCoords[texCoordsIndex]);
	}

	BindLightMap();

	DrawElements();

	qglDisableClientState(GL_TEXTURE_COORD_ARRAY);
//  qglDisable(GL_TEXTURE_2D);

	GL_CheckErrors();
}

static void Render_deluxemap_FFP(int stage, int texCoordsIndex)
{
	shaderStage_t  *pStage;

	GLimp_LogComment("--- Render_deluxemap_FFP ---\n");

	pStage = tess.surfaceStages[stage];

	if(!pStage->bundle[TB_LIGHTMAP].image[1])
		return;

	GL_Program(0);

	qglColor4fv(colorWhite);

	if(r_showLightMaps->integer)
	{
		GL_State(pStage->stateBits & ~(GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS));
	}
	else
	{
		GL_State(pStage->stateBits);
	}

	GL_ClientState(GLCS_VERTEX);
	GL_SetVertexAttribs();

	GL_SelectTexture(0);
//  qglEnable(GL_TEXTURE_2D);
	qglEnableClientState(GL_TEXTURE_COORD_ARRAY);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(tess.svars.texMatrices[texCoordsIndex]);
	qglMatrixMode(GL_MODELVIEW);

	if(glConfig.vertexBufferObjectAvailable && tess.vertexesVBO)
	{
		if(texCoordsIndex == TB_LIGHTMAP)
			qglTexCoordPointer(4, GL_FLOAT, 0, BUFFER_OFFSET(tess.ofsTexCoords2));
		else
			qglTexCoordPointer(4, GL_FLOAT, 0, BUFFER_OFFSET(tess.ofsTexCoords));
	}
	else
	{
		qglTexCoordPointer(2, GL_FLOAT, 0, tess.svars.texCoords[texCoordsIndex]);
	}

	BindDeluxeMap();

	DrawElements();

	qglDisableClientState(GL_TEXTURE_COORD_ARRAY);
//  qglDisable(GL_TEXTURE_2D);

	GL_CheckErrors();
}

static void Render_lighting_D_radiosity_FFP(int stage)
{
	shaderStage_t  *pStage;

	GLimp_LogComment("--- Render_lighting_D_radiosity_FFP ---\n");

	pStage = tess.surfaceStages[stage];

	ComputeColors(pStage);

	GL_Program(0);
	GL_SelectTexture(0);
	GL_State(pStage->stateBits);
	GL_ClientState(GLCS_VERTEX | GLCS_COLOR);
	GL_SetVertexAttribs();

	// base
	GL_SelectTexture(0);
//  qglEnable(GL_TEXTURE_2D);
	BindAnimatedImage(&pStage->bundle[TB_DIFFUSEMAP]);
	qglEnableClientState(GL_TEXTURE_COORD_ARRAY);
	qglTexCoordPointer(2, GL_FLOAT, 0, tess.svars.texCoords[TB_DIFFUSEMAP]);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(tess.svars.texMatrices[TB_DIFFUSEMAP]);
	qglMatrixMode(GL_MODELVIEW);

	// lightmap/secondary pass
	GL_SelectTexture(1);
	qglEnable(GL_TEXTURE_2D);
	qglEnableClientState(GL_TEXTURE_COORD_ARRAY);
	qglTexCoordPointer(2, GL_FLOAT, 0, tess.svars.texCoords[TB_LIGHTMAP]);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(tess.svars.texMatrices[TB_LIGHTMAP]);
	qglMatrixMode(GL_MODELVIEW);
	BindLightMap();

	DrawElements();

	// clean up
	qglDisableClientState(GL_TEXTURE_COORD_ARRAY);
	qglDisable(GL_TEXTURE_2D);

	GL_SelectTexture(0);
	qglDisableClientState(GL_TEXTURE_COORD_ARRAY);
//  qglDisable(GL_TEXTURE_2D);

	GL_CheckErrors();
}

static void Render_lighting_D_radiosity(int stage)
{
	shaderStage_t  *pStage;

	GLimp_LogComment("--- Render_lighting_D_radiosity ---\n");

	pStage = tess.surfaceStages[stage];

	GL_State(pStage->stateBits);

	// enable shader, set arrays
	GL_Program(tr.lightShader_D_radiosity.program);
	GL_ClientState(tr.lightShader_D_radiosity.attribs);
	GL_SetVertexAttribs();

	// bind u_DiffuseMap
	GL_SelectTexture(0);
	GL_Bind(pStage->bundle[TB_DIFFUSEMAP].image[0]);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(tess.svars.texMatrices[TB_DIFFUSEMAP]);
	qglMatrixMode(GL_MODELVIEW);

	// bind u_LightMap
	GL_SelectTexture(1);
	BindLightMap();
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(tess.svars.texMatrices[TB_LIGHTMAP]);
	qglMatrixMode(GL_MODELVIEW);

	DrawElements();

	GL_CheckErrors();
}

static void Render_lighting_DB_radiosity(int stage)
{
	shaderStage_t  *pStage;

	GLimp_LogComment("--- Render_lighting_DB_radiosity ---\n");

	pStage = tess.surfaceStages[stage];

	GL_State(pStage->stateBits);

	// enable shader, set arrays
	GL_Program(tr.lightShader_DB_radiosity.program);
	GL_ClientState(tr.lightShader_DB_radiosity.attribs);
	GL_SetVertexAttribs();

	// bind u_DiffuseMap
	GL_SelectTexture(0);
	GL_Bind(pStage->bundle[TB_DIFFUSEMAP].image[0]);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(tess.svars.texMatrices[TB_DIFFUSEMAP]);
	qglMatrixMode(GL_MODELVIEW);

	// bind u_NormalMap
	GL_SelectTexture(1);
	GL_Bind(pStage->bundle[TB_NORMALMAP].image[0]);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(tess.svars.texMatrices[TB_NORMALMAP]);
	qglMatrixMode(GL_MODELVIEW);

	// bind u_LightMap
	GL_SelectTexture(2);
	BindLightMap();
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(tess.svars.texMatrices[TB_LIGHTMAP]);
	qglMatrixMode(GL_MODELVIEW);

	// bind u_DeluxeMap
	GL_SelectTexture(3);
	BindDeluxeMap();

	DrawElements();

	GL_CheckErrors();
}

static void Render_lighting_DBS_radiosity(int stage)
{
	vec3_t          viewOrigin;
	float           specularExponent;
	shaderStage_t  *pStage;

	GLimp_LogComment("--- Render_lighting_DBS_radiosity ---\n");

	pStage = tess.surfaceStages[stage];

	GL_State(pStage->stateBits);

	// enable shader, set arrays
	GL_Program(tr.lightShader_DBS_radiosity.program);
	GL_ClientState(tr.lightShader_DBS_radiosity.attribs);
	GL_SetVertexAttribs();

	// set uniforms
	VectorCopy(backEnd.or.viewOrigin, viewOrigin);
	specularExponent = RB_EvalExpression(&pStage->specularExponentExp, r_specularExponent->value);

	qglUniform3fARB(tr.lightShader_DBS_radiosity.u_ViewOrigin, viewOrigin[0], viewOrigin[1], viewOrigin[2]);
	qglUniform1fARB(tr.lightShader_DBS_radiosity.u_SpecularExponent, specularExponent);

	// bind u_DiffuseMap
	GL_SelectTexture(0);
	GL_Bind(pStage->bundle[TB_DIFFUSEMAP].image[0]);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(tess.svars.texMatrices[TB_DIFFUSEMAP]);
	qglMatrixMode(GL_MODELVIEW);

	// bind u_NormalMap
	GL_SelectTexture(1);
	GL_Bind(pStage->bundle[TB_NORMALMAP].image[0]);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(tess.svars.texMatrices[TB_NORMALMAP]);
	qglMatrixMode(GL_MODELVIEW);

	// bind u_SpecularMap
	GL_SelectTexture(2);
	GL_Bind(pStage->bundle[TB_SPECULARMAP].image[0]);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(tess.svars.texMatrices[TB_SPECULARMAP]);
	qglMatrixMode(GL_MODELVIEW);

	// bind u_LightMap
	GL_SelectTexture(3);
	BindLightMap();
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(tess.svars.texMatrices[TB_LIGHTMAP]);
	qglMatrixMode(GL_MODELVIEW);

	// bind u_DeluxeMap
	GL_SelectTexture(4);
	BindDeluxeMap();

	DrawElements();

	GL_CheckErrors();
}

static void Render_reflection_C(int stage)
{
	vec3_t          viewOrigin;
	shaderStage_t  *pStage = tess.surfaceStages[stage];

	GLimp_LogComment("--- Render_reflection_C ---\n");

	GL_State(pStage->stateBits);

	// enable shader, set arrays
	GL_Program(tr.reflectionShader_C.program);
	GL_ClientState(tr.reflectionShader_C.attribs);
	GL_SetVertexAttribs();

	// set uniforms
	VectorCopy(backEnd.viewParms.or.origin, viewOrigin);	// in world space
	qglUniform3fARB(tr.reflectionShader_C.u_ViewOrigin, viewOrigin[0], viewOrigin[1], viewOrigin[2]);

	// bind u_ColorMap
	GL_SelectTexture(0);
	GL_Bind(pStage->bundle[TB_COLORMAP].image[0]);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(backEnd.or.transformMatrix);
	qglMatrixMode(GL_MODELVIEW);

	DrawElements();

	GL_CheckErrors();
}

static void Render_reflection_CB(int stage)
{
	vec3_t          viewOrigin;
	shaderStage_t  *pStage = tess.surfaceStages[stage];

	GLimp_LogComment("--- Render_reflection_CB ---\n");

	GL_State(pStage->stateBits);

	// enable shader, set arrays
	GL_Program(tr.reflectionShader_CB.program);
	GL_ClientState(tr.reflectionShader_CB.attribs);
	GL_SetVertexAttribs();

	// set uniforms
	VectorCopy(backEnd.viewParms.or.origin, viewOrigin);	// in world space
	qglUniform3fARB(tr.reflectionShader_CB.u_ViewOrigin, viewOrigin[0], viewOrigin[1], viewOrigin[2]);
	qglUniformMatrix4fvARB(tr.reflectionShader_CB.u_ModelMatrix, 1, GL_FALSE, backEnd.or.transformMatrix);

	// bind u_ColorMap
	GL_SelectTexture(0);
	GL_Bind(pStage->bundle[TB_COLORMAP].image[0]);
	/*
	   qglMatrixMode(GL_TEXTURE);
	   qglLoadMatrixf(backEnd.or.transformMatrix);
	   qglMatrixMode(GL_MODELVIEW);
	 */

	// bind u_NormalMap
	GL_SelectTexture(1);
	GL_Bind(pStage->bundle[TB_NORMALMAP].image[0]);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(tess.svars.texMatrices[TB_NORMALMAP]);
	qglMatrixMode(GL_MODELVIEW);

	DrawElements();

	GL_CheckErrors();
}

static void Render_refraction_C(int stage)
{
	vec3_t          viewOrigin;
	shaderStage_t  *pStage = tess.surfaceStages[stage];

	GLimp_LogComment("--- Render_refraction_C ---\n");

	GL_State(pStage->stateBits);

	// enable shader, set arrays
	GL_Program(tr.refractionShader_C.program);
	GL_ClientState(tr.refractionShader_C.attribs);
	GL_SetVertexAttribs();

	// set uniforms
	VectorCopy(backEnd.viewParms.or.origin, viewOrigin);	// in world space
	qglUniform3fARB(tr.refractionShader_C.u_ViewOrigin, viewOrigin[0], viewOrigin[1], viewOrigin[2]);
	qglUniform1fARB(tr.refractionShader_C.u_RefractionIndex, RB_EvalExpression(&pStage->refractionIndexExp, 1.0));
	qglUniform1fARB(tr.refractionShader_C.u_FresnelPower, RB_EvalExpression(&pStage->fresnelPowerExp, 2.0));
	qglUniform1fARB(tr.refractionShader_C.u_FresnelScale, RB_EvalExpression(&pStage->fresnelScaleExp, 2.0));
	qglUniform1fARB(tr.refractionShader_C.u_FresnelBias, RB_EvalExpression(&pStage->fresnelBiasExp, 1.0));

	// bind u_ColorMap
	GL_SelectTexture(0);
	GL_Bind(pStage->bundle[TB_COLORMAP].image[0]);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(backEnd.or.transformMatrix);
	qglMatrixMode(GL_MODELVIEW);

	DrawElements();

	GL_CheckErrors();
}

static void Render_dispersion_C(int stage)
{
	vec3_t          viewOrigin;
	shaderStage_t  *pStage = tess.surfaceStages[stage];
	float           eta;
	float           etaDelta;

	GLimp_LogComment("--- Render_dispersion_C ---\n");

	GL_State(pStage->stateBits);

	// enable shader, set arrays
	GL_Program(tr.dispersionShader_C.program);
	GL_ClientState(tr.dispersionShader_C.attribs);
	GL_SetVertexAttribs();

	// set uniforms
	VectorCopy(backEnd.viewParms.or.origin, viewOrigin);	// in world space
	eta = RB_EvalExpression(&pStage->etaExp, (float)1.1);
	etaDelta = RB_EvalExpression(&pStage->etaDeltaExp, (float)-0.02);

	qglUniform3fARB(tr.dispersionShader_C.u_ViewOrigin, viewOrigin[0], viewOrigin[1], viewOrigin[2]);
	qglUniform3fARB(tr.dispersionShader_C.u_EtaRatio, eta, eta + etaDelta, eta + (etaDelta * 2));
	qglUniform1fARB(tr.dispersionShader_C.u_FresnelPower, RB_EvalExpression(&pStage->fresnelPowerExp, 2.0f));
	qglUniform1fARB(tr.dispersionShader_C.u_FresnelScale, RB_EvalExpression(&pStage->fresnelScaleExp, 2.0f));
	qglUniform1fARB(tr.dispersionShader_C.u_FresnelBias, RB_EvalExpression(&pStage->fresnelBiasExp, 1.0f));

	// bind u_ColorMap
	GL_SelectTexture(0);
	GL_Bind(pStage->bundle[TB_COLORMAP].image[0]);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(backEnd.or.transformMatrix);
	qglMatrixMode(GL_MODELVIEW);

	DrawElements();

	GL_CheckErrors();
}

static void Render_skybox(int stage)
{
	vec3_t          viewOrigin;
	shaderStage_t  *pStage = tess.surfaceStages[stage];

	GLimp_LogComment("--- Render_skybox ---\n");

	GL_State(pStage->stateBits);

	// enable shader, set arrays
	GL_Program(tr.skyBoxShader.program);
	GL_ClientState(tr.skyBoxShader.attribs);
	GL_SetVertexAttribs();

	// set uniforms
	VectorCopy(backEnd.viewParms.or.origin, viewOrigin);	// in world space
	qglUniform3fARB(tr.skyBoxShader.u_ViewOrigin, viewOrigin[0], viewOrigin[1], viewOrigin[2]);

	// bind u_ColorMap
	GL_SelectTexture(0);
	GL_Bind(pStage->bundle[TB_COLORMAP].image[0]);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(backEnd.or.transformMatrix);
	qglMatrixMode(GL_MODELVIEW);

	DrawElements();

	GL_CheckErrors();
}

static void Render_portal(int stage)
{
	/*
	float           fbufWidthScale, fbufHeightScale;
	float           npotWidthScale, npotHeightScale;
	shaderStage_t  *pStage = tess.surfaceStages[stage];

	GLimp_LogComment("--- Render_portal ---\n");

	GL_State(pStage->stateBits);

	// enable shader, set arrays
	GL_Program(tr.screenShader.program);
	GL_ClientState(tr.screenShader.attribs);
	GL_SetVertexAttribs();
	
	// set uniforms
	fbufWidthScale = Q_recip((float)glConfig.vidWidth);
	fbufHeightScale = Q_recip((float)glConfig.vidHeight);
	npotWidthScale = (float)glConfig.vidWidth / (float)NearestPowerOfTwo(glConfig.vidWidth);
	npotHeightScale = (float)glConfig.vidHeight / (float)NearestPowerOfTwo(glConfig.vidHeight);

	qglUniform2fARB(tr.screenShader.u_FBufScale, fbufWidthScale, fbufHeightScale);
	qglUniform2fARB(tr.screenShader.u_NPOTScale, npotWidthScale, npotHeightScale);

	// bind u_CurrentMap
	GL_SelectTexture(0);
	GL_Bind(tr.portalRenderFBOImage[0]);

	DrawElements();

	GL_CheckErrors();
	*/
}

static void Render_heatHaze(int stage)
{
	float           deformMagnitude;
	float           fbufWidthScale, fbufHeightScale;
	float           npotWidthScale, npotHeightScale;
	shaderStage_t  *pStage = tess.surfaceStages[stage];

	GLimp_LogComment("--- Render_heatHaze ---\n");
	
	fbufWidthScale = Q_recip((float)glConfig.vidWidth);
	fbufHeightScale = Q_recip((float)glConfig.vidHeight);
	npotWidthScale = (float)glConfig.vidWidth / (float)NearestPowerOfTwo(glConfig.vidWidth);
	npotHeightScale = (float)glConfig.vidHeight / (float)NearestPowerOfTwo(glConfig.vidHeight);
	
	/*
	if(glConfig.framebufferObjectAvailable && glConfig.maxColorAttachments >= 4 &&
	   glConfig.drawBuffersAvailable && glConfig.maxDrawBuffers >= 4)
	{
		unsigned        stateBits;
		
		// remove blend mode
		stateBits = pStage->stateBits;
		stateBits &= ~(GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS);
		
		GL_State(stateBits);
		
		// enable shader, set arrays
		GL_Program(tr.depthTestShader.program);
		GL_ClientState(tr.depthTestShader.attribs);
		GL_SetVertexAttribs();
		
		// set uniforms		
		qglUniform2fARB(tr.depthTestShader.u_FBufScale, fbufWidthScale, fbufHeightScale);
		qglUniform2fARB(tr.depthTestShader.u_NPOTScale, npotWidthScale, npotHeightScale);
		
		// bind u_ColorMap
		GL_SelectTexture(0);
		GL_Bind(pStage->bundle[TB_COLORMAP].image[0]);
		qglMatrixMode(GL_TEXTURE);
		qglLoadMatrixf(tess.svars.texMatrices[TB_COLORMAP]);
		qglMatrixMode(GL_MODELVIEW);
		
		// bind u_CurrentMap
		GL_SelectTexture(1);
		if(backEnd.viewParms.isPortal)
		{
			GL_Bind(tr.portalRenderFBOImage[0]);
			GL_TextureFilter(tr.portalRenderFBOImage[0], FT_NEAREST);
		}
		else
		{
			GL_Bind(tr.currentRenderFBOImage[0]);
			GL_TextureFilter(tr.currentRenderFBOImage[0], FT_NEAREST);
		}
		qglMatrixMode(GL_TEXTURE);
		qglLoadIdentity();
		qglMatrixMode(GL_MODELVIEW);

		DrawElements();
	}
	else
	*/
	{
		unsigned        stateBits;
		
		// capture current color buffer for u_CurrentMap
		GL_SelectTexture(0);
		GL_Bind(tr.currentRenderImage);
		qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, tr.currentRenderImage->uploadWidth, tr.currentRenderImage->uploadHeight);
							 
		// clear color buffer
		qglClear(GL_COLOR_BUFFER_BIT);
		
		// remove blend mode
		stateBits = pStage->stateBits;
		stateBits &= ~(GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS);
		
		GL_State(stateBits);
		
		// enable shader, set arrays
		GL_Program(tr.depthTestShader.program);
		GL_ClientState(tr.depthTestShader.attribs);
		GL_SetVertexAttribs();
		
		// set uniforms	
		qglUniform2fARB(tr.depthTestShader.u_FBufScale, fbufWidthScale, fbufHeightScale);
		qglUniform2fARB(tr.depthTestShader.u_NPOTScale, npotWidthScale, npotHeightScale);
		
		// bind u_ColorMap
		GL_SelectTexture(0);
		GL_Bind(pStage->bundle[TB_COLORMAP].image[0]);
		qglMatrixMode(GL_TEXTURE);
		qglLoadMatrixf(tess.svars.texMatrices[TB_COLORMAP]);
		qglMatrixMode(GL_MODELVIEW);
		
		// bind u_CurrentMap
		GL_SelectTexture(1);
		GL_Bind(tr.currentRenderImage);

		DrawElements();
		
		// capture current color buffer for u_ContrastMap 
		GL_SelectTexture(0);
		GL_Bind(tr.contrastRenderImage);
		qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, tr.contrastRenderImage->uploadWidth, tr.contrastRenderImage->uploadHeight);
		
		
		//
		// restore color buffer
		//
		
		// enable shader, set arrays
		GL_Program(tr.screenShader.program);
		GL_State(GLS_DEPTHTEST_DISABLE);
		GL_ClientState(tr.screenShader.attribs);
		//GL_SetVertexAttribs();
		GL_Cull(CT_TWO_SIDED);
	
		// set uniforms
		qglUniform2fARB(tr.screenShader.u_FBufScale, fbufWidthScale, fbufHeightScale);
		qglUniform2fARB(tr.screenShader.u_NPOTScale, npotWidthScale, npotHeightScale);

		// bind u_CurrentMap
		GL_SelectTexture(0);
		GL_Bind(tr.currentRenderImage);

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
		qglVertex2f(backEnd.viewParms.viewportX + backEnd.viewParms.viewportWidth, backEnd.viewParms.viewportY + backEnd.viewParms.viewportHeight);
		qglVertex2f(backEnd.viewParms.viewportX, backEnd.viewParms.viewportY + backEnd.viewParms.viewportHeight);
		qglEnd();

		qglPopMatrix();
		qglMatrixMode(GL_MODELVIEW);
		qglPopMatrix();
	}
	
#if 1
	// enable shader, set arrays
	GL_Program(tr.heatHazeShader.program);
	GL_State(pStage->stateBits);
	GL_ClientState(tr.heatHazeShader.attribs);
	GL_SetVertexAttribs();

	// set uniforms
	deformMagnitude = RB_EvalExpression(&pStage->deformMagnitudeExp, 1.0);
	
	qglUniform1fARB(tr.heatHazeShader.u_DeformMagnitude, deformMagnitude);
	qglUniform2fARB(tr.heatHazeShader.u_FBufScale, fbufWidthScale, fbufHeightScale);
	qglUniform2fARB(tr.heatHazeShader.u_NPOTScale, npotWidthScale, npotHeightScale);

	if(glConfig.hardwareType == GLHW_ATI)
	{
		matrix_t        projectionMatrixTranspose;

		MatrixTranspose(backEnd.viewParms.projectionMatrix, projectionMatrixTranspose);
		qglUniformMatrix4fvARB(tr.heatHazeShader.u_ProjectionMatrixTranspose, 1, GL_FALSE, projectionMatrixTranspose);
	}
	
	// bind u_NormalMap
	GL_SelectTexture(0);
	GL_Bind(pStage->bundle[TB_COLORMAP].image[0]);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(tess.svars.texMatrices[TB_COLORMAP]);
	qglMatrixMode(GL_MODELVIEW);
	
	// bind u_CurrentMap
	GL_SelectTexture(1);
	/*
	if(glConfig.framebufferObjectAvailable)
	{
		if(backEnd.viewParms.isPortal)
		{
			GL_Bind(tr.portalRenderFBOImage[0]);
			GL_TextureFilter(tr.portalRenderFBOImage[0], FT_NEAREST);
		}
		else
		{
			GL_Bind(tr.currentRenderFBOImage[0]);
			GL_TextureFilter(tr.currentRenderFBOImage[0], FT_NEAREST);
		}
	}
	else
	*/
	{
		GL_Bind(tr.currentRenderImage);
		qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, tr.currentRenderImage->uploadWidth, tr.currentRenderImage->uploadHeight);
	}
	
	// bind u_ContrastMap
	GL_SelectTexture(2);
	/*
	if(glConfig.framebufferObjectAvailable)
	{
		if(backEnd.viewParms.isPortal)
		{
			GL_Bind(tr.portalRenderFBOImage[1]);
			GL_TextureFilter(tr.portalRenderFBOImage[1], FT_NEAREST);
		}
		else
		{
			GL_Bind(tr.currentRenderFBOImage[1]);
			GL_TextureFilter(tr.currentRenderFBOImage[1], FT_NEAREST);
		}
	}
	else
	*/
	{
		GL_Bind(tr.contrastRenderImage);
	}

	DrawElements();
#endif

	GL_CheckErrors();
}

static void Render_bloom(int stage)
{
	float           blurMagnitude;
	float           fbufWidthScale, fbufHeightScale;
	float           npotWidthScale, npotHeightScale;
	shaderStage_t  *pStage = tess.surfaceStages[stage];

	GLimp_LogComment("--- Render_bloom ---\n");

	GL_State(pStage->stateBits);

	// calc uniforms
	blurMagnitude = RB_EvalExpression(&pStage->blurMagnitudeExp, 3.0);
	fbufWidthScale = Q_recip((float)glConfig.vidWidth);
	fbufHeightScale = Q_recip((float)glConfig.vidHeight);
	npotWidthScale = (float)glConfig.vidWidth / (float)NearestPowerOfTwo(glConfig.vidWidth);
	npotHeightScale = (float)glConfig.vidHeight / (float)NearestPowerOfTwo(glConfig.vidHeight);

	// render contrast
	GL_Program(tr.contrastShader.program);
	GL_ClientState(tr.contrastShader.attribs);
	GL_SetVertexAttribs();

	qglUniform2fARB(tr.contrastShader.u_FBufScale, fbufWidthScale, fbufHeightScale);
	qglUniform2fARB(tr.contrastShader.u_NPOTScale, npotWidthScale, npotHeightScale);

	GL_SelectTexture(0);
	GL_Bind(tr.currentRenderImage);
	qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, tr.currentRenderImage->uploadWidth, tr.currentRenderImage->uploadHeight);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(tess.svars.texMatrices[TB_COLORMAP]);
	qglMatrixMode(GL_MODELVIEW);

	DrawElements();

	// render bloom
	GL_Program(tr.bloomShader.program);
	GL_ClientState(tr.bloomShader.attribs);
	GL_SetVertexAttribs();

	qglUniform1fARB(tr.bloomShader.u_BlurMagnitude, blurMagnitude);
	qglUniform2fARB(tr.bloomShader.u_FBufScale, fbufWidthScale, fbufHeightScale);
	qglUniform2fARB(tr.bloomShader.u_NPOTScale, npotWidthScale, npotHeightScale);

	GL_SelectTexture(1);
	GL_Bind(tr.contrastRenderImage);
	qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, tr.contrastRenderImage->uploadWidth, tr.contrastRenderImage->uploadHeight);

	DrawElements();

	GL_CheckErrors();
}

static void Render_bloom2(int stage)
{
	float           blurMagnitude;
	float           fbufWidthScale, fbufHeightScale;
	float           npotWidthScale, npotHeightScale;
	shaderStage_t  *pStage = tess.surfaceStages[stage];

	GLimp_LogComment("--- Render_bloom2 ---\n");

	GL_State(pStage->stateBits);

	// calc uniforms
	blurMagnitude = RB_EvalExpression(&pStage->blurMagnitudeExp, 3.0);
	fbufWidthScale = Q_recip((float)glConfig.vidWidth);
	fbufHeightScale = Q_recip((float)glConfig.vidHeight);
	npotWidthScale = (float)glConfig.vidWidth / (float)NearestPowerOfTwo(glConfig.vidWidth);
	npotHeightScale = (float)glConfig.vidHeight / (float)NearestPowerOfTwo(glConfig.vidHeight);

	// render contrast
	GL_Program(tr.contrastShader.program);
	GL_ClientState(tr.contrastShader.attribs);
	GL_SetVertexAttribs();

	qglUniform2fARB(tr.contrastShader.u_FBufScale, fbufWidthScale, fbufHeightScale);
	qglUniform2fARB(tr.contrastShader.u_NPOTScale, npotWidthScale, npotHeightScale);

	GL_SelectTexture(0);
	GL_Bind(tr.currentRenderImage);
	qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, tr.currentRenderImage->uploadWidth, tr.currentRenderImage->uploadHeight);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(tess.svars.texMatrices[TB_COLORMAP]);
	qglMatrixMode(GL_MODELVIEW);

	DrawElements();

	// render blurX
	GL_Program(tr.blurXShader.program);
	GL_ClientState(tr.blurXShader.attribs);
	GL_SetVertexAttribs();

	qglUniform2fARB(tr.blurXShader.u_FBufScale, fbufWidthScale, fbufHeightScale);
	qglUniform2fARB(tr.blurXShader.u_NPOTScale, npotWidthScale, npotHeightScale);

	GL_Bind(tr.contrastRenderImage);
	qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, tr.contrastRenderImage->uploadWidth, tr.contrastRenderImage->uploadHeight);

	DrawElements();

	// render blurY
	GL_Program(tr.blurYShader.program);
	GL_ClientState(tr.blurYShader.attribs);
	GL_SetVertexAttribs();

	qglUniform2fARB(tr.blurYShader.u_FBufScale, fbufWidthScale, fbufHeightScale);
	qglUniform2fARB(tr.blurYShader.u_NPOTScale, npotWidthScale, npotHeightScale);

	GL_Bind(tr.contrastRenderImage);
	qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, tr.contrastRenderImage->uploadWidth, tr.contrastRenderImage->uploadHeight);

	DrawElements();

	// render bloom
	GL_Program(tr.bloomShader.program);
	GL_ClientState(tr.bloomShader.attribs);
	GL_SetVertexAttribs();

	qglUniform1fARB(tr.bloomShader.u_BlurMagnitude, blurMagnitude);
	qglUniform2fARB(tr.bloomShader.u_FBufScale, fbufWidthScale, fbufHeightScale);
	qglUniform2fARB(tr.bloomShader.u_NPOTScale, npotWidthScale, npotHeightScale);

	GL_SelectTexture(0);
	GL_Bind(tr.currentRenderImage);

	GL_SelectTexture(1);
	GL_Bind(tr.contrastRenderImage);

	DrawElements();

	GL_CheckErrors();
}

static void Render_rotoscope(int stage)
{
	float           blurMagnitude;
	float           fbufWidthScale, fbufHeightScale;
	float           npotWidthScale, npotHeightScale;
	shaderStage_t  *pStage = tess.surfaceStages[stage];

	GLimp_LogComment("--- Render_rotoscope ---\n");

	GL_State(pStage->stateBits);

	// enable shader, set arrays
	GL_Program(tr.rotoscopeShader.program);
	GL_ClientState(tr.rotoscopeShader.attribs);
	GL_SetVertexAttribs();

	// set uniforms
	blurMagnitude = RB_EvalExpression(&pStage->blurMagnitudeExp, 3.0);
	fbufWidthScale = Q_recip((float)glConfig.vidWidth);
	fbufHeightScale = Q_recip((float)glConfig.vidHeight);
	npotWidthScale = (float)glConfig.vidWidth / (float)NearestPowerOfTwo(glConfig.vidWidth);
	npotHeightScale = (float)glConfig.vidHeight / (float)NearestPowerOfTwo(glConfig.vidHeight);

	qglUniform1fARB(tr.rotoscopeShader.u_BlurMagnitude, blurMagnitude);
	qglUniform2fARB(tr.rotoscopeShader.u_FBufScale, fbufWidthScale, fbufHeightScale);
	qglUniform2fARB(tr.rotoscopeShader.u_NPOTScale, npotWidthScale, npotHeightScale);

	// bind u_ColorMap
	GL_SelectTexture(0);
	GL_Bind(tr.currentRenderImage);
	qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, tr.currentRenderImage->uploadWidth, tr.currentRenderImage->uploadHeight);
	qglMatrixMode(GL_TEXTURE);
	qglLoadMatrixf(tess.svars.texMatrices[TB_COLORMAP]);
	qglMatrixMode(GL_MODELVIEW);

	DrawElements();

	GL_CheckErrors();
}

static void Render_fog()
{
	fog_t          *fog;
	int             i;

	GLimp_LogComment("--- Render_fog ---\n");

	fog = tr.world->fogs + tess.fogNum;

	for(i = 0; i < tess.numVertexes; i++)
	{
		*(int *)&tess.svars.colors[i] = fog->colorInt;
	}

	RB_CalcFogTexCoords((float *)tess.svars.texCoords[0]);

	GL_Program(0);
	GL_ClientState(GLCS_VERTEX | GLCS_COLOR);
	GL_SetVertexAttribs();

	GL_SelectTexture(0);
//  qglEnable(GL_TEXTURE_2D);
	qglEnableClientState(GL_TEXTURE_COORD_ARRAY);
	qglTexCoordPointer(2, GL_FLOAT, 0, tess.svars.texCoords[TB_COLORMAP]);
	GL_Bind(tr.fogImage);

	if(tess.surfaceShader->fogPass == FP_EQUAL)
	{
		GL_State(GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA | GLS_DEPTHFUNC_EQUAL);
	}
	else
	{
		GL_State(GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);
	}

	DrawElements();

	qglDisableClientState(GL_TEXTURE_COORD_ARRAY);
//  qglDisable(GL_TEXTURE_2D);

	GL_CheckErrors();
}

/*
===============
ComputeColors
===============
*/
static void ComputeColors(shaderStage_t * pStage)
{
	int             i;

	GLimp_LogComment("--- ComputeColors ---\n");

	// rgbGen
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
			Com_Memcpy(tess.svars.colors, tess.colors, tess.numVertexes * sizeof(tess.colors[0]));
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
				Com_Memcpy(tess.svars.colors, tess.colors, tess.numVertexes * sizeof(tess.colors[0]));
			}
			else
			{
				for(i = 0; i < tess.numVertexes; i++)
				{
					tess.svars.colors[i][0] = tess.colors[i][0] * tr.identityLight;
					tess.svars.colors[i][1] = tess.colors[i][1] * tr.identityLight;
					tess.svars.colors[i][2] = tess.colors[i][2] * tr.identityLight;
					tess.svars.colors[i][3] = tess.colors[i][3];
				}
			}
			break;

		case CGEN_ONE_MINUS_VERTEX:
			if(tr.identityLight == 1)
			{
				for(i = 0; i < tess.numVertexes; i++)
				{
					tess.svars.colors[i][0] = 255 - tess.colors[i][0];
					tess.svars.colors[i][1] = 255 - tess.colors[i][1];
					tess.svars.colors[i][2] = 255 - tess.colors[i][2];
				}
			}
			else
			{
				for(i = 0; i < tess.numVertexes; i++)
				{
					tess.svars.colors[i][0] = (255 - tess.colors[i][0]) * tr.identityLight;
					tess.svars.colors[i][1] = (255 - tess.colors[i][1]) * tr.identityLight;
					tess.svars.colors[i][2] = (255 - tess.colors[i][2]) * tr.identityLight;
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
			break;
		}

		case CGEN_WAVEFORM:
			RB_CalcWaveColor(&pStage->rgbWave, (unsigned char *)tess.svars.colors);
			break;

		case CGEN_ENTITY:
			RB_CalcColorFromEntity((unsigned char *)tess.svars.colors);
			break;

		case CGEN_ONE_MINUS_ENTITY:
			RB_CalcColorFromOneMinusEntity((unsigned char *)tess.svars.colors);
			break;

		case CGEN_CUSTOM_RGB:
			RB_CalcCustomColor(&pStage->rgbExp, (unsigned char *)tess.svars.colors);
			break;

		case CGEN_CUSTOM_RGBs:
			RB_CalcCustomColors(&pStage->redExp, &pStage->greenExp, &pStage->blueExp, (unsigned char *)tess.svars.colors);
			break;
	}

	// alphaGen
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
					tess.svars.colors[i][3] = tess.colors[i][3];
				}
			}
			break;

		case AGEN_ONE_MINUS_VERTEX:
			for(i = 0; i < tess.numVertexes; i++)
			{
				tess.svars.colors[i][3] = 255 - tess.colors[i][3];
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

				len /= tess.surfaceShader->portalRange;

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
			break;
		}

		case AGEN_CUSTOM:
			RB_CalcCustomAlpha(&pStage->alphaExp, (unsigned char *)tess.svars.colors);
			break;
	}

	// fog adjustment for colors to fade out as fog increases
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
ComputeColor
===============
*/
static void ComputeColor(shaderStage_t * pStage)
{
	float           rgb;
	float           red;
	float           green;
	float           blue;
	float           alpha;

	GLimp_LogComment("--- ComputeColor ---\n");

	// rgbGen
	switch (pStage->rgbGen)
	{
		case CGEN_IDENTITY:
		{
			tess.svars.color[0] = 1.0;
			tess.svars.color[1] = 1.0;
			tess.svars.color[2] = 1.0;
			tess.svars.color[3] = 1.0;
			break;
		}

		default:
		case CGEN_IDENTITY_LIGHTING:
		{
			tess.svars.color[0] = tr.identityLightByte * (1.0 / 255.0);
			tess.svars.color[1] = tr.identityLightByte * (1.0 / 255.0);
			tess.svars.color[2] = tr.identityLightByte * (1.0 / 255.0);
			tess.svars.color[3] = tr.identityLightByte * (1.0 / 255.0);
			break;
		}

		case CGEN_CONST:
		{
			tess.svars.color[0] = pStage->constantColor[0] * (1.0 / 255.0);
			tess.svars.color[1] = pStage->constantColor[1] * (1.0 / 255.0);
			tess.svars.color[2] = pStage->constantColor[2] * (1.0 / 255.0);
			tess.svars.color[3] = pStage->constantColor[3] * (1.0 / 255.0);
			break;
		}

		case CGEN_ENTITY:
		{
			if(backEnd.currentLight)
			{
				tess.svars.color[0] = Q_bound(0.0, backEnd.currentLight->l.color[0], 1.0);
				tess.svars.color[1] = Q_bound(0.0, backEnd.currentLight->l.color[1], 1.0);
				tess.svars.color[2] = Q_bound(0.0, backEnd.currentLight->l.color[2], 1.0);
				tess.svars.color[3] = 1.0;
			}
			else if(backEnd.currentEntity)
			{
				tess.svars.color[0] = Q_bound(0.0, backEnd.currentEntity->e.shaderRGBA[0] * (1.0 / 255.0), 1.0);
				tess.svars.color[1] = Q_bound(0.0, backEnd.currentEntity->e.shaderRGBA[1] * (1.0 / 255.0), 1.0);
				tess.svars.color[2] = Q_bound(0.0, backEnd.currentEntity->e.shaderRGBA[2] * (1.0 / 255.0), 1.0);
				tess.svars.color[3] = Q_bound(0.0, backEnd.currentEntity->e.shaderRGBA[3] * (1.0 / 255.0), 1.0);
			}
			else
			{
				tess.svars.color[0] = 1.0;
				tess.svars.color[1] = 1.0;
				tess.svars.color[2] = 1.0;
				tess.svars.color[3] = 1.0;
			}
			break;
		}

		case CGEN_ONE_MINUS_ENTITY:
		{
			if(backEnd.currentLight)
			{
				tess.svars.color[0] = 1.0 - Q_bound(0.0, backEnd.currentLight->l.color[0], 1.0);
				tess.svars.color[1] = 1.0 - Q_bound(0.0, backEnd.currentLight->l.color[1], 1.0);
				tess.svars.color[2] = 1.0 - Q_bound(0.0, backEnd.currentLight->l.color[2], 1.0);
				tess.svars.color[3] = 0.0;	// FIXME
			}
			else if(backEnd.currentEntity)
			{
				tess.svars.color[0] = 1.0 - Q_bound(0.0, backEnd.currentEntity->e.shaderRGBA[0] * (1.0 / 255.0), 1.0);
				tess.svars.color[1] = 1.0 - Q_bound(0.0, backEnd.currentEntity->e.shaderRGBA[1] * (1.0 / 255.0), 1.0);
				tess.svars.color[2] = 1.0 - Q_bound(0.0, backEnd.currentEntity->e.shaderRGBA[2] * (1.0 / 255.0), 1.0);
				tess.svars.color[3] = 1.0 - Q_bound(0.0, backEnd.currentEntity->e.shaderRGBA[3] * (1.0 / 255.0), 1.0);
			}
			else
			{
				tess.svars.color[0] = 0.0;
				tess.svars.color[1] = 0.0;
				tess.svars.color[2] = 0.0;
				tess.svars.color[3] = 0.0;
			}
			break;
		}

		case CGEN_CUSTOM_RGB:
		{
			rgb = Q_bound(0.0, RB_EvalExpression(&pStage->rgbExp, 1.0), 1.0);

			tess.svars.color[0] = rgb;
			tess.svars.color[1] = rgb;
			tess.svars.color[2] = rgb;
			break;
		}

		case CGEN_CUSTOM_RGBs:
		{
			if(backEnd.currentLight)
			{
				red = Q_bound(0.0, RB_EvalExpression(&pStage->redExp, backEnd.currentLight->l.color[0]), 1.0);
				green = Q_bound(0.0, RB_EvalExpression(&pStage->greenExp, backEnd.currentLight->l.color[1]), 1.0);
				blue = Q_bound(0.0, RB_EvalExpression(&pStage->blueExp, backEnd.currentLight->l.color[2]), 1.0);
			}
			else if(backEnd.currentEntity)
			{
				red =
					Q_bound(0.0, RB_EvalExpression(&pStage->redExp, backEnd.currentEntity->e.shaderRGBA[0] * (1.0 / 255.0)), 1.0);
				green =
					Q_bound(0.0, RB_EvalExpression(&pStage->greenExp, backEnd.currentEntity->e.shaderRGBA[1] * (1.0 / 255.0)),
							1.0);
				blue =
					Q_bound(0.0, RB_EvalExpression(&pStage->blueExp, backEnd.currentEntity->e.shaderRGBA[2] * (1.0 / 255.0)),
							1.0);
			}
			else
			{
				red = Q_bound(0.0, RB_EvalExpression(&pStage->redExp, 1.0), 1.0);
				green = Q_bound(0.0, RB_EvalExpression(&pStage->greenExp, 1.0), 1.0);
				blue = Q_bound(0.0, RB_EvalExpression(&pStage->blueExp, 1.0), 1.0);
			}

			tess.svars.color[0] = red;
			tess.svars.color[1] = green;
			tess.svars.color[2] = blue;
			break;
		}
	}

	// alphaGen
	switch (pStage->alphaGen)
	{
		case AGEN_SKIP:
			break;

		default:
		case AGEN_IDENTITY:
		{
			if(pStage->rgbGen != CGEN_IDENTITY)
			{
				if((pStage->rgbGen == CGEN_VERTEX && tr.identityLight != 1) || pStage->rgbGen != CGEN_VERTEX)
				{
					tess.svars.color[3] = 1.0;
				}
			}
			break;
		}

		case AGEN_CONST:
		{
			if(pStage->rgbGen != CGEN_CONST)
			{
				tess.svars.color[3] = pStage->constantColor[3] * (1.0 / 255.0);
			}
			break;
		}

		case AGEN_ENTITY:
		{
			if(backEnd.currentLight)
			{
				tess.svars.color[3] = 1.0;	// FIXME
			}
			else if(backEnd.currentEntity)
			{
				tess.svars.color[3] = Q_bound(0.0, backEnd.currentEntity->e.shaderRGBA[3] * (1.0 / 255.0), 1.0);
			}
			else
			{
				tess.svars.color[3] = 1.0;
			}
			break;
		}

		case AGEN_ONE_MINUS_ENTITY:
		{
			if(backEnd.currentLight)
			{
				tess.svars.color[3] = 0.0;	// FIXME
			}
			else if(backEnd.currentEntity)
			{
				tess.svars.color[3] = 1.0 - Q_bound(0.0, backEnd.currentEntity->e.shaderRGBA[3] * (1.0 / 255.0), 1.0);
			}
			else
			{
				tess.svars.color[3] = 0.0;
			}
			break;
		}

		case AGEN_CUSTOM:
		{
			alpha = Q_bound(0.0, RB_EvalExpression(&pStage->alphaExp, 1.0), 1.0);

			tess.svars.color[3] = alpha;
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

	GLimp_LogComment("--- ComputeTexCoords ---\n");

	// generate the lightmap coordinates
	if(tess.lightmapNum >= 0 && tess.lightmapNum < tr.numLightmaps)
	{
		MatrixIdentity(tess.svars.texMatrices[TB_LIGHTMAP]);

		for(i = 0; i < tess.numVertexes; i++)
		{
			tess.svars.texCoords[TB_LIGHTMAP][i][0] = tess.texCoords[i][1][0];
			tess.svars.texCoords[TB_LIGHTMAP][i][1] = tess.texCoords[i][1][1];
		}

		// tcMods will be ignored
	}

	// generate the texture coordinates
	for(b = 0; b < TB_LIGHTMAP; b++)
	{
		int             tm;

		// reset texture matrix
		MatrixIdentity(tess.svars.texMatrices[b]);

		// generate the texture coordinates by default
		tess.svars.skipCoords[b] = qfalse;

		switch (pStage->bundle[b].tcGen)
		{
			case TCGEN_BAD:
			case TCGEN_SKIP:
				tess.svars.skipCoords[b] = qtrue;
				continue;

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

			case TCGEN_VECTOR:
				for(i = 0; i < tess.numVertexes; i++)
				{
					tess.svars.texCoords[b][i][0] = DotProduct(tess.xyz[i], pStage->bundle[b].tcGenVectors[0]);
					tess.svars.texCoords[b][i][1] = DotProduct(tess.xyz[i], pStage->bundle[b].tcGenVectors[1]);
				}
				break;

			case TCGEN_FOG:
				RB_CalcFogTexCoords((float *)tess.svars.texCoords[b]);
				break;

			case TCGEN_ENVIRONMENT_MAPPED:
				RB_CalcEnvironmentTexCoords((float *)tess.svars.texCoords[b]);
				break;
		}

		// alter texture coordinates
		for(tm = 0; tm < pStage->bundle[b].numTexMods; tm++)
		{
			if(pStage->bundle[b].tcGen == TCGEN_SKIP)
				break;

			switch (pStage->bundle[b].texMods[tm].type)
			{
				case TMOD_NONE:
					tm = TR_MAX_TEXMODS;	// break out of for loop
					break;

				case TMOD_TURBULENT:
					RB_CalcTurbulentTexCoords(&pStage->bundle[b].texMods[tm].wave, (float *)tess.svars.texCoords[b]);
					break;

				case TMOD_ENTITY_TRANSLATE:
					RB_CalcScrollTexCoords(backEnd.currentEntity->e.shaderTexCoord, (float *)tess.svars.texCoords[b]);
					break;

				case TMOD_SCROLL:
					RB_CalcScrollTexCoords(pStage->bundle[b].texMods[tm].scroll, (float *)tess.svars.texCoords[b]);
					break;

				case TMOD_SCALE:
					RB_CalcScaleTexCoords(pStage->bundle[b].texMods[tm].scale, (float *)tess.svars.texCoords[b]);
					break;

				case TMOD_STRETCH:
					RB_CalcStretchTexCoords(&pStage->bundle[b].texMods[tm].wave, (float *)tess.svars.texCoords[b]);
					break;

				case TMOD_TRANSFORM:
					RB_CalcTransformTexCoords(&pStage->bundle[b].texMods[tm], (float *)tess.svars.texCoords[b]);
					break;

				case TMOD_ROTATE:
					RB_CalcRotateTexCoords(pStage->bundle[b].texMods[tm].rotateSpeed, (float *)tess.svars.texCoords[b]);
					break;

				case TMOD_SCROLL2:
					RB_CalcScrollTexCoords2(&pStage->bundle[b].texMods[tm].sExp,
											&pStage->bundle[b].texMods[tm].tExp, (float *)tess.svars.texCoords[b]);
					break;

				case TMOD_SCALE2:
					RB_CalcScaleTexCoords2(&pStage->bundle[b].texMods[tm].sExp,
										   &pStage->bundle[b].texMods[tm].tExp, (float *)tess.svars.texCoords[b]);
					break;

				case TMOD_CENTERSCALE:
					RB_CalcCenterScaleTexCoords(&pStage->bundle[b].texMods[tm].sExp,
												&pStage->bundle[b].texMods[tm].tExp, (float *)tess.svars.texCoords[b]);
					break;

				case TMOD_SHEAR:
					RB_CalcShearTexCoords(&pStage->bundle[b].texMods[tm].sExp,
										  &pStage->bundle[b].texMods[tm].tExp, (float *)tess.svars.texCoords[b]);
					break;

				case TMOD_ROTATE2:
					RB_CalcRotateTexCoords2(&pStage->bundle[b].texMods[tm].rExp, (float *)tess.svars.texCoords[b]);
					break;

				default:
					ri.Error(ERR_DROP, "ERROR: unknown texmod '%d' in shader '%s'\n",
							 pStage->bundle[b].texMods[tm].type, tess.surfaceShader->name);
					break;
			}
		}
	}
}


/*
===============
ComputeTexMatrices
===============
*/
static void ComputeTexMatrices(shaderStage_t * pStage)
{
	int             i, j;
	vec_t          *matrix;
	float           x, y;

	GLimp_LogComment("--- ComputeTexMatrices ---\n");

	for(i = 0; i < MAX_TEXTURE_BUNDLES; i++)
	{
		matrix = tess.svars.texMatrices[i];

		MatrixIdentity(matrix);

		for(j = 0; j < pStage->bundle[i].numTexMods; j++)
		{
			switch (pStage->bundle[i].texMods[j].type)
			{
				case TMOD_NONE:
					j = TR_MAX_TEXMODS;	// break out of for loop
					break;

				case TMOD_SCROLL2:
				{
					x = RB_EvalExpression(&pStage->bundle[TB_COLORMAP].texMods[i].sExp, 0);
					y = RB_EvalExpression(&pStage->bundle[TB_COLORMAP].texMods[i].tExp, 0);

					// clamp so coordinates don't continuously get larger, causing problems
					// with hardware limits
					x = x - floor(x);
					y = y - floor(y);

					MatrixMultiplyTranslation(matrix, x, y, 0.0);
					break;
				}

				case TMOD_SCALE2:
				{
					x = RB_EvalExpression(&pStage->bundle[TB_COLORMAP].texMods[i].sExp, 0);
					y = RB_EvalExpression(&pStage->bundle[TB_COLORMAP].texMods[i].tExp, 0);

					MatrixMultiplyScale(matrix, x, y, 0.0);
					break;
				}

				case TMOD_CENTERSCALE:
				{
					x = RB_EvalExpression(&pStage->bundle[TB_COLORMAP].texMods[i].sExp, 0);
					y = RB_EvalExpression(&pStage->bundle[TB_COLORMAP].texMods[i].tExp, 0);

					MatrixMultiplyTranslation(matrix, 0.5, 0.5, 0.0);
					MatrixMultiplyScale(matrix, x, y, 0.0);
					MatrixMultiplyTranslation(matrix, -0.5, -0.5, 0.0);
					break;
				}

				case TMOD_SHEAR:
				{
					x = RB_EvalExpression(&pStage->bundle[TB_COLORMAP].texMods[i].sExp, 0);
					y = RB_EvalExpression(&pStage->bundle[TB_COLORMAP].texMods[i].tExp, 0);

					MatrixMultiplyTranslation(matrix, 0.5, 0.5, 0.0);
					MatrixMultiplyShear(matrix, x, y);
					MatrixMultiplyTranslation(matrix, -0.5, -0.5, 0.0);
					break;
				}

				case TMOD_ROTATE2:
				{
					x = RAD2DEG(RB_EvalExpression(&pStage->bundle[TB_COLORMAP].texMods[i].rExp, 0)) * 5.0;

					MatrixMultiplyTranslation(matrix, 0.5, 0.5, 0.0);
					MatrixMultiplyZRotation(matrix, x);
					MatrixMultiplyTranslation(matrix, -0.5, -0.5, 0.0);
					break;
				}

				default:
					break;
			}
		}
	}
}

/*
===============
ComputeFinalAttenuation
===============
*/
static void ComputeFinalAttenuation(shaderStage_t * pStage, trRefLight_t * light)
{
	int             i;
	matrix_t        matrix;
	float           x, y;

	GLimp_LogComment("--- ComputeFinalAttenuation ---\n");

	MatrixIdentity(matrix);

	for(i = 0; i < pStage->bundle[TB_COLORMAP].numTexMods; i++)
	{
		switch (pStage->bundle[TB_COLORMAP].texMods[i].type)
		{
			case TMOD_NONE:
				i = TR_MAX_TEXMODS;	// break out of for loop
				break;


			case TMOD_SCROLL2:
			{
				x = RB_EvalExpression(&pStage->bundle[TB_COLORMAP].texMods[i].sExp, 0);
				y = RB_EvalExpression(&pStage->bundle[TB_COLORMAP].texMods[i].tExp, 0);

				// clamp so coordinates don't continuously get larger, causing problems
				// with hardware limits
				x = x - floor(x);
				y = y - floor(y);

				MatrixMultiplyTranslation(matrix, x, y, 0.0);
				break;
			}

			case TMOD_SCALE2:
			{
				x = RB_EvalExpression(&pStage->bundle[TB_COLORMAP].texMods[i].sExp, 0);
				y = RB_EvalExpression(&pStage->bundle[TB_COLORMAP].texMods[i].tExp, 0);

				MatrixMultiplyScale(matrix, x, y, 0.0);
				break;
			}

			case TMOD_CENTERSCALE:
			{
				x = RB_EvalExpression(&pStage->bundle[TB_COLORMAP].texMods[i].sExp, 0);
				y = RB_EvalExpression(&pStage->bundle[TB_COLORMAP].texMods[i].tExp, 0);

				MatrixMultiplyTranslation(matrix, 0.5, 0.5, 0.0);
				MatrixMultiplyScale(matrix, x, y, 0.0);
				MatrixMultiplyTranslation(matrix, -0.5, -0.5, 0.0);
				break;
			}

			case TMOD_SHEAR:
			{
				x = RB_EvalExpression(&pStage->bundle[TB_COLORMAP].texMods[i].sExp, 0);
				y = RB_EvalExpression(&pStage->bundle[TB_COLORMAP].texMods[i].tExp, 0);

				MatrixMultiplyTranslation(matrix, 0.5, 0.5, 0.0);
				MatrixMultiplyShear(matrix, x, y);
				MatrixMultiplyTranslation(matrix, -0.5, -0.5, 0.0);
				break;
			}

			case TMOD_ROTATE2:
			{
				x = RAD2DEG(RB_EvalExpression(&pStage->bundle[TB_COLORMAP].texMods[i].rExp, 0)) * 5.0;

				MatrixMultiplyTranslation(matrix, 0.5, 0.5, 0.0);
				MatrixMultiplyZRotation(matrix, x);
				MatrixMultiplyTranslation(matrix, -0.5, -0.5, 0.0);
				break;
			}

			default:
				break;
		}
	}

	MatrixMultiply(matrix, light->attenuationMatrix, light->attenuationMatrix2);
}


void Tess_StageIteratorGeneric()
{
	int             stage;

	// log this call
	if(r_logFile->integer)
	{
		// don't just call LogComment, or we will get
		// a call to va() every frame!
		GLimp_LogComment(va("--- Tess_StageIteratorGeneric( %s, %i vertices, %i triangles ) ---\n", tess.surfaceShader->name, tess.numVertexes, tess.numIndexes / 3));
	}
	
	GL_CheckErrors();

	Tess_DeformGeometry();

	// set face culling appropriately
	GL_Cull(tess.surfaceShader->cullType);

	// set polygon offset if necessary
	if(tess.surfaceShader->polygonOffset)
	{
		qglEnable(GL_POLYGON_OFFSET_FILL);
		qglPolygonOffset(r_offsetFactor->value, r_offsetUnits->value);
	}

	// lock XYZ
	if(glConfig.vertexBufferObjectAvailable && tess.vertexesVBO)
	{
		qglVertexPointer(4, GL_FLOAT, 0, BUFFER_OFFSET(tess.ofsXYZ));
	}
	else
	{
		qglVertexPointer(4, GL_FLOAT, 0, tess.xyz);
	}

	if(qglLockArraysEXT)
	{
		qglLockArraysEXT(0, tess.numVertexes);
		GLimp_LogComment("glLockArraysEXT\n");
	}

	// call shader function
	for(stage = 0; stage < MAX_SHADER_STAGES; stage++)
	{
		shaderStage_t  *pStage = tess.surfaceStages[stage];

		if(!pStage)
		{
			break;
		}

		if(!RB_EvalExpression(&pStage->ifExp, 1.0))
		{
			continue;
		}

		if(glConfig.vertexBufferObjectAvailable && tess.vertexesVBO)
		{
			ComputeColor(pStage);
			ComputeTexMatrices(pStage);
		}
		else
		{
			ComputeTexCoords(pStage);
		}

		switch (pStage->type)
		{
			case ST_DIFFUSEMAP:
			{
				if(r_dynamicLighting->integer == 2)
				{
					if(glConfig.shadingLanguage100Available)
					{
						Render_depthFill(stage);
					}
					else
					{
						Render_depthFill_FFP(stage);
					}
				}
				else if(tess.lightmapNum >= 0 && tess.lightmapNum < tr.numLightmaps)
				{
					if(r_showLightMaps->integer)
					{
						Render_lightmap_FFP(stage, TB_LIGHTMAP);
					}
					else if(tr.worldDeluxeMapping && r_showDeluxeMaps->integer)
					{
						Render_deluxemap_FFP(stage, TB_LIGHTMAP);
					}
					else
					{
						if(glConfig.shadingLanguage100Available)
						{
							Render_lighting_D_radiosity(stage);
						}
						else
						{
							Render_lighting_D_radiosity_FFP(stage);
						}
					}
				}
				else
				{
					if(glConfig.shadingLanguage100Available)
					{
						Render_genericSingle(stage);
					}
					else
					{
						Render_genericSingle_FFP(stage);
					}
				}
				break;
			}

			case ST_COLLAPSE_lighting_DB_generic:
			{
				if(r_dynamicLighting->integer == 2)
				{
					if(glConfig.shadingLanguage100Available)
					{
						Render_depthFill(stage);
					}
					else
					{
						Render_depthFill_FFP(stage);
					}
				}
				else if(tess.lightmapNum >= 0 && tess.lightmapNum < tr.numLightmaps)
				{
					if(r_showLightMaps->integer)
					{
						Render_lightmap_FFP(stage, TB_LIGHTMAP);
					}
					else if(tr.worldDeluxeMapping && r_showDeluxeMaps->integer)
					{
						Render_deluxemap_FFP(stage, TB_LIGHTMAP);
					}
					else
					{
						if(glConfig.shadingLanguage100Available)
						{
							if(tr.worldDeluxeMapping && r_lighting->integer == 1)
							{
								Render_lighting_DB_radiosity(stage);
							}
							else
							{
								Render_lighting_D_radiosity(stage);
							}
						}
						else
						{
							Render_lighting_D_radiosity_FFP(stage);
						}
					}
				}
				else
				{
					if(glConfig.shadingLanguage100Available)
					{
						Render_genericSingle(stage);
					}
					else
					{
						Render_genericSingle_FFP(stage);
					}
				}
				break;
			}

			case ST_COLLAPSE_lighting_DBS_generic:
			{
				if(r_dynamicLighting->integer == 2)
				{
					if(glConfig.shadingLanguage100Available)
					{
						Render_depthFill(stage);
					}
					else
					{
						Render_depthFill_FFP(stage);
					}
				}
				else if(tess.lightmapNum >= 0 && tess.lightmapNum < tr.numLightmaps)
				{
					if(r_showLightMaps->integer)
					{
						Render_lightmap_FFP(stage, TB_LIGHTMAP);
					}
					else if(tr.worldDeluxeMapping && r_showDeluxeMaps->integer)
					{
						Render_deluxemap_FFP(stage, TB_LIGHTMAP);
					}
					else
					{
						if(glConfig.shadingLanguage100Available)
						{
							if(tr.worldDeluxeMapping && r_lighting->integer == 2)
							{
								Render_lighting_DBS_radiosity(stage);
							}
							else if(tr.worldDeluxeMapping && r_lighting->integer == 1)
							{
								Render_lighting_DB_radiosity(stage);
							}
							else
							{
								Render_lighting_D_radiosity(stage);
							}
						}
						else
						{
							Render_lighting_D_radiosity_FFP(stage);
						}
					}
				}
				else
				{
					if(glConfig.shadingLanguage100Available)
					{
						Render_genericSingle(stage);
					}
					else
					{
						Render_genericSingle_FFP(stage);
					}
				}
				break;
			}

			case ST_LIGHTMAP:
			{
				if(r_dynamicLighting->integer == 2)
				{
					if(glConfig.shadingLanguage100Available)
					{
						Render_depthFill(stage);
					}
					else
					{
						Render_depthFill_FFP(stage);
					}
				}
				else
				{
					Render_lightmap_FFP(stage, TB_COLORMAP);
				}
				break;
			}

				// skip these if not merge into diffuse stage
			case ST_NORMALMAP:
			case ST_SPECULARMAP:
				break;

			case ST_COLLAPSE_genericMulti:
			{
				Render_genericMulti_FFP(stage);
				break;
			}

			case ST_COLLAPSE_lighting_DB_direct:
			{
				if(r_dynamicLighting->integer == 2)
				{
					if(glConfig.shadingLanguage100Available)
					{
						Render_depthFill(stage);
					}
					else
					{
						Render_depthFill_FFP(stage);
					}
				}
				else
				{
					if(glConfig.shadingLanguage100Available)
					{
						if(r_lighting->integer == 1)
						{
							Render_lighting_DB_direct(stage);
						}
						else
						{
							Render_lighting_D_direct(stage);
						}
					}
					else
					{
						Render_genericSingle_FFP(stage);
					}
				}
				break;
			}

			case ST_COLLAPSE_lighting_DBS_direct:
			{
				if(r_dynamicLighting->integer == 2)
				{
					if(glConfig.shadingLanguage100Available)
					{
						Render_depthFill(stage);
					}
					else
					{
						Render_depthFill_FFP(stage);
					}
				}
				else
				{
					if(glConfig.shadingLanguage100Available)
					{
						if(r_lighting->integer == 2)
						{
							Render_lighting_DBS_direct(stage);
						}
						else if(r_lighting->integer == 1)
						{
							Render_lighting_DB_direct(stage);
						}
						else
						{
							Render_lighting_D_direct(stage);
						}
					}
					else
					{
						Render_genericSingle_FFP(stage);
					}
				}
				break;
			}

			case ST_COLLAPSE_reflection_CB:
			{
				if(glConfig.shadingLanguage100Available)
				{
					Render_reflection_CB(stage);
				}
				else
				{
					// TODO
				}
				break;
			}

			case ST_REFLECTIONMAP:
			{
				if(glConfig.shadingLanguage100Available)
				{
					Render_reflection_C(stage);
				}
				else
				{
					// TODO
				}
				break;
			}

			case ST_REFRACTIONMAP:
			{
				if(glConfig.shadingLanguage100Available)
				{
					Render_refraction_C(stage);
				}
				else
				{
					// TODO
				}
				break;
			}

			case ST_DISPERSIONMAP:
			{
				if(glConfig.shadingLanguage100Available)
				{
					Render_dispersion_C(stage);
				}
				else
				{
					// TODO
				}
				break;
			}

			case ST_SKYBOXMAP:
			{
				if(glConfig.shadingLanguage100Available)
				{
					Render_skybox(stage);
				}
				else
				{
					// TODO
				}
				break;
			}
			
			case ST_PORTALMAP:
			{
				if(glConfig.shadingLanguage100Available && glConfig.framebufferObjectAvailable)
				{
					Render_portal(stage);
				}
				else
				{
					// TODO
				}
				break;
			}

			case ST_HEATHAZEMAP:
			{
				if(glConfig.shadingLanguage100Available)
				{
					Render_heatHaze(stage);
					//Render_fboTest(stage);
				}
				else
				{
					// TODO
				}
				break;
			}

			case ST_BLOOMMAP:
			{
				if(glConfig.shadingLanguage100Available)
				{
					Render_bloom(stage);
				}
				else
				{
					// TODO
				}
				break;
			}

			case ST_BLOOM2MAP:
			{
				if(glConfig.shadingLanguage100Available)
				{
					Render_bloom2(stage);
				}
				else
				{
					// TODO
				}
				break;
			}

			case ST_ROTOSCOPEMAP:
			{
				if(glConfig.shadingLanguage100Available)
				{
					Render_rotoscope(stage);
				}
				else
				{
					// TODO
				}
				break;
			}

			default:
			case ST_COLORMAP:
			{
				if(glConfig.shadingLanguage100Available)
				{
					Render_genericSingle(stage);
				}
				else
				{
					Render_genericSingle_FFP(stage);
				}
				break;
			}
		}

		// allow skipping out to show just lightmaps during development
		if(r_showLightMaps->integer && (pStage->type == ST_LIGHTMAP))
		{
			break;
		}
	}

	if(tess.fogNum >= 1 && tess.surfaceShader->fogPass)
	{
		Render_fog();
	}

	// unlock arrays
	if(qglUnlockArraysEXT)
	{
		qglUnlockArraysEXT();
		GLimp_LogComment("glUnlockArraysEXT\n");
	}

	// reset polygon offset
	if(tess.surfaceShader->polygonOffset)
	{
		qglDisable(GL_POLYGON_OFFSET_FILL);
	}
}

void Tess_StageIteratorShadowFill()
{
	int             stage;

	// log this call
	if(r_logFile->integer)
	{
		// don't just call LogComment, or we will get
		// a call to va() every frame!
		GLimp_LogComment(va("--- Tess_StageIteratorShadowFill( %s, %i vertices, %i triangles ) ---\n", tess.surfaceShader->name, tess.numVertexes, tess.numIndexes / 3));
	}
	
	GL_CheckErrors();

	Tess_DeformGeometry();
	
	// set face culling appropriately	
	GL_Cull(tess.surfaceShader->cullType);
	
	// set polygon offset if necessary
	qglEnable(GL_POLYGON_OFFSET_FILL);
	qglPolygonOffset(r_shadowOffsetFactor->value, r_shadowOffsetUnits->value);

	// lock XYZ
	if(glConfig.vertexBufferObjectAvailable && tess.vertexesVBO)
	{
		qglVertexPointer(4, GL_FLOAT, 0, BUFFER_OFFSET(tess.ofsXYZ));
	}
	else
	{
		qglVertexPointer(4, GL_FLOAT, 0, tess.xyz);
	}

	if(qglLockArraysEXT)
	{
		qglLockArraysEXT(0, tess.numVertexes);
		GLimp_LogComment("glLockArraysEXT\n");
	}

	// call shader function
	for(stage = 0; stage < MAX_SHADER_STAGES; stage++)
	{
		shaderStage_t  *pStage = tess.surfaceStages[stage];

		if(!pStage)
		{
			break;
		}

		if(!RB_EvalExpression(&pStage->ifExp, 1.0))
		{
			continue;
		}

		if(glConfig.vertexBufferObjectAvailable && tess.vertexesVBO)
		{
			ComputeColor(pStage);
			ComputeTexMatrices(pStage);
		}
		else
		{
			ComputeTexCoords(pStage);
		}

		switch (pStage->type)
		{
			case ST_COLORMAP:
			{
				if(tess.surfaceShader->forceOpaque)
				{
					if(glConfig.shadingLanguage100Available)
					{
						Render_shadowFill(stage);
					}
					else
					{
						// TODO
					}
				}
				break;
			}
			
			case ST_DIFFUSEMAP:
			case ST_LIGHTMAP:
			case ST_COLLAPSE_lighting_DB_direct:
			case ST_COLLAPSE_lighting_DBS_direct:
			case ST_COLLAPSE_lighting_DB_generic:
			case ST_COLLAPSE_lighting_DBS_generic:
			{
				if(glConfig.shadingLanguage100Available)
				{
					Render_shadowFill(stage);
				}
				else
				{
					// TODO
				}
				break;
			}

			default:
				break;
		}
	}

	// unlock arrays
	if(qglUnlockArraysEXT)
	{
		qglUnlockArraysEXT();
		GLimp_LogComment("glUnlockArraysEXT\n");
	}
	
	// reset polygon offset
	qglDisable(GL_POLYGON_OFFSET_FILL);
}

void Tess_StageIteratorStencilShadowVolume()
{
	// log this call
	if(r_logFile->integer)
	{
		// don't just call LogComment, or we will get
		// a call to va() every frame!
		GLimp_LogComment(va("--- Tess_StageIteratorStencilShadowVolume( %s, %i vertices, %i triangles ) ---\n", tess.surfaceShader->name, tess.numVertexes, tess.numIndexes / 3));
	}
	
	GL_CheckErrors();

	// lock XYZ
	if(glConfig.vertexBufferObjectAvailable && tess.vertexesVBO)
	{
		qglVertexPointer(4, GL_FLOAT, 0, BUFFER_OFFSET(tess.ofsXYZ));
	}
	else
	{
		qglVertexPointer(4, GL_FLOAT, 0, tess.xyz);
	}

	if(qglLockArraysEXT)
	{
		qglLockArraysEXT(0, tess.numVertexes);
		GLimp_LogComment("glLockArraysEXT\n");
	}

	if(r_showShadowVolumes->integer)
	{
		//GL_State(GLS_DEPTHMASK_TRUE | GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);
		GL_State(GLS_DEPTHFUNC_LESS | GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);
		//GL_State(GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE);
		//GL_State(GLS_DEPTHMASK_TRUE);
#if 1
		GL_Cull(CT_FRONT_SIDED);
		//qglColor4f(1.0f, 1.0f, 0.7f, 0.05f);
		qglColor4f(1.0f, 0.0f, 0.0f, 0.05f);
		DrawElements();
#endif

#if 1
		GL_Cull(CT_BACK_SIDED);
		qglColor4f(0.0f, 1.0f, 0.0f, 0.05f);
		DrawElements();
#endif

#if 1
		GL_State(GLS_DEPTHFUNC_LESS | GLS_POLYMODE_LINE | GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);
		GL_Cull(CT_TWO_SIDED);
		qglColor4f(0.0f, 0.0f, 1.0f, 0.05f);
		DrawElements();
#endif
	}
	else
	{
		if(backEnd.currentEntity->needZFail)
		{
			// mirrors have the culling order reversed
			//if(backEnd.viewParms.isMirror)
			//  qglFrontFace(GL_CW);

			if(qglActiveStencilFaceEXT)
			{
				// render both sides at once
				GL_Cull(CT_TWO_SIDED);
				
				qglEnable(GL_STENCIL_TEST_TWO_SIDE_EXT);

				qglActiveStencilFaceEXT(GL_BACK);
				if(glConfig.stencilWrapAvailable)
				{
					qglStencilOp(GL_KEEP, GL_DECR_WRAP_EXT, GL_KEEP);
				}
				else
				{
					qglStencilOp(GL_KEEP, GL_DECR, GL_KEEP);
				}

				qglActiveStencilFaceEXT(GL_FRONT);
				if(glConfig.stencilWrapAvailable)
				{
					qglStencilOp(GL_KEEP, GL_INCR_WRAP_EXT, GL_KEEP);
				}
				else
				{
					qglStencilOp(GL_KEEP, GL_INCR, GL_KEEP);
				}

				DrawElements();
				
				qglDisable(GL_STENCIL_TEST_TWO_SIDE_EXT);
			}
			else
			{
				// draw only the front faces of the shadow volume
				GL_Cull(CT_FRONT_SIDED);

				// increment the stencil value on zfail
				if(glConfig.stencilWrapAvailable)
				{
					qglStencilOp(GL_KEEP, GL_INCR_WRAP_EXT, GL_KEEP);
				}
				else
				{
					qglStencilOp(GL_KEEP, GL_INCR, GL_KEEP);
				}

				DrawElements();

				// draw only the back faces of the shadow volume
				GL_Cull(CT_BACK_SIDED);

				// decrement the stencil value on zfail
				if(glConfig.stencilWrapAvailable)
				{
					qglStencilOp(GL_KEEP, GL_DECR_WRAP_EXT, GL_KEEP);
				}
				else
				{
					qglStencilOp(GL_KEEP, GL_DECR, GL_KEEP);
				}

				DrawElements();
			}

			//if(backEnd.viewParms.isMirror)
			//  qglFrontFace(GL_CCW);
		}
		else
		{
			// Tr3B - zpass rendering is cheaper because we can skip the lightcap and darkcap
			// see GPU Gems1 9.5.4

			// mirrors have the culling order reversed
			//if(backEnd.viewParms.isMirror)
			//  qglFrontFace(GL_CW);

			if(qglActiveStencilFaceEXT)
			{
				// render both sides at once
				GL_Cull(CT_TWO_SIDED);
				
				qglEnable(GL_STENCIL_TEST_TWO_SIDE_EXT);

				qglActiveStencilFaceEXT(GL_BACK);
				if(glConfig.stencilWrapAvailable)
				{
					qglStencilOp(GL_KEEP, GL_KEEP, GL_INCR_WRAP_EXT);
				}
				else
				{
					qglStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
				}

				qglActiveStencilFaceEXT(GL_FRONT);
				if(glConfig.stencilWrapAvailable)
				{
					qglStencilOp(GL_KEEP, GL_KEEP, GL_DECR_WRAP_EXT);
				}
				else
				{
					qglStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
				}

				DrawElements();
				
				qglDisable(GL_STENCIL_TEST_TWO_SIDE_EXT);
			}
			else
			{
				// draw only the back faces of the shadow volume
				GL_Cull(CT_BACK_SIDED);

				// increment the stencil value on zpass
				if(glConfig.stencilWrapAvailable)
				{
					qglStencilOp(GL_KEEP, GL_KEEP, GL_INCR_WRAP_EXT);
				}
				else
				{
					qglStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
				}

				DrawElements();

				// draw only the front faces of the shadow volume
				GL_Cull(CT_FRONT_SIDED);

				// decrement the stencil value on zpass
				if(glConfig.stencilWrapAvailable)
				{
					qglStencilOp(GL_KEEP, GL_KEEP, GL_DECR_WRAP_EXT);
				}
				else
				{
					qglStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
				}

				DrawElements();
			}

			//if(backEnd.viewParms.isMirror)
			//  qglFrontFace(GL_CCW);
		}
	}

	backEnd.pc.c_shadowBatches++;

	// unlock arrays
	if(qglUnlockArraysEXT)
	{
		qglUnlockArraysEXT();
		GLimp_LogComment("glUnlockArraysEXT\n");
	}
}

void Tess_StageIteratorStencilLighting()
{
	int             i, j;
	trRefLight_t   *light;
	shaderStage_t  *attenuationXYStage;
	shaderStage_t  *attenuationZStage;

	// log this call
	if(r_logFile->integer)
	{
		// don't just call LogComment, or we will get
		// a call to va() every frame!
		GLimp_LogComment(va("--- Tess_StageIteratorStencilLighting( %s, %s, %i vertices, %i triangles ) ---\n", tess.surfaceShader->name, tess.lightShader->name, tess.numVertexes, tess.numIndexes / 3));
	}
	
	GL_CheckErrors();

	light = backEnd.currentLight;

	Tess_DeformGeometry();
	
	// set OpenGL state for lighting
#if 0
	if(!light->additive)
	{
		GL_State(GLS_SRCBLEND_DST_COLOR | GLS_DSTBLEND_ONE | GLS_DEPTHFUNC_EQUAL | GLS_STENCILTEST_ENABLE);
	}
	else
#endif
	{
		GL_State(GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE | GLS_DEPTHFUNC_EQUAL | GLS_STENCILTEST_ENABLE);
	}

	// set face culling appropriately
	GL_Cull(tess.surfaceShader->cullType);

	// set polygon offset if necessary
	if(tess.surfaceShader->polygonOffset)
	{
		qglEnable(GL_POLYGON_OFFSET_FILL);
		qglPolygonOffset(r_offsetFactor->value, r_offsetUnits->value);
	}

	// lock XYZ
	if(glConfig.vertexBufferObjectAvailable && tess.vertexesVBO)
	{
		qglVertexPointer(4, GL_FLOAT, 0, BUFFER_OFFSET(tess.ofsXYZ));
	}
	else
	{
		qglVertexPointer(4, GL_FLOAT, 0, tess.xyz);
	}

	if(qglLockArraysEXT)
	{
		qglLockArraysEXT(0, tess.numVertexes);
		GLimp_LogComment("glLockArraysEXT\n");
	}

	// call shader function
	attenuationZStage = tess.lightShader->stages[0];

	for(i = 0; i < MAX_SHADER_STAGES; i++)
	{
		shaderStage_t  *diffuseStage = tess.surfaceStages[i];

		if(!diffuseStage)
		{
			break;
		}

		if(!RB_EvalExpression(&diffuseStage->ifExp, 1.0))
		{
			continue;
		}

		if(glConfig.vertexBufferObjectAvailable && tess.vertexesVBO)
		{
			ComputeTexMatrices(diffuseStage);
		}
		else
		{
			ComputeTexCoords(diffuseStage);
		}

		for(j = 1; j < MAX_SHADER_STAGES; j++)
		{
			attenuationXYStage = tess.lightShader->stages[j];

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

			ComputeColor(attenuationXYStage);
			ComputeFinalAttenuation(attenuationXYStage, light);

			switch (diffuseStage->type)
			{
				case ST_DIFFUSEMAP:
					if(glConfig.shadingLanguage100Available)
					{
						if(light->l.rlType == RL_OMNI)
						{
							Render_lighting_D_omni(diffuseStage, attenuationXYStage, attenuationZStage, light);
						}
						else if(light->l.rlType == RL_PROJ)
						{
							Render_lighting_D_proj(diffuseStage, attenuationXYStage, attenuationZStage, light);
						}
						else
						{
							// TODO
						}
					}
					else
					{
						// TODO
					}
					break;

				case ST_COLLAPSE_lighting_DB_direct:
				case ST_COLLAPSE_lighting_DB_generic:
					if(glConfig.shadingLanguage100Available)
					{
						if(r_lighting->integer == 1)
						{
							if(light->l.rlType == RL_OMNI)
							{
								Render_lighting_DB_omni(diffuseStage, attenuationXYStage, attenuationZStage, light);
							}
							else if(light->l.rlType == RL_PROJ)
							{
								Render_lighting_D_proj(diffuseStage, attenuationXYStage, attenuationZStage, light);
							}
							else
							{
								// TODO
							}
						}
						else
						{
							if(light->l.rlType == RL_OMNI)
							{
								Render_lighting_D_omni(diffuseStage, attenuationXYStage, attenuationZStage, light);
							}
							else if(light->l.rlType == RL_PROJ)
							{
								Render_lighting_D_proj(diffuseStage, attenuationXYStage, attenuationZStage, light);
							}
							else
							{
								// TODO
							}
						}
					}
					else
					{
						// TODO
					}
					break;

				case ST_COLLAPSE_lighting_DBS_direct:
				case ST_COLLAPSE_lighting_DBS_generic:
					if(glConfig.shadingLanguage100Available)
					{
						if(r_lighting->integer == 3)
						{
							if(light->l.rlType == RL_OMNI)
							{
								Render_lighting_DBSP_omni(diffuseStage, attenuationXYStage, attenuationZStage, light);
							}
							else if(light->l.rlType == RL_PROJ)
							{
								Render_lighting_D_proj(diffuseStage, attenuationXYStage, attenuationZStage, light);
							}
							else
							{
								// TODO
							}
						}
						else if(r_lighting->integer == 2)
						{
							if(light->l.rlType == RL_OMNI)
							{
								Render_lighting_DBS_omni(diffuseStage, attenuationXYStage, attenuationZStage, light);
							}
							else if(light->l.rlType == RL_PROJ)
							{
								Render_lighting_D_proj(diffuseStage, attenuationXYStage, attenuationZStage, light);
							}
							else
							{
								// TODO
							}
						}
						else if(r_lighting->integer == 1)
						{
							if(light->l.rlType == RL_OMNI)
							{
								Render_lighting_DB_omni(diffuseStage, attenuationXYStage, attenuationZStage, light);
							}
							else if(light->l.rlType == RL_PROJ)
							{
								Render_lighting_D_proj(diffuseStage, attenuationXYStage, attenuationZStage, light);
							}
							else
							{
								// TODO
							}
						}
						else
						{
							if(light->l.rlType == RL_OMNI)
							{
								Render_lighting_D_omni(diffuseStage, attenuationXYStage, attenuationZStage, light);
							}
							else if(light->l.rlType == RL_PROJ)
							{
								Render_lighting_D_proj(diffuseStage, attenuationXYStage, attenuationZStage, light);
							}
							else
							{
								// TODO
							}
						}
					}
					else
					{
						// TODO
					}
					break;

				default:
					break;
			}
		}
	}

	// unlock arrays
	if(qglUnlockArraysEXT)
	{
		qglUnlockArraysEXT();
		GLimp_LogComment("glUnlockArraysEXT\n");
	}

	// reset polygon offset
	if(tess.surfaceShader->polygonOffset)
	{
		qglDisable(GL_POLYGON_OFFSET_FILL);
	}
}

void Tess_StageIteratorLighting()
{
	int             i, j;
	trRefLight_t   *light;
	shaderStage_t  *attenuationXYStage;
	shaderStage_t  *attenuationZStage;

	// log this call
	if(r_logFile->integer)
	{
		// don't just call LogComment, or we will get
		// a call to va() every frame!
		GLimp_LogComment(va("--- Tess_StageIteratorLighting( %s, %s, %i vertices, %i triangles ) ---\n", tess.surfaceShader->name, tess.lightShader->name, tess.numVertexes, tess.numIndexes / 3));
	}
	
	GL_CheckErrors();

	light = backEnd.currentLight;

	Tess_DeformGeometry();
	
	// set OpenGL state for lighting
#if 0
	if(!light->additive)
	{
		GL_State(GLS_SRCBLEND_DST_COLOR | GLS_DSTBLEND_ONE | GLS_DEPTHFUNC_EQUAL);
	}
	else
#endif
	{
		GL_State(GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE | GLS_DEPTHFUNC_EQUAL);
	}

	// set face culling appropriately
	GL_Cull(tess.surfaceShader->cullType);

	// set polygon offset if necessary
	if(tess.surfaceShader->polygonOffset)
	{
		qglEnable(GL_POLYGON_OFFSET_FILL);
		qglPolygonOffset(r_offsetFactor->value, r_offsetUnits->value);
	}

	// lock XYZ
	if(glConfig.vertexBufferObjectAvailable && tess.vertexesVBO)
	{
		qglVertexPointer(4, GL_FLOAT, 0, BUFFER_OFFSET(tess.ofsXYZ));
	}
	else
	{
		qglVertexPointer(4, GL_FLOAT, 0, tess.xyz);
	}

	if(qglLockArraysEXT)
	{
		qglLockArraysEXT(0, tess.numVertexes);
		GLimp_LogComment("glLockArraysEXT\n");
	}

	// call shader function
	attenuationZStage = tess.lightShader->stages[0];

	for(i = 0; i < MAX_SHADER_STAGES; i++)
	{
		shaderStage_t  *diffuseStage = tess.surfaceStages[i];

		if(!diffuseStage)
		{
			break;
		}

		if(!RB_EvalExpression(&diffuseStage->ifExp, 1.0))
		{
			continue;
		}

		if(glConfig.vertexBufferObjectAvailable && tess.vertexesVBO)
		{
			ComputeTexMatrices(diffuseStage);
		}
		else
		{
			ComputeTexCoords(diffuseStage);
		}

		for(j = 1; j < MAX_SHADER_STAGES; j++)
		{
			attenuationXYStage = tess.lightShader->stages[j];

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

			ComputeColor(attenuationXYStage);
			ComputeFinalAttenuation(attenuationXYStage, light);

			switch (diffuseStage->type)
			{
				case ST_DIFFUSEMAP:
					if(glConfig.shadingLanguage100Available)
					{
						if(light->l.rlType == RL_OMNI)
						{
							Render_lighting_D_omni(diffuseStage, attenuationXYStage, attenuationZStage, light);
						}
						else if(light->l.rlType == RL_PROJ)
						{
							Render_lighting_D_proj(diffuseStage, attenuationXYStage, attenuationZStage, light);
						}
						else
						{
							// TODO
						}
					}
					else
					{
						// TODO
					}
					break;

				case ST_COLLAPSE_lighting_DB_direct:
				case ST_COLLAPSE_lighting_DB_generic:
					if(glConfig.shadingLanguage100Available)
					{
						if(r_lighting->integer == 1)
						{
							if(light->l.rlType == RL_OMNI)
							{
								Render_lighting_DB_omni(diffuseStage, attenuationXYStage, attenuationZStage, light);
							}
							else if(light->l.rlType == RL_PROJ)
							{
								Render_lighting_D_proj(diffuseStage, attenuationXYStage, attenuationZStage, light);
							}
							else
							{
								// TODO
							}
						}
						else
						{
							if(light->l.rlType == RL_OMNI)
							{
								Render_lighting_D_omni(diffuseStage, attenuationXYStage, attenuationZStage, light);
							}
							else if(light->l.rlType == RL_PROJ)
							{
								Render_lighting_D_proj(diffuseStage, attenuationXYStage, attenuationZStage, light);
							}
							else
							{
								// TODO
							}
						}
					}
					else
					{
						// TODO
					}
					break;

				case ST_COLLAPSE_lighting_DBS_direct:
				case ST_COLLAPSE_lighting_DBS_generic:
					if(glConfig.shadingLanguage100Available)
					{
						if(r_lighting->integer == 3)
						{
							if(light->l.rlType == RL_OMNI)
							{
								Render_lighting_DBSP_omni(diffuseStage, attenuationXYStage, attenuationZStage, light);
							}
							else if(light->l.rlType == RL_PROJ)
							{
								Render_lighting_D_proj(diffuseStage, attenuationXYStage, attenuationZStage, light);
							}
							else
							{
								// TODO
							}
						}
						else if(r_lighting->integer == 2)
						{
							if(light->l.rlType == RL_OMNI)
							{
								Render_lighting_DBS_omni(diffuseStage, attenuationXYStage, attenuationZStage, light);
							}
							else if(light->l.rlType == RL_PROJ)
							{
								Render_lighting_D_proj(diffuseStage, attenuationXYStage, attenuationZStage, light);
							}
							else
							{
								// TODO
							}
						}
						else if(r_lighting->integer == 1)
						{
							if(light->l.rlType == RL_OMNI)
							{
								Render_lighting_DB_omni(diffuseStage, attenuationXYStage, attenuationZStage, light);
							}
							else if(light->l.rlType == RL_PROJ)
							{
								Render_lighting_D_proj(diffuseStage, attenuationXYStage, attenuationZStage, light);
							}
							else
							{
								// TODO
							}
						}
						else
						{
							if(light->l.rlType == RL_OMNI)
							{
								Render_lighting_D_omni(diffuseStage, attenuationXYStage, attenuationZStage, light);
							}
							else if(light->l.rlType == RL_PROJ)
							{
								Render_lighting_D_proj(diffuseStage, attenuationXYStage, attenuationZStage, light);
							}
							else
							{
								// TODO
							}
						}
					}
					else
					{
						// TODO
					}
					break;

				default:
					break;
			}
		}
	}

	// unlock arrays
	if(qglUnlockArraysEXT)
	{
		qglUnlockArraysEXT();
		GLimp_LogComment("glUnlockArraysEXT\n");
	}

	// reset polygon offset
	if(tess.surfaceShader->polygonOffset)
	{
		qglDisable(GL_POLYGON_OFFSET_FILL);
	}
}

/*
=================
Tess_End

Render tesselated data
=================
*/
void Tess_End()
{
	if(tess.numIndexes == 0)
	{
		return;
	}

	if(tess.indexes[SHADER_MAX_INDEXES - 1] != 0)
	{
		ri.Error(ERR_DROP, "Tess_End() - SHADER_MAX_INDEXES hit");
	}
	if(tess.xyz[SHADER_MAX_VERTEXES - 1][0] != 0)
	{
		ri.Error(ERR_DROP, "Tess_End() - SHADER_MAX_VERTEXES hit");
	}

	// for debugging of sort order issues, stop rendering after a given sort value
	if(r_debugSort->integer && r_debugSort->integer < tess.surfaceShader->sort)
	{
		return;
	}

	// update performance counter
	backEnd.pc.c_batches++;

	// call off to shader specific tess end function
	tess.stageIteratorFunc();

	if(!tess.shadowVolume)
	{
		// draw debugging stuff
		if(r_showtris->integer)
		{
			DrawTris();
		}
		
		if(r_shownormals->integer)
		{
			DrawNormals();
		}
		
		if(r_showTangentSpaces->integer)
		{
			DrawTangentSpaces();
		}
	}

	// unbind VBO
	if(glConfig.vertexBufferObjectAvailable)
	{
		if(tess.indexesVBO)
		{
			qglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
			tess.indexesVBO = 0;

			backEnd.pc.c_vboIndexBuffers++;
		}

		if(tess.vertexesVBO)
		{
			qglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
			tess.vertexesVBO = 0;

			backEnd.pc.c_vboVertexBuffers++;
		}
	}

	// clear shader so we can tell we don't have any unclosed surfaces
	tess.numIndexes = 0;
	tess.numVertexes = 0;

	GLimp_LogComment("--- Tess_End ---\n");
}
