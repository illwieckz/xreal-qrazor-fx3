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

#include "server.h"

/*
===============================================================================

OPERATOR CONSOLE ONLY COMMANDS

These commands can only be entered from stdin or by a remote operator datagram
===============================================================================
*/

serverban_t     serverbans = { 0 };

useraccount_t   accounts = { 0 };

qboolean SV_ClientHasPermission(client_t * cl, int permission)
{
	if(cl->account && (cl->account->permissions & (1 << permission)))
		return qtrue;

	return qfalse;
}

qboolean StringIsNumeric(const char *s)
{
	const char     *p;

	if(!s[0])
		return qfalse;

	p = s;

	while(p[0])
	{
		if(!isdigit(p[0]))
			return qfalse;
		p++;
	}

	return qtrue;
}


void SV_ReadAccounts(void)
{
	int             len, i;
	byte           *buff;
	useraccount_t  *data, *u;

	if(!sv_accountfile->string[0])
		return;

	u = &accounts;

	while(u->next)
		u = u->next;

	len = FS_ReadFile(sv_accountfile->string, (void **)&buff);

	if(len == -1)
	{
		Com_Printf("Can't read sv_accountfile %s\n", sv_accountfile->string);
		return;
	}

	if(len % sizeof(useraccount_t))
	{
		Com_Printf("Malformed sv_accountfile %s\n", sv_accountfile->string);
		return;
	}

	i = 0;

	while(len)
	{
		data = (useraccount_t *) buff + i;
		u->next = S_Malloc(sizeof(*u));
		u = u->next;

		u->next = NULL;
		Q_strncpyz(u->username, data->username, sizeof(u->username));
		Q_strncpyz(u->password, data->password, sizeof(u->password));
		u->permissions = data->permissions;
		i++;
		len -= sizeof(useraccount_t);
	}

	FS_FreeFile(buff);

	Com_Printf("Loaded %d accounts from %s\n", i, sv_accountfile->string);
}

void SV_WriteAccounts(void)
{
	fileHandle_t    f;
	useraccount_t  *u;
	int             i;

	if(!sv_accountfile->string[0])
		return;

	f = FS_FOpenFileWrite(sv_accountfile->string);
	if(!f)
	{
		Com_Printf("Couldn't write sv_accountfile %s\n", sv_accountfile->string);
		return;
	}

	u = &accounts;

	i = 0;

	while(u->next)
	{
		u = u->next;
		FS_Write(u, sizeof(*u), f);
		i++;
	}

	FS_FCloseFile(f);
	Com_DPrintf("Wrote %d accounts to %s\n", i, sv_accountfile->string);
}

void SV_AddAccount_f(void)
{
	useraccount_t  *u;

	if(Cmd_Argc() != 4)
	{
		Com_Printf("Usage: addaccount username password permissions\n");
		return;
	}

	u = &accounts;

	while(u->next)
		u = u->next;

	u->next = S_Malloc(sizeof(*u));
	u = u->next;

	u->next = NULL;
	Q_strncpyz(u->username, Cmd_Argv(1), sizeof(u->username));
	Q_strncpyz(u->password, Cmd_Argv(2), sizeof(u->password));
	u->permissions = atoi(Cmd_Argv(3));
	SV_WriteAccounts();
	Com_Printf("User account '%s' added.\n", u->username);
}

useraccount_t  *SV_CheckLogin(const char *username, const char *password)
{
	useraccount_t  *u;

	u = &accounts;

	while(u->next)
	{
		u = u->next;
		if(!strcmp(u->username, username) && !strcmp(u->password, password))
			return u;
	}

	return NULL;
}

void SV_RemoveAccount_f(void)
{
	useraccount_t  *u, *last;

	if(Cmd_Argc() != 2)
	{
		Com_Printf("Usage: removeaccount username\n");
		return;
	}

	u = last = &accounts;

	while(u->next)
	{
		u = u->next;
		if(!Q_stricmp(Cmd_Argv(1), u->username))
		{
			last->next = u->next;
			Com_Printf("User account '%s' removed.\n", u->username);
			Z_Free(u);
			SV_WriteAccounts();
			return;
		}
	}

	Com_Printf("User account '%s' not found.\n", Cmd_Argv(1));
}

