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



/*
===============================================================================

OPERATOR CONSOLE ONLY COMMANDS

These commands can only be entered from stdin or by a remote operator datagram
===============================================================================
*/

/*
====================
SV_SetMaster_f

Specify a list of master servers
====================
*/
static void 	SV_SetMaster_f()
{
	int		i, slot;

	// only dedicated servers send heartbeats
	if(!dedicated->getValue())
	{
		Com_Printf ("Only dedicated servers use masters.\n");
		return;
	}

	// make sure the server is listed public
	Cvar_Set("public", "1");

	for(i=1; i<MAX_MASTERS; i++)
		memset (&master_adr[i], 0, sizeof(master_adr[i]));

	slot = 1;		// slot 0 will always contain the id master
	for(i=1; i<Cmd_Argc(); i++)
	{
		if(slot == MAX_MASTERS)
			break;

		if(!Sys_StringToAdr(Cmd_Argv(i), master_adr[i]))
		{
			Com_Printf ("Bad address: %s\n", Cmd_Argv(i));
			continue;
		}
		if(master_adr[slot].port == 0)
			master_adr[slot].port = BigShort(PORT_MASTER);

		Com_Printf("Master server at %s\n", Sys_AdrToString(master_adr[slot]));

		Com_Printf("Sending a ping.\n");

		Netchan_OutOfBandPrint(master_adr[slot], "ping");

		slot++;
	}

	svs.last_heartbeat = -9999999;
}



/*
==================
SV_SetPlayer

Sets sv_client and sv_player to the player with idnum Cmd_Argv(1)
==================
*/
static sv_client_c*	SV_GetPlayer()
{
	sv_client_c	*cl;
	int			idnum;
	const char		*s;

	if(Cmd_Argc() < 2)
		return false;

	s = Cmd_Argv(1);

	// numeric values are just slot numbers
	if(s[0] >= '0' && s[0] <= '9')
	{
		idnum = atoi(Cmd_Argv(1));
		
		if(idnum < 0 || idnum >= (int)svs.clients.size())
		{
			Com_Printf("Bad client slot: %i\n", idnum);
			return NULL;
		}

		cl = svs.clients[idnum];
		
		if(!cl || !cl->getState())
		{
			Com_Printf("Client %i is not active\n", idnum);
			return NULL;
		}
		return cl;
	}

	// check for a name match
	for(std::vector<sv_client_c*>::const_iterator ir = svs.clients.begin(); ir != svs.clients.end(); ir++)
	{
		cl = *ir;
	
		if(!cl)
			continue;
			
		if(!cl->getState())
			continue;
			
		if(!strcmp(cl->getName(), s))
		{
			return cl;
		}
	}

	Com_Printf("Userid %s is not on the server\n", s);
	
	return NULL;
}


/*
==================
SV_Map_f

Goes directly to a given map without any savegame archiving.
For development work
==================
*/
static void 	SV_Map_f()
{
	std::string	map;
	std::string	expanded;

	// check to make sure the level exists
	map = Cmd_Argv(1);
	if(!strstr(map.c_str(), "."))
	{
#if 1
		expanded = "maps/" + map + ".map";
		if(VFS_FLoad(expanded, NULL) == -1)
		{
			Com_Printf("Can't find '%s'\n", expanded.c_str());
			return;
		}
		
		expanded = "maps/" + map + ".cm";
		if(VFS_FLoad(expanded, NULL) == -1)
		{
			Com_Printf("Can't find '%s'\n", expanded.c_str());
			return;
		}
		
		expanded = "maps/" + map + ".proc";
		if(VFS_FLoad(expanded, NULL) == -1)
		{
			Com_Printf("Can't find '%s'\n", expanded.c_str());
			return;
		}
#else		
		expanded = "maps/" + map + ".bsp";
		if(VFS_FLoad(expanded, NULL) == -1)
		{
			Com_Printf("Can't find '%s'\n", expanded.c_str());
			return;
		}
#endif
	}

	sv.state = SS_DEAD;		// don't save current level when changing
	
	// start up the next map
	SV_Map(false, Cmd_Argv(1), false);

	// archive server state
	strncpy(svs.mapcmd, Cmd_Argv(1), sizeof(svs.mapcmd)-1);
}


/*
==================
SV_DemoMap_f

Puts the server in demo mode on a specific map/cinematic
==================
*/
static void 	SV_DemoMap_f()
{
	SV_Map(true, Cmd_Argv(1), false);
}


/*
==================
SV_Kick_f

Kick a user off of the server
==================
*/
static void 	SV_Kick_f()
{
	if(!svs.initialized)
	{
		Com_Printf("No server running.\n");
		return;
	}

	if(Cmd_Argc() != 2)
	{
		Com_Printf("Usage: kick <userid>\n");
		return;
	}

	sv_client_c *cl = SV_GetPlayer();
	if(!cl)
		return;
	
	SV_BroadcastPrintf(PRINT_HIGH, "%s was kicked\n", cl->getName());
	
	// print directly, because the dropped client won't get the SV_BroadcastPrintf message
	cl->printf(PRINT_HIGH, "You were kicked from the game\n");
	
	cl->kick();
	return;
}


