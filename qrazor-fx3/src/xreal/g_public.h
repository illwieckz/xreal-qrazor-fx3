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
#ifndef G_PUBLIC_H
#define G_PUBLIC_H

/// includes ===================================================================
// system -------------------------------------------------------------------
// xreal --------------------------------------------------------------------
// shared -------------------------------------------------------------------
#include "../x_shared.h"
#include "../x_ode.h"

// xg_public.h -- game dll information visible to server

#define	GAME_API_VERSION	9

// edict->svflags

enum
{
	SVF_NONE		= 0,
	SVF_NOCLIENT		= (1<<0),	// don't send entity to clients, even if it has effects
	SVF_CORPSE		= (1<<1)	// treat as CONTENTS_DEADMONSTER for collision
};

// gi.SV_BoxEntities() can return a list of either solid or trigger entities
// FIXME: eliminate AREA_ distinction?
enum area_type_e
{
	AREA_SOLID		= 1,
	AREA_TRIGGERS
};


struct entity_shared_t
{
	entity_shared_t()
	{
		inuse		= false;
		
		islinked	= false;
		linkcount	= 0;
		
		clusters	= std::vector<int>(16);
		headnode	= 0;
		area		= 0;
		area2		= 0;
		areaportal	= -1;
		
		svflags		= SVF_NONE;
		
		solid		= SOLID_NOT;
		clipmask	= 0;
		
		owner		= NULL;
		
		isclient	= false;
		isbot		= false;
		ps.clear();
		
		ping		= 9999;
	}

	// common for all server/game entities
	bool			inuse;
	
	bool			islinked;
	int			linkcount;
	
	std::vector<int>	clusters;
	int			headnode;			// unused if num_clusters != -1
	int			area;
	int			area2;
	int			areaportal;

	int			svflags;			// SVF_NOCLIENT, SVF_DEADMONSTER, SVF_MONSTER, etc
	cbbox_c			bbox_abs;			// world space axis aligned bounding box = entity origin + local bbox
	cbbox_c			bbox;				// object space bounding box
	vec3_c			size;				// object space bbox size
	solid_e			solid;
	int			clipmask;
	
	entity_c*		owner;
	
	// pawn specific
	bool			isclient;
	bool			isbot;
	player_state_t		ps;				// communicated by server to clients
	int			ping;
};


class sv_entity_c : public entity_c
{
public:	
	entity_shared_t		_r;	// only used by gamecode and server
};