void SV_ReadBans(void)
{
	int             len, i;
	byte           *buff;
	serverban_t    *data, *s;

	if(!sv_banfile->string[0])
		return;

	s = &serverbans;

	while(s->next)
		s = s->next;

	len = FS_ReadFile(sv_banfile->string, (void **)&buff);

	if(len == -1)
	{
		Com_Printf("Can't read sv_banfile %s\n", sv_banfile->string);
		return;
	}

	if(len % sizeof(serverban_t))
	{
		Com_Printf("Malformed sv_banfile %s\n", sv_banfile->string);
		return;
	}

	i = 0;

	while(len)
	{
		data = (serverban_t *) buff + i;
		s->next = S_Malloc(sizeof(*s));
		s = s->next;

		s->next = NULL;
		s->ip = data->ip;
		s->mask = data->mask;
		s->expiretime = data->expiretime;
		Q_strncpyz(s->reason, data->reason, sizeof(s->reason));
		i++;
		len -= sizeof(serverban_t);
	}

	FS_FreeFile(buff);

	Com_Printf("Loaded %d IP bans from %s\n", i, sv_banfile->string);
}

void SV_WriteBans(void)
{
	fileHandle_t    f;
	serverban_t    *s;
	int             i;

	if(!sv_banfile->string[0])
		return;

	f = FS_FOpenFileWrite(sv_banfile->string);
	if(!f)
	{
		Com_Printf("Couldn't write sv_banfile %s\n", sv_banfile->string);
		return;
	}

	s = &serverbans;

	i = 0;

	while(s->next)
	{
		s = s->next;
		FS_Write(s, sizeof(*s), f);
		i++;
	}

	FS_FCloseFile(f);
	Com_DPrintf("Wrote %d IP bans to %s\n", i, sv_banfile->string);
}

static void SV_Ban(unsigned int ip, unsigned int duration, unsigned int mask, const char *reason)
{
	serverban_t    *s;

	s = &serverbans;

	while(s->next)
		s = s->next;

	s->next = S_Malloc(sizeof(*s));
	s = s->next;

	s->next = NULL;
	s->ip = ip;
	s->mask = mask;
	if(duration == -1)
		s->expiretime = 0;
	else
		s->expiretime = time(NULL) + duration;
	Q_strncpyz(s->reason, reason, sizeof(s->reason));

	SV_WriteBans();
}

void SV_BanClient(client_t * cl, unsigned int duration, unsigned int mask, const char *reason)
{
	SV_Ban(*(unsigned int *)cl->netchan.remoteAddress.ip, duration, mask, reason);
}

serverban_t    *SV_BanMatch(netadr_t * adr)
{
	serverban_t    *s, *last;
	int             now;

	s = last = &serverbans;

	now = time(NULL);

	while(s->next)
	{
		s = s->next;

		if(s->expiretime && s->expiretime < now)
		{
			Com_Printf("Expiring IP ban %s.\n", NET_inet_ntoa(s->ip));
			last->next = s->next;
			Z_Free(s);
			continue;
		}

		if(((*(unsigned int *)adr->ip) & s->mask) == (s->ip & s->mask))
			return s;
	}

	return NULL;
}

