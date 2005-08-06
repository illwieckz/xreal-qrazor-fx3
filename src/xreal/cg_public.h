/// ============================================================================
/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2003 Robert Beckebans <trebor_7@users.sourceforge.net>
Copyright (C) 2003, 2004  contributors of the XreaL project
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
#ifndef CG_PUBLIC_H
#define CG_PUBLIC_H

/// includes ===================================================================
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
// system -------------------------------------------------------------------
// shared -------------------------------------------------------------------
#include "../x_shared.h"

#if defined(ODE)
#include "../x_ode.h"
#endif

#include "../x_protocol.h"

// qrazor-fx ----------------------------------------------------------------
#include "../r_public.h"
#include "../files.h"

// xreal --------------------------------------------------------------------





#define	CMD_BACKUP			64	// allow a lot of command backups for very fast systems
#define	CMD_MASK			(CMD_BACKUP-1)

struct vrect_t
{
	int				x, y, width, height;
};

struct frame_t
{
	void	clear()
	{
		valid			= false;
		
		serverframe		= 0;
		servertime		= 0;
		deltaframe		= 0;
		
		areabits.clear();
		
		playerstate.clear();
		
		entities_first		= 0;
		entities_num		= 0;
	}

	bool			valid;				// cleared if delta parsing was invalid
	
	int			serverframe;
	int			servertime;			// server time the message is valid for (in msec)
	int			deltaframe;
	
	boost::dynamic_bitset<byte>	areabits;		// portalarea visibility bits
	player_state_t			playerstate;
	int				entities_first;		// non-masked index into cl_parse_entities array
	int				entities_num;
};


enum connection_state_t
{
	CA_UNINITIALIZED,
	CA_DISCONNECTED, 	// not talking to a server
	CA_CONNECTING,		// sending request packets to the server
	CA_CONNECTED,		// netchan_t established, waiting for svc_serverdata
	CA_ACTIVE		// game views should be displayed
};



#define CG_API_VERSION		10


