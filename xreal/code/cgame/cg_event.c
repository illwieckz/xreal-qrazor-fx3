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
// cg_event.c -- handle entity events at snapshot or playerstate transitions

#include "cg_local.h"

// for the voice chats
#ifdef MISSIONPACK // bk001205
#include "../../ui/menudef.h"
#endif
//==========================================================================

/*
===================
CG_PlaceString

Also called by scoreboard drawing
===================
*/
const char	*CG_PlaceString( int rank ) {
	static char	str[64];
	char	*s, *t;

	if ( rank & RANK_TIED_FLAG ) {
		rank &= ~RANK_TIED_FLAG;
		t = "Tied for ";
	} else {
		t = "";
	}

	if ( rank == 1 ) {
		s = S_COLOR_BLUE "1st" S_COLOR_WHITE;		// draw in blue
	} else if ( rank == 2 ) {
		s = S_COLOR_RED "2nd" S_COLOR_WHITE;		// draw in red
	} else if ( rank == 3 ) {
		s = S_COLOR_YELLOW "3rd" S_COLOR_WHITE;		// draw in yellow
	} else if ( rank == 11 ) {
		s = "11th";
	} else if ( rank == 12 ) {
		s = "12th";
	} else if ( rank == 13 ) {
		s = "13th";
	} else if ( rank % 10 == 1 ) {
		s = va("%ist", rank);
	} else if ( rank % 10 == 2 ) {
		s = va("%ind", rank);
	} else if ( rank % 10 == 3 ) {
		s = va("%ird", rank);
	} else {
		s = va("%ith", rank);
	}

	Com_sprintf( str, sizeof( str ), "%s%s", t, s );
	return str;
}

/*
=============
CG_Obituary
=============
*/
static void CG_Obituary( entityState_t *ent ) {
	int			mod;
	int			target, attacker;
	char		*message;
	char		*message2;
	const char	*targetInfo;
	const char	*attackerInfo;
	char		targetName[64];
	char		targetName2[64];
	char		attackerName[64];
	gender_t	gender;
	clientInfo_t	*ci;

	target = ent->otherEntityNum;
	attacker = ent->otherEntityNum2;
	mod = ent->eventParm;

	if ( target < 0 || target >= MAX_CLIENTS ) {
		CG_Error( "CG_Obituary: target out of range" );
	}
	ci = &cgs.clientinfo[target];

	if ( attacker < 0 || attacker >= MAX_CLIENTS ) {
		attacker = ENTITYNUM_WORLD;
		attackerInfo = NULL;
	} else {
		attackerInfo = CG_ConfigString( CS_PLAYERS + attacker );
	}

	targetInfo = CG_ConfigString( CS_PLAYERS + target );
	if ( !targetInfo ) {
		return;
	}
	Q_strncpyz( targetName, Info_ValueForKey( targetInfo, "n" ), sizeof(targetName) - 2);
	strcat( targetName, S_COLOR_WHITE );
	Q_strncpyz( targetName2, Info_ValueForKey( targetInfo, "n" ), sizeof(targetName2) - 2);

	message2 = "^n^7.";

	// check for single client messages

	switch( mod ) {
	case MOD_SUICIDE:
		message = "^Nsuicides";
		break;
	case MOD_FALLING:
		message = "^Ncratered";
		break;
	case MOD_CRUSH:
		message = "^Nwas squished";
		break;
	case MOD_WATER:
		message = "^Nneeds to learn you can't breathe underwater";
		break;
	case MOD_SLIME:
		message = "^Nwent swimming in some acid";
		break;
	case MOD_LAVA:
		message = "^Ndoes a back flip into the lava";
		break;
	case MOD_TARGET_LASER:
		message = "^Nsaw the light";
		break;
	case MOD_TRIGGER_HURT:
		message = "^Nwas in the wrong place";
		break;
	default:
		message = NULL;
		break;
	}

	if (attacker == target) {
		gender = ci->gender;
		switch (mod) {
#ifdef MISSIONPACK
		case MOD_KAMIKAZE:
			message = "^Ngoes out with a bang";
			break;
#endif
		case MOD_GRENADE_SPLASH:
			if ( gender == GENDER_FEMALE )
				message = "^Ntripped on her own grenade";
			else if ( gender == GENDER_NEUTER )
				message = "^Ntripped on its own grenade";
			else
				message = "^Ntripped on his own grenade";
			break;
		case MOD_ROCKET_SPLASH:
			if ( gender == GENDER_FEMALE )
				message = "^Nblew herself up";
			else if ( gender == GENDER_NEUTER )
				message = "^Nblew itself up";
			else
				message = "^Nblew himself up";
			break;
		case MOD_PLASMA_SPLASH:
			if ( gender == GENDER_FEMALE )
				message = "^Nmelted herself";
			else if ( gender == GENDER_NEUTER )
				message = "^Nmelted itself";
			else
				message = "^Nmelted himself";
			break;
		case MOD_BFG_SPLASH:
			message = "^Nshould stick to a pea shooter";
			break;
		case MOD_FLAMETHROWER:
			if ( gender == GENDER_FEMALE )
				message = "^Nfried herself";
			else if ( gender == GENDER_NEUTER )
				message = "^Nfried itself";
			else
				message = "^Nfried himself";
			break;
#ifdef MISSIONPACK
		case MOD_PROXIMITY_MINE:
			if( gender == GENDER_FEMALE ) {
				message = "found her prox mine";
			} else if ( gender == GENDER_NEUTER ) {
				message = "found it's prox mine";
			} else {
				message = "found his prox mine";
			}
			break;
#endif
		default:
			if ( gender == GENDER_FEMALE )
				message = "^Nkilled herself";
			else if ( gender == GENDER_NEUTER )
				message = "^Nkilled itself";
			else
				message = "^Nkilled himself";
			break;
		}
	}

	if (message) {
		CG_AddToTeamChat( va("%s %s.", targetName, message));
		CG_Printf( "%s %s.\n", Q_CleanAbsoluteColorStr( targetName), message);
		return;
	}

	// check for kill messages from the current clientNum
	if ( attacker == cg.snap->ps.clientNum ) {
		char	*s;
		float    x;

		if ( cgs.gametype < GT_TEAM ) {
			s = va("You fragged %s\n%s place with %i", targetName2, 
				CG_PlaceString( cg.snap->ps.persistant[PERS_RANK] + 1 ),
				cg.snap->ps.persistant[PERS_SCORE] );
		} else {
			s = va("You fragged %s", targetName2 );
		}
	x = FragMessageSize.integer;
	CG_CenterPrint( s, SCREEN_HEIGHT * 0.30, x);
		// print the text message as well
	}

	// check for double client messages
	if ( !attackerInfo ) {
		attacker = ENTITYNUM_WORLD;
		strcpy( attackerName, "noname" );
	} else {
		Q_strncpyz( attackerName, Info_ValueForKey( attackerInfo, "n" ), sizeof(attackerName) - 2);
	//	strcat( attackerName, S_COLOR_WHITE );
		// check for kill messages about the current clientNum
		if ( target == cg.snap->ps.clientNum ) {
			Q_strncpyz( cg.killerName, attackerName, sizeof( cg.killerName ) );
		}
	}

	if ( attacker != ENTITYNUM_WORLD ) {
		switch (mod) {
		case MOD_GRAPPLE:
			message = "^Nwas caught by";
			break;
		case MOD_GAUNTLET:
			message = "^Nwas beat down by";
			break;
		case MOD_FLAMETHROWER:
			message = "^Nwas fried to a crisp by";
			message2 = "'s^N^7 flamethrower";
			break;
		case MOD_MACHINEGUN:
			message = "^Nwas cut in half by";
			message2 = "'s^N^7 machinegun";
			break;
		case MOD_SHOTGUN:
			message = "^Nwas pumped full of lead by";
			break;
		case MOD_GRENADE:
			message = "^Nswallowed";
			message2 = "'s^N^7 grenade";
			break;
		case MOD_GRENADE_SPLASH:
			message = "^Nwas shredded by";
			message2 = "'s^N^7 shrapnel";
			break;
		case MOD_ROCKET:
			message = "^Nswallowed a direct hit by";
			message2 = "'s^N^7 rocket";
			break;
		case MOD_ROCKET_SPLASH:
			message = "^Nwas to close to";
			message2 = "'s^N^7 rocket";
			break;
		case MOD_PLASMA:
			message = "^Ncouldn't take the heat from";
			message2 = "'s^N^7 plasmagun";
			break;
		case MOD_PLASMA_SPLASH:
			message = "^Nwas melted by";
			message2 = "'s^N^7 hot plasma splash";
			break;
		case MOD_RAILGUN:
			message = "^Nwas railed by";
			break;
		case MOD_LIGHTNING:
			message = "^Nwas electrocuted by";
			break;
		case MOD_BFG:
		case MOD_BFG_SPLASH:
			message = "^Ngot creamed by";
			message2 = "'s^N^7 BFG";
			break;
#ifdef MISSIONPACK
		case MOD_NAIL:
			message = "^Nwas nailed by";
			break;
		case MOD_CHAINGUN:
			message = "^Ngot lead poisoning from";
			message2 = "'s^N^7 Chaingun";
			break;
		case MOD_PROXIMITY_MINE:
			message = "^Nwas too close to";
			message2 = "'s^N^7 Prox Mine";
			break;
		case MOD_KAMIKAZE:
			message = "^Nfalls to";
			message2 = "'s^N^7 Kamikaze blast";
			break;
		case MOD_JUICED:
			message = "^Nwas juiced by";
			break;
#endif
		case MOD_TELEFRAG:
			message = "^Nwas too stupid to move out of";
			message2 = "'s^N^7 spawn location";
			break;
		default:
			message = "^Nwas killed by";
			break;
		}

		if (message) {
			CG_AddToTeamChat( va("%s %s %s%s", 
				targetName, message, attackerName, message2));
			CG_Printf( "%s %s %s%s\n", 
				Q_CleanAbsoluteColorStr( targetName), message, Q_CleanAbsoluteColorStr(attackerName), message2);
			return;
		}
	}

	// we don't know what it was
	CG_AddToTeamChat( va("%s ^Ndied.",targetName));
	CG_Printf( "%s died.\n", Q_CleanAbsoluteColorStr( targetName));
}

