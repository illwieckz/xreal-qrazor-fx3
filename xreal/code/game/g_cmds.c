/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006 Robert Beckebans <trebor_7@users.sourceforge.net>
Copyright (C) 2007 Jeremy Hughes <Encryption767@msn.com>

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
#include "g_local.h"

#include "../ui/menudef.h"		// for the voice chats

/*
==================
DeathmatchScoreboardMessage

==================
*/
void DeathmatchScoreboardMessage(gentity_t * ent)
{
	char            entry[1024];
	char            string[1400];
	int             stringlength;
	int             i, j;
	gclient_t      *cl;
	int             numSorted, scoreFlags;	//, accuracy, perfect;

	// send the latest information on all clients
	string[0] = 0;
	stringlength = 0;
	scoreFlags = 0;

	numSorted = level.numConnectedClients;

	for(i = 0; i < numSorted; i++)
	{
		int             ping;

		cl = &level.clients[level.sortedClients[i]];

		if(cl->pers.connected == CON_CONNECTING)
		{
			ping = -1;
		}
		else
		{
//unlagged - true ping
			//ping = cl->ps.ping < 999 ? cl->ps.ping : 999;
			ping = cl->pers.realPing < 999 ? cl->pers.realPing : 999;
//unlagged - true ping
		}

		//  if( cl->accuracy_shots ) {
		//      accuracy = cl->accuracy_hits * 100 / cl->accuracy_shots;
		//  }
		//  else {
		//      accuracy = 0;
		//  }
		/// perfect = ( cl->ps.persistant[PERS_RANK] == 0 && cl->ps.persistant[PERS_KILLED] == 0 ) ? 1 : 0;

		Com_sprintf(entry, sizeof(entry),
					" %i %i %i %i %i %i %i %i %i %i %i %i %i", level.sortedClients[i],
					cl->ps.persistant[PERS_SCORE], ping, (level.time - cl->pers.enterTime) / 60000,
					scoreFlags, g_entities[level.sortedClients[i]].s.powerups,
					cl->ps.persistant[PERS_IMPRESSIVE_COUNT],
					cl->ps.persistant[PERS_EXCELLENT_COUNT],
					cl->ps.persistant[PERS_GAUNTLET_FRAG_COUNT],
					cl->ps.persistant[PERS_DEFEND_COUNT],
					cl->ps.persistant[PERS_ASSIST_COUNT], cl->ps.persistant[PERS_CAPTURES], cl->ps.persistant[PERS_REF]);
		j = strlen(entry);
		if(stringlength + j > 1024)
			break;
		strcpy(string + stringlength, entry);
		stringlength += j;
	}

	trap_SendServerCommand(ent - g_entities, va("scores %i %i %i%s", i,
												level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE], string));
}

static void Playerinfo(gentity_t * ent)
{
	char            entry[1024];
	char            string[1400];
	int             stringlength;
	int             i, j;
	gclient_t      *cl;
	int             numSorted;
	char           *s;

	// send the latest information on all clients
	string[0] = 0;
	stringlength = 0;

	numSorted = level.numConnectedClients;
	trap_SendServerCommand(ent - g_entities, "printc \"^4---------------------------------------------- \n\"");
	trap_SendServerCommand(ent - g_entities, "printc \"^3NUM      Ping                 Name       \n\"");
	trap_SendServerCommand(ent - g_entities, "printc \"^4---------------------------------------------- \n\"");

	for(i = 0; i < numSorted; i++)
	{
		int             ping;

		cl = &level.clients[level.sortedClients[i]];

		if(cl->pers.connected == CON_CONNECTING)
		{
			ping = -1;
		}
		else
		{
			ping = cl->pers.realPing < 999 ? cl->pers.realPing : 999;
		}

		s = va("printc \" %i         %i             %s\n\"", cl->ps.clientNum, ping, Q_CleanAbsoluteColorStr(cl->pers.netname));

		trap_SendServerCommand(ent - g_entities, s);


		j = strlen(entry);
		if(stringlength + j > 1024)
			break;
		strcpy(string + stringlength, entry);
		stringlength += j;
	}
	trap_SendServerCommand(ent - g_entities, "printc \"^4---------------------------------------------- \n\"");

}


/*
==================
Cmd_Score_f

Request current scoreboard information
==================
*/
void Cmd_Score_f(gentity_t * ent)
{
	DeathmatchScoreboardMessage(ent);
}



/*
==================
CheatsOk
==================
*/
static qboolean CheatsOk(gentity_t * ent)
{
	if(!g_cheats.integer)
	{
		trap_SendServerCommand(ent - g_entities, va("printc \"Cheats are not enabled on this server."));
		return qfalse;
	}
	if(ent->health <= 0)
	{
		trap_SendServerCommand(ent - g_entities, va("printc \"You must be alive to use this command."));
		return qfalse;
	}
	return qtrue;
}


/*
==================
ConcatArgs
==================
*/
char           *ConcatArgs(int start)
{
	int             i, c, tlen;
	static char     line[MAX_STRING_CHARS];
	int             len;
	char            arg[MAX_STRING_CHARS];

	len = 0;
	c = trap_Argc();
	for(i = start; i < c; i++)
	{
		trap_Argv(i, arg, sizeof(arg));
		tlen = strlen(arg);
		if(len + tlen >= MAX_STRING_CHARS - 1)
		{
			break;
		}
		memcpy(line + len, arg, tlen);
		len += tlen;
		if(i != c - 1)
		{
			line[len] = ' ';
			len++;
		}
	}

	line[len] = 0;

	return line;
}

/*
==================
SanitizeString

Remove case and control characters
==================
*/
void SanitizeString(char *in, char *out)
{
	while(*in)
	{
		if(*in == 27)
		{
			in += 2;			// skip color code
			continue;
		}
		if(*in < 32)
		{
			in++;
			continue;
		}
		*out++ = tolower(*in++);
	}

	*out = 0;
}


/*
==================
ClientNumberFromString

Returns a player number for either a number or name string
Returns -1 if invalid
this is without the gentity for just checking the number
without anything being sent to the client
==================
*/
static int ClientNumberFromString2(char *s)
{
	gclient_t      *cl;
	int             idnum;
	char            s2[MAX_STRING_CHARS];
	char            n2[MAX_STRING_CHARS];

	// numeric values are just slot numbers
	if(s[0] >= '0' && s[0] <= '9')
	{
		idnum = atoi(s);
		if(idnum < 0 || idnum >= level.maxclients)
		{
			return -1;
		}

		cl = &level.clients[idnum];

		if(cl->pers.connected != CON_CONNECTED)
		{
			return -1;
		}

		return idnum;
	}

	// check for a name match
	SanitizeString(s, s2);
	for(idnum = 0, cl = level.clients; idnum < level.maxclients; idnum++, cl++)
	{
		if(cl->pers.connected != CON_CONNECTED)
		{
			continue;
		}
		SanitizeString(cl->pers.netname, n2);
		if(!strcmp(n2, s2))
		{
			return idnum;
		}
	}

	return -1;
}

static int ClientNumberFromString(gentity_t * to, char *s)
{
	gclient_t      *cl;
	int             idnum;
	char            s2[MAX_STRING_CHARS];
	char            n2[MAX_STRING_CHARS];

	// numeric values are just slot numbers
	if(s[0] >= '0' && s[0] <= '9')
	{
		idnum = atoi(s);
		if(idnum < 0 || idnum >= level.maxclients)
		{
			trap_SendServerCommand(to - g_entities, va("printc \"Bad client slot: %i", idnum));
			return -1;
		}

		cl = &level.clients[idnum];

		if(cl->pers.connected != CON_CONNECTED)
		{
			trap_SendServerCommand(to - g_entities, va("printc \"Client %i is not active", idnum));
			return -1;
		}

		return idnum;
	}

	// check for a name match
	SanitizeString(s, s2);
	for(idnum = 0, cl = level.clients; idnum < level.maxclients; idnum++, cl++)
	{
		if(cl->pers.connected != CON_CONNECTED)
		{
			continue;
		}
		SanitizeString(cl->pers.netname, n2);
		if(!strcmp(n2, s2))
		{
			return idnum;
		}
	}

	trap_SendServerCommand(to - g_entities, va("printc \"User %s is not on the server", s));
	return -1;
}

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
static void Cmd_Give_f(gentity_t * ent)
{
	char           *name;
	gitem_t        *it;
	int             i;
	qboolean        give_all;
	gentity_t      *it_ent;
	trace_t         trace;

	if(!CheatsOk(ent))
	{
		return;
	}

	name = ConcatArgs(1);

	if(Q_stricmp(name, "all") == 0)
		give_all = qtrue;
	else
		give_all = qfalse;

	if(give_all || Q_stricmp(name, "health") == 0)
	{
		ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];
		if(!give_all)
			return;
	}

	if(give_all || Q_stricmp(name, "weapons") == 0)
	{
		ent->client->ps.stats[STAT_WEAPONS] = (1 << WP_NUM_WEAPONS) - 1 - (1 << WP_GRAPPLING_HOOK) - (1 << WP_NONE);
		if(!give_all)
			return;
	}

	if(give_all || Q_stricmp(name, "ammo") == 0)
	{
		for(i = 0; i < MAX_WEAPONS; i++)
		{
			ent->client->ps.ammo[i] = 999;
		}
		if(!give_all)
			return;
	}

	if(give_all || Q_stricmp(name, "armor") == 0)
	{
		ent->client->ps.stats[STAT_ARMOR] = 200;

		if(!give_all)
			return;
	}

	if(Q_stricmp(name, "excellent") == 0)
	{
		ent->client->ps.persistant[PERS_EXCELLENT_COUNT]++;
		return;
	}
	if(Q_stricmp(name, "impressive") == 0)
	{
		ent->client->ps.persistant[PERS_IMPRESSIVE_COUNT]++;
		return;
	}
/*	if (Q_stricmp(name, "rocketrailcombo") == 0) {
		ent->client->ps.persistant[PERS_RLRGCOMBO_COUNT]++;
		return;
	}*/
	if(Q_stricmp(name, "gauntletaward") == 0)
	{
		ent->client->ps.persistant[PERS_GAUNTLET_FRAG_COUNT]++;
		return;
	}
	if(Q_stricmp(name, "defend") == 0)
	{
		ent->client->ps.persistant[PERS_DEFEND_COUNT]++;
		return;
	}
	if(Q_stricmp(name, "assist") == 0)
	{
		ent->client->ps.persistant[PERS_ASSIST_COUNT]++;
		return;
	}

	// spawn a specific item right on the player
	if(!give_all)
	{
		it = BG_FindItem(name);
		if(!it)
		{
			return;
		}

		it_ent = G_Spawn();
		VectorCopy(ent->r.currentOrigin, it_ent->s.origin);
		it_ent->classname = it->classname;
		G_SpawnItem(it_ent, it);
		FinishSpawningItem(it_ent);
		memset(&trace, 0, sizeof(trace));
		Touch_Item(it_ent, ent, &trace);
		if(it_ent->inuse)
		{
			G_FreeEntity(it_ent);
		}
	}
}


/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
static void Cmd_God_f(gentity_t * ent)
{
	char           *msg;

	if(!CheatsOk(ent))
	{
		return;
	}

	ent->flags ^= FL_GODMODE;
	if(!(ent->flags & FL_GODMODE))
		msg = "godmode OFF";
	else
		msg = "godmode ON";

	trap_SendServerCommand(ent - g_entities, va("print \"%s", msg));
}

static void Cmd_Knock_f(gentity_t * ent)
{
	char           *msg;

	if(!CheatsOk(ent))
	{
		return;
	}

	ent->flags ^= FL_NO_KNOCKBACK;
	if(!(ent->flags & FL_NO_KNOCKBACK))
		msg = "knockback ON";
	else
		msg = "knockback OFF";

	trap_SendServerCommand(ent - g_entities, va("print \"%s", msg));
}


/*
================
LaunchItem

Spawns an item and tosses it forward
================
*/

static void ThrownItemThink(gentity_t * dropped)
{

	dropped->touch = Touch_Item;

	if(dropped->item->giType != IT_TEAM)
	{
		dropped->think = G_FreeEntity;
	}
	else
	{
		dropped->think = Team_DroppedFlagThink;
	}
	dropped->nextthink = level.time + 30000;
	dropped->flags = FL_DROPPED_ITEM;

	trap_LinkEntity(dropped);

}

static void ThrowItem(gitem_t * item, vec3_t origin, vec3_t velocity, int count)
{
	gentity_t      *dropped;


	dropped = G_Spawn();
	dropped->s.eType = ET_ITEM;
	dropped->s.modelindex = item - bg_itemlist;	// store item number in modelindex
	dropped->s.modelindex2 = 1;	// This is non-zero is it's a dropped item


	dropped->classname = item->classname;
	dropped->item = item;
	VectorSet(dropped->r.mins, -ITEM_RADIUS, -ITEM_RADIUS, -ITEM_RADIUS);
	VectorSet(dropped->r.maxs, ITEM_RADIUS, ITEM_RADIUS, ITEM_RADIUS);

	dropped->r.contents = CONTENTS_TRIGGER;

	G_SetOrigin(dropped, origin);
	G_RunItem(dropped);
	dropped->s.pos.trType = TR_GRAVITY;
	// reflect the velocity on the trace plane
	dropped->physicsObject = qtrue;
	dropped->physicsBounce = 0.5f;

	dropped->s.pos.trTime = level.time;
	VectorCopy(velocity, dropped->s.pos.trDelta);

	dropped->item->quantity = count;

	dropped->s.eFlags |= EF_BOUNCE_HALF;

	dropped->think = ThrownItemThink;

	dropped->nextthink = level.time + 700;

}
static void Cmd_ThrowW_f(gentity_t * ent)
{
	gitem_t        *item;
	int             weapon;
	gclient_t      *client;
	vec3_t          velocity;
	int             count;
	vec3_t          up;

	vec3_t          angles;

	client = &level.clients[ent->client->ps.clientNum];
	if(client->ps.stats[STAT_HEALTH] > 0)
	{
		weapon = ent->s.weapon;

		item = BG_FindItemForWeapon(weapon);

		if(weapon != WP_GAUNTLET && weapon != WP_MACHINEGUN)
		{
			if(client->ps.ammo[weapon] != 0)
			{
				if(g_gametype.integer >= GT_TEAM)
				{
					if(Instagib.integer == 0)
					{
						if(client->ps.persistant[PERS_TEAM] != TEAM_SPECTATOR)
						{
							count = client->ps.ammo[weapon];
							VectorCopy(ent->s.apos.trBase, angles);
							angles[YAW] = 90;
							angles[PITCH] = 0;	// always forward

							AngleVectors(client->ps.viewangles, velocity, angles, up);

							VectorScale(velocity, 200, velocity);
							velocity[2] += 250;

							ThrowItem(item, ent->s.pos.trBase, velocity, count);

							client->ps.ammo[weapon] = 0;
							client->ps.stats[STAT_WEAPONS] -= (1 << ent->s.weapon);
							trap_SendConsoleCommand(EXEC_INSERT, "weapprev");
						}
					}
				}
			}
		}
	}
}

static void Cmd_ThrowA_f(gentity_t * ent)
{
	gitem_t        *item;
	int             ammo;
	gclient_t      *client;
	vec3_t          velocity;
	vec3_t          angles;
	vec3_t          up;
	int             count;
	char           *cmd;


	cmd = ConcatArgs(1);

	count = atoi(cmd);

	client = &level.clients[ent->client->ps.clientNum];
	if(client->ps.stats[STAT_HEALTH] > 0)
	{
		if(ent->s.weapon != WP_GAUNTLET)
		{
			ammo = ent->s.weapon;

			item = BG_FindAmmoForWeapon(ammo);

			if(g_gametype.integer >= GT_TEAM)
			{
				if(Instagib.integer == 0)
				{
					if(client->ps.ammo[ammo] > 10)
					{
						if(client->ps.persistant[PERS_TEAM] != TEAM_SPECTATOR)
						{
							VectorCopy(ent->s.apos.trBase, angles);
							angles[YAW] = 0;
							angles[PITCH] = 0;	// always forward

							AngleVectors(client->ps.viewangles, velocity, angles, up);

							VectorScale(velocity, 200, velocity);
							velocity[2] += 250;

							ThrowItem(item, ent->s.pos.trBase, velocity, count);

							client->ps.ammo[ammo] -= count;
						}
					}
				}
			}
		}
	}
}

