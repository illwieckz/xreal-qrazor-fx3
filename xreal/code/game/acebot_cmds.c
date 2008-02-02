/*
===========================================================================
Copyright (C) 1998 Steve Yeager
Copyright (C) 2008 Robert Beckebans <trebor_7@users.sourceforge.net>

This file is part of XreaL source code.

XreaL source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

XreaL source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XreaL source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
// acebot_cmds.c - Main internal command processor


#include "g_local.h"
#include "acebot.h"

#if defined(ACEBOT)

qboolean        debug_mode = qtrue;

///////////////////////////////////////////////////////////////////////
// Special command processor
///////////////////////////////////////////////////////////////////////
qboolean ACECM_Commands(gentity_t * ent)
{
	char            cmd[MAX_TOKEN_CHARS];
	char            arg1[MAX_TOKEN_CHARS];
	char            arg2[MAX_TOKEN_CHARS];
	char            arg3[MAX_TOKEN_CHARS];
	char            arg4[MAX_TOKEN_CHARS];
	int             node;

	trap_Argv(0, cmd, sizeof(cmd));

	if(Q_stricmp(cmd, "addnode") == 0 && debug_mode)
	{
		trap_Argv(1, arg1, sizeof(arg1));
		ent->bs.last_node = ACEND_AddNode(ent, atoi(arg1));
	}
	else if(Q_stricmp(cmd, "removelink") == 0 && debug_mode)
	{
		trap_Argv(1, arg1, sizeof(arg1));
		trap_Argv(2, arg2, sizeof(arg2));
		ACEND_RemoveNodeEdge(ent, atoi(arg1), atoi(arg2));
	}
	else if(Q_stricmp(cmd, "addlink") == 0 && debug_mode)
	{
		trap_Argv(1, arg1, sizeof(arg1));
		trap_Argv(2, arg2, sizeof(arg2));
		ACEND_UpdateNodeEdge(atoi(arg1), atoi(arg2));
	}
	else if(Q_stricmp(cmd, "showpath") == 0 && debug_mode)
	{
		trap_Argv(1, arg1, sizeof(arg1));
		ACEND_ShowPath(ent, atoi(arg1));
	}
	else if(Q_stricmp(cmd, "findnode") == 0 && debug_mode)
	{
		node = ACEND_FindClosestReachableNode(ent, NODE_DENSITY, NODE_ALL);
		safe_bprintf(PRINT_MEDIUM, "node: %d type: %d x: %f y: %f z %f\n", node, nodes[node].type, nodes[node].origin[0],
					 nodes[node].origin[1], nodes[node].origin[2]);
	}
	else if(Q_stricmp(cmd, "movenode") == 0 && debug_mode)
	{
		trap_Argv(1, arg1, sizeof(arg1));
		trap_Argv(2, arg2, sizeof(arg2));
		trap_Argv(3, arg3, sizeof(arg3));
		trap_Argv(4, arg4, sizeof(arg4));

		node = atoi(arg1);
		nodes[node].origin[0] = atof(arg2);
		nodes[node].origin[1] = atof(arg3);
		nodes[node].origin[2] = atof(arg4);
		safe_bprintf(PRINT_MEDIUM, "node: %d moved to x: %f y: %f z %f\n", node, nodes[node].origin[0], nodes[node].origin[1],
					 nodes[node].origin[2]);
	}
	else
	{
		return qfalse;
	}

	return qtrue;
}


///////////////////////////////////////////////////////////////////////
// Called when the level changes, store maps and bots (disconnected)
///////////////////////////////////////////////////////////////////////
void ACECM_Store()
{
	ACEND_SaveNodes();
}

///////////////////////////////////////////////////////////////////////
// These routines are bot safe print routines, all id code needs to be 
// changed to these so the bots do not blow up on messages sent to them. 
// Do a find and replace on all code that matches the below criteria. 
//
// (Got the basic idea from Ridah)
//  
//  change: gi.cprintf to safe_cprintf
//  change: gi.bprintf to safe_bprintf
//  change: gi.centerprintf to safe_centerprintf
// 
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// Debug print, could add a "logging" feature to print to a file
///////////////////////////////////////////////////////////////////////
void debug_printf(char *fmt, ...)
{
	char            bigbuffer[0x10000];
	int             len;
	va_list         argptr;
	int             i;
	gclient_t      *cl;
	gentity_t      *ent;

	va_start(argptr, fmt);
	len = vsprintf(bigbuffer, fmt, argptr);
	va_end(argptr);

	//if(g_dedicated.integer)
	{
		G_Printf(bigbuffer);
	}

	for(i = 0; i < g_maxclients.integer; i++)
	{
		cl = level.clients + i;
		ent = level.gentities + cl->ps.clientNum;
		
		if(cl->pers.connected != CON_CONNECTED)
		{
			continue;
		}

		if(ent->r.svFlags & SVF_BOT)
		{
			continue;
		}

		trap_SendServerCommand(ent - g_entities, va("print \"%s\"", bigbuffer));
	}
}

///////////////////////////////////////////////////////////////////////
// botsafe cprintf
///////////////////////////////////////////////////////////////////////
void safe_cprintf(gentity_t * ent, int printlevel, char *fmt, ...)
{
	char            bigbuffer[0x10000];
	va_list         argptr;
	int             len;

	if(ent && (ent->r.svFlags & SVF_BOT))
		return;

	va_start(argptr, fmt);
	len = vsprintf(bigbuffer, fmt, argptr);
	va_end(argptr);

	trap_SendServerCommand(ent - g_entities, va("print \"%s\"", bigbuffer));

}

///////////////////////////////////////////////////////////////////////
// botsafe centerprintf
///////////////////////////////////////////////////////////////////////
void safe_centerprintf(gentity_t * ent, char *fmt, ...)
{
	char            bigbuffer[0x10000];
	va_list         argptr;
	int             len;

	if(ent && (ent->r.svFlags & SVF_BOT))
		return;

	va_start(argptr, fmt);
	len = vsprintf(bigbuffer, fmt, argptr);
	va_end(argptr);

	trap_SendServerCommand(ent - g_entities, va("centerprint \"%s\"", bigbuffer));
}

///////////////////////////////////////////////////////////////////////
// botsafe bprintf
///////////////////////////////////////////////////////////////////////
void safe_bprintf(int printlevel, char *fmt, ...)
{
	char            bigbuffer[0x10000];
	int             len;
	va_list         argptr;
	int             i;
	gclient_t      *cl;
	gentity_t      *ent;

	va_start(argptr, fmt);
	len = vsprintf(bigbuffer, fmt, argptr);
	va_end(argptr);

	//if(g_dedicated.integer)
	{
		G_Printf(bigbuffer);
	}

	for(i = 0; i < g_maxclients.integer; i++)
	{
		cl = level.clients + i;
		ent = level.gentities + cl->ps.clientNum;
		
		if(cl->pers.connected != CON_CONNECTED)
		{
			continue;
		}

		if(ent->r.svFlags & SVF_BOT)
		{
			continue;
		}

		trap_SendServerCommand(ent - g_entities, va("print \"%s\"", bigbuffer));
	}
}

#endif



