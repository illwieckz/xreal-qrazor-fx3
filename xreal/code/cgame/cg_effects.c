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
// cg_effects.c -- these functions generate localentities, usually as a result
// of event processing

#include "cg_local.h"


/*
==================
CG_BubbleTrail

Bullets shot underwater
==================
*/
void CG_BubbleTrail(vec3_t start, vec3_t end, float spacing)
{
	vec3_t          move;
	vec3_t          vec;
	float           len;
	int             i;

	if(cg_noProjectileTrail.integer)
	{
		return;
	}

	VectorCopy(start, move);
	VectorSubtract(end, start, vec);
	len = VectorNormalize(vec);

	// advance a random amount first
	i = rand() % (int)spacing;
	VectorMA(move, i, vec, move);

	VectorScale(vec, spacing, vec);

	for(; i < len; i += spacing)
	{
		/*
		   localEntity_t    *le;
		   refEntity_t      *re;

		   le = CG_AllocLocalEntity();
		   le->leFlags = LEF_PUFF_DONT_SCALE;
		   le->leType = LE_MOVE_SCALE_FADE;
		   le->startTime = cg.time;
		   le->endTime = cg.time + 1000 + random() * 250;
		   le->lifeRate = 1.0 / ( le->endTime - le->startTime );

		   re = &le->refEntity;
		   re->shaderTime = cg.time / 1000.0f;

		   re->reType = RT_SPRITE;
		   re->rotation = 0;
		   re->radius = 3;
		   re->customShader = cgs.media.waterBubbleShader;
		   re->shaderRGBA[0] = 0xff;
		   re->shaderRGBA[1] = 0xff;
		   re->shaderRGBA[2] = 0xff;
		   re->shaderRGBA[3] = 0xff;

		   le->color[3] = 1.0;

		   le->pos.trType = TR_LINEAR;
		   le->pos.trTime = cg.time;
		   VectorCopy( move, le->pos.trBase );
		   le->pos.trDelta[0] = crandom()*5;
		   le->pos.trDelta[1] = crandom()*5;
		   le->pos.trDelta[2] = crandom()*5 + 6;
		 */
		if(spacing < 8)
		{
			CG_ParticleBubbles(cgs.media.waterBubbleShader, move, 0.3f, 6.0f);
		}
		else
		{
			CG_ParticleBubbles(cgs.media.waterBubbleShader, move, 2, 0.5f);
		}
		VectorAdd(move, vec, move);
	}
}

/*
=====================
CG_SmokePuff

Adds a smoke puff or blood trail localEntity.
=====================
*/
localEntity_t  *CG_SmokePuff(const vec3_t p, const vec3_t vel,
							 float radius,
							 float r, float g, float b, float a,
							 float duration, int startTime, int fadeInTime, int leFlags, qhandle_t hShader)
{
	static int      seed = 0x92;
	localEntity_t  *le;
	refEntity_t    *re;

//  int fadeInTime = startTime + duration / 2;

	le = CG_AllocLocalEntity();
	le->leFlags = leFlags;
	le->radius = radius;

	re = &le->refEntity;
	re->rotation = Q_random(&seed) * 360;
	re->radius = radius;
	re->shaderTime = startTime / 1000.0f;

	le->leType = LE_MOVE_SCALE_FADE;
	le->entNumber = le - cg_freeLocalEntities;
	le->startTime = startTime;
	le->fadeInTime = fadeInTime;
	le->endTime = startTime + duration;
	if(fadeInTime > startTime)
	{
		le->lifeRate = 1.0 / (le->endTime - le->fadeInTime);
	}
	else
	{
		le->lifeRate = 1.0 / (le->endTime - le->startTime);
	}
	le->color[0] = r;
	le->color[1] = g;
	le->color[2] = b;
	le->color[3] = a;


	le->pos.trType = TR_LINEAR;
	le->pos.trTime = startTime;
	VectorCopy(vel, le->pos.trDelta);
	VectorCopy(p, le->pos.trBase);

	VectorCopy(p, re->origin);
	re->customShader = hShader;

	// rage pro can't alpha fade, so use a different shader

	re->shaderRGBA[0] = le->color[0] * 0xff;
	re->shaderRGBA[1] = le->color[1] * 0xff;
	re->shaderRGBA[2] = le->color[2] * 0xff;
	re->shaderRGBA[3] = 0xff;


	re->reType = RT_SPRITE;
	re->radius = le->radius;

	return le;
}

/*
==================
CG_SpawnEffect

Player teleporting in or out
==================
*/
void CG_SpawnEffect(vec3_t org, int team)
{
	localEntity_t  *le;
	refEntity_t    *re;

	le = CG_AllocLocalEntity();
	le->leFlags = 0;
	le->leType = LE_FADE_RGB;
	le->entNumber = le - cg_freeLocalEntities;
	le->startTime = cg.time;
	le->endTime = cg.time + 500;
	le->lifeRate = 1.0 / (le->endTime - le->startTime);

	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0;

	re = &le->refEntity;

	re->reType = RT_MODEL;
	re->shaderTime = cg.time / 1000.0f;

	if(cgs.gametype >= GT_TEAM)
	{
		if(team == TEAM_RED)
		{
			re->customShader = cgs.media.teleportREffectShader;
		}
		else if(team == TEAM_BLUE)
		{
			re->customShader = cgs.media.teleportBEffectShader;
		}
	}
	else
	{
		re->customShader = cgs.media.teleportEffectShader;
	}


	re->hModel = cgs.media.teleportEffectModel;
	AxisClear(re->axis);

	VectorCopy(org, re->origin);

	re->origin[2] -= 30;

}

