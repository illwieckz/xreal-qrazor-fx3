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



bool 	G_OnSameTeam(g_entity_c *ent1, g_entity_c *ent2)
{
	std::string	ent1Team;
	std::string	ent2Team;

	if(!((int)(dmflags->getInteger()) & (DF_MODELTEAMS | DF_SKINTEAMS)))
		return false;

	ent1Team = ent1->clientTeam();
	ent2Team = ent2->clientTeam();

	if(strcmp(ent1Team.c_str(), ent2Team.c_str()) == 0)
		return true;
	else
		return false;
}


/*
============
CanDamage

Returns true if the inflictor can directly damage the target.  Used for
explosions and melee attacks.
============
*/
bool	G_CanDamage(g_entity_c *targ, g_entity_c *inflictor)
{
	/*
	vec3_c	dest;
	trace_t	trace;
	cbbox_c	bbox;
	*/

	// bmodels need special checking because their origin is 0,0,0
	/*
	if(targ->_movetype == MOVETYPE_PUSH)
	{
		dest = targ->_r.bbox_abs._mins + targ->_r.bbox_abs._maxs;
		Vector3_Scale (dest, 0.5, dest);
		trace = trap_SV_Trace (inflictor->_s.origin, bbox, dest, inflictor, MASK_SOLID);
		
		if (trace.fraction == 1.0)
			return true;
			
		if (trace.ent == targ)
			return true;
			
		return false;
	}
	*/
	
	/*
	trace = trap_SV_Trace(inflictor->_s.origin, bbox, targ->_s.origin, inflictor, MASK_SOLID);
	if(trace.fraction == 1.0)
		return true;

	Vector3_Copy(targ->_s.origin, dest);
	dest[0] += 15.0;
	dest[1] += 15.0;
	trace = trap_SV_Trace(inflictor->_s.origin, bbox, dest, inflictor, MASK_SOLID);
	if(trace.fraction == 1.0)
		return true;

	Vector3_Copy(targ->_s.origin, dest);
	dest[0] += 15.0;
	dest[1] -= 15.0;
	trace = trap_SV_Trace(inflictor->_s.origin, bbox, dest, inflictor, MASK_SOLID);
	if(trace.fraction == 1.0)
		return true;

	Vector3_Copy(targ->_s.origin, dest);
	dest[0] -= 15.0;
	dest[1] += 15.0;
	trace = trap_SV_Trace(inflictor->_s.origin, bbox, dest, inflictor, MASK_SOLID);
	if(trace.fraction == 1.0)
		return true;

	Vector3_Copy(targ->_s.origin, dest);
	dest[0] -= 15.0;
	dest[1] -= 15.0;
	trace = trap_SV_Trace(inflictor->_s.origin, bbox, dest, inflictor, MASK_SOLID);
	if(trace.fraction == 1.0)
		return true;
	*/

	return false;
}


/*
============
Killed
============
*/
void 	G_Killed(g_entity_c *targ, g_entity_c *inflictor, g_entity_c *attacker, int damage, vec3_t point)
{
	if(targ->_health < -999)
		targ->_health = -999;

	targ->_enemy = attacker;

	
	if(targ->_movetype == MOVETYPE_PUSH || targ->_movetype == MOVETYPE_STOP || targ->_movetype == MOVETYPE_NONE)
	{	
		// doors, triggers, etc
		targ->die(inflictor, attacker, damage, point);
		return;
	}
	
	targ->die(inflictor, attacker, damage, point);
}


/*
================
SpawnDamage
================
*/
void 	G_SpawnDamage(int type, vec3_t origin, vec3_t normal, int damage)
{
	if (damage > 255)
		damage = 255;
		
	trap_SV_WriteBits(SVC_TEMP_ENTITY, svc_bitcount);
	trap_SV_WriteByte(type);
//	trap_SV_WriteByte(damage);
	trap_SV_WritePosition(origin);
	trap_SV_WriteDir(normal);
	trap_SV_Multicast(origin, MULTICAST_PVS);
}


