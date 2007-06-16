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

// cg_localents.c -- every frame, generate renderer commands for locally
// processed entities, like smoke puffs, gibs, shells, etc.

#include "cg_local.h"

static char *shaderAnimNamesw[MAX_SHADER_ANIMS] = {
	"explode1",
	"wave1",
	"rlexplosion1",
	"smokegry1",
	"zsmoke",
	NULL
};
static qhandle_t shaderAnimsw[MAX_SHADER_ANIMS][MAX_SHADER_ANIM_FRAMES];
static int	shaderAnimCountsw[MAX_SHADER_ANIMS] = {
	23,
	15,
	56,
	7,
	41
};
static float	shaderAnimSTRatiow[MAX_SHADER_ANIMS] = {
	1.0f,
	1.9f,
	1.0f,
	1.0f,
	1.0f
};
static int	numShaderAnimsw;

/*
===================
CG_InitLocalEntities

This is called at startup and for tournement restarts
===================
*/
void	CG_InitLocalEntities( void ) {
	int		i;

	memset( cg_localEntities, 0, sizeof( cg_localEntities ) );
	cg_activeLocalEntities.next = &cg_activeLocalEntities;
	cg_activeLocalEntities.prev = &cg_activeLocalEntities;
	cg_freeLocalEntities = cg_localEntities;
	for ( i = 0 ; i < MAX_LOCAL_ENTITIES - 1 ; i++ ) {
		cg_localEntities[i].next = &cg_localEntities[i+1];
	}
	for (i=0; shaderAnimNamesw[i]; i++) {
		int j;

		for (j=0; j<shaderAnimCountsw[i]; j++) {
			shaderAnimsw[i][j] = trap_R_RegisterShader( va("%s%i", shaderAnimNamesw[i], j+1) );
		}
	}
	numShaderAnimsw = i;
}


/*
==================
CG_FreeLocalEntity
==================
*/
void CG_FreeLocalEntity( localEntity_t *le ) {
	if ( !le->prev ) {
		CG_Error( "CG_FreeLocalEntity: not active" );
	}

	// remove from the doubly linked active list
	le->prev->next = le->next;
	le->next->prev = le->prev;

	// the free list is only singly linked
	le->next = cg_freeLocalEntities;
	cg_freeLocalEntities = le;
}

/*
===================
CG_AllocLocalEntity

Will allways succeed, even if it requires freeing an old active entity
===================
*/
localEntity_t	*CG_AllocLocalEntity( void ) {
	localEntity_t	*le;

	if ( !cg_freeLocalEntities ) {
		// no free entities, so free the one at the end of the chain
		// remove the oldest active entity
		if(!(cg_activeLocalEntities.prev->leFlags & LEF_NEVER_FREE)){
			CG_FreeLocalEntity( cg_activeLocalEntities.prev );
		}
	}

	le = cg_freeLocalEntities;
	cg_freeLocalEntities = cg_freeLocalEntities->next;

	memset( le, 0, sizeof( *le ) );

	// link into the active list
	le->next = cg_activeLocalEntities.next;
	le->prev = &cg_activeLocalEntities;
	cg_activeLocalEntities.next->prev = le;
	cg_activeLocalEntities.next = le;
	le->splashTime = cg.snap->serverTime;
	return le;
}


/*
====================================================================================

FRAGMENT PROCESSING

A fragment localentity interacts with the environment in some way (hitting walls),
or generates more localentities along a trail.

====================================================================================
*/

// The following code is brought over from the server so 
// localents can recieve knockback from explosions.
// This is a small effort to add some reality to movements of
// localents
/*
============
LocalEnt_Damage
Figures out how much damage and or knockback a localent should recieve.
============
static void LocalEnt_Damage( localEntity_t *targ, localEntity_t *attacker,
			   vec3_t dir, vec3_t point, int damage) {
	int			knockback;
		vec3_t	kvel;
		float	mass;

//	if ( targ == attacker) {
//		return;
//	}

//	VectorNormalize(dir);

	knockback = damage;
	if ( knockback > 200 ) {
		knockback = 200;
	}
	
	// figure momentum add, even if the damage won't be taken
//	if ( knockback ) {

//	dot = DotProduct( velocity, trace->plane.normal);
//	VectorMA( velocity, -2*dot, trace->plane.normal, le->pos.trDelta);

//	VectorScale( cg_activeLocalEntities.pos.trDelta, kvel, lcg_activeLocalEntities.pos.trDelta );


		targ->Take_Damage = qtrue;
		mass = 200;

		VectorScale (dir, 1000 * (float)knockback / mass, kvel);

		VectorAdd (cg_activeLocalEntities.pos.trDelta,kvel, cg_activeLocalEntities.pos.trDelta);
	//	VectorCopy(kvel,targ->pos.trDelta);
	//	VectorScale (targ->pos.trDelta, 1000 * (float)knockback / mass, targ->pos.trDelta);
		
	//	cg_activeLocalEntities.pos.trTime = cg.time;
//	}
}*/
/*
============
CG_CanDamage
Returns qtrue if the inflictor can directly damage the target.  Used for
explosions.
============
qboolean CG_CanDamage (localEntity_t *targ, vec3_t origin) {
	vec3_t	dest;
	trace_t	tr;
	vec3_t	midpoint;

	// use the midpoint of the bounds instead of the origin, because
	// bmodels may have their origin is 0,0,0
	VectorAdd (targ->mins, targ->maxs, midpoint);
	VectorScale (midpoint, 0.5, midpoint);

	VectorCopy (midpoint, dest);
	CG_Trace( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID );
	if (tr.fraction == 1.0 || tr.entityNum == targ->entNumber)
		return qtrue;

	// this should probably check in the plane of projection, 
	// rather than in world coordinate, and also include Z
	VectorCopy (midpoint, dest);
	dest[0] += 15.0;
	dest[1] += 15.0;
	CG_Trace( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID );
	if (tr.fraction == 1.0)
		return qtrue;

	VectorCopy (midpoint, dest);
	dest[0] += 15.0;
	dest[1] -= 15.0;
	CG_Trace( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID );
	if (tr.fraction == 1.0)
		return qtrue;

	VectorCopy (midpoint, dest);
	dest[0] -= 15.0;
	dest[1] += 15.0;
	CG_Trace( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID );
	if (tr.fraction == 1.0)
		return qtrue;

	VectorCopy (midpoint, dest);
	dest[0] -= 15.0;
	dest[1] -= 15.0;
	CG_Trace( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID );

	if (tr.fraction == 1.0)
		return qtrue;


	return qfalse;
}
*/
/*
============
CG_RadiusDamage
============
static void CG_RadiusDamage ( vec3_t origin, localEntity_t *attacker, float damage, float radius,
					 localEntity_t *ignore) {
////	float		points, dist;
//	localEntity_t	*ent;
//	int			entityList[MAX_LOCAL_ENTITIES];
//	int			numListedEntities;
	vec3_t		mins, maxs;
//	vec3_t		v;
//	vec3_t		dir;
	int			i;//, e;
	clipHandle_t 	cmodel;
	qboolean	hitEntity = qfalse;
//	trace_t	tr;

	if ( radius < 1 ) {
		radius = 1;
	}

	for ( i = 0 ; i < 3 ; i++ ) {
		mins[i] = origin[i] - radius;
		maxs[i] = origin[i] + radius;
	}
	cmodel = trap_CM_TempBoxModel( mins, maxs );
//	trap_CM_TransformedBoxTrace ( &trace, origin, origin,
//			mins, maxs, cmodel, CONTENTS_LOCALENT, origin, vec3_origin);

//	CG_Trace( &tr, origin, mins, maxs, origin, ENTITYNUM_NONE, MASK_SOLID );

	numListedEntities = CG_ClipMoveToLocalEntities ( origin,mins, maxs, origin,attacker->entNumber, MASK_ALL, &tr );//trap_EntitiesInBox( mins, maxs, entityList, MAX_LOCAL_ENTITIES );

	for ( e = 0 ; e < MAX_LOCAL_ENTITIES ; e++ ) {
		ent = &cg_localEntities[MAX_LOCAL_ENTITIES];

	//	if (!ent->takedamage)
		//	continue;

		// find the distance from the edge of the bounding box
		for ( i = 0 ; i < 3 ; i++ ) {
			if ( origin[i] < ent->mins[i] ) {
				v[i] = ent->mins[i] - origin[i];
			} else if ( origin[i] > ent->maxs[i] ) {
				v[i] = origin[i] - ent->maxs[i];
			} else {
				v[i] = 0;
			}
		}

		dist = VectorLength( v );
		if ( dist >= radius ) {
			continue;
		}

		points = damage * ( 1.0 - dist / radius );

	//	if( CG_CanDamage (ent, origin) ) {
			hitEntity = qtrue;
			VectorSubtract (ent->refEntity.origin, origin, dir);
			// push the center of mass higher than the origin so players
			// get knocked into the air more
			dir[2] += 24;
			LocalEnt_Damage (ent, attacker, dir, origin, (int)points);
	//	}
	}

//	return hitEntity;
}
*/
/*
================
CG_BloodTrail
Leave expanding blood puffs behind gibs
================
*/
static void CG_BloodTrail( localEntity_t *le ,vec3_t vel) {
	int		t;
	int		t2;
	int		step;
	vec3_t	newOrigin;
	int i;
	qhandle_t hshader;

	step = 40;
	t = step * ( (cg.time - cg.frametime + step ) / step );
	t2 = step * ( cg.time / step );

	for ( ; t <= t2; t += step ) {
		BG_EvaluateTrajectory( &le->pos, t, newOrigin );
		i = rand()%1;
		if(i < 0){
			i = 0;
		}
		if(i > 1){
			i = 1;
		}

		if(i == 0){
			hshader = cgs.media.zbloodSpray1Shader;
		}else if ( i == 1) {
			hshader = cgs.media.zbloodSpray2Shader;
		}

		CG_Particle_Bleed (hshader
			, newOrigin
			, vec3_origin
			, 0
			, 1000+rand()%400
			, 7,17+rand()%10);
	}
}
/***************************************************
CG_FragmentSplash
Adds water splash effects.
***************************************************/
static void CG_FragmentSplash( localEntity_t *le ) {
	vec3_t	origin,lastPos;
	int		contents;
	int		lastContents;
	vec3_t	start2,dir,end,rotate_ang,axis[3];
	trace_t	trace2;

	if(	le->pos.trType == TR_STATIONARY){
		return;
	}

	if ( le->leBounceSoundType == LEBS_BRASS || le->leBounceSoundType == LEBS_SHELLS ) {
		return;
	}

	BG_EvaluateTrajectory( &le->pos, cg.time, origin );
	contents = CG_PointContents( origin, -1 );

	BG_EvaluateTrajectory( &le->pos, le->splashTime, lastPos );
	lastContents = CG_PointContents( lastPos, -1 );
	le->splashTime = cg.time;

	if ( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) {

		if ( contents & lastContents & CONTENTS_WATER ) {
			if(le->inwater && le->watertraced && cg.time > le->startTime + 5){

				if ( VectorNormalize2( le->pos.trDelta, axis[0] ) == 0 ) {
					axis[0][2] = 0;
				}

				vectoangles( axis[1], rotate_ang );
				AngleVectors ( rotate_ang, dir, NULL, NULL);

				VectorMA( lastPos, -4, dir, start2 );	// back off a little so it doesn't start in solid
				VectorMA( lastPos, 4, dir, end );

				trap_CM_BoxTrace( &trace2, start2, end, NULL, NULL, 0, MASK_WATER );
				if(trace2.fraction != 1.0 /*&& !trace2.startsolid*/){
						CG_AddBulletSplashParticles( lastPos, trace2.plane.normal,
												230,	// speed
												700,	// duration
												3,	// count
												0.25f, 50,13, 0.2f, "watersplash" );

						trap_S_StartSound( lastPos, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.waterHitSound );
					le->inwater = qfalse;
				}
			}
		}
		return;
	}

	if (!( contents & lastContents & CONTENTS_WATER )) {
		le->inwater = qtrue;
	}

}

/*
================
CG_FragmentBounceMark
================
*/
static void CG_FragmentBounceMark( localEntity_t *le, trace_t *trace ) {
	int			radius;
	int i;
	qhandle_t hshader;

	if ( le->leMarkType == LEMT_BLOOD ) {
		i = rand()%1;
		if(i < 0){
			i = 0;
		}
		if(i > 1){
			i = 1;
		}

		if(i == 0){
			hshader = cgs.media.zbloodSplat1Shader;
		}else if ( i == 1) {
			hshader = cgs.media.bloodMarkShader;
		}

		radius = 16 + (rand()&31);
		CG_ImpactMark( hshader, trace->endpos, trace->plane.normal, random()*360,
			1,1,1,1, qtrue, radius, qfalse );
	} else if ( le->leMarkType == LEMT_QBLOOD ) {

		radius = 16 + (rand()&31);
	} else if ( le->leMarkType == LEMT_BURN ) {

		radius = 8 + (rand()&15);
		CG_ImpactMark( cgs.media.burnMarkShader, trace->endpos, trace->plane.normal, random()*360,
			1,1,1,1, qtrue, radius, qfalse );
	}


	// don't allow a fragment to make multiple marks, or they
	// pile up while settling
	le->leMarkType = LEMT_NONE;
}

/*
================
CG_FragmentBounceSound
================
*/
static void CG_FragmentBounceSound( localEntity_t *le, trace_t *trace ) {
	if ( le->leBounceSoundType == LEBS_BLOOD ) {
		// half the gibs will make splat sounds
		if ( rand() & 1 ) {
			int r = rand()&3;
			sfxHandle_t	s;

			if ( r == 0 ) {
				s = cgs.media.gibBounce1Sound;
			} else if ( r == 1 ) {
				s = cgs.media.gibBounce2Sound;
			} else {
				s = cgs.media.gibBounce3Sound;
			}
			trap_S_StartSound( trace->endpos, ENTITYNUM_WORLD, CHAN_AUTO, s );
		}
	} else if ( le->leBounceSoundType == LEBS_BRASS ) {
			int r = rand()&4;
			sfxHandle_t	s;

			if ( r == 0 ) {
				s = cgs.media.brassBounce1Sound;
			} else if ( r == 1 ) {
				s = cgs.media.brassBounce2Sound;
			} else if ( r == 2 ) {
				s = cgs.media.brassBounce3Sound;
			}else {
				s = cgs.media.brassBounce4Sound;
			}
			trap_S_StartSound( trace->endpos, ENTITYNUM_WORLD, CHAN_AUTO, s );
	} else if ( le->leBounceSoundType == LEBS_SHELLS ) {
			int r = rand()&2;
			sfxHandle_t	s;

			if ( r == 0 ) {
				s = cgs.media.shellBounce1Sound;
			} else {
				s = cgs.media.shellBounce2Sound;
			}
			trap_S_StartSound( trace->endpos, ENTITYNUM_WORLD, CHAN_AUTO, s );
	}

	// don't allow a fragment to make multiple bounce sounds,
	// or it gets too noisy as they settle
	le->leBounceSoundType = LEBS_NONE;
}

/*
================
CG_WaterVelocity
Check for contents change (low grav for in water and
normal grav for out of water).
================
*/
static void CG_WaterVelocity( localEntity_t *le, trace_t *trace ) {
	vec3_t	velocity;
	int		hitTime;

	// reflect the velocity on the trace plane
	hitTime = cg.time - cg.frametime + cg.frametime * trace->fraction;
	BG_EvaluateTrajectoryDelta( &le->pos, hitTime, velocity );

/*	if(!le->inwater && le->pos.trType == TR_GRAVITY){
		velocity[0] -= le->pos.trDelta[0]/2 ;
		velocity[1] -= le->pos.trDelta[1]/2 ;
		velocity[2] -= le->pos.trDelta[2]/2;
	}*/

	VectorScale( velocity, le->bounceFactor, le->pos.trDelta );

	VectorCopy( trace->endpos, le->pos.trBase );

	le->pos.trTime = cg.time;

	le->pos.trType = TR_GRAVITY_LOW;

}

/*
================
CG_ReflectVelocity
================
*/
static void CG_ReflectVelocity( localEntity_t *le, trace_t *trace ) {
	vec3_t	velocity;
	float	dot;
	int		hitTime;

	// reflect the velocity on the trace plane

	hitTime = cg.time - cg.frametime + cg.frametime * trace->fraction;
	BG_EvaluateTrajectoryDelta( &le->pos, hitTime, velocity );

	dot = DotProduct( velocity, trace->plane.normal);
	VectorMA( velocity, -2*dot, trace->plane.normal, le->pos.trDelta);

	VectorScale( le->pos.trDelta, le->bounceFactor, le->pos.trDelta );

	VectorCopy( trace->endpos, le->pos.trBase );
	le->pos.trTime = cg.time;


	// check for stop, making sure that even on low FPS systems it doesn't bobble
	if ( trace->allsolid || 
		( trace->plane.normal[2] > 0 && 
		( le->pos.trDelta[2] < 40 || le->pos.trDelta[2] < -cg.frametime * le->pos.trDelta[2] ) ) ) {
			if(le->leBounceSoundType != LEBS_BRASS && le->leBounceSoundType != LEBS_SHELLS){
				le->pos.trType = TR_STATIONARY;
			}
	} else {
		if (le->leFlags & LEF_TUMBLE){
			// collided with a surface so calculate the new rotation axis
			CrossProduct (trace->plane.normal, velocity, le->rotAxis);
			le->angVel = VectorNormalize (le->rotAxis) / le->radius;
			// save current orientation as a rotation from model's base orientation
			QuatMul (le->quatOrient, le->quatRot, le->quatRot);
			// reset the orientation
			QuatInit(1,0,0,0,le->quatOrient);
		}
	}
	le->NumBounces ++;
}

/*
================
CG_AddFragment
================
*/
static void CG_AddFragment( localEntity_t *le ) {
	vec3_t	newOrigin;
	trace_t	trace;
	trace_t	tracew;
	vec3_t mins = {-6,-6,-6};
	vec3_t maxs = {6,6,6};
	vec3_t mins1 = {-1,-1,-1};
	vec3_t maxs1 = {1,1,1};
	vec3_t	origin;
	int		contents;


	BG_EvaluateTrajectory( &le->pos, cg.time, origin );
	contents = CG_PointContents( origin, -1 );

	CG_FragmentSplash( le );

	if ( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) {
		le->leFlags &= ~LEF_ON_FIRE;
	}

	if ( le->pos.trType == TR_STATIONARY ) {
		// sink into the ground if near the removal time
		int		t;
		float	oldZ;
		
		t = le->endTime - cg.time;
		if ( t < SINK_TIME ) {
			// we must use an explicit lighting origin, otherwise the
			// lighting would be lost as soon as the origin went
			// into the ground
			VectorCopy( le->refEntity.origin, le->refEntity.lightingOrigin );
			le->refEntity.renderfx |= RF_LIGHTING_ORIGIN;
			oldZ = le->refEntity.origin[2];
			le->refEntity.origin[2] -= 16 * ( 1.0 - (float)t / SINK_TIME );
			trap_R_AddRefEntityToScene( &le->refEntity );
			le->refEntity.origin[2] = oldZ;
			if(le->leFlags & LEF_ON_FIRE) {
				CG_OnFireBase(le->refEntity.origin, 8+rand()%6,3+rand()%3,2);
				//	CG_OnFireSparks(le->refEntity.origin, 1+rand()%3,0,3);
			}
		} else {
			if(le->leFlags & LEF_ON_FIRE) {
				CG_OnFireBase(le->refEntity.origin, 8+rand()%6,3+rand()%3,2);
				//	CG_OnFireSparks(le->refEntity.origin, 1+rand()%3,0,3);
			}
			trap_R_AddRefEntityToScene( &le->refEntity );
		}

		return;
	}

	// calculate new position
	BG_EvaluateTrajectory( &le->pos, cg.time, newOrigin );

	// trace a line from previous position to new position
	if(le->leBounceSoundType == LEBS_BRASS || le->leBounceSoundType == LEBS_SHELLS){
		CG_Trace( &trace, le->refEntity.origin, mins1, maxs1, newOrigin, -1, CONTENTS_SOLID );
		CG_Trace( &tracew, le->refEntity.origin, mins1, maxs1, newOrigin, -1, MASK_WATER );
	}else{
		CG_Trace( &trace, le->refEntity.origin, mins, maxs, newOrigin, -1, CONTENTS_SOLID );
		CG_Trace( &tracew, le->refEntity.origin, mins, maxs, newOrigin, -1, MASK_WATER );
	}

	if(tracew.fraction != 1.0){

		if(!tracew.startsolid){
			CG_WaterVelocity( le, &tracew );
		}

		if ( trace.fraction == 1.0 ) {

			if(!le->inwater && le->pos.trType == TR_GRAVITY){
				le->pos.trType = TR_GRAVITY_LOW;
				le->pos.trTime = cg.time;
			}

			// still in free fall
			VectorCopy( newOrigin, le->refEntity.origin );

			if (le->leFlags & LEF_TUMBLE){
				vec4_t qrot;
				// angular rotation for this frame
				float angle = le->angVel * (cg.time - le->angles.trTime) * 0.001/2;
				// create the rotation quaternion
				qrot[0] = cos (angle); // real part
				VectorScale (le->rotAxis, sin(angle), &qrot[1]);// imaginary part
				// create the new orientation
				QuatMul (le->quatOrient, qrot, le->quatOrient);
				// apply the combined previous rotations around other axes
				QuatMul (le->quatOrient, le->quatRot, qrot);
				// convert the orientation into the form the renderer wants
				QuatToAxis (qrot, le->refEntity.axis);
				le->angles.trTime = cg.time;
			}


			trap_R_AddRefEntityToScene( &le->refEntity );

			// add a blood trail
			if ( le->leBounceSoundType == LEBS_BLOOD ) {

					CG_BloodTrail( le ,le->pos.trDelta);

			}

			return;
		}

		if ( trap_CM_PointContents( trace.endpos, 0 ) & CONTENTS_NODROP ) {
			CG_FreeLocalEntity( le );
			return;
		}

		// leave a mark
		CG_FragmentBounceMark( le, &trace );

		// do a bouncy sound
		CG_FragmentBounceSound( le, &trace );

		// reflect the velocity on the trace plane
		CG_ReflectVelocity( le, &trace );

		trap_R_AddRefEntityToScene( &le->refEntity );

		return;
	}

	if ( trace.fraction == 1.0 ) {
		// still in free fall
		if(le->inwater && le->pos.trType == TR_GRAVITY_LOW){
			le->pos.trType = TR_GRAVITY;
			le->pos.trTime = cg.time;
		}

		VectorCopy( newOrigin, le->refEntity.origin );

		if (le->leFlags & LEF_TUMBLE){
			vec4_t qrot;
			// angular rotation for this frame
			float angle = le->angVel * (cg.time - le->angles.trTime) * 0.001/2;
			// create the rotation quaternion
			qrot[0] = cos (angle); // real part
			VectorScale (le->rotAxis, sin(angle), &qrot[1]);// imaginary part
			// create the new orientation
			QuatMul (le->quatOrient, qrot, le->quatOrient);
			// apply the combined previous rotations around other axes
			QuatMul (le->quatOrient, le->quatRot, qrot);
			// convert the orientation into the form the renderer wants
			QuatToAxis (qrot, le->refEntity.axis);
			le->angles.trTime = cg.time;
		}


		trap_R_AddRefEntityToScene( &le->refEntity );

		// add a blood trail
		if ( le->leBounceSoundType == LEBS_BLOOD ) {

				CG_BloodTrail( le ,le->pos.trDelta);

		}
		if(le->leFlags & LEF_ON_FIRE) {
			CG_OnFireBase(le->refEntity.origin, 8+rand()%6,3+rand()%3,2);
			//			CG_OnFireSparks(le->refEntity.origin, 1+rand()%3,0,3);
		}

		return;
	}

	// if it is in a nodrop zone, remove it
	// this keeps gibs from waiting at the bottom of pits of death
	// and floating levels
	if ( trap_CM_PointContents( trace.endpos, 0 ) & CONTENTS_NODROP ) {
		CG_FreeLocalEntity( le );
		return;
	}

	// leave a mark
	CG_FragmentBounceMark( le, &trace );

	// do a bouncy sound
	CG_FragmentBounceSound( le, &trace );

	// reflect the velocity on the trace plane
	CG_ReflectVelocity( le, &trace );

	trap_R_AddRefEntityToScene( &le->refEntity );
}