/*
==================
CG_KamikazeEffect
==================
*/
void CG_KamikazeEffect(vec3_t org)
{
	localEntity_t  *le;
	refEntity_t    *re;

	le = CG_AllocLocalEntity();
	le->leFlags = 0;
	le->leType = LE_KAMIKAZE;
	le->entNumber = le - cg_freeLocalEntities;
	le->startTime = cg.time;
	le->endTime = cg.time + 3000;	//2250;
	le->lifeRate = 1.0 / (le->endTime - le->startTime);

	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0;

	VectorClear(le->angles.trBase);

	re = &le->refEntity;

	re->reType = RT_MODEL;
	re->shaderTime = cg.time / 1000.0f;

	re->hModel = cgs.media.kamikazeEffectModel;

	VectorCopy(org, re->origin);

}

#ifdef MISSIONPACK
/*
===============
CG_LightningBoltBeam
===============
*/
void CG_LightningBoltBeam(vec3_t start, vec3_t end)
{
	localEntity_t  *le;
	refEntity_t    *beam;

	le = CG_AllocLocalEntity();
	le->leFlags = 0;
	le->leType = LE_SHOWREFENTITY;
	le->entNumber = le - cg_activeLocalEntities;
	le->startTime = cg.time;
	le->endTime = cg.time + 50;

	beam = &le->refEntity;

	VectorCopy(start, beam->origin);
	// this is the end point
	VectorCopy(end, beam->oldorigin);

	beam->reType = RT_LIGHTNING;
	beam->customShader = cgs.media.lightningShader;
}



/*
==================
CG_ObeliskExplode
==================
*/
void CG_ObeliskExplode(vec3_t org, int entityNum)
{

	vec3_t          origin;

	// create an explosion
	VectorCopy(org, origin);
	origin[2] += 64;

}

/*
==================
CG_ObeliskPain
==================
*/
void CG_ObeliskPain(vec3_t org)
{
	float           r;
	sfxHandle_t     sfx;

	// hit sound
	r = rand() & 3;
	if(r < 2)
	{
		sfx = cgs.media.obeliskHitSound1;
	}
	else if(r == 2)
	{
		sfx = cgs.media.obeliskHitSound2;
	}
	else
	{
		sfx = cgs.media.obeliskHitSound3;
	}
	trap_S_StartSound(org, ENTITYNUM_NONE, CHAN_BODY, sfx);
}


/*
==================
CG_InvulnerabilityImpact
==================
*/
void CG_InvulnerabilityImpact(vec3_t org, vec3_t angles)
{
	localEntity_t  *le;
	refEntity_t    *re;
	int             r;
	sfxHandle_t     sfx;

	le = CG_AllocLocalEntity();
	le->leFlags = 0;
	le->leType = LE_INVULIMPACT;
	le->entNumber = le - cg_activeLocalEntities;
	le->startTime = cg.time;
	le->endTime = cg.time + 1000;
	le->lifeRate = 1.0 / (le->endTime - le->startTime);

	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0;

	re = &le->refEntity;

	re->reType = RT_MODEL;
	re->shaderTime = cg.time / 1000.0f;

	re->hModel = cgs.media.invulnerabilityImpactModel;

	VectorCopy(org, re->origin);
	AnglesToAxis(angles, re->axis);

	r = rand() & 3;
	if(r < 2)
	{
		sfx = cgs.media.invulnerabilityImpactSound1;
	}
	else if(r == 2)
	{
		sfx = cgs.media.invulnerabilityImpactSound2;
	}
	else
	{
		sfx = cgs.media.invulnerabilityImpactSound3;
	}
	trap_S_StartSound(org, ENTITYNUM_NONE, CHAN_BODY, sfx);
}

/*
==================
CG_InvulnerabilityJuiced
==================
*/
void CG_InvulnerabilityJuiced(vec3_t org)
{
	localEntity_t  *le;
	refEntity_t    *re;
	vec3_t          angles;

	le = CG_AllocLocalEntity();
	le->leFlags = 0;
	le->leType = LE_INVULJUICED;
	le->entNumber = le - cg_activeLocalEntities;
	le->startTime = cg.time;
	le->endTime = cg.time + 10000;
	le->lifeRate = 1.0 / (le->endTime - le->startTime);

	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0;

	re = &le->refEntity;

	re->reType = RT_MODEL;
	re->shaderTime = cg.time / 1000.0f;

	re->hModel = cgs.media.invulnerabilityJuicedModel;

	VectorCopy(org, re->origin);
	VectorClear(angles);
	AnglesToAxis(angles, re->axis);

	trap_S_StartSound(org, ENTITYNUM_NONE, CHAN_BODY, cgs.media.invulnerabilityJuicedSound);
}

#endif

/*
==================
CG_ScorePlum
==================
*/
void CG_ScorePlum(int client, vec3_t org, int score)
{
	localEntity_t  *le;
	refEntity_t    *re;
	vec3_t          angles;
	static vec3_t   lastPos;

	// only visualize for the client that scored
	if(client != cg.predictedPlayerState.clientNum || cg_scorePlum.integer == 0)
	{
		return;
	}

	le = CG_AllocLocalEntity();
	le->leFlags = 0;
	le->leType = LE_SCOREPLUM;
	le->entNumber = le - cg_freeLocalEntities;
	le->startTime = cg.time;
	le->endTime = cg.time + 4000;
	le->lifeRate = 1.0 / (le->endTime - le->startTime);


	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0;
	le->radius = score;

	VectorCopy(org, le->pos.trBase);
	if(org[2] >= lastPos[2] - 20 && org[2] <= lastPos[2] + 20)
	{
		le->pos.trBase[2] -= 20;
	}

	//CG_Printf( "Plum origin %i %i %i -- %i\n", (int)org[0], (int)org[1], (int)org[2], (int)Distance(org, lastPos));
	VectorCopy(org, lastPos);


	re = &le->refEntity;

	re->reType = RT_SPRITE;
	re->radius = 16;

	VectorClear(angles);
	AnglesToAxis(angles, re->axis);
}


