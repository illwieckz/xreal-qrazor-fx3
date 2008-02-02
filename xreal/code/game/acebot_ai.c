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
//  acebot_ai.c -      This file contains all of the 
//                     AI routines for the ACE II bot.



#include "g_local.h"
#include "acebot.h"

#if defined(ACEBOT)

///////////////////////////////////////////////////////////////////////
// Main Think function for bot
///////////////////////////////////////////////////////////////////////
void ACEAI_Think(gentity_t * self)
{
	int             i;
	usercmd_t       ucmd;
	
	//if(debug_mode)
	//	G_Printf("ACEAI_Think(%s)\n", self->client->pers.netname);

	// Set up client movement
	VectorCopy(self->client->ps.viewangles, self->bs.viewAngles);
	VectorSet(self->client->ps.delta_angles, 0, 0, 0);
	memset(&ucmd, 0, sizeof(ucmd));
	self->enemy = NULL;
	self->bs.movetarget = NULL;
		
	// do this to avoid a time out
	ACEAI_CheckServerCommands(self);

	// Force respawn 
	if(self->health <= 0)
	{
		self->client->buttons = 0;
		ucmd.buttons = BUTTON_ATTACK;
	}

	if(self->bs.state == STATE_WANDER && self->bs.wander_timeout < level.time)
		ACEAI_PickLongRangeGoal(self);	// pick a new long range goal

#if 0
	// Kill the bot if completely stuck somewhere
	if(VectorLength(self->client->ps.velocity) > 37)	//
		self->bs.suicide_timeout = level.time + 10000;

	if(self->bs.suicide_timeout < level.time)
	{
		self->client->ps.stats[STAT_HEALTH] = self->health = -999;
		player_die(self, self, self, 100000, MOD_SUICIDE);
	}
#endif

	// Find any short range goal
	ACEAI_PickShortRangeGoal(self);

	// Look for enemies
	if(ACEAI_FindEnemy(self))
	{
		ACEAI_ChooseWeapon(self);
		ACEMV_Attack(self, &ucmd);
	}
	else
	{
		// Execute the move, or wander
		if(self->bs.state == STATE_WANDER)
		{
			ACEMV_Wander(self, &ucmd);
		}
		else if(self->bs.state == STATE_MOVE)
		{
			ACEMV_Move(self, &ucmd);
		}
	}

	//debug_printf("State: %d\n",self->state);

	// set approximate ping
	ucmd.serverTime = 75 + floor(random() * 25) + 1;

	// show random ping values in scoreboard
	//self->client->ping = ucmd.msec;

	// copy the state that the cgame is currently sending
	//cmd->weapon = cl.cgameUserCmdValue;

	// send the current server time so the amount of movement
	// can be determined without allowing cheating
	//cmd->serverTime = cl.serverTime;

	for(i = 0; i < 3; i++)
	{
		ucmd.angles[i] = ANGLE2SHORT(self->bs.viewAngles[i]);
	}

	// send command through id's code
	self->client->pers.cmd = ucmd;
	ClientThink_real(self);

	//self->nextthink = level.time + FRAMETIME;
}


void ACEAI_CheckServerCommands(gentity_t * self)
{
	char            buf[1024];

	while(trap_BotGetServerCommand(self->client - level.clients, buf, sizeof(buf)))
	{
#if 0
		//have buf point to the command and args to the command arguments
		if(debug_mode)
		{
			G_Printf("ACEAI_CheckServerCommands for %s: %s\n", self->client->pers.netname, buf);		
		}
#endif
	}
}

/*
=============
ACEAI_InFront

returns 1 if the entity is in front (in sight) of self
=============
*/
qboolean ACEAI_InFront(gentity_t * self, gentity_t * other)
{
	vec3_t          vec;
	float           dot;
	vec3_t          forward;

	AngleVectors(self->bs.viewAngles, forward, NULL, NULL);
	VectorSubtract(other->s.origin, self->client->ps.origin, vec);
	VectorNormalize(vec);
	dot = DotProduct(vec, forward);

	if(dot > 0.3)
		return qtrue;
	
	return qfalse;
}

/*
=============
ACEAI_Visible

returns 1 if the entity is visible to self, even if not infront ()
=============
*/
qboolean ACEAI_Visible(gentity_t * self, gentity_t * other)
{
	vec3_t          spot1;
	vec3_t          spot2;
	trace_t         trace;
	
	//if(!self->client || !other->client)
	//	return qfalse;

	VectorCopy(self->client->ps.origin, spot1);
	//spot1[2] += self->client->ps.viewheight;
	
	VectorCopy(other->client->ps.origin, spot2);
	//spot2[2] += other->client->ps.viewheight;
	
	trap_Trace(&trace, spot1, NULL, NULL, spot2, self->s.number, MASK_PLAYERSOLID);

	if(trace.entityNum == other->s.number)
		return qtrue;
	
	return qfalse;
}

