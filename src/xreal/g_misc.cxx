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
#include "g_local.h"
#include "g_entity.h"



/*
static void	G_VelocityForDamage (int damage, vec3_t v)
{
	v[0] = 100.0 * crandom();
	v[1] = 100.0 * crandom();
	v[2] = 200.0 + 100.0 * random();

	if (damage < 50)
		Vector3_Scale (v, 0.7, v);
	else 
		Vector3_Scale (v, 1.2, v);
}
*/

/*
static void	G_ClipGibVelocity (g_entity_c *ent)
{
	if (ent->velocity[0] < -300)
		ent->velocity[0] = -300;
		
	else if (ent->velocity[0] > 300)
		ent->velocity[0] = 300;
		
		
	if (ent->velocity[1] < -300)
		ent->velocity[1] = -300;
		
	else if (ent->velocity[1] > 300)
		ent->velocity[1] = 300;
		
		
	if (ent->velocity[2] < 200)
		ent->velocity[2] = 200;	// always some upwards
		
	else if (ent->velocity[2] > 500)
		ent->velocity[2] = 500;
}

*/

/*
void gib_think (g_entity_c *self)
{
	self->s.frame++;
	self->nextthink = level.time + FRAMETIME;

	if (self->s.frame == 10)
	{
		self->think = G_FreeEdict;
		self->nextthink = level.time + 8 + random()*10;
	}
}
*/

/*
void gib_touch (g_entity_c *self, g_entity_c *other, cplane_c *plane, csurface_c *surf)
{
	vec3_c	normal_angles;
	vec3_c	forward, right, up;

	if (!self->groundentity)
		return;

	self->touch = NULL;

	if (plane)
	{
		gi.SV_StartSound (NULL, self, CHAN_VOICE, gi.SV_SoundIndex ("misc/fhit3.wav"), 1, ATTN_NORM, 0);

		vectoangles (plane->_normal, normal_angles);
		Angles_ToVectors (normal_angles, forward, right, up);
		vectoangles (right, self->s.angles);

		if (self->s.modelindex == sm_meat_index)
		{
			self->s.frame++;
			self->think = gib_think;
			self->nextthink = level.time + FRAMETIME;
		}
	}
}
*/

/*
void gib_die (g_entity_c *self, g_entity_c *inflictor, g_entity_c *attacker, int damage, vec3_t point)
{
	G_FreeEdict (self);
}
*/

/*
void ThrowGib (g_entity_c *self, char *gibname, int damage, int type)
{
	g_entity_c *gib;
	vec3_t	vd;
	vec3_t	origin;
	vec3_t	size;
	float	vscale;

	//gib = G_Spawn();
	gib = new g_entity_c();

	Vector3_Scale (self->r.size, 0.5, size);
	Vector3_Add (self->r.bbox_abs._mins, size, origin);
	gib->s.origin[0] = origin[0] + crandom() * size[0];
	gib->s.origin[1] = origin[1] + crandom() * size[1];
	gib->s.origin[2] = origin[2] + crandom() * size[2];

	gi.SV_SetModel (gib, gibname);
	gib->r.solid = SOLID_NOT;
	gib->s.effects |= EF_GIB;
	gib->flags |= FL_NO_KNOCKBACK;
	gib->takedamage = DAMAGE_YES;
	gib->die = gib_die;

	if (type == GIB_ORGANIC)
	{
		gib->movetype = MOVETYPE_TOSS;
		gib->touch = gib_touch;
		vscale = 0.5;
	}
	else
	{
		gib->movetype = MOVETYPE_BOUNCE;
		vscale = 1.0;
	}

	VelocityForDamage (damage, vd);
	Vector3_MA (self->velocity, vscale, vd, gib->velocity);
	ClipGibVelocity (gib);
	gib->avelocity[0] = random()*600;
	gib->avelocity[1] = random()*600;
	gib->avelocity[2] = random()*600;

	gib->think = G_FreeEdict;
	gib->nextthink = level.time + 10 + random()*10;

	gi.SV_LinkEdict (gib);
}
*/

