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
// g_weapon.c 
// perform the server side effects of a weapon firing

#include "g_local.h"

static float    s_quadFactor;
static vec3_t   forward, right, up;
static vec3_t   muzzle;

#define NUM_NAILSHOTS 15

/*
================
G_BounceProjectile
================
*/
void G_BounceProjectile(vec3_t start, vec3_t impact, vec3_t dir, vec3_t endout)
{
	vec3_t          v, newv;
	float           dot;

	VectorSubtract(impact, start, v);
	dot = DotProduct(v, dir);
	VectorMA(v, -2 * dot, dir, newv);

	VectorNormalize(newv);
	VectorMA(impact, 8192, newv, endout);
}


/*
======================================================================

GAUNTLET

======================================================================
*/

void Weapon_Gauntlet(gentity_t * ent)
{

}

/*
===============
CheckGauntletAttack
===============
*/
qboolean CheckGauntletAttack(gentity_t * ent)
{
	trace_t         tr;
	vec3_t          end;
	gentity_t      *tent;
	gentity_t      *traceEnt;
	int             damage;

	// set aiming directions
	AngleVectors(ent->client->ps.viewangles, forward, right, up);

	CalcMuzzlePoint(ent, forward, right, up, muzzle);

	VectorMA(muzzle, 32, forward, end);
//unlagged - backward reconciliation #2
	// backward-reconcile the other clients
	//G_DoTimeShiftFor( ent );
//unlagged - backward reconciliation #2
	trap_Trace(&tr, muzzle, NULL, NULL, end, ent->s.number, MASK_SHOT);
//unlagged - backward reconciliation #2
	// put them back
	//G_UndoTimeShiftFor( ent );
//unlagged - backward reconciliation #2
	if(tr.surfaceFlags & SURF_NOIMPACT)
	{
		return qfalse;
	}

	traceEnt = &g_entities[tr.entityNum];

	// send blood impact
	if(traceEnt->takedamage && traceEnt->client || traceEnt->s.eFlags & EF_DEAD)
	{
		tent = G_TempEntity(tr.endpos, EV_MISSILE_HIT);
		tent->s.otherEntityNum = traceEnt->s.number;
		tent->s.eventParm = DirToByte(tr.plane.normal);
		tent->s.weapon = ent->s.weapon;
	}

	if(!traceEnt->takedamage)
	{
		return qfalse;
	}

	if(ent->client->ps.powerups[PW_QUAD])
	{
		G_AddEvent(ent, EV_POWERUP_QUAD, 0);
		s_quadFactor = g_quadfactor.value;
	}
	else
	{
		s_quadFactor = 1;
	}
#ifdef MISSIONPACK
	if(ent->client->persistantPowerup && ent->client->persistantPowerup->item &&
	   ent->client->persistantPowerup->item->giTag == PW_DOUBLER)
	{
		s_quadFactor *= 2;
	}
#endif
	if(Instagib.integer == 1)
	{
		damage = 300 * s_quadFactor;
	}
	else
	{
		damage = 50 * s_quadFactor;
	}
	G_Damage(traceEnt, ent, ent, forward, tr.endpos, damage, 0, MOD_GAUNTLET);

	return qtrue;
}


/*
======================================================================

MACHINEGUN

======================================================================
*/


#ifdef MISSIONPACK
#define CHAINGUN_SPREAD		600
#endif
#define MACHINEGUN_SPREAD	140
#define	MACHINEGUN_DAMAGE	5
#define	MACHINEGUN_TEAM_DAMAGE	4	// wimpier MG in teamplay

void Bullet_Fire(gentity_t * ent, float spread, int damage)
{
	trace_t         tr;
	vec3_t          end;

#ifdef MISSIONPACK
	vec3_t          impactpoint, bouncedir;
#endif
	float           r;
	float           u;
	gentity_t      *tent;
	gentity_t      *traceEnt;
	int             i, passent;

//unlagged - attack prediction #2
	// we have to use something now that the client knows in advance
	int             seed = ent->client->attackTime % 256;

//unlagged - attack prediction #2
	damage *= s_quadFactor;

//unlagged - attack prediction #2
	// this has to match what's on the client
/*
	r = random() * M_PI * 2.0f;
	u = sin(r) * crandom() * spread * 16;
	r = cos(r) * crandom() * spread * 16;
*/
	r = Q_random(&seed) * M_PI * 2.0f;
	u = sin(r) * Q_crandom(&seed) * spread * 16;
	r = cos(r) * Q_crandom(&seed) * spread * 16;
//unlagged - attack prediction #2
	VectorMA(muzzle, 8192 * 16, forward, end);
	VectorMA(end, r, right, end);
	VectorMA(end, u, up, end);

	passent = ent->s.number;
	for(i = 0; i < 10; i++)
	{

//unlagged - backward reconciliation #2
		// backward-reconcile the other clients
		//G_DoTimeShiftFor( ent );
//unlagged - backward reconciliation #2

		trap_Trace(&tr, muzzle, NULL, NULL, end, passent, MASK_SHOT);

//unlagged - backward reconciliation #2
		// put them back
		//G_UndoTimeShiftFor( ent );
//unlagged - backward reconciliation #2
		if(tr.surfaceFlags & SURF_NOIMPACT)
		{
			return;
		}

		traceEnt = &g_entities[tr.entityNum];

		// snap the endpos to integers, but nudged towards the line
		SnapVectorTowards(tr.endpos, muzzle);
		// send bullet impact
		if(traceEnt->takedamage && traceEnt->client || traceEnt->s.eFlags & EF_DEAD)
		{
			tent = G_TempEntity(tr.endpos, EV_BULLET_HIT_FLESH);
			tent->s.eventParm = traceEnt->s.number;
//unlagged - attack prediction #2
			// we need the client number to determine whether or not to
			// suppress this event
			tent->s.clientNum = ent->s.clientNum;
//unlagged - attack prediction #2
			if(LogAccuracyHit(traceEnt, ent))
			{
				ent->client->accuracy_hits++;
			}
		}
		else
		{
			tent = G_TempEntity(tr.endpos, EV_BULLET_HIT_WALL);
			tent->s.eventParm = DirToByte(tr.plane.normal);
//unlagged - attack prediction #2
			// we need the client number to determine whether or not to
			// suppress this event
			tent->s.clientNum = ent->s.clientNum;
//unlagged - attack prediction #2
		}
		tent->s.otherEntityNum = ent->s.number;

		if(traceEnt->takedamage)
		{
#ifdef MISSIONPACK
			if(traceEnt->client && traceEnt->client->invulnerabilityTime > level.time)
			{
				if(G_InvulnerabilityEffect(traceEnt, forward, tr.endpos, impactpoint, bouncedir))
				{
					G_BounceProjectile(muzzle, impactpoint, bouncedir, end);
					VectorCopy(impactpoint, muzzle);
					// the player can hit him/herself with the bounced rail
					passent = ENTITYNUM_NONE;
				}
				else
				{
					VectorCopy(tr.endpos, muzzle);
					passent = traceEnt->s.number;
				}
				continue;
			}
			else
			{
#endif
				G_Damage(traceEnt, ent, ent, forward, tr.endpos, damage, 0, MOD_MACHINEGUN);
#ifdef MISSIONPACK
			}
#endif
		}
		break;
	}
}


