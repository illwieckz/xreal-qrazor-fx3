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
// cg_weapons.c -- events and effects dealing with weapons
#include "cg_local.h"

/*
==========================
CG_MachineGunEjectBrass
==========================
*/
static void CG_MachineGunEjectBrass(centity_t * cent)
{
	localEntity_t  *le;
	refEntity_t    *re;
	vec3_t          velocity, xvelocity;
	vec3_t          offset, xoffset;
	float           waterScale = 1.0f;
	vec3_t          v[3];

	if(cg_brassTime.integer <= 0)
	{
		return;
	}

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	velocity[0] = 0;
	velocity[1] = -50 + 40 * crandom();
	velocity[2] = 100 + 50 * crandom();

	le->leType = LE_FRAGMENT;
	le->startTime = cg.time;
	le->endTime = le->startTime + cg_brassTime.integer + (cg_brassTime.integer / 4) * random();

	le->pos.trType = TR_GRAVITY;
	le->pos.trTime = cg.time - (rand() & 15);

	AnglesToAxis(cent->lerpAngles, v);

	offset[0] = 8;
	offset[1] = -4;
	offset[2] = 24;

	xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
	xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
	xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];
	VectorAdd(cent->lerpOrigin, xoffset, re->origin);

	VectorCopy(re->origin, le->pos.trBase);

	if(CG_PointContents(re->origin, -1) & CONTENTS_WATER)
	{
		waterScale = 0.10f;
	}

	xvelocity[0] = velocity[0] * v[0][0] + velocity[1] * v[1][0] + velocity[2] * v[2][0];
	xvelocity[1] = velocity[0] * v[0][1] + velocity[1] * v[1][1] + velocity[2] * v[2][1];
	xvelocity[2] = velocity[0] * v[0][2] + velocity[1] * v[1][2] + velocity[2] * v[2][2];
	VectorScale(xvelocity, waterScale, le->pos.trDelta);

	AxisCopy(axisDefault, re->axis);
	re->hModel = cgs.media.machinegunBrassModel;

	le->bounceFactor = 0.4 * waterScale;

	le->angles.trType = TR_LINEAR;
	le->angles.trTime = cg.time;
	le->angles.trBase[0] = rand() & 31;
	le->angles.trBase[1] = rand() & 31;
	le->angles.trBase[2] = rand() & 31;
	le->angles.trDelta[0] = 2;
	le->angles.trDelta[1] = 1;
	le->angles.trDelta[2] = 0;

	AnglesToQuat(le->angles.trBase, le->quatOrient);
	le->angVel = 10 * random();
	le->rotAxis[0] = crandom();
	le->rotAxis[1] = crandom();
	le->rotAxis[2] = crandom();
	VectorNormalize(le->rotAxis);
	le->radius = 4;
	QuatInit(1, 0, 0, 0, le->quatRot);

	le->leFlags |= LEF_TUMBLE;
	le->leBounceSoundType = LEBS_BRASS;
	le->leMarkType = LEMT_NONE;
}

/*
==========================
CG_ShotgunEjectBrass
==========================
*/
static void CG_ShotgunEjectBrass(centity_t * cent)
{
	localEntity_t  *le;
	refEntity_t    *re;
	vec3_t          velocity, xvelocity;
	vec3_t          offset, xoffset;
	vec3_t          v[3];
	int             i;

	if(cg_brassTime.integer <= 0)
	{
		return;
	}

	for(i = 0; i < 2; i++)
	{
		float           waterScale = 1.0f;

		le = CG_AllocLocalEntity();
		re = &le->refEntity;

		velocity[0] = 60 + 60 * crandom();
		if(i == 0)
		{
			velocity[1] = 40 + 10 * crandom();
		}
		else
		{
			velocity[1] = -40 + 10 * crandom();
		}
		velocity[2] = 100 + 50 * crandom();

		le->leType = LE_FRAGMENT;
		le->startTime = cg.time;
		le->endTime = le->startTime + cg_brassTime.integer * 3 + cg_brassTime.integer * random();

		le->pos.trType = TR_GRAVITY;
		le->pos.trTime = cg.time;

		AnglesToAxis(cent->lerpAngles, v);

		offset[0] = 8;
		offset[1] = 0;
		offset[2] = 24;

		xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
		xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
		xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];
		VectorAdd(cent->lerpOrigin, xoffset, re->origin);
		VectorCopy(re->origin, le->pos.trBase);
		if(CG_PointContents(re->origin, -1) & CONTENTS_WATER)
		{
			waterScale = 0.10f;
		}

		xvelocity[0] = velocity[0] * v[0][0] + velocity[1] * v[1][0] + velocity[2] * v[2][0];
		xvelocity[1] = velocity[0] * v[0][1] + velocity[1] * v[1][1] + velocity[2] * v[2][1];
		xvelocity[2] = velocity[0] * v[0][2] + velocity[1] * v[1][2] + velocity[2] * v[2][2];
		VectorScale(xvelocity, waterScale, le->pos.trDelta);

		AxisCopy(axisDefault, re->axis);
		re->hModel = cgs.media.shotgunBrassModel;
		le->bounceFactor = 0.3f;

		le->angles.trType = TR_LINEAR;
		le->angles.trTime = cg.time;
		le->angles.trBase[0] = rand() & 31;
		le->angles.trBase[1] = rand() & 31;
		le->angles.trBase[2] = rand() & 31;
		le->angles.trDelta[0] = 1;
		le->angles.trDelta[1] = 0.5;
		le->angles.trDelta[2] = 0;

		AnglesToQuat(le->angles.trBase, le->quatOrient);
		le->angVel = 10 * random();
		le->rotAxis[0] = crandom();
		le->rotAxis[1] = crandom();
		le->rotAxis[2] = crandom();
		VectorNormalize(le->rotAxis);
		le->radius = 6;
		QuatInit(1, 0, 0, 0, le->quatRot);

		le->leFlags |= LEF_TUMBLE;
		le->leBounceSoundType = LEBS_SHELLS;
		le->leMarkType = LEMT_NONE;
	}
}


#ifdef MISSIONPACK
/*
==========================
CG_NailgunEjectBrass
==========================
*/
static void CG_NailgunEjectBrass(centity_t * cent)
{
	localEntity_t  *smoke;
	vec3_t          origin;
	vec3_t          v[3];
	vec3_t          offset;
	vec3_t          xoffset;
	vec3_t          up;

	AnglesToAxis(cent->lerpAngles, v);

	offset[0] = 0;
	offset[1] = -12;
	offset[2] = 24;

	xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
	xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
	xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];
	VectorAdd(cent->lerpOrigin, xoffset, origin);

	VectorSet(up, 0, 0, 64);

	smoke = CG_SmokePuff(origin, up, 32, 1, 1, 1, 0.33f, 700, cg.time, 0, 0, cgs.media.smokePuffShader);
	// use the optimized local entity add
	smoke->leType = LE_SCALE_FADE;
}
#endif


#define RADIUS   2
#define ROTATION 1

/*
=================
CG_AddRailParticles
=================
*/
static void CG_AddRailParticles(clientInfo_t * ci, vec3_t origin, vec3_t dir, int count, int duration,
								int speed, int trailtime, qhandle_t hShader, int step, float startsize, float endsize, vec3_t ma)
{
	localEntity_t  *le;
	refEntity_t    *re;
	vec3_t          velocity;
	int             i;
	float           randScale;


	// add the sparks
	for(i = 0; i < count; i++)
	{

		le = CG_AllocLocalEntity();
		re = &le->refEntity;

		speed = speed + rand() % 10;
		duration = duration + rand() % 20;
		randScale = 0.9f;


		le->leType = LE_ADDRAILSPARK;
		le->entNumber = le - cg_freeLocalEntities;
		VectorNormalize(dir);
		velocity[0] = (dir[0] + crandom() * randScale) / 4;
		velocity[1] = (dir[1] + crandom() * randScale) / 4;
		velocity[2] = (dir[2] + crandom() * randScale) / 4;
		VectorScale(velocity, (float)speed, le->pos.trDelta);

		le->startTime = cg.time;
		le->endTime = le->startTime + duration * 8;
		le->lifeRate = 1.0 / (le->endTime - le->startTime);
		le->hShader = hShader;
		le->trailLength = trailtime;
		le->step = step;
		le->startSize = startsize + 1;
		le->endSize = endsize;

		le->pos.trType = TR_GRAVITY;
		le->pos.trTime = cg.time;
		le->bounceFactor = 0.9f;
		VectorCopy(origin, re->origin);
		VectorCopy(origin, le->pos.trBase);
		VectorMA(le->pos.trBase, startsize + 6, ma, le->pos.trBase);

	}
}


/*
==========================
CG_RailTrail1
==========================
*/
void CG_RailTrail(clientInfo_t * ci, vec3_t start, vec3_t end)
{
	vec3_t          axis[36], move, move2, next_move, vec, temp, dir;
	float           len;
	int             i, j, skip, d;
	vec3_t          angles, end2;
	int             space;
	trace_t         trace;
	trace_t         trace2;

	localEntity_t  *le;
	refEntity_t    *re;

	start[2] -= 4;
	VectorCopy(start, move);
	VectorSubtract(end, start, vec);
	len = VectorNormalize(vec);
	PerpendicularVector(temp, vec);
	for(i = 0; i < 36; i++)
	{
		RotatePointAroundVector(axis[i], vec, temp, i * 10);	//banshee 2.4 was 10
	}

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_FADE_RGBA;
	le->entNumber = le - cg_freeLocalEntities;
	le->startTime = cg.time;
	le->endTime = cg.time + cg_railTrailTime.value;
	le->lifeRate = 1.0 / (le->endTime - le->startTime);

	re->shaderTime = cg.time / 1000.0f;
	re->reType = RT_RAIL_CORE;
	re->customShader = cgs.media.railCoreShader;

	VectorCopy(start, re->origin);
	VectorCopy(end, re->oldorigin);

	re->shaderRGBA[0] = 0;
	re->shaderRGBA[1] = 0;
	re->shaderRGBA[2] = 0;
	re->shaderRGBA[3] = 255;

	le->color[0] = 0;
	le->color[1] = 0;
	le->color[2] = 0;
	le->color[3] = 1.0f;

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_FADE_RGBA;
	le->entNumber = le - cg_freeLocalEntities;
	le->startTime = cg.time;
	le->endTime = cg.time + cg_railTrailTime.value;
	le->lifeRate = 1.0 / (le->endTime - le->startTime);

	re->shaderTime = cg.time / 1000.0f;
	re->reType = RT_RAIL_CORE;
	re->customShader = cgs.media.railCoreglowShader;

	VectorCopy(start, re->origin);
	VectorCopy(end, re->oldorigin);

	re->shaderRGBA[0] = ci->color1[0] * 255;
	re->shaderRGBA[1] = ci->color1[1] * 255;
	re->shaderRGBA[2] = ci->color1[2] * 255;
	re->shaderRGBA[3] = 255;

	le->color[0] = ci->color1[0] * 1;
	le->color[1] = ci->color1[1] * 1;
	le->color[2] = ci->color1[2] * 1;
	le->color[3] = 0.9f;

	AxisClear(re->axis);

	// Cap model
	VectorClear(dir);
	VectorSubtract(start, end, dir);
	VectorNormalize(dir);
	vectoangles(dir, angles);

	le = CG_AllocLocalEntity();
	re = &le->refEntity;
	le->leType = LE_FADE_RGBA;
	le->entNumber = le - cg_freeLocalEntities;
	re->reType = RT_MODEL;
	le->startTime = cg.time;
	le->endTime = cg.time + cg_railTrailTime.value;
	le->lifeRate = 1.0 / (le->endTime - le->startTime);

	re->shaderTime = cg.time / 1000.0f;
	le->pos.trType = TR_STATIONARY;
	le->pos.trTime = cg.time;

	re->shaderRGBA[0] = ci->color1[0] * 255;
	re->shaderRGBA[1] = ci->color1[1] * 255;
	re->shaderRGBA[2] = ci->color1[2] * 255;
	re->shaderRGBA[3] = 255;

	le->color[0] = ci->color1[0] * 1.0;
	le->color[1] = ci->color1[1] * 1.0;
	le->color[2] = ci->color1[2] * 1.0;
	le->color[3] = 1.0f;

	re->hModel = trap_R_RegisterModel("models/weapons2/railgun/railgun_flash.md3");
	le->refEntity.customShader = trap_R_RegisterShader("rgflash1");
	AnglesToAxis(angles, re->axis);
	VectorCopy(start, re->origin);

	le = CG_AllocLocalEntity();
	re = &le->refEntity;
	le->leType = LE_FADE_RGB;
	le->entNumber = le - cg_freeLocalEntities;
	re->reType = RT_MODEL;
	le->startTime = cg.time;
	le->endTime = cg.time + cg_railTrailTime.value;
	le->lifeRate = 1.0 / (le->endTime - le->startTime);

	re->shaderTime = cg.time / 1000.0f;
	le->pos.trType = TR_STATIONARY;
	le->pos.trTime = cg.time;

	re->shaderRGBA[0] = 255;
	re->shaderRGBA[1] = 255;
	re->shaderRGBA[2] = 255;
	re->shaderRGBA[3] = 255;

	le->color[0] = 0.5f;
	le->color[1] = 0.5f;
	le->color[2] = 0.5f;
	le->color[3] = 0.5f;;

	re->hModel = trap_R_RegisterModel("models/weapons2/railgun/railgun_flash.md3");
	le->refEntity.customShader = trap_R_RegisterShader("rgflash2");
	AnglesToAxis(angles, re->axis);
	VectorCopy(start, re->origin);

	// end cap model


	VectorMA(move, 20, vec, move);
	VectorCopy(move, next_move);

	space = rt_ringspace.integer;
	if(space < 1)
	{
		space = 1;
	}
	VectorScale(vec, space, vec);

	skip = -1;

	j = 18;
	if(cg_oldRail.integer == 1)
	{
		for(i = 0; i < len; i += space)
		{

			if(i != skip)
			{
				skip = i + space;
				le = CG_AllocLocalEntity();
				re = &le->refEntity;
				le->leType = LE_RAILDISC;
				le->entNumber = le - cg_freeLocalEntities;
				le->startTime = cg.time;
				le->endTime = cg.time + (i >> 1) + (cg_railTrailTime.value + cg_railTrailTime.integer / 2);
				le->lifeRate = 1.0 / (le->endTime - le->startTime);

				re->shaderTime = cg.time / 1000.0f;

				re->customShader = cgs.media.railRings1Shader;	// = trap_R_RegisterShader( "railDisc1" );
				re->radius = 1;
				le->radius = re->radius;
				le->endradius = 6;
				le->pos.trType = TR_STATIONARY;
				le->pos.trTime = cg.time;

				le->color[0] = ci->color2[0] * 1.0f;
				le->color[1] = ci->color2[1] * 1.0f;
				le->color[2] = ci->color2[2] * 1.0f;
				le->color[3] = ci->color2[3] * 1.0f;

				AnglesToAxis(angles, re->axis);

				VectorCopy(move, move2);
				VectorCopy(move2, le->pos.trBase);
			}
			VectorAdd(move, vec, move);
			j = j + ROTATION < 36 ? j + ROTATION : (j + ROTATION) % 36;
		}
	}
	else
	{
		for(i = 0; i < len; i += space)
		{
			if(i != skip)
			{
				skip = i + space;
				le = CG_AllocLocalEntity();
				re = &le->refEntity;
				le->leFlags = LEF_PUFF_DONT_SCALE;
				le->leType = LE_RAILDISC;
				le->entNumber = le - cg_freeLocalEntities;
				le->startTime = cg.time;
				le->endTime = cg.time + (i >> 1) + (cg_railTrailTime.value + cg_railTrailTime.integer / 2);
				le->lifeRate = 1.0 / (le->endTime - le->startTime);

				re->shaderTime = cg.time / 1000.0f;
				re->sparklesT = 1;
				re->customShader = cgs.media.railRings3Shader;	// = trap_R_RegisterShader( "railDisc" );
				re->radius = 2.1f;

				le->color[0] = ci->color2[0] * 1.0f;
				le->color[1] = ci->color2[1] * 1.0f;
				le->color[2] = ci->color2[2] * 1.0f;
				le->color[3] = ci->color2[3] * 1.0f;

				le->pos.trType = TR_LINEAR;
				le->pos.trTime = cg.time;

				VectorCopy(move, move2);
				VectorMA(move2, RADIUS, axis[j], move2);
				VectorCopy(move2, le->pos.trBase);
				AnglesToAxis(angles, re->axis);
				le->pos.trDelta[0] = axis[j][0] * 6;
				le->pos.trDelta[1] = axis[j][1] * 6;
				le->pos.trDelta[2] = axis[j][2] * 6;
			}
			VectorAdd(move, vec, move);
			j = j + ROTATION < 36 ? j + ROTATION : (j + ROTATION) % 36;
		}
	}
	VectorMA(start, 8192 * 16, vec, end2);
	CG_Trace(&trace, end, NULL, NULL, end2, -1, MASK_SOLID);
	CG_Trace(&trace2, start, NULL, NULL, end2, -1, MASK_SOLID);
	if(trace.surfaceFlags & SURF_NOIMPACT)
	{
		return;
	}
	else if(trace.surfaceFlags & SURF_SKY)
	{
		return;
	}
	else
	{
		if(cg_QSParticles.integer == 1)
		{
			d = cg_RGParticles.integer;
			CG_AddRailParticles(ci, end, vec,
								/*count */ d,
								/*speed */ 70,
								/*duration */ 1000,
								/*trailtime */ 100,
								/*shader */ cgs.media.tracerTrailzShader,
								/*step */ 10,
								/*startsize */ 2.0,
								/*endsize */ 0,
								trace2.plane.normal);
		}
	}

}

/*
==========================
CG_RocketTrail

  THINKABOUTME: Should this use the optimized localentity add?
==========================
*/
void CG_RocketTrail(centity_t * ent, const weaponInfo_t * wi)
{
	int             step, step2, step3;
	vec3_t          origin, lastPos, org;
	int             contents;
	int             lastContents;
	vec3_t          start2, dir, end, rotate_ang;
	trace_t         trace2;
	int             t, t2, t3;
	int             startTime;
	entityState_t  *es;
	vec3_t          up;
	static vec3_t   col = { 0, 0, 0 };
	static vec3_t   col1 = { 1, 1, 1 };



	up[0] = 0;
	up[1] = 0;
	up[2] = 0;
	if(cgs.InstaGib == 0)
	{
		step = 10;
		step2 = 16;
		step3 = 2;
	}
	else
	{
		step = 13 / 1.5;
		step2 = 19 / 1.5;
		step3 = 5 / 1.5;
	}

	es = &ent->currentState;
	startTime = ent->trailTime;
	t = step * ((startTime + step) / step);
	t2 = step2 * ((startTime + step2) / step2);
	t3 = step3 * ((startTime + step3) / step3);

	BG_EvaluateTrajectory(&es->pos, cg.time, origin);
	contents = CG_PointContents(origin, -1);

	BG_EvaluateTrajectory(&es->pos, ent->trailTime, lastPos);
	lastContents = CG_PointContents(lastPos, -1);

	ent->trailTime = cg.time;

	for(; t3 <= ent->trailTime; t3 += step3)
	{
		BG_EvaluateTrajectory(&es->pos, t3, lastPos);
		VectorCopy(lastPos, org);
		VectorMA(org, 4, &ent->lerpAngles[0], org);
		if(cgs.InstaGib == 0)
		{
			CG_FlarePuff(org, 0, 8.5f, 1, 1, 0.3f, 1, 150, t3, 0, LEF_PUFF_SCALE_DOWN, cgs.media.tracerShader, "", qfalse);
		}
		else
		{
			CG_FlarePuff(org, 0, 8.5f, 1, 1, 0.3f, 1, 150 / 1.5f, t3, 0, LEF_PUFF_SCALE_DOWN, cgs.media.tracerShader, "", qfalse);
		}

	}

	if(contents & (CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA))
	{

		if(contents & lastContents & CONTENTS_WATER)
		{

			CG_BubbleTrail(lastPos, origin, 6);

			if(ent->inwater && ent->watertraced && cg.time > ent->startTime + 5)
			{
				vectoangles(ent->axis[1], rotate_ang);
				AngleVectors(rotate_ang, dir, NULL, NULL);

				VectorMA(lastPos, -16, dir, start2);	// back off a little so it doesn't start in solid
				VectorMA(lastPos, 2, dir, end);

				trap_CM_BoxTrace(&trace2, start2, end, NULL, NULL, 0, MASK_WATER);
				if(trace2.fraction != 1.0 && !trace2.startsolid)
				{
					CG_AddBulletSplashParticles(lastPos, trace2.plane.normal, 225,	// speed
												900,	// duration
												2,	// count
												0.25f, 50, 25, 0.7f, "watersplash");
					CG_AddBulletSplashParticles(lastPos, trace2.plane.normal, 235,	// speed
												900,	// duration
												2,	// count
												0.25f, 80, 20, 0.6f, "watersplash");
					trap_S_StartSound(lastPos, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.waterHitSound);
					ent->inwater = qfalse;
				}
			}
		}
		return;
	}

	if(!(contents & CONTENTS_WATER))
	{
		ent->inwater = qtrue;
		ent->startTime = cg.time;
	}

	if(cg_noProjectileTrail.integer)
	{
		return;
	}

	for(; t <= ent->trailTime; t += step)
	{
		BG_EvaluateTrajectory(&es->pos, t, lastPos);
		if(cgs.InstaGib == 0)
		{
			CG_FlarePuff(lastPos, 0,
						 11 + rand() % 15,
						 0.6f, 0.6f, 0.6f, 0.4f, 1000 + rand() % 400, t, 0, LEF_PUFF_DONT_SCALE /*|LEF_NO_RGB_FADE */ ,
						 cgs.media.GREYSmokeShader, "", qfalse);
		}
		else
		{
			CG_FlarePuff(lastPos, 0,
						 11 + rand() % 15,
						 0.6f, 0.6f, 0.6f, 0.4f, (1000 / 1.5f) + rand() % 400, t, 0, LEF_PUFF_DONT_SCALE /*|LEF_NO_RGB_FADE */ ,
						 cgs.media.GREYSmokeShader, "", qfalse);
		}

	}
	for(; t2 <= ent->trailTime; t2 += step2)
	{
		BG_EvaluateTrajectory(&es->pos, t2, lastPos);
		if(cgs.InstaGib == 0)
		{
			CG_FlarePuff(lastPos, 0,
						 8.5f, 0.8f, 0.8f, 0.3f, 1, 325, t2, 0, LEF_PUFF_DONT_SCALE, cgs.media.PLSmokeShader, "", qfalse);
		}
		else
		{
			CG_FlarePuff(lastPos, 0,
						 8.5f, 0.8f, 0.8f, 0.3f, 1, 325 / 1.5f, t2, 0, LEF_PUFF_DONT_SCALE, cgs.media.PLSmokeShader, "", qfalse);
		}

		ent->lastTrailTime = cg.time;
	}
}