/*
void ThrowHead (g_entity_c *self, char *gibname, int damage, int type)
{
	vec3_t	vd;
	float	vscale;

	self->s.skinnum = 0;
	self->s.frame = 0;
	self->r.bbox.zero();

	self->s.modelindex2 = 0;
	gi.SV_SetModel (self, gibname);
	self->r.solid = SOLID_NOT;
	self->s.effects |= EF_GIB;
	self->s.effects &= ~EF_FLIES;
	self->s.sound = 0;
	self->flags |= FL_NO_KNOCKBACK;
	self->takedamage = DAMAGE_YES;
	self->die = gib_die;

	if (type == GIB_ORGANIC)
	{
		self->movetype = MOVETYPE_TOSS;
		self->touch = gib_touch;
		vscale = 0.5;
	}
	else
	{
		self->movetype = MOVETYPE_BOUNCE;
		vscale = 1.0;
	}

	VelocityForDamage (damage, vd);
	Vector3_MA (self->velocity, vscale, vd, self->velocity);
	ClipGibVelocity (self);

	self->avelocity[YAW] = crandom()*600;

	self->think = G_FreeEdict;
	self->nextthink = level.time + 10 + random()*10;

	gi.SV_LinkEdict (self);
}
*/


/*
void ThrowClientHead (g_entity_c *self, int damage)
{
	vec3_t	vd;
	char	*gibname;

	if (rand()&1)
	{
		gibname = "models/objects/gibs/head2/tris.md2";
		self->s.skinnum = 1;		// second skin is player
	}
	else
	{
		gibname = "models/objects/gibs/skull/tris.md2";
		self->s.skinnum = 0;
	}

	self->s.origin[2] += 32;
	self->s.frame = 0;
	gi.SV_SetModel (self, gibname);
	self->r.bbox._mins.set(-16,-16, 0);
	self->r.bbox._maxs.set( 16, 16, 16);

	self->takedamage = DAMAGE_NO;
	self->r.solid = SOLID_NOT;
	self->s.effects = EF_GIB;
	self->s.sound = 0;
	self->flags |= FL_NO_KNOCKBACK;

	self->movetype = MOVETYPE_BOUNCE;
	VelocityForDamage (damage, vd);
	Vector3_Add (self->velocity, vd, self->velocity);

	if (self->r.client)	// bodies in the queue don't have a client anymore
	{
		self->r.client->anim_priority = ANIM_DEATH;
		self->r.client->anim_end = self->s.frame;
	}
	else
	{
		self->think = NULL;
		self->nextthink = 0;
	}

	gi.SV_LinkEdict (self);
}
*/



void	G_BecomeExplosion1 (g_entity_c *self)
{
	gi.SV_WriteByte(SVC_TEMP_ENTITY);
	gi.SV_WriteByte(TE_EXPLOSION1);
	gi.SV_WritePosition(self->_s.origin);
	gi.SV_Multicast(self->_s.origin, MULTICAST_PVS);

	self->remove();
}


void	G_BecomeExplosion2 (g_entity_c *self)
{
	gi.SV_WriteByte(SVC_TEMP_ENTITY);
	gi.SV_WriteByte(TE_EXPLOSION2);
	gi.SV_WritePosition(self->_s.origin);
	gi.SV_Multicast(self->_s.origin, MULTICAST_PVS);

	self->remove();
}









/*QUAKED func_wall (0 .5 .8) ? TRIGGER_SPAWN TOGGLE START_ON ANIMATED ANIMATED_FAST
This is just a solid wall if not inhibited

TRIGGER_SPAWN	the wall will not be present until triggered
				it will then blink in to existance; it will
				kill anything that was in it's way

TOGGLE			only valid for TRIGGER_SPAWN walls
				this allows the wall to be turned on and off

START_ON		only valid for TRIGGER_SPAWN walls
				the wall will initially be present
*/

/*
void func_wall_use (g_entity_c *self, g_entity_c *other, g_entity_c *activator)
{
	if (self->r.solid == SOLID_NOT)
	{
		self->r.solid = SOLID_BSP;
		self->r.svflags &= ~SVF_NOCLIENT;
		G_KillBox (self);
	}
	else
	{
		self->r.solid = SOLID_NOT;
		self->r.svflags |= SVF_NOCLIENT;
	}
	gi.SV_LinkEdict (self);

	if (!(self->spawnflags & 2))
		self->use = NULL;
}
*/