/*
==================
SV_GetPlayerByName

Returns the player with player name from Cmd_Argv(1)
==================
*/
client_t       *SV_GetPlayerByName(void)
{
	client_t       *cl;
	int             i;
	char           *s;
	char            cleanName[64];

	// make sure server is running
	if(!com_sv_running->integer)
	{
		return NULL;
	}

	if(Cmd_Argc() < 2)
	{
		Com_Printf("No player specified.\n");
		return NULL;
	}

	s = Cmd_Argv(1);

	// check for a name match
	for(i = 0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++)
	{
		if(!cl->state)
		{
			continue;
		}
		if(!Q_stricmp(cl->name, s))
		{
			return cl;
		}

		Q_strncpyz(cleanName, cl->name, sizeof(cleanName));
		Q_CleanStr(cleanName);
		if(!Q_stricmp(cleanName, s))
		{
			return cl;
		}
	}

	// r1: substring matching
	if(sv_enhanced_getplayer->integer)
	{
		Q_strlwr(s);
		for(i = 0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++)
		{
			if(!cl->state)
				continue;
			if(strstr(cl->name, s))
				return cl;

			Q_strncpyz(cleanName, cl->name, sizeof(cleanName));
			Q_CleanStr(cleanName);
			Q_strlwr(cleanName);

			if(strstr(cleanName, s))
				return cl;
		}
	}

	Com_Printf("Player %s is not on the server\n", s);

	return NULL;
}

/*
==================
SV_GetPlayerByNum

Returns the player with idnum from Cmd_Argv(1)
==================
*/
client_t       *SV_GetPlayerByNum(void)
{
	client_t       *cl;
	int             i;
	int             idnum;
	char           *s;

	// make sure server is running
	if(!com_sv_running->integer)
	{
		return NULL;
	}

	if(Cmd_Argc() < 2)
	{
		Com_Printf("No player specified.\n");
		return NULL;
	}

	s = Cmd_Argv(1);

	for(i = 0; s[i]; i++)
	{
		if(s[i] < '0' || s[i] > '9')
		{
			Com_Printf("Bad slot number: %s\n", s);
			return NULL;
		}
	}
	idnum = atoi(s);
	if(idnum < 0 || idnum >= sv_maxclients->integer)
	{
		Com_Printf("Bad client slot: %i\n", idnum);
		return NULL;
	}

	cl = &svs.clients[idnum];
	if(!cl->state)
	{
		Com_Printf("Client %i is not active\n", idnum);
		return NULL;
	}
	return cl;
}

//=========================================================

/*
==================
SV_Map_f

Restart the server on a different map
==================
*/
static void SV_Map_f(void)
{
	char           *cmd;
	char           *map;
	qboolean        killBots, cheat;
	char            expanded[MAX_QPATH];
	char            mapname[MAX_QPATH];
	int             i;

	map = Cmd_Argv(1);
	if(!map)
	{
		return;
	}

	// make sure the level exists before trying to change, so that
	// a typo at the server console won't end the game
	Com_sprintf(expanded, sizeof(expanded), "maps/%s.bsp", map);
	if(FS_ReadFile(expanded, NULL) == -1)
	{
		Com_Printf("Can't find map %s\n", expanded);
		return;
	}

	// force latched values to get set
	Cvar_Get("g_gametype", "0", CVAR_SERVERINFO | CVAR_USERINFO | CVAR_LATCH);

	cmd = Cmd_Argv(0);
	if(Q_stricmpn(cmd, "sp", 2) == 0)
	{
		Cvar_SetValue("g_gametype", GT_SINGLE_PLAYER);
		Cvar_SetValue("g_doWarmup", 0);
		// may not set sv_maxclients directly, always set latched
		Cvar_SetLatched("sv_maxclients", "8");
		cmd += 2;
		cheat = qfalse;
		killBots = qtrue;
	}
	else
	{
		if(!Q_stricmp(cmd, "devmap") || !Q_stricmp(cmd, "spdevmap"))
		{
			cheat = qtrue;
			killBots = qtrue;
		}
		else
		{
			cheat = qfalse;
			killBots = qfalse;
		}
		if(sv_gametype->integer == GT_SINGLE_PLAYER)
		{
			Cvar_SetValue("g_gametype", GT_FFA);
		}
	}

	// save the map name here cause on a map restart we reload the xreal.cfg
	// and thus nuke the arguments of the map command
	Q_strncpyz(mapname, map, sizeof(mapname));

	// start up the map
	SV_SpawnServer(mapname, killBots);

	// set the cheat value
	// if the level was started with "map <levelname>", then
	// cheats will not be allowed.  If started with "devmap <levelname>"
	// then cheats will be allowed
	if(cheat)
	{
		Cvar_Set("sv_cheats", "1");
	}
	else
	{
		Cvar_Set("sv_cheats", "0");
	}

	// This forces the local master server IP address cache
	// to be updated on sending the next heartbeat
	for(i = 0; i < MAX_MASTER_SERVERS; i++)
		sv_master[i]->modified = qtrue;
}