///////////////////////////////////////////////////////////////////////
// Evaluate the best long range goal and send the bot on
// its way. This is a good time waster, so use it sparingly. 
// Do not call it for every think cycle.
///////////////////////////////////////////////////////////////////////
void ACEAI_PickLongRangeGoal(gentity_t * self)
{
#if 1
	int             i;
	int             node;
	float           weight, best_weight = 0.0;
	int             current_node, goal_node;
	gentity_t      *goal_ent;
	gclient_t      *cl;
	gentity_t      *player;
	float           cost;

	// look for a target 
	current_node = ACEND_FindClosestReachableNode(self, NODE_DENSITY, NODE_ALL);

	self->bs.current_node = current_node;

	if(current_node == -1)
	{
		self->bs.state = STATE_WANDER;
		self->bs.wander_timeout = level.time + 1000;
		self->bs.goal_node = -1;
		return;
	}

	///////////////////////////////////////////////////////
	// Items
	///////////////////////////////////////////////////////
	for(i = 0; i < num_items; i++)
	{
		if(item_table[i].ent == NULL /*|| item_table[i].ent->solid == SOLID_NOT */)	// ignore items that are not there.
			continue;

		cost = ACEND_FindCost(current_node, item_table[i].node);

		if(cost == INVALID || cost < 2)	// ignore invalid and very short hops
			continue;

		weight = ACEIT_ItemNeed(self, item_table[i].item);

		// If I am on team one and I have the flag for the other team....return it
		if(g_gametype.integer >= GT_CTF && (item_table[i].item == ITEMLIST_FLAG2 || item_table[i].item == ITEMLIST_FLAG1) &&
		   (self->client->sess.sessionTeam == TEAM_RED && self->client->ps.powerups[PW_BLUEFLAG] ||
		   (self->client->sess.sessionTeam == TEAM_BLUE && self->client->ps.powerups[PW_BLUEFLAG])))
		  {
			weight = 10.0;
		  }

		weight *= random();		// Allow random variations
		weight /= cost;			// Check against cost of getting there

		if(weight > best_weight)
		{
			best_weight = weight;
			goal_node = item_table[i].node;
			goal_ent = item_table[i].ent;
		}
	}

	///////////////////////////////////////////////////////
	// Players
	///////////////////////////////////////////////////////
	// This should be its own function and is for now just
	// finds a player to set as the goal.
	for(i = 0; i < g_maxclients.integer; i++)
	{
		cl = level.clients + i;
		player = level.gentities + cl->ps.clientNum;
		
		if(player == self)
		{
			continue;
		}
		
		if(cl->pers.connected != CON_CONNECTED)
		{
			continue;
		}

		node = ACEND_FindClosestReachableNode(player, NODE_DENSITY, NODE_ALL);
		cost = ACEND_FindCost(current_node, node);

		if(cost == INVALID || cost < 3)	// ignore invalid and very short hops
			continue;

		// Player carrying the flag?
		if(g_gametype.integer >= GT_CTF &&
		   (self->client->ps.powerups[PW_REDFLAG] || self->client->ps.powerups[PW_BLUEFLAG]))
			weight = 2.0;
		else
			weight = 0.3;

		weight *= random();		// Allow random variations
		weight /= cost;			// Check against cost of getting there

		if(weight > best_weight)
		{
			best_weight = weight;
			goal_node = node;
			goal_ent = player;
		}
	}

	// If do not find a goal, go wandering....
	if(best_weight == 0.0 || goal_node == INVALID)
	{
		self->bs.goal_node = INVALID;
		self->bs.state = STATE_WANDER;
		self->bs.wander_timeout = level.time + 1000;
		if(debug_mode)
			debug_printf("%s did not find a LR goal, wandering.\n", self->client->pers.netname);
		return;					// no path? 
	}

	// OK, everything valid, let's start moving to our goal.
	self->bs.state = STATE_MOVE;
	self->bs.tries = 0;			// Reset the count of how many times we tried this goal

	if(goal_ent != NULL && debug_mode)
		debug_printf("%s selected a %s at node %d for LR goal.\n", self->client->pers.netname, goal_ent->classname, goal_node);

	ACEND_SetGoal(self, goal_node);
#endif
}

