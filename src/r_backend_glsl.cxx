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
// shared -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
#include "r_backend.h"

// xreal --------------------------------------------------------------------

enum
{
	VATTRIB_VERTEX		= (1<<0),
	VATTRIB_TEX0		= (1<<1),
	VATTRIB_TEX1		= (1<<2),
	VATTRIB_TANGENT		= (1<<3),
	VATTRIB_BINORMAL	= (1<<4),
	VATTRIB_NORMAL		= (1<<5),
	VATTRIB_LIGHT		= (1<<6),
	VATTRIB_COLOR		= (1<<7)
};


/*
================================================================================
			ABSTRACT UNIFORM PARAMETER HELPER CLASSES
================================================================================
*/
class u_view_origin_a
{
private:
	u_view_origin_a();
	
protected:
	u_view_origin_a(GLhandleARB handle)
	{
		_u_view_origin	= xglGetUniformLocationARB(handle, "u_view_origin");	RB_CheckForError();
	}
	
public:
	void	setUniform_view_origin(const r_command_t *cmd)
	{
		setUniform_view_origin_inObjectSpace(cmd);
	}

	void	setUniform_view_origin_inObjectSpace(const r_command_t *cmd)
	{
		vec3_c view_origin = cmd->getEntity()->getTransform().affineInverse() * (r_origin - cmd->getEntity()->getShared().origin);
		xglUniform3fARB(_u_view_origin, view_origin[0], view_origin[1], view_origin[2]);	RB_CheckForError();
	}
	
	void	setUniform_view_origin_inWorldSpace(const r_command_t *cmd)
	{
		const vec3_c& view_origin = r_origin;
		xglUniform3fARB(_u_view_origin, view_origin[0], view_origin[1], view_origin[2]);	RB_CheckForError();
	}
	
private:
	uint_t		_u_view_origin;
};


class u_light_origin_a
{
private:
	u_light_origin_a();
	
protected:
	u_light_origin_a(GLhandleARB handle)
	{
		_u_light_origin	= xglGetUniformLocationARB(handle, "u_light_origin");	RB_CheckForError();
	}
	
public:
	// set in object space
	void	setUniform_light_origin(const r_command_t *cmd)
	{
		vec3_c light_origin = cmd->getEntity()->getTransform().affineInverse() * (cmd->getLight()->getOrigin() - cmd->getEntity()->getShared().origin);
		xglUniform3fARB(_u_light_origin, light_origin[0], light_origin[1], light_origin[2]);	RB_CheckForError();
	}
	
private:
	uint_t		_u_light_origin;
};


class u_light_color_a
{
private:
	u_light_color_a();
	
protected:
	u_light_color_a(GLhandleARB handle)
	{
		_u_light_color	= xglGetUniformLocationARB(handle, "u_light_color");	RB_CheckForError();
	}
	
public:
	void	setUniform_light_color(const r_command_t *cmd, const r_shader_stage_c *stage_attenuationmap_xy)
	{
		vec4_c light_color;
		RB_ModifyColor(cmd->getLight()->getShared(), stage_attenuationmap_xy, light_color);
		xglUniform3fARB(_u_light_color, light_color[0], light_color[1], light_color[2]);	RB_CheckForError();
	}
	
private:
	uint_t		_u_light_color;
};


class u_bump_scale_a
{
private:
	u_bump_scale_a();
	
protected:
	u_bump_scale_a(GLhandleARB handle)
	{
		_u_bump_scale	= xglGetUniformLocationARB(handle, "u_bump_scale");	RB_CheckForError();
	}
	
public:
	void	setUniform_bump_scale(const r_command_t *cmd, const r_shader_stage_c *stage_bumpmap)
	{
		float bump_scale = RB_Evaluate(cmd->getEntity()->getShared(), stage_bumpmap->bump_scale, 1.0);
		xglUniform1fARB(_u_bump_scale,	bump_scale);	RB_CheckForError();
	}
	
private:
	uint_t		_u_bump_scale;
};


class u_parallax_a
{
private:
	u_parallax_a();
	
protected:
	u_parallax_a(GLhandleARB handle)
	{
		_u_height_scale	= xglGetUniformLocationARB(handle, "u_height_scale");	RB_CheckForError();
		_u_height_bias	= xglGetUniformLocationARB(handle, "u_height_bias");	RB_CheckForError();
	}
	
public:
	void	setUniform_height_scale(const r_command_t *cmd, const r_shader_stage_c *stage_bumpmap)
	{
		float height_scale = RB_Evaluate(cmd->getEntity()->getShared(), stage_bumpmap->height_scale, 0.04);
		xglUniform1fARB(_u_height_scale, height_scale);	RB_CheckForError();
	}
	
	void	setUniform_height_bias(const r_command_t *cmd, const r_shader_stage_c *stage_bumpmap)
	{
		float height_bias = RB_Evaluate(cmd->getEntity()->getShared(), stage_bumpmap->height_bias, -0.02);
		xglUniform1fARB(_u_height_bias, height_bias);	RB_CheckForError();
	}
	
private:
	uint_t		_u_height_scale;
	uint_t		_u_height_bias;
};


class u_specular_exponent_a
{
private:
	u_specular_exponent_a();
	
protected:
	u_specular_exponent_a(GLhandleARB handle)
	{
		_u_specular_exponent	= xglGetUniformLocationARB(handle, "u_specular_exponent");	RB_CheckForError();
	}
	
public:
	void	setUniform_specular_exponent(const r_command_t *cmd, const r_shader_stage_c *stage_specularmap)
	{
		float specular_exponent = X_max(0, RB_Evaluate(cmd->getEntity()->getShared(), stage_specularmap->specular_exponent, 32.0));
		xglUniform1fARB(_u_specular_exponent, specular_exponent);	RB_CheckForError();
	}
	
private:
	uint_t		_u_specular_exponent;
};


class u_refraction_index_a
{
private:
	u_refraction_index_a();
	
protected:
	u_refraction_index_a(GLhandleARB handle)
	{
		_u_refraction_index	= xglGetUniformLocationARB(handle, "u_refraction_index");	RB_CheckForError();
	}
	
public:
	void	setUniform_refraction_index(const r_command_t *cmd, const r_shader_stage_c *stage)
	{
		float refraction_index = RB_Evaluate(cmd->getEntity()->getShared(), stage->refraction_index, 1.0);
		xglUniform1fARB(_u_refraction_index, refraction_index);		RB_CheckForError();
	}
	
private:
	uint_t		_u_refraction_index;
};


class u_eta_ratio_a
{
private:
	u_eta_ratio_a();
	
protected:
	u_eta_ratio_a(GLhandleARB handle)
	{
		_u_eta_ratio	= xglGetUniformLocationARB(handle, "u_eta_ratio");	RB_CheckForError();
	}
	
public:
	void	setUniform_eta_ratio(const r_command_t *cmd, const r_shader_stage_c *stage)
	{
		float eta	= RB_Evaluate(cmd->getEntity()->getShared(), stage->eta, 1.1);
		float eta_delta	= RB_Evaluate(cmd->getEntity()->getShared(), stage->eta_delta, -0.02);
		xglUniform3fARB(_u_eta_ratio, eta, eta + eta_delta, eta + (eta_delta * 2));	RB_CheckForError();
	}
	
private:
	uint_t		_u_eta_ratio;
};


class u_fresnel_a
{
private:
	u_fresnel_a();
	
protected:
	u_fresnel_a(GLhandleARB handle)
	{
		_u_fresnel_power	= xglGetUniformLocationARB(handle, "u_fresnel_power");	RB_CheckForError();
		_u_fresnel_scale	= xglGetUniformLocationARB(handle, "u_fresnel_scale");	RB_CheckForError();
		_u_fresnel_bias		= xglGetUniformLocationARB(handle, "u_fresnel_bias");	RB_CheckForError();
	}
	
public:
	void	setUniform_fresnel_power(const r_command_t *cmd, const r_shader_stage_c *stage)
	{
		float fresnel_power = RB_Evaluate(cmd->getEntity()->getShared(), stage->fresnel_power, 2.0);
		xglUniform1fARB(_u_fresnel_power, fresnel_power);	RB_CheckForError();
	}
	
	void	setUniform_fresnel_scale(const r_command_t *cmd, const r_shader_stage_c *stage)
	{
		float fresnel_scale = RB_Evaluate(cmd->getEntity()->getShared(), stage->fresnel_scale, 2.0);
		xglUniform1fARB(_u_fresnel_scale, fresnel_scale);	RB_CheckForError();
	}
	
	void	setUniform_fresnel_bias(const r_command_t *cmd, const r_shader_stage_c *stage)
	{
		float fresnel_bias = RB_Evaluate(cmd->getEntity()->getShared(), stage->fresnel_bias, 1.0);
		xglUniform1fARB(_u_fresnel_bias, fresnel_bias);		RB_CheckForError();
	}
	
private:
	uint_t		_u_fresnel_power;
	uint_t		_u_fresnel_scale;
	uint_t		_u_fresnel_bias;
};


class u_frame_buffer_a
{
private:
	u_frame_buffer_a();
	
protected:
	u_frame_buffer_a(GLhandleARB handle)
	{
		_u_fbuf_scale	= xglGetUniformLocationARB(handle, "u_fbuf_scale");	RB_CheckForError();
		_u_npot_scale	= xglGetUniformLocationARB(handle, "u_npot_scale");	RB_CheckForError();
	}
	
public:
	void	setUniform_fbuf_scale(const r_command_t *cmd)
	{
		xglUniform2fARB(_u_fbuf_scale, X_recip((float)vid.width), X_recip((float)vid.height));	RB_CheckForError();
	}
	
	void	setUniform_npot_scale(const r_command_t *cmd)
	{
		xglUniform2fARB(_u_npot_scale, (float)vid.width/(float)r_img_currentrender->getWidth(), (float)vid.height/(float)r_img_currentrender->getHeight());	RB_CheckForError();
	}
	
private:
	uint_t		_u_fbuf_scale;
	uint_t		_u_npot_scale;
};


class u_deform_magnitude_a
{
private:
	u_deform_magnitude_a();
	
protected:
	u_deform_magnitude_a(GLhandleARB handle)
	{
		_u_deform_magnitude	= xglGetUniformLocationARB(handle, "u_deform_magnitude");	RB_CheckForError();
	}
	
public:
	void	setUniform_deform_magnitude(const r_command_t *cmd, const r_shader_stage_c *stage)
	{
		float value = X_max(0, RB_Evaluate(cmd->getEntity()->getShared(), stage->deform_magnitude, 1.0));
		xglUniform1fARB(_u_deform_magnitude, value);	RB_CheckForError();
	}
	
private:
	uint_t		_u_deform_magnitude;
};


class u_fog_a
{
private:
	u_fog_a();
	
protected:
	u_fog_a(GLhandleARB handle)
	{
		_u_fog_density	= xglGetUniformLocationARB(handle, "u_fog_density");	RB_CheckForError();
		_u_fog_color	= xglGetUniformLocationARB(handle, "u_fog_color");	RB_CheckForError();
	}
	
public:
	void	setUniform_fog_density(const r_command_t *cmd, const r_shader_stage_c *stage)
	{
		xglUniform1fARB(_u_fog_density, 0.00125f);	RB_CheckForError();
	}
	
	void	setUniform_fog_color(const r_command_t *cmd, const r_shader_stage_c *stage)
	{
		vec4_c color = color_blue;
		//RB_ModifyColor(cmd->getLight()->getShared(), stage, light_color);
		xglUniform4fARB(_u_fog_color, color[0], color[1], color[2], color[3]);	RB_CheckForError();
	}
	
private:
	uint_t		_u_fog_density;
	uint_t		_u_fog_color;
};


static char*	RB_PrintInfoLog(GLhandleARB object)
{
	static char	msg[4096*2];
	
	int max_length = 0;
	xglGetObjectParameterivARB(object, GL_OBJECT_INFO_LOG_LENGTH_ARB, &max_length);
	
	if(max_length >= (int)sizeof(msg))
	{
		ri.Com_Error(ERR_FATAL, "RB_PrintInfoLog: max length >= sizeof(msg)");
		return NULL;
	}	
	
	xglGetInfoLogARB(object, max_length, &max_length, msg);
	
	return msg;
}


/*
================================================================================
			MAIN GLSL PROGRAM CLASS
================================================================================
*/
class rb_program_c
{
public:
	rb_program_c(const std::string &name, uint_t vflags, bool fragment_shader = true);
	
	void		loadShader(const std::string &name, GLenum shader_type);
	void		link();
	void		validate();
	
	void		enable();
	void		disable();
	void		setVertexAttribs(const r_command_t *cmd);
private:
	void		enableVertexAttribs();
	void		disableVertexAttribs();
public:
	GLhandleARB	getHandle() const		{return _handle;}
//	uint_t		getVertexFlags() const		{return _vflags;}
	
private:
	GLhandleARB	_handle;
	uint_t		_vflags;
};

rb_program_c::rb_program_c(const std::string &name, uint_t vflags, bool fragment_shader)
{
	_handle = xglCreateProgramObjectARB();	RB_CheckForError();
	_vflags = vflags;
	
	loadShader("glsl/" + name + "_vp.glsl", GL_VERTEX_SHADER_ARB);
	
	if(fragment_shader)
		loadShader("glsl/" + name + "_fp.glsl", GL_FRAGMENT_SHADER_ARB);
	
//	if(_vflags & VATTRIB_VERTEX)
//		xglBindAttribLocationARB(_handle, 0, "attr_Vertex");	RB_CheckForError();
	
	if(_vflags & VATTRIB_TEX0)
		xglBindAttribLocationARB(_handle, 7, "attr_TexCoord0");	RB_CheckForError();
	
	if(_vflags & VATTRIB_TEX1)
		xglBindAttribLocationARB(_handle, 8, "attr_TexCoord1");	RB_CheckForError();
		
	if(_vflags & VATTRIB_TANGENT)
		xglBindAttribLocationARB(_handle, 4, "attr_Tangent");	RB_CheckForError();
		
	if(_vflags & VATTRIB_BINORMAL)
		xglBindAttribLocationARB(_handle, 5, "attr_Binormal");	RB_CheckForError();
		
	if(_vflags & VATTRIB_LIGHT)
		xglBindAttribLocationARB(_handle, 6, "attr_Light");	RB_CheckForError();
		
//	if(_vflags & VATTRIB_COLOR)
//		xglBindAttribLocationARB(_handle, 6, "attr_Color");	RB_CheckForError();
	
	link();
	validate();
}

