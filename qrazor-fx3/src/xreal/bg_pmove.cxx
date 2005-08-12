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
#include "../x_shared.h"
#include "bg_local.h"

static const int PM_STEPSIZE = 18;
//static const vec_t PM_STOP_EPSILON = 0.1;
static const vec_t PM_MIN_STEP_NORMAL = 0.7;		// can't step up onto very steep slopes
static const int PM_MAX_CLIP_PLANES = 5;
static const vec_t PM_OVERCLIP = 1.001f;


// all of the locals will be zeroed before each
// pmove, just to make damn sure we don't have
// any differences when running on client or server

struct pml_t
{
	vec3_c		origin_prev;
	vec3_c		origin;
	
	vec3_c		velocity_linear;	// full float precision
	vec3_c		velocity_angular;

	vec_t		impact_speed;

	vec3_c		forward, right, up;
	float		frametime;		// in seconds

	bool		walking;
	bool		ground_plane;
	trace_t		ground_trace;

	bool		ladder;
};

static pmove_t*	pm;
static pml_t	pml;

static void	PM_AddTouchEntity(entity_c* ent)
{
	if(!ent)
		return;

	std::deque<entity_c*>::const_iterator ir = std::find(pm->touchents.begin(), pm->touchents.end(), static_cast<entity_c*>(ent));

	if(ir == pm->touchents.end())
		pm->touchents.push_back(ent);
}


/*
============
PM_CmdScale

Returns the scale factor to apply to cmd movements
This allows the clients to use axial -127 to 127 values for all directions
without getting a sqrt(2) distortion in speed.
============
*/
static float	PM_CmdScale(const usercmd_t &cmd)
{
	int max = abs(cmd.forwardmove);
	
	if(abs(cmd.rightmove) > max)
		max = abs(cmd.rightmove);
		
	if(abs(cmd.upmove) > max)
		max = abs(cmd.upmove);
	
	if(max <= 0)
		return 0;
		
	float total = X_sqrt(cmd.forwardmove*cmd.forwardmove + cmd.rightmove*cmd.rightmove + cmd.upmove*cmd.upmove);
	float scale = (float)320 * max / (127.0 * total);
	
	return scale;
}

/*
==================
PM_ClipVelocity

Slide off of the impacting object
==================
*/
static void	PM_ClipVelocity(const vec3_c &in, const vec3_c &normal, vec3_c &out, vec_t overbounce)
{
	vec_t backoff = in.dotProduct(normal) * overbounce;
	
	if(backoff < 0)
	{
		backoff *= overbounce;
	}
	else
	{
		backoff /= overbounce;
	}

	for(int i=0; i<3; i++)
	{
		vec_t change = normal[i] * backoff;
	
		out[i] = in[i] - change;
		
		//if(out[i] > -PM_STOP_EPSILON && out[i] < PM_STOP_EPSILON)
		//	out[i] = 0;
	}
}