#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	void 		(*Com_Printf)(const char *fmt, ...);
	void 		(*Com_DPrintf)(const char *fmt, ...);
	void 		(*Com_Error)(err_type_e type, const char *fmt, ...);
		
	//void 		(*Cbuf_AddText)(const std::string &text);
	//void 		(*Cbuf_InsertText)(const std::string &text);
	//void 		(*Cbuf_ExecuteText)(exec_type_t, const std::string &text);
	//void 		(*Cbuf_Execute)();
	
	void		(*Cmd_AddCommand)(const std::string &name, void(*cmd)());
	void		(*Cmd_RemoveCommand)(const std::string &name);
	int		(*Cmd_Argc)();
	const char*	(*Cmd_Argv)(int i);	

	cvar_t*		(*Cvar_Get)(const std::string &name, const std::string &value, uint_t flags);
	cvar_t*		(*Cvar_Set)(const std::string &name, const std::string &value);
	//cvar_t*	(*Cvar_ForceSet)(const std::string &name, const std::string &value);
	void		(*Cvar_SetValue)(const std::string &name, float value);
	//void		(*Cvar_SetModified)(const std::string &name);
	float		(*Cvar_VariableValue)(const std::string &name);
	int		(*Cvar_VariableInteger)(const std::string &name);
	//const char*	(*Cvar_VariableString)(const std::string &name);
	//float		(*Cvar_ClampVariable)(const std::string &name, float min, float max);
	
	void		(*R_BeginRegistration)(const std::string &map);
	int		(*R_RegisterModel)(const std::string &name);
	int		(*R_RegisterAnimation)(const std::string &name);
	int		(*R_RegisterSkin)(const std::string &name);
	int		(*R_RegisterShader)(const std::string &name);
	int		(*R_RegisterPic)(const std::string &name);
	int		(*R_RegisterParticle)(const std::string &name);
	int		(*R_RegisterLight)(const std::string &name);
	void		(*R_EndRegistration)();

	void		(*R_BeginFrame)();
	void		(*R_RenderFrame)(const r_refdef_t &fd);
	void		(*R_EndFrame)();
	
	void		(*R_DrawPic)(int x, int y, int w, int h, const vec4_c &color, int shader);
	void		(*R_DrawStretchPic)(int x, int y, int w, int h, float s1, float t1, float s2, float t2, const vec4_c &color, int shader);
	void		(*R_DrawFill)(int x, int y, int w, int h, const vec4_c &color);
	
	void		(*R_ClearScene)();
	
	void		(*R_AddEntity)(int index, const r_entity_t &shared);
	void		(*R_UpdateEntity)(int index, const r_entity_t &shared);
	void		(*R_RemoveEntity)(int index);
	
	void		(*R_AddLight)(int index, const r_entity_t &shared, r_light_type_t type);
	void		(*R_UpdateLight)(int index, const r_entity_t &shared, r_light_type_t type);
	void		(*R_RemoveLight)(int index);
	
	void		(*R_AddParticle)(const r_particle_t &part);
	void		(*R_AddPoly)(const r_poly_t &poly);
	void		(*R_AddContact)(const r_contact_t &contact);

	bool		(*R_SetupTag)(r_tag_t &tag, const r_entity_t &ent, const std::string &name);
	bool		(*R_SetupAnimation)(int model, int anim);	

	// virtual fileystem access
	int		(*VFS_FOpenRead)(const std::string &filename, VFILE **stream);
	int		(*VFS_FOpenWrite)(const std::string &filename, VFILE **stream);
        void		(*VFS_FClose)(VFILE **stream);
	
	int		(*VFS_FLoad)(const std::string &name, void **buf);
	void		(*VFS_FSave)(const std::string &path, void *buffer, int len);
	void		(*VFS_FFree)(void *buf);

	int		(*VFS_FRead)(void *buffer, int len, VFILE *stream);
	int		(*VFS_FWrite)(const void *buffer, int len, VFILE *stream);
	
		
	char*		(*Key_KeynumToString)(int keynum);
	char*		(*Key_GetBinding)(int keynum);
	//void 		(*Key_SetBinding)(int keynum, char *binding);
	//void 		(*Key_ClearStates)(void);
	//keydest_t	(*Key_GetKeyDest)(void);
	//void		(*Key_SetKeyDest)( keydest_t key_dest );
	
	void		(*Con_ClearNotify)();
	
	void		(*S_Init)();
	void		(*S_Shutdown)();
	
	void		(*S_StartSound)(const vec3_c &origin, int ent_num, int ent_channel, int sound);
	
	void		(*S_StartLoopSound)(const vec3_c &origin, const vec3_c &velocity, int ent_num, int ent_channel, int sound);
	void		(*S_UpdateLoopSound)(const vec3_c &origin, const vec3_c &velocity, int ent_num, int ent_channel, int sound);
	void		(*S_StopLoopSound)(int ent_num);
	
	void		(*S_StopAllSounds)();
	void		(*S_Update)(const vec3_c &origin, const vec3_c &velocity, const vec3_c &v_forward, const vec3_c &v_right, const vec3_c &v_up);
	
	void		(*S_BeginRegistration)();
	int		(*S_RegisterSound)(const std::string &name);
	void		(*S_EndRegistration)();
	
	cmodel_c*	(*CM_BeginRegistration)(const std::string &name, bool clientload, unsigned *checksum);
	cmodel_c*	(*CM_RegisterModel)(const std::string &name);
	void		(*CM_EndRegistration)();
	
	int		(*CL_GetTime)();
	void		(*CL_SetTime)(int time);
	const char*	(*CL_GetConfigString)(int index);
	void		(*CL_GetUserCommand)(int frame, usercmd_t &cmd);
	void		(*CL_GetCurrentUserCommand)(usercmd_t &cmd);
	bool		(*CL_GetRefreshPrepped)();
	void		(*CL_SetRefreshPrepped)(bool val);
	bool		(*CL_GetForceRefdef)();
	void		(*CL_SetForceRefdef)(bool val);
	int		(*CL_GetPlayerNum)();
	
	connection_state_t	(*CLS_GetConnectionState)();
	float		(*CLS_GetRealTime)();
	float		(*CLS_GetFrameTime)();
	int		(*CLS_GetFrameCount)();
	void		(*CLS_GetCurrentNetState)(int &incoming_acknowledged, int &outgoing_sequence);
	
	uint_t		(*VID_GetWidth)();
	uint_t		(*VID_GetHeight)();
	
	int		(*Sys_Milliseconds)();
} cg_import_t;


typedef struct
{
	int		apiversion;
	
	void		(*Init)();
	void		(*Shutdown)();
	
	// delta entities
	void		(*CG_BeginFrame)(const frame_t &frame);
	void		(*CG_AddEntity)(int newnum, const entity_state_t *state);
	void		(*CG_UpdateEntity)(int newnum, const entity_state_t *state, bool changed);
	void		(*CG_RemoveEntity)(int oldnum, const entity_state_t *state);
	void		(*CG_EndFrame)(int entities_num);
	
	// main
	void		(*CG_RunFrame)();
	void		(*CG_UpdateConfig)(int index, const std::string &configstring);
	
	// misc
	void		(*CG_ParseLayout)(bitmessage_c &msg);
	void 		(*CG_ParseTEnt)(bitmessage_c &msg);
	void		(*CG_ParseMuzzleFlash)(bitmessage_c &msg);
	void		(*CG_PrepRefresh)();
	void 		(*CG_GetEntitySoundOrigin)(int ent, vec3_c &org);
	
	void		(*CG_ClearState)();
	
	// inventory
	void		(*CG_ParseInventory)(bitmessage_c &msg);
	
	// helper routines
	void		(*CG_DrawChar)(int x, int y, int num, const vec4_c &color, int flags);
	void		(*CG_DrawString)(int x, int y, const vec4_c &color, int flags, const char *s);
	
	// screen
	void		(*CG_CenterPrint)(const char *str);
	
	// client information
	void		(*CG_ParseClientinfo)(int player);	
	
} cg_export_t;



cg_export_t*	GetCGameAPI(cg_import_t *import);



#ifdef __cplusplus
}
#endif


#endif	// G_PUBLIC_H
