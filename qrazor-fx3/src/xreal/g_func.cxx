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
// xreal --------------------------------------------------------------------
#include "g_func.h"
#include "g_trigger.h"
#include "g_local.h"

/*
=========================================================

  PLATS

  movement options:

  linear
  smooth start, hard stop
  smooth start, smooth stop

  start
  end
  acceleration
  speed
  deceleration
  begin sound
  end sound
  target fired when reaching end
  wait at end

  object characteristics that use move segments
  ---------------------------------------------
  movetype_push, or movetype_stop
  action when touched
  action when blocked
  action when used
	disabled?
  auto trigger spawning


=========================================================
*/

#define PLAT_LOW_TRIGGER	1

#define	STATE_TOP		0
#define	STATE_BOTTOM		1
#define STATE_UP		2
#define STATE_DOWN		3

#define DOOR_START_OPEN		1
#define DOOR_REVERSE		2
#define DOOR_CRUSHER		4
#define DOOR_NOMONSTER		8
#define DOOR_TOGGLE		32
#define DOOR_X_AXIS		64
#define DOOR_Y_AXIS		128

enum
{
	// move interface
	THINK_BEGIN_MOVE,
	THINK_FINISH_MOVE,
	
	// func_button
	THINK_PUSH_BUTTON,
	THINK_RETURN_BUTTON,
	THINK_FINISH_BUTTON,
	
	// func_door
	THINK_CALC_MOVE_SPEED,
	THINK_SPAWN_DOOR_TRIGGER,
	THINK_HIT_TOP,
	THINK_HIT_BOTTOM,
	THINK_GO_DOWN
};




g_func_c::g_func_c()
{
	addField(g_field_c("lip", &_lip, F_INT));
	addField(g_field_c("movedir", &_s.quat, F_ANGLEHACK_TO_QUATERNION));
//	addField(g_field_c("endfunc", &_moveinfo.endfunc, F_INT, FFL_NOSPAWN));
}


//
// Support routines for movement (changes in origin using velocity)
//
void	g_func_c::calcMove(const vec3_c &dest, int endfunc)
{
	_body->setLinearVel(vec3_origin);
	
	_moveinfo.dir = dest - _s.origin;
	_moveinfo.remaining_distance = _moveinfo.dir.normalize();
	_moveinfo.endfunc = endfunc;

	if(_moveinfo.speed == _moveinfo.accel && _moveinfo.speed == _moveinfo.decel)
	{
		if(this == ((_flags & FL_TEAMSLAVE) ? _teammaster : this))
		{
			beginMove();
		}
		else
		{
			_nextthink = level.time + FRAMETIME;
			//ent->think = Move_Begin;
			_thinktype = THINK_BEGIN_MOVE;
		}
	}
	/*
	else
	{
		// accelerative
		ent->moveinfo.current_speed = 0;
		ent->think = Think_AccelMove;
		ent->nextthink = level.time + FRAMETIME;
	}
	*/
}

void	g_func_c::beginMove()
{
	float	frames;

	if((_moveinfo.speed * FRAMETIME) >= _moveinfo.remaining_distance)
	{
		finishMove();
		return;
	}
	
	_body->setLinearVel(_moveinfo.dir * _moveinfo.speed);
	
	frames = floor((_moveinfo.remaining_distance / _moveinfo.speed) / FRAMETIME);
	_moveinfo.remaining_distance -= frames * _moveinfo.speed * FRAMETIME;
	
	_nextthink = level.time + (frames * FRAMETIME);

	_thinktype = THINK_FINISH_MOVE;
}

void	g_func_c::finishMove()
{
	if(_moveinfo.remaining_distance == 0)
	{
		clearMove();
		return;
	}

	_body->setLinearVel(_moveinfo.dir * (_moveinfo.remaining_distance / FRAMETIME));
	
	_nextthink = level.time + FRAMETIME;

	_thinktype = _moveinfo.endfunc;	//THINK_FINISH_MOVE;
}

void	g_func_c::clearMove()
{
	_body->setLinearVel(vec3_origin);
}

//void	Think_AccelMove (g_entity_c *ent);




//
// Support routines for angular movement (changes in angle using avelocity)
//
/*
void AngleMove_Done (g_entity_c *ent)
{
	ent->avelocity.clear();
	ent->moveinfo.endfunc (ent);
}

void AngleMove_Final (g_entity_c *ent)
{
	vec3_t	move;

	if (ent->moveinfo.state == STATE_UP)
		Vector3_Subtract (ent->moveinfo.end_angles, ent->s.angles, move);
	else
		Vector3_Subtract (ent->moveinfo.start_angles, ent->s.angles, move);

	if (Vector3_Compare (move, vec3_origin))
	{
		AngleMove_Done (ent);
		return;
	}

	Vector3_Scale (move, 1.0/FRAMETIME, ent->avelocity);

	ent->think = AngleMove_Done;
	ent->nextthink = level.time + FRAMETIME;
}

void AngleMove_Begin (g_entity_c *ent)
{
	vec3_t	destdelta;
	float	len;
	float	traveltime;
	float	frames;

	// set destdelta to the vector needed to move
	if (ent->moveinfo.state == STATE_UP)
		Vector3_Subtract (ent->moveinfo.end_angles, ent->s.angles, destdelta);
	else
		Vector3_Subtract (ent->moveinfo.start_angles, ent->s.angles, destdelta);
	
	// calculate length of vector
	len = Vector3_Length (destdelta);
	
	// divide by speed to get time to reach dest
	traveltime = len / ent->moveinfo.speed;

	if (traveltime < FRAMETIME)
	{
		AngleMove_Final (ent);
		return;
	}

	frames = floor(traveltime / FRAMETIME);

	// scale the destdelta vector by the time spent traveling to get velocity
	Vector3_Scale (destdelta, 1.0 / traveltime, ent->avelocity);

	// set nextthink to trigger a think when dest is reached
	ent->nextthink = level.time + frames * FRAMETIME;
	ent->think = AngleMove_Final;
}

void AngleMove_Calc (g_entity_c *ent, void(*func)(g_entity_c*))
{
	ent->avelocity.clear();
	ent->moveinfo.endfunc = func;
	if (level.current_entity == ((ent->flags & FL_TEAMSLAVE) ? ent->teammaster : ent))
	{
		AngleMove_Begin (ent);
	}
	else
	{
		ent->nextthink = level.time + FRAMETIME;
		ent->think = AngleMove_Begin;
	}
}
*/



/*
==============
Think_AccelMove

The team has completed a frame of movement, so
change the speed for the next frame
==============
*/

//#define AccelerationDistance(target, rate)	(target * ((target / rate) + 1) / 2)

/*
void plat_CalcAcceleratedMove(moveinfo_t *moveinfo)
{
	float	accel_dist;
	float	decel_dist;

	moveinfo->move_speed = moveinfo->speed;

	if (moveinfo->remaining_distance < moveinfo->accel)
	{
		moveinfo->current_speed = moveinfo->remaining_distance;
		return;
	}

	accel_dist = AccelerationDistance (moveinfo->speed, moveinfo->accel);
	decel_dist = AccelerationDistance (moveinfo->speed, moveinfo->decel);

	if ((moveinfo->remaining_distance - accel_dist - decel_dist) < 0)
	{
		float	f;

		f = (moveinfo->accel + moveinfo->decel) / (moveinfo->accel * moveinfo->decel);
		moveinfo->move_speed = (-2 + X_sqrt(4 - 4 * f * (-2 * moveinfo->remaining_distance))) / (2 * f);
		decel_dist = AccelerationDistance (moveinfo->move_speed, moveinfo->decel);
	}

	moveinfo->decel_distance = decel_dist;
};
*/

/*
void plat_Accelerate (moveinfo_t *moveinfo)
{
	// are we decelerating?
	if (moveinfo->remaining_distance <= moveinfo->decel_distance)
	{
		if (moveinfo->remaining_distance < moveinfo->decel_distance)
		{
			if (moveinfo->next_speed)
			{
				moveinfo->current_speed = moveinfo->next_speed;
				moveinfo->next_speed = 0;
				return;
			}
			if (moveinfo->current_speed > moveinfo->decel)
				moveinfo->current_speed -= moveinfo->decel;
		}
		return;
	}

	// are we at full speed and need to start decelerating during this move?
	if (moveinfo->current_speed == moveinfo->move_speed)
		if ((moveinfo->remaining_distance - moveinfo->current_speed) < moveinfo->decel_distance)
		{
			float	p1_distance;
			float	p2_distance;
			float	distance;

			p1_distance = moveinfo->remaining_distance - moveinfo->decel_distance;
			p2_distance = moveinfo->move_speed * (1.0 - (p1_distance / moveinfo->move_speed));
			distance = p1_distance + p2_distance;
			moveinfo->current_speed = moveinfo->move_speed;
			moveinfo->next_speed = moveinfo->move_speed - moveinfo->decel * (p2_distance / distance);
			return;
		}

	// are we accelerating?
	if (moveinfo->current_speed < moveinfo->speed)
	{
		float	old_speed;
		float	p1_distance;
		float	p1_speed;
		float	p2_distance;
		float	distance;

		old_speed = moveinfo->current_speed;

		// figure simple acceleration up to move_speed
		moveinfo->current_speed += moveinfo->accel;
		if (moveinfo->current_speed > moveinfo->speed)
			moveinfo->current_speed = moveinfo->speed;

		// are we accelerating throughout this entire move?
		if ((moveinfo->remaining_distance - moveinfo->current_speed) >= moveinfo->decel_distance)
			return;

		// during this move we will accelrate from current_speed to move_speed
		// and cross over the decel_distance; figure the average speed for the
		// entire move
		p1_distance = moveinfo->remaining_distance - moveinfo->decel_distance;
		p1_speed = (old_speed + moveinfo->move_speed) / 2.0;
		p2_distance = moveinfo->move_speed * (1.0 - (p1_distance / p1_speed));
		distance = p1_distance + p2_distance;
		moveinfo->current_speed = (p1_speed * (p1_distance / distance)) + (moveinfo->move_speed * (p2_distance / distance));
		moveinfo->next_speed = moveinfo->move_speed - moveinfo->decel * (p2_distance / distance);
		return;
	}

	// we are at constant velocity (move_speed)
	return;
};
*/