static void Team_ThrowFlags(gentity_t * ent)
{
	gitem_t        *item;
	gclient_t      *client;
	vec3_t          velocity;
	vec3_t          angles;
	vec3_t          up;
	int             count;


	item = NULL;
	client = &level.clients[ent->client->ps.clientNum];
	if(client->ps.stats[STAT_HEALTH] > 0)
	{
		//  if (ent->s.weapon != WP_GAUNTLET ){
		if(g_gametype.integer >= GT_TEAM)
		{
			if(client->ps.persistant[PERS_TEAM] != TEAM_SPECTATOR)
			{
				if(ent->client->ps.powerups[PW_REDFLAG] && ent->client->sess.sessionTeam != TEAM_RED)
				{
					item = BG_FindItemForPowerup(PW_REDFLAG);
					if(item)
					{
						// decide how many seconds it has left
						count = (ent->client->ps.powerups[PW_REDFLAG] - level.time) / 1000;

						VectorCopy(ent->s.apos.trBase, angles);
						angles[YAW] = 0;
						angles[PITCH] = 0;	// always forward

						AngleVectors(client->ps.viewangles, velocity, angles, up);

						VectorScale(velocity, 200, velocity);
						velocity[2] += 250;

						ThrowItem(item, ent->s.pos.trBase, velocity, count);
						ent->client->ps.powerups[PW_REDFLAG] = 0;
					}
				}
				item = NULL;
				if(ent->client->ps.powerups[PW_BLUEFLAG] && ent->client->sess.sessionTeam != TEAM_BLUE)
				{
					item = BG_FindItemForPowerup(PW_BLUEFLAG);
					if(item)
					{
						// decide how many seconds it has left
						count = (ent->client->ps.powerups[PW_BLUEFLAG] - level.time) / 1000;

						VectorCopy(ent->s.apos.trBase, angles);
						angles[YAW] = 0;
						angles[PITCH] = 0;	// always forward

						AngleVectors(client->ps.viewangles, velocity, angles, up);

						VectorScale(velocity, 200, velocity);
						velocity[2] += 250;

						ThrowItem(item, ent->s.pos.trBase, velocity, count);
						ent->client->ps.powerups[PW_BLUEFLAG] = 0;
					}
				}
			}
			//  }
		}
	}

}