/*
======================================================================

BFG

======================================================================
*/

void BFG_Fire(gentity_t * ent)
{
	gentity_t      *m;

	m = fire_bfg(ent, muzzle, forward);
	m->damage *= s_quadFactor;
	m->splashDamage *= s_quadFactor;

//  VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );  // "real" physics
}


/*
======================================================================

SHOTGUN

======================================================================
*/

// DEFAULT_SHOTGUN_SPREAD and DEFAULT_SHOTGUN_COUNT are in bg_public.h, because
// client predicts same spreads
#define	DEFAULT_SHOTGUN_DAMAGE	10

qboolean ShotgunPellet(vec3_t start, vec3_t end, gentity_t * ent)
{
	trace_t         tr;
	int             damage, i, passent;
	gentity_t      *traceEnt;
	vec3_t          tr_start, tr_end;

	passent = ent->s.number;
	VectorCopy(start, tr_start);
	VectorCopy(end, tr_end);
	for(i = 0; i < 10; i++)
	{
		trap_Trace(&tr, tr_start, NULL, NULL, tr_end, passent, MASK_SHOT);
		traceEnt = &g_entities[tr.entityNum];

		// send bullet impact
		if(tr.surfaceFlags & SURF_NOIMPACT)
		{
			return qfalse;
		}

		if(traceEnt->takedamage)
		{
			damage = DEFAULT_SHOTGUN_DAMAGE * s_quadFactor;
#ifdef MISSIONPACK
			if(traceEnt->client && traceEnt->client->invulnerabilityTime > level.time)
			{
				if(G_InvulnerabilityEffect(traceEnt, forward, tr.endpos, impactpoint, bouncedir))
				{
					G_BounceProjectile(tr_start, impactpoint, bouncedir, tr_end);
					VectorCopy(impactpoint, tr_start);
					// the player can hit him/herself with the bounced rail
					passent = ENTITYNUM_NONE;
				}
				else
				{
					VectorCopy(tr.endpos, tr_start);
					passent = traceEnt->s.number;
				}
				continue;
			}
			else
			{
				G_Damage(traceEnt, ent, ent, forward, tr.endpos, damage, 0, MOD_SHOTGUN);
				if(LogAccuracyHit(traceEnt, ent))
				{
					return qtrue;
				}
			}
#else
			G_Damage(traceEnt, ent, ent, forward, tr.endpos, damage, 0, MOD_SHOTGUN);
			if(LogAccuracyHit(traceEnt, ent))
			{
				return qtrue;
			}
#endif
		}
		return qfalse;
	}
	return qfalse;
}

// this should match CG_ShotgunPattern
void ShotgunPattern(vec3_t origin, vec3_t origin2, int seed, gentity_t * ent)
{
	int             i;
	float           r, u;
	vec3_t          end;
	vec3_t          forward, right, up;
	int             oldScore;
	qboolean        hitClient = qfalse;

//unlagged - attack prediction #2
	// use this for testing
	//Com_Printf( "Server seed: %d\n", seed );
//unlagged - attack prediction #2
	// derive the right and up vectors from the forward vector, because
	// the client won't have any other information
	VectorNormalize2(origin2, forward);
	PerpendicularVector(right, forward);
	CrossProduct(forward, right, up);

	oldScore = ent->client->ps.persistant[PERS_SCORE];
//unlagged - backward reconciliation #2
	// backward-reconcile the other clients
	//G_DoTimeShiftFor( ent );
//unlagged - backward reconciliation #2
	// generate the "random" spread pattern
	for(i = 0; i < DEFAULT_SHOTGUN_COUNT; i++)
	{
		r = Q_crandom(&seed) * DEFAULT_SHOTGUN_SPREAD * 16;
		u = Q_crandom(&seed) * DEFAULT_SHOTGUN_SPREAD * 16;
		VectorMA(origin, 8192 * 16, forward, end);
		VectorMA(end, r, right, end);
		VectorMA(end, u, up, end);
		if(ShotgunPellet(origin, end, ent) && !hitClient)
		{
			hitClient = qtrue;
			ent->client->accuracy_hits++;
		}
	}
//unlagged - backward reconciliation #2
	// put them back
	//G_UndoTimeShiftFor( ent );
//unlagged - backward reconciliation #2
}


void weapon_supershotgun_fire(gentity_t * ent)
{
	gentity_t      *tent;

	// send shotgun blast
	tent = G_TempEntity(muzzle, EV_SHOTGUN);
	VectorScale(forward, 4096, tent->s.origin2);
	SnapVector(tent->s.origin2);
//unlagged - attack prediction #2
	// this has to be something the client can predict now
	//tent->s.eventParm = rand() & 255;     // seed for spread pattern
	tent->s.eventParm = ent->client->attackTime % 256;	// seed for spread pattern
//unlagged - attack prediction #2
	tent->s.otherEntityNum = ent->s.number;

	ShotgunPattern(tent->s.pos.trBase, tent->s.origin2, tent->s.eventParm, ent);
}


/*
======================================================================

GRENADE LAUNCHER

======================================================================
*/

void weapon_grenadelauncher_fire(gentity_t * ent)
{
	gentity_t      *m;

	// extra vertical velocity
	forward[2] += 0.2f;
	VectorNormalize(forward);

	VectorMA(muzzle, -20, &forward[0], muzzle);
	m = fire_grenade(ent, muzzle, forward);
	m->damage *= s_quadFactor;
	m->splashDamage *= s_quadFactor;

//  VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );  // "real" physics
}

/*
======================================================================

ROCKET

======================================================================
*/