/*
==================
PM_SlideMove

Returns qtrue if the velocity was clipped in some way
==================
*/
bool	PM_SlideMove(bool gravity)
{
	int		bumpcount, numbumps;
	vec3_c		dir;
	float		d;
	int		numplanes;
	vec3_c		planes[PM_MAX_CLIP_PLANES];
	int		i, j=0, k;
	trace_t		trace;
	vec3_c		end;
	float		time_left;
	
	vec3_c		velocity_primal;
	vec3_c		velocity_end;
	vec3_c		velocity_clip;
	vec3_c		velocity_clip_end;
	
	numbumps = 4;
	
	velocity_primal = pml.velocity_linear;

	if(gravity)
	{
		velocity_end = pml.velocity_linear;
		velocity_end[2] -= pm->ps->gravity * pml.frametime;
		pml.velocity_linear[2] = (pml.velocity_linear[2] + velocity_end[2]) * 0.5;
		velocity_primal[2] = velocity_end[2];
		
		if(pml.ground_plane)
		{
			// slide along the ground plane
			PM_ClipVelocity(pml.velocity_linear, pml.ground_trace.plane._normal, pml.velocity_linear, PM_OVERCLIP);
		}
	}
	
	time_left = pml.frametime;

	// never turn against the ground plane
	if(pml.ground_plane)
	{
		numplanes = 1;
		planes[0] = pml.ground_trace.plane._normal;
	}
	else
	{
		numplanes = 0;
	}

	// never turn against original velocity
	planes[numplanes] = pml.velocity_linear;
	planes[numplanes].normalize();
	numplanes++;

	for(bumpcount=0; bumpcount<numbumps; bumpcount++)
	{
		end = pml.origin + pml.velocity_linear * time_left;

		// see if we can make it there
		trace = pm->boxTrace(pml.origin, pm->bbox, end);

		if(trace.allsolid)
		{
			// entity is trapped in another solid
			pml.velocity_linear[2] = 0;	// don't build up falling damage
			return true;
		}

		if(trace.fraction > 0)
		{
			// actually covered some distance
			pml.origin = trace.pos;
		}

		if(trace.fraction == 1)
		{
			 break;		// moved the entire distance
		}

		// save entity for contact
		PM_AddTouchEntity(trace.ent);
		
		time_left -= time_left * trace.fraction;

		// slide along this plane
		if(numplanes >= PM_MAX_CLIP_PLANES)
		{	
			// this shouldn't really happen
			pml.velocity_linear.clear();
			return true;
		}

		// if this is the same plane we hit before, nudge velocity
		// out along it, which fixes some epsilon issues with
		// non-axial planes
		for(i=0; i<numplanes; i++)
		{
			if(trace.plane._normal.dotProduct(planes[i]) > 0.99)
			{
				pml.velocity_linear += trace.plane._normal;
				break;
			}
		}

		if(i<numplanes)
		{
			continue;
		}

		planes[numplanes] = trace.plane._normal;
		numplanes++;

		// modify original_velocity so it parallels all of the clip planes
		for(i=0; i<numplanes; i++)
		{
			vec_t into = pml.velocity_linear.dotProduct(planes[i]);
			if(into >= 0.1)
				continue;

			// see how hard we are hitting things
			if(-into > pml.impact_speed) 
			{
				pml.impact_speed = -into;
			}

			// slide along the plane
			PM_ClipVelocity(pml.velocity_linear, planes[i], velocity_clip, PM_OVERCLIP);

			// slide along the plane
			PM_ClipVelocity(velocity_end, planes[i], velocity_clip_end, PM_OVERCLIP);
			
			// see if there is a second plane that the new move enters
			for(j=0; j<numplanes; j++)
			{
				if(j == i)
					continue;

				if(velocity_clip.dotProduct(planes[j]) >= 0.1)
					continue;	// move doesn't interact with the plane
				

				// try clipping the move to the plane
				PM_ClipVelocity(velocity_clip, planes[j], velocity_clip, PM_OVERCLIP);
				PM_ClipVelocity(velocity_clip_end, planes[j], velocity_clip_end, PM_OVERCLIP);

				// see if it goes back into the first clip plane
				if(velocity_clip.dotProduct(planes[i]) >= 0)
					continue;

				// slide the original velocity along the crease
				dir.crossProduct(planes[i], planes[j]);
				dir.normalize();
				
				d = dir.dotProduct(pml.velocity_linear);
				velocity_clip = dir * d;

				d = dir.dotProduct(velocity_end);
				velocity_clip_end = dir * d;

				// see if there is a third plane the the new move enters
				for(k=0; k<numplanes; k++)
				{
					if(k==i || k==j)
						continue;
					
					if(velocity_clip.dotProduct(planes[k]) >= 0.1)
						continue;	// move doesn't interact with the plane

					// stop dead at a tripple plane interaction
					pml.velocity_linear.clear();
					return true;
				}
			}

			// if we have fixed all interactions, try another move
			pml.velocity_linear = velocity_clip;
			velocity_end = velocity_clip_end;
			break;
		}
	}

	if(gravity)
		pml.velocity_linear = velocity_end;

	// don't change velocity if in a timer (FIXME: is this correct?)
	if(pm->ps->pm_time)
		pml.velocity_linear = velocity_primal;

	return (bumpcount != 0);
}


static void 	PM_StepSlideMove(bool gravity)
{
	const vec3_c start_o = pml.origin;
	const vec3_c start_v = pml.velocity_linear;

	if(PM_SlideMove(gravity) == 0)
		return;		// we got exactly where we wanted to go first try

	vec3_c down = start_o;
	down[2] -= PM_STEPSIZE;
	
	trace_t trace = pm->boxTrace(start_o, pm->bbox, down);

	// never step up when you still have up velocity
	if(start_v[2] > 0 && (trace.fraction == 1.0 || trace.plane._normal.dotProduct(vec3_c(0.0, 0.0, 1.0)) < PM_MIN_STEP_NORMAL))
		return;
	
	const vec3_c down_o = pml.origin;
	const vec3_c down_v = pml.velocity_linear;

	vec3_c up = start_o;
	up[2] += PM_STEPSIZE;

	// test the player position if they were a stepheight higher
	trace = pm->boxTrace(start_o, pm->bbox, up);
	if(trace.allsolid)
		return;		// can't step up

	vec_t step_size = trace.pos[2] - start_o[2];	

	// try sliding above
	pml.origin = trace.pos;
	pml.velocity_linear = start_v;

	PM_SlideMove(gravity);

	// push down the final amount
	down = start_o;
	down[2] -= step_size;
	
	trace = pm->boxTrace(start_o, pm->bbox, down);
	if(!trace.allsolid)
	{
		pml.origin = trace.pos;
	}

	if(trace.fraction == 1.0)
	{
		PM_ClipVelocity(pml.velocity_linear, trace.plane._normal, pml.velocity_linear, PM_OVERCLIP);
	}
	
	/*
	up = pml.origin;

	// decide which one went farther
   	vec_t down_dist = (down_o[0] - start_o[0])*(down_o[0] - start_o[0]) + (down_o[1] - start_o[1])*(down_o[1] - start_o[1]);
	vec_t up_dist = (up[0] - start_o[0])*(up[0] - start_o[0]) + (up[1] - start_o[1])*(up[1] - start_o[1]);

	if(down_dist > up_dist || trace.plane._normal[2] < PM_MIN_STEP_NORMAL)
	{
		pml.origin = down_o;
		pml.velocity_linear = down_v;
		return;
	}
	
	//!! special case, if we were walking along a plane, then we need to copy the Z over
	pml.velocity_linear[2] = down_v[2];
*/
}


