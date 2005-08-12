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

static const int	PM_STEPSIZE = 18;
//static const vec_t	PM_STOP_EPSILON = 0.1;
static const vec_t	PM_MIN_STEP_NORMAL = 0.7;		// can't step up onto very steep slopes
static const int	PM_MAX_CLIP_PLANES = 5;
static const vec_t	PM_OVERCLIP = 1.001f;


player_move_c::player_move_c
(
	const usercmd_t &cmd,
	player_state_t*	ps,
	trace_t (*traceAABB)(const vec3_c &start, const aabb_c &aabb, const vec3_c &end),
	int	(*pointContents)(const vec3_c &point)
)
{
	clear();

	_cmd			= cmd;
	_ps			= ps;
	_traceAABB		= traceAABB;
	_pointContents		= pointContents;
}

void	player_move_c::clear()
{
//	_cmd.clear();

//	_ps			= NULL;	
	
//	_taceAABB		= NULL;
//	_pointContents		= NULL;
	
	_touchents.clear();
		
	_viewheight		= 0;
		
	_aabb.zero();
		
	_watertype		= 0;
	_waterlevel		= 0;

	_impact_speed		= 0;

	_forward.clear();
	_right.clear();
	_up.clear();

	_frametime		= 0;

	_walking		= false;
	_ground_plane		= false;
	_ground_trace		= trace_t();
	_ground_entity		= NULL;

	_ladder			= false;
}

void 	player_move_c::runPMove()
{
	// clear results
	clear();

	// convert command time
	_frametime = _cmd.msec * 0.001;

	updateViewAngles();

	if(_ps->pm_type >= PM_DEAD)
	{
		_cmd.forwardmove = 0;
		_cmd.rightmove = 0;
		_cmd.upmove = 0;
	}

	if(_ps->pm_type == PM_SPECTATOR)
	{
		checkDuck();	
		spectatorMove();
		dropTimers();
		return;
	}

	if(_ps->pm_type == PM_FREEZE)
		return;		// no movement at all

	// set watertype, and waterlevel
	checkWaterLevel();
	
	// set mins, maxs, and viewheight
	checkDuck();

	// set groundentity
	checkOnGround();

	if(_ps->pm_type == PM_DEAD)
	{
		deadMove();
	}

	// check for special movement like on ladders
//	checkSpecialMovement();

	// drop timing counters
	dropTimers();

	if(_ps->pm_flags & PMF_TIME_TELEPORT)
	{	
		// teleport pause stays exactly in place
	}
	else if(_ps->pm_flags & PMF_TIME_WATERJUMP)
	{
//		trap_Com_Printf("player_move_c::runPMove: water jump\n");

		waterJumpMove();
	}
	else if(_waterlevel > 1)
	{
//		trap_Com_Printf("player_move_c::runPMove: water move\n");

		// swimming
		waterMove();
	}
	else if(_walking)
	{
//		trap_Com_Printf("player_move_c::runPMove: walk move\n");

		walkMove();
	}
	else
	{
//		trap_Com_Printf("player_move_c::runPMove: air move\n");

		// airborne
		airMove();
	}

//	TODO PM_Animate();

	// set groundentity, watertype, and waterlevel for final spot
	checkOnGround();
	checkWaterLevel();
	
	// weapon animations
//	TODO PM_Weapons();

	// footstep events / legs animations
//	TODO PM_FootSteps();

	// entering / leaving water splashes
//	TODO PM_WaterEvents();
}

/*
============
PM_CmdScale

Returns the scale factor to apply to cmd movements
This allows the clients to use axial -127 to 127 values for all directions
without getting a sqrt(2) distortion in speed.
============
*/
float	player_move_c::commandScale()
{
	int max = abs(_cmd.forwardmove);
	
	if(abs(_cmd.rightmove) > max)
		max = abs(_cmd.rightmove);
		
	if(abs(_cmd.upmove) > max)
		max = abs(_cmd.upmove);
	
	if(max <= 0)
		return 0;
		
	float total = X_sqrt(_cmd.forwardmove*_cmd.forwardmove + _cmd.rightmove*_cmd.rightmove + _cmd.upmove*_cmd.upmove);
	float scale = (float)320 * max / (127.0 * total);
	
	return scale;
}

void	player_move_c::addTouchEntity(entity_c* ent)
{
	if(!ent)
		return;

	std::deque<entity_c*>::const_iterator ir = std::find(_touchents.begin(), _touchents.end(), static_cast<entity_c*>(ent));

	if(ir == _touchents.end())
		_touchents.push_back(ent);
}