void	rb_program_c::loadShader(const std::string &name, GLenum shader_type)
{
	GLcharARB *buffer = NULL;
	int size;

	size = ri.VFS_FLoad(name, (void**)&buffer);
	if(!buffer)
        {
		ri.Com_Error(ERR_DROP, "rb_program_c::loadShader: couldn't load '%s'", name.c_str());
		return;
	}
	
	ri.Com_Printf("compiling '%s' ...\n", name.c_str());
	
	GLhandleARB shader = xglCreateShaderObjectARB(shader_type);
	xglShaderSourceARB(shader, 1, (const GLcharARB**)&buffer, &size);
	
	// compile shader
	xglCompileShaderARB(shader);
	
	// check if shader compiled
	GLint compiled = 0;
	xglGetObjectParameterivARB(shader, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);
	if(!compiled)
	{
		ri.Com_Error(ERR_FATAL, "rb_program_c::loadShader: %s", RB_PrintInfoLog(shader));
		return;
	}
	
	// attach shader to program
	xglAttachObjectARB(_handle, shader);
	
	// delete shader, no longer needed
	xglDeleteObjectARB(shader);
	
	ri.VFS_FFree(buffer);
	
	RB_CheckForError();
}

void	rb_program_c::link()
{
	xglLinkProgramARB(_handle);
	
	GLint linked = false;
	xglGetObjectParameterivARB(_handle, GL_OBJECT_LINK_STATUS_ARB, &linked);
	if(!linked)
	{
		ri.Com_Error(ERR_FATAL, "rb_program_c::link: %s\nshaders failed to link", RB_PrintInfoLog(_handle));
	}
}

void	rb_program_c::validate()
{
	xglValidateProgramARB(_handle);
	
	GLint validated = false;
	xglGetObjectParameterivARB(_handle, GL_OBJECT_VALIDATE_STATUS_ARB, &validated);
	if(!validated)
	{
		ri.Com_Error(ERR_FATAL, "rb_program_c::validate: %s\nshaders failed to validate", RB_PrintInfoLog(_handle));
	}
}

void	rb_program_c::enable()
{
	RB_CheckForError();

	xglUseProgramObjectARB(_handle);//	RB_CheckForError();
	
	enableVertexAttribs();
}

void	rb_program_c::disable()
{
	RB_CheckForError();

	xglUseProgramObjectARB(0);//	RB_CheckForError();
	
	disableVertexAttribs();
}

void	rb_program_c::setVertexAttribs(const r_command_t *cmd)
{
	const r_mesh_c*	entity_mesh = cmd->getEntityMesh();
	
	if(gl_config.arb_vertex_buffer_object && entity_mesh->vbo_array_buffer)
	{
		/*
		if(	(gl_state.current_vbo_array_buffer == entity_mesh->vbo_array_buffer) &&
			(gl_state.current_vbo_vertexes_ofs == entity_mesh->vbo_vertexes_ofs)	)
		{
			xglBindBufferARB(GL_ARRAY_BUFFER_ARB, entity_mesh->vbo_array_buffer);	RB_CheckForError();
			return;
		}
		else
		{
			gl_state.current_vbo_array_buffer = entity_mesh->vbo_array_buffer;
			gl_state.current_vbo_vertexes_ofs = entity_mesh->vbo_vertexes_ofs;
		}
		*/
	
		xglBindBufferARB(GL_ARRAY_BUFFER_ARB, entity_mesh->vbo_array_buffer);	RB_CheckForError();
		
		if(_vflags & VATTRIB_VERTEX)
//			xglVertexAttribPointerARB(0, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(entity_mesh->vbo_vertexes_ofs));	RB_CheckForError();
#if !defined(DOUBLEVEC_T) && defined(SIMD_SSE)
			xglVertexPointer(3, GL_FLOAT, 16, VBO_BUFFER_OFFSET(entity_mesh->vbo_vertexes_ofs));			RB_CheckForError();
#else
			xglVertexPointer(3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(entity_mesh->vbo_vertexes_ofs));			RB_CheckForError();
#endif
			
		if(_vflags & VATTRIB_TEX0)
			xglVertexAttribPointerARB(7, 2, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(entity_mesh->vbo_texcoords_ofs));	RB_CheckForError();		
			
		if(_vflags & VATTRIB_TEX1)
			xglVertexAttribPointerARB(8, 2, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(entity_mesh->vbo_texcoords_lm_ofs));	RB_CheckForError();
		
		if(_vflags & VATTRIB_TANGENT)
#if !defined(DOUBLEVEC_T) && defined(SIMD_SSE)
			xglVertexAttribPointerARB(4, 3, GL_FLOAT, 0, 16, VBO_BUFFER_OFFSET(entity_mesh->vbo_tangents_ofs));	RB_CheckForError();
#else
			xglVertexAttribPointerARB(4, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(entity_mesh->vbo_tangents_ofs));	RB_CheckForError();
#endif
		
		if(_vflags & VATTRIB_BINORMAL)
#if !defined(DOUBLEVEC_T) && defined(SIMD_SSE)
			xglVertexAttribPointerARB(5, 3, GL_FLOAT, 0, 16, VBO_BUFFER_OFFSET(entity_mesh->vbo_binormals_ofs));	RB_CheckForError();
#else
			xglVertexAttribPointerARB(5, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(entity_mesh->vbo_binormals_ofs));	RB_CheckForError();
#endif
			
		if(_vflags & VATTRIB_NORMAL)
//			xglVertexAttribPointerARB(5, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(entity_mesh->vbo_normals_ofs));	RB_CheckForError();
#if !defined(DOUBLEVEC_T) && defined(SIMD_SSE)
			xglNormalPointer(GL_FLOAT, 16, VBO_BUFFER_OFFSET(entity_mesh->vbo_normals_ofs));			RB_CheckForError();
#else
			xglNormalPointer(GL_FLOAT, 0, VBO_BUFFER_OFFSET(entity_mesh->vbo_normals_ofs));				RB_CheckForError();
#endif

		if(_vflags & VATTRIB_LIGHT)
#if !defined(DOUBLEVEC_T) && defined(SIMD_SSE)
			xglVertexAttribPointerARB(6, 3, GL_FLOAT, 0, 16, VBO_BUFFER_OFFSET(entity_mesh->vbo_lights_ofs));	RB_CheckForError();
#else
			xglVertexAttribPointerARB(6, 3, GL_FLOAT, 0, 0, VBO_BUFFER_OFFSET(entity_mesh->vbo_lights_ofs));	RB_CheckForError();
#endif
		
		if(_vflags & VATTRIB_COLOR)
			xglColorPointer(4, GL_FLOAT, 0, VBO_BUFFER_OFFSET(entity_mesh->vbo_colors_ofs));			RB_CheckForError();
	}
	else
	{
		if(gl_config.arb_vertex_buffer_object)
		{
			gl_state.current_vbo_array_buffer = 0;
			gl_state.current_vbo_vertexes_ofs = 0;
			
			xglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);	RB_CheckForError();
		}
		
		if(_vflags & VATTRIB_VERTEX)
#if !defined(DOUBLEVEC_T) && defined(SIMD_SSE)
			xglVertexPointer(3, GL_FLOAT, 16, &(entity_mesh->vertexes[0]));				RB_CheckForError();
#else
			xglVertexPointer(3, GL_FLOAT, 0, &(entity_mesh->vertexes[0]));				RB_CheckForError();
#endif
		
		if(_vflags & VATTRIB_TEX0)
			xglVertexAttribPointerARB(7, 2, GL_FLOAT, 0, 0, &(entity_mesh->texcoords[0]));		RB_CheckForError();
			
		if(_vflags & VATTRIB_TEX1)
			xglVertexAttribPointerARB(8, 2, GL_FLOAT, 0, 0, &(entity_mesh->texcoords_lm[0]));	RB_CheckForError();
		
		if(_vflags & VATTRIB_TANGENT)
#if !defined(DOUBLEVEC_T) && defined(SIMD_SSE)
			xglVertexAttribPointerARB(4, 3, GL_FLOAT, 0, 16, &(entity_mesh->tangents[0]));		RB_CheckForError();
#else
			xglVertexAttribPointerARB(4, 3, GL_FLOAT, 0, 0, &(entity_mesh->tangents[0]));		RB_CheckForError();
#endif
		
		if(_vflags & VATTRIB_BINORMAL)
#if !defined(DOUBLEVEC_T) && defined(SIMD_SSE)
			xglVertexAttribPointerARB(5, 3, GL_FLOAT, 0, 16, &(entity_mesh->binormals[0]));		RB_CheckForError();
#else
			xglVertexAttribPointerARB(5, 3, GL_FLOAT, 0, 0, &(entity_mesh->binormals[0]));		RB_CheckForError();
#endif
		
		if(_vflags & VATTRIB_NORMAL)
#if !defined(DOUBLEVEC_T) && defined(SIMD_SSE)
			xglNormalPointer(GL_FLOAT, 16, &(entity_mesh->normals[0]));				RB_CheckForError();
#else
			xglNormalPointer(GL_FLOAT, 0, &(entity_mesh->normals[0]));				RB_CheckForError();
#endif

		if(_vflags & VATTRIB_LIGHT)
#if !defined(DOUBLEVEC_T) && defined(SIMD_SSE)
			xglVertexAttribPointerARB(6, 3, GL_FLOAT, 0, 16, &(entity_mesh->lights[0]));		RB_CheckForError();
#else
			xglVertexAttribPointerARB(6, 3, GL_FLOAT, 0, 0, &(entity_mesh->lights[0]));		RB_CheckForError();
#endif
			
		if(_vflags & VATTRIB_COLOR)
			xglColorPointer(4, GL_FLOAT, 0, &(entity_mesh->colors[0]));				RB_CheckForError();
	}
}

void	rb_program_c::enableVertexAttribs()
{
	if(_vflags & VATTRIB_VERTEX)
		xglEnableClientState(GL_VERTEX_ARRAY);			RB_CheckForError();
		
	if(_vflags & VATTRIB_TEX0)
		xglEnableVertexAttribArrayARB(7);			RB_CheckForError();
		
	if(_vflags & VATTRIB_TEX1)
		xglEnableVertexAttribArrayARB(8);			RB_CheckForError();
		
	if(_vflags & VATTRIB_TANGENT)
		xglEnableVertexAttribArrayARB(4);			RB_CheckForError();
	
	if(_vflags & VATTRIB_BINORMAL)
		xglEnableVertexAttribArrayARB(5);			RB_CheckForError();
		
	if(_vflags & VATTRIB_NORMAL)
		xglEnableClientState(GL_NORMAL_ARRAY);			RB_CheckForError();
		
	if(_vflags & VATTRIB_LIGHT)
		xglEnableVertexAttribArrayARB(6);			RB_CheckForError();
		
	if(_vflags & VATTRIB_COLOR)
		xglEnableClientState(GL_COLOR_ARRAY);			RB_CheckForError();
}

void	rb_program_c::disableVertexAttribs()
{
	if(_vflags & VATTRIB_VERTEX)
		xglDisableClientState(GL_VERTEX_ARRAY);			RB_CheckForError();
		
	if(_vflags & VATTRIB_TEX0)
		xglDisableVertexAttribArrayARB(7);			RB_CheckForError();
		
	if(_vflags & VATTRIB_TEX1)
		xglDisableVertexAttribArrayARB(8);			RB_CheckForError();
		
	if(_vflags & VATTRIB_TANGENT)
		xglDisableVertexAttribArrayARB(4);			RB_CheckForError();
	
	if(_vflags & VATTRIB_BINORMAL)
		xglDisableVertexAttribArrayARB(5);			RB_CheckForError();
		
	if(_vflags & VATTRIB_NORMAL)
		xglDisableClientState(GL_NORMAL_ARRAY);			RB_CheckForError();
		
	if(_vflags & VATTRIB_LIGHT)
		xglDisableVertexAttribArrayARB(6);			RB_CheckForError();
		
	if(_vflags & VATTRIB_COLOR)
		xglDisableClientState(GL_COLOR_ARRAY);			RB_CheckForError();
}


/*
================================================================================
			GLSL PROGRAM CLASSES
================================================================================
*/
class rb_generic_c : public rb_program_c
{
public:
	rb_generic_c()
	:rb_program_c("generic", VATTRIB_VERTEX | VATTRIB_TEX0, false)
	{
/*
		_u_colormap	= xglGetUniformLocationARB(getHandle(), "u_colormap");	RB_CheckForError();
		_u_color	= xglGetUniformLocationARB(getHandle(), "u_color");	RB_CheckForError();
		
		xglUseProgramObjectARB(getHandle());	RB_CheckForError();
		
		xglUniform1iARB(_u_colormap, 0);	RB_CheckForError();
		
		xglUseProgramObjectARB(0);	RB_CheckForError();
*/
	}

//	uint_t		getColor() const		{return _u_color;}

private:
//	uint_t		_u_colormap;
//	uint_t		_u_color;
};


class rb_zfill_c : public rb_program_c
{
public:
	rb_zfill_c()
	:rb_program_c("zfill", VATTRIB_VERTEX | VATTRIB_TEX0, false)
	{
	/*
		_u_colormap	= xglGetUniformLocationARB(getHandle(), "u_colormap");	RB_CheckForError();

		xglUseProgramObjectARB(getHandle());	RB_CheckForError();
		
		xglUniform1iARB(_u_colormap, 0);	RB_CheckForError();
		
		xglUseProgramObjectARB(0);	RB_CheckForError();
	*/
	}

private:
//	uint_t		_u_colormap;
};


class rb_lighting_R_c : public rb_program_c
{
public:
	rb_lighting_R_c()
	:rb_program_c("lighting_R", VATTRIB_VERTEX | VATTRIB_TEX0 | VATTRIB_TEX1 | VATTRIB_NORMAL)
	{
		_u_diffusemap	= xglGetUniformLocationARB(getHandle(), "u_diffusemap");	RB_CheckForError();
		_u_lightmap	= xglGetUniformLocationARB(getHandle(), "u_lightmap");		RB_CheckForError();
		_u_deluxemap	= xglGetUniformLocationARB(getHandle(), "u_deluxemap");		RB_CheckForError();
		
		xglUseProgramObjectARB(getHandle());	RB_CheckForError();
		
		xglUniform1iARB(_u_diffusemap, 0);	RB_CheckForError();
		xglUniform1iARB(_u_lightmap, 1);	RB_CheckForError();
		xglUniform1iARB(_u_deluxemap, 2);	RB_CheckForError();
		
		xglUseProgramObjectARB(0);	RB_CheckForError();
	}

private:
	uint_t		_u_diffusemap;
	uint_t		_u_lightmap;
	uint_t		_u_deluxemap;
};


