/// ============================================================================
/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2004 Robert Beckebans <trebor_7@users.sourceforge.net>
Please see the file "AUTHORS" for a list of contributors

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
/// ============================================================================


/// includes ===================================================================
// system -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
#include "x_shared.h"


#define	PM_STEPSIZE			18
#define	PM_STOP_EPSILON			0.1
#define	PM_MIN_STEP_NORMAL		0.7		// can't step up onto very steep slopes
#define	PM_MAX_CLIP_PLANES		5


// all of the locals will be zeroed before each
// pmove, just to make damn sure we don't have
// any differences when running on client or server

struct pml_t
{
	vec3_c		origin_prev;
	vec3_c		origin;
	
	vec3_c		velocity_linear;	// full float precision
	vec3_c		velocity_angular;

	vec3_c		forward, right, up;
	float		frametime;		// in seconds


//	cshader_t*	groundshader;
	cplane_c	groundplane;
	int		groundcontents;

	bool		ladder;
};

static pmove_t*	pm;
static pml_t	pml;


// movement parameters
static float	pm_stopspeed = 100;
//static float	pm_maxspeed = 300;
//static float	pm_duckspeed = 100;
//static float	pm_noclipspeed = 200;

//static float	pm_accelerate = 10;
//static float	pm_airaccelerate = 0;
//static float	pm_wateraccelerate = 6;

//static float	pm_friction = 6;
//static float	pm_groundfriction = 6;
//static float	pm_waterfriction = 1;
static float	pm_airfriction = 1.5;
//static float	pm_waterspeed = 400;

/*
  walking up a step should kill some velocity
*/


/*
==================
PM_ClipVelocity

Slide off of the impacting object
returns the blocked flags (1 = floor, 2 = step / wall)
==================
*/
/*
static int	PM_ClipVelocity(vec3_t in, vec3_t normal, vec3_t out, float overbounce)
{
	float	backoff;
	float	change;
	int		i, blocked;
	
	blocked = 0;
	if(normal[2] > 0)
		blocked |= 1;		// floor
	if(!normal[2])
		blocked |= 2;		// step

	
	backoff = Vector3_DotProduct(in, normal) * overbounce;

	for(i=0; i<3; i++)
	{
		change = normal[i]*backoff;
		
		out[i] = in[i] - change;
		
		if(out[i] > -PM_STOP_EPSILON && out[i] < PM_STOP_EPSILON)
			out[i] = 0;
	}
	
	return blocked;
}
*/



/*
==================
PM_StepSlideMove

Each intersection will try to step over the obstruction instead of
sliding along it.

Returns a new origin, velocity, and contact entity
Does not modify any world state?
==================
*/
/*
static int	PM_SlideMove()
{
	int		bumpcount, numbumps;
	vec3_c		dir;
	float		d;
	int		numplanes;
	vec3_t		planes[PM_MAX_CLIP_PLANES];
	vec3_t		primal_velocity;
	int		i, j;
	trace_t	trace;
	vec3_t		end;
	float		time_left;
	int		blocked = 0;
	
	numbumps = 4;
	
	Vector3_Copy (pml.velocity, primal_velocity);
	numplanes = 0;
	
	time_left = pml.frametime;

	for (bumpcount=0; bumpcount<numbumps; bumpcount++)
	{
		Vector3_MA (pml.origin, time_left, pml.velocity, end);

		trace = pm->trace (pml.origin, pm->bbox, end);

		if (trace.allsolid)
		{
			// entity is trapped in another solid
			pml.velocity[2] = 0;	// don't build up falling damage
			return 3;
		}

		if (trace.fraction > 0)
		{
			// actually covered some distance
			Vector3_Copy (trace.endpos, pml.origin);
			numplanes = 0;
		}

		if (trace.fraction == 1)
			 break;		// moved the entire distance

		// save entity for contact
		
		if (pm->numtouch < MAXTOUCH && trace.ent)
		{
			pm->touchents[pm->numtouch] = trace.ent;
			pm->numtouch++;
		}
		
	
#if 1	
		if (!trace.plane._normal[2])
		{
			blocked |= 2;
		}
#endif
		
		time_left -= time_left * trace.fraction;

		// slide along this plane
		if (numplanes >= PM_MAX_CLIP_PLANES)
		{	
			// this shouldn't really happen
			Vector3_Copy (vec3_origin, pml.velocity);
			break;
		}

		Vector3_Copy (trace.plane._normal, planes[numplanes]);
		numplanes++;


	
		// modify original_velocity so it parallels all of the clip planes
		for (i=0; i<numplanes; i++)
		{
			PM_ClipVelocity (pml.velocity, planes[i], pml.velocity, 1.01);
			
			for (j=0 ; j<numplanes ; j++)
			{
				if (j != i)
				{
					if (Vector3_DotProduct (pml.velocity, planes[j]) < 0)
						break;	// not ok
				}
			}
						
			if (j == numplanes)
				break;
		}
		
		if (i != numplanes)
		{	
			// go along this plane
		}
		else
		{	
			// go along the crease
			if (numplanes != 2)
			{
				pml.velocity.clear();
				break;
			}
			
			dir.crossProduct(planes[0], planes[1]);
			d = Vector3_DotProduct (dir, pml.velocity);
			Vector3_Scale (dir, d, pml.velocity);
		}

		//
		// if velocity is against the original velocity, stop dead
		// to avoid tiny occilations in sloping corners
		//
		if (Vector3_DotProduct (pml.velocity, primal_velocity) <= 0)
		{
			pml.velocity.clear();
			break;
		}
	}

	if (pm->s.pm_time)
	{
		Vector3_Copy (primal_velocity, pml.velocity);
	}
	
	return blocked;
}
*/

