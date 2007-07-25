/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2007 Pat Raynor <raynorpat@sbcglobal.net>

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

// bg_xreal.c -- both games xreal definitions (weapon damage, etc.)
#include "q_shared.h"
#include "bg_public.h"
#include "bg_local.h"
#include "bg_xreal.h"

// XreaL Weapons
float knockback_z = 40.0f;

// XreaL Movement Physics
float pm_airStopAccelerate = 2.5f;
float pm_airControlAmount = 150.0f;
float pm_strafeAccelerate = 70.0f;
float pm_wishSpeed = 30.0f;

// raynorpat: Air Control from CPM Land... :)
void PM_Aircontrol(pmove_t *pm, vec3_t wishdir, float wishspeed)
{
	float		zspeed, speed, dot, k;
	int			i;

	if((pm->ps->movementDir && pm->ps->movementDir != 4) || wishspeed == 0.0) 
		return; // can't control movement if not moving forward or backward

	zspeed = pm->ps->velocity[2];
	pm->ps->velocity[2] = 0;
	speed = VectorNormalize(pm->ps->velocity);

	dot = DotProduct(pm->ps->velocity,wishdir);
	k = 32; 
	k *= pm_airControlAmount * dot * dot * pml.frametime;

	if(dot > 0)
	{
		// we can't change direction while slowing down
		for(i = 0; i < 2; i++)
			pm->ps->velocity[i] = pm->ps->velocity[i] * speed + wishdir[i] * k;
		VectorNormalize(pm->ps->velocity);
	}

	for(i = 0; i < 2; i++) 
		pm->ps->velocity[i] *= speed;

	pm->ps->velocity[2] = zspeed;
}