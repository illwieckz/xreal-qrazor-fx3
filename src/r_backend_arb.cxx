/// ============================================================================
/*
Copyright (C) 2004 Robert Beckebans <trebor_7@users.sourceforge.net>
Please see the file "AUTHORS" for a list of contributors

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
/// ============================================================================


/// includes ===================================================================
// system -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
#include "r_backend.h"


//
// backend ARB GPU shaders
//
static GLuint 			rb_shader_generic_vertex;			// color
static GLuint			rb_shader_generic_fragment;

static GLuint			rb_shader_zfill_vertex;
static GLuint			rb_shader_zfill_fragment;

static GLuint 			rb_shader_lighting_R_vertex;			// radiosity diffuse
static GLuint			rb_shader_lighting_R_fragment;

static GLuint 			rb_shader_lighting_RB_vertex;			// radiosity diffuse + bump
static GLuint			rb_shader_lighting_RB_fragment;

static GLuint 			rb_shader_lighting_RBH_vertex;			// radiosity diffuse + bump + parallax
static GLuint			rb_shader_lighting_RBH_fragment;

static GLuint 			rb_shader_lighting_RBHS_vertex;			// radiosity diffuse + bump + parallax + specular
static GLuint			rb_shader_lighting_RBHS_fragment;

static GLuint 			rb_shader_lighting_RBS_vertex;			// radiosity diffuse + bump + specular
static GLuint			rb_shader_lighting_RBS_fragment;

static GLuint 			rb_shader_lighting_D_XY_Z_CUBE_vertex;		// diffuse
static GLuint			rb_shader_lighting_D_XY_Z_CUBE_fragment;

static GLuint 			rb_shader_lighting_D_XY_Z_proj_vertex;		// diffuse
static GLuint			rb_shader_lighting_D_XY_Z_proj_fragment;

static GLuint			rb_shader_lighting_DB_XY_Z_CUBE_vertex;		// diffuse + bump
static GLuint			rb_shader_lighting_DB_XY_Z_CUBE_fragment;

static GLuint			rb_shader_lighting_DBH_XY_Z_CUBE_vertex;	// diffuse + bump + parallax
static GLuint			rb_shader_lighting_DBH_XY_Z_CUBE_fragment;

static GLuint			rb_shader_lighting_DBHS_XY_Z_CUBE_vertex;	// diffuse + bump + parallax + specular
static GLuint			rb_shader_lighting_DBHS_XY_Z_CUBE_fragment;

static GLuint			rb_shader_lighting_DBS_XY_Z_CUBE_vertex;	// diffuse + bump + specular
static GLuint			rb_shader_lighting_DBS_XY_Z_CUBE_fragment;

static GLuint			rb_shader_reflection_C_vertex;			// color
static GLuint			rb_shader_reflection_C_fragment;

static GLuint			rb_shader_refraction_C_vertex;			// color
static GLuint			rb_shader_refraction_C_fragment;

static GLuint			rb_shader_dispersion_C_vertex;			// color
static GLuint			rb_shader_dispersion_C_fragment;



static void	RB_CheckShader(int size, const char *buffer)
{
	if(GL_INVALID_OPERATION == xglGetError())
	{
		GLint errpos;
		xglGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errpos);
		
		ri.Com_Printf("RB_CheckShader: error:\n");
		
		int bgn = errpos - 10;
		//bgn < 0 ? 0 : bgn;
		const char * c = (const char *)(buffer + bgn);
		for(int i = 0; i < 30; i++)
		{
			if(bgn+i >= int(size-1))
				break;
			
			ri.Com_Printf("%c", *c++);
		}
		ri.Com_Printf("\n");
		
		const GLubyte *errstr = xglGetString(GL_PROGRAM_ERROR_STRING_ARB);
		ri.Com_Error(ERR_DROP, "RB_CheckShader: check your GPU asm code: %s", errstr);
	}
}


static void	RB_LoadShader(const std::string &name, GLuint *id, GLenum target)	
{
	char *buffer = NULL;
	int size;

	ri.Com_Printf("RB_LoadShader: '%s'\n", name.c_str());

	size = ri.VFS_FLoad(name, (void**)&buffer);
	if(!buffer)
        {
		ri.Com_Error(ERR_DROP, "RB_LoadShader: couldn't load '%s'", name.c_str());
		return;
	}
	
	xglGenProgramsARB(1, id);	RB_CheckForError();
	
	xglBindProgramARB(target, *id);	RB_CheckForError();
	
	xglProgramStringARB(target, GL_PROGRAM_FORMAT_ASCII_ARB, size, buffer);
	
	RB_CheckShader(size, buffer);
	
	ri.VFS_FFree(buffer);
}



void	RB_CheckOpenGLExtensions()
{
	if(!strstr(gl_config.extensions_string, "GL_ARB_vertex_program"))
	{
		ri.Com_Error(ERR_FATAL, "RB_CheckOpenGLExtensions: GL_ARB_vertex_program needed by ARB backend");
	}
	
	if(!strstr(gl_config.extensions_string, "GL_ARB_fragment_program"))
	{
		ri.Com_Error(ERR_FATAL, "RB_CheckOpenGLExtensions: GL_ARB_fragment_program needed by ARB backend");
	}
	
	ri.Com_Printf("...using GL_ARB_vertex_program\n");
	ri.Com_Printf("...using GL_ARB_fragment_program\n");
		
	
	xglVertexAttribPointerARB = (void (GLAPIENTRY*) (GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid *)) XGL_GetSymbol("glVertexAttribPointerARB");
	xglEnableVertexAttribArrayARB = (void (GLAPIENTRY*) (GLuint)) XGL_GetSymbol("glEnableVertexAttribArrayARB");
	xglDisableVertexAttribArrayARB = (void (GLAPIENTRY*) (GLuint)) XGL_GetSymbol("glDisableVertexAttribArrayARB");
	xglProgramStringARB = (void (GLAPIENTRY*) (GLenum, GLenum, GLsizei, const GLvoid *)) XGL_GetSymbol("glProgramStringARB");
	xglBindProgramARB = (void (GLAPIENTRY*) (GLenum, GLuint)) XGL_GetSymbol("glBindProgramARB");
	xglDeleteProgramsARB = (void (GLAPIENTRY*) (GLsizei, const GLuint *)) XGL_GetSymbol("glDeleteProgramsARB");
	xglGenProgramsARB = (void (GLAPIENTRY*) (GLsizei, GLuint *)) XGL_GetSymbol("glGenProgramsARB");
	xglProgramEnvParameter4fARB = (void (GLAPIENTRY*) (GLenum, GLuint, GLfloat, GLfloat, GLfloat, GLfloat)) XGL_GetSymbol("glProgramEnvParameter4fARB");
	xglProgramEnvParameter4fvARB = (void (GLAPIENTRY*) (GLenum, GLuint, const GLfloat *)) XGL_GetSymbol("glProgramEnvParameter4fvARB");
	xglProgramLocalParameter4fARB = (void (GLAPIENTRY*) (GLenum, GLuint, GLfloat, GLfloat, GLfloat, GLfloat)) XGL_GetSymbol("glProgramLocalParameter4fARB");
	xglProgramLocalParameter4fvARB = (void (GLAPIENTRY*) (GLenum, GLuint, const GLfloat *)) XGL_GetSymbol("glProgramLocalParameter4fvARB");
	xglGetProgramEnvParameterfvARB = (void (GLAPIENTRY*) (GLenum, GLuint, GLfloat *)) XGL_GetSymbol("glGetProgramEnvParameterfvARB");
	xglGetProgramLocalParameterfvARB = (void (GLAPIENTRY*) (GLenum, GLuint, GLfloat *)) XGL_GetSymbol("glGetProgramLocalParameterfvARB");
	xglGetProgramivARB = (void (GLAPIENTRY*) (GLenum, GLenum, GLint *)) XGL_GetSymbol("glGetProgramivARB");
	xglGetProgramStringARB = (void (GLAPIENTRY*) (GLenum, GLenum, GLvoid *)) XGL_GetSymbol("glGetProgramStringARB");
	xglGetVertexAttribfvARB = (void (GLAPIENTRY*) (GLuint, GLenum, GLfloat *)) XGL_GetSymbol("glGetVertexAttribfvARB");
	xglGetVertexAttribPointervARB = (void (GLAPIENTRY*) (GLuint, GLenum, GLvoid* *)) XGL_GetSymbol("glGetVertexAttribPointervARB");
	xglIsProgramARB = (GLboolean (GLAPIENTRY*) (GLuint)) XGL_GetSymbol("glIsProgramARB");
	
	int	tmp;
	xglGetIntegerv(GL_MAX_TEXTURE_COORDS_ARB, &tmp);
	ri.Com_Printf("GL_MAX_TEXTURE_COORDS_ARB: %i\n", tmp);
	
	xglGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS_ARB, &tmp);
	ri.Com_Printf("GL_MAX_TEXTURE_IMAGE_UNITS_ARB: %i\n", tmp);
}



void	RB_InitGPUShaders()
{
	ri.Com_Printf("------- RB_InitGPUShaders (ARB) -------\n");
	
	RB_LoadShader("cg/generic_arbvp1.s", &rb_shader_generic_vertex, GL_VERTEX_PROGRAM_ARB);
	RB_LoadShader("cg/generic_arbfp1.s", &rb_shader_generic_fragment, GL_FRAGMENT_PROGRAM_ARB);
	
	RB_LoadShader("cg/zfill_arbvp1.s", &rb_shader_zfill_vertex, GL_VERTEX_PROGRAM_ARB);
	RB_LoadShader("cg/zfill_arbfp1.s", &rb_shader_zfill_fragment, GL_FRAGMENT_PROGRAM_ARB);
	
	RB_LoadShader("cg/lighting_R_arbvp1.s", &rb_shader_lighting_R_vertex, GL_VERTEX_PROGRAM_ARB);
	RB_LoadShader("cg/lighting_R_arbfp1.s", &rb_shader_lighting_R_fragment, GL_FRAGMENT_PROGRAM_ARB);
	
	RB_LoadShader("cg/lighting_RB_arbvp1.s", &rb_shader_lighting_RB_vertex, GL_VERTEX_PROGRAM_ARB);
	RB_LoadShader("cg/lighting_RB_arbfp1.s", &rb_shader_lighting_RB_fragment, GL_FRAGMENT_PROGRAM_ARB);
	
	RB_LoadShader("cg/lighting_RBH_arbvp1.s", &rb_shader_lighting_RBH_vertex, GL_VERTEX_PROGRAM_ARB);
	RB_LoadShader("cg/lighting_RBH_arbfp1.s", &rb_shader_lighting_RBH_fragment, GL_FRAGMENT_PROGRAM_ARB);
	
	RB_LoadShader("cg/lighting_RBHS_arbvp1.s", &rb_shader_lighting_RBHS_vertex, GL_VERTEX_PROGRAM_ARB);
	RB_LoadShader("cg/lighting_RBHS_arbfp1.s", &rb_shader_lighting_RBHS_fragment, GL_FRAGMENT_PROGRAM_ARB);
	
	RB_LoadShader("cg/lighting_RBS_arbvp1.s", &rb_shader_lighting_RBS_vertex, GL_VERTEX_PROGRAM_ARB);
	RB_LoadShader("cg/lighting_RBS_arbfp1.s", &rb_shader_lighting_RBS_fragment, GL_FRAGMENT_PROGRAM_ARB);
	
	RB_LoadShader("cg/lighting_D_XY_Z_CUBE_arbvp1.s", &rb_shader_lighting_D_XY_Z_CUBE_vertex, GL_VERTEX_PROGRAM_ARB);
	RB_LoadShader("cg/lighting_D_XY_Z_CUBE_arbfp1.s", &rb_shader_lighting_D_XY_Z_CUBE_fragment, GL_FRAGMENT_PROGRAM_ARB);
	
	RB_LoadShader("cg/lighting_D_XY_Z_proj_arbvp1.s", &rb_shader_lighting_D_XY_Z_proj_vertex, GL_VERTEX_PROGRAM_ARB);
	RB_LoadShader("cg/lighting_D_XY_Z_proj_arbfp1.s", &rb_shader_lighting_D_XY_Z_proj_fragment, GL_FRAGMENT_PROGRAM_ARB);
	
	RB_LoadShader("cg/lighting_DB_XY_Z_CUBE_arbvp1.s", &rb_shader_lighting_DB_XY_Z_CUBE_vertex, GL_VERTEX_PROGRAM_ARB);
	RB_LoadShader("cg/lighting_DB_XY_Z_CUBE_arbfp1.s", &rb_shader_lighting_DB_XY_Z_CUBE_fragment, GL_FRAGMENT_PROGRAM_ARB);
	
	RB_LoadShader("cg/lighting_DBH_XY_Z_CUBE_arbvp1.s", &rb_shader_lighting_DBH_XY_Z_CUBE_vertex, GL_VERTEX_PROGRAM_ARB);
	RB_LoadShader("cg/lighting_DBH_XY_Z_CUBE_arbfp1.s", &rb_shader_lighting_DBH_XY_Z_CUBE_fragment, GL_FRAGMENT_PROGRAM_ARB);
	
	RB_LoadShader("cg/lighting_DBHS_XY_Z_CUBE_arbvp1.s", &rb_shader_lighting_DBHS_XY_Z_CUBE_vertex, GL_VERTEX_PROGRAM_ARB);
	RB_LoadShader("cg/lighting_DBHS_XY_Z_CUBE_arbfp1.s", &rb_shader_lighting_DBHS_XY_Z_CUBE_fragment, GL_FRAGMENT_PROGRAM_ARB);
	
	RB_LoadShader("cg/lighting_DBS_XY_Z_CUBE_arbvp1.s", &rb_shader_lighting_DBS_XY_Z_CUBE_vertex, GL_VERTEX_PROGRAM_ARB);
	RB_LoadShader("cg/lighting_DBS_XY_Z_CUBE_arbfp1.s", &rb_shader_lighting_DBS_XY_Z_CUBE_fragment, GL_FRAGMENT_PROGRAM_ARB);
	
	RB_LoadShader("cg/reflection_C_arbvp1.s", &rb_shader_reflection_C_vertex, GL_VERTEX_PROGRAM_ARB);
	RB_LoadShader("cg/reflection_C_arbfp1.s", &rb_shader_reflection_C_fragment, GL_FRAGMENT_PROGRAM_ARB);
	
	RB_LoadShader("cg/refraction_C_arbvp1.s", &rb_shader_refraction_C_vertex, GL_VERTEX_PROGRAM_ARB);
	RB_LoadShader("cg/refraction_C_arbfp1.s", &rb_shader_refraction_C_fragment, GL_FRAGMENT_PROGRAM_ARB);
	
	RB_LoadShader("cg/dispersion_C_arbvp1.s", &rb_shader_dispersion_C_vertex, GL_VERTEX_PROGRAM_ARB);
	RB_LoadShader("cg/dispersion_C_arbfp1.s", &rb_shader_dispersion_C_fragment, GL_FRAGMENT_PROGRAM_ARB);
}


void	RB_ShutdownGPUShaders()
{
	xglDeleteProgramsARB(1, &rb_shader_generic_vertex);
	xglDeleteProgramsARB(1, &rb_shader_generic_fragment);
	
	xglDeleteProgramsARB(1, &rb_shader_zfill_vertex);
	xglDeleteProgramsARB(1, &rb_shader_zfill_fragment);
	
	xglDeleteProgramsARB(1, &rb_shader_lighting_R_vertex);
	xglDeleteProgramsARB(1, &rb_shader_lighting_R_fragment);
	
	xglDeleteProgramsARB(1, &rb_shader_lighting_RB_vertex);
	xglDeleteProgramsARB(1, &rb_shader_lighting_RB_fragment);
	
	xglDeleteProgramsARB(1, &rb_shader_lighting_RBH_vertex);
	xglDeleteProgramsARB(1, &rb_shader_lighting_RBH_fragment);
	
	xglDeleteProgramsARB(1, &rb_shader_lighting_RBHS_vertex);
	xglDeleteProgramsARB(1, &rb_shader_lighting_RBHS_fragment);
	
	xglDeleteProgramsARB(1, &rb_shader_lighting_RBS_vertex);
	xglDeleteProgramsARB(1, &rb_shader_lighting_RBS_fragment);	
	
	xglDeleteProgramsARB(1, &rb_shader_lighting_D_XY_Z_CUBE_vertex);
	xglDeleteProgramsARB(1, &rb_shader_lighting_D_XY_Z_CUBE_fragment);
	
	xglDeleteProgramsARB(1, &rb_shader_lighting_D_XY_Z_proj_vertex);
	xglDeleteProgramsARB(1, &rb_shader_lighting_D_XY_Z_proj_fragment);
	
	xglDeleteProgramsARB(1, &rb_shader_lighting_DB_XY_Z_CUBE_vertex);
	xglDeleteProgramsARB(1, &rb_shader_lighting_DB_XY_Z_CUBE_fragment);
	
	xglDeleteProgramsARB(1, &rb_shader_lighting_DBH_XY_Z_CUBE_vertex);
	xglDeleteProgramsARB(1, &rb_shader_lighting_DBH_XY_Z_CUBE_fragment);
	
	xglDeleteProgramsARB(1, &rb_shader_lighting_DBHS_XY_Z_CUBE_vertex);
	xglDeleteProgramsARB(1, &rb_shader_lighting_DBHS_XY_Z_CUBE_fragment);
	
	xglDeleteProgramsARB(1, &rb_shader_lighting_DBS_XY_Z_CUBE_vertex);
	xglDeleteProgramsARB(1, &rb_shader_lighting_DBS_XY_Z_CUBE_fragment);
	
	xglDeleteProgramsARB(1, &rb_shader_reflection_C_vertex);
	xglDeleteProgramsARB(1, &rb_shader_reflection_C_fragment);
				
	xglDeleteProgramsARB(1, &rb_shader_refraction_C_vertex);
	xglDeleteProgramsARB(1, &rb_shader_refraction_C_fragment);
	
	xglDeleteProgramsARB(1, &rb_shader_dispersion_C_vertex);
	xglDeleteProgramsARB(1, &rb_shader_dispersion_C_fragment);
}


void	RB_EnableShader_generic()
{
	xglEnable(GL_VERTEX_PROGRAM_ARB);					RB_CheckForError();
	xglBindProgramARB(GL_VERTEX_PROGRAM_ARB, rb_shader_generic_vertex);	RB_CheckForError();
	
	xglEnable(GL_FRAGMENT_PROGRAM_ARB);					RB_CheckForError();
	xglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, rb_shader_generic_fragment);	RB_CheckForError();
	
	xglEnableVertexAttribArrayARB(0);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(8);	RB_CheckForError();
}

void	RB_DisableShader_generic()
{
	xglDisable(GL_VERTEX_PROGRAM_ARB);	RB_CheckForError();
	xglDisable(GL_FRAGMENT_PROGRAM_ARB);	RB_CheckForError();	
	
	xglDisableVertexAttribArrayARB(0);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(8);	RB_CheckForError();
}

void	RB_RenderCommand_generic(const r_command_t *cmd, const r_shader_stage_c *stage)
{
	RB_SetShaderStageState(cmd->getEntity(), stage, RENDER_TYPE_GENERIC);		RB_CheckForError();
	
	if(gl_config.arb_vertex_buffer_object && cmd->getEntityMesh()->vbo_array_buffer)
	{
		xglBindBufferARB(GL_ARRAY_BUFFER_ARB, cmd->getEntityMesh()->vbo_array_buffer);				RB_CheckForError();
		
		xglVertexAttribPointerARB(0, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_vertexes_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(8, 2, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_texcoords_ofs));	RB_CheckForError();
	}
	else
	{
		if(gl_config.arb_vertex_buffer_object)
			xglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);		RB_CheckForError();
		
		xglVertexAttribPointerARB(0, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->vertexes[0]));		RB_CheckForError();
		xglVertexAttribPointerARB(8, 2, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->texcoords[0]));		RB_CheckForError();
	}
	
	RB_SelectTexture(GL_TEXTURE0);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage);	RB_CheckForError();
	RB_Bind(stage->image);
	
	// set color
	vec4_c color;
	RB_ModifyColor(cmd->getEntity()->getShared(), stage, color);
	xglProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 0, color[0], color[1], color[2], color[3]);	RB_CheckForError();

	RB_FlushMesh(cmd->getEntityMesh());
}


void	RB_EnableShader_zfill()
{
	xglEnable(GL_VERTEX_PROGRAM_ARB);					RB_CheckForError();
	xglBindProgramARB(GL_VERTEX_PROGRAM_ARB, rb_shader_zfill_vertex);	RB_CheckForError();
	
	xglEnable(GL_FRAGMENT_PROGRAM_ARB);					RB_CheckForError();
	xglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, rb_shader_zfill_fragment);	RB_CheckForError();
		
	xglEnableVertexAttribArrayARB(0);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(8);	RB_CheckForError();
}

void	RB_DisableShader_zfill()
{
	xglDisable(GL_VERTEX_PROGRAM_ARB);	RB_CheckForError();
	xglDisable(GL_FRAGMENT_PROGRAM_ARB);	RB_CheckForError();	
	
	xglDisableVertexAttribArrayARB(0);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(8);	RB_CheckForError();
}

void	RB_RenderCommand_zfill(const r_command_t *cmd,	const r_shader_stage_c *stage)
{
	RB_SetShaderStageDepthState(stage);
	RB_SetShaderStageState(cmd->getEntity(), stage, RENDER_TYPE_ZFILL);
	
	if(gl_config.arb_vertex_buffer_object && cmd->getEntityMesh()->vbo_array_buffer)
	{
		xglBindBufferARB(GL_ARRAY_BUFFER_ARB, cmd->getEntityMesh()->vbo_array_buffer);				RB_CheckForError();
		
		xglVertexAttribPointerARB(0, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_vertexes_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(8, 2, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_texcoords_ofs));	RB_CheckForError();
	}
	else
	{
		if(gl_config.arb_vertex_buffer_object)
			xglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);		RB_CheckForError();
		
		xglVertexAttribPointerARB(0, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->vertexes[0]));	RB_CheckForError();
		xglVertexAttribPointerARB(8, 2, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->texcoords[0]));	RB_CheckForError();
	}

	RB_SelectTexture(GL_TEXTURE0);
	
	if(stage->flags & SHADER_STAGE_ALPHATEST)
	{
		RB_ModifyTextureMatrix(cmd->getEntity(), stage);
		RB_Bind(stage->image);
	}
	else
	{
		xglMatrixMode(GL_TEXTURE);
		xglLoadIdentity();
		xglMatrixMode(GL_MODELVIEW);

		RB_Bind(r_img_white);
	}
	
	RB_FlushMesh(cmd->getEntityMesh());
}



/*
================================================================================
				STATIC LIGHTING
================================================================================
*/

