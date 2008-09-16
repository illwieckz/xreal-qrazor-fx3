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
// ui_xppm.c -- handle md5 model stuff ( instead of ui_players.c )
#include "ui_local.h"

#ifdef XPPM

/*
======================
UI_XPPMParseCharacterFile

Read a configuration file containing body.md5mesh custom
models/players/visor/character.cfg, etc
======================
*/

static qboolean UI_XPPMParseCharacterFile(const char *filename, playerInfo_t * pi)
{
	char           *text_p, *prev;
	int             len;
	int             i;
	char           *token;
	int             skip;
	char            text[20000];
	fileHandle_t    f;

	Com_Printf("UI_XPPM: Loading configuration %s\n", filename);

	// load the file
	len = trap_FS_FOpenFile(filename, &f, FS_READ);
	if(len <= 0)
	{
		return qfalse;
	}
	if(len >= sizeof(text) - 1)
	{
		Com_Printf("File %s too long\n", filename);
		trap_FS_FCloseFile(f);
		return qfalse;
	}

	trap_FS_Read(text, len, f);
	text[len] = 0;
	trap_FS_FCloseFile(f);

	// parse the text
	text_p = text;
	skip = 0;					// quite the compiler warning


	pi->gender = GENDER_MALE;
	pi->firstTorsoBoneName[0] = '\0';
	pi->lastTorsoBoneName[0] = '\0';
	pi->torsoControlBoneName[0] = '\0';
	pi->neckControlBoneName[0] = '\0';
	pi->modelScale[0] = 1;
	pi->modelScale[1] = 1;
	pi->modelScale[2] = 1;

	// read optional parameters
	while(1)
	{
		prev = text_p;			// so we can unget
		token = Com_Parse(&text_p);
		if(!token[0])
		{
			break;
		}

		if(!Q_stricmp(token, "sex"))
		{
			token = Com_Parse(&text_p);
			if(!token)
			{
				break;
			}
			if(token[0] == 'f' || token[0] == 'F')
			{
				pi->gender = GENDER_FEMALE;
			}
			else if(token[0] == 'n' || token[0] == 'N')
			{
				pi->gender = GENDER_NEUTER;
			}
			else
			{
				pi->gender = GENDER_MALE;
			}
			continue;
		}
		else if(!Q_stricmp(token, "firstTorsoBoneName"))
		{
			token = Com_Parse(&text_p);
			Q_strncpyz(pi->firstTorsoBoneName, token, sizeof(pi->firstTorsoBoneName));
			continue;
		}
		else if(!Q_stricmp(token, "lastTorsoBoneName"))
		{
			token = Com_Parse(&text_p);
			Q_strncpyz(pi->lastTorsoBoneName, token, sizeof(pi->lastTorsoBoneName));
			continue;
		}
		else if(!Q_stricmp(token, "torsoControlBoneName"))
		{
			token = Com_Parse(&text_p);
			Q_strncpyz(pi->torsoControlBoneName, token, sizeof(pi->torsoControlBoneName));
			continue;
		}
		else if(!Q_stricmp(token, "neckControlBoneName"))
		{
			token = Com_Parse(&text_p);
			Q_strncpyz(pi->neckControlBoneName, token, sizeof(pi->neckControlBoneName));
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
				pi->modelScale[i] = atof(token);
			}
			continue;
		}

		//Com_Printf("unknown token '%s' is %s\n", token, filename);
	}

	return qtrue;
}


static qboolean UI_XPPM_RegisterPlayerAnimation(playerInfo_t * pi, const char *modelName, int anim, const char *animName,
										   qboolean loop, qboolean reversed, qboolean clearOrigin)
{
	char            filename[MAX_QPATH];
	int             frameRate;


	Com_sprintf(filename, sizeof(filename), "models/players/%s/%s.md5anim", modelName, animName);

	Com_Printf("UI_XPPM: Loading animation %s\n", filename);

	pi->animations[anim].handle = trap_R_RegisterAnimation(filename);


	if(!pi->animations[anim].handle)
	{
		Com_Printf("Failed to load animation file %s\n", filename);
		return qfalse;
	}

	pi->animations[anim].firstFrame = 0;
	pi->animations[anim].numFrames = trap_R_AnimNumFrames(pi->animations[anim].handle);
	frameRate = trap_R_AnimFrameRate(pi->animations[anim].handle);

	if(frameRate == 0)
	{
		frameRate = 1;
	}
	pi->animations[anim].frameTime = 1000 / frameRate;
	pi->animations[anim].initialLerp = 1000 / frameRate;

	if(loop)
	{
		pi->animations[anim].loopFrames = pi->animations[anim].numFrames;
	}
	else
	{
		pi->animations[anim].loopFrames = 0;
	}

	pi->animations[anim].reversed = reversed;
	pi->animations[anim].clearOrigin = clearOrigin;

	return qtrue;
}



