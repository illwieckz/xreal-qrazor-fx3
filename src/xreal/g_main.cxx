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
// xreal --------------------------------------------------------------------
#include "g_local.h"
#include "g_player.h"
#include "g_target.h"
#include "g_world.h"

#include "bg_local.h"
#include "bg_physics.h"


game_export_t	globals;

g_game_c	game;
g_level_c	level;



int meansOfDeath;

g_entity_c*			g_world = NULL;
cmodel_c*			g_world_cmodel = NULL;
std::vector<sv_entity_c*>	g_entities;
//std::vector<g_item_c*>	g_items;

#if defined(ODE)
d_world_c*			g_ode_world = NULL;
d_space_c*			g_ode_space_toplevel = NULL;
//d_space_c*			g_ode_space_world = NULL;
d_bsp_c*			g_ode_bsp = NULL;
d_plane_c*			g_ode_testplane = NULL;
d_joint_group_c*		g_ode_contact_group = NULL;
#endif


cvar_t	*dedicated;
cvar_t	*maxclients;
cvar_t	*maxspectators;

cvar_t	*g_deathmatch;
cvar_t	*g_coop;
cvar_t	*g_skill;
cvar_t	*g_cheats;
cvar_t	*g_dmflags;
cvar_t	*g_fraglimit;
cvar_t	*g_timelimit;
cvar_t	*g_password;
cvar_t	*g_spectator_password;
cvar_t	*g_needpass;
cvar_t	*g_gravity;
cvar_t	*g_maxvelocity;
cvar_t	*g_filterban;
cvar_t	*g_flood_msgs;
cvar_t	*g_flood_persecond;
cvar_t	*g_flood_waitdelay;
cvar_t	*g_maplist;

cvar_t	*pm_gun_x;
cvar_t	*pm_gun_y;
cvar_t	*pm_gun_z;
cvar_t	*pm_rollspeed;
cvar_t	*pm_rollangle;
cvar_t	*pm_run_pitch;
cvar_t	*pm_run_roll;
cvar_t	*pm_bob_up;
cvar_t	*pm_bob_pitch;
cvar_t	*pm_bob_roll;
cvar_t	*pm_speed_cmd;
cvar_t	*pm_speed_stop;
cvar_t	*pm_speed_max;
cvar_t	*pm_accelerate;
cvar_t	*pm_accelerate_water;
cvar_t	*pm_accelerate_air;
cvar_t	*pm_accelerate_spectator;
cvar_t	*pm_friction;
cvar_t	*pm_friction_water;
cvar_t	*pm_friction_air;
cvar_t	*pm_friction_spectator;


void 	G_RunFrame();
void 	G_SpawnEntities(const std::string &mapname, char *entities, const std::string &spawnpoint);



void 	G_RunEntity(entity_c *ent);
void 	G_WriteGame(const char *filename, bool autosave);
void 	G_ReadGame(const char *filename);
void 	G_WriteLevel(const char *filename);
void 	G_ReadLevel(const char *filename);


static bool 	G_ClientConnect(sv_entity_c *ent, info_c &userinfo)
{
	g_player_c *player = (g_player_c*)ent;
	
	return player->clientConnect(userinfo);
}

static void 	G_ClientBegin(sv_entity_c *ent)
{
	g_player_c *player = (g_player_c*)ent;
	
	player->clientBegin();
}

static void 	G_ClientUserinfoChanged(sv_entity_c *ent, info_c &userinfo)
{
	g_player_c *player = (g_player_c*)ent;
	
	player->clientUserinfoChanged(userinfo);
}

static void	G_ClientDisconnect(sv_entity_c *ent)
{
	g_player_c *player = (g_player_c*)ent;
	
	player->clientDisconnect();
}

static void	G_ClientCommand(sv_entity_c *ent)
{
	g_player_c *player = (g_player_c*)ent;
	
	player->clientCommand();
}

static void 	G_ClientThink(sv_entity_c *ent, const usercmd_t &cmd)
{
	g_player_c *player = (g_player_c*)ent;
	
	player->clientThink(cmd);
}


