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

#include "g_local.h"

void            Weapon_GrapplingHook_Fire(gentity_t * ent);

/*
===============
G_DamageFeedback

Called just before a snapshot is sent to the given player.
Totals up all damage and generates both the player_state_t
damage values to that client for pain blends and kicks, and
global pain sound events for all clients.
===============
*/
void P_DamageFeedback(gentity_t * player)
{
	gclient_t      *client;
	float           count;
	vec3_t          angles;

	client = player->client;
	if(client->ps.pm_type == PM_DEAD)
	{
		return;
	}



	// total points of damage shot at the player this frame
	count = client->damage_blood + client->damage_armor;
	if(count == 0)
	{
		return;					// didn't take any damage
	}

	if(count > 255)
	{
		count = 255;
	}

	// send the information to the client

	// world damage (falling, slime, etc) uses a special code
	// to make the blend blob centered instead of positional
	if(client->damage_fromWorld)
	{
		client->ps.damagePitch = 255;
		client->ps.damageYaw = 255;

		client->damage_fromWorld = qfalse;
	}
	else
	{
		vectoangles(client->damage_from, angles);
		client->ps.damagePitch = angles[PITCH] / 360.0 * 256;
		client->ps.damageYaw = angles[YAW] / 360.0 * 256;
	}

	// play an apropriate pain sound
	if((level.time > player->pain_debounce_time) && !(player->flags & FL_GODMODE))
	{
		player->pain_debounce_time = level.time + 700;
		G_AddEvent(player, EV_PAIN, player->health);
		client->ps.damageEvent++;
	}


	client->ps.damageCount = count;

	//
	// clear totals
	//
	client->damage_blood = 0;
	client->damage_armor = 0;
	client->damage_knockback = 0;
}



/*
=============
P_WorldEffects

Check for lava / slime contents and drowning
=============
*/
void P_WorldEffects(gentity_t * ent)
{
	qboolean        envirosuit;
	qboolean        spawnprotect;
	int             waterlevel;


	if(ent->client->noclip)
	{
		ent->client->airOutTime = level.time + 12000;	// don't need air
		return;
	}

	waterlevel = ent->waterlevel;

	envirosuit = ent->client->ps.powerups[PW_BATTLESUIT] > level.time;
	spawnprotect = ent->client->ps.powerups[PW_SPAWNPROT] > level.time;


	//
	// check for drowning
	//
	if(waterlevel == 3)
	{
		// envirosuit give air
		if(envirosuit || spawnprotect)
		{
			ent->client->airOutTime = level.time + 10000;
		}

		// if out of air, start drowning
		if(ent->client->airOutTime < level.time)
		{
			// drown!
			ent->client->airOutTime += 1000;
			if(ent->health > 0)
			{
				// take more damage the longer underwater
				ent->damage += 2;
				if(ent->damage > 15)
					ent->damage = 15;

				// play a gurp sound instead of a normal pain sound
				if(ent->health <= ent->damage)
				{
					G_Sound(ent, CHAN_VOICE, G_SoundIndex("*drown.wav"));
				}
				else if(rand() & 1)
				{
					G_Sound(ent, CHAN_VOICE, G_SoundIndex("sound/player/gurp1.wav"));
				}
				else
				{
					G_Sound(ent, CHAN_VOICE, G_SoundIndex("sound/player/gurp2.wav"));
				}

				// don't play a normal pain sound
				ent->pain_debounce_time = level.time + 200;

				G_Damage(ent, NULL, NULL, NULL, NULL, ent->damage, DAMAGE_NO_ARMOR, MOD_WATER);
			}
		}
	}
	else
	{
		ent->client->airOutTime = level.time + 12000;
		ent->damage = 2;
	}

	//
	// check for sizzle damage (move to pmove?)
	//
	if(waterlevel && (ent->watertype & (CONTENTS_LAVA | CONTENTS_SLIME)))
	{
		if(ent->health > 0)
		{

			if(envirosuit || spawnprotect)
			{
				G_AddEvent(ent, EV_POWERUP_BATTLESUIT, 0);
			}
			else
			{
				if(ent->watertype & CONTENTS_LAVA)
				{
					G_Damage(ent, NULL, NULL, NULL, NULL, 8 * waterlevel, 0, MOD_LAVA);
				}

				if(ent->watertype & CONTENTS_SLIME)
				{
					G_Damage(ent, NULL, NULL, NULL, NULL, 4 * waterlevel, 0, MOD_SLIME);
				}
			}
		}
	}

	if(ent->onFireEnd && ent->client)
	{
		if(level.time - ent->client->lastBurnTime >= 399)
		{

			// JPW NERVE server-side incremental damage routine / player damage/health is int (not float)
			// so I can't allocate 1.5 points per server tick, and 1 is too weak and 2 is too strong.  
			// solution: allocate damage far less often (MIN_BURN_INTERVAL often) and do more damage.
			// That way minimum resolution (1 point) damage changes become less critical.

			ent->client->lastBurnTime = level.time;
			if((ent->onFireEnd > level.time) && (ent->health > 0))
			{
				gentity_t      *attacker;

				attacker = &g_entities[ent->flameBurnEnt];
				G_Damage(ent, attacker, attacker, vec3_origin, attacker->r.currentOrigin, 5, 0, MOD_FLAMETHROWER);
			}
		}
	}

}



/*
===============
G_SetClientSound
===============
*/
void G_SetClientSound(gentity_t * ent)
{
#ifdef MISSIONPACK
	if(ent->s.eFlags & EF_TICKING)
	{
		ent->client->ps.loopSound = G_SoundIndex("sound/weapons/proxmine/wstbtick.wav");
	}
	else
#endif
	if(ent->waterlevel && (ent->watertype & (CONTENTS_LAVA | CONTENTS_SLIME)))
	{
		ent->client->ps.loopSound = level.snd_fry;
	}
	else
	{
		ent->client->ps.loopSound = 0;
	}
}



//==============================================================