/*
static void 	PM_StepSlideMove()
{
	vec3_t		start_o, start_v;
	vec3_t		down_o, down_v;
	trace_t		trace;
	float		down_dist, up_dist;
	//vec3_t		delta;
	vec3_t		up, down;
	//int		blocked;

	Vector3_Copy (pml.origin, start_o);
	Vector3_Copy (pml.velocity, start_v);

	PM_SlideMove();

	Vector3_Copy (pml.origin, down_o);
	Vector3_Copy (pml.velocity, down_v);

	Vector3_Copy (start_o, up);
	up[2] += PM_STEPSIZE;

	trace = pm->trace (up, pm->bbox, up);
	if (trace.allsolid)
		return;		// can't step up

	// try sliding above
	Vector3_Copy (up, pml.origin);
	Vector3_Copy (start_v, pml.velocity);

	PM_SlideMove();

	// push down the final amount
	Vector3_Copy (pml.origin, down);
	down[2] -= PM_STEPSIZE;
	trace = pm->trace (pml.origin, pm->bbox, down);
	
	if (!trace.allsolid)
	{
		Vector3_Copy (trace.endpos, pml.origin);
	}
	
	
#if 0
	Vector3_Subtract (pml.origin, up, delta);
	up_dist = Vector3_DotProduct (delta, start_v);

	Vector3_Subtract (down_o, start_o, delta);
	down_dist = Vector3_DotProduct (delta, start_v);
#else
	Vector3_Copy(pml.origin, up);

	// decide which one went farther
   	down_dist = (down_o[0] - start_o[0])*(down_o[0] - start_o[0]) + (down_o[1] - start_o[1])*(down_o[1] - start_o[1]);
	up_dist = (up[0] - start_o[0])*(up[0] - start_o[0]) + (up[1] - start_o[1])*(up[1] - start_o[1]);
#endif

	if (down_dist > up_dist || trace.plane._normal[2] < PM_MIN_STEP_NORMAL)
	{
		Vector3_Copy (down_o, pml.origin);
		Vector3_Copy (down_v, pml.velocity);
		return;
	}
	
#if 0
	if ((blocked & 2) || trace.plane.normal[2] == 1)
		pm->step = true;
#endif
	
	//!! Special case
	// if we were walking along a plane, then we need to copy the Z over
	pml.velocity[2] = down_v[2];
}
*/