static void CG_BFGTrail(centity_t * ent, const weaponInfo_t * wi)
{
	vec3_t          origin, lastPos;
	int             contents;
	int             lastContents;
	vec3_t          start2, dir, end, rotate_ang;
	trace_t         trace2;
	entityState_t  *es;

	es = &ent->currentState;

	BG_EvaluateTrajectory(&es->pos, cg.time, origin);
	contents = CG_PointContents(origin, -1);

	BG_EvaluateTrajectory(&es->pos, ent->trailTime, lastPos);
	lastContents = CG_PointContents(lastPos, -1);
	ent->trailTime = cg.time;


	if(contents & (CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA))
	{

		if(contents & lastContents & CONTENTS_WATER)
		{
			if(ent->inwater && ent->watertraced && cg.time > ent->startTime + 5)
			{
				vectoangles(ent->axis[1], rotate_ang);
				AngleVectors(rotate_ang, dir, NULL, NULL);

				VectorMA(lastPos, -16, dir, start2);	// back off a little so it doesn't start in solid
				VectorMA(lastPos, 2, dir, end);

				trap_CM_BoxTrace(&trace2, start2, end, NULL, NULL, 0, MASK_WATER);
				if(trace2.fraction != 1.0 && !trace2.startsolid)
				{
					CG_AddBulletSplashParticles(lastPos, trace2.plane.normal, 225,	// speed
												900,	// duration
												3,	// count
												0.25f, 50, 45, 0.7f, "watersplash");
					CG_AddBulletSplashParticles(lastPos, trace2.plane.normal, 235,	// speed
												900,	// duration
												3,	// count
												0.25f, 80, 40, 0.6f, "watersplash");
					trap_S_StartSound(lastPos, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.waterHitSound);
					ent->inwater = qfalse;
				}
			}
		}
		return;
	}

	if(!(contents & CONTENTS_WATER))
	{
		ent->inwater = qtrue;
		ent->startTime = cg.time;
	}

}

void CG_MissileTrail(centity_t * ent, const weaponInfo_t * wi, vec3_t axis)
{
	int             step;
	vec3_t          newOrigin;
	int             t, t2;
	entityState_t  *es;
	static vec3_t   col1 = { 1, 1, 0 };
	static vec3_t   col2 = { 1, 0, 0 };



	step = 50;

	es = &ent->currentState;
	if(es->pos.trType != TR_STATIONARY)
	{

		t = step * ((cg.time - cg.frametime + step) / step);
		t2 = step * (cg.time / step);


	}
}

/*
==========================
CG_RocketTrail2
==========================
*/
void CG_RocketTrail2(localEntity_t * ent)
{
	int             step;
	vec3_t          origin, lastPos;
	int             t;
	int             startTime, contents;
	int             lastContents;
	entityState_t  *es;
	vec3_t          up;
	localEntity_t  *smoke;

	if(cg_noProjectileTrail.integer)
	{
		return;
	}

	up[0] = 0;
	up[1] = 0;
	up[2] = 0;

	step = 30;

	es = &ent->currentState;
	startTime = 2000;
	t = step * ((startTime + step) / step);

	BG_EvaluateTrajectory(&es->pos, cg.time, origin);
	contents = CG_PointContents(origin, -1);

	BG_EvaluateTrajectory(&es->pos, 2000, lastPos);
	lastContents = CG_PointContents(lastPos, -1);

	if(contents & (CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA))
	{
		if(contents & lastContents & CONTENTS_WATER)
		{
			CG_BubbleTrail(lastPos, origin, 8);
		}
		return;
	}

	for(; t <= 2000; t += step)
	{
		BG_EvaluateTrajectory(&es->pos, t, lastPos);

		smoke = CG_SmokePuff(lastPos, up, 32, 1, 1, 1, 0.33f, 2000, t, 0, 0, cgs.media.smokePuffShader);
		// use the optimized local entity add
		smoke->leType = LE_SCALE_FADE;
	}
}

/*
==========================
CG_PlasmaTrail
==========================
*/
void CG_PlasmaTrail(centity_t * cent, const weaponInfo_t * wi, vec3_t * axis, int team)
{
	int             step;
	vec3_t          origin, lastPos;
	int             contents;
	int             lastContents;
	vec3_t          start2, dir, end, rotate_ang;
	trace_t         trace2;
	int             t;
	int             startTime;
	entityState_t  *es;
	vec3_t          up;

	if(cg_noProjectileTrail.integer)
	{
		return;
	}

	up[0] = 0;
	up[1] = 0;
	up[2] = 0;

	step = 6;

	es = &cent->currentState;
	startTime = cent->trailTime;
	t = step * ((startTime + step) / step);

	BG_EvaluateTrajectory(&es->pos, cg.time, origin);
	contents = CG_PointContents(origin, -1);

	BG_EvaluateTrajectory(&es->pos, cent->trailTime, lastPos);
	lastContents = CG_PointContents(lastPos, -1);

	cent->trailTime = cg.time;

	if(contents & (CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA))
	{
		if(contents & lastContents & CONTENTS_WATER)
		{
			if(cent->inwater && cent->watertraced && cg.time > cent->startTime + 5)
			{
				vectoangles(cent->axis[1], rotate_ang);
				AngleVectors(rotate_ang, dir, NULL, NULL);

				VectorMA(lastPos, -16, dir, start2);	// back off a little so it doesn't start in solid
				VectorMA(lastPos, 2, dir, end);

				trap_CM_BoxTrace(&trace2, start2, end, NULL, NULL, 0, MASK_WATER);
				if(trace2.fraction != 1.0 && !trace2.startsolid)
				{
					CG_AddBulletSplashParticles(lastPos, trace2.plane.normal, 170,	// speed
												500,	// duration
												3,	// count
												0.25f, 70, 16, 0.8f, "watersplash");
					trap_S_StartSound(lastPos, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.waterHitsmallSound);
					cent->inwater = qfalse;
				}
			}
		}
	}

	if(!(contents & CONTENTS_WATER) && !(lastContents & CONTENTS_WATER))
	{
		cent->inwater = qtrue;
		cent->startTime = cg.time;
	}

	for(; t <= cent->trailTime; t += step)
	{
		BG_EvaluateTrajectory(&es->pos, t, lastPos);
		//  if ( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) {
		if(cgs.gametype >= GT_TEAM)
		{
			if(team == TEAM_RED)
			{
				CG_FlarePuff(lastPos, up,
							 0.5f + rand() % 1,
							 1, 0.5f, 0.5f, 1, 300 + rand() % 100, t, 0, 0, cgs.media.LIGHTSmokeRingShader, "", qfalse);
				CG_FlarePuff(lastPos, up,
							 0.5f + rand() % 1,
							 1, 0.5f, 0.5f, 1, 300 + rand() % 100, t, 0, 0, cgs.media.GREYSmokeRingShader, "", qfalse);
			}
			else if(team == TEAM_BLUE)
			{
				CG_FlarePuff(lastPos, up,
							 0.5f + rand() % 1,
							 0.5f, 0.5f, 1, 1, 300 + rand() % 100, t, 0, 0, cgs.media.LIGHTSmokeRingShader, "", qfalse);
				CG_FlarePuff(lastPos, up,
							 0.5f + rand() % 1,
							 0.5f, 0.5f, 1, 1, 300 + rand() % 100, t, 0, 0, cgs.media.GREYSmokeRingShader, "", qfalse);
			}
		}
		else
		{
			CG_FlarePuff(lastPos, up,
						 0.5f + rand() % 1,
						 0.5f, 1, 0.5f, 1, 300 + rand() % 100, t, 0, 0, cgs.media.LIGHTSmokeRingShader, "", qfalse);
			CG_FlarePuff(lastPos, up,
						 0.5f + rand() % 1,
						 0.5f, 1, 0.5f, 1, 300 + rand() % 100, t, 0, 0, cgs.media.GREYSmokeRingShader, "", qfalse);
		}
		//  }
		cent->trailTime = cg.time;

	}
}

static void CG_PlasmaRing(const vec3_t p, const vec3_t vel, float duration, int startTime, vec3_t axis[3], int team)
{
	static int      seed = 0x92;
	localEntity_t  *le;
	refEntity_t    *re;

	le = CG_AllocLocalEntity();

	re = &le->refEntity;
	re->shaderTime = startTime / 1000.0f;

	re->reType = RT_MODEL;

	re->hModel = cgs.media.PlasmaWave;
	if(cgs.gametype >= GT_TEAM)
	{
		if(team == TEAM_RED)
		{
			re->customShader = cgs.media.PlasmaRingRShader;
		}
		else if(team == TEAM_BLUE)
		{
			re->customShader = cgs.media.PlasmaRingBShader;
		}
	}
	else
	{
		re->customShader = cgs.media.PlasmaRingGShader;
	}

	le->leType = LE_FADE_RGB;
	le->entNumber = le - cg_freeLocalEntities;
	le->startTime = startTime;
	le->endTime = startTime + duration;

	le->lifeRate = 1.0 / (le->endTime - le->startTime);
	le->fadeInTime = 40;
	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 0.4f;
	le->pos.trType = TR_STATIONARY;
	le->pos.trTime = startTime;

	AxisCopy(axis, re->axis);

	VectorCopy(p, re->origin);
}

void CG_PlasmaTrail2(centity_t * ent, const weaponInfo_t * wi, vec3_t axis[3], int team)
{
	int             step;
	vec3_t          lastPos;
	int             t, t2;
	entityState_t  *es;
	vec3_t          up;


	if(cg_noProjectileTrail.integer)
	{
		return;
	}

	up[0] = 0;
	up[1] = 0;
	up[2] = 0;

	step = 50;

	es = &ent->currentState;

	t = step * ((cg.time - cg.frametime + step) / step);
	t2 = step * (cg.time / step);


	for(; t <= t2; t += step)
	{
		BG_EvaluateTrajectory(&es->pos, t, lastPos);

		CG_PlasmaRing(lastPos, up, 200, t, axis, team);
	}
}

/*
==========================
CG_GrappleTrail
==========================
*/
void CG_GrappleTrail(centity_t * ent, const weaponInfo_t * wi)
{
	vec3_t          origin;
	entityState_t  *es;
	vec3_t          forward, up;
	refEntity_t     beam;

	es = &ent->currentState;

	BG_EvaluateTrajectory(&es->pos, cg.time, origin);
	ent->trailTime = cg.time;

	memset(&beam, 0, sizeof(beam));
	//FIXME adjust for muzzle position
	VectorCopy(cg_entities[ent->currentState.otherEntityNum].lerpOrigin, beam.origin);
	beam.origin[2] += 26;
	AngleVectors(cg_entities[ent->currentState.otherEntityNum].lerpAngles, forward, NULL, up);
	VectorMA(beam.origin, -6, up, beam.origin);
	VectorCopy(origin, beam.oldorigin);

	if(Distance(beam.origin, beam.oldorigin) < 64)
		return;					// Don't draw if close
	beam.radius = 3;
	beam.reType = RT_LIGHTNING;
	beam.customShader = cgs.media.railCoreShader;

	AxisClear(beam.axis);
	beam.shaderRGBA[0] = 0xff;
	beam.shaderRGBA[1] = 0xff;
	beam.shaderRGBA[2] = 0xff;
	beam.shaderRGBA[3] = 0xff;
	trap_R_AddRefEntityToScene(&beam);
}

/*
==========================
CG_GrenadeTrail
==========================
*/
static void CG_GrenadeTrail(centity_t * ent, const weaponInfo_t * wi)
{
	int             step;
	vec3_t          origin, lastPos;
	int             contents;
	int             lastContents;
	vec3_t          start2, dir, end, rotate_ang;
	trace_t         trace2;
	int             t;
	int             startTime;
	entityState_t  *es;
	vec3_t          up;


	up[0] = 0;
	up[1] = 0;
	up[2] = 0;

	step = 15;

	es = &ent->currentState;
	startTime = ent->trailTime;
	t = step * ((startTime + step) / step);

	BG_EvaluateTrajectory(&es->pos, cg.time, origin);
	contents = CG_PointContents(origin, -1);

	BG_EvaluateTrajectory(&es->pos, ent->trailTime, lastPos);
	lastContents = CG_PointContents(lastPos, -1);

	if(es->pos.trType == TR_STATIONARY)
	{
		ent->trailTime = cg.time;
		return;
	}

	ent->trailTime = cg.time;

	if(contents & (CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA))
	{
		if(contents & lastContents & CONTENTS_WATER)
		{

			CG_BubbleTrail(lastPos, origin, 6);

			if(ent->inwater && ent->watertraced && cg.time > ent->startTime + 5)
			{
				vectoangles(ent->axis[1], rotate_ang);
				AngleVectors(rotate_ang, dir, NULL, NULL);

				VectorMA(lastPos, -16, dir, start2);	// back off a little so it doesn't start in solid
				VectorMA(lastPos, 2, dir, end);

				trap_CM_BoxTrace(&trace2, start2, end, NULL, NULL, 0, MASK_WATER);
				if(trace2.fraction != 1.0 && !trace2.startsolid)
				{
					CG_AddBulletSplashParticles(lastPos, trace2.plane.normal, 225,	// speed
												900,	// duration
												2,	// count
												0.25f, 50, 25, 0.7f, "watersplash");
					CG_AddBulletSplashParticles(lastPos, trace2.plane.normal, 235,	// speed
												900,	// duration
												2,	// count
												0.25f, 80, 20, 0.6f, "watersplash");

					trap_S_StartSound(lastPos, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.waterHitSound);
					ent->inwater = qfalse;
				}
			}
		}
	}

	if(cg_noProjectileTrail.integer)
	{
		return;
	}

	if(contents & (CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA))
	{
		return;
	}

	if(!(contents & CONTENTS_WATER))
	{
		ent->inwater = qtrue;
		ent->startTime = cg.time;
	}

	for(; t <= ent->trailTime; t += step)
	{
		BG_EvaluateTrajectory(&es->pos, t, lastPos);

		CG_FlarePuff(lastPos, 0, 9, 1, 1, 1, 1, 800, t, 0,
					 /*LEF_PUFF_DONT_SCALE */ 0 /*|LEF_NO_RGB_FADE */ ,
					 cgs.media.GREYSmokeShader, "", qfalse);
	}
}