/*
==================
PM_Friction

Handles both ground friction and water friction
==================
*/
static void 	PM_Friction()
{
	// ignore slope movement
	if(pml.walking)
		pml.velocity_linear[2] = 0;

	vec_t speed = pml.velocity_linear.length();
	if(speed < 1.0)
	{
		pml.velocity_linear[0] = 0;
		pml.velocity_linear[1] = 0;
		// FIXME: still have z friction underwater?
		return;
	}

	vec_t drop = 0;

	// apply ground friction
	if(pm->waterlevel < 1)
	{
		if(pml.walking && !(pml.ground_trace.pos_flags & X_SURF_SLICK) || pml.ladder)
		{
			vec_t control = speed < pm->ps->speed_stop ? pm->ps->speed_stop : speed;
			drop += control * pm->ps->friction * pml.frametime;
		}
	}

	// apply water friction
	if(pm->waterlevel >= 2 && !pml.ladder)
	{
		drop += speed * pm->ps->friction_water * pm->waterlevel * pml.frametime;
	}

	// apply spectator friction
	if(pm->ps->pm_type == PM_SPECTATOR)
	{
		drop += speed * pm->ps->friction_spectator * pml.frametime;
	}
		
	// scale the velocity
	vec_t newspeed = speed - drop;
	if(newspeed <= 0)
	{
		newspeed = 0;
		pml.velocity_linear.clear();
	}
	else
	{
		newspeed /= speed;
		pml.velocity_linear *= newspeed;
	}
}


void	PM_AirFriction()
{
	vec_t speed = pml.velocity_linear.length();
	
	if(speed < 1)
	{
		pml.velocity_linear.clear();
	}
	else
	{
		vec_t control = speed < pm->ps->speed_stop ? pm->ps->speed_stop : speed;
		vec_t drop = control * pm->ps->friction_air * pml.frametime;

		// scale the velocity
		vec_t newspeed = speed - drop;
		if(newspeed <= 0)
		{
			newspeed = 0;
			pml.velocity_linear.clear();
		}
		else
		{
			newspeed /= speed;
			pml.velocity_linear *= newspeed;
		}
	}
}


/*
==============
PM_Accelerate

Handles user intended acceleration
==============
*/
static void	PM_Accelerate(const vec3_c &wishdir, vec_t wishspeed, vec_t accel)
{
	vec_t	addspeed, accelspeed, currentspeed;

	currentspeed = pml.velocity_linear.dotProduct(wishdir);
	addspeed = wishspeed - currentspeed;
	
	if(addspeed <= 0)
		return;
	
	accelspeed = accel * pml.frametime * wishspeed;
	
	if(accelspeed > addspeed)
		accelspeed = addspeed;
		
	pml.velocity_linear += (wishdir * accelspeed);
}

static void	PM_AirAccelerate(const vec3_c &wishdir, vec_t wishspeed, vec_t accel)
{
	vec_t	addspeed, accelspeed, currentspeed, wishspd = wishspeed;
		
	if(wishspd > 30)
		wishspd = 30;
	
	currentspeed = pml.velocity_linear.dotProduct(wishdir);
	
	addspeed = wishspd - currentspeed;
	
	if(addspeed <= 0)
		return;
	
	accelspeed = accel * wishspeed * pml.frametime;
	
	if(accelspeed > addspeed)
		accelspeed = addspeed;
	
	pml.velocity_linear += (wishdir * accelspeed);
}

static void	PM_AddCurrents(vec3_c &wishvel)
{
	// account for ladders
	if(pml.ladder && fabs(pml.velocity_linear[2]) <= 200)
	{
		if((pm->viewangles[PITCH] <= -15) && (pm->cmd.forwardmove > 0))
			wishvel[2] = 200;
		
		else if((pm->viewangles[PITCH] >= 15) && (pm->cmd.forwardmove > 0))
			wishvel[2] = -200;
		
		else if(pm->cmd.upmove > 0)
			wishvel[2] = 200;
		
		else if(pm->cmd.upmove < 0)
			wishvel[2] = -200;
		else
			wishvel[2] = 0;

		// limit horizontal speed when on a ladder
		X_clamp(wishvel[0], -25, 25);
		X_clamp(wishvel[1], -25, 25);
	}
}

