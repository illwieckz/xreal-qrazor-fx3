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

#define	MISSILE_PRESTEP_TIME	50

void            G_ExplodeMissile(gentity_t * ent);

/*
================
G_BounceMissile

================
*/
void G_BounceMissile(gentity_t * ent, trace_t * trace)
{
	vec3_t          velocity;
	float           dot;
	int             hitTime;


	// reflect the velocity on the trace plane
	hitTime = level.previousTime + (level.time - level.previousTime) * trace->fraction;
	BG_EvaluateTrajectoryDelta(&ent->s.pos, hitTime, velocity);
	dot = DotProduct(velocity, trace->plane.normal);
	VectorMA(velocity, -2 * dot, trace->plane.normal, ent->s.pos.trDelta);

	if(ent->s.eFlags & EF_BOUNCE_HALF)
	{
		VectorScale(ent->s.pos.trDelta, 0.65, ent->s.pos.trDelta);
		// check for stop
		if(trace->plane.normal[2] > 0.2 && VectorLength(ent->s.pos.trDelta) < 40)
		{
			G_SetOrigin(ent, trace->endpos);
			return;
		}
	}

	VectorAdd(ent->r.currentOrigin, trace->plane.normal, ent->r.currentOrigin);
	VectorCopy(ent->r.currentOrigin, ent->s.pos.trBase);
	ent->s.pos.trTime = level.time;
}


/*
===============
KamikazeRadiusDamage
===============
*/
static void KamikazeRadiusDamage(vec3_t origin, gentity_t * attacker, float damage, float radius)
{
	float           dist;
	gentity_t      *ent;
	int             entityList[MAX_GENTITIES];
	int             numListedEntities;
	vec3_t          mins, maxs;
	vec3_t          v;
	vec3_t          dir;
	int             i, e;

	if(radius < 1)
	{
		radius = 1;
	}
	if(radius >= 290)
	{
		radius = 220;
	}

	for(i = 0; i < 3; i++)
	{
		mins[i] = origin[i] - radius;
		maxs[i] = origin[i] + radius;
	}

	numListedEntities = trap_EntitiesInBox(mins, maxs, entityList, MAX_GENTITIES);

	for(e = 0; e < numListedEntities; e++)
	{
		ent = &g_entities[entityList[e]];

		//  if (!ent->takedamage) {
		//      continue;
		//  }

		//  // dont hit things we have already hit
		//  if( ent->kamikazeTime > level.time ) {
		//      continue;
		//  }

		// find the distance from the edge of the bounding box
		for(i = 0; i < 3; i++)
		{
			if(origin[i] < ent->r.absmin[i])
			{
				v[i] = ent->r.absmin[i] - origin[i];
			}
			else if(origin[i] > ent->r.absmax[i])
			{
				v[i] = origin[i] - ent->r.absmax[i];
			}
			else
			{
				v[i] = 0;
			}
		}

		dist = VectorLength(v);
		if(dist >= radius)
		{
			continue;
		}

//      if( CanDamage (ent, origin) ) {
		VectorSubtract(ent->r.currentOrigin, origin, dir);
		// push the center of mass higher than the origin so players
		// get knocked into the air more
		dir[2] += 24;
		G_Damage(ent, NULL, attacker, dir, origin, damage, 0, MOD_BFG_SPLASH);
		//  ent->kamikazeTime = level.time + 3000;
//      }
	}
}

/*
===============
KamikazeShockWave
===============
*/
static void KamikazeShockWave(vec3_t origin, gentity_t * attacker, float damage, float push, float radius)
{

	gentity_t      *ent;
	int             entityList[MAX_GENTITIES];
	int             numListedEntities;
	vec3_t          mins, maxs;
	float           dist;
	vec3_t          v;
	vec3_t          dir;
	int             i, e;

	if(radius < 1)
	{
		radius = 1;
	}

	if(radius >= 350)
	{
		radius = 290;
	}

	for(i = 0; i < 3; i++)
	{
		mins[i] = origin[i] - radius;
		maxs[i] = origin[i] + radius;
	}

	numListedEntities = trap_EntitiesInBox(mins, maxs, entityList, MAX_GENTITIES);

	for(e = 0; e < numListedEntities; e++)
	{
		ent = &g_entities[entityList[e]];

		// dont hit things we have already hit
		//  if( ent->kamikazeShockTime > level.time ) {
		//      continue;
		//  }

		// find the distance from the edge of the bounding box
		for(i = 0; i < 3; i++)
		{
			if(origin[i] < ent->r.absmin[i])
			{
				v[i] = ent->r.absmin[i] - origin[i];
			}
			else if(origin[i] > ent->r.absmax[i])
			{
				v[i] = origin[i] - ent->r.absmax[i];
			}
			else
			{
				v[i] = 0;
			}
		}

		dist = VectorLength(v);
		if(dist >= radius)
		{
			continue;
		}

		VectorSubtract(ent->r.currentOrigin, origin, dir);
		dir[2] += 24;
		G_Damage(ent, NULL, attacker, dir, origin, damage, 0, MOD_BFG_SPLASH);
		//
		dir[2] = 0;
		VectorNormalize(dir);
		if(ent->client)
		{
			ent->client->ps.velocity[0] = dir[0] * push;
			ent->client->ps.velocity[1] = dir[1] * push;
			ent->client->ps.velocity[2] = 50;
		}
		//  ent->kamikazeShockTime = level.time + 3000;
	}
}