/*
============
InitGame

This will be called when the dll is first loaded, which
only happens when a new game is started or a save game
is loaded.
============
*/
static void	G_InitGame()
{
	trap_Com_Printf("======= G_InitGame \"XreaL\" =======\n");

	trap_Cvar_Get("gamename", GAMEVERSION , CVAR_SERVERINFO | CVAR_LATCH);
	trap_Cvar_Get("gamedate", __DATE__ , CVAR_SERVERINFO | CVAR_LATCH);

	dedicated		= trap_Cvar_Get("dedicated", "0", CVAR_INIT);
	maxclients		= trap_Cvar_Get("maxclients", "4", CVAR_SERVERINFO | CVAR_LATCH);
	maxspectators		= trap_Cvar_Get("maxspectators", "4", CVAR_SERVERINFO);

	g_deathmatch		= trap_Cvar_Get("g_deathmatch", "0", CVAR_LATCH);
	g_coop			= trap_Cvar_Get("g_coop", "0", CVAR_LATCH);
	g_skill			= trap_Cvar_Get("g_skill", "1", CVAR_LATCH);
	g_cheats		= trap_Cvar_Get("g_cheats", "0", CVAR_SERVERINFO | CVAR_LATCH);
	g_dmflags		= trap_Cvar_Get("g_dmflags", "0", CVAR_SERVERINFO);
	g_fraglimit		= trap_Cvar_Get("g_fraglimit", "0", CVAR_SERVERINFO);
	g_timelimit		= trap_Cvar_Get("g_timelimit", "0", CVAR_SERVERINFO);
	g_password		= trap_Cvar_Get("g_password", "", CVAR_USERINFO);
	g_spectator_password	= trap_Cvar_Get("g_spectator_password", "", CVAR_USERINFO);
	g_needpass		= trap_Cvar_Get("g_needpass", "0", CVAR_SERVERINFO);
#if defined(ODE)
	g_gravity		= trap_Cvar_Get("g_gravity", "1", CVAR_SERVERINFO);
#else
	g_gravity		= trap_Cvar_Get("g_gravity", "800", CVAR_SERVERINFO);
#endif
	g_maxvelocity		= trap_Cvar_Get("g_maxvelocity", "2000", CVAR_NONE);
	g_filterban		= trap_Cvar_Get("g_filterban", "1", CVAR_NONE);
	g_flood_msgs		= trap_Cvar_Get("g_flood_msgs", "4", CVAR_NONE);
	g_flood_persecond	= trap_Cvar_Get("g_flood_persecond", "4", CVAR_NONE);
	g_flood_waitdelay	= trap_Cvar_Get("g_flood_waitdelay", "10", CVAR_NONE);
	g_maplist		= trap_Cvar_Get("g_maplist", "", CVAR_NONE);

	pm_gun_x		= trap_Cvar_Get("pm_gun_x", "0", CVAR_NONE);
	pm_gun_y		= trap_Cvar_Get("pm_gun_y", "0", CVAR_NONE);
	pm_gun_z		= trap_Cvar_Get("pm_gun_z", "0", CVAR_NONE);
	pm_rollspeed		= trap_Cvar_Get("pm_rollspeed", "200", CVAR_NONE);
	pm_rollangle		= trap_Cvar_Get("pm_rollangle", "2", CVAR_NONE);
	pm_run_pitch		= trap_Cvar_Get("pm_run_pitch", "0.002", CVAR_NONE);
	pm_run_roll		= trap_Cvar_Get("pm_run_roll", "0.005", CVAR_NONE);
	pm_bob_up		= trap_Cvar_Get("pm_bob_up", "0.005", CVAR_NONE);
	pm_bob_pitch		= trap_Cvar_Get("pm_bob_pitch", "0.002", CVAR_NONE);
	pm_bob_roll		= trap_Cvar_Get("pm_bob_roll", "0.002", CVAR_NONE);
	pm_speed_cmd		= trap_Cvar_Get("pm_speed_cmd", "320", CVAR_NONE);
	pm_speed_stop		= trap_Cvar_Get("pm_speed_stop", "100", CVAR_NONE);
	pm_speed_max		= trap_Cvar_Get("pm_speed_max", "400", CVAR_NONE);
	pm_accelerate		= trap_Cvar_Get("pm_accelerate", "10", CVAR_NONE);
	pm_accelerate_water	= trap_Cvar_Get("pm_accelerate_water", "5", CVAR_NONE);
	pm_accelerate_air	= trap_Cvar_Get("pm_accelerate_air", "1", CVAR_NONE);
	pm_accelerate_spectator	= trap_Cvar_Get("pm_accelerate_spectator", "8", CVAR_NONE);
	pm_friction		= trap_Cvar_Get("pm_friction", "6", CVAR_NONE);
	pm_friction_water	= trap_Cvar_Get("pm_friction_water", "1", CVAR_NONE);
	pm_friction_air		= trap_Cvar_Get("pm_friction_air", "3", CVAR_NONE);
	pm_friction_spectator	= trap_Cvar_Get("pm_friction_spectator", "5", CVAR_NONE);
	

	// initialize all client entities for this game
	game.maxclients = maxclients->getInteger();
	
#if defined(ODE)
	G_InitDynamics();	// ODE
#endif
	BG_InitPhysics();	// Tr3B - new custom physics engine
	
	g_world = new g_world_c();
		
	for(int i=0; i<game.maxclients; i++)
	{
		new g_player_c();
	}
		
	globals.entities = &g_entities;
	trap_Com_Printf("G_InitGame: entities num: %i\n", g_entities.size());
	
//	G_InitPythonVM();
}

