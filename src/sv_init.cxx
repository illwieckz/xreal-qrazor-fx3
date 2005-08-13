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


/// includes ===================================================================
// system -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
#include "sv_local.h"

#include "cmd.h"
#include "cvar.h"
#include "cm.h"
#include "vfs.h"



server_static_t		svs;				// persistant server info
server_t		sv;				// local server


int 	SV_FindIndex(const std::string &name, int start, int max, bool create)
{
	int		i;
	
	if(name.empty())
	{
		Com_Error(ERR_DROP, "SV_FindIndex: empty name");
		return 0;
	}

	for(i=1; i<max && sv.configstrings[start+i][0]; i++)
		if(!strcmp(sv.configstrings[start+i], name.c_str()))
			return i;

	if(!create)
		return 0;

	if(i == max)
	{
		Com_Error(ERR_DROP, "*Index: overflow");
		return 0;
	}

	strncpy(sv.configstrings[start+i], name.c_str(), sizeof(sv.configstrings[i]));

	if(sv.state != SS_LOADING)
	{
		// send the update to everyone
		sv.multicast.beginWriting();
		sv.multicast.writeBits(SVC_CONFIGSTRING, svc_bitcount);
		sv.multicast.writeShort(start + i);
		sv.multicast.writeString(name.c_str());
		
		SV_Multicast(vec3_origin, MULTICAST_ALL_R);
	}

	return i;
}


int 	SV_ModelIndex(const std::string &name)
{
	if(name.empty())
	{
		Com_Printf("SV_ModelIndex: empty name\n");
		return 0;
	}

	return SV_FindIndex(name, CS_MODELS, MAX_MODELS, true);
}

int 	SV_ShaderIndex(const std::string &name)
{
	if(name.empty())
	{
		Com_Printf("SV_ShaderIndex: empty name\n");
		return 0;
	}

	return SV_FindIndex(name, CS_SHADERS, MAX_SHADERS, true);
}

int 	SV_AnimationIndex(const std::string &name)
{
	if(name.empty())
	{
		Com_Printf("SV_AnimationIndex: empty name\n");
		return 0;
	}

	return SV_FindIndex(name, CS_ANIMATIONS, MAX_ANIMATIONS, true);
}

int 	SV_SoundIndex(const std::string &name)
{
	if(name.empty())
	{
		Com_Printf("SV_SoundIndex: empty name\n");
		return 0;
	}

	return SV_FindIndex(name, CS_SOUNDS, MAX_SOUNDS, true);
}

int 	SV_LightIndex(const std::string &name)
{
	if(name.empty())
	{
		Com_Printf("SV_LightIndex: empty name\n");
		return 0;
	}

	return SV_FindIndex(name, CS_LIGHTS, MAX_LIGHTS, true);
}



/*
================
SV_CreateBaseline

Entity baselines are used to compress the update messages
to the clients -- only the fields that differ from the
baseline will be transmitted
================
*/
static void 	SV_CreateBaseline()
{
	for(unsigned int i=1; i<ge->entities->size(); i++)
	{
		sv_entity_c *ent = SV_GetEntityByNum(i);
		
		if(!ent)
			continue;
		
		if(!ent->_r.inuse)
			continue;
		
		if(!ent->_s.index_model && !ent->_s.index_sound && !ent->_s.effects)
			continue;

		// take current state as baseline
		sv.baselines[i] = ent->_s;
	}
}


/*
================
SV_SpawnServer

Change the server to a new map, taking all connected
clients along with it.
================
*/
void 	SV_SpawnServer(const std::string &server, char *spawnpoint, bool attractloop, bool loadgame)
{
	if(attractloop)
		Cvar_Set("paused", "0");

	Com_Printf("------- Server Initialization -------\n");

	Com_DPrintf("SpawnServer: %s\n", server.c_str());

	svs.spawncount++;		// any partially connected client will be
							// restarted
	sv.state = SS_DEAD;
	Com_SetServerState(sv.state);

	// wipe the entire per-level structure
	sv.clear();
	svs.realtime = 0;
	sv.loadgame = loadgame;
	sv.attractloop = attractloop;

	// save name for levels that don't set message
	strcpy(sv.configstrings[CS_MAPNAME], server.c_str());

	// leave slots at start for clients only
	//FIXME
	/*
	for(std::vector<sv_client_c*>::const_iterator ir = svs.clients.begin(); ir != svs.clients.end(); ir++)
	{
		sv_client_c *cl = *ir;
	
		if(!cl)
			continue;
		
		// needs to reconnect
		if(cl->getState() > CS_CONNECTED)
			cl->setState(CS_CONNECTED);
			
		cl->lastframe = -1;
	}
	*/

	sv.time = 1000;
	
	sv.name = server;
	strcpy(sv.configstrings[CS_MAPNAME], server.c_str());
	
	//
	// create collision map
	//
	ge->G_ClearWorld(server);

	//
	// spawn entities
	//	

	// precache and static commands can be issued during
	// map initialization
	sv.state = SS_LOADING;
	Com_SetServerState(sv.state);

	// load and spawn all other entities
	cmodel_c* cworld = CM_GetModelByNum(0);
	ge->G_SpawnEntities(sv.name, (char*)cworld->entityString(), spawnpoint);

	// run two frames to allow everything to settle
	ge->G_RunFrame();
	ge->G_RunFrame();

	// all precaches are complete
	sv.state = SS_GAME;
	Com_SetServerState(sv.state);
	
	// create a baseline for more efficient communications
	SV_CreateBaseline();

	// check for a savegame
	//SV_CheckForSavegame();

	// set serverinfo variable
	Cvar_Set2("mapname", sv.name, CVAR_SERVERINFO | CVAR_INIT, true);
	
	// free unused models
	CM_EndRegistration();

	Com_Printf("-------------------------------------\n");
}