/*
==================
PM_Friction

Handles both ground friction and water friction
==================
*/
/*
static void 	PM_Friction()
{
	vec_t speed = pml.velocity.length();
	
	if(speed < 1)
	{
		//pml.velocity[0] = 0;
		//pml.velocity[1] = 0;
		pml.velocity.clear();
		return;
	}

	vec_t drop = 0;

	// apply ground friction
	if((pm->groundentity && pml.groundshader && !(pml.groundshader->flags & SURF_SLICK) ) || (pml.ladder) )
	{
		vec_t control = speed < pm_stopspeed ? pm_stopspeed : speed;
		drop += control * pm_groundfriction * pml.frametime;
	}

	// apply water friction
	if(pm->waterlevel && !pml.ladder)
		drop += speed * pm_waterfriction * pm->waterlevel * pml.frametime;
		
	// scale the velocity
	vec_t newspeed = speed - drop;
	if(newspeed < 0)
	{
		newspeed = 0;
		pml.velocity.clear();
	}
	
	newspeed /= speed;
	pml.velocity.scale(newspeed);
}
*/


void	PM_AirFriction()
{
	vec_t speed = pml.velocity_linear.length();
	
	if(speed < 1)
	{
		pml.velocity_linear.clear();
	}
	else
	{
		vec_t control = speed < pm_stopspeed ? pm_stopspeed : speed;
		vec_t drop = control * pm_airfriction * pml.frametime;

		// scale the velocity
		vec_t newspeed = speed - drop;
		
		if(newspeed < 0)
			newspeed = 0;
			
		newspeed /= speed;
		
		pml.velocity_linear.scale(newspeed);
	}
}


/*
==============
PM_Accelerate

Handles user intended acceleration
==============
*/
/*
static void	PM_Accelerate(const vec3_c &wishdir, float wishspeed, float accel)
{
	int		i;
	float		addspeed, accelspeed, currentspeed;

	currentspeed = pml.velocity.dotProduct(wishdir);
	addspeed = wishspeed - currentspeed;
	
	if (addspeed <= 0)
		return;
	
	accelspeed = accel*pml.frametime*wishspeed;
	
	if (accelspeed > addspeed)
		accelspeed = addspeed;
		
	for (i=0 ; i<3 ; i++)
		pml.velocity[i] += accelspeed*wishdir[i];
}
*/

/*
static void	PM_AirAccelerate(vec3_t wishdir, float wishspeed, float accel)
{
	int		i;
	float		addspeed, accelspeed, currentspeed, wishspd = wishspeed;
		
	if (wishspd > 30)
		wishspd = 30;
	
	
	currentspeed = Vector3_DotProduct (pml.velocity, wishdir);
	
	addspeed = wishspd - currentspeed;
	
	if (addspeed <= 0)
		return;
	
	accelspeed = accel * wishspeed * pml.frametime;
	
	if (accelspeed > addspeed)
		accelspeed = addspeed;
	
	for (i=0 ; i<3 ; i++)
		pml.velocity[i] += accelspeed*wishdir[i];
}
*/

/*
static void	PM_AddCurrents(vec3_t	wishvel)
{
	//
	// account for ladders
	//

	if (pml.ladder && fabs(pml.velocity[2]) <= 200)
	{
		if ((pm->viewangles[PITCH] <= -15) && (pm->cmd.forwardmove > 0))
			wishvel[2] = 200;
		
		else if ((pm->viewangles[PITCH] >= 15) && (pm->cmd.forwardmove > 0))
			wishvel[2] = -200;
		
		else if (pm->cmd.upmove > 0)
			wishvel[2] = 200;
		
		else if (pm->cmd.upmove < 0)
			wishvel[2] = -200;
		else
			wishvel[2] = 0;

		// limit horizontal speed when on a ladder
		clamp(wishvel[0], -25, 25);
		clamp(wishvel[1], -25, 25);
	}
}
*/

/*
static void	PM_WaterMove()
{
	int		i;
	vec3_t	wishvel;
	float	wishspeed;
	vec3_t	wishdir;

	//
	// user intentions
	//
	for (i=0; i<3; i++)
		wishvel[i] = pml.forward[i]*pm->cmd.forwardmove + pml.right[i]*pm->cmd.sidemove;

	if (!pm->cmd.forwardmove && !pm->cmd.sidemove && !pm->cmd.upmove)
		wishvel[2] -= 60;		// drift towards bottom
	else
		wishvel[2] += pm->cmd.upmove;

	PM_AddCurrents (wishvel);

	Vector3_Copy (wishvel, wishdir);
	wishspeed = Vector3_Normalize(wishdir);

	if (wishspeed > pm_maxspeed)
	{
		wishspeed = pm_maxspeed/wishspeed;
		Vector3_Scale (wishvel, wishspeed, wishvel);
		wishspeed = pm_maxspeed;
	}
	
	wishspeed *= 0.5;

	PM_Accelerate (wishdir, wishspeed, pm_wateraccelerate);

	PM_StepSlideMove ();
}
*/