/*
void Think_AccelMove (g_entity_c *ent)
{
	ent->moveinfo.remaining_distance -= ent->moveinfo.current_speed;

	if (ent->moveinfo.current_speed == 0)		// starting or blocked
		plat_CalcAcceleratedMove(&ent->moveinfo);

	plat_Accelerate (&ent->moveinfo);

	// will the entire move complete on next frame?
	if (ent->moveinfo.remaining_distance <= ent->moveinfo.current_speed)
	{
		Move_Final (ent);
		return;
	}

	Vector3_Scale (ent->moveinfo.dir, ent->moveinfo.current_speed*10, ent->velocity);
	ent->nextthink = level.time + FRAMETIME;
	ent->think = Think_AccelMove;
}
*/


//void plat_go_down (g_entity_c *ent);

/*
void plat_hit_top (g_entity_c *ent)
{
	if (!(ent->flags & FL_TEAMSLAVE))
	{
		if (ent->moveinfo.sound_end)
			trap_SV_StartSound (NULL, ent, CHAN_NO_PHS_ADD+CHAN_VOICE, ent->moveinfo.sound_end, 1, ATTN_STATIC, 0);
		ent->s.sound = 0;
	}
	ent->moveinfo.state = STATE_TOP;

	ent->think = plat_go_down;
	ent->nextthink = level.time + 3;
}
*/

/*
void plat_hit_bottom (g_entity_c *ent)
{
	if (!(ent->flags & FL_TEAMSLAVE))
	{
		if (ent->moveinfo.sound_end)
			trap_SV_StartSound (NULL, ent, CHAN_NO_PHS_ADD+CHAN_VOICE, ent->moveinfo.sound_end, 1, ATTN_STATIC, 0);
		ent->s.sound = 0;
	}
	ent->moveinfo.state = STATE_BOTTOM;
}
*/

/*
void plat_go_down (g_entity_c *ent)
{
	if (!(ent->flags & FL_TEAMSLAVE))
	{
		if (ent->moveinfo.sound_start)
			trap_SV_StartSound (NULL, ent, CHAN_NO_PHS_ADD+CHAN_VOICE, ent->moveinfo.sound_start, 1, ATTN_STATIC, 0);
		ent->s.sound = ent->moveinfo.sound_middle;
	}
	ent->moveinfo.state = STATE_DOWN;
	Move_Calc (ent, ent->moveinfo.end_origin, plat_hit_bottom);
}
*/

/*
void plat_go_up (g_entity_c *ent)
{
	if (!(ent->flags & FL_TEAMSLAVE))
	{
		if (ent->moveinfo.sound_start)
			trap_SV_StartSound (NULL, ent, CHAN_NO_PHS_ADD+CHAN_VOICE, ent->moveinfo.sound_start, 1, ATTN_STATIC, 0);
		ent->s.sound = ent->moveinfo.sound_middle;
	}
	ent->moveinfo.state = STATE_UP;
	Move_Calc (ent, ent->moveinfo.start_origin, plat_hit_top);
}
*/

/*
void plat_blocked (g_entity_c *self, g_entity_c *other)
{
	if (!other->r.client)
	{
		// give it a chance to go away on it's own terms (like gibs)
		T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, 100000, 1, 0, MOD_CRUSH);
		// if it's still there, nuke it
		if (other)
			BecomeExplosion1 (other);
		return;
	}

	T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);

	if (self->moveinfo.state == STATE_UP)
		plat_go_down (self);
	else if (self->moveinfo.state == STATE_DOWN)
		plat_go_up (self);
}
*/


/*
void Use_Plat (g_entity_c *ent, g_entity_c *other, g_entity_c *activator)
{ 
	if (ent->think)
		return;		// already down
	plat_go_down (ent);
}
*/

/*
void Touch_Plat_Center (g_entity_c *ent, g_entity_c *other, cplane_c *plane, csurface_t *surf)
{
	if (!other->r.client)
		return;
		
	if (other->health <= 0)
		return;

	ent = ent->enemy;	// now point at the plat, not the trigger
	if (ent->moveinfo.state == STATE_BOTTOM)
		plat_go_up (ent);
	else if (ent->moveinfo.state == STATE_TOP)
		ent->nextthink = level.time + 1;	// the player is still on the plat, so delay going down
}
*/

/*
void plat_spawn_inside_trigger (g_entity_c *ent)
{
	g_entity_c	*trigger;
	vec3_c	tmin, tmax;

	//
	// middle trigger
	//	
	//trigger = G_Spawn();
	trigger = new g_entity_c();
	trigger->touch = Touch_Plat_Center;
	trigger->movetype = MOVETYPE_NONE;
	trigger->r.solid = SOLID_TRIGGER;
	trigger->enemy = ent;
	
	tmin[0] = ent->r.bbox._mins[0] + 25;
	tmin[1] = ent->r.bbox._mins[1] + 25;
	tmin[2] = ent->r.bbox._mins[2];

	tmax[0] = ent->r.bbox._maxs[0] - 25;
	tmax[1] = ent->r.bbox._maxs[1] - 25;
	tmax[2] = ent->r.bbox._maxs[2] + 8;

	tmin[2] = tmax[2] - (ent->pos1[2] - ent->pos2[2] + st.lip);

	if (ent->spawnflags & PLAT_LOW_TRIGGER)
		tmax[2] = tmin[2] + 8;
	
	if (tmax[0] - tmin[0] <= 0)
	{
		tmin[0] = (ent->r.bbox._mins[0] + ent->r.bbox._maxs[0]) *0.5;
		tmax[0] = tmin[0] + 1;
	}
	if (tmax[1] - tmin[1] <= 0)
	{
		tmin[1] = (ent->r.bbox._mins[1] + ent->r.bbox._maxs[1]) *0.5;
		tmax[1] = tmin[1] + 1;
	}
	
	trigger->r.bbox._mins = tmin;
	trigger->r.bbox._maxs = tmax;

	trap_SV_LinkEdict (trigger);
}
*/



/*QUAKED func_plat (0 .5 .8) ? PLAT_LOW_TRIGGER
speed	default 150

Plats are always drawn in the extended position, so they will light correctly.

If the plat is the target of another trigger or button, it will start out disabled in the extended position until it is trigger, when it will lower and become a normal plat.

"speed"	overrides default 200.
"accel" overrides default 500
"lip"	overrides default 8 pixel lip

If the "height" key is set, that will determine the amount the plat moves, instead of being implicitly determoveinfoned by the model's height.

Set "sounds" to one of the following:
1) base fast
2) chain slow
*/
/*
void SP_func_plat (g_entity_c *ent)
{
	ent->s.angles.clear();
	ent->r.solid = SOLID_BSP;
	ent->movetype = MOVETYPE_PUSH;

	G_SetModel (ent, ent->model);

	ent->blocked = plat_blocked;

	if (!ent->speed)
		ent->speed = 20;
	else
		ent->speed *= 0.1;

	if (!ent->accel)
		ent->accel = 5;
	else
		ent->accel *= 0.1;

	if (!ent->decel)
		ent->decel = 5;
	else
		ent->decel *= 0.1;

	if (!ent->dmg)
		ent->dmg = 2;

	if (!st.lip)
		st.lip = 8;

	// pos1 is the top position, pos2 is the bottom
	Vector3_Copy (ent->s.origin, ent->pos1);
	Vector3_Copy (ent->s.origin, ent->pos2);
	if (st.height)
		ent->pos2[2] -= st.height;
	else
		ent->pos2[2] -= (ent->r.bbox._maxs[2] - ent->r.bbox._mins[2]) - st.lip;

	ent->use = Use_Plat;

	plat_spawn_inside_trigger (ent);	// the "start moving" trigger	

	if (ent->targetname)
	{
		ent->moveinfo.state = STATE_UP;
	}
	else
	{
		Vector3_Copy (ent->pos2, ent->s.origin);
		trap_SV_LinkEdict (ent);
		ent->moveinfo.state = STATE_BOTTOM;
	}

	ent->moveinfo.speed = ent->speed;
	ent->moveinfo.accel = ent->accel;
	ent->moveinfo.decel = ent->decel;
	ent->moveinfo.wait = ent->wait;
	
	ent->moveinfo.start_origin = ent->pos1;
	ent->moveinfo.start_angles = ent->s.angles;
	ent->moveinfo.end_origin = ent->pos2;
	ent->moveinfo.end_angles = ent->s.angles;

	ent->moveinfo.sound_start = trap_SV_SoundIndex ("plats/pt1_strt.wav");
	ent->moveinfo.sound_middle = trap_SV_SoundIndex ("plats/pt1_mid.wav");
	ent->moveinfo.sound_end = trap_SV_SoundIndex ("plats/pt1_end.wav");
}
*/