void Weapon_RocketLauncher_Fire(gentity_t * ent)
{
	gentity_t      *m;

	m = fire_rocket(ent, muzzle, forward);
	m->damage *= s_quadFactor;
	m->splashDamage *= s_quadFactor;

//  VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );  // "real" physics
}


/*
======================================================================

PLASMA GUN

======================================================================
*/

void Weapon_Plasmagun_Fire(gentity_t * ent)
{
	gentity_t      *m;

	m = fire_plasma(ent, muzzle, forward);
	m->damage *= s_quadFactor;
	m->splashDamage *= s_quadFactor;

//  VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );  // "real" physics
}

/*
======================================================================

LIGHTNING GUN

======================================================================
*/

void Weapon_LightningFire(gentity_t * ent)
{
	trace_t         tr;
	vec3_t          end;

#ifdef MISSIONPACK
	vec3_t          impactpoint, bouncedir;
#endif
	gentity_t      *traceEnt /*, *tent, *tent2 */ ;
	int             damage, i, passent;


	damage = g_lightningDamage.integer * s_quadFactor;

	passent = ent->s.number;
	for(i = 0; i < 10; i++)
	{
		VectorMA(muzzle, LIGHTNING_RANGE, forward, end);

		//G_DoTimeShiftFor( ent );

		trap_Trace(&tr, muzzle, NULL, NULL, end, passent, MASK_SHOT);

		// put them back
		//G_UndoTimeShiftFor( ent );

		traceEnt = &g_entities[tr.entityNum];

		if(tr.entityNum == ENTITYNUM_NONE)
		{
			return;
		}

		if(traceEnt->takedamage)
		{
			G_Damage(traceEnt, ent, ent, forward, tr.endpos, damage, 0, MOD_LIGHTNING);
		}

		if(traceEnt->takedamage && traceEnt->client || traceEnt->s.eFlags & EF_DEAD)
		{
			gentity_t      *te;

			te = G_TempEntity(traceEnt->r.currentOrigin, EV_LG_HIT);
			te->s.otherEntityNum2 = traceEnt->client->ps.clientNum;
			//  te->s.eventParm = soundIndex;
			if(LogAccuracyHit(traceEnt, ent))
			{
				ent->client->accuracy_hits++;
			}
		}
		break;
	}
}

/*
======================================================================

RAILGUN

======================================================================
*/
#define	MAX_RAIL_HITS	20

qboolean        G_RadiusRailDamage(vec3_t origin, gentity_t * attacker, float damage, int radius,
								   gentity_t * ignore, int mod, vec3_t start, const vec3_t end);

/*
=================
weapon_railgun_fire2
=================

void weapon_irailgun_fire (gentity_t *ent) {
	vec3_t		end, knock;
	trace_t		trace;
	gentity_t	*tent;
	int			damage;
	int			passent;
	vec3_t		move, step;
	float		len, j, spacing;
	int			extra;
	int			extra2;
	gentity_t	*unlinkedEntities[MAX_RAIL_HITS];
	gclient_t	*client;

	gentity_t	*traceEnt;


	client = ent->client;

	extra = client->charge / 1.5;
	if ( extra > 104){
		extra = 104;
	}
	extra2 = 46 + extra;
	damage = extra2 * s_quadFactor;

	VectorMA (muzzle, 8192, forward, end);

//unlagged - backward reconciliation #2
	// backward-reconcile the other clients
	//G_DoTimeShiftFor( ent );
//unlagged - backward reconciliation #2

	passent = ent->s.number;

	trap_Trace (&trace, muzzle, NULL, NULL, end, passent, MASK_SOLID );
	traceEnt = &g_entities[ trace.entityNum ];
   radius = damage - 46 / 10;
	spacing = radius;
	VectorCopy( muzzle, move );
	VectorSubtract( trace.endpos, muzzle, step );
	len = VectorNormalize( step );
	VectorScale( step, spacing, step );

 

//	for (j = 0; j < len; j += spacing ) {
		if( G_RadiusRailDamage( muzzle, ent, damage, radius, ent, MOD_RAILGUN ,muzzle,trace.endpos) ) {
		//	G_Damage (traceEnt, ent, ent, forward, trace.endpos, damage, 0, MOD_RAILGUN);
			ent->client->accuracy_hits ++;

		}
	//	VectorAdd( move, step, move );
//	}

	// give the thing a big kick
	VectorScale( forward, -1.0f, knock );
	G_Damage( ent, ent, ent, knock, muzzle, damage/4, DAMAGE_NO_SELF, MOD_RAILGUN );

	// the final trace endpos will be the terminal point of the rail trail

	// snap the endpos to integers to save net bandwidth, but nudged towards the line
	SnapVectorTowards( trace.endpos, muzzle );

	// send railgun beam effect
	tent = G_TempEntity( trace.endpos, EV_RAILTRAIL );

	// set player number for custom colors on the railtrail
	tent->s.clientNum = ent->s.clientNum;

	VectorCopy( muzzle, tent->s.origin2 );
	// move origin a bit to come closer to the drawn gun muzzle
	VectorMA( tent->s.origin2, 4, right, tent->s.origin2 );
	VectorMA( tent->s.origin2, -1, up, tent->s.origin2 );

	// no explosion at end if SURF_NOIMPACT, but still make the trail
	if ( trace.surfaceFlags & SURF_NOIMPACT ) {
		tent->s.eventParm = 255;	// don't make the explosion at the end
	} else {
		tent->s.eventParm = DirToByte( trace.plane.normal );
	}
	tent->s.clientNum = ent->s.clientNum;
//unlagged - backward reconciliation #2
	// put them back
	//G_UndoTimeShiftFor( ent );
//unlagged - backward reconciliation #2
}
*/

/*
=================
weapon_railgun_fire
=================
*/