/*
================
SV_Status_f
================
*/
static void 	SV_Status_f()
{
	int			i, j, l;
	char		*s;
	int			ping;
	
	if(!svs.clients.size())
	{
		Com_Printf("No server running.\n");
		return;
	}
	
	Com_Printf("map              : %s\n", sv.name.c_str());

	Com_Printf("num score ping name            lastmsg address               qport \n");
	Com_Printf("--- ----- ---- --------------- ------- --------------------- ------\n");
	
	for(i=0; i<(int)svs.clients.size(); i++)
	{
		sv_client_c *cl = svs.clients[i];
	
		if(!cl)
			continue;
			
		if(!cl->getState())
			continue;
			
		Com_Printf("%3i ", i);
		
		Com_Printf("%5i ", cl->getEntity()->_r.ps.stats[STAT_FRAGS]);

		if(cl->getState() == CS_CONNECTED)
			Com_Printf("CNCT ");
			
		else if(cl->getState() == CS_ZOMBIE)
			Com_Printf("ZMBI ");
			
		else
		{
			ping = cl->getPing() < 9999 ? cl->getPing() : 9999;
			Com_Printf("%4i ", ping);
		}

		Com_Printf("%s", cl->getName());
		l = 16 - strlen(cl->getName());
		for (j=0 ; j<l ; j++)
			Com_Printf (" ");

		Com_Printf("%7i ", svs.realtime - cl->getLastMessageTime());

		s = Sys_AdrToString(cl->netchan.getRemoteAddress());
		Com_Printf("%s", s);
		l = 22 - strlen(s);
		for (j=0 ; j<l ; j++)
			Com_Printf(" ");
		
		Com_Printf("%5i", cl->netchan.getRemoteAddress().port);

		Com_Printf("\n");
	}
	Com_Printf("\n");
}

/*
==================
SV_ConSay_f
==================
*/
static void 	SV_ConSay_f()
{
	char	*p;
	char	text[1024];

	if(Cmd_Argc () < 2)
		return;

	strcpy(text, "console: ");
	p = (char*)Cmd_Args();

	if(*p == '"')
	{
		p++;
		p[strlen(p)-1] = 0;
	}

	strcat(text, p);

	for(std::vector<sv_client_c*>::const_iterator ir = svs.clients.begin(); ir != svs.clients.end(); ir++)
	{
		sv_client_c *cl = *ir;
	
		if(!cl)
			continue;
			
		if(cl->getState() != CS_SPAWNED)
			continue;
			
		cl->printf(PRINT_CHAT, "%s\n", text);
	}
}


/*
==================
SV_Heartbeat_f
==================
*/
static void 	SV_Heartbeat_f()
{
	svs.last_heartbeat = -9999999;
}


/*
===========
SV_Serverinfo_f

  Examine or change the serverinfo string
===========
*/
static void 	SV_Serverinfo_f()
{
	Com_Printf("Server info settings:\n");
	Com_PrintInfo(Cvar_Serverinfo());
}


/*
===========
SV_DumpUser_f

Examine all a users info strings
===========
*/
static void 	SV_DumpUser_f()
{
	if(Cmd_Argc() != 2)
	{
		Com_Printf("Usage: info <userid>\n");
		return;
	}

	sv_client_c *cl = SV_GetPlayer();
	if(!cl)
		return;

	Com_Printf("userinfo\n");
	Com_Printf("--------\n");
	Com_PrintInfo(cl->getUserInfo());

}


/*
===============
SV_KillServer_f

Kick everyone off, possibly in preparation for a new game

===============
*/
static void 	SV_KillServer_f()
{
	if(!svs.initialized)
		return;
		
	SV_Shutdown("Server was killed.\n", false);
	Sys_ShutdownNet();	// close network sockets
}

/*
===============
SV_ServerCommand_f

Let the game dll handle a command
===============
*/
static void 	SV_ServerCommand_f()
{
	if(!ge)
	{
		Com_Printf("No game loaded.\n");
		return;
	}

	ge->G_ServerCommand();
}


void	SV_InitOperatorCommands()
{
	Cmd_AddCommand("heartbeat",	SV_Heartbeat_f);
	Cmd_AddCommand("kick",		SV_Kick_f);
	Cmd_AddCommand("status",	SV_Status_f);
	Cmd_AddCommand("serverinfo",	SV_Serverinfo_f);
	Cmd_AddCommand("dumpuser",	SV_DumpUser_f);

	Cmd_AddCommand("map",		SV_Map_f);
	Cmd_AddCommand("demomap",	SV_DemoMap_f);
	Cmd_AddCommand("setmaster",	SV_SetMaster_f);

	if(dedicated->getValue())
		Cmd_AddCommand("say",	SV_ConSay_f);

	Cmd_AddCommand("killserver",	SV_KillServer_f);
	Cmd_AddCommand("sv",		SV_ServerCommand_f);
}