//====================================================================

/*QUAKED func_rotating (0 .5 .8) ? START_ON REVERSE X_AXIS Y_AXIS TOUCH_PAIN STOP ANIMATED ANIMATED_FAST
You need to have an origin brush as part of this entity.  The center of that brush will be
the point around which it is rotated. It will rotate around the Z axis by default.  You can
check either the X_AXIS or Y_AXIS box to change that.

"speed" determines how fast it moves; default value is 100.
"dmg"	damage to inflict when blocked (2 default)

REVERSE will cause the it to rotate in the opposite direction.
STOP mean it will stop moving instead of pushing entities
*/


g_func_rotating_c::g_func_rotating_c()
{
	//TODO
}



void	g_func_rotating_c::blocked(g_entity_c *other)
{
	if(!_dmg)
		return;
	
	other->takeDamage(this, this, vec3_origin, other->_s.origin, vec3_origin, _dmg, 1, 0, MOD_CRUSH);
}



bool	g_func_rotating_c::touch(g_entity_c *other, const cplane_c &plane, csurface_c *surf)
{
	if(!(_spawnflags & 16))
		return true;
	
//	if(_avelocity[0] || _avelocity[1] || _avelocity[2])
//		other->takeDamage(this, this, vec3_origin, other->_s.origin, vec3_origin, _dmg, 1, 0, MOD_CRUSH);
		
	return true;
}



void	g_func_rotating_c::use(g_entity_c *other, g_entity_c *activator)
{
	vec3_c avel = _body->getAngularVel();
	
	if(!avel.isZero())
	{
		_s.index_sound = 0;
		_body->setAngularVel(vec3_c(0.0, 0, 0));
		//self->touch = NULL;
	}
	else
	{
		_s.index_sound = _moveinfo.sound_middle;
		_body->setAngularVel(_movedir * _speed);
		
		//if (self->spawnflags & 16)
		//	self->touch = rotating_touch;
	}
}


void	g_func_rotating_c::activate()
{
	_r.inuse = true;
	_r.solid = SOLID_BSP;
	
	if (_spawnflags & 32)
		_movetype = MOVETYPE_STOP;
	else
		_movetype = MOVETYPE_PUSH;

	// set the axis of rotation
	_movedir.clear();
	
	if (_spawnflags & 4)
		_movedir[2] = 1.0;
		
	else if (_spawnflags & 8)
		_movedir[0] = 1.0;
		
	else // Z_AXIS
		_movedir[1] = 1.0;

	// check for reverse rotation
	if (_spawnflags & 2)
		_movedir.negate();

	if (!_speed)
		_speed = 100;
		
	if (!_dmg)
		_dmg = 2;

	_moveinfo.sound_middle = trap_SV_SoundIndex("doors/hydro1.wav");

	//ent->use = rotating_use;
	
	//if (_dmg)
	//	_blocked = rotating_blocked;

	if(_spawnflags & 1)
		use(NULL, NULL);

	if(_spawnflags & 64)
		_s.effects |= EF_AUTOANIM_2;
		
	if (_spawnflags & 128)
		_s.effects |= EF_AUTOANIM_10;

	G_SetModel(this, _model);
}

void	SP_func_rotating(g_entity_c **entity)
{
	g_entity_c *ent = new g_func_rotating_c();
	*entity = ent;
}




/*
================================================================================
				BUTTON FUNC
================================================================================
*/

/*QUAKED func_button (0 .5 .8) ?
When a button is touched, it moves some distance in the direction of it's angle, triggers all of it's targets, waits some time, then returns to it's original position where it can be triggered again.

"angle"		determines the opening direction
"target"	all entities with a matching targetname will be used
"speed"		override the default 40 speed
"wait"		override the default 1 second wait (-1 = never return)
"lip"		override the default 4 pixel lip remaining at end of move
"health"	if set, the button must be killed instead of touched
"sounds"
1) silent
2) steam metal
3) wooden clunk
4) metallic click
5) in-out
*/

g_func_button_c::g_func_button_c()
{
	//TODO
}	



void	g_func_button_c::think()
{
	switch(_thinktype)
	{
		case THINK_BEGIN_MOVE:
			beginMove();
			break;
		
		case THINK_FINISH_MOVE:
			finishMove();
			break;
		
		case THINK_PUSH_BUTTON:
			clearMove();
			wait();
			break;
		
		case THINK_RETURN_BUTTON:
			returnButton();
			break;
		
		case THINK_FINISH_BUTTON:
			clearMove();
			finish();
			break;
			
		default:
			trap_Com_Error(ERR_DROP, "g_func_button_c::think: bad think type %i\n", _thinktype);
			break;
	}
}


void	g_func_button_c::use(g_entity_c *other, g_entity_c *activator)
{
	_activator = activator;
	
	fire();
}


bool	g_func_button_c::touch(g_entity_c *other, const cplane_c &plane, csurface_c *surf)
{
	if(!other->_r.isclient)
		return true;

	if(other->_health <= 0)
		return true;

	_activator = other;
	
	fire();
	
	return true;
}



void	g_func_button_c::die(g_entity_c *inflictor, g_entity_c *attacker, int damage, vec3_t point)
{
	if(!_health)
		return;
		
	_activator = attacker;
	_health = _max_health;
	_takedamage = DAMAGE_NO;
	
	//button_fire (self);
}


void	g_func_button_c::activate()
{
	vec3_t	abs_movedir;
	float	dist;

	_r.inuse = true;
	
	// setup rigid body
	_body->setPosition(_s.origin);
	_body->setQuaternion(_s.quat);
	//_body->setLinearVel(_velocity);
		
	// setup geom
	G_SetWorldModel(this, _model);
	
	_body->setGravityMode(0);
	
	G_SetMovedir(_s.quat, _movedir);
	
	_movetype = MOVETYPE_STOP;
	_r.solid = SOLID_BSP;

	if(_sounds != 1)
		_moveinfo.sound_start = trap_SV_SoundIndex("switches/butn2.wav");
	
	if(!_speed)
		_speed = 40;
		
	if(!_accel)
		_accel = _speed;
		
	if(!_decel)
		_decel = _speed;

	if(!_wait)
		_wait = 3;
		
	if(!_lip)
		_lip = 4;
	
	_pos1 = _s.origin;
	abs_movedir[0] = fabs(_movedir[0]);
	abs_movedir[1] = fabs(_movedir[1]);
	abs_movedir[2] = fabs(_movedir[2]);
	dist = abs_movedir[0] * _r.size[0] + abs_movedir[1] * _r.size[1] + abs_movedir[2] * _r.size[2] - _lip;
	Vector3_MA (_pos1, dist, _movedir, _pos2);

	//ent->use = button_use;
	_s.effects |= EF_AUTOANIM_TOGGLE_01_2;

	if(_health)
	{
		_max_health = _health;
		//ent->die = button_killed;
		_takedamage = DAMAGE_YES;
	}
	else if (!_targetname.length())
	{
		//ent->touch = button_touch;
	}

	_moveinfo.state = STATE_BOTTOM;

	_moveinfo.speed = _speed;
	_moveinfo.accel = _accel;
	_moveinfo.decel = _decel;
	_moveinfo.wait = _wait;
	_moveinfo.start_origin = _pos1;
	_moveinfo.start_quat = _s.quat;
	_moveinfo.end_origin = _pos2;
	_moveinfo.end_quat = _s.quat;
}




void 	g_func_button_c::finish()
{
	_moveinfo.state = STATE_BOTTOM;
	_s.effects &= ~EF_AUTOANIM_TOGGLE_23_2;
	_s.effects |= EF_AUTOANIM_TOGGLE_01_2;
}



void	g_func_button_c::returnButton()
{
	_moveinfo.state = STATE_DOWN;

	calcMove(_moveinfo.start_origin, THINK_FINISH_BUTTON);

	_s.frame = 0;

	if(_health)
		_takedamage = DAMAGE_YES;
}


void	g_func_button_c::wait()
{
	_moveinfo.state = STATE_TOP;
	_s.effects &= ~EF_AUTOANIM_TOGGLE_01_2;
	_s.effects |= EF_AUTOANIM_TOGGLE_23_2;

	G_UseTargets(this, _activator);
	
	_s.frame = 1;
	
	if(_moveinfo.wait >= 0)
	{
		_nextthink = level.time + _moveinfo.wait;
		//_think = button_return;
		_thinktype = THINK_RETURN_BUTTON;
	}
}