/*
====================
CG_MakeExplosion
====================
*/
localEntity_t  *CG_MakeExplosion2(vec3_t origin, vec3_t dir,
								  qhandle_t hModel, qhandle_t shader, int msec, qboolean isSprite, int radius, int team)
{
	float           ang;
	localEntity_t  *ex;
	int             offset;
	vec3_t          newOrigin;

	if(msec <= 0)
	{
		CG_Error("CG_MakeExplosion: msec = %i", msec);
	}

	// skew the time a bit so they aren't all in sync
	offset = rand() & 63;

	ex = CG_AllocLocalEntity();

	ex->leType = LE_EXPLOSION;
	ex->entNumber = ex - cg_freeLocalEntities;
	VectorCopy(origin, newOrigin);

	// set axis with random rotate
	if(!dir)
	{
		AxisClear(ex->refEntity.axis);
	}
	else
	{
		ang = rand() % 360;
		VectorCopy(dir, ex->refEntity.axis[0]);
		RotateAroundDirection(ex->refEntity.axis, ang);
	}
	ex->refEntity.hModel = hModel;


	ex->startTime = cg.time - offset;
	ex->endTime = ex->startTime + msec;

	// bias the time so all shader effects start correctly
	ex->refEntity.shaderTime = ex->startTime / 1000.0f;


	ex->refEntity.customShader = shader;

	// set origin
	VectorCopy(newOrigin, ex->refEntity.origin);
	VectorCopy(newOrigin, ex->refEntity.oldorigin);
	if(cgs.gametype >= GT_TEAM)
	{
		if(team == TEAM_RED)
		{
			ex->color[0] = 1.0;
			ex->color[1] = 0;
			ex->color[2] = 0;
		}
		else if(team == TEAM_BLUE)
		{
			ex->color[0] = 0;
			ex->color[1] = 0;
			ex->color[2] = 1.0;
		}
	}
	else
	{
		ex->color[0] = 0;
		ex->color[1] = 1.0;
		ex->color[2] = 0;
	}

	return ex;
}

localEntity_t  *CG_MakeExplosion(vec3_t origin, vec3_t dir,
								 qhandle_t hModel, qhandle_t shader, int msec, qboolean isSprite, int radius)
{
	float           ang;
	localEntity_t  *ex;
	int             offset;
	vec3_t          tmpVec, newOrigin;

	if(msec <= 0)
	{
		CG_Error("CG_MakeExplosion: msec = %i", msec);
	}

	// skew the time a bit so they aren't all in sync
	offset = rand() & 63;

	ex = CG_AllocLocalEntity();
	if(isSprite)
	{
		ex->leType = LE_SPRITE_EXPLOSION;
		ex->entNumber = ex - cg_freeLocalEntities;
		ex->radius = radius;
		// randomly rotate sprite orientation
		ex->refEntity.rotation = rand() % 360;
		VectorScale(dir, 16, tmpVec);
		VectorAdd(tmpVec, origin, newOrigin);
	}
	else
	{
		ex->leType = LE_EXPLOSION;
		ex->entNumber = ex - cg_freeLocalEntities;
		VectorCopy(origin, newOrigin);

		// set axis with random rotate
		if(!dir)
		{
			AxisClear(ex->refEntity.axis);
		}
		else
		{
			ang = rand() % 360;
			VectorCopy(dir, ex->refEntity.axis[0]);
			RotateAroundDirection(ex->refEntity.axis, ang);
		}
		ex->refEntity.hModel = hModel;
	}

	ex->startTime = cg.time - offset;
	ex->endTime = ex->startTime + msec;

	// bias the time so all shader effects start correctly
	ex->refEntity.shaderTime = ex->startTime / 1000.0f;


	ex->refEntity.customShader = shader;

	// set origin
	VectorCopy(newOrigin, ex->refEntity.origin);
	VectorCopy(newOrigin, ex->refEntity.oldorigin);

	ex->color[0] = ex->color[1] = ex->color[2] = 1.0;

	return ex;
}


/*
=================
CG_Bleed

This is the spurt of blood when a character gets hit
=================
*/
void CG_Bleed(vec3_t origin, int entityNum)
{
	localEntity_t  *ex;

	if(!cg_blood.integer)
	{
		return;
	}

	ex = CG_AllocLocalEntity();
	ex->leType = LE_EXPLOSION;
	ex->entNumber = ex - cg_freeLocalEntities;
	ex->startTime = cg.time;
	ex->endTime = ex->startTime + 500;

	VectorCopy(origin, ex->refEntity.origin);
	ex->refEntity.reType = RT_SPRITE;
	ex->refEntity.rotation = rand() % 360;
	ex->refEntity.radius = 10;

	ex->refEntity.customShader = cgs.media.bloodExplosionShader;

	// don't show player's own blood in view
	if(entityNum == cg.snap->ps.clientNum)
	{
		ex->refEntity.renderfx |= RF_THIRD_PERSON;
	}
}

#define	GIB_VELOCITY	250
#define	GIB_JUMP		250