//==========================================================================

/*
===============
CG_UseItem
===============
*/
static void CG_UseItem( centity_t *cent ) {
	clientInfo_t *ci;
	int			itemNum, clientNum;
	gitem_t		*item;
	entityState_t *es;

	es = &cent->currentState;
	
	itemNum = (es->event & ~EV_EVENT_BITS) - EV_USE_ITEM0;
	if ( itemNum < 0 || itemNum > HI_NUM_HOLDABLE ) {
		itemNum = 0;
	}

	// print a message if the local player
	if ( es->number == cg.snap->ps.clientNum ) {
		if ( !itemNum ) {
			CG_CenterPrint( "No item to use", SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH );
		} else {
			item = BG_FindItemForHoldable( itemNum );
			CG_CenterPrint( va("Use %s", item->pickup_name), SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH );
		}
	}

	switch ( itemNum ) {
	default:
	case HI_NONE:
		trap_S_StartSound (NULL, es->number, CHAN_BODY, cgs.media.useNothingSound );
		break;

	case HI_TELEPORTER:
		break;

	case HI_MEDKIT:
		clientNum = cent->currentState.clientNum;
		if ( clientNum >= 0 && clientNum < MAX_CLIENTS ) {
			ci = &cgs.clientinfo[ clientNum ];
			ci->medkitUsageTime = cg.time;
		}
		trap_S_StartSound (NULL, es->number, CHAN_BODY, cgs.media.medkitSound );
		break;

#ifdef MISSIONPACK
	case HI_KAMIKAZE:
		break;

	case HI_PORTAL:
		break;
	case HI_INVULNERABILITY:
		trap_S_StartSound (NULL, es->number, CHAN_BODY, cgs.media.useInvulnerabilitySound );
		break;
#endif
	}

}

/*
================
CG_ItemPickup

A new item was picked up this frame
================
*/
void CG_ItemPickup( int itemNum ,int clientNum) {
	clientInfo_t	*ci;
	centity_t		*cent;

	cent = &cg_entities[ clientNum ];
	ci = &cgs.clientinfo[ clientNum ];

	cg.itemPickup = itemNum;
	cg.itemPickupTime = cg.time;
	cg.itemPickupBlendTime = cg.time;
	// see if it should be the grabbed weapon
	if ( bg_itemlist[itemNum].giType == IT_WEAPON ) {
		// select it immediately

	if ( bg_itemlist[itemNum].giTag == WP_ROCKET_LAUNCHER && ci->pickrl == qfalse) {
		cg.selectflash = cg.time + 650;
		cg.weaponSelect2 = WP_ROCKET_LAUNCHER;
		ci->pickrl = qtrue;
	}
	if ( bg_itemlist[itemNum].giTag == WP_GRENADE_LAUNCHER && ci->pickgl == qfalse) {
		cg.selectflash = cg.time + 650;
		cg.weaponSelect2 = WP_GRENADE_LAUNCHER;
		ci->pickgl = qtrue;
	}
	if ( bg_itemlist[itemNum].giTag == WP_PLASMAGUN && ci->pickpg == qfalse) {
		cg.selectflash = cg.time + 650;
		cg.weaponSelect2 = WP_PLASMAGUN;
		ci->pickpg = qtrue;
	}
	if ( bg_itemlist[itemNum].giTag == WP_SHOTGUN && ci->picksg == qfalse) {
		cg.selectflash = cg.time + 650;
		cg.weaponSelect2 = WP_SHOTGUN;
		ci->picksg = qtrue;
	}
	if ( bg_itemlist[itemNum].giTag == WP_RAILGUN && ci->pickrg == qfalse) {
		cg.selectflash = cg.time + 650;
		cg.weaponSelect2 = WP_RAILGUN;
		ci->pickrg = qtrue;
	}
	if ( bg_itemlist[itemNum].giTag == WP_IRAILGUN && ci->pickirg == qfalse) {
		cg.selectflash = cg.time + 650;
		cg.weaponSelect2 = WP_IRAILGUN;
		ci->pickirg = qtrue;
	}
	if ( bg_itemlist[itemNum].giTag == WP_BFG && ci->pickbfg == qfalse) {
		cg.selectflash = cg.time + 650;
		cg.weaponSelect2 = WP_BFG;
		ci->pickbfg = qtrue;
	}
	if ( bg_itemlist[itemNum].giTag == WP_FLAMETHROWER && ci->pickft == qfalse) {
		cg.selectflash = cg.time + 650;
		cg.weaponSelect2 = WP_FLAMETHROWER;
		ci->pickft = qtrue;
	}
	if ( bg_itemlist[itemNum].giTag == WP_LIGHTNING && ci->picklg == qfalse) {
		cg.selectflash = cg.time + 650;
		cg.weaponSelect2 = WP_LIGHTNING;
		ci->picklg = qtrue;
	}
		if ( cg_autoswitch.integer && bg_itemlist[itemNum].giTag != WP_MACHINEGUN ) {
		//	cg.weaponSelectTime = cg.time;
		//	cg.weaponSelect = bg_itemlist[itemNum].giTag;
		}
	}
}


/*
================
CG_PainEvent

Also called by playerstate transition
================
*/
void CG_PainEvent( centity_t *cent, int health ) {
	char	*snd;

	// don't do more than two pain sounds a second
	if ( cg.time - cent->pe.painTime < 500 ) {
		return;
	}

	if ( health < 25 ) {
		snd = "*pain25_1.wav";
	} else if ( health < 50 ) {
		snd = "*pain50_1.wav";
	} else if ( health < 75 ) {
		snd = "*pain75_1.wav";
	} else {
		snd = "*pain100_1.wav";
	}
	trap_S_StartSound( NULL, cent->currentState.number, CHAN_VOICE, 
		CG_CustomSound( cent->currentState.number, snd ) );

	// save pain time for programitic twitch animation
	cent->pe.painTime = cg.time;
	cent->pe.painDirection ^= 1;
}

/*
=================
CG_AddBulletParticles
=================
*/
static void CG_AddSparkParticles( vec3_t origin, vec3_t dir,int count ,int duration,
								  int speed,int trailtime
								  ,qhandle_t hShader,int step,
								  float	startsize,float	endsize) {
	localEntity_t	*le;
	refEntity_t		*re;
	vec3_t	velocity;
	int	i;  
    float randScale;


	// add the sparks
	for (i=0; i<count;i++) {

		le = CG_AllocLocalEntity();
		re = &le->refEntity;

		speed = speed  + rand()%20;
		duration = duration + rand()%20;
		randScale = 1.0;

		le->leType = LE_ADDSPARK;
		le->entNumber = le - cg_freeLocalEntities;
		velocity[0] = dir[0] + crandom()*randScale;
		velocity[1] = dir[1] + crandom()*randScale;
		velocity[2] = dir[2] + crandom()*randScale;
		VectorScale( velocity, (float)speed, velocity );

		le->startTime = cg.time;
		le->endTime = le->startTime + duration;
		le->lifeRate = 1.0 / ( le->endTime - le->startTime );
		le->hShader = hShader;
		le->trailLength = trailtime;
		le->step = step;
		le->startSize = startsize;
		le->endSize = endsize;

		le->pos.trType = TR_GRAVITY;
		le->pos.trTime = cg.time;

		VectorCopy( origin, re->origin );
		VectorCopy( origin, le->pos.trBase );
		VectorMA( le->pos.trBase, 2 + random()*2, dir, le->pos.trBase );
		VectorCopy( velocity, le->pos.trDelta ); 
	}
}