static void	PM_WaterJumpMove()
{
	PM_StepSlideMove(true);

	// waterjump has no control, but falls
	pml.velocity_linear[2] -= pm->ps->gravity * pml.frametime;
		
	if(pml.velocity_linear[2] < 0)
	{
		// cancel as soon as we are falling down again
		pm->ps->pm_flags &= ~(PMF_TIME_WATERJUMP | PMF_TIME_LAND | PMF_TIME_TELEPORT);
		pm->ps->pm_time = 0;
	}
}

static void	PM_WaterMove()
{
	// user intentions
	vec3_c wishvel = pml.forward*pm->cmd.forwardmove + pml.right*pm->cmd.rightmove;

	if(!pm->cmd.forwardmove && !pm->cmd.rightmove && !pm->cmd.upmove)
		wishvel[2] -= 60;		// drift towards bottom
	else
		wishvel[2] += pm->cmd.upmove;

	PM_AddCurrents(wishvel);

	vec3_c wishdir = wishvel;
	vec_t wishspeed = wishdir.normalize();

	if(wishspeed > pm->ps->speed_max)
	{
		wishspeed = pm->ps->speed_max / wishspeed;
		wishvel *= wishspeed;
		wishspeed = pm->ps->speed_max;
	}
	
	wishspeed *= 0.5;

	PM_Accelerate(wishdir, wishspeed, pm->ps->accelerate_water);

	PM_StepSlideMove(true);
}


static void	PM_AirMove()
{
	vec3_c		wishvel;
	vec3_c		wishdir;
	float		wishspeed;

	// normal slowdown
	PM_Friction();

	vec_t scale = PM_CmdScale(pm->cmd);

	// project moves down to flat plane
	pml.forward[2] = 0;
	pml.forward.normalize();

	pml.right[2] = 0;
	pml.right.normalize();

	wishvel = (pml.forward * pm->cmd.forwardmove) + (pml.right * pm->cmd.rightmove);
	
	wishdir = wishvel;
	wishspeed = wishdir.normalize();
	wishspeed *= scale;

	// not on ground, so little effect on velocity
	if(pm->ps->accelerate_air)
		PM_AirAccelerate(wishdir, wishspeed, pm->ps->accelerate_air);
	else
		PM_Accelerate(wishdir, wishspeed, 1.0);
		
	// we may have a ground plane that is very steep, even
	// though we don't have a groundentity
	// slide along the steep plane
	if(pml.ground_plane)
	{
		PM_ClipVelocity(pml.velocity_linear, pml.ground_trace.plane._normal, pml.velocity_linear, PM_OVERCLIP);
	}

	PM_StepSlideMove(true);

	/*
	vec3_c wishvel = pml.forward*pm->cmd.forwardmove + pml.right*pm->cmd.sidemove;
	wishvel[2] = 0;

	PM_AddCurrents(wishvel);

	vec3_c wishdir = wishvel;
	vec_t wishspeed = wishdir.normalize();
	
	// clamp to server defined max speed
	vec_t maxspeed = (pm->ps->pm_flags & PMF_DUCKED) ? pm_duckspeed : pm_maxspeed;

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
			if(pml.velocity_linear[2] > 0)
			{
				pml.velocity_linear[2] -= pm->ps->gravity * pml.frametime;
				
				if(pml.velocity_linear[2] < 0)
					pml.velocity_linear[2]  = 0;
			}
			else
			{
				pml.velocity_linear[2] += pm->ps->gravity * pml.frametime;
				
				if(pml.velocity_linear[2] > 0)
					pml.velocity_linear[2]  = 0;
			}
		}
		
		PM_StepSlideMove(false);
	}
	else if(pml.walking)
	{	
		// walking on ground
		pml.velocity_linear[2] = 0; //!!! this is before the accel
		
		PM_Accelerate(wishdir, wishspeed, pm_accelerate);

		//fix for negative trigger_gravity fields
		if(pm->ps->gravity > 0)
		{
			pml.velocity_linear[2] = 0;
		}
		else
		{
			pml.velocity_linear[2] -= pm->ps->gravity * pml.frametime;
		}

		
		if(!pml.velocity_linear[0] && !pml.velocity_linear[1])
			return;
		
		PM_StepSlideMove(false);
	}
	else
	{	
		// not on ground, so little effect on velocity
		if(pm_airaccelerate)
			PM_AirAccelerate(wishdir, wishspeed, pm_accelerate);
		else
			PM_Accelerate(wishdir, wishspeed, 1);
		
		// add gravity
		pml.velocity_linear[2] -= pm->ps->gravity * pml.frametime;
		PM_StepSlideMove(false);
	}
	*/
}

