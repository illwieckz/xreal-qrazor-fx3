/// ============================================================================
/*
Copyright (C) 1997-2001 Id Software, Inc.
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
#ifndef CG_LOCAL_H
#define CG_LOCAL_H

/// includes ===================================================================
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
// system -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
// shared -------------------------------------------------------------------
#include "../x_shared.h"
#include "../x_bitmessage.h"

// xreal --------------------------------------------------------------------
#include "cg_public.h"




#define MAX_SUSTAINS		32


#define	PARTICLE_GRAVITY	40
#define INSTANT_PARTICLE	-10000.0

enum cg_particle_type_e
{
	PART_BUBBLE	= 1,
	PART_SMOKE,
	PART_SPLASH,
	PART_SPLASH1,
	PART_SPARK,
	PART_SPARK1,
	PART_SPARK2,
	PART_RAIN,
	PART_DUST,
	PART_STEAM,
	PART_BLOOD,
	PART_BLOOD1,
	PART_BLOOD2,
	PART_BLOOD3,
	PART_BLOOD4,
	PART_BLASTER,
	PART_RAIL,
	PART_LBLAST,
	PART_ROCKET,
	PART_SNOW1,
	PART_SNOW2,
	PART_SNOW3,
	PART_SNOW4,
	PART_SNOW5,
	PART_SNOW6,
	PART_SNOW7,
	PART_SNOW8,
	PART_SNOW9,
	PART_SNOW10,
	PART_SNOW11,
	PART_SNOW12,
	PART_HYPER,
	PART_EXPLODE,
	PART_RESPAWN,
	PART_EXPSMOKE,
	
	PART_BOSSTELE	= 1000,

	PART_DOT	= 9999
};


#define  BLOOD_WEIGHT   .4F
#define  SPLASH_WEIGHT  .4F
#define  SPARK_WEIGHT   .3F
#define  RAIN_WEIGHT    .7F
#define  SNOW_WEIGHT    .3F
#define  DUST_WEIGHT    .1F
#define  ROCKET_WEIGHT  .1F

#define  SPLASH_VELX_BASE     -20
#define  SPLASH_VELY_BASE     -20
#define  SPLASH_VELZ_BASE      80
#define  SPLASH_ACCELX_BASE    80
#define  SPLASH_ACCELY_BASE    80
#define  SPLASH_ACCELZ_BASE    100






struct anim_buffer_t
{
	void	clear()
	{
		newanim		= 0;
		serverframe	= 0;
		updated		= 0;
	}

	int		newanim;
	int		serverframe;
	int		updated;
};

struct anim_info_t
{
	void	clear()
	{
		animations.clear();
			
		frame		= 0;
		frame_old	= 0;
		frame_current	= 0;
			
		backlerp	= 0;
			
		frame_delay	= 0;
		
		buffer.clear();
	}

	// animation sequences
	std::vector<r_animation_t>	animations;
		
	// animation
	int		frame;
	int		frame_old;
	
	int		frame_current;
	
	// timing
	float		backlerp;
	
	//
	int		frame_delay;
	
	// buffer
	anim_buffer_t	buffer;
	
	
};

struct client_info_t
{
	void	clear()
	{
		name		= "noname";
		cinfo		= "";
		
		model		= -1;
		skin		= -1;
		
		anim.clear();
	}

	std::string	name;
	std::string	cinfo;
		
	int		model;
	int		skin;
	
	anim_info_t	anim;
};


struct cg_entity_t
{
	inline cg_entity_t()
	{
	}

	entity_state_t	current;
	entity_state_t	prev;			// will always be valid, but might just be a copy of current

	int		serverframe;		// if not current, this ent isn't in the frame
	int		serverframe_old;
};


/*
struct cg_light_t
{
	light_type_t	type;
	
	int		shader;
	int		key;				// so entities can reuse same entry
	float		die;				// stop lighting after this time
	float		decay;				// drop this each second
	float		minlight;			// don't add when contributing less

	vec3_c		origin;
	vec4_c		color;
	vec3_c		radius;
	
	vec3_c		dir;
	float		cone_inner;
	float		cone_outer;
};
*/

struct cg_sustain_t
{
	int		id;
	int		type;
	int		endtime;
	int		nextthink;
	int		thinkinterval;
	vec3_t		org;
	vec3_t		dir;
	int		color;
	int		count;
	int		magnitude;
	void		(*think)(cg_sustain_t *self);
};


struct cg_particle_t
{
	cg_particle_t*		next;
	
	cg_particle_type_e	type;
	
	int			time;		// in milliseconds

	vec3_c			vel;
	vec3_c			accel;
	vec3_c			colorvel;
	vec_t			alphavel;
	
	float			weight;
	
	r_particle_t		shared;
};


struct cg_media_t
{
	int		shader_crosshair;
};