class rb_lighting_RB_c : public rb_program_c, public u_bump_scale_a
{
public:
	rb_lighting_RB_c()
	:rb_program_c("lighting_RB", VATTRIB_VERTEX | VATTRIB_TEX0 | VATTRIB_TEX1 | VATTRIB_TANGENT | VATTRIB_BINORMAL | VATTRIB_NORMAL),
	u_bump_scale_a(getHandle())
	{
		_u_diffusemap	= xglGetUniformLocationARB(getHandle(), "u_diffusemap");	RB_CheckForError();
		_u_bumpmap	= xglGetUniformLocationARB(getHandle(), "u_bumpmap");		RB_CheckForError();
		_u_lightmap	= xglGetUniformLocationARB(getHandle(), "u_lightmap");		RB_CheckForError();
		_u_deluxemap	= xglGetUniformLocationARB(getHandle(), "u_deluxemap");		RB_CheckForError();
		
		xglUseProgramObjectARB(getHandle());	RB_CheckForError();
		
		xglUniform1iARB(_u_diffusemap, 0);	RB_CheckForError();
		xglUniform1iARB(_u_bumpmap, 1);		RB_CheckForError();
		xglUniform1iARB(_u_lightmap, 2);	RB_CheckForError();
		xglUniform1iARB(_u_deluxemap, 3);	RB_CheckForError();
		
		xglUseProgramObjectARB(0);	RB_CheckForError();
	}
	
private:
	uint_t		_u_diffusemap;
	uint_t		_u_bumpmap;
	uint_t		_u_lightmap;
	uint_t		_u_deluxemap;
};


class rb_lighting_RBH_c :
public rb_program_c,
public u_view_origin_a,
public u_bump_scale_a,
public u_parallax_a
{
public:
	rb_lighting_RBH_c()
	:rb_program_c("lighting_RBH", VATTRIB_VERTEX | VATTRIB_TEX0 | VATTRIB_TEX1 | VATTRIB_TANGENT | VATTRIB_BINORMAL | VATTRIB_NORMAL),
	u_view_origin_a(getHandle()),
	u_bump_scale_a(getHandle()),
	u_parallax_a(getHandle())
	{
		_u_diffusemap	= xglGetUniformLocationARB(getHandle(), "u_diffusemap");	RB_CheckForError();
		_u_bumpmap	= xglGetUniformLocationARB(getHandle(), "u_bumpmap");		RB_CheckForError();
		_u_lightmap	= xglGetUniformLocationARB(getHandle(), "u_lightmap");		RB_CheckForError();
		_u_deluxemap	= xglGetUniformLocationARB(getHandle(), "u_deluxemap");		RB_CheckForError();
		
		xglUseProgramObjectARB(getHandle());	RB_CheckForError();
		
		xglUniform1iARB(_u_diffusemap, 0);	RB_CheckForError();
		xglUniform1iARB(_u_bumpmap, 1);		RB_CheckForError();
		xglUniform1iARB(_u_lightmap, 2);	RB_CheckForError();
		xglUniform1iARB(_u_deluxemap, 3);	RB_CheckForError();
		
		xglUseProgramObjectARB(0);	RB_CheckForError();
	}

private:
	uint_t		_u_diffusemap;
	uint_t		_u_bumpmap;
	uint_t		_u_lightmap;
	uint_t		_u_deluxemap;
};


class rb_lighting_RBHS_c :
public rb_program_c,
public u_view_origin_a,
public u_bump_scale_a,
public u_parallax_a,
public u_specular_exponent_a
{
public:
	rb_lighting_RBHS_c()
	:rb_program_c("lighting_RBHS", VATTRIB_VERTEX | VATTRIB_TEX0 | VATTRIB_TEX1 | VATTRIB_TANGENT | VATTRIB_BINORMAL | VATTRIB_NORMAL),
	u_view_origin_a(getHandle()),
	u_bump_scale_a(getHandle()),
	u_parallax_a(getHandle()),
	u_specular_exponent_a(getHandle())
	{
		_u_diffusemap		= xglGetUniformLocationARB(getHandle(), "u_diffusemap");	RB_CheckForError();
		_u_bumpmap		= xglGetUniformLocationARB(getHandle(), "u_bumpmap");		RB_CheckForError();
		_u_specularmap		= xglGetUniformLocationARB(getHandle(), "u_specularmap");	RB_CheckForError();
		_u_lightmap		= xglGetUniformLocationARB(getHandle(), "u_lightmap");		RB_CheckForError();
		_u_deluxemap		= xglGetUniformLocationARB(getHandle(), "u_deluxemap");		RB_CheckForError();
		
		xglUseProgramObjectARB(getHandle());	RB_CheckForError();
		
		xglUniform1iARB(_u_diffusemap, 0);	RB_CheckForError();
		xglUniform1iARB(_u_bumpmap, 1);		RB_CheckForError();
		xglUniform1iARB(_u_specularmap, 2);	RB_CheckForError();
		xglUniform1iARB(_u_lightmap, 3);	RB_CheckForError();
		xglUniform1iARB(_u_deluxemap, 4);	RB_CheckForError();
		
		xglUseProgramObjectARB(0);	RB_CheckForError();
	}
	
private:
	uint_t		_u_diffusemap;
	uint_t		_u_bumpmap;
	uint_t		_u_specularmap;
	uint_t		_u_lightmap;
	uint_t		_u_deluxemap;
};


class rb_lighting_RBS_c :
public rb_program_c,
public u_view_origin_a,
public u_bump_scale_a,
public u_specular_exponent_a
{
public:
	rb_lighting_RBS_c()
	:rb_program_c("lighting_RBS", VATTRIB_VERTEX | VATTRIB_TEX0 | VATTRIB_TEX1 | VATTRIB_TANGENT | VATTRIB_BINORMAL | VATTRIB_NORMAL),
	u_view_origin_a(getHandle()),
	u_bump_scale_a(getHandle()),
	u_specular_exponent_a(getHandle())
	{
		_u_diffusemap		= xglGetUniformLocationARB(getHandle(), "u_diffusemap");	RB_CheckForError();
		_u_bumpmap		= xglGetUniformLocationARB(getHandle(), "u_bumpmap");		RB_CheckForError();
		_u_specularmap		= xglGetUniformLocationARB(getHandle(), "u_specularmap");	RB_CheckForError();
		_u_lightmap		= xglGetUniformLocationARB(getHandle(), "u_lightmap");		RB_CheckForError();
		_u_deluxemap		= xglGetUniformLocationARB(getHandle(), "u_deluxemap");		RB_CheckForError();
		
		xglUseProgramObjectARB(getHandle());	RB_CheckForError();
		
		xglUniform1iARB(_u_diffusemap, 0);	RB_CheckForError();
		xglUniform1iARB(_u_bumpmap, 1);		RB_CheckForError();
		xglUniform1iARB(_u_specularmap, 2);	RB_CheckForError();
		xglUniform1iARB(_u_lightmap, 3);	RB_CheckForError();
		xglUniform1iARB(_u_deluxemap, 4);	RB_CheckForError();
		
		xglUseProgramObjectARB(0);	RB_CheckForError();
	}

private:
	uint_t		_u_diffusemap;
	uint_t		_u_bumpmap;
	uint_t		_u_specularmap;
	uint_t		_u_lightmap;
	uint_t		_u_deluxemap;
};


class rb_lighting_D_vstatic_c :
public rb_program_c
{
public:
	rb_lighting_D_vstatic_c()
	:rb_program_c("lighting_D_vstatic", VATTRIB_VERTEX | VATTRIB_TEX0 | VATTRIB_NORMAL | VATTRIB_LIGHT | VATTRIB_COLOR)
	{
		_u_diffusemap		= xglGetUniformLocationARB(getHandle(), "u_diffusemap");		RB_CheckForError();
		
		xglUseProgramObjectARB(getHandle());	RB_CheckForError();
		
		xglUniform1iARB(_u_diffusemap,		0);	RB_CheckForError();
		
		xglUseProgramObjectARB(0);	RB_CheckForError();
	}

private:
	uint_t		_u_diffusemap;
};


class rb_lighting_DB_vstatic_c :
public rb_program_c,
public u_bump_scale_a
{
public:
	rb_lighting_DB_vstatic_c()
	:rb_program_c("lighting_DB_vstatic", VATTRIB_VERTEX | VATTRIB_TEX0 | VATTRIB_TANGENT | VATTRIB_BINORMAL | VATTRIB_NORMAL | VATTRIB_LIGHT | VATTRIB_COLOR),
	u_bump_scale_a(getHandle())
	{
		_u_diffusemap		= xglGetUniformLocationARB(getHandle(), "u_diffusemap");		RB_CheckForError();
		_u_bumpmap		= xglGetUniformLocationARB(getHandle(), "u_bumpmap");			RB_CheckForError();
				
		xglUseProgramObjectARB(getHandle());	RB_CheckForError();
		
		xglUniform1iARB(_u_diffusemap,		0);	RB_CheckForError();
		xglUniform1iARB(_u_bumpmap,		1);	RB_CheckForError();
		
		xglUseProgramObjectARB(0);	RB_CheckForError();
	}

private:
	uint_t		_u_diffusemap;
	uint_t		_u_bumpmap;
};


class rb_lighting_DBH_vstatic_c :
public rb_program_c,
public u_view_origin_a,
public u_bump_scale_a,
public u_parallax_a
{
public:
	rb_lighting_DBH_vstatic_c()
	:rb_program_c("lighting_DBH_vstatic", VATTRIB_VERTEX | VATTRIB_TEX0 | VATTRIB_TANGENT | VATTRIB_BINORMAL | VATTRIB_NORMAL | VATTRIB_LIGHT | VATTRIB_COLOR),
	u_view_origin_a(getHandle()),
	u_bump_scale_a(getHandle()),
	u_parallax_a(getHandle())
	{
		_u_diffusemap		= xglGetUniformLocationARB(getHandle(), "u_diffusemap");		RB_CheckForError();
		_u_bumpmap		= xglGetUniformLocationARB(getHandle(), "u_bumpmap");			RB_CheckForError();
				
		xglUseProgramObjectARB(getHandle());	RB_CheckForError();
		
		xglUniform1iARB(_u_diffusemap,		0);	RB_CheckForError();
		xglUniform1iARB(_u_bumpmap,		1);	RB_CheckForError();
		
		xglUseProgramObjectARB(0);	RB_CheckForError();
	}

private:
	uint_t		_u_diffusemap;
	uint_t		_u_bumpmap;
};


class rb_lighting_DBHS_vstatic_c :
public rb_program_c,
public u_view_origin_a,
public u_bump_scale_a,
public u_parallax_a,
public u_specular_exponent_a
{
public:
	rb_lighting_DBHS_vstatic_c()
	:rb_program_c("lighting_DBHS_vstatic", VATTRIB_VERTEX | VATTRIB_TEX0 | VATTRIB_TANGENT | VATTRIB_BINORMAL | VATTRIB_NORMAL | VATTRIB_LIGHT | VATTRIB_COLOR),
	u_view_origin_a(getHandle()),
	u_bump_scale_a(getHandle()),
	u_parallax_a(getHandle()),
	u_specular_exponent_a(getHandle())
	{
		_u_diffusemap		= xglGetUniformLocationARB(getHandle(), "u_diffusemap");		RB_CheckForError();
		_u_bumpmap		= xglGetUniformLocationARB(getHandle(), "u_bumpmap");			RB_CheckForError();
		_u_specularmap		= xglGetUniformLocationARB(getHandle(), "u_specularmap");		RB_CheckForError();
				
		xglUseProgramObjectARB(getHandle());	RB_CheckForError();
		
		xglUniform1iARB(_u_diffusemap,		0);	RB_CheckForError();
		xglUniform1iARB(_u_bumpmap,		1);	RB_CheckForError();
		xglUniform1iARB(_u_specularmap,		2);	RB_CheckForError();
		
		xglUseProgramObjectARB(0);	RB_CheckForError();
	}

private:
	uint_t		_u_diffusemap;
	uint_t		_u_bumpmap;
	uint_t		_u_specularmap;
};


class rb_lighting_DBS_vstatic_c :
public rb_program_c,
public u_view_origin_a,
public u_bump_scale_a,
public u_specular_exponent_a
{
public:
	rb_lighting_DBS_vstatic_c()
	:rb_program_c("lighting_DBS_vstatic", VATTRIB_VERTEX | VATTRIB_TEX0 | VATTRIB_TANGENT | VATTRIB_BINORMAL | VATTRIB_NORMAL | VATTRIB_LIGHT | VATTRIB_COLOR),
	u_view_origin_a(getHandle()),
	u_bump_scale_a(getHandle()),
	u_specular_exponent_a(getHandle())
	{
		_u_diffusemap		= xglGetUniformLocationARB(getHandle(), "u_diffusemap");		RB_CheckForError();
		_u_bumpmap		= xglGetUniformLocationARB(getHandle(), "u_bumpmap");			RB_CheckForError();
		_u_specularmap		= xglGetUniformLocationARB(getHandle(), "u_specularmap");		RB_CheckForError();
				
		xglUseProgramObjectARB(getHandle());	RB_CheckForError();
		
		xglUniform1iARB(_u_diffusemap,		0);	RB_CheckForError();
		xglUniform1iARB(_u_bumpmap,		1);	RB_CheckForError();
		xglUniform1iARB(_u_specularmap,		2);	RB_CheckForError();
		
		xglUseProgramObjectARB(0);	RB_CheckForError();
	}

private:
	uint_t		_u_diffusemap;
	uint_t		_u_bumpmap;
	uint_t		_u_specularmap;
};


class rb_lighting_D_omni_c :
public rb_program_c,
public u_light_origin_a,
public u_light_color_a
{
public:
	rb_lighting_D_omni_c()
	:rb_program_c("lighting_D_omni", VATTRIB_VERTEX | VATTRIB_TEX0 | VATTRIB_NORMAL),
	u_light_origin_a(getHandle()),
	u_light_color_a(getHandle())
	{
		_u_diffusemap		= xglGetUniformLocationARB(getHandle(), "u_diffusemap");		RB_CheckForError();
		_u_attenuationmap_xy	= xglGetUniformLocationARB(getHandle(), "u_attenuationmap_xy");		RB_CheckForError();
		_u_attenuationmap_z	= xglGetUniformLocationARB(getHandle(), "u_attenuationmap_z");		RB_CheckForError();
//		_u_attenuationmap_cube	= xglGetUniformLocationARB(getHandle(), "u_attenuationmap_cube");	RB_CheckForError();
//		_u_shadowmap		= xglGetUniformLocationARB(getHandle(), "u_shadowmap");			RB_CheckForError();
				
		xglUseProgramObjectARB(getHandle());	RB_CheckForError();
		
		
		xglUniform1iARB(_u_diffusemap,		0);	RB_CheckForError();
		xglUniform1iARB(_u_attenuationmap_xy,	1);	RB_CheckForError();
		xglUniform1iARB(_u_attenuationmap_z,	2);	RB_CheckForError();
//		xglUniform1iARB(_u_attenuationmap_cube,	3);	RB_CheckForError();
//		xglUniform1iARB(_u_shadowmap,		4);	RB_CheckForError();
		
		xglUseProgramObjectARB(0);	RB_CheckForError();
	}

private:
	uint_t		_u_diffusemap;
	uint_t		_u_attenuationmap_xy;
	uint_t		_u_attenuationmap_z;
//	uint_t		_u_attenuationmap_cube;
//	uint_t		_u_shadowmap;
};


