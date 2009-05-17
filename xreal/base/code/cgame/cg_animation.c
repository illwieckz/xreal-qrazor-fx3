/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006-2009 Robert Beckebans <trebor_7@users.sourceforge.net>

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
// cg_animation.c

#include "cg_local.h"


qboolean CG_RegisterAnimation(animation_t * anim, const char *filename,
										   qboolean loop, qboolean reversed, qboolean clearOrigin)
{
	int             frameRate;

	anim->handle = trap_R_RegisterAnimation(filename);
	if(!anim->handle)
	{
		Com_Printf("Failed to load animation file %s\n", filename);
		return qfalse;
	}

	anim->firstFrame = 0;
	anim->numFrames = trap_R_AnimNumFrames(anim->handle);
	frameRate = trap_R_AnimFrameRate(anim->handle);

	if(frameRate == 0)
	{
		frameRate = 1;
	}
	anim->frameTime = 1000 / frameRate;
	anim->initialLerp = 1000 / frameRate;

	if(loop)
	{
		anim->loopFrames = anim->numFrames;
	}
	else
	{
		anim->loopFrames = 0;
	}

	anim->reversed = reversed;
	anim->clearOrigin = clearOrigin;

	return qtrue;
}