/*
==============
CG_EntityEvent

An entity has an event value
also called by CG_CheckPlayerstateEvents
==============
*/
#define	DEBUGNAME(x) if(cg_debugEvents.integer){CG_Printf(x"\n");}
void CG_EntityEvent( centity_t *cent, vec3_t position ) {
	entityState_t	*es;
	int				event;
	vec3_t			dir;
	const char		*s;
	int				clientNum;
	clientInfo_t	*ci;
	weaponInfo_t *weapon;
	int			weaponNum;
	playerState_t		*ps;
//	centity_t *cent1;
	vec3_t		dieorg;
	int c;

	ps = &cent->currentState.clientNum[&cg.snap->ps];

	es = &cent->currentState;
	event = es->event & ~EV_EVENT_BITS;
	weaponNum = cent->currentState.weapon;
	
	if ( cg_debugEvents.integer ) {
		CG_Printf( "ent:%3i  event:%3i ", es->number, event );
	}

	weapon = &cg_weapons[weaponNum];

	if ( !event ) {
		DEBUGNAME("ZEROEVENT");
		return;
	}

	clientNum = es->clientNum;
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
		clientNum = 0;
	}
	ci = &cgs.clientinfo[ clientNum ];

	switch ( event ) {
	//
	// movement generated events
	//
	case EV_FOOTSTEP:
		DEBUGNAME("EV_FOOTSTEP");
		if (cgs.g_Footsteps == 1) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY, 
				cgs.media.footsteps[ ci->footsteps ][rand()&3] );
		}
		break;
	case EV_FOOTSTEP_METAL:
		DEBUGNAME("EV_FOOTSTEP_METAL");
		if (cgs.g_Footsteps == 1) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY, 
				cgs.media.footsteps[ FOOTSTEP_METAL ][rand()&3] );
		}
		break;
	case EV_FOOTSPLASH:
		DEBUGNAME("EV_FOOTSPLASH");
		if (cgs.g_Footsteps == 1) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY, 
				cgs.media.footsteps[ FOOTSTEP_SPLASH ][rand()&3] );
		}
		break;
	case EV_FOOTWADE:
		DEBUGNAME("EV_FOOTWADE");
		if (cgs.g_Footsteps == 1) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY, 
				cgs.media.footsteps[ FOOTSTEP_SPLASH ][rand()&3] );
		}
		break;
	case EV_SWIM:
		DEBUGNAME("EV_SWIM");
		if (cgs.g_Footsteps == 1) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY, 
				cgs.media.footsteps[ FOOTSTEP_SPLASH ][rand()&3] );
		}
		break;


	case EV_FALL_SHORT:
		DEBUGNAME("EV_FALL_SHORT");
		if (cgs.g_Footsteps == 1) {
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.landSound );
			if ( clientNum == cg.predictedPlayerState.clientNum ) {
				// smooth landing z changes
				cg.landChange = -8;
				cg.landTime = cg.time;
			}
		}
		break;
	case EV_FALL_MEDIUM:
		DEBUGNAME("EV_FALL_MEDIUM");
		// use normal pain sound
		trap_S_StartSound( NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*pain100_1.wav" ) );
		if ( clientNum == cg.predictedPlayerState.clientNum ) {
			// smooth landing z changes
			cg.landChange = -16;
			cg.landTime = cg.time;
		}
		break;
	case EV_FALL_FAR:
		DEBUGNAME("EV_FALL_FAR");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, CG_CustomSound( es->number, "*fall1.wav" ) );
		cent->pe.painTime = cg.time;	// don't play a pain sound right after this
		if ( clientNum == cg.predictedPlayerState.clientNum ) {
			// smooth landing z changes
			cg.landChange = -24;
			cg.landTime = cg.time;
		}
		break;

	case EV_STEP_4:
	case EV_STEP_8:
	case EV_STEP_12:
	case EV_STEP_16:		// smooth out step up transitions
		DEBUGNAME("EV_STEP");
	{
		float	oldStep;
		int		delta;
		int		step;

		if ( clientNum != cg.predictedPlayerState.clientNum ) {
			break;
		}
		// if we are interpolating, we don't need to smooth steps
		if ( cg.demoPlayback || (cg.snap->ps.pm_flags & PMF_FOLLOW) ||
			cg_nopredict.integer || cg_synchronousClients.integer ) {
			break;
		}
		// check for stepping up before a previous step is completed
		delta = cg.time - cg.stepTime;
		if (delta < STEP_TIME) {
			oldStep = cg.stepChange * (STEP_TIME - delta) / STEP_TIME;
		} else {
			oldStep = 0;
		}

		// add this amount
		step = 4 * (event - EV_STEP_4 + 1 );
		cg.stepChange = oldStep + step;
		if ( cg.stepChange > MAX_STEP_CHANGE ) {
			cg.stepChange = MAX_STEP_CHANGE;
		}
		cg.stepTime = cg.time;
		break;
	}

	case EV_JUMP_PAD:
		DEBUGNAME("EV_JUMP_PAD");
//		CG_Printf( "EV_JUMP_PAD w/effect #%i\n", es->eventParm );
		{
			localEntity_t	*smoke;
			vec3_t			up = {0, 0, 1};


			smoke = CG_SmokePuff( cent->lerpOrigin, up, 
						  32, 
						  1, 1, 1, 0.33f,
						  1000, 
						  cg.time, 0,
						  LEF_PUFF_DONT_SCALE, 
						  cgs.media.smokePuffShader );
		}

		// boing sound at origin, jump sound on player
		trap_S_StartSound ( cent->lerpOrigin, -1, CHAN_VOICE, cgs.media.jumpPadSound );
	if (cgs.g_Footsteps == 1) {
		trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*jump1.wav" ) );
	}
		break;

	case EV_JUMP:
		DEBUGNAME("EV_JUMP");
	if (cgs.g_Footsteps == 1) {
		trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*jump1.wav" ) );
	}
		break;
	case EV_WALLJUMP:
		DEBUGNAME("EV_WALLJUMP");
	if (cgs.g_Footsteps == 1) {
		trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*jump1.wav" ) );
	}
		break;
	case EV_TAUNT:
		DEBUGNAME("EV_TAUNT");
		trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*taunt.wav" ) );
		break;
#ifdef MISSIONPACK
	case EV_TAUNT_YES:
		DEBUGNAME("EV_TAUNT_YES");
		CG_VoiceChatLocal(SAY_TEAM, qfalse, es->number, COLOR_CYAN, VOICECHAT_YES);
		break;
	case EV_TAUNT_NO:
		DEBUGNAME("EV_TAUNT_NO");
		CG_VoiceChatLocal(SAY_TEAM, qfalse, es->number, COLOR_CYAN, VOICECHAT_NO);
		break;
	case EV_TAUNT_FOLLOWME:
		DEBUGNAME("EV_TAUNT_FOLLOWME");
		CG_VoiceChatLocal(SAY_TEAM, qfalse, es->number, COLOR_CYAN, VOICECHAT_FOLLOWME);
		break;
	case EV_TAUNT_GETFLAG:
		DEBUGNAME("EV_TAUNT_GETFLAG");
		CG_VoiceChatLocal(SAY_TEAM, qfalse, es->number, COLOR_CYAN, VOICECHAT_ONGETFLAG);
		break;
	case EV_TAUNT_GUARDBASE:
		DEBUGNAME("EV_TAUNT_GUARDBASE");
		CG_VoiceChatLocal(SAY_TEAM, qfalse, es->number, COLOR_CYAN, VOICECHAT_ONDEFENSE);
		break;
	case EV_TAUNT_PATROL:
		DEBUGNAME("EV_TAUNT_PATROL");
		CG_VoiceChatLocal(SAY_TEAM, qfalse, es->number, COLOR_CYAN, VOICECHAT_ONPATROL);
		break;
#endif
	case EV_WATER_TOUCH:
		DEBUGNAME("EV_WATER_TOUCH");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.watrInSound );
		break;
	case EV_WATER_LEAVE:
		DEBUGNAME("EV_WATER_LEAVE");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.watrOutSound );
		break;
	case EV_WATER_UNDER:
		DEBUGNAME("EV_WATER_UNDER");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.watrUnSound );
		break;
	case EV_WATER_CLEAR:
		DEBUGNAME("EV_WATER_CLEAR");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, CG_CustomSound( es->number, "*gasp.wav" ) );
		break;

	case EV_RAILGUN_PREFIRE:
		DEBUGNAME("EV_RAILGUN_PREFIRE");
		CG_RailgunPrefire( cent );
		break;

	case EV_RAILGUN_READY:
		DEBUGNAME("EV_RAILGUN_READY");
		trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "sound/weapons/irailgun/railbeep.wav" ) );
		break;


	case EV_ITEM_PICKUP:
		DEBUGNAME("EV_ITEM_PICKUP");
		{
			gitem_t	*item;
			int		index;

			index = es->eventParm;		// player predicted


			if ( index < 1 || index >= bg_numItems ) {
				break;
			}
			item = &bg_itemlist[ index ];

			if ( item->giType == IT_POWERUP || item->giType == IT_TEAM) {
				trap_S_StartSound (NULL, es->number, CHAN_AUTO,	cgs.media.n_healthSound );
				trap_S_StartSound (NULL, es->number, CHAN_AUTO,	cgs.media.n_healthSound );
			} else {
				trap_S_StartSound (NULL, es->number, CHAN_AUTO,	trap_S_RegisterSound( item->pickup_sound, qfalse ) );
				trap_S_StartSound (NULL, es->number, CHAN_AUTO,	trap_S_RegisterSound( item->pickup_sound, qfalse ) );
			}

			// show icon and name on status bar
			if ( es->number == cg.snap->ps.clientNum ) {
				CG_ItemPickup( index ,es->number);
			}
		}
		break;

	case EV_GLOBAL_ITEM_PICKUP:
		DEBUGNAME("EV_GLOBAL_ITEM_PICKUP");
		{
			gitem_t	*item;
			int		index;

			index = es->eventParm;		// player predicted

			if ( index < 1 || index >= bg_numItems ) {
				break;
			}

			item = &bg_itemlist[ index ];

			// powerup pickups are global
			if( item->pickup_sound ) {
				trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_AUTO, trap_S_RegisterSound( item->pickup_sound, qfalse ) );
			}

			// show icon and name on status bar
			if ( es->number == cg.snap->ps.clientNum ) {
				CG_ItemPickup( index ,es->number);
			}
		}
		break;

	//
	// weapon events
	//
	case EV_NOAMMO:
		DEBUGNAME("EV_NOAMMO");