static void	PM_CheckOnGround()
{
	vec3_c point(pml.origin[0], pml.origin[1], pml.origin[2] - 0.25);
		
	trace_t trace = pm->boxTrace(pml.origin, pm->bbox, point);
	pml.ground_trace = trace;

	// if the trace didn't hit anything, we are in free fall
	if(!trace.ent || trace.fraction == 1)
	{
		pm->groundentity = NULL;
		pm->ps->pm_flags &= ~PMF_ON_GROUND;
		
		pml.walking = false;
		pml.ground_plane = false;
		return;
	}

	// check if getting thrown off the ground
	if(pml.velocity_linear[2] > 0 && pml.velocity_linear.dotProduct(trace.plane._normal[2]> 10))
	{
		pm->groundentity = NULL;
		pm->ps->pm_flags &= ~PMF_ON_GROUND;

		pml.walking = false;
		pml.ground_plane = false;
		return;
	}

	// slopes that are too steep will not be considered onground
	if(trace.plane._normal[2] < PM_MIN_STEP_NORMAL)
	{
		// FIXME: if they can't slide down the slope, let them
		// walk (sharp crevices)
		pm->groundentity = NULL;
		pm->ps->pm_flags &= ~PMF_ON_GROUND;
	
		pml.walking = false;
		pml.ground_plane = true;
		return;
	}
	
	pm->groundentity = trace.ent;
	
	pml.walking = true;
	pml.ground_plane = true;
		
	// hitting solid ground will end a waterjump
	if(pm->ps->pm_flags & PMF_TIME_WATERJUMP)
	{
		pm->ps->pm_flags &= ~(PMF_TIME_WATERJUMP | PMF_TIME_LAND | PMF_TIME_TELEPORT);
		pm->ps->pm_time = 0;
	}

	if(!(pm->ps->pm_flags & PMF_ON_GROUND))
	{	
		// just hit the ground
		pm->ps->pm_flags |= PMF_ON_GROUND;

		// don't do landing time if we were just going down a slope
		if(pml.velocity_linear[2] < -200)
		{
			pm->ps->pm_flags |= PMF_TIME_LAND;
				
			// don't allow another jump for a little while
			if(pml.velocity_linear[2] < -400)
				pm->ps->pm_time = 25;	
			else
				pm->ps->pm_time = 18;
		}
	}

#if 0	// don't reset the z velocity for slopes
	if(trace.fraction < 1.0 && trace.ent && pml.velocity[2] < 0)
		pml.velocity[2] = 0;
#endif

	PM_AddTouchEntity(trace.ent);
}

static void	PM_CheckWaterLevel()
{
	pm->watertype = 0;
	pm->waterlevel = 0;

	vec3_c point(pml.origin[0], pml.origin[1], pml.origin[2] - MINS_Z + 1);	
	int cont = pm->pointContents(point);

	if(cont & MASK_WATER)
	{
		int sample2 = (int)(pm->viewheight - MINS_Z);
		int sample1 = sample2 / 2;
	
		pm->waterlevel = 1;
		pm->watertype = cont;
		
		point[2] = pml.origin[2] + MINS_Z + sample1;
		cont = pm->pointContents(point);
		
		if(cont & MASK_WATER)
		{
			pm->waterlevel = 2;
			
			point[2] = pml.origin[2] + MINS_Z + sample2;
			cont = pm->pointContents(point);
			
			if(cont & MASK_WATER)
			{
				pm->waterlevel = 3;
			}
		}
	}
}

static bool	PM_CheckJump()
{
	if(pm->ps->pm_flags & PMF_TIME_LAND)
	{	
		// hasn't been long enough since landing to jump again
		return false;
	}

	if(pm->cmd.upmove < 10)
	{	
		// not holding jump
		pm->ps->pm_flags &= ~PMF_JUMP_HELD;
		return false;
	}

	// must wait for jump to be released
	if(pm->ps->pm_flags & PMF_JUMP_HELD)
		return false;

	if(pm->ps->pm_type == PM_DEAD)
		return false;

	if(pm->waterlevel >= 2)
	{	
		// swimming, not jumping
		pm->groundentity = NULL;
#if 0
		if(pml.velocity_linear[2] <= -300)
			return;

		if(pm->watertype == X_CONT_WATER)
			pml.velocity_linear[2] = 100;
			
		else if(pm->watertype == X_CONT_SLIME)
			pml.velocity_linear[2] = 80;
			
		else
			pml.velocity_linear[2] = 50;
#endif
		return false;
	}

	if(pm->groundentity == NULL)
		return false;		// in air, so no effect

	pm->ps->pm_flags |= PMF_JUMP_HELD;
	pm->groundentity = NULL;
	
	pml.velocity_linear[2] += 270;
	pml.walking = false;
	pml.ground_plane = false;
	
	if(pml.velocity_linear[2] < 270)
		pml.velocity_linear[2] = 270;

	return true;
}