/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
static void Cmd_Notarget_f(gentity_t * ent)
{
	char           *msg;

	if(!CheatsOk(ent))
	{
		return;
	}

	ent->flags |= FL_NOTARGET;
	if(!(ent->flags & FL_NOTARGET))
		msg = "notarget OFF";
	else
		msg = "notarget ON";

	trap_SendServerCommand(ent - g_entities, va("print \"%s", msg));
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
static void Cmd_Noclip_f(gentity_t * ent)
{
	char           *msg;

	if(!CheatsOk(ent))
	{
		return;
	}

	if(ent->client->noclip)
	{
		msg = "noclip OFF";
	}
	else
	{
		msg = "noclip ON";
	}
	ent->client->noclip = !ent->client->noclip;

	trap_SendServerCommand(ent - g_entities, va("print \"%s", msg));
}


/*
==================
Cmd_LevelShot_f

This is just to help generate the level pictures
for the menus.  It goes to the intermission immediately
and sends over a command to the client to resize the view,
hide the scoreboard, and take a special screenshot
==================
*/
static void Cmd_LevelShot_f(gentity_t * ent)
{
	if(!CheatsOk(ent))
	{
		return;
	}

	// doesn't work in single player
	if(g_gametype.integer != 0)
	{
		trap_SendServerCommand(ent - g_entities, "printc \"Must be in g_gametype 0 for levelshot\n\"");
		return;
	}

	BeginIntermission();
	trap_SendServerCommand(ent - g_entities, "clientLevelShot");
}


/*
==================
Cmd_LevelShot_f

This is just to help generate the level pictures
for the menus.  It goes to the intermission immediately
and sends over a command to the client to resize the view,
hide the scoreboard, and take a special screenshot
==================
*/
static void Cmd_TeamTask_f(gentity_t * ent)
{
	char            userinfo[MAX_INFO_STRING];
	char            arg[MAX_TOKEN_CHARS];
	int             task;
	int             client = ent->client - level.clients;

	if(trap_Argc() != 2)
	{
		return;
	}
	trap_Argv(1, arg, sizeof(arg));
	task = atoi(arg);

	trap_GetUserinfo(client, userinfo, sizeof(userinfo));
	Info_SetValueForKey(userinfo, "teamtask", va("%d", task));
	trap_SetUserinfo(client, userinfo);
	ClientUserinfoChanged(client);
}



/*
=================
Cmd_Kill_f
=================
*/
static void Cmd_Kill_f(gentity_t * ent)
{
	if(ent->client->sess.sessionTeam == TEAM_SPECTATOR)
	{
		return;
	}
	if(ent->health <= 0)
	{
		return;
	}
	if(ent->client->ps.pm_flags & PMF_RESPAWNED)
	{
		return;
	}
	ent->flags &= ~FL_GODMODE;
	ent->client->ps.stats[STAT_HEALTH] = ent->health = -999;
	player_die(ent, ent, ent, 100000, MOD_SUICIDE);
}

/*
=================
BroadCastTeamChange

Let everyone know about a team change
=================
*/
void BroadcastTeamChange(gclient_t * client, int oldTeam)
{
	if(client->sess.sessionTeam == TEAM_RED)
	{
		trap_SendServerCommand(-1, va("cp \"%s" S_COLOR_WHITE " ^Njoined the red team.", client->pers.netname));
	}
	else if(client->sess.sessionTeam == TEAM_BLUE)
	{
		trap_SendServerCommand(-1, va("cp \"%s" S_COLOR_WHITE " ^Njoined the blue team.", client->pers.netname));
	}
	else if(client->sess.sessionTeam == TEAM_SPECTATOR && oldTeam != TEAM_SPECTATOR)
	{
		trap_SendServerCommand(-1, va("cp \"%s" S_COLOR_WHITE " ^Njoined the spectators.", client->pers.netname));
	}
	else if(client->sess.sessionTeam == TEAM_FREE)
	{
		trap_SendServerCommand(-1, va("cp \"%s" S_COLOR_WHITE " ^Njoined the battle.", client->pers.netname));
	}
}

/*
=================
SetTeam
=================
*/
void SetTeam(gentity_t * ent, char *s)
{
	int             team, oldTeam;
	gclient_t      *client;
	int             clientNum;
	spectatorState_t specState;
	int             specClient;
	int             teamLeader;

	//
	// see what change is requested
	//
	client = ent->client;

	clientNum = client - level.clients;
	specClient = 0;
	specState = SPECTATOR_NOT;
	if(!Q_stricmp(s, "scoreboard") || !Q_stricmp(s, "score"))
	{
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_SCOREBOARD;
	}
	else if(!Q_stricmp(s, "follow1"))
	{
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FOLLOW;
		specClient = -1;
	}
	else if(!Q_stricmp(s, "follow2"))
	{
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FOLLOW;
		specClient = -2;
	}
	else if(!Q_stricmp(s, "spectator") || !Q_stricmp(s, "s"))
	{
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FREE;
	}
	else if(g_gametype.integer >= GT_TEAM)
	{
		// if running a team game, assign player to one of the teams
		specState = SPECTATOR_NOT;
		if(!Q_stricmp(s, "red") || !Q_stricmp(s, "r"))
		{
			team = TEAM_RED;
		}
		else if(!Q_stricmp(s, "blue") || !Q_stricmp(s, "b"))
		{
			team = TEAM_BLUE;
		}
		else
		{
			// pick the team with the least number of players
			team = PickTeam(clientNum);
		}

		if(g_teamForceBalance.integer)
		{
			int             counts[TEAM_NUM_TEAMS];

			counts[TEAM_BLUE] = TeamCount(ent->client->ps.clientNum, TEAM_BLUE);
			counts[TEAM_RED] = TeamCount(ent->client->ps.clientNum, TEAM_RED);

			// We allow a spread of two
			if(team == TEAM_RED && counts[TEAM_RED] - counts[TEAM_BLUE] > 1)
			{
				trap_SendServerCommand(ent->client->ps.clientNum, "cp \"Red team has too many players.\n\"");
				return;			// ignore the request
			}
			if(team == TEAM_BLUE && counts[TEAM_BLUE] - counts[TEAM_RED] > 1)
			{
				trap_SendServerCommand(ent->client->ps.clientNum, "cp \"Blue team has too many players.\n\"");
				return;			// ignore the request
			}

			// It's ok, the team we are switching to has less or same number of players
		}

	}
	else
	{
		// force them to spectators if there aren't any spots free
		team = TEAM_FREE;
	}

	// override decision if limiting the players
	if((g_gametype.integer == GT_TOURNAMENT) && level.numNonSpectatorClients >= 2)
	{
		team = TEAM_SPECTATOR;
	}
	else if(g_maxGameClients.integer > 0 && level.numNonSpectatorClients >= g_maxGameClients.integer)
	{
		team = TEAM_SPECTATOR;
	}

	//
	// decide if we will allow the change
	//
	oldTeam = client->sess.sessionTeam;
	if(team == oldTeam && team != TEAM_SPECTATOR)
	{
		return;
	}

	//
	// execute the team change
	//

	// if the player was dead leave the body
	if(client->ps.stats[STAT_HEALTH] <= 0)
	{
		CopyToBodyQue(ent);
	}

	// he starts at 'base'
	client->pers.teamState.state = TEAM_BEGIN;
	if(oldTeam != TEAM_SPECTATOR)
	{
		// Kill him (makes sure he loses flags, etc)
		ent->flags &= ~FL_GODMODE;
		ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
		player_die(ent, ent, ent, 100000, MOD_SUICIDE);

	}
	// they go to the end of the line for tournements
	if(team == TEAM_SPECTATOR)
	{
		client->sess.spectatorTime = level.time;
	}

	client->sess.sessionTeam = team;
	client->sess.spectatorState = specState;
	client->sess.spectatorClient = specClient;

	client->sess.teamLeader = qfalse;
	if(team == TEAM_RED || team == TEAM_BLUE)
	{
		teamLeader = TeamLeader(team);
		// if there is no team leader or the team leader is a bot and this client is not a bot
		if(teamLeader == -1 || (!(g_entities[clientNum].r.svFlags & SVF_BOT) && (g_entities[teamLeader].r.svFlags & SVF_BOT)))
		{
			SetLeader(team, clientNum);
		}
	}
	// make sure there is a team leader on the team the player came from
	if(oldTeam == TEAM_RED || oldTeam == TEAM_BLUE)
	{
		CheckTeamLeader(oldTeam);
	}

	BroadcastTeamChange(client, oldTeam);

	// get and distribute relevent paramters
	ClientUserinfoChanged(clientNum);

	ClientBegin(clientNum);
}

/*
=================
StopFollowing

If the client being followed leaves the game, or you just want to drop
to free floating spectator mode
=================
*/
void StopFollowing(gentity_t * ent)
{
	ent->client->ps.persistant[PERS_TEAM] = TEAM_SPECTATOR;
	ent->client->sess.sessionTeam = TEAM_SPECTATOR;
	ent->client->sess.spectatorState = SPECTATOR_FREE;
	ent->client->ps.pm_flags &= ~PMF_FOLLOW;
	ent->r.svFlags &= ~SVF_BOT;
	ent->client->ps.clientNum = ent - g_entities;
}

/*
=================
Cmd_Team_f
=================
*/
static void Cmd_Team_f(gentity_t * ent)
{
	int             oldTeam;
	char            s[MAX_TOKEN_CHARS];

	if(trap_Argc() != 2)
	{
		oldTeam = ent->client->sess.sessionTeam;
		switch (oldTeam)
		{
			case TEAM_BLUE:
				trap_SendServerCommand(ent - g_entities, "printc \"Blue team\n\"");
				break;
			case TEAM_RED:
				trap_SendServerCommand(ent - g_entities, "printc \"Red team\n\"");
				break;
			case TEAM_FREE:
				trap_SendServerCommand(ent - g_entities, "printc \"Free team\n\"");
				break;
			case TEAM_SPECTATOR:
				trap_SendServerCommand(ent - g_entities, "printc \"Spectator team\n\"");
				break;
		}
		return;
	}

	if(ent->client->switchTeamTime > level.time)
	{
		trap_SendServerCommand(ent - g_entities, "printc \"May not switch teams more than once per 5 seconds.\n\"");
		return;
	}

	// if they are playing a tournement game, count as a loss
	if((g_gametype.integer == GT_TOURNAMENT) && ent->client->sess.sessionTeam == TEAM_FREE)
	{
		ent->client->sess.losses++;
	}

	trap_Argv(1, s, sizeof(s));

	SetTeam(ent, s);

	ent->client->switchTeamTime = level.time + 5000;
}


/*
=================
Cmd_Follow_f
=================
*/
static void Cmd_Follow_f(gentity_t * ent)
{
	int             i;
	char            arg[MAX_TOKEN_CHARS];

	if(trap_Argc() != 2)
	{
		if(ent->client->sess.spectatorState == SPECTATOR_FOLLOW)
		{
			StopFollowing(ent);
		}
		return;
	}

	trap_Argv(1, arg, sizeof(arg));
	i = ClientNumberFromString(ent, arg);
	if(i == -1)
	{
		return;
	}

	// can't follow self
	if(&level.clients[i] == ent->client)
	{
		return;
	}

	// can't follow another spectator
	if(level.clients[i].sess.sessionTeam == TEAM_SPECTATOR)
	{
		return;
	}

	// if they are playing a tournement game, count as a loss
	if((g_gametype.integer == GT_TOURNAMENT) && ent->client->sess.sessionTeam == TEAM_FREE)
	{
		ent->client->sess.losses++;
	}

	// first set them to spectator
	if(ent->client->sess.sessionTeam != TEAM_SPECTATOR)
	{
		SetTeam(ent, "spectator");
	}

	ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
	ent->client->sess.spectatorClient = i;
}

/*
=================
Cmd_FollowCycle_f
=================
*/
void Cmd_StatCycle_f(gentity_t * ent, int dir)
{
	int             pickupgl, pickuprl, pickupmg, pickuprg, pickuplg, pickuppg, pickupbfg, pickupsg;
	int             pickupmega, pickupyarmor, pickuprarmor, pickupshard, pickup5h, pickup20h, pickup40h;
	int             pickupquad, pickupsuit, pickuphaste, pickupregen, pickupinvis, pickuprflag, pickupbflag, pickupflight;
	int             hitsrg, shotsrg, accuracyrg;
	int             hitsrl, shotsrl, accuracyrl;
	int             hitspg, shotspg, accuracypg;
	int             hitsgl, shotsgl, accuracygl;
	int             hitssg, shotssg, accuracysg;
	int             hitsmg, shotsmg, accuracymg;
	int             hitslg, shotslg, accuracylg;
	int             hitsbfg, shotsbfg, accuracybfg;
	int             pickupHealtht, pickupArmort;
	int             damagert, damagegt;
	gclient_t      *client;
	int             clientnum;
	int             original;



	if(dir != 1 && dir != -1)
	{
		G_Error("Cmd_FollowCycle_f: bad dir %i", dir);
	}

	clientnum = ent->client->ps.clientNum;
	original = clientnum;
//  do {
	clientnum += dir;
	if(clientnum >= level.maxclients)
	{
		clientnum = 0;
	}
	if(clientnum < 0)
	{
		clientnum = level.maxclients - 1;
	}

	if(level.clients[clientnum].pers.connected != CON_CONNECTED)
	{
		return;
	}

	// can't follow another spectator
	if(level.clients[clientnum].sess.sessionTeam == TEAM_SPECTATOR)
	{
		return;
	}

	// this is good, we can use it




	client = &level.clients[clientnum];



	if(client->pickupHealthT)
	{
		pickupHealtht = client->pickupHealthT;
	}
	else
	{
		pickupHealtht = 0;
	}

	if(client->pickupArmorT)
	{
		pickupArmort = client->pickupArmorT;
	}
	else
	{
		pickupArmort = 0;
	}

	if(client->damageRT)
	{
		damagert = client->damageRT;
	}
	else
	{
		damagert = 0;
	}

	if(client->damageGT)
	{
		damagegt = client->damageGT;
	}
	else
	{
		damagegt = 0;
	}

// railgun stats
	if(client->accuracy_shot[WP_RAILGUN])
	{
		hitsrg = client->accuracy_hit[WP_RAILGUN];
		shotsrg = client->accuracy_shot[WP_RAILGUN];
		accuracyrg = hitsrg * 100 / shotsrg;
	}
	else
	{
		hitsrg = 0;
		shotsrg = 0;
		accuracyrg = 0;
	}

//rocket launcher stats
	if(client->accuracy_shot[WP_ROCKET_LAUNCHER])
	{
		hitsrl = client->accuracy_hit[WP_ROCKET_LAUNCHER];
		shotsrl = client->accuracy_shot[WP_ROCKET_LAUNCHER];
		accuracyrl = hitsrl * 100 / shotsrl;
	}
	else
	{
		hitsrl = 0;
		shotsrl = 0;
		accuracyrl = 0;
	}

//grenade launcher stats
	if(client->accuracy_shot[WP_GRENADE_LAUNCHER])
	{
		hitsgl = client->accuracy_hit[WP_GRENADE_LAUNCHER];
		shotsgl = client->accuracy_shot[WP_GRENADE_LAUNCHER];
		accuracygl = hitsgl * 100 / shotsgl;
	}
	else
	{
		hitsgl = 0;
		shotsgl = 0;
		accuracygl = 0;
	}

//plasmagun stats
	if(client->accuracy_shot[WP_PLASMAGUN])
	{
		hitspg = client->accuracy_hit[WP_PLASMAGUN];
		shotspg = client->accuracy_shot[WP_PLASMAGUN];
		accuracypg = hitspg * 100 / shotspg;
	}
	else
	{
		hitspg = 0;
		shotspg = 0;
		accuracypg = 0;
	}

//machinegun stats
	if(client->accuracy_shot[WP_MACHINEGUN])
	{
		hitsmg = client->accuracy_hit[WP_MACHINEGUN];
		shotsmg = client->accuracy_shot[WP_MACHINEGUN];
		accuracymg = hitsmg * 100 / shotsmg;
	}
	else
	{
		hitsmg = 0;
		shotsmg = 0;
		accuracymg = 0;
	}



//shotgun stats
	if(client->accuracy_shot[WP_SHOTGUN])
	{
		hitssg = client->accuracy_hit[WP_SHOTGUN];
		shotssg = client->accuracy_shot[WP_SHOTGUN] * 8;
		accuracysg = hitssg * 100 / shotssg;
	}
	else
	{
		hitssg = 0;
		shotssg = 0;
		accuracysg = 0;
	}


//lightning gun stats
	if(client->accuracy_shot[WP_LIGHTNING])
	{
		hitslg = client->accuracy_hit[WP_LIGHTNING];
		shotslg = client->accuracy_shot[WP_LIGHTNING];
		accuracylg = hitslg * 100 / shotslg;
	}
	else
	{
		hitslg = 0;
		shotslg = 0;
		accuracylg = 0;
	}


//bfg stats
	if(client->accuracy_shot[WP_BFG])
	{
		hitsbfg = client->accuracy_hit[WP_BFG];
		shotsbfg = client->accuracy_shot[WP_BFG];
		accuracybfg = hitsbfg * 100 / shotsbfg;
	}
	else
	{
		hitsbfg = 0;
		shotsbfg = 0;
		accuracybfg = 0;
	}

// weapon pickups
	if(client->pickup[WP_GRENADE_LAUNCHER] > 0)
	{
		pickupgl = client->pickup[WP_GRENADE_LAUNCHER];
	}
	else
	{
		pickupgl = 0;
	}

	if(client->pickup[WP_MACHINEGUN] > 0)
	{
		pickupmg = client->pickup[WP_MACHINEGUN];
	}
	else
	{
		pickupmg = 0;
	}

	if(client->pickup[WP_LIGHTNING] > 0)
	{
		pickuplg = client->pickup[WP_LIGHTNING];
	}
	else
	{
		pickuplg = 0;
	}

	if(client->pickup[WP_SHOTGUN] > 0)
	{
		pickupsg = client->pickup[WP_SHOTGUN];
	}
	else
	{
		pickupsg = 0;
	}

	if(client->pickup[WP_RAILGUN] > 0)
	{
		pickuprg = client->pickup[WP_RAILGUN];
	}
	else
	{
		pickuprg = 0;
	}

	if(client->pickup[WP_ROCKET_LAUNCHER] > 0)
	{
		pickuprl = client->pickup[WP_ROCKET_LAUNCHER];
	}
	else
	{
		pickuprl = 0;
	}

	if(client->pickup[WP_PLASMAGUN] > 0)
	{
		pickuppg = client->pickup[WP_PLASMAGUN];
	}
	else
	{
		pickuppg = 0;
	}

	if(client->pickup[WP_BFG] > 0)
	{
		pickupbfg = client->pickup[WP_BFG];
	}
	else
	{
		pickupbfg = 0;
	}

// health pickups
	if(client->pickupmega > 0)
	{
		pickupmega = client->pickupmega;
	}
	else
	{
		pickupmega = 0;
	}

	if(client->pickup5h > 0)
	{
		pickup5h = client->pickup5h;
	}
	else
	{
		pickup5h = 0;
	}

	if(client->pickup20h > 0)
	{
		pickup20h = client->pickup20h;
	}
	else
	{
		pickup20h = 0;
	}

	if(client->pickup40h > 0)
	{
		pickup40h = client->pickup40h;
	}
	else
	{
		pickup40h = 0;
	}

// armor pickups
	if(client->pickuprarmor > 0)
	{
		pickuprarmor = client->pickuprarmor;
	}
	else
	{
		pickuprarmor = 0;
	}

	if(client->pickupyarmor > 0)
	{
		pickupyarmor = client->pickupyarmor;
	}
	else
	{
		pickupyarmor = 0;
	}

	if(client->pickupshard > 0)
	{
		pickupshard = client->pickupshard;
	}
	else
	{
		pickupshard = 0;
	}

// powerup pickups
	if(client->pickupquad > 0)
	{
		pickupquad = client->pickupquad;
	}
	else
	{
		pickupquad = 0;
	}

	if(client->pickupsuit > 0)
	{
		pickupsuit = client->pickupsuit;
	}
	else
	{
		pickupsuit = 0;
	}

	if(client->pickuphaste > 0)
	{
		pickuphaste = client->pickuphaste;
	}
	else
	{
		pickuphaste = 0;
	}

	if(client->pickupregen > 0)
	{
		pickupregen = client->pickupregen;
	}
	else
	{
		pickupregen = 0;
	}

	if(client->pickupinvis > 0)
	{
		pickupinvis = client->pickupinvis;
	}
	else
	{
		pickupinvis = 0;
	}

	if(client->pickupflight > 0)
	{
		pickupflight = client->pickupflight;
	}
	else
	{
		pickupflight = 0;
	}

// flag pickups
	if(client->pickuprflag > 0)
	{
		pickuprflag = client->pickuprflag;
	}
	else
	{
		pickuprflag = 0;
	}

	if(client->pickupbflag > 0)
	{
		pickupbflag = client->pickupbflag;
	}
	else
	{
		pickupbflag = 0;
	}


// now send this crap to the client for stats
/*	trap_SendServerCommand( ent-g_entities, va("fstinfo %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i", clientnum,hitsmg,
		shotsmg,hitsrl,shotsrl,hitsgl,shotsgl,
				hitspg,shotspg,hitsrg,shotsrg,hitslg,shotslg,
				hitssg,shotssg,hitsbfg,shotsbfg,pickupgl, pickuprl,pickupmg,pickuprg,pickuplg,pickuppg,
				pickupbfg,pickupsg,pickupmega,pickupyarmor,pickuprarmor,pickupshard,pickup5h,pickup20h,
				pickup40h,pickupquad,pickupsuit,pickuphaste,pickupregen,pickupinvis,pickuprflag,pickupbflag,pickupflight,pickupArmort, pickupHealtht,damagert, damagegt) );
*/
//      ent->client->sess.spectatorClient = clientnum;
//      ent->client->sess.spectatorState = SPECTATOR_NOT;
//      return;
//  } while ( clientnum != -1 );
}

/*
=================
Cmd_FollowCycle_f
=================
*/
void Cmd_FollowCycle_f(gentity_t * ent, int dir)
{
	int             clientnum;
	int             original;

	// if they are playing a tournement game, count as a loss
	if((g_gametype.integer == GT_TOURNAMENT) && ent->client->sess.sessionTeam == TEAM_FREE)
	{
		ent->client->sess.losses++;
	}
	// first set them to spectator
	if(ent->client->sess.spectatorState == SPECTATOR_NOT && ent->client->ps.pm_type != PM_INTERMISSION)
	{
		SetTeam(ent, "spectator");
	}

	if(dir != 1 && dir != -1)
	{
		G_Error("Cmd_FollowCycle_f: bad dir %i", dir);
	}

	clientnum = ent->client->sess.spectatorClient;
	original = clientnum;
	do
	{
		clientnum += dir;
		if(clientnum >= level.maxclients)
		{
			clientnum = 0;
		}
		if(clientnum < 0)
		{
			clientnum = level.maxclients - 1;
		}

		// can only follow connected clients
		if(level.clients[clientnum].pers.connected != CON_CONNECTED)
		{
			continue;
		}

		// can't follow another spectator
		if(level.clients[clientnum].sess.sessionTeam == TEAM_SPECTATOR)
		{
			continue;
		}

		// this is good, we can use it
		ent->client->sess.spectatorClient = clientnum;
		ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
		return;
	} while(clientnum != original);

	// leave it where it was
}


/*
==================
G_Say
==================
*/

static void G_SayTo(gentity_t * ent, gentity_t * other, int mode, int color, const char *name, const char *message)
{
	if(!other)
	{
		return;
	}
	if(!other->inuse)
	{
		return;
	}
	if(!other->client)
	{
		return;
	}
	if(other->client->pers.connected != CON_CONNECTED)
	{
		return;
	}
	if(mode == SAY_TEAM && !OnSameTeam(ent, other))
	{
		return;
	}
	// no chatting to players in tournements
/*	if ( (g_gametype.integer == GT_TOURNAMENT )
		&& other->client->sess.sessionTeam == TEAM_FREE
		&& ent->client->sess.sessionTeam != TEAM_FREE ) {
		return;
	}*/
	trap_SendServerCommand(other - g_entities, va("%s \"%s^N%c%c%s\"",
												  mode == SAY_TEAM ? "tchat" : "chat", name, Q_COLOR_ESCAPE, color, message));
}

#define EC		"\x19"

static void G_Say(gentity_t * ent, gentity_t * target, int mode, const char *chatText)
{
	int             j;
	gentity_t      *other;
	int             color;
	char            name[64];

	// don't let text be too long for malicious reasons
	char            text[MAX_SAY_TEXT];
	char            location[64];

	if(g_gametype.integer < GT_TEAM && mode == SAY_TEAM)
	{
		mode = SAY_ALL;
	}

	if(g_gametype.integer > GT_TEAM && mode == SAY_TELL && ent->r.svFlags & SVF_BOT)
	{
		mode = SAY_TEAM;
	}

	if(ent->client->sess.sessionTeam == TEAM_SPECTATOR && mutespecs.integer == 1)
	{
		mode = SAY_TEAM;
	}

	switch (mode)
	{
		default:
		case SAY_ALL:
			G_LogPrintf("say: %s: %s\n", ent->client->pers.netname, chatText);
			Com_sprintf(name, sizeof(name), "%s%c%c" EC ": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE);
			color = COLOR_GREEN;
			break;
		case SAY_TEAM:
			G_LogPrintf("sayteam: %s: %s\n", ent->client->pers.netname, chatText);
			if(ent->client->sess.sessionTeam != TEAM_SPECTATOR)
			{
				if(Team_GetLocationMsg(ent, location, sizeof(location)))
					Com_sprintf(name, sizeof(name), EC "(%s%c%c" EC ") (%s)" EC ": ",
								ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE, location);
				else
					Com_sprintf(name, sizeof(name), EC "(%s%c%c" EC ")" EC ": ",
								ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE);
				color = COLOR_CYAN;
			}
			else
			{
				Com_sprintf(name, sizeof(name), EC "^5[%s" EC "^5]" EC ": ", ent->client->pers.netname);
				color = COLOR_CYAN;
			}
			break;
		case SAY_TELL:
			if(target && g_gametype.integer >= GT_TEAM &&
			   target->client->sess.sessionTeam == ent->client->sess.sessionTeam &&
			   Team_GetLocationMsg(ent, location, sizeof(location)))
				Com_sprintf(name, sizeof(name), EC "[%s%c%c" EC "] (%s)" EC ": ", ent->client->pers.netname, Q_COLOR_ESCAPE,
							COLOR_WHITE, location);
			else
				Com_sprintf(name, sizeof(name), EC "[%s%c%c" EC "]" EC ": ", ent->client->pers.netname, Q_COLOR_ESCAPE,
							COLOR_WHITE);
			color = COLOR_MAGENTA;
			break;
	}

	Q_strncpyz(text, chatText, sizeof(text));

	if(target)
	{
		G_SayTo(ent, target, mode, color, name, text);
		return;
	}

	// echo the text to the console
	if(g_dedicated.integer)
	{
		G_Printf("%s%s\n", name, text);
	}

	// send it to all the apropriate clients
	for(j = 0; j < level.maxclients; j++)
	{
		other = &g_entities[j];
		G_SayTo(ent, other, mode, color, name, text);
	}
}


/*
==================
Cmd_Say_f
==================
*/
static void Cmd_Say_f(gentity_t * ent, int mode, qboolean arg0)
{
	char           *p;

	if(trap_Argc() < 2 && !arg0)
	{
		return;
	}

	if(arg0)
	{
		p = ConcatArgs(0);
	}
	else
	{
		p = ConcatArgs(1);
	}

	G_Say(ent, NULL, mode, p);
}

/*
==================
Cmd_Tell_f
==================
*/
static void Cmd_Tell_f(gentity_t * ent)
{
	int             targetNum;
	gentity_t      *target;
	char           *p;
	char            arg[MAX_TOKEN_CHARS];

	if(trap_Argc() < 2)
	{
		return;
	}

	trap_Argv(1, arg, sizeof(arg));
	targetNum = atoi(arg);
	if(targetNum < 0 || targetNum >= level.maxclients)
	{
		return;
	}

	target = &g_entities[targetNum];
	if(!target || !target->inuse || !target->client)
	{
		return;
	}

	p = ConcatArgs(2);

	G_LogPrintf("tell: %s to %s: %s\n", ent->client->pers.netname, target->client->pers.netname, p);
	G_Say(ent, target, SAY_TELL, p);
	// don't tell to the player self if it was already directed to this player
	// also don't send the chat back to a bot
	if(ent != target && !(ent->r.svFlags & SVF_BOT))
	{
		G_Say(ent, ent, SAY_TELL, p);
	}
}


static void G_VoiceTo(gentity_t * ent, gentity_t * other, int mode, const char *id, qboolean voiceonly)
{
	int             color;
	char           *cmd;

	if(!other)
	{
		return;
	}
	if(!other->inuse)
	{
		return;
	}
	if(!other->client)
	{
		return;
	}
	if(mode == SAY_TEAM && !OnSameTeam(ent, other))
	{
		return;
	}
	// no chatting to players in tournements
	if((g_gametype.integer == GT_TOURNAMENT))
	{
		return;
	}

	if(mode == SAY_TEAM)
	{
		color = COLOR_CYAN;
		cmd = "vtchat";
	}
	else if(mode == SAY_TELL)
	{
		color = COLOR_MAGENTA;
		cmd = "vtell";
	}
	else
	{
		color = COLOR_GREEN;
		cmd = "vchat";
	}

	trap_SendServerCommand(other - g_entities, va("%s %d %d %d %s", cmd, voiceonly, ent->s.number, color, id));
}

void G_Voice(gentity_t * ent, gentity_t * target, int mode, const char *id, qboolean voiceonly)
{
	int             j;
	gentity_t      *other;

	if(g_gametype.integer < GT_TEAM && mode == SAY_TEAM)
	{
		mode = SAY_ALL;
	}

	if(target)
	{
		G_VoiceTo(ent, target, mode, id, voiceonly);
		return;
	}

	// echo the text to the console
	if(g_dedicated.integer)
	{
		G_Printf("voice: %s %s\n", ent->client->pers.netname, id);
	}

	// send it to all the apropriate clients
	for(j = 0; j < level.maxclients; j++)
	{
		other = &g_entities[j];
		G_VoiceTo(ent, other, mode, id, voiceonly);
	}
}

/*
==================
Cmd_Voice_f
==================
*/
static void Cmd_Voice_f(gentity_t * ent, int mode, qboolean arg0, qboolean voiceonly)
{
	char           *p;

	if(trap_Argc() < 2 && !arg0)
	{
		return;
	}

	if(arg0)
	{
		p = ConcatArgs(0);
	}
	else
	{
		p = ConcatArgs(1);
	}

	G_Voice(ent, NULL, mode, p, voiceonly);
}

/*
==================
Cmd_VoiceTell_f
==================
*/
static void Cmd_VoiceTell_f(gentity_t * ent, qboolean voiceonly)
{
	int             targetNum;
	gentity_t      *target;
	char           *id;
	char            arg[MAX_TOKEN_CHARS];

	if(trap_Argc() < 2)
	{
		return;
	}

	trap_Argv(1, arg, sizeof(arg));
	targetNum = atoi(arg);
	if(targetNum < 0 || targetNum >= level.maxclients)
	{
		return;
	}

	target = &g_entities[targetNum];
	if(!target || !target->inuse || !target->client)
	{
		return;
	}

	id = ConcatArgs(2);

	G_LogPrintf("vtell: %s to %s: %s\n", ent->client->pers.netname, target->client->pers.netname, id);
	G_Voice(ent, target, SAY_TELL, id, voiceonly);
	// don't tell to the player self if it was already directed to this player
	// also don't send the chat back to a bot
	if(ent != target && !(ent->r.svFlags & SVF_BOT))
	{
		G_Voice(ent, ent, SAY_TELL, id, voiceonly);
	}
}


/*
==================
Cmd_VoiceTaunt_f
==================
*/
static void Cmd_VoiceTaunt_f(gentity_t * ent)
{
	gentity_t      *who;
	int             i;

	if(!ent->client)
	{
		return;
	}

	// insult someone who just killed you
	if(ent->enemy && ent->enemy->client && ent->enemy->client->lastkilled_client == ent->s.number)
	{
		// i am a dead corpse
		if(!(ent->enemy->r.svFlags & SVF_BOT))
		{
			G_Voice(ent, ent->enemy, SAY_TELL, VOICECHAT_DEATHINSULT, qfalse);
		}
		if(!(ent->r.svFlags & SVF_BOT))
		{
			G_Voice(ent, ent, SAY_TELL, VOICECHAT_DEATHINSULT, qfalse);
		}
		ent->enemy = NULL;
		return;
	}
	// insult someone you just killed
	if(ent->client->lastkilled_client >= 0 && ent->client->lastkilled_client != ent->s.number)
	{
		who = g_entities + ent->client->lastkilled_client;
		if(who->client)
		{
			// who is the person I just killed
			if(who->client->lasthurt_mod == MOD_GAUNTLET)
			{
				if(!(who->r.svFlags & SVF_BOT))
				{
					G_Voice(ent, who, SAY_TELL, VOICECHAT_KILLGAUNTLET, qfalse);	// and I killed them with a gauntlet
				}
				if(!(ent->r.svFlags & SVF_BOT))
				{
					G_Voice(ent, ent, SAY_TELL, VOICECHAT_KILLGAUNTLET, qfalse);
				}
			}
			else
			{
				if(!(who->r.svFlags & SVF_BOT))
				{
					G_Voice(ent, who, SAY_TELL, VOICECHAT_KILLINSULT, qfalse);	// and I killed them with something else
				}
				if(!(ent->r.svFlags & SVF_BOT))
				{
					G_Voice(ent, ent, SAY_TELL, VOICECHAT_KILLINSULT, qfalse);
				}
			}
			ent->client->lastkilled_client = -1;
			return;
		}
	}

	if(g_gametype.integer >= GT_TEAM)
	{
		// praise a team mate who just got a reward
		for(i = 0; i < MAX_CLIENTS; i++)
		{
			who = g_entities + i;
			if(who->client && who != ent && who->client->sess.sessionTeam == ent->client->sess.sessionTeam)
			{
				if(who->client->rewardTime > level.time)
				{
					if(!(who->r.svFlags & SVF_BOT))
					{
						G_Voice(ent, who, SAY_TELL, VOICECHAT_PRAISE, qfalse);
					}
					if(!(ent->r.svFlags & SVF_BOT))
					{
						G_Voice(ent, ent, SAY_TELL, VOICECHAT_PRAISE, qfalse);
					}
					return;
				}
			}
		}
	}

	// just say something
	G_Voice(ent, NULL, SAY_ALL, VOICECHAT_TAUNT, qfalse);
}



static char    *gc_orders[] = {
	"hold your position",
	"hold this position",
	"come here",
	"cover me",
	"guard location",
	"search and destroy",
	"report"
};

static void Cmd_GameCommand_f(gentity_t * ent)
{
	int             player;
	int             order;
	char            str[MAX_TOKEN_CHARS];

	trap_Argv(1, str, sizeof(str));
	player = atoi(str);
	trap_Argv(2, str, sizeof(str));
	order = atoi(str);

	if(player < 0 || player >= MAX_CLIENTS)
	{
		return;
	}
	if(order < 0 || order > sizeof(gc_orders) / sizeof(char *))
	{
		return;
	}
	G_Say(ent, &g_entities[player], SAY_TELL, gc_orders[order]);
	G_Say(ent, ent, SAY_TELL, gc_orders[order]);
}

/*
==================
Cmd_Where_f
==================
*/
static void Cmd_Where_f(gentity_t * ent)
{
	trap_SendServerCommand(ent - g_entities, va("printc \"%s\n\"", vtos(ent->s.origin)));
}

static const char *gameNames[] = {
	"Free For All",
	"Tournament",
	"Single Player",
	"Team Deathmatch",
	"Capture the Flag",
	"One Flag CTF",
	"Overload",
	"Harvester"
};

static const char *instaNames[] = {
	"RailGun",
	"Rocket Launcher",
	"RailGun & Rocket Launcher",
};

//NT - redid a bunch of the voting
static const char *voteVars[] = {
	"Kick",
	"ResetRotation",
	"FragLimit",
	"DoWarmup",
	"FallingDamage",
	"FriendlyFire",
	"GameType",
	"SelfDamage",
	"Map",
	"Restart",
	"NextMap",
	"TimeLimit",
	"InstaGib",
	"InstaWeapon",
	"FootSteps",
	"Hook",
	"HookMissleSpeed",
	"DisableBFG",
	"DisableQUAD",
	"SpaceProtection",
	"FloodProtection",
	"Vampire",
	"SpeedSpawn",
	"SpawnProtect",
};
static const int nVoteVars = sizeof(voteVars) / sizeof(const char *);

static const char *voteVarMap[] = {
	"clientkick",
	"resetrotation",
	"fraglimit",
	"g_doWarmup",
	"Falling",
	"g_friendlyFire",
	"g_gametype",
	"Hurtself",
	"map",
	"map_restart",
	"nextmap",
	"timelimit",
	"Instagib",
	"InstaWeapon",
	"g_footsteps",
	"Hook",
	"HookFireSpeed",
	"DisableBFG",
	"DisableQuad",
	"Spaceprotect",
	"sv_floodProtect",
	"vampire",
	"SpeedSpawn",
	"Spawnprotect",
};
static const int nVoteVarMap = sizeof(voteVarMap) / sizeof(const char *);

static const char *voteValues[] = {
	"<idnum>",
	"<0/1>",
	"<num>",
	"<0/1>",
	"<0/1>",
	"<0/1>",
	"<1-4>",
	"<0/1>",
	"<name>",
	"",
	"",
	"<min>",
	"<0/1>",
	"<1-3>",
	"<0/1>",
	"<0/1>",
	"<num>",
	"<0/1>",
	"<0/1>",
	"<0/1>",
	"<0/1>",
	"<0/1>",
	"<0/1>",
	"<num>",
};
static const int nVoteValues = sizeof(voteValues) / sizeof(const char *);

static const char *comVars[] = {
	"players",
	"speconly",
	"kill",
	"tossweapon",
	"tossammo",
	"tossflag",
	"cg_railtrailtime",
	"cg_drawgun",
	"cg_noprojectiletrail",
	"cg_qsparticles",
	"cg_delag",
	"weaponselstyle",
	"statsw",
	"motd",
};
static const int ncomVars = sizeof(comVars) / sizeof(const char *);

static const char *comValues[] = {
	"",
	"",
	"",
	"",
	"<num>",
	"",
	"<num>",
	"<0/1>",
	"<0/1>",
	"<0/1>",
	"<0/1>",
	"<0/1>",
	"",
	"",
};
static const int ncomValues = sizeof(comValues) / sizeof(const char *);

static void PrintVoteHelp(gentity_t * ent)
{
	int             i, j, k;
	int             i2, j2, k2;
	char            temp2[256];
	char            temp[256];
	const char     *spaces1 = "                ";
	const int       nSpaces1 = strlen(spaces1);
	const char     *spaces2 = "          ";
	const int       nSpaces2 = strlen(spaces2);
	const int       nCols = 3;

	const char     *spaces3 = "                ";
	const int       nSpaces3 = strlen(spaces3);
	const char     *spaces4 = "          ";
	const int       nSpaces4 = strlen(spaces4);
	const int       nCols2 = 3;

	trap_SendServerCommand(ent - g_entities,
						   "printc \"^2-----------------------------------------------------------------------------\n\"");
	trap_SendServerCommand(ent - g_entities, "printc \"^1                        Vote commands are as follows:\n\"");
	trap_SendServerCommand(ent - g_entities,
						   "printc \"^2-----------------------------------------------------------------------------\n\"");

	for(i = 0; i < nVoteVars; i++)
	{
		j = strlen(voteVars[i]);
		if(j > nSpaces1)
			j = nSpaces1;

		k = strlen(voteValues[i]);
		if(k > nSpaces2)
			k = nSpaces2;

		if(i % nCols == nCols - 1 || i == nVoteVars - 1)
			Com_sprintf(temp, sizeof(temp), "printc \"^3%s%s^5%s\n\"", voteVars[i], &spaces1[j], voteValues[i]);
		else
			Com_sprintf(temp, sizeof(temp), "printc \"^3%s%s^5%s%s\"", voteVars[i], &spaces1[j], voteValues[i], &spaces2[k]);

		trap_SendServerCommand(ent - g_entities, temp);
	}
	trap_SendServerCommand(ent - g_entities,
						   "printc \"^2-----------------------------------------------------------------------------\n\"");
	trap_SendServerCommand(ent - g_entities,
						   "printc \"^3->								         ^1Other usefull commands are as follows:^3         									<-\n\"");
	trap_SendServerCommand(ent - g_entities,
						   "printc \"^2-----------------------------------------------------------------------------\n\"");

	for(i2 = 0; i2 < ncomVars; i2++)
	{
		j2 = strlen(comVars[i2]);
		if(j2 > nSpaces3)
			j2 = nSpaces3;

		k2 = strlen(comValues[i2]);
		if(k2 > nSpaces4)
			k2 = nSpaces4;

		if(i2 % nCols == nCols - 1 || i2 == ncomVars - 1)
			Com_sprintf(temp2, sizeof(temp2), "printc \"^3%s%s^5%s\n\"", comVars[i2], &spaces3[j2], comValues[i2]);
		else
			Com_sprintf(temp2, sizeof(temp2), "printc \"^3%s%s^5%s%s\"", comVars[i2], &spaces3[j2], comValues[i2], &spaces4[k2]);

		trap_SendServerCommand(ent - g_entities, temp2);
	}

	trap_SendServerCommand(ent - g_entities,
						   "printc \"^2-----------------------------------------------------------------------------\n\"");
	trap_SendServerCommand(ent - g_entities,
						   "printc \"^3->								           ^1Use /help <command> for more info.  ^3         									<-\n\"");
	trap_SendServerCommand(ent - g_entities,
						   "printc \"^2-----------------------------------------------------------------------------\n\"");

}

static void PrintHelpCMDInfo(gentity_t * ent, const char *arg1)
{


	trap_SendServerCommand(ent - g_entities,
						   "printc \"^2-----------------------------------------------------------------------------\n\"");
	if(!Q_stricmp(arg1, "Kick"))
	{
		trap_SendServerCommand(ent - g_entities, "printc \"^5This will kick the client by thier client number or id number.\n\"");
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5Use ^3/players ^5for a list of players and thier id numbers. (^1You Can Not Kick Referees^5)\n\"");
	}
	else if(!Q_stricmp(arg1, "ResetRotation"))
	{
		trap_SendServerCommand(ent - g_entities, "printc \"^5ResetRotation will reset the server predefined map rotation.\n\"");
	}
	else if(!Q_stricmp(arg1, "FragLimit"))
	{
		trap_SendServerCommand(ent - g_entities, "printc \"^5Use this to set the score limit required to end the match.\n\"");
	}
	else if(!Q_stricmp(arg1, "DoWarmup"))
	{
		trap_SendServerCommand(ent - g_entities, "printc \"^5Set this to 1 to enable a pre-match warmup round.\n\"");
	}
	else if(!Q_stricmp(arg1, "FallingDamage"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5With this command set to 0 you won't recieve any falling damage from jumping or falling from high areas in the map.\n\"");
	}
	else if(!Q_stricmp(arg1, "FriendlyFire"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5If Friendlyfire is enabled then your team mates can recieve damage from your fire and you from theirs.\n\"");
	}
	else if(!Q_stricmp(arg1, "GameType"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5This command will set the gametype but the map needs to be restarted in order for the change to take place.\n\"");
		trap_SendServerCommand(ent - g_entities, "printc \"^5Valid gametype values are:\n\"");
		trap_SendServerCommand(ent - g_entities, "printc \"^50 = Free For All\n\"");
		trap_SendServerCommand(ent - g_entities, "printc \"^51 = Tournement\n\"");
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^52 = Single Player (^1not for network games for obvious reasons^5)\n\"");
		trap_SendServerCommand(ent - g_entities, "printc \"^53 = Team Death Match\n\"");
		trap_SendServerCommand(ent - g_entities, "printc \"^54 = Capture The Flag - Return The Flag\n\"");
	}
	else if(!Q_stricmp(arg1, "SelfDamage"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5If SelfDamage is set to 0 then you will not recieve any damage from your own weapons.\n\"");
	}
	else if(!Q_stricmp(arg1, "Map"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5Usage is /map <map name>... of course for ref or callvote you must add the ref or callvote commands first also if the gametype has been changed before this command but the map has not yet reloaded then this will change the current gametype to what ever it was set to when the new map loads.\n\"");
	}
	else if(!Q_stricmp(arg1, "Restart"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5This will restart the match... if the gametype has been changed before this command but the map has not yet reloaded then this will cause it to reload and will update the gametype in the process.\n\"");
	}
	else if(!Q_stricmp(arg1, "NextMap"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5If the server is running on a rotation then this will cause the next map set for play to be loaded skipping the current map... if the gametype has been changed before this command but the map has not yet reloaded then this will cause it to reload and will update the gametype in the process.\n\"");
	}
	else if(!Q_stricmp(arg1, "TimeLimit"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5Set this to enable a match timelimit. If set to 0 then the timelimit will be unlimited.\n\"");
	}
	else if(!Q_stricmp(arg1, "InstaGib"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5Set this to 1 to enable instagib type modes. Not all modes are insta but are done this way to be simple. (see ^3InstaWeapon ^5for available insta modes)\n\"");
	}
	else if(!Q_stricmp(arg1, "InstaWeapon"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5This will change the weapon configurations for instagib mode only.\n\"");
		trap_SendServerCommand(ent - g_entities, "printc \"^5Current weapon configurations are:\n\"");
		trap_SendServerCommand(ent - g_entities, "printc \"^50 = Gauntlet & Railgun (classic insta)\n\"");
		trap_SendServerCommand(ent - g_entities, "printc \"^51 = Gauntlet & Rocket Launcher\n\"");
		trap_SendServerCommand(ent - g_entities, "printc \"^52 = Gauntlet & Rocket Launcher & Railgun\n\"");
	}
	else if(!Q_stricmp(arg1, "FootSteps"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5With this set to 1 players will not generate any movement sounds unless falling from high places.\n\"");
	}
	else if(!Q_stricmp(arg1, "Hook"))
	{
		trap_SendServerCommand(ent - g_entities, "printc \"^5Enable this for the Grapple hook.\n\"");
	}
	else if(!Q_stricmp(arg1, "HookMissleSpeed"))
	{
		trap_SendServerCommand(ent - g_entities, "printc \"^5This will control how fast the grapple missle will travel.\n\"");
	}
	else if(!Q_stricmp(arg1, "DisableBFG"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5Since the BFG has been labeled by some as a (newbs) weapon... I have enabled this option so that it can be disabled in maps... But the BFG in this mod is far from what it was in Q3A.\n\"");
	}
	else if(!Q_stricmp(arg1, "DisableQUAD"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5Some more experienced players prefer to play without Quad damage so disable this if it causes any problems on the server.\n\"");
	}
	else if(!Q_stricmp(arg1, "SpaceProtection"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5With this enabled players who fall off of space maps or in to death pits will be teleported to a random spot in the map with out loosing any points and keeping any weapons they had when they fell.\n\"");
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5If the player is on the blue team or red team then he/she will be teleported back to his/her base and if he/she happens to be carrying a CTF flag then it will be returned to its base causing the fallen player to lose the flag.\n\"");
	}
	else if(!Q_stricmp(arg1, "FloodProtection"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5With Floodprotection enabled clients will not be able to send multiple chat commands to the server... but when disabled it will let multiple chat commands through and that can cause serious problems for some players connections. Enable this if a player is disrupting gameplay with useless chat or constant /bind say commands.\n\"");
	}
	else if(!Q_stricmp(arg1, "Vampire"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5Vampire mode will allow the attacker to pull some health out of his/her target.\n\"");
	}
	else if(!Q_stricmp(arg1, "SpeedSpawn"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5This command will make a player spawn instantly back into the world after he/she dies.\n\"");
	}
	else if(!Q_stricmp(arg1, "SpawnProtect"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5This setting will enable a protective shield over the player when they spawn in to the world.Set the value to the time you wish the players to be protected.\n\"");
	}
	else if(!Q_stricmp(arg1, "players"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5This command will list all connected players and their id numbers.\n\"");
	}
	else if(!Q_stricmp(arg1, "speconly"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5This setting will enable spectator only mode.When this mode is enabled the player will stay in spectator mode until either he/she disables it or the gametype is changed.\n\"");
	}
	else if(!Q_stricmp(arg1, "kill"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5This command will cause you to commit suicide and lose a point.\n\"");
	}
	else if(!Q_stricmp(arg1, "tossweapon"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5This will throw your current held weapon out in front of you.Only works in team games.\n\"");
	}
	else if(!Q_stricmp(arg1, "tossammo"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5This will throw what ever amount of ammo you set it at out in front of you.Only works in team games.\n\"");
	}
	else if(!Q_stricmp(arg1, "tossflag"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5If you are carrying the enemy flag then this will throw it out in front of you.Only works in CTF for obvious reasons.\n\"");
	}
	else if(!Q_stricmp(arg1, "cg_railtrailtime"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5This is the setting that dictates how long a rail trail will stay in the world.\n\"");
	}
	else if(!Q_stricmp(arg1, "cg_drawgun"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5Set this to either 1 or 0.When set to 1 the gun you hold will draw in first person and when set to 0 it will not draw.\n\"");
	}
	else if(!Q_stricmp(arg1, "cg_noprojectiletrail"))
	{
		trap_SendServerCommand(ent - g_entities, "printc \"^5When this is set to 1 no projectile trails will be drawn.\n\"");
	}
	else if(!Q_stricmp(arg1, "cg_qsparticles"))
	{
		trap_SendServerCommand(ent - g_entities, "printc \"^5When this is set to 0 no weapon particles will be drawn.\n\"");
	}
	else if(!Q_stricmp(arg1, "cg_delag"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5If the Unlagged code is enabled on the server you can take advantage of it by setting this to 1.\n\"");
	}
	else if(!Q_stricmp(arg1, "weaponselstyle"))
	{
		trap_SendServerCommand(ent - g_entities, "printc \"^5This will change how weapon select is drawn.\n\"");
	}
	else if(!Q_stricmp(arg1, "statsw"))
	{
		trap_SendServerCommand(ent - g_entities, "printc \"^5This will give you a short summary of your current statistics.\n\"");
	}
	else if(!Q_stricmp(arg1, "motd"))
	{
		trap_SendServerCommand(ent - g_entities, "printc \"^5This will show the servers motto of the day.\n\"");
	}
	else
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^1No info on this command.Use ^3/help ^1for a list of valid commands.\n\"");
	}
	trap_SendServerCommand(ent - g_entities,
						   "printc \"^2-----------------------------------------------------------------------------\n\"");

}

