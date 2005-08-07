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

	bool		walking;
	bool		ground_plane;
	trace_t		ground_trace;

	bool		ladder;
};

static pmove_t*	pm;
static pml_t	pml;


// movement parameters
static float	pm_stopspeed = 100;
static float	pm_maxspeed = 300;
static float	pm_duckspeed = 100;

static float	pm_accelerate = 10;
static float	pm_airaccelerate = 0;
static float	pm_wateraccelerate = 6;

static float	pm_friction = 6;
static float	pm_waterfriction = 1;
static float	pm_airfriction = 3;


/*
==================
PM_ClipVelocity

Slide off of the impacting object
==================
*/
static void	PM_ClipVelocity(const vec3_c &in, const vec3_c &normal, vec3_t out, vec_t overbounce)
{
	vec_t backoff = in.dotProduct(normal) * overbounce;
	
	/*
	if(backoff < 0)
	{
		backoff *= overbounce;
	}
	else
	{
		backoff /= overbounce;
	}
	*/

	for(int i=0; i<3; i++)
	{
		vec_t change = normal[i] * backoff;
		
		out[i] = in[i] - change;
		
		if(out[i] > -PM_STOP_EPSILON && out[i] < PM_STOP_EPSILON)
			out[i] = 0;
	}
}

/*
==================
PM_SlideMove
==================
*/
void	PM_SlideMove()
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
	
	numbumps = 4;
	
	Vector3_Copy(pml.velocity_linear, primal_velocity);
	numplanes = 0;
	
	time_left = pml.frametime;

	for(bumpcount=0; bumpcount<numbumps; bumpcount++)
	{
		Vector3_MA(pml.origin, time_left, pml.velocity_linear, end);

		trace = pm->boxTrace(pml.origin, pm->bbox, end);

		if(trace.allsolid)
		{
			// entity is trapped in another solid
			pml.velocity_linear[2] = 0;	// don't build up falling damage
			return;
		}

		if(trace.fraction > 0)
		{
			// actually covered some distance
			pml.origin = trace.pos;
			numplanes = 0;
		}

		if(trace.fraction == 1)
			 break;		// moved the entire distance

		// save entity for contact
		if(trace.ent)
		{
			pm->touchents.push_back(trace.ent);
		}
		
		time_left -= time_left * trace.fraction;

		// slide along this plane
		if(numplanes >= PM_MAX_CLIP_PLANES)
		{	
			// this shouldn't really happen
			pml.velocity_linear.clear();
			break;
		}

		Vector3_Copy(trace.plane._normal, planes[numplanes]);
		numplanes++;


		// modify original_velocity so it parallels all of the clip planes
		for(i=0; i<numplanes; i++)
		{
			PM_ClipVelocity(pml.velocity_linear, planes[i], pml.velocity_linear, 1.01);
			
			for(j=0; j<numplanes; j++)
			{
				if(j != i)
				{
					if(pml.velocity_linear.dotProduct(planes[j]) < 0)
						break;	// not ok
				}
			}
						
			if(j == numplanes)
				break;
		}
		
		if(i != numplanes)
		{
			// go along this plane
		}
		else
		{	
			// go along the crease
			if(numplanes != 2)
			{
				pml.velocity_linear.clear();
				break;
			}
			
			dir.crossProduct(planes[0], planes[1]);
			d = pml.velocity_linear.dotProduct(dir);
			pml.velocity_linear = dir * d;
		}

		//
		// if velocity is against the original velocity, stop dead
		// to avoid tiny occilations in sloping corners
		//
		if(pml.velocity_linear.dotProduct(primal_velocity) <= 0)
		{
			pml.velocity_linear.clear();
			break;
		}
	}

	if(pm->s.pm_time)
	{
		pml.velocity_linear = primal_velocity;
	}
}