void	RB_EnableShader_lighting_R()
{
	xglEnable(GL_VERTEX_PROGRAM_ARB);						RB_CheckForError();
	xglBindProgramARB(GL_VERTEX_PROGRAM_ARB, rb_shader_lighting_R_vertex);		RB_CheckForError();
	
	xglEnable(GL_FRAGMENT_PROGRAM_ARB);						RB_CheckForError();
	xglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, rb_shader_lighting_R_fragment);	RB_CheckForError();
		
	xglEnableVertexAttribArrayARB(0);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(8);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(9);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(2);	RB_CheckForError();
}

void	RB_DisableShader_lighting_R()
{
	xglDisable(GL_VERTEX_PROGRAM_ARB);	RB_CheckForError();
	xglDisable(GL_FRAGMENT_PROGRAM_ARB);	RB_CheckForError();	
	
	xglDisableVertexAttribArrayARB(0);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(8);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(9);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(2);	RB_CheckForError();
}

void	RB_RenderCommand_lighting_R(const r_command_t *cmd,	const r_shader_stage_c *stage_diffusemap,
								const r_shader_stage_c *stage_lightmap,
								const r_shader_stage_c *stage_deluxemap)
{	
	RB_SetShaderStageState(cmd->getEntity(), stage_diffusemap, RENDER_TYPE_LIGHTING_R);
	
	if(gl_config.arb_vertex_buffer_object && cmd->getEntityMesh()->vbo_array_buffer)
	{
		xglBindBufferARB(GL_ARRAY_BUFFER_ARB, cmd->getEntityMesh()->vbo_array_buffer);				RB_CheckForError();
		
		xglVertexAttribPointerARB(0, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_vertexes_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(8, 2, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_texcoords_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(9, 2, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_texcoords_lm_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(2, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_normals_ofs));	RB_CheckForError();
	}
	else
	{
		if(gl_config.arb_vertex_buffer_object)
			xglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);		RB_CheckForError();
		
		xglVertexAttribPointerARB(0, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->vertexes[0]));		RB_CheckForError();
		xglVertexAttribPointerARB(8, 2, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->texcoords[0]));		RB_CheckForError();
		xglVertexAttribPointerARB(9, 2, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->texcoords_lm[0]));	RB_CheckForError();
		xglVertexAttribPointerARB(2, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->normals[0]));		RB_CheckForError();
	}

	RB_SelectTexture(GL_TEXTURE0);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_diffusemap);
	RB_Bind(stage_diffusemap->image);
	
	RB_SelectTexture(GL_TEXTURE1);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_lightmap);
	RB_Bind(R_GetLightMapImageByNum(cmd->getInfoKey()));
	
	RB_SelectTexture(GL_TEXTURE2);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_deluxemap);
	RB_Bind(R_GetLightMapImageByNum(cmd->getInfoKey() + 1));
	
	RB_SelectTexture(GL_TEXTURE3);
	xglMatrixMode(GL_TEXTURE);
	xglLoadTransposeMatrixf(&rb_matrix_model[0][0]);
	xglMatrixMode(GL_MODELVIEW);
	
	RB_FlushMesh(cmd->getEntityMesh());
}