/*
void SP_func_wall (g_entity_c *self)
{
	self->movetype = MOVETYPE_PUSH;
	gi.SV_SetModel (self, self->model);

	if (self->spawnflags & 8)
		self->s.effects |= EF_ANIM_ALL;
	if (self->spawnflags & 16)
		self->s.effects |= EF_ANIM_ALLFAST;

	// just a wall
	if ((self->spawnflags & 7) == 0)
	{
		self->r.solid = SOLID_BSP;
		gi.SV_LinkEdict (self);
		return;
	}

	// it must be TRIGGER_SPAWN
	if (!(self->spawnflags & 1))
	{
//		gi.Com_Printf("func_wall missing TRIGGER_SPAWN\n");
		self->spawnflags |= 1;
	}

	// yell if the spawnflags are odd
	if (self->spawnflags & 4)
	{
		if (!(self->spawnflags & 2))
		{
			gi.Com_Printf("func_wall START_ON without TOGGLE\n");
			self->spawnflags |= 2;
		}
	}

	self->use = func_wall_use;
	if (self->spawnflags & 4)
	{
		self->r.solid = SOLID_BSP;
	}
	else
	{
		self->r.solid = SOLID_NOT;
		self->r.svflags |= SVF_NOCLIENT;
	}
	gi.SV_LinkEdict (self);
}
*/


/*QUAKED func_object (0 .5 .8) ? TRIGGER_SPAWN ANIMATED ANIMATED_FAST
This is solid bmodel that will fall if it's support it removed.
*/
/*
void func_object_touch (g_entity_c *self, g_entity_c *other, cplane_c *plane, csurface_c *surf)
{
	// only squash thing we fall on top of
	if (!plane)
		return;
		
	if (plane->_normal[2] < 1.0)
		return;
		
	if (other->takedamage == DAMAGE_NO)
		return;
		
	T_Damage (other, self, self, vec3_origin, self->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);
}
*/

/*
void func_object_release (g_entity_c *self)
{
	self->movetype = MOVETYPE_TOSS;
	self->touch = func_object_touch;
}
*/

/*
void func_object_use (g_entity_c *self, g_entity_c *other, g_entity_c *activator)
{
	self->r.solid = SOLID_BSP;
	self->r.svflags &= ~SVF_NOCLIENT;
	self->use = NULL;
	G_KillBox (self);
	func_object_release (self);
}
*/

/*
void SP_func_object (g_entity_c *self)
{
	gi.SV_SetModel (self, self->model);

	self->r.bbox._mins[0] += 1;
	self->r.bbox._mins[1] += 1;
	self->r.bbox._mins[2] += 1;
	self->r.bbox._maxs[0] -= 1;
	self->r.bbox._maxs[1] -= 1;
	self->r.bbox._maxs[2] -= 1;

	if (!self->dmg)
		self->dmg = 100;

	if (self->spawnflags == 0)
	{
		self->r.solid = SOLID_BSP;
		self->movetype = MOVETYPE_PUSH;
		self->think = func_object_release;
		self->nextthink = level.time + 2 * FRAMETIME;
	}
	else
	{
		self->r.solid = SOLID_NOT;
		self->movetype = MOVETYPE_PUSH;
		self->use = func_object_use;
		self->r.svflags |= SVF_NOCLIENT;
	}

	if (self->spawnflags & 2)
		self->s.effects |= EF_ANIM_ALL;
	if (self->spawnflags & 4)
		self->s.effects |= EF_ANIM_ALLFAST;

	self->r.clipmask = MASK_MONSTERSOLID;

	gi.SV_LinkEdict (self);
}
*/


/*QUAKED func_explosive (0 .5 .8) ? Trigger_Spawn ANIMATED ANIMATED_FAST
Any brush that you want to explode or break apart.  If you want an
ex0plosion, set dmg and it will do a radius explosion of that amount
at the center of the bursh.

If targeted it will not be shootable.

health defaults to 100.

mass defaults to 75.  This determines how much debris is emitted when
it explodes.  You get one large chunk per 100 of mass (up to 8) and
one small chunk per 25 of mass (up to 16).  So 800 gives the most.
*/
/*
void func_explosive_explode (g_entity_c *self, g_entity_c *inflictor, g_entity_c *attacker, int damage, vec3_t point)
{
	vec3_t	origin;
	vec3_t	chunkorigin;
	vec3_t	size;
	int		count;
	int		mass;

	// bmodel origins are (0 0 0), we need to adjust that here
	Vector3_Scale (self->r.size, 0.5, size);
	Vector3_Add (self->r.bbox_abs._mins, size, origin);
	Vector3_Copy (origin, self->s.origin);

	self->takedamage = DAMAGE_NO;

	if (self->dmg)
		T_RadiusDamage (self, attacker, self->dmg, NULL, self->dmg+40, MOD_EXPLOSIVE);

	Vector3_Subtract (self->s.origin, inflictor->s.origin, self->velocity);
	Vector3_Normalize (self->velocity);
	Vector3_Scale (self->velocity, 150, self->velocity);

	// start chunks towards the center
	Vector3_Scale (size, 0.5, size);

	mass = self->mass;
	if (!mass)
		mass = 75;

	// big chunks
	if (mass >= 100)
	{
		count = mass / 100;
		if (count > 8)
			count = 8;
		while(count--)
		{
			chunkorigin[0] = origin[0] + crandom() * size[0];
			chunkorigin[1] = origin[1] + crandom() * size[1];
			chunkorigin[2] = origin[2] + crandom() * size[2];
			ThrowDebris (self, "models/objects/debris1/tris.md2", 1, chunkorigin);
		}
	}

	// small chunks
	count = mass / 25;
	if (count > 16)
		count = 16;
	while(count--)
	{
		chunkorigin[0] = origin[0] + crandom() * size[0];
		chunkorigin[1] = origin[1] + crandom() * size[1];
		chunkorigin[2] = origin[2] + crandom() * size[2];
		ThrowDebris (self, "models/objects/debris2/tris.md2", 2, chunkorigin);
	}

	G_UseTargets (self, attacker);

	if (self->dmg)
		BecomeExplosion1 (self);
	else
		G_FreeEdict (self);
}
*/