static const char *GetVoteVar(const char *cmd)
{
	int             i;

	for(i = 0; i < nVoteVars; i++)
	{
		if(Q_stricmp(cmd, voteVars[i]) == 0)
			return voteVarMap[i];
	}

	return "";
}

static qboolean CheckValid(const char *cmd, const char *value, char **err)
{
	int             i;

//  int         c;
	gentity_t      *ent2;
	gclient_t      *client2;
	char           *arg;


	// special case for g_gametype, check for bad values
	if(!Q_stricmp(cmd, "g_gametype"))
	{
		i = atoi(value);
		if(i == GT_SINGLE_PLAYER || i < GT_FFA || i >= GT_MAX_GAME_TYPE)
		{
			*err = "printc \"Invalid gametype.\n\"";
			return qfalse;
		}
	}
	else if(!Q_stricmp(cmd, "InstaWeapon"))
	{
		i = atoi(value);
		if(i < 0 || i > 2)
		{
			*err =
				"printc \"^3Invalid Weapontype. Valid Weapontypes are 0 = Rail Gun, 1 = Rocket Launcher, 2 = Particle Accelerator & Rocket Launcher.\n\"";
			return qfalse;
		}
	}
	else if(!Q_stricmp(cmd, "clientkick"))
	{
		i = atoi(value);
		arg = va("%s", value);
		ent2 = &g_entities[i];

		client2 = ent2->client;

		if(ClientNumberFromString2(arg) >= 0 && ent2->client->sess.ref == 1)
		{
			*err = "printc \"^1You cannot call a vote to kick a referee.\n\"";
			return qfalse;
		}
		else if(ClientNumberFromString2(arg) < 0)
		{
			*err = "printc \"^1Invalid id number. ^3Use /players for a list of client numbers..\n\"";
			return qfalse;
		}

	}
	else if(!Q_stricmp(cmd, "fraglimit") || !Q_stricmp(cmd, "timelimit") || !Q_stricmp(cmd, "SpawnProtect"))
	{
		i = atoi(value);
		if(i < 0)
		{
			*err = "printc \"Value must be a positive integer.\n\"";
			return qfalse;
		}
		if(value[0] < '0' || value[0] > '9')
		{
			*err = "printc \"Value must be a positive integer.\n\"";
			return qfalse;
		}
	}
	else if(!Q_stricmp(cmd, "g_doWarmup") || !Q_stricmp(cmd, "Falling") || !Q_stricmp(cmd, "g_friendlyFire"))
	{
		if(strlen(value) == 0 || strlen(value) > 1)
		{
			*err = "printc \"Valid values are 0 and 1.\n\"";
			return qfalse;
		}
		if(value[0] != '0' && value[0] != '1')
		{
			*err = "printc \"Valid values are 0 and 1.\n\"";
			return qfalse;
		}
	}
	else if(!Q_stricmp(cmd, "Hurtself") || !Q_stricmp(cmd, "sv_floodProtect"))
	{
		if(strlen(value) == 0 || strlen(value) > 1)
		{
			*err = "printc \"Valid values are 0, 1.\n\"";
			return qfalse;
		}
		if(value[0] != '0' && value[0] != '1' && value[0] != '2')
		{
			*err = "printc \"Valid values are 0, 1.\n\"";
			return qfalse;
		}
	}
	else if(!Q_stricmp(cmd, "map_restart") || !Q_stricmp(cmd, "map_restart"))
	{
		if(strlen(value) > 0)
		{
			*err = "printc \"This command requires no argument.\n\"";
			return qfalse;
		}
	}
	else if(!Q_stricmp(cmd, "Instagib"))
	{
		if(strlen(value) == 0 || strlen(value) > 1)
		{
			*err = "printc \"Valid values are 0, 1.\n\"";
			return qfalse;
		}
		if(value[0] != '0' && value[0] != '1' && value[0] != '2')
		{
			*err = "printc \"Valid values are 0, 1.\n\"";
			return qfalse;
		}
	}
	else if(!Q_stricmp(cmd, "g_footsteps"))
	{
		if(strlen(value) == 0 || strlen(value) > 1)
		{
			*err = "printc \"Valid values are 0, 1.\n\"";
			return qfalse;
		}
		if(value[0] != '0' && value[0] != '1' && value[0] != '2')
		{
			*err = "printc \"Valid values are 0, 1.\n\"";
			return qfalse;
		}
	}
	else if(!Q_stricmp(cmd, "HookFireSpeed"))
	{
		i = atoi(value);
		if(i < 0)
		{
			*err = "printc \"Value must be a positive integer.\n\"";
			return qfalse;
		}
	}
	else if(!Q_stricmp(cmd, "Hook") || !Q_stricmp(cmd, "DisableBFG") || !Q_stricmp(cmd, "Spaceprotect")
			|| !Q_stricmp(cmd, "DisableQuad") || !Q_stricmp(cmd, "Vampire") || !Q_stricmp(cmd, "SpeedSpawn"))
	{
		if(strlen(value) == 0 || strlen(value) > 1)
		{
			*err = "printc \"^3Valid values are 0, 1.\n\"";
			return qfalse;
		}
		if(value[0] != '0' && value[0] != '1' && value[0] != '2')
		{
			*err = "printc \"^3Valid values are 0, 1.\n\"";
			return qfalse;
		}
	}

	return qtrue;
}