/*
==============
SV_InitGame

A brand new game has been started
==============
*/
void 	SV_InitGame()
{
	if(svs.initialized)
	{
		// cause any connected clients to reconnect
		SV_Shutdown("Server restarted\n", true);
	}
	else
	{
		// make sure the client is down
		CL_Drop();
		//SCR_BeginLoadingPlaque ();
	}

	// get any latched variable changes (maxclients, etc)
	Cvar_SetLatchedVars();

	svs.initialized = true;

	if(Cvar_VariableValue("g_coop") && Cvar_VariableValue("g_deathmatch"))
	{
		Com_Printf("Deathmatch and Coop both set, disabling Coop\n");
		Cvar_Set2("g_coop", "0",  CVAR_SERVERINFO | CVAR_LATCH, true);
	}

	// dedicated servers are can't be single player and are usually DM
	// so unless they explicity set coop, force it to deathmatch
	if(dedicated->getInteger())
	{
		if(!Cvar_VariableValue("g_coop"))
			Cvar_Set2("g_deathmatch", "1",  CVAR_SERVERINFO | CVAR_LATCH, true);
	}

	// init clients
	if(Cvar_VariableValue("g_deathmatch"))
	{
		if(maxclients->getInteger() <= 1)
			Cvar_Set2("maxclients", "8", CVAR_SERVERINFO | CVAR_LATCH, true);
			
		else if(maxclients->getInteger() > MAX_CLIENTS)
			Cvar_Set2("maxclients", va("%i", MAX_CLIENTS), CVAR_SERVERINFO | CVAR_LATCH, true);
	}
	else if(Cvar_VariableValue("g_coop"))
	{
		if(maxclients->getInteger() <= 1 || maxclients->getInteger() > 4)
			Cvar_Set2("maxclients", "4", CVAR_SERVERINFO | CVAR_LATCH, true);
	}
	else	// non-deathmatch, non-coop is one player
	{
		Cvar_Set2("maxclients", "1", CVAR_SERVERINFO | CVAR_LATCH, true);
	}

	svs.spawncount = rand();
#if 1
	svs.clients = std::vector<sv_client_c*>(maxclients->getInteger());
	//for(int i=0; i<maxclients->getInteger(); i++)
	//	svs.clients[i] = new sv_client_c(	
	
	svs.num_client_entities = maxclients->getInteger() * UPDATE_BACKUP * 64;
	svs.client_entities = (entity_state_t*)Com_Alloc(sizeof(entity_state_t)*svs.num_client_entities);
#else
	svs.clients = (sv_client_c*)Com_Alloc(sizeof(sv_client_c)*maxclients->getInteger());
	svs.num_client_entities = maxclients->getInteger()*UPDATE_BACKUP*64;
	svs.client_entities = (entity_state_t*)Com_Alloc(sizeof(entity_state_t)*svs.num_client_entities);
#endif


	//
	// init network stuff
	//
	//Sys_InitNet(true);
	
	
	//
	// heartbeats will always be sent to the id master
	//
	char	idmaster[32];
	
	svs.last_heartbeat = -99999;		// send immediately
	Com_sprintf(idmaster, sizeof(idmaster), "192.246.40.37:%i", PORT_MASTER);
	Sys_StringToAdr(idmaster, master_adr[0]);


	//
	// init game
	//
	SV_InitGameProgs();
	
	/*
	for(int i=0; i<maxclients->getInteger(); i++)
	{
		sv_entity_c *ent = SV_GetEntityByNum(i+1);
		svs.clients[i].entity = ent;
		memset(&svs.clients[i].lastcmd, 0, sizeof(svs.clients[i].lastcmd));
	}
	*/
}


/*
======================
SV_Map

  the full syntax is:

  map [*]<map>$<startspot>+<nextserver>

command from the console or progs.
Map can also be a.cin, .pcx, or .dm2 file
Nextserver is used to allow a cinematic to play, then proceed to
another level:

	map tram.cin+jail_e3
======================
*/
void 	SV_Map(bool attractloop, const char *levelstring, bool loadgame)
{
	char	level[MAX_QPATH];
	char	*ch;
	int		l;
	char	spawnpoint[MAX_QPATH];

	sv.loadgame = loadgame;
	sv.attractloop = attractloop;

	if(sv.state == SS_DEAD && !sv.loadgame)
		SV_InitGame();	// the game is just starting

	strcpy(level, levelstring);

	// if there is a + in the map, set nextserver to the remainder
	ch = strstr(level, "+");
	if(ch)
	{
		*ch = 0;
			Cvar_Set("nextserver", va("gamemap \"%s\"", ch+1));
	}
	else
		Cvar_Set("nextserver", "");

	// if there is a $, use the remainder as a spawnpoint
	ch = strstr(level, "$");
	if(ch)
	{
		*ch = 0;
		strcpy(spawnpoint, ch+1);
	}
	else
		spawnpoint[0] = 0;

	// skip the end-of-unit flag if necessary
	if(level[0] == '*')
		strcpy(level, level+1);

	l = strlen(level);
	
	SV_BroadcastCommand("changing\n");
	SV_SendClientMessages();
	SV_SpawnServer(level, spawnpoint, attractloop, loadgame);
	Cbuf_CopyToDefer();

	SV_BroadcastCommand("reconnect\n");
}