static void 	PM_StepSlideMove()
{
#if 1
	const vec3_c start_o = pml.origin;
	const vec3_c start_v = pml.velocity_linear;

	PM_SlideMove();
	
	const vec3_c down_o = pml.origin;
	const vec3_c down_v = pml.velocity_linear;

	vec3_c up = start_o;
	up[2] += PM_STEPSIZE;

	trace_t trace = pm->boxTrace(up, pm->bbox, up);
	if(trace.allsolid)
		return;		// can't step up

	// try sliding above
	pml.origin = up;
	pml.velocity_linear = start_v;

	PM_SlideMove();

	// push down the final amount
	vec3_c down = pml.origin;
	down[2] -= PM_STEPSIZE;
	
	trace = pm->boxTrace(pml.origin, pm->bbox, down);
	
	if(!trace.allsolid)
	{
		pml.origin = trace.pos;
	}
	
#if 0
	Vector3_Subtract (pml.origin, up, delta);
	up_dist = Vector3_DotProduct (delta, start_v);

	Vector3_Subtract (down_o, start_o, delta);
	down_dist = Vector3_DotProduct (delta, start_v);
#else
	up = pml.origin;

	// decide which one went farther
   	vec_t down_dist = (down_o[0] - start_o[0])*(down_o[0] - start_o[0]) + (down_o[1] - start_o[1])*(down_o[1] - start_o[1]);
	vec_t up_dist = (up[0] - start_o[0])*(up[0] - start_o[0]) + (up[1] - start_o[1])*(up[1] - start_o[1]);
#endif

	if(down_dist > up_dist || trace.plane._normal[2] < PM_MIN_STEP_NORMAL)
	{
		pml.origin = down_o;
		pml.velocity_linear = down_v;
		return;
	}
	
	//!! special case, if we were walking along a plane, then we need to copy the Z over
	pml.velocity_linear[2] = down_v[2];
#endif
}


/*
==================
PM_Friction

Handles both ground friction and water friction
==================
*/
static void 	PM_Friction()
{
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
			vec_t control = speed < pm_stopspeed ? pm_stopspeed : speed;
			drop += control * pm_friction * pml.frametime;
		}
	}

	// apply water friction
	if(pm->waterlevel >= 1 && !pml.ladder)
	{
		drop += speed * pm_waterfriction * pm->waterlevel * pml.frametime;
	}
		
	// scale the velocity
	vec_t newspeed = speed - drop;
	newspeed = X_max(0, newspeed);
	newspeed /= speed;
	
	pml.velocity_linear.scale(newspeed);
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
		vec_t control = speed < pm_stopspeed ? pm_stopspeed : speed;
		vec_t drop = control * pm_airfriction * pml.frametime;

		// scale the velocity
		vec_t newspeed = speed - drop;
		newspeed = X_max(0, newspeed);
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
static void	PM_Accelerate(const vec3_c &wishdir, vec_t wishspeed, vec_t accel)
{
	vec_t		addspeed, accelspeed, currentspeed;

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
	vec_t		addspeed, accelspeed, currentspeed, wishspd = wishspeed;
		
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

static void	PM_AddCurrents(vec3_t	wishvel)
{
	//
	// account for ladders
	//
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

static void	PM_WaterMove()
{
	//
	// user intentions
	//
	vec3_c wishvel = pml.forward*pm->cmd.forwardmove + pml.right*pm->cmd.sidemove;

	if(!pm->cmd.forwardmove && !pm->cmd.sidemove && !pm->cmd.upmove)
		wishvel[2] -= 60;		// drift towards bottom
	else
		wishvel[2] += pm->cmd.upmove;

	PM_AddCurrents(wishvel);

	vec3_c wishdir = wishvel;
	vec_t wishspeed = wishdir.normalize();

	if(wishspeed > pm_maxspeed)
	{
		wishspeed = pm_maxspeed / wishspeed;
		wishvel *= wishspeed;
		wishspeed = pm_maxspeed;
	}
	
	wishspeed *= 0.5;

	PM_Accelerate(wishdir, wishspeed, pm_wateraccelerate);

	PM_StepSlideMove();
}


static void	PM_AirMove()
{
	vec3_c wishvel = pml.forward*pm->cmd.forwardmove + pml.right*pm->cmd.sidemove;
	wishvel[2] = 0;

	PM_AddCurrents(wishvel);

	vec3_c wishdir = wishvel;
	vec_t wishspeed = wishdir.normalize();
	
	//
	// clamp to server defined max speed
	//
	vec_t maxspeed = (pm->s.pm_flags & PMF_DUCKED) ? pm_duckspeed : pm_maxspeed;

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
				pml.velocity_linear[2] -= pm->s.gravity * pml.frametime;
				
				if(pml.velocity_linear[2] < 0)
					pml.velocity_linear[2]  = 0;
			}
			else
			{
				pml.velocity_linear[2] += pm->s.gravity * pml.frametime;
				
				if(pml.velocity_linear[2] > 0)
					pml.velocity_linear[2]  = 0;
			}
		}
		
		PM_StepSlideMove();
	}
	else if(pml.walking)
	{	
		// walking on ground
		pml.velocity_linear[2] = 0; //!!! this is before the accel
		
		PM_Accelerate(wishdir, wishspeed, pm_accelerate);

		//fix for negative trigger_gravity fields
		if(pm->s.gravity > 0)
		{
			pml.velocity_linear[2] = 0;
		}
		else
		{
			pml.velocity_linear[2] -= pm->s.gravity * pml.frametime;
		}

		
		if(!pml.velocity_linear[0] && !pml.velocity_linear[1])
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
		pml.velocity_linear[2] -= pm->s.gravity * pml.frametime;
		PM_StepSlideMove();
	}
}