void	player_move_c::updateViewAngles()
{
	if(_ps->pm_flags & PMF_TIME_TELEPORT)
	{
		_ps->view_angles[YAW] = _cmd.angles[YAW] + _ps->delta_angles[YAW];
		_ps->view_angles[PITCH] = 0;
		_ps->view_angles[ROLL] = 0;
	}
	else
	{
		// circularly clamp the angles with deltas
		_ps->view_angles = _cmd.angles + _ps->delta_angles;
	
		// don't let the player look up or down more than 90 degrees
		if(_ps->view_angles[PITCH] > 89 && _ps->view_angles[PITCH] < 180)
			_ps->view_angles[PITCH] = 89;
		
		else if(_ps->view_angles[PITCH] < 271 && _ps->view_angles[PITCH] >= 180)
			_ps->view_angles[PITCH] = 271;
	}
	
	Angles_ToVectors(_ps->view_angles, _forward, _right, _up);
}

/*
==================
PM_ClipVelocity

Slide off of the impacting object
==================
*/
void	player_move_c::clipVelocity(const vec3_c &in, const vec3_c &normal, vec3_c &out, vec_t overbounce)
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
bool	player_move_c::slideMove(bool gravity)
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
	
	velocity_primal = _ps->velocity_linear;

	if(gravity)
	{
		velocity_end = _ps->velocity_linear;
		velocity_end[2] -= _ps->gravity * _frametime;
		_ps->velocity_linear[2] = (_ps->velocity_linear[2] + velocity_end[2]) * 0.5;
		velocity_primal[2] = velocity_end[2];
		
		if(_ground_plane)
		{
			// slide along the ground plane
			clipVelocity(_ps->velocity_linear, _ground_trace.plane._normal, _ps->velocity_linear, PM_OVERCLIP);
		}
	}
	
	time_left = _frametime;

	// never turn against the ground plane
	if(_ground_plane)
	{
		numplanes = 1;
		planes[0] = _ground_trace.plane._normal;
	}
	else
	{
		numplanes = 0;
	}

	// never turn against original velocity
	planes[numplanes] = _ps->velocity_linear;
	planes[numplanes].normalize();
	numplanes++;

	for(bumpcount=0; bumpcount<numbumps; bumpcount++)
	{
		end = _ps->origin + _ps->velocity_linear * time_left;

		// see if we can make it there
		trace = _traceAABB(_ps->origin, _aabb, end);

		if(trace.allsolid)
		{
			// entity is trapped in another solid
			_ps->velocity_linear[2] = 0;	// don't build up falling damage
			return true;
		}

		if(trace.fraction > 0)
		{
			// actually covered some distance
			_ps->origin = trace.pos;
		}

		if(trace.fraction == 1)
		{
			 break;		// moved the entire distance
		}

		// save entity for contact
		addTouchEntity(trace.ent);
		
		time_left -= time_left * trace.fraction;

		// slide along this plane
		if(numplanes >= PM_MAX_CLIP_PLANES)
		{	
			// this shouldn't really happen
			_ps->velocity_linear.clear();
			return true;
		}

		// if this is the same plane we hit before, nudge velocity
		// out along it, which fixes some epsilon issues with
		// non-axial planes
		for(i=0; i<numplanes; i++)
		{
			if(trace.plane._normal.dotProduct(planes[i]) > 0.99)
			{
				_ps->velocity_linear += trace.plane._normal;
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
			vec_t into = _ps->velocity_linear.dotProduct(planes[i]);
			if(into >= 0.1)
				continue;

			// see how hard we are hitting things
			if(-into > _impact_speed) 
			{
				_impact_speed = -into;
			}

			// slide along the plane
			clipVelocity(_ps->velocity_linear, planes[i], velocity_clip, PM_OVERCLIP);

			// slide along the plane
			clipVelocity(velocity_end, planes[i], velocity_clip_end, PM_OVERCLIP);
			
			// see if there is a second plane that the new move enters
			for(j=0; j<numplanes; j++)
			{
				if(j == i)
					continue;

				if(velocity_clip.dotProduct(planes[j]) >= 0.1)
					continue;	// move doesn't interact with the plane
				

				// try clipping the move to the plane
				clipVelocity(velocity_clip, planes[j], velocity_clip, PM_OVERCLIP);
				clipVelocity(velocity_clip_end, planes[j], velocity_clip_end, PM_OVERCLIP);

				// see if it goes back into the first clip plane
				if(velocity_clip.dotProduct(planes[i]) >= 0)
					continue;

				// slide the original velocity along the crease
				dir.crossProduct(planes[i], planes[j]);
				dir.normalize();
				
				d = dir.dotProduct(_ps->velocity_linear);
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
					_ps->velocity_linear.clear();
					return true;
				}
			}

			// if we have fixed all interactions, try another move
			_ps->velocity_linear = velocity_clip;
			velocity_end = velocity_clip_end;
			break;
		}
	}

	if(gravity)
		_ps->velocity_linear = velocity_end;

	// don't change velocity if in a timer (FIXME: is this correct?)
	if(_ps->pm_time)
		_ps->velocity_linear = velocity_primal;

	return (bumpcount != 0);
}


