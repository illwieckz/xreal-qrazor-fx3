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
//  acebot_spawn.c - This file contains all of the 
//                   spawing support routines for the ACE bot.


#include "g_local.h"
#include "acebot.h"

#if defined(ACEBOT)

///////////////////////////////////////////////////////////////////////
// Had to add this function in this version for some reason.
// any globals are wiped out between level changes....so
// save the bots out to a file. 
//
// NOTE: There is still a bug when the bots are saved for
//       a dm game and then reloaded into a CTF game.
///////////////////////////////////////////////////////////////////////
/*
void ACESP_SaveBots()
{
	gentity_t        *bot;
	FILE           *pOut;
	int             i, count = 0;

	if((pOut = fopen("ace\\bots.tmp", "wb")) == NULL)
		return;					// bail

	// Get number of bots
	for(i = maxclients->value; i > 0; i--)
	{
		bot = g_edicts + i + 1;

		if(bot->inuse && bot->is_bot)
			count++;
	}

	fwrite(&count, sizeof(int), 1, pOut);	// Write number of bots

	for(i = maxclients->value; i > 0; i--)
	{
		bot = g_edicts + i + 1;

		if(bot->inuse && bot->is_bot)
			fwrite(bot->client->pers.userinfo, sizeof(char) * MAX_INFO_STRING, 1, pOut);
	}

	fclose(pOut);
}
*/

///////////////////////////////////////////////////////////////////////
// Had to add this function in this version for some reason.
// any globals are wiped out between level changes....so
// load the bots from a file.
//
// Side effect/benifit are that the bots persist between games.
///////////////////////////////////////////////////////////////////////
/*
void ACESP_LoadBots()
{
	FILE           *pIn;
	char            userinfo[MAX_INFO_STRING];
	int             i, count;

	if((pIn = fopen("ace\\bots.tmp", "rb")) == NULL)
		return;					// bail

	fread(&count, sizeof(int), 1, pIn);

	for(i = 0; i < count; i++)
	{
		fread(userinfo, sizeof(char) * MAX_INFO_STRING, 1, pIn);
		ACESP_SpawnBot(NULL, NULL, NULL, userinfo);
	}

	fclose(pIn);
}
*/


void ACESP_Respawn(gentity_t * self)
{
#if 0
	CopyToBodyQue(self);

	if(ctf->value)
		ACESP_PutClientInServer(self, true, self->client->resp.ctf_team);
	else
		ACESP_PutClientInServer(self, true, 0);

	// add a teleportation effect
	self->s.event = EV_PLAYER_TELEPORT;

	// hold in place briefly
	self->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
	self->client->ps.pmove.pm_time = 14;

	self->client->respawn_time = level.time;
#endif
}

gentity_t        *ACESP_FindFreeClient(void)
{
#if 0
	gentity_t        *bot;
	int             i;
	int             max_count = 0;

	// This is for the naming of the bots
	for(i = maxclients->value; i > 0; i--)
	{
		bot = g_edicts + i + 1;

		if(bot->count > max_count)
			max_count = bot->count;
	}

	// Check for free spot
	for(i = maxclients->value; i > 0; i--)
	{
		bot = g_edicts + i + 1;

		if(!bot->inuse)
			break;
	}

	bot->count = max_count + 1;	// Will become bot name...

	if(bot->inuse)
		bot = NULL;

	return bot;
#else
	return NULL;
#endif
}

