/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2006 Tim Angus
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
// bg_slidemove.c -- part of bg_pmove functionality

#include "../../../code/qcommon/q_shared.h"
#include "bg_public.h"
#include "bg_local.h"

/*

input: origin, velocity, bounds, groundPlane, trace function

output: origin, velocity, impacts, stairup boolean

*/

/*
==================
PM_SlideMove

Returns qtrue if the velocity was clipped in some way
==================
*/
#define	MAX_CLIP_PLANES	5
qboolean PM_SlideMove(qboolean gravity)
{
	int             bumpcount, numbumps, extrabumps;
	vec3_t          dir;
	float           d;
	int             numplanes;
	vec3_t          planes[MAX_CLIP_PLANES];
	vec3_t          primal_velocity;
	vec3_t          clipVelocity;
	int             i, j, k;
	trace_t         trace;
	vec3_t          end;
	float           time_left;
	float           into;
	vec3_t          endVelocity;
	vec3_t          endClipVelocity;

	numbumps = 4;
	extrabumps = 0;

	VectorCopy(pm->ps->velocity, primal_velocity);

	if(gravity)
	{
		VectorCopy(pm->ps->velocity, endVelocity);
		endVelocity[2] -= pm->ps->gravity * pml.frametime;
		pm->ps->velocity[2] = (pm->ps->velocity[2] + endVelocity[2]) * 0.5;
		primal_velocity[2] = endVelocity[2];
		if(pml.groundPlane)
		{
			// slide along the ground plane
			PM_ClipVelocity(pm->ps->velocity, pml.groundTrace.plane.normal, pm->ps->velocity, OVERCLIP);
		}
	}
#if 0
	else
	{
		VectorClear(endVelocity);
	}
#endif

	time_left = pml.frametime;

	// never turn against the ground plane
	if(pml.groundPlane)
	{
		numplanes = 1;
		VectorCopy(pml.groundTrace.plane.normal, planes[0]);
	}
	else
	{
		numplanes = 0;
	}

	// never turn against original velocity
	VectorNormalize2(pm->ps->velocity, planes[numplanes]);
	numplanes++;

	for(bumpcount = 0; bumpcount < numbumps; bumpcount++)
	{
		// calculate position we are trying to move to
		VectorMA(pm->ps->origin, time_left, pm->ps->velocity, end);

		// see if we can make it there
		PM_TraceAll(&trace, pm->ps->origin, end);
		if(pm->debugLevel > 1)
		{
			Com_Printf("%i:%d %d (%f %f %f)\n",
					   c_pmove, trace.allsolid, trace.startsolid, trace.endpos[0], trace.endpos[1], trace.endpos[2]);
		}

		if(trace.allsolid)
		{
			// entity is completely trapped in another solid
			pm->ps->velocity[2] = 0;	// don't build up falling damage, but allow sideways acceleration
			return qtrue;
		}

		if(trace.fraction > 0)
		{
			// actually covered some distance
			VectorCopy(trace.endpos, pm->ps->origin);
		}

		if(trace.fraction == 1)
		{
			break;				// moved the entire distance
		}

		// save entity for contact
		PM_AddTouchEnt(trace.entityNum);

		time_left -= time_left * trace.fraction;

		if(numplanes >= MAX_CLIP_PLANES)
		{
			// this shouldn't really happen
			VectorClear(pm->ps->velocity);
			return qtrue;
		}

		//
		// if this is the same plane we hit before, nudge velocity
		// out along it, which fixes some epsilon issues with
		// non-axial planes
		//
		for(i = 0; i < numplanes; i++)
		{
			if(DotProduct(trace.plane.normal, planes[i]) > 0.99)
			{
#if 0
				if(extrabumps <= 0)
				{
					VectorAdd(trace.plane.normal, pm->ps->velocity, pm->ps->velocity);
					extrabumps++;
					numbumps++;

					if(pm->debugLevel)
						Com_Printf("%i:planevelocitynudge\n", c_pmove);
				}
				else
				{
					// zinx - if it happens again, nudge the origin instead,
					// and trace it, to make sure we don't end up in a solid

					VectorAdd(pm->ps->origin, trace.plane.normal, end);
					PM_TraceAll(&trace, pm->ps->origin, end);
					VectorCopy(trace.endpos, pm->ps->origin);

					if(pm->debugLevel)
						Com_Printf("%i:planeoriginnudge\n", c_pmove);
				}
#else
				VectorAdd(trace.plane.normal, pm->ps->velocity, pm->ps->velocity);
#endif
				break;
			}
		}

		if(i < numplanes)
		{
			continue;
		}
		VectorCopy(trace.plane.normal, planes[numplanes]);
		numplanes++;

		//
		// modify velocity so it parallels all of the clip planes
		//

		// find a plane that it enters
		for(i = 0; i < numplanes; i++)
		{
			into = DotProduct(pm->ps->velocity, planes[i]);
			if(into >= 0.1)
			{
				continue;		// move doesn't interact with the plane
			}

			// see how hard we are hitting things
			if(-into > pml.impactSpeed)
			{
				pml.impactSpeed = -into;
			}

			// slide along the plane
			PM_ClipVelocity(pm->ps->velocity, planes[i], clipVelocity, OVERCLIP);

			// slide along the plane
			PM_ClipVelocity(endVelocity, planes[i], endClipVelocity, OVERCLIP);

			// see if there is a second plane that the new move enters
			for(j = 0; j < numplanes; j++)
			{
				if(j == i)
				{
					continue;
				}
				if(DotProduct(clipVelocity, planes[j]) >= 0.1)
				{
					continue;	// move doesn't interact with the plane
				}

				// try clipping the move to the plane
				PM_ClipVelocity(clipVelocity, planes[j], clipVelocity, OVERCLIP);
				PM_ClipVelocity(endClipVelocity, planes[j], endClipVelocity, OVERCLIP);

				// see if it goes back into the first clip plane
				if(DotProduct(clipVelocity, planes[i]) >= 0)
				{
					continue;
				}

				// slide the original velocity along the crease
				CrossProduct(planes[i], planes[j], dir);
				VectorNormalize(dir);
				d = DotProduct(dir, pm->ps->velocity);
				VectorScale(dir, d, clipVelocity);

				CrossProduct(planes[i], planes[j], dir);
				VectorNormalize(dir);
				d = DotProduct(dir, endVelocity);
				VectorScale(dir, d, endClipVelocity);

				// see if there is a third plane the the new move enters
				for(k = 0; k < numplanes; k++)
				{
					if(k == i || k == j)
					{
						continue;
					}
					if(DotProduct(clipVelocity, planes[k]) >= 0.1)
					{
						continue;	// move doesn't interact with the plane
					}

					// stop dead at a tripple plane interaction
					VectorClear(pm->ps->velocity);
					return qtrue;
				}
			}

			// if we have fixed all interactions, try another move
			VectorCopy(clipVelocity, pm->ps->velocity);
			VectorCopy(endClipVelocity, endVelocity);
			break;
		}
	}

	if(gravity)
	{
		VectorCopy(endVelocity, pm->ps->velocity);
	}

	// don't change velocity if in a timer (FIXME: is this correct?)
	if(pm->ps->pm_time)
	{
		VectorCopy(primal_velocity, pm->ps->velocity);
	}

	return (bumpcount != 0);
}

