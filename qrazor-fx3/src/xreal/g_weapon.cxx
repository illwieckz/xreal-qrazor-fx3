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
#include "g_projectile.h"


/*
=================
fire_hit

Used for all impact (hit/punch/slash) attacks
=================
*/
/*
bool	fire_hit(g_entity_c *self, vec3_t aim, int damage, int kick)
{
	trace_t		tr;
	//vec3_c		forward, right, up;
	vec3_c		v;
	vec3_c		point;
	float		range;
	vec3_c		dir;
	cbbox_c		bbox;

	//see if enemy is in range
	Vector3_Subtract (self->_enemy->_s.origin, self->_s.origin, dir);
	range = Vector3_Length(dir);
	if (range > aim[0])
		return false;

	if (aim[1] > self->_r.bbox._mins[0] && aim[1] < self->_r.bbox._maxs[0])
	{
		// the hit is straight on so back the range up to the edge of their bbox
		range -= self->_enemy->_r.bbox._maxs[0];
	}
	else
	{
		// this is a side hit so adjust the "right" value out to the edge of their bbox
		if (aim[1] < 0)
			aim[1] = self->_enemy->_r.bbox._mins[0];
		else
			aim[1] = self->_enemy->_r.bbox._maxs[0];
	}

	Vector3_MA(self->_s.origin, range, dir, point);

	tr = gi.SV_Trace(self->_s.origin, bbox, point, self, MASK_SHOT);
	if(tr.fraction < 1)
	{
		if (!((g_entity_c*)tr.ent)->_takedamage)
			return false;
			
		// if it will hit any client/monster then hit the one we wanted to hit
		if (tr.ent->_r.isclient)
			tr.ent = self->_enemy;
	}

	//Angles_ToVectors(self->_s.angles, forward, right, up); 
	
	
		
	Vector3_MA(self->_s.origin, range, forward, point);
	Vector3_MA(point, aim[1], right, point);
	Vector3_MA(point, aim[2], up, point);
	Vector3_Subtract (point, self->_enemy->_s.origin, dir);

	// do the damage
	((g_entity_c*)tr.ent)->takeDamage(self, self, dir, point, vec3_origin, damage, kick/2, DAMAGE_NO_KNOCKBACK, MOD_HIT);

	if(!tr.ent->_r.isclient)
		return false;

	// do our special form of knockback here
	Vector3_MA (self->_enemy->_r.bbox_abs._mins, 0.5, self->_enemy->_r.size, v);
	Vector3_Subtract (v, point, v);
	Vector3_Normalize (v);
	Vector3_MA (self->_enemy->_velocity, kick, v, self->_enemy->_velocity);
	if (self->_enemy->_velocity[2] > 0)
		self->_enemy->_groundentity = NULL;
	return true;
}
*/