///////////////////////////////////////////////////////////////////////
// Set the name of the bot and update the userinfo
///////////////////////////////////////////////////////////////////////
/*
void ACESP_SetName(gentity_t * bot, char *name, char *skin, char *team)
{
#if 0
	float           rnd;
	char            userinfo[MAX_INFO_STRING];
	char            bot_skin[MAX_INFO_STRING];
	char            bot_name[MAX_INFO_STRING];

	// Set the name for the bot.
	// name
	if(strlen(name) == 0)
		sprintf(bot_name, "ACEBot_%d", bot->count);
	else
		strcpy(bot_name, name);

	// skin
	if(strlen(skin) == 0)
	{
		// randomly choose skin 
		rnd = random();
		if(rnd < 0.05)
			sprintf(bot_skin, "female/athena");
		else if(rnd < 0.1)
			sprintf(bot_skin, "female/brianna");
		else if(rnd < 0.15)
			sprintf(bot_skin, "female/cobalt");
		else if(rnd < 0.2)
			sprintf(bot_skin, "female/ensign");
		else if(rnd < 0.25)
			sprintf(bot_skin, "female/jezebel");
		else if(rnd < 0.3)
			sprintf(bot_skin, "female/jungle");
		else if(rnd < 0.35)
			sprintf(bot_skin, "female/lotus");
		else if(rnd < 0.4)
			sprintf(bot_skin, "female/stiletto");
		else if(rnd < 0.45)
			sprintf(bot_skin, "female/venus");
		else if(rnd < 0.5)
			sprintf(bot_skin, "female/voodoo");
		else if(rnd < 0.55)
			sprintf(bot_skin, "male/cipher");
		else if(rnd < 0.6)
			sprintf(bot_skin, "male/flak");
		else if(rnd < 0.65)
			sprintf(bot_skin, "male/grunt");
		else if(rnd < 0.7)
			sprintf(bot_skin, "male/howitzer");
		else if(rnd < 0.75)
			sprintf(bot_skin, "male/major");
		else if(rnd < 0.8)
			sprintf(bot_skin, "male/nightops");
		else if(rnd < 0.85)
			sprintf(bot_skin, "male/pointman");
		else if(rnd < 0.9)
			sprintf(bot_skin, "male/psycho");
		else if(rnd < 0.95)
			sprintf(bot_skin, "male/razor");
		else
			sprintf(bot_skin, "male/sniper");
	}
	else
		strcpy(bot_skin, skin);

	// initialise userinfo
	memset(userinfo, 0, sizeof(userinfo));

	// add bot's name/skin/hand to userinfo
	Info_SetValueForKey(userinfo, "name", bot_name);
	Info_SetValueForKey(userinfo, "skin", bot_skin);
	Info_SetValueForKey(userinfo, "hand", "2");	// bot is center handed for now!

	ClientConnect(bot, userinfo);

	//ACESP_SaveBots();			// make sure to save the bots
#endif
}
*/


void ACESP_SpawnBot(char *name, char *team)
{
	int             clientNum;
//	char           *botinfo;
//	gentity_t      *bot;
	char           *key;
	char           *s;
//	char           *botname;
	char           *model;
	char           *headmodel;
	char            userinfo[MAX_INFO_STRING];
	
	G_Printf("ACESP_SpawnBot(%s, %s)\n", name, team);

	// have the server allocate a client slot
	clientNum = trap_BotAllocateClient();
	if(clientNum == -1)
	{
		G_Printf(S_COLOR_RED "Unable to add bot.  All player slots are in use.\n");
		G_Printf(S_COLOR_RED "Start server with more 'open' slots (or check setting of sv_maxclients cvar).\n");
		return;
	}

//	bot = &g_entities[clientNum];
//	bot->r.svFlags |= SVF_BOT;
//	bot->inuse = qtrue;

	// create the bot's userinfo
	userinfo[0] = '\0';

	Info_SetValueForKey(userinfo, "rate", "25000");
	Info_SetValueForKey(userinfo, "snaps", "20");
	Info_SetValueForKey(userinfo, "skill", va("%1.2f", 3.0));
	
	// TODO LUA bot characterristics

	key = "name";
	//model = Info_ValueForKey(botinfo, key);
	if(!name || !*name)
	{
		name = va("ACEBot%d", clientNum);
	}
	Info_SetValueForKey(userinfo, key, name);

	key = "model";
	//model = Info_ValueForKey(botinfo, key);
	//if(!*model)
	{
		model = "visor/default";
	}
	Info_SetValueForKey(userinfo, key, model);

	key = "headmodel";
	//headmodel = Info_ValueForKey(botinfo, key);
	//if(!*headmodel)
	{
		headmodel = model;
	}
	Info_SetValueForKey(userinfo, key, headmodel);

	key = "gender";
	//s = Info_ValueForKey(botinfo, key);
	//if(!*s)
	{
		s = "male";
	}
	Info_SetValueForKey(userinfo, "sex", s);

	key = "color1";
	//s = Info_ValueForKey(botinfo, key);
	//if(!*s)
	{
		s = "4";
	}
	Info_SetValueForKey(userinfo, key, s);

	key = "color2";
	//s = Info_ValueForKey(botinfo, key);
	//if(!*s)
	{
		s = "5";
	}
	Info_SetValueForKey(userinfo, key, s);

	// initialize the bot settings
	if(!team || !*team)
	{
		if(g_gametype.integer >= GT_TEAM)
		{
			if(PickTeam(clientNum) == TEAM_RED)
			{
				team = "red";
			}
			else
			{
				team = "blue";
			}
		}
		else
		{
			team = "red";
		}
	}
	//Info_SetValueForKey(userinfo, "characterfile", Info_ValueForKey(botinfo, "aifile"));
	//Info_SetValueForKey(userinfo, "skill", va("%5.2f", skill));
	Info_SetValueForKey(userinfo, "team", team);

	// register the userinfo
	trap_SetUserinfo(clientNum, userinfo);

	// have it connect to the game as a normal client
	if(ClientConnect(clientNum, qtrue, qtrue) != NULL)
	{
		return;
	}

	ClientBegin(clientNum);
}


