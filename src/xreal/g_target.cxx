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
#include "g_target.h"
#include "g_local.h"




g_target_c::g_target_c()
:g_entity_c(false)
{
}



/*QUAKED target_temp_entity (1 0 0) (-8 -8 -8) (8 8 8)
Fire an origin based temp entity event to the clients.
"style"		type byte
*/
/*
void Use_Target_Tent (edict_t *ent, edict_t *other, edict_t *activator)
{
	trap_SV_WriteByte (svc_temp_entity);
	trap_SV_WriteByte (ent->style);
	trap_SV_WritePosition (ent->s.origin);
	trap_SV_Multicast (ent->s.origin, MULTICAST_PVS);
}
*/

/*
void SP_target_temp_entity (edict_t *ent)
{
	ent->use = Use_Target_Tent;
}
*/




/*
================================================================================
				SPEAKER TARGET
================================================================================
*/


/*QUAKED target_speaker (1 0 0) (-8 -8 -8) (8 8 8) LOOPED_ON LOOPED_OFF GLOBAL
"noise"		wav file to play
"attenuation"
-1 = none, send to whole level
1 = normal fighting sounds
2 = idle sound level
3 = ambient sound level
"volume"	0.0 to 1.0

Normal sounds play each time the target is used.  The reliable flag can be set for crucial voiceovers.

Looped sounds are always atten 3 / vol 1, and the use function toggles it on/off.
Multiple identical looping sounds will just increase volume without any speed cost.
*/

g_target_speaker_c::g_target_speaker_c()
{
	_s.type = ET_TARGET_SPEAKER;

	addField(g_field_c("s_shader", &_s_shader, F_STRING));
	addField(g_field_c("s_looping", &_s_looping, F_BOOL));
}

void	g_target_speaker_c::use(g_entity_c *other, g_entity_c *activator)
{
	int		chan;

	if(_spawnflags & 3)
	{	
		// looping sound toggles
		if(_s.index_sound)
			_s.index_sound = 0;	// turn it off
		else
			_s.index_sound = _noise_index;	// start it
	}
	else
	{	
		// normal sound
		if(_spawnflags & 4)
			chan = CHAN_VOICE|CHAN_RELIABLE;
		else
			chan = CHAN_VOICE;
			
		// use a positioned_sound, because this entity won't normally be
		// sent to any clients because it is invisible
		trap_SV_StartSound(_s.origin, this, chan, _noise_index, _volume, _attenuation, 0);
	}
}

void	g_target_speaker_c::activate()
{
	if(_s_shader.empty())
	{
		trap_Com_Printf("target_speaker with no sound shader set at %s\n", Vector3_String(_s.origin));
		remove();
		return;
	}
	
	_r.inuse = true;
	
	trap_Com_Printf("g_target_speaker::activate: sound '%s'\n", _s_shader.c_str());
		
	_noise_index = trap_SV_SoundIndex(_s_shader);

	if(!_volume)
		_volume = 1.0;

	if(!_attenuation)
		_attenuation = 1.0;
		
	else if(_attenuation == -1)	// use -1 so 0 defaults to 1
		_attenuation = 0;

	// check for prestarted looping sound
	if(_s_looping)
	{
		_s.index_sound = _noise_index;
		_r.networksync = false;		// just pulse as baseline entity
	}
}


void SP_target_speaker(g_entity_c  **entity)
{
	g_entity_c *ent = new g_target_speaker_c();
	*entity = ent;
}




/*
================================================================================
				EXPLOSION TARGET
================================================================================
*/

/*QUAKED target_explosion (1 0 0) (-8 -8 -8) (8 8 8)
Spawns an explosion temporary entity when used.

"delay"		wait this long before going off
"dmg"		how much radius damage should be done, defaults to 0
*/

g_target_explosion_c::g_target_explosion_c()
{
	_r.svflags = SVF_NOCLIENT;
}