class rb_lighting_D_proj_c :
public rb_program_c,
public u_light_origin_a,
public u_light_color_a
{
public:
	rb_lighting_D_proj_c()
	:rb_program_c("lighting_D_proj", VATTRIB_VERTEX | VATTRIB_TEX0 | VATTRIB_NORMAL),
	u_light_origin_a(getHandle()),
	u_light_color_a(getHandle())
	{
		_u_diffusemap		= xglGetUniformLocationARB(getHandle(), "u_diffusemap");		RB_CheckForError();
		_u_attenuationmap_xy	= xglGetUniformLocationARB(getHandle(), "u_attenuationmap_xy");		RB_CheckForError();
		_u_shadowmap		= xglGetUniformLocationARB(getHandle(), "u_shadowmap");			RB_CheckForError();
				
		xglUseProgramObjectARB(getHandle());	RB_CheckForError();
		
		xglUniform1iARB(_u_diffusemap,		0);	RB_CheckForError();
		xglUniform1iARB(_u_attenuationmap_xy,	1);	RB_CheckForError();
		xglUniform1iARB(_u_shadowmap,		2);	RB_CheckForError();
		
		xglUseProgramObjectARB(0);	RB_CheckForError();
	}

private:
	uint_t		_u_diffusemap;
	uint_t		_u_attenuationmap_xy;
	uint_t		_u_shadowmap;
};


class rb_lighting_DB_omni_c :
public rb_program_c,
public u_light_origin_a,
public u_light_color_a,
public u_bump_scale_a
{
public:
	rb_lighting_DB_omni_c()
	:rb_program_c("lighting_DB_omni", VATTRIB_VERTEX | VATTRIB_TEX0 | VATTRIB_TANGENT | VATTRIB_BINORMAL | VATTRIB_NORMAL),
	u_light_origin_a(getHandle()),
	u_light_color_a(getHandle()),
	u_bump_scale_a(getHandle())
	{
		_u_diffusemap		= xglGetUniformLocationARB(getHandle(), "u_diffusemap");		RB_CheckForError();
		_u_bumpmap		= xglGetUniformLocationARB(getHandle(), "u_bumpmap");			RB_CheckForError();
		_u_attenuationmap_xy	= xglGetUniformLocationARB(getHandle(), "u_attenuationmap_xy");		RB_CheckForError();
		_u_attenuationmap_z	= xglGetUniformLocationARB(getHandle(), "u_attenuationmap_z");		RB_CheckForError();
//		_u_attenuationmap_cube	= xglGetUniformLocationARB(getHandle(), "u_attenuationmap_cube");	RB_CheckForError();
				
		xglUseProgramObjectARB(getHandle());	RB_CheckForError();
		
		xglUniform1iARB(_u_diffusemap,		0);	RB_CheckForError();
		xglUniform1iARB(_u_bumpmap,		1);	RB_CheckForError();
		xglUniform1iARB(_u_attenuationmap_xy,	2);	RB_CheckForError();
		xglUniform1iARB(_u_attenuationmap_z,	3);	RB_CheckForError();
//		xglUniform1iARB(_u_attenuationmap_cube,	4);	RB_CheckForError();
		
		xglUseProgramObjectARB(0);	RB_CheckForError();
	}

private:
	uint_t		_u_diffusemap;
	uint_t		_u_bumpmap;
	uint_t		_u_attenuationmap_xy;
	uint_t		_u_attenuationmap_z;
//	uint_t		_u_attenuationmap_cube;
};


class rb_lighting_DBH_omni_c :
public rb_program_c,
public u_view_origin_a,
public u_light_origin_a,
public u_light_color_a,
public u_bump_scale_a,
public u_parallax_a
{
public:
	rb_lighting_DBH_omni_c()
	:rb_program_c("lighting_DBH_omni", VATTRIB_VERTEX | VATTRIB_TEX0 | VATTRIB_TANGENT | VATTRIB_BINORMAL | VATTRIB_NORMAL),
	u_view_origin_a(getHandle()),
	u_light_origin_a(getHandle()),
	u_light_color_a(getHandle()),
	u_bump_scale_a(getHandle()),
	u_parallax_a(getHandle())
	{
		_u_diffusemap		= xglGetUniformLocationARB(getHandle(), "u_diffusemap");		RB_CheckForError();
		_u_bumpmap		= xglGetUniformLocationARB(getHandle(), "u_bumpmap");			RB_CheckForError();
		_u_attenuationmap_xy	= xglGetUniformLocationARB(getHandle(), "u_attenuationmap_xy");		RB_CheckForError();
		_u_attenuationmap_z	= xglGetUniformLocationARB(getHandle(), "u_attenuationmap_z");		RB_CheckForError();
//		_u_attenuationmap_cube	= xglGetUniformLocationARB(getHandle(), "u_attenuationmap_cube");	RB_CheckForError();
				
		xglUseProgramObjectARB(getHandle());	RB_CheckForError();
		
		xglUniform1iARB(_u_diffusemap,		0);	RB_CheckForError();
		xglUniform1iARB(_u_bumpmap,		1);	RB_CheckForError();
		xglUniform1iARB(_u_attenuationmap_xy,	2);	RB_CheckForError();
		xglUniform1iARB(_u_attenuationmap_z,	3);	RB_CheckForError();
//		xglUniform1iARB(_u_attenuationmap_cube,	4);	RB_CheckForError();
		
		xglUseProgramObjectARB(0);	RB_CheckForError();
	}

private:
	uint_t		_u_diffusemap;
	uint_t		_u_bumpmap;
	uint_t		_u_attenuationmap_xy;
	uint_t		_u_attenuationmap_z;
//	uint_t		_u_attenuationmap_cube;
};


class rb_lighting_DBHS_omni_c :
public rb_program_c,
public u_view_origin_a,
public u_light_origin_a,
public u_light_color_a,
public u_bump_scale_a,
public u_parallax_a,
public u_specular_exponent_a
{
public:
	rb_lighting_DBHS_omni_c()
	:rb_program_c("lighting_DBHS_omni", VATTRIB_VERTEX | VATTRIB_TEX0 | VATTRIB_TANGENT | VATTRIB_BINORMAL | VATTRIB_NORMAL),
	u_view_origin_a(getHandle()),
	u_light_origin_a(getHandle()),
	u_light_color_a(getHandle()),
	u_bump_scale_a(getHandle()),
	u_parallax_a(getHandle()),
	u_specular_exponent_a(getHandle())
	{
		_u_diffusemap		= xglGetUniformLocationARB(getHandle(), "u_diffusemap");		RB_CheckForError();
		_u_bumpmap		= xglGetUniformLocationARB(getHandle(), "u_bumpmap");			RB_CheckForError();
		_u_specularmap		= xglGetUniformLocationARB(getHandle(), "u_specularmap");		RB_CheckForError();
		_u_attenuationmap_xy	= xglGetUniformLocationARB(getHandle(), "u_attenuationmap_xy");		RB_CheckForError();
		_u_attenuationmap_z	= xglGetUniformLocationARB(getHandle(), "u_attenuationmap_z");		RB_CheckForError();
//		_u_attenuationmap_cube	= xglGetUniformLocationARB(getHandle(), "u_attenuationmap_cube");	RB_CheckForError();
				
		xglUseProgramObjectARB(getHandle());	RB_CheckForError();
		
		xglUniform1iARB(_u_diffusemap,		0);	RB_CheckForError();
		xglUniform1iARB(_u_bumpmap,		1);	RB_CheckForError();
		xglUniform1iARB(_u_specularmap,		2);	RB_CheckForError();
		xglUniform1iARB(_u_attenuationmap_xy,	3);	RB_CheckForError();
		xglUniform1iARB(_u_attenuationmap_z,	4);	RB_CheckForError();
//		xglUniform1iARB(_u_attenuationmap_cube,	5);	RB_CheckForError();
		
		xglUseProgramObjectARB(0);	RB_CheckForError();
	}

private:
	uint_t		_u_diffusemap;
	uint_t		_u_bumpmap;
	uint_t		_u_specularmap;
	uint_t		_u_attenuationmap_xy;
	uint_t		_u_attenuationmap_z;
//	uint_t		_u_attenuationmap_cube;
};


class rb_lighting_DBS_omni_c :
public rb_program_c,
public u_view_origin_a,
public u_light_origin_a,
public u_light_color_a,
public u_bump_scale_a,
public u_specular_exponent_a
{
public:
	rb_lighting_DBS_omni_c()
	:rb_program_c("lighting_DBS_omni", VATTRIB_VERTEX | VATTRIB_TEX0 | VATTRIB_TANGENT | VATTRIB_BINORMAL | VATTRIB_NORMAL),
	u_view_origin_a(getHandle()),
	u_light_origin_a(getHandle()),
	u_light_color_a(getHandle()),
	u_bump_scale_a(getHandle()),
	u_specular_exponent_a(getHandle())
	{
		_u_diffusemap		= xglGetUniformLocationARB(getHandle(), "u_diffusemap");		RB_CheckForError();
		_u_bumpmap		= xglGetUniformLocationARB(getHandle(), "u_bumpmap");			RB_CheckForError();
		_u_specularmap		= xglGetUniformLocationARB(getHandle(), "u_specularmap");		RB_CheckForError();
		_u_attenuationmap_xy	= xglGetUniformLocationARB(getHandle(), "u_attenuationmap_xy");		RB_CheckForError();
		_u_attenuationmap_z	= xglGetUniformLocationARB(getHandle(), "u_attenuationmap_z");		RB_CheckForError();
//		_u_attenuationmap_cube	= xglGetUniformLocationARB(getHandle(), "u_attenuationmap_cube");	RB_CheckForError();
				
		xglUseProgramObjectARB(getHandle());	RB_CheckForError();
		
		xglUniform1iARB(_u_diffusemap,		0);	RB_CheckForError();
		xglUniform1iARB(_u_bumpmap,		1);	RB_CheckForError();
		xglUniform1iARB(_u_specularmap,		2);	RB_CheckForError();
		xglUniform1iARB(_u_attenuationmap_xy,	3);	RB_CheckForError();
		xglUniform1iARB(_u_attenuationmap_z,	4);	RB_CheckForError();
//		xglUniform1iARB(_u_attenuationmap_cube,	5);	RB_CheckForError();
		
		xglUseProgramObjectARB(0);	RB_CheckForError();
	}

private:
	uint_t		_u_diffusemap;
	uint_t		_u_bumpmap;
	uint_t		_u_specularmap;
	uint_t		_u_attenuationmap_xy;
	uint_t		_u_attenuationmap_z;
//	uint_t		_u_attenuationmap_cube;
};


class rb_fog_uniform_c :
public rb_program_c,
public u_frame_buffer_a,
public u_fog_a
{
public:
	rb_fog_uniform_c()
	:rb_program_c("fog_uniform", VATTRIB_VERTEX | VATTRIB_TEX0),
	u_frame_buffer_a(getHandle()),
	u_fog_a(getHandle())
	{
		_u_currentrendermap	= xglGetUniformLocationARB(getHandle(), "u_currentrendermap");	RB_CheckForError();
//		_u_heathazemap		= xglGetUniformLocationARB(getHandle(), "u_heathazemap");	RB_CheckForError();
		
		xglUseProgramObjectARB(getHandle());	RB_CheckForError();
		
		xglUniform1iARB(_u_currentrendermap, 0);	RB_CheckForError();
//		xglUniform1iARB(_u_heathazemap, 1);		RB_CheckForError();
		
		xglUseProgramObjectARB(0);	RB_CheckForError();
	}

private:
	uint_t		_u_currentrendermap;
//	uint_t		_u_heathazemap;
};


class rb_reflection_C_c :
public rb_program_c,
public u_view_origin_a
{
public:
	rb_reflection_C_c()
	:rb_program_c("reflection_C", VATTRIB_VERTEX | VATTRIB_NORMAL),
	u_view_origin_a(getHandle())
	{
		_u_colormap	= xglGetUniformLocationARB(getHandle(), "u_colormap");	RB_CheckForError();
		
		xglUseProgramObjectARB(getHandle());	RB_CheckForError();
		
		xglUniform1iARB(_u_colormap, 0);	RB_CheckForError();
		
		xglUseProgramObjectARB(0);	RB_CheckForError();
	}
	
private:
	uint_t		_u_colormap;
};


class rb_refraction_C_c :
public rb_program_c,
public u_view_origin_a,
public u_refraction_index_a,
public u_fresnel_a
{
public:
	rb_refraction_C_c()
	:rb_program_c("refraction_C", VATTRIB_VERTEX | VATTRIB_NORMAL),
	u_view_origin_a(getHandle()),
	u_refraction_index_a(getHandle()),
	u_fresnel_a(getHandle())
	{
		_u_colormap	= xglGetUniformLocationARB(getHandle(), "u_colormap");	RB_CheckForError();
		
		xglUseProgramObjectARB(getHandle());	RB_CheckForError();
		
		xglUniform1iARB(_u_colormap, 0);	RB_CheckForError();
		
		xglUseProgramObjectARB(0);	RB_CheckForError();
	}
	
private:
	uint_t		_u_colormap;
};


class rb_dispersion_C_c :
public rb_program_c,
public u_view_origin_a,
public u_eta_ratio_a,
public u_fresnel_a
{
public:
	rb_dispersion_C_c()
	:rb_program_c("dispersion_C", VATTRIB_VERTEX | VATTRIB_NORMAL),
	u_view_origin_a(getHandle()),
	u_eta_ratio_a(getHandle()),
	u_fresnel_a(getHandle())
	{
		_u_colormap	= xglGetUniformLocationARB(getHandle(), "u_colormap");	RB_CheckForError();
		
		xglUseProgramObjectARB(getHandle());	RB_CheckForError();
		
		xglUniform1iARB(_u_colormap, 0);	RB_CheckForError();
		
		xglUseProgramObjectARB(0);	RB_CheckForError();
	}
	
private:
	uint_t		_u_colormap;
};


class rb_liquid_C_c :
public rb_program_c,
public u_view_origin_a
{
public:
	rb_liquid_C_c()
	:rb_program_c("liquid_C", VATTRIB_VERTEX | VATTRIB_NORMAL),
	u_view_origin_a(getHandle())
	{
		_u_colormap	= xglGetUniformLocationARB(getHandle(), "u_colormap");	RB_CheckForError();
		
		xglUseProgramObjectARB(getHandle());	RB_CheckForError();
		
		xglUniform1iARB(_u_colormap, 0);	RB_CheckForError();
		
		xglUseProgramObjectARB(0);	RB_CheckForError();
	}
	
private:
	uint_t		_u_colormap;
};