/*
============
T_Damage

targ		entity that is being damaged
inflictor	entity that is causing the damage
attacker	entity that caused the inflictor to damage targ
	example: targ=monster, inflictor=rocket, attacker=player

dir			direction of the attack
point		point at which the damage is being inflicted
normal		normal vector from that point
damage		amount of damage being inflicted
knockback	force to be applied against targ as a result of the damage

dflags		these flags are used to control how T_Damage works
	DAMAGE_RADIUS			damage was indirect (from a nearby explosion)
	DAMAGE_NO_ARMOR			armor does not protect from this damage
	DAMAGE_ENERGY			damage is from an energy based weapon
	DAMAGE_NO_KNOCKBACK		do not affect velocity, just view angles
	DAMAGE_BULLET			damage is from a bullet (used for ricochets)
	DAMAGE_NO_PROTECTION	kills godmode, armor, everything
============
*/
/*
static int 	CheckPowerArmor(g_entity_c *ent, vec3_t point, vec3_t normal, int damage, int dflags)
{
	g_client_c	*client;
	int			save;
	int			power_armor_type;
	int			damagePerCell;
	int			pa_te_type;
	int			power;
	int			power_used;
	int			index = 0;

	if (!damage)
		return 0;

	client = ent->getClient();

	if (dflags & DAMAGE_NO_ARMOR)
		return 0;

	if (client)
	{
		power_armor_type = PowerArmorType (ent);
		if (power_armor_type != POWER_ARMOR_NONE)
		{
			index = G_GetNumForItem(G_FindItem("Cells"));
			power = client->pers.inventory[index];
		}
	}
	else
		return 0;

	if (power_armor_type == POWER_ARMOR_NONE)
		return 0;
	if (!power)
		return 0;

	if (power_armor_type == POWER_ARMOR_SCREEN)
	{
		vec3_c		vec;
		float		dot;
		vec3_c		forward, right, up;

		// only works if damage point is in front
		Angles_ToVectors (ent->_s.angles, forward, right, up);
		Vector3_Subtract (point, ent->_s.origin, vec);
		Vector3_Normalize (vec);
		dot = Vector3_DotProduct (vec, forward);
		if (dot <= 0.3)
			return 0;

		damagePerCell = 1;
		pa_te_type = TE_SCREEN_SPARKS;
		damage = damage / 3;
	}
	else
	{
		damagePerCell = 2;
		pa_te_type = TE_SHIELD_SPARKS;
		damage = (2 * damage) / 3;
	}

	save = power * damagePerCell;
	if (!save)
		return 0;
	if (save > damage)
		save = damage;

	SpawnDamage (pa_te_type, point, normal, save);
	ent->_powerarmor_time = level.time + 0.2;

	power_used = save / damagePerCell;

	if (client)
		client->pers.inventory[index] -= power_used;
	
	return save;
}
*/

/*
static int 	CheckArmor (g_entity_c *ent, vec3_t point, vec3_t normal, int damage, int te_sparks, int dflags)
{
	g_client_c	*client;
	int			save;
	int			index;
	g_item_c		*armor;

	if (!damage)
		return 0;

	client = ent->getClient();

	if (!client)
		return 0;

	if (dflags & DAMAGE_NO_ARMOR)
		return 0;

	index = ArmorIndex (ent);
	if (!index)
		return 0;

	armor = G_GetItemByNum (index);

	if (dflags & DAMAGE_ENERGY)
		save = (int)ceil(((gitem_armor_t*)armor->getInfo())->energy_protection*damage);
	else
		save = (int)ceil(((gitem_armor_t*)armor->getInfo())->normal_protection*damage);
		
	if (save >= client->pers.inventory[index])
		save = client->pers.inventory[index];

	if (!save)
		return 0;

	client->pers.inventory[index] -= save;
	SpawnDamage (te_sparks, point, normal, save);

	return save;
}
*/


/*
bool 	CheckTeamDamage (g_entity_c *targ, g_entity_c *attacker)
{
		//FIXME make the next line real and uncomment this block
		// if ((ability to damage a teammate == OFF) && (targ's team == attacker's team))
	return false;
}
*/