/*
================
SV_MapRestart_f

Completely restarts a level, but doesn't send a new gamestate to the clients.
This allows fair starts with variable load times.
================
*/
static void SV_MapRestart_f(void)
{
	int             i;
	client_t       *client;
	char           *denied;
	qboolean        isBot;
	int             delay;

	// make sure we aren't restarting twice in the same frame
	if(com_frameTime == sv.serverId)
	{
		return;
	}

	// make sure server is running
	if(!com_sv_running->integer)
	{
		Com_Printf("Server is not running.\n");
		return;
	}

	if(sv.restartTime)
	{
		return;
	}

	if(Cmd_Argc() > 1)
	{
		delay = atoi(Cmd_Argv(1));
	}
	else
	{
		delay = 5;
	}
	if(delay && !Cvar_VariableValue("g_doWarmup"))
	{
		sv.restartTime = sv.time + delay * 1000;
		SV_SetConfigstring(CS_WARMUP, va("%i", sv.restartTime));
		return;
	}

	// check for changes in variables that can't just be restarted
	// check for maxclients change
	if(sv_maxclients->modified || sv_gametype->modified)
	{
		char            mapname[MAX_QPATH];

		Com_Printf("variable change -- restarting.\n");
		// restart the map the slow way
		Q_strncpyz(mapname, Cvar_VariableString("mapname"), sizeof(mapname));

		SV_SpawnServer(mapname, qfalse);
		return;
	}

	// toggle the server bit so clients can detect that a
	// map_restart has happened
	svs.snapFlagServerBit ^= SNAPFLAG_SERVERCOUNT;

	// generate a new serverid  
	// TTimo - don't update restartedserverId there, otherwise we won't deal correctly with multiple map_restart
	sv.serverId = com_frameTime;
	Cvar_Set("sv_serverid", va("%i", sv.serverId));

	// reset all the vm data in place without changing memory allocation
	// note that we do NOT set sv.state = SS_LOADING, so configstrings that
	// had been changed from their default values will generate broadcast updates
	sv.state = SS_LOADING;
	sv.restarting = qtrue;

	SV_RestartGameProgs();

	// run a few frames to allow everything to settle
	for(i = 0; i < 3; i++)
	{
		VM_Call(gvm, GAME_RUN_FRAME, sv.time);
		sv.time += 100;
		svs.time += 100;
	}

	sv.state = SS_GAME;
	sv.restarting = qfalse;

	// connect and begin all the clients
	for(i = 0; i < sv_maxclients->integer; i++)
	{
		client = &svs.clients[i];

		// send the new gamestate to all connected clients
		if(client->state < CS_CONNECTED)
		{
			continue;
		}

		if(client->netchan.remoteAddress.type == NA_BOT)
		{
			isBot = qtrue;
		}
		else
		{
			isBot = qfalse;
		}

		// add the map_restart command
		SV_AddServerCommand(client, "map_restart\n");

		// connect the client again, without the firstTime flag
		denied = VM_ExplicitArgPtr(gvm, VM_Call(gvm, GAME_CLIENT_CONNECT, i, qfalse, isBot));
		if(denied)
		{
			// this generally shouldn't happen, because the client
			// was connected before the level change
			SV_DropClient(client, denied);
			Com_Printf("SV_MapRestart_f(%d): dropped client %i - denied!\n", delay, i);	// bk010125
			continue;
		}

		client->state = CS_ACTIVE;

		SV_ClientEnterWorld(client, &client->lastUsercmd);
	}

	// run another frame to allow things to look at all the players
	VM_Call(gvm, GAME_RUN_FRAME, sv.time);
	sv.time += 100;
	svs.time += 100;
}