/*
==============
ClientImpacts
==============
*/
void ClientImpacts(gentity_t * ent, pmove_t * pm)
{
	int             i, j;
	trace_t         trace;
	gentity_t      *other;

	memset(&trace, 0, sizeof(trace));
	for(i = 0; i < pm->numtouch; i++)
	{
		for(j = 0; j < i; j++)
		{
			if(pm->touchents[j] == pm->touchents[i])
			{
				break;
			}
		}
		if(j != i)
		{
			continue;			// duplicated
		}
		other = &g_entities[pm->touchents[i]];

		if((ent->r.svFlags & SVF_BOT) && (ent->touch))
		{
			ent->touch(ent, other, &trace);
		}

		if(!other->touch)
		{
			continue;
		}

		other->touch(other, ent, &trace);
	}

}

/*
============
G_TouchTriggers

Find all trigger entities that ent's current position touches.
Spectators will only interact with teleporters.
============
*/
void G_TouchTriggers(gentity_t * ent)
{
	int             i, num;
	int             touch[MAX_GENTITIES];
	gentity_t      *hit;
	trace_t         trace;
	vec3_t          mins, maxs;
	static vec3_t   range = { 40, 40, 52 };

	if(!ent->client)
	{
		return;
	}

	// dead clients don't activate triggers!
	if(ent->client->ps.stats[STAT_HEALTH] <= 0)
	{
		return;
	}

	VectorSubtract(ent->client->ps.origin, range, mins);
	VectorAdd(ent->client->ps.origin, range, maxs);

	num = trap_EntitiesInBox(mins, maxs, touch, MAX_GENTITIES);

	// can't use ent->absmin, because that has a one unit pad
	VectorAdd(ent->client->ps.origin, ent->r.mins, mins);
	VectorAdd(ent->client->ps.origin, ent->r.maxs, maxs);

	for ( i=0 ; i<num ; i++ ) {
		hit = &g_entities[touch[i]];

		if ( !hit->touch && !ent->touch ) {
			continue;
		}
		if(!(hit->flags & FL_DROPPED_ITEM) &&
		   strcmp(hit->classname, "team_CTF_blueflag") == 0 &&
			ent->client->sess.sessionTeam == TEAM_BLUE &&
			ent->client->ps.powerups[PW_BLUEFLAG] ) {
		}
		else if(!(hit->flags & FL_DROPPED_ITEM) &&
				strcmp(hit->classname, "team_CTF_redflag") == 0 &&
			ent->client->sess.sessionTeam == TEAM_RED &&
			ent->client->ps.powerups[PW_REDFLAG] ) {
		}
		else if ( !( hit->r.contents & CONTENTS_TRIGGER ) ) {
			continue;
		}

		// ignore most entities if a spectator
		if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
			if(hit->s.eType != ET_TELEPORT_TRIGGER &&
			   // this is ugly but adding a new ET_? type will
			   // most likely cause network incompatibilities
				hit->touch != Touch_DoorTrigger) {
				continue;
			}
		}

		// use seperate code for determining if an item is picked up
		// so you don't have to actually contact its bounding box
		if(hit->s.eType == ET_ITEM)
		{
			if(!BG_PlayerTouchesItem(&ent->client->ps, &hit->s, level.time))
			{
				continue;
			}
		}
		else
		{
			if(!trap_EntityContact(mins, maxs, hit))
			{
				continue;
			}
		}

		memset(&trace, 0, sizeof(trace));

		if(hit->touch)
		{
			hit->touch(hit, ent, &trace);
		}

		if((ent->r.svFlags & SVF_BOT) && (ent->touch))
		{
			ent->touch(ent, hit, &trace);
		}
	}

	// if we didn't touch a jump pad this pmove frame
	if(ent->client->ps.jumppad_frame != ent->client->ps.pmove_framecount)
	{
		ent->client->ps.jumppad_frame = 0;
		ent->client->ps.jumppad_ent = 0;
	}
}

/*
=================
SpectatorThink
=================
*/
void SpectatorThink(gentity_t * ent, usercmd_t * ucmd)
{
	pmove_t         pm;
	gclient_t      *client;

	client = ent->client;

	if(client->sess.spectatorState != SPECTATOR_FOLLOW)
	{
		client->ps.pm_type = PM_NOCLIP;
		client->ps.speed = 400;	// faster than normal
		client->noclip = qtrue;
		client->ps.weapon = WP_NONE;
		// set up for pmove
		memset(&pm, 0, sizeof(pm));
		pm.ps = &client->ps;
		pm.cmd = *ucmd;
		pm.tracemask = 0;		// spectators can fly through bodies
		pm.trace = trap_Trace;
		pm.pointcontents = trap_PointContents;


		client->ps.speed *= 1.9;

		// perform a pmove
		Pmove(&pm);
		// save results of pmove
		VectorCopy(client->ps.origin, ent->s.origin);

		G_TouchTriggers(ent);
		trap_UnlinkEntity(ent);
	}

	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;

	// attack button cycles through spectators
	if((client->buttons & BUTTON_ATTACK) && !(client->oldbuttons & BUTTON_ATTACK))
	{
		Cmd_FollowCycle_f(ent, 1);
	}
	if(client->pers.cmd.upmove)
	{
		StopFollowing(ent);
	}

}



/*
=================
ClientInactivityTimer

Returns qfalse if the client is dropped
=================
*/
qboolean ClientInactivityTimer(gclient_t * client)
{
	if(!g_inactivity.integer)
	{
		// give everyone some time, so if the operator sets g_inactivity during
		// gameplay, everyone isn't kicked
		client->inactivityTime = level.time + 60 * 1000;
		client->inactivityWarning = qfalse;
	}
	else if(client->pers.cmd.forwardmove ||
			client->pers.cmd.rightmove || client->pers.cmd.upmove || (client->pers.cmd.buttons & BUTTON_ATTACK))
	{
		client->inactivityTime = level.time + g_inactivity.integer * 1000;
		client->inactivityWarning = qfalse;
	}
	else if(!client->pers.localClient)
	{
		if(level.time > client->inactivityTime)
		{
			trap_DropClient(client - level.clients, "Dropped due to inactivity");
			return qfalse;
		}
		if(level.time > client->inactivityTime - 10000 && !client->inactivityWarning)
		{
			client->inactivityWarning = qtrue;
			trap_SendServerCommand(client - level.clients, "cp \"^1Ten seconds until inactivity drop!\n\"");
		}
	}
	return qtrue;
}