/*
==================
CG_LaunchGib
==================
*/
static void CG_LaunchGib(vec3_t origin, qhandle_t hModel, vec3_t dir, centity_t * cent, int PercentOnFire)
{
	localEntity_t  *le;
	refEntity_t    *re;
	vec3_t          velocity, gdir, ggdir;
	vec3_t          axis[3], angles;
	int             random;
	vec3_t          mins = { -7, -7, -7 };
	vec3_t          maxs = { 7, 7, 7 };


	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_FRAGMENT;
	le->entNumber = le - cg_freeLocalEntities;

	le->startTime = cg.time;
	le->endTime = le->startTime + 5000 + random() * 3000;
	VectorCopy(maxs, le->maxs);
	VectorCopy(mins, le->mins);


	VectorSubtract(origin, re->origin, gdir);
	VectorNormalize2(gdir, ggdir);
	velocity[0] = (ggdir[0] + gdir[0] / (109.3f + random())) + (crandom() * GIB_VELOCITY);
	velocity[1] = (ggdir[1] + gdir[1] / (109.3f + random())) + (crandom() * GIB_VELOCITY);
	velocity[2] = GIB_JUMP + crandom() * GIB_VELOCITY;
	VectorMA(velocity, GIB_VELOCITY, dir, velocity);


	AxisToAngles(axis, angles);
	VectorCopy(angles, le->angles.trBase);
	VectorCopy(origin, re->origin);
	AnglesToAxis(angles, re->axis);
	VectorCopy(origin, le->pos.trBase);

	le->pos.trType = TR_GRAVITY;

	re->hModel = hModel;
	le->bounceFactor = 0.6f;

	if(!le->watertraced)
	{
		if(CG_PointContents(origin, -1) & CONTENTS_WATER)
		{
			le->inwater = qfalse;
		}
		le->watertraced = qtrue;
	}

	if(cg_blood.integer)
	{
		le->leBounceSoundType = LEBS_BLOOD;
		le->leMarkType = LEMT_BLOOD;
		le->angles.trType = TR_LINEAR;
		le->angles.trTime = cg.time;
		le->angles.trDelta[0] = (10 + (rand() & 50)) - 30;
		le->angles.trDelta[1] = (100 + (rand() & 500)) - 300;
		le->angles.trDelta[2] = (10 + (rand() & 50)) - 30;
		le->angVel = 20 * crandom();	// random angular velocity
		le->rotAxis[0] = crandom();	// random axis of rotation
		le->rotAxis[1] = crandom();
		le->rotAxis[2] = crandom();
		VectorNormalize(le->rotAxis);	// normalize the rotation axis
		QuatInit(1, 0, 0, 0, le->quatRot);
		QuatInit(1, 0, 0, 0, le->quatOrient);
		le->radius = 12;
		le->leFlags |= LEF_TUMBLE;
	}
	VectorCopy(origin, le->pos.trBase);
	VectorCopy(velocity, le->pos.trDelta);
	le->pos.trTime = cg.time;

	random = rand() % 100;
	if(random < 0)
	{
		random = 0;
	}
	if(random > 100)
	{
		random = 100;
	}
	if(cent->currentState.generic1 & GNF_ONFIRE)
	{
		le->leFlags |= LEF_ON_FIRE;
	}
	else if(random >= 0 && random < PercentOnFire)
	{
		le->leFlags |= LEF_ON_FIRE;
	}
}

/*
==================
CG_TossWeapon
==================
*/
static void CG_TossWeapon(vec3_t origin, vec3_t velocity, qhandle_t hModel, clientInfo_t * ci)
{
	localEntity_t  *le;
	refEntity_t    *re;
	int             contents;


	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	contents = CG_PointContents(origin, -1);
	le->leType = LE_BODYFRAGMENT;
	le->entNumber = le - cg_freeLocalEntities;
	le->startTime = cg.time;
	le->endTime = le->startTime + 5000 + random() * 3000;

	le->pos.trType = TR_GRAVITY;



	VectorCopy(origin, re->origin);
	AxisCopy(axisDefault, re->axis);
	re->hModel = hModel;



	le->radius = 100;
	le->width = 100;

	VectorCopy(origin, le->pos.trBase);
	VectorCopy(velocity, le->pos.trDelta);
	le->pos.trTime = cg.time;

	le->bounceFactor = 0.2f;

	if(!le->watertraced)
	{
		if(CG_PointContents(origin, -1) & CONTENTS_WATER)
		{
			le->inwater = qfalse;
		}
		le->watertraced = qtrue;
	}

	re->shaderRGBA[0] = ci->color1[0] * 1.0f * 255;
	re->shaderRGBA[1] = ci->color1[1] * 1.0f * 255;
	re->shaderRGBA[2] = ci->color1[2] * 1.0f * 255;
	re->shaderRGBA[3] = 255;

	le->angles.trType = TR_LINEAR;
	le->angles.trTime = cg.time;
	le->angVel = 10 * crandom();	// random angular velocity
	le->rotAxis[0] = 2;			// random axis of rotation
	le->rotAxis[1] = 0;
	le->rotAxis[2] = 0;
	VectorNormalize(le->rotAxis);	// normalize the rotation axis
	QuatClear(le->quatRot);
	QuatClear(le->quatOrient);
	le->radius = 12;
	le->leFlags = LEF_TUMBLE;
}