//===============================================================

/*
==================
SV_Kick_f

Kick a user off of the server  FIXME: move to game
==================
*/
static void SV_Kick_f(void)
{
	client_t       *cl;

	// make sure server is running
	if(!com_sv_running->integer)
	{
		Com_Printf("Server is not running.\n");
		return;
	}

	if(Cmd_Argc() != 2)
	{
		Com_Printf("Usage: kick <player name/id>\nkick all = kick everyone\nkick allbots = kick all bots\n");
		return;
	}

	//r1: support both id/name here.
	if(StringIsNumeric(Cmd_Argv(1)))
	{
		cl = SV_GetPlayerByNum();
	}
	else
	{
		cl = SV_GetPlayerByName();
	}

	if(!cl)
	{
		/*if ( !Q_stricmp(Cmd_Argv(1), "all") ) {
		   for ( i=0, cl=svs.clients ; i < sv_maxclients->integer ; i++,cl++ ) {
		   if ( !cl->state ) {
		   continue;
		   }
		   if( cl->netchan.remoteAddress.type == NA_LOOPBACK ) {
		   continue;
		   }
		   SV_DropClient( cl, "was kicked" );
		   cl->lastPacketTime = svs.time;   // in case there is a funny zombie
		   }
		   }
		   else if ( !Q_stricmp(Cmd_Argv(1), "allbots") ) {
		   for ( i=0, cl=svs.clients ; i < sv_maxclients->integer ; i++,cl++ ) {
		   if ( !cl->state ) {
		   continue;
		   }
		   if( cl->netchan.remoteAddress.type != NA_BOT ) {
		   continue;
		   }
		   SV_DropClient( cl, "was kicked" );
		   cl->lastPacketTime = svs.time;   // in case there is a funny zombie
		   }
		   } */
		return;
	}

	if(cl->netchan.remoteAddress.type == NA_LOOPBACK)
	{
		Com_Printf("Cannot kick host player.\n");
		return;
	}

	SV_DropClient(cl, "was kicked");
	cl->lastPacketTime = svs.time;	// in case there is a funny zombie
}

unsigned int CalcMask(int bits)
{
	int             i;
	unsigned int    mask;

	mask = 0xFFFFFFFF;

	for(i = 0; i < 32; i++)
	{
		if(i >= bits)
			mask &= ~(1 << i);
	}

	return mask;
}

static void SV_AddBan_f(void)
{
	int             mask, duration, x;
	unsigned        realmask;
	netadr_t        adr;
	const char     *reason;
	char           *ip, *p, *unit;

	if(Cmd_Argc() < 4)
	{
		Com_Printf("Usage: addban ip[/bitmask] time unit [reason]\n");
		return;
	}

	ip = Cmd_Argv(1);
	p = strchr(ip, '/');
	if(p)
	{
		p[0] = 0;
		p++;
		mask = atoi(p);
	}
	else
		mask = 32;

	if(mask <= 0 || mask > 32)
	{
		Com_Printf("Invalid ban mask %d, must be 1-32\n", mask);
		return;
	}

	if(!NET_StringToAdr(ip, &adr))
	{
		Com_Printf("Bad address %s\n", Cmd_Argv(1));
		return;
	}

	duration = atoi(Cmd_Argv(2));
	if(!duration)
	{
		Com_Printf("Must specify ban time (use -1 for permanent).\n");
		return;
	}

	if(duration > 0)
	{
		unit = Cmd_Argv(3);
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
			Com_Printf("Unit must be one of: secs, mins, hours, days, weeks, years\n");
			return;
		}
		x = 4;
	}
	else
		x = 3;

	realmask = CalcMask(mask);
	reason = Cmd_ArgsFrom(x);

	if(!reason[0])
		reason = "unspecified";

	SV_Ban(*(unsigned int *)&adr.ip, duration, realmask, reason);
	Com_Printf("%s/%d added to IP banlist.\n", NET_inet_ntoa(*(unsigned int *)&adr.ip), MaskBits(realmask));
}