// Remove a bot by name or all bots
void ACESP_RemoveBot(char *name)
{
#if 0
	int             i;
	qboolean        freed = false;
	gentity_t        *bot;

	for(i = 0; i < maxclients->value; i++)
	{
		bot = g_edicts + i + 1;
		if(bot->inuse)
		{
			if(bot->is_bot && (strcmp(bot->client->pers.netname, name) == 0 || strcmp(name, "all") == 0))
			{
				bot->health = 0;
				player_die(bot, bot, bot, 100000, vec3_origin);
				// don't even bother waiting for death frames
				bot->deadflag = DEAD_DEAD;
				bot->inuse = false;
				freed = true;
				ACEIT_PlayerRemoved(bot);
				safe_bprintf(PRINT_MEDIUM, "%s removed\n", bot->client->pers.netname);
			}
		}
	}

	if(!freed)
		safe_bprintf(PRINT_MEDIUM, "%s not found\n", name);

	//ACESP_SaveBots();			// Save them again
#endif
}

qboolean ACESP_BotConnect(int clientNum, qboolean restart)
{
#if 1
	char            userinfo[MAX_INFO_STRING];

	trap_GetUserinfo(clientNum, userinfo, sizeof(userinfo));

	//Q_strncpyz(settings.characterfile, Info_ValueForKey(userinfo, "characterfile"), sizeof(settings.characterfile));
	//settings.skill = atof(Info_ValueForKey(userinfo, "skill"));
	//Q_strncpyz(settings.team, Info_ValueForKey(userinfo, "team"), sizeof(settings.team));

	//if(!BotAISetupClient(clientNum, &settings, restart))
	//{
	//	trap_DropClient(clientNum, "BotAISetupClient failed");
	//	return qfalse;
	//}
	
	/*
	bot = &g_entities[clientNum];
	
	// set bot state
	bot = g_entities + clientNum;

	bot->enemy = NULL;
	bot->bs.moveTarget = NULL;
	bot->bs.state = STATE_MOVE;

	// set the current node
	bot->bs.currentNode = ACEND_FindClosestReachableNode(bot, NODE_DENSITY, NODE_ALL);
	bot->bs.goalNode = bot->bs.currentNode;
	bot->bs.nextNode = bot->bs.currentNode;
	bot->bs.next_move_time = level.time;
	bot->bs.suicide_timeout = level.time + 15000;
	*/

	return qtrue;
#endif
}


void  ACESP_SetupBotState(gentity_t * self)
{
	int             clientNum;
	char            userinfo[MAX_INFO_STRING];
	
	clientNum = self->client - level.clients;
	trap_GetUserinfo(clientNum, userinfo, sizeof(userinfo));
	
	self->enemy = NULL;
	
	self->bs.yawSpeed = 100;		// FIXME 100 is deadly fast
	self->bs.moveTarget = NULL;
	self->bs.state = STATE_MOVE;

	// set the current node
	self->bs.currentNode = ACEND_FindClosestReachableNode(self, NODE_DENSITY, NODE_ALL);
	self->bs.goalNode = self->bs.currentNode;
	self->bs.nextNode = self->bs.currentNode;
	self->bs.lastNode = INVALID;
	self->bs.next_move_time = level.time;
	self->bs.suicide_timeout = level.time + 15000;
	
	if(g_gametype.integer != GT_TOURNAMENT)
	{
		// need to send this or bots will be spectators
		trap_BotClientCommand(self - g_entities, va("team %s", Info_ValueForKey(userinfo, "team")));
	}
}



#endif