/*
static void	PM_AirMove()
{
	int			i;
	vec3_t		wishvel;
	float		fmove, smove;
	vec3_t		wishdir;
	float		wishspeed;
	float		maxspeed;

	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.sidemove;
	
	for (i=0; i<2; i++)
		wishvel[i] = pml.forward[i]*fmove + pml.right[i]*smove;
	wishvel[2] = 0;

	PM_AddCurrents (wishvel);

	Vector3_Copy (wishvel, wishdir);
	wishspeed = Vector3_Normalize(wishdir);
	
	//
	// clamp to server defined max speed
	//
	maxspeed = (pm->s.pm_flags & PMF_DUCKED) ? pm_duckspeed : pm_maxspeed;

	if(wishspeed > maxspeed)
	{
		wishspeed = maxspeed/wishspeed;
		Vector3_Scale (wishvel,wishspeed, wishvel);
		wishspeed = maxspeed;
	}
	
	if(pml.ladder)
	{
		PM_Accelerate(wishdir, wishspeed, pm_accelerate);
		
		if(!wishvel[2])
		{
			if(pml.velocity[2] > 0)
			{
				pml.velocity[2] -= pm->s.gravity * pml.frametime;
				
				if(pml.velocity[2] < 0)
					pml.velocity[2]  = 0;
			}
			else
			{
				pml.velocity[2] += pm->s.gravity * pml.frametime;
				
				if(pml.velocity[2] > 0)
					pml.velocity[2]  = 0;
			}
		}
		
		PM_StepSlideMove();
	}
	else if(pm->groundentity)
	{	
		// walking on ground
		//if (pml.velocity[2] > 0)
		pml.velocity[2] = 0; //!!! this is before the accel
		
		PM_Accelerate(wishdir, wishspeed, pm_accelerate);

		//fix for negative trigger_gravity fields
		if(pm->s.gravity > 0)
		{
			pml.velocity[2] = 0;
		}
		else
			pml.velocity[2] -= pm->s.gravity * pml.frametime;

		
		if (!pml.velocity[0] && !pml.velocity[1])
			return;
		
		PM_StepSlideMove();
	}
	else
	{	
		// not on ground, so little effect on velocity
		if(pm_airaccelerate)
			PM_AirAccelerate(wishdir, wishspeed, pm_accelerate);
		else
			PM_Accelerate(wishdir, wishspeed, 1);
		
		// add gravity
		pml.velocity[2] -= pm->s.gravity * pml.frametime;
		PM_StepSlideMove();
	}
}
*/