void	player_move_c::stepSlideMove(bool gravity)
{
	const vec3_c start_o = _ps->origin;
	const vec3_c start_v = _ps->velocity_linear;

	if(slideMove(gravity) == 0)
		return;		// we got exactly where we wanted to go first try

	vec3_c down = start_o;
	down[2] -= PM_STEPSIZE;
	
	trace_t trace = _traceAABB(start_o, _aabb, down);

	// never step up when you still have up velocity
	if(_ps->velocity_linear[2] > 0 && (trace.fraction == 1.0 || trace.plane._normal.dotProduct(vec3_c(0.0, 0.0, 1.0)) < PM_MIN_STEP_NORMAL))
		return;
	
	const vec3_c down_o = _ps->origin;
	const vec3_c down_v = _ps->velocity_linear;

	vec3_c up = start_o;
	up[2] += PM_STEPSIZE;

	// test the player position if they were a stepheight higher
	trace = _traceAABB(start_o, _aabb, up);
	if(trace.allsolid)
		return;		// can't step up

	vec_t step_size = trace.pos[2] - start_o[2];

	// try sliding above
	_ps->origin = trace.pos;
	_ps->velocity_linear = start_v;

	slideMove(gravity);

	// push down the final amount
	down = _ps->origin;
	down[2] -= step_size;
	
	trace = _traceAABB(_ps->origin, _aabb, down);
	if(!trace.allsolid)
	{
		_ps->origin = trace.pos;
	}

	if(trace.fraction < 1.0)
	{
		clipVelocity(_ps->velocity_linear, trace.plane._normal, _ps->velocity_linear, PM_OVERCLIP);
	}
	
	/*
	up = _origin;

	// decide which one went farther
   	vec_t down_dist = (down_o[0] - start_o[0])*(down_o[0] - start_o[0]) + (down_o[1] - start_o[1])*(down_o[1] - start_o[1]);
	vec_t up_dist = (up[0] - start_o[0])*(up[0] - start_o[0]) + (up[1] - start_o[1])*(up[1] - start_o[1]);

	if(down_dist > up_dist || trace.plane._normal[2] < PM_MIN_STEP_NORMAL)
	{
		_origin = down_o;
		_velocity_linear = down_v;
		return;
	}
	
	//!! special case, if we were walking along a plane, then we need to copy the Z over
	_velocity_linear[2] = down_v[2];
*/
}


/*
==================
PM_Friction

Handles both ground friction and water friction
==================
*/
void	player_move_c::applyFriction()
{
	vec3_c vec = _ps->velocity_linear;

	// ignore slope movement
	if(_walking)
		vec[2] = 0;

	vec_t speed = vec.length();
	if(speed < 1.0)
	{
		_ps->velocity_linear[0] = 0;
		_ps->velocity_linear[1] = 0;
		// FIXME: still have z friction underwater?
		return;
	}

	vec_t drop = 0;

	// apply ground friction
	if(_waterlevel <= 1)
	{
		if(_walking && !(_ground_trace.pos_flags & X_SURF_SLICK) || _ladder)
		{
			vec_t control = speed < _ps->speed_stop ? _ps->speed_stop : speed;
			drop += control * _ps->friction * _frametime;
		}
	}

	// apply water friction
	if(_waterlevel >= 1 && !_ladder)
	{
		drop += speed * _ps->friction_water * _waterlevel * _frametime;
	}

	// apply spectator friction
	if(_ps->pm_type == PM_SPECTATOR)
	{
		drop += speed * _ps->friction_spectator * _frametime;
	}
		
	// scale the velocity
	vec_t newspeed = speed - drop;
	if(newspeed <= 0)
	{
		newspeed = 0;
		_ps->velocity_linear.clear();
	}
	else
	{
		newspeed /= speed;
		_ps->velocity_linear *= newspeed;
	}
}


void	player_move_c::applyAirFriction()
{
	vec_t speed = _ps->velocity_linear.length();
	
	if(speed < 1)
	{
		_ps->velocity_linear.clear();
	}
	else
	{
		vec_t control = speed < _ps->speed_stop ? _ps->speed_stop : speed;
		vec_t drop = control * _ps->friction_air * _frametime;

		// scale the velocity
		vec_t newspeed = speed - drop;
		if(newspeed <= 0)
		{
			newspeed = 0;
			_ps->velocity_linear.clear();
		}
		else
		{
			newspeed /= speed;
			_ps->velocity_linear *= newspeed;
		}
	}
}


