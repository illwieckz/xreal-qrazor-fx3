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
// acebot.h - Main header file for ACEBOT


#ifndef _ACEBOT_H
#define _ACEBOT_H

//#include "g_local.h"
#if defined(ACEBOT)


// FIXME remove: game print flags
#define	PRINT_LOW			0	// pickup messages
#define	PRINT_MEDIUM		1	// death messages
#define	PRINT_HIGH			2	// critical messages
#define	PRINT_CHAT			3	// chat messages

// Platform states
#define	STATE_TOP			0
#define	STATE_BOTTOM		1
#define STATE_UP			2
#define STATE_DOWN			3

// Maximum nodes
#define MAX_NODES 1000

// Link types
#define INVALID -1

// Node types
#define NODE_MOVE 0
#define NODE_LADDER 1
#define NODE_PLATFORM 2
#define NODE_TELEPORTER 3
#define NODE_ITEM 4
#define NODE_WATER 5
#define NODE_GRAPPLE 6
#define NODE_JUMP 7
#define NODE_ALL 99				// For selecting all nodes

// Density setting for nodes
#define NODE_DENSITY 128		// fomerly 128

// Bot state types
enum
{
	STATE_STAND,
	STATE_MOVE,
	STATE_ATTACK,
	STATE_WANDER,
	STATE_FLEE,
};

enum
{
	MOVE_LEFT,
	MOVE_RIGHT,
	MOVE_FORWARD,
	MOVE_BACK,
};

// Node structure
typedef struct node_s
{
	vec3_t          origin;		// Using Id's representation
	int             type;		// type of node
} node_t;


// extern decs
extern int      numnodes;
extern node_t   nodes[MAX_NODES];

extern qboolean debug_mode;


// id Function Protos I need
void            LookAtKiller(gentity_t * self, gentity_t * inflictor, gentity_t * attacker);
//void            ClientObituary(gentity_t * self, gentity_t * inflictor, gentity_t * attacker);
//void            TossClientWeapon(gentity_t * self);
void            ClientThink_real(gentity_t * ent);
gentity_t      *SelectSpawnPoint(vec3_t avoidPoint, vec3_t origin, vec3_t angles);
void            ClientUserinfoChanged(int clientNum);
void            CopyToBodyQue(gentity_t * ent);
char           *ClientConnect(int clientNum, qboolean firstTime, qboolean isBot);
//void            Use_Plat(gentity_t * ent, gentity_t * other, gentity_t * activator);

// acebot_ai.c protos
void            ACEAI_Think(gentity_t * self);
void            ACEAI_CheckServerCommands(gentity_t * self);
void            ACEAI_PickLongRangeGoal(gentity_t * self);
void            ACEAI_PickShortRangeGoal(gentity_t * self);
qboolean        ACEAI_FindEnemy(gentity_t * self);
void            ACEAI_ChooseWeapon(gentity_t * self);

// acebot_cmds.c protos
qboolean        ACECM_Commands(gentity_t * ent);
void            ACECM_Store();

// acebot_items.c protos
void            ACEIT_PlayerAdded(gentity_t * ent);
void            ACEIT_PlayerRemoved(gentity_t * ent);
qboolean        ACEIT_IsVisible(gentity_t * self, vec3_t goal);
qboolean        ACEIT_IsReachable(gentity_t * self, vec3_t goal);
qboolean        ACEIT_ChangeWeapon(gentity_t * ent, weapon_t weapon);
qboolean        ACEIT_CanUseArmor(gitem_t * item, gentity_t * other);
float           ACEIT_ItemNeed(gentity_t * self, gitem_t * item);
//int             ACEIT_ClassnameToIndex(char *classname);
void            ACEIT_BuildItemNodeTable(qboolean rebuild);

// acebot_movement.c protos
qboolean        ACEMV_SpecialMove(gentity_t * self);
void            ACEMV_Move(gentity_t * self);
void            ACEMV_Attack(gentity_t * self);
void            ACEMV_Wander(gentity_t * self);

// acebot_nodes.c protos
int             ACEND_FindCost(int from, int to);
int             ACEND_FindCloseReachableNode(gentity_t * self, float range, int type);
int             ACEND_FindClosestReachableNode(gentity_t * self, float range, int type);
void            ACEND_SetGoal(gentity_t * self, int goal_node);
qboolean        ACEND_FollowPath(gentity_t * self);
void            ACEND_GrapFired(gentity_t * self);
qboolean        ACEND_CheckForLadder(gentity_t * self);
void            ACEND_PathMap(gentity_t * self);
void            ACEND_InitNodes(void);
void            ACEND_ShowNode(int node);
void            ACEND_DrawPath();
void            ACEND_ShowPath(gentity_t * self, int goal_node);
int             ACEND_AddNode(gentity_t * self, int type);
void            ACEND_UpdateNodeEdge(int from, int to);
void            ACEND_RemoveNodeEdge(gentity_t * self, int from, int to);
void            ACEND_ResolveAllPaths();
void            ACEND_SaveNodes();
void            ACEND_LoadNodes();

// acebot_spawn.c protos
//void            ACESP_SaveBots();
//void            ACESP_LoadBots();
void            ACESP_HoldSpawn(gentity_t * self);
void            ACESP_PutClientInServer(gentity_t * bot, qboolean respawn, int team);
void            ACESP_Respawn(gentity_t * self);
gentity_t        *ACESP_FindFreeClient(void);
void            ACESP_SetName(gentity_t * bot, char *name, char *skin, char *team);
void            ACESP_SpawnBot(char *name, char *team);
void            ACESP_ReAddBots();
void            ACESP_RemoveBot(char *name);
qboolean        ACESP_BotConnect(int clientNum, qboolean restart);
void            ACESP_SetupBotState(gentity_t * bot);
void            safe_cprintf(gentity_t * ent, int printlevel, char *fmt, ...);
void            safe_centerprintf(gentity_t * ent, char *fmt, ...);
void            safe_bprintf(int printlevel, char *fmt, ...);
void            debug_printf(char *fmt, ...);


#endif // ACEBOT

#endif