void	g_target_explosion_c::think()
{
	trap_SV_WriteBits(SVC_TEMP_ENTITY, svc_bitcount);
	trap_SV_WriteByte(TE_EXPLOSION1);
	trap_SV_WritePosition(_s.origin);
	trap_SV_Multicast(_s.origin, MULTICAST_PHS);

	//T_RadiusDamage(this, _activator, _dmg, NULL, _dmg+40, MOD_EXPLOSIVE);

	int save = _time_delay;
	_time_delay = 0;
	G_UseTargets(this, _activator);
	_time_delay = save;
}



void	g_target_explosion_c::use(g_entity_c *other, g_entity_c *activator)
{
	_activator = activator;

	if(!_time_delay)
	{
		think();
		return;
	}

	_nextthink = level.time + _time_delay;
}

void	g_target_explosion_c::activate()
{
	_r.inuse	= true;
}

void	SP_target_explosion(g_entity_c **entity)
{
	g_entity_c *ent = new g_target_explosion_c();
	*entity = ent;
}



//==========================================================

/*QUAKED target_changelevel (1 0 0) (-8 -8 -8) (8 8 8)
Changes level to "map" when fired
*/
/*
void use_target_changelevel (edict_t *self, edict_t *other, edict_t *activator)
{
	if (level.intermissiontime)
		return;		// already activated

	if (!deathmatch->value && !coop->value)
	{
		if (g_edicts[1]->health <= 0)
			return;
	}

	// if noexit, do a ton of damage to other
	if (deathmatch->value && !( (int)dmflags->value & DF_ALLOW_EXIT) && other != world)
	{
		T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, 10 * other->max_health, 1000, 0, MOD_EXIT);
		return;
	}

	// if multiplayer, let everyone know who hit the exit
	if (deathmatch->value)
	{
		if (activator && activator->r.client)
			trap_SV_BPrintf (PRINT_HIGH, "%s exited the level.\n", activator->r.client->pers.netname);
	}

	// if going to a new unit, clear cross triggers
	if (strstr(self->map, "*"))	
		game.serverflags &= ~(SFL_CROSS_TRIGGER_MASK);

	BeginIntermission (self);
}
*/

/*
void SP_target_changelevel (edict_t *ent)
{
	if (!ent->map)
	{
		trap_Com_Printf("target_changelevel with no map at %s\n", Vector3_String(ent->s.origin));
		G_FreeEdict (ent);
		return;
	}

	// ugly hack because *SOMEBODY* screwed up their map
	if((X_stricmp(level.mapname, "fact1") == 0) && (X_stricmp(ent->map, "fact3") == 0))
	   ent->map = "fact3$secret1";

	ent->use = use_target_changelevel;
	ent->r.svflags = SVF_NOCLIENT;
}
*/


//==========================================================

/*QUAKED target_splash (1 0 0) (-8 -8 -8) (8 8 8)
Creates a particle splash effect when used.

Set "sounds" to one of the following:
  1) sparks
  2) blue water
  3) brown water
  4) slime
  5) lava
  6) blood

"count"	how many pixels in the splash
"dmg"	if set, does a radius damage at this location when it splashes
		useful for lava/sparks
*/

/*
void use_target_splash (edict_t *self, edict_t *other, edict_t *activator)
{
	trap_SV_WriteByte (svc_temp_entity);
	trap_SV_WriteByte (TE_SPLASH);
	trap_SV_WriteByte (self->count);
	trap_SV_WritePosition (self->s.origin);
	trap_SV_WriteDir (self->movedir);
	trap_SV_WriteByte (self->sounds);
	trap_SV_Multicast (self->s.origin, MULTICAST_PVS);

	if (self->dmg)
		T_RadiusDamage (self, activator, self->dmg, NULL, self->dmg+40, MOD_SPLASH);
}
*/

/*
void SP_target_splash (edict_t *self)
{
	self->use = use_target_splash;
	G_SetMovedir (self->s.angles, self->movedir);

	if (!self->count)
		self->count = 32;

	self->r.svflags = SVF_NOCLIENT;
}
*/


//==========================================================

/*QUAKED target_spawner (1 0 0) (-8 -8 -8) (8 8 8)
Set target to the type of entity you want spawned.
Useful for spawning monsters and gibs in the factory levels.

For monsters:
	Set direction to the facing you want it to have.

For gibs:
	Set direction if you want it moving and
	speed how fast it should be moving otherwise it
	will just be dropped
*/
//void ED_CallSpawn (edict_t *ent);