/*
==================
ClientTimerActions

Actions that happen once a second
==================
*/
void ClientTimerActions(gentity_t * ent, int msec)
{
	gclient_t      *client;

#ifdef MISSIONPACK
	int             maxHealth;
#endif

	client = ent->client;
	client->timeResidual += msec;

	while(client->timeResidual >= 1000)
	{
		client->timeResidual -= 1000;

		// regenerate
		if(client->ps.powerups[PW_REGEN])
		{
			if(ent->health < client->ps.stats[STAT_MAX_HEALTH])
			{
				ent->health += 15;
				if(ent->health > client->ps.stats[STAT_MAX_HEALTH] * 1.1)
				{
					ent->health = client->ps.stats[STAT_MAX_HEALTH] * 1.1;
				}
				G_AddEvent(ent, EV_POWERUP_REGEN, 0);
			}
			else if(ent->health < client->ps.stats[STAT_MAX_HEALTH] * 2)
			{
				ent->health += 10;
				if(ent->health > client->ps.stats[STAT_MAX_HEALTH] * 2)
				{

					ent->health = client->ps.stats[STAT_MAX_HEALTH] * 2;
				}
				G_AddEvent(ent, EV_POWERUP_REGEN, 0);
			}
			else if(client->ps.stats[STAT_ARMOR] < client->ps.stats[STAT_MAX_HEALTH])
			{
				client->ps.stats[STAT_ARMOR] += 15;
				if(client->ps.stats[STAT_ARMOR] > client->ps.stats[STAT_MAX_HEALTH] * 1.1)
				{
					client->ps.stats[STAT_ARMOR] = client->ps.stats[STAT_MAX_HEALTH] * 1.1;
				}
				G_AddEvent(ent, EV_POWERUP_REGEN, 0);
			}
			else if(client->ps.stats[STAT_ARMOR] < client->ps.stats[STAT_MAX_HEALTH] * 2)
			{
				client->ps.stats[STAT_ARMOR] += 10;
				if(client->ps.stats[STAT_ARMOR] > client->ps.stats[STAT_MAX_HEALTH] * 2)
				{
					client->ps.stats[STAT_ARMOR] = client->ps.stats[STAT_MAX_HEALTH] * 2;
				}
				G_AddEvent(ent, EV_POWERUP_REGEN, 0);
			}

		}
		else
		{
			if(Instagib.integer == 0)
			{

				// count down health when over max unless player has regen
				if(client->ps.powerups[PW_REGEN] == 0)
				{
					if(ent->health > client->ps.stats[STAT_MAX_HEALTH])
					{
						ent->health--;
					}
				}
				// count down armor when over max
				/*      if ( client->ps.stats[STAT_ARMOR] > client->ps.stats[STAT_MAX_HEALTH] ) {
				   client->ps.stats[STAT_ARMOR]--;
				   } */
			}
		}
	}

#ifdef MISSIONPACK
	if(bg_itemlist[client->ps.stats[STAT_PERSISTANT_POWERUP]].giTag == PW_AMMOREGEN)
	{
		int             w, max, inc, t, i;
		int             weapList[] =
			{ WP_MACHINEGUN, WP_SHOTGUN, WP_GRENADE_LAUNCHER, WP_ROCKET_LAUNCHER, WP_LIGHTNING, WP_RAILGUN, WP_IRAILGUN,
			WP_PLASMAGUN, WP_BFG, WP_NAILGUN, WP_PROX_LAUNCHER, WP_CHAINGUN
		};
		int             weapCount = sizeof(weapList) / sizeof(int);

		//
		for(i = 0; i < weapCount; i++)
		{
			w = weapList[i];

			switch (w)
			{
				case WP_MACHINEGUN:
					max = 50;
					inc = 4;
					t = 1000;
					break;
				case WP_SHOTGUN:
					max = 10;
					inc = 1;
					t = 1500;
					break;
				case WP_GRENADE_LAUNCHER:
					max = 10;
					inc = 1;
					t = 2000;
					break;
				case WP_ROCKET_LAUNCHER:
					max = 10;
					inc = 1;
					t = 1750;
					break;
				case WP_LIGHTNING:
					max = 50;
					inc = 5;
					t = 1500;
					break;
				case WP_RAILGUN:
					max = 10;
					inc = 1;
					t = 1750;
					break;
				case WP_IRAILGUN:
					max = 10;
					inc = 1;
					t = 1750;
					break;
				case WP_PLASMAGUN:
					max = 50;
					inc = 5;
					t = 1500;
					break;
				case WP_BFG:
					max = 10;
					inc = 1;
					t = 4000;
					break;
				case WP_NAILGUN:
					max = 10;
					inc = 1;
					t = 1250;
					break;
				case WP_PROX_LAUNCHER:
					max = 5;
					inc = 1;
					t = 2000;
					break;
				case WP_CHAINGUN:
					max = 100;
					inc = 5;
					t = 1000;
					break;
				default:
					max = 0;
					inc = 0;
					t = 1000;
					break;
			}
			client->ammoTimes[w] += msec;
			if(client->ps.ammo[w] >= max)
			{
				client->ammoTimes[w] = 0;
			}
			if(client->ammoTimes[w] >= t)
			{
				while(client->ammoTimes[w] >= t)
					client->ammoTimes[w] -= t;
				client->ps.ammo[w] += inc;
				if(client->ps.ammo[w] > max)
				{
					client->ps.ammo[w] = max;
				}
			}
		}
	}
#endif
}