void weapon_irailgun_fire(gentity_t * ent)
{
/*	vec3_t		end;
#ifdef MISSIONPACK
	vec3_t impactpoint, bouncedir;
#endif
	trace_t		trace;
	gentity_t	*tent;
	gentity_t	*traceEnt;
	int			damage;
	int			i;
	int			hits;
	int			unlinked;
	int			passent;
	int			extra;
	int			extra2;
	gentity_t	*unlinkedEntities[MAX_RAIL_HITS];
	gclient_t	*client;
	int			radius;
//	vec3_t		mins, maxs;
//	int			i2;
//	vec3_t		delta;
//	float		dist,dist2;
//	float		mins0, maxs0,mins1, maxs1,mins2, maxs2;

	client = ent->client;


	extra = client->charge / 1.5;
	if ( extra > 104){
		extra = 104;
	}
	extra2 = 46 + extra;
	damage = extra2 * s_quadFactor;
//	trap_SendServerCommand( ent-g_entities,va("print \"RAILGUN DAMAGE = %i\n\"", damage) );
   radius = extra2 - 46 / 10;
//	for ( i2 = 0 ; i2 < 3 ; i2++ ) {

//	}

	VectorMA (muzzle, 8192, forward, end);
//unlagged - backward reconciliation #2
	// backward-reconcile the other clients
	//G_DoTimeShiftFor( ent );
//unlagged - backward reconciliation #2
	// trace only against the solids, so the railgun will go through people
	unlinked = 0;
	hits = 0;
	passent = ent->s.number;


//	trap_Trace (&trace2, muzzle, NULL, NULL, end, passent, MASK_SOLID );
//		VectorSubtract( muzzle, trace2.endpos, delta );
	//	dist = VectorLength( delta );
	//	dist2 = dist/2;

//	VectorScale( forward, -1.0f, knock );
//	G_Damage( ent, ent, ent, knock, muzzle, damage/4, DAMAGE_NO_SELF, MOD_RAILGUN );

	do {
	//	trap_Trace (&trace, muzzle, mins, maxs, end, passent, MASK_SHOT );
		trap_TraceCapsule (&trace, muzzle, NULL, NULL, end, passent, MASK_SHOT );
		if ( trace.entityNum >= ENTITYNUM_MAX_NORMAL ) {
			break;
		}


		traceEnt = &g_entities[ trace.entityNum ];
		if ( traceEnt->takedamage ) {
				if( LogAccuracyHit( traceEnt, ent ) ) {
					hits++;
				}
				G_Damage (traceEnt, ent, ent, forward, trace.endpos, damage, 0, MOD_RAILGUN);
		}
		if ( trace.contents & CONTENTS_SOLID ) {
			break;		// we hit something solid enough to stop the beam
		}
		// unlink this entity, so the next trace will go past it
		trap_UnlinkEntity( traceEnt );
		unlinkedEntities[unlinked] = traceEnt;
		unlinked++;
	} while ( unlinked < MAX_RAIL_HITS );
//unlagged - backward reconciliation #2
	// put them back
	//G_UndoTimeShiftFor( ent );
//unlagged - backward reconciliation #2
	// link back in any entities we unlinked
	for ( i = 0 ; i < unlinked ; i++ ) {
		trap_LinkEntity( unlinkedEntities[i] );
	}

	// the final trace endpos will be the terminal point of the rail trail

	// snap the endpos to integers to save net bandwidth, but nudged towards the line
	SnapVectorTowards( trace.endpos, muzzle );

	// send railgun beam effect
	tent = G_TempEntity( trace.endpos, EV_RAILTRAIL2 );
	// set player number for custom colors on the railtrail
	tent->s.clientNum = ent->s.clientNum;

	VectorCopy( muzzle, tent->s.origin2 );
	// move origin a bit to come closer to the drawn gun muzzle
	VectorMA( tent->s.origin2, 4, right, tent->s.origin2 );
	VectorMA( tent->s.origin2, -1, up, tent->s.origin2 );

	// no explosion at end if SURF_NOIMPACT, but still make the trail
	if ( trace.surfaceFlags & SURF_NOIMPACT ) {
		tent->s.eventParm = 255;	// don't make the explosion at the end
	} else {
		tent->s.eventParm = DirToByte( trace.plane.normal );
	}
	tent->s.clientNum = ent->s.clientNum;

	// give the shooter a reward sound if they have made two railgun hits in a row
	if ( hits == 0 ) {
		// complete miss
		ent->client->accurateCount = 0;
	} else {
		// check for "impressive" reward sound
		ent->client->accurateCount += hits;
		if ( ent->client->accurateCount >= 2 ) {
			ent->client->accurateCount -= 2;
			ent->client->ps.persistant[PERS_IMPRESSIVE_COUNT]++;
			// add the sprite over the player's head
			ent->client->ps.eFlags &= ~(EF_AWARD_RLRGCOMBO | EF_AWARD_IMPRESSIVE | EF_AWARD_EXCELLENT | EF_AWARD_GAUNTLET | EF_AWARD_ASSIST | EF_AWARD_DEFEND | EF_AWARD_CAP );
			ent->client->ps.eFlags |= EF_AWARD_IMPRESSIVE;
			ent->client->rewardTime = level.time + REWARD_SPRITE_TIME;
		}
		ent->client->accuracy_hits++;
	}*/
	vec3_t          end;

#ifdef MISSIONPACK
	vec3_t          impactpoint, bouncedir;
#endif
	trace_t         trace;
	gentity_t      *tent;
	gentity_t      *traceEnt;
	int             damage;
	int             i;
	int             hits;
	int             unlinked;
	int             passent;
	gentity_t      *unlinkedEntities[MAX_RAIL_HITS];
	gclient_t      *client;


	client = ent->client;


	damage = 90 * s_quadFactor;


	VectorMA(muzzle, 8192, forward, end);

//unlagged - backward reconciliation #2
	// backward-reconcile the other clients
	//G_DoTimeShiftFor( ent );
//unlagged - backward reconciliation #2
	// trace only against the solids, so the railgun will go through people
	unlinked = 0;
	hits = 0;
	passent = ent->s.number;
	do
	{
		trap_Trace(&trace, muzzle, NULL, NULL, end, passent, MASK_SHOT);
		if(trace.entityNum >= ENTITYNUM_MAX_NORMAL)
		{
			break;
		}
		traceEnt = &g_entities[trace.entityNum];
		if(traceEnt->takedamage)
		{
#ifdef MISSIONPACK
			if(traceEnt->client && traceEnt->client->invulnerabilityTime > level.time)
			{
				if(G_InvulnerabilityEffect(traceEnt, forward, trace.endpos, impactpoint, bouncedir))
				{
					G_BounceProjectile(muzzle, impactpoint, bouncedir, end);
					// snap the endpos to integers to save net bandwidth, but nudged towards the line
					SnapVectorTowards(trace.endpos, muzzle);
					// send railgun beam effect
					tent = G_TempEntity(trace.endpos, EV_RAILTRAIL);
					// set player number for custom colors on the railtrail
					tent->s.clientNum = ent->s.clientNum;
					VectorCopy(muzzle, tent->s.origin2);
					// move origin a bit to come closer to the drawn gun muzzle
					VectorMA(tent->s.origin2, 4, right, tent->s.origin2);
					VectorMA(tent->s.origin2, -1, up, tent->s.origin2);
					tent->s.eventParm = 255;	// don't make the explosion at the end
					//
					VectorCopy(impactpoint, muzzle);
					// the player can hit him/herself with the bounced rail
					passent = ENTITYNUM_NONE;
				}
			}
			else
			{
				if(LogAccuracyHit(traceEnt, ent))
				{
					hits++;
				}
				G_Damage(traceEnt, ent, ent, forward, trace.endpos, damage, 0, MOD_RAILGUN);
			}
#else
			if(LogAccuracyHit(traceEnt, ent))
			{
				hits++;
			}
			G_Damage(traceEnt, ent, ent, forward, trace.endpos, damage, 0, MOD_RAILGUN);



#endif
		}
		if(trace.contents & CONTENTS_SOLID)
		{
			break;				// we hit something solid enough to stop the beam
		}
		// unlink this entity, so the next trace will go past it
		trap_UnlinkEntity(traceEnt);
		unlinkedEntities[unlinked] = traceEnt;
		unlinked++;
	} while(unlinked < MAX_RAIL_HITS);
//unlagged - backward reconciliation #2
	// put them back
	//G_UndoTimeShiftFor( ent );
//unlagged - backward reconciliation #2
	// link back in any entities we unlinked
	for(i = 0; i < unlinked; i++)
	{
		trap_LinkEntity(unlinkedEntities[i]);
	}

	// the final trace endpos will be the terminal point of the rail trail

	// snap the endpos to integers to save net bandwidth, but nudged towards the line
	SnapVectorTowards(trace.endpos, muzzle);

	// send railgun beam effect
	tent = G_TempEntity(trace.endpos, EV_RAILTRAIL);
	// set player number for custom colors on the railtrail
	tent->s.clientNum = ent->s.clientNum;

	VectorCopy(muzzle, tent->s.origin2);
	// move origin a bit to come closer to the drawn gun muzzle
	VectorMA(tent->s.origin2, 4, right, tent->s.origin2);
	VectorMA(tent->s.origin2, -1, up, tent->s.origin2);

	// no explosion at end if SURF_NOIMPACT, but still make the trail
	if(trace.surfaceFlags & SURF_NOIMPACT)
	{
		tent->s.eventParm = 255;	// don't make the explosion at the end
	}
	else
	{
		tent->s.eventParm = DirToByte(trace.plane.normal);
	}
	tent->s.clientNum = ent->s.clientNum;

	// give the shooter a reward sound if they have made two railgun hits in a row
	if(hits == 0)
	{
		// complete miss
		ent->client->accurateCount = 0;
	}
	else
	{
		// check for "impressive" reward sound
		ent->client->accurateCount += hits;
		if(ent->client->accurateCount >= 2)
		{
			ent->client->accurateCount -= 2;
			ent->client->ps.persistant[PERS_IMPRESSIVE_COUNT]++;
			// add the sprite over the player's head
			ent->client->ps.eFlags &=
				~(EF_AWARD_RLRGCOMBO | EF_AWARD_IMPRESSIVE | EF_AWARD_EXCELLENT | EF_AWARD_GAUNTLET | EF_AWARD_ASSIST |
				  EF_AWARD_DEFEND | EF_AWARD_CAP);
			ent->client->ps.eFlags |= EF_AWARD_IMPRESSIVE;
			ent->client->rewardTime = level.time + REWARD_SPRITE_TIME;
		}
		ent->client->accuracy_hits++;
	}
}