/*
==================
Cmd_CallVote_f
==================
*/
static void Cmd_CallVote_f(gentity_t * ent)
{
	int             i;
	char            arg1[MAX_STRING_TOKENS];
	char            arg2[MAX_STRING_TOKENS];
	const char     *cmd;
	char           *err;

	if(nVoteVars != nVoteVarMap)
	{
		trap_SendServerCommand(ent - g_entities, "printc \"Sanity check failed: nVoteVars != nVoteVarMap\n\"");
		return;
	}

	if(!g_allowVote.integer)
	{
		trap_SendServerCommand(ent - g_entities, "printc \"Voting not allowed here.\n\"");
		return;
	}

	if(level.voteTime)
	{
		trap_SendServerCommand(ent - g_entities, "printc \"A vote is already in progress.\n\"");
		return;
	}
	if(ent->client->pers.voteCount >= MAX_VOTE_COUNT)
	{
		trap_SendServerCommand(ent - g_entities, "printc \"You have called the maximum number of votes.\n\"");
		return;
	}
	/*
	   if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
	   trap_SendServerCommand( ent-g_entities, "printc \"Not allowed to call a vote as spectator.\n\"" );
	   return;
	   } */

	// make sure it is a valid command to vote on
	trap_Argv(1, arg1, sizeof(arg1));
	trap_Argv(2, arg2, sizeof(arg2));

	if(strchr(arg1, ';') || strchr(arg2, ';'))
	{
		trap_SendServerCommand(ent - g_entities, "printc \"Invalid vote string.\n\"");
		return;
	}

	cmd = GetVoteVar(arg1);
	if(cmd[0] == '\0')
	{
		PrintVoteHelp(ent);
		return;
	}

	// if there is still a vote to be executed
	if(level.voteExecuteTime)
	{
		level.voteExecuteTime = 0;
		trap_SendConsoleCommand(EXEC_APPEND, va("%s\n", level.voteString));
	}

	if(!CheckValid(cmd, arg2, &err))
	{
		trap_SendServerCommand(ent - g_entities, err);
		return;
	}

	if(!Q_stricmp(cmd, "map"))
	{
		// special case for map changes, we want to reset the nextmap setting
		// this allows a player to change maps, but not upset the map rotation
		char            s[MAX_STRING_CHARS];

		trap_Cvar_VariableStringBuffer("nextmap", s, sizeof(s));
		if(*s)
		{
			Com_sprintf(level.voteString, sizeof(level.voteString), "%s %s; set nextmap \"%s\"", cmd, arg2, s);
		}
		else
		{
			Com_sprintf(level.voteString, sizeof(level.voteString), "%s %s", cmd, arg2);
		}
		Com_sprintf(level.ExecuteString, sizeof(level.ExecuteString), "map");
	}
	else if(!Q_stricmp(cmd, "nextmap"))
	{
		char            s[MAX_STRING_CHARS];

		trap_Cvar_VariableStringBuffer("nextmap", s, sizeof(s));
		if(!*s)
		{
			trap_SendServerCommand(ent - g_entities, "printc \"nextmap not set.\n\"");
			return;
		}
		Com_sprintf(level.ExecuteString, sizeof(level.ExecuteString), "map");
		Com_sprintf(level.voteString, sizeof(level.voteString), "vstr nextmap");
	}
	else if(!Q_stricmp(cmd, "resetrotation"))
	{
		Com_sprintf(level.voteString, sizeof(level.voteString), "set nextmap \"vstr d1\"");
		Com_sprintf(level.ExecuteString, sizeof(level.ExecuteString), "map");
	}
	else
	{
		Com_sprintf(level.voteString, sizeof(level.voteString), "%s \"%s\"", cmd, arg2);
	}

	if(!Q_stricmp(cmd, "g_gametype"))
	{
		i = atoi(arg2);
		Com_sprintf(level.voteDisplayString, sizeof(level.voteDisplayString), "%s %s", cmd, gameNames[i]);
	}
	else
	{
		Com_sprintf(level.voteDisplayString, sizeof(level.voteDisplayString), "%s", level.voteString);
	}

	trap_SendServerCommand(-1, va("print \"%s ^Ncalled a vote.", ent->client->pers.netname));

	// start the voting, the caller autoamtically votes yes
	level.voteTime = level.time;
	if(ent->client->ps.persistant[PERS_REF] == 1)
	{
		//  trap_SendServerCommand( ent-g_entities, "print \"ref vote.\n\"" );
		level.voteRYes = 1;
		level.voteRNo = 0;
	}
	else
	{
		level.voteYes = 1;
		level.voteNo = 0;
	}

	for(i = 0; i < level.maxclients; i++)
	{
		level.clients[i].ps.eFlags &= ~EF_VOTED;
	}
	ent->client->ps.eFlags |= EF_VOTED;

	trap_SetConfigstring(CS_VOTE_TIME, va("%i", level.voteTime));
	trap_SetConfigstring(CS_VOTE_STRING, level.voteDisplayString);
	if(ent->client->ps.persistant[PERS_REF] == 1)
	{
		trap_SetConfigstring(CS_VOTE_YES, va("%i", level.voteRYes));
		trap_SetConfigstring(CS_VOTE_NO, va("%i", level.voteRNo));
	}
	else
	{
		trap_SetConfigstring(CS_VOTE_YES, va("%i", level.voteYes));
		trap_SetConfigstring(CS_VOTE_NO, va("%i", level.voteNo));
	}
}

static void Cmd_help(gentity_t * ent)
{
	char            arg1[MAX_STRING_TOKENS];


	// make sure it is a valid command
	trap_Argv(1, arg1, sizeof(arg1));

	if(!Q_stricmp(arg1, ""))
	{
		PrintVoteHelp(ent);
		return;
	}
	else if(Q_stricmp(arg1, ""))
	{
		PrintHelpCMDInfo(ent, arg1);
		return;
	}





}

/*
==================
Cmd_Vote_f
==================
*/
static void Cmd_Vote_f(gentity_t * ent)
{
	char            msg[64];

	if(!level.voteTime)
	{
		trap_SendServerCommand(ent - g_entities, "printc \"No vote in progress.\n\"");
		return;
	}
	if(ent->client->ps.eFlags & EF_VOTED)
	{
		trap_SendServerCommand(ent - g_entities, "printc \"Vote already cast.\n\"");
		return;
	}
	if(ent->client->sess.sessionTeam == TEAM_SPECTATOR)
	{
		trap_SendServerCommand(ent - g_entities, "printc \"Not allowed to vote as spectator.\n\"");
		return;
	}

	trap_SendServerCommand(ent - g_entities, "print \"Vote cast.");

	ent->client->ps.eFlags |= EF_VOTED;

	trap_Argv(1, msg, sizeof(msg));

	if(msg[0] == 'y' || msg[1] == 'Y' || msg[1] == '1')
	{
		if(ent->client->ps.persistant[PERS_REF] == 1)
		{
			level.voteRYes = 1;
			trap_SetConfigstring(CS_VOTE_YES, va("%i", level.voteRYes));
		}
		else
		{
			level.voteYes++;
			trap_SetConfigstring(CS_VOTE_YES, va("%i", level.voteYes));
		}
	}
	else
	{
		if(ent->client->ps.persistant[PERS_REF] == 1)
		{
			level.voteRNo = 1;
			trap_SetConfigstring(CS_VOTE_NO, va("%i", level.voteRNo));
		}
		else
		{
			level.voteNo++;
			trap_SetConfigstring(CS_VOTE_NO, va("%i", level.voteNo));
		}
	}

	// a majority will be determined in CheckVote, which will also account
	// for players entering or leaving
}