/*
=================
CG_RegisterWeapon

The server says this item is used on this level
=================
*/
void CG_RegisterWeapon(int weaponNum)
{
	weaponInfo_t   *weaponInfo;
	gitem_t        *item, *ammo;
	char            path[MAX_QPATH];
	vec3_t          mins, maxs;
	int             i;

	weaponInfo = &cg_weapons[weaponNum];

	if(weaponNum == 0)
	{
		return;
	}

	if(weaponInfo->registered)
	{
		return;
	}

	memset(weaponInfo, 0, sizeof(*weaponInfo));
	weaponInfo->registered = qtrue;

	for(item = bg_itemlist + 1; item->classname; item++)
	{
		if(item->giType == IT_WEAPON && item->giTag == weaponNum)
		{
			weaponInfo->item = item;
			break;
		}
	}
	if(!item->classname)
	{
		CG_Error("Couldn't find weapon %i", weaponNum);
	}
	CG_RegisterItemVisuals(item - bg_itemlist);

	// load cmodel before model so filecache works
	weaponInfo->weaponModel = trap_R_RegisterModel(item->world_model[0]);

	// calc midpoint for rotation
	trap_R_ModelBounds(weaponInfo->weaponModel, mins, maxs);
	for(i = 0; i < 3; i++)
	{
		weaponInfo->weaponMidpoint[i] = mins[i] + 0.5 * (maxs[i] - mins[i]);
	}

	weaponInfo->weaponIcon = trap_R_RegisterShader(item->icon);
	weaponInfo->ammoIcon = trap_R_RegisterShader(item->icon);

	for(ammo = bg_itemlist + 1; ammo->classname; ammo++)
	{
		if(ammo->giType == IT_AMMO && ammo->giTag == weaponNum)
		{
			break;
		}
	}
	if(ammo->classname && ammo->world_model[0])
	{
		weaponInfo->ammoModel = trap_R_RegisterModel(ammo->world_model[0]);
	}

	strcpy(path, item->world_model[0]);
	Com_StripExtension(path, path, sizeof(path));
	strcat(path, "_flash.md3");
	weaponInfo->flashModel = trap_R_RegisterModel(path);

	strcpy(path, item->world_model[0]);
	Com_StripExtension(path, path, sizeof(path));
	strcat(path, "_barrel.md3");
	weaponInfo->barrelModel = trap_R_RegisterModel(path);

	strcpy(path, item->world_model[0]);
	Com_StripExtension(path, path, sizeof(path));
	strcat(path, "_hand.md3");
	weaponInfo->handsModel = trap_R_RegisterModel(path);

	if(!weaponInfo->handsModel)
	{
		weaponInfo->handsModel = trap_R_RegisterModel("models/weapons2/shotgun/shotgun_hand.md3");
	}

	weaponInfo->loopFireSound = qfalse;

	switch (weaponNum)
	{
		case WP_GAUNTLET:
			MAKERGB(weaponInfo->flashDlightColor, 0.07f, 0.07f, 0.2f);
			MAKERGB(weaponInfo->flashDlightColor2, 0.3f, 0.3f, 1.0f);
			weaponInfo->firingSound = trap_S_RegisterSound("sound/weapons/melee/fstrun.wav", qfalse);
			weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/melee/fstatck.wav", qfalse);
			break;

		case WP_LIGHTNING:
			MAKERGB(weaponInfo->flashDlightColor, 0.07f, 0.07f, 0.2f);
			MAKERGB(weaponInfo->flashDlightColor2, 0.3f, 0.3f, 1.0f);
			weaponInfo->readySound = trap_S_RegisterSound("sound/weapons/melee/fsthum.wav", qfalse);
			weaponInfo->firingSound = trap_S_RegisterSound("sound/weapons/lightning/lg_hum.wav", qfalse);

			weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/lightning/lg_fire.wav", qfalse);
			cgs.media.lightningShader = trap_R_RegisterShader("lightningtest");
			cgs.media.lightningEXPShader = trap_R_RegisterShader("lightningExplosionq");
			cgs.media.lightningExplosionModel = trap_R_RegisterModel("models/weaphits/crackle.md3");
//      cgs.media.sfx_lghit1 = trap_S_RegisterSound( "sound/weapons/lg/lg_zap1.wav", qfalse );
//      cgs.media.sfx_lghit2 = trap_S_RegisterSound( "sound/weapons/lg/lg_zap2.wav", qfalse );
//      cgs.media.sfx_lghit3 = trap_S_RegisterSound( "sound/weapons/lg/lg_zap3.wav", qfalse );

			break;

		case WP_GRAPPLING_HOOK:
			weaponInfo->missileModel = trap_R_RegisterModel("models/ammo/rocket/rocket.md3");
			weaponInfo->missileTrailFunc = CG_GrappleTrail;
			weaponInfo->wiTrailTime = 2000;
			weaponInfo->trailRadius = 64;
			weaponInfo->readySound = trap_S_RegisterSound("sound/weapons/melee/fsthum.wav", qfalse);
			weaponInfo->firingSound = trap_S_RegisterSound("sound/weapons/melee/fstrun.wav", qfalse);
			break;

#ifdef MISSIONPACK
		case WP_CHAINGUN:
			weaponInfo->firingSound = trap_S_RegisterSound("sound/weapons/vulcan/wvulfire.wav", qfalse);
			weaponInfo->loopFireSound = qtrue;
			MAKERGB(weaponInfo->flashDlightColor, 1, 1, 0);
			weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/vulcan/vulcanf1b.wav", qfalse);
			weaponInfo->flashSound[1] = trap_S_RegisterSound("sound/weapons/vulcan/vulcanf2b.wav", qfalse);
			weaponInfo->flashSound[2] = trap_S_RegisterSound("sound/weapons/vulcan/vulcanf3b.wav", qfalse);
			weaponInfo->flashSound[3] = trap_S_RegisterSound("sound/weapons/vulcan/vulcanf4b.wav", qfalse);
			weaponInfo->ejectBrassFunc = CG_MachineGunEjectBrass;
			cgs.media.bulletExplosionShader = trap_R_RegisterShader("bulletExplosion");
			break;
#endif

		case WP_MACHINEGUN:
			weaponInfo->firingSound = trap_S_RegisterSound("sound/weapons/machinegun/whir.wav", qfalse);
			MAKERGB(weaponInfo->flashDlightColor, 0.2f, 0.1f, 0.0f);
			MAKERGB(weaponInfo->flashDlightColor2, 0.6f, 0.6f, 0.0f);
			weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/machinegun/machgf1b.wav", qfalse);
			weaponInfo->flashSound[1] = trap_S_RegisterSound("sound/weapons/machinegun/machgf2b.wav", qfalse);
			weaponInfo->flashSound[2] = trap_S_RegisterSound("sound/weapons/machinegun/machgf3b.wav", qfalse);
			weaponInfo->flashSound[3] = trap_S_RegisterSound("sound/weapons/machinegun/machgf4b.wav", qfalse);
			weaponInfo->ejectBrassFunc = CG_MachineGunEjectBrass;
			cgs.media.bulletExplosionShader = trap_R_RegisterShader("zbulletexp");
			break;

		case WP_SHOTGUN:
			MAKERGB(weaponInfo->flashDlightColor, 0.3f, 0.3f, 0.0f);
			MAKERGB(weaponInfo->flashDlightColor2, 0.8f, 0.8f, 0.0f);
			weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/zpm/weapons/supershotgun.wav", qfalse);
			weaponInfo->ejectBrassFunc = CG_ShotgunEjectBrass;
			break;

		case WP_ROCKET_LAUNCHER:
			weaponInfo->missileModel = trap_R_RegisterModel("models/ammo/rocket/rocket.md3");
			weaponInfo->missileSound = trap_S_RegisterSound("sound/weapons/rocket/rockfly.wav", qfalse);
			weaponInfo->missileTrailFunc = CG_RocketTrail;
			weaponInfo->missileTrailFunc2 = CG_MissileTrail;
			weaponInfo->wiTrailTime = 1000;
			weaponInfo->trailRadius = 34;

			MAKERGB(weaponInfo->flashDlightColor, 0.3f, 0.2f, 0.0f);
			MAKERGB(weaponInfo->flashDlightColor2, 0.9f, 0.7f, 0.0f);

			weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/rocket/rocklf1a.wav", qfalse);
			cgs.media.rocketShockRing = trap_R_RegisterModel("models/weaphits/ringexp/expring1.md3");

			cgs.media.rocketShockWaveShader = trap_R_RegisterShader("fireflashring");

			cgs.media.rocketShockWave = trap_R_RegisterModel("models/weaphits/ringexp/expring.md3");
			cgs.media.rocketRingShader = trap_R_RegisterShader("models/weaphits/ringexp/expring.tga");
			cgs.media.PLSmokeShader = trap_R_RegisterShader("smokePlasma");
			cgs.media.GREYSmokeShader = trap_R_RegisterShader("smokegry12");
			cgs.media.rocketCoronaShader = trap_R_RegisterShader("rlcorona");
			cgs.media.smallFire1 = trap_R_RegisterShader("SmallFire1");
			cgs.media.smallFire2 = trap_R_RegisterShader("SmallFire2");
			cgs.media.smallFire3 = trap_R_RegisterShader("SmallFire3");
			cgs.media.smallFire4 = trap_R_RegisterShader("SmallFire4");
			cgs.media.smallFire5 = trap_R_RegisterShader("SmallFire5");
			cgs.media.smallFire6 = trap_R_RegisterShader("SmallFire6");
			cgs.media.smallFire7 = trap_R_RegisterShader("SmallFire7");
			cgs.media.smallFire8 = trap_R_RegisterShader("SmallFire8");
			cgs.media.smallFire9 = trap_R_RegisterShader("SmallFire9");
			cgs.media.smallFire10 = trap_R_RegisterShader("SmallFire10");
			cgs.media.smallFire11 = trap_R_RegisterShader("SmallFire11");
			cgs.media.smallFire12 = trap_R_RegisterShader("SmallFire12");
			cgs.media.smallFire13 = trap_R_RegisterShader("SmallFire13");
			cgs.media.smallFire14 = trap_R_RegisterShader("SmallFire14");
			cgs.media.smallFire15 = trap_R_RegisterShader("SmallFire15");
			cgs.media.smallFire16 = trap_R_RegisterShader("SmallFire16");
			cgs.media.smallFire17 = trap_R_RegisterShader("SmallFire17");
			break;

#ifdef MISSIONPACK
		case WP_PROX_LAUNCHER:
			weaponInfo->missileModel = trap_R_RegisterModel("models/weaphits/proxmine.md3");
			weaponInfo->missileTrailFunc = CG_GrenadeTrail;
			weaponInfo->wiTrailTime = 700;
			weaponInfo->trailRadius = 32;
			MAKERGB(weaponInfo->flashDlightColor, 1, 0.70f, 0);
			weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/proxmine/wstbfire.wav", qfalse);
			cgs.media.grenadeExplosionShader = trap_R_RegisterShader("grenadeExplosion1");
			break;
#endif

		case WP_GRENADE_LAUNCHER:
			weaponInfo->missileModel = trap_R_RegisterModel("models/zpm/grenade/grenade1.md3");
			weaponInfo->missileTrailFunc = CG_GrenadeTrail;
			MAKERGB(weaponInfo->flashDlightColor, 0.3f, 0.2f, 0.0f);
			MAKERGB(weaponInfo->flashDlightColor2, 0.9f, 0.7f, 0.0f);
			weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/grenade/grenlf1a.wav", qfalse);
			cgs.media.grenadeExplosionShader = trap_R_RegisterShader("grenadeExplosion1");
			cgs.media.chargeMetergShader = trap_R_RegisterShader("gchargem");
			cgs.media.smokeTrailShader = trap_R_RegisterShader("smokeTrailq");
			cgs.media.smallFire1 = trap_R_RegisterShader("SmallFire1");
			cgs.media.smallFire2 = trap_R_RegisterShader("SmallFire2");
			cgs.media.smallFire3 = trap_R_RegisterShader("SmallFire3");
			cgs.media.smallFire4 = trap_R_RegisterShader("SmallFire4");
			cgs.media.smallFire5 = trap_R_RegisterShader("SmallFire5");
			cgs.media.smallFire6 = trap_R_RegisterShader("SmallFire6");
			cgs.media.smallFire7 = trap_R_RegisterShader("SmallFire7");
			cgs.media.smallFire8 = trap_R_RegisterShader("SmallFire8");
			cgs.media.smallFire9 = trap_R_RegisterShader("SmallFire9");
			cgs.media.smallFire10 = trap_R_RegisterShader("SmallFire10");
			cgs.media.smallFire11 = trap_R_RegisterShader("SmallFire11");
			cgs.media.smallFire12 = trap_R_RegisterShader("SmallFire12");
			cgs.media.smallFire13 = trap_R_RegisterShader("SmallFire13");
			cgs.media.smallFire14 = trap_R_RegisterShader("SmallFire14");
			cgs.media.smallFire15 = trap_R_RegisterShader("SmallFire15");
			cgs.media.smallFire16 = trap_R_RegisterShader("SmallFire16");
			cgs.media.smallFire17 = trap_R_RegisterShader("SmallFire17");
			break;

#ifdef MISSIONPACK
		case WP_NAILGUN:
			weaponInfo->ejectBrassFunc = CG_NailgunEjectBrass;
//      weaponInfo->missileTrailFunc = CG_NailTrail;
			weaponInfo->trailRadius = 16;
			weaponInfo->wiTrailTime = 250;
			weaponInfo->missileModel = trap_R_RegisterModel("models/weaphits/nail.md3");
			MAKERGB(weaponInfo->flashDlightColor, 1, 0.75f, 0);
			weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/nailgun/wnalfire.wav", qfalse);
			break;
#endif

		case WP_PLASMAGUN:
			trap_R_RegisterModel("models/ammo/plasma/plasmaballr.md3");
			trap_R_RegisterModel("models/ammo/plasma/plasmaball.md3");
			weaponInfo->missileSound = trap_S_RegisterSound("sound/weapons/plasma/lasfly.wav", qfalse);

			MAKERGB(weaponInfo->flashDlightColor, 0.0f, 0.05f, 0.2f);
			MAKERGB(weaponInfo->flashDlightColor2, 0.0f, 0.2f, 0.9f);
			weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/plasma/hyprbf1a.wav", qfalse);
			cgs.media.plasmaExplosionShader = trap_R_RegisterShader("plasmaExplosion1");
			cgs.media.plasmaExplosiongShader = trap_R_RegisterShader("plasmaExplosion1g");
			cgs.media.plasmaExplosionrShader = trap_R_RegisterShader("plasmaExplosion1r");
//      cgs.media.railRings1Shader = trap_R_RegisterShader( "railDisc1" );
			cgs.media.PlasmaRingBShader = trap_R_RegisterShader("plasmaring");
			cgs.media.PlasmaRingRShader = trap_R_RegisterShader("plasmaringr");
			cgs.media.PlasmaRingGShader = trap_R_RegisterShader("plasmaringg");
			cgs.media.PLSmokeShader = trap_R_RegisterShader("smokePlasma");
			cgs.media.energyMarkrShader = trap_R_RegisterShader("gfx/damage/plasma_mrk1r");
			cgs.media.PGPartShader = trap_R_RegisterShader("gfx/misc/plasmapart");
			cgs.media.smallFire1 = trap_R_RegisterShader("SmallFire1");
			cgs.media.smallFire2 = trap_R_RegisterShader("SmallFire2");
			cgs.media.smallFire3 = trap_R_RegisterShader("SmallFire3");
			cgs.media.smallFire4 = trap_R_RegisterShader("SmallFire4");
			cgs.media.smallFire5 = trap_R_RegisterShader("SmallFire5");
			cgs.media.smallFire6 = trap_R_RegisterShader("SmallFire6");
			cgs.media.smallFire7 = trap_R_RegisterShader("SmallFire7");
			cgs.media.smallFire8 = trap_R_RegisterShader("SmallFire8");
			cgs.media.smallFire9 = trap_R_RegisterShader("SmallFire9");
			cgs.media.smallFire10 = trap_R_RegisterShader("SmallFire10");
			cgs.media.smallFire11 = trap_R_RegisterShader("SmallFire11");
			cgs.media.smallFire12 = trap_R_RegisterShader("SmallFire12");
			cgs.media.smallFire13 = trap_R_RegisterShader("SmallFire13");
			cgs.media.smallFire14 = trap_R_RegisterShader("SmallFire14");
			cgs.media.smallFire15 = trap_R_RegisterShader("SmallFire15");
			cgs.media.smallFire16 = trap_R_RegisterShader("SmallFire16");
			cgs.media.smallFire17 = trap_R_RegisterShader("SmallFire17");
			cgs.media.GREYSmokeRingShader = trap_R_RegisterShader("smokegryring");
			cgs.media.LIGHTSmokeRingShader = trap_R_RegisterShader("smokelightring");

			break;

		case WP_RAILGUN:
			weaponInfo->readySound = trap_S_RegisterSound("sound/weapons/railgun/rg_hum.wav", qfalse);
			MAKERGB(weaponInfo->flashDlightColor, 0.2f, 0.2f, 0.2f);
			MAKERGB(weaponInfo->flashDlightColor, 0.7f, 0.7f, 0.7f);
			weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/railgun/railgf1a.wav", qfalse);
//      cgs.media.railExplosionShader = trap_R_RegisterShader( "railExplosion1" );
			cgs.media.rgExplosionShader = trap_R_RegisterShader("rgexp");
			cgs.media.railCoronaShader = trap_R_RegisterShader("rgcorona");

			cgs.media.railRings1Shader = trap_R_RegisterShader("railDisc1");
			cgs.media.railCoreShader = trap_R_RegisterShader("railCore1");
			cgs.media.railCoreglowShader = trap_R_RegisterShader("railCoreglow");
			cgs.media.railRings3Shader = trap_R_RegisterShader("railDisc2");
			cgs.media.railRings2Shader = trap_R_RegisterShader("railDisc");
			trap_R_RegisterShader("rgflash2");
			trap_R_RegisterShader("rgflash1");
			cgs.media.zoomtopModel = trap_R_RegisterModel("models/zpm/zoomv.md3");
			cgs.media.zoombotModel = trap_R_RegisterModel("models/zpm/view/zoombottom.md3");

			trap_R_RegisterShader("zvring");
			cgs.media.railexpringShader = trap_R_RegisterShader("railexpring");
			cgs.media.railexpring2Shader = trap_R_RegisterShader("railexpring2");
			cgs.media.rgringShockWave = trap_R_RegisterModel("models/weaphits/ringexp/railexp/expring.md3");
			cgs.media.lightningShader = trap_R_RegisterShader("lightningtest");

			break;

		case WP_IRAILGUN:
			weaponInfo->readySound = trap_S_RegisterSound("sound/weapons/railgun/rg_hum.wav", qfalse);
			MAKERGB(weaponInfo->flashDlightColor, 0.2f, 0.2f, 0.2f);
			MAKERGB(weaponInfo->flashDlightColor, 0.7f, 0.7f, 0.7f);
			weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/railgun/railgf1a.wav", qfalse);
			weaponInfo->flashSound[1] = trap_S_RegisterSound("sound/weapons/irailgun/railgf1a1.wav", qfalse);
			weaponInfo->flashSound[2] = trap_S_RegisterSound("sound/weapons/irailgun/railgf1a3.wav", qfalse);
			cgs.media.railExplosion2Shader = trap_R_RegisterShader("railExplosion1");
			cgs.media.railRings1Shader = trap_R_RegisterShader("railDisc1");
			cgs.media.railCoreShader = trap_R_RegisterShader("railCore1");
			cgs.media.railCoreglowShader = trap_R_RegisterShader("railCoreglow");
			cgs.media.railRings3Shader = trap_R_RegisterShader("railDisc2");
			cgs.media.railRings2Shader = trap_R_RegisterShader("railDisc");
			cgs.media.chargeMeterShader = trap_R_RegisterShader("rchargem");
			cgs.media.chargefxMeterShader = trap_R_RegisterShader("meterfx");
			cgs.media.irailCorebShader = trap_R_RegisterShader("icore1");
			cgs.media.irailCorerShader = trap_R_RegisterShader("irailCorer");
//      cgs.media.tipShader = trap_R_RegisterShader( "tip001" );
//      cgs.media.tip1Shader = trap_R_RegisterShader( "tip002" );
			trap_R_RegisterModel("models/weaphits/irail/trailtip.md3");
			trap_R_RegisterModel("models/weaphits/irail/trailtip1.md3");
			cgs.media.zoomtopModel = trap_R_RegisterModel("models/zpm/zoomv.md3");
			cgs.media.zoombotModel = trap_R_RegisterModel("models/zpm/view/zoombottom.md3");
			trap_R_RegisterShader("zvring");
			break;

		case WP_BFG:
			weaponInfo->readySound = trap_S_RegisterSound("sound/weapons/bfg/bfg_hum.wav", qfalse);
			weaponInfo->missileTrailFunc = CG_BFGTrail;
			weaponInfo->missileDlight = 80;
			MAKERGB(weaponInfo->missileDlightColor, 0.0f, 0.1f, 0.3f);
			MAKERGB(weaponInfo->flashDlightColor, 0.0f, 0.1f, 0.3f);
			weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/bfg/bfg_fire.wav", qfalse);
			cgs.media.bfgExplosionShader = trap_R_RegisterShader("bfgExplosion1");
			weaponInfo->missileModel = trap_R_RegisterModel("models/weaphits/bfg.md3");
			weaponInfo->missileSound = trap_S_RegisterSound("sound/weapons/rocket/rockfly.wav", qfalse);
			cgs.media.smallFire1 = trap_R_RegisterShader("SmallFire1");
			cgs.media.smallFire2 = trap_R_RegisterShader("SmallFire2");
			cgs.media.smallFire3 = trap_R_RegisterShader("SmallFire3");
			cgs.media.smallFire4 = trap_R_RegisterShader("SmallFire4");
			cgs.media.smallFire5 = trap_R_RegisterShader("SmallFire5");
			cgs.media.smallFire6 = trap_R_RegisterShader("SmallFire6");
			cgs.media.smallFire7 = trap_R_RegisterShader("SmallFire7");
			cgs.media.smallFire8 = trap_R_RegisterShader("SmallFire8");
			cgs.media.smallFire9 = trap_R_RegisterShader("SmallFire9");
			cgs.media.smallFire10 = trap_R_RegisterShader("SmallFire10");
			cgs.media.smallFire11 = trap_R_RegisterShader("SmallFire11");
			cgs.media.smallFire12 = trap_R_RegisterShader("SmallFire12");
			cgs.media.smallFire13 = trap_R_RegisterShader("SmallFire13");
			cgs.media.smallFire14 = trap_R_RegisterShader("SmallFire14");
			cgs.media.smallFire15 = trap_R_RegisterShader("SmallFire15");
			cgs.media.smallFire16 = trap_R_RegisterShader("SmallFire16");
			cgs.media.smallFire17 = trap_R_RegisterShader("SmallFire17");
			break;
		case WP_FLAMETHROWER:
			weaponInfo->firingSound = cgs.media.flameStreamSound;
			cgs.media.smallFire1 = trap_R_RegisterShader("SmallFire1");
			cgs.media.smallFire2 = trap_R_RegisterShader("SmallFire2");
			cgs.media.smallFire3 = trap_R_RegisterShader("SmallFire3");
			cgs.media.smallFire4 = trap_R_RegisterShader("SmallFire4");
			cgs.media.smallFire5 = trap_R_RegisterShader("SmallFire5");
			cgs.media.smallFire6 = trap_R_RegisterShader("SmallFire6");
			cgs.media.smallFire7 = trap_R_RegisterShader("SmallFire7");
			cgs.media.smallFire8 = trap_R_RegisterShader("SmallFire8");
			cgs.media.smallFire9 = trap_R_RegisterShader("SmallFire9");
			cgs.media.smallFire10 = trap_R_RegisterShader("SmallFire10");
			cgs.media.smallFire11 = trap_R_RegisterShader("SmallFire11");
			cgs.media.smallFire12 = trap_R_RegisterShader("SmallFire12");
			cgs.media.smallFire13 = trap_R_RegisterShader("SmallFire13");
			cgs.media.smallFire14 = trap_R_RegisterShader("SmallFire14");
			cgs.media.smallFire15 = trap_R_RegisterShader("SmallFire15");
			cgs.media.smallFire16 = trap_R_RegisterShader("SmallFire16");
			cgs.media.smallFire17 = trap_R_RegisterShader("SmallFire17");
			return;
			break;

		default:
			MAKERGB(weaponInfo->flashDlightColor, 0.5f, 0.5f, 0.5f);
			MAKERGB(weaponInfo->flashDlightColor2, 0.7f, 0.7f, 0.7f);
			weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/rocket/rocklf1a.wav", qfalse);
			break;
	}
}