void weapon_railgun_fire(gentity_t * ent)
{
	vec3_t          end;

#ifdef MISSIONPACK
	vec3_t          impactpoint, bouncedir;
#endif
	trace_t         trace;
	gentity_t      *tent;
	gentity_t      *traceEnt;
	int             damage;
	int             i;
	int             hits;
	int             unlinked;
	int             passent;
	gentity_t      *unlinkedEntities[MAX_RAIL_HITS];
	gclient_t      *client;


	client = ent->client;

	if(Instagib.integer == 1)
	{
		if(InstaWeapon.integer == 0)
		{
			damage = 1000 * s_quadFactor;
		}
		else if(InstaWeapon.integer == 2)
		{
			damage = 90 * s_quadFactor;
		}
	}

	if(Instagib.integer == 0)
	{
		damage = 90 * s_quadFactor;
	}

	VectorMA(muzzle, 8192, forward, end);

//unlagged - backward reconciliation #2
	// backward-reconcile the other clients
	//G_DoTimeShiftFor( ent );
//unlagged - backward reconciliation #2
	// trace only against the solids, so the railgun will go through people
	unlinked = 0;
	hits = 0;
	passent = ent->s.number;
	do
	{
		trap_Trace(&trace, muzzle, NULL, NULL, end, passent, MASK_SHOT);
		if(trace.entityNum >= ENTITYNUM_MAX_NORMAL)
		{
			break;
		}
		traceEnt = &g_entities[trace.entityNum];
		if(traceEnt->takedamage)
		{
#ifdef MISSIONPACK
			if(traceEnt->client && traceEnt->client->invulnerabilityTime > level.time)
			{
				if(G_InvulnerabilityEffect(traceEnt, forward, trace.endpos, impactpoint, bouncedir))
				{
					G_BounceProjectile(muzzle, impactpoint, bouncedir, end);
					// snap the endpos to integers to save net bandwidth, but nudged towards the line
					SnapVectorTowards(trace.endpos, muzzle);
					// send railgun beam effect
					tent = G_TempEntity(trace.endpos, EV_RAILTRAIL);
					// set player number for custom colors on the railtrail
					tent->s.clientNum = ent->s.clientNum;
					VectorCopy(muzzle, tent->s.origin2);
					// move origin a bit to come closer to the drawn gun muzzle
					VectorMA(tent->s.origin2, 4, right, tent->s.origin2);
					VectorMA(tent->s.origin2, -1, up, tent->s.origin2);
					tent->s.eventParm = 255;	// don't make the explosion at the end
					//
					VectorCopy(impactpoint, muzzle);
					// the player can hit him/herself with the bounced rail
					passent = ENTITYNUM_NONE;
				}
			}
			else
			{
				if(LogAccuracyHit(traceEnt, ent))
				{
					hits++;
				}
				G_Damage(traceEnt, ent, ent, forward, trace.endpos, damage, 0, MOD_RAILGUN);
			}
#else
			if(LogAccuracyHit(traceEnt, ent))
			{
				hits++;
			}
			G_Damage(traceEnt, ent, ent, forward, trace.endpos, damage, 0, MOD_RAILGUN);



#endif
		}
		if(trace.contents & CONTENTS_SOLID)
		{
			break;				// we hit something solid enough to stop the beam
		}
		// unlink this entity, so the next trace will go past it
		trap_UnlinkEntity(traceEnt);
		unlinkedEntities[unlinked] = traceEnt;
		unlinked++;
	} while(unlinked < MAX_RAIL_HITS);
//unlagged - backward reconciliation #2
	// put them back
	//G_UndoTimeShiftFor( ent );
//unlagged - backward reconciliation #2
	// link back in any entities we unlinked
	for(i = 0; i < unlinked; i++)
	{
		trap_LinkEntity(unlinkedEntities[i]);
	}

	// the final trace endpos will be the terminal point of the rail trail

	// snap the endpos to integers to save net bandwidth, but nudged towards the line
	SnapVectorTowards(trace.endpos, muzzle);

	// send railgun beam effect
	tent = G_TempEntity(trace.endpos, EV_RAILTRAIL);
	// set player number for custom colors on the railtrail
	tent->s.clientNum = ent->s.clientNum;

	VectorCopy(muzzle, tent->s.origin2);
	// move origin a bit to come closer to the drawn gun muzzle
	VectorMA(tent->s.origin2, 4, right, tent->s.origin2);
	VectorMA(tent->s.origin2, -1, up, tent->s.origin2);

	// no explosion at end if SURF_NOIMPACT, but still make the trail
	if(trace.surfaceFlags & SURF_NOIMPACT)
	{
		tent->s.eventParm = 255;	// don't make the explosion at the end
	}
	else
	{
		tent->s.eventParm = DirToByte(trace.plane.normal);
	}
	tent->s.clientNum = ent->s.clientNum;

	// give the shooter a reward sound if they have made two railgun hits in a row
	if(hits == 0)
	{
		// complete miss
		ent->client->accurateCount = 0;
	}
	else
	{
		// check for "impressive" reward sound
		ent->client->accurateCount += hits;
		if(ent->client->accurateCount >= 2)
		{
			ent->client->accurateCount -= 2;
			ent->client->ps.persistant[PERS_IMPRESSIVE_COUNT]++;
			// add the sprite over the player's head
			ent->client->ps.eFlags &=
				~(EF_AWARD_RLRGCOMBO | EF_AWARD_IMPRESSIVE | EF_AWARD_EXCELLENT | EF_AWARD_GAUNTLET | EF_AWARD_ASSIST |
				  EF_AWARD_DEFEND | EF_AWARD_CAP);
			ent->client->ps.eFlags |= EF_AWARD_IMPRESSIVE;
			ent->client->rewardTime = level.time + REWARD_SPRITE_TIME;
		}
		ent->client->accuracy_hits++;
	}

}



