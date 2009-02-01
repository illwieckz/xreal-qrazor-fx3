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
// cg_xppm.c -- handle md5 model stuff ( instead of cg_players.c )
#include "cg_local.h"

#ifdef XPPM


/*
======================
CG_ParseCharacterFile

Read a configuration file containing body.md5mesh custom
models/players/visor/character.cfg, etc
======================
*/

static qboolean CG_ParseCharacterFile(const char *filename, clientInfo_t * ci)
{
	char           *text_p, *prev;
	int             len;
	int             i;
	char           *token;
	int             skip;
	char            text[20000];
	fileHandle_t    f;

	// load the file
	len = trap_FS_FOpenFile(filename, &f, FS_READ);
	if(len <= 0)
	{
		return qfalse;
	}
	if(len >= sizeof(text) - 1)
	{
		CG_Printf("File %s too long\n", filename);
		trap_FS_FCloseFile(f);
		return qfalse;
	}
	trap_FS_Read(text, len, f);
	text[len] = 0;
	trap_FS_FCloseFile(f);

	// parse the text
	text_p = text;
	skip = 0;					// quite the compiler warning

	ci->footsteps = FOOTSTEP_STONE;
	VectorClear(ci->headOffset);
	ci->gender = GENDER_MALE;
	ci->fixedlegs = qfalse;
	ci->fixedtorso = qfalse;
	ci->firstTorsoBoneName[0] = '\0';
	ci->lastTorsoBoneName[0] = '\0';
	ci->torsoControlBoneName[0] = '\0';
	ci->neckControlBoneName[0] = '\0';
	ci->modelScale[0] = 1;
	ci->modelScale[1] = 1;
	ci->modelScale[2] = 1;

	// read optional parameters
	while(1)
	{
		prev = text_p;			// so we can unget
		token = Com_Parse(&text_p);
		if(!token[0])
		{
			break;
		}

		if(!Q_stricmp(token, "footsteps"))
		{
			token = Com_Parse(&text_p);
			if(!token)
			{
				break;
			}
			if(!Q_stricmp(token, "default") || !Q_stricmp(token, "normal") || !Q_stricmp(token, "stone"))
			{
				ci->footsteps = FOOTSTEP_STONE;
			}
			else if(!Q_stricmp(token, "boot"))
			{
				ci->footsteps = FOOTSTEP_BOOT;
			}
			else if(!Q_stricmp(token, "flesh"))
			{
				ci->footsteps = FOOTSTEP_FLESH;
			}
			else if(!Q_stricmp(token, "mech"))
			{
				ci->footsteps = FOOTSTEP_MECH;
			}
			else if(!Q_stricmp(token, "energy"))
			{
				ci->footsteps = FOOTSTEP_ENERGY;
			}
			else
			{
				CG_Printf("Bad footsteps parm in %s: %s\n", filename, token);
			}
			continue;
		}
		else if(!Q_stricmp(token, "headoffset"))
		{
			for(i = 0; i < 3; i++)
			{
				token = Com_Parse(&text_p);
				if(!token)
				{
					break;
				}
				ci->headOffset[i] = atof(token);
			}
			continue;
		}
		else if(!Q_stricmp(token, "sex"))
		{
			token = Com_Parse(&text_p);
			if(!token)
			{
				break;
			}
			if(token[0] == 'f' || token[0] == 'F')
			{
				ci->gender = GENDER_FEMALE;
			}
			else if(token[0] == 'n' || token[0] == 'N')
			{
				ci->gender = GENDER_NEUTER;
			}
			else
			{
				ci->gender = GENDER_MALE;
			}
			continue;
		}
		else if(!Q_stricmp(token, "fixedlegs"))
		{
			ci->fixedlegs = qtrue;
			continue;
		}
		else if(!Q_stricmp(token, "fixedtorso"))
		{
			ci->fixedtorso = qtrue;
			continue;
		}
		else if(!Q_stricmp(token, "firstTorsoBoneName"))
		{
			token = Com_Parse(&text_p);
			Q_strncpyz(ci->firstTorsoBoneName, token, sizeof(ci->firstTorsoBoneName));
			continue;
		}
		else if(!Q_stricmp(token, "lastTorsoBoneName"))
		{
			token = Com_Parse(&text_p);
			Q_strncpyz(ci->lastTorsoBoneName, token, sizeof(ci->lastTorsoBoneName));
			continue;
		}
		else if(!Q_stricmp(token, "torsoControlBoneName"))
		{
			token = Com_Parse(&text_p);
			Q_strncpyz(ci->torsoControlBoneName, token, sizeof(ci->torsoControlBoneName));
			continue;
		}
		else if(!Q_stricmp(token, "neckControlBoneName"))
		{
			token = Com_Parse(&text_p);
			Q_strncpyz(ci->neckControlBoneName, token, sizeof(ci->neckControlBoneName));
			continue;
		}
		else if(!Q_stricmp(token, "modelScale"))
		{
			for(i = 0; i < 3; i++)
			{
				token = Com_ParseExt(&text_p, qfalse);
				if(!token)
				{
					break;
				}
				ci->modelScale[i] = atof(token);
			}
			continue;
		}

		Com_Printf("unknown token '%s' is %s\n", token, filename);
	}

	return qtrue;
}

static qboolean CG_RegisterPlayerAnimation(clientInfo_t * ci, const char *modelName, int anim, const char *animName,
										   qboolean loop, qboolean reversed, qboolean clearOrigin)
{
	char            filename[MAX_QPATH];
	int             frameRate;

	Com_sprintf(filename, sizeof(filename), "models/players/%s/%s.md5anim", modelName, animName);
	ci->animations[anim].handle = trap_R_RegisterAnimation(filename);
	if(!ci->animations[anim].handle)
	{
		Com_Printf("Failed to load animation file %s\n", filename);
		return qfalse;
	}

	ci->animations[anim].firstFrame = 0;
	ci->animations[anim].numFrames = trap_R_AnimNumFrames(ci->animations[anim].handle);
	frameRate = trap_R_AnimFrameRate(ci->animations[anim].handle);

	if(frameRate == 0)
	{
		frameRate = 1;
	}
	ci->animations[anim].frameTime = 1000 / frameRate;
	ci->animations[anim].initialLerp = 1000 / frameRate;

	if(loop)
	{
		ci->animations[anim].loopFrames = ci->animations[anim].numFrames;
	}
	else
	{
		ci->animations[anim].loopFrames = 0;
	}

	ci->animations[anim].reversed = reversed;
	ci->animations[anim].clearOrigin = clearOrigin;

	return qtrue;
}