static void 	G_ShutdownGame()
{
	trap_Com_Printf("======= G_ShutdownGame \"XreaL\" =======\n");
	
//	G_ShutdownPythonVM();

	G_ShutdownEntities();
#if defined(ODE)
	G_ShutdownDynamics();
#endif
	BG_ShutdownPhysics();
}


/*
=================
GetGameAPI

Returns a pointer to the structure with all entry points
and global variables
=================
*/

#ifdef __cplusplus
extern "C" {
#endif

game_export_t*		GetGameAPI (game_import_t *import)
{
	extern game_import_t gi;
	 
	gi = *import;

	globals.apiversion 		= GAME_API_VERSION;
	
	globals.G_InitGame		= G_InitGame;
	globals.G_ShutdownGame		= G_ShutdownGame;
	
	globals.G_ClearWorld		= G_ClearWorld;
	globals.G_SpawnEntities		= G_SpawnEntities;
	
	globals.G_ClientConnect 	= G_ClientConnect;
	globals.G_ClientBegin		= G_ClientBegin;
	globals.G_ClientUserinfoChanged = G_ClientUserinfoChanged;
	globals.G_ClientDisconnect 	= G_ClientDisconnect;
	globals.G_ClientCommand 	= G_ClientCommand;
	globals.G_ClientThink		= G_ClientThink;

	globals.G_RunFrame 		= G_RunFrame;

	globals.G_ServerCommand 	= G_ServerCommand;

	return &globals;
}

#ifdef __cplusplus
}
#endif

#ifndef GAME_HARD_LINKED
// this is only here so the functions in q_shared.c and q_shwin.c can link
void 	Com_Error(err_type_e type, const char *fmt, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start(argptr, fmt);
	vsprintf(text, fmt, argptr);
	va_end(argptr);

	extern game_import_t gi;
	gi.Com_Error(type, "%s", text);
}

void 	Com_Printf(const char *fmt, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start(argptr, fmt);
	vsprintf(text, fmt, argptr);
	va_end(argptr);

	extern game_import_t gi;
	gi.Com_Printf("%s", text);
}
#endif



static void	G_ClientBeginServerFrames()
{
	g_player_c	*player;

	// calc the player views now that all pushing
	// and damage has been added
	for(int i=1; i<maxclients->getInteger()+1; i++)
	{
		player = (g_player_c*)g_entities[i];
		
		if(!player->_r.inuse)
			continue;
			
		player->beginServerFrame();
	}

}