void G_BurnMeGood(gentity_t * self, gentity_t * body)
{

	// add the new damage
	body->flameQuota += 10;
	body->flameQuotaTime = level.time;

	// JPW NERVE -- yet another flamethrower damage model, trying to find a feels-good damage combo that isn't overpowered
	if(body->lastBurnedFrameNumber != level.framenum)
	{
		G_Damage(body, self, self, vec3_origin, self->r.currentOrigin, 5 * s_quadFactor, 0, MOD_FLAMETHROWER);	// was 2 dmg in release ver, hit avg. 2.5 times per frame
		body->lastBurnedFrameNumber = level.framenum;
	}
	if(body->client && (body->health <= 0 || body->flameQuota > 0))
	{
		body->onFireEnd = level.time + FIRE_FLASH_TIME;
		body->flameBurnEnt = self->s.number;
	}
	else if(!body->client)
	{
		body->s.generic1 |= GNF_ONFIRE;
	}
}
void            G_FlameDamage(gentity_t * self);

// those are used in the cg_ traces calls
static vec3_t   flameChunkMins = { -4, -4, -4 };
static vec3_t   flameChunkMaxs = { 4, 4, 4 };

#define SQR_SIN_T 0.44			// ~ sqr(sin(20))

void Weapon_FlamethrowerFire(gentity_t * ent)
{
	gentity_t      *traceEnt;
	vec3_t          start;

//  vec3_t      trace_start;
//  vec3_t      trace_end;
//  trace_t     trace;
	trace_t         tr2;
	vec3_t          end;
	gentity_t      *traceEnt2;
	int             i, passent;

	passent = ent->s.number;
	for(i = 0; i < 10; i++)
	{
		VectorMA(muzzle, 220, forward, end);
		trap_Trace(&tr2, muzzle, NULL, NULL, end, passent, MASK_FLAME | CONTENTS_BODY);
		if(tr2.startsolid)
		{
			continue;
		}

		traceEnt2 = &g_entities[tr2.entityNum];

		if(traceEnt2->takedamage)
		{
			G_BurnMeGood(ent, traceEnt2);
		}

	}

	VectorCopy(ent->r.currentOrigin, start);
	start[2] += ent->client->ps.viewheight;

	VectorMA(start, -8, forward, start);
	VectorMA(start, 4, right, start);
	VectorMA(start, -8, up, start);
	traceEnt = fire_flamechunk(ent, start, forward);
}



/*
======================================================================

INSTARAILGUN

======================================================================
*/