void	RB_EnableShader_lighting_RB()
{
	xglEnable(GL_VERTEX_PROGRAM_ARB);						RB_CheckForError();
	xglBindProgramARB(GL_VERTEX_PROGRAM_ARB, rb_shader_lighting_RB_vertex);		RB_CheckForError();
	
	xglEnable(GL_FRAGMENT_PROGRAM_ARB);						RB_CheckForError();
	xglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, rb_shader_lighting_RB_fragment);	RB_CheckForError();
		
	xglEnableVertexAttribArrayARB(0);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(8);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(9);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(14);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(15);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(2);	RB_CheckForError();
}

void	RB_DisableShader_lighting_RB()
{
	xglDisable(GL_VERTEX_PROGRAM_ARB);	RB_CheckForError();
	xglDisable(GL_FRAGMENT_PROGRAM_ARB);	RB_CheckForError();	
	
	xglDisableVertexAttribArrayARB(0);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(8);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(9);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(14);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(15);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(2);	RB_CheckForError();
}

void	RB_RenderCommand_lighting_RB(const r_command_t *cmd,	const r_shader_stage_c *stage_diffusemap,
								const r_shader_stage_c *stage_bumpmap,
								const r_shader_stage_c *stage_lightmap,
								const r_shader_stage_c *stage_deluxemap)
{	
	RB_SetShaderStageState(cmd->getEntity(), stage_diffusemap, RENDER_TYPE_LIGHTING_RB);
	
	if(gl_config.arb_vertex_buffer_object && cmd->getEntityMesh()->vbo_array_buffer)
	{
		xglBindBufferARB(GL_ARRAY_BUFFER_ARB, cmd->getEntityMesh()->vbo_array_buffer);			RB_CheckForError();
		
		xglVertexAttribPointerARB(0, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_vertexes_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(8, 2, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_texcoords_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(9, 2, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_texcoords_lm_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(14, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_tangents_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(15, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_binormals_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(2, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_normals_ofs));	RB_CheckForError();
	}
	else
	{
		if(gl_config.arb_vertex_buffer_object)
			xglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);	RB_CheckForError();
		
		xglVertexAttribPointerARB(0, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->vertexes[0]));		RB_CheckForError();
		xglVertexAttribPointerARB(8, 2, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->texcoords[0]));		RB_CheckForError();
		xglVertexAttribPointerARB(9, 2, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->texcoords_lm[0]));	RB_CheckForError();
		xglVertexAttribPointerARB(14, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->tangents[0]));		RB_CheckForError();
		xglVertexAttribPointerARB(15, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->binormals[0]));	RB_CheckForError();
		xglVertexAttribPointerARB(2, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->normals[0]));		RB_CheckForError();
	}

	RB_SelectTexture(GL_TEXTURE0);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_diffusemap);
	RB_Bind(stage_diffusemap->image);
	
	RB_SelectTexture(GL_TEXTURE1);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_bumpmap);
	RB_Bind(stage_bumpmap->image);
	
	RB_SelectTexture(GL_TEXTURE2);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_lightmap);
	RB_Bind(R_GetLightMapImageByNum(cmd->getInfoKey()));
	
	RB_SelectTexture(GL_TEXTURE3);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_deluxemap);
	RB_Bind(R_GetLightMapImageByNum(cmd->getInfoKey() + 1));
	
	// set bump_scale
	float bump_scale = RB_Evaluate(cmd->getEntity()->getShared(), stage_bumpmap->bump_scale);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, bump_scale, 0, 0, 0);		RB_CheckForError();
	
	RB_FlushMesh(cmd->getEntityMesh());
}


void		RB_EnableShader_lighting_RBH()
{
	xglEnable(GL_VERTEX_PROGRAM_ARB);						RB_CheckForError();
	xglBindProgramARB(GL_VERTEX_PROGRAM_ARB, rb_shader_lighting_RBH_vertex);	RB_CheckForError();
	
	xglEnable(GL_FRAGMENT_PROGRAM_ARB);						RB_CheckForError();
	xglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, rb_shader_lighting_RBH_fragment);	RB_CheckForError();
		
	xglEnableVertexAttribArrayARB(0);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(8);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(9);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(14);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(15);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(2);	RB_CheckForError();
}

