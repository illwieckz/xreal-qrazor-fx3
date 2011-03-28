/*
===========================================================================
Copyright (C) 2010 Robert Beckebans <trebor_7@users.sourceforge.net>

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
// gl_shader.cpp -- GLSL shader handling

#include "gl_shader.h"

// *INDENT-OFF*

void		 GLSL_InitGPUShader3(shaderProgram_t * program,
								const char *vertexMainShader,
								const char *fragmentMainShader,
								const char *vertexLibShaders,
								const char *fragmentLibShaders,
								const char *compileMacros,
								int attribs,
								qboolean optimize);

void		GLSL_ValidateProgram(GLhandleARB program);
void		GLSL_ShowProgramUniforms(GLhandleARB program);



GLShader_generic* gl_genericShader = NULL;
GLShader_lightMapping* gl_lightMappingShader = NULL;
GLShader_vertexLighting_DBS_entity* gl_vertexLightingShader_DBS_entity = NULL;
GLShader_vertexLighting_DBS_world* gl_vertexLightingShader_DBS_world = NULL;
GLShader_forwardLighting* gl_forwardLightingShader = NULL;




const char* GLShader::GetCompileMacrosString(int permutation)
{
	static char compileMacros[4096];

	Com_Memset(compileMacros, 0, sizeof(compileMacros));

	for(size_t j = 0; j < _compileMacros.size(); j++)
	{
		GLCompileMacro* macro = _compileMacros[j];

		if(permutation & macro->GetBit())
		{
			Q_strcat(compileMacros, sizeof(compileMacros), macro->GetName());
			Q_strcat(compileMacros, sizeof(compileMacros), " ");
		}
	}

	return compileMacros;
}

void GLShader::UpdateShaderProgramUniformLocations(shaderProgram_t *shaderProgram) const
{
	for(size_t j = 0; j < _uniforms.size(); j++)
	{
		GLUniform* uniform = _uniforms[j];

		size_t ofs = uniform->Get_shaderProgram_t_Offset();

		*(GLint *)(((byte*)shaderProgram) + ofs) = glGetUniformLocationARB(shaderProgram->program, uniform->GetName());
	}
}




GLShader_generic::GLShader_generic():
		GLShader(	ATTR_POSITION | ATTR_TEXCOORD | ATTR_NORMAL,
					ATTR_COLOR | ATTR_POSITION2 | ATTR_NORMAL2,
					ATTR_TANGENT | ATTR_TANGENT2 | ATTR_BINORMAL | ATTR_BINORMAL2),
		u_ColorTextureMatrix(this),
		u_ViewOrigin(this),
		u_AlphaTest(this),
		u_ColorModulate(this),
		u_Color(this),
		u_ModelMatrix(this),
		u_ModelViewProjectionMatrix(this),
		u_BoneMatrix(this),
		u_VertexInterpolation(this),
		u_PortalPlane(this),
		GLDeformStage(this),
		GLCompileMacro_USE_PORTAL_CLIPPING(this),
		GLCompileMacro_USE_ALPHA_TESTING(this),
		GLCompileMacro_USE_VERTEX_SKINNING(this),
		GLCompileMacro_USE_VERTEX_ANIMATION(this),
		GLCompileMacro_USE_DEFORM_VERTEXES(this),
		GLCompileMacro_USE_TCGEN_ENVIRONMENT(this)
{
	_shaderPrograms = std::vector<shaderProgram_t>(1 << _compileMacros.size());
	
	//Com_Memset(_shaderPrograms, 0, sizeof(_shaderPrograms));

	size_t numPermutations = (1 << _compileMacros.size());	// same as 2^n, n = no. compile macros
	for(size_t i = 0; i < numPermutations; i++)
	{
		const char* compileMacros = GetCompileMacrosString(i);

		ri.Printf(PRINT_DEVELOPER, "Compile macros: '%s'\n", compileMacros);

		shaderProgram_t *shaderProgram = &_shaderPrograms[i];

		GLSL_InitGPUShader3(shaderProgram,
						"generic",
						"generic",
						"vertexAnimation deformVertexes",
						"",
						compileMacros,
						_vertexAttribsRequired | _vertexAttribsOptional,
						qtrue);

		UpdateShaderProgramUniformLocations(shaderProgram);

		shaderProgram->u_ColorMap = glGetUniformLocationARB(shaderProgram->program, "u_ColorMap");

		glUseProgramObjectARB(shaderProgram->program);
		glUniform1iARB(shaderProgram->u_ColorMap, 0);
		glUseProgramObjectARB(0);

		GLSL_ValidateProgram(shaderProgram->program);
		GLSL_ShowProgramUniforms(shaderProgram->program);
		GL_CheckErrors();
	}

	SelectProgram();
}











GLShader_lightMapping::GLShader_lightMapping():
		GLShader(	ATTR_POSITION | ATTR_TEXCOORD | ATTR_LIGHTCOORD | ATTR_NORMAL,
					0,
					ATTR_TANGENT | ATTR_TANGENT2 | ATTR_BINORMAL | ATTR_BINORMAL2),
		u_DiffuseTextureMatrix(this),
		u_AlphaTest(this),
		u_ModelViewProjectionMatrix(this),
		u_PortalPlane(this),
		GLDeformStage(this),
		GLCompileMacro_USE_PORTAL_CLIPPING(this),
		GLCompileMacro_USE_ALPHA_TESTING(this),
		GLCompileMacro_USE_DEFORM_VERTEXES(this)
{
	_shaderPrograms = std::vector<shaderProgram_t>(1 << _compileMacros.size());
	
	//Com_Memset(_shaderPrograms, 0, sizeof(_shaderPrograms));

	size_t numPermutations = (1 << _compileMacros.size());	// same as 2^n, n = no. compile macros
	for(size_t i = 0; i < numPermutations; i++)
	{
		const char* compileMacros = GetCompileMacrosString(i);

		ri.Printf(PRINT_DEVELOPER, "Compile macros: '%s'\n", compileMacros);

		shaderProgram_t *shaderProgram = &_shaderPrograms[i];

		GLSL_InitGPUShader3(shaderProgram,
						"lightMapping",
						"lightMapping",
						"deformVertexes",
						"",
						compileMacros,
						_vertexAttribsRequired | _vertexAttribsOptional,
						qtrue);

		UpdateShaderProgramUniformLocations(shaderProgram);

		shaderProgram->u_DiffuseMap = glGetUniformLocationARB(shaderProgram->program, "u_DiffuseMap");
		shaderProgram->u_LightMap = glGetUniformLocationARB(shaderProgram->program, "u_LightMap");

		glUseProgramObjectARB(shaderProgram->program);
		glUniform1iARB(shaderProgram->u_DiffuseMap, 0);
		glUniform1iARB(shaderProgram->u_LightMap, 1);
		glUseProgramObjectARB(0);

		GLSL_ValidateProgram(shaderProgram->program);
		GLSL_ShowProgramUniforms(shaderProgram->program);
		GL_CheckErrors();
	}

	SelectProgram();
}



GLShader_vertexLighting_DBS_entity::GLShader_vertexLighting_DBS_entity():
		GLShader(	ATTR_POSITION | ATTR_TEXCOORD | ATTR_TANGENT | ATTR_BINORMAL | ATTR_NORMAL,
					ATTR_POSITION2 | ATTR_TANGENT2 | ATTR_BINORMAL2 | ATTR_NORMAL2,
					0),
		u_DiffuseTextureMatrix(this),
		u_NormalTextureMatrix(this),
		u_SpecularTextureMatrix(this),
		u_AlphaTest(this),
		u_AmbientColor(this),
		u_ViewOrigin(this),
		u_LightDir(this),
		u_LightColor(this),
		u_ModelMatrix(this),
		u_ModelViewProjectionMatrix(this),
		u_BoneMatrix(this),
		u_VertexInterpolation(this),
		u_PortalPlane(this),
		u_DepthScale(this),
		GLDeformStage(this),
		GLCompileMacro_USE_PORTAL_CLIPPING(this),
		GLCompileMacro_USE_ALPHA_TESTING(this),
		GLCompileMacro_USE_VERTEX_SKINNING(this),
		GLCompileMacro_USE_VERTEX_ANIMATION(this),
		GLCompileMacro_USE_DEFORM_VERTEXES(this),
		GLCompileMacro_USE_PARALLAX_MAPPING(this)
{
	_shaderPrograms = std::vector<shaderProgram_t>(1 << _compileMacros.size());
	
	//Com_Memset(_shaderPrograms, 0, sizeof(_shaderPrograms));

	size_t numPermutations = (1 << _compileMacros.size());	// same as 2^n, n = no. compile macros
	for(size_t i = 0; i < numPermutations; i++)
	{
		const char* compileMacros = GetCompileMacrosString(i);

		ri.Printf(PRINT_DEVELOPER, "Compile macros: '%s'\n", compileMacros);

		shaderProgram_t *shaderProgram = &_shaderPrograms[i];

		GLSL_InitGPUShader3(shaderProgram,
						"vertexLighting_DBS_entity",
						"vertexLighting_DBS_entity",
						"vertexAnimation deformVertexes",
						"reliefMapping",
						compileMacros,
						_vertexAttribsRequired | _vertexAttribsOptional,
						qtrue);

		UpdateShaderProgramUniformLocations(shaderProgram);

		shaderProgram->u_DiffuseMap	= glGetUniformLocationARB(shaderProgram->program, "u_DiffuseMap");
		shaderProgram->u_NormalMap = glGetUniformLocationARB(shaderProgram->program, "u_NormalMap");
		shaderProgram->u_SpecularMap = glGetUniformLocationARB(shaderProgram->program, "u_SpecularMap");

		glUseProgramObjectARB(shaderProgram->program);
		glUniform1iARB(shaderProgram->u_DiffuseMap, 0);
		glUniform1iARB(shaderProgram->u_NormalMap, 1);
		glUniform1iARB(shaderProgram->u_SpecularMap, 2);
		glUseProgramObjectARB(0);

		GLSL_ValidateProgram(shaderProgram->program);
		GLSL_ShowProgramUniforms(shaderProgram->program);
		GL_CheckErrors();
	}

	SelectProgram();
}





GLShader_vertexLighting_DBS_world::GLShader_vertexLighting_DBS_world():
		GLShader(	ATTR_POSITION | ATTR_TEXCOORD | ATTR_TANGENT | ATTR_BINORMAL | ATTR_NORMAL | ATTR_COLOR | ATTR_LIGHTDIRECTION,
					0,
					ATTR_TANGENT | ATTR_TANGENT2 | ATTR_BINORMAL | ATTR_BINORMAL2),
		u_DiffuseTextureMatrix(this),
		u_NormalTextureMatrix(this),
		u_SpecularTextureMatrix(this),
		u_ColorModulate(this),
		u_Color(this),
		u_AlphaTest(this),
		u_ViewOrigin(this),
		u_ModelMatrix(this),
		u_ModelViewProjectionMatrix(this),
		u_PortalPlane(this),
		u_DepthScale(this),
		u_LightWrapAround(this),
		GLDeformStage(this),
		GLCompileMacro_USE_PORTAL_CLIPPING(this),
		GLCompileMacro_USE_ALPHA_TESTING(this),
		GLCompileMacro_USE_DEFORM_VERTEXES(this),
		GLCompileMacro_USE_PARALLAX_MAPPING(this)
{
	_shaderPrograms = std::vector<shaderProgram_t>(1 << _compileMacros.size());
	
	//Com_Memset(_shaderPrograms, 0, sizeof(_shaderPrograms));

	size_t numPermutations = (1 << _compileMacros.size());	// same as 2^n, n = no. compile macros
	for(size_t i = 0; i < numPermutations; i++)
	{
		const char* compileMacros = GetCompileMacrosString(i);

		ri.Printf(PRINT_DEVELOPER, "Compile macros: '%s'\n", compileMacros);

		shaderProgram_t *shaderProgram = &_shaderPrograms[i];

		GLSL_InitGPUShader3(shaderProgram,
						"vertexLighting_DBS_world",
						"vertexLighting_DBS_world",
						"vertexAnimation deformVertexes",
						"reliefMapping",
						compileMacros,
						_vertexAttribsRequired | _vertexAttribsOptional,
						qtrue);

		UpdateShaderProgramUniformLocations(shaderProgram);

		shaderProgram->u_DiffuseMap	= glGetUniformLocationARB(shaderProgram->program, "u_DiffuseMap");
		shaderProgram->u_NormalMap = glGetUniformLocationARB(shaderProgram->program, "u_NormalMap");
		shaderProgram->u_SpecularMap = glGetUniformLocationARB(shaderProgram->program, "u_SpecularMap");

		glUseProgramObjectARB(shaderProgram->program);
		glUniform1iARB(shaderProgram->u_DiffuseMap, 0);
		glUniform1iARB(shaderProgram->u_NormalMap, 1);
		glUniform1iARB(shaderProgram->u_SpecularMap, 2);
		glUseProgramObjectARB(0);

		GLSL_ValidateProgram(shaderProgram->program);
		GLSL_ShowProgramUniforms(shaderProgram->program);
		GL_CheckErrors();
	}

	SelectProgram();
}



GLShader_forwardLighting::GLShader_forwardLighting():
		GLShader(	ATTR_POSITION | ATTR_TEXCOORD | ATTR_TANGENT | ATTR_BINORMAL | ATTR_NORMAL,
					ATTR_POSITION2 | ATTR_TANGENT2 | ATTR_BINORMAL2 | ATTR_NORMAL2 | ATTR_COLOR,
					0),
		u_DiffuseTextureMatrix(this),
		u_NormalTextureMatrix(this),
		u_SpecularTextureMatrix(this),
		u_AlphaTest(this),
		u_ColorModulate(this),
		u_Color(this),
		u_ViewOrigin(this),
		u_LightOrigin(this),
		u_LightColor(this),
		u_LightRadius(this),
		u_LightScale(this),
		u_LightWrapAround(this),
		u_LightAttenuationMatrix(this),
		u_ShadowTexelSize(this),
		u_ShadowBlur(this),
		u_ModelMatrix(this),
		u_ModelViewProjectionMatrix(this),
		u_BoneMatrix(this),
		u_VertexInterpolation(this),
		u_PortalPlane(this),
		u_DepthScale(this),
		GLDeformStage(this),
		GLCompileMacro_USE_PORTAL_CLIPPING(this),
		GLCompileMacro_USE_ALPHA_TESTING(this),
		GLCompileMacro_USE_VERTEX_SKINNING(this),
		GLCompileMacro_USE_VERTEX_ANIMATION(this),
		GLCompileMacro_USE_DEFORM_VERTEXES(this),
		GLCompileMacro_USE_PARALLAX_MAPPING(this),
		GLCompileMacro_USE_SHADOWING(this)
{
	_shaderPrograms = std::vector<shaderProgram_t>(1 << _compileMacros.size());
	
	//Com_Memset(_shaderPrograms, 0, sizeof(_shaderPrograms));

	size_t numPermutations = (1 << _compileMacros.size());	// same as 2^n, n = no. compile macros
	for(size_t i = 0; i < numPermutations; i++)
	{
		const char* compileMacros = GetCompileMacrosString(i);

		ri.Printf(PRINT_DEVELOPER, "Compile macros: '%s'\n", compileMacros);

		shaderProgram_t *shaderProgram = &_shaderPrograms[i];

		GLSL_InitGPUShader3(shaderProgram,
						"forwardLighting",
						"forwardLighting",
						"vertexAnimation deformVertexes",
						"reliefMapping",
						compileMacros,
						_vertexAttribsRequired | _vertexAttribsOptional,
						qtrue);

		UpdateShaderProgramUniformLocations(shaderProgram);

		shaderProgram->u_DiffuseMap	= glGetUniformLocationARB(shaderProgram->program, "u_DiffuseMap");
		shaderProgram->u_NormalMap = glGetUniformLocationARB(shaderProgram->program, "u_NormalMap");
		shaderProgram->u_SpecularMap = glGetUniformLocationARB(shaderProgram->program, "u_SpecularMap");
		shaderProgram->u_AttenuationMapXY = glGetUniformLocationARB(shaderProgram->program, "u_AttenuationMapXY");
		shaderProgram->u_AttenuationMapZ = glGetUniformLocationARB(shaderProgram->program, "u_AttenuationMapZ");
		if(r_shadows->integer >= SHADOWING_VSM16)
		{
			shaderProgram->u_ShadowMap = glGetUniformLocationARB(shaderProgram->program, "u_ShadowMap");
		}

		glUseProgramObjectARB(shaderProgram->program);
		glUniform1iARB(shaderProgram->u_DiffuseMap, 0);
		glUniform1iARB(shaderProgram->u_NormalMap, 1);
		glUniform1iARB(shaderProgram->u_SpecularMap, 2);
		glUniform1iARB(shaderProgram->u_AttenuationMapXY, 3);
		glUniform1iARB(shaderProgram->u_AttenuationMapZ, 4);
		if(r_shadows->integer >= SHADOWING_VSM16)
		{
			glUniform1iARB(shaderProgram->u_ShadowMap, 5);
		}
		glUseProgramObjectARB(0);

		GLSL_ValidateProgram(shaderProgram->program);
		GLSL_ShowProgramUniforms(shaderProgram->program);
		GL_CheckErrors();
	}

	SelectProgram();
}
