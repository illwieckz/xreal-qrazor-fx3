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
#include "g_trigger.h"
#include "g_player.h"
#include "g_local.h"






void 	g_trigger_c::init()
{
	if(!(_angles == vec3_origin))
		G_SetMovedir(_s.quat, _movedir);

	_r.solid = SOLID_TRIGGER;
	_movetype = MOVETYPE_NONE;
	G_SetModel(this, _model);
	_r.svflags = SVF_NOCLIENT;
	
#if defined(ODE)
	_body->setPosition(_s.origin);
	_body->setQuaternion(_s.quat);
	_body->setGravityMode(0);
#endif
}


bool	g_trigger_c::touch(g_entity_c *other, const plane_c &plane, csurface_c *surf)
{
	return false;
}


/*
================================================================================
				MULTIPLE TRIGGER
================================================================================
*/
g_trigger_multiple_c::g_trigger_multiple_c()
{
	//TODO
}

// the wait time has passed, so set back up for another activation
void	g_trigger_multiple_c::think()
{
	if(_wait > 0)
	{
		_nextthink = 0;
	}
	else
	{
		remove();
	}
}


// the trigger was just activated
// ent->activator should be set to the activator so it can be held through a delay
// so wait for the delay time before firing
static void	multi_trigger(g_entity_c *ent)
{
	if(ent->_nextthink)
		return;		// already been triggered

	G_UseTargets(ent, ent->_activator);

	if(ent->_wait > 0)
	{
		//ent->think = multi_wait;
		ent->_nextthink = level.time + ent->_wait;
	}
	else
	{	// we can't just remove (self) here, because this is a touch function
		// called while looping through area links...
		
		//ent->touch = NULL;
		ent->_nextthink = level.time + FRAMETIME;
		//ent->think = G_FreeEdict;
	}
}


void	g_trigger_multiple_c::use(g_entity_c *other, g_entity_c *activator)
{
	_activator = activator;
	multi_trigger(this);
}


bool	g_trigger_multiple_c::touch(g_entity_c *other, const plane_c &plane, csurface_c *surf)
{
	if(other->_r.isclient)
	{
		if(_spawnflags & 2)
			return false;
	}
	else
	{
		return false;
	}

	if(_movedir != vec3_origin)
	{
		vec3_c		forward, right, up;
		matrix_c	matrix;
		
		matrix.fromQuaternion(_s.quat);
		matrix.toVectorsFRU(forward, right, up);
		
		if(Vector3_DotProduct(forward, _movedir) < 0)
			return false;
	}

	_activator = other;
	
	multi_trigger(this);
	
	return false;
}


/*QUAKED trigger_multiple (.5 .5 .5) ? MONSTER NOT_PLAYER TRIGGERED
Variable sized repeatable trigger.  Must be targeted at one or more entities.
If "delay" is set, the trigger waits some time after activating before firing.
"wait" : Seconds between triggerings. (.2 default)
sounds
1)	secret
2)	beep beep
3)	large switch
4)
set "message" to text string
*/
/*
void trigger_enable (g_entity_c *self, g_entity_c *other, g_entity_c *activator)
{
	self->r.solid = SOLID_TRIGGER;
	self->use = Use_Multi;
	trap_SV_LinkEdict (self);
}
*/

void	g_trigger_multiple_c::activate()
{
	_r.inuse = true;

	if (_sounds == 1)
		_noise_index = trap_SV_SoundIndex("misc/secret.wav");
		
	else if (_sounds == 2)
		_noise_index = trap_SV_SoundIndex("misc/talk.wav");
		
	else if (_sounds == 3)
		_noise_index = trap_SV_SoundIndex("misc/trigger1.wav");
	
	if (!_wait)
		_wait = 0.2;
		
	//ent->touch = Touch_Multi;
	_movetype = MOVETYPE_NONE;
	_r.svflags |= SVF_NOCLIENT;


	if(_spawnflags & 4)
	{
		_r.solid = SOLID_NOT;
		//use = trigger_enable;
	}
	else
	{
		_r.solid = SOLID_TRIGGER;
		//use = Use_Multi;
	}

	if(!(_angles == vec3_origin))
		G_SetMovedir(_s.quat, _movedir);
	
#if defined(ODE)	
	_body->setPosition(_s.origin);
	_body->setQuaternion(_s.quat);
	_body->setGravityMode(0);
#endif

	G_SetModel(this, _model);
}


void	SP_trigger_multiple (g_entity_c **entity)
{
	g_entity_c *ent = new g_trigger_multiple_c();
	*entity = ent;
}