/*
===============
KamikazeDamage
===============
*/
static void KamikazeDamage(gentity_t * self)
{
	int             i;
	float           t;
	gentity_t      *ent;
	vec3_t          newangles;
	float           dist;
	vec3_t          dir;
	int             quad;


	self->count += 100;

	if(self->activator->client->ps.powerups[PW_QUAD])
	{
		G_AddEvent(self->activator, EV_POWERUP_QUAD, 0);
		quad = g_quadfactor.value;
	}
	else
	{
		quad = 1;
	}

	if(self->count >= 1)
	{
		// shockwave push back
		t = self->count;
		KamikazeShockWave(self->r.currentOrigin, self->activator, 25 * quad, 200, (int)(float)t * 1200.0 / (1200 - 50));
	}
	//
	if(self->count >= 1)
	{
		// do our damage
		t = self->count;
		if(self->count < 850)
		{
			KamikazeRadiusDamage(self->r.currentOrigin, self->activator, 400 * quad, /*(int) (float) t * 2.0 /(3 - 1) */ 390);
		}
	}

	// either cycle or kill self
	if(self->count >= 1400)
	{
		G_FreeEntity(self);
		return;
	}
	self->nextthink = level.time + 100;

	// add earth quake effect
	newangles[0] = crandom() * 2;
	newangles[1] = crandom() * 2;
	newangles[2] = 0;
	for(i = 0; i < MAX_CLIENTS; i++)
	{
		ent = &g_entities[i];
		if(!ent->inuse)
			continue;
		if(!ent->client)
			continue;

		VectorSubtract(ent->r.currentOrigin, self->r.currentOrigin, dir);
		dist = VectorLength(dir);
		if(dist >= 600)
		{
			continue;
		}

		if(ent->client->ps.groundEntityNum != ENTITYNUM_NONE)
		{
			ent->client->ps.velocity[0] += crandom() * 120;
			ent->client->ps.velocity[1] += crandom() * 120;
			//  ent->client->ps.velocity[2] = 30 + random() * 25;
		}

		ent->client->ps.delta_angles[0] += ANGLE2SHORT(newangles[0] - self->movedir[0]);
		ent->client->ps.delta_angles[1] += ANGLE2SHORT(newangles[1] - self->movedir[1]);
		ent->client->ps.delta_angles[2] += ANGLE2SHORT(newangles[2] - self->movedir[2]);
	}
	VectorCopy(newangles, self->movedir);
}

/*
===============
G_StartKamikaze
===============
*/
void G_StartKamikaze(gentity_t * ent)
{
	gentity_t      *explosion;
	gentity_t      *te;
	vec3_t          snapped;

	// start up the explosion logic
	explosion = G_Spawn();

	explosion->s.eType = ET_GENERAL /*+ EV_KAMIKAZE */ ;
	explosion->eventTime = level.time;

//  if ( ent->client ) {
	VectorCopy(ent->r.currentOrigin, snapped);
//  }
//  else {
//      VectorCopy( ent->activator->s.pos.trBase, snapped );
//  }
	SnapVector(snapped);		// save network bandwidth
	G_SetOrigin(explosion, snapped);

	explosion->classname = "bfg";
	explosion->s.pos.trType = TR_STATIONARY;

	explosion->kamikazeTime = level.time;
	explosion->parent = ent->parent;
	explosion->think = KamikazeDamage;
	explosion->nextthink = level.time + 100;
	explosion->count = 0;
	VectorClear(explosion->movedir);

	trap_LinkEntity(explosion);
	explosion->activator = ent->activator;


	// play global sound at all clients
	te = G_TempEntity(snapped, EV_GLOBAL_TEAM_SOUND);
	te->r.svFlags |= SVF_BROADCAST;
	te->s.eventParm = GTS_KAMIKAZE;
}