/*
=================
CG_RegisterItemVisuals

The server says this item is used on this level
=================
*/
void CG_RegisterItemVisuals(int itemNum)
{
	itemInfo_t     *itemInfo;
	gitem_t        *item;

	if(itemNum < 0 || itemNum >= bg_numItems)
	{
//      CG_Error( "CG_RegisterItemVisuals: itemNum %d out of range [0-%d]", itemNum, bg_numItems-1 );
	}

	itemInfo = &cg_items[itemNum];
	item = &bg_itemlist[itemNum];

	if(itemInfo->registered)
	{
		return;
	}


	memset(itemInfo, 0, sizeof(&itemInfo));
	itemInfo->registered = qtrue;


	//
	// powerups have an accompanying ring or sphere
	//
	if(item->giType == IT_POWERUP || item->giType == IT_TEAM)
	{

		switch (item->giTag)
		{
			case PW_QUAD:
				if(!itemInfo->quadregi)
				{
					cgs.media.pQuadShader = trap_R_RegisterShader("quadprojectile");
					cgs.media.quadWeaponShader = trap_R_RegisterShader("powerups/quadWeapon");
					cgs.media.quadShader = trap_R_RegisterShader("powerups/quad");
					cgs.media.quadexpShader = trap_R_RegisterShader("quadexp");
					itemInfo->models[0] = trap_R_RegisterModel("models/zpm/powerups/instant/quad.md3");
					itemInfo->icon = trap_R_RegisterShader("icons/quad");
					if(item->world_model[1])
					{
						itemInfo->models[1] = trap_R_RegisterModel("models/zpm/powerups/instant/quadring.md3");
					}
					itemInfo->quadregi = qtrue;
				}
				break;
			case PW_BATTLESUIT:
				if(!itemInfo->battleregi)
				{
					cgs.media.battleSuitShader = trap_R_RegisterShader("powerups/battleSuit");
					cgs.media.battleWeaponShader = trap_R_RegisterShader("powerups/battleWeapon");
					itemInfo->models[0] = trap_R_RegisterModel("models/powerups/instant/enviro.md3");
					itemInfo->icon = trap_R_RegisterShader("icons/envirosuit");
					if(item->world_model[1])
					{
						itemInfo->models[1] = trap_R_RegisterModel("models/powerups/instant/enviro_ring.md3");
					}
					itemInfo->battleregi = qtrue;
				}
				break;
			case PW_SPAWNPROT:
				if(!itemInfo->battleregi)
				{
					cgs.media.battleSuitShader = trap_R_RegisterShader("powerups/battleSuit");
					cgs.media.battleWeaponShader = trap_R_RegisterShader("powerups/battleWeapon");
					itemInfo->models[0] = trap_R_RegisterModel("models/powerups/instant/enviro.md3");
					itemInfo->icon = trap_R_RegisterShader("icons/envirosuit");
					if(item->world_model[1])
					{
						itemInfo->models[1] = trap_R_RegisterModel("models/powerups/instant/enviro_ring.md3");
					}
					itemInfo->battleregi = qtrue;
				}
				break;
			case PW_HASTE:
				if(!itemInfo->hasteregi)
				{
					cgs.media.hastezTrailShader = trap_R_RegisterShader("hastetrailz");
					itemInfo->models[0] = trap_R_RegisterModel("models/powerups/instant/haste.md3");
					itemInfo->icon = trap_R_RegisterShader("icons/haste");
					if(item->world_model[1])
					{
						itemInfo->models[1] = trap_R_RegisterModel("models/powerups/instant/haste_ring.md3");
					}
					itemInfo->hasteregi = qtrue;
				}
				break;
			case PW_INVIS:
				if(!itemInfo->invisregi)
				{
					cgs.media.invisShader = trap_R_RegisterShader("powerups/invisibility");
					itemInfo->models[0] = trap_R_RegisterModel("models/powerups/instant/invis.md3");
					itemInfo->icon = trap_R_RegisterShader("icons/invis");
					if(item->world_model[1])
					{
						itemInfo->models[1] = trap_R_RegisterModel("models/powerups/instant/invis_ring.md3");
					}
					itemInfo->invisregi = qtrue;
				}
				break;
			case PW_REGEN:
				if(!itemInfo->speedregi)
				{
					cgs.media.regenShader = trap_R_RegisterShader("powerups/regen");
					itemInfo->models[0] = trap_R_RegisterModel("models/powerups/instant/regen.md3");
					itemInfo->icon = trap_R_RegisterShader("icons/regen");
					if(item->world_model[1])
					{
						itemInfo->models[1] = trap_R_RegisterModel("models/powerups/instant/regen_ring.md3");
					}
					itemInfo->speedregi = qtrue;
				}
				break;
			case PW_FLIGHT:
				if(!itemInfo->flightregi)
				{
					itemInfo->models[0] = trap_R_RegisterModel("models/powerups/instant/flight.md3");
					itemInfo->icon = trap_R_RegisterShader("icons/flight");
					if(item->world_model[1])
					{
						itemInfo->models[1] = trap_R_RegisterModel("models/powerups/instant/flight_ring.md3");
					}
					itemInfo->flightregi = qtrue;
				}
				break;
			case PW_REDFLAG:
				itemInfo->models[0] = cgs.media.redFlagModel = trap_R_RegisterModel("models/flags/r_flag.md3");
				if(item->world_model[1])
				{
					itemInfo->models[1] = trap_R_RegisterModel(item->world_model[1]);
				}
				itemInfo->icon = trap_R_RegisterShader(item->icon);
				break;
			case PW_BLUEFLAG:
				itemInfo->models[0] = cgs.media.blueFlagModel = trap_R_RegisterModel("models/flags/b_flag.md3");
				if(item->world_model[1])
				{
					itemInfo->models[1] = trap_R_RegisterModel(item->world_model[1]);
				}
				itemInfo->icon = trap_R_RegisterShader(item->icon);
				break;
		}
	}

	if(item->giType == IT_HOLDABLE)
	{
		switch (item->giTag)
		{
			case HI_TELEPORTER:
				itemInfo->models[0] = trap_R_RegisterModel("models/powerups/holdable/teleporter.md3");
				itemInfo->icon = trap_R_RegisterShader("icons/teleporter");
				break;
			case HI_MEDKIT:
				itemInfo->models[0] = trap_R_RegisterModel("models/powerups/holdable/medkit.md3");
				itemInfo->icon = trap_R_RegisterShader("icons/medkit");
				if(item->world_model[1])
				{
					itemInfo->models[1] = trap_R_RegisterModel("models/powerups/holdable/medkit_sphere.md3");
				}
				break;
		}
	}

	if(item->giType == IT_HEALTH)
	{
		switch (item->giTag)
		{
			case IH_5HEALTH:
				itemInfo->models[0] = trap_R_RegisterModel("models/zpm/health/small/zsmall.md3");
				itemInfo->icon = trap_R_RegisterShader("icons/iconh_green");
				break;
			case IH_20HEALTH:
				itemInfo->models[0] = trap_R_RegisterModel("models/zpm/health/med/zmed.md3");
				itemInfo->icon = trap_R_RegisterShader("icons/iconh_yellow");
				break;
			case IH_40HEALTH:
				itemInfo->models[0] = trap_R_RegisterModel("models/zpm/health/large/zlarge.md3");
				itemInfo->icon = trap_R_RegisterShader("icons/iconh_red");
				break;
			case IH_MHEALTH:
				itemInfo->models[0] = trap_R_RegisterModel("models/zpm/powerups/health/mhealth.md3");
				itemInfo->icon = trap_R_RegisterShader("icons/iconh_mega");
				break;
		}
	}

	if(item->giType == IT_ARMOR)
	{
		switch (item->giTag)
		{
			case IA_SHARD:
				itemInfo->models[0] = trap_R_RegisterModel("models/zpm/powerups/armor/sarmor.md3");
				itemInfo->icon = trap_R_RegisterShader("icons/iconr_shard");
				break;
			case IA_YARMOR:
				itemInfo->models[0] = trap_R_RegisterModel("models/zpm/powerups/armor/yarmor.md3");
				itemInfo->icon = trap_R_RegisterShader("icons/iconr_yellow");
				break;
			case IA_RARMOR:
				itemInfo->models[0] = trap_R_RegisterModel("models/zpm/powerups/armor/rarmor.md3");
				itemInfo->icon = trap_R_RegisterShader("icons/iconr_red");
				break;
		}
	}

	if(item->giType == IT_WEAPON || IT_AMMO)
	{
		if(item->giType == IT_WEAPON)
		{
			switch (item->giTag)
			{
				case WP_GAUNTLET:
					itemInfo->models[0] = trap_R_RegisterModel("models/weapons2/gauntlet/gauntlet.md3");
					itemInfo->icon = trap_R_RegisterShader("icons/iconw_gauntlet");
					break;
				case WP_LIGHTNING:
					itemInfo->models[0] = trap_R_RegisterModel("models/weapons2/lightning/lightning.md3");
					itemInfo->icon = trap_R_RegisterShader("icons/iconw_lightning");
					break;
				case WP_GRAPPLING_HOOK:
					itemInfo->models[0] = trap_R_RegisterModel("models/weapons2/grapple/grapple.md3");
					itemInfo->icon = trap_R_RegisterShader("icons/iconw_grapple");
					break;
				case WP_MACHINEGUN:
					itemInfo->models[0] = trap_R_RegisterModel("models/weapons2/machinegun/machinegun.md3");
					itemInfo->icon = trap_R_RegisterShader("icons/iconw_machinegun");
					break;
				case WP_SHOTGUN:
					itemInfo->models[0] = trap_R_RegisterModel("models/weapons2/shotgun/shotgun.md3");
					itemInfo->icon = trap_R_RegisterShader("icons/iconw_shotgun");
					break;
				case WP_ROCKET_LAUNCHER:
					itemInfo->models[0] = trap_R_RegisterModel("models/weapons2/rocketl/rocketl.md3");
					itemInfo->icon = trap_R_RegisterShader("icons/iconw_rocket");
					break;
				case WP_GRENADE_LAUNCHER:
					itemInfo->models[0] = trap_R_RegisterModel("models/weapons2/grenadel/grenadel.md3");
					itemInfo->icon = trap_R_RegisterShader("icons/iconw_grenade");
					break;
				case WP_PLASMAGUN:
					itemInfo->models[0] = trap_R_RegisterModel("models/weapons2/plasma/plasma.md3");
					itemInfo->icon = trap_R_RegisterShader("icons/iconw_plasma");
					break;
				case WP_RAILGUN:
					itemInfo->models[0] = trap_R_RegisterModel("models/weapons2/railgun/railgun.md3");
					itemInfo->icon = trap_R_RegisterShader("icons/iconw_railgun");
					break;
				case WP_IRAILGUN:
					itemInfo->models[0] = trap_R_RegisterModel("models/weapons2/irailgun/irailgun.md3");
					itemInfo->icon = trap_R_RegisterShader("icons/iconw_irailgun");
					break;
				case WP_BFG:
					itemInfo->models[0] = trap_R_RegisterModel("models/weapons2/bfg/bfg.md3");
					itemInfo->icon = trap_R_RegisterShader("icons/iconw_bfg");
					break;
				case WP_FLAMETHROWER:
					itemInfo->models[0] = trap_R_RegisterModel("models/weapons2/ft/ft.md3");
					itemInfo->icon = trap_R_RegisterShader("icons/iconw_ft");
					break;

			}
		}
		if(item->giType == IT_AMMO)
		{
			switch (item->giTag)
			{
				case WP_LIGHTNING:
					itemInfo->models[0] = trap_R_RegisterModel("models/zpm/powerups/ammo/lgammo.md3");
					itemInfo->icon = trap_R_RegisterShader("icons/icona_lightning");
					break;
				case WP_MACHINEGUN:
					itemInfo->models[0] = trap_R_RegisterModel("models/zpm/powerups/ammo/mgammo.md3");
					itemInfo->icon = trap_R_RegisterShader("icons/icona_machinegun");
					break;
				case WP_SHOTGUN:
					itemInfo->models[0] = trap_R_RegisterModel("models/zpm/powerups/ammo/sgammo.md3");
					itemInfo->icon = trap_R_RegisterShader("icons/icona_shotgun");
					break;
				case WP_ROCKET_LAUNCHER:
					itemInfo->models[0] = trap_R_RegisterModel("models/zpm/powerups/ammo/rlammo.md3");
					itemInfo->icon = trap_R_RegisterShader("icons/icona_rocket");
					break;
				case WP_GRENADE_LAUNCHER:
					itemInfo->models[0] = trap_R_RegisterModel("models/zpm/powerups/ammo/glammo.md3");
					itemInfo->icon = trap_R_RegisterShader("icons/icona_grenade");
					break;
				case WP_PLASMAGUN:
					itemInfo->models[0] = trap_R_RegisterModel("models/zpm/powerups/ammo/pgammo.md3");
					itemInfo->icon = trap_R_RegisterShader("icons/icona_plasma");
					break;
				case WP_RAILGUN:
					itemInfo->models[0] = trap_R_RegisterModel("models/zpm/powerups/ammo/rgammo.md3");
					itemInfo->icon = trap_R_RegisterShader("icons/icona_railgun");
					break;
				case WP_IRAILGUN:
					itemInfo->models[0] = trap_R_RegisterModel("models/powerups/ammo/railgunam.md3");
					itemInfo->icon = trap_R_RegisterShader("icons/icona_irailgun");
					break;
				case WP_BFG:
					itemInfo->models[0] = trap_R_RegisterModel("models/powerups/ammo/bfgam.md3");
					itemInfo->icon = trap_R_RegisterShader("icons/icona_bfg");
					break;
				case WP_FLAMETHROWER:
					itemInfo->models[0] = trap_R_RegisterModel("models/powerups/ammo/bfgam.md3");
					itemInfo->icon = trap_R_RegisterShader("icons/icona_ft");
					break;
			}
		}
		CG_RegisterWeapon(item->giTag);
	}
}


/*
========================================================================================

VIEW WEAPON

========================================================================================
*/

/*
=================
CG_MapTorsoToWeaponFrame

=================
*/
static int CG_MapTorsoToWeaponFrame(clientInfo_t * ci, int frame)
{

	// change weapon
	if(frame >= ci->animations[TORSO_DROP].firstFrame && frame < ci->animations[TORSO_DROP].firstFrame + 9)
	{
		return frame - ci->animations[TORSO_DROP].firstFrame + 6;
	}

	// stand attack
	if(frame >= ci->animations[TORSO_ATTACK].firstFrame && frame < ci->animations[TORSO_ATTACK].firstFrame + 6)
	{
		return 1 + frame - ci->animations[TORSO_ATTACK].firstFrame;
	}

	// stand attack 2
	if(frame >= ci->animations[TORSO_ATTACK2].firstFrame && frame < ci->animations[TORSO_ATTACK2].firstFrame + 6)
	{
		return 1 + frame - ci->animations[TORSO_ATTACK2].firstFrame;
	}

	return 0;
}


/*
==============
CG_CalculateWeaponPosition
==============
*/
static void CG_CalculateWeaponPosition(vec3_t origin, vec3_t angles)
{
	float           scale;
	int             delta;
	float           fracsin;

	VectorCopy(cg.refdef[0].vieworg, origin);
	VectorCopy(cg.refdefViewAngles[0], angles);

	// on odd legs, invert some angles
	if(cg.bobcycle & 1)
	{
		scale = -cg.xyspeed;
	}
	else
	{
		scale = cg.xyspeed;
	}

	// gun angles from bobbing
	angles[ROLL] += scale * cg.bobfracsin * 0.005;
	angles[YAW] += scale * cg.bobfracsin * 0.01;
	angles[PITCH] += cg.xyspeed * cg.bobfracsin * 0.005;

	// drop the weapon when landing
	delta = cg.time - cg.landTime;
	if(delta < LAND_DEFLECT_TIME)
	{
		origin[2] += cg.landChange * 0.25 * delta / LAND_DEFLECT_TIME;
	}
	else if(delta < LAND_DEFLECT_TIME + LAND_RETURN_TIME)
	{
		origin[2] += cg.landChange * 0.25 * (LAND_DEFLECT_TIME + LAND_RETURN_TIME - delta) / LAND_RETURN_TIME;
	}

#if 0
	// drop the weapon when stair climbing
	delta = cg.time - cg.stepTime;
	if(delta < STEP_TIME / 2)
	{
		origin[2] -= cg.stepChange * 0.25 * delta / (STEP_TIME / 2);
	}
	else if(delta < STEP_TIME)
	{
		origin[2] -= cg.stepChange * 0.25 * (STEP_TIME - delta) / (STEP_TIME / 2);
	}
#endif

	// idle drift
	scale = cg.xyspeed + 40;
	fracsin = sin(cg.time * 0.001);
	angles[ROLL] += scale * fracsin * 0.01;
	angles[YAW] += scale * fracsin * 0.01;
	angles[PITCH] += scale * fracsin * 0.01;
}

#define	STYPE_LIGHTNING	 STYPE_REPEAT

/*
==============
CG_LightningBolt

  TODO: this needs to be fixed for multiplay. entities being hurt need to be sent
  by server to all clients, so they draw the correct effects.
==============
*/
static void CG_LightningBolt(centity_t * cent, vec3_t flashorigin)
{

}

/*
===============
CG_SpawnRailTrail

Origin will be the exact tag point, which is slightly
different than the muzzle point used for determining hits.
===============
*/
static void CG_SpawnRailTrail(centity_t * cent, vec3_t origin, vec3_t dir)
{
	clientInfo_t   *ci;

	if(cent->currentState.weapon == WP_RAILGUN)
	{
		if(!cent->pe.railgunFlash)
		{
			return;
		}
		cent->pe.railgunFlash = qtrue;
		ci = &cgs.clientinfo[cent->currentState.clientNum];
		CG_RailTrail(ci, origin, cent->pe.railgunImpact);
	}
	else if(cent->currentState.weapon == WP_IRAILGUN)
	{
		if(!cent->pe.railgunFlash)
		{
			return;
		}
		cent->pe.railgunFlash = qtrue;
		ci = &cgs.clientinfo[cent->currentState.clientNum];
		CG_RailTrail(ci, origin, cent->pe.railgunImpact);
	}
}

#define		SPIN_SPEED	0.9
#define		COAST_TIME	1200
//NT - no longer static, called from cg_players.c/CG_PlayerProxy
float CG_MachinegunSpinAngle(centity_t * cent)
{
	int             delta;
	float           angle;
	float           speed;

	delta = cg.time - cent->pe.barrelTime;
	if(cent->pe.barrelSpinning)
	{
		angle = cent->pe.barrelAngle + delta * SPIN_SPEED;
	}
	else
	{
		if(delta > COAST_TIME)
		{
			delta = COAST_TIME;
		}

		speed = 0.5 * (SPIN_SPEED + (float)(COAST_TIME - delta) / COAST_TIME);
		angle = cent->pe.barrelAngle + delta * speed;
	}
	if(cent->currentState.weapon == WP_MACHINEGUN || WP_GAUNTLET)
	{
		if(cent->pe.barrelSpinning == !(cent->currentState.eFlags & EF_FIRING))
		{
			cent->pe.barrelTime = cg.time;
			cent->pe.barrelAngle = AngleMod(angle);
			cent->pe.barrelSpinning = !!(cent->currentState.eFlags & EF_FIRING);

			if(cent->currentState.weapon == WP_MACHINEGUN && !cent->pe.barrelSpinning)
			{
				trap_S_StartSound(NULL, cent->currentState.number, CHAN_WEAPON,
								  trap_S_RegisterSound("sound/weapons/machinegun/winddown.wav", qfalse));
			}

		}
	}

	return angle;
}


/*
======================
CG_MachinegunSpinAngle
======================

#define		ACCEL_SPEED	1.5
#define		RAILTIME_TIME	1500
static float	CG_RailSpinAngle( centity_t *cent ) {
	int		delta;
	float	angle;
	float	speed;

	delta = cg.time - cent->pe.barrelTime;
	if ( cent->pe.barrelSpinning ) {
		angle = cent->pe.barrelAngle + delta * ACCEL_SPEED;
	} else {
		if ( delta > RAILTIME_TIME ) {
			delta = RAILTIME_TIME;
		}
		speed = 0.5 * ( ACCEL_SPEED + (float)( RAILTIME_TIME - delta ) / RAILTIME_TIME );
		angle = cent->pe.barrelAngle + delta * speed;
	}
	cent->pe.RbarrelTime = 3500;
	if (cent->ps->weaponstate == WEAPON_PREFIRING){
		cent->pe.RbarrelTime -= cg.time;
		cent->pe.barrelAngle = AngleMod( angle );
	}
		cent->pe.barrelTime = cg.time;


	return angle;
}
*/

/*
========================
CG_AddWeaponWithPowerups
========================
*/
static void CG_AddWeaponWithPowerups(refEntity_t * gun, int powerups, int clientnum)
{
	playerState_t  *ps;
	centity_t      *cent;

	ps = &cg.predictedPlayerState;

	if(clientnum != cg.predictedPlayerState.clientNum)
	{
		cent = &cg_entities[clientnum];
	}
	else
	{
		cent = &cg.predictedPlayerEntity;
	}

	// add powerup effects
	if(powerups & (1 << PW_INVIS))
	{
		gun->customShader = cgs.media.invisShader;
		trap_R_AddRefEntityToScene(gun);
	}
	else
	{
		trap_R_AddRefEntityToScene(gun);

		if(powerups & (1 << PW_BATTLESUIT))
		{
			gun->customShader = cgs.media.battleWeaponShader;
			trap_R_AddRefEntityToScene(gun);
		}
		if(powerups & (1 << PW_SPAWNPROT))
		{
			gun->customShader = cgs.media.battleWeaponShader;
			trap_R_AddRefEntityToScene(gun);
		}

		if(powerups & (1 << PW_QUAD))
		{
			if(ps->persistant[PERS_TEAM] == TEAM_RED)
			{
				gun->customShader = cgs.media.redquadWeaponShader;
				trap_R_AddRefEntityToScene(gun);
			}
			else
			{
				gun->customShader = cgs.media.quadWeaponShader;
				trap_R_AddRefEntityToScene(gun);
			}
		}

		if(cent->pe.armorDamagedTime > cg.time - 400)
		{
			float           alpha;

			alpha = (400.0 - (float)(cg.time - cent->pe.armorDamagedTime)) / 400.0;

			gun->shaderRGBA[0] = (unsigned char)(130.0 * alpha);
			gun->shaderRGBA[1] = (unsigned char)(255.0 * alpha);
			gun->shaderRGBA[2] = (unsigned char)(50.0 * alpha);

			gun->customShader = cgs.media.lghitweapShader;
			trap_R_AddRefEntityToScene(gun);
		}

		if(cent->pe.teslaDamagedTime > cg.time - 400)
		{
			float           alpha;

			alpha = (400.0 - (float)(cg.time - cent->pe.teslaDamagedTime)) / 400.0;

			gun->shaderRGBA[0] = (unsigned char)(50.0 * alpha);
			gun->shaderRGBA[1] = (unsigned char)(130.0 * alpha);
			gun->shaderRGBA[2] = (unsigned char)(255.0 * alpha);

			gun->customShader = cgs.media.lghitweapShader;
			trap_R_AddRefEntityToScene(gun);
		}
	}
}