struct cg_state_t
{
	frame_t		frame;
	frame_t		frame_old;
	bool		frame_running;
	float		frame_lerp;		// between frame_old and frame

	// the cl_parse_entities must be large enough to hold UPDATE_BACKUP frames of
	// entities, so that when a delta compressed message arives from the server
	// it can be un-deltad from the original 
	std::vector<cg_entity_t>	entities;
	std::vector<entity_state_t>	entities_parse;
	int				entities_first;		// index (not anded off) into cl_parse_entities[]
	
	// player prediction
	vec3_c		predicted_origins[CMD_BACKUP];	// for debug comparing against server

	float		predicted_step;				// for stair up smoothing
	unsigned	predicted_step_time;

	vec3_c		predicted_origin;	// generated by CL_PredictMovement
	vec3_c		predicted_angles;
	vec3_c		prediction_error;
	
	r_refdef_t	refdef;

	vec3_c		v_velocity;
	vec3_c		v_forward, v_right, v_up;	// set when refdef.angles is set
	vec4_c		v_blend;
	
	
	cg_media_t	media;
	
	
	client_info_t	clientinfo[MAX_CLIENTS];
	client_info_t	baseclientinfo;
	
	//vweaponinfo_t	vweapon;
	
	//
	// transient data from server
	//
	char		layout[1024];		// general 2D overlay
	int		inventory[MAX_ITEMS];
	
	//
	// locally derived information from server state
	//
	int		model_draw[MAX_MODELS];
	cmodel_c*	model_clip[MAX_MODELS];

	int		shader_precache[MAX_SHADERS];
	int		animation_precache[MAX_ANIMATIONS];
	int		sound_precache[MAX_SOUNDS];
	int		light_precache[MAX_LIGHTS];
};



struct cg_static_t
{
	//TODO
};


extern d_world_c*		cg_ode_world;
extern d_space_c*		cg_ode_space;
extern d_joint_group_c*		cg_ode_contact_group;

//
// cvars
//
extern cvar_t	*cg_gun;
extern cvar_t	*cg_footsteps;
extern cvar_t	*cg_crosshair;
extern cvar_t	*cg_crosshair_size;
extern cvar_t	*cg_stats;
extern cvar_t	*cg_vwep;
extern cvar_t	*cg_noskins;
extern cvar_t	*cg_showclamp;
extern cvar_t	*cg_shownet;
extern cvar_t	*cg_predict;
extern cvar_t	*cg_showmiss;
extern cvar_t	*cg_viewsize;
extern cvar_t	*cg_centertime;
extern cvar_t	*cg_showturtle;
extern cvar_t	*cg_showfps;
extern cvar_t	*cg_showlayout;
extern cvar_t	*cg_printspeed;
extern cvar_t	*cg_paused;
extern cvar_t	*cg_gravity;




//extern cg_import_t	cgi;
extern cg_export_t	cg_globals;

extern vrect_t		scr_vrect;		// position of render window

extern cg_state_t	cg;
extern cg_static_t	cgs;




//
// cg_cin.cxx
//
void	CG_PlayCinematic(char *name);
bool	CG_DrawCinematic();
void	CG_RunCinematic();
void	CG_StopCinematic();
void	CG_FinishCinematic();


//
// cg_light.cxx
//
void	CG_AddLightEntity(const cg_entity_t *cent);
void	CG_UpdateLightEntity(const cg_entity_t *cent);
void	CG_RemoveLightEntity(const cg_entity_t *cent);


//
// cg_entity.cxx -- delta compressed entity management
//
void	CG_BeginFrame(const frame_t &frame);
void	CG_AddEntity(int newnum, const entity_state_t *state);
void	CG_UpdateEntity(int newnum, const entity_state_t *state, bool changed);
void	CG_RemoveEntity(int oldnum, const entity_state_t *state);
void	CG_EndFrame(int entities_num);

void	CG_GetEntitySoundOrigin(int ent, vec3_c &org);

void	CG_UpdateOrigin(const cg_entity_t *cent, r_entity_t &rent, bool &update);
void	CG_UpdateFrame(const cg_entity_t *cent, r_entity_t &rent, bool &update);
void	CG_UpdateRotation(const cg_entity_t *cent, r_entity_t &rent, bool &update);
void	CG_UpdateModel(const cg_entity_t *cent, r_entity_t &rent, bool &update);
void	CG_UpdateShader(const cg_entity_t *cent, r_entity_t &rent, bool &update);
void	CG_UpdateLightShader(const cg_entity_t *cent, r_entity_t &rent, bool &update);
void	CG_UpdateShaderParms(const cg_entity_t *cent, r_entity_t &rent, bool &update);
void	CG_UpdateRenderFXFlags(const cg_entity_t *cent, r_entity_t &rent, bool &update);

