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
// backend NV30 GPU shaders
//
static GLuint 			rb_shader_generic_vertex;
static GLuint			rb_shader_generic_fragment;

static GLuint			rb_shader_zfill_vertex;
static GLuint			rb_shader_zfill_fragment;

static GLuint 			rb_shader_lighting_R_vertex;
static GLuint			rb_shader_lighting_R_fragment;

static GLuint 			rb_shader_lighting_RB_vertex;
static GLuint			rb_shader_lighting_RB_fragment;

static GLuint 			rb_shader_lighting_RBH_vertex;
static GLuint			rb_shader_lighting_RBH_fragment;

static GLuint 			rb_shader_lighting_RBHS_vertex;
static GLuint			rb_shader_lighting_RBHS_fragment;

static GLuint 			rb_shader_lighting_RBS_vertex;
static GLuint			rb_shader_lighting_RBS_fragment;

static GLuint 			rb_shader_lighting_D_XY_Z_CUBE_vertex;
static GLuint			rb_shader_lighting_D_XY_Z_CUBE_fragment;

static GLuint 			rb_shader_lighting_DB_XY_Z_CUBE_vertex;
static GLuint			rb_shader_lighting_DB_XY_Z_CUBE_fragment;

static GLuint 			rb_shader_lighting_DBH_XY_Z_CUBE_vertex;
static GLuint			rb_shader_lighting_DBH_XY_Z_CUBE_fragment;

static GLuint 			rb_shader_lighting_DBHS_XY_Z_CUBE_vertex;
static GLuint			rb_shader_lighting_DBHS_XY_Z_CUBE_fragment;

static GLuint 			rb_shader_lighting_DBS_XY_Z_CUBE_vertex;
static GLuint			rb_shader_lighting_DBS_XY_Z_CUBE_fragment;

static GLuint			rb_shader_reflection_C_vertex;
static GLuint			rb_shader_reflection_C_fragment;

static GLuint			rb_shader_refraction_C_vertex;
static GLuint			rb_shader_refraction_C_fragment;

static GLuint			rb_shader_dispersion_C_vertex;
static GLuint			rb_shader_dispersion_C_fragment;



static void	RB_CheckShader(int size, const char *buffer)
{
	if(GL_INVALID_OPERATION == xglGetError())
	{
		GLint errpos;
		xglGetIntegerv(GL_PROGRAM_ERROR_POSITION_NV, &errpos);
		
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
		
		const GLubyte *errstr = xglGetString(GL_PROGRAM_ERROR_STRING_NV);
		ri.Com_Error(ERR_FATAL, "RB_CheckShader: check your NV30 GPU asm code: %s", errstr);
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
	
	xglGenProgramsNV(1, id);	RB_CheckForError();
	
	xglBindProgramNV(target, *id);	RB_CheckForError();
	
	xglLoadProgramNV(target, *id, size, (const GLubyte*)buffer);
	
	RB_CheckShader(size, buffer);
	
	ri.VFS_FFree(buffer);
}

void	RB_CheckOpenGLExtensions()
{
	if(!strstr(gl_config.extensions_string, "GL_NV_vertex_program2"))
	{
		ri.Com_Error(ERR_FATAL, "RB_CheckOpenGLExtensions: GL_NV_vertex_program2 needed by NV30 backend");
	}
	
	if(!strstr(gl_config.extensions_string, "GL_NV_fragment_program"))
	{
		ri.Com_Error(ERR_FATAL, "RB_CheckOpenGLExtensions: GL_NV_fragment_program needed by NV30 backend");
	}
	
	ri.Com_Printf("...using GL_NV_vertex_program2\n");
	ri.Com_Printf("...using GL_NV_fragment_program\n");
	
	
	xglAreProgramsResidentNV = (GLboolean (GLAPIENTRY*) (GLsizei, const GLuint *, GLboolean *)) XGL_GetSymbol("glAreProgramsResidentNV");
	xglBindProgramNV = (void (GLAPIENTRY*) (GLenum, GLuint)) XGL_GetSymbol("glBindProgramNV");
	xglDeleteProgramsNV = (void (GLAPIENTRY*) (GLsizei, const GLuint *)) XGL_GetSymbol("glDeleteProgramsNV");
	xglExecuteProgramNV = (void (GLAPIENTRY*) (GLenum, GLuint, const GLfloat *)) XGL_GetSymbol("glExecuteProgramNV");
	xglGenProgramsNV = (void (GLAPIENTRY*) (GLsizei, GLuint *)) XGL_GetSymbol("glGenProgramsNV");
	xglGetProgramParameterdvNV = (void (GLAPIENTRY*) (GLenum, GLuint, GLenum, GLdouble *)) XGL_GetSymbol("glGetProgramParameterdvNV");
	xglGetProgramParameterfvNV = (void (GLAPIENTRY*) (GLenum, GLuint, GLenum, GLfloat *)) XGL_GetSymbol("glGetProgramParameterfvNV");
	xglGetProgramivNV = (void (GLAPIENTRY*) (GLuint, GLenum, GLint *)) XGL_GetSymbol("glGetProgramivNV");
	xglGetProgramStringNV = (void (GLAPIENTRY*) (GLuint, GLenum, GLubyte *)) XGL_GetSymbol("glGetProgramStringNV");
	xglGetTrackMatrixivNV = (void (GLAPIENTRY*) (GLenum, GLuint, GLenum, GLint *)) XGL_GetSymbol("glGetTrackMatrixivNV");
	xglGetVertexAttribdvNV = (void (GLAPIENTRY*) (GLuint, GLenum, GLdouble *)) XGL_GetSymbol("glGetVertexAttribdvNV");
	xglGetVertexAttribfvNV = (void (GLAPIENTRY*) (GLuint, GLenum, GLfloat *)) XGL_GetSymbol("glGetVertexAttribfvNV");
	xglGetVertexAttribivNV = (void (GLAPIENTRY*) (GLuint, GLenum, GLint *)) XGL_GetSymbol("glGetVertexAttribivNV");
	xglGetVertexAttribPointervNV = (void (GLAPIENTRY*) (GLuint, GLenum, GLvoid* *)) XGL_GetSymbol("glGetVertexAttribPointervNV");
	xglIsProgramNV = (GLboolean (GLAPIENTRY*) (GLuint)) XGL_GetSymbol("glIsProgramNV");
	xglLoadProgramNV = (void (GLAPIENTRY*) (GLenum, GLuint, GLsizei, const GLubyte *)) XGL_GetSymbol("glLoadProgramNV");
	xglProgramParameter4fNV = (void (GLAPIENTRY*) (GLenum, GLuint, GLfloat, GLfloat, GLfloat, GLfloat)) XGL_GetSymbol("glProgramParameter4fNV");
	xglProgramParameter4fvNV = (void (GLAPIENTRY*) (GLenum, GLuint, const GLfloat *)) XGL_GetSymbol("glProgramParameter4fvNV");
	xglProgramParameters4fvNV = (void (GLAPIENTRY*) (GLenum, GLuint, GLuint, const GLfloat *)) XGL_GetSymbol("glProgramParameters4fvNV");
	xglRequestResidentProgramsNV = (void (GLAPIENTRY*) (GLsizei, const GLuint *)) XGL_GetSymbol("glRequestResidentProgramsNV");
	xglTrackMatrixNV = (void (GLAPIENTRY*) (GLenum, GLuint, GLenum, GLenum)) XGL_GetSymbol("glTrackMatrixNV");
	xglVertexAttribPointerNV = (void (GLAPIENTRY*) (GLuint, GLint, GLenum, GLsizei, const GLvoid *)) XGL_GetSymbol("glVertexAttribPointerNV");
	xglVertexAttrib1fNV = (void (GLAPIENTRY*) (GLuint, GLfloat)) XGL_GetSymbol("glVertexAttrib1fNV");
	xglVertexAttrib1fvNV = (void (GLAPIENTRY*) (GLuint, const GLfloat *)) XGL_GetSymbol("glVertexAttrib1fvNV");
	xglVertexAttrib2fNV = (void (GLAPIENTRY*) (GLuint, GLfloat, GLfloat)) XGL_GetSymbol("glVertexAttrib2fNV");
	xglVertexAttrib2fvNV = (void (GLAPIENTRY*) (GLuint, const GLfloat *)) XGL_GetSymbol("glVertexAttrib2fvNV");
	xglVertexAttrib3fNV = (void (GLAPIENTRY*) (GLuint, GLfloat, GLfloat, GLfloat)) XGL_GetSymbol("glVertexAttrib3fNV");
	xglVertexAttrib3fvNV = (void (GLAPIENTRY*) (GLuint, const GLfloat *)) XGL_GetSymbol("glVertexAttrib3fvNV");
	xglVertexAttrib4fNV = (void (GLAPIENTRY*) (GLuint, GLfloat, GLfloat, GLfloat, GLfloat)) XGL_GetSymbol("glVertexAttrib4fNV");
	xglVertexAttrib4fvNV = (void (GLAPIENTRY*) (GLuint, const GLfloat *)) XGL_GetSymbol("glVertexAttrib4fvNV");
	xglVertexAttribs1fvNV = (void (GLAPIENTRY*) (GLuint, GLsizei, const GLfloat *)) XGL_GetSymbol("glVertexAttribs1fvNV");
	xglVertexAttribs2fvNV = (void (GLAPIENTRY*) (GLuint, GLsizei, const GLfloat *)) XGL_GetSymbol("glVertexAttribs2fvNV");
	xglVertexAttribs3fvNV = (void (GLAPIENTRY*) (GLuint, GLsizei, const GLfloat *)) XGL_GetSymbol("glVertexAttribs3fvNV");
	xglVertexAttribs4fvNV = (void (GLAPIENTRY*) (GLuint, GLsizei, const GLfloat *)) XGL_GetSymbol("glVertexAttribs4fvNV");
	
	xglProgramNamedParameter4fNV = (void (GLAPIENTRY*) (GLuint, GLsizei, const GLubyte *, GLfloat, GLfloat, GLfloat, GLfloat)) XGL_GetSymbol("glProgramNamedParameter4fNV");
	xglProgramNamedParameter4fvNV = (void (GLAPIENTRY*) (GLuint, GLsizei, const GLubyte *, const GLfloat *)) XGL_GetSymbol("glProgramNamedParameter4fvNV");
	xglGetProgramNamedParameterfvNV = (void (GLAPIENTRY*) (GLuint, GLsizei, const GLubyte *, GLfloat *)) XGL_GetSymbol("glGetProgramNamedParameterfvNV");
}


void	RB_InitGPUShaders()
{
	ri.Com_Printf("------- RB_InitGPUShaders (NV30) -------\n");
	
	RB_LoadShader("cg/generic_vp30.s", &rb_shader_generic_vertex, GL_VERTEX_PROGRAM_NV);
	RB_LoadShader("cg/generic_fp30.s", &rb_shader_generic_fragment, GL_FRAGMENT_PROGRAM_NV);
	
	RB_LoadShader("cg/zfill_vp30.s", &rb_shader_zfill_vertex, GL_VERTEX_PROGRAM_NV);
	RB_LoadShader("cg/zfill_fp30.s", &rb_shader_zfill_fragment, GL_FRAGMENT_PROGRAM_NV);
	
	RB_LoadShader("cg/lighting_R_vp30.s", &rb_shader_lighting_R_vertex, GL_VERTEX_PROGRAM_NV);
	RB_LoadShader("cg/lighting_R_fp30.s", &rb_shader_lighting_R_fragment, GL_FRAGMENT_PROGRAM_NV);
	
	RB_LoadShader("cg/lighting_RB_vp30.s", &rb_shader_lighting_RB_vertex, GL_VERTEX_PROGRAM_NV);
	RB_LoadShader("cg/lighting_RB_fp30.s", &rb_shader_lighting_RB_fragment, GL_FRAGMENT_PROGRAM_NV);
	
	RB_LoadShader("cg/lighting_RBH_vp30.s", &rb_shader_lighting_RBH_vertex, GL_VERTEX_PROGRAM_NV);
	RB_LoadShader("cg/lighting_RBH_fp30.s", &rb_shader_lighting_RBH_fragment, GL_FRAGMENT_PROGRAM_NV);
	
	RB_LoadShader("cg/lighting_RBHS_vp30.s", &rb_shader_lighting_RBHS_vertex, GL_VERTEX_PROGRAM_NV);
	RB_LoadShader("cg/lighting_RBHS_fp30.s", &rb_shader_lighting_RBHS_fragment, GL_FRAGMENT_PROGRAM_NV);
	
	RB_LoadShader("cg/lighting_RBS_vp30.s", &rb_shader_lighting_RBS_vertex, GL_VERTEX_PROGRAM_NV);
	RB_LoadShader("cg/lighting_RBS_fp30.s", &rb_shader_lighting_RBS_fragment, GL_FRAGMENT_PROGRAM_NV);
	
	RB_LoadShader("cg/lighting_D_XY_Z_CUBE_vp30.s", &rb_shader_lighting_D_XY_Z_CUBE_vertex, GL_VERTEX_PROGRAM_NV);
	RB_LoadShader("cg/lighting_D_XY_Z_CUBE_fp30.s", &rb_shader_lighting_D_XY_Z_CUBE_fragment, GL_FRAGMENT_PROGRAM_NV);
	
	RB_LoadShader("cg/lighting_DB_XY_Z_CUBE_vp30.s", &rb_shader_lighting_DB_XY_Z_CUBE_vertex, GL_VERTEX_PROGRAM_NV);
	RB_LoadShader("cg/lighting_DB_XY_Z_CUBE_fp30.s", &rb_shader_lighting_DB_XY_Z_CUBE_fragment, GL_FRAGMENT_PROGRAM_NV);
	
	RB_LoadShader("cg/lighting_DBH_XY_Z_CUBE_vp30.s", &rb_shader_lighting_DBH_XY_Z_CUBE_vertex, GL_VERTEX_PROGRAM_NV);
	RB_LoadShader("cg/lighting_DBH_XY_Z_CUBE_fp30.s", &rb_shader_lighting_DBH_XY_Z_CUBE_fragment, GL_FRAGMENT_PROGRAM_NV);
	
	RB_LoadShader("cg/lighting_DBHS_XY_Z_CUBE_vp30.s", &rb_shader_lighting_DBHS_XY_Z_CUBE_vertex, GL_VERTEX_PROGRAM_NV);
	RB_LoadShader("cg/lighting_DBHS_XY_Z_CUBE_fp30.s", &rb_shader_lighting_DBHS_XY_Z_CUBE_fragment, GL_FRAGMENT_PROGRAM_NV);
	
	RB_LoadShader("cg/lighting_DBS_XY_Z_CUBE_vp30.s", &rb_shader_lighting_DBS_XY_Z_CUBE_vertex, GL_VERTEX_PROGRAM_NV);
	RB_LoadShader("cg/lighting_DBS_XY_Z_CUBE_fp30.s", &rb_shader_lighting_DBS_XY_Z_CUBE_fragment, GL_FRAGMENT_PROGRAM_NV);
	
	RB_LoadShader("cg/reflection_C_vp30.s", &rb_shader_reflection_C_vertex, GL_VERTEX_PROGRAM_NV);
	RB_LoadShader("cg/reflection_C_fp30.s", &rb_shader_reflection_C_fragment, GL_FRAGMENT_PROGRAM_NV);
	
	RB_LoadShader("cg/refraction_C_vp30.s", &rb_shader_refraction_C_vertex, GL_VERTEX_PROGRAM_NV);
	RB_LoadShader("cg/refraction_C_fp30.s", &rb_shader_refraction_C_fragment, GL_FRAGMENT_PROGRAM_NV);
	
	RB_LoadShader("cg/dispersion_C_vp30.s", &rb_shader_dispersion_C_vertex, GL_VERTEX_PROGRAM_NV);
	RB_LoadShader("cg/dispersion_C_fp30.s", &rb_shader_dispersion_C_fragment, GL_FRAGMENT_PROGRAM_NV);
}

void	RB_ShutdownGPUShaders()
{
	xglDeleteProgramsNV(1, &rb_shader_generic_vertex);			RB_CheckForError();
	xglDeleteProgramsNV(1, &rb_shader_generic_fragment);			RB_CheckForError();
	
	xglDeleteProgramsNV(1, &rb_shader_zfill_vertex);			RB_CheckForError();
	xglDeleteProgramsNV(1, &rb_shader_zfill_fragment);			RB_CheckForError();
	
	xglDeleteProgramsNV(1, &rb_shader_lighting_R_vertex);			RB_CheckForError();
	xglDeleteProgramsNV(1, &rb_shader_lighting_R_fragment);			RB_CheckForError();
	
	xglDeleteProgramsNV(1, &rb_shader_lighting_RB_vertex);			RB_CheckForError();
	xglDeleteProgramsNV(1, &rb_shader_lighting_RB_fragment);		RB_CheckForError();
	
	xglDeleteProgramsNV(1, &rb_shader_lighting_RBH_vertex);			RB_CheckForError();
	xglDeleteProgramsNV(1, &rb_shader_lighting_RBH_fragment);		RB_CheckForError();
		
	xglDeleteProgramsNV(1, &rb_shader_lighting_RBHS_vertex);		RB_CheckForError();
	xglDeleteProgramsNV(1, &rb_shader_lighting_RBHS_fragment);		RB_CheckForError();
	
	xglDeleteProgramsNV(1, &rb_shader_lighting_RBS_vertex);			RB_CheckForError();
	xglDeleteProgramsNV(1, &rb_shader_lighting_RBS_fragment);		RB_CheckForError();
	
	xglDeleteProgramsNV(1, &rb_shader_lighting_D_XY_Z_CUBE_vertex);		RB_CheckForError();
	xglDeleteProgramsNV(1, &rb_shader_lighting_D_XY_Z_CUBE_fragment);	RB_CheckForError();
	
	xglDeleteProgramsNV(1, &rb_shader_lighting_DB_XY_Z_CUBE_vertex);	RB_CheckForError();
	xglDeleteProgramsNV(1, &rb_shader_lighting_DB_XY_Z_CUBE_fragment);	RB_CheckForError();
	
	xglDeleteProgramsNV(1, &rb_shader_lighting_DBH_XY_Z_CUBE_vertex);	RB_CheckForError();
	xglDeleteProgramsNV(1, &rb_shader_lighting_DBH_XY_Z_CUBE_fragment);	RB_CheckForError();
	
	xglDeleteProgramsNV(1, &rb_shader_lighting_DBHS_XY_Z_CUBE_vertex);	RB_CheckForError();
	xglDeleteProgramsNV(1, &rb_shader_lighting_DBHS_XY_Z_CUBE_fragment);	RB_CheckForError();
	
	xglDeleteProgramsNV(1, &rb_shader_lighting_DBS_XY_Z_CUBE_vertex);	RB_CheckForError();
	xglDeleteProgramsNV(1, &rb_shader_lighting_DBS_XY_Z_CUBE_fragment);	RB_CheckForError();
	
	xglDeleteProgramsNV(1, &rb_shader_reflection_C_vertex);			RB_CheckForError();
	xglDeleteProgramsNV(1, &rb_shader_reflection_C_fragment);		RB_CheckForError();
	
	xglDeleteProgramsNV(1, &rb_shader_refraction_C_vertex);			RB_CheckForError();
	xglDeleteProgramsNV(1, &rb_shader_refraction_C_fragment);		RB_CheckForError();
	
	xglDeleteProgramsNV(1, &rb_shader_dispersion_C_vertex);			RB_CheckForError();
	xglDeleteProgramsNV(1, &rb_shader_dispersion_C_fragment);		RB_CheckForError();
}


void	RB_EnableShader_generic()
{
	xglEnable(GL_VERTEX_PROGRAM_NV);					RB_CheckForError();
	xglBindProgramNV(GL_VERTEX_PROGRAM_NV, rb_shader_generic_vertex);	RB_CheckForError();
	
	xglEnable(GL_FRAGMENT_PROGRAM_NV);					RB_CheckForError();
	xglBindProgramNV(GL_FRAGMENT_PROGRAM_NV, rb_shader_generic_fragment);	RB_CheckForError();

			
	RB_SelectTexture(GL_TEXTURE0_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY0_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY8_NV);	RB_CheckForError();
}

void	RB_DisableShader_generic()
{
	xglDisable(GL_VERTEX_PROGRAM_NV);	RB_CheckForError();
	xglDisable(GL_FRAGMENT_PROGRAM_NV);	RB_CheckForError();
	
	
	RB_SelectTexture(GL_TEXTURE0_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY0_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY8_NV);	RB_CheckForError();
}

void	RB_RenderCommand_generic(const r_command_t *cmd, 			const r_shader_stage_c *stage)
{
	RB_SetShaderStageState(cmd->getEntity(), stage, RENDER_TYPE_GENERIC);		RB_CheckForError();
	
	if(gl_config.arb_vertex_buffer_object && cmd->getEntityMesh()->vbo_array_buffer)
	{
		xglBindBufferARB(GL_ARRAY_BUFFER_ARB, cmd->getEntityMesh()->vbo_array_buffer);				RB_CheckForError();
		
		xglVertexAttribPointerNV(0, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_vertexes_ofs));		RB_CheckForError();
		xglVertexAttribPointerNV(8, 2, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_texcoords_ofs));	RB_CheckForError();
	}
	else
	{
		if(gl_config.arb_vertex_buffer_object)
			xglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);		RB_CheckForError();
		
		xglVertexAttribPointerNV(0, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->vertexes[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(8, 2, GL_FLOAT, 0, &(cmd->getEntityMesh()->texcoords[0]));		RB_CheckForError();
	}
	
	RB_SelectTexture(GL_TEXTURE0_ARB);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage);	RB_CheckForError();
	RB_Bind(stage->image);
	
	// set matrix_model_view_projection
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 0, GL_MODELVIEW_PROJECTION_NV, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_texture0
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 4, GL_TEXTURE0_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set color
	vec4_c color;
	RB_ModifyColor(cmd->getEntity()->getShared(), stage, color);