/*
====================
ClientIntermissionThink
====================
*/
void ClientIntermissionThink(gclient_t * client)
{
	gentity_t      *ent;

//  int i;


	ent = &g_entities[client->ps.clientNum];
	client->ps.eFlags &= ~EF_TALK;
	client->ps.eFlags &= ~EF_FIRING;

	// the level will exit when everyone wants to or after timeouts

	// swap and latch button actions
	client->oldbuttons = client->buttons;
	client->buttons = client->pers.cmd.buttons;
	if ( client->buttons & BUTTON_ATTACK & ( client->oldbuttons ^ client->buttons ) ) {
		// this used to be an ^1 but once a player says ready, it should stick
		//  StopFollowing( ent );
		client->readyToExit = 1;
	}
//  if ( client->buttons & BUTTON_USE_HOLDABLE & ( client->oldbuttons ^ client->buttons ) ) {
//      Cmd_FollowCycle_f( ent, 1 );
//  }

//  for ( i = 0 ; i < level.numConnectedClients ; i++ ) {
//          Cmd_Statistics_f( &g_entities[i] );
//  }

	if(client->pers.cmd.upmove && client->sess.spectatorState != SPECTATOR_FOLLOW){
		//  Cmd_FollowCycle_f( ent, 1 );
//      Cmd_StatCycle_f( ent, 1 );
	}

}


/*
================
ClientEvents

Events will be passed on to the clients for presentation,
but any server game effects are handled here
================
*/
void ClientEvents(gentity_t * ent, int oldEventSequence, int charge)
{
//  int     i, j;
	int             i;
	int             e;
	int             event;
	gclient_t      *client;
	int             damage;
	vec3_t          dir;
	vec3_t          origin, angles;

//  qboolean    fired;
//  gitem_t *item;
//  gentity_t *drop;

	client = ent->client;

	if(oldEventSequence < client->ps.eventSequence - MAX_PS_EVENTS)
	{
		oldEventSequence = client->ps.eventSequence - MAX_PS_EVENTS;
	}
	for(i = oldEventSequence; i < client->ps.eventSequence; i++)
	{
		event = client->ps.events[i & (MAX_PS_EVENTS - 1)];

		switch (event)
		{
			case EV_FALL_MEDIUM:
			case EV_FALL_FAR:
				if(ent->client->ps.stats[STAT_HEALTH] > 0)
				{
					if(ent->s.eType != ET_PLAYER)
					{
						break;	// not in the player model
					}
					if(g_dmflags.integer & DF_NO_FALLING)
					{
						break;
					}
					if(event == EV_FALL_FAR)
					{
						damage = 10;
					}
					else
					{
						damage = 5;
					}
					VectorSet(dir, 0, 0, 1);
					ent->pain_debounce_time = level.time + 200;	// no normal pain sound
					G_Damage(ent, NULL, NULL, NULL, NULL, damage, 0, MOD_FALLING);
				}
				break;

			case EV_FIRE_WEAPON:

				if(ent->client->ps.weapon != WP_IRAILGUN)
				{
					FireWeapon(ent, 0);
				}
				else
				{
					e = ent->client->ps.weaponTime;
					FireWeapon(ent, charge);
				}
				break;

			case EV_USE_ITEM1:	// teleporter
				// drop flags in CTF
				Team_DropFlags(ent);
				SelectSpawnPoint(ent->client->ps.origin, origin, angles);
				TeleportPlayer(ent, origin, angles);
				break;

			case EV_USE_ITEM2:	// medkit
				ent->health = ent->client->ps.stats[STAT_MAX_HEALTH] + 25;

				break;

			default:
				break;
		}
	}

}

#ifdef MISSIONPACK
/*
==============
StuckInOtherClient
==============
*/
static int StuckInOtherClient(gentity_t * ent)
{
	int             i;
	gentity_t      *ent2;

	ent2 = &g_entities[0];
	for(i = 0; i < MAX_CLIENTS; i++, ent2++)
	{
		if(ent2 == ent)
		{
			continue;
		}
		if(!ent2->inuse)
		{
			continue;
		}
		if(!ent2->client)
		{
			continue;
		}
		if(ent2->health <= 0)
		{
			continue;
		}
		//
		if(ent2->r.absmin[0] > ent->r.absmax[0])
			continue;
		if(ent2->r.absmin[1] > ent->r.absmax[1])
			continue;
		if(ent2->r.absmin[2] > ent->r.absmax[2])
			continue;
		if(ent2->r.absmax[0] < ent->r.absmin[0])
			continue;
		if(ent2->r.absmax[1] < ent->r.absmin[1])
			continue;
		if(ent2->r.absmax[2] < ent->r.absmin[2])
			continue;
		return qtrue;
	}
	return qfalse;
}
#endif

void            BotTestSolid(vec3_t origin);

/*
==============
SendPendingPredictableEvents
==============
*/
void SendPendingPredictableEvents(playerState_t * ps)
{
	gentity_t      *t;
	int             event, seq;
	int             extEvent, number;
	gentity_t      *attacker;
	qboolean        quad;

	attacker = &g_entities[ps->persistant[PERS_ATTACKER]];
	if(attacker->client->ps.powerups[PW_QUAD] >= 1)
	{
		quad = qtrue;
	}
	else
	{
		quad = qfalse;
	}

	// if there are still events pending
	if(ps->entityEventSequence < ps->eventSequence)
	{
		// create a temporary entity for this event which is sent to everyone
		// except the client who generated the event
		seq = ps->entityEventSequence & (MAX_PS_EVENTS - 1);
		event = ps->events[seq] | ((ps->entityEventSequence & 3) << 8);
		// set external event to zero before calling BG_PlayerStateToEntityState
		extEvent = ps->externalEvent;
		ps->externalEvent = 0;
		// create temporary entity for event
		t = G_TempEntity(ps->origin, event);
		number = t->s.number;
		BG_PlayerStateToEntityState(ps, &t->s, qtrue, quad);
		t->s.number = number;
		t->s.eType = ET_EVENTS + event;
		t->s.eFlags |= EF_PLAYER_EVENT;
		t->s.otherEntityNum = ps->clientNum;
		// send to everyone except the client who generated the event
		t->r.svFlags |= SVF_NOTSINGLECLIENT;
		t->r.singleClient = ps->clientNum;
		// set back external event
		ps->externalEvent = extEvent;
	}
}