/*
void func_explosive_use(g_entity_c *self, g_entity_c *other, g_entity_c *activator)
{
	func_explosive_explode (self, self, other, self->health, vec3_origin);
}
*/

/*
void func_explosive_spawn (g_entity_c *self, g_entity_c *other, g_entity_c *activator)
{
	self->r.solid = SOLID_BSP;
	self->r.svflags &= ~SVF_NOCLIENT;
	self->use = NULL;
	G_KillBox (self);
	gi.SV_LinkEdict (self);
}
*/

/*
void SP_func_explosive (g_entity_c *self)
{
	if (deathmatch->value)
	{	// auto-remove for deathmatch
		G_FreeEdict (self);
		return;
	}

	self->movetype = MOVETYPE_PUSH;

	gi.SV_ModelIndex ("models/objects/debris1/tris.md2");
	gi.SV_ModelIndex ("models/objects/debris2/tris.md2");

	gi.SV_SetModel (self, self->model);

	if (self->spawnflags & 1)
	{
		self->r.svflags |= SVF_NOCLIENT;
		self->r.solid = SOLID_NOT;
		self->use = func_explosive_spawn;
	}
	else
	{
		self->r.solid = SOLID_BSP;
		if (self->targetname)
			self->use = func_explosive_use;
	}

	if (self->spawnflags & 2)
		self->s.effects |= EF_ANIM_ALL;
	if (self->spawnflags & 4)
		self->s.effects |= EF_ANIM_ALLFAST;

	if (self->use != func_explosive_use)
	{
		if (!self->health)
			self->health = 100;
		self->die = func_explosive_explode;
		self->takedamage = DAMAGE_YES;
	}

	gi.SV_LinkEdict (self);
}
*/


//
// miscellaneous specialty items
//






/*QUAKED target_character (0 0 1) ?
used with target_string (must be on same "team")
"count" is position in the string (starts at 1)
*/
/*
void SP_target_character (g_entity_c *self)
{
	self->movetype = MOVETYPE_PUSH;
	gi.SV_SetModel (self, self->model);
	self->r.solid = SOLID_BSP;
	self->s.frame = 12;
	gi.SV_LinkEdict (self);
	return;
}
*/


/*QUAKED target_string (0 0 1) (-8 -8 -8) (8 8 8)
*/
/*
void target_string_use (g_entity_c *self, g_entity_c *other, g_entity_c *activator)
{
	g_entity_c *e;
	int		n, l;
	char	c;

	l = strlen(self->message);
	for (e = self->teammaster; e; e = e->teamchain)
	{
		if (!e->count)
			continue;
		n = e->count - 1;
		if (n > l)
		{
			e->s.frame = 12;
			continue;
		}

		c = self->message[n];
		if (c >= '0' && c <= '9')
			e->s.frame = c - '0';
		else if (c == '-')
			e->s.frame = 10;
		else if (c == ':')
			e->s.frame = 11;
		else
			e->s.frame = 12;
	}
}
*/

/*
void SP_target_string (g_entity_c *self)
{
	if (!self->message)
		self->message = "";
	self->use = target_string_use;
}
*/


