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
#ifndef R_BACKEND_H
#define R_BACKEND_H

/// includes ===================================================================
// system -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
#include "r_local.h"



//
// usefull matrices to handle transformations
//
extern matrix_c			rb_matrix_framebuffer_to_vid;
extern matrix_c			rb_matrix_view;
extern matrix_c			rb_matrix_model;


void		RB_CheckOpenGLExtensions();

void		RB_InitGPUShaders();
void		RB_ShutdownGPUShaders();


void		RB_EnableShader_generic();
void		RB_DisableShader_generic();
void		RB_RenderCommand_generic(const r_command_t *cmd,			const r_shader_stage_c *stage);


void		RB_EnableShader_zfill();
void		RB_DisableShader_zfill();
void		RB_RenderCommand_zfill(const r_command_t *cmd,				const r_shader_stage_c *stage);


//
// static lighting
//
void		RB_EnableShader_lighting_R();
void		RB_DisableShader_lighting_R();
void		RB_RenderCommand_lighting_R(const r_command_t *cmd,			const r_shader_stage_c *stage_diffusemap,
											const r_shader_stage_c *stage_lightmap,
											const r_shader_stage_c *stage_deluxemap);
										
			
void		RB_EnableShader_lighting_RB();
void		RB_DisableShader_lighting_RB();
void		RB_RenderCommand_lighting_RB(const r_command_t *cmd,			const r_shader_stage_c *stage_diffusemap,
											const r_shader_stage_c *stage_bumpmap,
											const r_shader_stage_c *stage_lightmap,
											const r_shader_stage_c *stage_deluxemap);
										

void		RB_EnableShader_lighting_RBH();
void		RB_DisableShader_lighting_RBH();
void		RB_RenderCommand_lighting_RBH(const r_command_t *cmd,			const r_shader_stage_c *stage_diffusemap,
											const r_shader_stage_c *stage_bumpmap,
											const r_shader_stage_c *stage_lightmap,
											const r_shader_stage_c *stage_deluxemap);
										

void		RB_EnableShader_lighting_RBHS();
void		RB_DisableShader_lighting_RBHS();
void		RB_RenderCommand_lighting_RBHS(const r_command_t *cmd,			const r_shader_stage_c *stage_diffusemap,
											const r_shader_stage_c *stage_bumpmap,
											const r_shader_stage_c *stage_specularmap,
											const r_shader_stage_c *stage_lightmap,
											const r_shader_stage_c *stage_deluxemap);
										
									
void		RB_EnableShader_lighting_RBS();
void		RB_DisableShader_lighting_RBS();
void		RB_RenderCommand_lighting_RBS(const r_command_t *cmd,			const r_shader_stage_c *stage_diffusemap,
											const r_shader_stage_c *stage_bumpmap,
											const r_shader_stage_c *stage_specularmap,
											const r_shader_stage_c *stage_lightmap,
											const r_shader_stage_c *stage_deluxemap);

//
// dynamic lighting
//
void		RB_EnableShader_lighting_D_omni();
void		RB_DisableShader_lighting_D_omni();
void		RB_RenderCommand_lighting_D_omni(const r_command_t *cmd,		const r_shader_stage_c *stage_diffusemap,
											const r_shader_stage_c *stage_attenuationmap_xy,
											const r_shader_stage_c *stage_attenuationmap_z,
											const r_shader_stage_c *stage_attenuationmap_cube);
											
void		RB_EnableShader_lighting_D_proj();
void		RB_DisableShader_lighting_D_proj();
void		RB_RenderCommand_lighting_D_proj(const r_command_t *cmd,		const r_shader_stage_c *stage_diffusemap,
											const r_shader_stage_c *stage_attenuationmap_xy,
											const r_shader_stage_c *stage_attenuationmap_z);


void		RB_EnableShader_lighting_DB_omni();
void		RB_DisableShader_lighting_DB_omni();
void		RB_RenderCommand_lighting_DB_omni(const r_command_t *cmd,		const r_shader_stage_c *stage_diffusemap,
											const r_shader_stage_c *stage_bumpmap,
											const r_shader_stage_c *stage_attenuationmap_xy,
											const r_shader_stage_c *stage_attenuationmap_z,
											const r_shader_stage_c *stage_attenuationmap_cube);
										

void		RB_EnableShader_lighting_DBH_omni();
void		RB_DisableShader_lighting_DBH_omni();
void		RB_RenderCommand_lighting_DBH_omni(const r_command_t *cmd,		const r_shader_stage_c *stage_diffusemap,
											const r_shader_stage_c *stage_bumpmap,
											const r_shader_stage_c *stage_attenuationmap_xy,
											const r_shader_stage_c *stage_attenuationmap_z,
											const r_shader_stage_c *stage_attenuationmap_cube);


void		RB_EnableShader_lighting_DBHS_omni();
void		RB_DisableShader_lighting_DBHS_omni();			
void		RB_RenderCommand_lighting_DBHS_omni(const r_command_t *cmd,		const r_shader_stage_c *stage_diffusemap,
											const r_shader_stage_c *stage_bumpmap,
											const r_shader_stage_c *stage_specularmap,
											const r_shader_stage_c *stage_attenuationmap_xy,
											const r_shader_stage_c *stage_attenuationmap_z,
											const r_shader_stage_c *stage_attenuationmap_cube);
								
								
void		RB_EnableShader_lighting_DBS_omni();
void		RB_DisableShader_lighting_DBS_omni();
void		RB_RenderCommand_lighting_DBS_omni(const r_command_t *cmd,		const r_shader_stage_c *stage_diffusemap,
											const r_shader_stage_c *stage_bumpmap,
											const r_shader_stage_c *stage_specularmap,
											const r_shader_stage_c *stage_attenuationmap_xy,
											const r_shader_stage_c *stage_attenuationmap_z,
											const r_shader_stage_c *stage_attenuationmap_cube);
																						
//
// environment mapping
//
void		RB_EnableShader_reflection_C();
void		RB_DisableShader_reflection_C();
void		RB_RenderCommand_reflection_C(const r_command_t *cmd,			const r_shader_stage_c *stage_colormap);


void		RB_EnableShader_refraction_C();
void		RB_DisableShader_refraction_C();
void		RB_RenderCommand_refraction_C(const r_command_t *cmd,			const r_shader_stage_c *stage_colormap);


void		RB_EnableShader_dispersion_C();
void		RB_DisableShader_dispersion_C();
void		RB_RenderCommand_dispersion_C(const r_command_t *cmd,			const r_shader_stage_c *stage_colormap);


void		RB_EnableShader_liquid_C();
void		RB_DisableShader_liquid_C();
void		RB_RenderCommand_liquid_C(const r_command_t *cmd,			const r_shader_stage_c *stage_colormap);


void		RB_EnableShader_heathaze();
void		RB_DisableShader_heathaze();
void		RB_RenderCommand_heathaze(const r_command_t *cmd,			const r_shader_stage_c *stage_colormap);


#endif // R_BACKEND_H