static void CG_TossHead(centity_t * cent, vec3_t origin, vec3_t velocity, qhandle_t model, qhandle_t skin)
{
	localEntity_t  *le;
	refEntity_t    *re;
	int             contents;
	vec3_t          mins = { -7, -7, -7 };
	vec3_t          maxs = { 7, 7, 7 };


	le = CG_AllocLocalEntity();
	re = &le->refEntity;


	contents = CG_PointContents(origin, +10);
	le->leType = LE_BODYFRAGMENT;
	le->entNumber = le - cg_freeLocalEntities;
	re->hModel = model;
	if(!re->hModel)
	{
		return;
	}
	re->customSkin = skin;
	re->renderfx = RF_NOSHADOW;
	VectorCopy(maxs, le->maxs);
	VectorCopy(mins, le->mins);
	le->startTime = cg.time;
	le->endTime = le->startTime + 5000 + random() * 3000;
	le->pos.trType = TR_GRAVITY;
	le->radius = 10;
	le->width = 10;
	VectorCopy(origin, re->origin);
	AxisCopy(axisDefault, re->axis);
	VectorCopy(origin, le->pos.trBase);
	VectorCopy(velocity, le->pos.trDelta);



	le->pos.trTime = cg.time;


	le->bounceFactor = 0.3f;

	if(!le->watertraced)
	{
		if(CG_PointContents(origin, -1) & CONTENTS_WATER)
		{
			le->inwater = qfalse;
		}
		le->watertraced = qtrue;
	}

	le->angles.trType = TR_LINEAR;
	le->angles.trTime = cg.time;
	le->angVel = 15;			// random angular velocity
	le->rotAxis[0] = 1;			// random axis of rotation
	le->rotAxis[1] = 0;
	le->rotAxis[2] = 1;
	VectorNormalize(le->rotAxis);	// normalize the rotation axis
	QuatInit(0, 0, 0, 1, le->quatRot);
	QuatInit(1, 0, 0, 0, le->quatOrient);
	le->leFlags |= LEF_TUMBLE;
	if(cent->currentState.generic1 & GNF_ONFIRE)
	{
		le->leFlags |= LEF_ON_FIRE;
	}

}

/*
==================
CG_LaunchQuadGib
==================
*/
static void CG_LaunchQuadGib(vec3_t origin, qhandle_t hModel, vec3_t dir, int team, centity_t * cent, int PercentOnFire)
{
	localEntity_t  *le;
	refEntity_t    *re;
	vec3_t          velocity, gdir, ggdir;
	vec3_t          axis[3], angles;
	int             random;
	vec3_t          mins = { -7, -7, -7 };
	vec3_t          maxs = { 7, 7, 7 };


	le = CG_AllocLocalEntity();
	re = &le->refEntity;


	le->leType = LE_QFRAGMENT;
	le->entNumber = le - cg_freeLocalEntities;
	le->startTime = cg.time;
	le->endTime = le->startTime + 5000 + random() * 3000;
	VectorCopy(maxs, le->maxs);
	VectorCopy(mins, le->mins);

	le->pos.trType = TR_GRAVITY;

	VectorSubtract(origin, re->origin, gdir);
	VectorNormalize2(gdir, ggdir);
	velocity[0] = (ggdir[0] + gdir[0] / (109.3f + random())) + (crandom() * GIB_VELOCITY);
	velocity[1] = (ggdir[1] + gdir[1] / (109.3f + random())) + (crandom() * GIB_VELOCITY);
	velocity[2] = GIB_JUMP + crandom() * GIB_VELOCITY;
	VectorMA(velocity, GIB_VELOCITY, dir, velocity);
	AxisToAngles(axis, angles);
	VectorCopy(angles, le->angles.trBase);
	VectorCopy(origin, re->origin);
	AnglesToAxis(angles, re->axis);


	re->hModel = hModel;
	le->bounceFactor = 0.6f;

	if(!le->watertraced)
	{
		if(CG_PointContents(origin, -1) & CONTENTS_WATER)
		{
			le->inwater = qfalse;
		}
		le->watertraced = qtrue;
	}

	le->generic1 = team;

	if(cg_blood.integer)
	{
		le->leBounceSoundType = LEBS_BLOOD;
		le->leMarkType = LEMT_BLOOD;
		le->angles.trType = TR_LINEAR;
		le->angles.trTime = cg.time;
		le->angles.trDelta[0] = (10 + (rand() & 50)) - 30;
		le->angles.trDelta[1] = (100 + (rand() & 500)) - 300;
		le->angles.trDelta[2] = (10 + (rand() & 50)) - 30;
		le->angVel = 20 * crandom();	// random angular velocity
		le->rotAxis[0] = crandom();	// random axis of rotation
		le->rotAxis[1] = crandom();
		le->rotAxis[2] = crandom();
		VectorNormalize(le->rotAxis);	// normalize the rotation axis
		QuatInit(1, 0, 0, 0, le->quatRot);
		QuatInit(1, 0, 0, 0, le->quatOrient);
		le->radius = 12;
		le->leFlags |= LEF_TUMBLE;
	}
	VectorCopy(origin, le->pos.trBase);
	VectorCopy(velocity, le->pos.trDelta);
	le->pos.trTime = cg.time;

	random = rand() % 100;
	if(random < 0)
	{
		random = 0;
	}
	if(random > 100)
	{
		random = 100;
	}
	if(cent->currentState.generic1 & GNF_ONFIRE)
	{
		le->leFlags |= LEF_ON_FIRE;
	}
	else if(random >= 0 && random < PercentOnFire)
	{
		le->leFlags |= LEF_ON_FIRE;
	}

}