void		RB_DisableShader_lighting_RBH()
{
	xglDisable(GL_VERTEX_PROGRAM_ARB);	RB_CheckForError();
	xglDisable(GL_FRAGMENT_PROGRAM_ARB);	RB_CheckForError();	
	
	xglDisableVertexAttribArrayARB(0);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(8);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(9);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(14);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(15);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(2);	RB_CheckForError();
}

void		RB_RenderCommand_lighting_RBH(const r_command_t *cmd,		const r_shader_stage_c *stage_diffusemap,
										const r_shader_stage_c *stage_bumpmap,
										const r_shader_stage_c *stage_heightmap,
										const r_shader_stage_c *stage_lightmap,
										const r_shader_stage_c *stage_deluxemap)
{
	RB_SetShaderStageState(cmd->getEntity(), stage_diffusemap, RENDER_TYPE_LIGHTING_RBH);
	
	if(gl_config.arb_vertex_buffer_object && cmd->getEntityMesh()->vbo_array_buffer)
	{
		xglBindBufferARB(GL_ARRAY_BUFFER_ARB, cmd->getEntityMesh()->vbo_array_buffer);			RB_CheckForError();
		
		xglVertexAttribPointerARB(0, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_vertexes_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(8, 2, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_texcoords_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(9, 2, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_texcoords_lm_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(14, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_tangents_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(15, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_binormals_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(2, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_normals_ofs));	RB_CheckForError();
	}
	else
	{
		if(gl_config.arb_vertex_buffer_object)
			xglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);	RB_CheckForError();
		
		xglVertexAttribPointerARB(0, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->vertexes[0]));		RB_CheckForError();
		xglVertexAttribPointerARB(8, 2, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->texcoords[0]));		RB_CheckForError();
		xglVertexAttribPointerARB(9, 2, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->texcoords_lm[0]));	RB_CheckForError();
		xglVertexAttribPointerARB(14, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->tangents[0]));		RB_CheckForError();
		xglVertexAttribPointerARB(15, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->binormals[0]));	RB_CheckForError();
		xglVertexAttribPointerARB(2, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->normals[0]));		RB_CheckForError();
	}

	RB_SelectTexture(GL_TEXTURE0);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_diffusemap);
	RB_Bind(stage_diffusemap->image);
	
	RB_SelectTexture(GL_TEXTURE1);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_bumpmap);
	RB_Bind(stage_bumpmap->image);
	
	RB_SelectTexture(GL_TEXTURE2);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_heightmap);
	RB_Bind(stage_heightmap->image);
	
	RB_SelectTexture(GL_TEXTURE3);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_lightmap);
	RB_Bind(R_GetLightMapImageByNum(cmd->getInfoKey()));
	
	RB_SelectTexture(GL_TEXTURE4);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_deluxemap);
	RB_Bind(R_GetLightMapImageByNum(cmd->getInfoKey() + 1));
	
	// set view_origin in object space
	vec3_c view_origin = cmd->getEntity()->getTransform().affineInverse() * (r_origin - cmd->getEntity()->getShared().origin);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 2, view_origin[0], view_origin[1], view_origin[2], 0);	RB_CheckForError();
	
	// set bump_scale
	float bump_scale = RB_Evaluate(cmd->getEntity()->getShared(), stage_bumpmap->bump_scale);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 3, bump_scale, 0, 0, 0);		RB_CheckForError();
	
	// set height_scale
	float height_scale = RB_Evaluate(cmd->getEntity()->getShared(), stage_heightmap->height_scale);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, height_scale, 0, 0, 0);	RB_CheckForError();
	
	// set height_bias
	float height_bias = RB_Evaluate(cmd->getEntity()->getShared(), stage_heightmap->height_bias);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 1, height_bias, 0, 0, 0);	RB_CheckForError();
	
	RB_FlushMesh(cmd->getEntityMesh());
}

void		RB_EnableShader_lighting_RBHS()
{
	xglEnable(GL_VERTEX_PROGRAM_ARB);						RB_CheckForError();
	xglBindProgramARB(GL_VERTEX_PROGRAM_ARB, rb_shader_lighting_RBHS_vertex);	RB_CheckForError();
	
	xglEnable(GL_FRAGMENT_PROGRAM_ARB);						RB_CheckForError();
	xglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, rb_shader_lighting_RBHS_fragment);	RB_CheckForError();
		
	xglEnableVertexAttribArrayARB(0);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(8);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(9);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(14);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(15);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(2);	RB_CheckForError();
}

void		RB_DisableShader_lighting_RBHS()
{
	xglDisable(GL_VERTEX_PROGRAM_ARB);	RB_CheckForError();
	xglDisable(GL_FRAGMENT_PROGRAM_ARB);	RB_CheckForError();	
	
	xglDisableVertexAttribArrayARB(0);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(8);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(9);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(14);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(15);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(2);	RB_CheckForError();
}

void		RB_RenderCommand_lighting_RBHS(const r_command_t *cmd,		const r_shader_stage_c *stage_diffusemap,
										const r_shader_stage_c *stage_bumpmap,
										const r_shader_stage_c *stage_heightmap,
										const r_shader_stage_c *stage_specularmap,
										const r_shader_stage_c *stage_lightmap,
										const r_shader_stage_c *stage_deluxemap)
{
	RB_SetShaderStageState(cmd->getEntity(), stage_diffusemap, RENDER_TYPE_LIGHTING_RBHS);
	
	if(gl_config.arb_vertex_buffer_object && cmd->getEntityMesh()->vbo_array_buffer)
	{
		xglBindBufferARB(GL_ARRAY_BUFFER_ARB, cmd->getEntityMesh()->vbo_array_buffer);			RB_CheckForError();
		
		xglVertexAttribPointerARB(0, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_vertexes_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(8, 2, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_texcoords_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(9, 2, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_texcoords_lm_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(14, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_tangents_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(15, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_binormals_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(2, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_normals_ofs));	RB_CheckForError();
	}
	else
	{
		if(gl_config.arb_vertex_buffer_object)
			xglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);	RB_CheckForError();
		
		xglVertexAttribPointerARB(0, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->vertexes[0]));		RB_CheckForError();
		xglVertexAttribPointerARB(8, 2, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->texcoords[0]));		RB_CheckForError();
		xglVertexAttribPointerARB(9, 2, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->texcoords_lm[0]));	RB_CheckForError();
		xglVertexAttribPointerARB(14, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->tangents[0]));		RB_CheckForError();
		xglVertexAttribPointerARB(15, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->binormals[0]));	RB_CheckForError();
		xglVertexAttribPointerARB(2, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->normals[0]));		RB_CheckForError();
	}

	RB_SelectTexture(GL_TEXTURE0);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_diffusemap);
	RB_Bind(stage_diffusemap->image);
	
	RB_SelectTexture(GL_TEXTURE1);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_bumpmap);
	RB_Bind(stage_bumpmap->image);
	
	RB_SelectTexture(GL_TEXTURE2);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_heightmap);
	RB_Bind(stage_heightmap->image);
	
	RB_SelectTexture(GL_TEXTURE3);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_specularmap);
	RB_Bind(stage_specularmap->image);
	
	RB_SelectTexture(GL_TEXTURE4);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_lightmap);
	RB_Bind(R_GetLightMapImageByNum(cmd->getInfoKey()));
	
	RB_SelectTexture(GL_TEXTURE5);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_deluxemap);
	RB_Bind(R_GetLightMapImageByNum(cmd->getInfoKey() + 1));
	
	// set view_origin in object space
	vec3_c view_origin = cmd->getEntity()->getTransform().affineInverse() * (r_origin - cmd->getEntity()->getShared().origin);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 2, view_origin[0], view_origin[1], view_origin[2], 0);	RB_CheckForError();
	
	// set bump_scale
	float bump_scale = RB_Evaluate(cmd->getEntity()->getShared(), stage_bumpmap->bump_scale);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 4, bump_scale, 0, 0, 0);		RB_CheckForError();
	
	// set height_scale
	float height_scale = RB_Evaluate(cmd->getEntity()->getShared(), stage_heightmap->height_scale);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, height_scale, 0, 0, 0);	RB_CheckForError();
	
	// set height_bias
	float height_bias = RB_Evaluate(cmd->getEntity()->getShared(), stage_heightmap->height_bias);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 1, height_bias, 0, 0, 0);	RB_CheckForError();
	
	// set specular_exponent
	float specular_exponent = X_max(0, RB_Evaluate(cmd->getEntity()->getShared(), stage_specularmap->specular_exponent));
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 3, specular_exponent, 0, 0, 0);	RB_CheckForError();
	
	RB_FlushMesh(cmd->getEntityMesh());
}

void	RB_EnableShader_lighting_RBS()
{
	xglEnable(GL_VERTEX_PROGRAM_ARB);						RB_CheckForError();
	xglBindProgramARB(GL_VERTEX_PROGRAM_ARB, rb_shader_lighting_RBS_vertex);	RB_CheckForError();
	
	xglEnable(GL_FRAGMENT_PROGRAM_ARB);						RB_CheckForError();
	xglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, rb_shader_lighting_RBS_fragment);	RB_CheckForError();
		
	xglEnableVertexAttribArrayARB(0);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(8);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(9);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(14);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(15);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(2);	RB_CheckForError();
}

void	RB_DisableShader_lighting_RBS()
{
	xglDisable(GL_VERTEX_PROGRAM_ARB);	RB_CheckForError();
	xglDisable(GL_FRAGMENT_PROGRAM_ARB);	RB_CheckForError();
	
	xglDisableVertexAttribArrayARB(0);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(8);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(9);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(14);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(15);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(2);	RB_CheckForError();
}