void	g_func_button_c::fire()
{
	if(_moveinfo.state == STATE_UP || _moveinfo.state == STATE_TOP)
		return;

	_moveinfo.state = STATE_UP;
	
	if (_moveinfo.sound_start && !(_flags & FL_TEAMSLAVE))
		trap_SV_StartSound(NULL, this, CHAN_NO_PHS_ADD+CHAN_VOICE, _moveinfo.sound_start, 1, ATTN_STATIC, 0);
		
	calcMove(_moveinfo.end_origin, THINK_PUSH_BUTTON);
}



void	SP_func_button(g_entity_c **entity)
{
	g_entity_c *ent = new g_func_button_c();
	*entity = ent;
}



/*
================================================================================
				DOOR FUNC
================================================================================
*/

/*QUAKED func_door (0 .5 .8) ? START_OPEN x CRUSHER NOMONSTER ANIMATED TOGGLE ANIMATED_FAST
-------- KEYS --------
"message"	is printed when the door is touched if it is a trigger door and it hasn't been fired yet
"angle"		determines the opening direction
"targetname" if set, no touch field will be spawned and a remote button or trigger field activates the door.
"health"	if set, door must be shot open
"speed"		movement speed (100 default)
"wait"		wait before returning (3 default, -1 = never return)
"lip"		lip remaining at end of move (8 default)
"dmg"		damage to inflict when blocked (2 default)
"sounds"
1)	silent
2)	light
3)	medium
4)	heavy
-------- SPAWNFLAGS --------
TOGGLE		wait in both the start and end states for a trigger event.
START_OPEN	the door to moves to its destination when spawned, and operate in reverse.  It is used to temporarily or permanently close off an area when triggered (not useful for touch or takedamage doors).
NOMONSTER	monsters will not trigger this door
*/


g_func_door_c::g_func_door_c()
{
	addField(g_field_c("snd_open", &_snd_open, F_STRING));
	addField(g_field_c("snd_close", &_snd_close, F_STRING));
}

void	g_func_door_c::think()
{
	switch(_thinktype)
	{
		case THINK_BEGIN_MOVE:
			beginMove();
			break;
		
		case THINK_FINISH_MOVE:
			finishMove();
			break;
			
		case THINK_CALC_MOVE_SPEED:
			calcMoveSpeed();
			break;
		
		case THINK_SPAWN_DOOR_TRIGGER:
			spawnDoorTrigger();
			break;
			
		case THINK_HIT_TOP:
			clearMove();
			hitTop();
			break;
			
		case THINK_HIT_BOTTOM:
			clearMove();
			hitBottom();
			break;
		
		case THINK_GO_DOWN:
			goDown();
			break;
		
		default:
			trap_Com_Error(ERR_DROP, "g_func_door_c::think: bad think type %i\n", _thinktype);
			break;
	}
}


void	g_func_door_c::blocked(g_entity_c *other)
{
	g_func_door_c	*ent;

	/*
	if(!other->getClient())
	{
		// give it a chance to go away on it's own terms (like gibs)
		T_Damage(other, this, this, vec3_origin, other->_s.origin, vec3_origin, 100000, 1, 0, MOD_CRUSH);
		
		// if it's still there, nuke it
		if(other)
			G_BecomeExplosion1(other);
		return;
	}
	*/

	other->takeDamage(this, this, vec3_origin, _s.origin, vec3_origin, _dmg, 1, 0, MOD_CRUSH);

	if(_spawnflags & DOOR_CRUSHER)
		return;


	// if a door has a negative wait, it would never come back if blocked,
	// so let it just squash the object to death real fast
	if (_moveinfo.wait >= 0)
	{
		if (_moveinfo.state == STATE_DOWN)
		{
			for (ent = (g_func_door_c*) _teammaster ; ent ; ent = (g_func_door_c*) ent->_teamchain)
				ent->goUp(ent->_activator);
		}
		else
		{
			for (ent = (g_func_door_c*) _teammaster ; ent ; ent = (g_func_door_c*) ent->_teamchain)
				ent->goDown();
		}
	}
}

bool	g_func_door_c::touch(g_entity_c *other, const cplane_c &plane, csurface_c *surf)
{
	if(!other->_r.isclient)
		return true;

	if(level.time < _touch_debounce_time)
		return true;
		
	trap_Com_Printf("g_func_door_c::touch: origin %s\n", _s.origin.toString());
		
	_touch_debounce_time = level.time + 5.0;

	trap_SV_CenterPrintf(other, "%s", _message.c_str());
	trap_SV_StartSound(NULL, other, CHAN_AUTO, trap_SV_SoundIndex("misc/talk1.wav"), 1, ATTN_NORM, 0);
	
	use(other, other);
	
	return true;
}

void	g_func_door_c::use(g_entity_c *other, g_entity_c *activator)
{
#if 0
	//trap_Com_Printf("g_func_door_c::use:\n");

	g_func_door_c	*ent;

	if(_flags & FL_TEAMSLAVE)
		return;

	if(_spawnflags & DOOR_TOGGLE)
	{
		if(_moveinfo.state == STATE_UP || _moveinfo.state == STATE_TOP)
		{
			// trigger all paired doors
			for(ent = this ; ent ; ent = (g_func_door_c*) ent->_teamchain)
			{
				ent->_message = "";
				//ent->_touch = NULL;
				//ent->_has_touch = false;
				ent->goDown();
			}
			return;
		}
	}
	
	// trigger all paired doors
	for(ent = this ; ent ; ent = (g_func_door_c*) ent->_teamchain)
	{
		ent->_message = "";
		//ent->touch = NULL;
		//ent->_has_touch = false;
		ent->goUp(activator);
	}
#endif
};




void	g_func_door_c::die(g_entity_c *inflictor, g_entity_c *attacker, int damage, vec3_t point)
{
	g_entity_c	*ent;

	for (ent = _teammaster ; ent ; ent = ent->_teamchain)
	{
		ent->_health = ent->_max_health;
		ent->_takedamage = DAMAGE_NO;
	}
	
	_teammaster->use(attacker, attacker);
}

void	g_func_door_c::activate()
{
	vec3_t	abs_movedir;
	
	if(_model.empty())// || _model[0] != '*')
	{
		trap_Com_Printf("g_func_door_c::activate: door has bad model '%s'\n", _model.c_str());
		remove();
		return;
	}

	_r.inuse = true;
	
	if(_targetname.length())
		trap_Com_Printf("g_func_door_c::activate: '%s'\n", _targetname.c_str());
	
	if(_sounds != 1)
	{
		_moveinfo.sound_start = trap_SV_SoundIndex("doors/dr1_strt.wav");
		_moveinfo.sound_middle = trap_SV_SoundIndex("doors/dr1_mid.wav");
		_moveinfo.sound_end = trap_SV_SoundIndex("doors/dr1_end.wav");
	}
	
	// setup rigid body
	_body->setPosition(_s.origin);
	_body->setQuaternion(_s.quat);
	//_body->setLinearVel(_velocity);
	_body->setGravityMode(0);
		
	// setup geom
	//G_SetWorldModel(this, _model);
	
	_s.index_model = trap_SV_ModelIndex(_model);
	
	
	//dMass m;
	//dMassSetZero(&m);
	//dMassSetBoxTotal(&m, 50, _r.size[0], _r.size[1], _r.size[2]);
	//_body->setMass(&m);
	
	//_body->disable();
	

	G_SetMovedir(_s.quat, _movedir);
	_movetype = MOVETYPE_PUSH;
	_r.solid = SOLID_BSP;
	
	_r.areaportal = trap_CM_GetClosestAreaPortal(_s.origin);		

	if(!_speed)
		_speed = 100;
		
	if(deathmatch->getInteger())
		_speed *= 2;
	
	if(!_accel)
		_accel = _speed;
		
	if(!_decel)
		_decel = _speed;

	if(!_wait)
		_wait = 3;
		
	if(!_lip)
		_lip = 8;
		
	if(!_dmg)
		_dmg = 2;

	// calculate second position
	//if(hasEPair("pos1") && hasEPair("pos2))
	{
		_pos1 = _s.origin;
		abs_movedir[0] = fabs(_movedir[0]);
		abs_movedir[1] = fabs(_movedir[1]);
		abs_movedir[2] = fabs(_movedir[2]);
		_moveinfo.distance = abs_movedir[0] * _r.size[0] + abs_movedir[1] * _r.size[1] + abs_movedir[2] * _r.size[2] - _lip;
		Vector3_MA(_pos1, _moveinfo.distance, _movedir, _pos2);
	}

	// if it starts open, switch the positions
	if(_spawnflags & DOOR_START_OPEN)
	{
		_s.origin = _pos2;
		_pos2 = _pos1;
		_pos1 = _s.origin;
	}
	
	//_body->setPosition(_s.origin);

	_moveinfo.state = STATE_BOTTOM;

	if (_health)
	{
		_takedamage = DAMAGE_YES;
		//die = door_killed;
		_max_health = _health;
	}
	else if (_targetname.length() && _message.length())
	{
		trap_SV_SoundIndex("misc/talk.wav");
		//ent->touch = door_touch;
	}
	
	_moveinfo.speed = _speed;
	_moveinfo.accel = _accel;
	_moveinfo.decel = _decel;
	_moveinfo.wait = _wait;
	_moveinfo.start_origin = _pos1;
	_moveinfo.start_quat = _s.quat;
	_moveinfo.end_origin = _pos2;
	_moveinfo.end_quat = _s.quat;

	if(_spawnflags & 16)
		_s.effects |= EF_AUTOANIM_2;
		
	if(_spawnflags & 64)
		_s.effects |= EF_AUTOANIM_10;

	// to simplify logic elsewhere, make non-teamed doors into a team of one
	if(!_team.length())
		_teammaster = this;

	_nextthink = level.time + FRAMETIME;
	
	if (_health || _targetname.length())
		_thinktype = THINK_CALC_MOVE_SPEED;
	else
		_thinktype = THINK_SPAWN_DOOR_TRIGGER;
}