/*
===================
CG_GibPlayer

Generated a bunch of gibs launching out from the bodies location
===================
*/
#define MAXJUNCTIONS 8
void CG_GibPlayer(vec3_t playerOrigin, int clientNum, qhandle_t model, vec3_t gdir, int PercentOnFire)
{
	vec3_t          origin, velocity, dir;
	clientInfo_t   *ci;
	centity_t      *cent;
	int             i, j;
	qhandle_t       hshader;

	cent = &cg_entities[clientNum];

	ci = &cgs.clientinfo[clientNum];

	// allow gibs to be turned off for speed
	if(!cg_gibs.integer)
	{
		return;
	}

	VectorCopy(playerOrigin, origin);

	AxisToAngles(cg.autoAxis, dir);
	for(j = 0; j < 20; j++)
	{

		i = rand() % 1;
		if(i < 0)
		{
			i = 0;
		}
		if(i > 1)
		{
			i = 1;
		}

		if(i == 0)
		{
			hshader = cgs.media.zbloodSpray1Shader;
		}
		else if(i == 1)
		{
			hshader = cgs.media.zbloodSpray2Shader;
		}

		CG_Particle_Full360BurstBlood(hshader, origin, dir);
	}

	if(cgs.InstaGib == 1)
	{
		velocity[0] = 250;
		velocity[1] = 250;
		velocity[2] = GIB_JUMP + crandom() * GIB_VELOCITY;
		if(!model)
		{
			CG_TossWeapon(origin, velocity, trap_R_RegisterModel("models/weapons2/gauntlet/gauntlet.md3"), ci);
		}
		else
		{
			CG_TossWeapon(origin, velocity, model, ci);
		}
	}

	CG_LaunchGib(origin, cgs.media.gibArm, gdir, cent, PercentOnFire);

	VectorCopy(playerOrigin, origin);
	CG_LaunchGib(origin, cgs.media.gibChest, gdir, cent, PercentOnFire);

	VectorCopy(playerOrigin, origin);
	CG_LaunchGib(origin, cgs.media.gibChest, gdir, cent, PercentOnFire);

	VectorCopy(playerOrigin, origin);
	CG_LaunchGib(origin, cgs.media.gibChest, gdir, cent, PercentOnFire);

	VectorCopy(playerOrigin, origin);
	CG_LaunchGib(origin, cgs.media.gibForearm, gdir, cent, PercentOnFire);

	VectorCopy(playerOrigin, origin);
	CG_LaunchGib(origin, cgs.media.gibIntestine, gdir, cent, PercentOnFire);

	VectorCopy(playerOrigin, origin);
	CG_LaunchGib(origin, cgs.media.gibAbdomen, gdir, cent, PercentOnFire);

	VectorCopy(playerOrigin, origin);
	CG_LaunchGib(origin, cgs.media.gibChest, gdir, cent, PercentOnFire);

	VectorCopy(playerOrigin, origin);
	CG_LaunchGib(origin, cgs.media.gibArm, gdir, cent, PercentOnFire);

	VectorCopy(playerOrigin, origin);
	CG_LaunchGib(origin, cgs.media.gibFist, gdir, cent, PercentOnFire);

	VectorCopy(playerOrigin, origin);
	CG_LaunchGib(origin, cgs.media.gibFoot, gdir, cent, PercentOnFire);

}

void CG_GibPlayerHead(vec3_t playerOrigin, int clientNum, qhandle_t model, vec3_t gdir, int PercentOnFire)
{
	vec3_t          origin, velocity, dir;
	clientInfo_t   *ci;
	centity_t      *cent;
	int             i, j;
	qhandle_t       hshader;

	cent = &cg_entities[clientNum];

	ci = &cgs.clientinfo[clientNum];

	// allow gibs to be turned off for speed
	if(!cg_gibs.integer)
	{
		return;
	}

	VectorCopy(playerOrigin, origin);

	AxisToAngles(cg.autoAxis, dir);
	for(j = 0; j < 20; j++)
	{

		i = rand() % 1;
		if(i < 0)
		{
			i = 0;
		}
		if(i > 1)
		{
			i = 1;
		}

		if(i == 0)
		{
			hshader = cgs.media.zbloodSpray1Shader;
		}
		else if(i == 1)
		{
			hshader = cgs.media.zbloodSpray2Shader;
		}

		CG_Particle_Full360BurstBlood(hshader, origin, dir);
	}


	if(cgs.InstaGib == 1)
	{
		velocity[0] = 250;
		velocity[1] = 250;
		velocity[2] = GIB_JUMP + crandom() * GIB_VELOCITY;
		if(!model)
		{
			CG_TossWeapon(origin, velocity, trap_R_RegisterModel("models/weapons2/gauntlet/gauntlet.md3"), ci);
		}
		else
		{
			CG_TossWeapon(origin, velocity, model, ci);
		}
	}

	VectorCopy(playerOrigin, origin);
	CG_LaunchGib(origin, cgs.media.gibSkull, gdir, cent, PercentOnFire);

	VectorCopy(playerOrigin, origin);
	CG_LaunchGib(origin, cgs.media.gibBrain, gdir, cent, PercentOnFire);

}

/*
===================
CG_GibPlayer

Generated a bunch of gibs launching out from the bodies location
===================
*/
void CG_GibPlayerLegs(vec3_t playerOrigin, int clientNum, qhandle_t model, vec3_t gdir, int PercentOnFire)
{
	vec3_t          origin, velocity, dir;
	clientInfo_t   *ci;
	centity_t      *cent;
	int             i, j;
	qhandle_t       hshader;

	cent = &cg_entities[clientNum];

	ci = &cgs.clientinfo[clientNum];

	// allow gibs to be turned off for speed
	if(!cg_gibs.integer)
	{
		return;
	}

	VectorCopy(playerOrigin, origin);

	AxisToAngles(cg.autoAxis, dir);
	for(j = 0; j < 20; j++)
	{

		i = rand() % 1;
		if(i < 0)
		{
			i = 0;
		}
		if(i > 1)
		{
			i = 1;
		}

		if(i == 0)
		{
			hshader = cgs.media.zbloodSpray1Shader;
		}
		else if(i == 1)
		{
			hshader = cgs.media.zbloodSpray2Shader;
		}

		CG_Particle_Full360BurstBlood(hshader, origin, dir);
	}

	if(cgs.InstaGib == 1)
	{
		velocity[0] = 250;
		velocity[1] = 250;
		velocity[2] = GIB_JUMP + crandom() * GIB_VELOCITY;
		if(!model)
		{
			CG_TossWeapon(origin, velocity, trap_R_RegisterModel("models/weapons2/gauntlet/gauntlet.md3"), ci);
		}
		else
		{
			CG_TossWeapon(origin, velocity, model, ci);
		}
	}

	CG_LaunchGib(origin, cgs.media.gibArm, gdir, cent, PercentOnFire);

	VectorCopy(playerOrigin, origin);
	CG_LaunchGib(origin, cgs.media.gibForearm, gdir, cent, PercentOnFire);

	VectorCopy(playerOrigin, origin);
	CG_LaunchGib(origin, cgs.media.gibForearm, gdir, cent, PercentOnFire);

	VectorCopy(playerOrigin, origin);
	CG_LaunchGib(origin, cgs.media.gibForearm, gdir, cent, PercentOnFire);

	VectorCopy(playerOrigin, origin);
	CG_LaunchGib(origin, cgs.media.gibForearm, gdir, cent, PercentOnFire);

	VectorCopy(playerOrigin, origin);
	CG_LaunchGib(origin, cgs.media.gibIntestine, gdir, cent, PercentOnFire);

	VectorCopy(playerOrigin, origin);
	CG_LaunchGib(origin, cgs.media.gibIntestine, gdir, cent, PercentOnFire);

	VectorCopy(playerOrigin, origin);
	CG_LaunchGib(origin, cgs.media.gibIntestine, gdir, cent, PercentOnFire);

	VectorCopy(playerOrigin, origin);
	CG_LaunchGib(origin, cgs.media.gibFoot, gdir, cent, PercentOnFire);

	VectorCopy(playerOrigin, origin);
	CG_LaunchGib(origin, cgs.media.gibFoot, gdir, cent, PercentOnFire);

}