class rb_skybox_c :
public rb_program_c,
public u_view_origin_a
{
public:
	rb_skybox_c()
	:rb_program_c("skybox", VATTRIB_VERTEX | VATTRIB_NORMAL),
	u_view_origin_a(getHandle())
	{
		_u_colormap	= xglGetUniformLocationARB(getHandle(), "u_colormap");	RB_CheckForError();
		
		xglUseProgramObjectARB(getHandle());	RB_CheckForError();
		
		xglUniform1iARB(_u_colormap, 0);	RB_CheckForError();
		
		xglUseProgramObjectARB(0);	RB_CheckForError();
	}
	
private:
	uint_t		_u_colormap;
};


class rb_skycloud_c :
public rb_program_c,
public u_view_origin_a
{
public:
	rb_skycloud_c()
	:rb_program_c("skycloud", VATTRIB_VERTEX | VATTRIB_TEX0),
	u_view_origin_a(getHandle())
	{
		_u_cloudmap	= xglGetUniformLocationARB(getHandle(), "u_cloudmap");	RB_CheckForError();
		
		xglUseProgramObjectARB(getHandle());	RB_CheckForError();
		
		xglUniform1iARB(_u_cloudmap, 0);	RB_CheckForError();
		
		xglUseProgramObjectARB(0);	RB_CheckForError();
	}
	
private:
	uint_t		_u_cloudmap;
};


class rb_heathaze_c :
public rb_program_c,
public u_deform_magnitude_a,
public u_frame_buffer_a,
public u_bump_scale_a
{
public:
	rb_heathaze_c()
	:rb_program_c("heathaze", VATTRIB_VERTEX | VATTRIB_TEX0),
	u_deform_magnitude_a(getHandle()),
	u_frame_buffer_a(getHandle()),
	u_bump_scale_a(getHandle())
	{
		_u_currentrendermap	= xglGetUniformLocationARB(getHandle(), "u_currentrendermap");	RB_CheckForError();
		_u_heathazemap		= xglGetUniformLocationARB(getHandle(), "u_heathazemap");	RB_CheckForError();
		
		xglUseProgramObjectARB(getHandle());	RB_CheckForError();
		
		xglUniform1iARB(_u_currentrendermap, 0);	RB_CheckForError();
		xglUniform1iARB(_u_heathazemap, 1);		RB_CheckForError();
		
		xglUseProgramObjectARB(0);	RB_CheckForError();
	}

private:
	uint_t		_u_currentrendermap;
	uint_t		_u_heathazemap;
};


static rb_generic_c*			rb_program_generic = NULL;
static rb_zfill_c*			rb_program_zfill = NULL;

static rb_lighting_R_c*			rb_program_lighting_R = NULL;
static rb_lighting_RB_c*		rb_program_lighting_RB = NULL;
static rb_lighting_RBH_c*		rb_program_lighting_RBH = NULL;
static rb_lighting_RBHS_c*		rb_program_lighting_RBHS = NULL;
static rb_lighting_RBS_c*		rb_program_lighting_RBS = NULL;

static rb_lighting_D_vstatic_c*		rb_program_lighting_D_vstatic = NULL;
static rb_lighting_DB_vstatic_c*	rb_program_lighting_DB_vstatic = NULL;
static rb_lighting_DBH_vstatic_c*	rb_program_lighting_DBH_vstatic = NULL;
static rb_lighting_DBHS_vstatic_c*	rb_program_lighting_DBHS_vstatic = NULL;
static rb_lighting_DBS_vstatic_c*	rb_program_lighting_DBS_vstatic = NULL;

static rb_lighting_D_omni_c*		rb_program_lighting_D_omni = NULL;
static rb_lighting_D_proj_c*		rb_program_lighting_D_proj = NULL;
static rb_lighting_DB_omni_c*		rb_program_lighting_DB_omni = NULL;
static rb_lighting_DBH_omni_c*		rb_program_lighting_DBH_omni = NULL;
static rb_lighting_DBHS_omni_c*		rb_program_lighting_DBHS_omni = NULL;
static rb_lighting_DBS_omni_c*		rb_program_lighting_DBS_omni = NULL;

static rb_fog_uniform_c*		rb_program_fog_uniform = NULL;

static rb_reflection_C_c*		rb_program_reflection_C = NULL;
static rb_refraction_C_c*		rb_program_refraction_C = NULL;
static rb_dispersion_C_c*		rb_program_dispersion_C = NULL;
static rb_liquid_C_c*			rb_program_liquid_C = NULL;

static rb_skybox_c*			rb_program_skybox = NULL;
static rb_skycloud_c*			rb_program_skycloud = NULL;

static rb_heathaze_c*			rb_program_heathaze = NULL;


static void	RB_CheckOpenGLExtension(const std::string &name)
{
	if(!strstr(gl_config.extensions_string, name.c_str()))
	{
		ri.Com_Error(ERR_FATAL, "RB_CheckOpenGLExtensions: %s needed by GLSL backend", name.c_str());
	}
	else
	{
		ri.Com_Printf("...using %s\n", name.c_str());
	}
}

void		RB_CheckOpenGLExtensions()
{
	RB_CheckOpenGLExtension("GL_ARB_vertex_program");
	RB_CheckOpenGLExtension("GL_ARB_shader_objects");
	RB_CheckOpenGLExtension("GL_ARB_vertex_shader");
	RB_CheckOpenGLExtension("GL_ARB_fragment_shader");
	RB_CheckOpenGLExtension("GL_ARB_shading_language_100");
	
	/// GL_ARB_vertex_program
	xglVertexAttribPointerARB = (void (GLAPIENTRY*) (GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid *)) xglGetProcAddress("glVertexAttribPointerARB");
	xglEnableVertexAttribArrayARB = (void (GLAPIENTRY*) (GLuint)) xglGetProcAddress("glEnableVertexAttribArrayARB");
	xglDisableVertexAttribArrayARB = (void (GLAPIENTRY*) (GLuint)) xglGetProcAddress("glDisableVertexAttribArrayARB");
//	xglProgramStringARB = (void (GLAPIENTRY*) (GLenum, GLenum, GLsizei, const GLvoid *)) XGL_GetSymbol("glProgramStringARB");
//	xglBindProgramARB = (void (GLAPIENTRY*) (GLenum, GLuint)) XGL_GetSymbol("glBindProgramARB");
//	xglDeleteProgramsARB = (void (GLAPIENTRY*) (GLsizei, const GLuint *)) XGL_GetSymbol("glDeleteProgramsARB");
//	xglGenProgramsARB = (void (GLAPIENTRY*) (GLsizei, GLuint *)) XGL_GetSymbol("glGenProgramsARB");
//	xglProgramEnvParameter4fARB = (void (GLAPIENTRY*) (GLenum, GLuint, GLfloat, GLfloat, GLfloat, GLfloat)) XGL_GetSymbol("glProgramEnvParameter4fARB");
//	xglProgramEnvParameter4fvARB = (void (GLAPIENTRY*) (GLenum, GLuint, const GLfloat *)) XGL_GetSymbol("glProgramEnvParameter4fvARB");
//	xglProgramLocalParameter4fARB = (void (GLAPIENTRY*) (GLenum, GLuint, GLfloat, GLfloat, GLfloat, GLfloat)) XGL_GetSymbol("glProgramLocalParameter4fARB");
//	xglProgramLocalParameter4fvARB = (void (GLAPIENTRY*) (GLenum, GLuint, const GLfloat *)) XGL_GetSymbol("glProgramLocalParameter4fvARB");
//	xglGetProgramEnvParameterfvARB = (void (GLAPIENTRY*) (GLenum, GLuint, GLfloat *)) XGL_GetSymbol("glGetProgramEnvParameterfvARB");
//	xglGetProgramLocalParameterfvARB = (void (GLAPIENTRY*) (GLenum, GLuint, GLfloat *)) XGL_GetSymbol("glGetProgramLocalParameterfvARB");
//	xglGetProgramivARB = (void (GLAPIENTRY*) (GLenum, GLenum, GLint *)) XGL_GetSymbol("glGetProgramivARB");
//	xglGetProgramStringARB = (void (GLAPIENTRY*) (GLenum, GLenum, GLvoid *)) XGL_GetSymbol("glGetProgramStringARB");
//	xglGetVertexAttribfvARB = (void (GLAPIENTRY*) (GLuint, GLenum, GLfloat *)) XGL_GetSymbol("glGetVertexAttribfvARB");
//	xglGetVertexAttribPointervARB = (void (GLAPIENTRY*) (GLuint, GLenum, GLvoid* *)) XGL_GetSymbol("glGetVertexAttribPointervARB");
//	xglIsProgramARB = (GLboolean (GLAPIENTRY*) (GLuint)) XGL_GetSymbol("glIsProgramARB");
	
	/// GL_ARB_shader_objects
	xglDeleteObjectARB = (void (GLAPIENTRY*) (GLhandleARB)) xglGetProcAddress("glDeleteObjectARB");
	xglGetHandleARB = (GLhandleARB (GLAPIENTRY*) (GLenum)) xglGetProcAddress("glGetHandleARB");
	xglDetachObjectARB = (void (GLAPIENTRY*) (GLhandleARB, GLhandleARB)) xglGetProcAddress("glDetachObjectARB");
	xglCreateShaderObjectARB = (GLhandleARB (GLAPIENTRY*) (GLenum)) xglGetProcAddress("glCreateShaderObjectARB");
	xglShaderSourceARB = (void (GLAPIENTRY*) (GLhandleARB, GLsizei, const GLcharARB* *, const GLint *)) xglGetProcAddress("glShaderSourceARB");
	xglCompileShaderARB = (void (GLAPIENTRY*) (GLhandleARB)) xglGetProcAddress("glCompileShaderARB");
	xglCreateProgramObjectARB = (GLhandleARB (GLAPIENTRY*) (void)) xglGetProcAddress("glCreateProgramObjectARB");
	xglAttachObjectARB = (void (GLAPIENTRY*) (GLhandleARB, GLhandleARB)) xglGetProcAddress("glAttachObjectARB");
	xglLinkProgramARB = (void (GLAPIENTRY*) (GLhandleARB)) xglGetProcAddress("glLinkProgramARB");
	xglUseProgramObjectARB = (void (GLAPIENTRY*) (GLhandleARB)) xglGetProcAddress("glUseProgramObjectARB");
	xglValidateProgramARB = (void (GLAPIENTRY*) (GLhandleARB)) xglGetProcAddress("glValidateProgramARB");
	xglUniform1fARB = (void (GLAPIENTRY*) (GLint, GLfloat)) xglGetProcAddress("glUniform1fARB");
	xglUniform2fARB = (void (GLAPIENTRY*) (GLint, GLfloat, GLfloat)) xglGetProcAddress("glUniform2fARB");
	xglUniform3fARB = (void (GLAPIENTRY*) (GLint, GLfloat, GLfloat, GLfloat)) xglGetProcAddress("glUniform3fARB");
	xglUniform4fARB = (void (GLAPIENTRY*) (GLint, GLfloat, GLfloat, GLfloat, GLfloat)) xglGetProcAddress("glUniform4fARB");
	xglUniform1iARB = (void (GLAPIENTRY*) (GLint, GLint)) xglGetProcAddress("glUniform1iARB");
	xglUniform2iARB = (void (GLAPIENTRY*) (GLint, GLint, GLint)) xglGetProcAddress("glUniform2iARB");
	xglUniform3iARB = (void (GLAPIENTRY*) (GLint, GLint, GLint, GLint)) xglGetProcAddress("glUniform3iARB");
	xglUniform4iARB = (void (GLAPIENTRY*) (GLint, GLint, GLint, GLint, GLint)) xglGetProcAddress("glUniform4iARB");
	xglUniform1fvARB = (void (GLAPIENTRY*) (GLint, GLsizei, const GLfloat *)) xglGetProcAddress("glUniform1fvARB");
	xglUniform2fvARB = (void (GLAPIENTRY*) (GLint, GLsizei, const GLfloat *)) xglGetProcAddress("glUniform2fvARB");
	xglUniform3fvARB = (void (GLAPIENTRY*) (GLint, GLsizei, const GLfloat *)) xglGetProcAddress("glUniform3fvARB");
	xglUniform4fvARB = (void (GLAPIENTRY*) (GLint, GLsizei, const GLfloat *)) xglGetProcAddress("glUniform4fvARB");
	xglUniform1ivARB = (void (GLAPIENTRY*) (GLint, GLsizei, const GLint *)) xglGetProcAddress("glUniform1ivARB");
	xglUniform2ivARB = (void (GLAPIENTRY*) (GLint, GLsizei, const GLint *)) xglGetProcAddress("glUniform2ivARB");
	xglUniform3ivARB = (void (GLAPIENTRY*) (GLint, GLsizei, const GLint *)) xglGetProcAddress("glUniform3ivARB");
	xglUniform4ivARB = (void (GLAPIENTRY*) (GLint, GLsizei, const GLint *)) xglGetProcAddress("glUniform4ivARB");
	xglUniformMatrix2fvARB = (void (GLAPIENTRY*) (GLint, GLsizei, GLboolean, const GLfloat *)) xglGetProcAddress("glUniformMatrix2fvARB");
	xglUniformMatrix3fvARB = (void (GLAPIENTRY*) (GLint, GLsizei, GLboolean, const GLfloat *)) xglGetProcAddress("glUniformMatrix3fvARB");
	xglUniformMatrix4fvARB = (void (GLAPIENTRY*) (GLint, GLsizei, GLboolean, const GLfloat *)) xglGetProcAddress("glUniformMatrix4fvARB");
	xglGetObjectParameterfvARB = (void (GLAPIENTRY*) (GLhandleARB, GLenum, GLfloat *)) xglGetProcAddress("glGetObjectParameterfvARB");
	xglGetObjectParameterivARB = (void (GLAPIENTRY*) (GLhandleARB, GLenum, GLint *)) xglGetProcAddress("glGetObjectParameterivARB");
	xglGetInfoLogARB = (void (GLAPIENTRY*) (GLhandleARB, GLsizei, GLsizei *, GLcharARB *)) xglGetProcAddress("glGetInfoLogARB");
	xglGetAttachedObjectsARB = (void (GLAPIENTRY*) (GLhandleARB, GLsizei, GLsizei *, GLhandleARB *)) xglGetProcAddress("glGetAttachedObjectsARB");
	xglGetUniformLocationARB = (GLint (GLAPIENTRY*) (GLhandleARB, const GLcharARB *)) xglGetProcAddress("glGetUniformLocationARB");
	xglGetActiveUniformARB = (void (GLAPIENTRY*) (GLhandleARB, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLcharARB *)) xglGetProcAddress("glGetActiveUniformARB");
	xglGetUniformfvARB = (void (GLAPIENTRY*) (GLhandleARB, GLint, GLfloat *)) xglGetProcAddress("glGetUniformfvARB");
	xglGetUniformivARB = (void (GLAPIENTRY*) (GLhandleARB, GLint, GLint *)) xglGetProcAddress("glGetUniformivARB");
	xglGetShaderSourceARB = (void (GLAPIENTRY*) (GLhandleARB, GLsizei, GLsizei *, GLcharARB *)) xglGetProcAddress("glGetShaderSourceARB");

	/// GL_ARB_vertex_shader
	xglBindAttribLocationARB = (void (GLAPIENTRY*) (GLhandleARB, GLuint, const GLcharARB *)) xglGetProcAddress("glBindAttribLocationARB");
	xglGetActiveAttribARB = (void (GLAPIENTRY*) (GLhandleARB, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLcharARB *)) xglGetProcAddress("glGetActiveAttribARB");
	xglGetAttribLocationARB = (GLint (GLAPIENTRY*) (GLhandleARB, const GLcharARB *)) xglGetProcAddress("glGetAttribLocationARB");
}