/*
==========================
CG_XPPM_RegisterClientModel
==========================
*/
qboolean CG_XPPM_RegisterClientModel(clientInfo_t * ci, const char *modelName, const char *skinName,
									 const char *headModelName, const char *headSkinName, const char *teamName)
{
	int             i;
	char            filename[MAX_QPATH * 2];
	const char     *headName;

	if(headModelName[0] == '\0')
	{
		headName = modelName;
	}
	else
	{
		headName = headModelName;
	}

	Com_sprintf(filename, sizeof(filename), "models/players/%s/body.md5mesh", modelName);
	ci->bodyModel = trap_R_RegisterModel(filename, qfalse);

	if(!ci->bodyModel)
	{
		Com_Printf("Failed to load body mesh file  %s\n", filename);
		return qfalse;
	}


	if(ci->bodyModel)
	{
		// load the animations
		Com_sprintf(filename, sizeof(filename), "models/players/%s/character.cfg", modelName);
		if(!CG_ParseCharacterFile(filename, ci))
		{
			Com_Printf("Failed to load character file %s\n", filename);
			return qfalse;
		}


		if(!CG_RegisterPlayerAnimation(ci, modelName, LEGS_IDLE, "idle", qtrue, qfalse, qfalse))
		{
			Com_Printf("Failed to load idle animation file %s\n", filename);
			return qfalse;
		}

		// make LEGS_IDLE the default animation
		for(i = 0; i < MAX_ANIMATIONS; i++)
		{
			if(i == LEGS_IDLE)
				continue;

			ci->animations[i] = ci->animations[LEGS_IDLE];
		}

		// FIXME fail register of the entire model if one of these animations is missing

		// FIXME add death animations

		CG_RegisterPlayerAnimation(ci, modelName, BOTH_DEATH1, "die", qfalse, qfalse, qfalse);
		//CG_RegisterPlayerAnimation(ci, modelName, BOTH_DEATH2, "death2", qfalse, qfalse, qfalse);
		//CG_RegisterPlayerAnimation(ci, modelName, BOTH_DEATH3, "death3", qfalse, qfalse, qfalse);

		if(!CG_RegisterPlayerAnimation(ci, modelName, TORSO_GESTURE, "gesture", qfalse, qfalse, qfalse))
			ci->animations[TORSO_GESTURE] = ci->animations[LEGS_IDLE];

		if(!CG_RegisterPlayerAnimation(ci, modelName, TORSO_ATTACK, "attack", qfalse, qfalse, qfalse))
			ci->animations[TORSO_ATTACK] = ci->animations[LEGS_IDLE];

		if(!CG_RegisterPlayerAnimation(ci, modelName, TORSO_ATTACK2, "idle", qfalse, qfalse, qfalse))
			ci->animations[TORSO_ATTACK2] = ci->animations[LEGS_IDLE];

		if(!CG_RegisterPlayerAnimation(ci, modelName, TORSO_STAND2, "idle", qfalse, qfalse, qfalse))
			ci->animations[TORSO_STAND2] = ci->animations[LEGS_IDLE];

		if(!CG_RegisterPlayerAnimation(ci, modelName, LEGS_IDLECR, "crouch", qtrue, qfalse, qfalse))
			ci->animations[LEGS_IDLECR] = ci->animations[LEGS_IDLE];

		if(!CG_RegisterPlayerAnimation(ci, modelName, LEGS_WALKCR, "crouch_forward", qtrue, qfalse, qtrue))
			ci->animations[LEGS_WALKCR] = ci->animations[LEGS_IDLE];

		if(!CG_RegisterPlayerAnimation(ci, modelName, LEGS_BACKCR, "crouch_forward", qtrue, qtrue, qtrue))
			ci->animations[LEGS_BACKCR] = ci->animations[LEGS_IDLE];

		if(!CG_RegisterPlayerAnimation(ci, modelName, LEGS_WALK, "walk", qtrue, qfalse, qtrue))
			ci->animations[LEGS_WALK] = ci->animations[LEGS_IDLE];

		if(!CG_RegisterPlayerAnimation(ci, modelName, LEGS_BACKWALK, "walk_backwards", qtrue, qfalse, qtrue))
			ci->animations[LEGS_BACKWALK] = ci->animations[LEGS_IDLE];

		if(!CG_RegisterPlayerAnimation(ci, modelName, LEGS_RUN, "run", qtrue, qfalse, qtrue))
			ci->animations[LEGS_RUN] = ci->animations[LEGS_IDLE];

		if(!CG_RegisterPlayerAnimation(ci, modelName, LEGS_BACK, "run_backwards", qtrue, qtrue, qtrue))
			ci->animations[LEGS_BACK] = ci->animations[LEGS_IDLE];

		if(!CG_RegisterPlayerAnimation(ci, modelName, LEGS_SWIM, "swim", qtrue, qfalse, qtrue))
			ci->animations[LEGS_SWIM] = ci->animations[LEGS_IDLE];

		if(!CG_RegisterPlayerAnimation(ci, modelName, LEGS_JUMP, "jump", qfalse, qfalse, qfalse))
			ci->animations[LEGS_JUMP] = ci->animations[LEGS_IDLE];

		if(!CG_RegisterPlayerAnimation(ci, modelName, LEGS_LAND, "land", qfalse, qfalse, qfalse))
			ci->animations[LEGS_LAND] = ci->animations[LEGS_IDLE];

		if(!CG_RegisterPlayerAnimation(ci, modelName, LEGS_JUMPB, "jump", qfalse, qtrue, qfalse))
			ci->animations[LEGS_JUMPB] = ci->animations[LEGS_IDLE];

		if(!CG_RegisterPlayerAnimation(ci, modelName, LEGS_LANDB, "land", qfalse, qtrue, qfalse))
			ci->animations[LEGS_LANDB] = ci->animations[LEGS_IDLE];

		if(!CG_RegisterPlayerAnimation(ci, modelName, LEGS_TURN, "step", qfalse, qfalse, qfalse))
			ci->animations[LEGS_TURN] = ci->animations[LEGS_IDLE];


		if(CG_FindClientModelFile(filename, sizeof(filename), ci, teamName, modelName, skinName, "body", "skin"))
		{
			Com_Printf("loading skin %s\n", filename);

			ci->bodySkin = trap_R_RegisterSkin(filename);
		}
		if(!ci->bodySkin)
		{
			Com_Printf("Body skin load failure: %s\n", filename);
			return qfalse;
		}
	}

	if(CG_FindClientHeadFile(filename, sizeof(filename), ci, teamName, headName, headSkinName, "icon", "skin"))
	{
		ci->modelIcon = trap_R_RegisterShaderNoMip(filename);
	}
	else if(CG_FindClientHeadFile(filename, sizeof(filename), ci, teamName, headName, headSkinName, "icon", "png"))
	{
		ci->modelIcon = trap_R_RegisterShaderNoMip(filename);
	}

	if(!ci->modelIcon)
	{
		Com_Printf("Failed to load icon file %s\n", filename);
		return qfalse;
	}

	return qtrue;
}