//	xglColor4fv(color);
//	xglProgramParameter4fNV(GL_VERTEX_PROGRAM_NV, 8, color[0], color[1], color[2], color[3]);	RB_CheckForError();
//	xglProgramParameter4fvNV(GL_VERTEX_PROGRAM_NV, 8, color);	RB_CheckForError();

	RB_FlushMesh(cmd->getEntityMesh());
}




void	RB_EnableShader_zfill()
{
	xglEnable(GL_VERTEX_PROGRAM_NV);					RB_CheckForError();
	xglBindProgramNV(GL_VERTEX_PROGRAM_NV, rb_shader_zfill_vertex);		RB_CheckForError();
	
	xglEnable(GL_FRAGMENT_PROGRAM_NV);					RB_CheckForError();
	xglBindProgramNV(GL_FRAGMENT_PROGRAM_NV, rb_shader_zfill_fragment);	RB_CheckForError();

			
	RB_SelectTexture(GL_TEXTURE0_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY0_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY8_NV);	RB_CheckForError();
}

void	RB_DisableShader_zfill()
{
	xglDisable(GL_VERTEX_PROGRAM_NV);	RB_CheckForError();
	xglDisable(GL_FRAGMENT_PROGRAM_NV);	RB_CheckForError();
	
	
	RB_SelectTexture(GL_TEXTURE0_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY0_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY8_NV);	RB_CheckForError();
}

void	RB_RenderCommand_zfill(const r_command_t *cmd,				const r_shader_stage_c *stage)
{
	RB_SetShaderStageState(cmd->getEntity(), stage, RENDER_TYPE_ZFILL);		RB_CheckForError();
	
	if(gl_config.arb_vertex_buffer_object && cmd->getEntityMesh()->vbo_array_buffer)
	{
		xglBindBufferARB(GL_ARRAY_BUFFER_ARB, cmd->getEntityMesh()->vbo_array_buffer);				RB_CheckForError();
		
		xglVertexAttribPointerNV(0, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_vertexes_ofs));		RB_CheckForError();
		xglVertexAttribPointerNV(8, 2, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_texcoords_ofs));	RB_CheckForError();
	}
	else
	{
		if(gl_config.arb_vertex_buffer_object)
			xglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);		RB_CheckForError();
		
		xglVertexAttribPointerNV(0, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->vertexes[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(8, 2, GL_FLOAT, 0, &(cmd->getEntityMesh()->texcoords[0]));		RB_CheckForError();
	}
	
	RB_SelectTexture(GL_TEXTURE0_ARB);
	
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
	
	// set matrix_model_view_projection
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 0, GL_MODELVIEW_PROJECTION_NV, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_texture0
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 4, GL_TEXTURE0_ARB, GL_IDENTITY_NV);	RB_CheckForError();

	RB_FlushMesh(cmd->getEntityMesh());
}


