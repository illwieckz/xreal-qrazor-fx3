/// ============================================================================
/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2003 Robert Beckebans <trebor_7@users.sourceforge.net>
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
#ifndef REF_H
#define REF_H

/// includes ===================================================================
// system -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
// xreal --------------------------------------------------------------------
// shared -------------------------------------------------------------------
#include "x_shared.h"


#define	MAX_REFLIGHTS		MAX_LIGHTS
#define	MAX_REFENTITIES		MAX_ENTITIES*2
#define	MAX_PARTICLES		4096
#define MAX_PARTICLE_FRAMES	32
#define MAX_POLY_VERTEXES	3000
#define MAX_POLYS		1024

struct roq_info_t;


enum r_light_type_t
{
	LIGHT_OMNI,
	LIGHT_PROJ
};

class r_entity_t
{
public:
	r_entity_t()
	{
		clear();
	}
	
	void	clear()
	{
		origin.clear();
		quat.identity();
		scale		= 1;
	
		model		= -1;
		custom_shader	= -1;
		custom_skin	= -1;
		
		shader_parms[0]	= 1;
		shader_parms[1]	= 1;
		shader_parms[2]	= 1;
		shader_parms[3]	= 1;
		shader_parms[4]	= 0;
		shader_parms[5]	= 0;
		shader_parms[6]	= 0;
		shader_parms[7]	= 0;
		shader_sound	= 0;
		
		frame		= 0;
		frame_old	= 0;
		
		radius.clear();
		radius_bbox.zero();
		radius_value	= 0;
		
		center.clear();
		target.clear();
		right.clear();
		up.clear();
		
		backlerp	= 0;
		flags		= RF_NONE;
	}
	
	// transform
	vec3_c			origin;
	quaternion_c		quat;
	float			scale;
	
	// entity specific
	int			model;			// opaque type outside refresh
	int			custom_shader;		// -1 for inline shader
	int			custom_skin;		// -1 for inline skin
		
	float			shader_parms[8];	// needed by shader system
	float			shader_sound;		// needed by shader system
	
	int			frame;
	int			frame_old;
	
	// light specific
	vec3_c			radius;
	cbbox_c			radius_bbox;
	float			radius_value;
	
	vec3_c			center;
	vec3_c			target;
	vec3_c			right;
	vec3_c			up;
	
	// misc
	float			backlerp;		// 0.0 = current, 1.0 = old
	uint_t			flags;			// renderfx
};


/*
struct r_light_t
{
	r_light_t()
	{
		clear();
	}

	void	clear()
	{
		//type			= LIGHT_POINT;
		
		shader			= -1;
		
		origin.clear();
		color.clear();
		radius_bbox.zero();
		radius_value		= 0;
		
		dir.clear();
		cone_inner		= 0;
		cone_outer		= 0;
	}

	// common for all
	//light_type_t	type;
	
	int		shader;
	
	r_parms_t	parms;
	
	vec3_c		origin;
	vec3_c		color;
	
	cbbox_c		radius_bbox;
	float		radius_value;
	
	// spot light specific
	vec3_c		dir;
	float		cone_inner;
	float		cone_outer;
};
*/


struct r_particle_t
{
	vec3_c			origin;
	
	float			x;
	float			y;
	
	vec4_c			color;
	vec3_c			color_velocity;
	
	/*
	int			anim_frames[MAX_PARTICLE_FRAMES];
	float			anim_frames_num;
	int			anim_cur;
	float			anim_time;
	float			anim_last;
	*/
	
	int			shader;
};

struct r_poly_t
{
	
	//r_vertex_t*		vertexes;
	
	int			shader;
};


struct r_tag_t
{
	vec3_c			origin;
	quaternion_c		quat;
};


struct r_animation_t
{
	int	model;
	int	sequence;

	int	first_frame;
	int	frames_num;
	int	looping_frames;
	int	frames_per_second;
	int	last_frame;
};

struct r_refdef_t
{
	inline r_refdef_t()
	{
		clear();
	}
	
	inline void	clear()
	{
		x		= 0;
		y		= 0;
		
		width		= 0;
		height		= 0;
		
		fov_x		= 90;
		fov_y		= 0;
		
		view_origin.clear();
		view_angles.clear();
		
		time		= 0;
		
		rdflags		= 0;
		
		areabits	= NULL;
	}
	
	inline void	setFOV(float x)
	{
		fov_x = x;
		fov_y = CalcFOV(x, width, height);
	}
	
	int			x, y;
	int			width, height;
	
	float			fov_x, fov_y;
	
	vec3_c			view_origin;
	vec3_c			view_angles;
	