void	RB_RenderCommand_lighting_RBS(const r_command_t *cmd,	const r_shader_stage_c *stage_diffusemap,
								const r_shader_stage_c *stage_bumpmap,
								const r_shader_stage_c *stage_specularmap,
								const r_shader_stage_c *stage_lightmap,
								const r_shader_stage_c *stage_deluxemap)
{	
	RB_SetShaderStageState(cmd->getEntity(), stage_diffusemap, RENDER_TYPE_LIGHTING_RBS);
	
	if(gl_config.arb_vertex_buffer_object && cmd->getEntityMesh()->vbo_array_buffer)
	{
		xglBindBufferARB(GL_ARRAY_BUFFER_ARB, cmd->getEntityMesh()->vbo_array_buffer);			RB_CheckForError();
		
		xglVertexAttribPointerARB(0, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_vertexes_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(8, 2, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_texcoords_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(9, 2, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_texcoords_lm_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(14, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_tangents_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(15, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_binormals_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(2, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_normals_ofs));	RB_CheckForError();
	}
	else
	{
		if(gl_config.arb_vertex_buffer_object)
			xglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);	RB_CheckForError();
		
		xglVertexAttribPointerARB(0, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->vertexes[0]));		RB_CheckForError();
		xglVertexAttribPointerARB(8, 2, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->texcoords[0]));		RB_CheckForError();
		xglVertexAttribPointerARB(9, 2, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->texcoords_lm[0]));	RB_CheckForError();
		xglVertexAttribPointerARB(14, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->tangents[0]));		RB_CheckForError();
		xglVertexAttribPointerARB(15, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->binormals[0]));	RB_CheckForError();
		xglVertexAttribPointerARB(2, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->normals[0]));		RB_CheckForError();
	}

	RB_SelectTexture(GL_TEXTURE0);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_diffusemap);
	RB_Bind(stage_diffusemap->image);
	
	RB_SelectTexture(GL_TEXTURE1);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_bumpmap);
	RB_Bind(stage_bumpmap->image);
	
	RB_SelectTexture(GL_TEXTURE2);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_specularmap);
	RB_Bind(stage_specularmap->image);
	
	RB_SelectTexture(GL_TEXTURE3);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_lightmap);
	RB_Bind(R_GetLightMapImageByNum(cmd->getInfoKey()));
	
	RB_SelectTexture(GL_TEXTURE4);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_deluxemap);
	RB_Bind(R_GetLightMapImageByNum(cmd->getInfoKey() + 1));
	
	// set view_origin in object space
	vec3_c view_origin = cmd->getEntity()->getTransform().affineInverse() * (r_origin - cmd->getEntity()->getShared().origin);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 2, view_origin[0], view_origin[1], view_origin[2], 0);	RB_CheckForError();
	
	// set bump_scale
	float bump_scale = RB_Evaluate(cmd->getEntity()->getShared(), stage_bumpmap->bump_scale);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 1, bump_scale, 0, 0, 0);		RB_CheckForError();
	
	// set specular_exponent
	float specular_exponent = X_max(0, RB_Evaluate(cmd->getEntity()->getShared(), stage_specularmap->specular_exponent));
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, specular_exponent, 0, 0, 0);	RB_CheckForError();
	
	RB_FlushMesh(cmd->getEntityMesh());
}



/*
================================================================================
				DYNAMIC LIGHTING
================================================================================
*/

void	RB_EnableShader_lighting_D_XY_Z_CUBE()
{
	xglEnable(GL_VERTEX_PROGRAM_ARB);							RB_CheckForError();
	xglBindProgramARB(GL_VERTEX_PROGRAM_ARB, rb_shader_lighting_D_XY_Z_CUBE_vertex);	RB_CheckForError();
	
	xglEnable(GL_FRAGMENT_PROGRAM_ARB);							RB_CheckForError();
	xglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, rb_shader_lighting_D_XY_Z_CUBE_fragment);	RB_CheckForError();
	
	xglEnableVertexAttribArrayARB(0);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(8);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(2);	RB_CheckForError();
}

void	RB_DisableShader_lighting_D_XY_Z_CUBE()
{
	xglDisable(GL_VERTEX_PROGRAM_ARB);		RB_CheckForError();
	xglDisable(GL_FRAGMENT_PROGRAM_ARB);		RB_CheckForError();
	
	xglDisableVertexAttribArrayARB(0);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(8);	RB_CheckForError();	
	xglDisableVertexAttribArrayARB(2);	RB_CheckForError();
}



void	RB_RenderCommand_lighting_D_XY_Z_CUBE(const r_command_t *cmd,	const r_shader_stage_c *stage_diffusemap,
									const r_shader_stage_c *stage_attenuationmap_xy,
									const r_shader_stage_c *stage_attenuationmap_z,
									const r_shader_stage_c *stage_attenuationmap_cube)
{
	RB_SetShaderStageState(cmd->getEntity(), stage_diffusemap, RENDER_TYPE_LIGHTING_D_omni);
			
	if(gl_config.arb_vertex_buffer_object && cmd->getEntityMesh()->vbo_array_buffer)
	{
		xglBindBufferARB(GL_ARRAY_BUFFER_ARB, cmd->getEntityMesh()->vbo_array_buffer);				RB_CheckForError();
		
		xglVertexAttribPointerARB(0, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_vertexes_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(8, 2, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_texcoords_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(2, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_normals_ofs));	RB_CheckForError();
	}
	else
	{
		if(gl_config.arb_vertex_buffer_object)
			xglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);	RB_CheckForError();
		
		xglVertexAttribPointerARB(0, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->vertexes[0]));		RB_CheckForError();
		xglVertexAttribPointerARB(8, 2, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->texcoords[0]));		RB_CheckForError();
		xglVertexAttribPointerARB(2, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->normals[0]));		RB_CheckForError();
	}
	
	RB_SelectTexture(GL_TEXTURE0);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_diffusemap);
	RB_Bind(stage_diffusemap->image);
	
	RB_SelectTexture(GL_TEXTURE1);
	RB_ModifyOmniLightTextureMatrix(cmd->getLight(), cmd->getEntity(), stage_attenuationmap_xy);
	RB_Bind(stage_attenuationmap_xy->image);
	
	RB_SelectTexture(GL_TEXTURE2);
	xglMatrixMode(GL_TEXTURE);
	xglLoadIdentity();
	xglMatrixMode(GL_MODELVIEW);
	RB_Bind(stage_attenuationmap_z->image);
	
	RB_SelectTexture(GL_TEXTURE3);
	RB_ModifyOmniLightCubeTextureMatrix(cmd->getLight(), cmd->getEntity(), stage_attenuationmap_cube);
	RB_Bind(stage_attenuationmap_cube->image);
	
	// set light_origin in object space
	vec3_c light_origin = cmd->getEntity()->getTransform().affineInverse() * (cmd->getLight()->getOrigin() - cmd->getEntity()->getShared().origin);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 1, light_origin[0], light_origin[1], light_origin[2], 1);	RB_CheckForError();
	
	// set light_color
	vec4_c light_color;
	RB_ModifyColor(cmd->getLight()->getShared(), stage_attenuationmap_xy, light_color);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, light_color[0], light_color[1], light_color[2], 0);	RB_CheckForError();
	
	RB_FlushMesh(cmd->getEntityMesh());
}


void	RB_EnableShader_lighting_D_XY_Z_proj()
{
	xglEnable(GL_VERTEX_PROGRAM_ARB);							RB_CheckForError();
	xglBindProgramARB(GL_VERTEX_PROGRAM_ARB, rb_shader_lighting_D_XY_Z_proj_vertex);	RB_CheckForError();
	
	xglEnable(GL_FRAGMENT_PROGRAM_ARB);							RB_CheckForError();
	xglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, rb_shader_lighting_D_XY_Z_proj_fragment);	RB_CheckForError();
	
	xglEnableVertexAttribArrayARB(0);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(8);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(2);	RB_CheckForError();
}

void	RB_DisableShader_lighting_D_XY_Z_proj()
{
	xglDisable(GL_VERTEX_PROGRAM_ARB);		RB_CheckForError();
	xglDisable(GL_FRAGMENT_PROGRAM_ARB);		RB_CheckForError();
	
	xglDisableVertexAttribArrayARB(0);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(8);	RB_CheckForError();	
	xglDisableVertexAttribArrayARB(2);	RB_CheckForError();
}



void	RB_RenderCommand_lighting_D_XY_Z_proj(const r_command_t *cmd,	const r_shader_stage_c *stage_diffusemap,
									const r_shader_stage_c *stage_attenuationmap_xy,
									const r_shader_stage_c *stage_attenuationmap_z)
{
	RB_SetShaderStageState(cmd->getEntity(), stage_diffusemap, RENDER_TYPE_LIGHTING_D_proj);
			
	if(gl_config.arb_vertex_buffer_object && cmd->getEntityMesh()->vbo_array_buffer)
	{
		xglBindBufferARB(GL_ARRAY_BUFFER_ARB, cmd->getEntityMesh()->vbo_array_buffer);				RB_CheckForError();
		
		xglVertexAttribPointerARB(0, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_vertexes_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(8, 2, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_texcoords_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(2, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_normals_ofs));	RB_CheckForError();
	}
	else
	{
		if(gl_config.arb_vertex_buffer_object)
			xglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);	RB_CheckForError();
		
		xglVertexAttribPointerARB(0, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->vertexes[0]));		RB_CheckForError();
		xglVertexAttribPointerARB(8, 2, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->texcoords[0]));		RB_CheckForError();
		xglVertexAttribPointerARB(2, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->normals[0]));		RB_CheckForError();
	}
	
	RB_SelectTexture(GL_TEXTURE0);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_diffusemap);
	RB_Bind(stage_diffusemap->image);
	
	RB_SelectTexture(GL_TEXTURE1);
	RB_ModifyProjLightTextureMatrix(cmd->getLight(), cmd->getEntity(), stage_attenuationmap_xy);
	RB_Bind(stage_attenuationmap_xy->image);
	
	RB_SelectTexture(GL_TEXTURE2);
	xglMatrixMode(GL_TEXTURE);
	xglLoadTransposeMatrixf(&rb_matrix_model[0][0]);
	xglMatrixMode(GL_MODELVIEW);
	RB_Bind(stage_attenuationmap_z->image);
	
	// set light_origin in world space
	const vec3_c& light_origin = cmd->getLight()->getOrigin();
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 1, light_origin[0], light_origin[1], light_origin[2], 1);	RB_CheckForError();
	
	// set light_color
	vec4_c light_color;
	RB_ModifyColor(cmd->getLight()->getShared(), stage_attenuationmap_xy, light_color);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, light_color[0], light_color[1], light_color[2], 0);	RB_CheckForError();
	
	RB_FlushMesh(cmd->getEntityMesh());
}