/*
=============
CG_AddPlayerWeapon

Used for both the view weapon (ps is valid) and the world modelother character models (ps is NULL)
The main player will have this called for BOTH cases, so effects like light and
sound should only be done on the world model case.
=============
*/
void CG_AddPlayerWeapon(refEntity_t * parent, playerState_t * ps, centity_t * cent, int team)
{
	refEntity_t     gun, barrel, flash;
	vec3_t          angles, muzzlePoint, forward, right, up;
	weapon_t        weaponNum;
	weaponInfo_t   *weapon;
	centity_t      *nonPredictedCent, *nonPredictedCent1;
	int             charge, charge2, brightness, c, start, end;
	clientInfo_t   *ci;
	qboolean        isPlayer, firing;



	nonPredictedCent = &cg_entities[cent->currentState.clientNum];

	// if the index of the nonPredictedCent is not the same as the clientNum
	// then this is a fake player (like on teh single player podiums), so
	// go ahead and use the cent
	if((nonPredictedCent - cg_entities) != cent->currentState.clientNum)
	{
		nonPredictedCent = cent;
	}

	nonPredictedCent1 = &cg.predictedPlayerEntity;

	VectorCopy(cg.predictedPlayerState.origin, &muzzlePoint[0]);

	firing = ((cent->currentState.eFlags & EF_FIRING) != 0);

	AngleVectors(cg.predictedPlayerState.viewangles, forward, right, up);
	VectorMA(muzzlePoint, 14, forward, muzzlePoint);

	// (SA) might as well have this check consistant throughout the routine
	isPlayer = (qboolean) (cent->currentState.clientNum == cg.snap->ps.clientNum);


	charge = cg.snap->ps.stats[STAT_RAIL_CHARGE] / 70;
	if(charge < -111)
	{
		charge = -111;
	}
	if(charge > 0)
	{
		charge = -111;
	}
	charge2 = (charge + charge);
	brightness = charge - charge2;

	weaponNum = cent->currentState.weapon;
	CG_RegisterWeapon(weaponNum);
	weapon = &cg_weapons[weaponNum];

	// add the weapon
	memset(&gun, 0, sizeof(gun));
	VectorCopy(parent->lightingOrigin, gun.lightingOrigin);
	gun.shadowPlane = parent->shadowPlane;
	gun.renderfx = parent->renderfx;

	// set custom shading for railgun refire rate
	if(ps)
	{
		if(weaponNum == WP_RAILGUN && ps->weaponstate == WEAPON_FIRING)
		{
			float           f;

			ci = &cgs.clientinfo[cent->currentState.clientNum];
			f = (float)ps->weaponTime / 1500;
			gun.shaderRGBA[0] = ci->color1[0] * (0.9f - f) * 0xff;
			gun.shaderRGBA[1] = ci->color1[1] * (0.9f - f) * 0xff;
			gun.shaderRGBA[2] = ci->color1[2] * (0.9f - f) * 0xff;
		}
		else if(weaponNum == WP_RAILGUN && ps->weaponstate == WEAPON_FIRING)
		{
			float           f;

			ci = &cgs.clientinfo[cent->currentState.clientNum];
			f = (float)ps->weaponTime / 3500;
			gun.shaderRGBA[0] = ci->color1[0] * (0.01f - f) * 0xff;
			gun.shaderRGBA[1] = ci->color1[1] * (0.01f - f) * 0xff;
			gun.shaderRGBA[2] = ci->color1[2] * (0.01f - f) * 0xff;
			//NT - charge during radius rail prefire
		}
		else if(weaponNum == WP_IRAILGUN && ps->weaponstate == WEAPON_FIRING)
		{
			float           f;

			f = (float)ps->weaponTime / 1500;
			gun.shaderRGBA[1] = 0;
			gun.shaderRGBA[0] = gun.shaderRGBA[2] = 255 * (1.0 - f);
		}
		else if(weaponNum == WP_IRAILGUN && ps->weaponstate == WEAPON_FIRING)
		{
			float           f;

			f = (float)ps->weaponTime / 3500;
			gun.shaderRGBA[1] = 0;
			gun.shaderRGBA[0] = gun.shaderRGBA[2] = 255 * (1.0 - f);
			//NT - charge during radius rail prefire
		}
		else if(weaponNum == WP_RAILGUN)
		{
			ci = &cgs.clientinfo[cent->currentState.clientNum];
			gun.shaderRGBA[0] = ci->color1[0] * 1.0f * 0xff;
			gun.shaderRGBA[1] = ci->color1[1] * 1.0f * 0xff;
			gun.shaderRGBA[2] = ci->color1[2] * 1.0f * 0xff;
			gun.shaderRGBA[3] = 255;
		}
		else
		{
			if(weaponNum != WP_PLASMAGUN)
			{
				gun.shaderRGBA[1] = 255;
				gun.shaderRGBA[0] = 255;
				gun.shaderRGBA[2] = 255;
				gun.shaderRGBA[3] = 255;
			}
		}
	}
	else
	{
		if(weaponNum != WP_PLASMAGUN)
		{
			gun.shaderRGBA[1] = 255;
			gun.shaderRGBA[0] = 255;
			gun.shaderRGBA[2] = 255;
			gun.shaderRGBA[3] = 255;
		}
	}
	if(cgs.gametype >= GT_TEAM)
	{
		ci = &cgs.clientinfo[cent->currentState.clientNum];
		if(ps)
		{
			if(weaponNum == WP_PLASMAGUN && ci->team == TEAM_RED)
			{
				gun.shaderRGBA[1] = 10;
				gun.shaderRGBA[0] = 255;
				gun.shaderRGBA[2] = 10;
				gun.shaderRGBA[3] = 255;
			}
			else if(weaponNum == WP_PLASMAGUN && ci->team == TEAM_BLUE)
			{
				gun.shaderRGBA[1] = 50;
				gun.shaderRGBA[0] = 50;
				gun.shaderRGBA[2] = 255;
				gun.shaderRGBA[3] = 255;
			}
		}
		else
		{
			gun.shaderRGBA[1] = 255;
			gun.shaderRGBA[0] = 255;
			gun.shaderRGBA[2] = 255;
			gun.shaderRGBA[3] = 255;
		}
	}
	else
	{
		if(ps)
		{
			if(weaponNum == WP_PLASMAGUN)
			{
				gun.shaderRGBA[1] = 255;
				gun.shaderRGBA[0] = 0;
				gun.shaderRGBA[2] = 0;
				gun.shaderRGBA[3] = 255;
			}
		}
		else
		{
			gun.shaderRGBA[1] = 255;
			gun.shaderRGBA[0] = 255;
			gun.shaderRGBA[2] = 255;
			gun.shaderRGBA[3] = 255;
		}
	}

	gun.hModel = weapon->weaponModel;
	if(!gun.hModel)
	{
		return;
	}

	//if ( !ps ) {
	// add weapon ready sound
	cent->pe.lightningFiring = qfalse;
	if((cent->currentState.eFlags & EF_FIRING))
	{
		// lightning gun and guantlet make a different sound when fire is held down&& weapon->firingSound
		if(weaponNum == WP_FLAMETHROWER)
		{
			trap_S_AddRealLoopingSound(cent->currentState.number, cent->lerpOrigin, vec3_origin, cgs.media.flameStreamSound);
			trap_S_AddRealLoopingSound(cent->currentState.number, cent->lerpOrigin, vec3_origin, cgs.media.flameStreamSound);
			trap_S_AddRealLoopingSound(cent->currentState.number, cent->lerpOrigin, vec3_origin, cgs.media.flameBlowSound);
		}
		if(weapon->firingSound)
		{
			trap_S_AddLoopingSound(cent->currentState.number, cent->lerpOrigin, vec3_origin, weapon->firingSound);
		}
		if(weapon->readySound)
		{
			trap_S_AddLoopingSound(cent->currentState.number, cent->lerpOrigin, vec3_origin, weapon->readySound);
		}
		cent->pe.lightningFiring = qtrue;
	}
	else if(weapon->readySound)
	{
		trap_S_AddLoopingSound(cent->currentState.number, cent->lerpOrigin, vec3_origin, weapon->readySound);
	}
//  }

	CG_PositionEntityOnTag(&gun, parent, parent->hModel, "tag_weapon");

	if(isPlayer)
	{
		if(cent->currentState.clientNum == cg.predictedPlayerState.clientNum)
		{
			CG_AddWeaponWithPowerups(&gun, cg.predictedPlayerEntity.currentState.powerups, cg.predictedPlayerState.clientNum);
		}
	}
	else
	{
		if(nonPredictedCent->currentState.clientNum != cg.predictedPlayerState.clientNum)
		{
			CG_AddWeaponWithPowerups(&gun, nonPredictedCent->currentState.powerups, nonPredictedCent->currentState.clientNum);
		}
	}

	// add the spinning barrel
	if(weapon->barrelModel)
	{
		memset(&barrel, 0, sizeof(barrel));
		VectorCopy(parent->lightingOrigin, barrel.lightingOrigin);
		barrel.shadowPlane = parent->shadowPlane;
		barrel.renderfx = parent->renderfx;
		barrel.hModel = weapon->barrelModel;
		angles[YAW] = 0;
		angles[PITCH] = 0;
		angles[ROLL] = CG_MachinegunSpinAngle(cent);

		AnglesToAxis(angles, barrel.axis);
		CG_PositionRotatedEntityOnTag(&barrel, &gun, weapon->weaponModel, "tag_barrel");
		CG_AddWeaponWithPowerups(&barrel, cent->currentState.powerups, cent->currentState.clientNum);
	}

	// make sure we aren't looking at cg.predictedPlayerEntity for LG
	nonPredictedCent = &cg_entities[cent->currentState.clientNum];

	// if the index of the nonPredictedCent is not the same as the clientNum
	// then this is a fake player (like on teh single player podiums), so
	// go ahead and use the cent
	if((nonPredictedCent - cg_entities) != cent->currentState.clientNum)
	{
		nonPredictedCent = cent;
	}

	if(weaponNum == WP_IRAILGUN && ps->weaponstate == WEAPON_PREFIRING)
	{
		ci = &cgs.clientinfo[cent->currentState.clientNum];

		if(charge < 0)
		{
			if(cg_QSLights.integer == 1)
			{
				trap_R_AddAdditiveLightToScene(gun.origin, brightness + (rand() & 15), ci->color1[0] * 0.2f,
											   ci->color1[1] * 0.2f, ci->color1[2] * 0.2f);
				return;
			}
		}

	}

	// add the flash
	if((weaponNum == WP_LIGHTNING || weaponNum == WP_GAUNTLET || weaponNum == WP_GRAPPLING_HOOK)
	   && (cent->currentState.eFlags & EF_FIRING))
	{
		// continuous flash
	}
	else
	{
		// impulse flash

		if(cg.time - cent->muzzleFlashTime > MUZZLE_FLASH_TIME && !cent->pe.railgunFlash)
		{
			if(weaponNum != WP_FLAMETHROWER)
			{
				return;
			}
		}
	}

	memset(&flash, 0, sizeof(flash));
//  VectorCopy( parent->lightingOrigin, flash.lightingOrigin );
//  flash.shadowPlane = parent->shadowPlane;
	flash.renderfx = parent->renderfx;

	flash.hModel = weapon->flashModel;
	if(!flash.hModel)
	{
		//  return;
	}
	angles[YAW] = 0;
	angles[PITCH] = 0;
	angles[ROLL] = crandom() * 10;
	AnglesToAxis(angles, flash.axis);

	if(weaponNum == WP_MACHINEGUN)
	{
		start = cent->muzzleFlashTime;
		end = cent->muzzleFlashTime + 60;
		c = (end - cg.time) / (float)(end - start);

		flash.shaderRGBA[0] = 255;
		flash.shaderRGBA[1] = 255;
		flash.shaderRGBA[2] = 255;
		flash.shaderRGBA[3] = 255;
	}

	CG_PositionRotatedEntityOnTag(&flash, &gun, weapon->weaponModel, "tag_flash");
	if(weaponNum != WP_IRAILGUN && weaponNum != WP_RAILGUN)
	{							//Ridah, hide the flash also for now
		trap_R_AddRefEntityToScene(&flash);
	}

	if(isPlayer)
	{
		if(!cg.renderingThirdPerson)
		{
			vec3_t          origin1;

			VectorCopy(cg.refdef[0].vieworg, origin1);
			VectorMA(origin1, 14, cg.refdef[0].viewaxis[0], origin1);
			VectorMA(origin1, -4, cg.refdef[0].viewaxis[1], origin1);
			VectorMA(origin1, -8, cg.refdef[0].viewaxis[2], origin1);
			CG_LightningBolt(&cg.predictedPlayerEntity, origin1);
		}
		else
		{
			CG_LightningBolt(&cg.predictedPlayerEntity, flash.origin);
		}

	}

	if(!isPlayer)
	{
		if(nonPredictedCent->currentState.clientNum != cg.predictedPlayerState.clientNum)
		{
			CG_LightningBolt(nonPredictedCent, flash.origin);
		}
	}

	if(ps || cg.renderingThirdPerson || !isPlayer)
	{

		CG_SpawnRailTrail(cent, flash.origin, cent->lerpAngles);

		if(cg_QSLights.integer == 1)
		{
			if(weapon->flashDlightColor[0] || weapon->flashDlightColor[1] || weapon->flashDlightColor[2])
			{
				if(weaponNum == WP_PLASMAGUN)
				{
					ci = &cgs.clientinfo[cent->currentState.clientNum];
					if(cgs.gametype >= GT_TEAM)
					{
						if(ci->team == TEAM_RED)
						{
							trap_R_AddAdditiveLightToScene(flash.origin, 110, 0.2f, 0.05f, 0);
							trap_R_AddLightToScene(flash.origin, 130, 0.9f, 0.2f, 0);
						}
						else if(ci->team == TEAM_BLUE)
						{
							trap_R_AddAdditiveLightToScene(flash.origin, 110, weapon->flashDlightColor[0],
														   weapon->flashDlightColor[1], weapon->flashDlightColor[2]);
							trap_R_AddLightToScene(flash.origin, 130, weapon->flashDlightColor2[0],
												   weapon->flashDlightColor2[1], weapon->flashDlightColor2[2]);
						}
					}
					else
					{
						trap_R_AddAdditiveLightToScene(flash.origin, 110, 0.05f, 0.2f, 0);
						trap_R_AddLightToScene(flash.origin, 130, 0.2f, 0.9f, 0);
					}
				}
				else
				{
					trap_R_AddAdditiveLightToScene(flash.origin, 110, weapon->flashDlightColor[0],
												   weapon->flashDlightColor[1], weapon->flashDlightColor[2]);
					trap_R_AddLightToScene(flash.origin, 130, weapon->flashDlightColor2[0],
										   weapon->flashDlightColor2[1], weapon->flashDlightColor2[2]);
				}

			}
		}
		else
		{
			if(weapon->flashDlightColor[0] || weapon->flashDlightColor[1] || weapon->flashDlightColor[2])
			{
				if(weaponNum == WP_PLASMAGUN)
				{
					trap_R_AddLightToScene(flash.origin, 300 + (rand() & 31), weapon->flashDlightColor[0],
										   weapon->flashDlightColor[1], weapon->flashDlightColor[2]);
				}
				else
				{
					trap_R_AddLightToScene(flash.origin, 300 + (rand() & 31), weapon->flashDlightColor[0],
										   weapon->flashDlightColor[1], weapon->flashDlightColor[2]);
				}
			}
		}

	}

	if(weaponNum != WP_FLAMETHROWER)
	{
		return;
	}

}

/*
==============
CG_AddViewWeapon

Add the weapon, and flash for the player's view
==============
*/
void CG_AddViewWeapon(playerState_t * ps)
{
	refEntity_t     hand;
	centity_t      *cent;
	clientInfo_t   *ci;
	float           fovOffset;
	vec3_t          angles;
	weaponInfo_t   *weapon;

//      vec3_t      origin;

	if(ps->persistant[PERS_TEAM] == TEAM_SPECTATOR)
	{
		return;
	}

	if(ps->pm_type == PM_INTERMISSION)
	{
		return;
	}



	// no gun if in third person view or a camera is active
	//if ( cg.renderingThirdPerson || cg.cameraMode) {
	if(cg.renderingThirdPerson)
	{
		return;
	}

	if(!cg_drawGun.integer)
	{
		return;
	}


	// don't draw if testing a gun model
	if(cg.testGun)
	{
		return;
	}

	// drop gun lower at higher fov
	if(cg_fov.integer > 90)
	{
		fovOffset = -0.2 * (cg_fov.integer - 90);
	}
	else
	{
		fovOffset = 0;
	}

	cent = &cg.predictedPlayerEntity;	// &cg_entities[cg.snap->ps.clientNum];
	CG_RegisterWeapon(ps->weapon);
	weapon = &cg_weapons[ps->weapon];

	memset(&hand, 0, sizeof(hand));

	// set up gun position
	CG_CalculateWeaponPosition(hand.origin, angles);

	if(ps->weapon != WP_IRAILGUN)
	{
		VectorMA(hand.origin, cg_gun_x.value, cg.refdef[0].viewaxis[0], hand.origin);
		VectorMA(hand.origin, cg_gun_y.value, cg.refdef[0].viewaxis[1], hand.origin);
		VectorMA(hand.origin, (cg_gun_z.value + fovOffset), cg.refdef[0].viewaxis[2], hand.origin);
	}
	else if(ps->weapon == WP_IRAILGUN)
	{
		VectorMA(hand.origin, cg_gun_x.value - 1, cg.refdef[0].viewaxis[0], hand.origin);
		VectorMA(hand.origin, cg_gun_y.value - 2, cg.refdef[0].viewaxis[1], hand.origin);
		VectorMA(hand.origin, (cg_gun_z.value - 2 + fovOffset), cg.refdef[0].viewaxis[2], hand.origin);
	}
	if(ps->weapon != WP_FLAMETHROWER)
	{
		AnglesToAxis(angles, hand.axis);
	}
	else
	{
		AnglesToAxis(cent->lerpAngles, hand.axis);
	}

	// map torso animations to weapon animations
	if(cg_gun_frame.integer)
	{
		// development tool
		hand.frame = hand.oldframe = cg_gun_frame.integer;
		hand.backlerp = 0;
	}
	else
	{
		// get clientinfo for animation map
		ci = &cgs.clientinfo[cent->currentState.clientNum];
		hand.frame = CG_MapTorsoToWeaponFrame(ci, cent->pe.torso.frame);
		hand.oldframe = CG_MapTorsoToWeaponFrame(ci, cent->pe.torso.oldFrame);
		hand.backlerp = cent->pe.torso.backlerp;
	}

	hand.hModel = weapon->handsModel;
	hand.renderfx = /*RF_DEPTHHACK | */ RF_FIRST_PERSON;

	// add everything onto the hand
	CG_AddPlayerWeapon(&hand, ps, &cg.predictedPlayerEntity, ps->persistant[PERS_TEAM]);
}

/*
==============================================================================

WEAPON SELECTION

==============================================================================
*/
#define BLINKMASK3 0x000007FF	// 2047

void CG_DrawWeaponSelectModel(float x, float y, float w, float h, weaponInfo_t * weapon)
{
	refdef_t        refdef;
	refEntity_t     ent;
	vec3_t          angles;
	vec3_t          origin;

	if(!cg_draw3dIcons.integer || !cg_drawIcons.integer)
	{
		return;
	}

	CG_AdjustFrom640(&x, &y, &w, &h);
	VectorClear(angles);
	memset(&refdef, 0, sizeof(refdef));

	memset(&ent, 0, sizeof(ent));

	refdef.rdflags = RDF_NOWORLDMODEL;

	AxisClear(refdef.viewaxis);

	refdef.fov_x = 30;
	refdef.fov_y = 30;

	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;

	refdef.time = cg.time;

	trap_R_ClearScene();

	origin[0] = 90;
	origin[1] = 0;
	origin[2] = -10;
	angles[YAW] = (cg.time & 2047) * 360 / 2048.0;
	AnglesToAxis(angles, ent.axis);
	origin[0] -=
		weapon->weaponMidpoint[0] * ent.axis[0][0] +
		weapon->weaponMidpoint[1] * ent.axis[1][0] + weapon->weaponMidpoint[2] * ent.axis[2][0];
	origin[1] -=
		weapon->weaponMidpoint[0] * ent.axis[0][1] +
		weapon->weaponMidpoint[1] * ent.axis[1][1] + weapon->weaponMidpoint[2] * ent.axis[2][1];
	origin[2] -=
		weapon->weaponMidpoint[0] * ent.axis[0][2] +
		weapon->weaponMidpoint[1] * ent.axis[1][2] + weapon->weaponMidpoint[2] * ent.axis[2][2];
	origin[2] += 8;				// an extra height boost

	VectorCopy(origin, ent.origin);
	ent.hModel = weapon->weaponModel;
	ent.renderfx = RF_NOSHADOW | RF_MINLIGHT;	// no stencil shadows
	if(weapon->barrelModel)
	{
		refEntity_t     barrel;

		memset(&barrel, 0, sizeof(barrel));

		barrel.hModel = weapon->barrelModel;

		barrel.renderfx = RF_NOSHADOW | RF_MINLIGHT;

		CG_PositionRotatedEntityOnTag(&barrel, &ent, weapon->weaponModel, "tag_barrel");

		AxisCopy(ent.axis, barrel.axis);
		trap_R_AddRefEntityToScene(&barrel);
	}
	trap_R_AddRefEntityToScene(&ent);
	trap_R_RenderScene(&refdef);
}