//		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.noAmmoSound );
		if ( es->number == cg.snap->ps.clientNum ) {
			CG_OutOfAmmoChange();
		}
		break;
	case EV_CHANGE_WEAPON:
		DEBUGNAME("EV_CHANGE_WEAPON");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.selectSound );
		break;
	case EV_FIRE_WEAPON:
		DEBUGNAME("EV_FIRE_WEAPON");
		CG_FireWeapon( cent );
		break;

	case EV_USE_ITEM0:
		DEBUGNAME("EV_USE_ITEM0");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM1:
		DEBUGNAME("EV_USE_ITEM1");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM2:
		DEBUGNAME("EV_USE_ITEM2");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM3:
		DEBUGNAME("EV_USE_ITEM3");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM4:
		DEBUGNAME("EV_USE_ITEM4");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM5:
		DEBUGNAME("EV_USE_ITEM5");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM6:
		DEBUGNAME("EV_USE_ITEM6");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM7:
		DEBUGNAME("EV_USE_ITEM7");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM8:
		DEBUGNAME("EV_USE_ITEM8");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM9:
		DEBUGNAME("EV_USE_ITEM9");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM10:
		DEBUGNAME("EV_USE_ITEM10");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM11:
		DEBUGNAME("EV_USE_ITEM11");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM12:
		DEBUGNAME("EV_USE_ITEM12");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM13:
		DEBUGNAME("EV_USE_ITEM13");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM14:
		DEBUGNAME("EV_USE_ITEM14");
		CG_UseItem( cent );
		break;

	//=================================================================

	//
	// other events
	//
	case EV_PLAYER_TELEPORT_IN:
		DEBUGNAME("EV_PLAYER_TELEPORT_IN");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.teleInSound );
		CG_SpawnEffect( position,ci->team);
		break;

	case EV_PLAYER_TELEPORT_OUT:
		DEBUGNAME("EV_PLAYER_TELEPORT_OUT");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.teleOutSound );
		CG_SpawnEffect(  position,ci->team);
		break;

	case EV_ITEM_POP:
		DEBUGNAME("EV_ITEM_POP");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.respawnSound );
		break;
	case EV_ITEM_RESPAWN:
		DEBUGNAME("EV_ITEM_RESPAWN");
		cent->miscTime = cg.time;	// scale up from this
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.respawnSound );
		break;

	case EV_GRENADE_BOUNCE:
		DEBUGNAME("EV_GRENADE_BOUNCE");
		if ( rand() & 1 ) {
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.hgrenb1aSound );
		} else {
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.hgrenb2aSound );
		}
		if(es->generic1 & GNF_GRENSPARKS){
			CG_AddSparkParticles( es->pos.trBase, es->angles,4 ,200,
								  100,60
								  ,cgs.media.tracerTrailzShader,16,
								  3.0,0.2f);
		}
		break;

#ifdef MISSIONPACK
	case EV_PROXIMITY_MINE_STICK:
		DEBUGNAME("EV_PROXIMITY_MINE_STICK");
		if( es->eventParm & SURF_FLESH ) {
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.wstbimplSound );
		} else 	if( es->eventParm & SURF_METALSTEPS ) {
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.wstbimpmSound );
		} else {
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.wstbimpdSound );
		}
		break;

	case EV_PROXIMITY_MINE_TRIGGER:
		DEBUGNAME("EV_PROXIMITY_MINE_TRIGGER");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.wstbactvSound );
		break;
	case EV_KAMIKAZE:
		DEBUGNAME("EV_KAMIKAZE");
		CG_KamikazeEffect( cent->lerpOrigin );
		break;
	case EV_OBELISKEXPLODE:
		DEBUGNAME("EV_OBELISKEXPLODE");
		CG_ObeliskExplode( cent->lerpOrigin, es->eventParm );
		break;
	case EV_OBELISKPAIN:
		DEBUGNAME("EV_OBELISKPAIN");
		CG_ObeliskPain( cent->lerpOrigin );
		break;
	case EV_INVUL_IMPACT:
		DEBUGNAME("EV_INVUL_IMPACT");
		CG_InvulnerabilityImpact( cent->lerpOrigin, cent->currentState.angles );
		break;
	case EV_JUICED:
		DEBUGNAME("EV_JUICED");
		CG_InvulnerabilityJuiced( cent->lerpOrigin );
		break;
	case EV_LIGHTNINGBOLT:
		DEBUGNAME("EV_LIGHTNINGBOLT");
	//	CG_LightningBoltBeam(es->origin2, es->pos.trBase);
	//	CG_LightningBolt( es->origin2, es->pos.trBase );
		break;
#endif
	case EV_SCOREPLUM:
		DEBUGNAME("EV_SCOREPLUM");
		CG_ScorePlum( cent->currentState.otherEntityNum, cent->lerpOrigin, cent->currentState.time );
		break;

	//
	// missile impacts
	//
	case EV_MISSILE_HIT:
		DEBUGNAME("EV_MISSILE_HIT");
			ByteToDir( es->eventParm, dir );
			if(es->powerups & ( 1 << PW_QUAD)){
				CG_MissileHitPlayer( es->weapon, position, dir, es->otherEntityNum ,es->clientNum,qtrue);
			}else{
				CG_MissileHitPlayer( es->weapon, position, dir, es->otherEntityNum ,es->clientNum,qfalse);
			}
		break;

	case EV_LG_HIT:
		DEBUGNAME("EV_LG_HIT");
			if(es->otherEntityNum2 != cg.predictedPlayerState.clientNum){
				cg_entities[es->otherEntityNum2].pe.teslaDamagedTime = cg.time;
			}else{
				cg.predictedPlayerEntity.pe.teslaDamagedTime = cg.time;
			}
		break;
	case EV_LG_MISS:
		DEBUGNAME("EV_LG_MISS");
		break;

	case EV_MISSILE_MISS:
		DEBUGNAME("EV_MISSILE_MISS");

		ByteToDir( es->eventParm, dir );
		if(es->powerups & ( 1 << PW_QUAD)){
			CG_MissileHitWall( es->weapon, es->clientNum, position, dir, IMPACTSOUND_DEFAULT ,qtrue,qfalse);
		}else{
			CG_MissileHitWall( es->weapon, es->clientNum, position, dir, IMPACTSOUND_DEFAULT ,qfalse,qfalse);
		}

		break;

	case EV_MISSILE_MISS_METAL:
		DEBUGNAME("EV_MISSILE_MISS_METAL");

		ByteToDir( es->eventParm, dir );
		if(es->powerups & ( 1 << PW_QUAD)){
			CG_MissileHitWall( es->weapon, es->clientNum, position, dir, IMPACTSOUND_METAL ,qtrue,qfalse);
		}else{
			CG_MissileHitWall( es->weapon, es->clientNum, position, dir, IMPACTSOUND_METAL ,qfalse,qfalse);
		}
		break;

	case EV_RAILTRAIL:
		DEBUGNAME("EV_RAILTRAIL");
		cent->currentState.weapon = WP_RAILGUN;
//unlagged - attack prediction #2
		// if the client is us, unlagged is on server-side, and we've got it client-side
		if ( es->clientNum == cg.predictedPlayerState.clientNum && 
				cgs.delagHitscan && (cg_delag.integer & 1 || cg_delag.integer & 16) ) {

			// do nothing, because it was already predicted
			//Com_Printf("Ignoring rail trail event\n");
		}
		else {
			// draw a rail trail, because it wasn't predicted

			CG_RailTrail( ci, es->origin2, es->pos.trBase );

			if ( es->eventParm != 255 ) {
				ByteToDir( es->eventParm, dir );
				if(es->powerups & ( 1 << PW_QUAD)){
					CG_MissileHitWall( es->weapon, es->clientNum, position, dir, IMPACTSOUND_DEFAULT ,qtrue,qfalse);
				}else{
					CG_MissileHitWall( es->weapon, es->clientNum, position, dir, IMPACTSOUND_DEFAULT ,qfalse,qfalse);
				}

			}
			//Com_Printf("Non-predicted rail trail\n");
		}
//unlagged - attack prediction #2
		break;

	case EV_RAILTRAIL2:
		DEBUGNAME("EV_RAILTRAIL2");
		cent->currentState.weapon = WP_IRAILGUN;