//
// static lighting
//
void		RB_EnableShader_lighting_R()
{
	xglEnable(GL_VERTEX_PROGRAM_NV);						RB_CheckForError();
	xglBindProgramNV(GL_VERTEX_PROGRAM_NV, rb_shader_lighting_R_vertex);		RB_CheckForError();
		
	xglEnable(GL_FRAGMENT_PROGRAM_NV);						RB_CheckForError();
	xglBindProgramNV(GL_FRAGMENT_PROGRAM_NV, rb_shader_lighting_R_fragment);	RB_CheckForError();

			
	RB_SelectTexture(GL_TEXTURE0_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE1_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE2_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY0_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY8_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY9_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY2_NV);	RB_CheckForError();
}

void		RB_DisableShader_lighting_R()
{
	xglDisable(GL_VERTEX_PROGRAM_NV);	RB_CheckForError();
	xglDisable(GL_FRAGMENT_PROGRAM_NV);	RB_CheckForError();
	
	
	RB_SelectTexture(GL_TEXTURE0_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE1_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE2_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY0_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY8_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY9_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY2_NV);	RB_CheckForError();
}

void		RB_RenderCommand_lighting_R(const r_command_t *cmd,		const r_shader_stage_c *stage_diffusemap,
										const r_shader_stage_c *stage_lightmap,
										const r_shader_stage_c *stage_deluxemap)
{
	RB_SetShaderStageState(cmd->getEntity(), stage_diffusemap, RENDER_TYPE_LIGHTING_R);		RB_CheckForError();
	
	if(gl_config.arb_vertex_buffer_object && cmd->getEntityMesh()->vbo_array_buffer)
	{
		xglBindBufferARB(GL_ARRAY_BUFFER_ARB, cmd->getEntityMesh()->vbo_array_buffer);				RB_CheckForError();
		
		xglVertexAttribPointerNV(0, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_vertexes_ofs));		RB_CheckForError();
		xglVertexAttribPointerNV(8, 2, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_texcoords_ofs));	RB_CheckForError();
		xglVertexAttribPointerNV(9, 2, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_texcoords_lm_ofs));	RB_CheckForError();
		xglVertexAttribPointerNV(2, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_normals_ofs));		RB_CheckForError();
	}
	else
	{
		if(gl_config.arb_vertex_buffer_object)
			xglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);		RB_CheckForError();
		
		xglVertexAttribPointerNV(0, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->vertexes[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(8, 2, GL_FLOAT, 0, &(cmd->getEntityMesh()->texcoords[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(9, 2, GL_FLOAT, 0, &(cmd->getEntityMesh()->texcoords_lm[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(2, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->normals[0]));		RB_CheckForError();
	}
	
	RB_SelectTexture(GL_TEXTURE0_ARB);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_diffusemap);	RB_CheckForError();
	RB_Bind(stage_diffusemap->image);
	
	RB_SelectTexture(GL_TEXTURE1_ARB);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_lightmap);
	RB_Bind(R_GetLightMapImageByNum(cmd->getInfoKey()));
	
	RB_SelectTexture(GL_TEXTURE2_ARB);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_deluxemap);
	RB_Bind(R_GetLightMapImageByNum(cmd->getInfoKey() + 1));
	
	RB_SelectTexture(GL_TEXTURE3_ARB);
	xglMatrixMode(GL_TEXTURE);
	xglLoadTransposeMatrixf(&rb_matrix_model[0][0]);
	xglMatrixMode(GL_MODELVIEW);
	
	// set matrix_model_view_projection
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV,  0, GL_MODELVIEW_PROJECTION_NV, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_texture0
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV,  4, GL_TEXTURE0_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_texture1
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV,  8, GL_TEXTURE1_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_texture2
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 12, GL_TEXTURE2_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_model
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 16, GL_TEXTURE3_ARB, GL_IDENTITY_NV);	RB_CheckForError();

	RB_FlushMesh(cmd->getEntityMesh());
}
										

						
void		RB_EnableShader_lighting_RB()
{
	xglEnable(GL_VERTEX_PROGRAM_NV);						RB_CheckForError();
	xglBindProgramNV(GL_VERTEX_PROGRAM_NV, rb_shader_lighting_RB_vertex);		RB_CheckForError();
		
	xglEnable(GL_FRAGMENT_PROGRAM_NV);						RB_CheckForError();
	xglBindProgramNV(GL_FRAGMENT_PROGRAM_NV, rb_shader_lighting_RB_fragment);	RB_CheckForError();

			
	RB_SelectTexture(GL_TEXTURE0_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE1_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE2_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE3_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY0_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY8_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY9_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY14_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY15_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY2_NV);	RB_CheckForError();
}

void		RB_DisableShader_lighting_RB()
{
	xglDisable(GL_VERTEX_PROGRAM_NV);	RB_CheckForError();
	xglDisable(GL_FRAGMENT_PROGRAM_NV);	RB_CheckForError();
	
	
	RB_SelectTexture(GL_TEXTURE0_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE1_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE2_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE3_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY0_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY8_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY9_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY14_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY15_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY2_NV);	RB_CheckForError();
}

void		RB_RenderCommand_lighting_RB(const r_command_t *cmd,		const r_shader_stage_c *stage_diffusemap,
										const r_shader_stage_c *stage_bumpmap,
										const r_shader_stage_c *stage_lightmap,
										const r_shader_stage_c *stage_deluxemap)
{
	RB_SetShaderStageState(cmd->getEntity(), stage_diffusemap, RENDER_TYPE_LIGHTING_RB);		RB_CheckForError();
	
	if(gl_config.arb_vertex_buffer_object && cmd->getEntityMesh()->vbo_array_buffer)
	{
		xglBindBufferARB(GL_ARRAY_BUFFER_ARB, cmd->getEntityMesh()->vbo_array_buffer);				RB_CheckForError();
		
		xglVertexAttribPointerNV(0, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_vertexes_ofs));		RB_CheckForError();
		xglVertexAttribPointerNV(8, 2, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_texcoords_ofs));	RB_CheckForError();
		xglVertexAttribPointerNV(9, 2, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_texcoords_lm_ofs));	RB_CheckForError();
		xglVertexAttribPointerNV(14, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_tangents_ofs));		RB_CheckForError();
		xglVertexAttribPointerNV(15, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_binormals_ofs));		RB_CheckForError();
		xglVertexAttribPointerNV(2, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_normals_ofs));		RB_CheckForError();
	}
	else
	{
		if(gl_config.arb_vertex_buffer_object)
			xglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);		RB_CheckForError();
		
		xglVertexAttribPointerNV(0, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->vertexes[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(8, 2, GL_FLOAT, 0, &(cmd->getEntityMesh()->texcoords[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(9, 2, GL_FLOAT, 0, &(cmd->getEntityMesh()->texcoords_lm[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(14, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->tangents[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(15, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->binormals[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(2, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->normals[0]));		RB_CheckForError();
	}
	
	RB_SelectTexture(GL_TEXTURE0_ARB);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_diffusemap);
	RB_Bind(stage_diffusemap->image);
	
	RB_SelectTexture(GL_TEXTURE1_ARB);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_bumpmap);
	RB_Bind(stage_bumpmap->image);
	
	RB_SelectTexture(GL_TEXTURE2_ARB);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_lightmap);
	RB_Bind(R_GetLightMapImageByNum(cmd->getInfoKey()));
	
	RB_SelectTexture(GL_TEXTURE3_ARB);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_deluxemap);
	RB_Bind(R_GetLightMapImageByNum(cmd->getInfoKey() + 1));
	
	// set matrix_model_view_projection
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV,  0, GL_MODELVIEW_PROJECTION_NV, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_texture0
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV,  4, GL_TEXTURE0_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_texture1
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV,  8, GL_TEXTURE1_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_texture2
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 12, GL_TEXTURE2_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_texture3
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 16, GL_TEXTURE3_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set bump_scale
	float bump_scale = RB_Evaluate(cmd->getEntity()->getShared(), stage_bumpmap->bump_scale);
	xglProgramNamedParameter4fNV(rb_shader_lighting_RB_fragment, strlen("bump_scale"), (GLubyte*)"bump_scale", bump_scale, 0, 0, 0);	RB_CheckForError();

	RB_FlushMesh(cmd->getEntityMesh());
}
										

void		RB_EnableShader_lighting_RBH()
{
	xglEnable(GL_VERTEX_PROGRAM_NV);						RB_CheckForError();
	xglBindProgramNV(GL_VERTEX_PROGRAM_NV, rb_shader_lighting_RBH_vertex);		RB_CheckForError();
		
	xglEnable(GL_FRAGMENT_PROGRAM_NV);						RB_CheckForError();
	xglBindProgramNV(GL_FRAGMENT_PROGRAM_NV, rb_shader_lighting_RBH_fragment);	RB_CheckForError();

			
	RB_SelectTexture(GL_TEXTURE0_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE1_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE2_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE3_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE4_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY0_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY8_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY9_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY14_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY15_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY2_NV);	RB_CheckForError();
}

void		RB_DisableShader_lighting_RBH()
{
	xglDisable(GL_VERTEX_PROGRAM_NV);	RB_CheckForError();
	xglDisable(GL_FRAGMENT_PROGRAM_NV);	RB_CheckForError();
	
	
	RB_SelectTexture(GL_TEXTURE0_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE1_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE2_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE3_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE4_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY0_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY8_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY9_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY14_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY15_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY2_NV);	RB_CheckForError();
}

void		RB_RenderCommand_lighting_RBH(const r_command_t *cmd,		const r_shader_stage_c *stage_diffusemap,
										const r_shader_stage_c *stage_bumpmap,
										const r_shader_stage_c *stage_heightmap,
										const r_shader_stage_c *stage_lightmap,
										const r_shader_stage_c *stage_deluxemap)
{
	RB_SetShaderStageState(cmd->getEntity(), stage_diffusemap, RENDER_TYPE_LIGHTING_RBH);		RB_CheckForError();
	
	if(gl_config.arb_vertex_buffer_object && cmd->getEntityMesh()->vbo_array_buffer)
	{
		xglBindBufferARB(GL_ARRAY_BUFFER_ARB, cmd->getEntityMesh()->vbo_array_buffer);				RB_CheckForError();
		
		xglVertexAttribPointerNV(0, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_vertexes_ofs));		RB_CheckForError();
		xglVertexAttribPointerNV(8, 2, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_texcoords_ofs));	RB_CheckForError();
		xglVertexAttribPointerNV(9, 2, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_texcoords_lm_ofs));	RB_CheckForError();
		xglVertexAttribPointerNV(14, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_tangents_ofs));	RB_CheckForError();
		xglVertexAttribPointerNV(15, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_binormals_ofs));	RB_CheckForError();
		xglVertexAttribPointerNV(2, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_normals_ofs));		RB_CheckForError();
	}
	else
	{
		if(gl_config.arb_vertex_buffer_object)
			xglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);		RB_CheckForError();
		
		xglVertexAttribPointerNV(0, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->vertexes[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(8, 2, GL_FLOAT, 0, &(cmd->getEntityMesh()->texcoords[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(9, 2, GL_FLOAT, 0, &(cmd->getEntityMesh()->texcoords_lm[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(14, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->tangents[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(15, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->binormals[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(2, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->normals[0]));		RB_CheckForError();
	}
	
	RB_SelectTexture(GL_TEXTURE0_ARB);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_diffusemap);
	RB_Bind(stage_diffusemap->image);
	
	RB_SelectTexture(GL_TEXTURE1_ARB);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_bumpmap);
	RB_Bind(stage_bumpmap->image);
	
	RB_SelectTexture(GL_TEXTURE2_ARB);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_heightmap);
	RB_Bind(stage_heightmap->image);
	
	RB_SelectTexture(GL_TEXTURE3_ARB);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_lightmap);
	RB_Bind(R_GetLightMapImageByNum(cmd->getInfoKey()));
	
	RB_SelectTexture(GL_TEXTURE4_ARB);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_deluxemap);
	RB_Bind(R_GetLightMapImageByNum(cmd->getInfoKey() + 1));
	
	// set matrix_model_view_projection
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV,  0, GL_MODELVIEW_PROJECTION_NV, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_texture0
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV,  4, GL_TEXTURE0_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_texture1
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV,  8, GL_TEXTURE1_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_texture2
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 12, GL_TEXTURE2_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_texture3
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 16, GL_TEXTURE3_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_texture4
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 20, GL_TEXTURE4_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set view_origin in object space
	vec3_c view_origin = cmd->getEntity()->getTransform().affineInverse() * (r_origin - cmd->getEntity()->getShared().origin);
	xglProgramNamedParameter4fNV(rb_shader_lighting_RBH_fragment, strlen("view_origin"), (GLubyte*)"view_origin", view_origin[0], view_origin[1], view_origin[2], 0);	RB_CheckForError();
	
	// set bump_scale
	float bump_scale = RB_Evaluate(cmd->getEntity()->getShared(), stage_bumpmap->bump_scale);
	xglProgramNamedParameter4fNV(rb_shader_lighting_RBH_fragment, strlen("bump_scale"), (GLubyte*)"bump_scale", bump_scale, 0, 0, 0);	RB_CheckForError();
	
	// set height_scale
	float height_scale = RB_Evaluate(cmd->getEntity()->getShared(), stage_heightmap->height_scale);
	xglProgramNamedParameter4fNV(rb_shader_lighting_RBH_fragment, strlen("height_scale"), (GLubyte*)"height_scale", height_scale, 0, 0, 0);	RB_CheckForError();
	
	// set height_bias
	float height_bias = RB_Evaluate(cmd->getEntity()->getShared(), stage_heightmap->height_bias);
	xglProgramNamedParameter4fNV(rb_shader_lighting_RBH_fragment, strlen("height_bias"), (GLubyte*)"height_bias", height_bias, 0, 0, 0);	RB_CheckForError();

	RB_FlushMesh(cmd->getEntityMesh());
}




void		RB_EnableShader_lighting_RBHS()
{
	xglEnable(GL_VERTEX_PROGRAM_NV);						RB_CheckForError();
	xglBindProgramNV(GL_VERTEX_PROGRAM_NV, rb_shader_lighting_RBHS_vertex);		RB_CheckForError();
		
	xglEnable(GL_FRAGMENT_PROGRAM_NV);						RB_CheckForError();
	xglBindProgramNV(GL_FRAGMENT_PROGRAM_NV, rb_shader_lighting_RBHS_fragment);	RB_CheckForError();

			
	RB_SelectTexture(GL_TEXTURE0_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE1_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE2_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE3_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE4_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE5_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY0_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY8_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY9_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY14_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY15_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY2_NV);	RB_CheckForError();
}

void		RB_DisableShader_lighting_RBHS()
{
	xglDisable(GL_VERTEX_PROGRAM_NV);	RB_CheckForError();
	xglDisable(GL_FRAGMENT_PROGRAM_NV);	RB_CheckForError();
	
	
	RB_SelectTexture(GL_TEXTURE0_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE1_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE2_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE3_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE4_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE5_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY0_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY8_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY9_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY14_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY15_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY2_NV);	RB_CheckForError();
}

void		RB_RenderCommand_lighting_RBHS(const r_command_t *cmd,		const r_shader_stage_c *stage_diffusemap,
										const r_shader_stage_c *stage_bumpmap,
										const r_shader_stage_c *stage_heightmap,
										const r_shader_stage_c *stage_specularmap,
										const r_shader_stage_c *stage_lightmap,
										const r_shader_stage_c *stage_deluxemap)
{
	RB_SetShaderStageState(cmd->getEntity(), stage_diffusemap, RENDER_TYPE_LIGHTING_RBHS);		RB_CheckForError();
	
	if(gl_config.arb_vertex_buffer_object && cmd->getEntityMesh()->vbo_array_buffer)
	{
		xglBindBufferARB(GL_ARRAY_BUFFER_ARB, cmd->getEntityMesh()->vbo_array_buffer);				RB_CheckForError();
		
		xglVertexAttribPointerNV(0, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_vertexes_ofs));		RB_CheckForError();
		xglVertexAttribPointerNV(8, 2, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_texcoords_ofs));	RB_CheckForError();
		xglVertexAttribPointerNV(9, 2, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_texcoords_lm_ofs));	RB_CheckForError();
		xglVertexAttribPointerNV(14, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_tangents_ofs));	RB_CheckForError();
		xglVertexAttribPointerNV(15, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_binormals_ofs));	RB_CheckForError();
		xglVertexAttribPointerNV(2, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_normals_ofs));		RB_CheckForError();
	}
	else
	{
		if(gl_config.arb_vertex_buffer_object)
			xglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);		RB_CheckForError();
		
		xglVertexAttribPointerNV(0, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->vertexes[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(8, 2, GL_FLOAT, 0, &(cmd->getEntityMesh()->texcoords[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(9, 2, GL_FLOAT, 0, &(cmd->getEntityMesh()->texcoords_lm[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(14, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->tangents[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(15, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->binormals[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(2, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->normals[0]));		RB_CheckForError();
	}
	
	RB_SelectTexture(GL_TEXTURE0_ARB);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_diffusemap);
	RB_Bind(stage_diffusemap->image);
	
	RB_SelectTexture(GL_TEXTURE1_ARB);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_bumpmap);
	RB_Bind(stage_bumpmap->image);
	
	RB_SelectTexture(GL_TEXTURE2_ARB);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_heightmap);
	RB_Bind(stage_heightmap->image);
	
	RB_SelectTexture(GL_TEXTURE3_ARB);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_specularmap);
	RB_Bind(stage_specularmap->image);
	
	RB_SelectTexture(GL_TEXTURE4_ARB);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_lightmap);
	RB_Bind(R_GetLightMapImageByNum(cmd->getInfoKey()));
	
	RB_SelectTexture(GL_TEXTURE5_ARB);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_deluxemap);
	RB_Bind(R_GetLightMapImageByNum(cmd->getInfoKey() + 1));
	
	// set matrix_model_view_projection
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV,  0, GL_MODELVIEW_PROJECTION_NV, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_texture0
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV,  4, GL_TEXTURE0_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_texture1
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV,  8, GL_TEXTURE1_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_texture2
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 12, GL_TEXTURE2_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_texture3
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 16, GL_TEXTURE3_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_texture4
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 20, GL_TEXTURE4_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_texture5
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 24, GL_TEXTURE5_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set view_origin in object space
	vec3_c view_origin = cmd->getEntity()->getTransform().affineInverse() * (r_origin - cmd->getEntity()->getShared().origin);
	xglProgramNamedParameter4fNV(rb_shader_lighting_RBHS_fragment, strlen("view_origin"), (GLubyte*)"view_origin", view_origin[0], view_origin[1], view_origin[2], 0);	RB_CheckForError();
	
	// set bump_scale
	float bump_scale = RB_Evaluate(cmd->getEntity()->getShared(), stage_bumpmap->bump_scale);
	xglProgramNamedParameter4fNV(rb_shader_lighting_RBHS_fragment, strlen("bump_scale"), (GLubyte*)"bump_scale", bump_scale, 0, 0, 0);	RB_CheckForError();
	
	// set height_scale
	float height_scale = RB_Evaluate(cmd->getEntity()->getShared(), stage_heightmap->height_scale);
	xglProgramNamedParameter4fNV(rb_shader_lighting_RBHS_fragment, strlen("height_scale"), (GLubyte*)"height_scale", height_scale, 0, 0, 0);	RB_CheckForError();
	
	// set height_bias
	float height_bias = RB_Evaluate(cmd->getEntity()->getShared(), stage_heightmap->height_bias);
	xglProgramNamedParameter4fNV(rb_shader_lighting_RBHS_fragment, strlen("height_bias"), (GLubyte*)"height_bias", height_bias, 0, 0, 0);	RB_CheckForError();
	
	// set specular_exponent
	float specular_exponent = X_max(0, RB_Evaluate(cmd->getEntity()->getShared(), stage_specularmap->specular_exponent));
	xglProgramNamedParameter4fNV(rb_shader_lighting_RBHS_fragment, strlen("specular_exponent"), (GLubyte*)"specular_exponent", specular_exponent, 0, 0, 0);	RB_CheckForError();

	RB_FlushMesh(cmd->getEntityMesh());
}
									
void		RB_EnableShader_lighting_RBS()
{
	xglEnable(GL_VERTEX_PROGRAM_NV);						RB_CheckForError();
	xglBindProgramNV(GL_VERTEX_PROGRAM_NV, rb_shader_lighting_RBS_vertex);		RB_CheckForError();
		
	xglEnable(GL_FRAGMENT_PROGRAM_NV);						RB_CheckForError();
	xglBindProgramNV(GL_FRAGMENT_PROGRAM_NV, rb_shader_lighting_RBS_fragment);	RB_CheckForError();

			
	RB_SelectTexture(GL_TEXTURE0_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE1_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE2_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE3_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE4_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY0_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY8_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY9_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY14_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY15_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY2_NV);	RB_CheckForError();
}

void		RB_DisableShader_lighting_RBS()
{
	xglDisable(GL_VERTEX_PROGRAM_NV);	RB_CheckForError();
	xglDisable(GL_FRAGMENT_PROGRAM_NV);	RB_CheckForError();
	
	
	RB_SelectTexture(GL_TEXTURE0_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE1_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE2_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE3_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE4_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY0_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY8_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY9_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY14_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY15_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY2_NV);	RB_CheckForError();
}

void		RB_RenderCommand_lighting_RBS(const r_command_t *cmd,		const r_shader_stage_c *stage_diffusemap,
										const r_shader_stage_c *stage_bumpmap,
										const r_shader_stage_c *stage_specularmap,
										const r_shader_stage_c *stage_lightmap,
										const r_shader_stage_c *stage_deluxemap)
{
	RB_SetShaderStageState(cmd->getEntity(), stage_diffusemap, RENDER_TYPE_LIGHTING_RBS);		RB_CheckForError();
	
	if(gl_config.arb_vertex_buffer_object && cmd->getEntityMesh()->vbo_array_buffer)
	{
		xglBindBufferARB(GL_ARRAY_BUFFER_ARB, cmd->getEntityMesh()->vbo_array_buffer);				RB_CheckForError();
		
		xglVertexAttribPointerNV(0, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_vertexes_ofs));		RB_CheckForError();
		xglVertexAttribPointerNV(8, 2, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_texcoords_ofs));	RB_CheckForError();
		xglVertexAttribPointerNV(9, 2, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_texcoords_lm_ofs));	RB_CheckForError();
		xglVertexAttribPointerNV(14, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_tangents_ofs));		RB_CheckForError();
		xglVertexAttribPointerNV(15, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_binormals_ofs));		RB_CheckForError();
		xglVertexAttribPointerNV(2, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_normals_ofs));		RB_CheckForError();
	}
	else
	{
		if(gl_config.arb_vertex_buffer_object)
			xglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);		RB_CheckForError();
		
		xglVertexAttribPointerNV(0, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->vertexes[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(8, 2, GL_FLOAT, 0, &(cmd->getEntityMesh()->texcoords[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(9, 2, GL_FLOAT, 0, &(cmd->getEntityMesh()->texcoords_lm[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(14, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->tangents[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(15, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->binormals[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(2, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->normals[0]));		RB_CheckForError();
	}
	
	RB_SelectTexture(GL_TEXTURE0_ARB);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_diffusemap);
	RB_Bind(stage_diffusemap->image);
	
	RB_SelectTexture(GL_TEXTURE1_ARB);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_bumpmap);
	RB_Bind(stage_bumpmap->image);
	
	RB_SelectTexture(GL_TEXTURE2_ARB);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_specularmap);
	RB_Bind(stage_specularmap->image);
	
	RB_SelectTexture(GL_TEXTURE3_ARB);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_lightmap);
	RB_Bind(R_GetLightMapImageByNum(cmd->getInfoKey()));
	
	RB_SelectTexture(GL_TEXTURE4_ARB);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_deluxemap);
	RB_Bind(R_GetLightMapImageByNum(cmd->getInfoKey() + 1));
	
	// set matrix_model_view_projection
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV,  0, GL_MODELVIEW_PROJECTION_NV, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_texture0
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV,  4, GL_TEXTURE0_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_texture1
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV,  8, GL_TEXTURE1_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_texture2
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 12, GL_TEXTURE2_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_texture3
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 16, GL_TEXTURE3_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_texture4
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 20, GL_TEXTURE4_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set view_origin in object space
	vec3_c view_origin = cmd->getEntity()->getTransform().affineInverse() * (r_origin - cmd->getEntity()->getShared().origin);
	xglProgramNamedParameter4fNV(rb_shader_lighting_RBS_fragment, strlen("view_origin"), (GLubyte*)"view_origin", view_origin[0], view_origin[1], view_origin[2], 0);	RB_CheckForError();
	
	// set bump_scale
	float bump_scale = RB_Evaluate(cmd->getEntity()->getShared(), stage_bumpmap->bump_scale);
	xglProgramNamedParameter4fNV(rb_shader_lighting_RBS_fragment, strlen("bump_scale"), (GLubyte*)"bump_scale", bump_scale, 0, 0, 0);	RB_CheckForError();
	
	// set specular_exponent
	float specular_exponent = X_max(0, RB_Evaluate(cmd->getEntity()->getShared(), stage_specularmap->specular_exponent));
	xglProgramNamedParameter4fNV(rb_shader_lighting_RBS_fragment, strlen("specular_exponent"), (GLubyte*)"specular_exponent", specular_exponent, 0, 0, 0);	RB_CheckForError();

	RB_FlushMesh(cmd->getEntityMesh());
}


//
// dynamic lighting
//
void		RB_EnableShader_lighting_D_XY_Z_CUBE()
{
	xglEnable(GL_VERTEX_PROGRAM_NV);							RB_CheckForError();
	xglBindProgramNV(GL_VERTEX_PROGRAM_NV, rb_shader_lighting_D_XY_Z_CUBE_vertex);		RB_CheckForError();
		
	xglEnable(GL_FRAGMENT_PROGRAM_NV);							RB_CheckForError();
	xglBindProgramNV(GL_FRAGMENT_PROGRAM_NV, rb_shader_lighting_D_XY_Z_CUBE_fragment);	RB_CheckForError();

			
	RB_SelectTexture(GL_TEXTURE0_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE1_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE2_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE3_ARB);
	xglEnable(GL_TEXTURE_CUBE_MAP_ARB);	RB_CheckForError();
	
	
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY0_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY8_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY2_NV);	RB_CheckForError();
}

void		RB_DisableShader_lighting_D_XY_Z_CUBE()
{
	xglDisable(GL_VERTEX_PROGRAM_NV);	RB_CheckForError();
	xglDisable(GL_FRAGMENT_PROGRAM_NV);	RB_CheckForError();
	
	
	RB_SelectTexture(GL_TEXTURE0_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE1_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE2_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE3_ARB);
	xglDisable(GL_TEXTURE_CUBE_MAP_ARB);	RB_CheckForError();
	
	
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY0_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY8_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY2_NV);	RB_CheckForError();
}

void		RB_RenderCommand_lighting_D_XY_Z_CUBE(const r_command_t *cmd,	const r_shader_stage_c *stage_diffusemap,
										const r_shader_stage_c *stage_attenuationmap_xy,
										const r_shader_stage_c *stage_attenuationmap_z,
										const r_shader_stage_c *stage_attenuationmap_cube)
{
	RB_SetShaderStageState(cmd->getEntity(), stage_diffusemap, RENDER_TYPE_LIGHTING_D);		RB_CheckForError();
	
	if(gl_config.arb_vertex_buffer_object && cmd->getEntityMesh()->vbo_array_buffer)
	{
		xglBindBufferARB(GL_ARRAY_BUFFER_ARB, cmd->getEntityMesh()->vbo_array_buffer);				RB_CheckForError();
		
		xglVertexAttribPointerNV(0, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_vertexes_ofs));		RB_CheckForError();
		xglVertexAttribPointerNV(8, 2, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_texcoords_ofs));	RB_CheckForError();
		xglVertexAttribPointerNV(2, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_normals_ofs));		RB_CheckForError();
	}
	else
	{
		if(gl_config.arb_vertex_buffer_object)
			xglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);		RB_CheckForError();
		
		xglVertexAttribPointerNV(0, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->vertexes[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(8, 2, GL_FLOAT, 0, &(cmd->getEntityMesh()->texcoords[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(2, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->normals[0]));		RB_CheckForError();
	}
	
	RB_SelectTexture(GL_TEXTURE0_ARB);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_diffusemap);	RB_CheckForError();
	RB_Bind(stage_diffusemap->image);
	
	RB_SelectTexture(GL_TEXTURE1_ARB);
	RB_ModifyLightTextureMatrix(cmd->getLight(), cmd->getEntity(), stage_attenuationmap_xy);
	RB_Bind(stage_attenuationmap_xy->image);
	
	RB_SelectTexture(GL_TEXTURE2_ARB);
	xglMatrixMode(GL_TEXTURE);
	xglLoadTransposeMatrixf(&rb_matrix_model[0][0]);
	xglMatrixMode(GL_MODELVIEW);
	RB_Bind(stage_attenuationmap_z->image);
	
	RB_SelectTexture(GL_TEXTURE3_ARB);
	RB_ModifyLightCubeTextureMatrix(cmd->getLight(), cmd->getEntity(), stage_attenuationmap_cube);
	RB_Bind(stage_attenuationmap_cube->image);
	
	// set matrix_model_view_projection
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV,  0, GL_MODELVIEW_PROJECTION_NV, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_model
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV,  4, GL_TEXTURE2_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_tcmod_diffuse
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV,  8, GL_TEXTURE0_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_tcmod_attenuation_xy_z
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 12, GL_TEXTURE1_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_tcmod_attenuation_cube
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 16, GL_TEXTURE3_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set light_origin in world space
	const vec3_c& light_origin = cmd->getLight()->getOrigin();
	xglProgramNamedParameter4fNV(rb_shader_lighting_D_XY_Z_CUBE_fragment, strlen("light_origin"), (GLubyte*)"light_origin", light_origin[0], light_origin[1], light_origin[2], 1);	RB_CheckForError();
	
	// set light_color
	vec4_c light_color;
	RB_ModifyColor(cmd->getLight()->getShared(), stage_attenuationmap_xy, light_color);
	xglProgramNamedParameter4fNV(rb_shader_lighting_D_XY_Z_CUBE_fragment, strlen("light_color"), (GLubyte*)"light_color", light_color[0], light_color[1], light_color[2], 0);	RB_CheckForError();

	RB_FlushMesh(cmd->getEntityMesh());
}


void		RB_EnableShader_lighting_DB_XY_Z_CUBE()
{
	xglEnable(GL_VERTEX_PROGRAM_NV);							RB_CheckForError();
	xglBindProgramNV(GL_VERTEX_PROGRAM_NV, rb_shader_lighting_DB_XY_Z_CUBE_vertex);		RB_CheckForError();
		
	xglEnable(GL_FRAGMENT_PROGRAM_NV);							RB_CheckForError();
	xglBindProgramNV(GL_FRAGMENT_PROGRAM_NV, rb_shader_lighting_DB_XY_Z_CUBE_fragment);	RB_CheckForError();

			
	RB_SelectTexture(GL_TEXTURE0_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE1_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE2_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE3_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE4_ARB);
	xglEnable(GL_TEXTURE_CUBE_MAP_ARB);	RB_CheckForError();
	
	
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY0_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY8_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY14_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY15_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY2_NV);	RB_CheckForError();
}