/*
void use_target_spawner (edict_t *self, edict_t *other, edict_t *activator)
{
	edict_t	*ent;

	//ent = G_Spawn();
	ent = new edict_t();
	
	ent->classname = self->target;
	Vector3_Copy (self->s.origin, ent->s.origin);
	Vector3_Copy (self->s.angles, ent->s.angles);
	ED_CallSpawn (ent);
	trap_SV_UnlinkEdict (ent);
	G_KillBox (ent);
	trap_SV_LinkEdict (ent);
	if (self->speed)
		Vector3_Copy (self->movedir, ent->velocity);
}
*/

/*
void SP_target_spawner (edict_t *self)
{
	self->use = use_target_spawner;
	self->r.svflags = SVF_NOCLIENT;
	if (self->speed)
	{
		G_SetMovedir (self->s.angles, self->movedir);
		Vector3_Scale (self->movedir, self->speed, self->movedir);
	}
}
*/

//==========================================================

/*QUAKED target_blaster (1 0 0) (-8 -8 -8) (8 8 8) NOTRAIL NOEFFECTS
Fires a blaster bolt in the set direction when triggered.

dmg		default is 15
speed	default is 1000
*/

/*
void use_target_blaster (edict_t *self, edict_t *other, edict_t *activator)
{
	int effect;

	if (self->spawnflags & 2)
		effect = 0;
	else if (self->spawnflags & 1)
		effect = EF_HYPERBLASTER;
	else
		effect = EF_BLASTER;

	fire_blaster (self, self->s.origin, self->movedir, self->dmg, (int)self->speed, EF_BLASTER, MOD_TARGET_BLASTER);
	trap_SV_StartSound (NULL, self, CHAN_VOICE, self->noise_index, 1, ATTN_NORM, 0);
}
*/

/*
void SP_target_blaster (edict_t *self)
{
	self->use = use_target_blaster;
	G_SetMovedir (self->s.angles, self->movedir);
	self->noise_index = trap_SV_SoundIndex ("weapons/laser2.wav");

	if (!self->dmg)
		self->dmg = 15;
	if (!self->speed)
		self->speed = 1000;

	self->r.svflags = SVF_NOCLIENT;
}
*/

//==========================================================

/*QUAKED target_crosslevel_trigger (.5 .5 .5) (-8 -8 -8) (8 8 8) trigger1 trigger2 trigger3 trigger4 trigger5 trigger6 trigger7 trigger8
Once this trigger is touched/used, any trigger_crosslevel_target with the same trigger number is automatically used when a level is started within the same unit.  It is OK to check multiple triggers.  Message, delay, target, and killtarget also work.
*/
/*
void trigger_crosslevel_trigger_use (edict_t *self, edict_t *other, edict_t *activator)
{
	game.serverflags |= self->spawnflags;
	G_FreeEdict (self);
}
*/

/*
void SP_target_crosslevel_trigger (edict_t *self)
{
	self->r.svflags = SVF_NOCLIENT;
	self->use = trigger_crosslevel_trigger_use;
}
*/

/*QUAKED target_crosslevel_target (.5 .5 .5) (-8 -8 -8) (8 8 8) trigger1 trigger2 trigger3 trigger4 trigger5 trigger6 trigger7 trigger8
Triggered by a trigger_crosslevel elsewhere within a unit.  If multiple triggers are checked, all must be true.  Delay, target and
killtarget also work.

"delay"		delay before using targets if the trigger has been activated (default 1)
*/
/*
void target_crosslevel_target_think (edict_t *self)
{
	if (self->spawnflags == (game.serverflags & SFL_CROSS_TRIGGER_MASK & self->spawnflags))
	{
		G_UseTargets (self, self);
		G_FreeEdict (self);
	}
}
*/

/*
void SP_target_crosslevel_target (edict_t *self)
{
	if (! self->delay)
		self->delay = 1;
	self->r.svflags = SVF_NOCLIENT;

	self->think = target_crosslevel_target_think;
	self->nextthink = level.time + self->delay;
}
*/