/*
==============
PM_Accelerate

Handles user intended acceleration
==============
*/
void	player_move_c::accelerate(const vec3_c &wishdir, vec_t wishspeed, vec_t accel)
{
	vec_t	addspeed, accelspeed, currentspeed;

	currentspeed = _ps->velocity_linear.dotProduct(wishdir);
	addspeed = wishspeed - currentspeed;
	
	if(addspeed <= 0)
		return;
	
	accelspeed = accel * _frametime * wishspeed;
	
	if(accelspeed > addspeed)
		accelspeed = addspeed;
		
	_ps->velocity_linear += (wishdir * accelspeed);
}

void	player_move_c::airAccelerate(const vec3_c &wishdir, vec_t wishspeed, vec_t accel)
{
	vec_t	addspeed, accelspeed, currentspeed, wishspd = wishspeed;
		
	if(wishspd > 30)
		wishspd = 30;
	
	currentspeed = _ps->velocity_linear.dotProduct(wishdir);
	
	addspeed = wishspd - currentspeed;
	
	if(addspeed <= 0)
		return;
	
	accelspeed = accel * wishspeed * _frametime;
	
	if(accelspeed > addspeed)
		accelspeed = addspeed;
	
	_ps->velocity_linear += (wishdir * accelspeed);
}

void	player_move_c::addCurrents(vec3_c &wishvel)
{
	/*

	// account for ladders
	if(_ladder && fabs(_ps->velocity_linear[2]) <= 200)
	{
		if((_viewangles[PITCH] <= -15) && (_cmd.forwardmove > 0))
			wishvel[2] = 200;
		
		else if((_viewangles[PITCH] >= 15) && (_cmd.forwardmove > 0))
			wishvel[2] = -200;
		
		else if(_cmd.upmove > 0)
			wishvel[2] = 200;
		
		else if(_cmd.upmove < 0)
			wishvel[2] = -200;
		else
			wishvel[2] = 0;

		// limit horizontal speed when on a ladder
		X_clamp(wishvel[0], -25, 25);
		X_clamp(wishvel[1], -25, 25);
	}
	*/
}

void	player_move_c::waterJumpMove()
{
	stepSlideMove(true);

	// waterjump has no control, but falls
	_ps->velocity_linear[2] -= _ps->gravity * _frametime;
		
	if(_ps->velocity_linear[2] < 0)
	{
		// cancel as soon as we are falling down again
		_ps->pm_flags &= ~(PMF_TIME_WATERJUMP | PMF_TIME_LAND | PMF_TIME_TELEPORT);
		_ps->pm_time = 0;
	}
}

void	player_move_c::waterMove()
{
	if(checkWaterJump())
	{
		waterJumpMove();
		return;
	}

	vec3_c		wishvel;
	vec3_c		wishdir;
	float		wishspeed;

	applyFriction();

	vec_t scale = commandScale();

	// user intentions
	if(!scale)
	{
		// sink towards bottom
		wishvel.set(0, 0, -60);
	}
	else
	{
		wishvel = ((_forward * _cmd.forwardmove) * scale) + ((_right * _cmd.rightmove) * scale);
		wishvel[2] += _cmd.upmove * scale;
	}

	wishdir = wishvel;
	wishspeed = wishdir.normalize();

	//TODO swim scale

	accelerate(wishdir, wishspeed, _ps->accelerate_water);

	//TODO check slopes

	slideMove(false);

	
	/*
	// user intentions
	vec3_c wishvel = _forward*_cmd.forwardmove + _right*_cmd.rightmove;

	if(!_cmd.forwardmove && !_cmd.rightmove && !_cmd.upmove)
		wishvel[2] -= 60;		// drift towards bottom
	else
		wishvel[2] += _cmd.upmove;

	addCurrents(wishvel);

	vec3_c wishdir = wishvel;
	vec_t wishspeed = wishdir.normalize();

	if(wishspeed > _ps->speed_max)
	{
		wishspeed = _ps->speed_max / wishspeed;
		wishvel *= wishspeed;
		wishspeed = _ps->speed_max;
	}
	
	wishspeed *= 0.5;

	accelerate(wishdir, wishspeed, _ps->accelerate_water);

	stepSlideMove(true);
	*/
}