/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame on fast clients.

If "g_synchronousClients 1" is set, this will be called exactly
once for each server frame, which makes for smooth demo recording.
==============
*/
void ClientThink_real(gentity_t * ent)
{
	gclient_t      *client;
	pmove_t         pm;
	int             oldEventSequence;
	int             msec;
	usercmd_t      *ucmd;
	int             Rcharge;
	gentity_t      *attacker;
	qboolean        quad;

//  int         i;
//  cvarTable_t *cv;

	client = ent->client;

	// don't think if the client is not yet connected (and thus not yet spawned in)
	if(client->pers.connected != CON_CONNECTED)
	{
		return;
	}
	// mark the time, so the connection sprite can be removed
	ucmd = &ent->client->pers.cmd;

	// sanity check the command time to prevent speedup cheating
	if(ucmd->serverTime > level.time + 200)
	{
		ucmd->serverTime = level.time + 200;
//      G_Printf("serverTime <<<<<\n" );
	}
	if(ucmd->serverTime < level.time - 1000)
	{
		ucmd->serverTime = level.time - 1000;
//      G_Printf("serverTime >>>>>\n" );
	}
	msec = ucmd->serverTime - client->ps.commandTime;
	// following others may result in bad times, but we still want
	// to check for follow toggles
	if(msec < 1 && client->sess.spectatorState != SPECTATOR_FOLLOW)
	{
		return;
	}
	if(msec > 200)
	{
		msec = 200;
	}

	if(pmove_msec.integer < 8)
	{
		trap_Cvar_Set("pmove_msec", "8");
	}
	else if(pmove_msec.integer > 33)
	{
		trap_Cvar_Set("pmove_msec", "33");
	}

	if(pmove_fixed.integer || client->pers.pmoveFixed)
	{
		ucmd->serverTime = ((ucmd->serverTime + pmove_msec.integer - 1) / pmove_msec.integer) * pmove_msec.integer;
		//if (ucmd->serverTime - client->ps.commandTime <= 0)
		//  return;
	}

	//
	// check for exiting intermission
	//
	if(level.intermissiontime)
	{
		ClientIntermissionThink(client);
		return;
	}

	if(client->sess.speconly == 1 &&
		client->sess.sessionTeam != TEAM_SPECTATOR ){
		SetTeam(ent, "s");
	}

	// spectators don't do much
	if(client->sess.sessionTeam == TEAM_SPECTATOR)
	{
		if(client->sess.spectatorState == SPECTATOR_SCOREBOARD)
		{
			return;
		}
		SpectatorThink(ent, ucmd);
		return;
	}

	// check for inactivity timer, but never drop the local client of a non-dedicated server
	if(!ClientInactivityTimer(client))
	{
		return;
	}

	// clear the rewards if time
	if ( level.time > client->rewardTime ) {
		client->ps.eFlags &= ~( EF_AWARD_IMPRESSIVE | EF_AWARD_EXCELLENT | EF_AWARD_GAUNTLET | EF_AWARD_ASSIST | EF_AWARD_DEFEND | EF_AWARD_CAP );
	}

	if(client->noclip)
	{
		client->ps.pm_type = PM_NOCLIP;
	}
	else if(level.intermissionQueued)
	{


	}
	else if(client->ps.stats[STAT_HEALTH] <= 0)
	{
		client->ps.pm_type = PM_DEAD;
	}
	else
	{
		client->ps.pm_type = PM_NORMAL;
	}
	if(client->ps.stats[STAT_HEALTH] > 200)
	{
		client->ps.stats[STAT_HEALTH] = 200;
	}
	if(client->ps.stats[STAT_ARMOR] > 200)
	{
		client->ps.stats[STAT_ARMOR] = 200;
	}
	client->ps.gravity = g_gravity.value;

	// set speed
	client->ps.speed = g_speed.value;
	client->ps.speed = 310;

	if(Instagib.integer == 1)
	{
		if(client->ps.stats[STAT_WEAPONS] & (1 << WP_MACHINEGUN))
		{
			trap_SendServerCommand(ent - g_entities, va("print \"^3InstaGib Mode is ^5Enabled.\n\""));
			if(client->ps.stats[STAT_WEAPONS] & (1 << WP_MACHINEGUN))
			{
				client->ps.ammo[WP_MACHINEGUN] = 0;
				client->ps.stats[STAT_WEAPONS] -= (1 << WP_MACHINEGUN);
			}
			if(client->ps.stats[STAT_WEAPONS] & (1 << WP_SHOTGUN))
			{
				client->ps.ammo[WP_SHOTGUN] = 0;
				client->ps.stats[STAT_WEAPONS] -= (1 << WP_SHOTGUN);
			}
			if(client->ps.stats[STAT_WEAPONS] & (1 << WP_GRENADE_LAUNCHER))
			{
				client->ps.ammo[WP_GRENADE_LAUNCHER] = 0;
				client->ps.stats[STAT_WEAPONS] -= (1 << WP_GRENADE_LAUNCHER);
			}
			if(client->ps.stats[STAT_WEAPONS] & (1 << WP_ROCKET_LAUNCHER))
			{
				client->ps.ammo[WP_ROCKET_LAUNCHER] = 0;
				client->ps.stats[STAT_WEAPONS] -= (1 << WP_ROCKET_LAUNCHER);
			}
			if(client->ps.stats[STAT_WEAPONS] & (1 << WP_LIGHTNING))
			{
				client->ps.ammo[WP_LIGHTNING] = 0;
				client->ps.stats[STAT_WEAPONS] -= (1 << WP_LIGHTNING);
			}
			if(client->ps.stats[STAT_WEAPONS] & (1 << WP_RAILGUN))
			{
				client->ps.ammo[WP_RAILGUN] = 0;
				client->ps.stats[STAT_WEAPONS] -= (1 << WP_RAILGUN);
			}
			if(client->ps.stats[STAT_WEAPONS] & (1 << WP_PLASMAGUN))
			{
				client->ps.ammo[WP_PLASMAGUN] = 0;
				client->ps.stats[STAT_WEAPONS] -= (1 << WP_PLASMAGUN);
			}
			if(client->ps.stats[STAT_WEAPONS] & (1 << WP_BFG))
			{
				client->ps.ammo[WP_BFG] = 0;
				client->ps.stats[STAT_WEAPONS] -= (1 << WP_BFG);
			}
			if(client->ps.stats[STAT_WEAPONS] & (1 << WP_IRAILGUN))
			{
				client->ps.ammo[WP_IRAILGUN] = 0;
				client->ps.stats[STAT_WEAPONS] -= (1 << WP_IRAILGUN);
			}
			if(client->ps.stats[STAT_WEAPONS] & (1 << WP_FLAMETHROWER))
			{
				client->ps.ammo[WP_FLAMETHROWER] = 0;
				client->ps.stats[STAT_WEAPONS] -= (1 << WP_FLAMETHROWER);
			}
		}

		if(InstaWeapon.integer == 0)
		{
			if(!(client->ps.stats[STAT_WEAPONS] & (1 << WP_RAILGUN)))
			{
				trap_SendServerCommand(ent - g_entities, va("print \"^3Railgun InstaGib Mode is ^5Enabled.\n\""));

				client->ps.stats[STAT_WEAPONS] |= (1 << WP_RAILGUN);
				client->ps.ammo[WP_RAILGUN] = 999;



				if(client->ps.stats[STAT_WEAPONS] & (1 << WP_ROCKET_LAUNCHER))
				{
					client->ps.ammo[WP_ROCKET_LAUNCHER] = 0;
					client->ps.stats[STAT_WEAPONS] -= (1 << WP_ROCKET_LAUNCHER);
				}

				if(client->ps.stats[STAT_WEAPONS] & (1 << WP_IRAILGUN))
				{
					client->ps.ammo[WP_IRAILGUN] = 0;
					client->ps.stats[STAT_WEAPONS] -= (1 << WP_IRAILGUN);
				}

			}
		}
		else if(InstaWeapon.integer == 1)
		{
			if(!(client->ps.stats[STAT_WEAPONS] & (1 << WP_ROCKET_LAUNCHER)))
			{
				trap_SendServerCommand(ent - g_entities, va("print \"^3Rocket InstaGib Mode is ^5Enabled.\n\""));
				client->ps.stats[STAT_WEAPONS] |= (1 << WP_ROCKET_LAUNCHER);
				client->ps.ammo[WP_ROCKET_LAUNCHER] = 999;


				if(client->ps.stats[STAT_WEAPONS] & (1 << WP_RAILGUN))
				{
					client->ps.ammo[WP_RAILGUN] = 0;
					client->ps.stats[STAT_WEAPONS] -= (1 << WP_RAILGUN);
				}
			}
			if(client->ps.stats[STAT_WEAPONS] & (1 << WP_IRAILGUN))
			{
				client->ps.ammo[WP_IRAILGUN] = 0;
				client->ps.stats[STAT_WEAPONS] -= (1 << WP_IRAILGUN);
			}

		}
		else if(InstaWeapon.integer == 2)
		{
			if(!(client->ps.stats[STAT_WEAPONS] & (1 << WP_ROCKET_LAUNCHER)))
			{
				trap_SendServerCommand(ent - g_entities, va("print \"^3Rocket & Rail InstaGib Mode is ^5Enabled.\n\""));
				client->ps.stats[STAT_WEAPONS] |= (1 << WP_ROCKET_LAUNCHER);
				client->ps.ammo[WP_ROCKET_LAUNCHER] = 999;
			}
			if(!(client->ps.stats[STAT_WEAPONS] & (1 << WP_RAILGUN)))
			{
				client->ps.stats[STAT_WEAPONS] |= (1 << WP_RAILGUN);
				client->ps.ammo[WP_RAILGUN] = 999;
			}
		}
	}
	else
	{
		if(!(client->ps.stats[STAT_WEAPONS] & (1 << WP_MACHINEGUN)))
		{
			trap_SendServerCommand(ent - g_entities, va("print \"^3InstaGib Mode is ^5Disabled.\n\""));
			client->ps.stats[STAT_WEAPONS] |= (1 << WP_MACHINEGUN);
			client->ps.ammo[WP_MACHINEGUN] = 150;
			client->firstTimeW = qfalse;

			if(client->ps.stats[STAT_WEAPONS] & (1 << WP_RAILGUN) && client->ps.ammo[WP_RAILGUN] == 999)
			{
				client->ps.ammo[WP_RAILGUN] = 0;
				client->ps.stats[STAT_WEAPONS] -= (1 << WP_RAILGUN);
			}
			if(client->ps.stats[STAT_WEAPONS] & (1 << WP_ROCKET_LAUNCHER) && client->ps.ammo[WP_ROCKET_LAUNCHER] == 999)
			{
				client->ps.ammo[WP_ROCKET_LAUNCHER] = 0;
				client->ps.stats[STAT_WEAPONS] -= (1 << WP_ROCKET_LAUNCHER);
			}
			if(client->ps.stats[STAT_WEAPONS] & (1 << WP_IRAILGUN) && client->ps.ammo[WP_IRAILGUN] == 999)
			{
				client->ps.ammo[WP_IRAILGUN] = 0;
				client->ps.stats[STAT_WEAPONS] -= (1 << WP_IRAILGUN);
			}
		}

	}

	WarmupWeapons(ent);

	if(client->ps.powerups[PW_HASTE])
	{
		client->ps.speed *= 1.3;
	}

	// Let go of the hook if we aren't firing
	// set up for pmove
	oldEventSequence = client->ps.eventSequence;

	memset(&pm, 0, sizeof(pm));

	// check for the hit-scan gauntlet, don't let the action
	// go through as an attack unless it actually hits something
	if(client->ps.weapon == WP_GAUNTLET && !(ucmd->buttons & BUTTON_TALK) &&
	   (ucmd->buttons & BUTTON_ATTACK) && client->ps.weaponTime <= 0)
	{
		pm.gauntletHit = CheckGauntletAttack(ent);
	}

	if(ent->flags & FL_FORCE_GESTURE)
	{
		ent->flags &= ~FL_FORCE_GESTURE;
		ent->client->pers.cmd.buttons |= BUTTON_GESTURE;
	}


	pm.ps = &client->ps;
	pm.cmd = *ucmd;
	if(pm.ps->pm_type == PM_DEAD)
	{
		pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;
	}
	else if(ent->r.svFlags & SVF_BOT)
	{
		pm.tracemask = MASK_PLAYERSOLID | CONTENTS_BOTCLIP;
	}
	else
	{
		pm.tracemask = MASK_PLAYERSOLID;
	}
	pm.trace = trap_Trace;
	pm.pointcontents = trap_PointContents;
	pm.debugLevel = g_debugMove.integer;
	pm.noFootsteps = (g_dmflags.integer & DF_NO_FOOTSTEPS) > 0;

	pm.pmove_fixed = pmove_fixed.integer | client->pers.pmoveFixed;
	pm.pmove_msec = pmove_msec.integer;

	VectorCopy(client->ps.origin, client->oldOrigin);


	if(level.intermissionQueued)
	{
		pm.cmd.buttons = 0;
		pm.cmd.forwardmove = 0;
		pm.cmd.rightmove = 0;
		pm.cmd.upmove = 0;
		client->ps.eFlags &= ~EF_FIRING;
		if(client->ps.stats[STAT_HEALTH] > 0)
		{
			client->ps.pm_type = PM_FREEZE;
		}
	}


	Pmove(&pm);

	// save results of pmove
	if(ent->client->ps.eventSequence != oldEventSequence)
	{
		ent->eventTime = level.time;
	}
		attacker = &g_entities[ent->client->ps.persistant[PERS_ATTACKER]];
		if(attacker->client->ps.powerups[PW_QUAD] >= 1){
			quad = qtrue;
		}else{
			quad = qfalse;
		}
	if (g_smoothClients.integer) {
		BG_PlayerStateToEntityStateExtraPolate( &ent->client->ps, &ent->s, ent->client->ps.commandTime, qtrue,quad );
	}
	else {


		if(!(ent->client->ps.generic1 & GNF_ONFIRE) && level.time < ent->onFireEnd){
			ent->client->ps.generic1 |= GNF_ONFIRE;
		}else if(level.time >= ent->onFireEnd){
			ent->client->ps.generic1 &= ~GNF_ONFIRE;
		}

		BG_PlayerStateToEntityState( &ent->client->ps, &ent->s, qtrue , quad);
	}
	SendPendingPredictableEvents(&ent->client->ps);

	if((pm.cmd.buttons & 32) && ent->client->ps.pm_type != PM_DEAD && !ent->client->hookhasbeenfired)
	{
		Weapon_GrapplingHook_Fire(ent);
		ent->client->hookhasbeenfired = qtrue;
	}
	if(!(pm.cmd.buttons & 32) && ent->client->ps.pm_type != PM_DEAD && ent->client->hookhasbeenfired && ent->client->fireHeld)
	{
		ent->client->fireHeld = qfalse;
		ent->client->hookhasbeenfired = qfalse;
	}
	if(client->hook && client->fireHeld == qfalse)
		Weapon_HookFree(client->hook);

	if(!(ent->client->ps.eFlags & EF_FIRING))
	{
		client->fireHeld = qfalse;	// for grapple
	}

	// use the snapped origin for linking so it matches client predicted versions
	VectorCopy(ent->s.pos.trBase, ent->r.currentOrigin);

	VectorCopy(pm.mins, ent->r.mins);
	VectorCopy(pm.maxs, ent->r.maxs);

	ent->waterlevel = pm.waterlevel;
	ent->watertype = pm.watertype;
	Rcharge = 0;

	if(ent->client->ps.generic1 & GNF_ONFIRE && pm.waterlevel >= 2)
	{
		ent->client->ps.generic1 &= ~GNF_ONFIRE;
		ent->onFireEnd = level.time;
	}


//  Rcharge = client->charge;
	// execute client events
	ClientEvents(ent, oldEventSequence, Rcharge);

	// link entity now, after any personal teleporters have been used
	trap_LinkEntity(ent);
	if(!ent->client->noclip)
	{
		G_TouchTriggers(ent);
	}

	// NOTE: now copy the exact origin over otherwise clients can be snapped into solid
	VectorCopy(ent->client->ps.origin, ent->r.currentOrigin);

	//test for solid areas in the AAS file
	BotTestAAS(ent->r.currentOrigin);

	// touch other objects
	ClientImpacts(ent, &pm);

	// save results of triggers and client events
	if(ent->client->ps.eventSequence != oldEventSequence)
	{
		ent->eventTime = level.time;
	}

	// swap and latch button actions
	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;
	client->latched_buttons |= client->buttons & ~client->oldbuttons;

	// check for respawning
	if(client->ps.stats[STAT_HEALTH] <= 0)
	{
		// wait for the attack button to be pressed
		if(level.time > client->respawnTime)
		{
			// forcerespawn is to prevent users from waiting out powerups
			if(!ent->DeadView)
			{
				respawn(ent, qtrue);
			}
			return;
		}
		return;
	}

	// perform once-a-second actions
	ClientTimerActions(ent, msec);
}