//unlagged - attack prediction #2
		// if the client is us, unlagged is on server-side, and we've got it client-side
		if ( es->clientNum == cg.predictedPlayerState.clientNum && 
				cgs.delagHitscan && (cg_delag.integer & 1 || cg_delag.integer & 16) ) {

			// do nothing, because it was already predicted
			//Com_Printf("Ignoring rail trail event\n");
		}
		else {
			// draw a rail trail, because it wasn't predicted
			ByteToDir( es->eventParm, dir );
			CG_RailTrail( ci, es->origin2, es->pos.trBase);
			if ( es->eventParm != 255 ) {
				if(es->powerups & ( 1 << PW_QUAD)){
					CG_MissileHitWall( es->weapon, es->clientNum, position, dir, IMPACTSOUND_DEFAULT ,qtrue,qfalse);
				}else{
					CG_MissileHitWall( es->weapon, es->clientNum, position, dir, IMPACTSOUND_DEFAULT ,qfalse,qfalse);
				}
			}
			//Com_Printf("Non-predicted rail trail\n");
		}
//unlagged - attack prediction #2
		break;

	case EV_BULLET_HIT_WALL:
		DEBUGNAME("EV_BULLET_HIT_WALL");
//unlagged - attack prediction #2
		// if the client is us, unlagged is on server-side, and we've got it client-side
		if ( es->clientNum == cg.predictedPlayerState.clientNum && 
				cgs.delagHitscan && (cg_delag.integer & 1 || cg_delag.integer & 2) ) {
			// do nothing, because it was already predicted
			//Com_Printf("Ignoring bullet event\n");
		}
		else {
			// do the bullet, because it wasn't predicted
			ByteToDir( es->eventParm, dir );
			CG_Bullet( es->pos.trBase, es->otherEntityNum, dir, qfalse, ENTITYNUM_WORLD );
			//Com_Printf("Non-predicted bullet\n");
		}
//unlagged - attack prediction #2
		break;

	case EV_BULLET_HIT_FLESH:
		DEBUGNAME("EV_BULLET_HIT_FLESH");
//unlagged - attack prediction #2
		// if the client is us, unlagged is on server-side, and we've got it client-side
		if ( es->clientNum == cg.predictedPlayerState.clientNum && 
				cgs.delagHitscan && (cg_delag.integer & 1 || cg_delag.integer & 2) ) {
			// do nothing, because it was already predicted
			//Com_Printf("Ignoring bullet event\n");
		}
		else {
			// do the bullet, because it wasn't predicted
			CG_Bullet( es->pos.trBase, es->otherEntityNum, dir, qtrue, es->eventParm );
			//Com_Printf("Non-predicted bullet\n");
		}
//unlagged - attack prediction #2
		break;

	case EV_RAIL_HIT_FLESH:
		DEBUGNAME("EV_RAIL_HIT_FLESH");

//			RailPlayer(position, dir, es->otherEntityNum );

		break;

	case EV_SHOTGUN:
		DEBUGNAME("EV_SHOTGUN");
//unlagged - attack prediction #2
		// if the client is us, unlagged is on server-side, and we've got it client-side
		if ( es->otherEntityNum == cg.predictedPlayerState.clientNum && 
				cgs.delagHitscan && (cg_delag.integer & 1 || cg_delag.integer & 4) ) {
			// do nothing, because it was already predicted
			//Com_Printf("Ignoring shotgun event\n");
		}
		else {
			// do the shotgun pattern, because it wasn't predicted
			CG_ShotgunFire( es );
			//Com_Printf("Non-predicted shotgun pattern\n");
		}