void	RB_EnableShader_lighting_DB_XY_Z_CUBE()
{
	xglEnable(GL_VERTEX_PROGRAM_ARB);							RB_CheckForError();
	xglBindProgramARB(GL_VERTEX_PROGRAM_ARB, rb_shader_lighting_DB_XY_Z_CUBE_vertex);	RB_CheckForError();
	
	xglEnable(GL_FRAGMENT_PROGRAM_ARB);							RB_CheckForError();
	xglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, rb_shader_lighting_DB_XY_Z_CUBE_fragment);	RB_CheckForError();
	
	xglEnableVertexAttribArrayARB(0);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(8);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(14);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(15);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(2);	RB_CheckForError();
}

void	RB_DisableShader_lighting_DB_XY_Z_CUBE()
{
	xglDisable(GL_VERTEX_PROGRAM_ARB);		RB_CheckForError();
	xglDisable(GL_FRAGMENT_PROGRAM_ARB);		RB_CheckForError();
	
	xglDisableVertexAttribArrayARB(0);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(8);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(14);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(15);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(2);	RB_CheckForError();
}

void	RB_RenderCommand_lighting_DB_XY_Z_CUBE(const r_command_t *cmd,	const r_shader_stage_c *stage_diffusemap,
									const r_shader_stage_c *stage_bumpmap,
									const r_shader_stage_c *stage_attenuationmap_xy,
									const r_shader_stage_c *stage_attenuationmap_z,
									const r_shader_stage_c *stage_attenuationmap_cube)
{
	RB_SetShaderStageState(cmd->getEntity(), stage_diffusemap, RENDER_TYPE_LIGHTING_DB_omni);
					
	if(gl_config.arb_vertex_buffer_object && cmd->getEntityMesh()->vbo_array_buffer)
	{
		xglBindBufferARB(GL_ARRAY_BUFFER_ARB, cmd->getEntityMesh()->vbo_array_buffer);			RB_CheckForError();
		
		xglVertexAttribPointerARB(0, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_vertexes_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(8, 2, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_texcoords_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(14, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_tangents_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(15, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_binormals_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(2, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_normals_ofs));	RB_CheckForError();
	}
	else
	{
		if(gl_config.arb_vertex_buffer_object)
			xglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);	RB_CheckForError();
		
		xglVertexAttribPointerARB(0, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->vertexes[0]));		RB_CheckForError();
		xglVertexAttribPointerARB(8, 2, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->texcoords[0]));		RB_CheckForError();
		xglVertexAttribPointerARB(14, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->tangents[0]));		RB_CheckForError();
		xglVertexAttribPointerARB(15, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->binormals[0]));	RB_CheckForError();
		xglVertexAttribPointerARB(2, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->normals[0]));		RB_CheckForError();
	}
	
	RB_SelectTexture(GL_TEXTURE0);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_diffusemap);
	RB_Bind(stage_diffusemap->image);
	
	RB_SelectTexture(GL_TEXTURE1);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_bumpmap);
	RB_Bind(stage_bumpmap->image);
	
	RB_SelectTexture(GL_TEXTURE2);
	RB_ModifyOmniLightTextureMatrix(cmd->getLight(), cmd->getEntity(), stage_attenuationmap_xy);
	RB_Bind(stage_attenuationmap_xy->image);
	
	RB_SelectTexture(GL_TEXTURE3);
	xglMatrixMode(GL_TEXTURE);
	xglLoadIdentity();
	xglMatrixMode(GL_MODELVIEW);
	RB_Bind(stage_attenuationmap_z->image);
	
	RB_SelectTexture(GL_TEXTURE4);
	RB_ModifyOmniLightCubeTextureMatrix(cmd->getLight(), cmd->getEntity(), stage_attenuationmap_cube);
	RB_Bind(stage_attenuationmap_cube->image);
	
	// set light_origin in object space
	vec3_c light_origin = cmd->getEntity()->getTransform().affineInverse() * (cmd->getLight()->getOrigin() - cmd->getEntity()->getShared().origin);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 2, light_origin[0], light_origin[1], light_origin[2], 1);	RB_CheckForError();
	
	// set light_color
	vec4_c light_color;
	RB_ModifyColor(cmd->getLight()->getShared(), stage_attenuationmap_xy, light_color);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, light_color[0], light_color[1], light_color[2], 0);	RB_CheckForError();
	
	// set bump_scale
	float bump_scale = RB_Evaluate(cmd->getEntity()->getShared(), stage_bumpmap->bump_scale);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 1, bump_scale, 0, 0, 0);		RB_CheckForError();

	RB_FlushMesh(cmd->getEntityMesh());
}


void	RB_EnableShader_lighting_DBH_XY_Z_CUBE()
{
	xglEnable(GL_VERTEX_PROGRAM_ARB);							RB_CheckForError();
	xglBindProgramARB(GL_VERTEX_PROGRAM_ARB, rb_shader_lighting_DBH_XY_Z_CUBE_vertex);	RB_CheckForError();
	
	xglEnable(GL_FRAGMENT_PROGRAM_ARB);							RB_CheckForError();
	xglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, rb_shader_lighting_DBH_XY_Z_CUBE_fragment);	RB_CheckForError();
	
	xglEnableVertexAttribArrayARB(0);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(8);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(14);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(15);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(2);	RB_CheckForError();
}

void	RB_DisableShader_lighting_DBH_XY_Z_CUBE()
{
	xglDisable(GL_VERTEX_PROGRAM_ARB);		RB_CheckForError();
	xglDisable(GL_FRAGMENT_PROGRAM_ARB);		RB_CheckForError();
	
	xglDisableVertexAttribArrayARB(0);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(8);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(14);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(15);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(2);	RB_CheckForError();
}

void	RB_RenderCommand_lighting_DBH_XY_Z_CUBE(const r_command_t *cmd,		const r_shader_stage_c *stage_diffusemap,
										const r_shader_stage_c *stage_bumpmap,
										const r_shader_stage_c *stage_heightmap,
										const r_shader_stage_c *stage_attenuationmap_xy,
										const r_shader_stage_c *stage_attenuationmap_z,
										const r_shader_stage_c *stage_attenuationmap_cube)
{
	RB_SetShaderStageState(cmd->getEntity(), stage_diffusemap, RENDER_TYPE_LIGHTING_DBH_omni);
					
	if(gl_config.arb_vertex_buffer_object && cmd->getEntityMesh()->vbo_array_buffer)
	{
		xglBindBufferARB(GL_ARRAY_BUFFER_ARB, cmd->getEntityMesh()->vbo_array_buffer);			RB_CheckForError();
		
		xglVertexAttribPointerARB(0, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_vertexes_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(8, 2, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_texcoords_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(14, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_tangents_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(15, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_binormals_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(2, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_normals_ofs));	RB_CheckForError();
	}
	else
	{
		if(gl_config.arb_vertex_buffer_object)
			xglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);	RB_CheckForError();
		
		xglVertexAttribPointerARB(0, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->vertexes[0]));		RB_CheckForError();
		xglVertexAttribPointerARB(8, 2, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->texcoords[0]));		RB_CheckForError();
		xglVertexAttribPointerARB(14, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->tangents[0]));		RB_CheckForError();
		xglVertexAttribPointerARB(15, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->binormals[0]));	RB_CheckForError();
		xglVertexAttribPointerARB(2, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->normals[0]));		RB_CheckForError();
	}
	
	RB_SelectTexture(GL_TEXTURE0);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_diffusemap);
	RB_Bind(stage_diffusemap->image);
	
	RB_SelectTexture(GL_TEXTURE1);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_bumpmap);
	RB_Bind(stage_bumpmap->image);
	
	RB_SelectTexture(GL_TEXTURE2);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_heightmap);
	RB_Bind(stage_heightmap->image);
	
	RB_SelectTexture(GL_TEXTURE3);
	RB_ModifyOmniLightTextureMatrix(cmd->getLight(), cmd->getEntity(), stage_attenuationmap_xy);
	RB_Bind(stage_attenuationmap_xy->image);
	
	RB_SelectTexture(GL_TEXTURE4);
	xglMatrixMode(GL_TEXTURE);
	xglLoadIdentity();
	xglMatrixMode(GL_MODELVIEW);
	RB_Bind(stage_attenuationmap_z->image);
	
	RB_SelectTexture(GL_TEXTURE5);
	RB_ModifyOmniLightCubeTextureMatrix(cmd->getLight(), cmd->getEntity(), stage_attenuationmap_cube);
	RB_Bind(stage_attenuationmap_cube->image);
	
	// set view_origin in object space
	vec3_c view_origin = cmd->getEntity()->getTransform().affineInverse() * (r_origin - cmd->getEntity()->getShared().origin);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 2, view_origin[0], view_origin[1], view_origin[2], 0);	RB_CheckForError();
	
	// set light_origin in object space
	vec3_c light_origin = cmd->getEntity()->getTransform().affineInverse() * (cmd->getLight()->getOrigin() - cmd->getEntity()->getShared().origin);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 5, light_origin[0], light_origin[1], light_origin[2], 1);	RB_CheckForError();
	
	// set light_color
	vec4_c light_color;
	RB_ModifyColor(cmd->getLight()->getShared(), stage_attenuationmap_xy, light_color);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 3, light_color[0], light_color[1], light_color[2], 0);	RB_CheckForError();
	
	// set bump_scale
	float bump_scale = RB_Evaluate(cmd->getEntity()->getShared(), stage_bumpmap->bump_scale);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 4, bump_scale, 0, 0, 0);		RB_CheckForError();
	
	// set height_scale
	float height_scale = RB_Evaluate(cmd->getEntity()->getShared(), stage_heightmap->height_scale);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, height_scale, 0, 0, 0);	RB_CheckForError();
	
	// set height_bias
	float height_bias = RB_Evaluate(cmd->getEntity()->getShared(), stage_heightmap->height_bias);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 1, height_bias, 0, 0, 0);	RB_CheckForError();

	RB_FlushMesh(cmd->getEntityMesh());
}