/*QUAKED trigger_once (.5 .5 .5) ? x x TRIGGERED
Triggers once, then removes itself.
You must set the key "target" to the name of another object in the level that has a matching "targetname".

If TRIGGERED, this trigger must be triggered before it is live.

sounds
 1)	secret
 2)	beep beep
 3)	large switch
 4)

"message"	string to be displayed when triggered
*/

	// make old maps work because I messed up on flag assignments here
	// triggered was on bit 1 when it should have been on bit 4
void	g_trigger_once_c::activate()
{
	if(_spawnflags & 1)
	{
		vec3_t	v;

		Vector3_MA (_r.bbox._mins, 0.5, _r.size, v);
		_spawnflags &= ~1;
		_spawnflags |= 4;
		trap_Com_Printf("fixed TRIGGERED flag on %s at %s\n", _classname.c_str(), Vector3_String(v));
	}

	_wait = -1;
	
	_r.inuse = true;

	if (_sounds == 1)
		_noise_index = trap_SV_SoundIndex("misc/secret.wav");
		
	else if (_sounds == 2)
		_noise_index = trap_SV_SoundIndex("misc/talk.wav");
		
	else if (_sounds == 3)
		_noise_index = trap_SV_SoundIndex("misc/trigger1.wav");
	
	if (!_wait)
		_wait = 0.2;
		
	//ent->touch = Touch_Multi;
	_movetype = MOVETYPE_NONE;
	_r.svflags |= SVF_NOCLIENT;


	if (_spawnflags & 4)
	{
		_r.solid = SOLID_NOT;
		//use = trigger_enable;
	}
	else
	{
		_r.solid = SOLID_TRIGGER;
		//use = Use_Multi;
	}

	if (!(_angles == vec3_origin))
		G_SetMovedir(_s.quat, _movedir);

	G_SetModel (this, _model);
}

void SP_trigger_once(g_entity_c **entity)
{
	g_entity_c *ent = new g_trigger_once_c();
	*entity = ent;
}


/*QUAKED trigger_relay (.5 .5 .5) (-8 -8 -8) (8 8 8)
This fixed size trigger cannot be touched, it can only be fired by other events.
*/
/*
void trigger_relay_use (g_entity_c *self, g_entity_c *other, g_entity_c *activator)
{
	G_UseTargets (self, activator);
}
*/

/*
void SP_trigger_relay (g_entity_c *self)
{
	self->use = trigger_relay_use;
}
*/


/*
==============================================================================

trigger_key

==============================================================================
*/

/*QUAKED trigger_key (.5 .5 .5) (-8 -8 -8) (8 8 8)
A relay trigger that only fires it's targets if player has the proper key.
Use "item" to specify the required key, for example "key_data_cd"
*/
/*
void trigger_key_use (g_entity_c *self, g_entity_c *other, g_entity_c *activator)
{
	int			index;

	if (!self->item)
		return;
	if (!activator->r.client)
		return;

	index = G_GetNumForItem(self->item);
	if (!activator->r.client->pers.inventory[index])
	{
		if (level.time < self->touch_debounce_time)
			return;
		self->touch_debounce_time = level.time + 5.0;
		trap_SV_CenterPrintf (activator, "You need the %s", self->item->getPickupName());
		trap_SV_StartSound (NULL, activator, CHAN_AUTO, trap_SV_SoundIndex ("misc/keytry.wav"), 1, ATTN_NORM, 0);
		return;
	}

	trap_SV_StartSound (NULL, activator, CHAN_AUTO, trap_SV_SoundIndex ("misc/keyuse.wav"), 1, ATTN_NORM, 0);
	if (coop->value)
	{
		int		player;
		g_entity_c	*ent;

		if (strcmp(self->item->getClassname(), "key_power_cube") == 0)
		{
			int	cube;

			for (cube = 0; cube < 8; cube++)
				if (activator->r.client->pers.power_cubes & (1 << cube))
					break;
					
			for (player = 1; player <= game.maxclients; player++)
			{
				ent = g_edicts[player];
				
				if (!ent->r.inuse)
					continue;
					
				if (!ent->r.client)
					continue;
					
				if (ent->r.client->pers.power_cubes & (1 << cube))
				{
					ent->r.client->pers.inventory[index]--;
					ent->r.client->pers.power_cubes &= ~(1 << cube);
				}
			}
		}
		else
		{
			for (player = 1; player <= game.maxclients; player++)
			{
				ent = g_edicts[player];
				
				if (!ent->r.inuse)
					continue;
					
				if (!ent->r.client)
					continue;
					
				ent->r.client->pers.inventory[index] = 0;
			}
		}
	}
	else
	{
		activator->r.client->pers.inventory[index]--;
	}

	G_UseTargets (self, activator);

	self->use = NULL;
}
*/