/*
==========================
qbooleanUI_XPPM_RegisterModel
==========================
*/
qboolean UI_XPPM_RegisterModel(playerInfo_t * pi, const char *modelName, const char *skinName)
{
	int             i;
	char            filename[MAX_QPATH * 2];

	Com_sprintf(filename, sizeof(filename), "models/players/%s/body.md5mesh", modelName);

	Com_Printf("UI_XPPM: Loading model %s\n", filename);
	
	pi->bodyModel = trap_R_RegisterModel(filename, qfalse);

	if(!pi->bodyModel)
	{
		Com_Printf("Failed to load body mesh file  %s\n", filename);
		return qfalse;
	}


	if(pi->bodyModel)
	{
		// load the animations
		Com_sprintf(filename, sizeof(filename), "models/players/%s/character.cfg", modelName);
		if(!UI_XPPMParseCharacterFile(filename, pi))
		{
			Com_Printf("Failed to load character file %s\n", filename);
			return qfalse;
		}


		if(!UI_XPPM_RegisterPlayerAnimation(pi, modelName, LEGS_IDLE, "idle", qtrue, qfalse, qfalse))
		{
			Com_Printf("Failed to load idle animation file %s\n", filename);
			return qfalse;
		}

		// make LEGS_IDLE the default animation
		for(i = 0; i < MAX_ANIMATIONS; i++)
		{
			if(i == LEGS_IDLE)
				continue;

			pi->animations[i] = pi->animations[LEGS_IDLE];
		}

		// FIXME we don't have death animations with player models created for Quake 4
		UI_XPPM_RegisterPlayerAnimation(pi, modelName, BOTH_DEATH1, "die", qfalse, qfalse, qfalse);
		//UI_XPPM_RegisterPlayerAnimation(pi, modelName, BOTH_DEATH2, "die", qfalse, qfalse, qfalse);
		//UI_XPPM_RegisterPlayerAnimation(pi, modelName, BOTH_DEATH3, "die", qfalse, qfalse, qfalse);

		UI_XPPM_RegisterPlayerAnimation(pi, modelName, TORSO_GESTURE, "idle", qfalse, qfalse, qfalse);

		UI_XPPM_RegisterPlayerAnimation(pi, modelName, TORSO_ATTACK, "attack", qfalse, qfalse, qfalse);
		UI_XPPM_RegisterPlayerAnimation(pi, modelName, TORSO_ATTACK2, "idle", qfalse, qfalse, qfalse);

		UI_XPPM_RegisterPlayerAnimation(pi, modelName, TORSO_STAND2, "idle", qfalse, qfalse, qfalse);

		UI_XPPM_RegisterPlayerAnimation(pi, modelName, LEGS_IDLECR, "crouch", qtrue, qfalse, qfalse);
		UI_XPPM_RegisterPlayerAnimation(pi, modelName, LEGS_WALKCR, "crouch_forward", qtrue, qfalse, qtrue);
		UI_XPPM_RegisterPlayerAnimation(pi, modelName, LEGS_BACKCR, "crouch_forward", qtrue, qtrue, qtrue);

		UI_XPPM_RegisterPlayerAnimation(pi, modelName, LEGS_WALK, "walk", qtrue, qfalse, qtrue);
		UI_XPPM_RegisterPlayerAnimation(pi, modelName, LEGS_BACKWALK, "walk_backwards", qtrue, qfalse, qtrue);

		UI_XPPM_RegisterPlayerAnimation(pi, modelName, LEGS_RUN, "run", qtrue, qfalse, qtrue);
		UI_XPPM_RegisterPlayerAnimation(pi, modelName, LEGS_BACK, "run_backwards", qtrue, qtrue, qtrue);

		// FIXME UI_XPPM_RegisterPlayerAnimation(pi, modelName, LEGS_SWIM, "swim", qtrue);

		UI_XPPM_RegisterPlayerAnimation(pi, modelName, LEGS_JUMP, "jump", qfalse, qfalse, qfalse);
		UI_XPPM_RegisterPlayerAnimation(pi, modelName, LEGS_LAND, "land", qfalse, qfalse, qfalse);

		UI_XPPM_RegisterPlayerAnimation(pi, modelName, LEGS_JUMPB, "jump", qfalse, qfalse, qfalse);	// FIXME ?
		UI_XPPM_RegisterPlayerAnimation(pi, modelName, LEGS_LANDB, "land", qfalse, qfalse, qfalse);


		UI_XPPM_RegisterPlayerAnimation(pi, modelName, LEGS_TURN, "step", qfalse, qfalse, qfalse);


		Com_sprintf(filename, sizeof(filename), "models/players/%s/body_%s.skin", modelName, skinName);

		Com_Printf("UI_XPPM: Loading skin %s\n", filename);

		pi->bodySkin = trap_R_RegisterSkin(filename);

		if(!pi->bodySkin)
		{
			Com_Printf("Body skin load failure: %s\n", filename);
			return qfalse;
		}


	}

	return qtrue;
}



void UI_XPPM_Player(float x, float y, float w, float h, playerInfo_t * pi, int time)
{


}


#endif