/*
================
G_ExplodeMissile

Explode a missile without an impact
================
*/
void G_ExplodeMissile(gentity_t * ent)
{
	vec3_t          dir;
	vec3_t          origin;
	gentity_t      *me;

	me = &g_entities[ent->r.ownerNum];

	BG_EvaluateTrajectory(&ent->s.pos, level.time, origin);
	SnapVector(origin);
	G_SetOrigin(ent, origin);

	// we don't have a valid direction, so just point straight up
	dir[0] = dir[1] = 0;
	dir[2] = 1;

	ent->s.eType = ET_GENERAL;
	G_AddEvent(ent, EV_MISSILE_MISS, DirToByte(dir));

	ent->freeAfterEvent = qtrue;

	// splash damage
	if(ent->splashDamage)
	{
		if(G_RadiusDamage(ent->r.currentOrigin, ent->parent, ent->splashDamage, ent->splashRadius, ent, ent->splashMethodOfDeath))
		{
			g_entities[ent->r.ownerNum].client->accuracy_hits++;
		}
	}

	trap_LinkEntity(ent);
}

/*
================
G_MissileImpact
================
*/
void G_MissileImpact(gentity_t * ent, trace_t * trace, vec3_t dir)
{
	gentity_t      *other;
	qboolean        hitClient = qfalse;

	other = &g_entities[trace->entityNum];
	// check for bounce
	if(!other->takedamage && (ent->s.eFlags & (EF_BOUNCE | EF_BOUNCE_HALF)))
	{
		vec3_t          velocity2;

		G_BounceMissile(ent, trace);
		G_AddEvent(ent, EV_GRENADE_BOUNCE, 0);
		BG_EvaluateTrajectoryDelta(&ent->s.pos, level.time, velocity2);

		if(VectorLength(velocity2) > 150)
		{
			ent->s.generic1 |= GNF_GRENSPARKS;
		}
		else
		{
			ent->s.generic1 &= ~GNF_GRENSPARKS;
		}
		VectorCopy(trace->plane.normal, ent->s.angles);
		return;
	}

	// impact damage

	if(!strcmp(ent->classname, "bfg"))
	{
		gentity_t      *te3;

		te3 = G_Spawn();

		VectorCopy(ent->s.pos.trBase, te3->s.pos.trBase);
		G_StartKamikaze(ent);
		te3->activator = ent->activator;
		te3->think = G_FreeEntity;
		te3->nextthink = level.time + 100;

		if(other->client)
		{
			G_AddEvent(ent, EV_MISSILE_HIT, DirToByte(dir));
			ent->s.otherEntityNum = other->s.number;
		}
		else if(trace->surfaceFlags & SURF_METALSTEPS)
		{
			G_AddEvent(ent, EV_MISSILE_MISS_METAL, DirToByte(trace->plane.normal));
			ent->s.clientNum = ent->r.ownerNum;
		}
		else
		{
			G_AddEvent(ent, EV_MISSILE_MISS, DirToByte(trace->plane.normal));
			ent->s.clientNum = ent->r.ownerNum;
		}
		ent->freeAfterEvent = qtrue;

		trap_LinkEntity(ent);

		return;
	}

	if(other->takedamage)
	{


		// FIXME: wrong damage direction?
		if(ent->damage)
		{
			vec3_t          velocity;

			if(LogAccuracyHit(other, &g_entities[ent->r.ownerNum]))
			{
				g_entities[ent->r.ownerNum].client->accuracy_hits++;
				hitClient = qtrue;
			}

			BG_EvaluateTrajectoryDelta(&ent->s.pos, level.time, velocity);
			if(VectorLength(velocity) == 0)
			{
				velocity[2] = 1;	// stepped on a grenade
			}

			G_Damage(other, ent, &g_entities[ent->r.ownerNum], ent->s.pos.trDelta,
					 ent->s.origin, ent->damage, 0, ent->methodOfDeath);
		}

	}

	if(!strcmp(ent->classname, "hook"))
	{
		gentity_t      *nent;
		vec3_t          v;

		nent = G_Spawn();
		if(other->takedamage && other->client)
		{
			G_AddEvent(nent, EV_MISSILE_HIT, DirToByte(dir));
			nent->s.otherEntityNum = other->s.number;

			ent->enemy = other;

			v[0] = other->r.currentOrigin[0] + (other->r.mins[0] + other->r.maxs[0]) * 0.5;
			v[1] = other->r.currentOrigin[1] + (other->r.mins[1] + other->r.maxs[1]) * 0.5;
			v[2] = other->r.currentOrigin[2] + (other->r.mins[2] + other->r.maxs[2]) * 0.5;

			SnapVectorTowards(v, ent->s.pos.trBase);	// save net bandwidth
		}
		else
		{
			VectorCopy(trace->endpos, v);
			G_AddEvent(nent, EV_MISSILE_MISS, DirToByte(trace->plane.normal));
			ent->enemy = NULL;
			nent->s.clientNum = ent->r.ownerNum;
		}

		SnapVectorTowards(v, ent->s.pos.trBase);	// save net bandwidth

		nent->freeAfterEvent = qtrue;
		// change over to a normal entity right at the point of impact
		nent->s.eType = ET_GENERAL;
		ent->s.eType = ET_GRAPPLE;

		G_SetOrigin(ent, v);
		G_SetOrigin(nent, v);

		ent->think = Weapon_HookThink;
		ent->nextthink = level.time + FRAMETIME;

		ent->parent->client->ps.pm_flags |= PMF_GRAPPLE_PULL;
		VectorCopy(ent->r.currentOrigin, ent->parent->client->ps.grapplePoint);

		trap_LinkEntity(ent);
		trap_LinkEntity(nent);

		return;
	}

	// is it cheaper in bandwidth to just remove this ent and create a new
	// one, rather than changing the missile into the explosion?

	if(other->client || other->s.eFlags & EF_DEAD)
	{
		G_AddEvent(ent, EV_MISSILE_HIT, DirToByte(dir));
		ent->s.otherEntityNum = other->s.number;
	}
	else if(trace->surfaceFlags & SURF_METALSTEPS)
	{
		G_AddEvent(ent, EV_MISSILE_MISS_METAL, DirToByte(trace->plane.normal));
		ent->s.clientNum = ent->r.ownerNum;
	}
	else
	{
		G_AddEvent(ent, EV_MISSILE_MISS, DirToByte(trace->plane.normal));
		ent->s.clientNum = ent->r.ownerNum;

	}


	ent->freeAfterEvent = qtrue;

	// change over to a normal entity right at the point of impact
	ent->s.eType = ET_GENERAL;

	SnapVectorTowards(trace->endpos, ent->s.pos.trBase);	// save net bandwidth

	G_SetOrigin(ent, trace->endpos);

	// splash damage (doesn't apply to person directly hit)
	if(ent->splashDamage)
	{
		if(G_RadiusDamage(trace->endpos, ent->parent, ent->splashDamage, ent->splashRadius, other, ent->splashMethodOfDeath))
		{
			if(!hitClient)
			{
				g_entities[ent->r.ownerNum].client->accuracy_hits++;
			}

		}

	}
	trap_LinkEntity(ent);
}