void	player_move_c::airMove()
{
//	trap_Com_Printf("player_move_c::airMove()\n");

	vec3_c		wishvel;
	vec3_c		wishdir;
	float		wishspeed;

	// normal slowdown
	applyFriction();

	vec_t scale = commandScale();

	// project moves down to flat plane
	_forward[2] = 0;
	_forward.normalize();

	_right[2] = 0;
	_right.normalize();

	wishvel = (_forward * _cmd.forwardmove) + (_right * _cmd.rightmove);
	
	wishdir = wishvel;
	wishspeed = wishdir.normalize();
	wishspeed *= scale;

	// not on ground, so little effect on velocity
	airAccelerate(wishdir, wishspeed, _ps->accelerate_air);
		
	// we may have a ground plane that is very steep, even
	// though we don't have a groundentity
	// slide along the steep plane
	if(_ground_plane)
	{
		clipVelocity(_ps->velocity_linear, _ground_trace.plane._normal, _ps->velocity_linear, PM_OVERCLIP);
	}

	stepSlideMove(true);

	/*
	vec3_c wishvel = _forward*_cmd.forwardmove + _right*_cmd.sidemove;
	wishvel[2] = 0;

	PM_AddCurrents(wishvel);

	vec3_c wishdir = wishvel;
	vec_t wishspeed = wishdir.normalize();
	
	// clamp to server defined max speed
	vec_t maxspeed = (_ps->pm_flags & PMF_DUCKED) ? pm_duckspeed : pm_maxspeed;

	if(wishspeed > maxspeed)
	{
		wishspeed = maxspeed/wishspeed;
		Vector3_Scale (wishvel,wishspeed, wishvel);
		wishspeed = maxspeed;
	}
	
	if(_ladder)
	{
		accelerate(wishdir, wishspeed, pm_accelerate);
		
		if(!wishvel[2])
		{
			if(_velocity_linear[2] > 0)
			{
				_velocity_linear[2] -= _ps->gravity * _frametime;
				
				if(_velocity_linear[2] < 0)
					_velocity_linear[2]  = 0;
			}
			else
			{
				_velocity_linear[2] += _ps->gravity * _frametime;
				
				if(_velocity_linear[2] > 0)
					_velocity_linear[2]  = 0;
			}
		}
		
		stepSlideMove(false);
	}
	else if(_walking)
	{	
		// walking on ground
		_velocity_linear[2] = 0; //!!! this is before the accel
		
		accelerate(wishdir, wishspeed, pm_accelerate);

		//fix for negative trigger_gravity fields
		if(_ps->gravity > 0)
		{
			_velocity_linear[2] = 0;
		}
		else
		{
			_velocity_linear[2] -= _ps->gravity * _frametime;
		}

		
		if(!_velocity_linear[0] && !_velocity_linear[1])
			return;
		
		stepSlideMove(false);
	}
	else
	{	
		// not on ground, so little effect on velocity
		if(pm_airaccelerate)
			PM_AirAccelerate(wishdir, wishspeed, pm_accelerate);
		else
			accelerate(wishdir, wishspeed, 1);
		
		// add gravity
		_velocity_linear[2] -= _ps->gravity * _frametime;
		stepSlideMove(false);
	}
	*/
}

void	player_move_c::spectatorMove()
{
#if 1
	// similar to q3a style
	vec3_c		wishvel;
	vec3_c		wishdir;
	float		wishspeed;

	// normal slowdown
	applyFriction();

	vec_t scale = commandScale();

	// user intentions
	wishvel = ((_forward * _cmd.forwardmove) * scale) + ((_right * _cmd.rightmove) * scale);
	wishvel[2] += _cmd.upmove * scale;

	wishdir = wishvel;
	wishspeed = wishdir.normalize();

	accelerate(wishdir, wishspeed, _ps->accelerate_spectator);

	stepSlideMove(false);

//	_ps->origin += _ps->velocity_linear * _frametime;

#else
	// old quake2 way
	float	currentspeed, addspeed, accelspeed;
	vec3_c		wishvel;
	vec3_c		wishdir;
	float		wishspeed;
	float		scale;

	// friction
	PM_AirFriction();
	
	scale = commandScale(_cmd);

	// user intentions
	_forward.normalize();
	_right.normalize();

	wishvel = ((_forward * _cmd.forwardmove) * scale) + ((_right * _cmd.sidemove) * scale);
	wishvel[2] += _cmd.upmove * scale;

	wishdir = wishvel;
	wishspeed = wishdir.normalize();	

	// accelerate
	if(wishspeed > pm_maxspeed)
	{
		wishvel.scale(pm_maxspeed / wishspeed);
		wishspeed = pm_maxspeed;
	}

	currentspeed = _velocity_linear.dotProduct(wishdir);
	addspeed = wishspeed - currentspeed;
	
	if(addspeed <= 0)
		return;
	
	accelspeed = pm_accelerate * _frametime * wishspeed;
	if(accelspeed > addspeed)
		accelspeed = addspeed;
	
	_velocity_linear += (wishdir * accelspeed);
	wishdir = _velocity_linear;
	wishdir.normalize();

	// move
	_origin += _velocity_linear * _frametime;
	
	_velocity_angular[0] = 0;
	_velocity_angular[1] = 0;
	_velocity_angular[2] = 0;
#endif
}