void		RB_InitGPUShaders()
{
	ri.Com_Printf("------- RB_InitGPUShaders (GLSL) -------\n");
	
	rb_program_generic			= new rb_generic_c();
	rb_program_zfill			= new rb_zfill_c();
	
	rb_program_lighting_R			= new rb_lighting_R_c();
	rb_program_lighting_RB			= new rb_lighting_RB_c();
	rb_program_lighting_RBH			= new rb_lighting_RBH_c();
	rb_program_lighting_RBHS		= new rb_lighting_RBHS_c();
	rb_program_lighting_RBS			= new rb_lighting_RBS_c();
	
	rb_program_lighting_D_vstatic		= new rb_lighting_D_vstatic_c();
	rb_program_lighting_DB_vstatic		= new rb_lighting_DB_vstatic_c();
	rb_program_lighting_DBH_vstatic		= new rb_lighting_DBH_vstatic_c();
	rb_program_lighting_DBHS_vstatic	= new rb_lighting_DBHS_vstatic_c();
	rb_program_lighting_DBS_vstatic		= new rb_lighting_DBS_vstatic_c();
		
	rb_program_lighting_D_omni		= new rb_lighting_D_omni_c();
	rb_program_lighting_D_proj		= new rb_lighting_D_proj_c();
	rb_program_lighting_DB_omni		= new rb_lighting_DB_omni_c();
	rb_program_lighting_DBH_omni		= new rb_lighting_DBH_omni_c();
	rb_program_lighting_DBHS_omni		= new rb_lighting_DBHS_omni_c();
	rb_program_lighting_DBS_omni		= new rb_lighting_DBS_omni_c();
	
	rb_program_fog_uniform			= new rb_fog_uniform_c();
	
	rb_program_reflection_C			= new rb_reflection_C_c();
	rb_program_refraction_C			= new rb_refraction_C_c();
	rb_program_dispersion_C			= new rb_dispersion_C_c();
	rb_program_liquid_C			= new rb_liquid_C_c();
	
	rb_program_skybox			= new rb_skybox_c();
	rb_program_skycloud			= new rb_skycloud_c();
	
	rb_program_heathaze			= new rb_heathaze_c();
}

void		RB_ShutdownGPUShaders()
{
	delete rb_program_generic;
	delete rb_program_zfill;
	
	delete rb_program_lighting_R;
	delete rb_program_lighting_RB;
	delete rb_program_lighting_RBH;
	delete rb_program_lighting_RBHS;
	delete rb_program_lighting_RBS;
	
	delete rb_program_lighting_D_vstatic;
	delete rb_program_lighting_DB_vstatic;
	delete rb_program_lighting_DBH_vstatic;
	delete rb_program_lighting_DBHS_vstatic;
	delete rb_program_lighting_DBS_vstatic;
	
	delete rb_program_lighting_D_omni;
	delete rb_program_lighting_D_proj;
	delete rb_program_lighting_DB_omni;
	delete rb_program_lighting_DBH_omni;
	delete rb_program_lighting_DBHS_omni;
	delete rb_program_lighting_DBS_omni;
	
	delete rb_program_fog_uniform;
	
	delete rb_program_reflection_C;
	delete rb_program_refraction_C;
	delete rb_program_dispersion_C;
	delete rb_program_liquid_C;
	
	delete rb_program_skybox;
	delete rb_program_skycloud;
	
	delete rb_program_heathaze;
}


void		RB_EnableShader_generic()
{
//	rb_program_generic->enable();		RB_CheckForError();

	xglEnableClientState(GL_VERTEX_ARRAY);			RB_CheckForError();

	RB_SelectTexture(GL_TEXTURE0);	
	xglEnableClientState(GL_TEXTURE_COORD_ARRAY);		RB_CheckForError();
	xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
}

void		RB_DisableShader_generic()
{
//	rb_program_generic->disable();		RB_CheckForError();

	xglDisableClientState(GL_VERTEX_ARRAY);			RB_CheckForError();

	RB_SelectTexture(GL_TEXTURE0);
	xglDisableClientState(GL_TEXTURE_COORD_ARRAY);		RB_CheckForError();
	xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
}

void		RB_RenderCommand_generic(const r_command_t *cmd,			const r_shader_stage_c *stage)
{
	RB_EnableShaderStageStates(cmd->getEntity(), stage);

//	rb_program_generic->setVertexAttribs(cmd);

	if(gl_config.arb_vertex_buffer_object && cmd->getEntityMesh()->vbo_array_buffer)
	{
		xglBindBufferARB(GL_ARRAY_BUFFER_ARB, cmd->getEntityMesh()->vbo_array_buffer);	RB_CheckForError();
		
#if !defined(DOUBLEVEC_T) && defined(SIMD_SSE)
		xglVertexPointer(3, GL_FLOAT, 16, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_vertexes_ofs));		RB_CheckForError();
#else
		xglVertexPointer(3, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_vertexes_ofs));		RB_CheckForError();
#endif
		RB_SelectTexture(GL_TEXTURE0);
		xglTexCoordPointer(2, GL_FLOAT, 0, VBO_BUFFER_OFFSET(cmd->getEntityMesh()->vbo_texcoords_ofs));		RB_CheckForError();
	}
	else
	{
		if(gl_config.arb_vertex_buffer_object)
		{
			gl_state.current_vbo_array_buffer = 0;
			gl_state.current_vbo_vertexes_ofs = 0;
			
			xglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);	RB_CheckForError();
		}
		
#if !defined(DOUBLEVEC_T) && defined(SIMD_SSE)
		xglVertexPointer(3, GL_FLOAT, 16, &(cmd->getEntityMesh()->vertexes[0]));	RB_CheckForError();
#else
		xglVertexPointer(3, GL_FLOAT, 0, &(cmd->getEntityMesh()->vertexes[0]));		RB_CheckForError();
#endif
		
		RB_SelectTexture(GL_TEXTURE0);
		xglTexCoordPointer(2, GL_FLOAT, 0, &(cmd->getEntityMesh()->texcoords[0]));	RB_CheckForError();
	}
	
	RB_ModifyTextureMatrix(cmd->getEntity(), stage);	RB_CheckForError();
	RB_Bind(stage->image);

	// set gl_Color
	vec4_c color;
	RB_ModifyColor(cmd->getEntity()->getShared(), stage, color);
	xglColor4fv(color);	RB_CheckForError();

	RB_FlushMesh(cmd);
	
	RB_DisableShaderStageStates(cmd->getEntity(), stage);
}


void		RB_EnableShader_zfill()
{
	rb_program_zfill->enable();	RB_CheckForError();
}

void		RB_DisableShader_zfill()
{
	rb_program_zfill->disable();	RB_CheckForError();
}

void		RB_RenderCommand_zfill(const r_command_t *cmd,				const r_shader_stage_c *stage)
{
	xglColor4fv(color_black);

//	RB_EnableShaderStageStates(cmd->getEntity(), stage);

	rb_program_zfill->setVertexAttribs(cmd);

	if(stage->flags & SHADER_STAGE_ALPHATEST)
	{
		xglEnable(GL_ALPHA_TEST);
		float	value = X_bound(0.0, RB_Evaluate(cmd->getEntity()->getShared(), stage->alpha_ref, 0.5), 1.0);		
		xglAlphaFunc(GL_GREATER, value);
	
		RB_ModifyTextureMatrix(cmd->getEntity(), stage);
		RB_Bind(stage->image);
		xglEnable(GL_TEXTURE_2D);		RB_CheckForError();
	}
	else
	{
		/*
		xglMatrixMode(GL_TEXTURE);
		xglLoadIdentity();
		xglMatrixMode(GL_MODELVIEW);

		RB_Bind(r_img_black);
		*/
	}
	
	RB_FlushMesh(cmd);
	
	if(stage->flags & SHADER_STAGE_ALPHATEST)
	{
		xglDisable(GL_ALPHA_TEST);
		xglDisable(GL_TEXTURE_2D);		RB_CheckForError();
	}
	
//	RB_DisableShaderStageStates(cmd->getEntity(), stage);
}


void		RB_EnableShader_lighting_R()
{
	rb_program_lighting_R->enable();	RB_CheckForError();
}

void		RB_DisableShader_lighting_R()
{
	rb_program_lighting_R->disable();	RB_CheckForError();
}

void		RB_RenderCommand_lighting_R(const r_command_t *cmd,			const r_shader_stage_c *stage_diffusemap,
											const r_shader_stage_c *stage_lightmap,
											const r_shader_stage_c *stage_deluxemap)
{
	RB_EnableShaderStageStates(cmd->getEntity(), stage_diffusemap);
	
	rb_program_lighting_R->setVertexAttribs(cmd);
	
	RB_SelectTexture(GL_TEXTURE0);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_diffusemap);
	RB_Bind(stage_diffusemap->image);
	
	RB_SelectTexture(GL_TEXTURE1);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_lightmap);
	RB_Bind(R_GetLightMapImageByNum(cmd->getInfoKey()));
	
	RB_SelectTexture(GL_TEXTURE2);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_deluxemap);
	RB_Bind(R_GetLightMapImageByNum(cmd->getInfoKey() + 1));
	
	RB_FlushMesh(cmd);
	
	RB_DisableShaderStageStates(cmd->getEntity(), stage_diffusemap);
}
										
			
void		RB_EnableShader_lighting_RB()
{
	rb_program_lighting_RB->enable();	RB_CheckForError();
}

void		RB_DisableShader_lighting_RB()
{
	rb_program_lighting_RB->disable();	RB_CheckForError();
}

void		RB_RenderCommand_lighting_RB(const r_command_t *cmd,			const r_shader_stage_c *stage_diffusemap,
											const r_shader_stage_c *stage_bumpmap,
											const r_shader_stage_c *stage_lightmap,
											const r_shader_stage_c *stage_deluxemap)
{
	RB_EnableShaderStageStates(cmd->getEntity(), stage_diffusemap);
	
	rb_program_lighting_RB->setVertexAttribs(cmd);

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
	
	rb_program_lighting_RB->setUniform_bump_scale(cmd, stage_bumpmap);
	
	RB_FlushMesh(cmd);
	
	RB_DisableShaderStageStates(cmd->getEntity(), stage_diffusemap);
}
										

void		RB_EnableShader_lighting_RBH()
{
	rb_program_lighting_RBH->enable();	RB_CheckForError();
}

void		RB_DisableShader_lighting_RBH()
{
	rb_program_lighting_RBH->disable();	RB_CheckForError();
}

void		RB_RenderCommand_lighting_RBH(const r_command_t *cmd,			const r_shader_stage_c *stage_diffusemap,
											const r_shader_stage_c *stage_bumpmap,
											const r_shader_stage_c *stage_lightmap,
											const r_shader_stage_c *stage_deluxemap)
{
	RB_EnableShaderStageStates(cmd->getEntity(), stage_diffusemap);
	
	rb_program_lighting_RBH->setVertexAttribs(cmd);

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
	
	rb_program_lighting_RBH->setUniform_view_origin(cmd);
	rb_program_lighting_RBH->setUniform_bump_scale(cmd, stage_bumpmap);
	rb_program_lighting_RBH->setUniform_height_scale(cmd, stage_bumpmap);
	rb_program_lighting_RBH->setUniform_height_bias(cmd, stage_bumpmap);
	
	RB_FlushMesh(cmd);
	
	RB_DisableShaderStageStates(cmd->getEntity(), stage_diffusemap);
}
										

void		RB_EnableShader_lighting_RBHS()
{
	rb_program_lighting_RBHS->enable();	RB_CheckForError();
}

void		RB_DisableShader_lighting_RBHS()
{
	rb_program_lighting_RBHS->disable();	RB_CheckForError();
}

void		RB_RenderCommand_lighting_RBHS(const r_command_t *cmd,			const r_shader_stage_c *stage_diffusemap,
											const r_shader_stage_c *stage_bumpmap,
											const r_shader_stage_c *stage_specularmap,
											const r_shader_stage_c *stage_lightmap,
											const r_shader_stage_c *stage_deluxemap)
{
	RB_EnableShaderStageStates(cmd->getEntity(), stage_diffusemap);
	
	rb_program_lighting_RBHS->setVertexAttribs(cmd);

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
	
	rb_program_lighting_RBHS->setUniform_view_origin(cmd);
	rb_program_lighting_RBHS->setUniform_bump_scale(cmd, stage_bumpmap);
	rb_program_lighting_RBHS->setUniform_height_scale(cmd, stage_bumpmap);
	rb_program_lighting_RBHS->setUniform_height_bias(cmd, stage_bumpmap);
	rb_program_lighting_RBHS->setUniform_specular_exponent(cmd, stage_specularmap);
	
	RB_FlushMesh(cmd);
	
	RB_DisableShaderStageStates(cmd->getEntity(), stage_diffusemap);
}
										
									
void		RB_EnableShader_lighting_RBS()
{
	rb_program_lighting_RBS->enable();	RB_CheckForError();
}

void		RB_DisableShader_lighting_RBS()
{
	rb_program_lighting_RBS->disable();	RB_CheckForError();
}

void		RB_RenderCommand_lighting_RBS(const r_command_t *cmd,			const r_shader_stage_c *stage_diffusemap,
											const r_shader_stage_c *stage_bumpmap,
											const r_shader_stage_c *stage_specularmap,
											const r_shader_stage_c *stage_lightmap,
											const r_shader_stage_c *stage_deluxemap)
{
	RB_EnableShaderStageStates(cmd->getEntity(), stage_diffusemap);
	
	rb_program_lighting_RBS->setVertexAttribs(cmd);

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
	
	rb_program_lighting_RBS->setUniform_view_origin(cmd);
	rb_program_lighting_RBS->setUniform_bump_scale(cmd, stage_bumpmap);
	rb_program_lighting_RBS->setUniform_specular_exponent(cmd, stage_specularmap);
	
	RB_FlushMesh(cmd);
	
	RB_DisableShaderStageStates(cmd->getEntity(), stage_diffusemap);
}