//unlagged - attack prediction #2
		break;

	case EV_ARMORHIT:
		DEBUGNAME("EV_ARMORHIT");
			trap_S_StartSound( NULL, es->number, CHAN_VOICE, cgs.media.armorHitSound );
		break;

	case EV_GENERAL_SOUND:
		DEBUGNAME("EV_GENERAL_SOUND");
		if ( cgs.gameSounds[ es->eventParm ] ) {
			if(cgs.gameSounds[ es->eventParm ] == cgs.media.armorHitSound){
				if(es->otherEntityNum2 != cg.predictedPlayerState.clientNum){
					cg_entities[es->otherEntityNum2].pe.armorDamagedTime = cg.time;
					trap_S_StartSound (cg_entities[es->otherEntityNum2].lerpOrigin, es->otherEntityNum2, CHAN_VOICE, cgs.gameSounds[ es->eventParm ] );
				}else{
					cg.predictedPlayerEntity.pe.armorDamagedTime = cg.time;
					trap_S_StartSound (cg.predictedPlayerState.origin, cg.predictedPlayerState.clientNum, CHAN_VOICE, cgs.gameSounds[ es->eventParm ] );
				}
			}else{
				trap_S_StartSound (NULL, es->number, CHAN_VOICE, cgs.gameSounds[ es->eventParm ] );
			}
		} else {
			s = CG_ConfigString( CS_SOUNDS + es->eventParm );
			trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, s ) );
		}
		break;

	case EV_GLOBAL_SOUND:	// play from the player's head so it never diminishes
		DEBUGNAME("EV_GLOBAL_SOUND");
		if ( cgs.gameSounds[ es->eventParm ] ) {

			trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_AUTO, cgs.gameSounds[ es->eventParm ] );
		} else {
			s = CG_ConfigString( CS_SOUNDS + es->eventParm );
			trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_AUTO, CG_CustomSound( es->number, s ) );
		}
		break;

	case EV_GLOBAL_TEAM_SOUND:	// play from the player's head so it never diminishes
		{
			DEBUGNAME("EV_GLOBAL_TEAM_SOUND");
			switch( es->eventParm ) {
				case GTS_RED_CAPTURE: // CTF: red team captured the blue flag, 1FCTF: red team captured the neutral flag
					if ( cgs.clientinfo[cg.clientNum].team == TEAM_RED )
						CG_AddBufferedSound( cgs.media.captureYourTeamSound );
					else
						CG_AddBufferedSound( cgs.media.captureOpponentSound );
					break;
				case GTS_BLUE_CAPTURE: // CTF: blue team captured the red flag, 1FCTF: blue team captured the neutral flag
					if ( cgs.clientinfo[cg.clientNum].team == TEAM_BLUE )
						CG_AddBufferedSound( cgs.media.captureYourTeamSound );
					else
						CG_AddBufferedSound( cgs.media.captureOpponentSound );
					break;
				case GTS_RED_RETURN: // CTF: blue flag returned, 1FCTF: never used
					if ( cgs.clientinfo[cg.clientNum].team == TEAM_RED )
						CG_AddBufferedSound( cgs.media.returnYourTeamSound );
					else
						CG_AddBufferedSound( cgs.media.returnOpponentSound );
					//
					CG_AddBufferedSound( cgs.media.blueFlagReturnedSound );
					break;
				case GTS_BLUE_RETURN: // CTF red flag returned, 1FCTF: neutral flag returned
					if ( cgs.clientinfo[cg.clientNum].team == TEAM_BLUE )
						CG_AddBufferedSound( cgs.media.returnYourTeamSound );
					else
						CG_AddBufferedSound( cgs.media.returnOpponentSound );
					//
					CG_AddBufferedSound( cgs.media.redFlagReturnedSound );
					break;

				case GTS_RED_TAKEN: // CTF: red team took blue flag, 1FCTF: blue team took the neutral flag
					// if this player picked up the flag then a sound is played in CG_CheckLocalSounds
					if (cg.snap->ps.powerups[PW_BLUEFLAG] ) {
					}
					else {
					//NT
					if (cgs.clientinfo[cg.clientNum].team == TEAM_BLUE ) {
#ifdef MISSIONPACK
							if (cgs.gametype == GT_1FCTF) 
								CG_AddBufferedSound( cgs.media.yourTeamTookTheFlagSound );
							else
#endif
						 	CG_AddBufferedSound( cgs.media.enemyTookYourFlagSound );
						}
						//NT
						else if (cgs.clientinfo[cg.clientNum].team == TEAM_RED ) {
#ifdef MISSIONPACK
							if (cgs.gametype == GT_1FCTF)
								CG_AddBufferedSound( cgs.media.enemyTookTheFlagSound );
							else
#endif
 							CG_AddBufferedSound( cgs.media.yourTeamTookEnemyFlagSound );
						}
					}
					break;
				case GTS_BLUE_TAKEN: // CTF: blue team took the red flag, 1FCTF red team took the neutral flag
					// if this player picked up the flag then a sound is played in CG_CheckLocalSounds
					if (cg.snap->ps.powerups[PW_REDFLAG]) {
					}
					else {
						//NT
						if (cgs.clientinfo[cg.clientNum].team == TEAM_RED ) {
#ifdef MISSIONPACK
							if (cgs.gametype == GT_1FCTF)
								CG_AddBufferedSound( cgs.media.yourTeamTookTheFlagSound );
							else
#endif
							CG_AddBufferedSound( cgs.media.enemyTookYourFlagSound );
						}
						//NT
						else if (cgs.clientinfo[cg.clientNum].team == TEAM_BLUE ) {
#ifdef MISSIONPACK
							if (cgs.gametype == GT_1FCTF)
								CG_AddBufferedSound( cgs.media.enemyTookTheFlagSound );
							else
#endif
							CG_AddBufferedSound( cgs.media.yourTeamTookEnemyFlagSound );
						}
					}
					break;

				//NT - added more cases for flags since you can grab your own
				case GTS_BLUE_TAKEN_OWN:
					// if this player picked up the flag then a sound is played in CG_CheckLocalSounds
					if ( cg.snap->ps.powerups[PW_BLUEFLAG] ) {
					}
					else {
						//NT
						if (cgs.clientinfo[cg.clientNum].team == TEAM_BLUE ) {
							CG_AddBufferedSound( cgs.media.yourTeamHasYourFlagSound );
						} else if (cgs.clientinfo[cg.clientNum].team == TEAM_RED ) {
							CG_AddBufferedSound( cgs.media.enemyHasFlagSound );
						}
					}
					break;

				//NT - added more cases for flags since you can grab your own
				case GTS_RED_TAKEN_OWN:
					// if this player picked up the flag then a sound is played in CG_CheckLocalSounds
					if ( cg.snap->ps.powerups[PW_REDFLAG] ) {
					}
					else {
						//NT
						if (cgs.clientinfo[cg.clientNum].team == TEAM_RED ) {
							CG_AddBufferedSound( cgs.media.yourTeamHasYourFlagSound );
						} else if (cgs.clientinfo[cg.clientNum].team == TEAM_BLUE ) {
							CG_AddBufferedSound( cgs.media.enemyHasFlagSound );
						}
					}
					break;


				case GTS_REDOBELISK_ATTACKED: // Overload: red obelisk is being attacked
					if (cgs.clientinfo[cg.clientNum].team == TEAM_RED) {
						CG_AddBufferedSound( cgs.media.yourBaseIsUnderAttackSound );
					}
					break;
				case GTS_BLUEOBELISK_ATTACKED: // Overload: blue obelisk is being attacked
					if (cgs.clientinfo[cg.clientNum].team == TEAM_BLUE) {
						CG_AddBufferedSound( cgs.media.yourBaseIsUnderAttackSound );
					}
					break;

				case GTS_REDTEAM_SCORED:
					CG_AddBufferedSound(cgs.media.redScoredSound);
					break;
				case GTS_BLUETEAM_SCORED:
					CG_AddBufferedSound(cgs.media.blueScoredSound);
					break;
				case GTS_REDTEAM_TOOK_LEAD:
					CG_AddBufferedSound(cgs.media.redLeadsSound);
					break;
				case GTS_BLUETEAM_TOOK_LEAD:
					CG_AddBufferedSound(cgs.media.blueLeadsSound);
					break;
				case GTS_TEAMS_ARE_TIED:
					CG_AddBufferedSound( cgs.media.teamsTiedSound );
					break;
//#ifdef MISSIONPACK
				case GTS_KAMIKAZE:
					trap_S_StartLocalSound(cgs.media.kamikazeFarSound, CHAN_ANNOUNCER);
					break;
//#endif
				default:
					break;
			}
			break;
		}

	case EV_PAIN:
		// local player sounds are triggered in CG_CheckLocalSounds,
		// so ignore events on the player
		DEBUGNAME("EV_PAIN");
		if ( cent->currentState.number != cg.snap->ps.clientNum ) {
			CG_PainEvent( cent, es->eventParm );
		}
		break;

	case EV_DEATH1:
	case EV_DEATH2:
	case EV_DEATH3:
		DEBUGNAME("EV_DEATHx");
		trap_S_StartSound( NULL, es->number, CHAN_VOICE, 
				CG_CustomSound( es->number, va("*death%i.wav", event - EV_DEATH1 + 1) ) );
		break;


	case EV_OBITUARY:
		DEBUGNAME("EV_OBITUARY");
		CG_Obituary( es );
		break;

	//
	// powerup events
	//
	case EV_POWERUP_QUAD:
		DEBUGNAME("EV_POWERUP_QUAD");
		if ( es->number == cg.snap->ps.clientNum ) {
			cg.powerupActive = PW_QUAD;
			cg.powerupTime = cg.time;
		}
		trap_S_StartSound (NULL, es->number, CHAN_ITEM, cgs.media.quadSound );
		break;
	case EV_POWERUP_BATTLESUIT:
		DEBUGNAME("EV_POWERUP_BATTLESUIT");
		if ( es->number == cg.snap->ps.clientNum ) {
			cg.powerupActive = PW_BATTLESUIT;
			cg.powerupTime = cg.time;
		}
		trap_S_StartSound (NULL, es->number, CHAN_ITEM, cgs.media.protectSound );
		break;
	case EV_SPAWN_PROTECT:
		DEBUGNAME("EV_SPAWN_PROTECT");

		break;

	case EV_POWERUP_REGEN:
		DEBUGNAME("EV_POWERUP_REGEN");
		if ( es->number == cg.snap->ps.clientNum ) {
			cg.powerupActive = PW_REGEN;
			cg.powerupTime = cg.time;
		}
		trap_S_StartSound (NULL, es->number, CHAN_ITEM, cgs.media.regenSound );
		break;

	case EV_GIB_PLAYER:
		DEBUGNAME("EV_GIB_PLAYER");
		// don't play gib sound when using the kamikaze because it interferes
		// with the kamikaze sound, downside is that the gib sound will also
		// not be played when someone is gibbed while just carrying the kamikaze
		ByteToDir( es->eventParm, dir );
		VectorCopy(cent->lerpOrigin,dieorg);

		c = rand()%100;
		if(c < 0){
			c = 0;
		}
		if(c > 100){
			c = 100;
		}
		if(c >= 0 && c <= 19){
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 20 && c <= 39) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}else if ( c >= 40 && c <= 59 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound3 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound3 );
		}else if ( c >= 60 && c <= 79) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 80 && c <= 100 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}
		CG_GibPlayer( dieorg,es->number,weapon->weaponModel, dir,0);
		break;

	case EV_GIB_PLAYERFIRE20:
		DEBUGNAME("EV_GIB_PLAYERFIRE20");
		// don't play gib sound when using the kamikaze because it interferes
		// with the kamikaze sound, downside is that the gib sound will also
		// not be played when someone is gibbed while just carrying the kamikaze
		ByteToDir( es->eventParm, dir );
		VectorCopy(cent->lerpOrigin,dieorg);

		c = rand()%100;
		if(c < 0){
			c = 0;
		}
		if(c > 100){
			c = 100;
		}
		if(c >= 0 && c <= 19){
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 20 && c <= 39) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}else if ( c >= 40 && c <= 59 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound3 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound3 );
		}else if ( c >= 60 && c <= 79) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 80 && c <= 100 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}
		CG_GibPlayer( dieorg,es->number,weapon->weaponModel, dir,20);
		break;

	case EV_GIB_PLAYERFIRE50:
		DEBUGNAME("EV_GIB_PLAYERFIRE20");
		// don't play gib sound when using the kamikaze because it interferes
		// with the kamikaze sound, downside is that the gib sound will also
		// not be played when someone is gibbed while just carrying the kamikaze
		ByteToDir( es->eventParm, dir );
		VectorCopy(cent->lerpOrigin,dieorg);

		c = rand()%100;
		if(c < 0){
			c = 0;
		}
		if(c > 100){
			c = 100;
		}
		if(c >= 0 && c <= 19){
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 20 && c <= 39) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}else if ( c >= 40 && c <= 59 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound3 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound3 );
		}else if ( c >= 60 && c <= 79) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 80 && c <= 100 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}
		CG_GibPlayer( dieorg,es->number,weapon->weaponModel, dir,40);
		break;

	case EV_GIB_PLAYERFIRE80:
		DEBUGNAME("EV_GIB_PLAYERFIRE20");
		// don't play gib sound when using the kamikaze because it interferes
		// with the kamikaze sound, downside is that the gib sound will also
		// not be played when someone is gibbed while just carrying the kamikaze
		ByteToDir( es->eventParm, dir );
		VectorCopy(cent->lerpOrigin,dieorg);

		c = rand()%100;
		if(c < 0){
			c = 0;
		}
		if(c > 100){
			c = 100;
		}
		if(c >= 0 && c <= 19){
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 20 && c <= 39) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}else if ( c >= 40 && c <= 59 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound3 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound3 );
		}else if ( c >= 60 && c <= 79) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 80 && c <= 100 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}
		CG_GibPlayer( dieorg,es->number,weapon->weaponModel, dir,80);
		break;



	case EV_GIB_PLAYERLEGS:
		DEBUGNAME("EV_GIB_PLAYERLEGS");
		// don't play gib sound when using the kamikaze because it interferes
		// with the kamikaze sound, downside is that the gib sound will also
		// not be played when someone is gibbed while just carrying the kamikaze
		ByteToDir( es->eventParm, dir );
		VectorCopy(cent->lerpOrigin,dieorg);

		c = rand()%100;
		if(c < 0){
			c = 0;
		}
		if(c > 100){
			c = 100;
		}
		if(c >= 0 && c <= 19){
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 20 && c <= 39) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}else if ( c >= 40 && c <= 59 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound3 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound3 );
		}else if ( c >= 60 && c <= 79) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 80 && c <= 100 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}
		CG_GibPlayerLegs( dieorg,es->number,weapon->weaponModel, dir,0);
		break;

	case EV_GIB_PLAYERFIRE20LEGS:
		DEBUGNAME("EV_GIB_PLAYERFIRE20LEGS");
		// don't play gib sound when using the kamikaze because it interferes
		// with the kamikaze sound, downside is that the gib sound will also
		// not be played when someone is gibbed while just carrying the kamikaze
		ByteToDir( es->eventParm, dir );
		VectorCopy(cent->lerpOrigin,dieorg);

		c = rand()%100;
		if(c < 0){
			c = 0;
		}
		if(c > 100){
			c = 100;
		}
		if(c >= 0 && c <= 19){
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 20 && c <= 39) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}else if ( c >= 40 && c <= 59 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound3 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound3 );
		}else if ( c >= 60 && c <= 79) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 80 && c <= 100 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}
		CG_GibPlayerLegs( dieorg,es->number,weapon->weaponModel, dir,20);
		break;

	case EV_GIB_PLAYERFIRE50LEGS:
		DEBUGNAME("EV_GIB_PLAYERFIRE20LEGS");
		// don't play gib sound when using the kamikaze because it interferes
		// with the kamikaze sound, downside is that the gib sound will also
		// not be played when someone is gibbed while just carrying the kamikaze
		ByteToDir( es->eventParm, dir );
		VectorCopy(cent->lerpOrigin,dieorg);

		c = rand()%100;
		if(c < 0){
			c = 0;
		}
		if(c > 100){
			c = 100;
		}
		if(c >= 0 && c <= 19){
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 20 && c <= 39) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}else if ( c >= 40 && c <= 59 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound3 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound3 );
		}else if ( c >= 60 && c <= 79) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 80 && c <= 100 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}
		CG_GibPlayerLegs( dieorg,es->number,weapon->weaponModel, dir,40);
		break;

	case EV_GIB_PLAYERFIRE80LEGS:
		DEBUGNAME("EV_GIB_PLAYERFIRE20");
		// don't play gib sound when using the kamikaze because it interferes
		// with the kamikaze sound, downside is that the gib sound will also
		// not be played when someone is gibbed while just carrying the kamikaze
		ByteToDir( es->eventParm, dir );
		VectorCopy(cent->lerpOrigin,dieorg);

		c = rand()%100;
		if(c < 0){
			c = 0;
		}
		if(c > 100){
			c = 100;
		}
		if(c >= 0 && c <= 19){
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 20 && c <= 39) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}else if ( c >= 40 && c <= 59 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound3 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound3 );
		}else if ( c >= 60 && c <= 79) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 80 && c <= 100 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}
		CG_GibPlayerLegs( dieorg,es->number,weapon->weaponModel, dir,80);
		break;



	case EV_GIB_PLAYERHEAD:
		DEBUGNAME("EV_GIB_PLAYERHEAD");
		// don't play gib sound when using the kamikaze because it interferes
		// with the kamikaze sound, downside is that the gib sound will also
		// not be played when someone is gibbed while just carrying the kamikaze
		ByteToDir( es->eventParm, dir );
		VectorCopy(cent->lerpOrigin,dieorg);

		c = rand()%100;
		if(c < 0){
			c = 0;
		}
		if(c > 100){
			c = 100;
		}
		if(c >= 0 && c <= 19){
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 20 && c <= 39) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}else if ( c >= 40 && c <= 59 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound3 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound3 );
		}else if ( c >= 60 && c <= 79) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 80 && c <= 100 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}
		CG_GibPlayerHead( dieorg,es->number,weapon->weaponModel, dir,0);
		break;

	case EV_GIB_PLAYERFIRE20HEAD:
		DEBUGNAME("EV_GIB_PLAYERFIRE20HEAD");
		// don't play gib sound when using the kamikaze because it interferes
		// with the kamikaze sound, downside is that the gib sound will also
		// not be played when someone is gibbed while just carrying the kamikaze
		ByteToDir( es->eventParm, dir );
		VectorCopy(cent->lerpOrigin,dieorg);

		c = rand()%100;
		if(c < 0){
			c = 0;
		}
		if(c > 100){
			c = 100;
		}
		if(c >= 0 && c <= 19){
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 20 && c <= 39) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}else if ( c >= 40 && c <= 59 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound3 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound3 );
		}else if ( c >= 60 && c <= 79) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 80 && c <= 100 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}
		CG_GibPlayerHead( dieorg,es->number,weapon->weaponModel, dir,20);
		break;

	case EV_GIB_PLAYERFIRE50HEAD:
		DEBUGNAME("EV_GIB_PLAYERFIRE20HEAD");
		// don't play gib sound when using the kamikaze because it interferes
		// with the kamikaze sound, downside is that the gib sound will also
		// not be played when someone is gibbed while just carrying the kamikaze
		ByteToDir( es->eventParm, dir );
		VectorCopy(cent->lerpOrigin,dieorg);

		c = rand()%100;
		if(c < 0){
			c = 0;
		}
		if(c > 100){
			c = 100;
		}
		if(c >= 0 && c <= 19){
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 20 && c <= 39) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}else if ( c >= 40 && c <= 59 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound3 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound3 );
		}else if ( c >= 60 && c <= 79) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 80 && c <= 100 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}
		CG_GibPlayerHead( dieorg,es->number,weapon->weaponModel, dir,40);
		break;

	case EV_GIB_PLAYERFIRE80HEAD:
		DEBUGNAME("EV_GIB_PLAYERFIRE20HEAD");
		// don't play gib sound when using the kamikaze because it interferes
		// with the kamikaze sound, downside is that the gib sound will also
		// not be played when someone is gibbed while just carrying the kamikaze
		ByteToDir( es->eventParm, dir );
		VectorCopy(cent->lerpOrigin,dieorg);

		c = rand()%100;
		if(c < 0){
			c = 0;
		}
		if(c > 100){
			c = 100;
		}
		if(c >= 0 && c <= 19){
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 20 && c <= 39) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}else if ( c >= 40 && c <= 59 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound3 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound3 );
		}else if ( c >= 60 && c <= 79) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 80 && c <= 100 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}
		CG_GibPlayerHead( dieorg,es->number,weapon->weaponModel, dir,80);
		break;




	case EV_GIB_PLAYERQFIRE80:
		DEBUGNAME("EV_GIB_PLAYERQFIRE80");
		// don't play gib sound when using the kamikaze because it interferes
		// with the kamikaze sound, downside is that the gib sound will also
		// not be played when someone is gibbed while just carrying the kamikaze
		ByteToDir( es->eventParm, dir );
		VectorCopy(cent->lerpOrigin,dieorg);
		c = rand()%100;
		if(c < 0){
			c = 0;
		}
		if(c > 100){
			c = 100;
		}
		if(c >= 0 && c <= 19){
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 20 && c <= 39) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}else if ( c >= 40 && c <= 59 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound3 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound3 );
		}else if ( c >= 60 && c <= 79) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 80 && c <= 100 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}
		CG_GibPlayerQ( cent->lerpOrigin ,dir,es->otherEntityNum2,es->number,80);
		break;

	case EV_GIB_PLAYERQFIRE50:
		DEBUGNAME("EV_GIB_PLAYERQFIRE50");
		// don't play gib sound when using the kamikaze because it interferes
		// with the kamikaze sound, downside is that the gib sound will also
		// not be played when someone is gibbed while just carrying the kamikaze
		ByteToDir( es->eventParm, dir );
		VectorCopy(cent->lerpOrigin,dieorg);
		c = rand()%100;
		if(c < 0){
			c = 0;
		}
		if(c > 100){
			c = 100;
		}
		if(c >= 0 && c <= 19){
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 20 && c <= 39) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}else if ( c >= 40 && c <= 59 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound3 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound3 );
		}else if ( c >= 60 && c <= 79) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 80 && c <= 100 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}
		CG_GibPlayerQ( cent->lerpOrigin ,dir,es->otherEntityNum2,es->number,50);
		break;

	case EV_GIB_PLAYERQFIRE20:
		DEBUGNAME("EV_GIB_PLAYERQFIRE20");
		// don't play gib sound when using the kamikaze because it interferes
		// with the kamikaze sound, downside is that the gib sound will also
		// not be played when someone is gibbed while just carrying the kamikaze
		ByteToDir( es->eventParm, dir );
		VectorCopy(cent->lerpOrigin,dieorg);
		c = rand()%100;
		if(c < 0){
			c = 0;
		}
		if(c > 100){
			c = 100;
		}
		if(c >= 0 && c <= 19){
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 20 && c <= 39) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}else if ( c >= 40 && c <= 59 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound3 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound3 );
		}else if ( c >= 60 && c <= 79) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 80 && c <= 100 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}
		CG_GibPlayerQ( cent->lerpOrigin ,dir,es->otherEntityNum2,es->number,20);
		break;

	case EV_GIB_PLAYERQ:
		DEBUGNAME("EV_GIB_PLAYERQ");
		// don't play gib sound when using the kamikaze because it interferes
		// with the kamikaze sound, downside is that the gib sound will also
		// not be played when someone is gibbed while just carrying the kamikaze
		ByteToDir( es->eventParm, dir );
		VectorCopy(cent->lerpOrigin,dieorg);
		c = rand()%100;
		if(c < 0){
			c = 0;
		}
		if(c > 100){
			c = 100;
		}
		if(c >= 0 && c <= 19){
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 20 && c <= 39) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}else if ( c >= 40 && c <= 59 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound3 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound3 );
		}else if ( c >= 60 && c <= 79) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 80 && c <= 100 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}
		CG_GibPlayerQ( cent->lerpOrigin ,dir,es->otherEntityNum2,es->number,0);
		break;




	case EV_GIB_PLAYERQFIRE80HEAD:
		DEBUGNAME("EV_GIB_PLAYERQFIRE80HEAD");
		// don't play gib sound when using the kamikaze because it interferes
		// with the kamikaze sound, downside is that the gib sound will also
		// not be played when someone is gibbed while just carrying the kamikaze
		ByteToDir( es->eventParm, dir );
		VectorCopy(cent->lerpOrigin,dieorg);
		c = rand()%100;
		if(c < 0){
			c = 0;
		}
		if(c > 100){
			c = 100;
		}
		if(c >= 0 && c <= 19){
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 20 && c <= 39) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}else if ( c >= 40 && c <= 59 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound3 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound3 );
		}else if ( c >= 60 && c <= 79) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 80 && c <= 100 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}
		CG_GibPlayerQHead( cent->lerpOrigin ,dir,es->otherEntityNum2,es->number,80);
		break;

	case EV_GIB_PLAYERQFIRE50HEAD:
		DEBUGNAME("EV_GIB_PLAYERQFIRE50HEAD");
		// don't play gib sound when using the kamikaze because it interferes
		// with the kamikaze sound, downside is that the gib sound will also
		// not be played when someone is gibbed while just carrying the kamikaze
		ByteToDir( es->eventParm, dir );
		VectorCopy(cent->lerpOrigin,dieorg);
		c = rand()%100;
		if(c < 0){
			c = 0;
		}
		if(c > 100){
			c = 100;
		}
		if(c >= 0 && c <= 19){
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 20 && c <= 39) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}else if ( c >= 40 && c <= 59 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound3 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound3 );
		}else if ( c >= 60 && c <= 79) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 80 && c <= 100 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}
		CG_GibPlayerQHead( cent->lerpOrigin ,dir,es->otherEntityNum2,es->number,50);
		break;

	case EV_GIB_PLAYERQFIRE20HEAD:
		DEBUGNAME("EV_GIB_PLAYERQFIRE20");
		// don't play gib sound when using the kamikaze because it interferes
		// with the kamikaze sound, downside is that the gib sound will also
		// not be played when someone is gibbed while just carrying the kamikaze
		ByteToDir( es->eventParm, dir );
		VectorCopy(cent->lerpOrigin,dieorg);
		c = rand()%100;
		if(c < 0){
			c = 0;
		}
		if(c > 100){
			c = 100;
		}
		if(c >= 0 && c <= 19){
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 20 && c <= 39) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}else if ( c >= 40 && c <= 59 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound3 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound3 );
		}else if ( c >= 60 && c <= 79) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 80 && c <= 100 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}
		CG_GibPlayerQHead( cent->lerpOrigin ,dir,es->otherEntityNum2,es->number,20);
		break;

	case EV_GIB_PLAYERQHEAD:
		DEBUGNAME("EV_GIB_PLAYERQ");
		// don't play gib sound when using the kamikaze because it interferes
		// with the kamikaze sound, downside is that the gib sound will also
		// not be played when someone is gibbed while just carrying the kamikaze
		ByteToDir( es->eventParm, dir );
		VectorCopy(cent->lerpOrigin,dieorg);
		c = rand()%100;
		if(c < 0){
			c = 0;
		}
		if(c > 100){
			c = 100;
		}
		if(c >= 0 && c <= 19){
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 20 && c <= 39) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}else if ( c >= 40 && c <= 59 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound3 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound3 );
		}else if ( c >= 60 && c <= 79) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 80 && c <= 100 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}
		CG_GibPlayerQHead( cent->lerpOrigin ,dir,es->otherEntityNum2,es->number,0);
		break;






	case EV_GIB_PLAYERQFIRE80LEGS:
		DEBUGNAME("EV_GIB_PLAYERQFIRE80LEGS");
		// don't play gib sound when using the kamikaze because it interferes
		// with the kamikaze sound, downside is that the gib sound will also
		// not be played when someone is gibbed while just carrying the kamikaze
		ByteToDir( es->eventParm, dir );
		VectorCopy(cent->lerpOrigin,dieorg);
		c = rand()%100;
		if(c < 0){
			c = 0;
		}
		if(c > 100){
			c = 100;
		}
		if(c >= 0 && c <= 19){
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 20 && c <= 39) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}else if ( c >= 40 && c <= 59 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound3 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound3 );
		}else if ( c >= 60 && c <= 79) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 80 && c <= 100 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}
		CG_GibPlayerQLegs( cent->lerpOrigin ,dir,es->otherEntityNum2,es->number,80);
		break;

	case EV_GIB_PLAYERQFIRE50LEGS:
		DEBUGNAME("EV_GIB_PLAYERQFIRE50LEGS");
		// don't play gib sound when using the kamikaze because it interferes
		// with the kamikaze sound, downside is that the gib sound will also
		// not be played when someone is gibbed while just carrying the kamikaze
		ByteToDir( es->eventParm, dir );
		VectorCopy(cent->lerpOrigin,dieorg);
		c = rand()%100;
		if(c < 0){
			c = 0;
		}
		if(c > 100){
			c = 100;
		}
		if(c >= 0 && c <= 19){
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 20 && c <= 39) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}else if ( c >= 40 && c <= 59 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound3 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound3 );
		}else if ( c >= 60 && c <= 79) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 80 && c <= 100 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}
		CG_GibPlayerQLegs( cent->lerpOrigin ,dir,es->otherEntityNum2,es->number,50);
		break;

	case EV_GIB_PLAYERQFIRE20LEGS:
		DEBUGNAME("EV_GIB_PLAYERQFIRE20LEGS");
		// don't play gib sound when using the kamikaze because it interferes
		// with the kamikaze sound, downside is that the gib sound will also
		// not be played when someone is gibbed while just carrying the kamikaze
		ByteToDir( es->eventParm, dir );
		VectorCopy(cent->lerpOrigin,dieorg);
		c = rand()%100;
		if(c < 0){
			c = 0;
		}
		if(c > 100){
			c = 100;
		}
		if(c >= 0 && c <= 19){
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 20 && c <= 39) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}else if ( c >= 40 && c <= 59 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound3 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound3 );
		}else if ( c >= 60 && c <= 79) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 80 && c <= 100 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}
		CG_GibPlayerQLegs( cent->lerpOrigin ,dir,es->otherEntityNum2,es->number,20);
		break;

	case EV_GIB_PLAYERQLEGS:
		DEBUGNAME("EV_GIB_PLAYERQLEGS");
		// don't play gib sound when using the kamikaze because it interferes
		// with the kamikaze sound, downside is that the gib sound will also
		// not be played when someone is gibbed while just carrying the kamikaze
		ByteToDir( es->eventParm, dir );
		VectorCopy(cent->lerpOrigin,dieorg);
		c = rand()%100;
		if(c < 0){
			c = 0;
		}
		if(c > 100){
			c = 100;
		}
		if(c >= 0 && c <= 19){
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 20 && c <= 39) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}else if ( c >= 40 && c <= 59 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound3 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound3 );
		}else if ( c >= 60 && c <= 79) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound );
		}else if ( c >= 80 && c <= 100 ) {
			trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound2 );
			trap_S_StartSound( dieorg, -1, CHAN_AUTO, cgs.media.gibSound2 );
		}
		CG_GibPlayerQLegs( cent->lerpOrigin ,dir,es->otherEntityNum2,es->number,0);
		break;








	case EV_STOPLOOPINGSOUND:
		DEBUGNAME("EV_STOPLOOPINGSOUND");
		trap_S_StopLoopingSound( es->number );
		es->loopSound = 0;
		break;

	case EV_DEBUG_LINE:
		DEBUGNAME("EV_DEBUG_LINE");
		CG_Beam( cent );
		break;

	default:
		DEBUGNAME("UNKNOWN");
		CG_Error( "Unknown event: %i", event );
		break;
	}

}


/*
==============
CG_CheckEvents

==============
*/
void CG_CheckEvents( centity_t *cent ) {
	// check for event-only entities
	if ( cent->currentState.eType > ET_EVENTS ) {
		if ( cent->previousEvent ) {
			return;	// already fired
		}
		// if this is a player event set the entity number of the client entity number
		if ( cent->currentState.eFlags & EF_PLAYER_EVENT ) {
			cent->currentState.number = cent->currentState.otherEntityNum;
		}

		cent->previousEvent = 1;

		cent->currentState.event = cent->currentState.eType - ET_EVENTS;
	} else {
		// check for events riding with another entity
		if ( cent->currentState.event == cent->previousEvent ) {
			return;
		}
		cent->previousEvent = cent->currentState.event;
		if ( ( cent->currentState.event & ~EV_EVENT_BITS ) == 0 ) {
			return;
		}
	}

	// calculate the position at exactly the frame time
	BG_EvaluateTrajectory( &cent->currentState.pos, cg.snap->serverTime, cent->lerpOrigin );
	CG_SetEntitySoundPosition( cent );

	CG_EntityEvent( cent, cent->lerpOrigin );
}