void 	g_func_door_c::hitTop()
{
	if(!(_flags & FL_TEAMSLAVE))
	{
		if(_moveinfo.sound_end)
			trap_SV_StartSound (NULL, this, CHAN_NO_PHS_ADD+CHAN_VOICE, _moveinfo.sound_end, 1, ATTN_STATIC, 0);
		_s.index_sound = 0;
	}
	
	_moveinfo.state = STATE_TOP;
	
	if(_spawnflags & DOOR_TOGGLE)
		return;
	

	if(_moveinfo.wait >= 0)
	{
		//self->_think = door_go_down;
		_thinktype = THINK_GO_DOWN;
		_nextthink = level.time + _moveinfo.wait;
	}
}


void	g_func_door_c::hitBottom()
{
	if(!(_flags & FL_TEAMSLAVE))
	{
		if(_moveinfo.sound_end)
			trap_SV_StartSound (NULL, this, CHAN_NO_PHS_ADD+CHAN_VOICE, _moveinfo.sound_end, 1, ATTN_STATIC, 0);
		_s.index_sound = 0;
	}
	
	_moveinfo.state = STATE_BOTTOM;
	
	useAreaportals(false);
}



void	g_func_door_c::calcMoveSpeed()
{
	g_func_c	*ent;
	float	min;
	float	time;
	float	newspeed;
	float	ratio;
	float	dist;

	if(_flags & FL_TEAMSLAVE)
		return;		// only the team master does this
	
	// find the smallest distance any member of the team will be moving
	min = fabs(_moveinfo.distance);
	for(ent = (g_func_c*)_teamchain; ent; ent = (g_func_c*)ent->_teamchain)
	{
		dist = fabs(ent->_moveinfo.distance);
		if (dist < min)
			min = dist;
	}

	time = min / _moveinfo.speed;

	// adjust speeds so they will all complete at the same time
	for(ent = (g_func_c*)this; ent; ent = (g_func_c*)ent->_teamchain)
	{
		newspeed = fabs(ent->_moveinfo.distance) / time;
		ratio = newspeed / ent->_moveinfo.speed;
		
		if(ent->_moveinfo.accel == ent->_moveinfo.speed)
			ent->_moveinfo.accel = newspeed;
		else
			ent->_moveinfo.accel *= ratio;
			
		if(ent->_moveinfo.decel == ent->_moveinfo.speed)
			ent->_moveinfo.decel = newspeed;
		else
			ent->_moveinfo.decel *= ratio;
			
		ent->_moveinfo.speed = newspeed;
	}
}

void	g_func_door_c::spawnDoorTrigger()
{
	//g_func_c*	other;
	//cbbox_c		bbox;

	if(_flags & FL_TEAMSLAVE)
		return;		// only the team leader spawns a trigger

	//bbox = _r.bbox;
	/*
	for(other = (g_func_c*)_teamchain; other; other = (g_func_c*)other->_teamchain)
	{
		//bbox.addPoint(_r.bbox_abs._mins);
		//bbox.addPoint(_r.bbox_abs._maxs);
	}
	*/

	/*
	// expand 
	bbox._mins[0] -= 60;
	bbox._mins[1] -= 60;
	bbox._maxs[0] += 60;
	bbox._maxs[1] += 60;
	*/

	/*
	// spawn the trigger
	g_entity_c *trigger = new g_trigger_door_c();
	
	G_SetModel(this, _model);
	
	_geom->setPosition(_s.origin);
	
	trigger->_r.owner = this;
	trigger->_r.solid = SOLID_TRIGGER;
	trigger->_movetype = MOVETYPE_NONE;
	*/
	
	if(_spawnflags & DOOR_START_OPEN)
		useAreaportals(true);

	calcMoveSpeed();
}


void	g_func_door_c::goDown()
{
	if(!(_flags & FL_TEAMSLAVE))
	{
		if(_moveinfo.sound_start)
			trap_SV_StartSound(NULL, this, CHAN_NO_PHS_ADD+CHAN_VOICE, _moveinfo.sound_start, 1, ATTN_STATIC, 0);
			
		_s.index_sound = _moveinfo.sound_middle;
	}
	
	if(_max_health)
	{
		_takedamage = DAMAGE_YES;
		_health = _max_health;
	}
	
	_moveinfo.state = STATE_DOWN;
	
	//if (_classname == "func_door")
		calcMove(_moveinfo.start_origin, THINK_HIT_BOTTOM);
				
	//else if (strcmp(self->classname, "func_door_rotating") == 0)
	//	AngleMove_Calc (self, door_hit_bottom);
	
	//useAreaportals(false);
}



void	g_func_door_c::goUp(g_entity_c *activator)
{
	if(_moveinfo.state == STATE_UP)
		return;		// already going up

	if(_moveinfo.state == STATE_TOP)
	{	
		// reset top wait time
		if(_moveinfo.wait >= 0)
			_nextthink = level.time + _moveinfo.wait;
		return;
	}
	
	if(!(_flags & FL_TEAMSLAVE))
	{
		if(_moveinfo.sound_start)
			trap_SV_StartSound(NULL, this, CHAN_NO_PHS_ADD+CHAN_VOICE, _moveinfo.sound_start, 1, ATTN_STATIC, 0);
			
		_s.index_sound = _moveinfo.sound_middle;
	}
	
	_moveinfo.state = STATE_UP;
	
	//if (strcmp(self->classname, "func_door") == 0)
	calcMove(_moveinfo.end_origin, THINK_HIT_TOP);//, door_hit_top);
		//hitTop();
	//else if (strcmp(self->classname, "func_door_rotating") == 0)
	//	AngleMove_Calc (self, door_hit_top);

	G_UseTargets(this, activator);
	
	useAreaportals(true);
}


void 	g_func_door_c::useAreaportals(bool open)
{
	//g_entity_c	*t = NULL;

	//if (!self->target)
	//	return;

	//while ((t = G_FindByTargetName (t, self->target)))
	//{
	//	if (X_stricmp(t->classname, "func_areaportal") == 0)
	//	{
	//		trap_SV_SetAreaPortalState (self, open);
	//	}
	//}
	
	if(_flags & FL_TEAMSLAVE)
		return;
		
	trap_CM_SetAreaPortalState(_r.areaportal, open);
}



void	SP_func_door(g_entity_c **entity)
{
	g_entity_c *ent = new g_func_door_c();
	*entity = ent;
}



/*QUAKED func_door_rotating (0 .5 .8) ? START_OPEN REVERSE CRUSHER NOMONSTER ANIMATED TOGGLE X_AXIS Y_AXIS
TOGGLE causes the door to wait in both the start and end states for a trigger event.

START_OPEN	the door to moves to its destination when spawned, and operate in reverse.  It is used to temporarily or permanently close off an area when triggered (not useful for touch or takedamage doors).
NOMONSTER	monsters will not trigger this door

You need to have an origin brush as part of this entity.  The center of that brush will be
the point around which it is rotated. It will rotate around the Z axis by default.  You can
check either the X_AXIS or Y_AXIS box to change that.

"distance" is how many degrees the door will be rotated.
"speed" determines how fast the door moves; default value is 100.

REVERSE will cause the door to rotate in the opposite direction.

"message"	is printed when the door is touched if it is a trigger door and it hasn't been fired yet
"angle"		determines the opening direction
"targetname" if set, no touch field will be spawned and a remote button or trigger field activates the door.
"health"	if set, door must be shot open
"speed"		movement speed (100 default)
"wait"		wait before returning (3 default, -1 = never return)
"dmg"		damage to inflict when blocked (2 default)
"sounds"
1)	silent
2)	light
3)	medium
4)	heavy
*/