/*
=================
weapon_instarailgun_fire
=================

void weapon_instarailgun_fire (gentity_t *ent) {
	vec3_t		end;
#ifdef MISSIONPACK
	vec3_t impactpoint, bouncedir;
#endif
	trace_t		trace;
	gentity_t	*tent;
	gentity_t	*traceEnt;
	int			damage;
	int			i;
	int			hits;
	int			unlinked;
	int			passent;
	gentity_t	*unlinkedEntities[MAX_RAIL_HITS];

	if (Instagib.integer == 1 ) {
		if (InstaWeapon.integer == 0){
			damage = 1000 * s_quadFactor;
		} else if (InstaWeapon.integer >= 0){
			damage = 69 * s_quadFactor;
		}
	}

	if (Instagib.integer == 0){
		damage = 85 * s_quadFactor;
	}
	VectorMA (muzzle, 8192, forward, end);
//unlagged - backward reconciliation #2
	// backward-reconcile the other clients
	//G_DoTimeShiftFor( ent );
//unlagged - backward reconciliation #2
	// trace only against the solids, so the railgun will go through people
	unlinked = 0;
	hits = 0;
	passent = ent->s.number;
	do {
		trap_Trace (&trace, muzzle, NULL, NULL, end, passent, MASK_SHOT );
		if ( trace.entityNum >= ENTITYNUM_MAX_NORMAL ) {
			break;
		}
		traceEnt = &g_entities[ trace.entityNum ];
		if ( traceEnt->takedamage ) {
#ifdef MISSIONPACK
			if ( traceEnt->client && traceEnt->client->invulnerabilityTime > level.time ) {
				if ( G_InvulnerabilityEffect( traceEnt, forward, trace.endpos, impactpoint, bouncedir ) ) {
					G_BounceProjectile( muzzle, impactpoint, bouncedir, end );
					// snap the endpos to integers to save net bandwidth, but nudged towards the line
					SnapVectorTowards( trace.endpos, muzzle );
					// send railgun beam effect
					tent = G_TempEntity( trace.endpos, EV_RAILTRAIL );
					// set player number for custom colors on the railtrail
					tent->s.clientNum = ent->s.clientNum;
					VectorCopy( muzzle, tent->s.origin2 );
					// move origin a bit to come closer to the drawn gun muzzle
					VectorMA( tent->s.origin2, 4, right, tent->s.origin2 );
					VectorMA( tent->s.origin2, -1, up, tent->s.origin2 );
					tent->s.eventParm = 255;	// don't make the explosion at the end
					//
					VectorCopy( impactpoint, muzzle );
					// the player can hit him/herself with the bounced rail
					passent = ENTITYNUM_NONE;
				}
			}
			else {
				if( LogAccuracyHit( traceEnt, ent ) ) {
					hits++;
				}
				G_Damage (traceEnt, ent, ent, forward, trace.endpos, damage, 0, MOD_RAILGUN);
			}
#else
				if( LogAccuracyHit( traceEnt, ent ) ) {
					hits++;
				}
				G_Damage (traceEnt, ent, ent, forward, trace.endpos, damage, 0, MOD_RAILGUN);
#endif
		}
		if ( trace.contents & CONTENTS_SOLID ) {
			break;		// we hit something solid enough to stop the beam
		}
		// unlink this entity, so the next trace will go past it
		trap_UnlinkEntity( traceEnt );
		unlinkedEntities[unlinked] = traceEnt;
		unlinked++;
	} while ( unlinked < MAX_RAIL_HITS );
//unlagged - backward reconciliation #2
	// put them back
	//G_UndoTimeShiftFor( ent );
//unlagged - backward reconciliation #2
	// link back in any entities we unlinked
	for ( i = 0 ; i < unlinked ; i++ ) {
		trap_LinkEntity( unlinkedEntities[i] );
	}

	// the final trace endpos will be the terminal point of the rail trail

	// snap the endpos to integers to save net bandwidth, but nudged towards the line
	SnapVectorTowards( trace.endpos, muzzle );

	// send railgun beam effect
	tent = G_TempEntity( trace.endpos, EV_RAILTRAIL );
	// set player number for custom colors on the railtrail
	tent->s.clientNum = ent->s.clientNum;

	VectorCopy( muzzle, tent->s.origin2 );
	// move origin a bit to come closer to the drawn gun muzzle
	VectorMA( tent->s.origin2, 4, right, tent->s.origin2 );
	VectorMA( tent->s.origin2, -1, up, tent->s.origin2 );

	// no explosion at end if SURF_NOIMPACT, but still make the trail
	if ( trace.surfaceFlags & SURF_NOIMPACT ) {
		tent->s.eventParm = 255;	// don't make the explosion at the end
	} else {
		tent->s.eventParm = DirToByte( trace.plane.normal );
	}
	tent->s.clientNum = ent->s.clientNum;

	// give the shooter a reward sound if they have made two railgun hits in a row
	if ( hits == 0 ) {
		// complete miss
		ent->client->accurateCount = 0;
	} else {
		// check for "impressive" reward sound
		ent->client->accurateCount += hits;
		if ( ent->client->accurateCount >= 2 ) {
			ent->client->accurateCount -= 2;
			ent->client->ps.persistant[PERS_IMPRESSIVE_COUNT]++;
			// add the sprite over the player's head
			ent->client->ps.eFlags &= ~(EF_AWARD_RLRGCOMBO | EF_AWARD_IMPRESSIVE | EF_AWARD_EXCELLENT | EF_AWARD_GAUNTLET | EF_AWARD_ASSIST | EF_AWARD_DEFEND | EF_AWARD_CAP );
			ent->client->ps.eFlags |= EF_AWARD_IMPRESSIVE;
			ent->client->rewardTime = level.time + REWARD_SPRITE_TIME;
		}
		ent->client->accuracy_hits++;
	}

}*/
/*
======================================================================

GRAPPLING HOOK

======================================================================
*/

void Weapon_GrapplingHook_Fire(gentity_t * ent)
{
	if(Hook.integer == 1)
	{
		AngleVectors(ent->client->ps.viewangles, forward, right, up);
		CalcMuzzlePoint(ent, forward, right, up, muzzle);
		if(!ent->client->fireHeld && !ent->client->hook)
			fire_grapple(ent, muzzle, forward);

		ent->client->fireHeld = qtrue;
	}
}

void Weapon_HookFree(gentity_t * ent)
{
	ent->parent->client->hook = NULL;
	ent->parent->client->ps.pm_flags &= ~PMF_GRAPPLE_PULL;
	G_FreeEntity(ent);
}

void Weapon_HookThink(gentity_t * ent)
{
	if(ent->enemy)
	{
		vec3_t          v, oldorigin;

		VectorCopy(ent->r.currentOrigin, oldorigin);
		v[0] = ent->enemy->r.currentOrigin[0] + (ent->enemy->r.mins[0] + ent->enemy->r.maxs[0]) * 0.5;
		v[1] = ent->enemy->r.currentOrigin[1] + (ent->enemy->r.mins[1] + ent->enemy->r.maxs[1]) * 0.5;
		v[2] = ent->enemy->r.currentOrigin[2] + (ent->enemy->r.mins[2] + ent->enemy->r.maxs[2]) * 0.5;
		SnapVectorTowards(v, oldorigin);	// save net bandwidth

		G_SetOrigin(ent, v);
	}

	VectorCopy(ent->r.currentOrigin, ent->parent->client->ps.grapplePoint);
}



//======================================================================