void	CG_AddGenericEntity(const cg_entity_t *cent);
void	CG_UpdateGenericEntity(const cg_entity_t *cent);
void	CG_RemoveGenericEntity(const cg_entity_t *cent);

void	CG_UpdateEntities();


//
// cg_event.cxx
//
void	CG_EntityEvent(const cg_entity_t *cent);
void	CG_CheckEntityEvents();


//
//
// cg_muzzleflash.cxx
//
void 	CG_ParseMuzzleFlash(bitmessage_c &msg);



//
// cg_inv.cxx
//
void	CG_ParseInventory(bitmessage_c &msg);
void	CG_KeyInventory(int key);
void	CG_DrawInventory();


//
// cg_main.cxx
//
void	CG_ParserLayout();

void	CG_ClearState();
void	CG_RegisterSounds();


//
// cg_particle.cxx
//
void	CG_ClearParticles();
void	CG_ExplosionParticles (vec3_t org);
void	CG_DiminishingTrail (vec3_t start, vec3_t end, cg_entity_t *old, int flags);
void	CG_RocketTrail (vec3_t start, vec3_t end, cg_entity_t *old);
void 	CG_RailTrail (vec3_t start, vec3_t end);
void	CG_BubbleTrail (vec3_t start, vec3_t end);
void	CG_BfgParticles (r_entity_t *ent);
void	CG_BFGExplosionParticles (vec3_t org);

void	CG_AddParticles();

void	CG_ParticleSpray(cg_particle_type_e type, const vec3_c &org, vec3_c &dir, const vec4_c &color, int count);
void	CG_ParticleTrail(cg_particle_type_e type, const vec3_c &start, const vec3_c &end, const vec4_c &color, float interval);


//
// g_physics.cxx
//
void	CG_InitDynamics();
void	CG_ShutdownDynamics();


//
// cg_player.cxx
//
void	CG_UpdateAnimationBuffer();

void	CG_AddPlayerEntities(r_entity_t &ent, cg_entity_t *cent, int effects, int renderfx);

void	CG_LoadClientinfo(client_info_t *ci, const std::string &s);
void	CG_ParseClientinfo(int player);


//
// cg_pred.cxx
//
/*
#define IGNORE_NOTHING		-1
#define IGNORE_WORLD		0
#define IGNORE_PLAYER		cgi.cl->playernum+1
*/

void 	CG_PredictMovement();
void 	CG_CheckPredictionError();


//
// cg_screen.cxx
//
void 	CG_AddNetgraph();

void	CG_InitScreen();

void	CG_SizeUp();
void	CG_SizeDown();
void	CG_CenterPrint(const char *str);

void 	CG_CalcVrect();
void 	CG_TileClear();
void 	CG_DrawFPS();
void	CG_DrawCrosshair();

void	CG_RegisterPics();

void	CG_DrawChar(int x, int y, int num, const vec4_c &color, int flags);
void	CG_DrawString(int x, int y, const vec4_c &color, int flags, const char *s);



//
// cg_tent.c
//
void 	CG_RegisterTEntSounds();
void 	CG_RegisterTEntModels();
void 	CG_ClearTEnts();

void 	CG_ParseTEnt(bitmessage_c &msg);
void 	CG_RegisterTEntPics();
void 	CG_SmokeAndFlash(vec3_t origin);


//
// cg_view.cxx
//
void	CG_InitView();
void	CG_RenderView();

void 	CG_PrepRefresh();


//
// cg_weapon.cxx
//
void	CG_InitWeapon();
void	CG_AddViewWeapon();


//
// cg_syscalls.cxx
//
// common printing
void 		trap_Com_Printf(const char *fmt, ...);
void 		trap_Com_DPrintf(const char *fmt, ...);

void 		trap_Com_Error(err_type_e type, const char *fmt, ...);

void		trap_Cmd_AddCommand(const std::string &name, void(*cmd)());
void		trap_Cmd_RemoveCommand(const std::string &name);
int		trap_Cmd_Argc();
const char*	trap_Cmd_Argv(int i);

cvar_t*		trap_Cvar_Get(const std::string &name, const std::string &value, uint_t flags);
cvar_t*		trap_Cvar_Set(const std::string &name, const std::string &value);
void		trap_Cvar_SetValue(const std::string &name, float value);
float		trap_Cvar_VariableValue(const std::string &name);
int		trap_Cvar_VariableInteger(const std::string &name);

void		trap_R_BeginRegistration(const std::string &map);
int		trap_R_RegisterModel(const std::string &name);
int		trap_R_RegisterAnimation(const std::string &name);
int		trap_R_RegisterSkin(const std::string &name);
int		trap_R_RegisterShader(const std::string &name);
int		trap_R_RegisterPic(const std::string &name);
int		trap_R_RegisterParticle(const std::string &name);
int		trap_R_RegisterLight(const std::string &name);
void		trap_R_EndRegistration();