/*
=================
fire_lead

This is an internal support routine used for bullet/pellet based weapons.
=================
*/
/*
static void	fire_lead(g_entity_c *self, vec3_t start, vec3_t aimdir, int damage, int kick, int te_impact, int hspread, int vspread, int mod)
{
	trace_t		tr;
	vec3_c		dir;
	vec3_c		forward, right, up;
	vec3_c		end;
	float		r;
	float		u;
	vec3_c		water_start;
	bool		water = false;
	int		content_mask = MASK_SHOT | MASK_WATER;
	cbbox_c		bbox;

	tr = gi.SV_Trace(self->_s.origin, bbox, start, self, MASK_SHOT);
	if(!(tr.fraction < 1.0))
	{
		vectoangles(aimdir, dir);
		Angles_ToVectors (dir, forward, right, up);

		r = crandom()*hspread;
		u = crandom()*vspread;
		Vector3_MA (start, 8192, forward, end);
		Vector3_MA (end, r, right, end);
		Vector3_MA (end, u, up, end);

		if(gi.SV_PointContents(start) & MASK_WATER)
		{
			water = true;
			Vector3_Copy (start, water_start);
			content_mask &= ~MASK_WATER;
		}

		tr = gi.SV_Trace(start, bbox, end, self, content_mask);

		// see if we hit water
		if(tr.contents & MASK_WATER)
		{
			int		color;

			water = true;
			Vector3_Copy (tr.endpos, water_start);

			if (!Vector3_Compare (start, tr.endpos))
			{
				if (tr.contents & X_CONT_WATER)
					color = SPLASH_BLUE_WATER;
				else if (tr.contents & X_CONT_SLIME)
					color = SPLASH_SLIME;
				else if (tr.contents & X_CONT_LAVA)
					color = SPLASH_LAVA;
				else
					color = SPLASH_UNKNOWN;

				if (color != SPLASH_UNKNOWN)
				{
					gi.SV_WriteByte (SVC_TEMP_ENTITY);
					gi.SV_WriteByte (TE_SPLASH);
					gi.SV_WriteByte (8);
					gi.SV_WritePosition (tr.endpos);
					gi.SV_WriteDir (tr.plane._normal);
					gi.SV_WriteByte (color);
					gi.SV_Multicast (tr.endpos, MULTICAST_PVS);
				}

				// change bullet's course when it enters water
				Vector3_Subtract (end, start, dir);
				vectoangles (dir, dir);
				Angles_ToVectors (dir, forward, right, up);
				r = crandom()*hspread*2;
				u = crandom()*vspread*2;
				Vector3_MA (water_start, 8192, forward, end);
				Vector3_MA (end, r, right, end);
				Vector3_MA (end, u, up, end);
			}

			// re-trace ignoring water this time
			tr = gi.SV_Trace (water_start, bbox, end, self, MASK_SHOT);
		}
	}

	// send gun puff / flash
	if(!((tr.surface) && (tr.surface->flags & SURF_SKY)))
	{
		if(tr.fraction < 1.0)
		{
			if(((g_entity_c*)tr.ent)->_takedamage)
			{
				((g_entity_c*)tr.ent)->takeDamage(self, self, aimdir, tr.endpos, tr.plane._normal, damage, kick, DAMAGE_BULLET, mod);
			}
			else
			{
				if(!(tr.surface->flags & SURF_NOIMPACT))
				{
					gi.SV_WriteByte(SVC_TEMP_ENTITY);
					gi.SV_WriteByte(te_impact);
					gi.SV_WritePosition(tr.endpos);
					gi.SV_WriteDir(tr.plane._normal);
					gi.SV_Multicast(tr.endpos, MULTICAST_PVS);

					//if(self->getClient())
					//	PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
				}
			}
		}
	}

	// if went through water, determine where the end and make a bubble trail
	if (water)
	{
		vec3_t	pos;

		Vector3_Subtract (tr.endpos, water_start, dir);
		Vector3_Normalize (dir);
		Vector3_MA (tr.endpos, -2, dir, pos);
		if(gi.SV_PointContents(pos) & MASK_WATER)
			Vector3_Copy(pos, tr.endpos);
		else
			tr = gi.SV_Trace(pos, bbox, water_start, tr.ent, MASK_WATER);

		Vector3_Add (water_start, tr.endpos, pos);
		Vector3_Scale (pos, 0.5, pos);

		gi.SV_WriteByte (SVC_TEMP_ENTITY);
		gi.SV_WriteByte (TE_BUBBLETRAIL);
		gi.SV_WritePosition (water_start);
		gi.SV_WritePosition (tr.endpos);
		gi.SV_Multicast (pos, MULTICAST_PVS);
	}
}
*/

/*
=================
fire_bullet

Fires a single round.  Used for machinegun and chaingun.  Would be fine for
pistols, rifles, etc....
=================
*/
/*
void fire_bullet (g_entity_c *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod)
{
	fire_lead(self, start, aimdir, damage, kick, TE_GUNSHOT, hspread, vspread, mod);
}
*/


/*
=================
fire_shotgun

Shoots shotgun pellets.  Used by shotgun and super shotgun.
=================
*/
/*
void	fire_shotgun(g_entity_c *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count, int mod)
{
#if 0
	for(int i=0; i<count; i++)
		fire_lead(self, start, aimdir, damage, kick, TE_SHOTGUN, hspread, vspread, mod);
#endif
}
*/

/*
void	fire_blaster(g_entity_c *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, bool hyper)
{
	vec3_c	s = start;
	vec3_c	d = dir;
	
	new g_projectile_bolt_c(self, s, d, damage, speed, effect);
}	
*/

/*
void	fire_grenade(g_entity_c *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
{
	g_entity_c	*grenade;
	vec3_c	s = start;
	vec3_c	d = aimdir;

	grenade = new g_projectile_grenade_c(self, s, d, damage, speed, timer, damage_radius);
}
*/

/*
void	fire_rocket(g_entity_c *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	vec3_c	s = start;
	vec3_c	d = dir;

	new g_projectile_rocket_c(self, s, d, damage, speed, damage_radius, radius_damage);
}
*/