#ifdef __cplusplus
extern "C" {
#endif

//
// functions provided by the main engine
//
typedef struct
{
	// common printing
	void 		(*Com_Printf)(const char *fmt, ...);
	void 		(*Com_DPrintf)(const char *fmt, ...);
	void 		(*Com_Error)(err_type_e type, const char *fmt, ...);
	
	// add commands to the server console as if they were typed in
	// for map changing, etc
	void 		(*Cbuf_AddText)(const std::string &text);
	
	// ClientCommand and ServerCommand parameter access
	void		(*Cmd_AddCommand)(const std::string &name, void(*cmd)());
	void		(*Cmd_RemoveCommand)(const std::string &name);
	int		(*Cmd_Argc)();
	const char*	(*Cmd_Argv)(int i);	
	const char*	(*Cmd_Args)();

	// console variable interaction
	cvar_t*		(*Cvar_Get)(const std::string &name, const std::string &value, uint_t flags);
	cvar_t*		(*Cvar_Set)(const std::string &name, const std::string &value);
	cvar_t*		(*Cvar_ForceSet)(const std::string &name, const std::string &value);
	void		(*Cvar_SetValue)(const std::string &name, float value);

	// special messages
	void		(*SV_BPrintf)(g_print_level_e level, const char *fmt, ...);
	void		(*SV_CPrintf)(sv_entity_c *ent, g_print_level_e level, const char *fmt, ...);
	void		(*SV_CenterPrintf)(sv_entity_c *ent, const char *fmt, ...);
	void		(*SV_StartSound)(vec3_t origin, sv_entity_c *ent, int channel, int soundinedex, float volume, float attenuation, float timeofs);

	// config strings hold all the index strings, the lightstyles,
	// and misc data like the sky definition and cdtrack.
	// All of the current configstrings are sent to clients when
	// they connect, and changes are sent to all connected clients.
	void		(*SV_SetConfigString)(int index, const std::string &val);
	
	// the *index functions create configstrings and some internal server state
	int		(*SV_ModelIndex)(const std::string &name);
	int		(*SV_ShaderIndex)(const std::string &name);
	int		(*SV_AnimationIndex)(const std::string &name);
	int		(*SV_SoundIndex)(const std::string &name);
	int		(*SV_LightIndex)(const std::string &name);

	// collision detection
	d_bsp_c*		(*CM_BeginRegistration)(const std::string &name, bool clientload, unsigned *checksum, dSpaceID space);
	cmodel_c*		(*CM_RegisterModel)(const std::string &name);
	cskel_animation_c*	(*CM_RegisterAnimation)(const std::string &name);
	cmodel_c*		(*CM_GetModelByNum)(int num);
	int			(*CM_LeafContents)(int leafnum);
	int			(*CM_LeafCluster)(int leafnum);
	int			(*CM_LeafArea)(int leafnum);
	int			(*CM_NumModels)();
	int			(*CM_HeadnodeForBox)(const cbbox_c& bbox);
	int			(*CM_PointContents)(const vec3_c &p, int headnode);
	int			(*CM_TransformedPointContents)(const vec3_c &p, int headnode, const vec3_c &origin, const quaternion_c &quat);
	trace_t			(*CM_BoxTrace)(const vec3_c &start, const vec3_c &end, const cbbox_c &bbox, int headnode, int brushmask);
	trace_t			(*CM_TransformedBoxTrace)(const vec3_c &start, const vec3_c &end,
						const cbbox_c &bbox,
						int headnode, int brushmask, 
						const vec3_c &origin, const quaternion_c &quat);
	int			(*CM_PointLeafnum)(const vec3_c &p);
	int			(*CM_PointAreanum)(const vec3_c &p);
	int			(*CM_BoxLeafnums)(const cbbox_c &bbox, std::deque<int> &list, int headnode);
	int			(*CM_GetClosestAreaPortal)(const vec3_c &p);
	bool			(*CM_GetAreaPortalState)(int portal);
	void			(*CM_SetAreaPortalState)(int portal, bool open);
	bool			(*CM_AreasConnected)(int area1, int area2);

	// network messaging
	void		(*SV_Multicast)(const vec3_c &origin, multicast_type_e to);
	void		(*SV_Unicast)(sv_entity_c *ent, bool reliable);
	
	void		(*SV_WriteBit)(bool bit);
	void		(*SV_WriteBits)(int bits, int bits_num);
	void		(*SV_WriteByte)(int c);
	void		(*SV_WriteShort)(int c);
	void		(*SV_WriteLong)(int c);
	void		(*SV_WriteFloat)(float f);
	void		(*SV_WriteString)(const char *s);
	void		(*SV_WritePosition)(const vec3_c &pos);	// some fractional bits
	void		(*SV_WriteDir)(const vec3_c &pos);	// single byte encoded, very coarse
	void		(*SV_WriteAngle)(float f);
	void		(*SV_WriteColor)(vec4_t color);

	// virtual fileystem access
	int		(*VFS_FOpenRead)(const std::string &filename, VFILE **stream);
	int		(*VFS_FOpenWrite)(const std::string &filename, VFILE **stream);
        void		(*VFS_FClose)(VFILE **stream);
	
	int		(*VFS_FLoad)(const std::string &name, void **buf);
	void		(*VFS_FSave)(const std::string &path, void *buffer, int len);
	void		(*VFS_FFree)(void *buf);

	int		(*VFS_FRead)(void *buffer, int len, VFILE *stream);
	int		(*VFS_FWrite)(const void *buffer, int len, VFILE *stream);
	
} game_import_t;

//
// functions exported by the game subsystem
//
typedef struct
{
	int		apiversion;

	// the init function will only be called when a game starts,
	// not each time a level is loaded.  Persistant data for clients
	// and the server can be allocated in init
	void		(*G_InitGame)();
	void		(*G_ShutdownGame)();

	// each new level entered will cause a call to SpawnEntities
	void		(*G_ClearWorld)(const std::string &map);
	void		(*G_SpawnEntities)(const std::string &mapname, char *entstring, const std::string &spawnpoint);

	// issue per client specific commands
	bool		(*G_ClientConnect)(sv_entity_c *ent, info_c &userinfo);
	void		(*G_ClientBegin)(sv_entity_c *ent);
	void		(*G_ClientUserinfoChanged)(sv_entity_c *ent, info_c &userinfo);
	void		(*G_ClientDisconnect)(sv_entity_c *ent);
	void		(*G_ClientCommand)(sv_entity_c *ent);
	void		(*G_ClientThink)(sv_entity_c *ent, const usercmd_t &cmd);

	void		(*G_RunFrame)();

	// ServerCommand will be called when an "sv <command>" command is issued on the
	// server console.
	// The game can issue gi.argc() / gi.argv() commands to get the rest
	// of the parameters
	void		(*G_ServerCommand)();


	// global variables shared between game and server

	// The edict array is allocated in the game dll so it
	// can vary in size from one game to another.
	std::vector<sv_entity_c*>*	entities;
	
} game_export_t;

game_export_t*	GetGameAPI (game_import_t *import);


#ifdef __cplusplus
}
#endif

#endif // G_PUBLIC_H