/*
======================
CG_XPPM_CopyClientInfoModel

copy of CG_CopyClientInfoModel
======================
*/
void CG_XPPM_CopyClientInfoModel(clientInfo_t * from, clientInfo_t * to)
{
	VectorCopy(from->headOffset, to->headOffset);
	to->footsteps = from->footsteps;
	to->gender = from->gender;

	Q_strncpyz(to->firstTorsoBoneName, from->firstTorsoBoneName, sizeof(to->firstTorsoBoneName));
	Q_strncpyz(to->lastTorsoBoneName, from->lastTorsoBoneName, sizeof(to->lastTorsoBoneName));

	Q_strncpyz(to->torsoControlBoneName, from->torsoControlBoneName, sizeof(to->torsoControlBoneName));
	Q_strncpyz(to->neckControlBoneName, from->neckControlBoneName, sizeof(to->neckControlBoneName));

	VectorCopy(from->modelScale, to->modelScale);

	to->bodyModel = from->bodyModel;
	to->bodySkin = from->bodySkin;

	to->legsModel = from->legsModel;
	to->legsAnimation = from->legsAnimation;
	to->legsSkin = from->legsSkin;

	to->torsoModel = from->torsoModel;
	to->torsoAnimation = from->torsoAnimation;
	to->torsoSkin = from->torsoSkin;

	to->headModel = from->headModel;
	to->headSkin = from->headSkin;

	to->modelIcon = from->modelIcon;

	to->newAnims = from->newAnims;

	memcpy(to->animations, from->animations, sizeof(to->animations));
	memcpy(to->sounds, from->sounds, sizeof(to->sounds));
}




/*
===============
CG_XPPM_PlayerAngles

copy of CG_PlayerAngles
===============
*/
static void CG_XPPM_PlayerAngles(centity_t * cent, const vec3_t sourceAngles, vec3_t legsAngles, vec3_t torsoAngles, vec3_t headAngles)
{
	float           dest;
	static int      movementOffsets[8] = { 0, 22, 45, -22, 0, 22, -45, -22 };
	vec3_t          velocity;
	float           speed;
	int             dir, clientNum;
	clientInfo_t   *ci;

	VectorCopy(sourceAngles, headAngles);
	headAngles[YAW] = AngleMod(headAngles[YAW]);
	VectorClear(legsAngles);
	VectorClear(torsoAngles);

	// --------- yaw -------------

	// allow yaw to drift a bit
	if((cent->currentState.legsAnim & ~ANIM_TOGGLEBIT) != LEGS_IDLE
	   || (cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT) != TORSO_STAND)
	{
		// if not standing still, always point all in the same direction
		cent->pe.torso.yawing = qtrue;	// always center
		cent->pe.torso.pitching = qtrue;	// always center
		cent->pe.legs.yawing = qtrue;	// always center
	}

	// adjust legs for movement dir
	if(cent->currentState.eFlags & EF_DEAD)
	{
		// don't let dead bodies twitch
		dir = 0;
	}
	else
	{
		// TA: did use angles2.. now uses time2.. looks a bit funny but time2 isn't used othwise
		dir = cent->currentState.time2;
		if(dir < 0 || dir > 7)
		{
			CG_Error("Bad player movement angle");
		}
	}
	legsAngles[YAW] = headAngles[YAW] + movementOffsets[dir];
	torsoAngles[YAW] = headAngles[YAW] + 0.25 * movementOffsets[dir];

	// torso
	CG_SwingAngles(torsoAngles[YAW], 25, 90, cg_swingSpeed.value, &cent->pe.torso.yawAngle, &cent->pe.torso.yawing);
	CG_SwingAngles(legsAngles[YAW], 40, 90, cg_swingSpeed.value, &cent->pe.legs.yawAngle, &cent->pe.legs.yawing);

	torsoAngles[YAW] = cent->pe.torso.yawAngle;
	legsAngles[YAW] = cent->pe.legs.yawAngle;


	// --------- pitch -------------

	// only show a fraction of the pitch angle in the torso
	if(headAngles[PITCH] > 180)
	{
		dest = (-360 + headAngles[PITCH]) * 0.75f;
	}
	else
	{
		dest = headAngles[PITCH] * 0.75f;
	}
	CG_SwingAngles(dest, 15, 30, 0.1f, &cent->pe.torso.pitchAngle, &cent->pe.torso.pitching);
	torsoAngles[PITCH] = cent->pe.torso.pitchAngle;

	//
	clientNum = cent->currentState.clientNum;
	if(clientNum >= 0 && clientNum < MAX_CLIENTS)
	{
		ci = &cgs.clientinfo[clientNum];
		if(ci->fixedtorso)
		{
			torsoAngles[PITCH] = 0.0f;
		}
	}

	// --------- roll -------------


	// lean towards the direction of travel
/*	VectorCopy(cent->currentState.pos.trDelta, velocity);
	speed = VectorNormalize(velocity);
	if(speed)
	{
		vec3_t          axis[3];
		float           side;

		speed *= 0.02f;

		AnglesToAxis(legsAngles, axis);
		side = speed * DotProduct(velocity, axis[1]);
		legsAngles[ROLL] -= side;

		side = speed * DotProduct(velocity, axis[0]);
		legsAngles[PITCH] += side;
	}
*/
	//
	clientNum = cent->currentState.clientNum;
	if(clientNum >= 0 && clientNum < MAX_CLIENTS)
	{
		ci = &cgs.clientinfo[clientNum];
		if(ci->fixedlegs)
		{
			legsAngles[YAW] = torsoAngles[YAW];
			legsAngles[PITCH] = 0.0f;
			legsAngles[ROLL] = 0.0f;
		}
	}

	// pain twitch
	CG_AddPainTwitch(cent, torsoAngles);

	// pull the angles back out of the hierarchial chain
	AnglesSubtract(headAngles, torsoAngles, headAngles);
	AnglesSubtract(torsoAngles, legsAngles, torsoAngles);
}