///////////////////////////////////////////////////////////////////////
// Pick best goal based on importance and range. This function
// overrides the long range goal selection for items that
// are very close to the bot and are reachable.
///////////////////////////////////////////////////////////////////////
void ACEAI_PickShortRangeGoal(gentity_t * self)
{
	gentity_t        *target;
	float           weight, best_weight = 0.0;
	gentity_t        *best;
	int             index;

	// look for a target (should make more efficent later)
	target = G_FindRadius(NULL, self->s.origin, 300);

	while(target)
	{
		if(target->classname == NULL)
			return;

		// Missle avoidance code
		// Set our movetarget to be the rocket or grenade fired at us. 
		if(strcmp(target->classname, "rocket") == 0 || strcmp(target->classname, "grenade") == 0)
		{
			if(debug_mode)
				debug_printf("ROCKET ALERT!\n");

			self->bs.movetarget = target;
			return;
		}

		if(ACEIT_IsReachable(self, target->s.origin))
		{
			if(ACEAI_InFront(self, target))
			{
				index = ACEIT_ClassnameToIndex(target->classname);
				weight = ACEIT_ItemNeed(self, index);

				if(weight > best_weight)
				{
					best_weight = weight;
					best = target;
				}
			}
		}

		// next target
		target = G_FindRadius(target, self->s.origin, 300);
	}

	if(best_weight)
	{
		self->bs.movetarget = best;

		if(debug_mode && self->bs.goalentity != self->bs.movetarget)
			debug_printf("%s selected a %s for SR goal.\n", self->client->pers.netname, self->bs.movetarget->classname);

		self->bs.goalentity = best;

	}
}

///////////////////////////////////////////////////////////////////////
// Scan for enemy (simplifed for now to just pick any visible enemy)
///////////////////////////////////////////////////////////////////////
qboolean ACEAI_FindEnemy(gentity_t * self)
{
	int             i;
	gclient_t      *cl;
	gentity_t      *player;

	for(i = 0; i < g_maxclients.integer; i++)
	{
		cl = level.clients + i;
		player = level.gentities + cl->ps.clientNum;
		
		if(player == self)
		{
			continue;
		}
		
		if(cl->pers.connected != CON_CONNECTED)
		{
			continue;
		}
		
		if(g_gametype.integer >= GT_CTF &&  self->client->sess.sessionTeam == player->client->sess.sessionTeam)
		{
			// don't attack team mates
			continue;
		}

		if(player->health && ACEAI_InFront(self, player) && ACEAI_Visible(self, player) && trap_InPVS(self->client->ps.origin, player->client->ps.origin))
		{
			if(debug_mode && self->enemy != player)
				debug_printf("%s found enemy %s\n", self->client->pers.netname, player->client->pers.netname);
			
			self->enemy = player;
			return qtrue;
		}
	}

	return qfalse;

}

///////////////////////////////////////////////////////////////////////
// Hold fire with RL/BFG?
///////////////////////////////////////////////////////////////////////
qboolean ACEAI_CheckShot(gentity_t * self)
{
#if 0
	trace_t         tr;

	tr = gi.trace(self->s.origin, tv(-8, -8, -8), tv(8, 8, 8), self->enemy->s.origin, self, MASK_OPAQUE);

	// Blocked, do not shoot
	if(tr.fraction != 1.0)
		return false;

	return true;
#else
	return qfalse;
#endif
}

///////////////////////////////////////////////////////////////////////
// Choose the best weapon for bot (simplified)
///////////////////////////////////////////////////////////////////////
void ACEAI_ChooseWeapon(gentity_t * self)
{
#if 0
	float           range;
	vec3_t          v;

	// if no enemy, then what are we doing here?
	if(!self->enemy)
		return;

	// always favor the railgun
	if(ACEIT_ChangeWeapon(self, FindItem("railgun")))
		return;

	// Base selection on distance.
	VectorSubtract(self->s.origin, self->enemy->s.origin, v);
	range = VectorLength(v);

	// Longer range 
	if(range > 300)
	{
		// choose BFG if enough ammo
		if(self->client->pers.inventory[ITEMLIST_CELLS] > 50)
			if(ACEAI_CheckShot(self) && ACEIT_ChangeWeapon(self, FindItem("bfg10k")))
				return;

		if(ACEAI_CheckShot(self) && ACEIT_ChangeWeapon(self, FindItem("rocket launcher")))
			return;
	}

	// Only use GL in certain ranges and only on targets at or below our level
	if(range > 100 && range < 500 && self->enemy->s.origin[2] - 20 < self->s.origin[2])
		if(ACEIT_ChangeWeapon(self, FindItem("grenade launcher")))
			return;

	if(ACEIT_ChangeWeapon(self, FindItem("hyperblaster")))
		return;

	// Only use CG when ammo > 50
	if(self->client->pers.inventory[ITEMLIST_BULLETS] >= 50)
		if(ACEIT_ChangeWeapon(self, FindItem("chaingun")))
			return;

	if(ACEIT_ChangeWeapon(self, FindItem("machinegun")))
		return;

	if(ACEIT_ChangeWeapon(self, FindItem("super shotgun")))
		return;

	if(ACEIT_ChangeWeapon(self, FindItem("shotgun")))
		return;

	if(ACEIT_ChangeWeapon(self, FindItem("blaster")))
		return;

	return;
#endif
}


#endif