void		RB_DisableShader_lighting_DB_XY_Z_CUBE()
{
	xglDisable(GL_VERTEX_PROGRAM_NV);	RB_CheckForError();
	xglDisable(GL_FRAGMENT_PROGRAM_NV);	RB_CheckForError();
	
	
	RB_SelectTexture(GL_TEXTURE0_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE1_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE2_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE3_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE4_ARB);
	xglDisable(GL_TEXTURE_CUBE_MAP_ARB);	RB_CheckForError();
	
	
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY0_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY8_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY14_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY15_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY2_NV);	RB_CheckForError();
}

void		RB_RenderCommand_lighting_DB_XY_Z_CUBE(const r_command_t *cmd,	const r_shader_stage_c *stage_diffusemap,
										const r_shader_stage_c *stage_bumpmap,
										const r_shader_stage_c *stage_attenuationmap_xy,
										const r_shader_stage_c *stage_attenuationmap_z,
										const r_shader_stage_c *stage_attenuationmap_cube)
{
	RB_SetShaderStageState(cmd->getEntity(), stage_diffusemap, RENDER_TYPE_LIGHTING_DB);		RB_CheckForError();
	
	if(gl_config.arb_vertex_buffer_object && cmd->getEntityMesh()->vbo_array_buffer)
	{
		xglBindBufferARB(GL_ARRAY_BUFFER_ARB, cmd->getEntityMesh()->vbo_array_buffer);				RB_CheckForError();
		
		xglVertexAttribPointerNV(0, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_vertexes_ofs));		RB_CheckForError();
		xglVertexAttribPointerNV(8, 2, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_texcoords_ofs));	RB_CheckForError();
		xglVertexAttribPointerNV(14, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_tangents_ofs));	RB_CheckForError();
		xglVertexAttribPointerNV(15, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_binormals_ofs));	RB_CheckForError();
		xglVertexAttribPointerNV(2, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_normals_ofs));		RB_CheckForError();
	}
	else
	{
		if(gl_config.arb_vertex_buffer_object)
			xglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);		RB_CheckForError();
		
		xglVertexAttribPointerNV(0, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->vertexes[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(8, 2, GL_FLOAT, 0, &(cmd->getEntityMesh()->texcoords[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(14, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->tangents[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(15, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->binormals[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(2, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->normals[0]));		RB_CheckForError();
	}
	
	RB_SelectTexture(GL_TEXTURE0_ARB);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_diffusemap);	RB_CheckForError();
	RB_Bind(stage_diffusemap->image);
	
	RB_SelectTexture(GL_TEXTURE1_ARB);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_bumpmap);	RB_CheckForError();
	RB_Bind(stage_bumpmap->image);
	
	RB_SelectTexture(GL_TEXTURE2_ARB);
	RB_ModifyLightTextureMatrix(cmd->getLight(), cmd->getEntity(), stage_attenuationmap_xy);
	RB_Bind(stage_attenuationmap_xy->image);
	
	RB_SelectTexture(GL_TEXTURE3_ARB);
	xglMatrixMode(GL_TEXTURE);
	xglLoadIdentity();
	xglMatrixMode(GL_MODELVIEW);
	RB_Bind(stage_attenuationmap_z->image);
	
	RB_SelectTexture(GL_TEXTURE4_ARB);
	RB_ModifyLightCubeTextureMatrix(cmd->getLight(), cmd->getEntity(), stage_attenuationmap_cube);
	RB_Bind(stage_attenuationmap_cube->image);
	
	// set matrix_model_view_projection
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV,  0, GL_MODELVIEW_PROJECTION_NV, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_tcmod_diffuse
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV,  4, GL_TEXTURE0_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_tcmod_bump
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV,  8, GL_TEXTURE1_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_tcmod_attenuation_xy_z
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 12, GL_TEXTURE2_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_tcmod_attenuation_cube
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 16, GL_TEXTURE4_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set light_origin in object space
	vec3_c light_origin = cmd->getEntity()->getTransform().affineInverse() * (cmd->getLight()->getOrigin() - cmd->getEntity()->getShared().origin);
	xglProgramNamedParameter4fNV(rb_shader_lighting_DB_XY_Z_CUBE_fragment, strlen("light_origin"), (GLubyte*)"light_origin", light_origin[0], light_origin[1], light_origin[2], 1);	RB_CheckForError();
	
	// set light_color
	vec4_c light_color;
	RB_ModifyColor(cmd->getLight()->getShared(), stage_attenuationmap_xy, light_color);
	xglProgramNamedParameter4fNV(rb_shader_lighting_DB_XY_Z_CUBE_fragment, strlen("light_color"), (GLubyte*)"light_color", light_color[0], light_color[1], light_color[2], 0);	RB_CheckForError();
	
	// set bump_scale
	float bump_scale = RB_Evaluate(cmd->getEntity()->getShared(), stage_bumpmap->bump_scale);
	xglProgramNamedParameter4fNV(rb_shader_lighting_DB_XY_Z_CUBE_fragment, strlen("bump_scale"), (GLubyte*)"bump_scale", bump_scale, 0, 0, 0);	RB_CheckForError();

	RB_FlushMesh(cmd->getEntityMesh());
}														
																								



void		RB_EnableShader_lighting_DBH_XY_Z_CUBE()
{
	xglEnable(GL_VERTEX_PROGRAM_NV);							RB_CheckForError();
	xglBindProgramNV(GL_VERTEX_PROGRAM_NV, rb_shader_lighting_DBH_XY_Z_CUBE_vertex);	RB_CheckForError();
		
	xglEnable(GL_FRAGMENT_PROGRAM_NV);							RB_CheckForError();
	xglBindProgramNV(GL_FRAGMENT_PROGRAM_NV, rb_shader_lighting_DBH_XY_Z_CUBE_fragment);	RB_CheckForError();

			
	RB_SelectTexture(GL_TEXTURE0_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE1_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE2_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE3_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE4_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE5_ARB);
	xglEnable(GL_TEXTURE_CUBE_MAP_ARB);	RB_CheckForError();
	
	
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY0_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY8_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY14_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY15_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY2_NV);	RB_CheckForError();
}

void		RB_DisableShader_lighting_DBH_XY_Z_CUBE()
{
	xglDisable(GL_VERTEX_PROGRAM_NV);	RB_CheckForError();
	xglDisable(GL_FRAGMENT_PROGRAM_NV);	RB_CheckForError();
	
	
	RB_SelectTexture(GL_TEXTURE0_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE1_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE2_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE3_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE4_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE5_ARB);
	xglDisable(GL_TEXTURE_CUBE_MAP_ARB);	RB_CheckForError();
	
	
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY0_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY8_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY14_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY15_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY2_NV);	RB_CheckForError();
}

void		RB_RenderCommand_lighting_DBH_XY_Z_CUBE(const r_command_t *cmd,	const r_shader_stage_c *stage_diffusemap,
										const r_shader_stage_c *stage_bumpmap,
										const r_shader_stage_c *stage_heightmap,
										const r_shader_stage_c *stage_attenuationmap_xy,
										const r_shader_stage_c *stage_attenuationmap_z,
										const r_shader_stage_c *stage_attenuationmap_cube)
{
	RB_SetShaderStageState(cmd->getEntity(), stage_diffusemap, RENDER_TYPE_LIGHTING_DBH);		RB_CheckForError();
	
	if(gl_config.arb_vertex_buffer_object && cmd->getEntityMesh()->vbo_array_buffer)
	{
		xglBindBufferARB(GL_ARRAY_BUFFER_ARB, cmd->getEntityMesh()->vbo_array_buffer);				RB_CheckForError();
		
		xglVertexAttribPointerNV(0, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_vertexes_ofs));		RB_CheckForError();
		xglVertexAttribPointerNV(8, 2, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_texcoords_ofs));	RB_CheckForError();
		xglVertexAttribPointerNV(14, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_tangents_ofs));	RB_CheckForError();
		xglVertexAttribPointerNV(15, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_binormals_ofs));	RB_CheckForError();
		xglVertexAttribPointerNV(2, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_normals_ofs));		RB_CheckForError();
	}
	else
	{
		if(gl_config.arb_vertex_buffer_object)
			xglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);		RB_CheckForError();
		
		xglVertexAttribPointerNV(0, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->vertexes[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(8, 2, GL_FLOAT, 0, &(cmd->getEntityMesh()->texcoords[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(14, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->tangents[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(15, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->binormals[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(2, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->normals[0]));		RB_CheckForError();
	}
	
	RB_SelectTexture(GL_TEXTURE0_ARB);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_diffusemap);	RB_CheckForError();
	RB_Bind(stage_diffusemap->image);
	
	RB_SelectTexture(GL_TEXTURE1_ARB);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_bumpmap);	RB_CheckForError();
	RB_Bind(stage_bumpmap->image);
	
	RB_SelectTexture(GL_TEXTURE2_ARB);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_heightmap);	RB_CheckForError();
	RB_Bind(stage_heightmap->image);
	
	RB_SelectTexture(GL_TEXTURE3_ARB);
	RB_ModifyLightTextureMatrix(cmd->getLight(), cmd->getEntity(), stage_attenuationmap_xy);
	RB_Bind(stage_attenuationmap_xy->image);
	
	RB_SelectTexture(GL_TEXTURE4_ARB);
	xglMatrixMode(GL_TEXTURE);
	xglLoadIdentity();
	xglMatrixMode(GL_MODELVIEW);
	RB_Bind(stage_attenuationmap_z->image);
	
	RB_SelectTexture(GL_TEXTURE5_ARB);
	RB_ModifyLightCubeTextureMatrix(cmd->getLight(), cmd->getEntity(), stage_attenuationmap_cube);
	RB_Bind(stage_attenuationmap_cube->image);
	
	// set matrix_model_view_projection
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV,  0, GL_MODELVIEW_PROJECTION_NV, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_tcmod_diffuse
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV,  4, GL_TEXTURE0_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_tcmod_bump
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV,  8, GL_TEXTURE1_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_tcmod_height
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 12, GL_TEXTURE2_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_tcmod_attenuation_xy_z
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 16, GL_TEXTURE3_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_tcmod_attenuation_cube
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 20, GL_TEXTURE5_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set view_origin in object space
	vec3_c view_origin = cmd->getEntity()->getTransform().affineInverse() * (r_origin - cmd->getEntity()->getShared().origin);
	xglProgramNamedParameter4fNV(rb_shader_lighting_DBH_XY_Z_CUBE_fragment, strlen("view_origin"), (GLubyte*)"view_origin", view_origin[0], view_origin[1], view_origin[2], 0);	RB_CheckForError();
	
	// set light_origin in object space
	vec3_c light_origin = cmd->getEntity()->getTransform().affineInverse() * (cmd->getLight()->getOrigin() - cmd->getEntity()->getShared().origin);
	xglProgramNamedParameter4fNV(rb_shader_lighting_DBH_XY_Z_CUBE_fragment, strlen("light_origin"), (GLubyte*)"light_origin", light_origin[0], light_origin[1], light_origin[2], 1);	RB_CheckForError();
	
	// set light_color
	vec4_c light_color;
	RB_ModifyColor(cmd->getLight()->getShared(), stage_attenuationmap_xy, light_color);
	xglProgramNamedParameter4fNV(rb_shader_lighting_DBH_XY_Z_CUBE_fragment, strlen("light_color"), (GLubyte*)"light_color", light_color[0], light_color[1], light_color[2], 0);	RB_CheckForError();
	
	// set bump_scale
	float bump_scale = RB_Evaluate(cmd->getEntity()->getShared(), stage_bumpmap->bump_scale);
	xglProgramNamedParameter4fNV(rb_shader_lighting_DBH_XY_Z_CUBE_fragment, strlen("bump_scale"), (GLubyte*)"bump_scale", bump_scale, 0, 0, 0);	RB_CheckForError();
	
	// set height_scale
	float height_scale = RB_Evaluate(cmd->getEntity()->getShared(), stage_heightmap->height_scale);
	xglProgramNamedParameter4fNV(rb_shader_lighting_DBH_XY_Z_CUBE_fragment, strlen("height_scale"), (GLubyte*)"height_scale", height_scale, 0, 0, 0);	RB_CheckForError();
	
	// set height_bias
	float height_bias = RB_Evaluate(cmd->getEntity()->getShared(), stage_heightmap->height_bias);
	xglProgramNamedParameter4fNV(rb_shader_lighting_DBH_XY_Z_CUBE_fragment, strlen("height_bias"), (GLubyte*)"height_bias", height_bias, 0, 0, 0);	RB_CheckForError();

	RB_FlushMesh(cmd->getEntityMesh());
}								
								



void		RB_EnableShader_lighting_DBHS_XY_Z_CUBE()
{
	xglEnable(GL_VERTEX_PROGRAM_NV);							RB_CheckForError();
	xglBindProgramNV(GL_VERTEX_PROGRAM_NV, rb_shader_lighting_DBHS_XY_Z_CUBE_vertex);	RB_CheckForError();
		
	xglEnable(GL_FRAGMENT_PROGRAM_NV);							RB_CheckForError();
	xglBindProgramNV(GL_FRAGMENT_PROGRAM_NV, rb_shader_lighting_DBHS_XY_Z_CUBE_fragment);	RB_CheckForError();

			
	RB_SelectTexture(GL_TEXTURE0_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE1_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE2_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE3_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE4_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE5_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE6_ARB);
	xglEnable(GL_TEXTURE_CUBE_MAP_ARB);	RB_CheckForError();
	
	
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY0_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY8_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY14_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY15_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY2_NV);	RB_CheckForError();
}

void		RB_DisableShader_lighting_DBHS_XY_Z_CUBE()
{
	xglDisable(GL_VERTEX_PROGRAM_NV);	RB_CheckForError();
	xglDisable(GL_FRAGMENT_PROGRAM_NV);	RB_CheckForError();
	
	
	RB_SelectTexture(GL_TEXTURE0_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE1_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE2_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE3_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE4_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE5_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE6_ARB);
	xglDisable(GL_TEXTURE_CUBE_MAP_ARB);	RB_CheckForError();
	
	
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY0_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY8_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY14_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY15_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY2_NV);	RB_CheckForError();
}

void		RB_RenderCommand_lighting_DBHS_XY_Z_CUBE(const r_command_t *cmd,	const r_shader_stage_c *stage_diffusemap,
											const r_shader_stage_c *stage_bumpmap,
											const r_shader_stage_c *stage_heightmap,
											const r_shader_stage_c *stage_specularmap,
											const r_shader_stage_c *stage_attenuationmap_xy,
											const r_shader_stage_c *stage_attenuationmap_z,
											const r_shader_stage_c *stage_attenuationmap_cube)
{
	RB_SetShaderStageState(cmd->getEntity(), stage_diffusemap, RENDER_TYPE_LIGHTING_DBHS);		RB_CheckForError();
	
	if(gl_config.arb_vertex_buffer_object && cmd->getEntityMesh()->vbo_array_buffer)
	{
		xglBindBufferARB(GL_ARRAY_BUFFER_ARB, cmd->getEntityMesh()->vbo_array_buffer);				RB_CheckForError();
		
		xglVertexAttribPointerNV(0, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_vertexes_ofs));		RB_CheckForError();
		xglVertexAttribPointerNV(8, 2, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_texcoords_ofs));	RB_CheckForError();
		xglVertexAttribPointerNV(14, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_tangents_ofs));	RB_CheckForError();
		xglVertexAttribPointerNV(15, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_binormals_ofs));	RB_CheckForError();
		xglVertexAttribPointerNV(2, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_normals_ofs));		RB_CheckForError();
	}
	else
	{
		if(gl_config.arb_vertex_buffer_object)
			xglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);		RB_CheckForError();
		
		xglVertexAttribPointerNV(0, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->vertexes[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(8, 2, GL_FLOAT, 0, &(cmd->getEntityMesh()->texcoords[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(14, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->tangents[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(15, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->binormals[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(2, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->normals[0]));		RB_CheckForError();
	}
	
	RB_SelectTexture(GL_TEXTURE0_ARB);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_diffusemap);	RB_CheckForError();
	RB_Bind(stage_diffusemap->image);
	
	RB_SelectTexture(GL_TEXTURE1_ARB);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_bumpmap);	RB_CheckForError();
	RB_Bind(stage_bumpmap->image);
	
	RB_SelectTexture(GL_TEXTURE2_ARB);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_heightmap);	RB_CheckForError();
	RB_Bind(stage_heightmap->image);
	
	RB_SelectTexture(GL_TEXTURE3_ARB);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_specularmap);	RB_CheckForError();
	RB_Bind(stage_specularmap->image);
	
	RB_SelectTexture(GL_TEXTURE4_ARB);
	RB_ModifyLightTextureMatrix(cmd->getLight(), cmd->getEntity(), stage_attenuationmap_xy);
	RB_Bind(stage_attenuationmap_xy->image);
	
	RB_SelectTexture(GL_TEXTURE5_ARB);
	xglMatrixMode(GL_TEXTURE);
	xglLoadIdentity();
	xglMatrixMode(GL_MODELVIEW);
	RB_Bind(stage_attenuationmap_z->image);
	
	RB_SelectTexture(GL_TEXTURE6_ARB);
	RB_ModifyLightCubeTextureMatrix(cmd->getLight(), cmd->getEntity(), stage_attenuationmap_cube);
	RB_Bind(stage_attenuationmap_cube->image);
	
	// set matrix_model_view_projection
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV,  0, GL_MODELVIEW_PROJECTION_NV, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_tcmod_diffuse
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV,  4, GL_TEXTURE0_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_tcmod_bump
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV,  8, GL_TEXTURE1_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_tcmod_height
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 12, GL_TEXTURE2_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_tcmod_specular
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 16, GL_TEXTURE3_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_tcmod_attenuation_xy_z
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 20, GL_TEXTURE4_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_tcmod_attenuation_cube
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 24, GL_TEXTURE6_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set view_origin in object space
	vec3_c view_origin = cmd->getEntity()->getTransform().affineInverse() * (r_origin - cmd->getEntity()->getShared().origin);
	xglProgramNamedParameter4fNV(rb_shader_lighting_DBHS_XY_Z_CUBE_fragment, strlen("view_origin"), (GLubyte*)"view_origin", view_origin[0], view_origin[1], view_origin[2], 0);	RB_CheckForError();
	
	// set light_origin in object space
	vec3_c light_origin = cmd->getEntity()->getTransform().affineInverse() * (cmd->getLight()->getOrigin() - cmd->getEntity()->getShared().origin);
	xglProgramNamedParameter4fNV(rb_shader_lighting_DBHS_XY_Z_CUBE_fragment, strlen("light_origin"), (GLubyte*)"light_origin", light_origin[0], light_origin[1], light_origin[2], 1);	RB_CheckForError();
	
	// set light_color
	vec4_c light_color;
	RB_ModifyColor(cmd->getLight()->getShared(), stage_attenuationmap_xy, light_color);
	xglProgramNamedParameter4fNV(rb_shader_lighting_DBHS_XY_Z_CUBE_fragment, strlen("light_color"), (GLubyte*)"light_color", light_color[0], light_color[1], light_color[2], 0);	RB_CheckForError();
	
	// set bump_scale
	float bump_scale = RB_Evaluate(cmd->getEntity()->getShared(), stage_bumpmap->bump_scale);
	xglProgramNamedParameter4fNV(rb_shader_lighting_DBHS_XY_Z_CUBE_fragment, strlen("bump_scale"), (GLubyte*)"bump_scale", bump_scale, 0, 0, 0);	RB_CheckForError();
	
	// set height_scale
	float height_scale = RB_Evaluate(cmd->getEntity()->getShared(), stage_heightmap->height_scale);
	xglProgramNamedParameter4fNV(rb_shader_lighting_DBHS_XY_Z_CUBE_fragment, strlen("height_scale"), (GLubyte*)"height_scale", height_scale, 0, 0, 0);	RB_CheckForError();
	
	// set height_bias
	float height_bias = RB_Evaluate(cmd->getEntity()->getShared(), stage_heightmap->height_bias);
	xglProgramNamedParameter4fNV(rb_shader_lighting_DBHS_XY_Z_CUBE_fragment, strlen("height_bias"), (GLubyte*)"height_bias", height_bias, 0, 0, 0);	RB_CheckForError();
	
	// set specular_exponent
	float specular_exponent = X_max(0, RB_Evaluate(cmd->getEntity()->getShared(), stage_specularmap->specular_exponent));
	xglProgramNamedParameter4fNV(rb_shader_lighting_DBHS_XY_Z_CUBE_fragment, strlen("specular_exponent"), (GLubyte*)"specular_exponent", specular_exponent, 0, 0, 0);	RB_CheckForError();

	RB_FlushMesh(cmd->getEntityMesh());
}


void		RB_EnableShader_lighting_DBS_XY_Z_CUBE()
{
	xglEnable(GL_VERTEX_PROGRAM_NV);							RB_CheckForError();
	xglBindProgramNV(GL_VERTEX_PROGRAM_NV, rb_shader_lighting_DBS_XY_Z_CUBE_vertex);	RB_CheckForError();
		
	xglEnable(GL_FRAGMENT_PROGRAM_NV);							RB_CheckForError();
	xglBindProgramNV(GL_FRAGMENT_PROGRAM_NV, rb_shader_lighting_DBS_XY_Z_CUBE_fragment);	RB_CheckForError();

			
	RB_SelectTexture(GL_TEXTURE0_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE1_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE2_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE3_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE4_ARB);
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE5_ARB);
	xglEnable(GL_TEXTURE_CUBE_MAP_ARB);	RB_CheckForError();
	
	
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY0_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY8_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY14_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY15_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY2_NV);	RB_CheckForError();
}