static void	G_ClientEndServerFrames()
{
	g_player_c	*player;

	// calc the player views now that all pushing
	// and damage has been added
	for(int i=1; i<maxclients->getInteger()+1; i++)
	{
		player = (g_player_c*)g_entities[i];
		
		if(!player->_r.inuse)
			continue;
			
		player->endServerFrame();
	}

}

/*
=================
CreateTargetChangeLevel

Returns the created target changelevel
=================
*/
static g_target_changelevel_c*	G_CreateTargetChangeLevel(const std::string &map)
{
	level.nextmap = map;

	return new g_target_changelevel_c();
}

static void	G_EndDMLevel()
{
	char *s, *t, *f;
	static const char *seps = " ,\n\r";

	// stay on same level flag
	if(g_dmflags->getInteger() & DF_SAME_LEVEL)
	{
		G_BeginIntermission(G_CreateTargetChangeLevel(level.mapname));
		return;
	}

	// see if it's in the map list
	if(!X_strequal(g_maplist->getString(), ""))
	{
		s = strdup(g_maplist->getString());
		f = NULL;
		t = strtok(s, seps);
		while(t != NULL)
		{
			if(X_stricmp(t, level.mapname.c_str()) == 0)
			{
				// it's in the list, go to the next one
				t = strtok(NULL, seps);
				if(t == NULL)
				{
					// end of list, go to first one
					if(f == NULL) // there isn't a first one, same level
						G_BeginIntermission(G_CreateTargetChangeLevel (level.mapname) );
					else
						G_BeginIntermission(G_CreateTargetChangeLevel (f) );
				} else
					G_BeginIntermission(G_CreateTargetChangeLevel (t) );
				free(s);
				return;
			}
			if (!f)
				f = t;
			t = strtok(NULL, seps);
		}
		free(s);
	}

	if(level.nextmap.length()) // go to a specific map
	{
		G_BeginIntermission(G_CreateTargetChangeLevel(level.nextmap));
	}
	else
	{	// search for a changelevel
		g_entity_c* ent = G_FindByClassName(NULL, "target_changelevel");
		if(!ent)
		{
			// the map designer didn't include a changelevel,
			// so create a fake ent that goes back to the same level
			G_BeginIntermission(G_CreateTargetChangeLevel(level.mapname));
			return;
		}
		
		G_BeginIntermission(static_cast<g_target_changelevel_c*>(ent));
	}
}


static void	G_CheckNeedPass()
{
	int need;

	// if password or spectator_password has changed, update needpass
	// as needed
	if(g_password->isModified() || g_spectator_password->isModified()) 
	{
		g_password->isModified(false);
		g_spectator_password->isModified(false);

		need = 0;

		if(!X_strequal(g_password->getString(), "") && X_stricmp(g_password->getString(), "none"))
			need |= 1;
			
		if(!X_strequal(g_spectator_password->getString(), "") && X_stricmp(g_spectator_password->getString(), "none"))
			need |= 2;

		trap_Cvar_Set("g_needpass", va("%d", need));
	}
}

static void	G_CheckDMRules()
{
	if(level.intermission_time)
		return;

	if(!g_deathmatch->getInteger())
		return;

	if(g_timelimit->getInteger())
	{
		if(level.time >= g_timelimit->getInteger()*60000)
		{
			trap_SV_BPrintf(PRINT_HIGH, "Timelimit hit.\n");
			G_EndDMLevel();
			return;
		}
	}

	if(g_fraglimit->getInteger())
	{
		for(int i=0 ; i<maxclients->getInteger(); i++)
		{
			g_player_c *player = (g_player_c*)g_entities[i+1];
			
			if(!player->_r.inuse)
				continue;

			if(player->hitFragLimit())
			{
				trap_SV_BPrintf(PRINT_HIGH, "Fraglimit hit.\n");
				G_EndDMLevel();
				return;
			}
		}
	}
}