void	player_move_c::checkOnGround()
{
	vec3_c point(_ps->origin[0], _ps->origin[1], _ps->origin[2] - 0.25);
		
	trace_t trace = _traceAABB(_ps->origin, _aabb, point);
	_ground_trace = trace;

	// do something corrective if the trace starts in a solid...
	if(trace.allsolid)
	{
		if(!correctAllSolid())
			return;
	}

	// if the trace didn't hit anything, we are in free fall
	if(trace.fraction == 1)
	{
		_ps->pm_flags &= ~PMF_ON_GROUND;

		_walking = false;
		_ground_plane = false;
		_ground_entity = NULL;
		return;
	}

	// check if getting thrown off the ground
#if 0
	if(_ps->velocity_linear[2] > 0 && _ps->velocity_linear.dotProduct(trace.plane._normal[2]> 10))
	{
		_ps->pm_flags &= ~PMF_ON_GROUND;

		_walking = false;
		_ground_plane = false;
		_ground_entity = NULL;
		return;
	}
#endif

	// slopes that are too steep will not be considered onground
	if((trace.plane._normal[2] < PM_MIN_STEP_NORMAL) && !trace.startsolid)
	{
		// FIXME: if they can't slide down the slope, let them
		// walk (sharp crevices)

		_ps->pm_flags &= ~PMF_ON_GROUND;
		
		_walking = false;
		_ground_plane = true;
		_ground_entity = NULL;
		return;
	}
	
	_walking = true;
	_ground_plane = true;
	_ground_entity = trace.ent;
		
	// hitting solid ground will end a waterjump
	if(_ps->pm_flags & PMF_TIME_WATERJUMP)
	{
		_ps->pm_flags &= ~(PMF_TIME_WATERJUMP | PMF_TIME_LAND | PMF_TIME_TELEPORT);
		_ps->pm_time = 0;
	}

	if(!(_ps->pm_flags & PMF_ON_GROUND))
	{	
		// just hit the ground
		_ps->pm_flags |= PMF_ON_GROUND;

		// don't do landing time if we were just going down a slope
		if(_ps->velocity_linear[2] < -200)
		{
			_ps->pm_flags |= PMF_TIME_LAND;
			_ps->pm_time = 250;
				
			// don't allow another jump for a little while
			//if(_ps->velocity_linear[2] < -400)
			//	_ps->pm_time = 25;	
			//else
			//	_ps->pm_time = 18;
		}
	}

#if 0	// don't reset the z velocity for slopes
	if(trace.fraction < 1.0 && trace.ent && _velocity[2] < 0)
		_velocity[2] = 0;
#endif

	addTouchEntity(trace.ent);
}

void	player_move_c::checkWaterLevel()
{
	_watertype = 0;
	_waterlevel = 0;

	vec3_c point(_ps->origin[0], _ps->origin[1], _ps->origin[2] + MINS_Z + 1);	
	int cont = _pointContents(point);

	if(cont & MASK_WATER)
	{
		int sample2 = (int)(_viewheight - MINS_Z);
		int sample1 = sample2 / 2;
	
		_waterlevel = 1;
		_watertype = cont;
		
		point[2] = _ps->origin[2] + MINS_Z + sample1;
		cont = _pointContents(point);
		
		if(cont & MASK_WATER)
		{
			_waterlevel = 2;
			
			point[2] = _ps->origin[2] + MINS_Z + sample2;
			cont = _pointContents(point);
			
			if(cont & MASK_WATER)
			{
				_waterlevel = 3;
			}
		}
	}
}