static void	PM_CheckOnGround()
{
	vec3_c point(pml.origin[0], pml.origin[1], pml.origin[2] - 0.25);	
		
	trace_t trace = pm->boxTrace(pml.origin, pm->bbox, point);
	pml.ground_trace = trace;
		
	if(!trace.ent || (trace.plane._normal[2] < PM_MIN_STEP_NORMAL && !trace.startsolid))
	//if(trace.nohit)
	{
		pm->groundentity = NULL;
		pm->s.pm_flags &= ~PMF_ON_GROUND;
		pml.walking = false;
		return;
	}
	
	pm->groundentity = trace.ent;
	pml.walking = true;
		
	// hitting solid ground will end a waterjump
	if(pm->s.pm_flags & PMF_TIME_WATERJUMP)
	{
		pm->s.pm_flags &= ~(PMF_TIME_WATERJUMP | PMF_TIME_LAND | PMF_TIME_TELEPORT);
		pm->s.pm_time = 0;
	}

	if(!(pm->s.pm_flags & PMF_ON_GROUND))
	{	
		// just hit the ground
		pm->s.pm_flags |= PMF_ON_GROUND;

		// don't do landing time if we were just going down a slope
		if(pml.velocity_linear[2] < -200)
		{
			pm->s.pm_flags |= PMF_TIME_LAND;
				
			// don't allow another jump for a little while
			if(pml.velocity_linear[2] < -400)
				pm->s.pm_time = 25;	
			else
				pm->s.pm_time = 18;
		}
	}

#if 0
	if(trace.fraction < 1.0 && trace.ent && pml.velocity[2] < 0)
		pml.velocity[2] = 0;
#endif
}

static void	PM_CheckWaterLevel()
{
	//FIXME replace MINS_Z
	pm->watertype = 0;
	pm->waterlevel = 0;

	vec3_c point(pml.origin[0], pml.origin[1], pml.origin[2] - 23);	
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

static void	PM_CheckJump()
{
	if(pm->s.pm_flags & PMF_TIME_LAND)
	{	
		// hasn't been long enough since landing to jump again
		return;
	}

	if(pm->cmd.upmove < 10)
	{	
		// not holding jump
		pm->s.pm_flags &= ~PMF_JUMP_HELD;
		return;
	}

	// must wait for jump to be released
	if(pm->s.pm_flags & PMF_JUMP_HELD)
		return;

	if(pm->s.pm_type == PM_DEAD)
		return;

	if(pm->waterlevel >= 2)
	{	
		// swimming, not jumping
		pm->groundentity = NULL;
#if 1
		if(pml.velocity_linear[2] <= -300)
			return;

		if(pm->watertype == X_CONT_WATER)
			pml.velocity_linear[2] = 100;
			
		else if(pm->watertype == X_CONT_SLIME)
			pml.velocity_linear[2] = 80;
			
		else
			pml.velocity_linear[2] = 50;
#endif
		return;
	}

	if(pm->groundentity == NULL)
		return;		// in air, so no effect

	pm->s.pm_flags |= PMF_JUMP_HELD;
	pm->groundentity = NULL;
	
	pml.velocity_linear[2] += 270;
	pml.walking = false;
	
	if(pml.velocity_linear[2] < 270)
		pml.velocity_linear[2] = 270;
}

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

	pm->s.pm_flags |= PMF_TIME_WATERJUMP;
	pm->s.pm_time = 255;
}