void		RB_DisableShader_lighting_DBS_XY_Z_CUBE()
{
	xglDisable(GL_VERTEX_PROGRAM_NV);	RB_CheckForError();
	xglDisable(GL_FRAGMENT_PROGRAM_NV);	RB_CheckForError();
	
	
	RB_SelectTexture(GL_TEXTURE0_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE1_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE2_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE3_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE4_ARB);
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	
	RB_SelectTexture(GL_TEXTURE5_ARB);
	xglDisable(GL_TEXTURE_CUBE_MAP_ARB);	RB_CheckForError();
	
	
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY0_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY8_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY14_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY15_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY2_NV);	RB_CheckForError();
}

void		RB_RenderCommand_lighting_DBS_XY_Z_CUBE(const r_command_t *cmd,	const r_shader_stage_c *stage_diffusemap,
										const r_shader_stage_c *stage_bumpmap,
										const r_shader_stage_c *stage_specularmap,
										const r_shader_stage_c *stage_attenuationmap_xy,
										const r_shader_stage_c *stage_attenuationmap_z,
										const r_shader_stage_c *stage_attenuationmap_cube)
{
	RB_SetShaderStageState(cmd->getEntity(), stage_diffusemap, RENDER_TYPE_LIGHTING_DBS);		RB_CheckForError();
	
	if(gl_config.arb_vertex_buffer_object && cmd->getEntityMesh()->vbo_array_buffer)
	{
		xglBindBufferARB(GL_ARRAY_BUFFER_ARB, cmd->getEntityMesh()->vbo_array_buffer);				RB_CheckForError();
		
		xglVertexAttribPointerNV(0, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_vertexes_ofs));		RB_CheckForError();
		xglVertexAttribPointerNV(8, 2, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_texcoords_ofs));	RB_CheckForError();
		xglVertexAttribPointerNV(14, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_tangents_ofs));	RB_CheckForError();
		xglVertexAttribPointerNV(15, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_binormals_ofs));	RB_CheckForError();
		xglVertexAttribPointerNV(2, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_normals_ofs));		RB_CheckForError();
	}
	else
	{
		if(gl_config.arb_vertex_buffer_object)
			xglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);		RB_CheckForError();
		
		xglVertexAttribPointerNV(0, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->vertexes[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(8, 2, GL_FLOAT, 0, &(cmd->getEntityMesh()->texcoords[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(14, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->tangents[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(15, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->binormals[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(2, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->normals[0]));		RB_CheckForError();
	}
	
	RB_SelectTexture(GL_TEXTURE0_ARB);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_diffusemap);	RB_CheckForError();
	RB_Bind(stage_diffusemap->image);
	
	RB_SelectTexture(GL_TEXTURE1_ARB);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_bumpmap);	RB_CheckForError();
	RB_Bind(stage_bumpmap->image);
	
	RB_SelectTexture(GL_TEXTURE2_ARB);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_specularmap);	RB_CheckForError();
	RB_Bind(stage_specularmap->image);
	
	RB_SelectTexture(GL_TEXTURE3_ARB);
	RB_ModifyLightTextureMatrix(cmd->getLight(), cmd->getEntity(), stage_attenuationmap_xy);
	RB_Bind(stage_attenuationmap_xy->image);
	
	RB_SelectTexture(GL_TEXTURE4_ARB);
	xglMatrixMode(GL_TEXTURE);
	xglLoadIdentity();
	xglMatrixMode(GL_MODELVIEW);
	RB_Bind(stage_attenuationmap_z->image);
	
	RB_SelectTexture(GL_TEXTURE5_ARB);
	RB_ModifyLightCubeTextureMatrix(cmd->getLight(), cmd->getEntity(), stage_attenuationmap_cube);
	RB_Bind(stage_attenuationmap_cube->image);
	
	// set matrix_model_view_projection
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV,  0, GL_MODELVIEW_PROJECTION_NV, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_tcmod_diffuse
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV,  4, GL_TEXTURE0_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_tcmod_bump
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV,  8, GL_TEXTURE1_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_tcmod_specular
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 12, GL_TEXTURE2_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_tcmod_attenuation_xy_z
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 16, GL_TEXTURE3_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_tcmod_attenuation_cube
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 20, GL_TEXTURE5_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set view_origin in object space
	vec3_c view_origin = cmd->getEntity()->getTransform().affineInverse() * (r_origin - cmd->getEntity()->getShared().origin);
	xglProgramNamedParameter4fNV(rb_shader_lighting_DBS_XY_Z_CUBE_fragment, strlen("view_origin"), (GLubyte*)"view_origin", view_origin[0], view_origin[1], view_origin[2], 0);	RB_CheckForError();
	
	// set light_origin in object space
	vec3_c light_origin = cmd->getEntity()->getTransform().affineInverse() * (cmd->getLight()->getOrigin() - cmd->getEntity()->getShared().origin);
	xglProgramNamedParameter4fNV(rb_shader_lighting_DBS_XY_Z_CUBE_fragment, strlen("light_origin"), (GLubyte*)"light_origin", light_origin[0], light_origin[1], light_origin[2], 1);	RB_CheckForError();
	
	// set light_color
	vec4_c light_color;
	RB_ModifyColor(cmd->getLight()->getShared(), stage_attenuationmap_xy, light_color);
	xglProgramNamedParameter4fNV(rb_shader_lighting_DBS_XY_Z_CUBE_fragment, strlen("light_color"), (GLubyte*)"light_color", light_color[0], light_color[1], light_color[2], 0);	RB_CheckForError();
	
	// set bump_scale
	float bump_scale = RB_Evaluate(cmd->getEntity()->getShared(), stage_bumpmap->bump_scale);
	xglProgramNamedParameter4fNV(rb_shader_lighting_DBS_XY_Z_CUBE_fragment, strlen("bump_scale"), (GLubyte*)"bump_scale", bump_scale, 0, 0, 0);	RB_CheckForError();
	
	// set specular_exponent
	float specular_exponent = X_max(0, RB_Evaluate(cmd->getEntity()->getShared(), stage_specularmap->specular_exponent));
	xglProgramNamedParameter4fNV(rb_shader_lighting_DBS_XY_Z_CUBE_fragment, strlen("specular_exponent"), (GLubyte*)"specular_exponent", specular_exponent, 0, 0, 0);	RB_CheckForError();

	RB_FlushMesh(cmd->getEntityMesh());
}


						
void		RB_EnableShader_reflection_C()
{
	xglEnable(GL_VERTEX_PROGRAM_NV);						RB_CheckForError();
	xglBindProgramNV(GL_VERTEX_PROGRAM_NV, rb_shader_reflection_C_vertex);		RB_CheckForError();
	
	xglEnable(GL_FRAGMENT_PROGRAM_NV);						RB_CheckForError();
	xglBindProgramNV(GL_FRAGMENT_PROGRAM_NV, rb_shader_reflection_C_fragment);	RB_CheckForError();

			
	RB_SelectTexture(GL_TEXTURE0_ARB);
	xglEnable(GL_TEXTURE_CUBE_MAP_ARB);		RB_CheckForError();
	
	
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY0_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY2_NV);	RB_CheckForError();
}

void		RB_DisableShader_reflection_C()
{
	xglDisable(GL_VERTEX_PROGRAM_NV);	RB_CheckForError();
	xglDisable(GL_FRAGMENT_PROGRAM_NV);	RB_CheckForError();
	
	
	RB_SelectTexture(GL_TEXTURE0_ARB);
	xglDisable(GL_TEXTURE_CUBE_MAP_ARB);		RB_CheckForError();
	
	
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY0_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY2_NV);	RB_CheckForError();
}

void		RB_RenderCommand_reflection_C(const r_command_t *cmd,		const r_shader_stage_c *stage_colormap)
{
	RB_SetShaderStageState(cmd->getEntity(), stage_colormap, RENDER_TYPE_REFLECTION);
	
	if(gl_config.arb_vertex_buffer_object && cmd->getEntityMesh()->vbo_array_buffer)
	{
		xglBindBufferARB(GL_ARRAY_BUFFER_ARB, cmd->getEntityMesh()->vbo_array_buffer);				RB_CheckForError();
		
		xglVertexAttribPointerNV(0, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_vertexes_ofs));		RB_CheckForError();
		xglVertexAttribPointerNV(2, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_normals_ofs));		RB_CheckForError();
	}
	else
	{
		if(gl_config.arb_vertex_buffer_object)
			xglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);		RB_CheckForError();
		
		xglVertexAttribPointerNV(0, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->vertexes[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(2, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->normals[0]));		RB_CheckForError();
	}

	RB_SelectTexture(GL_TEXTURE0_ARB);
	xglMatrixMode(GL_TEXTURE);
	xglLoadTransposeMatrixf(&rb_matrix_model[0][0]);
	xglMatrixMode(GL_MODELVIEW);
	RB_Bind(stage_colormap->image);
	
	// set matrix_model_view_projection
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV,  0, GL_MODELVIEW_PROJECTION_NV, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_model
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV,  4, GL_TEXTURE0_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set view_origin in world space
	const vec3_c& view_origin = r_origin;
	xglProgramNamedParameter4fNV(rb_shader_reflection_C_fragment, strlen("view_origin"), (GLubyte*)"view_origin", view_origin[0], view_origin[1], view_origin[2], 0);	RB_CheckForError();
	
	RB_FlushMesh(cmd->getEntityMesh());
}