/*
void SP_trigger_key (g_entity_c *self)
{
	if (!st.item)
	{
		trap_Com_Printf("no key item for trigger_key at %s\n", Vector3_String(self->s.origin));
		return;
	}
	self->item = G_FindItemByClassname (st.item);

	if (!self->item)
	{
		trap_Com_Printf("item %s not found for trigger_key at %s\n", st.item, Vector3_String(self->s.origin));
		return;
	}

	if (!self->target)
	{
		trap_Com_Printf("%s at %s has no target\n", self->classname, Vector3_String(self->s.origin));
		return;
	}

	trap_SV_SoundIndex ("misc/keytry.wav");
	trap_SV_SoundIndex ("misc/keyuse.wav");

	self->use = trigger_key_use;
}
*/

/*
==============================================================================

trigger_counter

==============================================================================
*/

/*QUAKED trigger_counter (.5 .5 .5) ? nomessage
Acts as an intermediary for an action that takes multiple inputs.

If nomessage is not set, t will print "1 more.. " etc when triggered and "sequence complete" when finished.

After the counter has been triggered "count" times (default 2), it will fire all of it's targets and remove itself.
*/
/*
void trigger_counter_use(g_entity_c *self, g_entity_c *other, g_entity_c *activator)
{
	if (self->count == 0)
		return;
	
	self->count--;

	if (self->count)
	{
		if (! (self->spawnflags & 1))
		{
			trap_SV_CenterPrintf(activator, "%i more to go...", self->count);
			trap_SV_StartSound (NULL, activator, CHAN_AUTO, trap_SV_SoundIndex ("misc/talk1.wav"), 1, ATTN_NORM, 0);
		}
		return;
	}
	
	if (! (self->spawnflags & 1))
	{
		trap_SV_CenterPrintf(activator, "Sequence completed!");
		trap_SV_StartSound (NULL, activator, CHAN_AUTO, trap_SV_SoundIndex ("misc/talk1.wav"), 1, ATTN_NORM, 0);
	}
	self->activator = activator;
	multi_trigger (self);
}
*/

/*
void SP_trigger_counter (g_entity_c *self)
{
	self->wait = -1;
	if (!self->count)
		self->count = 2;

	self->use = trigger_counter_use;
}
*/

/*
==============================================================================

trigger_always

==============================================================================
*/

/*QUAKED trigger_always (.5 .5 .5) (-8 -8 -8) (8 8 8)
This trigger will always fire.  It is activated by the world.
*/
/*
void SP_trigger_always (g_entity_c *ent)
{
	// we must have some delay to make sure our use targets are present
	if (ent->delay < 0.2)
		ent->delay = 0.2;
	G_UseTargets(ent, ent);
}
*/



/*
================================================================================
				PUSH TRIGGER
================================================================================
*/

#define PUSH_ONCE		1

g_trigger_push_c::g_trigger_push_c()
{
	//TODO
}

void	g_trigger_push_c::think()
{
	//float	height, gravity, time, forward;
        //float	dist;

	//trap_Com_Printf("g_trigger_push_c::think\n\n");

	vec3_c origin = _r.bbox._mins + _r.bbox._maxs;
	origin.scale(0.5);

	g_entity_c *target = G_PickTarget(_target);

	if(!target)
	{
		trap_Com_Error(ERR_DROP, "g_trigger_push_c::think: can't find target '%s'\n", _target.c_str());
		remove();
		return;
	}

	/*
	height = target->_s.origin[2] - origin[2];
	gravity = fabs(g_gravity->value);
	time = X_sqrt(2.0f * (height / gravity));

	if(!time)
	{
		remove();
		return;
	}
	*/

	// set s.origin2 to the push velocity
	_movedir =  target->_s.origin - origin;
	_movedir.normalize();
	//_movedir[2] = 0;
	//dist = _movedir.normalize();

	//forward = dist / time;
	//_movedir.scale(forward);

	//_movedir[2] = time * gravity;	
}