#if 0
void 	T_Damage(g_entity_c *targ, g_entity_c *inflictor, g_entity_c *attacker, vec3_t dir, vec3_t point, vec3_t normal, int damage, int knockback, int dflags, int mod)
{
	//g_client_c	*client;
	int			take;
	int			save;
	//int			asave;
	//int			psave;
	int			te_sparks;

	if(!targ->_takedamage)
		return;

	// friendly fire avoidance
	// if enabled you can't hurt teammates (but you can hurt yourself)
	// knockback still occurs
	if((targ != attacker) && ((deathmatch->value && ((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS))) || coop->value))
	{
		if(G_OnSameTeam(targ, attacker))
		{
			if ((int)(dmflags->value) & DF_NO_FRIENDLY_FIRE)
				damage = 0;
			else
				mod |= MOD_FRIENDLY_FIRE;
		}
	}
	meansOfDeath = mod;

	// easy mode takes half damage
	if(skill->value == 0 && deathmatch->value == 0 && targ->getClient())
	{
		damage *= (int)0.5;
		if(!damage)
			damage = 1;
	}

	//client = targ->getClient();

	if(dflags & DAMAGE_BULLET)
		te_sparks = TE_BULLET_SPARKS;
	else
		te_sparks = TE_SPARKS;

	Vector3_Normalize(dir);

	
	if(targ->_flags & FL_NO_KNOCKBACK)
		knockback = 0;

	// figure momentum add
	if(!(dflags & DAMAGE_NO_KNOCKBACK))
	{
		if((knockback) && (targ->_movetype != MOVETYPE_NONE) && (targ->_movetype != MOVETYPE_BOUNCE) && (targ->_movetype != MOVETYPE_PUSH) && (targ->_movetype != MOVETYPE_STOP))
		{
			vec3_t	kvel;
			float	mass;

			if(targ->_mass < 50)
				mass = 50;
			else
				mass = targ->_mass;

			if(targ->getClient()  && attacker == targ)
				Vector3_Scale (dir, 1600.0 * (float)knockback / mass, kvel);	// the rocket jump hack...
			else
				Vector3_Scale (dir, 500.0 * (float)knockback / mass, kvel);

			Vector3_Add (targ->_velocity, kvel, targ->_velocity);
		}
	}

	take = damage;
	save = 0;

	// check for godmode
	if((targ->_flags & FL_ULTRAMANMODE) && !(dflags & DAMAGE_NO_PROTECTION))
	{
		take = 0;
		save = damage;
		SpawnDamage(te_sparks, point, normal, save);
	}

	// check for invincibility
	/*
	if((client && client->invincible_framenum > level.framenum ) && !(dflags & DAMAGE_NO_PROTECTION))
	{
		if (targ->_pain_debounce_time < level.time)
		{
			trap_SV_StartSound (NULL, targ, CHAN_ITEM, trap_SV_SoundIndex("items/protect4.wav"), 1, ATTN_NORM, 0);
			targ->_pain_debounce_time = level.time + 2;
		}
		
		take = 0;
		save = damage;
	}
	*/

	//psave = CheckPowerArmor (targ, point, normal, take, dflags);
	//take -= psave;

	asave = CheckArmor (targ, point, normal, take, te_sparks, dflags);
	take -= asave;

	//treat cheat/powerup savings the same as armor
	asave += save;

	// team damage avoidance
	if(!(dflags & DAMAGE_NO_PROTECTION) && CheckTeamDamage(targ, attacker))
		return;

	// do the damage
	if(take)
	{
		if(client)
			SpawnDamage(TE_BLOOD, point, normal, take);
		else
			SpawnDamage(te_sparks, point, normal, take);


		targ->_health = targ->_health - take;
			
		if (targ->_health <= 0)
		{
			if (client)
				targ->_flags |= FL_NO_KNOCKBACK;
				
			Killed (targ, inflictor, attacker, take, point);
			return;
		}
	}

	/*
	if(client)
	{
		if(!(targ->_flags & FL_ULTRAMANMODE) && (take))
			targ->pain (attacker, knockback, take);
	}
	else */
	if(take)
	{
		if(targ->hasPain())
			targ->pain(attacker, knockback, take);
	}

	// add to the damage inflicted on a player this frame
	// the total will be turned into screen blends and view angle kicks
	// at the end of the frame
	/*
	if(client)
	{
		client->damage_parmor += psave;
		client->damage_armor += asave;
		client->damage_blood += take;
		client->damage_knockback += knockback;
		client->damage_from = point;
	}
	*/
}
#endif


/*
============
T_RadiusDamage
============
*/
void	G_RadiusDamage(g_entity_c *inflictor, g_entity_c *attacker, float damage, g_entity_c *ignore, float radius, int mod)
{
	float	points;
	g_entity_c	*ent = NULL;
	vec3_c	v;
	vec3_c	dir;

	while((ent = G_FindByRadius(ent, inflictor->_s.origin, radius)) != NULL)
	{
		if (ent == ignore)
			continue;
			
		if (!ent->_takedamage)
			continue;

		vec3_c v = ent->_r.bbox._mins + ent->_r.bbox._maxs;
		Vector3_MA (ent->_s.origin, 0.5, v, v);
		Vector3_Subtract (inflictor->_s.origin, v, v);
		points = damage - 0.5 * Vector3_Length (v);
		
		if(ent == attacker)
			points = points * 0.5;
			
		if(points > 0)
		{
			if(G_CanDamage(ent, inflictor))
			{
				vec3_c dir = ent->_s.origin - inflictor->_s.origin;
				
				ent->takeDamage(inflictor, attacker, dir, inflictor->_s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
			}
		}
	}
}