void		RB_EnableShader_refraction_C()
{
	xglEnable(GL_VERTEX_PROGRAM_NV);						RB_CheckForError();
	xglBindProgramNV(GL_VERTEX_PROGRAM_NV, rb_shader_refraction_C_vertex);		RB_CheckForError();
	
	xglEnable(GL_FRAGMENT_PROGRAM_NV);						RB_CheckForError();
	xglBindProgramNV(GL_FRAGMENT_PROGRAM_NV, rb_shader_refraction_C_fragment);	RB_CheckForError();

			
	RB_SelectTexture(GL_TEXTURE0_ARB);
	xglEnable(GL_TEXTURE_CUBE_MAP_ARB);		RB_CheckForError();
	
	
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY0_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY2_NV);	RB_CheckForError();
}

void		RB_DisableShader_refraction_C()
{
	xglDisable(GL_VERTEX_PROGRAM_NV);	RB_CheckForError();
	xglDisable(GL_FRAGMENT_PROGRAM_NV);	RB_CheckForError();
	
	
	RB_SelectTexture(GL_TEXTURE0_ARB);
	xglDisable(GL_TEXTURE_CUBE_MAP_ARB);		RB_CheckForError();
	
	
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY0_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY2_NV);	RB_CheckForError();
}