/*
================
CG_AddBodyFragment
================
*/
#define SPARK_WIDTH 2
static void CG_AddBodyFragment( localEntity_t *le )
{
	refEntity_t	*re;
	vec3_t  oldOrg ,vel;
	vec3_t  newOrg;
	trace_t trace;
	trace_t	tracew;
	vec3_t dir, view, right;
	vec3_t mins = {-7,-7,-7};
	vec3_t maxs = {7,7,7};
	vec3_t	origin;
	int		contents;


	BG_EvaluateTrajectory( &le->pos, cg.time, origin );
	contents = CG_PointContents( origin, -1 );

	CG_FragmentSplash( le );

	if ( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) {
		le->leFlags &= ~LEF_ON_FIRE;
	}
	
	re = &le->refEntity;

	BG_EvaluateTrajectory( &le->pos, le->angVel, vel );

	if ( le->pos.trType == TR_STATIONARY ) {
		// sink into the ground if near the removal time
		int		t;
		float	oldZ;
		
		t = le->endTime - cg.time;
		if ( t < SINK_TIME ) {
			// we must use an explicit lighting origin, otherwise the
			// lighting would be lost as soon as the origin went
			// into the ground
			VectorCopy( le->refEntity.origin, le->refEntity.lightingOrigin );
			le->refEntity.renderfx |= RF_LIGHTING_ORIGIN;
			oldZ = le->refEntity.origin[2];
			le->refEntity.origin[2] -= 16 * ( 1.0 - (float)t / SINK_TIME );
			trap_R_AddRefEntityToScene( &le->refEntity );
			le->refEntity.origin[2] = oldZ;
			if(le->leFlags & LEF_ON_FIRE) {
				CG_OnFireBase(le->refEntity.origin, 8+rand()%6,3+rand()%3,2);
				//				CG_OnFireSparks(le->refEntity.origin, 1+rand()%3,0,3);
			}
		} else {
			trap_R_AddRefEntityToScene( &le->refEntity );
			if(le->leFlags & LEF_ON_FIRE) {
				CG_OnFireBase(le->refEntity.origin, 8+rand()%6,3+rand()%3,2);
				//				CG_OnFireSparks(le->refEntity.origin, 1+rand()%3,0,3);
			}
		}

		return;
	}



	BG_EvaluateTrajectory( &le->pos, cg.time, newOrg );
	BG_EvaluateTrajectory( &le->pos, cg.oldTime, oldOrg ); // Check the old for direction tracing

	VectorSubtract( newOrg,oldOrg , dir );
	VectorNormalize( dir );

	VectorSubtract( newOrg, cg.refdef[le->view].vieworg, view );

	CrossProduct( dir, view, right );
	VectorNormalize( right );



	CG_Trace( &tracew, le->refEntity.origin, mins, maxs, newOrg, -1, MASK_WATER );
	CG_Trace( &trace, le->refEntity.origin, mins, maxs, newOrg, -1, CONTENTS_SOLID );

	if(tracew.fraction != 1.0){

		if(!tracew.startsolid){
			CG_WaterVelocity( le, &tracew );
		}

		if ( trace.fraction == 1.0 ) {

			if(!le->inwater && le->pos.trType == TR_GRAVITY){
				le->pos.trType = TR_GRAVITY_LOW;
				le->pos.trTime = cg.time;
			}

			// still in free fall
			VectorCopy( newOrg, le->refEntity.origin );

			if (le->leFlags & LEF_TUMBLE){
				vec4_t qrot;
				// angular rotation for this frame
				float angle = le->angVel * (cg.time - le->angles.trTime) * 0.001/2;
				// create the rotation quaternion
				qrot[0] = cos (angle); // real part
				VectorScale (le->rotAxis, sin(angle), &qrot[1]);// imaginary part
				// create the new orientation
				QuatMul (le->quatOrient, qrot, le->quatOrient);
				// apply the combined previous rotations around other axes
				QuatMul (le->quatOrient, le->quatRot, qrot);
				// convert the orientation into the form the renderer wants
				QuatToAxis (qrot, le->refEntity.axis);
				le->angles.trTime = cg.time;
			}


			trap_R_AddRefEntityToScene( &le->refEntity );

			// add a blood trail
			if ( le->leBounceSoundType == LEBS_BLOOD ) {

					CG_BloodTrail( le ,le->pos.trDelta);

			}

			return;
		}

		if ( trap_CM_PointContents( trace.endpos, 0 ) & CONTENTS_NODROP ) {
			CG_FreeLocalEntity( le );
			return;
		}

		// leave a mark
		CG_FragmentBounceMark( le, &trace );

		// do a bouncy sound
		CG_FragmentBounceSound( le, &trace );

		// reflect the velocity on the trace plane
		CG_ReflectVelocity( le, &trace );

		trap_R_AddRefEntityToScene( &le->refEntity );

		return;
	}

	if ( trace.fraction == 1.0 ) {
		if(le->inwater && le->pos.trType == TR_GRAVITY_LOW){
			le->pos.trType = TR_GRAVITY;
			le->pos.trTime = cg.time;
		}
		VectorCopy( newOrg, le->refEntity.origin );

		if (le->leFlags & LEF_TUMBLE){
			vec4_t qrot;
			// angular rotation for this frame
			float angle = le->angVel * (cg.time - le->angles.trTime) * 0.001/2;
			// create the rotation quaternion
			qrot[0] = cos (angle); // real part
			VectorScale (le->rotAxis, sin(angle), &qrot[1]);// imaginary part
			// create the new orientation
			QuatMul (le->quatOrient, qrot, le->quatOrient);
			// apply the combined previous rotations around other axes
			QuatMul (le->quatOrient, le->quatRot, qrot);
			// convert the orientation into the form the renderer wants
			QuatToAxis (qrot, le->refEntity.axis);
			le->angles.trTime = cg.time;
		}


		trap_R_AddRefEntityToScene( &le->refEntity );
		if ( le->leBounceSoundType == LEBS_BLOOD ) {

				CG_BloodTrail( le ,le->pos.trDelta);

		}
		if(le->leFlags & LEF_ON_FIRE) {
			CG_OnFireBase(le->refEntity.origin, 8+rand()%6,3+rand()%3,2);
			//			CG_OnFireSparks(le->refEntity.origin, 1+rand()%3,0,3);
		}
		return;
	}

	if ( trap_CM_PointContents( trace.endpos, 0 ) & CONTENTS_NODROP ) {
		CG_FreeLocalEntity( le );
		return;
	}
	
	CG_ReflectVelocity( le, &trace );


	trap_R_AddRefEntityToScene( &le->refEntity );



}

/*
================
CG_AddQFragment
This version of fragments have quad overlay.
================
*/
void CG_AddQFragment( localEntity_t *le ) {
	vec3_t	newOrigin,vel;
	trace_t	trace;
	trace_t	tracew;
	vec3_t mins = {-6,-6,-6};
	vec3_t maxs = {6,6,6};
	refEntity_t	powerup;
	vec3_t	origin;
	int		contents;


	BG_EvaluateTrajectory( &le->pos, cg.time, origin );
	contents = CG_PointContents( origin, -1 );

	CG_FragmentSplash( le );

	if ( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) {
		le->leFlags &= ~LEF_ON_FIRE;
	}

	BG_EvaluateTrajectory( &le->pos, le->angVel, vel );
	if ( le->pos.trType == TR_STATIONARY ) {
		// sink into the ground if near the removal time
		int		t;
		float	oldZ;
		float	oldZ2;
		
		t = le->endTime - cg.time;
		if ( t < SINK_TIME ) {
			// we must use an explicit lighting origin, otherwise the
			// lighting would be lost as soon as the origin went
			// into the ground
			VectorCopy( le->refEntity.origin, le->refEntity.lightingOrigin );
			le->refEntity.renderfx |= RF_LIGHTING_ORIGIN;
			oldZ = le->refEntity.origin[2];
			le->refEntity.origin[2] -= 16 * ( 1.0 - (float)t / SINK_TIME );
			trap_R_AddRefEntityToScene( &le->refEntity );
			le->refEntity.origin[2] = oldZ;
			if(le->leFlags & LEF_ON_FIRE) {
				CG_OnFireBase(le->refEntity.origin, 8+rand()%6,3+rand()%3,2);
				//				CG_OnFireSparks(le->refEntity.origin, 1+rand()%3,0,3);
			}
		} else {
			trap_R_AddRefEntityToScene( &le->refEntity );
			if(le->leFlags & LEF_ON_FIRE) {
				CG_OnFireBase(le->refEntity.origin, 8+rand()%6,3+rand()%3,2);
				//				CG_OnFireSparks(le->refEntity.origin, 1+rand()%3,0,3);
			}
		}
		if ( t < SINK_TIME ) {
			// we must use an explicit lighting origin, otherwise the
			// lighting would be lost as soon as the origin went
			// into the ground
			memcpy(&powerup, &le->refEntity, sizeof(le->refEntity));
				oldZ2 = powerup.origin[2];
				powerup.origin[2] -= 16 * ( 1.0 - (float)t / SINK_TIME );
				if(le->generic1 == TEAM_RED){
					powerup.customShader = cgs.media.redquadWeaponShader;
				}else{
					powerup.customShader = cgs.media.quadWeaponShader;
				}
			trap_R_AddRefEntityToScene( &powerup );
			powerup.origin[2] = oldZ2;
		} else {
			memcpy(&powerup, &le->refEntity, sizeof(le->refEntity));
				if(le->generic1 == TEAM_RED){
					powerup.customShader = cgs.media.redquadWeaponShader;
				}else{
					powerup.customShader = cgs.media.quadWeaponShader;
				}
			trap_R_AddRefEntityToScene( &powerup );
		}
		return;
	}

	// calculate new position
	BG_EvaluateTrajectory( &le->pos, cg.time, newOrigin );

	if ( le->leBounceSoundType == LEBS_BLOOD ) {
		CG_BloodTrail( le ,le->pos.trDelta);
	}

	// trace a line from previous position to new position
	CG_Trace( &trace, le->refEntity.origin, mins, maxs, newOrigin, -1, CONTENTS_SOLID );
	CG_Trace( &tracew, le->refEntity.origin, mins, maxs, newOrigin, -1, MASK_WATER );

	if(tracew.fraction != 1.0){

		if(!tracew.startsolid){
			CG_WaterVelocity( le, &tracew );
		}

		if ( trace.fraction == 1.0 ) {

			if(!le->inwater && le->pos.trType == TR_GRAVITY){
				le->pos.trType = TR_GRAVITY_LOW;
				le->pos.trTime = cg.time;
			}

			// still in free fall
			VectorCopy( newOrigin, le->refEntity.origin );

			if (le->leFlags & LEF_TUMBLE){
				vec4_t qrot;
				// angular rotation for this frame
				float angle = le->angVel * (cg.time - le->angles.trTime) * 0.001/2;
				// create the rotation quaternion
				qrot[0] = cos (angle); // real part
				VectorScale (le->rotAxis, sin(angle), &qrot[1]);// imaginary part
				// create the new orientation
				QuatMul (le->quatOrient, qrot, le->quatOrient);
				// apply the combined previous rotations around other axes
				QuatMul (le->quatOrient, le->quatRot, qrot);
				// convert the orientation into the form the renderer wants
				QuatToAxis (qrot, le->refEntity.axis);
				le->angles.trTime = cg.time;
			}


			trap_R_AddRefEntityToScene( &le->refEntity );

			memcpy(&powerup, &le->refEntity, sizeof(le->refEntity));
			if(le->generic1 == TEAM_RED){
				powerup.customShader = cgs.media.redquadWeaponShader;
			}else{
				powerup.customShader = cgs.media.quadWeaponShader;
			}
			trap_R_AddRefEntityToScene( &powerup );

			return;
		}

		if ( trap_CM_PointContents( trace.endpos, 0 ) & CONTENTS_NODROP ) {
			CG_FreeLocalEntity( le );
			return;
		}

		// leave a mark
		CG_FragmentBounceMark( le, &trace );

		// do a bouncy sound
		CG_FragmentBounceSound( le, &trace );

		// reflect the velocity on the trace plane
		CG_ReflectVelocity( le, &trace );

		trap_R_AddRefEntityToScene( &le->refEntity );

		memcpy(&powerup, &le->refEntity, sizeof(le->refEntity));
			if(le->generic1 == TEAM_RED){
				powerup.customShader = cgs.media.redquadWeaponShader;
			}else{
				powerup.customShader = cgs.media.quadWeaponShader;
			}
		trap_R_AddRefEntityToScene( &powerup );

		return;
	}

	if ( trace.fraction == 1.0 ) {
		if(le->inwater && le->pos.trType == TR_GRAVITY_LOW){
			le->pos.trType = TR_GRAVITY;
			le->pos.trTime = cg.time;
		}
		VectorCopy( newOrigin, le->refEntity.origin );

		if (le->leFlags & LEF_TUMBLE){
			vec4_t qrot;
			// angular rotation for this frame
			float angle = le->angVel * (cg.time - le->angles.trTime) * 0.001/2;
			// create the rotation quaternion
			qrot[0] = cos (angle); // real part
			VectorScale (le->rotAxis, sin(angle), &qrot[1]);// imaginary part
			// create the new orientation
			QuatMul (le->quatOrient, qrot, le->quatOrient);
			// apply the combined previous rotations around other axes
			QuatMul (le->quatOrient, le->quatRot, qrot);
			// convert the orientation into the form the renderer wants
			QuatToAxis (qrot, le->refEntity.axis);
			le->angles.trTime = cg.time;
		}

		trap_R_AddRefEntityToScene( &le->refEntity );
		if(le->leFlags & LEF_ON_FIRE) {
			CG_OnFireBase(le->refEntity.origin, 8+rand()%6,3+rand()%3,2);
			//			CG_OnFireSparks(le->refEntity.origin, 1+rand()%3,0,3);
		}

		memcpy(&powerup, &le->refEntity, sizeof(le->refEntity));
		if(le->generic1 == TEAM_RED){
			powerup.customShader = cgs.media.redquadWeaponShader;
		}else{
			powerup.customShader = cgs.media.quadWeaponShader;
		}
		trap_R_AddRefEntityToScene( &powerup );

		return;
	}

	// if it is in a nodrop zone, remove it
	// this keeps gibs from waiting at the bottom of pits of death
	// and floating levels
	if ( trap_CM_PointContents( trace.endpos, 0 ) & CONTENTS_NODROP ) {
		CG_FreeLocalEntity( le );
		return;
	}

	// leave a mark
	CG_FragmentBounceMark( le, &trace );

	// do a bouncy sound
	CG_FragmentBounceSound( le, &trace );

	// reflect the velocity on the trace plane
	CG_ReflectVelocity( le, &trace );

	trap_R_AddRefEntityToScene( &le->refEntity );

	memcpy(&powerup, &le->refEntity, sizeof(le->refEntity));
				if(le->generic1 == TEAM_RED){
					powerup.customShader = cgs.media.redquadWeaponShader;
				}else{
					powerup.customShader = cgs.media.quadWeaponShader;
				}
	trap_R_AddRefEntityToScene( &powerup );
}

/*
=====================================================================

TRIVIAL LOCAL ENTITIES

These only do simple scaling or modulation before passing to the renderer
=====================================================================
*/

/*
====================
CG_AddFadeRGB
====================
*/
void CG_AddFadeRGB( localEntity_t *le ) {
	refEntity_t *re;
	float c;

	re = &le->refEntity;
	if ( le->fadeInTime > le->startTime && cg.time < le->fadeInTime ) {
		// fade / grow time
		c = 1.0 - (float) ( le->fadeInTime - cg.time ) / ( le->fadeInTime - le->startTime );
	}else{
		c = ( le->endTime - cg.time ) * le->lifeRate;
	}
	if(le->NumBounces == 1){
		vec3_t	angles;

		VectorClear( angles );
		angles[YAW] = ( cg.time & 2047 ) * 360 / 2048.0;
		AnglesToAxis(angles,re->axis);
	}


	c *= 0xff;

	re->shaderRGBA[0] = le->color[0] * c;
	re->shaderRGBA[1] = le->color[1] * c;
	re->shaderRGBA[2] = le->color[2] * c;
	re->shaderRGBA[3] = le->color[3] * c;
	
	trap_R_AddRefEntityToScene( re );
}

/*
====================
CG_AddFadeRGBA
====================
*/
void CG_AddFadeRGBA( localEntity_t *le ) {
	refEntity_t *re;
	float c;

	re = &le->refEntity;

	c = ( le->endTime - cg.time ) * le->lifeRate;
	c *= 0xff;

	re->shaderRGBA[0] = le->color[0] * c;
	re->shaderRGBA[1] = le->color[1] * c;
	re->shaderRGBA[2] = le->color[2] * c;
	re->shaderRGBA[3] = le->color[3] * c;
	
	trap_R_AddRefEntityToScene( re );
}