/*
void	fire_rail(g_entity_c *self, vec3_t start, vec3_t aimdir, int damage, int kick)
{
#if 0
	vec3_c		from;
	vec3_c		end;
	trace_t		tr;
	g_entity_c		*ignore;
	int		mask;
	bool		water;
	cbbox_c		bbox;

	Vector3_MA (start, 8192, aimdir, end);
	Vector3_Copy (start, from);
	ignore = self;
	water = false;
	mask = MASK_SHOT|X_CONT_SLIME|X_CONT_LAVA;
	
	while(ignore)
	{
		tr = gi.SV_Trace(from, bbox, end, ignore, mask);

		if (tr.contents & (X_CONT_SLIME|X_CONT_LAVA))
		{
			mask &= ~(X_CONT_SLIME|X_CONT_LAVA);
			water = true;
		}
		else
		{
			//ZOID--added so rail goes through SOLID_BBOX entities (gibs, etc)
			if((tr.ent->_r.isclient) || (tr.ent->_r.solid == SOLID_BBOX))
				ignore = (g_entity_c*)tr.ent;
			else
				ignore = NULL;

			if ((tr.ent != self) && (((g_entity_c*)tr.ent)->_takedamage))
				((g_entity_c*)tr.ent)->takeDamage(self, self, aimdir, tr.endpos, tr.plane._normal, damage, kick, 0, MOD_RAILGUN);
		}

		Vector3_Copy(tr.endpos, from);
	}

	// send gun puff / flash
	gi.SV_WriteByte (SVC_TEMP_ENTITY);
	gi.SV_WriteByte (TE_RAILTRAIL);
	gi.SV_WritePosition (start);
	gi.SV_WritePosition (tr.endpos);
	gi.SV_Multicast (self->_s.origin, MULTICAST_PHS);
//	gi.SV_Multicast (start, MULTICAST_PHS);
	
	if (water)
	{
		gi.SV_WriteByte (SVC_TEMP_ENTITY);
		gi.SV_WriteByte (TE_RAILTRAIL);
		gi.SV_WritePosition (start);
		gi.SV_WritePosition (tr.endpos);
		gi.SV_Multicast (tr.endpos, MULTICAST_PHS);
	}

	//if (self->getClient())
	//	PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
#endif
}
*/


/*
void bfg_explode (g_entity_c *self)
{
	g_entity_c	*ent;
	float	points;
	vec3_t	v;
	float	dist;

	if (self->s.frame == 0)
	{
		// the BFG effect
		ent = NULL;
		while ((ent = findradius(ent, self->s.origin, self->dmg_radius)) != NULL)
		{
			if (!ent->takedamage)
				continue;
			if (ent == self->r.owner)
				continue;
			if (!CanDamage (ent, self))
				continue;
			if (!CanDamage (ent, self->r.owner))
				continue;

			Vector3_Add (ent->r.bbox._mins, ent->r.bbox._maxs, v);
			Vector3_MA (ent->s.origin, 0.5, v, v);
			Vector3_Subtract (self->s.origin, v, v);
			dist = Vector3_Length(v);
			points = self->radius_dmg * (1.0 - sqrt(dist/self->dmg_radius));
			if (ent == self->r.owner)
				points = points * 0.5;

			gi.SV_WriteByte (SVC_TEMP_ENTITY);
			gi.SV_WriteByte (TE_BFG_EXPLOSION);
			gi.SV_WritePosition (ent->s.origin);
			gi.SV_Multicast (ent->s.origin, MULTICAST_PHS);
			T_Damage (ent, self, self->r.owner, self->velocity, ent->s.origin, vec3_origin, (int)points, 0, DAMAGE_ENERGY, MOD_BFG_EFFECT);
		}
	}

	self->nextthink = level.time + FRAMETIME;
	self->s.frame++;
	
	if (self->s.frame == 5)
		self->think = G_FreeEdict;
}
*/

/*
void bfg_touch (g_entity_c *self, g_entity_c *other, cplane_c *plane, csurface_c *surf)
{
	if (other == self->r.owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->r.owner->r.client)
		PlayerNoise(self->r.owner, self->s.origin, PNOISE_IMPACT);

	// core explosion - prevents firing it into the wall/floor
	if (other->takedamage)
		T_Damage (other, self, self->r.owner, self->velocity, self->s.origin, plane->_normal, 200, 0, 0, MOD_BFG_BLAST);
	T_RadiusDamage(self, self->r.owner, 200, other, 100, MOD_BFG_BLAST);

	gi.SV_StartSound (NULL, self, CHAN_VOICE, gi.SV_SoundIndex ("weapons/bfg__x1b.wav"), 1, ATTN_NORM, 0);
	self->r.solid = SOLID_NOT;
	self->touch = NULL;
	Vector3_MA (self->s.origin, -1 * FRAMETIME, self->velocity, self->s.origin);
	self->velocity.clear();
	self->s.modelindex = gi.SV_ModelIndex ("sprites/s_bfg3.sp2");
	self->s.frame = 0;
	self->s.sound = 0;
	self->s.effects &= ~EF_ANIM_ALLFAST;
	self->think = bfg_explode;
	self->nextthink = level.time + FRAMETIME;
	self->enemy = other;

	gi.SV_WriteByte (SVC_TEMP_ENTITY);
	gi.SV_WriteByte (TE_BFG_BIGEXPLOSION);
	gi.SV_WritePosition (self->s.origin);
	gi.SV_Multicast (self->s.origin, MULTICAST_PVS);
}
*/