void		RB_RenderCommand_refraction_C(const r_command_t *cmd,		const r_shader_stage_c *stage_colormap)
{
	RB_SetShaderStageState(cmd->getEntity(), stage_colormap, RENDER_TYPE_REFRACTION);
	
	if(gl_config.arb_vertex_buffer_object && cmd->getEntityMesh()->vbo_array_buffer)
	{
		xglBindBufferARB(GL_ARRAY_BUFFER_ARB, cmd->getEntityMesh()->vbo_array_buffer);				RB_CheckForError();
		
		xglVertexAttribPointerNV(0, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_vertexes_ofs));		RB_CheckForError();
		xglVertexAttribPointerNV(2, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_normals_ofs));		RB_CheckForError();
	}
	else
	{
		if(gl_config.arb_vertex_buffer_object)
			xglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);		RB_CheckForError();
		
		xglVertexAttribPointerNV(0, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->vertexes[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(2, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->normals[0]));		RB_CheckForError();
	}

	RB_SelectTexture(GL_TEXTURE0_ARB);
	xglMatrixMode(GL_TEXTURE);
	xglLoadTransposeMatrixf(&rb_matrix_model[0][0]);
	xglMatrixMode(GL_MODELVIEW);
	RB_Bind(stage_colormap->image);
	
	// set matrix_model_view_projection
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV,  0, GL_MODELVIEW_PROJECTION_NV, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_model
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV,  4, GL_TEXTURE0_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set view_origin in world space
	const vec3_c& view_origin = r_origin;
	xglProgramNamedParameter4fNV(rb_shader_refraction_C_fragment, strlen("view_origin"), (GLubyte*)"view_origin", view_origin[0], view_origin[1], view_origin[2], 0);	RB_CheckForError();
	
	// set refraction_index
	float refraction_index = RB_Evaluate(cmd->getEntity()->getShared(), stage_colormap->refraction_index);
	xglProgramNamedParameter4fNV(rb_shader_refraction_C_fragment, strlen("refraction_index"), (GLubyte*)"refraction_index", refraction_index, 0, 0, 0);		RB_CheckForError();
	
	// set fresnel_power
	float fresnel_power = RB_Evaluate(cmd->getEntity()->getShared(), stage_colormap->fresnel_power);
	xglProgramNamedParameter4fNV(rb_shader_refraction_C_fragment, strlen("fresnel_power"), (GLubyte*)"fresnel_power", fresnel_power, 0, 0, 0);		RB_CheckForError();
	
	// set fresnel_scale
	float fresnel_scale = RB_Evaluate(cmd->getEntity()->getShared(), stage_colormap->fresnel_scale);
	xglProgramNamedParameter4fNV(rb_shader_refraction_C_fragment, strlen("fresnel_scale"), (GLubyte*)"fresnel_scale", fresnel_scale, 0, 0, 0);		RB_CheckForError();
	
	// set fresnel_bias
	float fresnel_bias = RB_Evaluate(cmd->getEntity()->getShared(), stage_colormap->fresnel_bias);
	xglProgramNamedParameter4fNV(rb_shader_refraction_C_fragment, strlen("fresnel_bias"), (GLubyte*)"fresnel_bias", fresnel_bias, 0, 0, 0);		RB_CheckForError();
	
	RB_FlushMesh(cmd->getEntityMesh());
}