/*
==================
Cmd_CallTeamVote_f
==================
*/
static void Cmd_CallTeamVote_f(gentity_t * ent)
{
	int             i, team, cs_offset;
	char            arg1[MAX_STRING_TOKENS];
	char            arg2[MAX_STRING_TOKENS];

	team = ent->client->sess.sessionTeam;
	if(team == TEAM_RED)
		cs_offset = 0;
	else if(team == TEAM_BLUE)
		cs_offset = 1;
	else
		return;

	if(!g_allowVote.integer)
	{
		trap_SendServerCommand(ent - g_entities, "printc \"Voting not allowed here.\n\"");
		return;
	}

	if(level.teamVoteTime[cs_offset])
	{
		trap_SendServerCommand(ent - g_entities, "printc \"A team vote is already in progress.\n\"");
		return;
	}
	if(ent->client->pers.teamVoteCount >= MAX_VOTE_COUNT)
	{
		trap_SendServerCommand(ent - g_entities, "printc \"You have called the maximum number of team votes.\n\"");
		return;
	}
	if(ent->client->sess.sessionTeam == TEAM_SPECTATOR)
	{
		trap_SendServerCommand(ent - g_entities, "printc \"Not allowed to call a vote as spectator.\n\"");
		return;
	}

	// make sure it is a valid command to vote on
	trap_Argv(1, arg1, sizeof(arg1));
	arg2[0] = '\0';
	for(i = 2; i < trap_Argc(); i++)
	{
		if(i > 2)
			strcat(arg2, " ");
		trap_Argv(i, &arg2[strlen(arg2)], sizeof(arg2) - strlen(arg2));
	}

	if(strchr(arg1, ';') || strchr(arg2, ';'))
	{
		trap_SendServerCommand(ent - g_entities, "printc \"Invalid vote string.\n\"");
		return;
	}

	if(!Q_stricmp(arg1, "leader"))
	{
		char            netname[MAX_NETNAME], leader[MAX_NETNAME];

		if(!arg2[0])
		{
			i = ent->client->ps.clientNum;
		}
		else
		{
			// numeric values are just slot numbers
			for(i = 0; i < 3; i++)
			{
				if(!arg2[i] || arg2[i] < '0' || arg2[i] > '9')
					break;
			}
			if(i >= 3 || !arg2[i])
			{
				i = atoi(arg2);
				if(i < 0 || i >= level.maxclients)
				{
					trap_SendServerCommand(ent - g_entities, va("printc \"Bad client slot: %i\n\"", i));
					return;
				}

				if(!g_entities[i].inuse)
				{
					trap_SendServerCommand(ent - g_entities, va("printc \"Client %i is not active\n\"", i));
					return;
				}
			}
			else
			{
				Q_strncpyz(leader, arg2, sizeof(leader));
				Q_CleanStr(leader);
				for(i = 0; i < level.maxclients; i++)
				{
					if(level.clients[i].pers.connected == CON_DISCONNECTED)
						continue;
					if(level.clients[i].sess.sessionTeam != team)
						continue;
					Q_strncpyz(netname, level.clients[i].pers.netname, sizeof(netname));
					Q_CleanStr(netname);
					if(!Q_stricmp(netname, leader))
					{
						break;
					}
				}
				if(i >= level.maxclients)
				{
					trap_SendServerCommand(ent - g_entities, va("printc \"%s is not a valid player on your team.\n\"", arg2));
					return;
				}
			}
		}
		Com_sprintf(arg2, sizeof(arg2), "%d", i);
	}
	else
	{
		trap_SendServerCommand(ent - g_entities, "printc \"Invalid vote string.\n\"");
		trap_SendServerCommand(ent - g_entities, "printc \"Team vote commands are: leader <player>.\n\"");
		return;
	}

	Com_sprintf(level.teamVoteString[cs_offset], sizeof(level.teamVoteString[cs_offset]), "%s %s", arg1, arg2);

	for(i = 0; i < level.maxclients; i++)
	{
		if(level.clients[i].pers.connected == CON_DISCONNECTED)
			continue;
		if(level.clients[i].sess.sessionTeam == team)
			trap_SendServerCommand(i, va("print \"%s ^Ncalled a team vote.", ent->client->pers.netname));
	}

	// start the voting, the caller autoamtically votes yes
	level.teamVoteTime[cs_offset] = level.time;
	level.teamVoteYes[cs_offset] = 1;
	level.teamVoteNo[cs_offset] = 0;

	for(i = 0; i < level.maxclients; i++)
	{
		if(level.clients[i].sess.sessionTeam == team)
			level.clients[i].ps.eFlags &= ~EF_TEAMVOTED;
	}
	ent->client->ps.eFlags |= EF_TEAMVOTED;

	trap_SetConfigstring(CS_TEAMVOTE_TIME + cs_offset, va("%i", level.teamVoteTime[cs_offset]));
	trap_SetConfigstring(CS_TEAMVOTE_STRING + cs_offset, level.teamVoteString[cs_offset]);
	trap_SetConfigstring(CS_TEAMVOTE_YES + cs_offset, va("%i", level.teamVoteYes[cs_offset]));
	trap_SetConfigstring(CS_TEAMVOTE_NO + cs_offset, va("%i", level.teamVoteNo[cs_offset]));
}

/*
==================
Cmd_TeamVote_f
==================
*/
static void Cmd_TeamVote_f(gentity_t * ent)
{
	int             team, cs_offset;
	char            msg[64];

	team = ent->client->sess.sessionTeam;
	if(team == TEAM_RED)
		cs_offset = 0;
	else if(team == TEAM_BLUE)
		cs_offset = 1;
	else
		return;

	if(!level.teamVoteTime[cs_offset])
	{
		trap_SendServerCommand(ent - g_entities, "printc \"No team vote in progress.\n\"");
		return;
	}
	if(ent->client->ps.eFlags & EF_TEAMVOTED)
	{
		trap_SendServerCommand(ent - g_entities, "printc \"Team vote already cast.\n\"");
		return;
	}
	if(ent->client->sess.sessionTeam == TEAM_SPECTATOR)
	{
		trap_SendServerCommand(ent - g_entities, "printc \"Not allowed to vote as spectator.\n\"");
		return;
	}

	trap_SendServerCommand(ent - g_entities, "print \"Team vote cast.");

	ent->client->ps.eFlags |= EF_TEAMVOTED;

	trap_Argv(1, msg, sizeof(msg));

	if(msg[0] == 'y' || msg[1] == 'Y' || msg[1] == '1')
	{
		level.teamVoteYes[cs_offset]++;
		trap_SetConfigstring(CS_TEAMVOTE_YES + cs_offset, va("%i", level.teamVoteYes[cs_offset]));
	}
	else
	{
		level.teamVoteNo[cs_offset]++;
		trap_SetConfigstring(CS_TEAMVOTE_NO + cs_offset, va("%i", level.teamVoteNo[cs_offset]));
	}

	// a majority will be determined in TeamCheckVote, which will also account
	// for players entering or leaving
}


/*
=================
Cmd_SetViewpos_f
=================
*/
static void Cmd_SetViewpos_f(gentity_t * ent)
{
	vec3_t          origin, angles;
	char            buffer[MAX_TOKEN_CHARS];
	int             i;

	if(!g_cheats.integer)
	{
		trap_SendServerCommand(ent - g_entities, va("printc \"Cheats are not enabled on this server.\n\""));
		return;
	}
	if(trap_Argc() != 5)
	{
		trap_SendServerCommand(ent - g_entities, va("printc \"usage: setviewpos x y z yaw\n\""));
		return;
	}

	VectorClear(angles);
	for(i = 0; i < 3; i++)
	{
		trap_Argv(i + 1, buffer, sizeof(buffer));
		origin[i] = atof(buffer);
	}

	trap_Argv(4, buffer, sizeof(buffer));
	angles[YAW] = atof(buffer);

	TeleportPlayer(ent, origin, angles);
}

static void Cmd_Stats2_f(gentity_t * ent)
{
	int             hits, shots, accuracy;
	int             hitsrg, shotsrg, accuracyrg;
	int             hitsrl, shotsrl, accuracyrl;
	int             hitspg, shotspg, accuracypg;
	int             hitsgl, shotsgl, accuracygl;
	int             hitssg, shotssg, accuracysg;
	int             hitsmg, shotsmg, accuracymg;
	int             hitslg, shotslg, accuracylg;
	int             hitsbfg, shotsbfg, accuracybfg;
	gclient_t      *client;


	client = &level.clients[ent->client->ps.clientNum];

// railgun stats
	if(client->accuracy_shot[WP_RAILGUN])
	{
		hitsrg = client->accuracy_hit[WP_RAILGUN];
		shotsrg = client->accuracy_shot[WP_RAILGUN];
		accuracyrg = hitsrg * 100 / shotsrg;
	}
	else
	{
		hitsrg = 0;
		shotsrg = 0;
		accuracyrg = 0;
	}

//rocket launcher stats
	if(client->accuracy_shot[WP_ROCKET_LAUNCHER])
	{
		hitsrl = client->accuracy_hit[WP_ROCKET_LAUNCHER];
		shotsrl = client->accuracy_shot[WP_ROCKET_LAUNCHER];
		accuracyrl = hitsrl * 100 / shotsrl;
	}
	else
	{
		hitsrl = 0;
		shotsrl = 0;
		accuracyrl = 0;
	}

//grenade launcher stats
	if(client->accuracy_shot[WP_GRENADE_LAUNCHER])
	{
		hitsgl = client->accuracy_hit[WP_GRENADE_LAUNCHER];
		shotsgl = client->accuracy_shot[WP_GRENADE_LAUNCHER];
		accuracygl = hitsgl * 100 / shotsgl;
	}
	else
	{
		hitsgl = 0;
		shotsgl = 0;
		accuracygl = 0;
	}

//plasmagun stats
	if(client->accuracy_shot[WP_PLASMAGUN])
	{
		hitspg = client->accuracy_hit[WP_PLASMAGUN];
		shotspg = client->accuracy_shot[WP_PLASMAGUN];
		accuracypg = hitspg * 100 / shotspg;
	}
	else
	{
		hitspg = 0;
		shotspg = 0;
		accuracypg = 0;
	}

//machinegun stats
	if(client->accuracy_shot[WP_MACHINEGUN])
	{
		hitsmg = client->accuracy_hit[WP_MACHINEGUN];
		shotsmg = client->accuracy_shot[WP_MACHINEGUN];
		accuracymg = hitsmg * 100 / shotsmg;
	}
	else
	{
		hitsmg = 0;
		shotsmg = 0;
		accuracymg = 0;
	}



//shotgun stats
	if(client->accuracy_shot[WP_SHOTGUN])
	{
		hitssg = client->accuracy_hit[WP_SHOTGUN];
		shotssg = client->accuracy_shot[WP_SHOTGUN] * 8;
		accuracysg = hitssg * 100 / shotssg;
	}
	else
	{
		hitssg = 0;
		shotssg = 0;
		accuracysg = 0;
	}


//lightning gun stats
	if(client->accuracy_shot[WP_LIGHTNING])
	{
		hitslg = client->accuracy_hit[WP_LIGHTNING];
		shotslg = client->accuracy_shot[WP_LIGHTNING];
		accuracylg = hitslg * 100 / shotslg;
	}
	else
	{
		hitslg = 0;
		shotslg = 0;
		accuracylg = 0;
	}


//bfg stats
	if(client->accuracy_shot[WP_BFG])
	{
		hitsbfg = client->accuracy_hit[WP_BFG];
		shotsbfg = client->accuracy_shot[WP_BFG];
		accuracybfg = hitsbfg * 100 / shotsbfg;
	}
	else
	{
		hitsbfg = 0;
		shotsbfg = 0;
		accuracybfg = 0;
	}

// total hits/shots/accuracy
	if(client->accuracy_shots)
	{
		hits = client->accuracy_hits;
		shots = client->accuracy_shots;
		accuracy = hits * 100 / shots;
	}
	else
	{
		hits = 0;
		shots = 0;
		accuracy = 0;
	};



	trap_SendServerCommand(ent - g_entities,
						   va("stinfo %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i", client->ps.clientNum, hitsmg, shotsmg,
							  hitsrl, shotsrl, hitsgl, shotsgl, hitspg, shotspg, hitsrg, shotsrg, hitslg, shotslg, hitssg,
							  shotssg, hitsbfg, shotsbfg));

}

void Cmd_Statistics_f(gentity_t * ent)
{
	int             pickupgl, pickuprl, pickupmg, pickuprg, pickuplg, pickuppg, pickupbfg, pickupsg;
	int             pickupmega, pickupyarmor, pickuprarmor, pickupshard, pickup5h, pickup20h, pickup40h;
	int             pickupquad, pickupsuit, pickuphaste, pickupregen, pickupinvis, pickuprflag, pickupbflag, pickupflight;
	int             hitsrg, shotsrg, accuracyrg;
	int             hitsrl, shotsrl, accuracyrl;
	int             hitspg, shotspg, accuracypg;
	int             hitsgl, shotsgl, accuracygl;
	int             hitssg, shotssg, accuracysg;
	int             hitsmg, shotsmg, accuracymg;
	int             hitslg, shotslg, accuracylg;
	int             hitsbfg, shotsbfg, accuracybfg;
	int             pickupHealtht, pickupArmort;
	int             damagert, damagegt;
	gclient_t      *client;

	client = &level.clients[ent->client->ps.clientNum];



	if(client->pickupHealthT)
	{
		pickupHealtht = client->pickupHealthT;
	}
	else
	{
		pickupHealtht = 0;
	}

	if(client->pickupArmorT)
	{
		pickupArmort = client->pickupArmorT;
	}
	else
	{
		pickupArmort = 0;
	}

	if(client->damageRT)
	{
		damagert = client->damageRT;
	}
	else
	{
		damagert = 0;
	}

	if(client->damageGT)
	{
		damagegt = client->damageGT;
	}
	else
	{
		damagegt = 0;
	}

// railgun stats
	if(client->accuracy_shot[WP_RAILGUN])
	{
		hitsrg = client->accuracy_hit[WP_RAILGUN];
		shotsrg = client->accuracy_shot[WP_RAILGUN];
		accuracyrg = hitsrg * 100 / shotsrg;
	}
	else
	{
		hitsrg = 0;
		shotsrg = 0;
		accuracyrg = 0;
	}

//rocket launcher stats
	if(client->accuracy_shot[WP_ROCKET_LAUNCHER])
	{
		hitsrl = client->accuracy_hit[WP_ROCKET_LAUNCHER];
		shotsrl = client->accuracy_shot[WP_ROCKET_LAUNCHER];
		accuracyrl = hitsrl * 100 / shotsrl;
	}
	else
	{
		hitsrl = 0;
		shotsrl = 0;
		accuracyrl = 0;
	}

//grenade launcher stats
	if(client->accuracy_shot[WP_GRENADE_LAUNCHER])
	{
		hitsgl = client->accuracy_hit[WP_GRENADE_LAUNCHER];
		shotsgl = client->accuracy_shot[WP_GRENADE_LAUNCHER];
		accuracygl = hitsgl * 100 / shotsgl;
	}
	else
	{
		hitsgl = 0;
		shotsgl = 0;
		accuracygl = 0;
	}

//plasmagun stats
	if(client->accuracy_shot[WP_PLASMAGUN])
	{
		hitspg = client->accuracy_hit[WP_PLASMAGUN];
		shotspg = client->accuracy_shot[WP_PLASMAGUN];
		accuracypg = hitspg * 100 / shotspg;
	}
	else
	{
		hitspg = 0;
		shotspg = 0;
		accuracypg = 0;
	}

//machinegun stats
	if(client->accuracy_shot[WP_MACHINEGUN])
	{
		hitsmg = client->accuracy_hit[WP_MACHINEGUN];
		shotsmg = client->accuracy_shot[WP_MACHINEGUN];
		accuracymg = hitsmg * 100 / shotsmg;
	}
	else
	{
		hitsmg = 0;
		shotsmg = 0;
		accuracymg = 0;
	}



//shotgun stats
	if(client->accuracy_shot[WP_SHOTGUN])
	{
		hitssg = client->accuracy_hit[WP_SHOTGUN];
		shotssg = client->accuracy_shot[WP_SHOTGUN] * 8;
		accuracysg = hitssg * 100 / shotssg;
	}
	else
	{
		hitssg = 0;
		shotssg = 0;
		accuracysg = 0;
	}


//lightning gun stats
	if(client->accuracy_shot[WP_LIGHTNING])
	{
		hitslg = client->accuracy_hit[WP_LIGHTNING];
		shotslg = client->accuracy_shot[WP_LIGHTNING];
		accuracylg = hitslg * 100 / shotslg;
	}
	else
	{
		hitslg = 0;
		shotslg = 0;
		accuracylg = 0;
	}


//bfg stats
	if(client->accuracy_shot[WP_BFG])
	{
		hitsbfg = client->accuracy_hit[WP_BFG];
		shotsbfg = client->accuracy_shot[WP_BFG];
		accuracybfg = hitsbfg * 100 / shotsbfg;
	}
	else
	{
		hitsbfg = 0;
		shotsbfg = 0;
		accuracybfg = 0;
	}

// weapon pickups
	if(client->pickup[WP_GRENADE_LAUNCHER] > 0)
	{
		pickupgl = client->pickup[WP_GRENADE_LAUNCHER];
	}
	else
	{
		pickupgl = 0;
	}

	if(client->pickup[WP_MACHINEGUN] > 0)
	{
		pickupmg = client->pickup[WP_MACHINEGUN];
	}
	else
	{
		pickupmg = 0;
	}

	if(client->pickup[WP_LIGHTNING] > 0)
	{
		pickuplg = client->pickup[WP_LIGHTNING];
	}
	else
	{
		pickuplg = 0;
	}

	if(client->pickup[WP_SHOTGUN] > 0)
	{
		pickupsg = client->pickup[WP_SHOTGUN];
	}
	else
	{
		pickupsg = 0;
	}

	if(client->pickup[WP_RAILGUN] > 0)
	{
		pickuprg = client->pickup[WP_RAILGUN];
	}
	else
	{
		pickuprg = 0;
	}

	if(client->pickup[WP_ROCKET_LAUNCHER] > 0)
	{
		pickuprl = client->pickup[WP_ROCKET_LAUNCHER];
	}
	else
	{
		pickuprl = 0;
	}

	if(client->pickup[WP_PLASMAGUN] > 0)
	{
		pickuppg = client->pickup[WP_PLASMAGUN];
	}
	else
	{
		pickuppg = 0;
	}

	if(client->pickup[WP_BFG] > 0)
	{
		pickupbfg = client->pickup[WP_BFG];
	}
	else
	{
		pickupbfg = 0;
	}


// health pickups
	if(client->pickupmega > 0)
	{
		pickupmega = client->pickupmega;
	}
	else
	{
		pickupmega = 0;
	}

	if(client->pickup5h > 0)
	{
		pickup5h = client->pickup5h;
	}
	else
	{
		pickup5h = 0;
	}

	if(client->pickup20h > 0)
	{
		pickup20h = client->pickup20h;
	}
	else
	{
		pickup20h = 0;
	}

	if(client->pickup40h > 0)
	{
		pickup40h = client->pickup40h;
	}
	else
	{
		pickup40h = 0;
	}

// armor pickups
	if(client->pickuprarmor > 0)
	{
		pickuprarmor = client->pickuprarmor;
	}
	else
	{
		pickuprarmor = 0;
	}

	if(client->pickupyarmor > 0)
	{
		pickupyarmor = client->pickupyarmor;
	}
	else
	{
		pickupyarmor = 0;
	}

	if(client->pickupshard > 0)
	{
		pickupshard = client->pickupshard;
	}
	else
	{
		pickupshard = 0;
	}

// powerup pickups
	if(client->pickupquad > 0)
	{
		pickupquad = client->pickupquad;
	}
	else
	{
		pickupquad = 0;
	}

	if(client->pickupsuit > 0)
	{
		pickupsuit = client->pickupsuit;
	}
	else
	{
		pickupsuit = 0;
	}

	if(client->pickuphaste > 0)
	{
		pickuphaste = client->pickuphaste;
	}
	else
	{
		pickuphaste = 0;
	}

	if(client->pickupregen > 0)
	{
		pickupregen = client->pickupregen;
	}
	else
	{
		pickupregen = 0;
	}

	if(client->pickupinvis > 0)
	{
		pickupinvis = client->pickupinvis;
	}
	else
	{
		pickupinvis = 0;
	}

	if(client->pickupflight > 0)
	{
		pickupflight = client->pickupflight;
	}
	else
	{
		pickupflight = 0;
	}

// flag pickups
	if(client->pickuprflag > 0)
	{
		pickuprflag = client->pickuprflag;
	}
	else
	{
		pickuprflag = 0;
	}

	if(client->pickupbflag > 0)
	{
		pickupbflag = client->pickupbflag;
	}
	else
	{
		pickupbflag = 0;
	}


// now send this crap to the client for stats
	trap_SendServerCommand(ent - g_entities,
						   va
						   ("fstinfo %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i",
							client->ps.clientNum, hitsmg, shotsmg, hitsrl, shotsrl, hitsgl, shotsgl, hitspg, shotspg, hitsrg,
							shotsrg, hitslg, shotslg, hitssg, shotssg, hitsbfg, shotsbfg, pickupgl, pickuprl, pickupmg, pickuprg,
							pickuplg, pickuppg, pickupbfg, pickupsg, pickupmega, pickupyarmor, pickuprarmor, pickupshard,
							pickup5h, pickup20h, pickup40h, pickupquad, pickupsuit, pickuphaste, pickupregen, pickupinvis,
							pickuprflag, pickupbflag, pickupflight, pickupArmort, pickupHealtht, damagert, damagegt));

}