static void 	G_ExitLevel()
{
	std::string	command;

	command =  "gamemap \"" + level.changemap + "\"\n";
	trap_Cbuf_AddText((char*)command.c_str());
	level.changemap = "";
	level.intermission_exit = false;
	level.intermission_time = 0;
	
	G_ClientEndServerFrames();

	// clear some things before going to next level
	for(int i=1; i<maxclients->getInteger()+1; i++)
	{
		g_player_c *player = (g_player_c*)g_entities[i];
		
		if(!player->_r.inuse)
			continue;
			
		player->resetHealth();
	}

}



static void	G_RemoveUnneededEntities()
{
	for(std::vector<sv_entity_c*>::const_iterator ir = g_entities.begin(); ir != g_entities.end(); ir++)
	{
		g_entity_c *ent = (g_entity_c*)*ir;
		
		if(ent)
		{		
			if(ent->getRemove())
			{
				//trap_Com_Printf("G_RunRemoveUneededEntities: killing '%s' %i ...\n", ent->_classname, ent->_s.number);
				delete ent;
			}
		}
	}
}

/*
================
G_RunFrame

Advances the world by 100 milliseconds
================
*/
void 	G_RunFrame()
{
	g_entity_c	*ent;

	level.framenum++;
	level.time = level.framenum * FRAMETIME;
	
	// for debugging, check if ODE hangs anywhere
//	trap_Com_Printf("G_RunFrame: %i\n", level.framenum); 

	// exit intermissions
	if(level.intermission_exit)
	{
		G_ExitLevel();
		return;
	}
	
	// prepare all active players for serverframe
	G_ClientBeginServerFrames();

	// treat each object in turn
	// even the world gets a chance to think
	// apply forces to the bodies as necessary
	for(unsigned int i=0; i<g_entities.size(); i++)
	{
		ent = (g_entity_c*)g_entities[i];
		
		if(!ent)
			continue;
			
		//trap_Com_Printf("G_RunFrame: running %s %i ...\n", ent->_classname.c_str(), ent->_s.number);
			
		//ent->_s.origin2 = ent->_s.origin;

		//TODO
		//ent->checkGroundEntity();
		
		ent->run();
	}
	
	
	// apply ODE dynamics
#if defined(ODE)
	G_RunDynamics(FRAMETIME);
	
	// update network entity states and area information for network culling
	// update entity states with applied dynamics from the rigid bodies
	for(unsigned int i=0; i<g_entities.size(); i++)
	{
		ent = (g_entity_c*) g_entities[i];
		
		if(!ent)
			continue;
			
		if(!ent->_r.inuse)
			continue;
			
		//if(i > 0 && i <= maxclients->getInteger())
		//	continue;
			
		// update network state	
		ent->updateOrigin();
		ent->updateRotation();
		ent->updateVelocity();
		
		//ent->_r.area = trap_CM_PointAreanum(ent->_s.origin);
		
		//if(ent->_s.origin != ent->_s.origin2)
		/*
		int area = trap_CM_PointAreanum(ent->_s.origin);
			
		if(area)
		{	
			// doors may legally straggle two areas,
			// but nothing should evern need more than that
			if(ent->_r.area && ent->_r.area != area)
			{
				//if(ent->_r.area2 && ent->_r.area2 != area && sv.state == SS_LOADING)
				//	Com_DPrintf("Object touching 3 areas at %f %f %f\n", ent->_r.bbox_abs._mins[0], ent->_r.bbox_abs._mins[1], ent->_r.bbox_abs._mins[2]);
				
				ent->_r.area2 = area;
			}
			else
				ent->_r.area = area;
		}
		
		ent->_s.origin2.clear();
		*/
	}
#endif // defined(ODE)
	
	// see if it is time to end a deathmatch
	G_CheckDMRules();

	// see if needpass needs updated
	G_CheckNeedPass();

	// build the playerstate_t structures for all players
	G_ClientEndServerFrames();
	
	// kill entities that can be removed
	G_RemoveUnneededEntities();
}