static void SV_RemoveBan_f(void)
{
	netadr_t        adr;
	unsigned        mask, realmask;
	serverban_t    *last, *s;
	char           *ip, *p;

	if(Cmd_Argc() != 2)
	{
		Com_Printf("Usage: removeban ip[/mask]\n");
		return;
	}

	ip = Cmd_Argv(1);
	p = strchr(ip, '/');
	if(p)
	{
		p[0] = 0;
		p++;
		mask = atoi(p);
	}
	else
		mask = 32;

	realmask = CalcMask(mask);

	if(!NET_StringToAdr(ip, &adr))
	{
		Com_Printf("Bad address %s\n", Cmd_Argv(1));
		return;
	}

	s = last = &serverbans;

	while(s->next)
	{
		s = s->next;
		if(s->ip == *(unsigned int *)&adr.ip && s->mask == realmask)
		{
			last->next = s->next;
			Com_Printf("Removed %s/%d from IP banlist.\n", NET_inet_ntoa(s->ip), mask);
			Z_Free(s);
			SV_WriteBans();
			return;
		}
		last = s;
	}

	Com_Printf("Could not find ban for %s.\n", Cmd_Argv(1));
}

static void SV_ListBans_f(void)
{
	const char     *unit;
	int             mask, timeleft;
	serverban_t    *s;

	s = &serverbans;

	Com_Printf("+------------------+---------+---------------------------------+\n");
	Com_Printf("| IP Address / Mask| Expires |    Reason for ban               |\n");
	Com_Printf("+------------------+---------+---------------------------------+\n");

	while(s->next)
	{
		s = s->next;
		mask = MaskBits(s->mask);

		if(s->expiretime)
		{
			timeleft = s->expiretime - time(NULL);
			if(timeleft >= 60 * 60 * 24 * 365.25)
			{
				timeleft = (int)(timeleft / (60 * 60 * 24 * 365.25));
				unit = "years";
			}
			else if(timeleft >= 60 * 60 * 24 * 7)
			{
				timeleft = (int)(timeleft / (60 * 60 * 24 * 7));
				unit = "weeks";
			}
			else if(timeleft >= 60 * 60 * 24)
			{
				timeleft = (int)(timeleft / (60 * 60 * 24));
				unit = "days ";
			}
			else if(timeleft >= 60 * 60)
			{
				timeleft = (int)(timeleft / (60 * 60));
				unit = "hours";
			}
			else if(timeleft >= 60)
			{
				timeleft = (int)(timeleft / (60));
				unit = "mins ";
			}
			else
			{
				unit = "secs ";
			}

			timeleft++;

			Com_Printf("|%-15.15s/%2d|%2d %s |%-33.33s|\n", NET_inet_ntoa(s->ip), mask, timeleft, unit, s->reason);
		}
		else
		{
			Com_Printf("|%-15.15s/%2d|  never  |%-33.33s|\n", NET_inet_ntoa(s->ip), mask, s->reason);
		}
	}

	Com_Printf("+------------------+---------+---------------------------------+\n");
}