/*
==================
CG_AddMoveScaleFade
==================
*/
static void CG_AddMoveScaleFade( localEntity_t *le ) {
	refEntity_t	*re;
	float		c;
	vec3_t		delta;
	float		len;

	re = &le->refEntity;

	if ( le->fadeInTime > le->startTime && cg.time < le->fadeInTime ) {
		// fade / grow time
		c = 1.0 - (float) ( le->fadeInTime - cg.time ) / ( le->fadeInTime - le->startTime );
	}
	else {
		// fade / grow time
		c = ( le->endTime - cg.time ) * le->lifeRate;
	}


	re->shaderRGBA[3] = 0xff * c * le->color[3];

	if ( !( le->leFlags & LEF_PUFF_DONT_SCALE ) ) {
		re->radius = le->radius * ( 1.0 - c ) + 8;
	}

	BG_EvaluateTrajectory( &le->pos, cg.time, re->origin );

	// if the view would be "inside" the sprite, kill the sprite
	// so it doesn't add too much overdraw

	VectorSubtract( re->origin, cg.refdef[le->view].vieworg, delta );

	len = VectorLength( delta );
	if ( len < le->radius ) {
		CG_FreeLocalEntity( le );
		return;
	}

	trap_R_AddRefEntityToScene( re );
}
/*
==================
CG_Flare
An attempt for creating the do all polygon. :) 
==================
*/
static void CG_Flare( localEntity_t *le ) {
	polyVert_t verts[4];
	refEntity_t	*re;
	float		c,len,sizeup,sizedown;
	vec3_t	p,rotate_ang,rr, ru,delta,rlforward, rlvright, rlvup/*,color*/;
	int i, j;
	float		ratio;
	float		time, time2;

	re = &le->refEntity;

	if ( le->fadeInTime > le->startTime && cg.time < le->fadeInTime ) {
		// fade / grow time
		c = 1.0 - (float) ( le->fadeInTime - cg.time ) / ( le->fadeInTime - le->startTime );
	}
	else {
	//	 fade / grow time
		c = ( le->endTime - cg.time ) / ( float ) ( le->endTime - le->startTime );
	//	c = (float) ( le->endTime - cg.time ) * le->lifeRate;
	}

	if( le->leFlags & LEF_NO_RGB_FADE ){ 
		re->shaderRGBA[0] = (unsigned char) 255 * le->color[0];
		re->shaderRGBA[1] = (unsigned char) 255 * le->color[1];
		re->shaderRGBA[2] = (unsigned char) 255 * le->color[2];
	}else{
		re->shaderRGBA[0] = (unsigned char) 255 * c * le->color[0];
		re->shaderRGBA[1] = (unsigned char) 255 * c * le->color[1];
		re->shaderRGBA[2] = (unsigned char) 255 * c * le->color[2];
	}
	if( le->leFlags & LEF_NO_ALPHA_FADE ){ 
		re->shaderRGBA[3] = 255;
	}else{
		re->shaderRGBA[3] = (unsigned char) 255 * c * le->color[3];
	}

		if (re->rotation) {
			VectorCopy( cg.refdef[le->view].viewaxis[0], rlforward );
			VectorCopy( cg.refdef[le->view].viewaxis[1], rlvright );
			VectorCopy( cg.refdef[le->view].viewaxis[2], rlvup );
			vectoangles(cg.refdef[le->view].viewaxis[0], rotate_ang );
			rotate_ang[ROLL] += re->rotation;
			AngleVectors ( rotate_ang, rlforward, rr, ru);
		} else {
			VectorCopy( cg.refdef[le->view].viewaxis[0], rlforward );
			VectorCopy( cg.refdef[le->view].viewaxis[1], rlvright );
			VectorCopy( cg.refdef[le->view].viewaxis[2], rlvup );
		}
	

	// scale up?
	sizeup = le->radius * ( 1.0 - c ) + 8;
	// or down 
	sizedown = le->radius * c;


	// 1st
		VectorCopy( re->origin, p );
	if (re->rotation) {
		if ( le->leFlags & LEF_PUFF_DONT_SCALE ) {
			VectorMA (p, re->radius, ru, p);	
			VectorMA (p, re->radius, rr, p);		
		}else if (  le->leFlags & LEF_PUFF_SCALE_DOWN  ) {
			VectorMA (p, sizedown, ru, p);	
			VectorMA (p, sizedown, rr, p);
		}else{
			VectorMA (p, sizeup, ru, p);	
			VectorMA (p, sizeup, rr, p);
		}
	}else{
		if ( le->leFlags & LEF_PUFF_DONT_SCALE ) {
			VectorMA (p, re->radius, rlvup, p);	
			VectorMA (p, re->radius, rlvright, p);		
		}else if (  le->leFlags & LEF_PUFF_SCALE_DOWN  ) {
			VectorMA (p, sizedown, rlvup, p);	
			VectorMA (p, sizedown, rlvright, p);
		}else{
			VectorMA (p, sizeup, rlvup, p);	
			VectorMA (p, sizeup, rlvright, p);
		}
	}
		VectorCopy (p, verts[0].xyz);	
		verts[0].st[0] = 0;	
		verts[0].st[1] = 0;	
		verts[0].modulate[0] = re->shaderRGBA[0];
		verts[0].modulate[1] = re->shaderRGBA[1];
		verts[0].modulate[2] = re->shaderRGBA[2];
		verts[0].modulate[3] = re->shaderRGBA[3];


	// 2nd
		VectorCopy( re->origin, p );
	if (re->rotation) {
		if (  le->leFlags & LEF_PUFF_DONT_SCALE  ) {
			VectorMA (p, re->radius, ru, p);	
			VectorMA (p, -re->radius, rr, p);
		}else if (  le->leFlags & LEF_PUFF_SCALE_DOWN  ) {
			VectorMA (p, sizedown, ru, p);	
			VectorMA (p, -sizedown, rr, p);
		}else{
			VectorMA (p, sizeup, ru, p);	
			VectorMA (p, -sizeup, rr, p);
		}
	}else{
		if (  le->leFlags & LEF_PUFF_DONT_SCALE  ) {
			VectorMA (p, re->radius, rlvup, p);	
			VectorMA (p, -re->radius, rlvright, p);
		}else if (  le->leFlags & LEF_PUFF_SCALE_DOWN  ) {
			VectorMA (p, sizedown, rlvup, p);	
			VectorMA (p, -sizedown, rlvright, p);
		}else{
			VectorMA (p, sizeup, rlvup, p);	
			VectorMA (p, -sizeup, rlvright, p);
		}
	}
		VectorCopy (p, verts[1].xyz);	
		verts[1].st[0] = 0;	
		verts[1].st[1] = 1;	
		verts[1].modulate[0] = re->shaderRGBA[0];
		verts[1].modulate[1] = re->shaderRGBA[1];
		verts[1].modulate[2] = re->shaderRGBA[2];
		verts[1].modulate[3] = re->shaderRGBA[3];
		
	// 3rd
		VectorCopy( re->origin, p );
	if (re->rotation) {
		if (  le->leFlags & LEF_PUFF_DONT_SCALE ) {
			VectorMA (p, -re->radius, ru, p);	
			VectorMA (p, -re->radius, rr, p);
		}else if (  le->leFlags & LEF_PUFF_SCALE_DOWN  ) {
			VectorMA (p, -sizedown, ru, p);	
			VectorMA (p, -sizedown, rr, p);
		}else{
			VectorMA (p, -sizeup, ru, p);	
			VectorMA (p, -sizeup, rr, p);
		}
	}else{
		if (  le->leFlags & LEF_PUFF_DONT_SCALE ) {
			VectorMA (p, -re->radius, rlvup, p);	
			VectorMA (p, -re->radius, rlvright, p);
		}else if (  le->leFlags & LEF_PUFF_SCALE_DOWN  ) {
			VectorMA (p, -sizedown, rlvup, p);	
			VectorMA (p, -sizedown, rlvright, p);
		}else{
			VectorMA (p, -sizeup, rlvup, p);	
			VectorMA (p, -sizeup, rlvright, p);
		}
	}
		VectorCopy (p, verts[2].xyz);	
		verts[2].st[0] = 1;	
		verts[2].st[1] = 1;	
		verts[2].modulate[0] = re->shaderRGBA[0];
		verts[2].modulate[1] = re->shaderRGBA[1];
		verts[2].modulate[2] = re->shaderRGBA[2];
		verts[2].modulate[3] = re->shaderRGBA[3];
		
	// 4th
		VectorCopy( re->origin, p );
	if (re->rotation) {
		if (  le->leFlags & LEF_PUFF_DONT_SCALE  ) {
			VectorMA (p, -re->radius, ru, p);	
			VectorMA (p, re->radius, rr, p);
		}else if (  le->leFlags & LEF_PUFF_SCALE_DOWN  ) {
			VectorMA (p, -sizedown, ru, p);	
			VectorMA (p, sizedown, rr, p);
		}else{
			VectorMA (p, -sizeup, ru, p);	
			VectorMA (p, sizeup, rr, p);
		}
	}else{
		if (  le->leFlags & LEF_PUFF_DONT_SCALE  ) {
			VectorMA (p, -re->radius, rlvup, p);	
			VectorMA (p, re->radius, rlvright, p);
		}else if (  le->leFlags & LEF_PUFF_SCALE_DOWN  ) {
			VectorMA (p, -sizedown, rlvup, p);	
			VectorMA (p, sizedown, rlvright, p);
		}else{
			VectorMA (p, -sizeup, rlvup, p);	
			VectorMA (p, sizeup, rlvright, p);
		}
	}
		VectorCopy (p, verts[3].xyz);	
		verts[3].st[0] = 1;	
		verts[3].st[1] = 0;	
		verts[3].modulate[0] = re->shaderRGBA[0];
		verts[3].modulate[1] = re->shaderRGBA[1];
		verts[3].modulate[2] = re->shaderRGBA[2];
		verts[3].modulate[3] = re->shaderRGBA[3];	
	// done

	BG_EvaluateTrajectory( &le->pos, cg.time, re->origin );

	VectorSubtract( re->origin, cg.refdef[le->view].vieworg, delta );

	len = VectorLength( delta );
	if ( len < le->radius ) {
		CG_FreeLocalEntity( le );
		return;
	}

	if(le->anim){
		time = cg.time - le->startTime;
		time2 = le->endTime - le->startTime;
		ratio = time / time2;
		i = re->shaderAnim;
		j = (int)floor(ratio * shaderAnimCountsw[re->shaderAnim]);
	//	le->refEntity.customShader = shaderAnimsw[i][j];
		trap_R_AddPolyToScene( shaderAnimsw[i][j], 4, verts );
	}else{
		trap_R_AddPolyToScene( re->customShader, 4, verts );
	}



}
/*
==================
CG_FlarePuff
==================
*/
void CG_FlarePuff( const vec3_t p, const vec3_t vel, 
				   float radius,
				   float r, float g, float b, float a,
				   float duration,
				   int startTime,
				   int fadeInTime,
				   int leFlags,
				   qhandle_t hShader ,char *animStr,qboolean animated) {
	static int	seed = 0x92;
	int anim;
	localEntity_t	*le;
	refEntity_t		*re;
//	int i, j;
	float		ratio;
	float		time, time2;


	if (animStr < (char *)10)
		CG_Error( "CG_FlarePuff: animStr is probably an index rather than a string" );

	for (anim=0; shaderAnimNamesw[anim]; anim++) {
		if (!Q_stricmp( animStr, shaderAnimNamesw[anim] ))
			break;
	}

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_FLARE;
	le->entNumber = le - cg_freeLocalEntities;
	le->leFlags = leFlags;
	re->radius = radius;
	le->radius = radius;

	re->rotation = Q_random( &seed ) * 360;
	re->shaderTime = startTime / 1000.0f;

	le->startTime = startTime;
	le->fadeInTime = fadeInTime;
	le->endTime = startTime + duration;

	time = cg.time - le->startTime;
	time2 = le->endTime - le->startTime;
	ratio = time / time2;

	if ( fadeInTime > startTime ) {
		le->lifeRate = 1.0 / ( le->endTime - le->fadeInTime );
	}
	else {
		le->lifeRate = 1.0 / ( le->endTime - le->startTime );
	}
	le->color[0] = r;
	le->color[1] = g; 
	le->color[2] = b;
	le->color[3] = a;


	AnglesToAxis( cg.refdefViewAngles[0],re->axis);


	le->pos.trType = TR_LINEAR;
	le->pos.trTime = startTime;
	//VectorCopy( vel, le->pos.trDelta );
	VectorCopy( p, le->pos.trBase );

	VectorCopy( p, re->origin );

	if (!animated) {
		re->customShader = hShader;
	}else{
		le->anim = qtrue;
		re->shaderAnim = anim;
	//	i = anim;
	//	j = (int)floor(ratio * shaderAnimCountsw[anim]);
	//	re->customShader = shaderAnimsw[i][j];
	}

}

void CG_RocketSMExplosion( vec3_t dir, vec3_t org ,char *animStr,int duration,int radius) {
	localEntity_t	*le;
	refEntity_t		*re;
	int anim;
	vec3_t			tmpVec, newOrigin;
	float			ang;

	if (animStr < (char *)10)
		CG_Error( "CG_RocketExplosion: animStr is probably an index rather than a string" );

	// find the animation string
	for (anim=0; shaderAnimNamesw[anim]; anim++) {
		if (!Q_stricmp( animStr, shaderAnimNamesw[anim] ))
			break;
	}
	if (!shaderAnimNamesw[anim]) {
		CG_Error("CG_ParticleExplosion: unknown animation string: %s\n", animStr);
		return;
	}

	if (duration < 0) {
		duration *= -1;
	}
	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_FLARE;
	le->entNumber = le - cg_freeLocalEntities;
	le->startTime = cg.time ;
	le->endTime = cg.time + duration;
	le->lifeRate = 1.0 /* ( le->endTime - le->startTime )*/;
	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0;
	le->pos.trType = TR_LINEAR;
	le->pos.trTime = cg.time;

	ang = rand() % 360;
	VectorCopy( dir, re->axis[0] );
	RotateAroundDirection( re->axis, ang );

	re->shaderAnim = anim;
	re->anim = qtrue;

	re->shaderTime = cg.time / 200.0f;
	re->radius = radius;
	re->rotation = rand() % 360;

	VectorScale( dir, 16, tmpVec );
	VectorAdd( tmpVec, org, newOrigin );

	VectorCopy( newOrigin, re->origin );
	VectorCopy( newOrigin, re->oldorigin );

}

/*
==================
CG_RLSMOKE
==================
*/
void CG_RLSMOKE( const vec3_t p, const vec3_t vel, 
				   float radius,
				   float r, float g, float b, float a,
				   float duration,
				   int startTime,
				   int fadeInTime,
				   int leFlags,char *animStr) {
	static int	seed = 0x92;
	int anim;
	localEntity_t	*le;
	refEntity_t		*re;


	if (animStr < (char *)10)
		CG_Error( "CG_FlarePuff: animStr is probably an index rather than a string" );

	for (anim=0; shaderAnimNamesw[anim]; anim++) {
		if (!Q_stricmp( animStr, shaderAnimNamesw[anim] ))
			break;
	}

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leFlags = leFlags;
	le->leType = LE_FLARE;
	le->entNumber = le - cg_freeLocalEntities;
	le->radius = radius;
	re->radius = radius;
	re->rotation = Q_random( &seed ) * 360;

	le->startTime = startTime;
	le->fadeInTime = fadeInTime;
	le->endTime = startTime + duration;

	re->shaderTime = startTime /10000.0f;

	if ( fadeInTime > startTime ) {
		le->lifeRate = 1.0 / ( le->endTime - le->fadeInTime );
	}
	else {
		le->lifeRate = 1.0  /( le->endTime - le->startTime );
	}
	le->color[0] = r ;
	le->color[1] = g ; 
	le->color[2] = b ;
	le->color[3] = a ;


	AnglesToAxis( cg.refdefViewAngles[0],re->axis);


	le->pos.trType = TR_LINEAR;
	le->pos.trTime = startTime;
	VectorCopy( vel, le->pos.trDelta );
	VectorCopy( p, le->pos.trBase );

	VectorCopy( p, re->origin );

	re->shaderAnim = anim;
	re->anim = qtrue;

}

/*
==================
CG_RGRingScaleFade
==================
*/
static void CG_RGRingScaleFade( localEntity_t *le ) {
	refEntity_t	*re;
	polyVert_t verts[4];
	float		c,len,sizeup,sizedown,time,time2,ratio;
	vec3_t	p,rotate_ang,rr, ru,delta,rlforward, rlvright, rlvup;

	re = &le->refEntity;

	if ( le->fadeInTime > le->startTime && cg.time < le->fadeInTime ) {
		// fade / grow time
		c = 1.0 - (float) ( le->fadeInTime - cg.time ) / ( le->fadeInTime - le->startTime );
	}
	else {
		c = ( le->endTime - cg.time ) * le->lifeRate;
		c *= 0xff;
	}

	if( le->leFlags & LEF_NO_RGB_FADE ){ 
		re->shaderRGBA[0] = (unsigned char) 255 * le->color[0];
		re->shaderRGBA[1] = (unsigned char) 255 * le->color[1];
		re->shaderRGBA[2] = (unsigned char) 255 * le->color[2];
	}else{
		re->shaderRGBA[0] = (unsigned char) le->color[0] * c;
		re->shaderRGBA[1] = (unsigned char) le->color[1] * c;
		re->shaderRGBA[2] = (unsigned char) le->color[2] * c;
	}
	if( le->leFlags & LEF_NO_ALPHA_FADE ){ 
		re->shaderRGBA[3] = 255;
	}else{
		re->shaderRGBA[3] = (unsigned char) le->color[3] * c;
	}

	if(re->sparklesT == 1){
			if (re->rotation) {
				VectorCopy( cg.refdef[le->view].viewaxis[0], rlforward );
				VectorCopy( cg.refdef[le->view].viewaxis[1], rlvright );
				VectorCopy( cg.refdef[le->view].viewaxis[2], rlvup );
				vectoangles(cg.refdef[le->view].viewaxis[0], rotate_ang );
				rotate_ang[ROLL] += re->rotation;
				AngleVectors ( rotate_ang, rlforward, rr, ru);
			} else {
				VectorCopy( cg.refdef[le->view].viewaxis[0], rlforward );
				VectorCopy( cg.refdef[le->view].viewaxis[1], rlvright );
				VectorCopy( cg.refdef[le->view].viewaxis[2], rlvup );
			}
	}else{
		if (re->rotation) {
			vectoangles(re->axis[0], rotate_ang );
			rotate_ang[ROLL] += re->rotation;
			AngleVectors ( rotate_ang, NULL, rr, ru);
		}
	}

	time = cg.time - le->startTime;
	time2 = le->endTime - le->startTime;
	ratio = time / time2;

	// scale up?		width = p->width + ( ratio * ( p->endwidth - p->width) );
	if(le->endradius){
		sizeup = le->radius + ( ratio * ( le->endradius - le->radius) );
	}else{
		sizeup = le->radius * ( 1.0 - c ) + 8;
	}
	// or down 
	sizedown = le->radius * c;

	// 1st
		VectorCopy( re->origin, p );
	if(re->sparklesT == 1){
		if (re->rotation) {
			if ( le->leFlags & LEF_PUFF_DONT_SCALE ) {
				VectorMA (p, re->radius, ru, p);	
				VectorMA (p, re->radius, rr, p);		
			}else if (  le->leFlags & LEF_PUFF_SCALE_DOWN  ) {
				VectorMA (p, sizedown, ru, p);	
				VectorMA (p, sizedown, rr, p);
			}else{
				VectorMA (p, sizeup, ru, p);	
				VectorMA (p, sizeup, rr, p);
			}
		}else{
			if ( le->leFlags & LEF_PUFF_DONT_SCALE ) {
				VectorMA (p, re->radius, rlvup, p);	
				VectorMA (p, re->radius, rlvright, p);		
			}else if (  le->leFlags & LEF_PUFF_SCALE_DOWN  ) {
				VectorMA (p, sizedown, rlvup, p);	
				VectorMA (p, sizedown, rlvright, p);
			}else{
				VectorMA (p, sizeup, rlvup, p);	
				VectorMA (p, sizeup, rlvright, p);
			}
		}
	}else{
		if (re->rotation) {
			if ( le->leFlags & LEF_PUFF_DONT_SCALE ) {
				VectorMA (p, -re->radius, ru, p);	
				VectorMA (p, -re->radius, rr, p);		
			}else if (  le->leFlags & LEF_PUFF_SCALE_DOWN  ) {
				VectorMA (p, -sizedown, ru, p);	
				VectorMA (p, -sizedown, rr, p);
			}else{
				VectorMA (p, -sizeup, ru, p);	
				VectorMA (p, -sizeup, rr, p);
			}
		}else{
			if ( le->leFlags & LEF_PUFF_DONT_SCALE ) {
				VectorMA (p, -re->radius, re->axis[2], p);	
				VectorMA (p, -re->radius, re->axis[1], p);		
			}else if (  le->leFlags & LEF_PUFF_SCALE_DOWN  ) {
				VectorMA (p, -sizedown, re->axis[2], p);	
				VectorMA (p, -sizedown, re->axis[1], p);
			}else{
				VectorMA (p, -sizeup, re->axis[2], p);	
				VectorMA (p, -sizeup, re->axis[1], p);
			}
		}
	}
	VectorCopy (p, verts[0].xyz);	
	verts[0].st[0] = 0;	
	verts[0].st[1] = 0;	
	verts[0].modulate[0] = re->shaderRGBA[0];	
	verts[0].modulate[1] = re->shaderRGBA[1];	
	verts[0].modulate[2] = re->shaderRGBA[2];	
	verts[0].modulate[3] = re->shaderRGBA[3];

	// 2nd
		VectorCopy( re->origin, p );
	if(re->sparklesT == 1){
		if (re->rotation) {
			if (  le->leFlags & LEF_PUFF_DONT_SCALE  ) {
				VectorMA (p, re->radius, ru, p);	
				VectorMA (p, -re->radius, rr, p);
			}else if (  le->leFlags & LEF_PUFF_SCALE_DOWN  ) {
				VectorMA (p, sizedown, ru, p);	
				VectorMA (p, -sizedown, rr, p);
			}else{
				VectorMA (p, sizeup, ru, p);	
				VectorMA (p, -sizeup, rr, p);
			}
		}else{
			if (  le->leFlags & LEF_PUFF_DONT_SCALE  ) {
				VectorMA (p, re->radius, rlvup, p);	
				VectorMA (p, -re->radius, rlvright, p);
			}else if (  le->leFlags & LEF_PUFF_SCALE_DOWN  ) {
				VectorMA (p, sizedown, rlvup, p);	
				VectorMA (p, -sizedown, rlvright, p);
			}else{
				VectorMA (p, sizeup, rlvup, p);	
				VectorMA (p, -sizeup, rlvright, p);
			}
		}
	}else{
		if (re->rotation) {
			if (  le->leFlags & LEF_PUFF_DONT_SCALE  ) {
				VectorMA (p, -re->radius, ru, p);	
				VectorMA (p, re->radius, rr, p);
			}else if (  le->leFlags & LEF_PUFF_SCALE_DOWN  ) {
				VectorMA (p, -sizedown, ru, p);	
				VectorMA (p, sizedown, rr, p);
			}else{
				VectorMA (p, -sizeup, ru, p);	
				VectorMA (p, sizeup, rr, p);
			}
		}else{
			if (  le->leFlags & LEF_PUFF_DONT_SCALE  ) {
				VectorMA (p, -re->radius, re->axis[2], p);	
				VectorMA (p, re->radius, re->axis[1], p);
			}else if (  le->leFlags & LEF_PUFF_SCALE_DOWN  ) {
				VectorMA (p, -sizedown, re->axis[2], p);	
				VectorMA (p, sizedown, re->axis[1], p);
			}else{
				VectorMA (p, -sizeup, re->axis[2], p);	
				VectorMA (p, sizeup, re->axis[1], p);
			}
		}
	}
	VectorCopy (p, verts[1].xyz);	
	verts[1].st[0] = 0;	
	verts[1].st[1] = 1;	
	verts[1].modulate[0] = re->shaderRGBA[0];	
	verts[1].modulate[1] = re->shaderRGBA[1];	
	verts[1].modulate[2] = re->shaderRGBA[2];	
	verts[1].modulate[3] = re->shaderRGBA[3];
		
	// 3rd
		VectorCopy( re->origin, p );
	if(re->sparklesT == 1){
		if (re->rotation) {
			if (  le->leFlags & LEF_PUFF_DONT_SCALE ) {
				VectorMA (p, -re->radius, ru, p);	
				VectorMA (p, -re->radius, rr, p);
			}else if (  le->leFlags & LEF_PUFF_SCALE_DOWN  ) {
				VectorMA (p, -sizedown, ru, p);	
				VectorMA (p, -sizedown, rr, p);
			}else{
				VectorMA (p, -sizeup, ru, p);	
				VectorMA (p, -sizeup, rr, p);
			}
		}else{
			if (  le->leFlags & LEF_PUFF_DONT_SCALE ) {
				VectorMA (p, -re->radius, rlvup, p);	
				VectorMA (p, -re->radius, rlvright, p);
			}else if (  le->leFlags & LEF_PUFF_SCALE_DOWN  ) {
				VectorMA (p, -sizedown, rlvup, p);	
				VectorMA (p, -sizedown, rlvright, p);
			}else{
				VectorMA (p, -sizeup, rlvup, p);	
				VectorMA (p, -sizeup, rlvright, p);
			}
		}
	}else{
		if (re->rotation) {
			if (  le->leFlags & LEF_PUFF_DONT_SCALE ) {
				VectorMA (p, re->radius, ru, p);	
				VectorMA (p, re->radius, rr, p);
			}else if (  le->leFlags & LEF_PUFF_SCALE_DOWN  ) {
				VectorMA (p, sizedown, ru, p);	
				VectorMA (p, sizedown, rr, p);
			}else{
				VectorMA (p, sizeup, ru, p);	
				VectorMA (p, sizeup, rr, p);
			}
		}else{
			if (  le->leFlags & LEF_PUFF_DONT_SCALE ) {
				VectorMA (p, re->radius, re->axis[2], p);	
				VectorMA (p, re->radius, re->axis[1], p);
			}else if (  le->leFlags & LEF_PUFF_SCALE_DOWN  ) {
				VectorMA (p, sizedown, re->axis[2], p);	
				VectorMA (p, sizedown, re->axis[1], p);
			}else{
				VectorMA (p, sizeup, re->axis[2], p);	
				VectorMA (p, sizeup, re->axis[1], p);
			}
		}
	}
	VectorCopy (p, verts[2].xyz);	
	verts[2].st[0] = 1;	
	verts[2].st[1] = 1;	
	verts[2].modulate[0] = re->shaderRGBA[0];	
	verts[2].modulate[1] = re->shaderRGBA[1];	
	verts[2].modulate[2] = re->shaderRGBA[2];	
	verts[2].modulate[3] = re->shaderRGBA[3];
		
	// 4th
		VectorCopy( re->origin, p );
	if(re->sparklesT == 1){
		if (re->rotation) {
			if (  le->leFlags & LEF_PUFF_DONT_SCALE  ) {
				VectorMA (p, -re->radius, ru, p);	
				VectorMA (p, re->radius, rr, p);
			}else if (  le->leFlags & LEF_PUFF_SCALE_DOWN  ) {
				VectorMA (p, -sizedown, ru, p);	
				VectorMA (p, sizedown, rr, p);
			}else{
				VectorMA (p, -sizeup, ru, p);	
				VectorMA (p, sizeup, rr, p);
			}
		}else{
			if (  le->leFlags & LEF_PUFF_DONT_SCALE  ) {
				VectorMA (p, -re->radius, rlvup, p);	
				VectorMA (p, re->radius, rlvright, p);
			}else if (  le->leFlags & LEF_PUFF_SCALE_DOWN  ) {
				VectorMA (p, -sizedown, rlvup, p);	
				VectorMA (p, sizedown, rlvright, p);
			}else{
				VectorMA (p, -sizeup, rlvup, p);	
				VectorMA (p, sizeup, rlvright, p);
			}
		}
	}else{
		if (re->rotation) {
			if (  le->leFlags & LEF_PUFF_DONT_SCALE  ) {
				VectorMA (p,  re->radius, ru, p);	
				VectorMA (p, -re->radius, rr, p);
			}else if (  le->leFlags & LEF_PUFF_SCALE_DOWN  ) {
				VectorMA (p,  sizedown, ru, p);	
				VectorMA (p, -sizedown, rr, p);
			}else{
				VectorMA (p,  sizeup, ru, p);	
				VectorMA (p, -sizeup, rr, p);
			}
		}else{
			if (  le->leFlags & LEF_PUFF_DONT_SCALE  ) {
				VectorMA (p,  re->radius, re->axis[2], p);	
				VectorMA (p, -re->radius, re->axis[1], p);
			}else if (  le->leFlags & LEF_PUFF_SCALE_DOWN  ) {
				VectorMA (p,  sizedown, re->axis[2], p);	
				VectorMA (p, -sizedown, re->axis[1], p);
			}else{
				VectorMA (p,  sizeup, re->axis[2], p);	
				VectorMA (p, -sizeup, re->axis[1], p);
			}
		}
	}
	VectorCopy (p, verts[3].xyz);	
	verts[3].st[0] = 1;	
	verts[3].st[1] = 0;	
	verts[3].modulate[0] = re->shaderRGBA[0];	
	verts[3].modulate[1] = re->shaderRGBA[1];	
	verts[3].modulate[2] = re->shaderRGBA[2];	
	verts[3].modulate[3] = re->shaderRGBA[3];


	BG_EvaluateTrajectory( &le->pos, cg.time, re->origin );

	// if the view would be "inside" the sprite, kill the sprite
	// so it doesn't add too much overdraw
	VectorSubtract( re->origin, cg.refdef[le->view].vieworg, delta );
	len = VectorLength( delta );
	if ( len < le->radius ) {
		CG_FreeLocalEntity( le );
		return;
	}

		trap_R_AddPolyToScene( le->refEntity.customShader, 4, verts );
}