/*
===============
CG_XPPM_PlayerWWSmoothing

Smooth the angles of transitioning wall walkers
===============
*/
#define MODEL_WWSMOOTHTIME 200
static void CG_XPPM_PlayerWWSmoothing(centity_t * cent, vec3_t in[3], vec3_t out[3])
{
	entityState_t  *es = &cent->currentState;
	int             i;
	vec3_t          surfNormal, rotAxis, temp;
	vec3_t          refNormal = { 0.0f, 0.0f, 1.0f };
	vec3_t          ceilingNormal = { 0.0f, 0.0f, -1.0f };
	float           stLocal, sFraction, rotAngle;
	vec3_t          inAxis[3], lastAxis[3], outAxis[3];

	//set surfNormal
	if(!(es->eFlags & EF_WALLCLIMB))
		VectorCopy(refNormal, surfNormal);
	else if(!(es->eFlags & EF_WALLCLIMBCEILING))
		VectorCopy(es->angles2, surfNormal);
	else
		VectorCopy(ceilingNormal, surfNormal);

	AxisCopy(in, inAxis);

	if(!VectorCompare(surfNormal, cent->pe.lastNormal))
	{
		//if we moving from the ceiling to the floor special case
		//( x product of colinear vectors is undefined)
		if(VectorCompare(ceilingNormal, cent->pe.lastNormal) && VectorCompare(refNormal, surfNormal))
		{
			VectorCopy(in[1], rotAxis);
			rotAngle = 180.0f;
		}
		else
		{
			AxisCopy(cent->pe.lastAxis, lastAxis);
			rotAngle = DotProduct(inAxis[0], lastAxis[0]) +
				DotProduct(inAxis[1], lastAxis[1]) + DotProduct(inAxis[2], lastAxis[2]);

			rotAngle = RAD2DEG(acos((rotAngle - 1.0f) / 2.0f));

			CrossProduct(lastAxis[0], inAxis[0], temp);
			VectorCopy(temp, rotAxis);
			CrossProduct(lastAxis[1], inAxis[1], temp);
			VectorAdd(rotAxis, temp, rotAxis);
			CrossProduct(lastAxis[2], inAxis[2], temp);
			VectorAdd(rotAxis, temp, rotAxis);

			VectorNormalize(rotAxis);
		}

		//iterate through smooth array
		for(i = 0; i < MAXSMOOTHS; i++)
		{
			//found an unused index in the smooth array
			if(cent->pe.sList[i].time + MODEL_WWSMOOTHTIME < cg.time)
			{
				//copy to array and stop
				VectorCopy(rotAxis, cent->pe.sList[i].rotAxis);
				cent->pe.sList[i].rotAngle = rotAngle;
				cent->pe.sList[i].time = cg.time;
				break;
			}
		}
	}

	//iterate through ops
	for(i = MAXSMOOTHS - 1; i >= 0; i--)
	{
		//if this op has time remaining, perform it
		if(cg.time < cent->pe.sList[i].time + MODEL_WWSMOOTHTIME)
		{
			stLocal = 1.0f - (((cent->pe.sList[i].time + MODEL_WWSMOOTHTIME) - cg.time) / MODEL_WWSMOOTHTIME);
			sFraction = -(cos(stLocal * M_PI) + 1.0f) / 2.0f;

			RotatePointAroundVector(outAxis[0], cent->pe.sList[i].rotAxis, inAxis[0], sFraction * cent->pe.sList[i].rotAngle);
			RotatePointAroundVector(outAxis[1], cent->pe.sList[i].rotAxis, inAxis[1], sFraction * cent->pe.sList[i].rotAngle);
			RotatePointAroundVector(outAxis[2], cent->pe.sList[i].rotAxis, inAxis[2], sFraction * cent->pe.sList[i].rotAngle);

			AxisCopy(outAxis, inAxis);
		}
	}

	//outAxis has been copied to inAxis
	AxisCopy(inAxis, out);
}

/*
===============
CG_XPPM_SetLerpFrameAnimation

may include ANIM_TOGGLEBIT
===============
*/
static void CG_XPPM_SetLerpFrameAnimation(clientInfo_t * ci, lerpFrame_t * lf, int newAnimation)
{
	animation_t    *anim;

	//save old animation

	lf->old_animationNumber = lf->animationNumber;
	lf->old_animation = lf->animation;

	lf->animationNumber = newAnimation;
	newAnimation &= ~ANIM_TOGGLEBIT;

	if(newAnimation < 0 || newAnimation >= MAX_TOTALANIMATIONS)
	{
		CG_Error("bad player animation number: %i", newAnimation);
	}

	anim = &ci->animations[newAnimation];

	lf->animation = anim;
	lf->animationTime = lf->frameTime + anim->initialLerp;

	if(cg_debugPlayerAnim.integer)
	{
		CG_Printf("player anim: %i\n", newAnimation);
	}


	debug_anim_current = lf->animationNumber;
	debug_anim_old = lf->old_animationNumber;

	if(lf->old_animationNumber <= 0)
	{							// skip initial / invalid blending
		lf->blendlerp = 0.0f;
		return;
	}

	//TODO: blend through two blendings!

	if((lf->blendlerp <= 0.0f))
		lf->blendlerp = 1.0f;
	else
		lf->blendlerp = 1.0f - lf->blendlerp;	// use old blending for smooth blending between two blended animations

	memcpy(&lf->oldSkeleton, &lf->skeleton, sizeof(refSkeleton_t));

	//Com_Printf("new: %i old %i\n", newAnimation,lf->old_animationNumber);

	if(!trap_R_BuildSkeleton
	   (&lf->oldSkeleton, lf->old_animation->handle, lf->oldFrame, lf->frame, lf->blendlerp, lf->old_animation->clearOrigin))
	{
		CG_Printf("Can't blend skeleton\n");
		return;
	}


}

//TODO: choose propper values and use blending speed from character.cfg
// blending is slow for testing issues

static void CG_XPPM_BlendLerpFrame(lerpFrame_t * lf)
{

	if(cg_animBlend.value <= 0.0f)
	{
		lf->blendlerp = 0.0f;
		return;
	}

	if((lf->blendlerp > 0.0f) && (cg.time > lf->blendtime))
	{

#if 0
		//linear blending
		lf->blendlerp -= 0.025f;
#else
		//exp blending
		lf->blendlerp -= lf->blendlerp / cg_animBlend.value;
#endif
		if(lf->blendlerp <= 0.0f)
			lf->blendlerp = 0.0f;
		if(lf->blendlerp >= 1.0f)
			lf->blendlerp = 1.0f;

		lf->blendtime = cg.time + 10;

		debug_anim_blend = lf->blendlerp;
	}

}