static void SV_Ban_f(void)
{
	int             duration, x;
	const char     *unit, *reason;
	client_t       *cl;

	if(!com_sv_running->integer)
	{
		Com_Printf("Server is not running.\n");
		return;
	}

	if(Cmd_Argc() < 4)
	{
		Com_Printf("Usage: ban name/id time unit [reason]\n");
		return;
	}

	if(StringIsNumeric(Cmd_Argv(1)))
	{
		cl = SV_GetPlayerByNum();
	}
	else
	{
		cl = SV_GetPlayerByName();
	}

	if(!cl)
		return;

	duration = atoi(Cmd_Argv(2));
	if(!duration)
	{
		Com_Printf("Must specify ban time (use -1 for permanent).\n");
		return;
	}

	if(duration > 0)
	{
		unit = Cmd_Argv(3);
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
			Com_Printf("Unit must be one of: secs, mins, hours, days, weeks, years\n");
			return;
		}
		x = 4;
	}
	else
		x = 3;

	reason = Cmd_ArgsFrom(x);
	if(!reason[0])
		reason = "unspecified";

	Com_Printf("%s[%s] was banned.\n", cl->name, NET_AdrToString(cl->netchan.remoteAddress));

	SV_BanClient(cl, duration, CalcMask(24), reason);
	SV_DropClient(cl, "was banned");
}

/*
==================
SV_KickNum_f

Kick a user off of the server  FIXME: move to game
==================
*/
static void SV_KickNum_f(void)
{
	client_t       *cl;

	// make sure server is running
	if(!com_sv_running->integer)
	{
		Com_Printf("Server is not running.\n");
		return;
	}

	if(Cmd_Argc() != 2)
	{
		Com_Printf("Usage: kicknum <client number>\n");
		return;
	}

	cl = SV_GetPlayerByNum();
	if(!cl)
	{
		return;
	}
	if(cl->netchan.remoteAddress.type == NA_LOOPBACK)
	{
		SV_SendServerCommand(NULL, "print \"%s\"", "Cannot kick host player\n");
		return;
	}

	SV_DropClient(cl, "was kicked");
	cl->lastPacketTime = svs.time;	// in case there is a funny zombie
}

/*
================
SV_Status_f
================
*/
static void SV_Status_f(void)
{
	int             i, j, l;
	client_t       *cl;
	playerState_t  *ps;
	const char     *s;
	int             ping;

	// make sure server is running
	if(!com_sv_running->integer)
	{
		Com_Printf("Server is not running.\n");
		return;
	}

	Com_Printf("map: %s\n", sv_mapname->string);

	Com_Printf("num score ping name            lastmsg address               qport rate\n");
	Com_Printf("--- ----- ---- --------------- ------- --------------------- ----- -----\n");
	for(i = 0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++)
	{
		if(!cl->state)
			continue;
		Com_Printf("%3i ", i);
		ps = SV_GameClientNum(i);
		Com_Printf("%5i ", ps->persistant[PERS_SCORE]);

		if(cl->state == CS_CONNECTED)
			Com_Printf("CNCT ");
		else if(cl->state == CS_ZOMBIE)
			Com_Printf("ZMBI ");
		else
		{
			ping = cl->ping < 9999 ? cl->ping : 9999;
			Com_Printf("%4i ", ping);
		}

		Com_Printf("%s", cl->name);
		// TTimo adding a ^7 to reset the color
		// NOTE: colored names in status breaks the padding (WONTFIX)
		Com_Printf("^7");
		l = 16 - strlen(cl->name);
		for(j = 0; j < l; j++)
			Com_Printf(" ");

		Com_Printf("%7i ", svs.time - cl->lastPacketTime);

		s = NET_AdrToString(cl->netchan.remoteAddress);
		Com_Printf("%s", s);
		l = 22 - strlen(s);
		for(j = 0; j < l; j++)
			Com_Printf(" ");

		Com_Printf("%5i", cl->netchan.qport);

		Com_Printf(" %5i", cl->rate);

		Com_Printf("\n");
	}
	Com_Printf("\n");
}

/*
==================
SV_ConSay_f
==================
*/
static void SV_ConSay_f(void)
{
	char           *p;
	char            text[1024];

	// make sure server is running
	if(!com_sv_running->integer)
	{
		Com_Printf("Server is not running.\n");
		return;
	}

	if(Cmd_Argc() < 2)
	{
		return;
	}

	strcpy(text, "console: ");
	p = Cmd_Args();

	if(*p == '"')
	{
		p++;
		p[strlen(p) - 1] = 0;
	}

	strcat(text, p);

	SV_SendServerCommand(NULL, "chat \"%s\n\"", text);
}