float	PM_CmdScale(const usercmd_t &cmd)
{
	int max = (int)X_fabs(cmd.forwardmove);
	
	if(X_fabs(cmd.sidemove) > max)
		max = (int)X_fabs(cmd.sidemove);
		
	if(X_fabs(cmd.upmove) > max)
		max = (int)X_fabs(cmd.upmove);
	
	if(max <= 0)
		return 0;
		
	float total = X_sqrt(cmd.forwardmove*cmd.forwardmove + cmd.sidemove*cmd.sidemove + cmd.upmove*cmd.upmove);
	float scale = (float)pml.velocity_linear.length() * max / (127.0 * total);
	
	return scale;
}

static void	PM_FlyMove(bool doclip)
{
	float	currentspeed, addspeed, accelspeed;
	vec3_c		wishvel;
	vec3_c		wishdir;
	float		wishspeed;
	float		scale;

	// friction
	PM_AirFriction();
	
	scale = 1.0; //PM_CmdScale(pm->cmd);

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
	if(!doclip)
	{
		pml.origin += pml.velocity_linear * pml.frametime;
	}
	else
	{
		trace_t trace = pm->boxTrace(pml.origin, pm->bbox, (pml.origin + (pml.velocity_linear * pml.frametime)));
		
		pml.origin = trace.pos;
	}
	
	
	pml.velocity_angular[0] = 0;
	pml.velocity_angular[1] = 0;
	pml.velocity_angular[2] = 0;
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
	pm->bbox._mins[0] = -16;
	pm->bbox._mins[1] = -16;
	pm->bbox._mins[2] = 0;

	pm->bbox._maxs[0] = 16;
	pm->bbox._maxs[1] = 16;

	if(pm->s.pm_type == PM_DEAD)
	{
		pm->s.pm_flags |= PMF_DUCKED;
	}
	else if(pm->cmd.upmove < 0 && (pm->s.pm_flags & PMF_ON_GROUND))
	{	
		// duck
		pm->s.pm_flags |= PMF_DUCKED;
	}
	else
	{	
		// stand up if possible
		if(pm->s.pm_flags & PMF_DUCKED)
		{
			// try to stand up
			pm->bbox._maxs[2] = 56;
			
			trace_t trace = pm->boxTrace(pml.origin, pm->bbox, pml.origin);
			
			if(!trace.allsolid)
			{
				pm->s.pm_flags &= ~PMF_DUCKED;
			}
		}
	}

	if(pm->s.pm_flags & PMF_DUCKED)
	{
		pm->bbox._maxs[2] = 16;
		pm->viewheight = VIEWHEIGHT_CROUCH;
	}
	else
	{
		pm->bbox._maxs[2] = 56;
		pm->viewheight = VIEWHEIGHT_DEFAULT;
	}
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


static void	PM_UpdateViewAngles()
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

static void	PM_DropTimers()
{
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
}

/*
================
Com_Pmove

Can be called by either the server game or the client game
================
*/
void 	Com_Pmove(pmove_t *pmove)
{
	pm = pmove;

	// clear results
	pm->touchents.clear();
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

	PM_UpdateViewAngles();

	if(pm->s.pm_type == PM_SPECTATOR)
	{
		pm->viewheight = 22;
	
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
	
	// set mins, maxs, and viewheight
	PM_CheckDuck();
	
//	if(pm->snapinitial)
//		PM_InitialSnapPosition();

	// set groundentity
	PM_CheckOnGround();
	
	// set watertype, and waterlevel
	PM_CheckWaterLevel();

	if(pm->s.pm_type == PM_DEAD)
		PM_DeadMove();

	// check for special movement like on ladders
	PM_CheckSpecialMovement();

	// drop timing counters
	PM_DropTimers();

	if(pm->s.pm_flags & PMF_TIME_TELEPORT)
	{	
		// teleport pause stays exactly in place
	}
	else if(pm->s.pm_flags & PMF_TIME_WATERJUMP)
	{	
		// waterjump has no control, but falls
		pml.velocity_linear[2] -= pm->s.gravity * pml.frametime;
		
		if(pml.velocity_linear[2] < 0)
		{
			// cancel as soon as we are falling down again
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
		{
			PM_WaterMove();
		}
		else 
		{
			vec3_c angles = pm->viewangles;
			
			if(angles[PITCH] > 180)
				angles[PITCH] = angles[PITCH] - 360;
			angles[PITCH] /= 3;

			Angles_ToVectors(angles, pml.forward, pml.right, pml.up);

			PM_AirMove();
		}
	}

	// set groundentity, watertype, and waterlevel for final spot
	PM_CheckOnGround();
	PM_CheckWaterLevel();
	
	PM_SnapPosition();
}