void	PM_CheckSpecialMovement()
{
	vec3_c	spot;
	int	cont;
	vec3_c	flatforward;
	trace_t	trace;

	if(pm->ps->pm_time)
		return;

	pml.ladder = false;

	// check for ladder
	flatforward[0] = pml.forward[0];
	flatforward[1] = pml.forward[1];
	flatforward[2] = 0;
	flatforward.normalize();

	Vector3_MA(pml.origin, 1, flatforward, spot);
	trace = pm->boxTrace(pml.origin, pm->bbox, spot);
	
	if((trace.fraction < 1) && (trace.pos_flags & X_SURF_LADDER))
		pml.ladder = true;

	// check for water jump
	if(pm->waterlevel != 2)
		return;

	Vector3_MA(pml.origin, 30, flatforward, spot);
	spot[2] += 4;
	cont = pm->pointContents(spot);
	
	if(!(cont & X_CONT_SOLID))
		return;

	spot[2] += 16;
	cont = pm->pointContents(spot);
	if(cont)
		return;
	// jump out of water
	pml.velocity_linear = flatforward * 50;
	pml.velocity_linear[2] = 350;

	pm->ps->pm_flags |= PMF_TIME_WATERJUMP;
	pm->ps->pm_time = 255;
}

static void	PM_FlyMove()
{
#if 1
	// similar to q3a style
	vec3_c		wishvel;
	vec3_c		wishdir;
	float		wishspeed;

	// normal slowdown
	PM_Friction();

	vec_t scale = PM_CmdScale(pm->cmd);

	// user intentions
	wishvel = ((pml.forward * pm->cmd.forwardmove) * scale) + ((pml.right * pm->cmd.rightmove) * scale);
	wishvel[2] += pm->cmd.upmove * scale;

	wishdir = wishvel;
	wishspeed = wishdir.normalize();

	PM_Accelerate(wishdir, wishspeed, pm->ps->accelerate_spectator);

//	PM_StepSlideMove(false);

	pml.origin += pml.velocity_linear * pml.frametime;
	
	pml.velocity_angular[0] = 0;
	pml.velocity_angular[1] = 0;
	pml.velocity_angular[2] = 0;

#else
	// old quake2 way
	float	currentspeed, addspeed, accelspeed;
	vec3_c		wishvel;
	vec3_c		wishdir;
	float		wishspeed;
	float		scale;

	// friction
	PM_AirFriction();
	
	scale = PM_CmdScale(pm->cmd);

	// user intentions
	pml.forward.normalize();
	pml.right.normalize();

	wishvel = ((pml.forward * pm->cmd.forwardmove) * scale) + ((pml.right * pm->cmd.sidemove) * scale);
	wishvel[2] += pm->cmd.upmove * scale;

	wishdir = wishvel;
	wishspeed = wishdir.normalize();	

	// accelerate
	if(wishspeed > pm_maxspeed)
	{
		wishvel.scale(pm_maxspeed / wishspeed);
		wishspeed = pm_maxspeed;
	}

	currentspeed = pml.velocity_linear.dotProduct(wishdir);
	addspeed = wishspeed - currentspeed;
	
	if(addspeed <= 0)
		return;
	
	accelspeed = pm_accelerate * pml.frametime * wishspeed;
	if(accelspeed > addspeed)
		accelspeed = addspeed;
	
	pml.velocity_linear += (wishdir * accelspeed);
	wishdir = pml.velocity_linear;
	wishdir.normalize();

	// move
	pml.origin += pml.velocity_linear * pml.frametime;
	
	pml.velocity_angular[0] = 0;
	pml.velocity_angular[1] = 0;
	pml.velocity_angular[2] = 0;
#endif
}

/*
void	PM_RollMove()
{
	vec3_c forward(false), right(false);
	
	forward[0] = pml.forward[0];
	forward[1] = pml.forward[1];
	forward[2] = 0;
	forward.normalize();
	
	right[0] = pml.right[0];
	right[1] = pml.right[1];
	right[2] = 0;
	right.normalize();
	
	vec_t factor = 4.0f;

	pml.velocity_angular[1] = forward[0] * pm->cmd.forwardmove * pml.frametime * factor;
	pml.velocity_angular[0] =-forward[1] * pm->cmd.forwardmove * pml.frametime * factor;
	
	pml.velocity_angular[0] +=-right[1] * pm->cmd.sidemove * pml.frametime * factor;
	pml.velocity_angular[1] += right[0] * pm->cmd.sidemove * pml.frametime * factor;
	
	pml.velocity_angular[2] = 0;
}
*/