/*
===================
CG_AddScaleFade

For rocket smokes that hang in place, fade out, and are
removed if the view passes through them.
There are often many of these, so it needs to be simple.
===================
*/
static void CG_AddScaleFade( localEntity_t *le ) {
	refEntity_t	*re;
	float		c;
	vec3_t		delta;
	float		len;

	re = &le->refEntity;

	// fade / grow time
	c = ( le->endTime - cg.time ) * le->lifeRate;

	re->shaderRGBA[3] = 0xff * c * le->color[3];
	re->radius = le->radius * ( 1.0 - c ) + 8;

	// if the view would be "inside" the sprite, kill the sprite
	// so it doesn't add too much overdraw
	VectorSubtract( re->origin, cg.refdef[le->view].vieworg, delta );
	len = VectorLength( delta );
	if ( len < le->radius ) {
		CG_FreeLocalEntity( le );
		return;
	}

	trap_R_AddRefEntityToScene( re );
}


/*
=================
CG_AddFallScaleFade

This is just an optimized CG_AddMoveScaleFade
For blood mists that drift down, fade out, and are
removed if the view passes through them.
There are often 100+ of these, so it needs to be simple.
=================
*/
static void CG_AddFallScaleFade( localEntity_t *le ) {
	refEntity_t	*re;
	float		c;
	vec3_t		delta;
	float		len;

	re = &le->refEntity;

	// fade time
	c = ( le->endTime - cg.time ) * le->lifeRate;

	re->shaderRGBA[3] = 0xff * c * le->color[3];

	re->origin[2] = le->pos.trBase[2] - ( 1.0 - c ) * le->pos.trDelta[2];

	re->radius = le->radius * ( 1.0 - c ) + 16;

	// if the view would be "inside" the sprite, kill the sprite
	// so it doesn't add too much overdraw

	VectorSubtract( re->origin, cg.refdef[le->view].vieworg, delta );
	len = VectorLength( delta );
	if ( len < le->radius ) {
		CG_FreeLocalEntity( le );
		return;
	}

	trap_R_AddRefEntityToScene( re );
}



/*
================
CG_AddExplosion
================
*/
static void CG_AddExplosion( localEntity_t *ex ) {
	refEntity_t	*ent;

	ent = &ex->refEntity;

	ent->shaderRGBA[0] = 255 * ex->color[0];
		ent->shaderRGBA[1] = 255 * ex->color[1];
			ent->shaderRGBA[2] = 255 * ex->color[2];
				ent->shaderRGBA[3] = 255 * ex->color[3];
	// add the entity
	trap_R_AddRefEntityToScene(ent);

	// add the dlight
	if ( ex->light ) {
		float		light;
		float		light2;
		if(cg_QSLights.integer == 1){
			light = (float)( cg.time - ex->startTime ) / ( ex->endTime - ex->startTime );
			if ( light < 0.5 ) {
				light = 1.0;
			} else {
				light = 1.0 - ( light - 0.5 ) * 2;
			}
			light = ex->light * light;
			trap_R_AddAdditiveLightToScene( ent->origin, light-20, ex->lightColor[0], ex->lightColor[1], ex->lightColor[2] );
		// light test 
			light2 = (float)( cg.time - ex->startTime ) / ( ex->endTime - ex->startTime );
			if ( light2 < 0.5 ) {
				light2 = 1.0;
			} else {
				light2 = 1.0 - ( light2 - 0.5 ) * 2;
			}
			light2 = ex->light2 * light2;
			trap_R_AddLightToScene(ent->origin, light2+20, ex->lightColor2[0], ex->lightColor2[1], ex->lightColor2[2] );

			trap_R_AddLightToScene(ent->origin, light2, ex->lightColor2[0], ex->lightColor2[1], ex->lightColor2[2] );
		}else{
			light = (float)( cg.time - ex->startTime ) / ( ex->endTime - ex->startTime );
			if ( light < 0.5 ) {
				light = 1.0;
			} else {
				light = 1.0 - ( light - 0.5 ) * 2;
			}
			light = ex->light * light;
			trap_R_AddLightToScene(ent->origin, light, ex->lightColor[0], ex->lightColor[1], ex->lightColor[2] );

		}
	}
}

/*
================
CG_AddSpriteExplosion
================
*/
static void CG_AddSpriteExplosion( localEntity_t *le ) {
	refEntity_t	re;
	float c;

	re = le->refEntity;

	c = ( le->endTime - cg.time ) / ( float ) ( le->endTime - le->startTime );
	if ( c > 1 ) {
		c = 1.0;	// can happen during connection problems
	}

	re.shaderRGBA[0] = 0xff;
	re.shaderRGBA[1] = 0xff;
	re.shaderRGBA[2] = 0xff;
	re.shaderRGBA[3] = 0xff * c * 0.33;

	re.reType = RT_SPRITE;
//	re.radius = 42 * ( 1.0 - c ) + 30;
	re.radius = le->radius;

	trap_R_AddRefEntityToScene( &re );

	// add the dlight
	if ( le->light ) {
		float		light;
		float		light2;

		if(cg_QSLights.integer == 1){
			light = (float)( cg.time - le->startTime ) / ( le->endTime - le->startTime );
			if ( light < 0.5 ) {
				light = 1.0;
			} else {
				light = 1.0 - ( light - 0.5 ) * 2;
			}
			light = le->light * light;
			trap_R_AddAdditiveLightToScene( re.origin, light-20, le->lightColor[0], le->lightColor[1], le->lightColor[2] );
		// light test

			light2 = (float)( cg.time - le->startTime ) / ( le->endTime - le->startTime );
			if ( light2 < 0.5 ) {
				light2 = 1.0;
			} else {
				light2 = 1.0 - ( light2 - 0.5 ) * 2;
			}
			light2 = le->light2 * light2;
			trap_R_AddLightToScene(re.origin, light2+20, le->lightColor2[0], le->lightColor2[1], le->lightColor2[2] );

			trap_R_AddLightToScene(re.origin, light2, le->lightColor2[0], le->lightColor2[1], le->lightColor2[2] );
		}else{
			light = (float)( cg.time - le->startTime ) / ( le->endTime - le->startTime );
			if ( light < 0.5 ) {
				light = 1.0;
			} else {
				light = 1.0 - ( light - 0.5 ) * 2;
			}
			light = le->light * light;
			trap_R_AddLightToScene(re.origin, light, le->lightColor[0], le->lightColor[1], le->lightColor[2] );
		}
	}

}

/*
====================
CG_AddRailRings
====================
*/
void CG_AddRailRings( localEntity_t *le ) {
	refEntity_t	*re;
	refEntity_t plasmaring;
	float		c;
	int			t;

	re = &le->refEntity;

	t = cg.time - le->startTime;

	if (t > PL_SHOCKWAVE_STARTTIME && t < PL_SHOCKWAVE_ENDTIME) {

		memset(&plasmaring, 0, sizeof(plasmaring));
		plasmaring.hModel = cgs.media.PlasmaWave;
		plasmaring.reType = RT_MODEL;
		plasmaring.shaderTime = re->shaderTime;
		plasmaring.customShader = re->customShader;
		VectorCopy(re->origin, plasmaring.origin);

		c = (float)(t - PL_SHOCKWAVE_STARTTIME) / (float)(PL_SHOCKWAVE_ENDTIME - PL_SHOCKWAVE_STARTTIME);
		VectorScale( re->axis[0], c * 0.3 / -1, plasmaring.axis[0] );
		VectorScale( re->axis[1], c * 0.3 / -1, plasmaring.axis[1] );
		VectorScale( re->axis[2], c * 0.3 / -1, plasmaring.axis[2] );
		plasmaring.nonNormalizedAxes = qtrue;

		if (t > PL_SHOCKWAVEFADE_STARTTIME) {
			c = (float)(t - PL_SHOCKWAVEFADE_STARTTIME) / (float)(PL_SHOCKWAVE_ENDTIME - PL_SHOCKWAVEFADE_STARTTIME);
		}
		else {
			c = 0;
		}
		c *= 0xff;
		plasmaring.shaderRGBA[0] = 0xff - c;
		plasmaring.shaderRGBA[1] = 0xff - c;
		plasmaring.shaderRGBA[2] = 0xff - c;
		plasmaring.shaderRGBA[3] = 0xff - c;

		trap_R_AddRefEntityToScene( &plasmaring );
	}

}


/*
====================
CG_AddKamikaze
====================
*/
void CG_AddKamikaze( localEntity_t *le ) {
	refEntity_t	*re;
	refEntity_t shockwave;
	float		c;
	vec3_t		test, axis[3];
	int			t;

	re = &le->refEntity;

	t = cg.time - le->startTime;
	VectorClear( test );
	AnglesToAxis( test, axis );

	if (t > KAMI_SHOCKWAVE_STARTTIME && t < KAMI_SHOCKWAVE_ENDTIME) {
		if (!(le->leFlags & LEF_SOUND1)) {
			trap_S_StartLocalSound(cgs.media.kamikazeExplodeSound, CHAN_AUTO);
			le->leFlags |= LEF_SOUND1;
		}
		// 1st kamikaze shockwave
		memset(&shockwave, 0, sizeof(shockwave));
		shockwave.hModel = cgs.media.kamikazeShockWave;
		shockwave.reType = RT_MODEL;
		shockwave.shaderTime = re->shaderTime;
		VectorCopy(re->origin, shockwave.origin);

		c = (float)(t - KAMI_SHOCKWAVE_STARTTIME) / (float)(KAMI_SHOCKWAVE_ENDTIME - KAMI_SHOCKWAVE_STARTTIME);
		VectorScale( axis[0], c * KAMI_SHOCKWAVE_MAXRADIUS / KAMI_SHOCKWAVEMODEL_RADIUS, shockwave.axis[0] );
		VectorScale( axis[1], c * KAMI_SHOCKWAVE_MAXRADIUS / KAMI_SHOCKWAVEMODEL_RADIUS, shockwave.axis[1] );
		VectorScale( axis[2], c * KAMI_SHOCKWAVE_MAXRADIUS / KAMI_SHOCKWAVEMODEL_RADIUS, shockwave.axis[2] );
		shockwave.nonNormalizedAxes = qtrue;

		if (t > KAMI_SHOCKWAVEFADE_STARTTIME) {
			c = (float)(t - KAMI_SHOCKWAVEFADE_STARTTIME) / (float)(KAMI_SHOCKWAVE_ENDTIME - KAMI_SHOCKWAVEFADE_STARTTIME);
		}
		else {
			c = 0;
		}
		c *= 0xff;
		shockwave.shaderRGBA[0] = 0xff - c;
		shockwave.shaderRGBA[1] = 0xff - c;
		shockwave.shaderRGBA[2] = 0xff - c;
		shockwave.shaderRGBA[3] = 0xff - c;

		trap_R_AddRefEntityToScene( &shockwave );
	}

	if (t > KAMI_EXPLODE_STARTTIME && t < KAMI_IMPLODE_ENDTIME) {
		// explosion and implosion
		c = ( le->endTime - cg.time ) * le->lifeRate;
		c *= 0xff;
		re->shaderRGBA[0] = le->color[0] * c;
		re->shaderRGBA[1] = le->color[1] * c;
		re->shaderRGBA[2] = le->color[2] * c;
		re->shaderRGBA[3] = le->color[3] * c;

		if( t < KAMI_IMPLODE_STARTTIME ) {
			c = (float)(t - KAMI_EXPLODE_STARTTIME) / (float)(KAMI_IMPLODE_STARTTIME - KAMI_EXPLODE_STARTTIME);
		}
		else {
			if (!(le->leFlags & LEF_SOUND2)) {
				trap_S_StartLocalSound(cgs.media.kamikazeImplodeSound, CHAN_AUTO);
				le->leFlags |= LEF_SOUND2;
			}
			c = (float)(KAMI_IMPLODE_ENDTIME - t) / (float) (KAMI_IMPLODE_ENDTIME - KAMI_IMPLODE_STARTTIME);
		}
		VectorScale( axis[0], c * KAMI_BOOMSPHERE_MAXRADIUS / KAMI_BOOMSPHEREMODEL_RADIUS, re->axis[0] );
		VectorScale( axis[1], c * KAMI_BOOMSPHERE_MAXRADIUS / KAMI_BOOMSPHEREMODEL_RADIUS, re->axis[1] );
		VectorScale( axis[2], c * KAMI_BOOMSPHERE_MAXRADIUS / KAMI_BOOMSPHEREMODEL_RADIUS, re->axis[2] );
		re->nonNormalizedAxes = qtrue;

		trap_R_AddRefEntityToScene( re );
		// add the dlight
		trap_R_AddLightToScene( re->origin, c * 1000.0, 1.0, 1.0, c );
	}

	if (t > KAMI_SHOCKWAVE2_STARTTIME && t < KAMI_SHOCKWAVE2_ENDTIME) {
		// 2nd kamikaze shockwave
		if (le->angles.trBase[0] == 0 &&
			le->angles.trBase[1] == 0 &&
			le->angles.trBase[2] == 0) {
			le->angles.trBase[0] = random() * 360;
			le->angles.trBase[1] = random() * 360;
			le->angles.trBase[2] = random() * 360;
		}
		else {
			c = 0;
		}
		memset(&shockwave, 0, sizeof(shockwave));
		shockwave.hModel = cgs.media.kamikazeShockWave;
		shockwave.reType = RT_MODEL;
		shockwave.shaderTime = re->shaderTime;
		VectorCopy(re->origin, shockwave.origin);

		test[0] = le->angles.trBase[0];
		test[1] = le->angles.trBase[1];
		test[2] = le->angles.trBase[2];
		AnglesToAxis( test, axis );

		c = (float)(t - KAMI_SHOCKWAVE2_STARTTIME) / (float)(KAMI_SHOCKWAVE2_ENDTIME - KAMI_SHOCKWAVE2_STARTTIME);
		VectorScale( axis[0], c * KAMI_SHOCKWAVE2_MAXRADIUS / KAMI_SHOCKWAVEMODEL_RADIUS, shockwave.axis[0] );
		VectorScale( axis[1], c * KAMI_SHOCKWAVE2_MAXRADIUS / KAMI_SHOCKWAVEMODEL_RADIUS, shockwave.axis[1] );
		VectorScale( axis[2], c * KAMI_SHOCKWAVE2_MAXRADIUS / KAMI_SHOCKWAVEMODEL_RADIUS, shockwave.axis[2] );
		shockwave.nonNormalizedAxes = qtrue;

		if (t > KAMI_SHOCKWAVE2FADE_STARTTIME) {
			c = (float)(t - KAMI_SHOCKWAVE2FADE_STARTTIME) / (float)(KAMI_SHOCKWAVE2_ENDTIME - KAMI_SHOCKWAVE2FADE_STARTTIME);
		}
		else {
			c = 0;
		}
		c *= 0xff;
		shockwave.shaderRGBA[0] = 0xff - c;
		shockwave.shaderRGBA[1] = 0xff - c;
		shockwave.shaderRGBA[2] = 0xff - c;
		shockwave.shaderRGBA[3] = 0xff - c;

		trap_R_AddRefEntityToScene( &shockwave );
	}
	if (t > KAMI_SHOCKWAVE3_STARTTIME && t < KAMI_SHOCKWAVE3_ENDTIME) {
		// 3rd kamikaze shockwave
		if (le->angles.trBase[0] == 0 &&
			le->angles.trBase[1] == 0 &&
			le->angles.trBase[2] == 0) {
			le->angles.trBase[0] = random() * 400;
			le->angles.trBase[1] = random() * 900;
			le->angles.trBase[2] = random() * 700;
		}
		else {
			c = 0;
		}
		memset(&shockwave, 0, sizeof(shockwave));
		shockwave.hModel = cgs.media.kamikazeShockWave;
		shockwave.reType = RT_MODEL;
		shockwave.shaderTime = re->shaderTime;
		VectorCopy(re->origin, shockwave.origin);

		test[0] = le->angles.trBase[1];
		test[1] = le->angles.trBase[2];
		test[2] = le->angles.trBase[0];
		AnglesToAxis( test, axis );

		c = (float)(t - KAMI_SHOCKWAVE3_STARTTIME) / (float)(KAMI_SHOCKWAVE3_ENDTIME - KAMI_SHOCKWAVE3_STARTTIME);
		VectorScale( axis[0], c * KAMI_SHOCKWAVE3_MAXRADIUS / KAMI_SHOCKWAVEMODEL_RADIUS, shockwave.axis[0] );
		VectorScale( axis[1], c * KAMI_SHOCKWAVE3_MAXRADIUS / KAMI_SHOCKWAVEMODEL_RADIUS, shockwave.axis[1] );
		VectorScale( axis[2], c * KAMI_SHOCKWAVE3_MAXRADIUS / KAMI_SHOCKWAVEMODEL_RADIUS, shockwave.axis[2] );
		shockwave.nonNormalizedAxes = qtrue;

		if (t > KAMI_SHOCKWAVE3FADE_STARTTIME) {
			c = (float)(t - KAMI_SHOCKWAVE3FADE_STARTTIME) / (float)(KAMI_SHOCKWAVE3_ENDTIME - KAMI_SHOCKWAVE3FADE_STARTTIME);
		}
		else {
			c = 0;
		}
		c *= 0xff;
		shockwave.shaderRGBA[0] = 0xff - c;
		shockwave.shaderRGBA[1] = 0xff - c;
		shockwave.shaderRGBA[2] = 0xff - c;
		shockwave.shaderRGBA[3] = 0xff - c;

		trap_R_AddRefEntityToScene( &shockwave );
	}
}