/*
===============
CG_XPPM_RunLerpFrame

copy of CG_RunLerpFrame
===============
*/
static void CG_XPPM_RunLerpFrame(clientInfo_t * ci, lerpFrame_t * lf, int newAnimation, float speedScale)
{
	int             f, numFrames;
	animation_t    *anim;
	qboolean        animChanged;

	// debugging tool to get no animations
	if(cg_animSpeed.integer == 0)
	{
		lf->oldFrame = lf->frame = lf->backlerp = 0;
		return;
	}

	// see if the animation sequence is switching
	if(newAnimation != lf->animationNumber || !lf->animation)
	{
		CG_XPPM_SetLerpFrameAnimation(ci, lf, newAnimation);

		if(!lf->animation)
		{
			memcpy(&lf->oldSkeleton, &lf->skeleton, sizeof(refSkeleton_t));
		}

		animChanged = qtrue;


	}
	else
	{
		animChanged = qfalse;
	}


	// if we have passed the current frame, move it to
	// oldFrame and calculate a new frame
	if(cg.time >= lf->frameTime || animChanged)
	{

		if(animChanged)
		{
			lf->oldFrame = 0;
			lf->oldFrameTime = cg.time;
		}
		else

		{
			lf->oldFrame = lf->frame;
			lf->oldFrameTime = lf->frameTime;
		}

		// get the next frame based on the animation
		anim = lf->animation;
		if(!anim->frameTime)
		{
			return;				// shouldn't happen
		}

		if(cg.time < lf->animationTime)
		{
			lf->frameTime = lf->animationTime;	// initial lerp
		}
		else
		{
			lf->frameTime = lf->oldFrameTime + anim->frameTime;
		}
		f = (lf->frameTime - lf->animationTime) / anim->frameTime;
		f *= speedScale;		// adjust for haste, etc

		numFrames = anim->numFrames;

		if(anim->flipflop)
		{
			numFrames *= 2;
		}

		if(f >= numFrames)
		{
			f -= numFrames;

			if(anim->loopFrames)
			{
				f %= anim->loopFrames;
				f += anim->numFrames - anim->loopFrames;
			}
			else
			{
				f = numFrames - 1;
				// the animation is stuck at the end, so it
				// can immediately transition to another sequence
				lf->frameTime = cg.time;
			}
		}

		if(anim->reversed)
		{
			lf->frame = anim->firstFrame + anim->numFrames - 1 - f;
		}
		else if(anim->flipflop && f >= anim->numFrames)
		{
			lf->frame = anim->firstFrame + anim->numFrames - 1 - (f % anim->numFrames);
		}
		else
		{
			lf->frame = anim->firstFrame + f;
		}

		if(cg.time > lf->frameTime)
		{
			lf->frameTime = cg.time;
			if(cg_debugPlayerAnim.integer)
			{
				CG_Printf("clamp player lf->frameTime\n");
			}
		}
	}

	if(lf->frameTime > cg.time + 200)
	{
		lf->frameTime = cg.time;
	}

	if(lf->oldFrameTime > cg.time)
	{
		lf->oldFrameTime = cg.time;
	}

	// calculate current lerp value
	if(lf->frameTime == lf->oldFrameTime)
	{
		lf->backlerp = 0;
	}
	else
	{
		lf->backlerp = 1.0 - (float)(cg.time - lf->oldFrameTime) / (lf->frameTime - lf->oldFrameTime);
	}

	//blend old and current animation
	CG_XPPM_BlendLerpFrame(lf);

	if(!trap_R_BuildSkeleton
	   (&lf->skeleton, lf->animation->handle, lf->oldFrame, lf->frame, 1.0 - lf->backlerp, lf->animation->clearOrigin))
	{
		CG_Printf("Can't build lf->skeleton\n");
	}

	// lerp between old and new animation if possible
	if(lf->blendlerp > 0.0f)
	{
		if(!trap_R_BlendSkeleton(&lf->skeleton, &lf->oldSkeleton, lf->blendlerp))
		{
			CG_Printf("Can't blend\n");
			return;
		}
	}

}

/*
===============
CG_XPPM_PlayerAnimation
===============
*/

static void CG_XPPM_PlayerAnimation(centity_t * cent)
{
	clientInfo_t   *ci;
	int             clientNum;
	float           speedScale;

	clientNum = cent->currentState.clientNum;

	if(cg_noPlayerAnims.integer)
	{
		return;
	}

	if(cent->currentState.powerups & (1 << PW_HASTE))
	{
		speedScale = 1.5;
	}
	else
	{
		speedScale = 1;
	}

	ci = &cgs.clientinfo[clientNum];

	// do the shuffle turn frames locally
	if(cent->pe.legs.yawing && (cent->currentState.legsAnim & ~ANIM_TOGGLEBIT) == LEGS_IDLE)
	{
		CG_XPPM_RunLerpFrame(ci, &cent->pe.legs, LEGS_TURN, speedScale);
	}
	else
	{
		CG_XPPM_RunLerpFrame(ci, &cent->pe.legs, cent->currentState.legsAnim, speedScale);
	}

	CG_XPPM_RunLerpFrame(ci, &cent->pe.torso, cent->currentState.torsoAnim, speedScale);
}





/*
===============
CG_XPPM_Player
===============
*/