static void	PM_CheckDuck()
{
	pm->bbox._mins[0] = -15;
	pm->bbox._mins[1] = -15;
	pm->bbox._mins[2] = 0;

	pm->bbox._maxs[0] = 15;
	pm->bbox._maxs[1] = 15;

	if(pm->ps->pm_type == PM_DEAD)
	{
		pm->viewheight = VIEWHEIGHT_DEAD;
		pm->ps->pm_flags |= PMF_DUCKED;
		return;
	}
	else if(pm->cmd.upmove < 0 && (pm->ps->pm_flags & PMF_ON_GROUND))
	{	
		// duck
		pm->ps->pm_flags |= PMF_DUCKED;
	}
	else
	{	
		// stand up if possible
		if(pm->ps->pm_flags & PMF_DUCKED)
		{
			// try to stand up
			pm->bbox._maxs[2] = 56;
			
			trace_t trace = pm->boxTrace(pml.origin, pm->bbox, pml.origin);
			
			if(!trace.allsolid)
			{
				pm->ps->pm_flags &= ~PMF_DUCKED;
			}
		}
	}

	if(pm->ps->pm_flags & PMF_DUCKED)
	{
		pm->bbox._maxs[2] = 16;
		pm->viewheight = VIEWHEIGHT_CROUCH;
	}
	else
	{
		pm->bbox._maxs[2] = 64;
		pm->viewheight = VIEWHEIGHT_DEFAULT;
	}
}

static void	PM_WalkMove()
{
	if(pm->waterlevel > 2 && pml.forward.dotProduct(pml.ground_trace.plane._normal) > 0)
	{
		// start swimming
		PM_WaterMove();
		return;
	}

	if(PM_CheckJump())
	{
		if(pm->waterlevel > 1)
			PM_WaterMove();
		else
			PM_AirMove();
		
		return;
	}

	vec3_c		wishvel;
	vec3_c		wishdir;
	float		wishspeed;

	// normal slowdown
	PM_Friction();

	vec_t scale = PM_CmdScale(pm->cmd);

	// project moves down to flat plane
	pml.forward[2] = 0;
	pml.right[2] = 0;

	// project the forward and right directions onto the ground plane
	PM_ClipVelocity(pml.forward, pml.ground_trace.plane._normal, pml.forward, PM_OVERCLIP);
	PM_ClipVelocity(pml.right, pml.ground_trace.plane._normal, pml.right, PM_OVERCLIP);

	pml.forward.normalize();
	pml.right.normalize();

	wishvel = (pml.forward * pm->cmd.forwardmove) + (pml.right * pm->cmd.rightmove);
	
	wishdir = wishvel;
	wishspeed = wishdir.normalize();
	wishspeed *= scale;

	PM_Accelerate(wishdir, wishspeed, pm->ps->accelerate);

	vec_t speed = pml.velocity_linear.length();

	// slide along the ground plane
	PM_ClipVelocity(pml.velocity_linear, pml.ground_trace.plane._normal, pml.velocity_linear, PM_OVERCLIP);

	// don't decrease velocity when going up or down a slope
	pml.velocity_linear.normalize();
	pml.velocity_linear *= speed;

	// don't do anything if standing still
	if(!pml.velocity_linear[0] && !pml.velocity_linear[1])
		return;

	PM_StepSlideMove(false);
}

static void	PM_DeadMove()
{
	float	forward;

	if(!pm->groundentity)
		return;

	// extra friction
	forward = pml.velocity_linear.length() - 20;
	
	if(forward <= 0)
	{
		pml.velocity_linear.clear();
	}
	else
	{
		pml.velocity_linear.normalize();
		pml.velocity_linear.scale(forward);
	}
}

/*
static bool	PM_GoodPosition()
{
	trace_t	trace;
	vec3_c	origin, end;

	if(pm->ps->pm_type == PM_SPECTATOR)
		return true;

	origin[i] = end[i] = pm->ps->origin[i];// mul 0.125;

	trace = pm->boxTrace(origin, pm->bbox, end);

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
		pm->ps->velocity[i] = (int)(pml.velocity[i]*8);

	for(i=0; i<3; i++)
	{
		if(pml.origin[i] >= 0)
			sign[i] = 1;
		else 
			sign[i] = -1;
		
		pm->ps->origin[i] = (int)(pml.origin[i]*8);
		
		if(pm->ps->origin[i]*0.125 == pml.origin[i])
			sign[i] = 0;
	}
	Vector3_Copy(pm->ps->origin, base);

	// try all combinations
	for(j=0; j<8; j++)
	{
		bits = jitterbits[j];
		
		Vector3_Copy(base, pm->ps->origin);
		
		for(i=0; i<3; i++)
		{
			if(bits & (1<<i) )
				pm->ps->origin[i] += sign[i];
		}

		if(PM_GoodPosition())
			return;
	}
	*/
	
	pm->ps->origin = pml.origin;
	pm->ps->velocity_linear = pml.velocity_linear;
	pm->ps->velocity_angular = pml.velocity_angular;

	// go back to the last position
	//pm->ps->origin = pml.origin_prev;
	//Vector3_Clear (pm->ps->velocity);
	//Com_Printf("using previous_origin\n");	
}