/*
==================
ClientThink

A new command has arrived from the client
==================
*/
void ClientThink(int clientNum)
{
	gentity_t      *ent;

	ent = g_entities + clientNum;
	trap_GetUsercmd(clientNum, &ent->client->pers.cmd);

	// mark the time we got info, so we can display the
	// phone jack if they don't get any for a while
	ent->client->lastCmdTime = level.time;

	if(!(ent->r.svFlags & SVF_BOT) && !g_synchronousClients.integer)
	{
		ClientThink_real(ent);
	}
}


void G_RunClient(gentity_t * ent)
{
	if(!(ent->r.svFlags & SVF_BOT) && !g_synchronousClients.integer)
	{
		return;
	}
	ent->client->pers.cmd.serverTime = level.time;
	ClientThink_real(ent);
}


/*
==================
SpectatorClientEndFrame

==================
*/
void SpectatorClientEndFrame(gentity_t * ent)
{
	gclient_t      *cl;

	// if we are doing a chase cam or a remote view, grab the latest info
	if(ent->client->sess.spectatorState == SPECTATOR_FOLLOW)
	{
		int             clientNum, flags;

		clientNum = ent->client->sess.spectatorClient;

		// team follow1 and team follow2 go to whatever clients are playing
		if(clientNum == -1)
		{
			clientNum = level.follow1;
		}
		else if(clientNum == -2)
		{
			clientNum = level.follow2;
		}
		if(clientNum >= 0)
		{
			cl = &level.clients[clientNum];
			if(cl->pers.connected == CON_CONNECTED && cl->sess.sessionTeam != TEAM_SPECTATOR)
			{
				flags = (cl->ps.eFlags & ~(EF_VOTED | EF_TEAMVOTED)) | (ent->client->ps.eFlags & (EF_VOTED | EF_TEAMVOTED));
				ent->client->ps = cl->ps;
				ent->client->ps.pm_flags |= PMF_FOLLOW;
				ent->client->ps.eFlags = flags;
				return;
			}
			else
			{
				// drop them to free spectators unless they are dedicated camera followers
				if(ent->client->sess.spectatorClient >= 0)
				{
					ent->client->sess.spectatorState = SPECTATOR_FREE;
					ClientBegin(ent->client - level.clients);
				}
			}
		}
	}

	if(ent->client->sess.spectatorState == SPECTATOR_SCOREBOARD)
	{
		ent->client->ps.pm_flags |= PMF_SCOREBOARD;
	}
	else
	{
		ent->client->ps.pm_flags &= ~PMF_SCOREBOARD;
	}
}