/*
===============
LogAccuracyHit
===============
*/
qboolean LogAccuracyHit(gentity_t * target, gentity_t * attacker)
{

	if(!target->takedamage)
	{
		return qfalse;
	}

	if(target == attacker)
	{
		return qfalse;
	}

	if(!target->client)
	{
		return qfalse;
	}

	if(!attacker->client)
	{
		return qfalse;
	}

	if(target->client->ps.stats[STAT_HEALTH] <= 0)
	{
		return qfalse;
	}

	if(OnSameTeam(target, attacker))
	{
		return qfalse;
	}
	switch (attacker->s.weapon)
	{
		case WP_ROCKET_LAUNCHER:
			attacker->client->accuracy_hit[WP_ROCKET_LAUNCHER]++;
			return qtrue;
			break;
		case WP_PLASMAGUN:
			attacker->client->accuracy_hit[WP_PLASMAGUN]++;
			return qtrue;
			break;
		case WP_GRENADE_LAUNCHER:
			attacker->client->accuracy_hit[WP_GRENADE_LAUNCHER]++;
			return qtrue;
			break;
		case WP_BFG:
			attacker->client->accuracy_hit[WP_BFG]++;
			return qtrue;
			break;
		case WP_RAILGUN:
			attacker->client->accuracy_hit[WP_RAILGUN]++;
			return qtrue;
			break;
		case WP_IRAILGUN:
			attacker->client->accuracy_hit[WP_IRAILGUN]++;
			return qtrue;
			break;
		case WP_MACHINEGUN:
			attacker->client->accuracy_hit[WP_MACHINEGUN]++;
			return qtrue;
			break;
		case WP_SHOTGUN:
			attacker->client->accuracy_hit[WP_SHOTGUN]++;
			return qtrue;
			break;
		case WP_LIGHTNING:
			attacker->client->accuracy_hit[WP_LIGHTNING]++;
			return qtrue;
			break;
		default:
			break;
	}

	return qtrue;
}

/*
===============
CalcMuzzlePoint

set muzzle location relative to pivoting eye
===============
*/
void CalcMuzzlePoint(gentity_t * ent, vec3_t forward, vec3_t right, vec3_t up, vec3_t muzzlePoint)
{
	VectorCopy(ent->s.pos.trBase, muzzlePoint);
	muzzlePoint[2] += ent->client->ps.viewheight;
	VectorMA(muzzlePoint, 14, forward, muzzlePoint);
	// snap to integer coordinates for more efficient network bandwidth usage
	SnapVector(muzzlePoint);
}

/*
===============
CalcMuzzlePointOrigin

set muzzle location relative to pivoting eye
===============
*/
void CalcMuzzlePointOrigin(gentity_t * ent, vec3_t origin, vec3_t forward, vec3_t right, vec3_t up, vec3_t muzzlePoint)
{
	VectorCopy(ent->s.pos.trBase, muzzlePoint);
	muzzlePoint[2] += ent->client->ps.viewheight;
	VectorMA(muzzlePoint, 14, forward, muzzlePoint);
	// snap to integer coordinates for more efficient network bandwidth usage
	SnapVector(muzzlePoint);
}



/*
===============
FireWeapon
===============
*/
void FireWeapon(gentity_t * ent, int time)
{
	if(ent->client->ps.powerups[PW_QUAD])
	{
		s_quadFactor = g_quadfactor.value;
	}
	else
	{
		s_quadFactor = 1;
	}
#ifdef MISSIONPACK
	if(ent->client->persistantPowerup && ent->client->persistantPowerup->item &&
	   ent->client->persistantPowerup->item->giTag == PW_DOUBLER)
	{
		s_quadFactor *= 2;
	}
#endif

	// track shots taken for accuracy tracking.  Grapple is not a weapon and gauntet is just not tracked
	if(ent->s.weapon != WP_GRAPPLING_HOOK && ent->s.weapon != WP_GAUNTLET)
	{
#ifdef MISSIONPACK
		if(ent->s.weapon == WP_NAILGUN)
		{
			ent->client->accuracy_shots += NUM_NAILSHOTS;
		}
		else
		{
			ent->client->accuracy_shots++;
		}
#else
		ent->client->accuracy_shots++;
#endif
	}

	// set aiming directions
	AngleVectors(ent->client->ps.viewangles, forward, right, up);

	CalcMuzzlePointOrigin(ent, ent->client->oldOrigin, forward, right, up, muzzle);

	// fire the specific weapon
	switch (ent->s.weapon)
	{
		case WP_GAUNTLET:
			Weapon_Gauntlet(ent);
			break;
		case WP_LIGHTNING:
			Weapon_LightningFire(ent);
			ent->client->accuracy_shot[WP_LIGHTNING]++;
			break;
		case WP_SHOTGUN:
			weapon_supershotgun_fire(ent);
			ent->client->accuracy_shot[WP_SHOTGUN]++;
			break;
		case WP_MACHINEGUN:
			Bullet_Fire(ent, MACHINEGUN_SPREAD, MACHINEGUN_TEAM_DAMAGE);
			ent->client->accuracy_shot[WP_MACHINEGUN]++;
			break;
		case WP_GRENADE_LAUNCHER:
			weapon_grenadelauncher_fire(ent);
			ent->client->accuracy_shot[WP_GRENADE_LAUNCHER]++;
			break;
		case WP_ROCKET_LAUNCHER:
			Weapon_RocketLauncher_Fire(ent);
			ent->client->accuracy_shot[WP_ROCKET_LAUNCHER]++;
			break;
		case WP_PLASMAGUN:
			Weapon_Plasmagun_Fire(ent);
			ent->client->accuracy_shot[WP_PLASMAGUN]++;
			break;
		case WP_RAILGUN:
			weapon_railgun_fire(ent);
			ent->client->accuracy_shot[WP_RAILGUN]++;
			break;
		case WP_IRAILGUN:
			weapon_irailgun_fire(ent);
			ent->client->accuracy_shot[WP_IRAILGUN]++;
			break;
		case WP_BFG:
			BFG_Fire(ent);

			ent->client->accuracy_shot[WP_BFG]++;
			break;
		case WP_GRAPPLING_HOOK:
			Weapon_GrapplingHook_Fire(ent);
			break;
		case WP_FLAMETHROWER:
			// RF, this is done client-side only now
			Weapon_FlamethrowerFire(ent);
			break;
#ifdef MISSIONPACK
		case WP_CHAINGUN:
			Bullet_Fire(ent, CHAINGUN_SPREAD, MACHINEGUN_DAMAGE);
			break;
#endif
		default:
// FIXME        G_Error( "Bad ent->s.weapon" );
			break;
	}
}