void		trap_R_BeginFrame();
void		trap_R_RenderFrame(const r_refdef_t &fd);
void		trap_R_EndFrame();
	
void		trap_R_DrawPic(int x, int y, int w, int h, const vec4_c &color, int shader);
void		trap_R_DrawStretchPic(int x, int y, int w, int h, float s1, float t1, float s2, float t2, const vec4_c &color, int shader);
void		trap_R_DrawFill(int x, int y, int w, int h, const vec4_c &color);
	
void		trap_R_ClearScene();
	
void		trap_R_AddEntity(int index, const r_entity_t &shared);
void		trap_R_UpdateEntity(int index, const r_entity_t &shared);
void		trap_R_RemoveEntity(int index);
	
void		trap_R_AddLight(int index, const r_entity_t &shared, r_light_type_t type);
void		trap_R_UpdateLight(int index, const r_entity_t &shared, r_light_type_t type);
void		trap_R_RemoveLight(int index);
	
void		trap_R_AddParticle(const r_particle_t &part);
void		trap_R_AddPoly(const r_poly_t &poly);
void		trap_R_AddContact(const r_contact_t &contact);

bool		trap_R_SetupTag(r_tag_t &tag, const r_entity_t &ent, const std::string &name);
bool		trap_R_SetupAnimation(int model, int anim);	

	// virtual fileystem access
int		trap_VFS_FOpenRead(const std::string &filename, VFILE **stream);
int		trap_VFS_FOpenWrite(const std::string &filename, VFILE **stream);
void		trap_VFS_FClose(VFILE **stream);
	
int		trap_VFS_FLoad(const std::string &name, void **buf);
void		trap_VFS_FSave(const std::string &path, void *buffer, int len);
void		trap_VFS_FFree(void *buf);

int		trap_VFS_FRead(void *buffer, int len, VFILE *stream);
int		trap_VFS_FWrite(const void *buffer, int len, VFILE *stream);
	
		
char*		trap_Key_KeynumToString(int keynum);
char*		trap_Key_GetBinding(int keynum);
//void 		(*Key_SetBinding)(int keynum, char *binding);
//void 		(*Key_ClearStates)(void);
//keydest_t	(*Key_GetKeyDest)(void);
//void		(*Key_SetKeyDest)( keydest_t key_dest );
	
void		trap_Con_ClearNotify();
	
void		trap_S_Init();
void		trap_S_Shutdown();
	
void		trap_S_StartSound(const vec3_c &origin, int ent_num, int ent_channel, int sound);

void		trap_S_StartLoopSound(const vec3_c &origin, const vec3_c &velocity, int entity_num, int entity_channel, int sound);
void		trap_S_UpdateLoopSound(const vec3_c &origin, const vec3_c &velocity, int entity_num, int entity_channel, int sound);
void		trap_S_StopLoopSound(int ent_num);

void		trap_S_StopAllSounds();
void		trap_S_Update(const vec3_c &origin, const vec3_c &velocity, const vec3_c &v_forward, const vec3_c &v_right, const vec3_c &v_up);
	
void		trap_S_BeginRegistration();
int		trap_S_RegisterSound(const std::string &name);
void		trap_S_EndRegistration();
	
d_bsp_c*	trap_CM_BeginRegistration(const std::string &name, bool clientload, unsigned *checksum, dSpaceID space);
cmodel_c*	trap_CM_RegisterModel(const std::string &name);
void		trap_CM_EndRegistration();
	
int		trap_CM_PointContents(const vec3_c &p, int headnode);
int		trap_CM_TransformedPointContents(const vec3_c &p, int headnode, const vec3_c &origin, const quaternion_c &quat);
	
int		trap_CL_GetTime();
void		trap_CL_SetTime(int time);
const char*	trap_CL_GetConfigString(int index);
void		trap_CL_GetUserCommand(int frame, usercmd_t &cmd);
void		trap_CL_GetCurrentUserCommand(usercmd_t &cmd);
bool		trap_CL_GetRefreshPrepped();
void		trap_CL_SetRefreshPrepped(bool val);
bool		trap_CL_GetForceRefdef();
void		trap_CL_SetForceRefdef(bool val);
int		trap_CL_GetPlayerNum();
	
connection_state_t	trap_CLS_GetConnectionState();
float		trap_CLS_GetRealTime();
float		trap_CLS_GetFrameTime();
int		trap_CLS_GetFrameCount();
void		trap_CLS_GetCurrentNetState(int &incoming_acknowledged, int &outgoing_sequence);

uint_t		trap_VID_GetWidth();
uint_t		trap_VID_GetHeight();

int		trap_Sys_Milliseconds();


#endif