/*
==================
PM_StepEvent
==================
*/
void PM_StepEvent(vec3_t from, vec3_t to, vec3_t normal)
{
	float           size;
	vec3_t          delta, dNormal;

	VectorSubtract(from, to, delta);
	VectorCopy(delta, dNormal);
	VectorNormalize(dNormal);

	size = DotProduct(normal, dNormal) * VectorLength(delta);

#if 0
	if(size > 0.0f)
	{
		if(size > 2.0f)
		{
			if(size < 7.0f)
				PM_AddEvent(EV_STEPDN_4);
			else if(size < 11.0f)
				PM_AddEvent(EV_STEPDN_8);
			else if(size < 15.0f)
				PM_AddEvent(EV_STEPDN_12);
			else
				PM_AddEvent(EV_STEPDN_16);
		}
	}
	else
#endif
	{
		size = fabs(size);

		if(size > 2.0f)
		{
			if(size < 7.0f)
				PM_AddEvent(EV_STEP_4);
			else if(size < 11.0f)
				PM_AddEvent(EV_STEP_8);
			else if(size < 15.0f)
				PM_AddEvent(EV_STEP_12);
			else
				PM_AddEvent(EV_STEP_16);
		}
	}

	if(pm->debugLevel)
		Com_Printf("%i:stepped\n", c_pmove);
}