void	PM_InitialSnapPosition()
{
/*
	static const vec_t offset[3] = {0, -1, 1};

	vec3_c base = pm->ps->origin;

	for(int z=0; z<3; z++) 
	{
		pm->ps->origin[2] = base[2] + offset[z];
		
		for(int y=0; y<3; y++) 
		{
			pm->ps->origin[1] = base[1] + offset[ y ];
			
			for(int x=0; x<3; x++) 
			{
				pm->ps->origin[0] = base[0] + offset[ x ];
				
				if(PM_GoodPosition())
				{
					pml.origin[0] = pm->ps->origin[0];// mul 0.125;
					pml.origin[1] = pm->ps->origin[1];// mul 0.125;
					pml.origin[2] = pm->ps->origin[2];// mul 0.125;
					
					pml.origin_prev = pm->ps->origin;
					return;
				}
			}
		}
	}

	trap_Com_Printf("Bad InitialSnapPosition\n");
*/
}


static void	PM_UpdateViewAngles()
{
	if(pm->ps->pm_flags & PMF_TIME_TELEPORT)
	{
		pm->viewangles[YAW] = pm->cmd.angles[YAW] + pm->ps->delta_angles[YAW];
		pm->viewangles[PITCH] = 0;
		pm->viewangles[ROLL] = 0;
	}
	else
	{
		// circularly clamp the angles with deltas
		pm->viewangles = pm->cmd.angles + pm->ps->delta_angles;
	
		// don't let the player look up or down more than 90 degrees
		if(pm->viewangles[PITCH] > 89 && pm->viewangles[PITCH] < 180)
			pm->viewangles[PITCH] = 89;
		
		else if(pm->viewangles[PITCH] < 271 && pm->viewangles[PITCH] >= 180)
			pm->viewangles[PITCH] = 271;
	}
	
	Angles_ToVectors(pm->viewangles, pml.forward, pml.right, pml.up);
}

static void	PM_DropTimers()
{
	// drop misc timing counter
	if(pm->ps->pm_time)
	{
		int msec = pm->cmd.msec >> 3;
		
		if(!msec)
			msec = 1;

		if(msec >= pm->ps->pm_time) 
		{
			pm->ps->pm_flags &= ~(PMF_TIME_WATERJUMP | PMF_TIME_LAND | PMF_TIME_TELEPORT);
			pm->ps->pm_time = 0;
		}
		else
		{
			pm->ps->pm_time -= msec;
		}
	}

	//TODO drop animation counter
}

void 	BG_PMove(pmove_t *pmove)
{
	pm = pmove;

	// clear results
	pm->touchents.clear();
	pm->viewangles.clear();
	pm->viewheight = 0;
	pm->groundentity = NULL;
	pm->watertype = 0;
	pm->waterlevel = 0;

	// clear all pmove local vars
	memset(&pml, 0, sizeof(pml));

	// convert origin and velocity to float values
	pml.origin		= pml.origin_prev = pm->ps->origin;
	pml.velocity_linear	= pm->ps->velocity_linear;
	pml.velocity_angular	= pm->ps->velocity_angular;

	// convert command time
	pml.frametime = pm->cmd.msec * 0.001;

	PM_UpdateViewAngles();

	if(pm->ps->pm_type == PM_SPECTATOR)
	{
		PM_CheckDuck();	
		PM_FlyMove();
		PM_SnapPosition();
		return;
	}
	
	if(pm->ps->pm_type >= PM_DEAD)
	{
		pm->cmd.forwardmove = 0;
		pm->cmd.rightmove = 0;
		pm->cmd.upmove = 0;
	}

	if(pm->ps->pm_type == PM_FREEZE)
		return;		// no movement at all

	// set watertype, and waterlevel
	PM_CheckWaterLevel();
	
	// set mins, maxs, and viewheight
	PM_CheckDuck();

	// set groundentity
	PM_CheckOnGround();

	if(pm->ps->pm_type == PM_DEAD)
	{
		PM_DeadMove();
	}

	// check for special movement like on ladders
//	PM_CheckSpecialMovement();

	// drop timing counters
	PM_DropTimers();

	if(pm->ps->pm_flags & PMF_TIME_TELEPORT)
	{	
		// teleport pause stays exactly in place
	}
	else if(pm->ps->pm_flags & PMF_TIME_WATERJUMP)
	{
		PM_WaterJumpMove();
	}
	else if(pm->waterlevel > 1)
	{
		// swimming
		PM_WaterMove();
	}
	else if(pml.walking)
	{
		PM_WalkMove();
	}
	else
	{
		// airborne
		PM_AirMove();
	}

//	TODO PM_Animate();

	// set groundentity, watertype, and waterlevel for final spot
	PM_CheckOnGround();
	PM_CheckWaterLevel();
	
	// weapon animations
//	TODO PM_Weapons();

	// footstep events / legs animations
//	TODO PM_FootSteps();

	// entering / leaving water splashes
//	TODO PM_WaterEvents();

	PM_SnapPosition();
}