static void CG_DrawField2(int x, int y, int width, int value, qboolean Dead)
{
	char            num[16], *ptr;
	int             l;
	int             frame;

	if(width < 1)
	{
		return;
	}

	// draw number string
	if(width > 5)
	{
		width = 5;
	}

	switch (width)
	{
		case 1:
			value = value > 9 ? 9 : value;
			value = value < 0 ? 0 : value;
			break;
		case 2:
			value = value > 99 ? 99 : value;
			value = value < -9 ? -9 : value;
			break;
		case 3:
			value = value > 999 ? 999 : value;
			value = value < -99 ? -99 : value;
			break;
		case 4:
			value = value > 9999 ? 9999 : value;
			value = value < -999 ? -999 : value;
			break;
	}

	Com_sprintf(num, sizeof(num), "%i", value);
	l = strlen(num);
	if(l > width)
		l = width;
	x += 11 * (width - l);

	ptr = num;
	while(*ptr && l)
	{
		if(*ptr == '-')
			frame = STAT_MINUS;
		else
			frame = *ptr - '0';
		if(!Dead && value != 999)
		{
			CG_DrawPic(x, y, 11, CHAR_HEIGHT / 2.15f, cgs.media.digitalnumShaders[frame]);
		}
		else if(!Dead && value == 999)
		{
			CG_DrawPic(x, y, 11, CHAR_HEIGHT / 2.15f, cgs.media.digitalnumShaders[STAT_MINUS]);
		}
		else
		{
			CG_DrawPic(x, y, 11, CHAR_HEIGHT / 2.15f, cgs.media.deaddigitalShader);
		}
		x += 11;
		ptr++;
		l--;
	}
}

/*
===================
CG_DrawWeaponSelect
===================
*/
void CG_DrawWeaponSelect(void)
{
	int             i;
	int             bits;
	int             count;
	float           t;
	int             x, y;
	float          *fadeColor;
	vec4_t          color1;
	float           fade;
	char           *ammo;
	char           *ammo1;
	int             value;
	weaponInfo_t   *weapon;


	if(WeaponSelStyle.integer == 1)
	{
		weapon = &cg_weapons[cg.snap->ps.weapon];
		CG_FillRect(0, 39 - 37, 130, 41, colorBlack);
		CG_FillRect(3, 51 - 37, 80, 30, colorDkGrey);
		CG_DrawWeaponSelectModel(3, 45 - 37, 80, 34, weapon);
		ammo = va("^a^x000000%s", weapon->item->pickup_name);
		CH_DrawStringExt(9, 39 - 37, ammo, 5, 9, colorWhite, 0, qfalse);
		CG_DrawField2(87, 54 - 37, 3, 888, qtrue);
		CG_DrawField2(87, 54 - 37, 3, cg.snap->ps.ammo[cg.snap->ps.weapon], qfalse);
		CG_DrawPic(0, 0, 178, 46, cgs.media.weaponZselect);
		cg.itemPickupTime = 1;
	}
	if(con_notifytime.integer > -1)
	{
		trap_Cvar_Set("con_notifytime", "-1");
	}

	if(WeaponSelStyle.integer == 0)
	{
		/*37 */
		fadeColor = colorWhite;
		fade = *fadeColor;
		// don't display if dead
		if(cg.snap->ps.stats[STAT_HEALTH] <= 0)
		{
			return;
		}

		if(cg.selectflash > 650)
		{
			cg.selectflash = 650;
		}



		if(cg.selectflash < 0)
		{
			cg.selectflash = 0;
		}

		// showing weapon select clears pickup item display, but not the blend blob
		cg.itemPickupTime = 1;
		x = 6;
		y = 40;
		// count the number of weapons owned
		bits = cg.snap->ps.stats[STAT_WEAPONS];
		count = 0;
		for(i = 1; i < 16; i++)
		{
			if(bits & (1 << i))
			{
				y -= 1;
				count++;
			}
		}

		y += count;
		for(i = 1; i < 16; i++)
		{
			if(!(bits & (1 << i)))
			{
				continue;
			}


		}
		 /**/ color1[0] = 0.8f;
		color1[1] = 0.8f;
		color1[2] = 0.8f;

		color1[3] = 0.5f;

		if(!color1)
		{
			return;
		}
		trap_R_SetColor(color1);


		for(i = 1; i < 16; i++)
		{
			if(!(bits & (1 << i)))
			{
				continue;
			}
			CG_RegisterWeapon(i);
			ammo = va("^a^x000000%i^N", cg.snap->ps.ammo[i]);
			ammo1 = va("^a^x000000^1%i^N", cg.snap->ps.ammo[i]);


			CG_FillRect(0, 36, 78, 1, colorWhite);

			// draw selection marker

			if(i == cg.weaponSelect && cg.selectflash <= 0)
			{
				y += 3;

				CG_FillRect(0, y - 8, 78, 28, color1);
				trap_R_SetColor(NULL);
				CG_DrawPic(x, y - 8, 28, 28, cg_weapons[i].weaponIcon);
				if(cg.snap->ps.ammo[i] != 999)
				{
					trap_R_SetColor(NULL);



					if(cg.snap->ps.ammo[i] <= 9)
					{
						CH_DrawStringExt(x + 26, y, ammo1, 15, 14, colorWhite, 0, qfalse);
					}
					else
					{
						CH_DrawStringExt(x + 26, y, ammo, 15, 14, colorWhite, 0, qfalse);
					}
				}
				if(!cg.snap->ps.ammo[i])
				{
					CG_DrawPic(x, y - 4, 28, 28, cgs.media.noammoShader);
				}
			}
			else
			{
				y += 1;
				// draw weapon icon
				CG_DrawPic(x, y, 16, 16, cg_weapons[i].weaponIcon);

				if(cg.snap->ps.ammo[i] != 999)
				{
					if(cg.snap->ps.ammo[i] <= 9)
					{
						CH_DrawStringExt(x + 28, y + 7, ammo1, 6, 7, colorWhite, 0, qfalse);
					}
					else
					{
						CH_DrawStringExt(x + 28, y + 7, ammo, 6, 7, colorWhite, 0, qfalse);
					}
				}
				if(!cg.snap->ps.ammo[i])
				{
					CG_DrawPic(x, y, 12, 12, cgs.media.noammoShader);
				}

				if(i == cg.weaponSelect2 && cg.selectflash > 0)
				{
					if(cg.frametime < 200)
					{
						cg.lastselectflash = cg.selectflash;
						cg.selectflash -= cg.frametime;
					}
					color1[0] = 0.6f;
					color1[1] = 0.9f;
					color1[2] = 0.2f;
					t = cg.time & BLINKMASK3;

					if(t > 256)
						t = -t + 512;
					color1[3] = ((float)t) / 128;
					trap_R_SetColor(color1);

					CG_FillRect(x, y - 2, 72, 20, color1);
					trap_R_SetColor(NULL);
				}

			}
			y += 20;

		}

		CG_FillRect(0, y, 78, 1, colorWhite);
		value = cg.snap->ps.stats[STAT_HOLDABLE_ITEM];
		if(value)
		{
			CG_RegisterItemVisuals(value);
			CG_DrawPic(x, y + 4, 16, 16, cg_items[value].icon);
			CG_FillRect(0, y + 20, 78, 1, colorWhite);
		}


		trap_R_SetColor(NULL);
	}
}


/*
===============
CG_WeaponSelectable
===============
*/
static qboolean CG_WeaponSelectable(int i)
{
	if(!cg.snap->ps.ammo[i])
	{
		return qfalse;
	}
	if(!(cg.snap->ps.stats[STAT_WEAPONS] & (1 << i)))
	{
		return qfalse;
	}

	return qtrue;
}


/*
===============
CG_NextWeapon_f
===============
*/
void CG_NextWeapon_f(void)
{
	int             i;
	int             original;
	playerState_t  *ps;

	ps = &cg.predictedPlayerState;

	if(!cg.snap)
	{
		return;
	}

	if(cg.VFlags & VFL_ZOOM && cg.predictedPlayerState.weapon == WP_RAILGUN ||
	   cg.VFlags & VFL_ZOOM && cg.predictedPlayerState.weapon == WP_IRAILGUN)
	{

		if(cg.zoomFov > 20)
		{
			cg.zoomFov += 10;
		}
		else
		{
			cg.zoomFov += 5;
		}
		if(cg.zoomFov > 60)
		{
			cg.zoomFov = 60;
		}
		return;
	}

	if(cg.snap->ps.pm_flags & PMF_FOLLOW)
	{
		if(ps->pm_type != PM_INTERMISSION)
		{
			trap_SendConsoleCommand("follownext");
		}
		return;
	}

	cg.weaponSelectTime = cg.time;
	original = cg.weaponSelect;

	for(i = 0; i < 16; i++)
	{
		cg.weaponSelect++;
		if(cg.weaponSelect == 16)
		{
			cg.weaponSelect = 0;
		}
		if(CG_WeaponSelectable(cg.weaponSelect))
		{
			break;
		}
	}
	if(i == 16)
	{
		cg.weaponSelect = original;
	}

	if(cg.VFlags & VFL_ZOOM)
	{
		cg.VFlags ^= VFL_ZOOM;
	}

}

/*
===============
CG_PrevWeapon_f
===============
*/
void CG_PrevWeapon_f(void)
{
	int             i;
	int             original;
	playerState_t  *ps;

	ps = &cg.predictedPlayerState;

	if(!cg.snap)
	{
		return;
	}

	if(cg.VFlags & VFL_ZOOM && cg.predictedPlayerState.weapon == WP_RAILGUN ||
	   cg.VFlags & VFL_ZOOM && cg.predictedPlayerState.weapon == WP_IRAILGUN)
	{
		if(cg.zoomFov > 20)
		{
			cg.zoomFov -= 10;
		}
		else
		{
			cg.zoomFov -= 5;
		}
		if(cg.zoomFov < 5)
		{
			cg.zoomFov = 5;
		}
		return;
	}

	if(cg.snap->ps.pm_flags & PMF_FOLLOW)
	{
		if(ps->pm_type != PM_INTERMISSION)
		{
			trap_SendConsoleCommand("followprev");
		}
		return;
	}

	cg.weaponSelectTime = cg.time;
	original = cg.weaponSelect;

	for(i = 0; i < 16; i++)
	{
		cg.weaponSelect--;
		if(cg.weaponSelect == -1)
		{
			cg.weaponSelect = 15;
		}
		if(CG_WeaponSelectable(cg.weaponSelect))
		{
			break;
		}
	}
	if(i == 16)
	{
		cg.weaponSelect = original;
	}
	if(cg.VFlags & VFL_ZOOM)
	{
		cg.VFlags ^= VFL_ZOOM;
	}
}

/*
===============
CG_Weapon_f
===============
*/
void CG_Weapon_f(void)
{
	int             num;

	if(!cg.snap)
	{
		return;
	}
	if(cg.snap->ps.pm_flags & PMF_FOLLOW)
	{
		return;
	}

	num = atoi(CG_Argv(1));

	if(num < 1 || num > 15)
	{
		return;
	}

	cg.weaponSelectTime = cg.time;

	if(!(cg.snap->ps.stats[STAT_WEAPONS] & (1 << num)))
	{
		return;					// don't have the weapon
	}

	if(cg.VFlags & VFL_ZOOM)
	{
		cg.VFlags ^= VFL_ZOOM;
	}
	cg.weaponSelect = num;
}

/*
===================
CG_OutOfAmmoChange

The current weapon has just run out of ammo
===================
*/
void CG_OutOfAmmoChange(void)
{
	int             i;

	cg.weaponSelectTime = cg.time;

	for(i = 15; i > 0; i--)
	{
		if(CG_WeaponSelectable(i))
		{
			cg.weaponSelect = i;
			break;
		}
	}
}

/*
====================================================================================

  Explosions

====================================================================================
*/

static void CG_BlastWave(vec3_t origin, vec3_t dir, int duration, float startRadius,
						 float endRadius, vec3_t color, qboolean sprite, qhandle_t hShader)
{
	localEntity_t  *le;
	refEntity_t    *re;
	vec3_t          angles, forward, right, up;


	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_RAILDISC;
	le->entNumber = le - cg_freeLocalEntities;
	le->startTime = cg.time;

	le->endTime = cg.time + duration;
	le->lifeRate = 1.0 / (le->endTime - le->startTime);
	le->pos.trTime = cg.time;
	le->pos.trType = TR_STATIONARY;
	re->shaderTime = cg.time / 1000.0f;
	re->customShader = hShader;
	re->radius = startRadius;
	le->endradius = endRadius;
	VectorCopy(color, le->color);
	VectorCopy(origin, le->pos.trBase);
	VectorCopy(origin, re->origin);
	if(sprite)
	{
		re->sparklesT = 1;
		AxisCopy(cg.refdef[0].viewaxis, re->axis);
	}
	else
	{
		vectoangles(dir, angles);
		AngleVectors(angles, forward, right, up);
		VectorCopy(forward, re->axis[0]);
		VectorCopy(right, re->axis[1]);
		VectorCopy(up, re->axis[2]);
	}
}

/*
====================================================================================

  Particles

====================================================================================
*/

/*
=================
CG_AddBulletParticles
=================
*/
static void CG_AddBulletParticles(vec3_t origin, vec3_t dir, int count, int duration,
								  int speed, int trailtime, qhandle_t hShader, int step, float startsize, float endsize)
{
	localEntity_t  *le;
	refEntity_t    *re;
	vec3_t          velocity;
	int             i;
	float           randScale;

	if(Distance(cg.refdef[0].vieworg, origin) < 16)
	{
		return;					// Don't draw if close
	}
	if(Distance(cg.refdef[0].vieworg, origin) > 2000)
	{
		return;					// Don't draw if so far we cant see it anyways
	}

	// add the sparks
	for(i = 0; i < count; i++)
	{

		le = CG_AllocLocalEntity();
		re = &le->refEntity;

		speed = speed + rand() % 20;
		duration = duration + rand() % 20;
		randScale = 1.0;

		le->leType = LE_ADDSPARK;
		le->entNumber = le - cg_freeLocalEntities;
		velocity[0] = dir[0] + crandom() * randScale;
		velocity[1] = dir[1] + crandom() * randScale;
		velocity[2] = dir[2] + crandom() * randScale;
		VectorScale(velocity, (float)speed, velocity);

		le->startTime = cg.time;
		le->endTime = le->startTime + duration;
		le->lifeRate = 1.0 / (le->endTime - le->startTime);
		le->hShader = hShader;
		le->trailLength = trailtime;
		le->step = step;
		le->startSize = startsize;
		le->endSize = endsize;

		le->pos.trType = TR_GRAVITY;
		le->pos.trTime = cg.time;

		VectorCopy(origin, re->origin);
		VectorCopy(origin, le->pos.trBase);
		VectorMA(le->pos.trBase, 1 + random() * 2, dir, le->pos.trBase);
		VectorCopy(velocity, le->pos.trDelta);
	}
}

/*
=================
CG_AddRocketParticles
=================
*/
static void CG_AddRocketParticles(vec3_t origin, vec3_t dir, int count, int duration,
								  int speed, int trailtime, qhandle_t hShader, int step, float startsize, float endsize)
{
	localEntity_t  *le;
	refEntity_t    *re;
	vec3_t          velocity;
	int             i;
	float           randScale;

	if(Distance(cg.refdef[0].vieworg, origin) < 32)
	{
		return;					// Don't draw if close
	}
	if(Distance(cg.refdef[0].vieworg, origin) > 1800)
	{
		return;					// Don't draw if so far we cant see it anyways
	}

	// add the sparks
	for(i = 0; i < count; i++)
	{

		le = CG_AllocLocalEntity();
		re = &le->refEntity;

		speed = speed + rand() % 20;
		duration = duration + rand() % 20;
		randScale = 1.0;

		le->leType = LE_ADDROCKETSPARK;
		le->entNumber = le - cg_freeLocalEntities;
		velocity[0] = dir[0] + crandom() * randScale;
		velocity[1] = dir[1] + crandom() * randScale;
		velocity[2] = dir[2] + crandom() * randScale;
		VectorScale(velocity, (float)speed, velocity);

		le->startTime = cg.time;
		le->endTime = le->startTime + duration;
		le->lifeRate = 1.0 / (le->endTime - le->startTime);
		le->hShader = hShader;
		le->trailLength = trailtime;
		le->step = step;
		le->startSize = startsize;
		le->endSize = endsize;

		le->pos.trType = TR_GRAVITY_LOW;
		le->pos.trTime = cg.time;

		VectorCopy(origin, re->origin);
		VectorCopy(origin, le->pos.trBase);
		VectorMA(le->pos.trBase, 1 + random() * 2, dir, le->pos.trBase);
		VectorCopy(velocity, le->pos.trDelta);
	}
}

/*
=================
CG_AddPlasmaParticles
=================
*/
static void CG_AddPlasmaParticles(vec3_t origin, vec3_t dir, int count, int team)
{
	int             i;

	if(Distance(cg.refdef[0].vieworg, origin) < 64)
	{
		return;					// Don't draw if close
	}
	if(Distance(cg.refdef[0].vieworg, origin) > 2000)
	{
		return;					// Don't draw if so far we cant see it anyways
	}
	for(i = 0; i < count; i++)
	{
		CG_Particle_HalfBurstSparks(cgs.media.PGPartShader, origin, dir, team);
	}
}

/*
=================
CG_AddBulletSplashParticles
=================
*/
void CG_AddBulletSplashParticles(vec3_t origin, vec3_t dir, int speed, int duration, int count, float randScale,
								 float width, float height, float alpha, char *shadername)
{
	vec3_t          velocity, pos, color;
	int             i;

	color[0] = 1.0f;
	color[1] = 1.0f;
	color[2] = 1.0f;
	color[3] = 1.0f;
	CG_BlastWave(origin, dir, duration, 2, width, color, qfalse, cgs.media.splashrippleShader);
	if(Distance(cg.refdef[0].vieworg, origin) < 98)
	{
		return;					// Don't draw if close
	}
	if(Distance(cg.refdef[0].vieworg, origin) > 1500)
	{
		return;					// Don't draw if so far we cant see it anyways
	}

	// add the big falling particle
	VectorSet(velocity, 0, 0, (float)speed);

	VectorCopy(origin, pos);

//  CG_ParticleDirtBulletDebris_Core( pos, velocity, duration, width,height, alpha, shadername);
	for(i = 0; i < count; i++)
	{
		VectorSet(velocity, dir[0] * crandom() * speed * randScale,
				  dir[1] * crandom() * speed * randScale, dir[2] * random() * speed);
		CG_ParticleDirtBulletDebris_Core(pos, velocity, duration + (rand() % (duration >> 1)), width, height, alpha, shadername);
	}

}

/*
===================================================================================================

WEAPON EVENTS

===================================================================================================
*/

/*
================
CG_FireWeapon

Caused by an EV_FIRE_WEAPON event
================
*/
void CG_FireWeapon(centity_t * cent)
{
	entityState_t  *ent;
	int             c;
	weaponInfo_t   *weap;
	int             charge;
	qboolean        firing;

	firing = ((cent->currentState.eFlags & EF_FIRING) != 0);

	ent = &cent->currentState;
	if(ent->weapon == WP_NONE)
	{
		return;
	}
	if(ent->weapon >= WP_NUM_WEAPONS)
	{
		CG_Error("CG_FireWeapon: ent->weapon >= WP_NUM_WEAPONS");
		return;
	}
	weap = &cg_weapons[ent->weapon];

	// mark the entity as muzzle flashing, so when it is added it will
	// append the flash to the weapon model
	cent->muzzleFlashTime = cg.time;

	// lightning gun only does this this on initial press
	if(ent->weapon == WP_LIGHTNING || ent->weapon == WP_FLAMETHROWER)
	{
		if(cent->pe.lightningFiring)
		{
			return;
		}
	}

	// play quad sound if needed
	if(cent->currentState.powerups & (1 << PW_QUAD))
	{
		trap_S_StartSound(NULL, cent->currentState.number, CHAN_ITEM, cgs.media.quadSound);
	}

	// play a sound
	if(ent->weapon != WP_IRAILGUN)
	{
		for(c = 0; c < 4; c++)
		{
			if(!weap->flashSound[c])
			{
				break;
			}
		}
		if(c > 0)
		{
			c = rand() % c;
			if(weap->flashSound[c])
			{
				trap_S_StartSound(NULL, ent->number, CHAN_WEAPON, weap->flashSound[c]);
			}
		}
	}
	else if(ent->weapon == WP_IRAILGUN)
	{
		charge = cg.snap->ps.stats[STAT_RAIL_CHARGE] / 70;
		if(charge >= -35)
		{
			trap_S_StartSound(NULL, ent->number, CHAN_WEAPON, weap->flashSound[2]);
		}
		else if(charge < -35 && charge >= -70)
		{
			trap_S_StartSound(NULL, ent->number, CHAN_WEAPON, weap->flashSound[0]);
		}
		else if(charge < -70)
		{
			trap_S_StartSound(NULL, ent->number, CHAN_WEAPON, weap->flashSound[1]);
		}
	}


	// do brass ejection
	if(weap->ejectBrassFunc && cg_brassTime.integer > 0)
	{
		weap->ejectBrassFunc(cent);
	}


	if(cg_delag.integer >= 1)
	{
//  CG_PredictMissleEffects( cent );
	}
	//CG_PredictWeaponEffects( cent );


}