/*
================
G_RunMissile
================
*/
void G_RunMissile(gentity_t * ent)
{
	vec3_t          origin;
	trace_t         tr;
	gentity_t      *me;
	int             passent;

//  float   dir[3];
//  vec3_t      forward, right, up;
//  vec3_t      angles,vel;

	me = &g_entities[ent->r.ownerNum];

	// get current position
	BG_EvaluateTrajectory(&ent->s.pos, level.time, origin);

	// if this missile bounced off an invulnerability sphere
	if(ent->target_ent)
	{
		passent = ent->target_ent->s.number;
	}

	else
	{
		// ignore interactions with the missile owner
		passent = ent->r.ownerNum;
	}
	// trace a line from the previous position to the current position
	trap_Trace(&tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, passent, ent->clipmask);

	if(tr.startsolid || tr.allsolid)
	{
		// make sure the tr.entityNum is set to the entity we're stuck in
		trap_Trace(&tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, ent->r.currentOrigin, passent, ent->clipmask);
		tr.fraction = 0;
	}
	else
	{
		VectorCopy(tr.endpos, ent->r.currentOrigin);
	}

	trap_LinkEntity(ent);


//  client = me->client;

	if(tr.fraction != 1)
	{
		// never explode or bounce on sky
		if(tr.surfaceFlags & SURF_NOIMPACT)
		{
			// If grapple, reset owner
			if(ent->parent && ent->parent->client && ent->parent->client->hook == ent)
			{
				ent->parent->client->hook = NULL;
				ent->parent->client->hookhasbeenfired = qfalse;
				ent->parent->client->fireHeld = qfalse;
			}
			G_FreeEntity(ent);
			return;
		}


//  VectorCopy(ent->s.angles, dir);
//  VectorNormalize(dir);


		G_MissileImpact(ent, &tr, ent->s.pos.trDelta);


		if(ent->s.eType != ET_MISSILE)
		{
			return;				// exploded
		}
	}

	// check think function after bouncing
	G_RunThink(ent);
}