bool	g_trigger_push_c::touch(g_entity_c *other, const plane_c &plane, csurface_c *surf)
{
	if(!other)
		return false;

	/*
	if(other->_classname == "grenade")
	{
		Vector3_Scale(_movedir, _speed * 10, other->_velocity);
	}
	else if(other->_health > 0 && other->_r.isclient)
	*/
	{
		//trap_Com_Printf("g_trigger_push_c::touching client\n");
	
		// set actors speed
		//other->_velocity = _movedir;
		//other->_body->addForce(_movedir * 10);
		//other->_body->addForce(_movedir * (_speed*10));
		//other->_body->setLinearVel(_movedir * _speed);
		//Vector3_Scale (self->movedir, self->speed * 10, other->velocity);

		// no prediction
		//other->getClient()->jumppad_time = level.time;
		
		// don't take falling damage immediately from this
		//other->getClient()->oldvelocity = other->_velocity;
		
		/*	
		if(other->_fly_sound_debounce_time < level.time)
		{
			other->_fly_sound_debounce_time = level.time + 1.5;
			trap_SV_StartSound (NULL, other, CHAN_AUTO, trap_SV_SoundIndex ("misc/windfly.wav"), 1, ATTN_NORM, 0);
		}
		*/
	}

	if(_spawnflags & PUSH_ONCE)
		remove();
		
	return false;
}

void	g_trigger_push_c::activate()
{
	_r.inuse = true;

	init();
	
	_nextthink = level.time + FRAMETIME;
	
	//if (!self->speed)
	//	self->speed = 1000;
}

/*QUAKED trigger_push (.5 .5 .5) ? PUSH_ONCE
Pushes the player
"speed"		defaults to 1000
*/

void	SP_trigger_push(g_entity_c **entity)
{
	g_entity_c *ent = new g_trigger_push_c();
	*entity = ent;
}



/*
================================================================================
				HURT TRIGGER
================================================================================
*/

/*QUAKED trigger_hurt (.5 .5 .5) ? START_OFF TOGGLE SILENT NO_PROTECTION SLOW
Any entity that touches this will be hurt.

It does dmg points of damage each server frame

SILENT			supresses playing the sound
SLOW			changes the damage rate to once per second
NO_PROTECTION	*nothing* stops the damage

"dmg"			default 5 (whole numbers only)

*/

g_trigger_hurt_c::g_trigger_hurt_c()
{
	//TODO
}

bool	g_trigger_hurt_c::touch(g_entity_c *other, const plane_c &plane, csurface_c *surf)
{
	int		dflags;

	if(!other->_takedamage)
		return false;

	if(_timestamp > level.time)
		return false;

	if(_spawnflags & 16)
		_timestamp = level.time + 1;
	else
		_timestamp = level.time + FRAMETIME;

	if(!(_spawnflags & 4))
	{
		if ((level.framenum % 10) == 0)
			trap_SV_StartSound (NULL, other, CHAN_AUTO, _noise_index, 1, ATTN_NORM, 0);
	}

	if (_spawnflags & 8)
		dflags = DAMAGE_NO_PROTECTION;
	else
		dflags = 0;
		
	other->takeDamage(this, this, vec3_origin, other->_s.origin, vec3_origin, _dmg, _dmg, dflags, MOD_TRIGGER_HURT);
	
	return false;
}


void	g_trigger_hurt_c::use(g_entity_c *other, g_entity_c *activator)
{
	if (!(_spawnflags & 2))
		return;
	
	if(_r.solid == SOLID_NOT)
		_r.solid = SOLID_TRIGGER;
	else
		_r.solid = SOLID_NOT;

	//if (!(self->spawnflags & 2))
	//	self->use = NULL;
}


void	g_trigger_hurt_c::activate()
{
	init();
	
	_r.inuse = true;

	_noise_index = trap_SV_SoundIndex("world/electro.wav");
	
	if (!_dmg)
		_dmg = 5;

	if (_spawnflags & 1)
		_r.solid = SOLID_NOT;
	else
		_r.solid = SOLID_TRIGGER;

	//if (_spawnflags & 2)
	//	self->use = hurt_use
}

void	SP_trigger_hurt(g_entity_c **entity)
{
	g_entity_c *ent = new g_trigger_hurt_c();
	*entity = ent;
}



/*
==============================================================================
				GRAVITY TRIGGER
==============================================================================
*/

/*QUAKED trigger_gravity (.5 .5 .5) ?
Changes the touching entites gravity to
the value of "gravity".  1.0 is standard
gravity for the level.
*/

g_trigger_gravity_c::g_trigger_gravity_c()
{
	//TODO
}


bool	g_trigger_gravity_c::touch(g_entity_c *other, const plane_c &plane, csurface_c *surf)
{
	other->_gravity = _gravity;
	
	return false;
}