/*
================
CG_RailgunPrefire

Caused by an EV_RAILGUN_PREFIRE event
================
*/
void CG_RailgunPrefire(centity_t * cent)
{
	entityState_t  *ent;
	weaponInfo_t   *weap;
	int             charge;
	int             charge2;
	int             brightness;
	clientInfo_t   *ci;
	vec3_t          color;


	ent = &cent->currentState;
	weap = &cg_weapons[ent->weapon];

	charge = cg.snap->ps.stats[STAT_RAIL_CHARGE] / 70;
	charge2 = (charge + charge);
	brightness = charge - charge2;

	ci = &cgs.clientinfo[ent->clientNum];

	color[0] = ci->color1[0] * 0.3f;
	color[1] = ci->color1[1] * 0.3f;
	color[2] = ci->color1[2] * 0.3f;

	if(ent->weapon == WP_IRAILGUN)
	{
		if(charge > 0)
		{
			trap_R_AddAdditiveLightToScene(ent->origin, brightness + (rand() & 15), color[0], color[1], color[2]);
		}
	}

}

/*
=================
CG_MissileHitWall

Caused by an EV_MISSILE_MISS event, or directly by local bullet tracing
=================
*/
void CG_MissileHitWall(int weapon, int clientNum, vec3_t origin, vec3_t dir, impactSound_t soundType, qboolean quad,
					   qboolean player)
{
	qhandle_t       mod;
	qhandle_t       mark;
	qhandle_t       shader;
	sfxHandle_t     sfx;
	float           radius, light, light2, z, w, x, d;
	vec3_t          lightColor, lightColor2, sprOrg, sprVel, spror, color, wcolor;
	localEntity_t  *le;
	int             r, duration;
	qboolean        alphaFade, isSprite;
	centity_t      *cent;
	clientInfo_t   *ci;

	cent = &cg_entities[clientNum];
	ci = &cgs.clientinfo[clientNum];

	mark = 0;
	radius = 0;
	sfx = 0;
	mod = 0;
	light = 0;
	light2 = 0;

	isSprite = qfalse;
	duration = 0;

	if(weapon != WP_LIGHTNING)
	{
		mark = 0;
		radius = 32;
		sfx = 0;
		mod = 0;
		shader = 0;
		light = 0;
		lightColor[0] = 0.4f;
		lightColor[1] = 0.3f;
		lightColor[2] = 0;
		light2 = 0;
		lightColor2[0] = 0.4f;
		lightColor2[1] = 0.3f;
		lightColor2[2] = 0;

		// set defaults
		isSprite = qfalse;
		duration = 600;
	}

	switch (weapon)
	{
		default:
		case WP_BFG:
		case WP_SHOTGUN:
		case WP_MACHINEGUN:
		case WP_LIGHTNING:
		case WP_FLAMETHROWER:
			break;
		case WP_GRENADE_LAUNCHER:
		case WP_ROCKET_LAUNCHER:
		case WP_RAILGUN:
		case WP_IRAILGUN:
		case WP_PLASMAGUN:
			if(weapon == WP_GRENADE_LAUNCHER)
			{
				vec3_t          top, bottom, check;
				trace_t         trace;
				trace_t         finaltrace;

				VectorCopy(origin, top);
				VectorCopy(origin, bottom);

				if(quad)
				{
					top[2] += 45;
					bottom[2] -= 45;
				}
				else
				{
					top[2] += 37;
					bottom[2] -= 37;
				}

				VectorMA(top, 3, dir, top);
				VectorMA(bottom, 3, dir, bottom);

				VectorCopy(top, check);

				check[2] += 3;
				if(CG_PointContents(check, -1) & (CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA))
				{
					break;
				}

				CG_Trace(&trace, top, NULL, NULL, bottom, -1, CONTENTS_SOLID);

				if(trace.fraction != 1.0)
				{
					CG_Trace(&finaltrace, top, NULL, NULL, trace.endpos, -1, CONTENTS_WATER);
				}
				else
				{
					CG_Trace(&finaltrace, top, NULL, NULL, bottom, -1, CONTENTS_WATER);
				}
				if(finaltrace.fraction != 1.0)
				{
					if(quad)
					{
						CG_AddBulletSplashParticles(finaltrace.endpos, finaltrace.plane.normal, 290,	// speed
													1000,	// duration
													5,	// count
													0.25f, 140, 75, 0.3f, "watersplash");
						CG_AddBulletSplashParticles(finaltrace.endpos, finaltrace.plane.normal, 290,	// speed
													1000,	// duration
													3,	// count
													0.25f, 140, 20, 0.3f, "watersplash");
						trap_S_StartSound(finaltrace.endpos, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.waterHitsmallSound);
						trap_S_StartSound(finaltrace.endpos, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.waterHitHugeSound);
					}
					else
					{
						CG_AddBulletSplashParticles(finaltrace.endpos, finaltrace.plane.normal, 265,	// speed
													1000,	// duration
													5,	// count
													0.25f, 100, 50, 0.3f, "watersplash");
						CG_AddBulletSplashParticles(finaltrace.endpos, finaltrace.plane.normal, 265,	// speed
													1000,	// duration
													3,	// count
													0.25f, 100, 20, 0.3f, "watersplash");
						trap_S_StartSound(finaltrace.endpos, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.waterHitsmallSound);
						trap_S_StartSound(finaltrace.endpos, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.waterHitHugeSound);
					}
				}
			}
			if(weapon == WP_ROCKET_LAUNCHER)
			{
				vec3_t          top, bottom, check;
				trace_t         trace;
				trace_t         finaltrace;

				VectorCopy(origin, top);
				VectorCopy(origin, bottom);

				if(quad)
				{
					top[2] += 45;
					bottom[2] -= 45;
				}
				else
				{
					top[2] += 37;
					bottom[2] -= 37;
				}

				VectorMA(top, 3, dir, top);
				VectorMA(bottom, 3, dir, bottom);

				VectorCopy(top, check);

				check[2] += 3;
				if(CG_PointContents(check, -1) & (CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA))
				{
					break;
				}

				CG_Trace(&trace, top, NULL, NULL, bottom, -1, CONTENTS_SOLID);

				if(trace.fraction != 1.0)
				{
					CG_Trace(&finaltrace, top, NULL, NULL, trace.endpos, -1, CONTENTS_WATER);
				}
				else
				{
					CG_Trace(&finaltrace, top, NULL, NULL, bottom, -1, CONTENTS_WATER);
				}
				if(finaltrace.fraction != 1.0)
				{
					if(quad)
					{
						CG_AddBulletSplashParticles(finaltrace.endpos, finaltrace.plane.normal, 290,	// speed
													1000,	// duration
													5,	// count
													0.25f, 140, 75, 0.3f, "watersplash");
						CG_AddBulletSplashParticles(finaltrace.endpos, finaltrace.plane.normal, 290,	// speed
													1000,	// duration
													3,	// count
													0.25f, 140, 20, 0.3f, "watersplash");
						trap_S_StartSound(finaltrace.endpos, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.waterHitsmallSound);
						trap_S_StartSound(finaltrace.endpos, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.waterHitHugeSound);
					}
					else
					{
						CG_AddBulletSplashParticles(finaltrace.endpos, finaltrace.plane.normal, 265,	// speed
													1000,	// duration
													5,	// count
													0.25f, 100, 50, 0.3f, "watersplash");
						CG_AddBulletSplashParticles(finaltrace.endpos, finaltrace.plane.normal, 265,	// speed
													1000,	// duration
													3,	// count
													0.25f, 100, 20, 0.3f, "watersplash");
						trap_S_StartSound(finaltrace.endpos, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.waterHitsmallSound);
						trap_S_StartSound(finaltrace.endpos, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.waterHitHugeSound);
					}
				}
			}
			if(weapon == WP_RAILGUN)
			{
				vec3_t          top, bottom, check;
				trace_t         trace;
				trace_t         finaltrace;

				VectorCopy(origin, top);
				VectorCopy(origin, bottom);

				top[2] += 8;
				bottom[2] -= 8;

				VectorMA(top, 3, dir, top);
				VectorMA(bottom, 3, dir, bottom);

				VectorCopy(top, check);

				check[2] += 3;
				if(CG_PointContents(check, -1) & (CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA))
				{
					break;
				}

				CG_Trace(&trace, top, NULL, NULL, bottom, -1, CONTENTS_SOLID);

				if(trace.fraction != 1.0)
				{
					CG_Trace(&finaltrace, top, NULL, NULL, trace.endpos, -1, CONTENTS_WATER);
				}
				else
				{
					CG_Trace(&finaltrace, top, NULL, NULL, bottom, -1, CONTENTS_WATER);
				}
				if(finaltrace.fraction != 1.0)
				{
					CG_AddBulletSplashParticles(finaltrace.endpos, finaltrace.plane.normal, 170,	// speed
												500,	// duration
												4,	// count
												0.25f, 70, 20, 0.6f, "watersplash");
					trap_S_StartSound(finaltrace.endpos, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.waterHitsmallSound);

				}
			}
			if(weapon == WP_IRAILGUN)
			{
				vec3_t          top, bottom, check;
				trace_t         trace;
				trace_t         finaltrace;

				VectorCopy(origin, top);
				VectorCopy(origin, bottom);

				top[2] += 8;
				bottom[2] -= 8;

				VectorMA(top, 3, dir, top);
				VectorMA(bottom, 3, dir, bottom);

				VectorCopy(top, check);

				check[2] += 3;
				if(CG_PointContents(check, -1) & (CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA))
				{
					break;
				}

				CG_Trace(&trace, top, NULL, NULL, bottom, -1, CONTENTS_SOLID);

				if(trace.fraction != 1.0)
				{
					CG_Trace(&finaltrace, top, NULL, NULL, trace.endpos, -1, CONTENTS_WATER);
				}
				else
				{
					CG_Trace(&finaltrace, top, NULL, NULL, bottom, -1, CONTENTS_WATER);
				}
				if(finaltrace.fraction != 1.0)
				{
					CG_AddBulletSplashParticles(finaltrace.endpos, finaltrace.plane.normal, 170,	// speed
												500,	// duration
												4,	// count
												0.25f, 70, 20, 0.6f, "watersplash");
					trap_S_StartSound(finaltrace.endpos, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.waterHitsmallSound);

				}
			}
			if(weapon == WP_PLASMAGUN)
			{
				vec3_t          top, bottom, check;
				trace_t         trace;
				trace_t         finaltrace;

				VectorCopy(origin, top);
				VectorCopy(origin, bottom);

				top[2] += 11;
				bottom[2] -= 11;

				VectorMA(top, 3, dir, top);
				VectorMA(bottom, 3, dir, bottom);

				VectorCopy(top, check);

				check[2] += 3;
				if(CG_PointContents(check, -1) & (CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA))
				{
					break;
				}

				CG_Trace(&trace, top, NULL, NULL, bottom, -1, CONTENTS_SOLID);

				if(trace.fraction != 1.0)
				{
					CG_Trace(&finaltrace, top, NULL, NULL, trace.endpos, -1, CONTENTS_WATER);
				}
				else
				{
					CG_Trace(&finaltrace, top, NULL, NULL, bottom, -1, CONTENTS_WATER);
				}
				if(finaltrace.fraction != 1.0)
				{
					CG_AddBulletSplashParticles(finaltrace.endpos, finaltrace.plane.normal, 170,	// speed
												500,	// duration
												3,	// count
												0.25f, 70, 20, 0.6f, "watersplash");
					trap_S_StartSound(finaltrace.endpos, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.waterHitsmallSound);

				}
			}

			break;
	}

	switch (weapon)
	{
		default:
		case WP_LIGHTNING:
			return;
			break;
		case WP_FLAMETHROWER:
			return;
			break;

		case WP_GRENADE_LAUNCHER:
			color[0] = 1.0f;
			color[1] = 1.0f;
			color[2] = 1.0f;
			color[3] = 1.0f;

			mod = cgs.media.dishFlashModel;
			shader = cgs.media.grenadeExplosionShader;
			sfx = cgs.media.sfx_rockexp;
			mark = cgs.media.burnMarkShader;
			radius = 64;


			if(cg_QSLights.integer == 1)
			{

				light = 300;
				lightColor[0] = 0.1f;
				lightColor[1] = 0.05f;
				lightColor[2] = 0.0f;
				light2 = 250;
				lightColor2[0] = 7.0f;
				lightColor2[1] = 5.0f;
				lightColor2[2] = 0.0f;

			}
			else
			{
				lightColor[0] = 1;
				lightColor[1] = 0.75;
				lightColor[2] = 0.0;
			}
			CG_GrenadeExplosion(dir, origin, "rlexplosion1", 800, light2, lightColor, light, lightColor2);


			VectorMA(origin, 11, dir, spror);
			if(quad)
			{
				CG_QSphereEffect(spror, dir, ci->team, 600, WP_GRENADE_LAUNCHER, quad);
			}

			CG_RocketCorona(spror, 810, 90);

			CG_QSphereEffect(spror, dir, ci->team, 700, WP_ROCKET_LAUNCHER, qfalse);
			CG_BlastWave(spror, dir, 130, 10, 130, color, qtrue, cgs.media.simpleShockWaveShader);

			isSprite = qtrue;

			if(cg_QSParticles.integer)
			{
				z = cg_GLParticles.integer;
				CG_AddRocketParticles(origin, dir,
									  /*count */ z,
									  /*speed */ 400,
									  /*duration */ 150,
									  /*trailtime */ 100,
									  /*shader */ cgs.media.tracerTrailzShader,
									  /*step */ 20,
									  /*startsize */ 3.0,
									  /*endsize */ 0);
			}
			return;
			break;
		case WP_ROCKET_LAUNCHER:
			if(cg_QSLights.integer == 1)
			{

				light = 300;
				lightColor[0] = 0.1f;
				lightColor[1] = 0.05f;
				lightColor[2] = 0.0f;
				light2 = 250;
				lightColor2[0] = 7.0f;
				lightColor2[1] = 5.0f;
				lightColor2[2] = 0.0f;

			}
			else
			{
				lightColor[0] = 1;
				lightColor[1] = 0.75;
				lightColor[2] = 0.0;
			}
			color[0] = 0.4f;
			color[1] = 0.4f;
			color[2] = 0.4f;
			color[3] = 0.1f;
			CG_BlastWave(origin, dir, 130, 10, 130, color, qtrue, cgs.media.simpleShockWaveShader);

			CG_RocketExplosion(dir, origin, "rlexplosion1", 1000, light2, lightColor, light, lightColor2);
			if(!player)
			{
				VectorMA(origin, 5, dir, spror);
			}
			else
			{
				VectorCopy(origin, spror);
			}
			if(quad)
			{
				CG_QSphereEffect(spror, dir, ci->team, 800, WP_ROCKET_LAUNCHER, quad);
			}
			VectorMA(origin, 10, dir, sprOrg);
			CG_RocketCorona(sprOrg, 1090, 90);
			//  CG_RailCorona( sprOrg ,1090,90);
			CG_RailCorona(sprOrg, 1090, 90);

			if(player)
			{
				CG_RingEffect2(spror, dir, "wave1", 700, qtrue);
			}
			else
			{
				CG_RingEffect2(spror, dir, "wave1", 700, qfalse);
			}

			if(cg_oldRocket.integer)
			{
				// explosion sprite animation
				VectorMA(origin, 24, dir, sprOrg);
				VectorScale(dir, 64, sprVel);

				CG_ParticleExplosion("explode1", sprOrg, sprVel, 1400, 20, 30);

			}

			if(!player)
			{
				VectorMA(spror, 5, dir, spror);
			}

			CG_QSphereEffect(spror, dir, ci->team, 800, WP_ROCKET_LAUNCHER, qfalse);

			if(cg_QSParticles.integer == 1)
			{
				r = rand() % 100;
				if(r < 0)
				{
					r = 0;
				}
				if(r > 100)
				{
					r = 100;
				}
				if(r >= 0 && r <= 33)
				{
					z = 3;
				}
				else if(r >= 34 && r <= 66)
				{
					z = 6;
				}
				else if(r >= 67 && r <= 100)
				{
					z = 8;
				}
				else
				{
					z = 3;
				}

				CG_AddRocketParticles(origin, dir, z, 400, 150, 100, cgs.media.tracerTrailzShader, 20, 3.0, 0);

			}

			return;
			break;
		case WP_RAILGUN:
			if(cg_QSLights.integer == 1)
			{
				lightColor[0] = 0.1f;
				lightColor[1] = 0.1f;
				lightColor[2] = 0.1f;
				lightColor2[0] = 2.0f;
				lightColor2[1] = 2.0f;
				lightColor2[2] = 2.0f;
			}
			CG_RgExplosion(dir, origin, 850, 150, lightColor, 220, lightColor2);

			VectorMA(origin, 6, dir, spror);

			CG_RailCorona(spror, 800, 100);
			CG_RailCorona(spror, 800, 100);

			//  CG_RailEXPRingEffect( spror , dir,700,ci);
			//  VectorMA( origin, 5.5f, dir, spror );
			//  CG_RailEXPRingEffect2( spror , dir,700,ci);

			color[0] = 1.0f;
			color[1] = 1.0f;
			color[2] = 1.0f;
			color[3] = 0.8f;
			VectorMA(origin, 5, dir, spror);

			CG_BlastWave(spror, dir, 400, 5, 64, color, qfalse, cgs.media.simpleShockWaveShader);

			return;
			break;
		case WP_IRAILGUN:
			mod = cgs.media.dish2FlashModel;
			shader = cgs.media.railExplosion2Shader;
			sfx = cgs.media.sfx_plasmaexp2;
			mark = cgs.media.energy21MarkShader;
			radius = 10;
			duration = 1000;
			isSprite = qtrue;
			if(cg_QSLights.integer == 1)
			{

				light = 150;
				lightColor[0] = 0.1f;
				lightColor[1] = 0.1f;
				lightColor[2] = 0.1f;
				light2 = 200;
				lightColor2[0] = 2.0f;
				lightColor2[1] = 2.0f;
				lightColor2[2] = 2.0f;
			}
			if(cg_QSParticles.integer == 1)
			{
				d = cg_RGParticles.integer;
			}
			break;

		case WP_PLASMAGUN:

			if(quad)
			{
				CG_QSphereEffect(origin, dir, ci->team, 300, WP_PLASMAGUN, quad);
			}

			CG_AddPlasmaParticles(origin, dir, 40, ci->team);

			color[0] = 1.0f;
			color[1] = 1.0f;
			color[2] = 1.0f;
			color[3] = 0.8f;
			VectorMA(origin, 5, dir, spror);
			if(ci->team == TEAM_RED)
			{
				wcolor[0] = 1.0f;
				wcolor[1] = 0.0f;
				wcolor[2] = 0.0f;
				wcolor[3] = 0.8f;
			}
			else if(ci->team == TEAM_BLUE)
			{
				wcolor[0] = 0.0f;
				wcolor[1] = 0.0f;
				wcolor[2] = 1.0f;
				wcolor[3] = 0.8f;
			}
			else
			{
				wcolor[0] = 0.0f;
				wcolor[1] = 1.0f;
				wcolor[2] = 0.0f;
				wcolor[3] = 0.8f;
			}

			CG_RailCorona(spror, 300, 20);
			CG_RailCorona(spror, 300, 20);

			CG_BlastWave(spror, dir, 250, 2, 40, color, qfalse, cgs.media.simpleShockWaveShader);
			CG_BlastWave(spror, dir, 250, 2, 40, wcolor, qfalse, cgs.media.simpleShockWaveShader);

			sfx = cgs.media.sfx_plasmaexp;
			if(cgs.gametype >= GT_TEAM)
			{
				if(ci->team == TEAM_RED)
				{
					mark = cgs.media.energyMarkrShader;
				}
				else if(ci->team == TEAM_BLUE)
				{
					mark = cgs.media.energyMarkShader;
				}
			}
			else
			{
				mark = cgs.media.energyMarkgShader;
			}
			radius = 12;

			trap_S_StartSound(origin, ENTITYNUM_WORLD, CHAN_AUTO, sfx);

			alphaFade = (mark == cgs.media.energyMarkShader);	// plasma fades alpha, all others fade color

			CG_ImpactMark(mark, origin, dir, random() * 360, 1, 1, 1, 1, alphaFade, radius, qfalse);

			return;
			break;

		case WP_BFG:
			mod = cgs.media.dishFlashModel;
			shader = cgs.media.bfgExplosionShader;
			sfx = cgs.media.sfx_rockexp;
			mark = cgs.media.burnMarkShader;
			CG_KamikazeEffect(origin);
			radius = 32;
			isSprite = qtrue;
			if(cg_QSLights.integer == 1)
			{

				light = 1000;
				lightColor[0] = 0.0f;
				lightColor[1] = 0.05f;
				lightColor[2] = 0.2f;
				light2 = 1000;
				lightColor2[0] = 0.0f;
				lightColor2[1] = 2.0f;
				lightColor2[2] = 8.0f;
			}
			break;
		case WP_SHOTGUN:
			mark = cgs.media.bulletMarkShader;
			VectorMA(origin, -15, dir, sprOrg);

			le = CG_MakeExplosion(sprOrg, dir, 0, cgs.media.bulletExplosionShader, 150, qtrue, 3);
			le->light = 0;
			le->light2 = 0;
			le->width = radius;

			sfx = 0;
			radius = 3;

			if(cg_QSParticles.integer == 1)
			{

				r = rand() % 100;
				if(r < 0)
				{
					r = 0;
				}
				if(r > 100)
				{
					r = 100;
				}
				if(r >= 0 && r <= 45)
				{
					x = 1;
				}
				else if(r >= 46 && r <= 75)
				{
					x = 0;
				}
				else if(r >= 76 && r <= 100)
				{
					x = 2;
				}

				CG_AddBulletParticles(origin, dir,
									  /*count */ x,
									  /*speed */ 230,
									  /*duration */ 160,
									  /*trailtime */ 60,
									  /*shader */ cgs.media.tracerTrailzShader,
									  /*step */ 15,
									  /*startsize */ 3.0,
									  /*endsize */ 0);
			}
			break;



		case WP_MACHINEGUN:
			//  isSprite = qtrue;
			//  duration = 200;
			//  shader = cgs.media.bulletExplosionShader;
			mark = cgs.media.bulletMarkShader;
			VectorMA(origin, -16, dir, sprOrg);
			le = CG_MakeExplosion(sprOrg, dir, 0, cgs.media.bulletExplosionShader, 150, qtrue, 3);

			//  CG_RailCorona( spror ,100,10);

			le->light = 0;
			le->light2 = 0;
			le->width = radius;

			r = rand() % 100;
			if(r < 0)
			{
				r = 0;
			}
			if(r > 100)
			{
				r = 100;
			}
			if(r >= 0 && r <= 16)
			{
				sfx = cgs.media.sfx_ric1;
			}
			else if(r >= 17 && r <= 32)
			{
				sfx = cgs.media.sfx_ric2;
			}
			else if(r >= 33 && r <= 49)
			{
				sfx = cgs.media.sfx_ric3;
			}
			else if(r >= 50 && r <= 66)
			{
				sfx = cgs.media.sfx_ric4;
			}
			else if(r >= 67 && r <= 82)
			{
				sfx = cgs.media.sfx_ric5;
			}
			else if(r >= 83 && r <= 100)
			{
				sfx = cgs.media.sfx_ric6;
			}

			radius = 3;
			if(cg_QSParticles.integer == 1)
			{

				r = rand() % 100;
				if(r < 0)
				{
					r = 0;
				}
				if(r > 100)
				{
					r = 100;
				}
				if(r >= 0 && r <= 45)
				{
					w = 1;
				}
				else if(r >= 46 && r <= 75)
				{
					w = 0;
				}
				else if(r >= 76 && r <= 100)
				{
					w = 3;
				}

				CG_AddBulletParticles(origin, dir,
									  /*count */ w,
									  /*speed */ 230,
									  /*duration */ 160,
									  /*trailtime */ 60,
									  /*shader */ cgs.media.tracerTrailzShader,
									  /*step */ 15,
									  /*startsize */ 3.0,
									  /*endsize */ 0);
			}
			break;
	}

	if(sfx)
	{
		trap_S_StartSound(origin, ENTITYNUM_WORLD, CHAN_AUTO, sfx);
	}

	//
	// create the explosion
	//

	if(mod)
	{
		if(weapon == WP_GRENADE_LAUNCHER)
		{
			VectorMA(origin, -8, dir, spror);
			le = CG_MakeExplosion(spror, dir, mod, shader, duration, isSprite, radius);
		}
		else
		{
			le = CG_MakeExplosion(origin, dir, mod, shader, duration, isSprite, radius);
		}

		le->light = light;
		le->light2 = light2;
		le->width = radius;

		VectorCopy(lightColor, le->lightColor);
		VectorCopy(lightColor2, le->lightColor2);
		if(weapon == WP_RAILGUN)
		{
			// colorize with client color
			VectorCopy(cgs.clientinfo[clientNum].color1, le->color);
		}
		if(weapon == WP_IRAILGUN)
		{
			// colorize with client color
			VectorCopy(cgs.clientinfo[clientNum].color1, le->color);
		}
	}

	//
	// impact mark
	//

	alphaFade = (mark == cgs.media.energyMarkShader);	// plasma fades alpha, all others fade color

	CG_ImpactMark(mark, origin, dir, random() * 360, 1, 1, 1, 1, alphaFade, radius, qfalse);

}