/*
static void	PM_CatagorizePosition()
{
	vec3_t		point;
	int			cont;
	trace_t		trace;
	int			sample1;
	int			sample2;

	// if the player hull point one unit down is solid, the player
	// is on ground

	// see if standing on something solid	
	point[0] = pml.origin[0];
	point[1] = pml.origin[1];
	point[2] = pml.origin[2] - 0.25;
	
	
	if (pml.velocity[2] > 180) //!!ZOID changed from 100 to 180 (ramp accel)
	{
		pm->s.pm_flags &= ~PMF_ON_GROUND;
		pm->groundentity = NULL;
	}
	else
	{
		trace = pm->trace (pml.origin, pm->bbox, point);
		pml.groundplane = trace.plane;
		pml.groundshader = trace.surface;
		pml.groundcontents = trace.contents;

		if (!trace.ent || (trace.plane._normal[2] < PM_MIN_STEP_NORMAL && !trace.startsolid) )
		{
			pm->groundentity = NULL;
			pm->s.pm_flags &= ~PMF_ON_GROUND;
		}
		else
		{
			pm->groundentity = trace.ent;

			// hitting solid ground will end a waterjump
			if (pm->s.pm_flags & PMF_TIME_WATERJUMP)
			{
				pm->s.pm_flags &= ~(PMF_TIME_WATERJUMP | PMF_TIME_LAND | PMF_TIME_TELEPORT);
				pm->s.pm_time = 0;
			}

			if (! (pm->s.pm_flags & PMF_ON_GROUND) )
			{	
				// just hit the ground
				pm->s.pm_flags |= PMF_ON_GROUND;

#if 1				
				// don't do landing time if we were just going down a slope
				if (pml.velocity[2] < -200)
				{
					pm->s.pm_flags |= PMF_TIME_LAND;
					// don't allow another jump for a little while
					if (pml.velocity[2] < -400)
						pm->s.pm_time = 25;	
					else
						pm->s.pm_time = 18;
				}
#endif
			}
		}

#if 0
		if (trace.fraction < 1.0 && trace.ent && pml.velocity[2] < 0)
			pml.velocity[2] = 0;
#endif

	}

	//
	// get waterlevel, accounting for ducking
	//
	pm->waterlevel = 0;
	pm->watertype = 0;

	sample2 = (int)(pm->viewheight - pm->bbox._mins[2]);
	sample1 = sample2 / 2;

	point[2] = pml.origin[2] + pm->bbox._mins[2] + 1;	
	cont = pm->pointcontents (point);

	if (cont & MASK_WATER)
	{
		pm->watertype = cont;
		pm->waterlevel = 1;
		point[2] = pml.origin[2] + pm->bbox._mins[2] + sample1;
		cont = pm->pointcontents (point);
		
		if (cont & MASK_WATER)
		{
			pm->waterlevel = 2;
			point[2] = pml.origin[2] + pm->bbox._mins[2] + sample2;
			cont = pm->pointcontents (point);
			
			if (cont & MASK_WATER)
				pm->waterlevel = 3;
		}
	}
}
*/

/*
static void	PM_CheckJump()
{
	if (pm->s.pm_flags & PMF_TIME_LAND)
	{	
		// hasn't been long enough since landing to jump again
		return;
	}

	if (pm->cmd.upmove < 10)
	{	
		// not holding jump
		pm->s.pm_flags &= ~PMF_JUMP_HELD;
		return;
	}

	// must wait for jump to be released
	if (pm->s.pm_flags & PMF_JUMP_HELD)
		return;

	if (pm->s.pm_type == PM_DEAD)
		return;

	if (pm->waterlevel >= 2)
	{	
		// swimming, not jumping
		pm->groundentity = NULL;
#if 1
		if (pml.velocity[2] <= -300)
			return;

		if (pm->watertype == CONTENTS_WATER)
			pml.velocity[2] = 100;
		else if (pm->watertype == CONTENTS_SLIME)
			pml.velocity[2] = 80;
		else
			pml.velocity[2] = 50;
#endif
		return;
	}

	if (pm->groundentity == NULL)
		return;		// in air, so no effect

	pm->s.pm_flags |= PMF_JUMP_HELD;

	pm->groundentity = NULL;
	pml.velocity[2] += 270;
	
	if (pml.velocity[2] < 270)
		pml.velocity[2] = 270;
}
*/

/*
static void	PM_CheckSpecialMovement()
{
	vec3_c	spot;
	int	cont;
	vec3_c	flatforward;
	trace_t	trace;

	if(pm->s.pm_time)
		return;

	pml.ladder = false;

	// check for ladder
	flatforward[0] = pml.forward[0];
	flatforward[1] = pml.forward[1];
	flatforward[2] = 0;
	flatforward.normalize();

	Vector3_MA (pml.origin, 1, flatforward, spot);
	trace = pm->trace (pml.origin, pm->bbox, spot);
	
	if((trace.fraction < 1) && (trace.surface->flags & SURF_LADDER))
		pml.ladder = true;

	// check for water jump
	if(pm->waterlevel != 2)
		return;

	Vector3_MA(pml.origin, 30, flatforward, spot);
	spot[2] += 4;
	cont = pm->pointcontents (spot);
	
	if (!(cont & CONTENTS_SOLID))
		return;

	spot[2] += 16;
	cont = pm->pointcontents (spot);
	if(cont)
		return;
	// jump out of water
	pml.velocity = flatforward * 50;
	pml.velocity[2] = 350;

	pm->s.pm_flags |= PMF_TIME_WATERJUMP;
	pm->s.pm_time = 255;
}
*/