/*
==================
REF MODE
==================
*/

static const char *refVars[] = {
	"Kick",
	"ResetRotation",
	"FragLimit",
	"DoWarmup",
	"FallingDamage",
	"FriendlyFire",
	"GameType",
	"SelfDamage",
	"Map",
	"Restart",
	"NextMap",
	"TimeLimit",
	"InstaGib",
	"InstaWeapon",
	"FootSteps",
	"Hook",
	"HookMissleSpeed",
	"DisableBFG",
	"DisableQUAD",
	"SpaceProtection",
	"FloodProtection",
	"Vampire",
	"SpeedSpawn",
	"SpawnProtect",
};
static const int nRefVars = sizeof(refVars) / sizeof(const char *);

static const char *refVarMap[] = {
	"clientkick",
	"resetrotation",
	"fraglimit",
	"g_doWarmup",
	"Falling",
	"g_friendlyFire",
	"g_gametype",
	"Hurtself",
	"map",
	"map_restart",
	"nextmap",
	"timelimit",
	"Instagib",
	"InstaWeapon",
	"g_footsteps",
	"Hook",
	"HookFireSpeed",
	"DisableBFG",
	"DisableQuad",
	"Spaceprotect",
	"sv_floodProtect",
	"vampire",
	"SpeedSpawn",
	"SpawnProtect",
};
static const int nRefVarMap = sizeof(refVarMap) / sizeof(const char *);

static const char *refValues[] = {
	"<idnum>",
	"<0/1>",
	"<num>",
	"<0/1>",
	"<0/1>",
	"<0/1>",
	"<1-4>",
	"<0/1>",
	"<name>",
	"",
	"",
	"<min>",
	"<0/1>",
	"<1-3>",
	"<0/1>",
	"<0/1>",
	"<num>",
	"<0/1>",
	"<0/1>",
	"<0/1>",
	"<0/1>",
	"<0/1>",
	"<0/1>",
	"<num>",
};
static const int nRefValues = sizeof(refValues) / sizeof(const char *);

static void PrintRefHelp(gentity_t * ent)
{
	int             i, j, k;
	char            temp[256];
	const char     *spaces1 = "                ";
	const int       nSpaces1 = strlen(spaces1);
	const char     *spaces2 = "          ";
	const int       nSpaces2 = strlen(spaces2);
	const int       nCols = 3;

	trap_SendServerCommand(ent - g_entities,
						   "printc \"^2-----------------------------------------------------------------------------\n\"");
	trap_SendServerCommand(ent - g_entities, "printc \"^1                        Ref commands are as follows:\n\"");
	trap_SendServerCommand(ent - g_entities,
						   "printc \"^2-----------------------------------------------------------------------------\n\"");
	for(i = 0; i < nRefVars; i++)
	{
		j = strlen(refVars[i]);
		if(j > nSpaces1)
			j = nSpaces1;

		k = strlen(refValues[i]);
		if(k > nSpaces2)
			k = nSpaces2;

		if(i % nCols == nCols - 1 || i == nRefVars - 1)
			Com_sprintf(temp, sizeof(temp), "printc \"^3%s%s^5%s\n\"", refVars[i], &spaces1[j], refValues[i]);
		else
			Com_sprintf(temp, sizeof(temp), "printc \"^3%s%s^5%s%s\"", refVars[i], &spaces1[j], refValues[i], &spaces2[k]);

		trap_SendServerCommand(ent - g_entities, temp);
	}
	trap_SendServerCommand(ent - g_entities,
						   "printc \"^2-----------------------------------------------------------------------------\n\"");
	trap_SendServerCommand(ent - g_entities,
						   "printc \"^3->								         ^1Use /ref help <command> for more info.^3         									<-\n\"");
	trap_SendServerCommand(ent - g_entities,
						   "printc \"^2-----------------------------------------------------------------------------\n\"");

}

static void PrintRefCMDInfo(gentity_t * ent, const char *arg1)
{


	trap_SendServerCommand(ent - g_entities,
						   "printc \"^2-----------------------------------------------------------------------------\n\"");
	if(!Q_stricmp(arg1, "Kick"))
	{
		trap_SendServerCommand(ent - g_entities, "printc \"^5This will kick the client by thier client number or id number.\n\"");
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5Use ^3/players ^5for a list of players and thier id numbers. (^1You Can Not Kick Referees^5)\n\"");
	}
	else if(!Q_stricmp(arg1, "ResetRotation"))
	{
		trap_SendServerCommand(ent - g_entities, "printc \"^5ResetRotation will reset the server predefined map rotation.\n\"");
	}
	else if(!Q_stricmp(arg1, "FragLimit"))
	{
		trap_SendServerCommand(ent - g_entities, "printc \"^5Use this to set the score limit required to end the match.\n\"");
	}
	else if(!Q_stricmp(arg1, "DoWarmup"))
	{
		trap_SendServerCommand(ent - g_entities, "printc \"^5Set this to 1 to enable a pre-match warmup round.\n\"");
	}
	else if(!Q_stricmp(arg1, "FallingDamage"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5With this command set to 0 you won't recieve any falling damage from jumping or falling from high areas in the map.\n\"");
	}
	else if(!Q_stricmp(arg1, "FriendlyFire"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5If Friendlyfire is enabled then your team mates can recieve damage from your fire and you from theirs.\n\"");
	}
	else if(!Q_stricmp(arg1, "GameType"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5This command will set the gametype but the map needs to be restarted in order for the change to take place.\n\"");
		trap_SendServerCommand(ent - g_entities, "printc \"^5Valid gametype values are:\n\"");
		trap_SendServerCommand(ent - g_entities, "printc \"^50 = Free For All\n\"");
		trap_SendServerCommand(ent - g_entities, "printc \"^51 = Tournement\n\"");
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^52 = Single Player (^1not for network games for obvious reasons^5)\n\"");
		trap_SendServerCommand(ent - g_entities, "printc \"^53 = Team Death Match\n\"");
		trap_SendServerCommand(ent - g_entities, "printc \"^54 = Capture The Flag - Return The Flag\n\"");
	}
	else if(!Q_stricmp(arg1, "SelfDamage"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5If SelfDamage is set to 0 then you will not recieve any damage from your own weapons.\n\"");
	}
	else if(!Q_stricmp(arg1, "Map"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5Usage is /map <map name>... of course for ref or callvote you must add the ref or callvote commands first also if the gametype has been changed before this command but the map has not yet reloaded then this will change the current gametype to what ever it was set to when the new map loads.\n\"");
	}
	else if(!Q_stricmp(arg1, "Restart"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5This will restart the match... if the gametype has been changed before this command but the map has not yet reloaded then this will cause it to reload and will update the gametype in the process.\n\"");
	}
	else if(!Q_stricmp(arg1, "NextMap"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5If the server is running on a rotation then this will cause the next map set for play to be loaded skipping the current map... if the gametype has been changed before this command but the map has not yet reloaded then this will cause it to reload and will update the gametype in the process.\n\"");
	}
	else if(!Q_stricmp(arg1, "TimeLimit"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5Set this to enable a match timelimit. If set to 0 then the timelimit will be unlimited.\n\"");
	}
	else if(!Q_stricmp(arg1, "InstaGib"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5Set this to 1 to enable instagib type modes. Not all modes are insta but are done this way to be simple. (see ^3InstaWeapon ^5for available insta modes)\n\"");
	}
	else if(!Q_stricmp(arg1, "InstaWeapon"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5This will change the weapon configurations for instagib mode only.\n\"");
		trap_SendServerCommand(ent - g_entities, "printc \"^5Current weapon configurations are:\n\"");
		trap_SendServerCommand(ent - g_entities, "printc \"^50 = Gauntlet & Railgun (classic insta)\n\"");
		trap_SendServerCommand(ent - g_entities, "printc \"^51 = Gauntlet & Rocket Launcher\n\"");
		trap_SendServerCommand(ent - g_entities, "printc \"^52 = Gauntlet & Rocket Launcher & Railgun\n\"");
	}
	else if(!Q_stricmp(arg1, "FootSteps"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5With this set to 1 players will not generate any movement sounds unless falling from high places.\n\"");
	}
	else if(!Q_stricmp(arg1, "Hook"))
	{
		trap_SendServerCommand(ent - g_entities, "printc \"^5Enable this for the Grapple hook.\n\"");
	}
	else if(!Q_stricmp(arg1, "HookMissleSpeed"))
	{
		trap_SendServerCommand(ent - g_entities, "printc \"^5This will control how fast the grapple missle will travel.\n\"");
	}
	else if(!Q_stricmp(arg1, "DisableBFG"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5Since the BFG has been labeled by some as a (newbs) weapon... I have enabled this option so that it can be disabled in maps... But the BFG in this mod is far from what it was in Q3A.\n\"");
	}
	else if(!Q_stricmp(arg1, "DisableQUAD"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5Some more experienced players prefer to play without Quad damage so disable this if it causes any problems on the server.\n\"");
	}
	else if(!Q_stricmp(arg1, "SpaceProtection"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5With this enabled players who fall off of space maps or in to death pits will be teleported to a random spot in the map with out loosing any points and keeping any weapons they had when they fell.\n\"");
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5If the player is on the blue team or red team then he/she will be teleported back to his/her base and if he/she happens to be carrying a CTF flag then it will be returned to its base causing the fallen player to lose the flag.\n\"");
	}
	else if(!Q_stricmp(arg1, "FloodProtection"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5With Floodprotection enabled clients will not be able to send multiple chat commands to the server... but when disabled it will let multiple chat commands through and that can cause serious problems for some players connections. Enable this if a player is disrupting gameplay with useless chat or constant /bind say commands.\n\"");
	}
	else if(!Q_stricmp(arg1, "Vampire"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5Vampire mode will allow the attacker to pull some health out of his/her target.\n\"");
	}
	else if(!Q_stricmp(arg1, "SpeedSpawn"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5This command will make a player spawn instantly back into the world after he/she dies.\n\"");
	}
	else if(!Q_stricmp(arg1, "SpawnProtect"))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^5This setting will enable a protective shield over the player when they spawn in to the world.Set the value to the time you wish the players to be protected.\n\"");
	}
	else
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^1No info on this command.Use ^3/ref help ^1for a list of valid commands.\n\"");
	}
	trap_SendServerCommand(ent - g_entities,
						   "printc \"^2-----------------------------------------------------------------------------\n\"");

}

static const char *GetRefVar(const char *cmd)
{
	int             i;

	for(i = 0; i < nRefVars; i++)
	{
		if(Q_stricmp(cmd, refVars[i]) == 0)
			return refVarMap[i];
	}

	return "";
}