void		RB_EnableShader_dispersion_C()
{
	xglEnable(GL_VERTEX_PROGRAM_NV);						RB_CheckForError();
	xglBindProgramNV(GL_VERTEX_PROGRAM_NV, rb_shader_dispersion_C_vertex);		RB_CheckForError();
	
	xglEnable(GL_FRAGMENT_PROGRAM_NV);						RB_CheckForError();
	xglBindProgramNV(GL_FRAGMENT_PROGRAM_NV, rb_shader_dispersion_C_fragment);	RB_CheckForError();

			
	RB_SelectTexture(GL_TEXTURE0_ARB);
	xglEnable(GL_TEXTURE_CUBE_MAP_ARB);		RB_CheckForError();
	
	
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY0_NV);	RB_CheckForError();
	xglEnableClientState(GL_VERTEX_ATTRIB_ARRAY2_NV);	RB_CheckForError();
}

void		RB_DisableShader_dispersion_C()
{
	xglDisable(GL_VERTEX_PROGRAM_NV);	RB_CheckForError();
	xglDisable(GL_FRAGMENT_PROGRAM_NV);	RB_CheckForError();
	
	
	RB_SelectTexture(GL_TEXTURE0_ARB);
	xglDisable(GL_TEXTURE_CUBE_MAP_ARB);		RB_CheckForError();
	
	
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY0_NV);	RB_CheckForError();
	xglDisableClientState(GL_VERTEX_ATTRIB_ARRAY2_NV);	RB_CheckForError();
}

void		RB_RenderCommand_dispersion_C(const r_command_t *cmd,		const r_shader_stage_c *stage_colormap)
{
	RB_SetShaderStageState(cmd->getEntity(), stage_colormap, RENDER_TYPE_DISPERSION);
	
	if(gl_config.arb_vertex_buffer_object && cmd->getEntityMesh()->vbo_array_buffer)
	{
		xglBindBufferARB(GL_ARRAY_BUFFER_ARB, cmd->getEntityMesh()->vbo_array_buffer);				RB_CheckForError();
		
		xglVertexAttribPointerNV(0, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_vertexes_ofs));		RB_CheckForError();
		xglVertexAttribPointerNV(2, 3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_normals_ofs));		RB_CheckForError();
	}
	else
	{
		if(gl_config.arb_vertex_buffer_object)
			xglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);		RB_CheckForError();
		
		xglVertexAttribPointerNV(0, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->vertexes[0]));		RB_CheckForError();
		xglVertexAttribPointerNV(2, 3, GL_FLOAT, 0, &(cmd->getEntityMesh()->normals[0]));		RB_CheckForError();
	}

	RB_SelectTexture(GL_TEXTURE0_ARB);
	xglMatrixMode(GL_TEXTURE);
	xglLoadTransposeMatrixf(&rb_matrix_model[0][0]);
	xglMatrixMode(GL_MODELVIEW);
	RB_Bind(stage_colormap->image);
	
	// set matrix_model_view_projection
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV,  0, GL_MODELVIEW_PROJECTION_NV, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set matrix_model
	xglTrackMatrixNV(GL_VERTEX_PROGRAM_NV,  4, GL_TEXTURE0_ARB, GL_IDENTITY_NV);	RB_CheckForError();
	
	// set view_origin in world space
	const vec3_c& view_origin = r_origin;
	xglProgramNamedParameter4fNV(rb_shader_dispersion_C_fragment, strlen("view_origin"), (GLubyte*)"view_origin", view_origin[0], view_origin[1], view_origin[2], 0);	RB_CheckForError();
	
	// set eta_ratio
	float eta	= RB_Evaluate(cmd->getEntity()->getShared(), stage_colormap->eta);
	float eta_delta	= RB_Evaluate(cmd->getEntity()->getShared(), stage_colormap->eta_delta);
	xglProgramNamedParameter4fNV(rb_shader_dispersion_C_fragment, strlen("eta_ratio"), (GLubyte*)"eta_ratio", eta, eta + eta_delta, eta + (eta_delta * 2), 0);	RB_CheckForError();
	
	// set fresnel_power
	float fresnel_power = RB_Evaluate(cmd->getEntity()->getShared(), stage_colormap->fresnel_power);
	xglProgramNamedParameter4fNV(rb_shader_dispersion_C_fragment, strlen("fresnel_power"), (GLubyte*)"fresnel_power", fresnel_power, 0, 0, 0);		RB_CheckForError();
	
	// set fresnel_scale
	float fresnel_scale = RB_Evaluate(cmd->getEntity()->getShared(), stage_colormap->fresnel_scale);
	xglProgramNamedParameter4fNV(rb_shader_dispersion_C_fragment, strlen("fresnel_scale"), (GLubyte*)"fresnel_scale", fresnel_scale, 0, 0, 0);		RB_CheckForError();
	
	// set fresnel_bias
	float fresnel_bias = RB_Evaluate(cmd->getEntity()->getShared(), stage_colormap->fresnel_bias);
	xglProgramNamedParameter4fNV(rb_shader_dispersion_C_fragment, strlen("fresnel_bias"), (GLubyte*)"fresnel_bias", fresnel_bias, 0, 0, 0);		RB_CheckForError();
	
	RB_FlushMesh(cmd->getEntityMesh());
}