/*
==================
SV_Heartbeat_f

Also called by SV_DropClient, SV_DirectConnect, and SV_SpawnServer
==================
*/
void SV_Heartbeat_f(void)
{
	svs.nextHeartbeatTime = -9999999;
}


/*
===========
SV_Serverinfo_f

Examine the serverinfo string
===========
*/
static void SV_Serverinfo_f(void)
{
	Com_Printf("Server info settings:\n");
	Info_Print(Cvar_InfoString(CVAR_SERVERINFO));
}


/*
===========
SV_Systeminfo_f

Examine or change the serverinfo string
===========
*/
static void SV_Systeminfo_f(void)
{
	Com_Printf("System info settings:\n");
	Info_Print(Cvar_InfoString(CVAR_SYSTEMINFO));
}


/*
===========
SV_DumpUser_f

Examine all a users info strings FIXME: move to game
===========
*/
static void SV_DumpUser_f(void)
{
	client_t       *cl;

	// make sure server is running
	if(!com_sv_running->integer)
	{
		Com_Printf("Server is not running.\n");
		return;
	}

	if(Cmd_Argc() != 2)
	{
		Com_Printf("Usage: info <userid>\n");
		return;
	}

	cl = SV_GetPlayerByName();
	if(!cl)
	{
		return;
	}

	Com_Printf("userinfo\n");
	Com_Printf("--------\n");
	Info_Print(cl->userinfo);
}


/*
=================
SV_KillServer
=================
*/
static void SV_KillServer_f(void)
{
	SV_Shutdown("killserver");
}

//===========================================================

/*
==================
SV_AddOperatorCommands
==================
*/
void SV_AddOperatorCommands(void)
{
	static qboolean initialized;

	if(initialized)
	{
		return;
	}
	initialized = qtrue;

	Cmd_AddCommand("heartbeat", SV_Heartbeat_f);
	Cmd_AddCommand("kick", SV_Kick_f);
	Cmd_AddCommand("addban", SV_AddBan_f);
	Cmd_AddCommand("removeban", SV_RemoveBan_f);
	Cmd_AddCommand("listbans", SV_ListBans_f);
	Cmd_AddCommand("addaccount", SV_AddAccount_f);
	Cmd_AddCommand("removeaccount", SV_RemoveAccount_f);
	Cmd_AddCommand("ban", SV_Ban_f);
	Cmd_AddCommand("clientkick", SV_KickNum_f);
	Cmd_AddCommand("status", SV_Status_f);
	Cmd_AddCommand("serverinfo", SV_Serverinfo_f);
	Cmd_AddCommand("systeminfo", SV_Systeminfo_f);
	Cmd_AddCommand("dumpuser", SV_DumpUser_f);
	Cmd_AddCommand("map_restart", SV_MapRestart_f);
	Cmd_AddCommand("sectorlist", SV_SectorList_f);
	Cmd_AddCommand("map", SV_Map_f);
	Cmd_AddCommand("devmap", SV_Map_f);
	Cmd_AddCommand("killserver", SV_KillServer_f);
	if(com_dedicated->integer)
	{
		Cmd_AddCommand("say", SV_ConSay_f);
	}
}

/*
==================
SV_RemoveOperatorCommands
==================
*/
void SV_RemoveOperatorCommands(void)
{
#if 0
	// removing these won't let the server start again
	Cmd_RemoveCommand("heartbeat");
	Cmd_RemoveCommand("kick");
	Cmd_RemoveCommand("status");
	Cmd_RemoveCommand("serverinfo");
	Cmd_RemoveCommand("systeminfo");
	Cmd_RemoveCommand("dumpuser");
	Cmd_RemoveCommand("map_restart");
	Cmd_RemoveCommand("sectorlist");
	Cmd_RemoveCommand("say");
#endif
}