/*
void SP_func_door_rotating (g_entity_c *ent)
{
	ent->s.angles.clear();

	// set the axis of rotation
	ent->movedir.clear();
	
	if (ent->spawnflags & DOOR_X_AXIS)
		ent->movedir[2] = 1.0;
		
	else if (ent->spawnflags & DOOR_Y_AXIS)
		ent->movedir[0] = 1.0;
		
	else // Z_AXIS
		ent->movedir[1] = 1.0;

	// check for reverse rotation
	if (ent->spawnflags & DOOR_REVERSE)
		ent->movedir.negate();

	if (!st.distance)
	{
		trap_Com_Printf("%s at %s with no distance set\n", ent->classname, Vector3_String(ent->s.origin));
		st.distance = 90;
	}

	Vector3_Copy (ent->s.angles, ent->pos1);
	Vector3_MA (ent->s.angles, st.distance, ent->movedir, ent->pos2);
	ent->moveinfo.distance = st.distance;

	ent->movetype = MOVETYPE_PUSH;
	ent->r.solid = SOLID_BSP;
	G_SetModel (ent, ent->model);

	ent->blocked = door_blocked;
	ent->use = door_use;

	if (!ent->speed)
		ent->speed = 100;
	if (!ent->accel)
		ent->accel = ent->speed;
	if (!ent->decel)
		ent->decel = ent->speed;

	if (!ent->wait)
		ent->wait = 3;
	if (!ent->dmg)
		ent->dmg = 2;

	if (ent->sounds != 1)
	{
		ent->moveinfo.sound_start = trap_SV_SoundIndex  ("doors/dr1_strt.wav");
		ent->moveinfo.sound_middle = trap_SV_SoundIndex  ("doors/dr1_mid.wav");
		ent->moveinfo.sound_end = trap_SV_SoundIndex  ("doors/dr1_end.wav");
	}

	// if it starts open, switch the positions
	if (ent->spawnflags & DOOR_START_OPEN)
	{
		Vector3_Copy (ent->pos2, ent->s.angles);
		Vector3_Copy (ent->pos1, ent->pos2);
		Vector3_Copy (ent->s.angles, ent->pos1);
		ent->movedir.negate();
	}

	if (ent->health)
	{
		ent->takedamage = DAMAGE_YES;
		ent->die = door_killed;
		ent->max_health = ent->health;
	}
	
	if (ent->targetname && ent->message)
	{
		trap_SV_SoundIndex ("misc/talk.wav");
		ent->touch = door_touch;
	}

	ent->moveinfo.state = STATE_BOTTOM;
	ent->moveinfo.speed = ent->speed;
	ent->moveinfo.accel = ent->accel;
	ent->moveinfo.decel = ent->decel;
	ent->moveinfo.wait = ent->wait;
	Vector3_Copy (ent->s.origin, ent->moveinfo.start_origin);
	Vector3_Copy (ent->pos1, ent->moveinfo.start_angles);
	Vector3_Copy (ent->s.origin, ent->moveinfo.end_origin);
	Vector3_Copy (ent->pos2, ent->moveinfo.end_angles);

	if (ent->spawnflags & 16)
		ent->s.effects |= EF_ANIM_ALL;

	// to simplify logic elsewhere, make non-teamed doors into a team of one
	if (!ent->team)
		ent->teammaster = ent;

	trap_SV_LinkEdict (ent);

	ent->nextthink = level.time + FRAMETIME;
	if (ent->health || ent->targetname)
		ent->think = Think_CalcMoveSpeed;
	else
		ent->think = Think_SpawnDoorTrigger;
}
*/

/*QUAKED func_water (0 .5 .8) ? START_OPEN
func_water is a moveable water brush.  It must be targeted to operate.  Use a non-water texture at your own risk.

START_OPEN causes the water to move to its destination when spawned and operate in reverse.

"angle"		determines the opening direction (up or down only)
"speed"		movement speed (25 default)
"wait"		wait before returning (-1 default, -1 = TOGGLE)
"lip"		lip remaining at end of move (0 default)
"sounds"	(yes, these need to be changed)
0)	no sound
1)	water
2)	lava
*/

/*
void SP_func_water (g_entity_c *self)
{
	vec3_t	abs_movedir;

	G_SetMovedir (self->s.angles, self->movedir);
	self->movetype = MOVETYPE_PUSH;
	self->r.solid = SOLID_BSP;
	G_SetModel (self, self->model);

	switch (self->sounds)
	{
		default:
			break;

		case 1: // water
			self->moveinfo.sound_start = trap_SV_SoundIndex  ("world/mov_watr.wav");
			self->moveinfo.sound_end = trap_SV_SoundIndex  ("world/stp_watr.wav");
			break;

		case 2: // lava
			self->moveinfo.sound_start = trap_SV_SoundIndex  ("world/mov_watr.wav");
			self->moveinfo.sound_end = trap_SV_SoundIndex  ("world/stp_watr.wav");
			break;
	}

	// calculate second position
	Vector3_Copy (self->s.origin, self->pos1);
	abs_movedir[0] = fabs(self->movedir[0]);
	abs_movedir[1] = fabs(self->movedir[1]);
	abs_movedir[2] = fabs(self->movedir[2]);
	self->moveinfo.distance = abs_movedir[0] * self->r.size[0] + abs_movedir[1] * self->r.size[1] + abs_movedir[2] * self->r.size[2] - st.lip;
	Vector3_MA (self->pos1, self->moveinfo.distance, self->movedir, self->pos2);

	// if it starts open, switch the positions
	if (self->spawnflags & DOOR_START_OPEN)
	{
		Vector3_Copy (self->pos2, self->s.origin);
		Vector3_Copy (self->pos1, self->pos2);
		Vector3_Copy (self->s.origin, self->pos1);
	}

	Vector3_Copy (self->pos1, self->moveinfo.start_origin);
	Vector3_Copy (self->s.angles, self->moveinfo.start_angles);
	Vector3_Copy (self->pos2, self->moveinfo.end_origin);
	Vector3_Copy (self->s.angles, self->moveinfo.end_angles);

	self->moveinfo.state = STATE_BOTTOM;

	if (!self->speed)
		self->speed = 25;
	self->moveinfo.accel = self->moveinfo.decel = self->moveinfo.speed = self->speed;

	if (!self->wait)
		self->wait = -1;
	self->moveinfo.wait = self->wait;

	self->use = door_use;

	if (self->wait == -1)
		self->spawnflags |= DOOR_TOGGLE;

	self->classname = "func_door";

	trap_SV_LinkEdict (self);
}
*/

#define TRAIN_START_ON		1
#define TRAIN_TOGGLE		2
#define TRAIN_BLOCK_STOPS	4

/*QUAKED func_train (0 .5 .8) ? START_ON TOGGLE BLOCK_STOPS
Trains are moving platforms that players can ride.
The targets origin specifies the min point of the train at each corner.
The train spawns at the first target it is pointing at.
If the train is the target of a button or trigger, it will not begin moving until activated.
speed	default 100
dmg		default	2
noise	looping sound to play when the train is in motion

*/
//void train_next (g_entity_c *self);

/*
void train_blocked (g_entity_c *self, g_entity_c *other)
{
	if (!other->r.client)
	{
		// give it a chance to go away on it's own terms (like gibs)
		T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, 100000, 1, 0, MOD_CRUSH);
		// if it's still there, nuke it
		if (other)
			BecomeExplosion1 (other);
		return;
	}

	if (level.time < self->touch_debounce_time)
		return;

	if (!self->dmg)
		return;
	self->touch_debounce_time = level.time + 0.5;
	T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);
}
*/

/*
void train_wait (g_entity_c *self)
{
	if (self->target_ent->pathtarget)
	{
		char	*savetarget;
		g_entity_c	*ent;

		ent = self->target_ent;
		savetarget = ent->target;
		ent->target = ent->pathtarget;
		G_UseTargets (ent, self->activator);
		ent->target = savetarget;

		// make sure we didn't get killed by a killtarget
		if (!self->r.inuse)
			return;
	}

	if (self->moveinfo.wait)
	{
		if (self->moveinfo.wait > 0)
		{
			self->nextthink = level.time + self->moveinfo.wait;
			self->think = train_next;
		}
		else if (self->spawnflags & TRAIN_TOGGLE)  // && wait < 0
		{
			train_next (self);
			self->spawnflags &= ~TRAIN_START_ON;
			self->velocity.clear();
			self->nextthink = 0;
		}

		if (!(self->flags & FL_TEAMSLAVE))
		{
			if (self->moveinfo.sound_end)
				trap_SV_StartSound (NULL, self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_end, 1, ATTN_STATIC, 0);
			self->s.sound = 0;
		}
	}
	else
	{
		train_next (self);
	}
	
}
*/