void		RB_EnableShader_lighting_D_vstatic()
{
	rb_program_lighting_D_vstatic->enable();	RB_CheckForError();
}

void		RB_DisableShader_lighting_D_vstatic()
{
	rb_program_lighting_D_vstatic->disable();	RB_CheckForError();
}

void		RB_RenderCommand_lighting_D_vstatic(const r_command_t *cmd,		const r_shader_stage_c *stage_diffusemap)
{
	RB_EnableShaderStageStates(cmd->getEntity(), stage_diffusemap);
		
	rb_program_lighting_D_vstatic->setVertexAttribs(cmd);
	
	RB_SelectTexture(GL_TEXTURE0);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_diffusemap);
	RB_Bind(stage_diffusemap->image);
	
	RB_FlushMesh(cmd);
	
	RB_DisableShaderStageStates(cmd->getEntity(), stage_diffusemap);
}



void		RB_EnableShader_lighting_DB_vstatic()
{
	rb_program_lighting_DB_vstatic->enable();	RB_CheckForError();
}

void		RB_DisableShader_lighting_DB_vstatic()
{
	rb_program_lighting_DB_vstatic->disable();	RB_CheckForError();
}

void		RB_RenderCommand_lighting_DB_vstatic(const r_command_t *cmd,		const r_shader_stage_c *stage_diffusemap,
											const r_shader_stage_c *stage_bumpmap)
{
	RB_EnableShaderStageStates(cmd->getEntity(), stage_diffusemap);
	
	rb_program_lighting_DB_vstatic->setVertexAttribs(cmd);
	
	RB_SelectTexture(GL_TEXTURE0);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_diffusemap);
	RB_Bind(stage_diffusemap->image);
	
	RB_SelectTexture(GL_TEXTURE1);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_bumpmap);
	RB_Bind(stage_bumpmap->image);
	
	rb_program_lighting_DB_vstatic->setUniform_bump_scale(cmd, stage_bumpmap);

	RB_FlushMesh(cmd);
	
	RB_DisableShaderStageStates(cmd->getEntity(), stage_diffusemap);
}


void		RB_EnableShader_lighting_DBH_vstatic()
{
	rb_program_lighting_DBH_vstatic->enable();	RB_CheckForError();
}

void		RB_DisableShader_lighting_DBH_vstatic()
{
	rb_program_lighting_DBH_vstatic->disable();	RB_CheckForError();
}

void		RB_RenderCommand_lighting_DBH_vstatic(const r_command_t *cmd,		const r_shader_stage_c *stage_diffusemap,
											const r_shader_stage_c *stage_bumpmap)
{
	RB_EnableShaderStageStates(cmd->getEntity(), stage_diffusemap);
	
	rb_program_lighting_DBH_vstatic->setVertexAttribs(cmd);
	
	RB_SelectTexture(GL_TEXTURE0);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_diffusemap);
	RB_Bind(stage_diffusemap->image);
	
	RB_SelectTexture(GL_TEXTURE1);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_bumpmap);
	RB_Bind(stage_bumpmap->image);
	
	rb_program_lighting_DBH_vstatic->setUniform_view_origin(cmd);
	rb_program_lighting_DBH_vstatic->setUniform_bump_scale(cmd, stage_bumpmap);
	rb_program_lighting_DBH_vstatic->setUniform_height_scale(cmd, stage_bumpmap);
	rb_program_lighting_DBH_vstatic->setUniform_height_bias(cmd, stage_bumpmap);

	RB_FlushMesh(cmd);
	
	RB_DisableShaderStageStates(cmd->getEntity(), stage_diffusemap);
}



void		RB_EnableShader_lighting_DBHS_vstatic()
{
	rb_program_lighting_DBHS_vstatic->enable();	RB_CheckForError();
}

void		RB_DisableShader_lighting_DBHS_vstatic()
{
	rb_program_lighting_DBHS_vstatic->disable();	RB_CheckForError();
}

void		RB_RenderCommand_lighting_DBHS_vstatic(const r_command_t *cmd,		const r_shader_stage_c *stage_diffusemap,
											const r_shader_stage_c *stage_bumpmap,
											const r_shader_stage_c *stage_specularmap)
{
	RB_EnableShaderStageStates(cmd->getEntity(), stage_diffusemap);
					
	rb_program_lighting_DBHS_vstatic->setVertexAttribs(cmd);
	
	RB_SelectTexture(GL_TEXTURE0);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_diffusemap);
	RB_Bind(stage_diffusemap->image);
	
	RB_SelectTexture(GL_TEXTURE1);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_bumpmap);
	RB_Bind(stage_bumpmap->image);
	
	RB_SelectTexture(GL_TEXTURE2);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_specularmap);
	RB_Bind(stage_specularmap->image);
	
	rb_program_lighting_DBHS_vstatic->setUniform_view_origin(cmd);
	rb_program_lighting_DBHS_vstatic->setUniform_bump_scale(cmd, stage_bumpmap);
	rb_program_lighting_DBHS_vstatic->setUniform_height_scale(cmd, stage_bumpmap);
	rb_program_lighting_DBHS_vstatic->setUniform_height_bias(cmd, stage_bumpmap);
	rb_program_lighting_DBHS_vstatic->setUniform_specular_exponent(cmd, stage_specularmap);

	RB_FlushMesh(cmd);
	
	RB_DisableShaderStageStates(cmd->getEntity(), stage_diffusemap);
}



void		RB_EnableShader_lighting_DBS_vstatic()
{
	rb_program_lighting_DBS_vstatic->enable();	RB_CheckForError();
}

void		RB_DisableShader_lighting_DBS_vstatic()
{
	rb_program_lighting_DBS_vstatic->disable();	RB_CheckForError();
}

void		RB_RenderCommand_lighting_DBS_vstatic(const r_command_t *cmd,		const r_shader_stage_c *stage_diffusemap,
											const r_shader_stage_c *stage_bumpmap,
											const r_shader_stage_c *stage_specularmap)
{
	RB_EnableShaderStageStates(cmd->getEntity(), stage_diffusemap);
					
	rb_program_lighting_DBS_vstatic->setVertexAttribs(cmd);
	
	RB_SelectTexture(GL_TEXTURE0);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_diffusemap);
	RB_Bind(stage_diffusemap->image);
	
	RB_SelectTexture(GL_TEXTURE1);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_bumpmap);
	RB_Bind(stage_bumpmap->image);
	
	RB_SelectTexture(GL_TEXTURE2);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_specularmap);
	RB_Bind(stage_specularmap->image);
	
	rb_program_lighting_DBS_vstatic->setUniform_view_origin(cmd);
	rb_program_lighting_DBS_vstatic->setUniform_bump_scale(cmd, stage_bumpmap);
	rb_program_lighting_DBS_vstatic->setUniform_specular_exponent(cmd, stage_specularmap);

	RB_FlushMesh(cmd);
	
	RB_DisableShaderStageStates(cmd->getEntity(), stage_diffusemap);
}



void		RB_EnableShader_lighting_D_omni()
{
	rb_program_lighting_D_omni->enable();	RB_CheckForError();
}

void		RB_DisableShader_lighting_D_omni()
{
	rb_program_lighting_D_omni->disable();	RB_CheckForError();
}

void		RB_RenderCommand_lighting_D_omni(const r_command_t *cmd,		const r_shader_stage_c *stage_diffusemap,
											const r_shader_stage_c *stage_attenuationmap_xy,
											const r_shader_stage_c *stage_attenuationmap_z,
											const r_shader_stage_c *stage_attenuationmap_cube)
{
	RB_EnableShaderStageStates(cmd->getEntity(), stage_diffusemap);
		
	rb_program_lighting_D_omni->setVertexAttribs(cmd);
	
	RB_SelectTexture(GL_TEXTURE0);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_diffusemap);
	RB_Bind(stage_diffusemap->image);
	
	RB_SelectTexture(GL_TEXTURE1);
	RB_ModifyOmniLightTextureMatrix(cmd, stage_attenuationmap_xy);
	RB_Bind(stage_attenuationmap_xy->image);
	
	RB_SelectTexture(GL_TEXTURE2);
	xglMatrixMode(GL_TEXTURE);
	xglLoadIdentity();
	xglMatrixMode(GL_MODELVIEW);
	RB_Bind(stage_attenuationmap_z->image);
	
//	RB_SelectTexture(GL_TEXTURE3);
//	RB_ModifyOmniLightCubeTextureMatrix(cmd, stage_attenuationmap_xy);
//	RB_Bind(stage_attenuationmap_cube->image);

//	RB_SelectTexture(GL_TEXTURE4);
//	xglMatrixMode(GL_TEXTURE);
//	xglLoadTransposeMatrixfARB(&(cmd->getLightTransform())[0][0]);
//	xglMatrixMode(GL_MODELVIEW);
//	cmd->getLight()->getShadowMap()->bind();
	
	rb_program_lighting_D_omni->setUniform_light_origin(cmd);
	rb_program_lighting_D_omni->setUniform_light_color(cmd, stage_attenuationmap_xy);
	
	RB_FlushMesh(cmd);
	
	RB_DisableShaderStageStates(cmd->getEntity(), stage_diffusemap);
}
											
void		RB_EnableShader_lighting_D_proj()
{
	rb_program_lighting_D_proj->enable();	RB_CheckForError();
}

void		RB_DisableShader_lighting_D_proj()
{
	rb_program_lighting_D_proj->disable();	RB_CheckForError();
}

void		RB_RenderCommand_lighting_D_proj(const r_command_t *cmd,		const r_shader_stage_c *stage_diffusemap,
											const r_shader_stage_c *stage_attenuationmap_xy,
											const r_shader_stage_c *stage_attenuationmap_z)
{
	RB_EnableShaderStageStates(cmd->getEntity(), stage_diffusemap);
			
	rb_program_lighting_D_proj->setVertexAttribs(cmd);
	
	RB_SelectTexture(GL_TEXTURE0);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_diffusemap);
	RB_Bind(stage_diffusemap->image);
	
	RB_SelectTexture(GL_TEXTURE1);
	RB_ModifyProjLightTextureMatrix(cmd, stage_attenuationmap_xy);
	RB_Bind(stage_attenuationmap_xy->image);

//	RB_SelectTexture(GL_TEXTURE2);
//	xglMatrixMode(GL_TEXTURE);
//	xglLoadTransposeMatrixfARB(&(cmd->getLight()->getShadowMapProjection())[0][0]);
//	xglMatrixMode(GL_MODELVIEW);
//	cmd->getLight()->getShadowMap()->bind();
	
	rb_program_lighting_D_proj->setUniform_light_origin(cmd);
	rb_program_lighting_D_proj->setUniform_light_color(cmd, stage_attenuationmap_xy);
	
	RB_FlushMesh(cmd);
	
	RB_DisableShaderStageStates(cmd->getEntity(), stage_diffusemap);
}


void		RB_EnableShader_lighting_DB_omni()
{
	rb_program_lighting_DB_omni->enable();	RB_CheckForError();
}

void		RB_DisableShader_lighting_DB_omni()
{
	rb_program_lighting_DB_omni->disable();	RB_CheckForError();
}

void		RB_RenderCommand_lighting_DB_omni(const r_command_t *cmd,		const r_shader_stage_c *stage_diffusemap,
											const r_shader_stage_c *stage_bumpmap,
											const r_shader_stage_c *stage_attenuationmap_xy,
											const r_shader_stage_c *stage_attenuationmap_z,
											const r_shader_stage_c *stage_attenuationmap_cube)
{
	RB_EnableShaderStageStates(cmd->getEntity(), stage_diffusemap);
	
	rb_program_lighting_DB_omni->setVertexAttribs(cmd);
	
	RB_SelectTexture(GL_TEXTURE0);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_diffusemap);
	RB_Bind(stage_diffusemap->image);
	
	RB_SelectTexture(GL_TEXTURE1);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_bumpmap);
	RB_Bind(stage_bumpmap->image);
	
	RB_SelectTexture(GL_TEXTURE2);
	RB_ModifyOmniLightTextureMatrix(cmd, stage_attenuationmap_xy);
	RB_Bind(stage_attenuationmap_xy->image);
	
	RB_SelectTexture(GL_TEXTURE3);
	xglMatrixMode(GL_TEXTURE);
	xglLoadIdentity();
	xglMatrixMode(GL_MODELVIEW);
	RB_Bind(stage_attenuationmap_z->image);
	
//	RB_SelectTexture(GL_TEXTURE4);
//	RB_ModifyOmniLightCubeTextureMatrix(cmd, stage_attenuationmap_xy);
//	RB_Bind(stage_attenuationmap_cube->image);
	
	rb_program_lighting_DB_omni->setUniform_light_origin(cmd);
	rb_program_lighting_DB_omni->setUniform_light_color(cmd, stage_attenuationmap_xy);
	rb_program_lighting_DB_omni->setUniform_bump_scale(cmd, stage_bumpmap);

	RB_FlushMesh(cmd);
	
	RB_DisableShaderStageStates(cmd->getEntity(), stage_diffusemap);
}
										

void		RB_EnableShader_lighting_DBH_omni()
{
	rb_program_lighting_DBH_omni->enable();		RB_CheckForError();
}

void		RB_DisableShader_lighting_DBH_omni()
{
	rb_program_lighting_DBH_omni->disable();	RB_CheckForError();
}

void		RB_RenderCommand_lighting_DBH_omni(const r_command_t *cmd,		const r_shader_stage_c *stage_diffusemap,
											const r_shader_stage_c *stage_bumpmap,
											const r_shader_stage_c *stage_attenuationmap_xy,
											const r_shader_stage_c *stage_attenuationmap_z,
											const r_shader_stage_c *stage_attenuationmap_cube)
{
	RB_EnableShaderStageStates(cmd->getEntity(), stage_diffusemap);
	
	rb_program_lighting_DBH_omni->setVertexAttribs(cmd);
	
	RB_SelectTexture(GL_TEXTURE0);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_diffusemap);
	RB_Bind(stage_diffusemap->image);
	
	RB_SelectTexture(GL_TEXTURE1);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_bumpmap);
	RB_Bind(stage_bumpmap->image);
	
	RB_SelectTexture(GL_TEXTURE2);
	RB_ModifyOmniLightTextureMatrix(cmd, stage_attenuationmap_xy);
	RB_Bind(stage_attenuationmap_xy->image);
	
	RB_SelectTexture(GL_TEXTURE3);
	xglMatrixMode(GL_TEXTURE);
	xglLoadIdentity();
	xglMatrixMode(GL_MODELVIEW);
	RB_Bind(stage_attenuationmap_z->image);
	