void	PM_FlyMove(bool doclip)
{
#if 0
	float	currentspeed, addspeed, accelspeed;
	vec3_c		wishvel;
	vec3_c		wishdir;
	float		wishspeed;

	pm->viewheight = 22;

	// friction
	PM_AirFriction();

	// accelerate	
	pml.forward.normalize();
	pml.right.normalize();

	wishvel = (pml.forward * pm->cmd.forwardmove) + (pml.right * pm->cmd.sidemove);
	wishvel[2] += pm->cmd.upmove;

	wishdir = wishvel;
	wishspeed = wishdir.normalize();

	//
	// clamp to server defined max speed
	//
	if(wishspeed > pm_maxspeed)
	{
		wishspeed = pm_maxspeed/wishspeed;
		Vector3_Scale (wishvel, wishspeed, wishvel);
		wishspeed = pm_maxspeed;
	}


	currentspeed = Vector3_DotProduct(pml.velocity, wishdir);
	addspeed = wishspeed - currentspeed;
	
	if(addspeed <= 0)
		return;
	
	accelspeed = pm_accelerate * pml.frametime * wishspeed;
	if(accelspeed > addspeed)
		accelspeed = addspeed;
	
	pml.velocity += (wishdir * accelspeed);

	// move
	if(!doclip)
	{
		pml.origin += pml.velocity * pml.frametime;
	}
	else
	{
		//TODO
		pml.origin += pml.velocity * pml.frametime;
	}
#else

	pm->viewheight = 22;

	// accelerate
	pml.forward.normalize();
	pml.right.normalize();
	pml.up.normalize();

	vec3_c wishvel = (pml.forward * pm->cmd.forwardmove) + (pml.right * pm->cmd.sidemove);
	wishvel[2] += pm->cmd.upmove;
	wishvel.normalize();
	
	pml.velocity_linear = wishvel * 200;
	
	if(!doclip)
	{
		pml.origin += pml.velocity_linear * pml.frametime;
	}
	else
	{
		trace_t trace = pm->rayTrace(pml.origin, wishvel, (pml.velocity_linear * pml.frametime).length());
		
		if(trace.nohit)
		{
			pml.origin += pml.velocity_linear * pml.frametime;
		}
	}
#endif
}

void	PM_RollMove()
{
	pm->viewheight = 22;
//	vec3_c wishvel = (pml.forward * pm->cmd.forwardmove) + (pml.right * pm->cmd.sidemove);
//	wishvel[2] += pm->cmd.upmove;
//	wishvel.normalize();
	
//	pml.velocity[PITCH]	= wishvel[0] * pml.frametime;	// roll forward around the Y-Axis
//	pml.velocity[YAW]	= 0;//wishvel[1] * pml.frametime;	// roll right around the X-Axis
//	pml.velocity[ROLL]	= 0;

	pml.velocity_angular[PITCH] = pm->cmd.forwardmove * pml.frametime * 0.5;	// roll forward around the Y-Axis
	pml.velocity_angular[ROLL] = pm->cmd.sidemove * pml.frametime * 0.5;		// roll right around the X-Axis
	pml.velocity_angular[YAW] = 0;
}

/*
static void	PM_CheckDuck()
{
	trace_t	trace;
	vec3_t	end;

	pm->bbox._mins[0] = -16;
	pm->bbox._mins[1] = -16;

	pm->bbox._maxs[0] = 16;
	pm->bbox._maxs[1] = 16;

	if (pm->s.pm_type == PM_GIB)
	{
		pm->bbox._mins[2] = 0;
		pm->bbox._maxs[2] = 16;
		pm->viewheight = 8;
		return;
	}

	pm->bbox._mins[2] = -24;

	if (pm->s.pm_type == PM_DEAD)
	{
		pm->s.pm_flags |= PMF_DUCKED;
	}
	else if (pm->cmd.upmove < 0 && (pm->s.pm_flags & PMF_ON_GROUND) )
	{	
		// duck
		pm->s.pm_flags |= PMF_DUCKED;
	}
	else
	{	
		// stand up if possible
		if (pm->s.pm_flags & PMF_DUCKED)
		{
			// try to stand up
			Vector3_Copy (pml.origin, end);
			end[2] += 32 - pm->bbox._maxs[2];

			trace = pm->trace (pml.origin, pm->bbox, end);
			
			if (trace.fraction == 1)
			{
				pm->bbox._maxs[2] = 32;
				pm->s.pm_flags &= ~PMF_DUCKED;
			}
		}
	}

	if (pm->s.pm_flags & PMF_DUCKED)
	{
		pm->bbox._maxs[2] = 4;
		pm->viewheight = -2;
	}
	else
	{
		pm->bbox._maxs[2] = 32;
		pm->viewheight = 22;
	}
}
*/