/*
==================
PM_StepSlideMove
==================
*/
qboolean PM_StepSlideMove(qboolean gravity, qboolean predictive)
{
	vec3_t          start_o, start_v;
	vec3_t          down_o, down_v;
	trace_t         trace;
	vec3_t          normal;
	vec3_t          step_v, step_vNormal;
	vec3_t          up, down;
	float           stepSize;
	qboolean        stepped = qfalse;

	if(pm->ps->pm_flags & PMF_WALLCLIMBING)
	{
		if(pm->ps->pm_flags & PMF_WALLCLIMBINGCEILING)
			VectorSet(normal, 0.0f, 0.0f, -1.0f);
		else
			VectorCopy(pm->ps->grapplePoint, normal);
	}
	else
		VectorSet(normal, 0.0f, 0.0f, 1.0f);

	VectorCopy(pm->ps->origin, start_o);
	VectorCopy(pm->ps->velocity, start_v);

	if(PM_SlideMove(gravity) == 0)
	{
		VectorCopy(start_o, down);
		VectorMA(down, -STEPSIZE, normal, down);
		PM_TraceAll(&trace, start_o, down);

		// we can step down
		if(trace.fraction > 0.01f && trace.fraction < 1.0f && !trace.allsolid && pml.groundPlane != qfalse)
		{
			if(pm->debugLevel)
				Com_Printf("%d: step down\n", c_pmove);

			stepped = qtrue;
		}
	}
	else
	{
		VectorCopy(start_o, down);
		VectorMA(down, -STEPSIZE, normal, down);
		PM_TraceAll(&trace, start_o, down);
		// never step up when you still have up velocity
		if(DotProduct(trace.plane.normal, pm->ps->velocity) > 0.0f &&
		   (trace.fraction == 1.0f || DotProduct(trace.plane.normal, normal) < 0.7f))
		{
			return stepped;
		}

		VectorCopy(pm->ps->origin, down_o);
		VectorCopy(pm->ps->velocity, down_v);

		VectorCopy(start_o, up);
		VectorMA(up, STEPSIZE, normal, up);

		// test the player position if they were a stepheight higher
		PM_TraceAll(&trace, start_o, up);
		if(trace.allsolid)
		{
			if(pm->debugLevel)
				Com_Printf("%i:bend can't step\n", c_pmove);

			return stepped;		// can't step up
		}

		VectorSubtract(trace.endpos, start_o, step_v);
		VectorCopy(step_v, step_vNormal);
		VectorNormalize(step_vNormal);

		stepSize = DotProduct(normal, step_vNormal) * VectorLength(step_v);
		// try slidemove from this position
		VectorCopy(trace.endpos, pm->ps->origin);
		VectorCopy(start_v, pm->ps->velocity);

		if(PM_SlideMove(gravity) == 0)
		{
			if(pm->debugLevel)
				Com_Printf("%d: step up\n", c_pmove);

			stepped = qtrue;
		}

		// push down the final amount
		VectorCopy(pm->ps->origin, down);
		VectorMA(down, -stepSize, normal, down);
		PM_TraceAll(&trace, pm->ps->origin, down);

		if(!trace.allsolid)
			VectorCopy(trace.endpos, pm->ps->origin);

		if(trace.fraction < 1.0f)
			PM_ClipVelocity(pm->ps->velocity, trace.plane.normal, pm->ps->velocity, OVERCLIP);
	}

	if(!predictive && stepped)
		PM_StepEvent(start_o, pm->ps->origin, normal);

	return stepped;
}


/*
==================
PM_PredictStepMove
==================
*/
qboolean PM_PredictStepMove(void)
{
	vec3_t          velocity, origin;
	float           impactSpeed;
	qboolean        stepped = qfalse;

	VectorCopy(pm->ps->velocity, velocity);
	VectorCopy(pm->ps->origin, origin);
	impactSpeed = pml.impactSpeed;

	if(PM_StepSlideMove(qfalse, qtrue))
		stepped = qtrue;

	VectorCopy(velocity, pm->ps->velocity);
	VectorCopy(origin, pm->ps->origin);
	pml.impactSpeed = impactSpeed;

	return stepped;
}