/*QUAKED func_clock (0 0 1) (-8 -8 -8) (8 8 8) TIMER_UP TIMER_DOWN START_OFF MULTI_USE
target a target_string with this

The default is to be a time of day clock

TIMER_UP and TIMER_DOWN run for "count" seconds and the fire "pathtarget"
If START_OFF, this entity must be used before it starts

"style"		0 "xx"
			1 "xx:xx"
			2 "xx:xx:xx"
*/

//#define	CLOCK_MESSAGE_SIZE	16

// don't let field width of any clock messages change, or it
// could cause an overwrite after a game load

/*
static void func_clock_reset (g_entity_c *self)
{
	self->activator = NULL;
	if (self->spawnflags & 1)
	{
		self->health = 0;
		self->wait = self->count;
	}
	else if (self->spawnflags & 2)
	{
		self->health = self->count;
		self->wait = 0;
	}
}
*/

/*
static void func_clock_format_countdown (g_entity_c *self)
{
	if (self->style == 0)
	{
		Com_sprintf (self->message, CLOCK_MESSAGE_SIZE, "%2i", self->health);
		return;
	}

	if (self->style == 1)
	{
		Com_sprintf(self->message, CLOCK_MESSAGE_SIZE, "%2i:%2i", self->health / 60, self->health % 60);
		if (self->message[3] == ' ')
			self->message[3] = '0';
		return;
	}

	if (self->style == 2)
	{
		Com_sprintf(self->message, CLOCK_MESSAGE_SIZE, "%2i:%2i:%2i", self->health / 3600, (self->health - (self->health / 3600) * 3600) / 60, self->health % 60);
		if (self->message[3] == ' ')
			self->message[3] = '0';
		if (self->message[6] == ' ')
			self->message[6] = '0';
		return;
	}
}
*/

/*
void func_clock_think (g_entity_c *self)
{
	if (!self->enemy)
	{
		self->enemy = G_FindByTargetName (NULL, self->target);
		if (!self->enemy)
			return;
	}

	if (self->spawnflags & 1)
	{
		func_clock_format_countdown (self);
		self->health++;
	}
	else if (self->spawnflags & 2)
	{
		func_clock_format_countdown (self);
		self->health--;
	}
	else
	{
		struct tm	*ltime;
		time_t		gmtime;

		time(&gmtime);
		ltime = localtime(&gmtime);
		Com_sprintf (self->message, CLOCK_MESSAGE_SIZE, "%2i:%2i:%2i", ltime->tm_hour, ltime->tm_min, ltime->tm_sec);
		if (self->message[3] == ' ')
			self->message[3] = '0';
		if (self->message[6] == ' ')
			self->message[6] = '0';
	}

	self->enemy->message = self->message;
	self->enemy->use (self->enemy, self, self);

	if (((self->spawnflags & 1) && (self->health > self->wait)) ||
		((self->spawnflags & 2) && (self->health < self->wait)))
	{
		if (self->pathtarget)
		{
			char *savetarget;
			char *savemessage;

			savetarget = self->target;
			savemessage = self->message;
			self->target = self->pathtarget;
			self->message = NULL;
			G_UseTargets (self, self->activator);
			self->target = savetarget;
			self->message = savemessage;
		}

		if (!(self->spawnflags & 8))
			return;

		func_clock_reset (self);

		if (self->spawnflags & 4)
			return;
	}

	self->nextthink = level.time + 1;
}
*/

/*
void func_clock_use (g_entity_c *self, g_entity_c *other, g_entity_c *activator)
{
	if (!(self->spawnflags & 8))
		self->use = NULL;
	if (self->activator)
		return;
	self->activator = activator;
	self->think (self);
}
*/

/*
void SP_func_clock (g_entity_c *self)
{
	if (!self->target)
	{
		gi.Com_Printf("%s with no target at %s\n", self->classname, Vector3_String(self->s.origin));
		G_FreeEdict (self);
		return;
	}

	if ((self->spawnflags & 2) && (!self->count))
	{
		gi.Com_Printf("%s with no count at %s\n", self->classname, Vector3_String(self->s.origin));
		G_FreeEdict (self);
		return;
	}

	if ((self->spawnflags & 1) && (!self->count))
		self->count = 60*60;;

	func_clock_reset (self);

	self->message = (char*)gi.Z_TagMalloc (CLOCK_MESSAGE_SIZE, TAG_LEVEL);
	//self->message = (char*)gi.Z_Malloc(CLOCK_MESSAGE_SIZE);

	self->think = func_clock_think;

	if (self->spawnflags & 4)
		self->use = func_clock_use;
	else
		self->nextthink = level.time + 1;
}
*/




	
	