bool	player_move_c::checkJump()
{
#if 0
	if(_ps->pm_flags & PMF_TIME_LAND)
	{	
		// hasn't been long enough since landing to jump again
		return false;
	}
#endif

	if(_cmd.upmove < 10)
	{	
		// not holding jump
		_ps->pm_flags &= ~PMF_JUMP_HELD;
		return false;
	}

	// must wait for jump to be released
	if(_ps->pm_flags & PMF_JUMP_HELD)
	{
		// clear upmove so cmdscale doesn't lower running speed
		_cmd.upmove = 0;
		return false;
	}

	if(_ps->pm_type == PM_DEAD)
		return false;

	/*
	if(_waterlevel >= 2)
	{	
		// swimming, not jumping
		_groundentity = NULL;
#if 0
		if(_velocity_linear[2] <= -300)
			return;

		if(_watertype == X_CONT_WATER)
			_velocity_linear[2] = 100;
			
		else if(_watertype == X_CONT_SLIME)
			_velocity_linear[2] = 80;
			
		else
			_velocity_linear[2] = 50;
#endif
		return false;
	}
	*/

	if(_ground_entity == NULL)
		return false;		// in air, so no effect
	
	// jumping away
	_ps->pm_flags |= PMF_JUMP_HELD;
	_ps->velocity_linear[2] += 270;

//	if(_ps->velocity_linear[2] < 270)
//		_ps->velocity_linear[2] = 270;

	_walking = false;
	_ground_plane = false;
	_ground_entity = NULL;

	//TODO add event

	//TODO animate

	return true;
}

bool	player_move_c::checkWaterJump()
{
	//TODO
	return false;
}

/*
void	checkSpecialMovement()
{
	vec3_c	spot;
	int	cont;
	vec3_c	flatforward;
	trace_t	trace;

	if(_ps->pm_time)
		return;

	_ladder = false;

	// check for ladder
	flatforward[0] = _forward[0];
	flatforward[1] = _forward[1];
	flatforward[2] = 0;
	flatforward.normalize();

	Vector3_MA(_origin, 1, flatforward, spot);
	trace = _traceAABB(_origin, _aabb, spot);
	
	if((trace.fraction < 1) && (trace.pos_flags & X_SURF_LADDER))
		_ladder = true;

	// check for water jump
	if(_waterlevel != 2)
		return;

	Vector3_MA(_origin, 30, flatforward, spot);
	spot[2] += 4;
	cont = _pointContents(spot);
	
	if(!(cont & X_CONT_SOLID))
		return;

	spot[2] += 16;
	cont = _pointContents(spot);
	if(cont)
		return;
	// jump out of water
	_velocity_linear = flatforward * 50;
	_velocity_linear[2] = 350;

	_ps->pm_flags |= PMF_TIME_WATERJUMP;
	_ps->pm_time = 255;
}
*/

void	player_move_c::checkDuck()
{
	_aabb._mins[0] = -15;
	_aabb._mins[1] = -15;
	_aabb._mins[2] = 0;

	_aabb._maxs[0] = 15;
	_aabb._maxs[1] = 15;

	if(_ps->pm_type == PM_SPECTATOR)
	{
		_aabb._maxs[2] = 16;
		_viewheight = VIEWHEIGHT_DEAD;
		return;
	}
	else if(_ps->pm_type == PM_DEAD)
	{
		_viewheight = VIEWHEIGHT_DEAD;
		_ps->pm_flags |= PMF_DUCKED;
		return;
	}
	else if(_cmd.upmove < 0)// && (_ps->pm_flags & PMF_ON_GROUND))
	{	
		// duck
		_ps->pm_flags |= PMF_DUCKED;
	}
	else
	{	
		// stand up if possible
		if(_ps->pm_flags & PMF_DUCKED)
		{
			// try to stand up
			_aabb._maxs[2] = 72;
			
			trace_t trace = _traceAABB(_ps->origin, _aabb, _ps->origin);
			
			if(!trace.allsolid)
			{
				_ps->pm_flags &= ~PMF_DUCKED;
			}
		}
	}

	if(_ps->pm_flags & PMF_DUCKED)
	{
		_aabb._maxs[2] = 16;
		_viewheight = VIEWHEIGHT_CROUCH;
	}
	else
	{
		_aabb._maxs[2] = 72;
		_viewheight = VIEWHEIGHT_DEFAULT;
	}
}