/*
void bfg_think (g_entity_c *self)
{
	g_entity_c	*ent;
	g_entity_c	*ignore;
	vec3_c	point;
	vec3_c	dir;
	vec3_c	start;
	vec3_c	end;
	int	dmg;
	trace_t	tr;
	cbbox_c	bbox;

	if (deathmatch->value)
		dmg = 5;
	else
		dmg = 10;

	ent = NULL;
	while ((ent = findradius(ent, self->s.origin, 256)) != NULL)
	{
		if (ent == self)
			continue;

		if (ent == self->r.owner)
			continue;

		if (!ent->takedamage)
			continue;

		if ((!ent->r.client) && (strcmp(ent->classname, "misc_explobox") != 0))
			continue;

		Vector3_MA (ent->r.bbox_abs._mins, 0.5, ent->r.size, point);

		Vector3_Subtract (point, self->s.origin, dir);
		Vector3_Normalize (dir);

		ignore = self;
		Vector3_Copy (self->s.origin, start);
		Vector3_MA (start, 2048, dir, end);
		while(1)
		{
			tr = gi.SV_Trace (start, bbox, end, ignore, MASK_SHOT);

			if (!tr.ent)
				break;

			// hurt it if we can
			if ((tr.ent->takedamage) && !(tr.ent->flags & FL_IMMUNE_LASER) && (tr.ent != self->r.owner))
				T_Damage (tr.ent, self, self->r.owner, dir, tr.endpos, vec3_origin, dmg, 1, DAMAGE_ENERGY, MOD_BFG_LASER);

			// if we hit something that's not a monster or player we're done
			if (!tr.ent->r.client)
			{
				gi.SV_WriteByte (SVC_TEMP_ENTITY);
				gi.SV_WriteByte (TE_LASER_SPARKS);
				gi.SV_WriteByte (4);
				gi.SV_WritePosition (tr.endpos);
				gi.SV_WriteDir (tr.plane._normal);
				gi.SV_WriteByte (self->s.skinnum);
				gi.SV_Multicast (tr.endpos, MULTICAST_PVS);
				break;
			}

			ignore = tr.ent;
			Vector3_Copy (tr.endpos, start);
		}

		gi.SV_WriteByte (SVC_TEMP_ENTITY);
		gi.SV_WriteByte (TE_BFG_LASER);
		gi.SV_WritePosition (self->s.origin);
		gi.SV_WritePosition (tr.endpos);
		gi.SV_Multicast (self->s.origin, MULTICAST_PHS);
	}

	self->nextthink = level.time + FRAMETIME;
}
*/

/*
void fire_bfg (g_entity_c *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius)
{
	g_entity_c	*bfg;

	//bfg = G_Spawn();
	bfg = new g_entity_c();
	Vector3_Copy (start, bfg->s.origin);
	Vector3_Copy (dir, bfg->movedir);
	vectoangles (dir, bfg->s.angles);
	Vector3_Scale (dir, speed, bfg->velocity);
	bfg->movetype = MOVETYPE_FLYMISSILE;
	bfg->r.clipmask = MASK_SHOT;
	bfg->r.solid = SOLID_BBOX;
	bfg->s.effects |= EF_BFG | EF_ANIM_ALLFAST;
	bfg->r.bbox.zero();
	bfg->s.modelindex = gi.SV_ModelIndex ("sprites/s_bfg1.sp2");
	bfg->r.owner = self;
	bfg->touch = bfg_touch;
	bfg->nextthink = level.time + 8000/speed;
	bfg->think = G_FreeEdict;
	bfg->radius_dmg = damage;
	bfg->dmg_radius = damage_radius;
	bfg->classname = "bfg blast";
	bfg->s.sound = gi.SV_SoundIndex ("weapons/bfg__l1a.wav");

	bfg->think = bfg_think;
	bfg->nextthink = level.time + FRAMETIME;
	bfg->teammaster = bfg;
	bfg->teamchain = NULL;

	gi.SV_LinkEdict (bfg);
}
*/