//=============================================================================

#define	FLAME_START_SIZE		1.0
#define	FLAME_START_MAX_SIZE	100.0	// when the flame is spawned, it should endevour to reach this size
#define	FLAME_START_SPEED		1200.0	// speed of flame as it leaves the nozzle
#define	FLAME_MIN_SPEED			60.0

// these are calculated (don't change)
#define	FLAME_LENGTH			(FLAMETHROWER_RANGE + 50.0)	// NOTE: only modify the range, since this should always reflect that range

#define	FLAME_LIFETIME			(int)((FLAME_LENGTH/FLAME_START_SPEED)*1000)	// life duration in milliseconds
#define	FLAME_FRICTION_PER_SEC	(2.0f*FLAME_START_SPEED)
#define	GET_FLAME_SIZE_SPEED(x)	(((float)x / FLAME_LIFETIME) / 0.3)	// x is the current sizeMax

#define	FLAME_THRESHOLD	50

void G_FlameDamage(gentity_t * self)
{
	gentity_t      *body;
	int             entityList[MAX_GENTITIES];
	int             i, e, numListedEntities;
	float           radius, boxradius, dist;
	vec3_t          mins, maxs, point, v;
	trace_t         tr;
	gentity_t      *attacker;


	attacker = &g_entities[self->r.ownerNum];
	radius = self->speed;
	boxradius = 1.41421356 * radius;	// radius * sqrt(2) for bounding box enlargement

	for(i = 0; i < 3; i++)
	{
		mins[i] = self->r.currentOrigin[i] - boxradius;
		maxs[i] = self->r.currentOrigin[i] + boxradius;
	}

	numListedEntities = trap_EntitiesInBox(mins, maxs, entityList, MAX_GENTITIES);

	for(e = 0; e < numListedEntities; e++)
	{
		body = &g_entities[entityList[e]];

		if(!body->takedamage)
			continue;

		if(body->client)
		{
			if(!(g_friendlyFire.integer) && OnSameTeam(body, self->parent) && body->client->ps.clientNum != self->r.ownerNum)
				continue;
		}
		if(body->waterlevel >= 3)
		{
			body->flameQuota = 0;
			continue;
		}
		if(!body->r.bmodel)
		{
			VectorCopy(body->r.currentOrigin, point);
			if(body->client)
				point[2] += body->client->ps.viewheight;
			VectorSubtract(point, self->r.currentOrigin, v);
		}
		else
		{
			for(i = 0; i < 3; i++)
			{
				if(self->s.origin[i] < body->r.absmin[i])
				{
					v[i] = body->r.absmin[i] - self->r.currentOrigin[i];
				}
				else if(self->r.currentOrigin[i] > body->r.absmax[i])
				{
					v[i] = self->r.currentOrigin[i] - body->r.absmax[i];
				}
				else
				{
					v[i] = 0;
				}
			}
		}

		dist = VectorLength(v);

		// The person who shot the flame only burns when within 1/2 the radius
		if(body->s.number == self->r.ownerNum && dist >= (radius * 0.5))
			continue;
		if(dist >= radius)
			continue;

		// Non-clients that take damage get damaged here
		if(!body->client)
		{
			if(body->health > 0)
				G_Damage(body, self->parent, self->parent, vec3_origin, self->r.currentOrigin, 40, 0, MOD_FLAMETHROWER);
			continue;
		}

		// JPW NERVE -- do a trace to see if there's a wall btwn. body & flame centroid -- prevents damage through walls
		trap_Trace(&tr, self->r.currentOrigin, NULL, NULL, point, body->s.number, MASK_SHOT);
		if(tr.fraction < 1.0)
			continue;

		// now check the damageQuota to see if we should play a pain animation
		// first reduce the current damageQuota with time
		if(body->flameQuotaTime && body->flameQuota > 0)
		{
			body->flameQuota -= (int)(((float)(level.time - body->flameQuotaTime) / 1000) * 2.5f);
			if(body->flameQuota < 0)
				body->flameQuota = 0;
		}
		G_BurnMeGood(attacker, body);
	}
}