void	RB_EnableShader_lighting_DBHS_XY_Z_CUBE()
{
	xglEnable(GL_VERTEX_PROGRAM_ARB);							RB_CheckForError();
	xglBindProgramARB(GL_VERTEX_PROGRAM_ARB, rb_shader_lighting_DBHS_XY_Z_CUBE_vertex);	RB_CheckForError();
	
	xglEnable(GL_FRAGMENT_PROGRAM_ARB);							RB_CheckForError();
	xglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, rb_shader_lighting_DBHS_XY_Z_CUBE_fragment);	RB_CheckForError();
	
	xglEnableVertexAttribArrayARB(0);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(8);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(14);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(15);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(2);	RB_CheckForError();
}

void	RB_DisableShader_lighting_DBHS_XY_Z_CUBE()
{
	xglDisable(GL_VERTEX_PROGRAM_ARB);		RB_CheckForError();
	xglDisable(GL_FRAGMENT_PROGRAM_ARB);		RB_CheckForError();
	
	xglDisableVertexAttribArrayARB(0);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(8);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(14);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(15);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(2);	RB_CheckForError();
}

void	RB_RenderCommand_lighting_DBHS_XY_Z_CUBE(const r_command_t *cmd,	const r_shader_stage_c *stage_diffusemap,
										const r_shader_stage_c *stage_bumpmap,
										const r_shader_stage_c *stage_heightmap,
										const r_shader_stage_c *stage_specularmap,
										const r_shader_stage_c *stage_attenuationmap_xy,
										const r_shader_stage_c *stage_attenuationmap_z,
										const r_shader_stage_c *stage_attenuationmap_cube)
{
	RB_SetShaderStageState(cmd->getEntity(), stage_diffusemap, RENDER_TYPE_LIGHTING_DBHS_omni);
					
	if(gl_config.arb_vertex_buffer_object && cmd->getEntityMesh()->vbo_array_buffer)
	{
		xglBindBufferARB(GL_ARRAY_BUFFER_ARB, cmd->getEntityMesh()->vbo_array_buffer);			RB_CheckForError();
		
		xglVertexAttribPointerARB(0, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_vertexes_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(8, 2, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_texcoords_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(14, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_tangents_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(15, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_binormals_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(2, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_normals_ofs));	RB_CheckForError();
	}
	else
	{
		if(gl_config.arb_vertex_buffer_object)
			xglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);	RB_CheckForError();
		
		xglVertexAttribPointerARB(0, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->vertexes[0]));		RB_CheckForError();
		xglVertexAttribPointerARB(8, 2, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->texcoords[0]));		RB_CheckForError();
		xglVertexAttribPointerARB(14, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->tangents[0]));		RB_CheckForError();
		xglVertexAttribPointerARB(15, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->binormals[0]));	RB_CheckForError();
		xglVertexAttribPointerARB(2, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->normals[0]));		RB_CheckForError();
	}
	
	RB_SelectTexture(GL_TEXTURE0);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_diffusemap);
	RB_Bind(stage_diffusemap->image);
	
	RB_SelectTexture(GL_TEXTURE1);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_bumpmap);
	RB_Bind(stage_bumpmap->image);
	
	RB_SelectTexture(GL_TEXTURE2);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_heightmap);
	RB_Bind(stage_heightmap->image);
	
	RB_SelectTexture(GL_TEXTURE3);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_specularmap);
	RB_Bind(stage_specularmap->image);
	
	RB_SelectTexture(GL_TEXTURE4);
	RB_ModifyOmniLightTextureMatrix(cmd->getLight(), cmd->getEntity(), stage_attenuationmap_xy);
	RB_Bind(stage_attenuationmap_xy->image);
	
	RB_SelectTexture(GL_TEXTURE5);
	xglMatrixMode(GL_TEXTURE);
	xglLoadIdentity();
	xglMatrixMode(GL_MODELVIEW);
	RB_Bind(stage_attenuationmap_z->image);
	
	RB_SelectTexture(GL_TEXTURE6);
	RB_ModifyOmniLightCubeTextureMatrix(cmd->getLight(), cmd->getEntity(), stage_attenuationmap_cube);
	RB_Bind(stage_attenuationmap_cube->image);
	
	// set view_origin in object space
	vec3_c view_origin = cmd->getEntity()->getTransform().affineInverse() * (r_origin - cmd->getEntity()->getShared().origin);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 3, view_origin[0], view_origin[1], view_origin[2], 0);	RB_CheckForError();
	
	// set light_origin in object space
	vec3_c light_origin = cmd->getEntity()->getTransform().affineInverse() * (cmd->getLight()->getOrigin() - cmd->getEntity()->getShared().origin);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 6, light_origin[0], light_origin[1], light_origin[2], 1);	RB_CheckForError();
	
	// set light_color
	vec4_c light_color;
	RB_ModifyColor(cmd->getLight()->getShared(), stage_attenuationmap_xy, light_color);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, light_color[0], light_color[1], light_color[2], 0);	RB_CheckForError();
	
	// set bump_scale
	float bump_scale = RB_Evaluate(cmd->getEntity()->getShared(), stage_bumpmap->bump_scale);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 5, bump_scale, 0, 0, 0);		RB_CheckForError();
	
	// set height_scale
	float height_scale = RB_Evaluate(cmd->getEntity()->getShared(), stage_heightmap->height_scale);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 1, height_scale, 0, 0, 0);	RB_CheckForError();
	
	// set height_bias
	float height_bias = RB_Evaluate(cmd->getEntity()->getShared(), stage_heightmap->height_bias);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 2, height_bias, 0, 0, 0);	RB_CheckForError();
	
	// set specular_exponent
	float specular_exponent = X_max(0, RB_Evaluate(cmd->getEntity()->getShared(), stage_specularmap->specular_exponent));
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 4, specular_exponent, 0, 0, 0);	RB_CheckForError();

	RB_FlushMesh(cmd->getEntityMesh());
}


void	RB_EnableShader_lighting_DBS_XY_Z_CUBE()
{
	xglEnable(GL_VERTEX_PROGRAM_ARB);							RB_CheckForError();
	xglBindProgramARB(GL_VERTEX_PROGRAM_ARB, rb_shader_lighting_DBS_XY_Z_CUBE_vertex);	RB_CheckForError();
	
	xglEnable(GL_FRAGMENT_PROGRAM_ARB);							RB_CheckForError();
	xglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, rb_shader_lighting_DBS_XY_Z_CUBE_fragment);	RB_CheckForError();	
	
	xglEnableVertexAttribArrayARB(0);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(8);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(14);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(15);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(2);	RB_CheckForError();
}

void	RB_DisableShader_lighting_DBS_XY_Z_CUBE()
{
	xglDisable(GL_VERTEX_PROGRAM_ARB);		RB_CheckForError();
	xglDisable(GL_FRAGMENT_PROGRAM_ARB);		RB_CheckForError();	
	
	xglDisableVertexAttribArrayARB(0);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(8);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(14);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(15);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(2);	RB_CheckForError();
}

void	RB_RenderCommand_lighting_DBS_XY_Z_CUBE(const r_command_t *cmd,	const r_shader_stage_c *stage_diffusemap,
									const r_shader_stage_c *stage_bumpmap,
									const r_shader_stage_c *stage_specularmap,
									const r_shader_stage_c *stage_attenuationmap_xy,
									const r_shader_stage_c *stage_attenuationmap_z,
									const r_shader_stage_c *stage_attenuationmap_cube)
{
	RB_SetShaderStageState(cmd->getEntity(), stage_diffusemap, RENDER_TYPE_LIGHTING_DBS_omni);
					
	if(gl_config.arb_vertex_buffer_object && cmd->getEntityMesh()->vbo_array_buffer)
	{
		xglBindBufferARB(GL_ARRAY_BUFFER_ARB, cmd->getEntityMesh()->vbo_array_buffer);			RB_CheckForError();
		
		xglVertexAttribPointerARB(0, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_vertexes_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(8, 2, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_texcoords_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(14, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_tangents_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(15, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_binormals_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(2, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_normals_ofs));	RB_CheckForError();
	}
	else
	{
		if(gl_config.arb_vertex_buffer_object)
			xglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);	RB_CheckForError();
		
		xglVertexAttribPointerARB(0, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->vertexes[0]));		RB_CheckForError();
		xglVertexAttribPointerARB(8, 2, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->texcoords[0]));		RB_CheckForError();
		xglVertexAttribPointerARB(14, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->tangents[0]));		RB_CheckForError();
		xglVertexAttribPointerARB(15, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->binormals[0]));	RB_CheckForError();
		xglVertexAttribPointerARB(2, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->normals[0]));		RB_CheckForError();
	}

	RB_SelectTexture(GL_TEXTURE0);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_diffusemap);
	RB_Bind(stage_diffusemap->image);
	
	RB_SelectTexture(GL_TEXTURE1);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_bumpmap);
	RB_Bind(stage_bumpmap->image);
	
	RB_SelectTexture(GL_TEXTURE2);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_specularmap);
	RB_Bind(stage_specularmap->image);
	
	RB_SelectTexture(GL_TEXTURE3);
	RB_ModifyOmniLightTextureMatrix(cmd->getLight(), cmd->getEntity(), stage_attenuationmap_xy);
	RB_Bind(stage_attenuationmap_xy->image);
	
	RB_SelectTexture(GL_TEXTURE4);
	xglMatrixMode(GL_TEXTURE);
	xglLoadIdentity();
	xglMatrixMode(GL_MODELVIEW);
	RB_Bind(stage_attenuationmap_z->image);
	
	RB_SelectTexture(GL_TEXTURE5);
	RB_ModifyOmniLightCubeTextureMatrix(cmd->getLight(), cmd->getEntity(), stage_attenuationmap_cube);
	RB_Bind(stage_attenuationmap_cube->image);
	
	// set view_origin in object space
	vec3_c view_origin = cmd->getEntity()->getTransform().affineInverse() * (r_origin - cmd->getEntity()->getShared().origin);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 4, view_origin[0], view_origin[1], view_origin[2], 0);	RB_CheckForError();
	
	// set light_origin in object space
	vec3_c light_origin = cmd->getEntity()->getTransform().affineInverse() * (cmd->getLight()->getOrigin() - cmd->getEntity()->getShared().origin);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 3, light_origin[0], light_origin[1], light_origin[2], 1);	RB_CheckForError();
	
	// set light_color
	vec4_c light_color;
	RB_ModifyColor(cmd->getLight()->getShared(), stage_attenuationmap_xy, light_color);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, light_color[0], light_color[1], light_color[2], 0);	RB_CheckForError();
	
	// set bump_scale
	float bump_scale = RB_Evaluate(cmd->getEntity()->getShared(), stage_bumpmap->bump_scale);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 2, bump_scale, 0, 0, 0);		RB_CheckForError();
	
	// set specular_exponent
	float specular_exponent = X_max(0, RB_Evaluate(cmd->getEntity()->getShared(), stage_specularmap->specular_exponent));
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 1, specular_exponent, 0, 0, 0);	RB_CheckForError();

	RB_FlushMesh(cmd->getEntityMesh());
}



