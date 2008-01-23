/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006 Robert Beckebans <trebor_7@users.sourceforge.net>

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

// g_svcmds.c - this file holds commands that can be executed by the server console, but not remote clients
#include "g_local.h"

void Svcmd_AddIP_f(void)
{
	int             bits, duration, x;
	unsigned        realmask, ip;
	const char     *reason;
	char            str[32], *p, unit[16];

	if(trap_Argc() < 4)
	{
		G_Printf("Usage: addip ip[/bitmask] time unit [reason]\n");
		return;
	}

	trap_Argv(1, str, sizeof(str));

	p = strchr(str, '/');
	if(p)
	{
		p[0] = 0;
		p++;
		bits = atoi(p);
	}
	else
		bits = 32;

	if(bits <= 0 || bits > 32)
	{
		G_Printf("Invalid ban mask %d, must be 1-32\n", bits);
		return;
	}

	ip = Admin_IPStringToInt(str);
	if(ip == (unsigned int)-1)
	{
		G_Printf("Bad address %s\n", str);
		return;
	}

	trap_Argv(2, str, sizeof(str));
	duration = atoi(str);
	if(!duration)
	{
		G_Printf("Must specify ban time (use -1 for permanent).\n");
		return;
	}

	if(duration > 0)
	{
		trap_Argv(3, unit, sizeof(unit));
		if(!Q_stricmpn(unit, "min", 3))
			duration *= (60);
		else if(!Q_stricmpn(unit, "hour", 4))
			duration *= (60 * 60);
		else if(!Q_stricmpn(unit, "day", 3))
			duration *= (60 * 60 * 24);
		else if(!Q_stricmpn(unit, "week", 4))
			duration *= (60 * 60 * 24 * 7);
		else if(!Q_stricmpn(unit, "year", 4))
			duration *= (60 * 60 * 24 * 365);
		else if(!Q_stricmpn(unit, "sec", 3))
			duration *= 1;
		else
		{
			G_Printf("Unit must be one of: secs, mins, hours, days, weeks, years\n");
			return;
		}
		x = 4;
	}
	else
		x = 3;

	realmask = Admin_MaskFromBits(bits);
	reason = ConcatArgs(x);

	if(!reason[0])
		reason = "unspecified";

	Admin_Ban(ip, duration, realmask, reason);
	G_Printf("%s/%d added to IP banlist.\n", Admin_IPIntToString(ip), Admin_BitsFromMask(realmask));
}

void Svcmd_RemoveIP_f(void)
{
	unsigned        bits, realmask, ip;
	char            str[32], *p;

	if(trap_Argc() != 2)
	{
		Com_Printf("Usage: removeip ip[/mask]\n");
		return;
	}

	trap_Argv(1, str, sizeof(str));

	p = strchr(str, '/');
	if(p)
	{
		p[0] = 0;
		p++;
		bits = atoi(p);
	}
	else
		bits = 32;

	if(bits <= 0 || bits > 32)
	{
		G_Printf("Invalid ban mask %d, must be 1-32\n", bits);
		return;
	}

	ip = Admin_IPStringToInt(str);
	if(ip == (unsigned int)-1)
	{
		G_Printf("Bad address %s\n", str);
		return;
	}

	realmask = Admin_MaskFromBits(bits);

	if(Admin_RemoveBan(ip, realmask))
		G_Printf("%s removed from IP banlist.\n", str);
	else
		G_Printf("Could not find ban for %s.\n", str);
}

void Svcmd_Addaccount_f(void)
{
	char            user[16], pass[16], str[16];
	unsigned int	permissions;

	if(trap_Argc() < 4)
	{
		G_Printf("Usage: addaccount <user> <password> <permissionbits>\n");
		return;
	}

	trap_Argv(1, user, sizeof(user));
	trap_Argv(2, pass, sizeof(pass));
	trap_Argv(3, str, sizeof(str));

	if(!Q_stricmp(str, "all"))
		permissions = PERMISSION_BITS;
	else
		permissions = atoi(str);

	if(Admin_AddAccount(user, pass, permissions))
		G_Printf("Account '%s' added.\n", user);
	else
		G_Printf("Username '%s' already in use.\n", user);
}

void Svcmd_Removeaccount_f(void)
{
	char            user[16];

	if(trap_Argc() < 2)
	{
		G_Printf("Usage: removeaccount <user>\n");
		return;
	}

	trap_Argv(1, user, sizeof(user));

	if(Admin_RemoveAccount(user))
		G_Printf("Account '%s' removed.\n", user);
	else
		G_Printf("Username '%s' not found.\n", user);
}