/*
==============
ClientEndFrame

Called at the end of each server frame for each connected client
A fast client will have multiple ClientThink for each ClientEdFrame,
while a slow client may have multiple ClientEndFrame between ClientThink.
==============
*/
void ClientEndFrame(gentity_t * ent)
{
	int             i;
	clientPersistant_t *pers;
	gentity_t      *attacker;
	qboolean        quad;

	if(ent->client->sess.sessionTeam == TEAM_SPECTATOR)
	{
		SpectatorClientEndFrame(ent);
		return;
	}

	pers = &ent->client->pers;

	// turn off any expired powerups
	for(i = 0; i < MAX_POWERUPS; i++)
	{
		if(ent->client->ps.powerups[i] < level.time)
		{
			ent->client->ps.powerups[i] = 0;
		}
	}

#ifdef MISSIONPACK
	// set powerup for player animation
	if(bg_itemlist[ent->client->ps.stats[STAT_PERSISTANT_POWERUP]].giTag == PW_GUARD)
	{
		ent->client->ps.powerups[PW_GUARD] = level.time;
	}
	if(bg_itemlist[ent->client->ps.stats[STAT_PERSISTANT_POWERUP]].giTag == PW_DOUBLER)
	{
		ent->client->ps.powerups[PW_DOUBLER] = level.time;
	}
	if(bg_itemlist[ent->client->ps.stats[STAT_PERSISTANT_POWERUP]].giTag == PW_AMMOREGEN)
	{
		ent->client->ps.powerups[PW_AMMOREGEN] = level.time;
	}
#endif

	// save network bandwidth
#if 0
	if(!g_synchronousClients->integer && ent->client->ps.pm_type == PM_NORMAL)
	{
		// FIXME: this must change eventually for non-sync demo recording
		VectorClear(ent->client->ps.viewangles);
	}
#endif

	//
	// If the end of unit layout is displayed, don't give
	// the player any normal movement attributes
	//
	if(level.intermissiontime)
	{
		return;
	}

	// burn from lava, etc
	P_WorldEffects(ent);

	if(ent->client->ps.weaponstate == WEAPON_PREFIRING)
	{
		ent->client->charge += 2;
	}
	else
	{
		ent->client->charge = 0;
	}

	// apply all the damage taken this frame
	P_DamageFeedback(ent);


	// add the EF_CONNECTION flag if we haven't gotten commands recently
	if ( level.time - ent->client->lastCmdTime > 1000 ) {
		ent->client->ps.eFlags |= EF_CONNECTION;
	} else {
		ent->client->ps.eFlags &= ~EF_CONNECTION;
	}

	ent->client->ps.stats[STAT_HEALTH] = ent->health;	// FIXME: get rid of ent->health...

	G_SetClientSound(ent);

		attacker = &g_entities[ent->client->ps.persistant[PERS_ATTACKER]];
		if(attacker->client->ps.powerups[PW_QUAD] >= 1){
			quad = qtrue;
		}else{
			quad = qfalse;
		}
	// set the latest information
	if (g_smoothClients.integer) {
		BG_PlayerStateToEntityStateExtraPolate( &ent->client->ps, &ent->s, ent->client->ps.commandTime, qtrue,quad);
	}
	else {





		BG_PlayerStateToEntityState( &ent->client->ps, &ent->s, qtrue , quad);
	}

	SendPendingPredictableEvents(&ent->client->ps);

	// set the bit for the reachability area the client is currently in
//  i = trap_AAS_PointReachabilityAreaIndex( ent->client->ps.origin );
//  ent->client->areabits[i >> 3] |= 1 << (i & 7);
}
