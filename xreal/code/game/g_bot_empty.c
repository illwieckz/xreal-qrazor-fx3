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
//
// g_bot.c

#include "g_local.h"

#if !defined(BRAINWORKS) && !defined(ACEBOT)

/*
===============
G_BotConnect
===============
*/
qboolean G_BotConnect(int clientNum, qboolean restart)
{
	trap_DropClient(clientNum, "no bot support");
	return qfalse;
}

/*
===============
G_RemoveQueuedBotBegin
===============
*/
void G_RemoveQueuedBotBegin(int clientNum)
{
}

/*
===============
Svcmd_AddBot_f
===============
*/
void Svcmd_AddBot_f(void)
{
	trap_Printf("no bot support\n");
}

/*
===============
Svcmd_BotList_f
===============
*/
void Svcmd_BotList_f(void)
{
	trap_Printf("no bot support\n");
}

/*
===============
G_InitBots
===============
*/
void G_InitBots(qboolean restart)
{
	trap_Printf("no bot support\n");
}

/*
=====================
BotInterbreedEndMatch

NOTE: This function only exists because it's called by a function
in g_main.c.  There's no more need for fuzzy logic interbreeding
because the item pickup code was completely rewritten.
=====================
*/
void BotInterbreedEndMatch()
{
}

/*
===============
BotAISetup
===============
*/
int BotAISetup(int restart)
{
	return qfalse;
}

/*
===============
BotAIShutdown
===============
*/
int BotAIShutdown(int restart)
{
	return qfalse;
}

/*
===============
BotAILoadMap
===============
*/
int BotAILoadMap(int restart)
{
	return qfalse;
}

/*
===============
BotAISetupClient
===============
*/
int BotAISetupClient(int client, struct bot_settings_s *settings, qboolean restart)
{
	return qfalse;
}

/*
===============
BotAIShutdownClient
===============
*/
int BotAIShutdownClient(int client, qboolean restart)
{
	return qfalse;
}

/*
===============
BotAIStartFrame
===============
*/
int BotAIStartFrame(int time)
{
	return qfalse;
}

/*
===============
BotAIDebug
===============
*/
void BotAIDebug(void)
{
}

//void          BotTestAAS(vec3_t origin);

#endif