/*
void train_next (g_entity_c *self)
{
	g_entity_c		*ent;
	vec3_c		dest;
	bool	first;

	first = true;
again:
	if (!self->target)
	{
//		trap_Com_Printf ("train_next: no next target\n");
		return;
	}

	ent = G_PickTarget (self->target);
	if (!ent)
	{
		trap_Com_Printf ("train_next: bad target %s\n", self->target);
		return;
	}

	self->target = ent->target;

	// check for a teleport path_corner
	if (ent->spawnflags & 1)
	{
		if (!first)
		{
			trap_Com_Printf ("connected teleport path_corners, see %s at %s\n", ent->classname, Vector3_String(ent->s.origin));
			return;
		}
		first = false;
		self->s.origin = ent->s.origin - self->r.bbox._mins;
		self->s.old_origin = self->s.origin;
		self->s.event = EV_OTHER_TELEPORT;
		trap_SV_LinkEdict (self);
		goto again;
	}

	self->moveinfo.wait = ent->wait;
	self->target_ent = ent;

	if (!(self->flags & FL_TEAMSLAVE))
	{
		if (self->moveinfo.sound_start)
			trap_SV_StartSound (NULL, self, CHAN_NO_PHS_ADD+CHAN_VOICE, self->moveinfo.sound_start, 1, ATTN_STATIC, 0);
		self->s.sound = self->moveinfo.sound_middle;
	}

	dest = ent->s.origin - self->r.bbox._mins;
	self->moveinfo.state = STATE_TOP;
	Vector3_Copy (self->s.origin, self->moveinfo.start_origin);
	Vector3_Copy (dest, self->moveinfo.end_origin);
	Move_Calc (self, dest, train_wait);
	self->spawnflags |= TRAIN_START_ON;
}
*/

/*
void train_resume (g_entity_c *self)
{
	g_entity_c	*ent;
	vec3_c	dest;

	ent = self->target_ent;

	dest = ent->s.origin - self->r.bbox._mins;
	self->moveinfo.state = STATE_TOP;
	Vector3_Copy (self->s.origin, self->moveinfo.start_origin);
	Vector3_Copy (dest, self->moveinfo.end_origin);
	Move_Calc (self, dest, train_wait);
	self->spawnflags |= TRAIN_START_ON;
}
*/

/*
void func_train_find (g_entity_c *self)
{
	g_entity_c *ent;

	if (!self->target)
	{
		trap_Com_Printf ("train_find: no target\n");
		return;
	}
	ent = G_PickTarget (self->target);
	if (!ent)
	{
		trap_Com_Printf ("train_find: target %s not found\n", self->target);
		return;
	}
	self->target = ent->target;

	self->s.origin = ent->s.origin - self->r.bbox._mins;
	trap_SV_LinkEdict (self);

	// if not triggered, start immediately
	if (!self->targetname)
		self->spawnflags |= TRAIN_START_ON;

	if (self->spawnflags & TRAIN_START_ON)
	{
		self->nextthink = level.time + FRAMETIME;
		self->think = train_next;
		self->activator = self;
	}
}
*/


/*
void train_use (g_entity_c *self, g_entity_c *other, g_entity_c *activator)
{
	self->activator = activator;

	if (self->spawnflags & TRAIN_START_ON)
	{
		if (!(self->spawnflags & TRAIN_TOGGLE))
			return;
		self->spawnflags &= ~TRAIN_START_ON;
		self->velocity.clear();
		self->nextthink = 0;
	}
	else
	{
		if (self->target_ent)
			train_resume(self);
		else
			train_next(self);
	}
}
*/

/*
void SP_func_train (g_entity_c *self)
{
	self->movetype = MOVETYPE_PUSH;

	self->s.angles.clear();
	self->blocked = train_blocked;
	if (self->spawnflags & TRAIN_BLOCK_STOPS)
		self->dmg = 0;
	else
	{
		if (!self->dmg)
			self->dmg = 100;
	}
	self->r.solid = SOLID_BSP;
	G_SetModel (self, self->model);

	if (st.noise)
		self->moveinfo.sound_middle = trap_SV_SoundIndex  (st.noise);

	if (!self->speed)
		self->speed = 100;

	self->moveinfo.speed = self->speed;
	self->moveinfo.accel = self->moveinfo.decel = self->moveinfo.speed;

	self->use = train_use;

	trap_SV_LinkEdict (self);

	if (self->target)
	{
		// start trains on the second frame, to make sure their targets have had
		// a chance to spawn
		self->nextthink = level.time + FRAMETIME;
		self->think = func_train_find;
	}
	else
	{
		trap_Com_Printf ("func_train without a target at %s\n", Vector3_String(self->r.bbox_abs._mins));
	}
}
*/

/*QUAKED trigger_elevator (0.3 0.1 0.6) (-8 -8 -8) (8 8 8)
*/
/*
void trigger_elevator_use (g_entity_c *self, g_entity_c *other, g_entity_c *activator)
{
	g_entity_c *target;

	if (self->movetarget->nextthink)
	{
//		trap_Com_Printf("elevator busy\n");
		return;
	}

	if (!other->pathtarget)
	{
		trap_Com_Printf("elevator used with no pathtarget\n");
		return;
	}

	target = G_PickTarget (other->pathtarget);
	if (!target)
	{
		trap_Com_Printf("elevator used with bad pathtarget: %s\n", other->pathtarget);
		return;
	}

	self->movetarget->target_ent = target;
	train_resume (self->movetarget);
}
*/

/*
void trigger_elevator_init (g_entity_c *self)
{
	if (!self->target)
	{
		trap_Com_Printf("trigger_elevator has no target\n");
		return;
	}
	self->movetarget = G_PickTarget (self->target);
	if (!self->movetarget)
	{
		trap_Com_Printf("trigger_elevator unable to find target %s\n", self->target);
		return;
	}
	if (strcmp(self->movetarget->classname, "func_train") != 0)
	{
		trap_Com_Printf("trigger_elevator target %s is not a train\n", self->target);
		return;
	}

	self->use = trigger_elevator_use;
	self->r.svflags = SVF_NOCLIENT;

}
*/

/*
void SP_trigger_elevator (g_entity_c *self)
{
	self->think = trigger_elevator_init;
	self->nextthink = level.time + FRAMETIME;
}
*/


/*QUAKED func_timer (0.3 0.1 0.6) (-8 -8 -8) (8 8 8) START_ON
"wait"			base time between triggering all targets, default is 1
"random"		wait variance, default is 0

so, the basic time between firing is a random time between
(wait - random) and (wait + random)

"delay"			delay before first firing when turned on, default is 0

"pausetime"		additional delay used only the very first time
				and only if spawned with START_ON

These can used but not touched.
*/
/*
void func_timer_think (g_entity_c *self)
{
	G_UseTargets (self, self->activator);
	self->nextthink = level.time + self->wait + crandom() * self->random;
}
*/

/*
void func_timer_use (g_entity_c *self, g_entity_c *other, g_entity_c *activator)
{
	self->activator = activator;

	// if on, turn it off
	if (self->nextthink)
	{
		self->nextthink = 0;
		return;
	}

	// turn it on
	if (self->delay)
		self->nextthink = level.time + self->delay;
	else
		func_timer_think (self);
}
*/


/*
void SP_func_timer (g_entity_c *self)
{
	if (!self->wait)
		self->wait = 1.0;

	self->use = func_timer_use;
	self->think = func_timer_think;

	if (self->random >= self->wait)
	{
		self->random = self->wait - FRAMETIME;
		trap_Com_Printf("func_timer at %s has random >= wait\n", Vector3_String(self->s.origin));
	}

	if (self->spawnflags & 1)
	{
		self->nextthink = level.time + 1.0 + st.pausetime + self->delay + self->wait + crandom() * self->random;
		self->activator = self;
	}

	self->r.svflags = SVF_NOCLIENT;
}
*/

/*QUAKED func_conveyor (0 .5 .8) ? START_ON TOGGLE
Conveyors are stationary brushes that move what's on them.
The brush should be have a surface with at least one current content enabled.
speed	default 100
*/
/*
void func_conveyor_use (g_entity_c *self, g_entity_c *other, g_entity_c *activator)
{
	if (self->spawnflags & 1)
	{
		self->speed = 0;
		self->spawnflags &= ~1;
	}
	else
	{
		self->speed = self->count;
		self->spawnflags |= 1;
	}

	if (!(self->spawnflags & 2))
		self->count = 0;
}
*/

/*
void SP_func_conveyor (g_entity_c *self)
{
	if (!self->speed)
		self->speed = 100;

	if (!(self->spawnflags & 1))
	{
		self->count = (int)self->speed;
		self->speed = 0;
	}

	self->use = func_conveyor_use;

	G_SetModel (self, self->model);
	self->r.solid = SOLID_BSP;
	trap_SV_LinkEdict (self);
}
*/


/*QUAKED func_door_secret (0 .5 .8) ? always_shoot 1st_left 1st_down
A secret door.  Slide back and then to the side.

open_once		doors never closes
1st_left		1st move is left of arrow
1st_down		1st move is down from arrow
always_shoot	door is shootebale even if targeted

"angle"		determines the direction
"dmg"		damage to inflic when blocked (default 2)
"wait"		how long to hold in the open position (default 5, -1 means hold)
*/
/*
#define SECRET_ALWAYS_SHOOT	1
#define SECRET_1ST_LEFT		2
#define SECRET_1ST_DOWN		4

void door_secret_move1 (g_entity_c *self);
void door_secret_move2 (g_entity_c *self);
void door_secret_move3 (g_entity_c *self);
void door_secret_move4 (g_entity_c *self);
void door_secret_move5 (g_entity_c *self);
void door_secret_move6 (g_entity_c *self);
void door_secret_done (g_entity_c *self);
*/

/*
void door_secret_use (g_entity_c *self, g_entity_c *other, g_entity_c *activator)
{
	// make sure we're not already moving
	if (!Vector3_Compare(self->s.origin, vec3_origin))
		return;

	Move_Calc (self, self->pos1, door_secret_move1);
	door_use_areaportals (self, true);
}
*/