void	player_move_c::walkMove()
{
	if(_waterlevel > 2 && _forward.dotProduct(_ground_trace.plane._normal) > 0)
	{
		trap_Com_Printf("player_move_c::walkMove: start swimming\n");

		// start swimming
		waterMove();
		return;
	}

	if(checkJump())
	{
		trap_Com_Printf("player_move_c::walkMove: jumping\n");

		if(_waterlevel > 1)
			waterMove();
		else
			airMove();
		return;
	}

	vec3_c		wishvel;
	vec3_c		wishdir;
	float		wishspeed;

	// normal slowdown
	applyFriction();

	vec_t scale = commandScale();

	// project moves down to flat plane
	_forward[2] = 0;
	_right[2] = 0;

	// project the forward and right directions onto the ground plane
	clipVelocity(_forward, _ground_trace.plane._normal, _forward, PM_OVERCLIP);
	clipVelocity(_right, _ground_trace.plane._normal, _right, PM_OVERCLIP);

	_forward.normalize();
	_right.normalize();

	wishvel = (_forward * _cmd.forwardmove) + (_right * _cmd.rightmove);
	
	wishdir = wishvel;
	wishspeed = wishdir.normalize();
	wishspeed *= scale;

	// clamp the speed lower if walking instead of running
	if(_cmd.buttons & BUTTON_WALK)
	{
		if(wishspeed > _ps->speed_cmd * 0.5)
			wishspeed = _ps->speed_cmd * 0.5;
	}

	//TODO clamp the speed lower if ducking

	//TODO clamp the speed lower if wading or walking on the bottom

	accelerate(wishdir, wishspeed, _ps->accelerate);

	vec_t speed = _ps->velocity_linear.length();

	// slide along the ground plane
	clipVelocity(_ps->velocity_linear, _ground_trace.plane._normal, _ps->velocity_linear, PM_OVERCLIP);

	// don't decrease velocity when going up or down a slope
	_ps->velocity_linear.normalize();
	_ps->velocity_linear *= speed;

	// don't do anything if standing still
	if(!_ps->velocity_linear[0] && !_ps->velocity_linear[1])
		return;

	stepSlideMove(false);
}

void	player_move_c::deadMove()
{
	if(!_walking)
		return;

	// extra friction
	vec_t forward = _ps->velocity_linear.length() - 20;
	forward -= 20;
	
	if(forward <= 0)
	{
		_ps->velocity_linear.clear();
	}
	else
	{
		_ps->velocity_linear.normalize();
		_ps->velocity_linear *= forward;
	}
}

void	player_move_c::dropTimers()
{
	// drop misc timing counter
	if(_ps->pm_time)
	{
		int msec = _cmd.msec >> 3;
		
		if(!msec)
			msec = 1;

		if(msec >= _ps->pm_time) 
		{
			_ps->pm_flags &= ~(PMF_TIME_WATERJUMP | PMF_TIME_LAND | PMF_TIME_TELEPORT);
			_ps->pm_time = 0;
		}
		else
		{
			_ps->pm_time -= msec;
		}
	}

	//TODO drop animation counter
}

/*
static bool	PM_GoodPosition()
{
	trace_t	trace;
	vec3_c	origin, end;

	if(_ps->pm_type == PM_SPECTATOR)
		return true;

	origin[i] = end[i] = _ps->origin[i];// mul 0.125;

	trace = _traceAABB(origin, _aabb, end);

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
/*
static void	PM_SnapPosition()
{
	int		sign[3];
	int		i, j, bits;
	vec3_t	base;
	*/
	
	// try all single bits first
	//static int jitterbits[8] = {0,4,1,2,3,5,6,7};

	// snap velocity to eigths
	/*
	for(i=0; i<3; i++)
		_ps->velocity[i] = (int)(_velocity[i]*8);

	for(i=0; i<3; i++)
	{
		if(_origin[i] >= 0)
			sign[i] = 1;
		else 
			sign[i] = -1;
		
		_ps->origin[i] = (int)(_origin[i]*8);
		
		if(_ps->origin[i]*0.125 == _origin[i])
			sign[i] = 0;
	}
	Vector3_Copy(_ps->origin, base);

	// try all combinations
	for(j=0; j<8; j++)
	{
		bits = jitterbits[j];
		
		Vector3_Copy(base, _ps->origin);
		
		for(i=0; i<3; i++)
		{
			if(bits & (1<<i) )
				_ps->origin[i] += sign[i];
		}

		if(PM_GoodPosition())
			return;
	}
	
	_ps->origin = _origin;
	_ps->velocity_linear = _velocity_linear;
	_ps->velocity_angular = _velocity_angular;

	// go back to the last position
	//_ps->origin = _origin_prev;
	//Vector3_Clear (_ps->velocity);
	//Com_Printf("using previous_origin\n");	
}
*/

bool	player_move_c::correctAllSolid()
{
	for(int x=-1; x <= 1; x++)
	{
		for(int y=-1; y <= 1; y++)
		{
			for(int z=-1; z <= 1; z++)
			{
				vec3_c point = _ps->origin;
				point[0] += (float)x;
				point[1] += (float)y;
				point[2] += (float)z;

				trace_t	trace = _traceAABB(point, _aabb, point);
				if(!trace.allsolid)
				{
					point[0] = _ps->origin[0];
					point[1] = _ps->origin[1];
					point[2] = _ps->origin[2] - 0.25;
					
					trace = _traceAABB(_ps->origin, _aabb, point);
					_ground_trace = trace;
					return true;
				}
			}
		}
	}

	_walking = false;
	_ground_plane = false;
	_ground_entity = NULL;

	return false;
}