/*
=================
CG_AddBloodParticles
=================

static void CG_AddBloodParticles( vec3_t origin, vec3_t dir,int count ,int duration,
								  int speed,int trailtime
								  ,qhandle_t hShader,int step,
								  float	startsize,float	endsize,int	clientNum) {
	localEntity_t	*le;
	refEntity_t		*re;
	vec3_t	velocity;
	int	i;  
    float randScale;

	// add the blood
	for (i=0; i<count;i++) {

		le = CG_AllocLocalEntity();
		re = &le->refEntity;

		speed = speed  + rand()%20;
		duration = duration + rand()%20;
		randScale = 1.0;

		le->leType = LE_ADDSPARK;
		le->entNumber = le - cg_activeLocalEntities;
//		VectorSet( velocity, dir[0] + crandom()*randScale, dir[1] + crandom()*randScale, dir[2] + crandom()*randScale );
		velocity[0] = dir[0] + crandom()*randScale;
		velocity[1] = dir[1] + crandom()*randScale;
		velocity[2] = dir[2] + crandom()*randScale;
		VectorScale( velocity, (float)speed, velocity );

		le->startTime = cg.time;
		le->endTime = le->startTime + (float)duration;
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
		VectorMA( le->pos.trBase, 1 + random()*2, dir, le->pos.trBase );
		VectorCopy( velocity, le->pos.trDelta ); 
	}
}
*/
/*
=================
CG_MissileHitPlayer
=================
*/
void CG_MissileHitPlayer(int weapon, vec3_t origin, vec3_t dir, int entityNum, int clientNum, qboolean quad)
{
	centity_t      *cent;
	centity_t      *cent2;
	vec3_t          dir2;
	clientInfo_t   *ci;
	weaponInfo_t   *weaponm;
	int             weaponNum;

	cent = &cg_entities[entityNum];
	cent2 = &cg_entities[clientNum];
	ci = &cgs.clientinfo[clientNum];
	weaponNum = cent->currentState.weapon;
	weaponm = &cg_weapons[weaponNum];

//  CG_Particle_Bleed (cgs.media.bloodTrailShader, origin, vec3_origin, 0, 700+rand()%200,8,24);
	CG_ParticleExplosion("zblood", origin, vec3_origin, 300 + rand() % 200, 16, 32);

	VectorSubtract(cent->lerpOrigin, cent2->lerpOrigin, dir2);
	VectorNormalize(dir2);

	// some weapons will make an explosion with the blood, while
	// others will just make the blood
	switch (weapon)
	{
		case WP_GRENADE_LAUNCHER:

		case WP_ROCKET_LAUNCHER:
		case WP_BFG:

			CG_MissileHitWall(weapon, clientNum, origin, dir, IMPACTSOUND_FLESH, quad, qtrue);
			break;
		case WP_PLASMAGUN:
			CG_AddPlasmaParticles(origin, dir, 20, ci->team);
			break;
		default:
			break;
	}
}



/*
============================================================================

SHOTGUN TRACING

============================================================================
*/

/*
======================
CG_CalcMuzzlePoint
======================
*/
static qboolean CG_CalcMuzzlePoint(int entityNum, vec3_t muzzle)
{
	vec3_t          forward;
	centity_t      *cent;
	int             anim;

	if(entityNum == cg.snap->ps.clientNum)
	{
		VectorCopy(cg.snap->ps.origin, muzzle);
		muzzle[2] += cg.snap->ps.viewheight;
		AngleVectors(cg.snap->ps.viewangles, forward, NULL, NULL);
		VectorMA(muzzle, 14, forward, muzzle);
		return qtrue;
	}

	cent = &cg_entities[entityNum];
	if(!cent->currentValid)
	{
		return qfalse;
	}

	VectorCopy(cent->currentState.pos.trBase, muzzle);

	AngleVectors(cent->currentState.apos.trBase, forward, NULL, NULL);
	anim = cent->currentState.legsAnim & ~ANIM_TOGGLEBIT;
	if(anim == LEGS_WALKCR || anim == LEGS_IDLECR)
	{
		muzzle[2] += CROUCH_VIEWHEIGHT;
	}
	else
	{
		muzzle[2] += DEFAULT_VIEWHEIGHT;
	}

	VectorMA(muzzle, 14, forward, muzzle);

	return qtrue;

}

/*
================
CG_ShotgunPellet
================
*/
static void CG_ShotgunPellet(vec3_t start, vec3_t end, int skipNum)
{
	trace_t         tr;
	int             sourceContentType, destContentType;

	CG_Trace(&tr, start, NULL, NULL, end, skipNum, MASK_SHOT);

	sourceContentType = trap_CM_PointContents(start, 0);
	destContentType = trap_CM_PointContents(tr.endpos, 0);

	// FIXME: should probably move this cruft into CG_BubbleTrail
	if(sourceContentType == destContentType)
	{
		if(sourceContentType & CONTENTS_WATER)
		{
			CG_BubbleTrail(start, tr.endpos, 32);
		}
	}
	else if(sourceContentType & CONTENTS_WATER)
	{
		trace_t         trace;

		trap_CM_BoxTrace(&trace, end, start, NULL, NULL, 0, CONTENTS_WATER);
		CG_BubbleTrail(start, trace.endpos, 32);
	}
	else if(destContentType & CONTENTS_WATER)
	{
		trace_t         trace;

		trap_CM_BoxTrace(&trace, start, end, NULL, NULL, 0, CONTENTS_WATER);

		//  CG_AddPlasmaParticles(trace.endpos,trace.plane.normal,100,TEAM_FREE);
		trap_S_StartSound(trace.endpos, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.waterHitsmallSound);

		CG_AddBulletSplashParticles(trace.endpos, trace.plane.normal, 170,	// speed
									400,	// duration
									2,	// count
									0.25, 70, 12, 0.8f, "watersplash");

		CG_BubbleTrail(tr.endpos, trace.endpos, 32);
	}

	if(tr.surfaceFlags & SURF_NOIMPACT)
	{
		return;
	}

	if(cg_entities[tr.entityNum].currentState.eType == ET_PLAYER || cg_entities[tr.entityNum].currentState.eFlags & EF_DEAD)
	{
		CG_MissileHitPlayer(WP_SHOTGUN, tr.endpos, tr.plane.normal, tr.entityNum, 0, qfalse);
	}
	else
	{
		if(tr.surfaceFlags & SURF_NOIMPACT)
		{
			// SURF_NOIMPACT will not make a flame puff or a mark
			return;
		}
		if(tr.surfaceFlags & SURF_METALSTEPS)
		{
			CG_MissileHitWall(WP_SHOTGUN, 0, tr.endpos, tr.plane.normal, IMPACTSOUND_METAL, qfalse, qfalse);
		}
		else
		{
			CG_MissileHitWall(WP_SHOTGUN, 0, tr.endpos, tr.plane.normal, IMPACTSOUND_DEFAULT, qfalse, qfalse);
		}
	}
}

/*
================
CG_ShotgunPattern

Perform the same traces the server did to locate the
hit splashes
================
*/
//unlagged - attack prediction
// made this non-static for access from cg_unlagged.c
void CG_ShotgunPattern(vec3_t origin, vec3_t origin2, int seed, int otherEntNum)
{
	int             i;
	float           r, u;
	vec3_t          end;
	vec3_t          forward, right, up;

	// derive the right and up vectors from the forward vector, because
	// the client won't have any other information
	VectorNormalize2(origin2, forward);
	PerpendicularVector(right, forward);
	CrossProduct(forward, right, up);

	// generate the "random" spread pattern
	for(i = 0; i < DEFAULT_SHOTGUN_COUNT; i++)
	{
		r = Q_crandom(&seed) * DEFAULT_SHOTGUN_SPREAD * 16;
		u = Q_crandom(&seed) * DEFAULT_SHOTGUN_SPREAD * 16;
		VectorMA(origin, 8192 * 16, forward, end);
		VectorMA(end, r, right, end);
		VectorMA(end, u, up, end);

		CG_ShotgunPellet(origin, end, otherEntNum);
	}
}

/*
==============
CG_ShotgunFire
==============
*/
void CG_ShotgunFire(entityState_t * es)
{
	vec3_t          v;
	int             contents;

	VectorSubtract(es->origin2, es->pos.trBase, v);
	VectorNormalize(v);
	VectorScale(v, 32, v);
	VectorAdd(es->pos.trBase, v, v);
	if(cgs.glconfig.hardwareType != GLHW_RAGEPRO)
	{
		// ragepro can't alpha fade, so don't even bother with smoke
		vec3_t          up;

		contents = trap_CM_PointContents(es->pos.trBase, 0);
		if(!(contents & CONTENTS_WATER))
		{
			VectorSet(up, 0, 0, 8);
			CG_SmokePuff(v, up, 32, 1, 1, 1, 0.33f, 900, cg.time, 0, LEF_PUFF_DONT_SCALE, cgs.media.shotgunSmokePuffShader);
		}
	}
	CG_ShotgunPattern(es->pos.trBase, es->origin2, es->eventParm, es->otherEntityNum);
}

/*
============================================================================

BULLETS

============================================================================
*/


/*
===============
CG_Tracer
===============
*/
void CG_Tracer(vec3_t source, vec3_t dest)
{
	vec3_t          forward, right;
	polyVert_t      verts[4];
	vec3_t          line;
	float           len, begin, end;
	vec3_t          start, finish;
	vec3_t          midpoint;

	// tracer
	VectorSubtract(dest, source, forward);
	len = VectorNormalize(forward);

	// start at least a little ways from the muzzle
	if(len < 100)
	{
		return;
	}
	begin = 50 + random() * (len - 60);
	end = begin + cg_tracerLength.value;
	if(end > len)
	{
		end = len;
	}
	VectorMA(source, begin, forward, start);
	VectorMA(source, end, forward, finish);

	line[0] = DotProduct(forward, cg.refdef[0].viewaxis[1]);
	line[1] = DotProduct(forward, cg.refdef[0].viewaxis[2]);

	VectorScale(cg.refdef[0].viewaxis[1], line[1], right);
	VectorMA(right, -line[0], cg.refdef[0].viewaxis[2], right);
	VectorNormalize(right);

	VectorMA(finish, cg_tracerWidth.value, right, verts[0].xyz);
	verts[0].st[0] = 0;
	verts[0].st[1] = 1;
	verts[0].modulate[0] = 255;
	verts[0].modulate[1] = 255;
	verts[0].modulate[2] = 255;
	verts[0].modulate[3] = 255;

	VectorMA(finish, -cg_tracerWidth.value, right, verts[1].xyz);
	verts[1].st[0] = 1;
	verts[1].st[1] = 0;
	verts[1].modulate[0] = 255;
	verts[1].modulate[1] = 255;
	verts[1].modulate[2] = 255;
	verts[1].modulate[3] = 255;

	VectorMA(start, -cg_tracerWidth.value, right, verts[2].xyz);
	verts[2].st[0] = 1;
	verts[2].st[1] = 1;
	verts[2].modulate[0] = 255;
	verts[2].modulate[1] = 255;
	verts[2].modulate[2] = 255;
	verts[2].modulate[3] = 255;

	VectorMA(start, cg_tracerWidth.value, right, verts[3].xyz);
	verts[3].st[0] = 0;
	verts[3].st[1] = 0;
	verts[3].modulate[0] = 255;
	verts[3].modulate[1] = 255;
	verts[3].modulate[2] = 255;
	verts[3].modulate[3] = 255;

	trap_R_AddPolyToScene(cgs.media.tracerShader, 4, verts);

	midpoint[0] = (start[0] + finish[0]) * 0.5;
	midpoint[1] = (start[1] + finish[1]) * 0.5;
	midpoint[2] = (start[2] + finish[2]) * 0.5;

	// add the tracer sound
	trap_S_StartSound(midpoint, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.tracerSound);

}

/*
======================
CG_Bullet

Renders bullet effects.
======================
*/
void CG_Bullet(vec3_t end, int sourceEntityNum, vec3_t normal, qboolean flesh, int fleshEntityNum)
{
	trace_t         trace;
	int             sourceContentType, destContentType;
	vec3_t          start, dir;

	// if the shooter is currently valid, calc a source point and possibly
	// do trail effects
	if(sourceEntityNum >= 0 && cg_tracerChance.value > 0)
	{
		if(CG_CalcMuzzlePoint(sourceEntityNum, start))
		{
			sourceContentType = trap_CM_PointContents(start, 0);
			destContentType = trap_CM_PointContents(end, 0);

			// do a complete bubble trail if necessary
			if((sourceContentType == destContentType) && (sourceContentType & CONTENTS_WATER))
			{
				CG_BubbleTrail(start, end, 32);
			}
			// bubble trail from water into air
			else if((sourceContentType & CONTENTS_WATER))
			{
				trap_CM_BoxTrace(&trace, end, start, NULL, NULL, 0, CONTENTS_WATER);
				CG_BubbleTrail(start, trace.endpos, 32);
			}
			// bubble trail from air into water
			else if((destContentType & CONTENTS_WATER))
			{
				trap_CM_BoxTrace(&trace, start, end, NULL, NULL, 0, CONTENTS_WATER);
				CG_BubbleTrail(trace.endpos, end, 32);
				CG_AddBulletSplashParticles(trace.endpos, trace.plane.normal, 170,	// speed
											400,	// duration
											2,	// count
											0.25, 70, 12, 0.8f, "watersplash");
				trap_S_StartSound(trace.endpos, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.waterHitsmallSound);
			}

			// draw a tracer
			//  if ( random() > cg_tracerChance.value ) {
//              CG_Tracer( start, end );
			//  }
		}
	}

	// impact splash and mark
	if(flesh)
	{
		centity_t      *cent;
		centity_t      *killer;
		clientInfo_t   *ci;
		weaponInfo_t   *weaponm;
		int             weaponNum, r, sfx;

		cent = &cg_entities[fleshEntityNum];
		killer = &cg_entities[sourceEntityNum];
		ci = &cgs.clientinfo[fleshEntityNum];
		weaponNum = cent->currentState.weapon;
		weaponm = &cg_weapons[weaponNum];

		VectorSubtract(cent->lerpOrigin, killer->lerpOrigin, dir);
		VectorNormalize(dir);
		CG_ParticleExplosion("zblood", end, vec3_origin, 300 + rand() % 200, 16, 32);

//      CG_Particle_Bleed (cgs.media.bloodTrailShader, end, vec3_origin, 0, 700+rand()%200,8,24);

		r = rand() % 100;
		if(r < 0)
		{
			r = 0;
		}
		if(r > 100)
		{
			r = 100;
		}
		if(r >= 0 && r <= 19)
		{
			sfx = cgs.media.sfx_flesh1;
		}
		else if(r >= 20 && r <= 39)
		{
			sfx = cgs.media.sfx_flesh2;
		}
		else if(r >= 40 && r <= 59)
		{
			sfx = cgs.media.sfx_flesh3;
		}
		else if(r >= 60 && r <= 79)
		{
			sfx = cgs.media.sfx_flesh4;
		}
		else
		{
			sfx = cgs.media.sfx_flesh5;
		}
		trap_S_StartSound(end, ENTITYNUM_WORLD, CHAN_AUTO, sfx);
	}
	else
	{
		CG_MissileHitWall(WP_MACHINEGUN, 0, end, normal, IMPACTSOUND_DEFAULT, qfalse, qfalse);
	}
}

/*
=================
CG_AddClientRocket
=================

void CG_AddClientRocket( vec3_t origin, qhandle_t hModel, int speed) {
	localEntity_t	*le;
	refEntity_t		*re;
	int			contents;	

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->startTime = cg.time;
	le->endTime = 999999;

	contents = CG_PointContents( origin, -1 );
	le->leType = LE_ROCKET;
	le->entNumber = le - cg_activeLocalEntities;

	le->startTime = cg.time;
	le->endTime = 80000;



	VectorCopy( origin, re->origin );
	AxisCopy( axisDefault, re->axis );
	re->hModel = hModel;

	le->angles.trType = TR_LINEAR;
	VectorCopy( origin, le->pos.trBase );
	VectorScale( origin, speed, le->pos.trDelta ); //	VectorCopy( speed, le->pos.trDelta );
	le->pos.trTime = cg.time;



//	CG_MissileHitWall( WP_ROCKET_LAUNCHER, 0, re->origin, origin, IMPACTSOUND_DEFAULT );

}
*/