static vec3_t   muzzle2;

void G_RunFlamechunk(gentity_t * ent)
{
	vec3_t          vel, add;
	vec3_t          neworg;
	trace_t         tr;
	float           speed, dot;

//  trace_t     tr2;
//  vec3_t      end;
//  gentity_t   *traceEnt2;
//  int          i;



	// Adust the current speed of the chunk
	if(level.time - ent->timestamp > 50)
	{
		VectorCopy(ent->s.pos.trDelta, vel);
		speed = VectorNormalize(vel);
		speed -= (50.f / 1000.f) * FLAME_FRICTION_PER_SEC;

		if(speed < FLAME_MIN_SPEED)
			speed = FLAME_MIN_SPEED;

		VectorScale(vel, speed, ent->s.pos.trDelta);
	}
	else
		speed = FLAME_START_SPEED;

	// Move the chunk
	VectorScale(ent->s.pos.trDelta, 50.f / 1000.f, add);
	VectorAdd(ent->r.currentOrigin, add, neworg);

	trap_Trace(&tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, neworg, ent->r.ownerNum, MASK_FLAME);	// JPW NERVE

	if(tr.startsolid)
		VectorCopy(vec3_origin, ent->s.pos.trDelta);
	else if(tr.fraction != 1.0f && !(tr.surfaceFlags & SURF_NOIMPACT))
	{
		VectorCopy(tr.endpos, ent->r.currentOrigin);

		dot = DotProduct(vel, tr.plane.normal);
		VectorMA(vel, -2 * dot, tr.plane.normal, vel);
		VectorNormalize(vel);
		speed *= 0.5 * (0.25 + 0.75 * ((dot + 1.0) * 0.5));
		VectorScale(vel, speed, ent->s.pos.trDelta);
	}
	else
		VectorCopy(neworg, ent->r.currentOrigin);

	// Do damage to nearby entities, every 100ms
	if(ent->flameQuotaTime <= level.time)
	{
		ent->flameQuotaTime = level.time + 100;
		G_FlameDamage(ent);
	}

	// Adjust the size
	if(ent->speed < FLAME_START_MAX_SIZE)
	{
		ent->speed += 10.f;

		if(ent->speed > FLAME_START_MAX_SIZE)
			ent->speed = FLAME_START_MAX_SIZE;
	}

	// Remove after 2 seconds
//	if(level.time - ent->timestamp > (FLAME_LIFETIME - 350))
//	{							// JPW NERVE increased to 350 from 250 to match visuals better
//		G_FreeEntity(ent);
		return;
//	}
	/*
	   if ( !tr.startsolid ){
	   for (i = 0; i < 2; i++) {
	   trap_Trace( &tr2, muzzle2, ent->r.mins, ent->r.maxs, end, ent->r.ownerNum, MASK_SHOT );
	   traceEnt2 = &g_entities[ tr2.entityNum ];
	   if(tr2.contents & CONTENTS_SOLID){
	   break;
	   }

	   if ( traceEnt2->takedamage) {
	   G_FlameDamage( traceEnt2 );
	   return;
	   }
	   }
	   } */
	G_RunThink(ent);

}

/*
=================
fire_flamechunk
=================
*/
gentity_t      *fire_flamechunk(gentity_t * self, vec3_t start, vec3_t dir)
{
	gentity_t      *bolt;



	// Only spawn every other frame
//  if ( self->count2 ) {
//      self->count2--;
//      return NULL;
//  }

//  self->count2 = 1;
	VectorNormalize(dir);

	bolt = G_Spawn();
	bolt->classname = "flamechunk";

	bolt->timestamp = level.time;
	bolt->flameQuotaTime = level.time + 50;
	bolt->s.eType = ET_FLAMETHROWER_CHUNK;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN | SVF_NOCLIENT;
	bolt->s.weapon = self->s.weapon;
	bolt->r.ownerNum = self->s.number;
	bolt->s.clientNum = self->s.number;
	bolt->parent = self;
	bolt->methodOfDeath = MOD_FLAMETHROWER;
	bolt->clipmask = MASK_FLAME;

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;	// move a bit on the very first frame
	bolt->s.pos.trDuration = 800;

	// 'speed' will be the current size radius of the chunk
	bolt->speed = FLAME_START_SIZE;
	VectorSet(bolt->r.mins, -4, -4, -4);
	VectorSet(bolt->r.maxs, 4, 4, 4);
	VectorCopy(start, bolt->s.pos.trBase);
	VectorScale(dir, FLAME_START_SPEED, bolt->s.pos.trDelta);

	SnapVector(bolt->s.pos.trDelta);	// save net bandwidth
	VectorCopy(start, bolt->r.currentOrigin);

	return bolt;
}