/*
static void	PM_DeadMove()
{
	float	forward;

	if (!pm->groundentity)
		return;

	// extra friction
	forward = pml.velocity.length() - 20;
	
	if (forward <= 0)
	{
		pml.velocity.clear();
	}
	else
	{
		pml.velocity.normalize();
		pml.velocity.scale(forward);
	}
}
*/

/*
static bool	PM_GoodPosition()
{
	trace_t	trace;
	vec3_t	origin, end;
	int		i;

	if (pm->s.pm_type == PM_SPECTATOR)
		return true;

	for (i=0; i<3; i++)
		origin[i] = end[i] = pm->s.origin[i]*0.125;

	trace = pm->trace(origin, pm->bbox, end);

	return !trace.allsolid;
}
*/

/*
================
PM_SnapPosition

On exit, the origin will have a value that is pre-quantized to the 0.125
precision of the network channel and in a valid position.
================
*/
static void	PM_SnapPosition()
{
	/*
	int		sign[3];
	int		i, j, bits;
	vec3_t	base;
	*/
	
	// try all single bits first
	//static int jitterbits[8] = {0,4,1,2,3,5,6,7};

	// snap velocity to eigths
	/*
	for(i=0; i<3; i++)
		pm->s.velocity[i] = (int)(pml.velocity[i]*8);

	for(i=0; i<3; i++)
	{
		if(pml.origin[i] >= 0)
			sign[i] = 1;
		else 
			sign[i] = -1;
		
		pm->s.origin[i] = (int)(pml.origin[i]*8);
		
		if(pm->s.origin[i]*0.125 == pml.origin[i])
			sign[i] = 0;
	}
	Vector3_Copy(pm->s.origin, base);

	// try all combinations
	for(j=0; j<8; j++)
	{
		bits = jitterbits[j];
		
		Vector3_Copy(base, pm->s.origin);
		
		for(i=0; i<3; i++)
		{
			if(bits & (1<<i) )
				pm->s.origin[i] += sign[i];
		}

		if(PM_GoodPosition())
			return;
	}
	*/
	
	pm->s.origin = pml.origin;
	pm->s.velocity_linear = pml.velocity_linear;
	pm->s.velocity_angular = pml.velocity_angular;

	// go back to the last position
	//pm->s.origin = pml.origin_prev;
	//Vector3_Clear (pm->s.velocity);
	//Com_Printf("using previous_origin\n");	
}

/*
static void	PM_InitialSnapPosition()
{
	int        x, y, z;
	//short      base[3];
	vec3_t	     base;
	static const int offset[3] = { 0, -1, 1 };

	Vector3_Copy(pm->s.origin, base);

	for(z=0; z<3; z++) 
	{
		pm->s.origin[2] = base[2] + offset[ z ];
		
		for(y=0; y<3; y++) 
		{
			pm->s.origin[1] = base[1] + offset[ y ];
			
			for(x=0; x<3; x++) 
			{
				pm->s.origin[0] = base[0] + offset[ x ];
				
				if(PM_GoodPosition())
				{
					pml.origin[0] = pm->s.origin[0]*0.125;
					pml.origin[1] = pm->s.origin[1]*0.125;
					pml.origin[2] = pm->s.origin[2]*0.125;
					
					pml.origin_prev = pm->s.origin;
					return;
				}
			}
		}
	}

	Com_Printf("Bad InitialSnapPosition\n");
}
*/