// has to be in sync with clientRespawnTime
#define DEATHANIM_TIME 1650
#define TRACE_DEPTH 32.0f
void CG_XPPM_Player(centity_t * cent)
{
	clientInfo_t   *ci;
	refEntity_t     body;
	int             clientNum;
	int             renderfx;
	qboolean        shadow;
	float           shadowPlane;
	int             noShadowID;

	vec3_t          angles;
	vec3_t          legsAngles;
	vec3_t          torsoAngles;
	vec3_t          headAngles;

	matrix_t		bodyRotation;
	quat_t          torsoQuat;
	quat_t          headQuat;

	vec3_t          tempAxis[3], tempAxis2[3];

	int             i;
	int             boneIndex;
	int             firstTorsoBone;
	int             lastTorsoBone;

	vec3_t          surfNormal = { 0.0f, 0.0f, 1.0f };
	vec3_t			playerOrigin;

	// the client number is stored in clientNum.  It can't be derived
	// from the entity number, because a single client may have
	// multiple corpses on the level using the same clientinfo
	clientNum = cent->currentState.clientNum;
	if(clientNum < 0 || clientNum >= MAX_CLIENTS)
	{
		CG_Error("Bad clientNum on player entity");
	}
	ci = &cgs.clientinfo[clientNum];

	// it is possible to see corpses from disconnected players that may
	// not have valid clientinfo
	if(!ci->infoValid)
	{
		CG_Printf("Bad clientInfo for player %i\n", clientNum);
		return;
	}

	// get the player model information
	renderfx = 0;
	if(cent->currentState.number == cg.snap->ps.clientNum)
	{
		if(!cg.renderingThirdPerson)
		{
			renderfx = RF_THIRD_PERSON;	// only draw in mirrors
		}
		else
		{
			if(cg_cameraMode.integer)
			{
				return;
			}
		}
	}


	memset(&body, 0, sizeof(body));

	// generate a new unique noShadowID to avoid that the lights of the quad damage
	// will cause bad player shadows
	noShadowID = CG_UniqueNoShadowID();
	body.noShadowID = noShadowID;

	AxisClear(body.axis);

	// get the rotation information
	VectorCopy(cent->lerpAngles, angles);
	AnglesToAxis(cent->lerpAngles, tempAxis);

	// rotate lerpAngles to floor
	if(cent->currentState.eFlags & EF_WALLCLIMB && BG_RotateAxis(cent->currentState.angles2, tempAxis, tempAxis2, qtrue, cent->currentState.eFlags & EF_WALLCLIMBCEILING))
		AxisToAngles(tempAxis2, angles);
	else
		VectorCopy(cent->lerpAngles, angles);

	// normalize the pitch
	if(angles[PITCH] < -180.0f)
		angles[PITCH] += 360.0f;

	CG_XPPM_PlayerAngles(cent, angles, legsAngles, torsoAngles, headAngles);
	AnglesToAxis(legsAngles, body.axis);

	AxisCopy(body.axis, tempAxis);

	// rotate the legs axis to back to the wall
	if(cent->currentState.eFlags & EF_WALLCLIMB && BG_RotateAxis(cent->currentState.angles2, body.axis, tempAxis, qfalse, cent->currentState.eFlags & EF_WALLCLIMBCEILING))
		AxisCopy(tempAxis, body.axis);

	// smooth out WW transitions so the model doesn't hop around
	CG_XPPM_PlayerWWSmoothing(cent, body.axis, body.axis);

	AxisCopy(tempAxis, cent->pe.lastAxis);

	// get the animation state (after rotation, to allow feet shuffle)
	CG_XPPM_PlayerAnimation(cent);

	// WIP: death effect
#if 1
	if(cent->currentState.eFlags & EF_DEAD)
	{
		int             time;

		if(cent->pe.deathTime <= 0)
		{
			cent->pe.deathTime = cg.time;
			cent->pe.deathScale = 0.0f;
		}

		time = (DEATHANIM_TIME - (cg.time - cent->pe.deathTime));

		cent->pe.deathScale = 1.0f - (1.0f / DEATHANIM_TIME * time);
		if(cent->pe.deathScale >= 1.0f)
				return;

		body.shaderTime = -cent->pe.deathScale;
	}
	else
#endif
	{
		cent->pe.deathTime = 0;
		cent->pe.deathScale = 0.0f;

		body.shaderTime = 0.0f;
	}

	// add the talk baloon or disconnect icon
	CG_PlayerSprites(cent);

	// add the shadow
	shadow = CG_PlayerShadow(cent, &shadowPlane, noShadowID);

	// add a water splash if partially in and out of water
	CG_PlayerSplash(cent);

	if(cg_shadows.integer == 2 && shadow)
	{
		renderfx |= RF_SHADOW_PLANE;
	}
	renderfx |= RF_LIGHTING_ORIGIN;	// use the same origin for all
#ifdef MISSIONPACK
	if(cgs.gametype == GT_HARVESTER)
	{
		CG_PlayerTokens(cent, renderfx);
	}
#endif

	// add the body
	body.hModel = ci->bodyModel;
	body.customSkin = ci->bodySkin;

	if(!body.hModel)
	{
		CG_Printf("No body model for player %i\n", clientNum);
		return;
	}

	body.shadowPlane = shadowPlane;
	body.renderfx = renderfx;

	// move the origin closer into the wall with a CapTrace
#if 1
	if(cent->currentState.eFlags & EF_WALLCLIMB && !(cent->currentState.eFlags & EF_DEAD) && !(cg.intermissionStarted))
	{
		vec3_t          start, end, mins, maxs;
		trace_t         tr;

		if(cent->currentState.eFlags & EF_WALLCLIMBCEILING)
			VectorSet(surfNormal, 0.0f, 0.0f, -1.0f);
		else
			VectorCopy(cent->currentState.angles2, surfNormal);

		VectorCopy(playerMins, mins);
		VectorCopy(playerMaxs, maxs);

		VectorMA(cent->lerpOrigin, -TRACE_DEPTH, surfNormal, end);
		VectorMA(cent->lerpOrigin, 1.0f, surfNormal, start);
		CG_CapTrace(&tr, start, mins, maxs, end, cent->currentState.number, MASK_PLAYERSOLID);

		// if the trace misses completely then just use body.origin
		// apparently capsule traces are "smaller" than box traces
		if(tr.fraction != 1.0f)
		{
			VectorCopy(tr.endpos, playerOrigin);

			// MD5 player models have their model origin at (0 0 0)
			VectorMA(playerOrigin, MINS_Z, surfNormal, body.origin);
		}
		else
		{
			VectorCopy(cent->lerpOrigin, playerOrigin);

			// MD5 player models have their model origin at (0 0 0)
			VectorMA(cent->lerpOrigin, -TRACE_DEPTH, surfNormal, body.origin);
		}

		VectorCopy(body.origin, body.lightingOrigin);
		VectorCopy(body.origin, body.oldorigin);	// don't positionally lerp at all
	}
	else
#endif
	{
		VectorCopy(cent->lerpOrigin, playerOrigin);
		VectorCopy(playerOrigin, body.origin);
		body.origin[2] += MINS_Z;
	}

	VectorCopy(body.origin, body.lightingOrigin);
	VectorCopy(body.origin, body.oldorigin);	// don't positionally lerp at all

	// copy legs skeleton to have a base
	memcpy(&body.skeleton, &cent->pe.legs.skeleton, sizeof(refSkeleton_t));

	if(cent->pe.legs.skeleton.numBones != cent->pe.torso.skeleton.numBones)
	{
		CG_Error("cent->pe.legs.skeleton.numBones != cent->pe.torso.skeleton.numBones");
		return;
	}

	// combine legs and torso skeletons
#if 1
	firstTorsoBone = trap_R_BoneIndex(body.hModel, ci->firstTorsoBoneName);

	if(firstTorsoBone >= 0 && firstTorsoBone < cent->pe.torso.skeleton.numBones)
	{
		lastTorsoBone = trap_R_BoneIndex(body.hModel, ci->lastTorsoBoneName);

		if(lastTorsoBone >= 0 && lastTorsoBone < cent->pe.torso.skeleton.numBones)
		{
			// copy torso bones
			for(i = firstTorsoBone; i < lastTorsoBone; i++)
			{
				memcpy(&body.skeleton.bones[i], &cent->pe.torso.skeleton.bones[i], sizeof(refBone_t));
			}
		}

		body.skeleton.type = SK_RELATIVE;

		// update AABB
		for(i = 0; i < 3; i++)
		{
			body.skeleton.bounds[0][i] =
				cent->pe.torso.skeleton.bounds[0][i] <
				cent->pe.legs.skeleton.bounds[0][i] ? cent->pe.torso.skeleton.bounds[0][i] : cent->pe.legs.skeleton.bounds[0][i];
			body.skeleton.bounds[1][i] =
				cent->pe.torso.skeleton.bounds[1][i] >
				cent->pe.legs.skeleton.bounds[1][i] ? cent->pe.torso.skeleton.bounds[1][i] : cent->pe.legs.skeleton.bounds[1][i];
		}
	}
	else
	{
		// bad no hips found
		body.skeleton.type = SK_INVALID;
	}
#endif

	// rotate legs
#if 0
	boneIndex = trap_R_BoneIndex(body.hModel, "origin");

	if(boneIndex >= 0 && boneIndex < cent->pe.legs.skeleton.numBones)
	{
		// HACK: convert angles to bone system
		QuatFromAngles(legsQuat, legsAngles[YAW], -legsAngles[ROLL], legsAngles[PITCH]);
		QuatMultiply0(body.skeleton.bones[boneIndex].rotation, legsQuat);
	}
#endif


	// rotate torso
#if 1
	boneIndex = trap_R_BoneIndex(body.hModel, ci->torsoControlBoneName);

	if(boneIndex >= 0 && boneIndex < cent->pe.legs.skeleton.numBones)
	{
		// HACK: convert angles to bone system
		QuatFromAngles(torsoQuat, torsoAngles[YAW], torsoAngles[ROLL], -torsoAngles[PITCH]);
		QuatMultiply0(body.skeleton.bones[boneIndex].rotation, torsoQuat);
	}
#endif

	// rotate head
#if 1
	boneIndex = trap_R_BoneIndex(body.hModel, ci->neckControlBoneName);

	if(boneIndex >= 0 && boneIndex < cent->pe.legs.skeleton.numBones)
	{
		// HACK: convert angles to bone system
		QuatFromAngles(headQuat, headAngles[YAW], headAngles[ROLL], -headAngles[PITCH]);
		QuatMultiply0(body.skeleton.bones[boneIndex].rotation, headQuat);
	}
#endif

	// transform relative bones to absolute ones required for vertex skinning and tag attachments
	CG_TransformSkeleton(&body.skeleton, ci->modelScale);

	// add body to renderer
	CG_AddRefEntityWithPowerups(&body, &cent->currentState, ci->team);

	if(cent->currentState.eFlags & EF_KAMIKAZE)
	{
		refEntity_t     skull;
		refEntity_t     powerup;
		int             t;
		float           c;
		float           angle;
		vec3_t          dir, angles;


		memset(&skull, 0, sizeof(skull));

		VectorCopy(cent->lerpOrigin, skull.lightingOrigin);
		skull.shadowPlane = shadowPlane;
		skull.renderfx = renderfx;

		if(cent->currentState.eFlags & EF_DEAD)
		{
			// one skull bobbing above the dead body
			angle = ((cg.time / 7) & 255) * (M_PI * 2) / 255;
			if(angle > M_PI * 2)
				angle -= (float)M_PI *2;

			dir[0] = sin(angle) * 20;
			dir[1] = cos(angle) * 20;
			angle = ((cg.time / 4) & 255) * (M_PI * 2) / 255;
			dir[2] = 15 + sin(angle) * 8;
			VectorAdd(body.origin, dir, skull.origin);
			skull.origin[2] -= MINS_Z;

			dir[2] = 0;
			VectorCopy(dir, skull.axis[1]);
			VectorNormalize(skull.axis[1]);
			VectorSet(skull.axis[2], 0, 0, 1);
			CrossProduct(skull.axis[1], skull.axis[2], skull.axis[0]);

			skull.hModel = cgs.media.kamikazeHeadModel;
			trap_R_AddRefEntityToScene(&skull);

			skull.hModel = cgs.media.kamikazeHeadTrail;
			trap_R_AddRefEntityToScene(&skull);
		}
		else
		{
			// three skulls spinning around the player
			angle = ((cg.time / 4) & 255) * (M_PI * 2) / 255;
			dir[0] = cos(angle) * 20;
			dir[1] = sin(angle) * 20;
			dir[2] = cos(angle) * 20;
			VectorAdd(body.origin, dir, skull.origin);
			skull.origin[2] -= MINS_Z;

			angles[0] = sin(angle) * 30;
			angles[1] = (angle * 180 / M_PI) + 90;
			if(angles[1] > 360)
				angles[1] -= 360;
			angles[2] = 0;
			AnglesToAxis(angles, skull.axis);

			/*
			   dir[2] = 0;
			   VectorInverse(dir);
			   VectorCopy(dir, skull.axis[1]);
			   VectorNormalize(skull.axis[1]);
			   VectorSet(skull.axis[2], 0, 0, 1);
			   CrossProduct(skull.axis[1], skull.axis[2], skull.axis[0]);
			 */

			skull.hModel = cgs.media.kamikazeHeadModel;
			trap_R_AddRefEntityToScene(&skull);

			// flip the trail because this skull is spinning in the other direction
			VectorInverse(skull.axis[1]);
			skull.hModel = cgs.media.kamikazeHeadTrail;
			trap_R_AddRefEntityToScene(&skull);

			angle = ((cg.time / 4) & 255) * (M_PI * 2) / 255 + M_PI;
			if(angle > M_PI * 2)
				angle -= (float)M_PI *2;

			dir[0] = sin(angle) * 20;
			dir[1] = cos(angle) * 20;
			dir[2] = cos(angle) * 20;
			VectorAdd(body.origin, dir, skull.origin);
			skull.origin[2] -= MINS_Z;

			angles[0] = cos(angle - 0.5 * M_PI) * 30;
			angles[1] = 360 - (angle * 180 / M_PI);
			if(angles[1] > 360)
				angles[1] -= 360;
			angles[2] = 0;
			AnglesToAxis(angles, skull.axis);

			/*
			   dir[2] = 0;
			   VectorCopy(dir, skull.axis[1]);
			   VectorNormalize(skull.axis[1]);
			   VectorSet(skull.axis[2], 0, 0, 1);
			   CrossProduct(skull.axis[1], skull.axis[2], skull.axis[0]);
			 */

			skull.hModel = cgs.media.kamikazeHeadModel;
			trap_R_AddRefEntityToScene(&skull);

			skull.hModel = cgs.media.kamikazeHeadTrail;
			trap_R_AddRefEntityToScene(&skull);

			angle = ((cg.time / 3) & 255) * (M_PI * 2) / 255 + 0.5 * M_PI;
			if(angle > M_PI * 2)
				angle -= (float)M_PI *2;

			dir[0] = sin(angle) * 20;
			dir[1] = cos(angle) * 20;
			dir[2] = 0;
			VectorAdd(body.origin, dir, skull.origin);
			skull.origin[2] -= MINS_Z;

			VectorCopy(dir, skull.axis[1]);
			VectorNormalize(skull.axis[1]);
			VectorSet(skull.axis[2], 0, 0, 1);
			CrossProduct(skull.axis[1], skull.axis[2], skull.axis[0]);

			skull.hModel = cgs.media.kamikazeHeadModel;
			trap_R_AddRefEntityToScene(&skull);

			skull.hModel = cgs.media.kamikazeHeadTrail;
			trap_R_AddRefEntityToScene(&skull);
		}
	}

#ifdef MISSIONPACK
	if(cent->currentState.powerups & (1 << PW_GUARD))
	{
		memcpy(&powerup, &torso, sizeof(torso));
		powerup.hModel = cgs.media.guardPowerupModel;
		powerup.frame = 0;
		powerup.oldframe = 0;
		powerup.customSkin = 0;
		trap_R_AddRefEntityToScene(&powerup);
	}
	if(cent->currentState.powerups & (1 << PW_SCOUT))
	{
		memcpy(&powerup, &torso, sizeof(torso));
		powerup.hModel = cgs.media.scoutPowerupModel;
		powerup.frame = 0;
		powerup.oldframe = 0;
		powerup.customSkin = 0;
		trap_R_AddRefEntityToScene(&powerup);
	}
	if(cent->currentState.powerups & (1 << PW_DOUBLER))
	{
		memcpy(&powerup, &torso, sizeof(torso));
		powerup.hModel = cgs.media.doublerPowerupModel;
		powerup.frame = 0;
		powerup.oldframe = 0;
		powerup.customSkin = 0;
		trap_R_AddRefEntityToScene(&powerup);
	}
	if(cent->currentState.powerups & (1 << PW_AMMOREGEN))
	{
		memcpy(&powerup, &torso, sizeof(torso));
		powerup.hModel = cgs.media.ammoRegenPowerupModel;
		powerup.frame = 0;
		powerup.oldframe = 0;
		powerup.customSkin = 0;
		trap_R_AddRefEntityToScene(&powerup);
	}
	if(cent->currentState.powerups & (1 << PW_INVULNERABILITY))
	{
		if(!ci->invulnerabilityStartTime)
		{
			ci->invulnerabilityStartTime = cg.time;
		}
		ci->invulnerabilityStopTime = cg.time;
	}
	else
	{
		ci->invulnerabilityStartTime = 0;
	}
	if((cent->currentState.powerups & (1 << PW_INVULNERABILITY)) || cg.time - ci->invulnerabilityStopTime < 250)
	{

		memcpy(&powerup, &torso, sizeof(torso));
		powerup.hModel = cgs.media.invulnerabilityPowerupModel;
		powerup.customSkin = 0;
		// always draw
		powerup.renderfx &= ~RF_THIRD_PERSON;
		VectorCopy(cent->lerpOrigin, powerup.origin);

		if(cg.time - ci->invulnerabilityStartTime < 250)
		{
			c = (float)(cg.time - ci->invulnerabilityStartTime) / 250;
		}
		else if(cg.time - ci->invulnerabilityStopTime < 250)
		{
			c = (float)(250 - (cg.time - ci->invulnerabilityStopTime)) / 250;
		}
		else
		{
			c = 1;
		}
		VectorSet(powerup.axis[0], c, 0, 0);
		VectorSet(powerup.axis[1], 0, c, 0);
		VectorSet(powerup.axis[2], 0, 0, c);
		trap_R_AddRefEntityToScene(&powerup);
	}

	t = cg.time - ci->medkitUsageTime;
	if(ci->medkitUsageTime && t < 500)
	{
		memcpy(&powerup, &torso, sizeof(torso));
		powerup.hModel = cgs.media.medkitUsageModel;
		powerup.customSkin = 0;

		// always draw
		powerup.renderfx &= ~RF_THIRD_PERSON;
		VectorClear(angles);
		AnglesToAxis(angles, powerup.axis);
		VectorCopy(cent->lerpOrigin, powerup.origin);
		powerup.origin[2] += -24 + (float)t *80 / 500;

		if(t > 400)
		{
			c = (float)(t - 1000) * 0xff / 100;
			powerup.shaderRGBA[0] = 0xff - c;
			powerup.shaderRGBA[1] = 0xff - c;
			powerup.shaderRGBA[2] = 0xff - c;
			powerup.shaderRGBA[3] = 0xff - c;
		}
		else
		{
			powerup.shaderRGBA[0] = 0xff;
			powerup.shaderRGBA[1] = 0xff;
			powerup.shaderRGBA[2] = 0xff;
			powerup.shaderRGBA[3] = 0xff;
		}
		trap_R_AddRefEntityToScene(&powerup);
	}
#endif

	// add the gun / barrel / flash
	CG_AddPlayerWeapon(&body, NULL, cent, ci->team);

	// add powerups floating behind the player
	CG_PlayerPowerups(cent, &body, noShadowID);

	// add the bounding box (if cg_drawPlayerCollision is 1)
	MatrixFromVectorsFLU(bodyRotation, body.axis[0], body.axis[1], body.axis[2]);
	CG_DrawPlayerCollision(cent, playerOrigin, bodyRotation);

	VectorCopy(surfNormal, cent->pe.lastNormal);
}


#endif