/*
===================
CG_GibPlayerQ

Generated a bunch of gibs launching out from the bodies location
===================
*/
#define	GIB_VELOCITY	250
#define	GIB_JUMP		250
void CG_GibPlayerQ(vec3_t playerOrigin, vec3_t dir, int team, int clientNum, int PercentOnFire)
{
	vec3_t          origin, bdir;
	centity_t      *cent;
	int             i, j;
	qhandle_t       hshader;

	cent = &cg_entities[clientNum];
	// allow gibs to be turned off for speed
	if(!cg_gibs.integer)
	{
		return;
	}

	VectorCopy(playerOrigin, origin);

	AxisToAngles(cg.autoAxisFast, bdir);
	for(j = 0; j < 20; j++)
	{

		i = rand() % 1;
		if(i < 0)
		{
			i = 0;
		}
		if(i > 1)
		{
			i = 1;
		}

		if(i == 0)
		{
			hshader = cgs.media.zbloodSpray1Shader;
		}
		else if(i == 1)
		{
			hshader = cgs.media.zbloodSpray2Shader;
		}

		CG_Particle_Full360BurstBlood(hshader, origin, bdir);
	}

	VectorCopy(playerOrigin, origin);
	CG_LaunchQuadGib(origin, cgs.media.gibFoot, dir, team, cent, PercentOnFire);

	VectorCopy(playerOrigin, origin);
	CG_LaunchQuadGib(origin, cgs.media.gibForearm, dir, team, cent, PercentOnFire);

	VectorCopy(playerOrigin, origin);
	CG_LaunchQuadGib(origin, cgs.media.gibIntestine, dir, team, cent, PercentOnFire);

	VectorCopy(playerOrigin, origin);
	CG_LaunchQuadGib(origin, cgs.media.gibLeg, dir, team, cent, PercentOnFire);

	VectorCopy(playerOrigin, origin);
	CG_LaunchQuadGib(origin, cgs.media.gibLeg, dir, team, cent, PercentOnFire);

	VectorCopy(playerOrigin, origin);
	CG_LaunchQuadGib(origin, cgs.media.gibAbdomen, dir, team, cent, PercentOnFire);

	VectorCopy(playerOrigin, origin);
	CG_LaunchQuadGib(origin, cgs.media.gibArm, dir, team, cent, PercentOnFire);

	VectorCopy(playerOrigin, origin);
	CG_LaunchQuadGib(origin, cgs.media.gibChest, dir, team, cent, PercentOnFire);

	VectorCopy(playerOrigin, origin);
	CG_LaunchQuadGib(origin, cgs.media.gibAbdomen, dir, team, cent, PercentOnFire);

	VectorCopy(playerOrigin, origin);
	CG_LaunchQuadGib(origin, cgs.media.gibAbdomen, dir, team, cent, PercentOnFire);

	VectorCopy(playerOrigin, origin);
	CG_LaunchQuadGib(origin, cgs.media.gibAbdomen, dir, team, cent, PercentOnFire);

	VectorCopy(playerOrigin, origin);
	CG_LaunchQuadGib(origin, cgs.media.gibIntestine, dir, team, cent, PercentOnFire);

	VectorCopy(playerOrigin, origin);
	CG_LaunchQuadGib(origin, cgs.media.gibIntestine, dir, team, cent, PercentOnFire);
}

void CG_GibPlayerQHead(vec3_t playerOrigin, vec3_t dir, int team, int clientNum, int PercentOnFire)
{
	vec3_t          origin, bdir;
	centity_t      *cent;
	int             i, j;
	qhandle_t       hshader;

	cent = &cg_entities[clientNum];
	// allow gibs to be turned off for speed
	if(!cg_gibs.integer)
	{
		return;
	}

	VectorCopy(playerOrigin, origin);

	AxisToAngles(cg.autoAxisFast, bdir);
	for(j = 0; j < 20; j++)
	{

		i = rand() % 1;
		if(i < 0)
		{
			i = 0;
		}
		if(i > 1)
		{
			i = 1;
		}

		if(i == 0)
		{
			hshader = cgs.media.zbloodSpray1Shader;
		}
		else if(i == 1)
		{
			hshader = cgs.media.zbloodSpray2Shader;
		}

		CG_Particle_Full360BurstBlood(hshader, origin, bdir);
	}

	VectorCopy(playerOrigin, origin);
	CG_LaunchQuadGib(origin, cgs.media.gibSkull, dir, team, cent, PercentOnFire);

	VectorCopy(playerOrigin, origin);
	CG_LaunchQuadGib(origin, cgs.media.gibBrain, dir, team, cent, PercentOnFire);
}