/*
===================
Svcmd_EntityList_f
===================
*/
void Svcmd_EntityList_f(void)
{
	int             e;
	gentity_t      *check;

	check = g_entities + 1;
	for(e = 1; e < level.numEntities; e++, check++)
	{
		if(!check->inuse)
		{
			continue;
		}
		G_Printf("%3i:", e);
		switch (check->s.eType)
		{
			case ET_GENERAL:
				G_Printf("ET_GENERAL          ");
				break;
			case ET_PLAYER:
				G_Printf("ET_PLAYER           ");
				break;
			case ET_ITEM:
				G_Printf("ET_ITEM             ");
				break;
			case ET_MISSILE:
				G_Printf("ET_MISSILE          ");
				break;
			case ET_MOVER:
				G_Printf("ET_MOVER            ");
				break;
			case ET_BEAM:
				G_Printf("ET_BEAM             ");
				break;
			case ET_PORTAL:
				G_Printf("ET_PORTAL           ");
				break;
			case ET_SPEAKER:
				G_Printf("ET_SPEAKER          ");
				break;
			case ET_PUSH_TRIGGER:
				G_Printf("ET_PUSH_TRIGGER     ");
				break;
			case ET_TELEPORT_TRIGGER:
				G_Printf("ET_TELEPORT_TRIGGER ");
				break;
			case ET_INVISIBLE:
				G_Printf("ET_INVISIBLE        ");
				break;
			case ET_GRAPPLE:
				G_Printf("ET_GRAPPLE          ");
				break;
			default:
				G_Printf("%3i                 ", check->s.eType);
				break;
		}

		if(check->classname)
		{
			G_Printf("%s", check->classname);
		}
		G_Printf("\n");
	}
}

gclient_t      *ClientForString(const char *s)
{
	gclient_t      *cl;
	int             i;
	int             idnum;

	// numeric values are just slot numbers
	if(s[0] >= '0' && s[0] <= '9')
	{
		idnum = atoi(s);
		if(idnum < 0 || idnum >= level.maxclients)
		{
			Com_Printf("Bad client slot: %i\n", idnum);
			return NULL;
		}

		cl = &level.clients[idnum];
		if(cl->pers.connected == CON_DISCONNECTED)
		{
			G_Printf("Client %i is not connected\n", idnum);
			return NULL;
		}
		return cl;
	}

	// check for a name match
	for(i = 0; i < level.maxclients; i++)
	{
		cl = &level.clients[i];
		if(cl->pers.connected == CON_DISCONNECTED)
		{
			continue;
		}
		if(!Q_stricmp(cl->pers.netname, s))
		{
			return cl;
		}
	}

	G_Printf("User %s is not on the server\n", s);

	return NULL;
}

/*
===================
Svcmd_ForceTeam_f

forceteam <player> <team>
===================
*/
void Svcmd_ForceTeam_f(void)
{
	gclient_t      *cl;
	char            str[MAX_TOKEN_CHARS];

	// find the player
	trap_Argv(1, str, sizeof(str));
	cl = ClientForString(str);
	if(!cl)
	{
		return;
	}

	// set the team
	trap_Argv(2, str, sizeof(str));
	SetTeam(&g_entities[cl - level.clients], str);
}

char           *ConcatArgs(int start);


/*
=================
Svcmd_LuaRestart_f
=================
*/
#ifdef LUA
static void Svcmd_LuaRestart_f(void)
{
	G_ShutdownLua();
	G_InitLua();
}
#endif


/*
=================
ConsoleCommand
=================
*/
qboolean ConsoleCommand(void)
{
	char            cmd[MAX_TOKEN_CHARS];

	trap_Argv(0, cmd, sizeof(cmd));

	if(Q_stricmp(cmd, "entitylist") == 0)
	{
		Svcmd_EntityList_f();
		return qtrue;
	}

	if(Q_stricmp(cmd, "forceteam") == 0)
	{
		Svcmd_ForceTeam_f();
		return qtrue;
	}

	if(Q_stricmp(cmd, "game_memory") == 0)
	{
		Svcmd_GameMem_f();
		return qtrue;
	}

	if(Q_stricmp(cmd, "addbot") == 0)
	{
		Svcmd_AddBot_f();
		return qtrue;
	}

	if(Q_stricmp(cmd, "botlist") == 0)
	{
		Svcmd_BotList_f();
		return qtrue;
	}

	if(Q_stricmp(cmd, "abort_podium") == 0)
	{
		Svcmd_AbortPodium_f();
		return qtrue;
	}

	if(Q_stricmp(cmd, "addip") == 0)
	{
		Svcmd_AddIP_f();
		return qtrue;
	}

	if(Q_stricmp(cmd, "removeip") == 0)
	{
		Svcmd_RemoveIP_f();
		return qtrue;
	}

	if(Q_stricmp(cmd, "listip") == 0)
	{
		Admin_PrintBans();
		return qtrue;
	}

	if(!Q_stricmp(cmd, "addaccount"))
	{
		Svcmd_Addaccount_f();
		return qtrue;
	}

	if(!Q_stricmp(cmd, "removeaccount"))
	{
		Svcmd_Removeaccount_f();
		return qtrue;
	}

	// brainworks
	if(Q_stricmp(cmd, "ai_debug") == 0)
	{
		BotAIDebug();
		return qtrue;
	}

#ifdef LUA
	if(Q_stricmp(cmd, "restartLuaGameVM") == 0)
	{
		Svcmd_LuaRestart_f();
		return qtrue;
	}
#endif

	if(g_dedicated.integer)
	{
		if(Q_stricmp(cmd, "say") == 0)
		{
			trap_SendServerCommand(-1, va("print \"server: %s\"", ConcatArgs(1)));
			return qtrue;
		}
		// everything else will also be printed as a say command
		trap_SendServerCommand(-1, va("print \"server: %s\"", ConcatArgs(0)));
		return qtrue;
	}

	return qfalse;
}