static qboolean CheckRValid(const char *cmd, const char *value, char **err)
{
	int             i;
	gentity_t      *ent2;
	gclient_t      *client2;
	char           *arg;


	// special case for g_gametype, check for bad values
	if(!Q_stricmp(cmd, "g_gametype"))
	{
		i = atoi(value);
		if(i == GT_SINGLE_PLAYER || i < GT_FFA || i >= GT_MAX_GAME_TYPE)
		{
			*err = "printc \"Invalid gametype.\n\"";
			return qfalse;
		}
	}
	else if(!Q_stricmp(cmd, "InstaWeapon"))
	{
		i = atoi(value);
		if(i < 0 || i > 2)
		{
			*err =
				"printc \"^3Invalid Weapontype. Valid Weapontypes are 1 = Rail Gun, 2 = Rocket Launcher, 3 = Particle Accelerator & Rocket Launcher.\n\"";
			return qfalse;
		}
	}
	else if(!Q_stricmp(cmd, "clientkick"))
	{
		i = atoi(value);
		arg = va("%s", value);

		ent2 = &g_entities[i];

		client2 = ent2->client;

		if(ClientNumberFromString2(arg) >= 0 && ent2->client->sess.ref == 1)
		{
			*err = "printc \"^1You cannot kick another referee.\n\"";
			return qfalse;
		}
		else if(ClientNumberFromString2(arg) < 0)
		{
			*err = "printc \"^1Invalid id number. ^3Use /players for a list of client numbers..\n\"";
			return qfalse;
		}


	}
	else if(!Q_stricmp(cmd, "fraglimit") || !Q_stricmp(cmd, "timelimit") || !Q_stricmp(cmd, "SpawnProtect"))
	{
		i = atoi(value);
		if(i < 0)
		{
			*err = "printc \"Value must be a positive integer.\n\"";
			return qfalse;
		}
		if(value[0] < '0' || value[0] > '9')
		{
			*err = "printc \"Value must be a positive integer.\n\"";
			return qfalse;
		}
	}
	else if(!Q_stricmp(cmd, "g_doWarmup") || !Q_stricmp(cmd, "Falling") ||
			!Q_stricmp(cmd, "g_friendlyFire") || !Q_stricmp(cmd, "sv_floodProtect"))
	{
		if(strlen(value) == 0 || strlen(value) > 1)
		{
			*err = "printc \"Valid values are 0 and 1.\n\"";
			return qfalse;
		}
		if(value[0] != '0' && value[0] != '1')
		{
			*err = "printc \"Valid values are 0 and 1.\n\"";
			return qfalse;
		}
	}
	else if(!Q_stricmp(cmd, "Hurtself"))
	{
		if(strlen(value) == 0 || strlen(value) > 1)
		{
			*err = "printc \"Valid values are 0, 1.\n\"";
			return qfalse;
		}
		if(value[0] != '0' && value[0] != '1' && value[0] != '2')
		{
			*err = "printc \"Valid values are 0, 1.\n\"";
			return qfalse;
		}
	}
	else if(!Q_stricmp(cmd, "map_restart") || !Q_stricmp(cmd, "map_restart"))
	{
		if(strlen(value) > 0)
		{
			*err = "printc \"This command requires no argument.\n\"";
			return qfalse;
		}
	}
	else if(!Q_stricmp(cmd, "Instagib"))
	{
		if(strlen(value) == 0 || strlen(value) > 1)
		{
			*err = "printc \"Valid values are 0, 1.\n\"";
			return qfalse;
		}
		if(value[0] != '0' && value[0] != '1' && value[0] != '2')
		{
			*err = "printc \"Valid values are 0, 1.\n\"";
			return qfalse;
		}
	}
	else if(!Q_stricmp(cmd, "g_footsteps"))
	{
		if(strlen(value) == 0 || strlen(value) > 1)
		{
			*err = "printc \"Valid values are 0, 1.\n\"";
			return qfalse;
		}
		if(value[0] != '0' && value[0] != '1' && value[0] != '2')
		{
			*err = "printc \"Valid values are 0, 1.\n\"";
			return qfalse;
		}
	}
	else if(!Q_stricmp(cmd, "HookFireSpeed"))
	{
		i = atoi(value);
		if(i < 0)
		{
			*err = "printc \"Value must be a positive integer.\n\"";
			return qfalse;
		}
	}
	else if(!Q_stricmp(cmd, "Hook") || !Q_stricmp(cmd, "DisableBFG") || !Q_stricmp(cmd, "Spaceprotect")
			|| !Q_stricmp(cmd, "DisableQuad") || !Q_stricmp(cmd, "Vampire") || !Q_stricmp(cmd, "SpeedSpawn"))
	{
		if(strlen(value) == 0 || strlen(value) > 1)
		{
			*err = "printc \"^3Valid values are 0, 1.\n\"";
			return qfalse;
		}
		if(value[0] != '0' && value[0] != '1' && value[0] != '2')
		{
			*err = "printc \"^3Valid values are 0, 1.\n\"";
			return qfalse;
		}
	}

	return qtrue;
}

static void Referee(gentity_t * ent)
{
	char           *cmd;

//  int     i;
	char            arg1[MAX_STRING_TOKENS];
	char            arg2[MAX_STRING_TOKENS];
	const char     *cmd2;
	char           *err;
	char           *cmd3;
	char           *cmd4;


	cmd = ConcatArgs(1);
	if(Q_stricmp(cmd, "none") == 0)
	{
		trap_SendServerCommand(ent - g_entities, "printc \"^3Referee is currently disabled on this server.\n\"");
		return;
	}

	if(*ref_password.string <= 0)
	{
		trap_SendServerCommand(ent - g_entities, "printc \"^3Referee password not set.\n\"");
		return;
	}
	if(ent->client->ps.persistant[PERS_REF] == 0)
	{
		if(Q_stricmp(cmd, ref_password.string) == 0)
		{
			ent->client->ps.persistant[PERS_REF] = 1;
			ent->client->sess.ref = 1;
			trap_SendServerCommand(ent - g_entities, "printc \"^3Use ^5/ref help ^3 for a list of referee commands.\n\"");
		}
		else
		{
			trap_SendServerCommand(ent - g_entities, "printc \"^1Incorrect Referee Password.\n\"");
			return;
		}
		return;
	}



	if(nRefVars != nRefVarMap)
	{
		trap_SendServerCommand(ent - g_entities, "printc \"Sanity check failed: nVoteVars != nVoteVarMap\n\"");
		return;
	}

	// make sure it is a valid command
	trap_Argv(1, arg1, sizeof(arg1));
	trap_Argv(2, arg2, sizeof(arg2));

	if(!Q_stricmp(arg1, "help") && !Q_stricmp(arg2, ""))
	{
		PrintRefHelp(ent);
		return;
	}
	else if(!Q_stricmp(arg1, "help") && Q_stricmp(arg2, ""))
	{
		PrintRefCMDInfo(ent, arg2);
		return;
	}

	if(strchr(arg1, ';') || strchr(arg2, ';'))
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^1Invalid ref command.\n\"^3Use /ref help for a valid list of ref commands.\n\"");
		trap_SendServerCommand(ent - g_entities, "printc \"^3Use ^8/ref help ^3for a list of valid ref commands.\n\"");
		return;
	}

	cmd2 = GetRefVar(arg1);
	if(cmd2[0] == '\0')
	{
		trap_SendServerCommand(ent - g_entities,
							   "printc \"^1Invalid ref command.\n\"^3Use /ref help for a valid list of ref commands.\n\"");
		trap_SendServerCommand(ent - g_entities, "printc \"^3Use ^8/ref help ^3for a list of valid ref commands.\n\"");
		return;
	}

	cmd4 = ConcatArgs(2);


	if(!CheckRValid(cmd2, cmd4, &err))
	{
		trap_SendServerCommand(ent - g_entities, err);
		return;
	}

	if(!Q_stricmp(cmd2, "map"))
	{
		// special case for map changes, we want to reset the nextmap setting
		// this allows a player to change maps, but not upset the map rotation
		char            s[MAX_STRING_CHARS];

		trap_Cvar_VariableStringBuffer("nextmap", s, sizeof(s));
		if(*s)
		{
			cmd3 = va("%s %s; set nextmap \"%s\"", cmd2, arg2, s);
		}
		else
		{
			cmd3 = va("%s %s", cmd2, arg2);

		}
		Com_sprintf(level.ExecuteString, sizeof(level.ExecuteString), "map");
	}
	else if(!Q_stricmp(cmd2, "nextmap"))
	{
		char            s[MAX_STRING_CHARS];

		trap_Cvar_VariableStringBuffer("nextmap", s, sizeof(s));
		if(!*s)
		{
			trap_SendServerCommand(ent - g_entities, "printc \"nextmap not set.\n\"");
			return;
		}
		Com_sprintf(level.ExecuteString, sizeof(level.ExecuteString), "map");
		cmd3 = "vstr nextmap";
	}
	else if(!Q_stricmp(cmd, "resetrotation"))
	{
		cmd3 = "set nextmap \"vstr d1\"";
		Com_sprintf(level.ExecuteString, sizeof(level.ExecuteString), "map");
		Com_sprintf(cmd3, sizeof(cmd3), "set nextmap \"vstr d1\"");
	}
	else
	{
		cmd3 = va("%s \"%s\"", cmd2, arg2);
	}

	trap_SendServerCommand(-1, va("print \"%s ^N^3changed a setting.", ent->client->pers.netname));
	trap_SendConsoleCommand(EXEC_APPEND, va("%s\n", cmd3));

}

/*
=================
Cmd_Stats_f
=================

void Cmd_Stats_f( gentity_t *ent ) {

	int max, n, i;

	max = trap_AAS_PointReachabilityAreaIndex( NULL );

	n = 0;
	for ( i = 0; i < max; i++ ) {
		if ( ent->client->areabits[i >> 3] & (1 << (i & 7)) )
			n++;
	}

	//trap_SendServerCommand( ent-g_entities, va("print \"visited %d of %d areas\n\"", n, max));
	trap_SendServerCommand( ent-g_entities, va("print \"%d%% level coverage\n\"", n * 100 / max));

}
*/

void Cmd_MOTD_f(gentity_t * ent)
{

	trap_SendServerCommand(ent - g_entities, va("motdi %s1 %s2 %s3", motd1.string, motd2.string, motd3.string));

}

/*
=================
SpectatorOnly_f
=================
*/
static void SpectatorOnly_f(gentity_t * ent)
{
	char           *msg;

	ent->client->sess.speconly ^= 1;
	if(ent->client->sess.speconly == 0)
	{
		msg = "^5Disabled.";
	}
	else
	{
		SetTeam(ent, "s");
		msg = "^5Enabled.";
	}
	trap_SendServerCommand(ent - g_entities, va("print \"^3Spectator Mode is %s", msg));

}

/*
=================
Multiview1_f
=================
*/
static void Multiview1_f(gentity_t * ent)
{
	gentity_t      *ent1;
	gentity_t      *view1;
	char           *cmd;
	int             i;

	cmd = ConcatArgs(1);
	i = ClientNumberFromString2(cmd);

	ent1 = &g_entities[1];
	view1 = G_Spawn();

	view1->s.eType = ET_MVIEW1;
	view1->s = ent1->s;
	view1->r.svFlags = SVF_PORTAL;
	view1->s.number = 1;
	VectorCopy(ent1->r.currentOrigin, view1->s.origin2);
	VectorCopy(ent1->r.currentOrigin, view1->r.currentOrigin);
	trap_SendServerCommand(ent - g_entities, va("print \"^1%s ^3%i", cmd, i));
	trap_LinkEntity(view1);
}

/*
	gentity_t *ent1;
	gentity_t *ent2;
	gentity_t *ent3;
	gentity_t *ent4;
	multips_t Mview;

//	char	arg1[MAX_STRING_TOKENS];

//	trap_Argv( 1, arg1, sizeof( arg1 ) );
	
	memset(&Mview,0,sizeof(Mview));

	ent1 = &g_entities[ 1 ];
	ent2 = &g_entities[ 2 ];
	ent3 = &g_entities[ 3 ];
	ent4 = &g_entities[ 4 ];

	if(ent1->client){
		Mview.psview1 = &ent1->client->ps;
	}
	if(ent2->client){
		Mview.psview2 = &ent2->client->ps;
	}
	if(ent3->client){
		Mview.psview3 = &ent3->client->ps;
	}
	if(ent4->client){
		Mview.psview4 = &ent4->client->ps;
	}
}*/


/*
=================
ClientCommand
=================
*/
void ClientCommand(int clientNum)
{
	gentity_t      *ent;
	char            cmd[MAX_TOKEN_CHARS];
	char            arg[MAX_TOKEN_CHARS];
	int             i;


	trap_Argv(1, arg, sizeof(arg));
	i = atoi(arg);

	ent = g_entities + clientNum;
	if(!ent->client)
	{
		return;					// not fully in game yet
	}


	trap_Argv(0, cmd, sizeof(cmd));

	if(Q_stricmp(cmd, "say") == 0)
	{
		Cmd_Say_f(ent, SAY_ALL, qfalse);
		return;
	}
	if(Q_stricmp(cmd, "say_team") == 0)
	{
		Cmd_Say_f(ent, SAY_TEAM, qfalse);
		return;
	}
	if(Q_stricmp(cmd, "tell") == 0)
	{
		Cmd_Tell_f(ent);
		return;
	}
	if(Q_stricmp(cmd, "vsay") == 0)
	{
		Cmd_Voice_f(ent, SAY_ALL, qfalse, qfalse);
		return;
	}
	if(Q_stricmp(cmd, "vsay_team") == 0)
	{
		Cmd_Voice_f(ent, SAY_TEAM, qfalse, qfalse);
		return;
	}
	if(Q_stricmp(cmd, "vtell") == 0)
	{
		Cmd_VoiceTell_f(ent, qfalse);
		return;
	}
	if(Q_stricmp(cmd, "vosay") == 0)
	{
		Cmd_Voice_f(ent, SAY_ALL, qfalse, qtrue);
		return;
	}
	if(Q_stricmp(cmd, "vosay_team") == 0)
	{
		Cmd_Voice_f(ent, SAY_TEAM, qfalse, qtrue);
		return;
	}
	if(Q_stricmp(cmd, "votell") == 0)
	{
		Cmd_VoiceTell_f(ent, qtrue);
		return;
	}
	if(Q_stricmp(cmd, "vtaunt") == 0)
	{
		Cmd_VoiceTaunt_f(ent);
		return;
	}
	if(Q_stricmp(cmd, "score") == 0)
	{
		Cmd_Score_f(ent);
		return;
	}
	if(Q_stricmp(cmd, "follownext") == 0)
	{
		Cmd_FollowCycle_f(ent, 1);
		return;
	}

	if(Q_stricmp(cmd, "followprev") == 0)
	{
		Cmd_FollowCycle_f(ent, -1);
		return;
	}

	if(Q_stricmp(cmd, "speconly") == 0)
	{
		SpectatorOnly_f(ent);
		return;
	}

	if(Q_stricmp(cmd, "stats2") == 0)
	{
		Cmd_Statistics_f(ent);
		return;
	}
	if(Q_stricmp(cmd, "statnext") == 0)
	{
		Cmd_StatCycle_f(ent, 1);
		return;
	}
	if(Q_stricmp(cmd, "statprev") == 0)
	{
		Cmd_StatCycle_f(ent, -1);
		return;
	}

	if(Q_stricmp(cmd, "ref") == 0)
	{
		Referee(ent);
		return;
	}
	if(Q_stricmp(cmd, "referee") == 0)
	{
		Referee(ent);
		return;
	}

	if(Q_stricmp(cmd, "callvote") == 0)
	{
		Cmd_CallVote_f(ent);
		return;
	}

	if(Q_stricmp(cmd, "vote") == 0)
	{
		Cmd_Vote_f(ent);
		return;
	}

	if(Q_stricmp(cmd, "players") == 0)
	{
		Playerinfo(ent);
		return;
	}
	if(Q_stricmp(cmd, "addview") == 0)
	{
		Multiview1_f(ent);
		return;
	}

	// ignore all other commands when at intermission
	if(level.intermissiontime)
	{
		Cmd_Say_f(ent, qfalse, qtrue);
		return;
	}

	if(Q_stricmp(cmd, "give") == 0)
		Cmd_Give_f(ent);
	else if(Q_stricmp(cmd, "god") == 0)
		Cmd_God_f(ent);
	else if(Q_stricmp(cmd, "noknock") == 0)
		Cmd_Knock_f(ent);
	else if(Q_stricmp(cmd, "notarget") == 0)
		Cmd_Notarget_f(ent);
	else if(Q_stricmp(cmd, "noclip") == 0)
		Cmd_Noclip_f(ent);
	else if(Q_stricmp(cmd, "kill") == 0)
		Cmd_Kill_f(ent);
	else if(Q_stricmp(cmd, "teamtask") == 0)
		Cmd_TeamTask_f(ent);
	else if(Q_stricmp(cmd, "levelshot") == 0)
		Cmd_LevelShot_f(ent);
	else if(Q_stricmp(cmd, "follow") == 0)
		Cmd_Follow_f(ent);
	else if(Q_stricmp(cmd, "tossWeapon") == 0)
		Cmd_ThrowW_f(ent);
	else if(Q_stricmp(cmd, "tossammo") == 0)
		Cmd_ThrowA_f(ent);
	else if(Q_stricmp(cmd, "tossflag") == 0)
		Team_ThrowFlags(ent);
	else if(Q_stricmp(cmd, "team") == 0)
		Cmd_Team_f(ent);
	else if(Q_stricmp(cmd, "where") == 0)
		Cmd_Where_f(ent);
	else if(Q_stricmp(cmd, "callteamvote") == 0)
		Cmd_CallTeamVote_f(ent);
	else if(Q_stricmp(cmd, "teamvote") == 0)
		Cmd_TeamVote_f(ent);
	else if(Q_stricmp(cmd, "gc") == 0)
		Cmd_GameCommand_f(ent);
	else if(Q_stricmp(cmd, "setviewpos") == 0)
		Cmd_SetViewpos_f(ent);
	else if(Q_stricmp(cmd, "stats") == 0)
		Cmd_Stats2_f(ent);
	else if(Q_stricmp(cmd, "help") == 0)
		Cmd_help(ent);
	else
		trap_SendServerCommand(clientNum, va("printc \"unknown cmd %s\n\"", cmd));
}