void	g_trigger_gravity_c::activate()
{
	if(_gravity == 0)
	{
		trap_Com_Printf("trigger_gravity without gravity set at %s\n",  _s.origin.toString());
		remove();
		return;
	}

	init();
	
	_r.inuse = true;
}

void	SP_trigger_gravity(g_entity_c **entity)
{
	g_entity_c *ent = new g_trigger_gravity_c();
	*entity = ent;
}





/*
================================================================================
				TELEPORT TRIGGER
================================================================================
*/

g_trigger_teleport_c::g_trigger_teleport_c()
{
	//TODO
}

void	g_trigger_teleport_c::think()
{
	if(!_target.length())
	{
		trap_Com_Printf("teleporter without a target.\n");
		remove();
		return;
	}
}


bool	g_trigger_teleport_c::touch(g_entity_c *other, const plane_c &plane, csurface_c *surf)
{
	g_entity_c		*dest;
	g_player_c		*player;

	if(!other->_r.isclient)
		return false;
		
	player = (g_player_c*)other;
		
	dest = G_FindByTargetName(NULL, _target);
	if(!dest)
	{
		trap_Com_Printf("Couldn't find destination\n");
		return false;
	}

	// unlink to make sure it can't possibly interfere with KillBox
#if defined(ODE)
	player->_body->setPosition(dest->_s.origin);
#else
	player->_s.origin = dest->_s.origin;
#endif
	player->_s.event = EV_PLAYER_TELEPORT;

	// clear the velocity and hold them in place briefly
	//player->_velocity.clear();
	player->_r.ps.pmove.pm_time = 160>>3;		// hold time
	player->_r.ps.pmove.pm_flags |= PMF_TIME_TELEPORT;

	// draw the teleport splash at source and on the player
	/*
	trap_SV_WriteBits(SVC_TEMP_ENTITY, svc_bitcount);
	trap_SV_WriteByte(TE_PLAYER_TELEPORT_OUT);
	trap_SV_WritePosition(self->s.origin);
	trap_SV_Multicast(_s.origin, MULTICAST_PVS);

	trap_SV_WriteBits(SVC_TEMP_ENTITY, svc_bitcount);
	trap_SV_WriteByte(TE_PLAYER_TELEPORT_IN);
	trap_SV_WritePosition(player->_s.origin);
	trap_SV_Multicast(player->_s.origin, MULTICAST_PVS);
	*/

	// set angles
	player->_r.ps.pmove.delta_angles = dest->_angles - player->_resp.cmd_angles;

	player->setViewAngles(vec3_origin);

	// kill anything at the destination
	G_KillBox(player);
	
	return false;
}


void	g_trigger_teleport_c::activate()
{
	init();

	_r.inuse = true;
	
	_nextthink = level.time + FRAMETIME;
	
	G_SetWorldModel(this, _model);
	
	//G_SetModel(this, "models/objects/dmspot/tris.md2");
	//_s.skinnum = 1;
	//_s.effects = EF_TELEPORTER;
	//_s.sound = trap_SV_SoundIndex ("world/amb10.wav");
	//_r.solid = SOLID_BBOX;

	//_r.bbox._mins.set(-32,-32,-24);
	//_r.bbox._maxs.set( 32, 32,-16);
	//trap_SV_LinkEdict (ent);

	//trig = G_Spawn ();
	//trig = new g_entity_c();
	
	//trig->touch = teleporter_touch;
	//_r.solid = SOLID_TRIGGER;
	//trig->r.owner = ent;
	//_s.origin.set(trig->s.origin);
	//_r.bbox._mins.set(-8, -8, 8);
	//_r.bbox._maxs.set( 8, 8, 24);
}

/*QUAKED misc_teleporter (1 0 0) (-32 -32 -24) (32 32 -16)
Stepping onto this disc will teleport players to the targeted misc_teleporter_dest object.
*/
void SP_trigger_teleport(g_entity_c **entity)
{
	g_entity_c *ent = new g_trigger_teleport_c();
	*entity = ent;
}


/*
================================================================================
				DOOR TRIGGER
================================================================================
*/

g_trigger_door_c::g_trigger_door_c()
{
	_r.inuse	= true;
}

bool	g_trigger_door_c::touch(g_entity_c *other, const plane_c &plane, csurface_c *surf)
{
	if(other->_health <= 0)
		return false;

	if(!other->_r.isclient)
		return false;

	if(level.time < _touch_debounce_time)
		return false;
		
	_touch_debounce_time = level.time + 1.0;

	((g_entity_c*)_r.owner)->use(other, other);
	
	return false;
}