/*
==================
CG_AddRLExplosion
==================
*/
static void CG_AddRLExplosion( localEntity_t *le ) {
	refEntity_t	re;
	float c,cl;
	int i, j;
	float		ratio;
	float		time, time2;


	time = cg.time - le->startTime;
	time2 = le->endTime - le->startTime;
	ratio = time / time2;

	re = le->refEntity;

	i = le->refEntity.shaderAnim;
	j = (int)floor(ratio * shaderAnimCountsw[le->refEntity.shaderAnim]);
	re.customShader = shaderAnimsw[i][j];
	re.shaderTime = le->refEntity.shaderTime;

	c = ( le->endTime - cg.time ) / ( float ) ( le->endTime - le->startTime );
	if ( c > 1 ) {
		c = 1.0;	// can happen during connection problems
	}

	re.shaderRGBA[0] = 0xff;
	re.shaderRGBA[1] = 0xff;
	re.shaderRGBA[2] = 0xff;
	re.shaderRGBA[3] = 0xff * c * 0.33;

	re.reType = RT_SPRITE;
	re.radius = 30 * ( 1.0 - c ) + 30;

	trap_R_AddRefEntityToScene( &re );

	// add the dlight
	if ( le->light ) {
		float		light;
		float		light2;
		vec3_t		color,color2;

		if(cg_QSLights.integer== 1){
			if(cg.time > le->startTime + 600){
				cl = ( le->endTime - cg.time ) * le->lifeRate;
			}else{
				cl = 1.0;
			}

			light = (float)( cg.time - le->startTime ) / ( le->endTime - le->startTime );
			if ( light < 0.5 ) {
				light = 1.0;
			} else {
				light = 1.0 - ( light - 0.5 ) * 2;
			}
			light = le->light * light;
			color[0] = le->lightColor[0] * cl;
			color[1] = le->lightColor[1] * cl;
			color[2] = le->lightColor[2] * cl;
			if(color[0] < 0){
				color[0] = 0;
			}
			if(color[1] < 0){
				color[1] = 0;
			}
			if(color[2] < 0){
				color[2] = 0;
			}

				trap_R_AddAdditiveLightToScene( re.origin, le->light-20, color[0], color[1], color[2] );

		// light test

			color2[0] = le->lightColor2[0] * cl;
			color2[1] = le->lightColor2[1] * cl;
			color2[2] = le->lightColor2[2] * cl;
			if(color2[0] < 0){
				color2[0] = 0;
			}
			if(color2[1] < 0){
				color2[1] = 0;
			}
			if(color2[2] < 0){
				color2[2] = 0;
			}

			light2 = (float)( cg.time - le->startTime ) / ( le->endTime - le->startTime );
			if ( light2 < 0.5 ) {
				light2 = 1.0;
			} else {
				light2 = 1.0 - ( light2 - 0.5 ) * 2;
			}
			light2 = le->light2 * light2;

				trap_R_AddLightToScene(re.origin, le->light2, color2[0], color2[1], color2[2] );
				trap_R_AddLightToScene(re.origin, le->light2+20, color2[0], color2[1], color2[2] );

		}else{
			light = (float)( cg.time - le->startTime ) / ( le->endTime - le->startTime );
			if ( light < 0.5 ) {
				light = 1.0;
			} else {
				light = 1.0 - ( light - 0.5 ) * 2;
			}
			light = le->light * light;
			trap_R_AddLightToScene(re.origin, light, le->lightColor[0], le->lightColor[1], le->lightColor[2] );
		}
	}



}
/*
==================
CG_AddRocketRing
==================
*/
void CG_AddRocketRing( localEntity_t *le ) {
	refEntity_t	*re;
//	refEntity_t	*re2;
	refEntity_t shockwave;
	float		c;
	int i, j;
	float		ratio;
	float		time, time2;


	time = cg.time - le->startTime;
	time2 = le->endTime - le->startTime;
	ratio = time / time2;


	re = &le->refEntity;

	if (time > KAMI_SHOCKWAVE_STARTTIME4 && time < KAMI_SHOCKWAVE_ENDTIME4) {


		// 1st kamikaze shockwave
		memset(&shockwave, 0, sizeof(shockwave));
		shockwave.hModel = cgs.media.rocketShockWave;
		shockwave.reType = RT_MODEL;
		i = re->shaderAnim;
		j = (int)floor(ratio * shaderAnimCountsw[re->shaderAnim]);
		shockwave.customShader = cgs.media.rocketShockWaveShader;//shaderAnimsw[i][j];
		shockwave.shaderTime = re->shaderTime;
		VectorCopy(re->origin, shockwave.origin);

		VectorCopy(re->axis[0],shockwave.axis[0]);
		VectorCopy(re->axis[1],shockwave.axis[1]);
		VectorCopy(re->axis[2],shockwave.axis[2]);
		c = (float)(time - KAMI_SHOCKWAVE_STARTTIME4) / (float)(KAMI_SHOCKWAVE_ENDTIME4 - KAMI_SHOCKWAVE_STARTTIME4);
		VectorScale( re->axis[0], c * KAMI_SHOCKWAVE_MAXRADIUS4 / KAMI_SHOCKWAVEMODEL_RADIUS4, shockwave.axis[0] );
		VectorScale( re->axis[1], c * KAMI_SHOCKWAVE_MAXRADIUS4 / KAMI_SHOCKWAVEMODEL_RADIUS4, shockwave.axis[1] );
		VectorScale( re->axis[2], c * KAMI_SHOCKWAVE_MAXRADIUS4 / KAMI_SHOCKWAVEMODEL_RADIUS4, shockwave.axis[2] );
	//	shockwave.nonNormalizedAxes = qfalse;

		if (time > KAMI_SHOCKWAVEFADE_STARTTIME4) {
			c = (float)(time - KAMI_SHOCKWAVEFADE_STARTTIME4) / (float)(KAMI_SHOCKWAVE_ENDTIME4 - KAMI_SHOCKWAVEFADE_STARTTIME4);
		}
		else {
			c = 0;
		}
		c *= 0xff;
		shockwave.shaderRGBA[0] = 0xff * ( le->endTime - cg.time ) * le->lifeRate;
		shockwave.shaderRGBA[1] = 0xff * ( le->endTime - cg.time ) * le->lifeRate;
		shockwave.shaderRGBA[2] = 0xff * ( le->endTime - cg.time ) * le->lifeRate;
		shockwave.shaderRGBA[3] = 0xff * ( le->endTime - cg.time ) * le->lifeRate;

		trap_R_AddRefEntityToScene( &shockwave );

	}
	if (time > KAMI_SHOCKWAVE_STARTTIME4 && time < KAMI_SHOCKWAVE_ENDTIME4) {
		// 1st kamikaze shockwave
		memset(&shockwave, 0, sizeof(shockwave));
		shockwave.hModel = cgs.media.rocketShockRing;
		shockwave.reType = RT_MODEL;
		shockwave.shaderTime = re->shaderTime;
		VectorCopy(re->origin, shockwave.origin);

		VectorCopy(re->axis[0],shockwave.axis[0]);
		VectorCopy(re->axis[1],shockwave.axis[1]);
		VectorCopy(re->axis[2],shockwave.axis[2]);

		c = (float)(time - KAMI_SHOCKWAVE_STARTTIME4) / (float)(KAMI_SHOCKWAVE_ENDTIME4 - KAMI_SHOCKWAVE_STARTTIME4);
		VectorScale( re->axis[0], c * KAMI_SHOCKWAVE_MAXRADIUS4 / KAMI_SHOCKWAVEMODEL_RADIUS4, shockwave.axis[0] );
		VectorScale( re->axis[1], c * KAMI_SHOCKWAVE_MAXRADIUS4 / KAMI_SHOCKWAVEMODEL_RADIUS4, shockwave.axis[1] );
		VectorScale( re->axis[2], c * KAMI_SHOCKWAVE_MAXRADIUS4 / KAMI_SHOCKWAVEMODEL_RADIUS4, shockwave.axis[2] );
	//	shockwave.nonNormalizedAxes = qfalse;

		if (time > KAMI_SHOCKWAVEFADE_STARTTIME4) {
			c = (float)(time - KAMI_SHOCKWAVEFADE_STARTTIME4) / (float)(KAMI_SHOCKWAVE_ENDTIME4 - KAMI_SHOCKWAVEFADE_STARTTIME4);
		}
		else {
			c = 0;
		}
		c *= 0xff;
		shockwave.shaderRGBA[0] = 0xff * ( le->endTime - cg.time ) * le->lifeRate;
		shockwave.shaderRGBA[1] = 0xff * ( le->endTime - cg.time ) * le->lifeRate;
		shockwave.shaderRGBA[2] = 0xff * ( le->endTime - cg.time ) * le->lifeRate;
		shockwave.shaderRGBA[3] = 0xff * ( le->endTime - cg.time ) * le->lifeRate;

		trap_R_AddRefEntityToScene( &shockwave );

	}
}

/*
==================
CG_AddRocketQSphere
==================
*/
void CG_AddRocketQSphere( localEntity_t *le ) {
	refEntity_t	*re;
	refEntity_t shockwave;
	float		c;
	int i, j;
	float		ratio;
	float		time, time2;


	time = cg.time - le->startTime;
	time2 = le->endTime - le->startTime;
	ratio = time / time2;

	re = &le->refEntity;


//	if (time > le->startTime && time < le->endTime) {

		// 1st kamikaze shockwave
		memset(&shockwave, 0, sizeof(shockwave));
		shockwave.hModel = cgs.media.quadSphereModel;
		shockwave.reType = RT_MODEL;
		i = re->shaderAnim;
		j = (int)floor(ratio * shaderAnimCountsw[re->shaderAnim]);
		shockwave.customShader = re->customShader;
		shockwave.shaderTime = re->shaderTime;
		VectorCopy(re->origin, shockwave.origin);

		if(re->sparklesT == 0){
			c = (float)(time - 0) / (float)(800 - 0);
			VectorScale( re->axis[0], c * 1.8f / 1.7f, shockwave.axis[0] );
			VectorScale( re->axis[1], c * 1.8f / 1.7f, shockwave.axis[1] );
			VectorScale( re->axis[2], c * 1.8f / 1.7f, shockwave.axis[2] );
		}else{
			c = (float)(time - 0) / (float)(1600 - 0);
			VectorScale( re->axis[0], c * 5.9f / 4.8f, shockwave.axis[0] );
			VectorScale( re->axis[1], c * 5.9f / 4.8f, shockwave.axis[1] );
			VectorScale( re->axis[2], c * 5.9f / 4.8f, shockwave.axis[2] );
		}
		shockwave.nonNormalizedAxes = qfalse;

		c = ( le->endTime - cg.time ) / ( float ) ( le->endTime - le->startTime );
		shockwave.shaderRGBA[0] = (unsigned char) 255 * c * 1.0f;
		shockwave.shaderRGBA[1] = (unsigned char) 255 * c * 1.0f;
		shockwave.shaderRGBA[2] = (unsigned char) 255 * c * 1.0f;
		shockwave.shaderRGBA[3] = (unsigned char) 255 * c * 1.0f;



		trap_R_AddRefEntityToScene( &shockwave );
//	}
//	if (time > le->startTime && time < le->endTime) {

		// 1st kamikaze shockwave
		memset(&shockwave, 0, sizeof(shockwave));
		shockwave.hModel = cgs.media.quadSphereModel;
		shockwave.reType = RT_MODEL;
		i = re->shaderAnim;
		j = (int)floor(ratio * shaderAnimCountsw[re->shaderAnim]);
		shockwave.customShader = re->customShader;
		shockwave.shaderTime = re->shaderTime;
		VectorCopy(re->origin, shockwave.origin);

		if(re->sparklesT == 0){
			c = (float)(time - 0) / (float)(800 - 0);
			VectorScale( re->axis[0], c * 1.7f / 1.6f, shockwave.axis[0] );
			VectorScale( re->axis[1], c * 1.7f / 1.6f, shockwave.axis[1] );
			VectorScale( re->axis[2], c * 1.7f / 1.6f, shockwave.axis[2] );
		}else{
			c = (float)(time - 0) / (float)(1600 - 0);
			VectorScale( re->axis[0], c * 5.8f / 4.7f, shockwave.axis[0] );
			VectorScale( re->axis[1], c * 5.8f / 4.7f, shockwave.axis[1] );
			VectorScale( re->axis[2], c * 5.8f / 4.7f, shockwave.axis[2] );
		}
		shockwave.nonNormalizedAxes = qfalse;

		c = ( le->endTime - cg.time ) / ( float ) ( le->endTime - le->startTime );
		shockwave.shaderRGBA[0] = (unsigned char) 255 * c * 1.0f;
		shockwave.shaderRGBA[1] = (unsigned char) 255 * c * 1.0f;
		shockwave.shaderRGBA[2] = (unsigned char) 255 * c * 1.0f;
		shockwave.shaderRGBA[3] = (unsigned char) 255 * c * 1.0f;

		trap_R_AddRefEntityToScene( &shockwave );
//	}

}

void CG_AddPlasmaQSphere( localEntity_t *le ) {
	refEntity_t	*re;
	refEntity_t shockwave;
	float		c;
	int i, j;
	float		ratio;
	float		time, time2;


	time = cg.time - le->startTime;
	time2 = le->endTime - le->startTime;
	ratio = time / time2;

	re = &le->refEntity;


	if (time > 0 && time < 300) {

		// 1st kamikaze shockwave
		memset(&shockwave, 0, sizeof(shockwave));
		shockwave.hModel = cgs.media.quadSphereModel;
		shockwave.reType = RT_MODEL;
		i = re->shaderAnim;
		j = (int)floor(ratio * shaderAnimCountsw[re->shaderAnim]);
		shockwave.customShader = re->customShader;
		shockwave.shaderTime = re->shaderTime;
		VectorCopy(re->origin, shockwave.origin);

		c = (float)(time - 0) / (float)(700 - 0);
		if(re->sparklesT == 0){
			VectorScale( re->axis[0], c * 5.9f / 4.8f, shockwave.axis[0] );
			VectorScale( re->axis[1], c * 5.9f / 4.8f, shockwave.axis[1] );
			VectorScale( re->axis[2], c * 5.9f / 4.8f, shockwave.axis[2] );
		}else{
			VectorScale( re->axis[0], c * 7.7f / 4.8f, shockwave.axis[0] );
			VectorScale( re->axis[1], c * 7.7f / 4.8f, shockwave.axis[1] );
			VectorScale( re->axis[2], c * 7.7f / 4.8f, shockwave.axis[2] );
		}
		shockwave.nonNormalizedAxes = qfalse;

		c = ( le->endTime - cg.time ) / ( float ) ( le->endTime - le->startTime );
		shockwave.shaderRGBA[0] = (unsigned char) 255 * c * 1.0f;
		shockwave.shaderRGBA[1] = (unsigned char) 255 * c * 1.0f;
		shockwave.shaderRGBA[2] = (unsigned char) 255 * c * 1.0f;
		shockwave.shaderRGBA[3] = (unsigned char) 255 * c * 1.0f;

		trap_R_AddRefEntityToScene( &shockwave );
	}
	if (time > 0 && time < 300) {

		// 1st kamikaze shockwave
		memset(&shockwave, 0, sizeof(shockwave));
		shockwave.hModel = cgs.media.quadSphereModel;
		shockwave.reType = RT_MODEL;
		i = re->shaderAnim;
		j = (int)floor(ratio * shaderAnimCountsw[re->shaderAnim]);
		shockwave.customShader = re->customShader;
		shockwave.shaderTime = re->shaderTime;
		VectorCopy(re->origin, shockwave.origin);

		c = (float)(time - 0) / (float)(700 - 0);
		if(re->sparklesT == 0){
			VectorScale( re->axis[0], c * 5.8f / 4.7f, shockwave.axis[0] );
			VectorScale( re->axis[1], c * 5.8f / 4.7f, shockwave.axis[1] );
			VectorScale( re->axis[2], c * 5.8f / 4.7f, shockwave.axis[2] );
		}else{
			VectorScale( re->axis[0], c * 7.6f / 4.7f, shockwave.axis[0] );
			VectorScale( re->axis[1], c * 7.6f / 4.7f, shockwave.axis[1] );
			VectorScale( re->axis[2], c * 7.6f / 4.7f, shockwave.axis[2] );
		}
		shockwave.nonNormalizedAxes = qfalse;

		c = ( le->endTime - cg.time ) / ( float ) ( le->endTime - le->startTime );
		shockwave.shaderRGBA[0] = (unsigned char) 255 * c * 1.0f;
		shockwave.shaderRGBA[1] = (unsigned char) 255 * c * 1.0f;
		shockwave.shaderRGBA[2] = (unsigned char) 255 * c * 1.0f;
		shockwave.shaderRGBA[3] = (unsigned char) 255 * c * 1.0f;

		trap_R_AddRefEntityToScene( &shockwave );
	}

}

void CG_QSphereEffect( vec3_t org , vec3_t dir,int team,int duration ,int weapon,qboolean quad) {
	localEntity_t	*le;
	refEntity_t		*re;
	vec3_t		forward, right, up;
	vec3_t	angles;


	if (duration < 0) {
		duration *= -1;
	}
	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	switch ( weapon ) {
	case WP_LIGHTNING:
		break;
	case WP_FLAMETHROWER:
		break;
	case WP_GRENADE_LAUNCHER:
		if(quad){
			le->leType = LE_QUADSPHERE;
		}else{
			le->leType = LE_PQUADSPHERE;
		}
		le->entNumber = le - cg_freeLocalEntities;
		break;
	case WP_ROCKET_LAUNCHER:
		le->leType = LE_QUADSPHERE;
		le->entNumber = le - cg_freeLocalEntities;
		break;
	case WP_RAILGUN:
		break;
	case WP_IRAILGUN:
		break;
	case WP_PLASMAGUN:
		le->leType = LE_PQUADSPHERE;
		le->entNumber = le - cg_freeLocalEntities;
		break;
	case WP_BFG:
		break;
	case WP_SHOTGUN:
		break;
	case WP_MACHINEGUN:
		break;
	}


	le->startTime = cg.time ;
	le->endTime = cg.time + duration;
	le->lifeRate = 1.0 / ( le->endTime - le->startTime );
	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0;
	if(quad){
		if (team == TEAM_RED){
			re->customShader = cgs.media.quadRexpShader;
		}else{
			re->customShader = cgs.media.quadexpShader;
		}
	}else{
		re->sparklesT = 1;
		re->customShader = cgs.media.rlfireexpShader;
	}

	vectoangles (dir, angles);
	AngleVectors( angles, forward, right, up );
	VectorCopy(forward ,re->axis[2]);
	VectorCopy( right,re->axis[0]);
	VectorCopy( up,re->axis[1]);


	re->shaderTime = cg.time / 1000.0f;
	VectorCopy( org, re->origin);
}

/*
==================
CG_RingEffect
==================
*/
void CG_RingEffect( vec3_t org , vec3_t dir,char *animStr,int duration,qboolean sprite) {
	localEntity_t	*le;
	refEntity_t		*re;
	int anim;
	vec3_t		forward, right, up;
	vec3_t	angles;

	if (animStr < (char *)10)
		CG_Error( "CG_ParticleExplosion: animStr is probably an index rather than a string" );

	// find the animation string
	for (anim=0; shaderAnimNamesw[anim]; anim++) {
		if (!Q_stricmp( animStr, shaderAnimNamesw[anim] ))
			break;
	}
	if (!shaderAnimNamesw[anim]) {
		CG_Error("CG_ParticleExplosion: unknown animation string: %s\n", animStr);
		return;
	}

	if (duration < 0) {
		duration *= -1;
	}
	le = CG_AllocLocalEntity();
	re = &le->refEntity;

		le->leType = LE_ROCKETRING;
	le->entNumber = le - cg_freeLocalEntities;

	le->startTime = cg.time ;
	le->endTime = cg.time + duration;
	le->lifeRate = 0.8 / ( le->endTime - le->startTime );
	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0;

	if(sprite){
		AngleVectors( cg.refdefViewAngles[0], forward, right, up );
		VectorCopy(forward ,re->axis[2]);
		VectorCopy( right,re->axis[1]);
		VectorCopy( up,re->axis[0]);
	}else{
		vectoangles (dir, angles);
		AngleVectors( angles, forward, right, up );
		VectorCopy(forward ,re->axis[2]);
		VectorCopy( right,re->axis[1]);
		VectorCopy( up,re->axis[0]);
	}

	re->shaderAnim = anim;


	re->shaderTime = cg.time / 200.0f;
	VectorCopy( org, re->origin);
}

/*
///////////////////////////////////
CG_RocketExplosion
///////////////////////////////////
*/

void CG_RocketExplosion( vec3_t dir, vec3_t org ,char *animStr,int duration, float light, vec3_t lightColor, float light2, vec3_t lightColor2) {
	localEntity_t	*le;
	refEntity_t		*re;
	int anim;
	vec3_t			tmpVec, newOrigin;
	float			ang;

	if (animStr < (char *)10)
		CG_Error( "CG_RocketExplosion: animStr is probably an index rather than a string" );

	// find the animation string
	for (anim=0; shaderAnimNamesw[anim]; anim++) {
		if (!Q_stricmp( animStr, shaderAnimNamesw[anim] ))
			break;
	}
	if (!shaderAnimNamesw[anim]) {
		CG_Error("CG_ParticleExplosion: unknown animation string: %s\n", animStr);
		return;
	}

	if (duration < 0) {
		duration *= -1;
	}
	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_ROCKETEXP;
	le->entNumber = le - cg_freeLocalEntities;
	le->startTime = cg.time ;
	le->endTime = cg.time + duration;
	le->lifeRate = 1.0 / ( le->endTime - le->startTime );
	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0;
	if(cg_QSLights.integer == 1){
		le->light2 = light2;
		VectorCopy(lightColor2,le->lightColor2);
		
		le->light = light;
	}else{
		le->light = 300;
	}
	VectorCopy(lightColor,le->lightColor);

	ang = rand() % 360;
	VectorCopy( dir, re->axis[0] );
	RotateAroundDirection( re->axis, ang );
	VectorCopy( dir, le->pos.trDelta); 
	re->shaderAnim = anim;

	re->shaderTime = cg.time / 200.0f;
	re->hModel = cgs.media.dishFlashModel;
	re->rotation = rand() % 360;
	VectorScale( dir, 16, tmpVec );
	VectorAdd( tmpVec, org, newOrigin );

	VectorCopy( newOrigin, re->origin );
	VectorCopy( newOrigin, re->oldorigin );

	CG_ImpactMark( cgs.media.burnMarkShader, org, dir, random()*360, 1,1,1,1, qtrue, 64, qfalse );

	trap_S_StartSound( org, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.sfx_rockexp );
	trap_S_StartSound( org, -1, CHAN_VOICE, cgs.media.sfx_rockexp );
	//trap_S_StartSound( org, -1, CHAN_AUTO, cgs.media.sfx_rockexp );
//	if( CG_RadiusDamage( re.origin, le, 80, 120, 
//		le) ) {
	//	LocalEnt_Damage( &cg_activeLocalEntities, le,
	//		   dir, org, 80);
//	}

}

/*
///////////////////////////////////
CG_GrenadeExplosion
///////////////////////////////////
*/