	float			time;		// time is uesed to auto animate
	
	int			rdflags;	// RDF_UNDERWATER, etc

	byte*			areabits;	// if not NULL, only areas with set bits will be drawn
};



#define	REF_API_VERSION		21

#ifdef __cplusplus
extern "C" {
#endif

//
// these are the functions exported by the refresh module
//
typedef struct
{
	// if api_version is different, the dll cannot be used
	int		api_version;

	// called when the library is loaded
	bool		(*Init)(void *hinstance, void *wndproc);

	// called before the library is unloaded
	void		(*Shutdown)();

	
	void		(*R_BeginRegistration)(const std::string &map);
	int		(*R_RegisterModel)(const std::string &name);		//model
	int		(*R_RegisterAnimation)(const std::string &name);	//model animation
	int		(*R_RegisterSkin)(const std::string &name);		//model skin definition
	int		(*R_RegisterShader)(const std::string &name);		//3d shader
	int		(*R_RegisterPic)(const std::string &name);		//2d shader
	int		(*R_RegisterParticle)(const std::string &name);		//2d shader in 3d space facing the camera
	int		(*R_RegisterLight)(const std::string &name);		//light attuation shader
	void		(*R_EndRegistration)();

	void		(*R_BeginFrame)();
	void		(*R_RenderFrame)(const r_refdef_t &fd);
	void		(*R_EndFrame)();
	
	void		(*R_DrawPic)(int x, int y, int w, int h, const vec4_c &color, int shader);
	void		(*R_DrawStretchPic)(int x, int y, int w, int h, float s1, float t1, float s2, float t2, const vec4_c &color, int shader);
	void		(*R_DrawFill)(int x, int y, int w, int h, const vec4_c &color);
	
	void 		(*R_SetSky)(const std::string &name);
	
	void		(*R_ClearScene)();
	
	void		(*R_AddEntity)(int entity_num, const r_entity_t &shared);
	void		(*R_UpdateEntity)(int entity_num, const r_entity_t &shared, bool update);
	void		(*R_RemoveEntity)(int entity_num);
	
	void		(*R_AddLight)(int entity_num, const r_entity_t &shared, r_light_type_t type);
	void		(*R_UpdateLight)(int entity_num, const r_entity_t &shared, r_light_type_t type);
	void		(*R_RemoveLight)(int entity_num);
	
	void		(*R_AddParticle)(const r_particle_t &part);
	void		(*R_AddPoly)(const r_poly_t &poly);
	
	bool		(*R_SetupTag)(r_tag_t &tag, const r_entity_t &ent, const std::string &name);
	bool		(*R_SetupAnimation)(int model, int anim);
	
	void		(*AppActivate)(bool activate);

} ref_export_t;

//
// these are the functions imported by the refresh module
//
typedef struct
{

	void 		(*Com_Printf)(const char *fmt, ...);
	void 		(*Com_DPrintf)(const char *fmt, ...);
	void 		(*Com_Error)(err_type_e type, const char *fmt, ...);
	
	void 		(*Cbuf_ExecuteText)(exec_type_e, const std::string &text);
	
	void		(*Cmd_AddCommand)(const std::string &name, void(*cmd)());
	void		(*Cmd_RemoveCommand)(const std::string &name);
	int		(*Cmd_Argc)();
	const char*	(*Cmd_Argv)(int i);
	const char*	(*Cmd_Args)();
	
	int		(*VFS_FLoad)(const std::string &name, void **buf);
	void		(*VFS_FSave)(const std::string &path, void *buffer, int len);
	void		(*VFS_FFree)(void *buf);

	std::vector<std::string>	(*VFS_ListFiles)(const std::string &dir, const std::string &extension);
	
	cvar_t*		(*Cvar_Get)(const std::string &name, const std::string &value, uint_t flags);
	cvar_t*		(*Cvar_Set)(const std::string &name, const std::string &value);
	void		(*Cvar_SetValue)(const std::string &name, float value);
	
	roq_info_t*	(*Roq_Open)(const std::string &name);
	void		(*Roq_Close)(roq_info_t *info);
	int		(*Roq_ReadVideo)(roq_info_t *info);
	void		(*Roq_ResetStream)(roq_info_t *info);
	
	bool		(*VID_GetModeInfo)(int *width, int *height, int mode);
	void		(*VID_NewWindow)(int width, int height);
	
	int		(*Sys_Milliseconds)();
	int		(*Sys_Microseconds)();
} ref_import_t;


// this is the only function actually exported at the linker level
typedef	ref_export_t	(*GetRefAPI_t) (ref_import_t);


#ifdef __cplusplus
}
#endif

#endif // REF_H