void	RB_EnableShader_reflection_C()
{
	xglEnable(GL_VERTEX_PROGRAM_ARB);						RB_CheckForError();
	xglBindProgramARB(GL_VERTEX_PROGRAM_ARB, rb_shader_reflection_C_vertex);	RB_CheckForError();
	
	xglEnable(GL_FRAGMENT_PROGRAM_ARB);						RB_CheckForError();
	xglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, rb_shader_reflection_C_fragment);	RB_CheckForError();
	
	xglEnableVertexAttribArrayARB(0);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(2);	RB_CheckForError();
}

void	RB_DisableShader_reflection_C()
{
	xglDisable(GL_VERTEX_PROGRAM_ARB);	RB_CheckForError();
	xglDisable(GL_FRAGMENT_PROGRAM_ARB);	RB_CheckForError();	
	
	xglDisableVertexAttribArrayARB(0);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(2);	RB_CheckForError();
}

void	RB_RenderCommand_reflection_C(const r_command_t *cmd,	const r_shader_stage_c *stage_colormap)
{
	RB_SetShaderStageState(cmd->getEntity(), stage_colormap, RENDER_TYPE_REFLECTION);
	
	if(gl_config.arb_vertex_buffer_object && cmd->getEntityMesh()->vbo_array_buffer)
	{
		xglBindBufferARB(GL_ARRAY_BUFFER_ARB, cmd->getEntityMesh()->vbo_array_buffer);				RB_CheckForError();
		
		xglVertexAttribPointerARB(0, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_vertexes_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(2, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_normals_ofs));	RB_CheckForError();
	}
	else
	{
		if(gl_config.arb_vertex_buffer_object)
			xglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);		RB_CheckForError();
		
		xglVertexAttribPointerARB(0, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->vertexes[0]));	RB_CheckForError();
		xglVertexAttribPointerARB(2, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->normals[0]));	RB_CheckForError();
	}

	RB_SelectTexture(GL_TEXTURE0);
	xglMatrixMode(GL_TEXTURE);
	xglLoadTransposeMatrixf(&rb_matrix_model[0][0]);
	xglMatrixMode(GL_MODELVIEW);
	RB_Bind(stage_colormap->image);
	
	// set view_origin in world space
	const vec3_c& view_origin = r_origin;
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, view_origin[0], view_origin[1], view_origin[2], 0);	RB_CheckForError();
	
	RB_FlushMesh(cmd->getEntityMesh());
}



void	RB_EnableShader_refraction_C()
{
	xglEnable(GL_VERTEX_PROGRAM_ARB);						RB_CheckForError();
	xglBindProgramARB(GL_VERTEX_PROGRAM_ARB, rb_shader_refraction_C_vertex);	RB_CheckForError();
	
	xglEnable(GL_FRAGMENT_PROGRAM_ARB);						RB_CheckForError();
	xglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, rb_shader_refraction_C_fragment);	RB_CheckForError();
		
	xglEnableVertexAttribArrayARB(0);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(2);	RB_CheckForError();
}

void	RB_DisableShader_refraction_C()
{
	xglDisable(GL_VERTEX_PROGRAM_ARB);	RB_CheckForError();
	xglDisable(GL_FRAGMENT_PROGRAM_ARB);	RB_CheckForError();
	
	xglDisableVertexAttribArrayARB(0);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(2);	RB_CheckForError();
}

void	RB_RenderCommand_refraction_C(const r_command_t *cmd,	const r_shader_stage_c *stage_colormap)
{
	RB_SetShaderStageState(cmd->getEntity(), stage_colormap, RENDER_TYPE_REFRACTION);
	
	if(gl_config.arb_vertex_buffer_object && cmd->getEntityMesh()->vbo_array_buffer)
	{
		xglBindBufferARB(GL_ARRAY_BUFFER_ARB, cmd->getEntityMesh()->vbo_array_buffer);				RB_CheckForError();
		
		xglVertexAttribPointerARB(0, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_vertexes_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(2, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_normals_ofs));	RB_CheckForError();
	}
	else
	{
		if(gl_config.arb_vertex_buffer_object)
			xglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);		RB_CheckForError();
		
		xglVertexAttribPointerARB(0, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->vertexes[0]));	RB_CheckForError();
		xglVertexAttribPointerARB(2, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->normals[0]));	RB_CheckForError();
	}

	RB_SelectTexture(GL_TEXTURE0);
	xglMatrixMode(GL_TEXTURE);
	xglLoadTransposeMatrixf(&rb_matrix_model[0][0]);
	xglMatrixMode(GL_MODELVIEW);
	RB_Bind(stage_colormap->image);
	
	// set view_origin in world space
	const vec3_c& view_origin = r_origin;
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 3, view_origin[0], view_origin[1], view_origin[2], 0);	RB_CheckForError();
	
	// set refraction_index
	float refraction_index = RB_Evaluate(cmd->getEntity()->getShared(), stage_colormap->refraction_index);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 4, refraction_index, 0, 0, 0);		RB_CheckForError();
	
	// set fresnel_power
	float fresnel_power = RB_Evaluate(cmd->getEntity()->getShared(), stage_colormap->fresnel_power);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 2, fresnel_power, 0, 0, 0);		RB_CheckForError();
	
	// set fresnel_scale
	float fresnel_scale = RB_Evaluate(cmd->getEntity()->getShared(), stage_colormap->fresnel_scale);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, fresnel_scale, 0, 0, 0);		RB_CheckForError();
	
	// set fresnel_bias
	float fresnel_bias = RB_Evaluate(cmd->getEntity()->getShared(), stage_colormap->fresnel_bias);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 1, fresnel_bias, 0, 0, 0);		RB_CheckForError();
	
	RB_FlushMesh(cmd->getEntityMesh());
}


void	RB_EnableShader_dispersion_C()
{
	xglEnable(GL_VERTEX_PROGRAM_ARB);						RB_CheckForError();
	xglBindProgramARB(GL_VERTEX_PROGRAM_ARB, rb_shader_dispersion_C_vertex);	RB_CheckForError();
	
	xglEnable(GL_FRAGMENT_PROGRAM_ARB);						RB_CheckForError();
	xglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, rb_shader_dispersion_C_fragment);	RB_CheckForError();
			
	xglEnableVertexAttribArrayARB(0);	RB_CheckForError();
	xglEnableVertexAttribArrayARB(2);	RB_CheckForError();
}

void	RB_DisableShader_dispersion_C()
{
	xglDisable(GL_VERTEX_PROGRAM_ARB);	RB_CheckForError();
	xglDisable(GL_FRAGMENT_PROGRAM_ARB);	RB_CheckForError();	
	
	xglDisableVertexAttribArrayARB(0);	RB_CheckForError();
	xglDisableVertexAttribArrayARB(2);	RB_CheckForError();
}

void	RB_RenderCommand_dispersion_C(const r_command_t *cmd,	const r_shader_stage_c *stage_colormap)
{
	RB_SetShaderStageState(cmd->getEntity(), stage_colormap, RENDER_TYPE_DISPERSION);
	
	if(gl_config.arb_vertex_buffer_object && cmd->getEntityMesh()->vbo_array_buffer)
	{
		xglBindBufferARB(GL_ARRAY_BUFFER_ARB, cmd->getEntityMesh()->vbo_array_buffer);				RB_CheckForError();
		
		xglVertexAttribPointerARB(0, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_vertexes_ofs));	RB_CheckForError();
		xglVertexAttribPointerARB(2, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_normals_ofs));	RB_CheckForError();
	}
	else
	{
		if(gl_config.arb_vertex_buffer_object)
			xglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);		RB_CheckForError();
		
		xglVertexAttribPointerARB(0, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->vertexes[0]));	RB_CheckForError();
		xglVertexAttribPointerARB(2, 3, GL_FLOAT, 0, 0, &(cmd->getEntityMesh()->normals[0]));	RB_CheckForError();
	}

	RB_SelectTexture(GL_TEXTURE0);
	xglMatrixMode(GL_TEXTURE);
	xglLoadTransposeMatrixf(&rb_matrix_model[0][0]);
	xglMatrixMode(GL_MODELVIEW);
	RB_Bind(stage_colormap->image);
	
	// set view_origin in world space
	const vec3_c& view_origin = r_origin;
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 3, view_origin[0], view_origin[1], view_origin[2], 0);	RB_CheckForError();
	
	// set eta_ratio
	float eta	= RB_Evaluate(cmd->getEntity()->getShared(), stage_colormap->eta);
	float eta_delta	= RB_Evaluate(cmd->getEntity()->getShared(), stage_colormap->eta_delta);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 4, eta, eta + eta_delta, eta + (eta_delta * 2), 0);	RB_CheckForError();
	
	// set fresnel_power
	float fresnel_power = RB_Evaluate(cmd->getEntity()->getShared(), stage_colormap->fresnel_power);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 2, fresnel_power, 0, 0, 0);		RB_CheckForError();
	
	// set fresnel_scale
	float fresnel_scale = RB_Evaluate(cmd->getEntity()->getShared(), stage_colormap->fresnel_scale);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, fresnel_scale, 0, 0, 0);		RB_CheckForError();
	
	// set fresnel_bias
	float fresnel_bias = RB_Evaluate(cmd->getEntity()->getShared(), stage_colormap->fresnel_bias);
	xglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 1, fresnel_bias, 0, 0, 0);		RB_CheckForError();
	
	RB_FlushMesh(cmd->getEntityMesh());
}