/*
=================
fire_plasma

=================
*/
gentity_t      *fire_plasma(gentity_t * self, vec3_t start, vec3_t dir)
{
	gentity_t      *bolt;
	int             speed;
	int             contents;


	contents = trap_PointContents(start, -1);

	speed = 1900;

	VectorNormalize(dir);

	bolt = G_Spawn();
	bolt->classname = "plasma";
	bolt->nextthink = level.time + 10000;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_PLASMAGUN;
	bolt->r.ownerNum = self->s.number;
	bolt->s.clientNum = self->s.number;
	bolt->s.powerups = self->s.powerups;
	bolt->parent = self;
	bolt->damage = 12;
	bolt->splashDamage = 12;
	bolt->splashRadius = 20;
	bolt->methodOfDeath = MOD_PLASMA;
	bolt->splashMethodOfDeath = MOD_PLASMA_SPLASH;
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = NULL;
	VectorCopy(dir, bolt->s.angles);

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time - 30;	// move a bit on the very first frame
	VectorCopy(start, bolt->s.pos.trBase);
	VectorScale(dir, speed, bolt->s.pos.trDelta);

	if(contents & CONTENTS_WATER)
	{
		bolt->s.generic1 |= GNF_INWATER;
	}
	else if(!(contents & CONTENTS_WATER))
	{
		bolt->s.generic1 |= GNF_OUTWATER;
	}

	SnapVector(bolt->s.pos.trDelta);	// save net bandwidth

	VectorCopy(start, bolt->r.currentOrigin);

	return bolt;
}

//=============================================================================


/*
=================
fire_grenade
=================
*/
gentity_t      *fire_grenade(gentity_t * self, vec3_t start, vec3_t dir)
{
	gentity_t      *bolt;
	int             a;
	vec3_t          mins = { -3, -3, -3 };
	vec3_t          maxs = { 3, 3, 3 };
	int             contents;


	contents = trap_PointContents(start, -1);

	VectorNormalize(dir);

	bolt = G_Spawn();
	bolt->classname = "grenade";
	bolt->nextthink = level.time + 2500;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_GRENADE_LAUNCHER;
	bolt->s.eFlags = EF_BOUNCE_HALF;
	bolt->r.ownerNum = self->s.number;
	bolt->s.clientNum = self->s.number;
	bolt->s.powerups = self->s.powerups;
	bolt->parent = self;
	bolt->damage = 140;
	bolt->splashDamage = 80;
	bolt->splashRadius = 150;
	bolt->methodOfDeath = MOD_GRENADE;
	bolt->splashMethodOfDeath = MOD_GRENADE_SPLASH;
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = NULL;
	VectorCopy(mins, bolt->r.mins);
	VectorCopy(maxs, bolt->r.maxs);
	VectorCopy(dir, bolt->s.angles);

	bolt->s.pos.trType = TR_GRAVITY;
	bolt->s.pos.trTime = level.time - 19;	// move a bit on the very first frame
	VectorCopy(start, bolt->s.pos.trBase);
	a = 700 + self->client->ps.stats[STAT_GREN_VELOC];
	VectorScale(dir, a, bolt->s.pos.trDelta);

	if(contents & CONTENTS_WATER)
	{
		bolt->s.generic1 |= GNF_INWATER;
	}
	else if(!(contents & CONTENTS_WATER))
	{
		bolt->s.generic1 |= GNF_OUTWATER;
	}

	SnapVector(bolt->s.pos.trDelta);	// save net bandwidth
	VectorCopy(start, bolt->r.currentOrigin);

	return bolt;
}

//=============================================================================