/*
void door_secret_move1 (g_entity_c *self)
{
	self->nextthink = level.time + 1.0;
	self->think = door_secret_move2;
}
*/

/*
void door_secret_move2 (g_entity_c *self)
{
	Move_Calc (self, self->pos2, door_secret_move3);
}
*/

/*
void door_secret_move3 (g_entity_c *self)
{
	if (self->wait == -1)
		return;
	self->nextthink = level.time + self->wait;
	self->think = door_secret_move4;
}
*/

/*
void door_secret_move4 (g_entity_c *self)
{
	Move_Calc (self, self->pos1, door_secret_move5);
}
*/

/*
void door_secret_move5 (g_entity_c *self)
{
	self->nextthink = level.time + 1.0;
	self->think = door_secret_move6;
}
*/

/*
void door_secret_move6 (g_entity_c *self)
{
	Move_Calc (self, vec3_origin, door_secret_done);
}
*/

/*
void door_secret_done (g_entity_c *self)
{
	if (!(self->targetname) || (self->spawnflags & SECRET_ALWAYS_SHOOT))
	{
		self->health = 0;
		self->takedamage = DAMAGE_YES;
	}
	door_use_areaportals (self, false);
}
*/

/*
void door_secret_blocked  (g_entity_c *self, g_entity_c *other)
{
	if (!other->r.client)
	{
		// give it a chance to go away on it's own terms (like gibs)
		T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, 100000, 1, 0, MOD_CRUSH);
		// if it's still there, nuke it
		if (other)
			BecomeExplosion1 (other);
		return;
	}

	if (level.time < self->touch_debounce_time)
		return;
	self->touch_debounce_time = level.time + 0.5;

	T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);
}
*/

/*
void door_secret_die (g_entity_c *self, g_entity_c *inflictor, g_entity_c *attacker, int damage, vec3_t point)
{
	self->takedamage = DAMAGE_NO;
	door_secret_use (self, attacker, attacker);
}
*/

/*
void SP_func_door_secret (g_entity_c *ent)
{
	vec3_c	forward, right, up;
	float	side;
	float	width;
	float	length;

	ent->moveinfo.sound_start = trap_SV_SoundIndex  ("doors/dr1_strt.wav");
	ent->moveinfo.sound_middle = trap_SV_SoundIndex  ("doors/dr1_mid.wav");
	ent->moveinfo.sound_end = trap_SV_SoundIndex  ("doors/dr1_end.wav");

	ent->movetype = MOVETYPE_PUSH;
	ent->r.solid = SOLID_BSP;
	G_SetModel (ent, ent->model);

	ent->blocked = door_secret_blocked;
	ent->use = door_secret_use;

	if (!(ent->targetname) || (ent->spawnflags & SECRET_ALWAYS_SHOOT))
	{
		ent->health = 0;
		ent->takedamage = DAMAGE_YES;
		ent->die = door_secret_die;
	}

	if (!ent->dmg)
		ent->dmg = 2;

	if (!ent->wait)
		ent->wait = 5;

	ent->moveinfo.accel =
	ent->moveinfo.decel =
	ent->moveinfo.speed = 50;

	// calculate positions
	Angles_ToVectors (ent->s.angles, forward, right, up);
	ent->s.angles.clear();
	side = 1.0 - (ent->spawnflags & SECRET_1ST_LEFT);
	if (ent->spawnflags & SECRET_1ST_DOWN)
		width = fabs(Vector3_DotProduct(up, ent->r.size));
	else
		width = fabs(Vector3_DotProduct(right, ent->r.size));
	length = fabs(Vector3_DotProduct(forward, ent->r.size));
	if (ent->spawnflags & SECRET_1ST_DOWN)
		Vector3_MA (ent->s.origin, -1 * width, up, ent->pos1);
	else
		Vector3_MA (ent->s.origin, side * width, right, ent->pos1);
	Vector3_MA (ent->pos1, length, forward, ent->pos2);

	if (ent->health)
	{
		ent->takedamage = DAMAGE_YES;
		ent->die = door_killed;
		ent->max_health = ent->health;
	}
	else if (ent->targetname && ent->message)
	{
		trap_SV_SoundIndex ("misc/talk.wav");
		ent->touch = door_touch;
	}
	
	ent->classname = "func_door";

	trap_SV_LinkEdict (ent);
}
*/

/*
================================================================================
				KILLBOX FUNC
================================================================================
*/

/*QUAKED func_killbox (1 0 0) ?
Kills everything inside when fired, irrespective of protection.
*/


g_func_killbox_c::g_func_killbox_c()
{	
	_r.svflags = SVF_NOCLIENT;
}


void	g_func_killbox_c::use(g_entity_c *other, g_entity_c *activator)
{
	G_KillBox(this);
}


void	g_func_killbox_c::activate()
{
	_r.inuse = true;
	G_SetModel(this, _model);
}


void	SP_func_killbox (g_entity_c **entity)
{
	g_entity_c *ent = new g_func_killbox_c();
	*entity = ent;
}



/*
================================================================================
				STATIC FUNC
================================================================================
*/

g_func_static_c::g_func_static_c()
:g_entity_c(false)
{
	_s.type	= ET_FUNC_STATIC;
	_s.renderfx = RF_STATIC;
}

void	g_func_static_c::activate()
{
	_r.inuse = true;
	
	if(_model.empty())// || _model[0] == '*')
	{
		trap_Com_Printf("g_func_static_c::activate: bad model '%s'\n", _model.c_str());
		remove();
		return;
	}
	
	//trap_Com_Printf("g_func_static_c::activate: model '%s'\n", _model.c_str());
	
	if(	_model[0] != '*' &&
		//X_strcaseequal(_model.substr(_model.length()-4, _model.length()).c_str(), ".ase") ||
		//(X_strcaseequal(_model.substr(_model.length()-4, _model.length()).c_str(), ".lwo") ||
		X_strcaseequal(_model.substr(_model.length()-4, _model.length()).c_str(), ".obj")	)
	{
		trap_Com_Printf("g_func_static_c::activate: model type not supported yet for '%s'\n", _model.c_str());
		remove();
		return;
	}
	
#if 1
//	_s.index_model = trap_SV_ModelIndex(_model);
	G_SetModel(this, _model);
#else
	cmodel_c *model = G_SetModel(this, _model);
	
	// setup rigid body
//	_body->setPosition(_s.origin);
//	_body->setQuaternion(_s.quat);
//	_body->setGravityMode(0);
	
	// setup collision
	g_geom_info_c *geom_info = new g_geom_info_c(this, model, NULL);
	
	d_geom_c *geom = new d_box_c(g_ode_space->getId(), _r.size * 0.5);
//	d_geom_c *geom = new d_trimesh_c(g_ode_space->getId(), model->vertexes, model->indexes);
//	d_geom_c *geom = new d_sphere_c(g_ode_space->getId(), _r.bbox._maxs[0]);
	
//	geom->disable();
//	geom->setBody(_body->getId());
//	geom->setPosition(_s.origin);
//	geom->setQuaternion(_s.quat);
	geom->setData(geom_info);
	geom->setCollideBits(MASK_SOLID);
//	geom->enable();
	
	_geoms.insert(std::make_pair(geom, geom_info));
#endif
}


void	SP_func_static(g_entity_c **entity)
{
	g_entity_c *ent = new g_func_static_c();
	*entity = ent;
}


/*
================================================================================
				FLARE FUNC
================================================================================
*/
/*
g_func_flare_c::g_func_flare_c()
:g_entity_c(false)
{
	_s.type	= ET_FUNC_FLARE;
}

void	g_func_flare_c::activate()
{
	_r.inuse = true;
	
	if(_model.empty() || _model[0] != '*')
	{
		trap_Com_Printf("g_func_flare_c::activate: bad inline model '%s'", _model.c_str());
		remove();
		return;
	}
	
	//trap_Com_Printf("g_func_flare_c::activate: model '%s'\n", _model.c_str());
	
	_s.index_model = trap_SV_ModelIndex(_model);
}


void	SP_func_flare(g_entity_c **entity)
{
	g_entity_c *ent = new g_func_flare_c();
	*entity = ent;
}
*/


/*
g_ode_box_c::g_ode_box_c()
{
}

void	g_ode_box_c::activate()
{
	_r.inuse = true;
	
	G_SetModel(this, _model);

	// setup rigid body
	_body->setPosition(_s.origin);
	_body->setQuaternion(_s.quat);
	_body->setGravityMode(1);
	
	// set mass
	dMass m;
	dMassSetBoxTotal(&m, _mass, _r.size[0], _r.size[1], _r.size[2]);
	_body->setMass(&m);
	
	// setup geom
	d_geom_c_geom = new d_box_c(g_ode_space->getId(), _r.size);
	_geom->setData(this);
	_geom->setCollideBits(MASK_SOLID);
}

void	SP_ode_box(g_entity_c **entity)
{
	g_entity_c *ent = new g_ode_box_c();
	*entity = ent;
}
*/