void CG_GibPlayerQLegs(vec3_t playerOrigin, vec3_t dir, int team, int clientNum, int PercentOnFire)
{
	vec3_t          origin, bdir;
	centity_t      *cent;
	int             i, j;
	qhandle_t       hshader;

	cent = &cg_entities[clientNum];
	// allow gibs to be turned off for speed
	if(!cg_gibs.integer)
	{
		return;
	}

	VectorCopy(playerOrigin, origin);

	AxisToAngles(cg.autoAxisFast, bdir);
	for(j = 0; j < 20; j++)
	{

		i = rand() % 1;
		if(i < 0)
		{
			i = 0;
		}
		if(i > 1)
		{
			i = 1;
		}

		if(i == 0)
		{
			hshader = cgs.media.zbloodSpray1Shader;
		}
		else if(i == 1)
		{
			hshader = cgs.media.zbloodSpray2Shader;
		}

		CG_Particle_Full360BurstBlood(hshader, origin, bdir);
	}

	VectorCopy(playerOrigin, origin);
	CG_LaunchQuadGib(origin, cgs.media.gibFoot, dir, team, cent, PercentOnFire);

	VectorCopy(playerOrigin, origin);
	CG_LaunchQuadGib(origin, cgs.media.gibFoot, dir, team, cent, PercentOnFire);

	VectorCopy(playerOrigin, origin);
	CG_LaunchQuadGib(origin, cgs.media.gibForearm, dir, team, cent, PercentOnFire);

	VectorCopy(playerOrigin, origin);
	CG_LaunchQuadGib(origin, cgs.media.gibForearm, dir, team, cent, PercentOnFire);

	VectorCopy(playerOrigin, origin);
	CG_LaunchQuadGib(origin, cgs.media.gibIntestine, dir, team, cent, PercentOnFire);

	VectorCopy(playerOrigin, origin);
	CG_LaunchQuadGib(origin, cgs.media.gibLeg, dir, team, cent, PercentOnFire);

	VectorCopy(playerOrigin, origin);
	CG_LaunchQuadGib(origin, cgs.media.gibLeg, dir, team, cent, PercentOnFire);


	VectorCopy(playerOrigin, origin);
	CG_LaunchQuadGib(origin, cgs.media.gibArm, dir, team, cent, PercentOnFire);


	VectorCopy(playerOrigin, origin);
	CG_LaunchQuadGib(origin, cgs.media.gibIntestine, dir, team, cent, PercentOnFire);

	VectorCopy(playerOrigin, origin);
	CG_LaunchQuadGib(origin, cgs.media.gibIntestine, dir, team, cent, PercentOnFire);
}

/*
==================
CG_LaunchExplode
==================
*/
void CG_LaunchExplode(vec3_t origin, vec3_t velocity, qhandle_t hModel)
{
	localEntity_t  *le;
	refEntity_t    *re;

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_FRAGMENT;
	le->entNumber = le - cg_freeLocalEntities;
	le->startTime = cg.time;
	le->endTime = le->startTime + 10000 + random() * 6000;

	VectorCopy(origin, re->origin);
	AxisCopy(axisDefault, re->axis);
	re->hModel = hModel;

	le->pos.trType = TR_GRAVITY;
	VectorCopy(origin, le->pos.trBase);
	VectorCopy(velocity, le->pos.trDelta);
	le->pos.trTime = cg.time;

	le->bounceFactor = 0.1f;

	le->leBounceSoundType = LEBS_BRASS;
	le->leMarkType = LEMT_NONE;
}

#define	EXP_VELOCITY	100
#define	EXP_JUMP		150
/*
===================
CG_BigExplode

Generated a bunch of gibs launching out from the bodies location
===================
*/
void CG_BigExplode(vec3_t playerOrigin)
{
	vec3_t          origin, velocity;

	if(!cg_blood.integer)
	{
		return;
	}

	VectorCopy(playerOrigin, origin);
	velocity[0] = crandom() * EXP_VELOCITY;
	velocity[1] = crandom() * EXP_VELOCITY;
	velocity[2] = EXP_JUMP + crandom() * EXP_VELOCITY;
//  CG_LaunchExplode( origin, velocity, cgs.media.smoke2 );

	VectorCopy(playerOrigin, origin);
	velocity[0] = crandom() * EXP_VELOCITY;
	velocity[1] = crandom() * EXP_VELOCITY;
	velocity[2] = EXP_JUMP + crandom() * EXP_VELOCITY;
//  CG_LaunchExplode( origin, velocity, cgs.media.smoke2 );

	VectorCopy(playerOrigin, origin);
	velocity[0] = crandom() * EXP_VELOCITY * 1.5;
	velocity[1] = crandom() * EXP_VELOCITY * 1.5;
	velocity[2] = EXP_JUMP + crandom() * EXP_VELOCITY;
//  CG_LaunchExplode( origin, velocity, cgs.media.smoke2 );

	VectorCopy(playerOrigin, origin);
	velocity[0] = crandom() * EXP_VELOCITY * 2.0;
	velocity[1] = crandom() * EXP_VELOCITY * 2.0;
	velocity[2] = EXP_JUMP + crandom() * EXP_VELOCITY;
//  CG_LaunchExplode( origin, velocity, cgs.media.smoke2 );

	VectorCopy(playerOrigin, origin);
	velocity[0] = crandom() * EXP_VELOCITY * 2.5;
	velocity[1] = crandom() * EXP_VELOCITY * 2.5;
	velocity[2] = EXP_JUMP + crandom() * EXP_VELOCITY;
//  CG_LaunchExplode( origin, velocity, cgs.media.smoke2 );
}