void CG_GrenadeExplosion( vec3_t dir, vec3_t org ,char *animStr,int duration, float light, vec3_t lightColor, float light2, vec3_t lightColor2) {
	localEntity_t	*le;
	refEntity_t		*re;
	int anim;
	vec3_t			tmpVec, newOrigin;
	float			ang;

	if (animStr < (char *)10)
		CG_Error( "CG_RocketExplosion: animStr is probably an index rather than a string" );

	// find the animation string
	for (anim=0; shaderAnimNamesw[anim]; anim++) {
		if (!Q_stricmp( animStr, shaderAnimNamesw[anim] ))
			break;
	}
	if (!shaderAnimNamesw[anim]) {
		CG_Error("CG_ParticleExplosion: unknown animation string: %s\n", animStr);
		return;
	}

	if (duration < 0) {
		duration *= -1;
	}
	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_ROCKETEXP;
	le->entNumber = le - cg_freeLocalEntities;
	le->startTime = cg.time ;
	le->endTime = cg.time + duration;
	le->lifeRate = 1.0 / ( le->endTime - le->startTime );
	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0;
	if(cg_QSLights.integer == 1){
		le->light2 = light2;
		VectorCopy(lightColor2,le->lightColor2);
		
		le->light = light;
	}else{
		le->light = 300;
	}
	VectorCopy(lightColor,le->lightColor);

	ang = rand() % 360;
	VectorCopy( dir, re->axis[0] );
	RotateAroundDirection( re->axis, ang );
	VectorCopy( dir, le->pos.trDelta); 
	re->shaderAnim = anim;

	re->shaderTime = cg.time / 200.0f;
	re->hModel = cgs.media.dishFlashModel;
	re->rotation = rand() % 360;
	VectorScale( dir, 16, tmpVec );
	VectorAdd( tmpVec, org, newOrigin );

	VectorCopy( newOrigin, re->origin );
	VectorCopy( newOrigin, re->oldorigin );

	CG_ImpactMark( cgs.media.burnMarkShader, org, dir, random()*360, 1,1,1,1, qtrue, 64, qfalse );

	trap_S_StartSound( org, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.sfx_grenadeExp );
	trap_S_StartSound( org, -1, CHAN_VOICE, cgs.media.sfx_grenadeExp );
	//trap_S_StartSound( org, -1, CHAN_AUTO, cgs.media.sfx_rockexp );
//	if( CG_RadiusDamage( re.origin, le, 80, 120, 
//		le) ) {
	//	LocalEnt_Damage( &cg_activeLocalEntities, le,
	//		   dir, org, 80);
//	}

}

/*
==================
CG_AddRocketRing2
==================
*/
void CG_AddRocketRing2( localEntity_t *le ) {
	refEntity_t	*re;
	refEntity_t shockwave;
	float		c;
	int i, j;
	float		ratio;
	float		time, time2;

	time = cg.time - le->startTime;
	time2 = le->endTime - le->startTime;
	ratio = time / time2;

	re = &le->refEntity;

	if (time > KAMI_SHOCKWAVE_STARTTIME5 && time < KAMI_SHOCKWAVE_ENDTIME5) {

		// 1st kamikaze shockwave
		memset(&shockwave, 0, sizeof(shockwave));
		shockwave.hModel = cgs.media.rocketShockWave;
		shockwave.reType = RT_MODEL;
		i = re->shaderAnim;
		j = (int)floor(ratio * shaderAnimCountsw[re->shaderAnim]);
		shockwave.customShader = cgs.media.rocketShockWaveShader;//shaderAnimsw[i][j];
		shockwave.shaderTime = re->shaderTime;
		VectorCopy(re->origin, shockwave.origin);
		VectorCopy(re->axis[0],shockwave.axis[0]);
		VectorCopy(re->axis[1],shockwave.axis[1]);
		VectorCopy(re->axis[2],shockwave.axis[2]);

		c = (float)(time - KAMI_SHOCKWAVE_STARTTIME5) / (float)(KAMI_SHOCKWAVE_ENDTIME5 - KAMI_SHOCKWAVE_STARTTIME5);
		VectorScale( re->axis[0], c * KAMI_SHOCKWAVE_MAXRADIUS5 / KAMI_SHOCKWAVEMODEL_RADIUS5, shockwave.axis[0] );
		VectorScale( re->axis[1], c * KAMI_SHOCKWAVE_MAXRADIUS5 / KAMI_SHOCKWAVEMODEL_RADIUS5, shockwave.axis[1] );
		VectorScale( re->axis[2], c * KAMI_SHOCKWAVE_MAXRADIUS5 / KAMI_SHOCKWAVEMODEL_RADIUS5, shockwave.axis[2] );
	//	shockwave.nonNormalizedAxes = qfalse;

		c = ( le->endTime - cg.time ) / ( float ) ( le->endTime - le->startTime );
		shockwave.shaderRGBA[0] = (unsigned char) 255 * c * 1.0f;
		shockwave.shaderRGBA[1] = (unsigned char) 255 * c * 1.0f;
		shockwave.shaderRGBA[2] = (unsigned char) 255 * c * 1.0f;
		shockwave.shaderRGBA[3] = (unsigned char) 255 * c * 1.0f;

		trap_R_AddRefEntityToScene( &shockwave );
	}
		if (time > KAMI_SHOCKWAVE_STARTTIME5 && time < KAMI_SHOCKWAVE_ENDTIME5) {

		// 1st kamikaze shockwave
		memset(&shockwave, 0, sizeof(shockwave));
		shockwave.hModel = cgs.media.rocketShockRing;
		shockwave.reType = RT_MODEL;
		shockwave.shaderTime = re->shaderTime;
		VectorCopy(re->origin, shockwave.origin);
		VectorCopy(re->axis[0],shockwave.axis[0]);
		VectorCopy(re->axis[1],shockwave.axis[1]);
		VectorCopy(re->axis[2],shockwave.axis[2]);
		c = (float)(time - KAMI_SHOCKWAVE_STARTTIME5) / (float)(KAMI_SHOCKWAVE_ENDTIME5 - KAMI_SHOCKWAVE_STARTTIME5);
		VectorScale( re->axis[0], c * KAMI_SHOCKWAVE_MAXRADIUS5 / KAMI_SHOCKWAVEMODEL_RADIUS5, shockwave.axis[0] );
		VectorScale( re->axis[1], c * KAMI_SHOCKWAVE_MAXRADIUS5 / KAMI_SHOCKWAVEMODEL_RADIUS5, shockwave.axis[1] );
		VectorScale( re->axis[2], c * KAMI_SHOCKWAVE_MAXRADIUS5 / KAMI_SHOCKWAVEMODEL_RADIUS5, shockwave.axis[2] );
	//	shockwave.nonNormalizedAxes = qfalse;

		if (time > 200) {
			c = (float)(time - 200) / (float)(KAMI_SHOCKWAVE_ENDTIME5 - 200);
		}
		else {
			c = 0;
		}
		c *= 0xff;
		shockwave.shaderRGBA[0] = 0xff - c;
		shockwave.shaderRGBA[1] = 0xff - c;
		shockwave.shaderRGBA[2] = 0xff - c;
		shockwave.shaderRGBA[3] = 0xff - c;

		trap_R_AddRefEntityToScene( &shockwave );
	}
}
/*
==================
CG_RingEffect2
==================
*/
void CG_RingEffect2( vec3_t org , vec3_t dir,char *animStr,int duration,qboolean sprite) {
	localEntity_t	*le;
	refEntity_t		*re;
	int anim;
	vec3_t		forward, right, up;
	vec3_t		angles;


	if (animStr < (char *)10)
		CG_Error( "CG_ParticleExplosion: animStr is probably an index rather than a string" );

	// find the animation string
	for (anim=0; shaderAnimNamesw[anim]; anim++) {
		if (!Q_stricmp( animStr, shaderAnimNamesw[anim] ))
			break;
	}
	if (!shaderAnimNamesw[anim]) {
		CG_Error("CG_ParticleExplosion: unknown animation string: %s\n", animStr);
		return;
	}


	if (duration < 0) {
		duration *= -1;
	}
	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_ROCKETRING2;
	le->entNumber = le - cg_freeLocalEntities;
	le->startTime = cg.time ;
	le->endTime = cg.time + duration;
	le->lifeRate = 1.0 / ( le->endTime - le->startTime );

	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0;

	re->shaderAnim = anim;

	if(sprite){
		AngleVectors( cg.refdefViewAngles[0], forward, right, up );
		VectorCopy(cg.refdef[0].viewaxis[0] ,re->axis[2]);
		VectorCopy( cg.refdef[0].viewaxis[1],re->axis[1]);
		VectorCopy( cg.refdef[0].viewaxis[2],re->axis[0]);
	}else{
		vectoangles (dir, angles);
		AngleVectors( angles, forward, right, up );
		VectorCopy(forward ,re->axis[2]);
		VectorCopy( right,re->axis[1]);
		VectorCopy( up,re->axis[0]);
	}
	
	re->shaderTime = cg.time / 1000.0f;
	VectorCopy( org, re->origin);

}
/*
==================
CG_AddRailEXPRing
==================
*/
void CG_AddRailEXPRing( localEntity_t *le ) {
	refEntity_t	*re;
	refEntity_t shockwave;
	float		c;
	int i, j;
	float		ratio;
	float		time, time2;


	time = cg.time - le->startTime;
	time2 = le->endTime - le->startTime;
	ratio = time / time2;

	re = &le->refEntity;


	if (time > KAMI_SHOCKWAVE_STARTTIME4 && time < KAMI_SHOCKWAVE_ENDTIME4) {

		// 1st kamikaze shockwave
		memset(&shockwave, 0, sizeof(shockwave));
		shockwave.hModel = cgs.media.rocketShockWave;
		shockwave.reType = RT_MODEL;
		i = re->shaderAnim;
		j = (int)floor(ratio * shaderAnimCountsw[re->shaderAnim]);
		shockwave.customShader = cgs.media.railexpringShader;
		shockwave.shaderTime = re->shaderTime;
		VectorCopy(re->origin, shockwave.origin);

		c = (float)(time - KAMI_SHOCKWAVE_STARTTIME4) / (float)(KAMI_SHOCKWAVE_ENDTIME4 - KAMI_SHOCKWAVE_STARTTIME4);
		VectorScale( re->axis[0], c * 5 / 7, shockwave.axis[0] );
		VectorScale( re->axis[1], c * 5 / 7, shockwave.axis[1] );
		VectorScale( re->axis[2], c * 5 / 7, shockwave.axis[2] );
		shockwave.nonNormalizedAxes = qfalse;

		if (time > KAMI_SHOCKWAVEFADE_STARTTIME4) {
			c = (float)(time - KAMI_SHOCKWAVEFADE_STARTTIME4) / (float)(KAMI_SHOCKWAVE_ENDTIME4 - KAMI_SHOCKWAVEFADE_STARTTIME4);
		}
		else {
			c = 0;
		}
		c *= 0xff;
		re->shaderRGBA[0] = 0xff * le->color[0] * ( le->endTime - cg.time ) * le->lifeRate;
		re->shaderRGBA[1] = 0xff * le->color[1] * ( le->endTime - cg.time ) * le->lifeRate;
		re->shaderRGBA[2] = 0xff * le->color[2] * ( le->endTime - cg.time ) * le->lifeRate;
		re->shaderRGBA[3] = 0xff * le->color[3] * ( le->endTime - cg.time ) * le->lifeRate;

		shockwave.shaderRGBA[0] = re->shaderRGBA[0];
		shockwave.shaderRGBA[1] = re->shaderRGBA[1];
		shockwave.shaderRGBA[2] = re->shaderRGBA[2];
		shockwave.shaderRGBA[3] = re->shaderRGBA[3];

		trap_R_AddRefEntityToScene( &shockwave );
	}
	if (time > 0 && time < 800) {

		// 1st kamikaze shockwave
		memset(&shockwave, 0, sizeof(shockwave));
		shockwave.hModel = cgs.media.rocketShockWave;
		shockwave.reType = RT_MODEL;
		i = re->shaderAnim;
		j = (int)floor(ratio * shaderAnimCountsw[re->shaderAnim]);
		shockwave.customShader = cgs.media.railexpringShader;
		shockwave.shaderTime = re->shaderTime;
		VectorCopy(re->origin, shockwave.origin);

		c = (float)(time - 0) / (float)(800 - 0);
		VectorScale( re->axis[0], c * 5 / 7, shockwave.axis[0] );
		VectorScale( re->axis[1], c * 5 / 7, shockwave.axis[1] );
		VectorScale( re->axis[2], c * 5 / 7, shockwave.axis[2] );
		shockwave.nonNormalizedAxes = qfalse;

		if (time > 200) {
			c = (float)(time - 200) / (float)(800 - 200);
		}
		else {
			c = 0;
		}
		c *= 0xff;
		re->shaderRGBA[0] = 0xff * le->color[0] * ( le->endTime - cg.time ) * le->lifeRate;
		re->shaderRGBA[1] = 0xff * le->color[1] * ( le->endTime - cg.time ) * le->lifeRate;
		re->shaderRGBA[2] = 0xff * le->color[2] * ( le->endTime - cg.time ) * le->lifeRate;
		re->shaderRGBA[3] = 0xff * le->color[3] * ( le->endTime - cg.time ) * le->lifeRate;

		shockwave.shaderRGBA[0] = re->shaderRGBA[0];
		shockwave.shaderRGBA[1] = re->shaderRGBA[1];
		shockwave.shaderRGBA[2] = re->shaderRGBA[2];
		shockwave.shaderRGBA[3] = re->shaderRGBA[3];

		trap_R_AddRefEntityToScene( &shockwave );
	}
}

/*
==================
CG_RingEffect
==================
*/
void CG_RailEXPRingEffect( vec3_t org , vec3_t dir,int duration,clientInfo_t *ci) {
	localEntity_t	*le;
	refEntity_t		*re;
	vec3_t		forward, right, up;
	vec3_t	angles;


	if (duration < 0) {
		duration *= -1;
	}
	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_RAILEXPRING;
	le->entNumber = le - cg_freeLocalEntities;
	le->startTime = cg.time ;
	le->endTime = cg.time + duration;
	le->lifeRate = 0.8 / ( le->endTime - le->startTime );


	le->color[0] = ci->color2[0] * 0.6f;
	le->color[1] = ci->color2[1] * 0.6f;
	le->color[2] = ci->color2[2] * 0.6f;
	le->color[3] = 1;

	re->shaderRGBA[0] = ci->color2[0] * 255;
    re->shaderRGBA[1] = ci->color2[1] * 255;
    re->shaderRGBA[2] = ci->color2[2] * 255;
    re->shaderRGBA[3] = 255;

	VectorNormalize(dir);
	vectoangles (dir, angles);
	AngleVectors( angles, forward, right, up );
	VectorCopy(forward ,re->axis[2]);
	VectorCopy( right,re->axis[1]);
	VectorCopy( up,re->axis[0]);



	re->shaderTime = cg.time / 200.0f;
	VectorCopy( org, re->origin);
}

/*
==================
CG_AddRailEXPRing2
==================
*/
void CG_AddRailEXPRing2( localEntity_t *le ) {
	refEntity_t	*re;
	refEntity_t shockwave;
	float		c;
	int i, j;
	float		ratio;
	float		time, time2;


	time = cg.time - le->startTime;
	time2 = le->endTime - le->startTime;
	ratio = time / time2;

	re = &le->refEntity;


	if (time > KAMI_SHOCKWAVE_STARTTIME4 && time < KAMI_SHOCKWAVE_ENDTIME4) {

		// 1st kamikaze shockwave
		memset(&shockwave, 0, sizeof(shockwave));
		shockwave.hModel = cgs.media.rocketShockWave;
		shockwave.reType = RT_MODEL;
		i = re->shaderAnim;
		j = (int)floor(ratio * shaderAnimCountsw[re->shaderAnim]);
		shockwave.customShader = cgs.media.railexpring2Shader;
		shockwave.shaderTime = re->shaderTime;
		VectorCopy(re->origin, shockwave.origin);

		c = (float)(time - KAMI_SHOCKWAVE_STARTTIME4) / (float)(KAMI_SHOCKWAVE_ENDTIME4 - KAMI_SHOCKWAVE_STARTTIME4);
		VectorScale( re->axis[0], c * 5 / 7, shockwave.axis[0] );
		VectorScale( re->axis[1], c * 5 / 7, shockwave.axis[1] );
		VectorScale( re->axis[2], c * 5 / 7, shockwave.axis[2] );
		shockwave.nonNormalizedAxes = qfalse;

		if (time > KAMI_SHOCKWAVEFADE_STARTTIME4) {
			c = (float)(time - KAMI_SHOCKWAVEFADE_STARTTIME4) / (float)(KAMI_SHOCKWAVE_ENDTIME4 - KAMI_SHOCKWAVEFADE_STARTTIME4);
		}
		else {
			c = 0;
		}
		c *= 0xff;
		re->shaderRGBA[0] = 0xff * le->color[0] * ( le->endTime - cg.time ) * le->lifeRate;
		re->shaderRGBA[1] = 0xff * le->color[1] * ( le->endTime - cg.time ) * le->lifeRate;
		re->shaderRGBA[2] = 0xff * le->color[2] * ( le->endTime - cg.time ) * le->lifeRate;
		re->shaderRGBA[3] = 0xff * le->color[3] * ( le->endTime - cg.time ) * le->lifeRate;

		shockwave.shaderRGBA[0] = re->shaderRGBA[0];
		shockwave.shaderRGBA[1] = re->shaderRGBA[1];
		shockwave.shaderRGBA[2] = re->shaderRGBA[2];
		shockwave.shaderRGBA[3] = re->shaderRGBA[3];

		trap_R_AddRefEntityToScene( &shockwave );
	}
	if (time > 0 && time < 800) {

		// 1st kamikaze shockwave
		memset(&shockwave, 0, sizeof(shockwave));
		shockwave.hModel = cgs.media.rocketShockWave;
		shockwave.reType = RT_MODEL;
		i = re->shaderAnim;
		j = (int)floor(ratio * shaderAnimCountsw[re->shaderAnim]);
		shockwave.customShader = cgs.media.railexpring2Shader;
		shockwave.shaderTime = re->shaderTime;
		VectorCopy(re->origin, shockwave.origin);

		c = (float)(time - 0) / (float)(800 - 0);
		VectorScale( re->axis[0], c * 5 / 7, shockwave.axis[0] );
		VectorScale( re->axis[1], c * 5 / 7, shockwave.axis[1] );
		VectorScale( re->axis[2], c * 5 / 7, shockwave.axis[2] );
		shockwave.nonNormalizedAxes = qfalse;

		if (time > 200) {
			c = (float)(time - 200) / (float)(800 - 200);
		}
		else {
			c = 0;
		}
		c *= 0xff;
		re->shaderRGBA[0] = 0xff * le->color[0] * ( le->endTime - cg.time ) * le->lifeRate;
		re->shaderRGBA[1] = 0xff * le->color[1] * ( le->endTime - cg.time ) * le->lifeRate;
		re->shaderRGBA[2] = 0xff * le->color[2] * ( le->endTime - cg.time ) * le->lifeRate;
		re->shaderRGBA[3] = 0xff * le->color[3] * ( le->endTime - cg.time ) * le->lifeRate;

		shockwave.shaderRGBA[0] = re->shaderRGBA[0];
		shockwave.shaderRGBA[1] = re->shaderRGBA[1];
		shockwave.shaderRGBA[2] = re->shaderRGBA[2];
		shockwave.shaderRGBA[3] = re->shaderRGBA[3];

		trap_R_AddRefEntityToScene( &shockwave );
	}
}

/*
==================
CG_RailEXPRingEffect2
==================
*/
void CG_RailEXPRingEffect2( vec3_t org , vec3_t dir,int duration,clientInfo_t *ci) {
	localEntity_t	*le;
	refEntity_t		*re;
	vec3_t		forward, right, up;
	vec3_t	angles;


	if (duration < 0) {
		duration *= -1;
	}
	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_RAILEXPRING2;
	le->entNumber = le - cg_freeLocalEntities;
	le->startTime = cg.time ;
	le->endTime = cg.time + duration;
	le->lifeRate = 0.8 / ( le->endTime - le->startTime );


	le->color[0] = ci->color1[0] * 0.6f;
	le->color[1] = ci->color1[1] * 0.6f;
	le->color[2] = ci->color1[2] * 0.6f;
	le->color[3] = 1;

	re->shaderRGBA[0] = ci->color1[0] * 255;
    re->shaderRGBA[1] = ci->color1[1] * 255;
    re->shaderRGBA[2] = ci->color1[2] * 255;
    re->shaderRGBA[3] = 255;

	VectorNormalize(dir);
	vectoangles (dir, angles);
	AngleVectors( angles, forward, right, up );
	VectorCopy(forward ,re->axis[2]);
	VectorCopy( right,re->axis[1]);
	VectorCopy( up,re->axis[0]);



	re->shaderTime = cg.time / 200.0f;
	VectorCopy( org, re->origin);
}

/*
==================
CG_AddRgExplosion
==================
*/
static void CG_AddRgExplosion( localEntity_t *le ) {
	refEntity_t	re;
	float c;
	float		ratio;
	float		time, time2;


	time = cg.time - le->startTime;
	time2 = le->endTime - le->startTime;
	ratio = time / time2;

	re = le->refEntity;


	re.customShader = le->refEntity.customShader;
	re.shaderTime = le->refEntity.shaderTime;

	c = ( le->endTime - cg.time ) / ( float ) ( le->endTime - le->startTime );
//	if ( c > 1 ) {
//		c = 1.0;	// can happen during connection problems
//	}

	re.shaderRGBA[0] = 0xff * le->color[0] * ( le->endTime - cg.time ) * le->lifeRate ;
	re.shaderRGBA[1] = 0xff * le->color[1] * ( le->endTime - cg.time ) * le->lifeRate ;
	re.shaderRGBA[2] = 0xff * le->color[2] * ( le->endTime - cg.time ) * le->lifeRate ;
	re.shaderRGBA[3] = 0xff * le->color[3] * ( le->endTime - cg.time ) * le->lifeRate ;

	re.reType = RT_MODEL;
	re.radius = 8 * ( 1.0 - c ) + 8;

	trap_R_AddRefEntityToScene( &re );

	// add the dlight
	if ( le->light ) {
		float		light;
		float		light2;

		light = (float)( cg.time - le->startTime ) / ( le->endTime - le->startTime );
		if ( light < 0.5 ) {
			light = 1.0;
		} else {
			light = 1.0 - ( light - 0.5 ) * 2;
		}
		light = le->light * light;
		trap_R_AddAdditiveLightToScene( re.origin, light-20, le->lightColor[0], le->lightColor[1], le->lightColor[2] );
	// light test

		light2 = (float)( cg.time - le->startTime ) / ( le->endTime - le->startTime );
		if ( light2 < 0.5 ) {
			light2 = 1.0;
		} else {
			light2 = 1.0 - ( light2 - 0.5 ) * 2;
		}
		light2 = le->light2 * light2;
		trap_R_AddLightToScene(re.origin, light2, le->lightColor2[0], le->lightColor2[1], le->lightColor2[2] );
		trap_R_AddLightToScene(re.origin, light2+20, le->lightColor2[0], le->lightColor2[1], le->lightColor2[2] );

	}

}