static void	PM_ClampAngles()
{
	if(pm->s.pm_flags & PMF_TIME_TELEPORT)
	{
		pm->viewangles[YAW] = pm->cmd.angles[YAW] + pm->s.delta_angles[YAW];
		pm->viewangles[PITCH] = 0;
		pm->viewangles[ROLL] = 0;
	}
	else
	{
		// circularly clamp the angles with deltas
		pm->viewangles = pm->cmd.angles + pm->s.delta_angles;
	
		// don't let the player look up or down more than 90 degrees
		if(pm->viewangles[PITCH] > 89 && pm->viewangles[PITCH] < 180)
			pm->viewangles[PITCH] = 89;
		
		else if(pm->viewangles[PITCH] < 271 && pm->viewangles[PITCH] >= 180)
			pm->viewangles[PITCH] = 271;
	}
	
	Angles_ToVectors(pm->viewangles, pml.forward, pml.right, pml.up);
}

/*
================
Com_Pmove

Can be called by either the server or the client
================
*/
void 	Com_Pmove(pmove_t *pmove)
{
	pm = pmove;

	// clear results
	pm->viewangles.clear();
	pm->viewheight = 0;
	pm->groundentity = 0;
	pm->watertype = 0;
	pm->waterlevel = 0;

	// clear all pmove local vars
	memset(&pml, 0, sizeof(pml));

	// convert origin and velocity to float values
	pml.origin		= pml.origin_prev = pm->s.origin;
	pml.velocity_linear	= pm->s.velocity_linear;
	pml.velocity_angular	= pm->s.velocity_angular;

	// convert command time
	pml.frametime = pm->cmd.msec * 0.001;

	PM_ClampAngles();

	if(pm->s.pm_type == PM_SPECTATOR)
	{
		PM_FlyMove(false);
		PM_SnapPosition();
		return;
	}
	
	if(pm->s.pm_type >= PM_DEAD)
	{
		pm->cmd.forwardmove = 0;
		pm->cmd.sidemove = 0;
		pm->cmd.upmove = 0;
	}

	if(pm->s.pm_type == PM_FREEZE)
		return;		// no movement at all
	
	PM_FlyMove(true);
	
	//PM_RollMove();
		
	
	// set mins, maxs, and viewheight
	//PM_CheckDuck();
	
	/*
	if(pm->snapinitial)
		PM_InitialSnapPosition();

	// set groundentity, watertype, and waterlevel
	PM_CatagorizePosition();

	if(pm->s.pm_type == PM_DEAD)
		PM_DeadMove();

	PM_CheckSpecialMovement();

	// drop timing counter
	if(pm->s.pm_time)
	{
		int msec = pm->cmd.msec >> 3;
		
		if(!msec)
			msec = 1;
		
		if(msec >= pm->s.pm_time) 
		{
			pm->s.pm_flags &= ~(PMF_TIME_WATERJUMP | PMF_TIME_LAND | PMF_TIME_TELEPORT);
			pm->s.pm_time = 0;
		}
		else
			pm->s.pm_time -= msec;
	}

	if(pm->s.pm_flags & PMF_TIME_TELEPORT)
	{	
		// teleport pause stays exactly in place
	}
	else if(pm->s.pm_flags & PMF_TIME_WATERJUMP)
	{	
		// waterjump has no control, but falls
		pml.velocity[2] -= pm->s.gravity * pml.frametime;
		
		if(pml.velocity[2] < 0)
		{	// cancel as soon as we are falling down again
			pm->s.pm_flags &= ~(PMF_TIME_WATERJUMP | PMF_TIME_LAND | PMF_TIME_TELEPORT);
			pm->s.pm_time = 0;
		}

		PM_StepSlideMove();
	}
	else
	{
		PM_CheckJump();

		PM_Friction();

		if(pm->waterlevel >= 2)
			PM_WaterMove ();
		else 
		{
			vec3_t	angles;

			Vector3_Copy(pm->viewangles, angles);
			
			if(angles[PITCH] > 180)
				angles[PITCH] = angles[PITCH] - 360;
			angles[PITCH] /= 3;

			Angles_ToVectors (angles, pml.forward, pml.right, pml.up);

			PM_AirMove();
		}
	}

	// set groundentity, watertype, and waterlevel for final spot
	PM_CatagorizePosition();
	*/
	
	PM_SnapPosition();
}



