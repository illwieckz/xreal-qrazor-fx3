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
#ifndef SERVER_H
#define SERVER_H

/// includes ===================================================================
// system -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
#include "common.h"
#include "sys.h"

#include "net_chan.h"
#include "files.h"
#include "sv_client.h"

// xreal --------------------------------------------------------------------
#include "xreal/g_public.h"

// shared -------------------------------------------------------------------
#include "x_bitmessage.h"
#include "x_protocol.h"


#define	MAX_MASTERS	8				// max recipients for heartbeat packets

enum server_state_t
{
	SS_DEAD,			// no map loaded
	SS_LOADING,			// spawning level edicts
	SS_GAME
};
// some qc commands are only valid before the server has finished
// initializing (precache commands, static sounds / objects, etc)

struct server_t
{
	void	clear()
	{
		state		= SS_DEAD;
		
		attractloop	= false;
		loadgame	= false;
		
		time		= 0;
		framenum	= 0;
		
		name		= "";
		
		memset(configstrings, 0, sizeof(configstrings));
		baselines	= std::vector<entity_state_t>(MAX_ENTITIES);
		
		multicast	= bitmessage_c(MAX_PACKETLEN*8);
	}

	server_state_t			state;			// precache commands are only valid during load

	bool				attractloop;		// running cinematics and demos for the local system only
	bool				loadgame;		// client begins should reuse existing entity

	int				time;			// always sv.framenum * 100 msec
	int				framenum;

	std::string			name;			// map name, or cinematic name
	
	char				configstrings[MAX_CONFIGSTRINGS][MAX_QPATH];
	std::vector<entity_state_t>	baselines;

	// the multicast buffer is used to send a message to a set of clients
	// it is only used to marshall data until SV_Multicast is called
	bitmessage_c			multicast;
};


// a client can leave the server in one of four ways:
// dropping properly by quiting or disconnecting
// timing out if no valid messages are received for timeout.value seconds
// getting kicked off by the server operator
// a program error, like an overflowed reliable buffer


// MAX_CHALLENGES is made large to prevent a denial
// of service attack that could cycle all of them
// out before legitimate users connected
#define	MAX_CHALLENGES	1024

struct challenge_t
{
	netadr_t		adr;
	int			challenge;
	int			time;
};


struct server_static_t
{
	void	clear()
	{
		initialized		= false;
		realtime		= 0;

		memset(mapcmd, 0, sizeof(mapcmd));
		
		spawncount		= 0;
		
		clients.clear();
		num_client_entities	= 0;
		next_client_entities	= 0;
		client_entities		= NULL;
		
		last_heartbeat		= 0;
		
		memset(challenges, 0, sizeof(challenges));
	}

	bool			initialized;			// sv_init has completed
	int			realtime;			// always increasing, no clamping, etc

	char			mapcmd[MAX_TOKEN_CHARS];	// ie: *intro.cin+base 

	int			spawncount;			// incremented each server start
								// used to check late spawns

	std::vector<sv_client_c*>	clients;			// [maxclients->value];
	int				num_client_entities;		// maxclients->value*UPDATE_BACKUP*MAX_PACKET_ENTITIES
	int				next_client_entities;		// next client_entity to use
	entity_state_t*			client_entities;		// [num_client_entities]

	int			last_heartbeat;

	challenge_t		challenges[MAX_CHALLENGES];	// to prevent invalid IPs from connecting
};


extern netadr_t		master_adr[MAX_MASTERS];	// address of the master server

extern server_static_t	svs;				// persistant server info
extern server_t		sv;					// local server

extern cvar_t*		sv_shownet;
extern cvar_t*		sv_paused;
extern cvar_t*		maxclients;
extern cvar_t*		sv_noreload;			// don't reload level state when reentering
extern cvar_t*		sv_airaccelerate;		// don't reload level state when reentering

extern cvar_t*		sv_allow_download;
extern cvar_t*		sv_allow_download_players;
extern cvar_t*		sv_allow_download_models;
extern cvar_t*		sv_allow_download_sounds;
extern cvar_t*		sv_allow_download_maps;							
																																// development tool
extern cvar_t*		sv_enforcetime;




//
// sv_main.c
//
int 	SV_ModelIndex(const std::string &name);
int 	SV_ShaderIndex(const std::string &name);
int 	SV_AnimationIndex(const std::string &name);
int 	SV_SoundIndex(const std::string &name);
int 	SV_LightIndex(const std::string &name);

void 	SV_WriteClientdataToMessage(sv_client_c *client, message_c *msg);

void	SV_InitOperatorCommands();

void 	SV_SendServerinfo(sv_client_c *client);
void 	SV_UserinfoChanged(sv_client_c *cl);


void 	Master_Heartbeat();
void	Master_Shutdown();


//
// sv_map.cxx 
//
void		SV_LoadMap(const std::string &name);
const char*	SV_EntityString();


//
// sv_init.c
//
void 	SV_InitGame();
void 	SV_Map(bool attractloop, const char *levelstring, bool loadgame);


//
// sv_send.c
//
#define	SV_OUTPUTBUF_LENGTH	(MAX_PACKETLEN - 16)

extern	char	sv_outputbuf[SV_OUTPUTBUF_LENGTH];

void 	SV_FlushRedirect(char *outputbuf, const netadr_t &adr);

void 	SV_SendClientMessages();

void 	SV_Multicast(const vec3_c &origin, multicast_type_e to);
void 	SV_StartSound(vec3_t origin, sv_entity_c *entity, int channel, int soundindex, float volume, float attenuation, float timeofs);
void 	SV_BroadcastPrintf(g_print_level_e level, const char *fmt, ...);
void 	SV_BroadcastCommand(const char *fmt, ...);


//
// sv_ccmds.c
//
void 	SV_ReadLevelFile();

//
// sv_ents.c
//
sv_entity_c*	SV_GetEntityByNum(int num);
int		SV_GetNumForEntity(sv_entity_c *ent);


void 	SV_Error(char *error, ...);

//
// sv_game.c
//
extern	game_export_t	*ge;

void 	SV_InitGameProgs();
void 	SV_ShutdownGameProgs();


#endif // SERVER_H