//==========================================================

/*QUAKED target_earthquake (1 0 0) (-8 -8 -8) (8 8 8)
When triggered, this initiates a level-wide earthquake.
All players and monsters are affected.
"speed"		severity of the quake (default:200)
"count"		duration of the quake (default:5)
*/

/*
void target_earthquake_think (edict_t *self)
{
	int		i;
	edict_t	*e;

	if (self->last_move_time < level.time)
	{
		trap_SV_StartSound (self->s.origin, self, CHAN_AUTO, self->noise_index, 1.0, ATTN_NONE, 0);
		self->last_move_time = level.time + 500;
	}

	for (i=1; i < g_edicts.size(); i++)
	{
		e = g_edicts[i];
				
		if (!e->r.inuse)
			continue;
			
		if (!e->r.client)
			continue;
			
		if (!e->groundentity)
			continue;

		e->groundentity = NULL;
		e->velocity[0] += crandom()* 150;
		e->velocity[1] += crandom()* 150;
		e->velocity[2] = self->speed * (100.0 / e->mass);
	}

	if (level.time < self->timestamp)
		self->nextthink = level.time + FRAMETIME;
}
*/

/*
void target_earthquake_use (edict_t *self, edict_t *other, edict_t *activator)
{
	self->timestamp = level.time + self->count;
	self->nextthink = level.time + FRAMETIME;
	self->activator = activator;
	self->last_move_time = 0;
}
*/

/*
void SP_target_earthquake (edict_t *self)
{
	if (!self->targetname)
		trap_Com_Printf("untargeted %s at %s\n", self->classname, Vector3_String(self->s.origin));

	if (!self->count)
		self->count = 5;

	if (!self->speed)
		self->speed = 200;

	self->r.svflags |= SVF_NOCLIENT;
	self->think = target_earthquake_think;
	self->use = target_earthquake_use;

	self->noise_index = trap_SV_SoundIndex ("world/quake.wav");
}
*/



/*
================================================================================
				POSITION TARGET
================================================================================
*/
g_target_position_c::g_target_position_c()
{
	_r.svflags = SVF_NOCLIENT;
}

void	g_target_position_c::activate()
{
	_r.inuse = true;
}

void	SP_target_position(g_entity_c **entity)
{
	g_entity_c *ent = new g_target_position_c();
	*entity = ent;
}


/*
================================================================================
				TELEPORT TARGET
================================================================================
*/

/*QUAKED misc_teleporter_dest (1 0 0) (-32 -32 -24) (32 32 -16)
Point teleporters at these.
*/

void	g_target_teleport_c::activate()
{
	_r.inuse = true;
	
	//trap_SV_SetModel (ent, "models/objects/dmspot/tris.md2");
	//ent->_s.skinnum = 0;
	//ent->_r.solid = SOLID_BBOX;
	//ent->s.effects |= EF_FLIES;
	//Vector3_Set (ent->_r.bbox._mins, -32, -32, -24);
	//Vector3_Set (ent->_r.bbox._maxs, 32, 32, -16);
}


void	SP_target_teleport(g_entity_c **entity)
{
	g_entity_c *ent = new g_target_teleport_c();
	*entity = ent;
}


/*
================================================================================
				DELAY TARGET
================================================================================
*/
g_target_delay_c::g_target_delay_c(g_entity_c *ent, g_entity_c *activator)
{
	_r.inuse	= true;
	
	_classname = "DelayedUse";
	_nextthink = level.time + ent->_time_delay;
	//target->think = Think_Delay;
	
	_activator = activator;	
	if(!activator)
		trap_Com_Printf("Think_Delay with no activator\n");
	
	_message = ent->_message;
	_target = ent->_target;
	_killtarget = ent->_killtarget;
}

void	g_target_delay_c::think()
{
	G_UseTargets(this, _activator);
	
	remove();
}


/*
================================================================================
				CHANGE LEVEL TARGET
================================================================================
*/
g_target_changelevel_c::g_target_changelevel_c()
{
	_r.inuse	= true;

	addField(g_field_c("map", &_map, F_STRING));
	
	_classname = "target_changelevel";
	_map = level.nextmap;
}