//	RB_SelectTexture(GL_TEXTURE4);
//	RB_ModifyOmniLightCubeTextureMatrix(cmd, stage_attenuationmap_xy);
//	RB_Bind(stage_attenuationmap_cube->image);
	
	rb_program_lighting_DBH_omni->setUniform_view_origin(cmd);
	rb_program_lighting_DBH_omni->setUniform_light_origin(cmd);
	rb_program_lighting_DBH_omni->setUniform_light_color(cmd, stage_attenuationmap_xy);
	rb_program_lighting_DBH_omni->setUniform_bump_scale(cmd, stage_bumpmap);
	rb_program_lighting_DBH_omni->setUniform_height_scale(cmd, stage_bumpmap);
	rb_program_lighting_DBH_omni->setUniform_height_bias(cmd, stage_bumpmap);

	RB_FlushMesh(cmd);
	
	RB_DisableShaderStageStates(cmd->getEntity(), stage_diffusemap);
}


void		RB_EnableShader_lighting_DBHS_omni()
{
	rb_program_lighting_DBHS_omni->enable();	RB_CheckForError();
}

void		RB_DisableShader_lighting_DBHS_omni()
{
	rb_program_lighting_DBHS_omni->disable();	RB_CheckForError();
}

void		RB_RenderCommand_lighting_DBHS_omni(const r_command_t *cmd,		const r_shader_stage_c *stage_diffusemap,
											const r_shader_stage_c *stage_bumpmap,
											const r_shader_stage_c *stage_specularmap,
											const r_shader_stage_c *stage_attenuationmap_xy,
											const r_shader_stage_c *stage_attenuationmap_z,
											const r_shader_stage_c *stage_attenuationmap_cube)
{
	RB_EnableShaderStageStates(cmd->getEntity(), stage_diffusemap);
					
	rb_program_lighting_DBHS_omni->setVertexAttribs(cmd);
	
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
	RB_ModifyOmniLightTextureMatrix(cmd, stage_attenuationmap_xy);
	RB_Bind(stage_attenuationmap_xy->image);
	
	RB_SelectTexture(GL_TEXTURE4);
	xglMatrixMode(GL_TEXTURE);
	xglLoadIdentity();
	xglMatrixMode(GL_MODELVIEW);
	RB_Bind(stage_attenuationmap_z->image);
	
//	RB_SelectTexture(GL_TEXTURE5);
//	RB_ModifyOmniLightCubeTextureMatrix(cmd, stage_attenuationmap_xy);
//	RB_Bind(stage_attenuationmap_cube->image);
	
	rb_program_lighting_DBHS_omni->setUniform_view_origin(cmd);
	rb_program_lighting_DBHS_omni->setUniform_light_origin(cmd);
	rb_program_lighting_DBHS_omni->setUniform_light_color(cmd, stage_attenuationmap_xy);
	rb_program_lighting_DBHS_omni->setUniform_bump_scale(cmd, stage_bumpmap);
	rb_program_lighting_DBHS_omni->setUniform_height_scale(cmd, stage_bumpmap);
	rb_program_lighting_DBHS_omni->setUniform_height_bias(cmd, stage_bumpmap);
	rb_program_lighting_DBHS_omni->setUniform_specular_exponent(cmd, stage_specularmap);

	RB_FlushMesh(cmd);
	
	RB_DisableShaderStageStates(cmd->getEntity(), stage_diffusemap);
}
								
								
void		RB_EnableShader_lighting_DBS_omni()
{
	rb_program_lighting_DBS_omni->enable();		RB_CheckForError();
}

void		RB_DisableShader_lighting_DBS_omni()
{
	rb_program_lighting_DBS_omni->disable();	RB_CheckForError();
}

void		RB_RenderCommand_lighting_DBS_omni(const r_command_t *cmd,		const r_shader_stage_c *stage_diffusemap,
											const r_shader_stage_c *stage_bumpmap,
											const r_shader_stage_c *stage_specularmap,
											const r_shader_stage_c *stage_attenuationmap_xy,
											const r_shader_stage_c *stage_attenuationmap_z,
											const r_shader_stage_c *stage_attenuationmap_cube)
{
	RB_EnableShaderStageStates(cmd->getEntity(), stage_diffusemap);
	
	rb_program_lighting_DBS_omni->setVertexAttribs(cmd);

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
	RB_ModifyOmniLightTextureMatrix(cmd, stage_attenuationmap_xy);
	RB_Bind(stage_attenuationmap_xy->image);
	
	RB_SelectTexture(GL_TEXTURE4);
	xglMatrixMode(GL_TEXTURE);
	xglLoadIdentity();
	xglMatrixMode(GL_MODELVIEW);
	RB_Bind(stage_attenuationmap_z->image);
	
//	RB_SelectTexture(GL_TEXTURE5);
//	RB_ModifyOmniLightCubeTextureMatrix(cmd, stage_attenuationmap_xy);
//	RB_Bind(stage_attenuationmap_cube->image);
	
	rb_program_lighting_DBS_omni->setUniform_view_origin(cmd);
	rb_program_lighting_DBS_omni->setUniform_light_origin(cmd);
	rb_program_lighting_DBS_omni->setUniform_light_color(cmd, stage_attenuationmap_xy);
	rb_program_lighting_DBS_omni->setUniform_bump_scale(cmd, stage_bumpmap);
	rb_program_lighting_DBS_omni->setUniform_specular_exponent(cmd, stage_specularmap);

	RB_FlushMesh(cmd);
	
	RB_DisableShaderStageStates(cmd->getEntity(), stage_diffusemap);
}



void		RB_EnableShader_fog_uniform()
{
	rb_program_fog_uniform->enable();		RB_CheckForError();
}

void		RB_DisableShader_fog_uniform()
{
	rb_program_fog_uniform->disable();	RB_CheckForError();
}

void		RB_RenderCommand_fog_uniform(const r_command_t *cmd,		const r_shader_stage_c *stage)
{
	RB_EnableShaderStageStates(cmd->getEntity(), stage);
	
	rb_program_fog_uniform->setVertexAttribs(cmd);
	
	RB_SelectTexture(GL_TEXTURE0);
	xglMatrixMode(GL_TEXTURE);
	xglLoadIdentity();
	xglMatrixMode(GL_MODELVIEW);
	RB_Bind(r_img_currentrender);

	rb_program_fog_uniform->setUniform_fbuf_scale(cmd);
	rb_program_fog_uniform->setUniform_npot_scale(cmd);
	rb_program_fog_uniform->setUniform_fog_density(cmd, stage);
	rb_program_fog_uniform->setUniform_fog_color(cmd, stage);

	RB_FlushMesh(cmd);
	
	RB_DisableShaderStageStates(cmd->getEntity(), stage);
}



void		RB_EnableShader_reflection_C()
{
	rb_program_reflection_C->enable();	RB_CheckForError();
}

void		RB_DisableShader_reflection_C()
{
	rb_program_reflection_C->disable();	RB_CheckForError();
}

void		RB_RenderCommand_reflection_C(const r_command_t *cmd,			const r_shader_stage_c *stage_colormap)
{
	RB_EnableShaderStageStates(cmd->getEntity(), stage_colormap);
	
	rb_program_reflection_C->setVertexAttribs(cmd);

	RB_SelectTexture(GL_TEXTURE0);
	xglMatrixMode(GL_TEXTURE);
	xglLoadTransposeMatrixfARB(&rb_matrix_model[0][0]);
	xglMatrixMode(GL_MODELVIEW);
	RB_Bind(stage_colormap->image);

	rb_program_reflection_C->setUniform_view_origin_inWorldSpace(cmd);

	RB_FlushMesh(cmd);
	
	RB_DisableShaderStageStates(cmd->getEntity(), stage_colormap);
}


void		RB_EnableShader_refraction_C()
{
	rb_program_refraction_C->enable();	RB_CheckForError();
}

void		RB_DisableShader_refraction_C()
{
	rb_program_refraction_C->disable();	RB_CheckForError();
}

void		RB_RenderCommand_refraction_C(const r_command_t *cmd,			const r_shader_stage_c *stage_colormap)
{
	RB_EnableShaderStageStates(cmd->getEntity(), stage_colormap);
	
	rb_program_refraction_C->setVertexAttribs(cmd);

	RB_SelectTexture(GL_TEXTURE0);
	xglMatrixMode(GL_TEXTURE);
	xglLoadTransposeMatrixfARB(&rb_matrix_model[0][0]);
	xglMatrixMode(GL_MODELVIEW);
	RB_Bind(stage_colormap->image);
	
	rb_program_refraction_C->setUniform_view_origin_inWorldSpace(cmd);
	rb_program_refraction_C->setUniform_refraction_index(cmd, stage_colormap);
	rb_program_refraction_C->setUniform_fresnel_power(cmd, stage_colormap);
	rb_program_refraction_C->setUniform_fresnel_scale(cmd, stage_colormap);
	rb_program_refraction_C->setUniform_fresnel_bias(cmd, stage_colormap);
	
	RB_FlushMesh(cmd);
	
	RB_DisableShaderStageStates(cmd->getEntity(), stage_colormap);
}


void		RB_EnableShader_dispersion_C()
{
	rb_program_dispersion_C->enable();	RB_CheckForError();
}

void		RB_DisableShader_dispersion_C()
{
	rb_program_dispersion_C->disable();	RB_CheckForError();
}

void		RB_RenderCommand_dispersion_C(const r_command_t *cmd,			const r_shader_stage_c *stage_colormap)
{
	RB_EnableShaderStageStates(cmd->getEntity(), stage_colormap);
	
	rb_program_dispersion_C->setVertexAttribs(cmd);

	RB_SelectTexture(GL_TEXTURE0);
	xglMatrixMode(GL_TEXTURE);
	xglLoadTransposeMatrixfARB(&rb_matrix_model[0][0]);
	xglMatrixMode(GL_MODELVIEW);
	RB_Bind(stage_colormap->image);
	
	rb_program_dispersion_C->setUniform_view_origin_inWorldSpace(cmd);
	rb_program_dispersion_C->setUniform_eta_ratio(cmd, stage_colormap);
	rb_program_dispersion_C->setUniform_fresnel_power(cmd, stage_colormap);
	rb_program_dispersion_C->setUniform_fresnel_scale(cmd, stage_colormap);
	rb_program_dispersion_C->setUniform_fresnel_bias(cmd, stage_colormap);
	
	RB_FlushMesh(cmd);
	
	RB_DisableShaderStageStates(cmd->getEntity(), stage_colormap);
}


void		RB_EnableShader_liquid_C()
{
	rb_program_liquid_C->enable();		RB_CheckForError();
}

void		RB_DisableShader_liquid_C()
{
	rb_program_liquid_C->disable();		RB_CheckForError();
}

void		RB_RenderCommand_liquid_C(const r_command_t *cmd,			const r_shader_stage_c *stage_colormap)
{
	RB_EnableShaderStageStates(cmd->getEntity(), stage_colormap);
	
	rb_program_liquid_C->setVertexAttribs(cmd);

	RB_SelectTexture(GL_TEXTURE0);
	xglMatrixMode(GL_TEXTURE);
	xglLoadTransposeMatrixfARB(&rb_matrix_model[0][0]);
	xglMatrixMode(GL_MODELVIEW);
	RB_Bind(stage_colormap->image);

	rb_program_liquid_C->setUniform_view_origin_inWorldSpace(cmd);

	RB_FlushMesh(cmd);
	
	RB_DisableShaderStageStates(cmd->getEntity(), stage_colormap);
}


void		RB_EnableShader_skybox()
{
	rb_program_skybox->enable();	RB_CheckForError();
}

void		RB_DisableShader_skybox()
{
	rb_program_skybox->disable();	RB_CheckForError();
}

void		RB_RenderCommand_skybox(const r_command_t *cmd,			const r_shader_stage_c *stage_skyboxmap)
{
	RB_EnableShaderStageStates(cmd->getEntity(), stage_skyboxmap);
	
	rb_program_skybox->setVertexAttribs(cmd);

	RB_SelectTexture(GL_TEXTURE0);
	xglMatrixMode(GL_TEXTURE);
	xglLoadTransposeMatrixfARB(&rb_matrix_model[0][0]);
	xglMatrixMode(GL_MODELVIEW);
	RB_Bind(stage_skyboxmap->image);

	rb_program_skybox->setUniform_view_origin_inWorldSpace(cmd);

	RB_FlushMesh(cmd);
	
	RB_DisableShaderStageStates(cmd->getEntity(), stage_skyboxmap);
}


void	RB_EnableShader_skycloud()
{
	rb_program_skycloud->enable();	RB_CheckForError();
}

void	RB_DisableShader_skycloud()
{
	rb_program_skycloud->disable();	RB_CheckForError();
}

void		RB_RenderCommand_skycloud(const r_command_t *cmd,		const r_shader_stage_c *stage_skycloudmap)
{
	RB_EnableShaderStageStates(cmd->getEntity(), stage_skycloudmap);
	
	rb_program_skycloud->setVertexAttribs(cmd);

	RB_SelectTexture(GL_TEXTURE0);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_skycloudmap);
	RB_Bind(stage_skycloudmap->image);

	RB_FlushMesh(cmd);
	
	RB_DisableShaderStageStates(cmd->getEntity(), stage_skycloudmap);
}



void	RB_EnableShader_heathaze()
{
	rb_program_heathaze->enable();		RB_CheckForError();
}

void	RB_DisableShader_heathaze()
{
	rb_program_heathaze->disable();		RB_CheckForError();
}

void	RB_RenderCommand_heathaze(const r_command_t *cmd, const r_shader_stage_c *stage_heathazemap)
{
	RB_EnableShaderStageStates(cmd->getEntity(), stage_heathazemap);
	
	rb_program_heathaze->setVertexAttribs(cmd);

	RB_SelectTexture(GL_TEXTURE0);
	xglMatrixMode(GL_TEXTURE);
	xglLoadIdentity();
	xglMatrixMode(GL_MODELVIEW);
	RB_Bind(r_img_currentrender);
	
	RB_SelectTexture(GL_TEXTURE1);
	RB_ModifyTextureMatrix(cmd->getEntity(), stage_heathazemap);
	RB_Bind(stage_heathazemap->image);
	
	rb_program_heathaze->setUniform_deform_magnitude(cmd, stage_heathazemap);

	rb_program_heathaze->setUniform_fbuf_scale(cmd);
	rb_program_heathaze->setUniform_npot_scale(cmd);
	rb_program_heathaze->setUniform_bump_scale(cmd, stage_heathazemap);

	RB_FlushMesh(cmd);
	
	RB_DisableShaderStageStates(cmd->getEntity(), stage_heathazemap);
}