/*
=================
fire_bfg
=================
*/
gentity_t      *fire_bfg(gentity_t * self, vec3_t start, vec3_t dir)
{
	gentity_t      *bolt;
	int             speed;
	int             contents;


	contents = trap_PointContents(start, -1);

        speed = 2000;

	VectorNormalize(dir);

	bolt = G_Spawn();
	bolt->classname = "bfg";
	bolt->nextthink = level.time + 10000;
	bolt->think = G_StartKamikaze;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_BFG;
	bolt->r.ownerNum = self->s.number;
	bolt->s.clientNum = self->s.number;
	bolt->s.powerups = self->s.powerups;
	bolt->activator = self;
	bolt->parent = self;
//  bolt->damage = 500;
//  bolt->splashDamage = 500;
//  bolt->splashRadius = 350;
	bolt->methodOfDeath = MOD_BFG;
	bolt->splashMethodOfDeath = MOD_BFG_SPLASH;
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = NULL;

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;	// move a bit on the very first frame
	VectorCopy(start, bolt->s.pos.trBase);
	VectorScale(dir, speed, bolt->s.pos.trDelta);

	if(contents & CONTENTS_WATER)
	{
		bolt->s.generic1 |= GNF_INWATER;
	}
	else if(!(contents & CONTENTS_WATER))
	{
		bolt->s.generic1 |= GNF_OUTWATER;
	}

	SnapVector(bolt->s.pos.trDelta);	// save net bandwidth
	VectorCopy(start, bolt->r.currentOrigin);

	return bolt;
}

//=============================================================================


/*
=================
fire_rocket
=================
*/
gentity_t      *fire_rocket(gentity_t * self, vec3_t start, vec3_t dir)
{
	gentity_t      *bolt;
	int             contents;


	contents = trap_PointContents(start, -1);

	VectorNormalize(dir);

	bolt = G_Spawn();
	bolt->classname = "rocket";
	bolt->nextthink = level.time + 15000;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_ROCKET_LAUNCHER;
	bolt->r.ownerNum = self->s.number;
	bolt->s.clientNum = self->s.number;
	bolt->s.powerups = self->s.powerups;
	bolt->parent = self;


	if(Instagib.integer == 0)
	{
		bolt->damage = 100;
		bolt->splashDamage = 100;
		bolt->splashRadius = 125;
	}

	if(Instagib.integer == 1)
	{
		bolt->damage = 1000;
		bolt->splashDamage = 80;
		bolt->splashRadius = 120;
	}

	bolt->methodOfDeath = MOD_ROCKET;
	bolt->splashMethodOfDeath = MOD_ROCKET_SPLASH;
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = NULL;
	if(Instagib.integer == 0)
	{
		VectorScale(dir, 900, bolt->s.pos.trDelta);
	}

	if(Instagib.integer == 1)
	{
		VectorScale(dir, 1200, bolt->s.pos.trDelta);
	}

	bolt->s.pos.trType = TR_LINEAR;

	bolt->s.pos.trTime = level.time /*- 30*/ ;	// move a bit on the very first frame

	VectorCopy(start, bolt->s.pos.trBase);

	if(contents & CONTENTS_WATER)
	{
		bolt->s.generic1 |= GNF_INWATER;
	}
	else if(!(contents & CONTENTS_WATER))
	{
		bolt->s.generic1 |= GNF_OUTWATER;
	}

	SnapVector(bolt->s.pos.trDelta);	// save net bandwidth
	VectorCopy(start, bolt->r.currentOrigin);

	return bolt;
}

/*
=================
fire_grapple
=================
*/
gentity_t      *fire_grapple(gentity_t * self, vec3_t start, vec3_t dir)
{
	gentity_t      *hook;
	float           x;

	int             hooktime;

	x = HookFireSpeed.integer;
	VectorNormalize(dir);

	hook = G_Spawn();
	hook->classname = "hook";
	hook->nextthink = level.time + 10000;
	hook->think = Weapon_HookFree;
	hook->s.eType = ET_MISSILE;
	hook->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	hook->s.weapon = WP_GRAPPLING_HOOK;
	hook->r.ownerNum = self->s.number;
	hook->methodOfDeath = MOD_GRAPPLE;
	hook->clipmask = MASK_SHOT;
	hook->parent = self;
	hook->target_ent = NULL;


	hook->s.pos.trType = TR_LINEAR;

	hook->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame

	hook->s.otherEntityNum = self->s.number;	// use to match beam in client
	VectorCopy(start, hook->s.pos.trBase);
	VectorScale(dir, x, hook->s.pos.trDelta);
	SnapVector(hook->s.pos.trDelta);	// save net bandwidth
	VectorCopy(start, hook->r.currentOrigin);

	self->client->hook = hook;

	return hook;
}