/*
==================
CG_RgExplosion
==================
*/
void CG_RgExplosion( vec3_t dir, vec3_t org ,int duration, float light, vec3_t lightColor, float light2, vec3_t lightColor2) {
	localEntity_t	*le;
	refEntity_t		*re;
	vec3_t			tmpVec, newOrigin;
	float			ang;


	if (duration < 0) {
		duration *= -1;
	}
	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_RGEXP;
	le->entNumber = le - cg_freeLocalEntities;
	le->startTime = cg.time ;
	le->endTime = cg.time + duration;
	le->lifeRate = 0.8 / ( le->endTime - le->startTime );
	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0;
	if(cg_QSLights.integer == 1){
		le->light2 = light2;
		VectorCopy(lightColor2,le->lightColor2);
		
		le->light = light;
		VectorCopy(lightColor,le->lightColor);
	}

//	AxisCopy(axisDefault,re->axis);

	ang = rand() % 360;
	VectorCopy( dir, re->axis[0] );
	RotateAroundDirection( re->axis, ang );

	re->customShader = cgs.media.rgExplosionShader;

	re->shaderTime = cg.time / 200.0f;
	re->hModel = cgs.media.rgringShockWave;
	re->rotation = rand() % 360;
	VectorScale( dir, 6.5f, tmpVec );
	VectorAdd( tmpVec, org, newOrigin );

	VectorCopy( newOrigin, re->origin );
	VectorCopy( newOrigin, re->oldorigin );

	CG_ImpactMark( cgs.media.energy1MarkShader, org, dir, random()*360, 1,1,1,1, qtrue, 10, qfalse );
	trap_S_StartSound( org, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.sfx_plasmaexp );

}

#ifdef MISSIONPACK
/*
===================
CG_AddInvulnerabilityImpact
===================
*/
void CG_AddInvulnerabilityImpact( localEntity_t *le ) {
	trap_R_AddRefEntityToScene( &le->refEntity );
}

/*
===================
CG_AddInvulnerabilityJuiced
===================

void CG_AddInvulnerabilityJuiced( localEntity_t *le ) {
	int t;

	t = cg.time - le->startTime;
	if ( t > 3000 ) {
		le->refEntity.axis[0][0] = (float) 1.0 + 0.3 * (t - 3000) / 2000;
		le->refEntity.axis[1][1] = (float) 1.0 + 0.3 * (t - 3000) / 2000;
		le->refEntity.axis[2][2] = (float) 0.7 + 0.3 * (2000 - (t - 3000)) / 2000;
	}
	if ( t > 5000 ) {
		le->endTime = 0;
		CG_GibPlayer( le->refEntity.origin , le->refEntity.);
	}
	else {
		trap_R_AddRefEntityToScene( &le->refEntity );
	}
}
*/
/*
===================
CG_AddRefEntity
===================
*/
void CG_AddRefEntity( localEntity_t *le ) {
	if (le->endTime < cg.time) {
		if(!(le->leFlags & LEF_NEVER_FREE)){
			CG_FreeLocalEntity( le );
			return;
		}
	}
	trap_R_AddRefEntityToScene( &le->refEntity );
}

#endif
/*
===================
CG_AddScorePlum
===================
*/
#define NUMBER_SIZE		8

void CG_AddScorePlum( localEntity_t *le ) {
	refEntity_t	*re;
	vec3_t		origin, delta, dir, vec, up = {0, 0, 1};
	float		c, len;
	int			i, score, digits[10], numdigits, negative;

	re = &le->refEntity;

	c = ( le->endTime - cg.time ) * le->lifeRate;

	score = le->radius;
	if (score < 0) {
		re->shaderRGBA[0] = 0xff;
		re->shaderRGBA[1] = 0x11;
		re->shaderRGBA[2] = 0x11;
	}
	else {
		re->shaderRGBA[0] = 0xff;
		re->shaderRGBA[1] = 0xff;
		re->shaderRGBA[2] = 0xff;
		if (score >= 50) {
			re->shaderRGBA[1] = 0;
		} else if (score >= 20) {
			re->shaderRGBA[0] = re->shaderRGBA[1] = 0;
		} else if (score >= 10) {
			re->shaderRGBA[2] = 0;
		} else if (score >= 2) {
			re->shaderRGBA[0] = re->shaderRGBA[2] = 0;
		}

	}
	if (c < 0.25)
		re->shaderRGBA[3] = 0xff * 4 * c;
	else
		re->shaderRGBA[3] = 0xff;

	re->radius = NUMBER_SIZE / 2;

	VectorCopy(le->pos.trBase, origin);
	origin[2] += 110 - c * 100;

	VectorSubtract(cg.refdef[le->view].vieworg, origin, dir);

	CrossProduct(dir, up, vec);
	VectorNormalize(vec);

	VectorMA(origin, -10 + 20 * sin(c * 2 * M_PI), vec, origin);

	// if the view would be "inside" the sprite, kill the sprite
	// so it doesn't add too much overdraw

	VectorSubtract( origin, cg.refdef[le->view].vieworg, delta );

	len = VectorLength( delta );
	if ( len < 20 ) {
		CG_FreeLocalEntity( le );
		return;
	}

	negative = qfalse;
	if (score < 0) {
		negative = qtrue;
		score = -score;
	}

	for (numdigits = 0; !(numdigits && !score); numdigits++) {
		digits[numdigits] = score % 10;
		score = score / 10;
	}

	if (negative) {
		digits[numdigits] = 10;
		numdigits++;
	}

	for (i = 0; i < numdigits; i++) {
		VectorMA(origin, (float) (((float) numdigits / 2) - i) * NUMBER_SIZE, vec, re->origin);
		re->customShader = cgs.media.numberShaders[digits[numdigits-1-i]];
		trap_R_AddRefEntityToScene( re );
	}
}

/*
====================
CG_AddRailEffect
====================
*/
void CG_AddRailEffect2( localEntity_t *le ) {
	localEntity_t	*nle;
	refEntity_t		*re, *nre;
	vec3_t			forward, right, up, posx, posy, pos, oldpos, move, vec;
	float			c, len, radius, theta, sn, cs, spacing;
	int				i = 0, r, g, b;

	re = &le->refEntity;
	c = ( le->endTime - cg.time ) * le->lifeRate;
	c *= 0xff;

	spacing = 60.0f;

	VectorCopy (re->origin, move);
	VectorSubtract (re->oldorigin, re->origin, vec );

	// Find the forward, right, and up vectors
	VectorCopy( vec, forward );
	VectorNormalize( forward );
	PerpendicularVector( right, forward );
	CrossProduct( forward, right, up );

	len = VectorNormalize (vec);
	VectorScale (vec, spacing, vec);

	if ( le->generic1 == TEAM_RED ) {
		r = 0.75 * c;
		g = 0.0625 * c;
		b = 0;
	}
	else if ( le->generic1 == TEAM_BLUE  ) {
		r = 0;
		g = 0.5 * c;
		b = 1.0 * c;
	}
	else {
		r = 0.9 * c;
		g = 0.0;
		b = 0.9 * c;
	}

	for (; i < len; i += spacing) {
		radius = ( 2.0 + ( 0xff - c ) / 16 )* sin( i / len * M_PI );

		theta = (float)(i - cg.time % 360) * M_PI / 180 * 2;

		cs = cos(theta);
		sn = sin(theta);

		VectorScale( right, cs, posx );
		VectorScale( up, sn, posy );
		VectorAdd( posx, posy, pos );

		VectorScale( pos, radius, pos );

		VectorAdd( pos, move, pos );

		if ( i > 0 ) {
			nle = CG_AllocLocalEntity();
			nle->startTime = cg.time;
			nle->endTime = cg.time + 1;

			nre = &nle->refEntity;

			nre->reType = RT_RAIL_CORE;
			nre->customShader = cgs.media.railCoreShader;

			nre->shaderRGBA[0] = r;
			nre->shaderRGBA[1] = g;
			nre->shaderRGBA[2] = b;
			nre->shaderRGBA[3] = 0xff * c * le->color[3];

			AxisClear( nre->axis );

			VectorCopy( pos, nre->origin );
			VectorCopy( oldpos, nre->oldorigin );

			trap_R_AddRefEntityToScene( nre );
		}

		VectorAdd (move, vec, move);
		VectorCopy( pos, oldpos );
	}

}


/*
==================
CG_AddRailEffect
==================
*/
void CG_AddRailEffect( localEntity_t *le ) {
	localEntity_t	*nle;
	refEntity_t		*re, *nre;
	vec3_t			forward, right, up, posx, posy, pos, oldpos, move, vec;
	float			c, len, radius, theta, sn, cs, spacing;
	int				i = 0;

	re = &le->refEntity;
	c = ( le->endTime - cg.time ) * le->lifeRate;
	c *= 0xff;

	spacing = 35.0f;

	VectorCopy (re->origin, move);
	VectorSubtract (re->oldorigin, re->origin, vec );

	// Find the forward, right, and up vectors
	VectorCopy( vec, forward );
	VectorNormalize( forward );
	PerpendicularVector( right, forward );
	CrossProduct( forward, right, up );

	len = VectorNormalize (vec);
	VectorScale (vec, spacing, vec);

	for (; i < len; i += spacing) {
		radius = ( 2.0 + ( 0xff - c ) / 22)* sin( i / len * M_PI );

		theta = (float)(i - cg.time % 360) * M_PI / 180 * 2;

		cs = cos(theta);
		sn = sin(theta);

		VectorScale( right, cs, posx );
		VectorScale( up, sn, posy );
		VectorAdd( posx, posy, pos );

		VectorScale( pos, radius, pos );

		VectorAdd( pos, move, pos );

		if ( i > 0 ) {
			
			nle = CG_AllocLocalEntity();
			nle->startTime = cg.time;
			nle->endTime = cg.time + 4;

			nre = &nle->refEntity;

			nre->reType = RT_RAIL_CORE;

			nre->customShader = cgs.media.railCoreShader;

			nre->shaderRGBA[0] = le->color[0] * c;
			nre->shaderRGBA[1] = le->color[1] * c;
			nre->shaderRGBA[2] = le->color[2] * c;
			nre->shaderRGBA[3] = le->color[3] * c;
			AxisClear( nre->axis );
			
			VectorCopy( pos, nre->origin );
			VectorCopy( oldpos, nre->oldorigin );

			trap_R_AddRefEntityToScene( nre );


		}

		VectorAdd (move, vec, move);
		VectorCopy( pos, oldpos );
	}

}

/*
===================
CG_AddSparks
===================
*/
static void CG_AddSpark( localEntity_t *le ){
	refEntity_t	*re;
	vec3_t  newOrg;
	int		t,t2,step;
	static vec3_t col = {1,1,1};

	re = &le->refEntity;	

	step = le->step;
	t = step * ( (cg.time - cg.frametime + step ) / step );
	t2 = step * ( cg.time / step );


	BG_EvaluateTrajectory( &le->pos, cg.time, newOrg );



	VectorCopy( newOrg, le->refEntity.origin );
}

/*
===================
CG_AddRocketSparks
===================
*/

static void CG_AddRocketSpark( localEntity_t *le )
{
	refEntity_t	*re;
	vec3_t  newOrg;
	trace_t trace;
	int		t,t2,step;
	static vec3_t col = {1,1,1};

	re = &le->refEntity;	

	step = le->step;
	t = step * ( (cg.time - cg.frametime + step ) / step );
	t2 = step * ( cg.time / step );

	BG_EvaluateTrajectory( &le->pos, cg.time, newOrg );



	CG_Trace( &trace, le->refEntity.origin, NULL, NULL, newOrg, 0, MASK_SOLID);

	if(trace.startsolid){
		CG_FreeLocalEntity( le );
		return;
	}

	if ( trap_CM_PointContents( trace.endpos, 0 ) & CONTENTS_NODROP ) {
		CG_FreeLocalEntity( le );
		return;
	}

	if ( trap_CM_PointContents( trace.endpos, 0 ) & CONTENTS_MOVER ) {
		CG_FreeLocalEntity( le );
		return;
	}

	if ( trace.fraction == 1.0 ){
		VectorCopy( newOrg, le->refEntity.origin );
		return;
	}else{
		CG_FreeLocalEntity( le );
		return;
	}
}

/*
==================
CG_AddGrenadeSpark
==================
*/
static void CG_AddGrenadeSpark( localEntity_t *le )
{
	refEntity_t	*re;
	vec3_t  oldOrg;
	vec3_t  newOrg;
	trace_t trace;
	vec3_t dir, view, right;
	polyVert_t verts[4];
	float		c;

	re = &le->refEntity;	

	

	le->lifeRate = 0.5 / ( le->endTime - le->startTime );

	if ( le->fadeInTime > le->startTime && cg.time < le->fadeInTime ) {
		// fade / grow time
		c = -1.0 - (float) ( le->fadeInTime - cg.time ) / ( le->fadeInTime - le->startTime );
	}
	else {
		// fade / grow time
		c = ( le->endTime - cg.time ) * le->lifeRate;
	}

	if ( le->light ) {
		trap_R_AddAdditiveLightToScene( re->origin, 50, le->lightColor[0], le->lightColor[1], le->lightColor[2] );
	}

	re->shaderRGBA[3] = 0xff * c * le->color[3];

	BG_EvaluateTrajectory( &le->pos, cg.time, newOrg );
	BG_EvaluateTrajectory( &le->pos, cg.oldTime, oldOrg ); // Check the old for direction tracing

	VectorSubtract( newOrg,oldOrg , dir );
	VectorNormalize( dir );
	VectorSubtract( newOrg, cg.refdef[le->view].vieworg, view );
	CrossProduct( dir, view, right );
	VectorNormalize( right );
	CG_Trace( &trace, le->refEntity.origin, NULL, NULL, newOrg, 0, CONTENTS_MOVER );
	if ( trace.fraction == 1.0 ) // Didn't hit anything
{
	VectorCopy( newOrg, le->refEntity.origin );
	VectorMA( le->refEntity.origin, le->width = 120, dir, le->refEntity.oldorigin );

	VectorMA( le->refEntity.origin, 1, right, verts[0].xyz );
	verts[0].st[0] = 0;
	verts[0].st[1] = 0;
	verts[0].modulate[0] = 255;
	verts[0].modulate[1] = 255;
	verts[0].modulate[3] = 255;
	verts[0].modulate[4] = 255;
	VectorMA( le->refEntity.origin, -1, right, verts[1].xyz );
	verts[1].st[0] = 0;
	verts[1].st[1] = 1;
	verts[1].modulate[0] = 255;
	verts[1].modulate[1] = 255;
	verts[1].modulate[3] = 255;
	verts[1].modulate[4] = 255;
	VectorMA( le->refEntity.oldorigin, -3, right, verts[2].xyz );
	verts[2].st[0] = 1;
	verts[2].st[1] = 1;
	verts[2].modulate[0] = 255;
	verts[2].modulate[1] = 255;
	verts[2].modulate[3] = 255;
	verts[2].modulate[4] = 255;
	VectorMA( le->refEntity.oldorigin, 3, right, verts[3].xyz );
	verts[3].st[0] = 1;
	verts[3].st[1] = 0;
	verts[3].modulate[0] = 255;
	verts[3].modulate[1] = 255;
	verts[3].modulate[3] = 255;
	verts[3].modulate[4] = 255;
	trap_R_AddPolyToScene( le->refEntity.customShader, 4, verts );
		return;
}
	if ( trap_CM_PointContents( trace.endpos, 0 ) & CONTENTS_NODROP | CONTENTS_MOVER ) {
		CG_FreeLocalEntity( le );
		return;
	}
}

/*
===================
CG_AddBFGSparks
===================
*/
static void CG_AddBFGSpark( localEntity_t *le ) {
	refEntity_t	*re;
	vec3_t newOrg, oldOrg;
	trace_t trace;
	vec3_t dir, view, right;

	re = &le->refEntity;

	BG_EvaluateTrajectory( &le->pos, cg.time, newOrg );
	BG_EvaluateTrajectory( &le->pos, cg.oldTime, oldOrg ); // Check the old for direction tracing

	VectorSubtract( newOrg,oldOrg , dir );
	VectorNormalize( dir );
	VectorSubtract( newOrg, cg.refdef[le->view].vieworg, view );
	CrossProduct( dir, view, right );
	VectorNormalize( right );
	CG_Trace( &trace, le->refEntity.origin, NULL, NULL, newOrg, -1, CONTENTS_SOLID );
	if ( trace.fraction == 1.0 ) {
	VectorCopy( newOrg, le->refEntity.origin );	
	trap_R_AddRefEntityToScene( re );
		return;
	}

	if ( le->light ) {
		trap_R_AddAdditiveLightToScene( re->origin, 50, le->lightColor[0], le->lightColor[1], le->lightColor[2] );
	}

	if ( trap_CM_PointContents( trace.endpos, 0 ) & CONTENTS_NODROP ) {
		CG_FreeLocalEntity( le );
		return;
	}
}

/*
///////////////////////////////
Rail Gun Particles
///////////////////////////////
*/

/*
===================
CG_AddRailSparkPARTICLE
===================
*/
static void CG_AddRailSparkPARTICLE( localEntity_t *le ){
	refEntity_t	*re;
	vec3_t  newOrg,p,rlforward, rlvright, rlvup;
	trace_t trace;
	polyVert_t verts[4];
	float	radius;
	float	c;

	re = &le->refEntity;

	BG_EvaluateTrajectory( &le->pos, cg.time, newOrg );

	CG_Trace( &trace, re->origin, NULL, NULL, newOrg, -1, MASK_SOLID);

	c = ( (le->startTime + 600) - cg.time ) / ( float ) ( (le->startTime + 600) - le->startTime + 200);

	VectorCopy( cg.refdef[le->view].viewaxis[0], rlforward );
	VectorCopy( cg.refdef[le->view].viewaxis[1], rlvright );
	VectorCopy( cg.refdef[le->view].viewaxis[2], rlvup );
	

	if(cg.time >= le->startTime + 200 || le->pos.trType == TR_STATIONARY){
		radius = le->startSize * c;
	}else{
		radius = le->startSize;
	}

	if(radius <= 0){
		CG_FreeLocalEntity( le );
		return;
	}

	// 1st
		VectorCopy( re->origin, p );
		VectorMA (p, radius, rlvup, p);	
		VectorMA (p, radius, rlvright, p);	
		VectorCopy (p, verts[0].xyz);	
		verts[0].st[0] = 0;	
		verts[0].st[1] = 0;	
		verts[0].modulate[0] = 255;	
		verts[0].modulate[1] = 255;	
		verts[0].modulate[2] = 255;	
		verts[0].modulate[3] = 255;

	// 2nd
		VectorCopy( re->origin, p );
		VectorMA (p, radius, rlvup, p);	
		VectorMA (p, -radius, rlvright, p);
		VectorCopy (p, verts[1].xyz);	
		verts[1].st[0] = 0;	
		verts[1].st[1] = 1;	
		verts[1].modulate[0] = 255;	
		verts[1].modulate[1] = 255;	
		verts[1].modulate[2] = 255;	
		verts[1].modulate[3] = 255;
		
	// 3rd
		VectorCopy( re->origin, p );
		VectorMA (p, -radius, rlvup, p);	
		VectorMA (p, -radius, rlvright, p);
		VectorCopy (p, verts[2].xyz);	
		verts[2].st[0] = 1;	
		verts[2].st[1] = 1;	
		verts[2].modulate[0] = 255;	
		verts[2].modulate[1] = 255;	
		verts[2].modulate[2] = 255;	
		verts[2].modulate[3] = 255;
		
	// 4th
		VectorCopy( re->origin, p );
		VectorMA (p, -radius, rlvup, p);	
		VectorMA (p, radius, rlvright, p);
		VectorCopy (p, verts[3].xyz);	
		verts[3].st[0] = 1;	
		verts[3].st[1] = 0;	
		verts[3].modulate[0] = 255;	
		verts[3].modulate[1] = 255;	
		verts[3].modulate[2] = 255;	
		verts[3].modulate[3] = 255;

	trap_R_AddPolyToScene( cgs.media.tracerShader, 4, verts );

	if ( trace.fraction == 1.0 ){
		VectorCopy( newOrg, le->refEntity.origin );
	//	trap_R_AddRefEntityToScene( re );
		return;
	}

	if ( trap_CM_PointContents( re->origin, 0 ) & (CONTENTS_NODROP|CONTENTS_MOVER|
		CONTENTS_WATER|CONTENTS_LAVA|CONTENTS_SLIME)) {
		CG_FreeLocalEntity( le );
		return;
	}

	CG_ReflectVelocity( le, &trace );

//	trap_R_AddRefEntityToScene( re );

}

static void CG_AddRailParticleEXP( vec3_t origin, vec3_t dir,int count ,int duration,
								  int speed,float startsize,float endsize) {
	localEntity_t	*le;
	refEntity_t		*re;
	vec3_t	velocity;
	int	i,start,stop;  
    float randScale;


	// add the sparks
	for (i=0; i<count;i++) {

		le = CG_AllocLocalEntity();
		re = &le->refEntity;

		speed = speed  + rand()%20;
		duration = duration + rand()%20;
		randScale = 1.0f;


		le->leType = LE_RGPARTEXP;
		le->entNumber = le - cg_freeLocalEntities;
		VectorNormalize(dir);
		velocity[0] = dir[0] + crandom()*randScale;
		velocity[1] = dir[1] + crandom()*randScale;
		velocity[2] = dir[2] + crandom()*randScale;
		VectorScale( velocity, (float)speed, le->pos.trDelta);

		le->startTime = cg.time;
		le->endTime = le->startTime + duration*8;
		le->lifeRate = 1.0 / ( le->endTime - le->startTime );
		le->startSize = startsize;
		le->endSize = endsize;

		le->pos.trType = TR_GRAVITY;
		le->pos.trTime = cg.time;
		le->bounceFactor = 0.8f;
		VectorCopy( origin, re->origin );
		VectorCopy( origin, le->pos.trBase );




 
	}

		start = cg.time;
		stop = start + 1500;
		if(cg.time < stop){

				trap_R_AddAdditiveLightToScene(  origin, 15, 0.2f, 0.2f, 0.2f );
				trap_R_AddLightToScene(  origin, 20, 4.0f, 4.0f, 4.0f );

		}

}

static void CG_SparkTrail( localEntity_t *le ,int trailtime
								,qhandle_t hShader
								,int lstep
								,float	startsize
								,float	endsize) {
	static vec3_t col = {1,1,1};
	int		t;
	int		t2;
	int		step;
	vec3_t	newOrigin;


	step = lstep;
	t = step * ( (cg.time - cg.frametime + step ) / step );
	t2 = step * ( cg.time / step );

	BG_EvaluateTrajectory( &le->pos, cg.time, newOrigin );


}

/*
===================
CG_AddRailSparks
===================
*/
static void CG_AddRailSpark( localEntity_t *le ){
	refEntity_t	*re;
	vec3_t  newOrg,p,rlforward, rlvright, rlvup,mins,maxs;
	trace_t trace;
	polyVert_t verts[4];
	int	radius;

	re = &le->refEntity;

	mins[0] = -le->startSize;
	mins[1] = -le->startSize;
	mins[2] = -le->startSize;

	maxs[0] = le->startSize;
	maxs[1] = le->startSize;
	maxs[2] = le->startSize;

	BG_EvaluateTrajectory( &le->pos, cg.time, newOrg );

	CG_Trace( &trace, le->refEntity.origin, mins, maxs, newOrg, -1, MASK_SOLID|CONTENTS_BODY);


	VectorCopy( cg.refdef[le->view].viewaxis[0], rlforward );
	VectorCopy( cg.refdef[le->view].viewaxis[1], rlvright );
	VectorCopy( cg.refdef[le->view].viewaxis[2], rlvup );

	radius = le->startSize;
	radius *= 2;

	// 1st
		VectorCopy( re->origin, p );
		VectorMA (p, radius, rlvup, p);	
		VectorMA (p, radius, rlvright, p);	
		VectorCopy (p, verts[0].xyz);	
		verts[0].st[0] = 0;	
		verts[0].st[1] = 0;	
		verts[0].modulate[0] = 255;	
		verts[0].modulate[1] = 255;	
		verts[0].modulate[2] = 255;	
		verts[0].modulate[3] = 255;

	// 2nd
		VectorCopy( re->origin, p );
		VectorMA (p, radius, rlvup, p);	
		VectorMA (p, -radius, rlvright, p);
		VectorCopy (p, verts[1].xyz);	
		verts[1].st[0] = 0;	
		verts[1].st[1] = 1;	
		verts[1].modulate[0] = 255;	
		verts[1].modulate[1] = 255;	
		verts[1].modulate[2] = 255;	
		verts[1].modulate[3] = 255;
		
	// 3rd
		VectorCopy( re->origin, p );
		VectorMA (p, -radius, rlvup, p);	
		VectorMA (p, -radius, rlvright, p);
		VectorCopy (p, verts[2].xyz);	
		verts[2].st[0] = 1;	
		verts[2].st[1] = 1;	
		verts[2].modulate[0] = 255;	
		verts[2].modulate[1] = 255;	
		verts[2].modulate[2] = 255;	
		verts[2].modulate[3] = 255;
		
	// 4th
		VectorCopy( re->origin, p );
		VectorMA (p, -radius, rlvup, p);	
		VectorMA (p, radius, rlvright, p);
		VectorCopy (p, verts[3].xyz);	
		verts[3].st[0] = 1;	
		verts[3].st[1] = 0;	
		verts[3].modulate[0] = 255;	
		verts[3].modulate[1] = 255;	
		verts[3].modulate[2] = 255;	
		verts[3].modulate[3] = 255;

	trap_R_AddPolyToScene( cgs.media.tracerShader, 4, verts );



	if ( trace.fraction == 1.0 ){
		if ( le->pos.trType != TR_STATIONARY ) { 
			CG_SparkTrail( le ,le->trailLength
										,le->hShader
										,le->step
										,le->startSize
										,le->endSize);
		}
		VectorCopy( newOrg, le->refEntity.origin );
		return;
	}
	if ( trap_CM_PointContents( re->origin, 0 ) & (CONTENTS_NODROP|CONTENTS_MOVER|
		CONTENTS_WATER|CONTENTS_LAVA|CONTENTS_SLIME)) {
		CG_FreeLocalEntity( le );
		return;
	}

	if ( le->pos.trType != TR_STATIONARY ) {
		CG_SparkTrail( le ,le->trailLength
									,le->hShader
									,le->step
									,le->startSize
									,le->endSize);
	}

	if(cg.time >= le->startTime+100 && trace.fraction < 1.0 ){
		CG_AddRailParticleEXP( re->origin, le->pos.trDelta,5 ,600,
								  70,3,1);
		CG_FreeLocalEntity( le );
		return;
	}

	if(le->NumBounces >= 2 && trace.fraction < 1.0 && cg.time >= le->startTime+50){
		CG_AddRailParticleEXP( re->origin, le->pos.trDelta,5 ,600,
								  70,3,1);
		CG_FreeLocalEntity( le );
		return;
	}

	CG_ReflectVelocity( le, &trace );


}
/*
//////////////////////////////
Haste Effects
//////////////////////////////
*/
static void CG_AddHasteFX( localEntity_t *le ){
	clientInfo_t	*ci;
	refEntity_t		*legs;
	refEntity_t		*torso;
	refEntity_t		*head;
	int				clientNum;
	int				renderfx;



	clientNum = le->Cent->currentState.clientNum;

	ci = &cgs.clientinfo[ clientNum ];

	renderfx = 0;
	if ( le->Cent->currentState.number == cg.snap->ps.clientNum) {
		if (!cg.renderingThirdPerson) {
			renderfx = RF_THIRD_PERSON;			// only draw in mirrors
		} else {
			if (cg_cameraMode.integer) {
				return;
			}
		}
	}

	legs = &le->legs;
	torso = &le->torso;
	head = &le->head;

	//
	// add the legs
	//
	legs->hModel = ci->legsModel;
	if(ci->team == TEAM_RED){
		legs->customShader = cgs.media.teleportREffectShader;
	}else if(ci->team == TEAM_BLUE){
		legs->customShader = cgs.media.teleportBEffectShader;
	}else{
		legs->customShader = cgs.media.hasteSuitShader;
	}

	VectorCopy( le->pos.trBase, legs->origin );

	legs->reType = RT_MODEL;
	legs->renderfx = renderfx;
	legs->shaderRGBA[0] = (unsigned char) 0xff * ( le->endTime - cg.time ) * le->lifeRate;
	legs->shaderRGBA[1] = (unsigned char) 0xff * ( le->endTime - cg.time ) * le->lifeRate;
	legs->shaderRGBA[2] = (unsigned char) 0xff * ( le->endTime - cg.time ) * le->lifeRate;
	legs->shaderRGBA[3] = (unsigned char) 0xff * ( le->endTime - cg.time ) * le->lifeRate;
	trap_R_AddRefEntityToScene( legs);

	//
	// add the torso
	//
	torso->hModel = ci->torsoModel;
	if(ci->team == TEAM_RED){
		torso->customShader = cgs.media.teleportREffectShader;
	}else if(ci->team == TEAM_BLUE){
		torso->customShader = cgs.media.teleportBEffectShader;
	}else{
		torso->customShader = cgs.media.hasteSuitShader;
	}
	torso->reType = RT_MODEL;
	torso->renderfx = renderfx;
	torso->shaderRGBA[0] = (unsigned char) 0xff * ( le->endTime - cg.time ) * le->lifeRate;
	torso->shaderRGBA[1] = (unsigned char) 0xff * ( le->endTime - cg.time ) * le->lifeRate;
	torso->shaderRGBA[2] = (unsigned char) 0xff * ( le->endTime - cg.time ) * le->lifeRate;
	torso->shaderRGBA[3] = (unsigned char) 0xff * ( le->endTime - cg.time ) * le->lifeRate;
	trap_R_AddRefEntityToScene( torso );

	//
	// add the head
	//
	head->hModel = ci->headModel;
	head->customShader = cgs.media.hasteSuitShader;
	if(ci->team == TEAM_RED){
		head->customShader = cgs.media.teleportREffectShader;
	}else if(ci->team == TEAM_BLUE){
		head->customShader = cgs.media.teleportBEffectShader;
	}else{
		head->customShader = cgs.media.hasteSuitShader;
	}
	head->reType = RT_MODEL;
	head->renderfx = renderfx;
	head->shaderRGBA[0] = (unsigned char) 0xff * ( le->endTime - cg.time ) * le->lifeRate;
	head->shaderRGBA[1] = (unsigned char) 0xff * ( le->endTime - cg.time ) * le->lifeRate;
	head->shaderRGBA[2] = (unsigned char) 0xff * ( le->endTime - cg.time ) * le->lifeRate;
	head->shaderRGBA[3] = (unsigned char) 0xff * ( le->endTime - cg.time ) * le->lifeRate;
	trap_R_AddRefEntityToScene( head);
}


static void CG_IntPROPS(  localEntity_t *le ) {
		vec3_t origin,sprVel;

		VectorMA( le->pos.trBase, 8, le->pos.trDelta, origin );
		VectorScale( le->pos.trDelta, 64, sprVel );

	//	CG_ParticleExplosion( "explode1", origin, sprVel, 1400, 20, 30 );

		le->endTime = cg.time + le->duration*1000;
}

void CG_FireTrail( centity_t *ent) {
	localEntity_t	*le;

	le = CG_AllocLocalEntity();

	le->leType = LE_PROPS;
	le->leFlags |= LEF_NEVER_FREE;
	le->startTime = cg.time;
	le->endTime = cg.time + ent->currentState.origin2[0];

	le->refEntity.radius = ent->currentState.otherEntityNum2;
	le->duration = ent->currentState.origin2[0];
	le->speed = ent->currentState.origin2[1];
	le->count = ent->currentState.angles2[0];
	le->startSize = ent->currentState.angles2[1];
	le->endSize = ent->currentState.angles2[2];

	le->pos.trType = ent->currentState.pos.trType;
	VectorCopy(ent->currentState.pos.trBase,le->pos.trBase);
	VectorCopy(ent->currentState.angles,le->pos.trDelta);


}


//==============================================================================

/*
===================
CG_AddLocalEntities

===================
*/
void CG_AddLocalEntities( int view ) {
	localEntity_t	*le, *next;

	// walk the list backwards, so any new local entities generated
	// (trails, marks, etc) will be present this frame
	le = cg_activeLocalEntities.prev;
	le->view = view;

	if(cg_activeLocalEntities.leType == LE_PROPS){
		if ( cg.time >= le->endTime ) {
	//		CG_IntPROPS( le );
		}
	}

	for ( ; le != &cg_activeLocalEntities ; le = next ) {
		// grab next now, so if the local entity is freed we
		// still have it
		next = le->prev;

		if ( cg.time >= le->endTime ) {
			if(!(le->leFlags & LEF_NEVER_FREE)){
				CG_FreeLocalEntity( le );
				continue;
			}
		}
		switch ( le->leType ) {
		default:
			CG_Error( "Bad leType: %i", le->leType );
			break;

		case LE_MARK:
			break;

		case LE_SPRITE_EXPLOSION:
			CG_AddSpriteExplosion( le );
			break;

		case LE_EXPLOSION:
			CG_AddExplosion( le );
			break;

		case LE_FRAGMENT:			// gibs and brass
			CG_AddFragment( le );
			break;

//		case LE_ROCKET:			
	//		CL_FlyRocket( le );
		//	break;

//		case LE_PLASMA:			
	//		CL_FlyPlasma( le );
		//	break;

		case LE_BODYFRAGMENT:			// gibs and brass
			CG_AddBodyFragment( le );
			break;

		case LE_QFRAGMENT:			// gibs and brass
			CG_AddQFragment( le );
			break;

		case LE_ADDSPARK:			
			CG_AddSpark( le );
			break;

		case LE_ADDROCKETSPARK:			
			CG_AddRocketSpark( le );
			break;

		case LE_ADDGRENADESPARK:			
			CG_AddGrenadeSpark( le );
			break;

		case LE_ADDRAILSPARK:			
			CG_AddRailSpark( le );
			break;

		case LE_ADDBFGSPARK:			
			CG_AddBFGSpark( le );
			break;

		case LE_MOVE_SCALE_FADE:		// water bubbles
			CG_AddMoveScaleFade( le );
			break;

		case LE_PROPS:			
	//		CG_IntPROPS( le );
			break;

		case LE_FADE_RGB:				// teleporters, railtrails
			CG_AddFadeRGB( le );
			break;

		case LE_FADE_RGBA:				// teleporters, railtrails
			CG_AddFadeRGBA( le );
			break;

		case LE_FALL_SCALE_FADE: // gib blood trails
			CG_AddFallScaleFade( le );
			break;

		case LE_SCALE_FADE:		// rocket trails
			CG_AddScaleFade( le );
			break;

		case LE_SCOREPLUM:
			CG_AddScorePlum( le );
			break;

		case LE_RAIL_EFFECT:
			CG_AddRailEffect( le );
			break;

		case LE_FLARE:
			CG_Flare(le );
			break;

		case LE_RAIL_EFFECT2:
			CG_AddRailEffect2( le );
			break;

		case LE_ROCKETRING:
			CG_AddRocketRing( le );
			break;

		case LE_QUADSPHERE:
			CG_AddRocketQSphere( le );
			break;

		case LE_PQUADSPHERE:
			CG_AddPlasmaQSphere( le );
			break;

		case LE_RAILEXPRING:
			CG_AddRailEXPRing( le );
			break;

		case LE_RAILEXPRING2:
			CG_AddRailEXPRing2( le );
			break;

		case LE_ROCKETRING2:
			CG_AddRocketRing2( le );
			break;

		case LE_KAMIKAZE:
			CG_AddKamikaze( le );
			break;

		case LE_RAILRING:
			CG_AddRailRings( le );
			break;

		case LE_RAILDISC:
			CG_RGRingScaleFade( le );
			break;

		case LE_ROCKETEXP:
			CG_AddRLExplosion( le );
			break;

		case LE_RGEXP:
			CG_AddRgExplosion( le );
			break;

		case LE_RGPARTEXP:
			CG_AddRailSparkPARTICLE( le );
			break;

		case LE_HASTEFX:
			CG_AddHasteFX( le );
			break;

#ifdef MISSIONPACK

		case LE_INVULIMPACT:
			CG_AddInvulnerabilityImpact( le );
			break;

		case LE_SHOWREFENTITY:
			CG_AddRefEntity( le );
			break;
#endif
		}
	}
}
/**************************************
Below is nonfunctional client side calls for missles
for client side prediction. (to be added at a later 
date as an extra option geared more for 56k and other 
high ping connections so players can have fun without 
having to work so hard at telling the future on servers)
***************************************/

// NOTE: Clientside anything should ultimately be controlled
// by the server and for missles it should be server wide
// giving the client 0 control over prediction just to keep it fair.
/*
================
CL_RocketTrail

Leave expanding blood puffs behind gibs
================

void CL_RocketTrail( localEntity_t *le ) {
	int		t;
	int		t2;
	int		step;
	vec3_t	newOrigin;
	localEntity_t	*smoke;


	step = 30;
	t = step * ( (cg.time - cg.frametime + step ) / step );
	t2 = step * ( cg.time / step );

	for ( ; t <= t2; t += step ) {
		BG_EvaluateTrajectory( &le->pos, t, newOrigin );

		smoke = CG_SmokePuff( newOrigin, vec3_origin, 
					  32,		// radius
					  1, 1, 1, 0.33f,	// color
					  2000,		// trailTime
					  t,		// startTime
					  0,		// fadeInTime
					  0,		// flags
					  cgs.media.smokePuffShader );
		// use the optimized version
		smoke->leType = LE_SCALE_FADE;

	}
}
*/
/*
===============
CL_FlyRocket
===============

static void CL_FlyRocket( localEntity_t *le ) {
	refEntity_t			*ent;
	playerState_t		*s1;
	vec3_t				newOrigin;
	const weaponInfo_t	*weapon;
	trace_t				trace;
	vec3_t				velocity;
	centity_t	*cent;
	usercmd_t	hitCmd;
	int			cmdNum;

	cent = &cg.predictedPlayerEntity;

	ent = &le->refEntity;
		
	// calculate new position
	BG_EvaluateTrajectory( &le->pos, cg.time, newOrigin );

	// trace a line from previous position to new position
	CG_Trace( &trace, le->refEntity.origin, NULL, NULL, newOrigin, cg.snap->ps.clientNum, MASK_SHOT );

	s1 = &cg.predictedPlayerState;
	cmdNum = trap_GetCurrentCmdNumber();
	trap_GetUserCmd( cmdNum, &hitCmd );
	if ( trace.fraction == 1.0 ) {
		// still in free fall
		VectorCopy( newOrigin, le->refEntity.origin );
		if ( trap_CM_PointContents( trace.endpos, 0 ) & CONTENTS_NODROP ) {
			CG_FreeLocalEntity( le );
			return;
		}  if (trap_CM_PointContents( trace.endpos, 0 ) & SURF_SKY){
			CG_FreeLocalEntity( le );
			return;
		} else if (trap_CM_PointContents( trace.endpos, 0 ) & SURF_NOIMPACT){
			CG_FreeLocalEntity( le );
			return;
		}
	} else {
		if (trace.surfaceFlags & SURF_SKY){
			CG_FreeLocalEntity( le );
			return;
		} else if (trace.surfaceFlags & SURF_NOIMPACT){
			CG_FreeLocalEntity( le );
			return;
		} else if (trap_CM_PointContents( trace.endpos, 0 ) & CONTENTS_BODY || CONTENTS_CORPSE){
	//		CG_MissileHitPlayer( WP_ROCKET_LAUNCHER, le->refEntity.origin, trace.plane.normal, cg.snap->ps.clientNum );

//			trap_SetUserCmdValue( hitCmd.hitclientcl, qtrue );

//			trap_SetUserCmdValue( hitCmd.damage, cg.snap->ps.clientNum );
			CG_FreeLocalEntity( le );
			return;
		} else if (trap_CM_PointContents( trace.endpos, 0 ) & CONTENTS_SOLID){
			CG_MissileHitWall( WP_ROCKET_LAUNCHER, cg.predictedPlayerState.clientNum, le->refEntity.origin, trace.plane.normal, IMPACTSOUND_DEFAULT);
		//	CL_MissileImpact( cent, &trace );
			CG_FreeLocalEntity( le );
			return;
		}
	}


	if ( s1->weapon > WP_NUM_WEAPONS ) {
		s1->weapon = 0;
	}
	weapon = &cg_weapons[s1->weapon];

	trap_R_AddAdditiveLightToScene( le->refEntity.origin, 100+(rand()&60),0.2f, 0.2f, 0.2f  );

	BG_EvaluateTrajectoryDelta( &le->currentState.pos, cg.time, velocity );

	trap_S_AddLoopingSound( 0, le->refEntity.origin, velocity, trap_S_RegisterSound( "sound/weapons/rocket/rockfly.wav", qfalse ));

	// create the render entity
	memset (&ent, 0, sizeof(ent));

	if ( cg_noProjectileTrail.integer == 0) {
		CL_RocketTrail( le );
	}

	// flicker between two skins
	ent->renderfx = weapon->missileRenderfx | RF_NOSHADOW;

	// convert direction of travel into axis
	if ( VectorNormalize2( le->pos.trDelta, le->refEntity.axis[0] ) == 0 ) {
		le->refEntity.axis[0][2] = 0;
	}

	// spin as it moves
	if ( le->pos.trType != TR_STATIONARY ) {
			RotateAroundDirection( le->refEntity.axis, cg.time / 4 );
	}
	
	trap_R_AddRefEntityToScene( &le->refEntity );
}
*/
/*
===============
CL_FlyPlasma
===============

static void CL_FlyPlasma( localEntity_t *le ) {
	refEntity_t			*ent;
	playerState_t		*s1;
	vec3_t				newOrigin;
	const weaponInfo_t	*weapon;
	trace_t				trace;
	vec3_t				velocity;
	centity_t	*cent;

	cent = &cg.predictedPlayerEntity;

	ent = &le->refEntity;
		
	// calculate new position
	BG_EvaluateTrajectory( &le->pos, cg.time, newOrigin );

	// trace a line from previous position to new position
	CG_Trace( &trace, le->refEntity.origin, NULL, NULL, newOrigin, cg.predictedPlayerState.clientNum, MASK_SHOT );


	if ( trace.fraction == 1.0 ) {
		// still in free fall
		VectorCopy( newOrigin, le->refEntity.origin );
		if ( trap_CM_PointContents( trace.endpos, 0 ) & CONTENTS_NODROP ) {
			CG_FreeLocalEntity( le );
			return;
		}  if (trap_CM_PointContents( trace.endpos, 0 ) & SURF_SKY){
			CG_FreeLocalEntity( le );
			return;
		} else if (trap_CM_PointContents( trace.endpos, 0 ) & SURF_NOIMPACT){
			CG_FreeLocalEntity( le );
			return;
		}
	} else {
		if (trace.surfaceFlags & SURF_SKY){
			CG_FreeLocalEntity( le );
			return;
		} else if (trace.surfaceFlags & SURF_NOIMPACT){
			CG_FreeLocalEntity( le );
			return;
	//	} else if (trap_CM_PointContents( trace.endpos, 0 ) & CONTENTS_BODY || CONTENTS_CORPSE){
	//		CG_MissileHitPlayer( WP_PLASMAGUN, le->refEntity.origin, trace.plane.normal, cg.predictedPlayerState.clientNum );
//			CL_MissileImpact( cent, &trace );
	//		CG_FreeLocalEntity( le );
	//		return;
		} else if (trap_CM_PointContents( trace.endpos, 0 ) & CONTENTS_SOLID){
	//		CG_MissileHitWall( WP_PLASMAGUN, cg.predictedPlayerState.clientNum, le->refEntity.origin, trace.plane.normal, IMPACTSOUND_DEFAULT);
//			CL_MissileImpact( cent, &trace );
			CG_FreeLocalEntity( le );
			return;
		}
	}

	s1 = &cg.snap->ps;
	if ( s1->weapon > WP_NUM_WEAPONS ) {
		s1->weapon = 0;
	}
	weapon = &cg_weapons[s1->weapon];

	trap_R_AddAdditiveLightToScene( le->refEntity.origin, 50,0.0f, 0.1f, 0.4f  );

	BG_EvaluateTrajectoryDelta( &le->currentState.pos, cg.time, velocity );

	trap_S_AddLoopingSound( 0, le->refEntity.origin, velocity, trap_S_RegisterSound( "sound/weapons/plasma/lasfly.wav", qfalse ));

	// create the render entity
	memset (&ent, 0, sizeof(ent));

	if ( cg_noProjectileTrail.integer == 0) {
//		CL_RocketTrail( le );
	}

	// flicker between two skins
	ent->renderfx = weapon->missileRenderfx | RF_NOSHADOW;

	// convert direction of travel into axis
	if ( VectorNormalize2( le->pos.trDelta, le->refEntity.axis[0] ) == 0 ) {
		le->refEntity.axis[0][2] = 1;
	}

	// spin as it moves
	if ( le->pos.trType != TR_STATIONARY ) {
			RotateAroundDirection( le->refEntity.axis, cg.time / 4 );
	}
	
	trap_R_AddRefEntityToScene( &le->refEntity );
}
*/



