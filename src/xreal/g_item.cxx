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
#include "g_item.h"
#include "g_player.h"
#include "g_local.h"



g_item_c::~g_item_c()
{
	//DO NOTHING
}

cskel_animation_c*	g_item_c::registerAnimation(const std::string &name)
{
	_precaches.push_back(name);
	return trap_CM_RegisterAnimation(name);
}


/*
================================================================================
				NORMAL ARMOR
================================================================================
*/

/*
class g_item_armor_c : public g_item_c
{
public:
	g_item_armor_c();
	
	virtual bool	pickup(g_entity_c *ent, g_entity_c *other);
	//virtual void	use(g_entity_c *ent, g_item_c *item)	{};
	//virtual void	drop(g_entity_c *ent, g_item_c *item)	{};
	//virtual void	weaponthink(g_entity_c *ent)		{};
};


class g_item_armor_body_c : public g_item_armor_c
{
public:
	g_item_armor_body_c();
};


class g_item_armor_combat_c : public g_item_armor_c
{
public:
	g_item_armor_combat_c();
};


class g_item_armor_jacket_c : public g_item_armor_c
{
public:
	g_item_armor_jacket_c();
};


class g_item_armor_shard_c : public g_item_armor_c
{
public:
	g_item_armor_shard_c();
};
*/


/*
================================================================================
				POWER ARMOR
================================================================================
*/
/*
class g_item_power_armor_c : public g_item_c
{
public:
	g_item_power_armor_c();

	virtual bool	pickup(g_entity_c *ent, g_entity_c *other);
	virtual void	use(g_entity_c *ent, g_item_c *item);
	virtual void	drop(g_entity_c *ent, g_item_c *item);
	//virtual void	weaponthink(g_entity_c *ent)		{};
};


class g_item_power_screen_c : public g_item_power_armor_c
{
public:
	g_item_power_screen_c();
};


class g_item_power_shield_c : public g_item_power_armor_c
{
public:
	g_item_power_shield_c();
};
*/


/*
================================================================================
				AMMO
================================================================================
*/
class g_item_ammo_c : public g_item_c
{
public:
	g_item_ammo_c()
	{
		_has_pickup		= true;
		_has_drop		= true;
	}
	
	virtual bool	pickup(g_entity_c *ent, g_player_c *other);
	virtual void	drop(g_player_c *player);
};


class g_item_ammo_shells_c : public g_item_ammo_c
{
public:
	g_item_ammo_shells_c();
};


class g_item_ammo_bullets_c : public g_item_ammo_c
{
public:
	g_item_ammo_bullets_c();
};


class g_item_ammo_cells_c : public g_item_ammo_c
{
public:
	g_item_ammo_cells_c();
};


class g_item_ammo_rockets_c : public g_item_ammo_c
{
public:
	g_item_ammo_rockets_c();
};


/*
class g_item_ammo_grenades_c : public g_item_ammo_c
{
public:
	g_item_ammo_grenades_c();
};
*/


class g_item_ammo_slugs_c : public g_item_ammo_c
{
public:
	g_item_ammo_slugs_c();
};






gitem_armor_t jacketarmor_info	= { 25,  50, .30, .00, ARMOR_JACKET};
gitem_armor_t combatarmor_info	= { 50, 100, .60, .30, ARMOR_COMBAT};
gitem_armor_t bodyarmor_info	= {100, 200, .80, .60, ARMOR_BODY};


#define HEALTH_IGNORE_MAX	1
#define HEALTH_TIMED		2



int	G_GetNumForItem(g_item_c *item)
{
	if(!item)
	{
		//trap_Com_Error(ERR_DROP, "G_GetNumForItem: NULL parameter\n");
		return -1;
	}

	for(unsigned int i=0; i<g_items.size(); i++)
	{
		if(g_items[i] == item)
			return i;
	}
	
	trap_Com_Error(ERR_DROP, "G_GetNumForItem: bad pointer\n");
	return -1;
}


g_item_c*	G_GetItemByNum(int num)
{
	/*
	if (index == 0 || index >= game.num_items)
		return NULL;

	return &itemlist[index]
	*/

	if(num < 0 || num >= (int)g_items.size())
	{
		//trap_Com_Error(ERR_DROP, "G_GetItemByNum: bad number %i\n", num);
		return NULL;
	}

	return g_items[num];
}

g_item_c*	G_FindItem(const std::string &pickup_name)
{
	for(std::vector<g_item_c*>::const_iterator ir = g_items.begin(); ir != g_items.end(); ir++)
	{
		g_item_c *item = *ir;
		
		if(!item->getPickupName())
			continue;
			
		if(X_strcaseequal(item->getPickupName(), pickup_name.c_str()))
			return item;
	}

	trap_Com_Error(ERR_DROP, "G_FindItem: no item '%s'\n", pickup_name.c_str());
	return NULL;
}


g_item_c*	G_FindItemByClassname(const std::string &classname)
{
	for(std::vector<g_item_c*>::const_iterator ir = g_items.begin(); ir != g_items.end(); ir++)
	{
		g_item_c *item = *ir;
		
		if(!item->getClassname())
			continue;
			
		if(X_strcaseequal(item->getClassname(), classname.c_str()))
			return item;
	}

	trap_Com_Error(ERR_DROP, "G_FindItemByClassname: no item '%s'\n", classname.c_str());
	return NULL;
}




/*
void	DoRespawn (g_entity_c *ent)
{
	if (ent->team)
	{
		g_entity_c	*master;
		int	count;
		int choice;

		master = ent->teammaster;

		for (count = 0, ent = master; ent; ent = ent->chain, count++)
			;

		choice = rand() % count;

		for (count = 0, ent = master; count < choice; ent = ent->chain, count++)
			;
	}

	ent->r.svflags &= ~SVF_NOCLIENT;
	ent->r.solid = SOLID_TRIGGER;
	trap_SV_LinkEdict (ent);

	// send an effect
	ent->s.event = EV_ITEM_RESPAWN;
}
*/


void	SetRespawn (g_entity_c *ent, float delay)
{
#if 0
	ent->flags |= FL_RESPAWN;
	ent->r.svflags |= SVF_NOCLIENT;
	ent->r.solid = SOLID_NOT;
	ent->nextthink = level.time + delay;
	ent->think = DoRespawn;
	trap_SV_LinkEdict (ent);
#else
	ent->remove();
#endif
}



//======================================================================

/*
bool	Pickup_Powerup(g_entity_c *ent, g_player_c *other)
{
	int		quantity;

	quantity = other->_pers.inventory[G_GetNumForItem(ent->_item)];
	
	if((skill->getInteger() == 1 && quantity >= 2) || (skill->getInteger() >= 2 && quantity >= 1))
		return false;

	if((coop->getInteger()) && (ent->_item->getFlags() & IT_STAY_COOP) && (quantity > 0))
		return false;

	other->_pers.inventory[G_GetNumForItem(ent->_item)]++;

	if (deathmatch->getInteger())
	{
		if (!(ent->_spawnflags & DROPPED_ITEM) )
			SetRespawn (ent, ent->_item->getQuantity());
			
		if (((int)dmflags->getInteger() & DF_INSTANT_ITEMS) || (!X_stricmp(ent->_item->getClassname(), "item_quad") && (ent->_spawnflags & DROPPED_PLAYER_ITEM)))
		{
			if (!X_stricmp(ent->_item->getClassname(), "item_quad") && (ent->_spawnflags & DROPPED_PLAYER_ITEM))
				quad_drop_timeout_hack = (int)((ent->_nextthink - level.time) / FRAMETIME);
				
			ent->_item->use (other, ent->_item);
		}
	}

	return true;
}
*/

/*
void	Drop_General(g_entity_c *ent, g_item_c *item)
{
	G_DropItem (ent, item);
	ent->getClient()->pers.inventory[G_GetNumForItem(item)]--;
	ValidateSelectedItem (ent);
}
*/


//======================================================================

/*
bool	Pickup_Adrenaline (g_entity_c *ent, g_entity_c *other)
{
	if (!deathmatch->getInteger())
		other->_max_health += 1;

	if (other->_health < other->_max_health)
		other->_health = other->_max_health;

	if (!(ent->_spawnflags & DROPPED_ITEM) && (deathmatch->getInteger()))
		SetRespawn (ent, ent->_item->getQuantity());

	return true;
}
*/

/*
bool	Pickup_AncientHead (g_entity_c *ent, g_entity_c *other)
{
	other->_max_health += 2;

	if (!(ent->_spawnflags & DROPPED_ITEM) && (deathmatch->getInteger()))
		SetRespawn (ent, ent->_item->getQuantity());

	return true;
}
*/

/*
bool	Pickup_Bandolier (g_entity_c *ent, g_entity_c *other)
{
	g_item_c	*item;
	int		index;

	if (other->getClient()->pers.max_bullets < 250)
		other->getClient()->pers.max_bullets = 250;
		
	if (other->getClient()->pers.max_shells < 150)
		other->getClient()->pers.max_shells = 150;
		
	if (other->getClient()->pers.max_cells < 250)
		other->getClient()->pers.max_cells = 250;
		
	if (other->getClient()->pers.max_slugs < 75)
		other->getClient()->pers.max_slugs = 75;

	item = G_FindItem("Bullets");
	if (item)
	{
		index = G_GetNumForItem(item);
		other->getClient()->pers.inventory[index] += item->getQuantity();
		if (other->getClient()->pers.inventory[index] > other->getClient()->pers.max_bullets)
			other->getClient()->pers.inventory[index] = other->getClient()->pers.max_bullets;
	}

	item = G_FindItem("Shells");
	if (item)
	{
		index = G_GetNumForItem(item);
		other->getClient()->pers.inventory[index] += item->getQuantity();
		if (other->getClient()->pers.inventory[index] > other->getClient()->pers.max_shells)
			other->getClient()->pers.inventory[index] = other->getClient()->pers.max_shells;
	}

	if (!(ent->_spawnflags & DROPPED_ITEM) && (deathmatch->getInteger()))
		SetRespawn (ent, ent->_item->getQuantity());

	return true;
}
*/

/*
bool	Pickup_Pack (g_entity_c *ent, g_entity_c *other)
{
	g_item_c	*item;
	int		index;

	if (other->getClient()->pers.max_bullets < 300)
		other->getClient()->pers.max_bullets = 300;
		
	if (other->getClient()->pers.max_shells < 200)
		other->getClient()->pers.max_shells = 200;
		
	if (other->getClient()->pers.max_rockets < 100)
		other->getClient()->pers.max_rockets = 100;
		
	if (other->getClient()->pers.max_grenades < 100)
		other->getClient()->pers.max_grenades = 100;
		
	if (other->getClient()->pers.max_cells < 300)
		other->getClient()->pers.max_cells = 300;
		
	if (other->getClient()->pers.max_slugs < 100)
		other->getClient()->pers.max_slugs = 100;

	item = G_FindItem("Bullets");
	if (item)
	{
		index = G_GetNumForItem(item);
		other->getClient()->pers.inventory[index] += item->getQuantity();
		if (other->getClient()->pers.inventory[index] > other->getClient()->pers.max_bullets)
			other->getClient()->pers.inventory[index] = other->getClient()->pers.max_bullets;
	}

	item = G_FindItem("Shells");
	if (item)
	{
		index = G_GetNumForItem(item);
		other->getClient()->pers.inventory[index] += item->getQuantity();
		if (other->getClient()->pers.inventory[index] > other->getClient()->pers.max_shells)
			other->getClient()->pers.inventory[index] = other->getClient()->pers.max_shells;
	}

	item = G_FindItem("Cells");
	if (item)
	{
		index = G_GetNumForItem(item);
		other->getClient()->pers.inventory[index] += item->getQuantity();
		if (other->getClient()->pers.inventory[index] > other->getClient()->pers.max_cells)
			other->getClient()->pers.inventory[index] = other->getClient()->pers.max_cells;
	}

	item = G_FindItem("Grenades");
	if (item)
	{
		index = G_GetNumForItem(item);
		other->getClient()->pers.inventory[index] += item->getQuantity();
		if (other->getClient()->pers.inventory[index] > other->getClient()->pers.max_grenades)
			other->getClient()->pers.inventory[index] = other->getClient()->pers.max_grenades;
	}

	item = G_FindItem("Rockets");
	if (item)
	{
		index = G_GetNumForItem(item);
		other->getClient()->pers.inventory[index] += item->getQuantity();
		if (other->getClient()->pers.inventory[index] > other->getClient()->pers.max_rockets)
			other->getClient()->pers.inventory[index] = other->getClient()->pers.max_rockets;
	}

	item = G_FindItem("Slugs");
	if (item)
	{
		index = G_GetNumForItem(item);
		other->getClient()->pers.inventory[index] += item->getQuantity();
		if (other->getClient()->pers.inventory[index] > other->getClient()->pers.max_slugs)
			other->getClient()->pers.inventory[index] = other->getClient()->pers.max_slugs;
	}

	if (!(ent->_spawnflags & DROPPED_ITEM) && (deathmatch->getInteger()))
		SetRespawn (ent, ent->_item->getQuantity());

	return true;
}
*/

//======================================================================

/*
void Use_Quad (g_entity_c *ent, g_item_c *item)
{
	int		timeout;

	ent->getClient()->pers.inventory[G_GetNumForItem(item)]--;
	ValidateSelectedItem (ent);

	if (quad_drop_timeout_hack)
	{
		timeout = quad_drop_timeout_hack;
		quad_drop_timeout_hack = 0;
	}
	else
	{
		timeout = 300;
	}

	if (ent->getClient()->quad_framenum > level.framenum)
		ent->getClient()->quad_framenum += timeout;
	else
		ent->getClient()->quad_framenum = level.framenum + timeout;

	trap_SV_StartSound(NULL, ent, CHAN_ITEM, trap_SV_SoundIndex("items/damage.wav"), 1, ATTN_NORM, 0);
}
*/

//======================================================================

/*
void Use_Breather (g_entity_c *ent, g_item_c *item)
{
	ent->getClient()->pers.inventory[G_GetNumForItem(item)]--;
	ValidateSelectedItem (ent);

	if (ent->getClient()->breather_framenum > level.framenum)
		ent->getClient()->breather_framenum += 300;
	else
		ent->getClient()->breather_framenum = level.framenum + 300;

//	trap_SV_StartSound(ent, CHAN_ITEM, trap_SV_SoundIndex("items/damage.wav"), 1, ATTN_NORM, 0);
}
*/

//======================================================================

/*
void Use_Envirosuit (g_entity_c *ent, g_item_c *item)
{
	ent->getClient()->pers.inventory[G_GetNumForItem(item)]--;
	ValidateSelectedItem (ent);

	if (ent->getClient()->enviro_framenum > level.framenum)
		ent->getClient()->enviro_framenum += 300;
	else
		ent->getClient()->enviro_framenum = level.framenum + 300;

//	trap_SV_StartSound(ent, CHAN_ITEM, trap_SV_SoundIndex("items/damage.wav"), 1, ATTN_NORM, 0);
}
*/

//======================================================================

/*
void	Use_Invulnerability (g_entity_c *ent, g_item_c *item)
{
	ent->getClient()->pers.inventory[G_GetNumForItem(item)]--;
	ValidateSelectedItem (ent);

	if (ent->getClient()->invincible_framenum > level.framenum)
		ent->getClient()->invincible_framenum += 300;
	else
		ent->getClient()->invincible_framenum = level.framenum + 300;

	trap_SV_StartSound(NULL, ent, CHAN_ITEM, trap_SV_SoundIndex("items/protect.wav"), 1, ATTN_NORM, 0);
}
*/

//======================================================================


//======================================================================

/*
bool Pickup_Key (g_entity_c *ent, g_entity_c *other)
{
	if (coop->getInteger())
	{
		if (ent->_classname ==  "key_power_cube")
		{
			if (other->getClient()->pers.power_cubes & ((ent->_spawnflags & 0x0000ff00)>> 8))
				return false;
			other->getClient()->pers.inventory[G_GetNumForItem(ent->_item)]++;
			other->getClient()->pers.power_cubes |= ((ent->_spawnflags & 0x0000ff00) >> 8);
		}
		else
		{
			if (other->getClient()->pers.inventory[G_GetNumForItem(ent->_item)])
				return false;
			other->getClient()->pers.inventory[G_GetNumForItem(ent->_item)] = 1;
		}
		return true;
	}
	other->getClient()->pers.inventory[G_GetNumForItem(ent->_item)]++;
	return true;
}
*/

//======================================================================


bool	g_item_ammo_c::pickup(g_entity_c *ent, g_player_c *other)
{
	int			oldcount;
	int			count;
	bool		weapon;

	weapon = (ent->_item->getFlags() & IT_WEAPON);
	if((weapon) && (dmflags->getInteger() & DF_INFINITE_AMMO))
		count = 1000;
		
	else if (ent->_count)
		count = ent->_count;
		
	else
		count = ent->_item->getQuantity();

	oldcount = other->_pers.inventory[G_GetNumForItem(ent->_item)];

	if(!other->addAmmo(ent->_item, count))
		return false;

	if(weapon && !oldcount)
	{
		if((g_item_c*)other->_pers.weapon != ent->_item && (!deathmatch->getInteger() || (g_item_c*)other->_pers.weapon == G_FindItem("blaster")))
			other->_newweapon = (g_item_weapon_c*)ent->_item;
	}

	if(!(ent->_spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)) && (deathmatch->getInteger()))
		SetRespawn(ent, 30);
	return true;
}

void	g_item_ammo_c::drop(g_player_c *player)
{
	g_entity_c	*dropped;
	int		index;

	index = G_GetNumForItem(this);
	
	dropped = player->dropItem(this);
	
	if(player->_pers.inventory[index] >= getQuantity())
		dropped->_count = getQuantity();
	else
		dropped->_count = player->_pers.inventory[index];

	if(	player->_pers.weapon 
		&& player->_pers.weapon->getTag() == AMMO_GRENADES 
		&& getTag() == AMMO_GRENADES 
		&& player->_pers.inventory[index] - dropped->_count <= 0	)
	{
		trap_SV_CPrintf(player, PRINT_HIGH, "Can't drop current weapon\n");
		
		dropped->remove();
		return;
	}

	player->_pers.inventory[index] -= dropped->_count;
	
	player->validateSelectedItem();
}


//======================================================================

/*
void MegaHealth_think (g_entity_c *self)
{
	if (self->r.owner->health > self->r.owner->max_health)
	{
		self->nextthink = level.time + 1;
		self->r.owner->health -= 1;
		return;
	}

	if (!(self->spawnflags & DROPPED_ITEM) && (deathmatch->getInteger()))
		SetRespawn (self, 20);
	else
		G_FreeEdict (self);
}
*/

/*
bool Pickup_Health (g_entity_c *ent, g_entity_c *other)
{
	if (!(ent->style & HEALTH_IGNORE_MAX))
		if (other->health >= other->max_health)
			return false;

	other->health += ent->count;

	if (!(ent->style & HEALTH_IGNORE_MAX))
	{
		if (other->health > other->max_health)
			other->health = other->max_health;
	}

	if (ent->style & HEALTH_TIMED)
	{
		ent->think = MegaHealth_think;
		ent->nextthink = level.time + 5;
		ent->r.owner = other;
		ent->flags |= FL_RESPAWN;
		ent->r.svflags |= SVF_NOCLIENT;
		ent->r.solid = SOLID_NOT;
	}
	else
	{
		if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->getInteger()))
			SetRespawn (ent, 30);
	}

	return true;
}
*/

//======================================================================

/*
int	ArmorIndex(g_entity_c *ent)
{
	if (!ent->getClient())
		return 0;

	if (ent->getClient()->pers.inventory[jacket_armor_index] > 0)
		return jacket_armor_index;

	if (ent->getClient()->pers.inventory[combat_armor_index] > 0)
		return combat_armor_index;

	if (ent->getClient()->pers.inventory[body_armor_index] > 0)
		return body_armor_index;

	return 0;
}
*/


/*
================================================================================
			NORMAL ARMOR
================================================================================
*/
/*
g_item_armor_c::g_item_armor_c()
{
	_has_pickup		= true;
	_has_use		= false;
	_has_drop		= false;
	_has_weaponthink	= false;	
}

bool	g_item_armor_c::pickup(g_entity_c *ent, g_entity_c *other)
{
	int				old_armor_index;
	gitem_armor_t	*oldinfo;
	gitem_armor_t	*newinfo;
	int				newcount;
	float			salvage;
	int				salvagecount;

	// get info on new armor
	newinfo = (gitem_armor_t *)ent->_item->getInfo();

	old_armor_index = ArmorIndex (other);

	// handle armor shards specially
	if (ent->_item->getTag() == ARMOR_SHARD)
	{
		if (!old_armor_index)
			other->getClient()->pers.inventory[jacket_armor_index] = 2;
		else
			other->getClient()->pers.inventory[old_armor_index] += 2;
	}

	// if player has no armor, just use it
	else if (!old_armor_index)
	{
		other->getClient()->pers.inventory[G_GetNumForItem(ent->_item)] = newinfo->base_count;
	}

	// use the better armor
	else
	{
		// get info on old armor
		if (old_armor_index == jacket_armor_index)
			oldinfo = &jacketarmor_info;
			
		else if (old_armor_index == combat_armor_index)
			oldinfo = &combatarmor_info;
			
		else // (old_armor_index == body_armor_index)
			oldinfo = &bodyarmor_info;

		if (newinfo->normal_protection > oldinfo->normal_protection)
		{
			// calc new armor values
			salvage = oldinfo->normal_protection / newinfo->normal_protection;
			salvagecount = (int)(salvage * other->getClient()->pers.inventory[old_armor_index]);
			newcount = newinfo->base_count + salvagecount;
			if (newcount > newinfo->max_count)
				newcount = newinfo->max_count;

			// zero count of old armor so it goes away
			other->getClient()->pers.inventory[old_armor_index] = 0;

			// change armor to new item with computed value
			other->getClient()->pers.inventory[G_GetNumForItem(ent->_item)] = newcount;
		}
		else
		{
			// calc new armor values
			salvage = newinfo->normal_protection / oldinfo->normal_protection;
			salvagecount = (int)(salvage * newinfo->base_count);
			newcount = other->getClient()->pers.inventory[old_armor_index] + salvagecount;
			if (newcount > oldinfo->max_count)
				newcount = oldinfo->max_count;

			// if we're already maxed out then we don't need the new armor
			if (other->getClient()->pers.inventory[old_armor_index] >= newcount)
				return false;

			// update current armor value
			other->getClient()->pers.inventory[old_armor_index] = newcount;
		}
	}

	if (!(ent->_spawnflags & DROPPED_ITEM) && (deathmatch->getInteger()))
		SetRespawn (ent, 20);

	return true;
}
*/



/*
================================================================================
			POWER ARMOR
================================================================================
*/
/*
int	PowerArmorType (g_entity_c *ent)
{
	if (!ent->getClient())
		return POWER_ARMOR_NONE;

	if (!(ent->_flags & FL_POWER_ARMOR))
		return POWER_ARMOR_NONE;

	if (ent->getClient()->pers.inventory[power_shield_index] > 0)
		return POWER_ARMOR_SHIELD;

	if (ent->getClient()->pers.inventory[power_screen_index] > 0)
		return POWER_ARMOR_SCREEN;

	return POWER_ARMOR_NONE;
}
*/


/*
g_item_power_armor_c::g_item_power_armor_c()
{
	_has_pickup		= true;
	_has_use		= true;
	_has_drop		= true;
	_has_weaponthink	= false;	
}


bool	g_item_power_armor_c::pickup(g_entity_c *ent, g_entity_c *other)
{
	int		quantity;

	quantity = other->getClient()->pers.inventory[G_GetNumForItem(ent->_item)];

	other->getClient()->pers.inventory[G_GetNumForItem(ent->_item)]++;

	if (deathmatch->getInteger())
	{
		if (!(ent->_spawnflags & DROPPED_ITEM) )
			SetRespawn (ent, ent->_item->getQuantity());
			
		// auto-use for DM only if we didn't already have one
		if (!quantity)
			ent->_item->use (other, ent->_item);
	}

	return true;
}

void	g_item_power_armor_c::use(g_entity_c *ent, g_item_c *item)
{
	int		index;

	if (ent->_flags & FL_POWER_ARMOR)
	{
		ent->_flags &= ~FL_POWER_ARMOR;
		trap_SV_StartSound(NULL, ent, CHAN_AUTO, trap_SV_SoundIndex("misc/power2.wav"), 1, ATTN_NORM, 0);
	}
	else
	{
		index = G_GetNumForItem(G_FindItem("cells"));
		if (!ent->getClient()->pers.inventory[index])
		{
			trap_SV_CPrintf (ent, PRINT_HIGH, "No cells for power armor.\n");
			return;
		}
		ent->_flags |= FL_POWER_ARMOR;
		trap_SV_StartSound(NULL, ent, CHAN_AUTO, trap_SV_SoundIndex("misc/power1.wav"), 1, ATTN_NORM, 0);
	}
}



void	g_item_power_armor_c::drop(g_entity_c *ent, g_item_c *item)
{
	if ((ent->_flags & FL_POWER_ARMOR) && (ent->getClient()->pers.inventory[G_GetNumForItem(item)] == 1))
		use(ent, item);
		
	Drop_General (ent, item);
}
*/




/*
================================================================================
				DROPABLE ITEM
================================================================================
*/

g_item_dropable_c::g_item_dropable_c(g_player_c *player, g_item_c *item, const vec3_c &position, const quaternion_c &orientation, const vec3_c &velocity)
{
	//trap_Com_DPrintf("%s is dropping item %s\n", player->_pers.netname, item->getWorldModel());

	_s.origin = position;
	_s.origin2.set(0, 0, 8);
	_s.index_model = trap_SV_ModelIndex(item->getWorldModel());
	_s.effects = item->getWorldModelFlags();
	_s.renderfx = RF_GLOW;

	_r.inuse = true;
	_r.owner = this;
//	_r.bbox._mins.set(-15, -15, -15);
//	_r.bbox._maxs.set( 15,  15,  15);
//	_r.size = _r.bbox.size();
	_r.solid = SOLID_TRIGGER;
	
//	_nextthink = level.time + 30;
	_nextthink = level.time + FRAMETIME;
	
	_classname = item->getClassname();
	_item = item;
	_spawnflags = DROPPED_ITEM;
	_movetype = MOVETYPE_ODE_TOSS;
	//_movetype = MOVETYPE_TOSS;
	
	// setup rigid body
	_body->setPosition(position);
	_body->setQuaternion(orientation);
	_body->setLinearVel(velocity);
//	_body->setGravityMode(1);
	
	/*cmodel_c *model = */G_SetModel(this, _item->getWorldModel());
	
	// setup mass
	dMass m;
	m.setBoxTotal(3, _r.size[0], _r.size[1], _r.size[2]);
//	m.setSphereTotal(3, _r.size.length() * 0.5);
	_body->setMass(&m);
	
	// setup collision
	d_geom_c *geom = new d_box_c(g_ode_space_toplevel->getId(), _r.size);
//	d_geom_c *geom = new d_trimesh_c(g_ode_space_toplevel->getId(), model->vertexes, model->indexes);
//	d_geom_c *geom = new d_sphere_c(g_ode_space_toplevel->getId(), _r.size.length() * 0.5);
	
	geom->setBody(_body->getId());
	geom->setData(this);
	geom->setCollideBits(MASK_SOLID);
	
	_geoms.push_back(geom);
}

g_item_dropable_c::~g_item_dropable_c()
{
	//delete _geom;
}


void	g_item_dropable_c::think()
{
	if(deathmatch->getInteger() && (level.time >= (getSpawnTime() + 30)))
	{
		remove();
	}
	else
	{
//		int leafnum = trap_CM_PointLeafnum(_s.origin);
//		trap_Com_Printf("g_item_dropable_c::think: at %s\n", _s.origin.toString());
		_nextthink = level.time + FRAMETIME;
	}
}

bool	g_item_dropable_c::touch(g_entity_c *other, const cplane_c &plane, csurface_c *surf)
{
#if 0
	bool	taken;
	
	if (other == _r.owner)
		return;

	if (!other->_r.isclient)
		return;
		
	g_player_c* player = (g_player_c*)other;
		
	if (other->_health < 1)
		return;		// dead people can't pickup
		
	if (!_item->hasPickup())
		return;		// not a grabbable item?

	taken = _item->pickup(this, other);

	if(taken)
	{
		// flash the screen
		other->getClient()->bonus_alpha = 0.25;	

		// show icon and name on status bar
		other->getClient()->ps.stats[STAT_PICKUP_ICON] = trap_SV_ImageIndex (_item->getIcon());
		other->getClient()->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS + G_GetNumForItem(_item);
		other->getClient()->pickup_msg_time = level.time + 3.0;

		// change selected item
		if(_item->hasUse())
			other->getClient()->pers.selected_item = other->getClient()->ps.stats[STAT_SELECTED_ITEM] = G_GetNumForItem(_item);

		if(!X_stricmp(_item->getPickupName(), "Health"))
		{
			if (_count == 2)
				trap_SV_StartSound(NULL, other, CHAN_ITEM, trap_SV_SoundIndex("items/s_health.wav"), 1, ATTN_NORM, 0);
				
			else if (_count == 10)
				trap_SV_StartSound(NULL, other, CHAN_ITEM, trap_SV_SoundIndex("items/n_health.wav"), 1, ATTN_NORM, 0);
				
			else if (_count == 25)
				trap_SV_StartSound(NULL, other, CHAN_ITEM, trap_SV_SoundIndex("items/l_health.wav"), 1, ATTN_NORM, 0);
				
			else // (ent->count == 100)
				trap_SV_StartSound(NULL, other, CHAN_ITEM, trap_SV_SoundIndex("items/m_health.wav"), 1, ATTN_NORM, 0);
		}
		else if(_item->getPickupSound())
		{
			trap_SV_StartSound(NULL, other, CHAN_ITEM, trap_SV_SoundIndex(_item->getPickupSound()), 1, ATTN_NORM, 0);
		}
	}

	if(!(_spawnflags & ITEM_TARGETS_USED))
	{
		G_UseTargets (this, other);
		_spawnflags |= ITEM_TARGETS_USED;
	}

	if(!taken)
		return;

	if(!((coop->getInteger()) &&  (_item->getFlags() & IT_STAY_COOP)) || (_spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)))
	{
		if (_flags & FL_RESPAWN)
			_flags &= ~FL_RESPAWN;
		else
			_remove = true;
	}
#endif

	return true;
}






/*
================================================================================
				SPAWNABLE ITEM
================================================================================
*/
g_item_spawnable_c::g_item_spawnable_c(g_item_c *item)
{
	_r.inuse = true;
	
	_classname = item->getClassname();
	_item = item;
	_s.effects = item->getWorldModelFlags();
	_s.renderfx = RF_GLOW;
	
	G_SetModel(this, item->getWorldModel());
	
	_r.bbox._mins.set(-15, -15, -15);
	_r.bbox._maxs.set( 15,  15,  15);
	_r.size = _r.bbox.size();
	
	_r.solid = SOLID_TRIGGER;
	_movetype = MOVETYPE_TOSS;
	_r.owner = g_world;
	_nextthink = level.time + 30;
}


void	g_item_spawnable_c::think()
{
	//if(deathmatch->getInteger())
	//	_remove = true;
}

bool	g_item_spawnable_c::touch(g_entity_c *other, const cplane_c &plane, csurface_c *surf)
{
	//TODO
	return true;
}

void	g_item_spawnable_c::activate()
{
	// setup rigid body
	_body->setPosition(_s.origin);
	_body->setQuaternion(_s.quat);
	_body->setGravityMode(0);
	_body->disable();
	
	/*cmodel_c* model = */G_SetModel(this, _item->getWorldModel());
	
	// setup mass
	//dMass m;
	//dMassSetBoxTotal(&m, 5, _r.size[0], _r.size[1], _r.size[2]);
	//_body->setMass(&m);
	
	// setup collision
	d_geom_c *geom = new d_box_c(g_ode_space_toplevel->getId(), _r.size);
	//d_geom_c* geom = new d_trimesh_c(g_ode_space_toplevel->getId(), model->vertexes, model->indexes);
	//_geom = new d_sphere_c(g_ode_space->getId(), _r.bbox._maxs[0]);
	
	geom->setBody(_body->getId());
	geom->setData(this);
	
	_geoms.push_back(geom);
}



/*
static void	Use_Item (g_entity_c *ent, g_entity_c *other, g_entity_c *activator)
{
	ent->r.svflags &= ~SVF_NOCLIENT;
	
	ent->use = NULL;

	if (ent->spawnflags & ITEM_NO_TOUCH)
	{
		ent->r.solid = SOLID_BBOX;
		ent->touch = NULL;
	}
	else
	{
		ent->r.solid = SOLID_TRIGGER;
		ent->touch = Touch_Item;
	}


	trap_SV_LinkEdict (ent);
}
*/

/*
void	droptofloor(g_entity_c *ent)
{
	trace_t		tr;
	vec3_c		dest;

	ent->_r.bbox._mins.set(-15,-15,-15);
	ent->_r.bbox._maxs.set( 15, 15, 15);

	if (ent->_model.length())
		G_SetModel(ent, ent->_model);
	else
		G_SetModel(ent, ent->_item->getWorldModel());
		
	ent->_r.solid = SOLID_TRIGGER;
	ent->_movetype = MOVETYPE_TOSS;  
	//ent->touch = Touch_Item;

	vec3_c v(0, 0, -128);
	dest = ent->_s.origin + v;

	tr = trap_SV_Trace(ent->_s.origin, ent->_r.bbox, dest, ent, MASK_SOLID);
	if(tr.startsolid)
	{
		trap_Com_DPrintf("droptofloor: '%s' startsolid at '%s'\n", ent->getClassName(), Vector3_String(ent->_s.origin));
		ent->remove();
		return;
	}

	Vector3_Copy (tr.endpos, ent->_s.origin);

#if 0
	if (ent->team)
	{
		ent->flags &= ~FL_TEAMSLAVE;
		ent->chain = ent->teamchain;
		ent->teamchain = NULL;

		ent->r.svflags |= SVF_NOCLIENT;
		ent->r.solid = SOLID_NOT;
		if (ent == ent->teammaster)
		{
			ent->nextthink = level.time + FRAMETIME;
			ent->think = DoRespawn;
		}
	}

	if (ent->spawnflags & ITEM_NO_TOUCH)
	{
		ent->r.solid = SOLID_BBOX;
		ent->touch = NULL;
		ent->s.effects &= ~EF_ROTATE;
		ent->s.renderfx &= ~RF_GLOW;
	}

	if (ent->spawnflags & ITEM_TRIGGER_SPAWN)
	{
		ent->r.svflags |= SVF_NOCLIENT;
		ent->r.solid = SOLID_NOT;
		ent->use = Use_Item;
	}
#endif
}
*/


/*
===============
PrecacheItem

Precaches all data needed for a given item.
This will be called for each item spawned in a level,
and for each item in each client's inventory.
===============
*/
void	G_PrecacheItem(g_item_c *item)
{
	if(!item)
		return;

	if(item->getPickupSound())
		trap_SV_SoundIndex(item->getPickupSound());
		
	if(item->getWorldModel())
		trap_SV_ModelIndex(item->getWorldModel());
		
	if(item->getViewModel())
		trap_SV_ModelIndex(item->getViewModel());
		
	if(item->getIcon())
		trap_SV_ShaderIndex(item->getIcon());
	
	// parse everything for its ammo
	if(item->getAmmo() && item->getAmmo()[0])
	{
		g_item_c *ammo = G_FindItem(item->getAmmo());
		
		if(ammo != item)
			G_PrecacheItem(ammo);
	}

	// parse the seperated precache string for other items
	const std::vector<std::string> &precaches = item->getPrecaches();
	
	if(!precaches.size())
		return;
		
	for(std::vector<std::string>::const_iterator ir = precaches.begin(); ir != precaches.end(); ++ir)
	{
		const std::string &s = *ir;
	
		if(s.length() < 5 || s.length() > MAX_QPATH)
		{
			trap_Com_Error(ERR_DROP, "G_PrecacheItem: '%s' has bad precache string length %i for entry '%s'", item->getClassname(), s.length(), s.c_str());
			break;
		}
		
		const char *ext = s.substr(s.length()-4, s.length()).c_str();
		
		// determine type based on extension
		if(X_strcaseequal(ext, ".ase"))
			trap_SV_ModelIndex(s);
			
		else if(X_strcaseequal(ext, ".lwo"))
			trap_SV_ModelIndex(s);
		else if(X_strcaseequal(ext, ".md2"))
			trap_SV_ModelIndex(s);
			
		else if(X_strcaseequal(ext, ".md3"))
			trap_SV_ModelIndex(s);
		
		else if(X_strcaseequal(ext, ".md5mesh"))
			trap_SV_ModelIndex(s);
		
		else if(X_strcaseequal(ext, ".md5anim"))
			trap_SV_AnimationIndex(s);
			
		else if(X_strcaseequal(ext, ".tga"))
			trap_SV_ShaderIndex(s);
			
		else if(X_strcaseequal(ext, ".jpg"))
			trap_SV_ShaderIndex(s);
		
		else if(X_strcaseequal(ext, ".png"))
			trap_SV_ShaderIndex(s);
			
		else if(X_strcaseequal(ext, ".wav"))
			trap_SV_SoundIndex(s);
			
		else if(X_strcaseequal(ext, ".ogg"))
			trap_SV_SoundIndex(s);
			
		else if(X_strcaseequal(ext, ".mp3"))
			trap_SV_SoundIndex(s);
	}
}

/*
============
SpawnItem

Sets the clipping size and plants the object on the floor.

Items can't be immediately dropped to floor, because they might
be on an entity that hasn't spawned yet.
============
*/
void	G_SpawnItem(g_entity_c **entity, g_item_c *item)
{
	G_PrecacheItem(item);

	/*
	if (ent->_spawnflags)
	{
		if (strcmp(ent->_classname, "key_power_cube") != 0)
		{
			ent->_spawnflags = 0;
			trap_Com_Printf("%s at %s has invalid spawnflags set\n", ent->_classname, Vector3_String(ent->_s.origin));
		}
	}
	*/

	// some items will be prevented in deathmatch
	/*
	if(deathmatch->getInteger())
	{
		if(dmflags->getInteger() & DF_NO_ARMOR )
		{
			//if (item->pickup == Pickup_Armor || item->pickup == Pickup_PowerArmor)
			if(item->flags & IT_ARMOR)
			{
				G_FreeEdict (ent);
				return;
			}
		}
		
		if(dmflags->getInteger() & DF_NO_ITEMS)
		{
			if(item->pickup == Pickup_Powerup)
			{
				G_FreeEdict(ent);
				return;
			}
		}
		
		if(dmflags->getInteger() & DF_NO_HEALTH)
		{
			if(item->pickup == Pickup_Health || item->pickup == Pickup_Adrenaline || item->pickup == Pickup_AncientHead)
			{
				G_FreeEdict(ent);
				return;
			}
		}
		
		if(dmflags->getInteger() & DF_INFINITE_AMMO)
		{
			if((item->flags == IT_AMMO) || (strcmp(ent->classname, "weapon_bfg") == 0))
			{
				G_FreeEdict(ent);
				return;
			}
		}
	}
	*/

	
	// don't let them drop items that stay in a coop game
	/*
	if ((coop->value) && (item->flags & IT_STAY_COOP))
	{
		item->drop = NULL;
	}
	*/
	
#if 0
	g_entity_c *ent =  new g_item_spawnable_c;	//(g_entity_c*)g_edicts[0];
	*entity = ent;
	
	ent->_item = item;
	//ent->nextthink = level.time + 2 * FRAMETIME;    // items start after other solids
	//ent->think = droptofloor;
	ent->_s.effects = item->getWorldModelFlags();
	ent->_s.renderfx = RF_GLOW;
	
	if(ent->_model.length())
		trap_SV_ModelIndex(ent->model);
		
#else
	g_entity_c *ent =  new g_item_spawnable_c(item);
	*entity = ent;
#endif
}




/*
g_item_armor_body_c::g_item_armor_body_c()
{
	_classname 	= "item_armor_body";
	
	_pickup_sound 		= "misc/ar1_pkup.wav";
	_world_model 		= "models/items/armor/body/tris.md2";
	_world_model_flags	= EF_ROTATE;
	_model_view 		= "";
	
	_icon		= "textures/pics/i_bodyarmor.pcx";
	_pickup_name	= "Body Armor";
	_count_width	= 3;

	_quantity	= 0;			
	_ammo		= "";			
	_flags		= IT_ARMOR;	

	_weapmodel	= 0;

	_info		= &bodyarmor_info;
	_tag		= ARMOR_BODY;

	_precaches	= "";
}


g_item_armor_combat_c::g_item_armor_combat_c()
{
	_classname 	= "item_armor_combat";
	
	_pickup_sound 		= "misc/ar1_pkup.wav";
	_world_model 		= "models/items/armor/combat/tris.md2";
	_world_model_flags	= EF_ROTATE;
	_model_view 		= "";
	
	_icon		= "textures/pics/i_combatarmor.pcx";
	_pickup_name	= "Combat Armor";
	_count_width	= 3;

	_quantity	= 0;			
	_ammo		= "";			
	_flags		= IT_ARMOR;	

	_weapmodel	= 0;

	_info		= &combatarmor_info;
	_tag		= ARMOR_COMBAT;

	_precaches	= "";
}

	
g_item_armor_jacket_c::g_item_armor_jacket_c()
{
	_classname 	= "item_armor_jacket";
	
	_pickup_sound 		= "misc/ar1_pkup.wav";
	_world_model 		= "models/items/armor/jacket/tris.md2";
	_world_model_flags	= EF_ROTATE;
	_model_view 		= "";
	
	_icon		= "textures/pics/i_jacketarmor.pcx";
	_pickup_name	= "Jacket Armor";
	_count_width	= 3;

	_quantity	= 0;			
	_ammo		= "";			
	_flags		= IT_ARMOR;	

	_weapmodel	= 0;

	_info		= &jacketarmor_info;
	_tag		= ARMOR_JACKET;

	_precaches	= "";
}


g_item_armor_shard_c::g_item_armor_shard_c()
{
	_classname 	= "item_armor_jacket";
	
	_pickup_sound 		= "misc/ar2_pkup.wav";
	_world_model 		= "models/items/armor/shard/tris.md2";
	_world_model_flags	= EF_ROTATE;
	_model_view 		= "";
	
	_icon		= "textures/pics/i_jacketarmor.pcx";
	_pickup_name	= "Shard Armor";
	_count_width	= 3;

	_quantity	= 0;			
	_ammo		= "";			
	_flags		= IT_ARMOR;	

	_weapmodel	= 0;

	_info		= NULL;
	_tag		= ARMOR_SHARD;

	_precaches	= "";
}



g_item_power_screen_c::g_item_power_screen_c()
{
	_classname 	= "item_power_screen";
	
	_pickup_sound 		= "misc/ar3_pkup.wav";
	_world_model 		= "models/items/armor/screen/tris.md2";
	_world_model_flags	= EF_ROTATE;
	_model_view 		= "";
	
	_icon		= "textures/pics/i_powerscreen.pcx";
	_pickup_name	= "Power Screen";
	_count_width	= 0;

	_quantity	= 60;			
	_ammo		= "";			
	_flags		= IT_ARMOR;	

	_weapmodel	= 0;

	_info		= NULL;
	_tag		= 0;

	_precaches	= "";
}



g_item_power_shield_c::g_item_power_shield_c()
{
	_classname 	= "item_power_shield";
	
	_pickup_sound 		= "misc/ar3_pkup.wav";
	_world_model 		= "models/items/armor/shield/tris.md2";
	_world_model_flags	= EF_ROTATE;
	_model_view 		= "";
	
	_icon		= "textures/pics/i_powershield.pcx";
	_pickup_name	= "Power Shield";
	_count_width	= 0;

	_quantity	= 60;			
	_ammo		= "";			
	_flags		= IT_ARMOR;	

	_weapmodel	= 0;

	_info		= NULL;
	_tag		= 0;

	_precaches	= "misc/power2.wav misc/power1.wav";
}
*/




g_item_ammo_shells_c::g_item_ammo_shells_c()
{
	_classname 	= "ammo_shells";
	
	_pickup_sound 		= "misc/am_pkup.wav";
	_model_world		= "models/powerups/ammo/shotgunam.md3";
	_model_world_flags	= 0;
	_model_view 		= "";
	
	_icon		= "icons/icona_shotgun.tga";
	_pickup_name	= "Shells";
	_count_width	= 3;

	_quantity	= 10;
	_ammo		= "";
	_flags		= IT_AMMO;	

	_weapmodel	= 0;

	_info		= NULL;
	_tag		= AMMO_SHELLS;
}


g_item_ammo_bullets_c::g_item_ammo_bullets_c()
{
	_classname 	= "ammo_bullets";
	
	_pickup_sound 		= "misc/am_pkup.wav";
	_model_world 		= "models/powerups/ammo/machinegunam.md3";
	_model_world_flags	= 0;
	_model_view 		= "";
	
	_icon		= "icons/icona_machinegun.tga";
	_pickup_name	= "Bullets";
	_count_width	= 3;

	_quantity	= 50;			
	_ammo		= "";			
	_flags		= IT_AMMO;	

	_weapmodel	= 0;

	_info		= NULL;
	_tag		= AMMO_BULLETS;
}


g_item_ammo_cells_c::g_item_ammo_cells_c()
{
	_classname 	= "ammo_cells";
	
	_pickup_sound 		= "misc/am_pkup.wav";
	_model_world 		= "models/powerups/ammo/plasmaam.md3";
	_model_world_flags	= 0;
	_model_view 		= "";
	
	_icon		= "icons/icona_plasma";
	_pickup_name	= "Cells";
	_count_width	= 3;

	_quantity	= 50;			
	_ammo		= "";			
	_flags		= IT_AMMO;	

	_weapmodel	= 0;

	_info		= NULL;
	_tag		= AMMO_CELLS;
}

g_item_ammo_rockets_c::g_item_ammo_rockets_c()
{
	_classname 	= "ammo_rockets";
	
	_pickup_sound 		= "misc/am_pkup.wav";
	_model_world 		= "models/powerups/ammo/rocketam.md3";
	_model_world_flags	= 0;//EF_ROTATE;
	_model_view 		= "";
	
	_icon		= "icons/icona_rocket.tga";
	_pickup_name	= "Rockets";
	_count_width	= 3;

	_quantity	= 5;
	_ammo		= "";
	_flags		= IT_AMMO;

	_weapmodel	= 0;

	_info		= NULL;
	_tag		= AMMO_ROCKETS;
}

/*
g_item_ammo_grenades_c::g_item_ammo_grenades_c()
{
	_classname 	= "ammo_grenades";
	
	_pickup_sound 		= "misc/am_pkup.wav";
	_model_world 		= "models/items/ammo/grenades/medium/tris.md2";
	_model_world_flags	= 0;
	_model_view 		= "";
	
	_icon		= "textures/pics/a_grenades.pcx";
	_pickup_name	= "Grenades";
	_count_width	= 3;

	_quantity	= 5;
	_ammo		= "";	
	_flags		= IT_AMMO;

	_weapmodel	= 0;

	_info		= NULL;
	_tag		= AMMO_GRENADES;

	_precaches	= "";
}
*/

g_item_ammo_slugs_c::g_item_ammo_slugs_c()
{
	_classname 	= "ammo_slugs";
	
	_pickup_sound 		= "misc/am_pkup.wav";
	_model_world 		= "models/powerups/ammo/railgunam.md3";
	_model_world_flags	= 0;
	_model_view 		= "";
	
	_icon		= "icons/icona_railgun.tga";
	_pickup_name	= "Slugs";
	_count_width	= 3;

	_quantity	= 10;
	_ammo		= "";	
	_flags		= IT_AMMO;

	_weapmodel	= 0;

	_info		= NULL;
	_tag		= AMMO_SLUGS;
}




#if 0

g_item_c	itemlist[] = 
{
	{
		NULL
	},	// leave index 0 alone

	

	//
	// WEAPONS 
	//


/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_grenades",
		Pickup_Ammo,
		Use_Weapon,
		Drop_Ammo,
		Weapon_Grenade,
		"misc/am_pkup.wav",
		"models/items/ammo/grenades/medium/tris.md2", 0,
		"models/weapons/v_handgr/tris.md2",
/* icon */	"textures/pics/a_grenades.pcx",
/* pickup */	"Grenades",
/* width */		3,
		5,
		"grenades",
		IT_AMMO|IT_WEAPON,
		WEAP_GRENADES,
		NULL,
		AMMO_GRENADES,
/* precache */ "weapons/hgrent1a.wav weapons/hgrena1b.wav weapons/hgrenc1b.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav "
	},







	//
	// AMMO ITEMS
	//

/*QUAKED ammo_shells (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_shells",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/shells/medium/tris.md2", 0,
		NULL,
/* icon */	"textures/pics/a_shells.pcx",
/* pickup */	"Shells",
/* width */		3,
		10,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_SHELLS,
/* precache */ ""
	},

/*QUAKED ammo_bullets (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_bullets",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/bullets/medium/tris.md2", 0,
		NULL,
/* icon */	"textures/pics/a_bullets.pcx",
/* pickup */	"Bullets",
/* width */		3,
		50,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_BULLETS,
/* precache */ ""
	},

/*QUAKED ammo_cells (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_cells",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/cells/medium/tris.md2", 0,
		NULL,
/* icon */	"textures/pics/a_cells.pcx",
/* pickup */	"Cells",
/* width */		3,
		50,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_CELLS,
/* precache */ ""
	},

/*QUAKED ammo_rockets (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_rockets",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/rockets/medium/tris.md2", 0,
		NULL,
/* icon */	"textures/pics/a_rockets.pcx",
/* pickup */	"Rockets",
/* width */		3,
		5,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_ROCKETS,
/* precache */ ""
	},

/*QUAKED ammo_slugs (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_slugs",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/slugs/medium/tris.md2", 0,
		NULL,
/* icon */	"textures/pics/a_slugs.pcx",
/* pickup */	"Slugs",
/* width */		3,
		10,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_SLUGS,
/* precache */ ""
	},


	//
	// POWERUP ITEMS
	//
/*QUAKED item_quad (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_quad", 
		Pickup_Powerup,
		Use_Quad,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/quaddama/tris.md2", EF_ROTATE,
		NULL,
/* icon */	"textures/pics/p_quad.pcx",
/* pickup */	"Quad Damage",
/* width */	2,
		60,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		0,
/* precache */ "items/damage.wav items/damage2.wav items/damage3.wav"
	},

/*QUAKED item_invulnerability (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_invulnerability",
		Pickup_Powerup,
		Use_Invulnerability,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/invulner/tris.md2", EF_ROTATE,
		NULL,
/* icon */	"textures/pics/p_invulnerability.pcx",
/* pickup */	"Invulnerability",
/* width */		2,
		300,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		0,
/* precache */ "items/protect.wav items/protect2.wav items/protect4.wav"
	},

/*QUAKED item_silencer (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_silencer",
		Pickup_Powerup,
		Use_Silencer,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/silencer/tris.md2", EF_ROTATE,
		NULL,
/* icon */	"textures/pics/p_silencer.pcx",
/* pickup */	"Silencer",
/* width */		2,
		60,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED item_breather (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_breather",
		Pickup_Powerup,
		Use_Breather,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/breather/tris.md2", EF_ROTATE,
		NULL,
/* icon */	"textures/pics/p_rebreather.pcx",
/* pickup */	"Rebreather",
/* width */		2,
		60,
		NULL,
		IT_STAY_COOP|IT_POWERUP,
		0,
		NULL,
		0,
/* precache */ "items/airout.wav"
	},

/*QUAKED item_enviro (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_enviro",
		Pickup_Powerup,
		Use_Envirosuit,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/enviro/tris.md2", EF_ROTATE,
		NULL,
/* icon */	"textures/pics/p_envirosuit.pcx",
/* pickup */	"Environment Suit",
/* width */		2,
		60,
		NULL,
		IT_STAY_COOP|IT_POWERUP,
		0,
		NULL,
		0,
/* precache */ "items/airout.wav"
	},

/*QUAKED item_ancient_head (.3 .3 1) (-16 -16 -16) (16 16 16)
Special item that gives +2 to maximum health
*/
	{
		"item_ancient_head",
		Pickup_AncientHead,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/c_head/tris.md2", EF_ROTATE,
		NULL,
/* icon */	"textures/pics/i_fixme.pcx",
/* pickup */	"Ancient Head",
/* width */		2,
		60,
		NULL,
		0,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED item_adrenaline (.3 .3 1) (-16 -16 -16) (16 16 16)
gives +1 to maximum health
*/
	{
		"item_adrenaline",
		Pickup_Adrenaline,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/adrenal/tris.md2", EF_ROTATE,
		NULL,
/* icon */	"textures/pics/p_adrenaline.pcx",
/* pickup */	"Adrenaline",
/* width */		2,
		60,
		NULL,
		0,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED item_bandolier (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_bandolier",
		Pickup_Bandolier,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/band/tris.md2", EF_ROTATE,
		NULL,
/* icon */	"textures/pics/p_bandolier.pcx",
/* pickup */	"Bandolier",
/* width */		2,
		60,
		NULL,
		0,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED item_pack (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_pack",
		Pickup_Pack,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/pack/tris.md2", EF_ROTATE,
		NULL,
/* icon */	"textures/pics/i_pack.pcx",
/* pickup */	"Ammo Pack",
/* width */		2,
		180,
		NULL,
		0,
		0,
		NULL,
		0,
/* precache */ ""
	},

	//
	// KEYS
	//
/*QUAKED key_data_cd (0 .5 .8) (-16 -16 -16) (16 16 16)
key for computer centers
*/
	{
		"key_data_cd",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/data_cd/tris.md2", EF_ROTATE,
		NULL,
		"textures/pics/k_datacd.pcx",
		"Data CD",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_power_cube (0 .5 .8) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN NO_TOUCH
warehouse circuits
*/
	{
		"key_power_cube",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/power/tris.md2", EF_ROTATE,
		NULL,
		"textures/pics/k_powercube.pcx",
		"Power Cube",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_pyramid (0 .5 .8) (-16 -16 -16) (16 16 16)
key for the entrance of jail3
*/
	{
		"key_pyramid",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pyramid/tris.md2", EF_ROTATE,
		NULL,
		"textures/pics/k_pyramid.pcx",
		"Pyramid Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_data_spinner (0 .5 .8) (-16 -16 -16) (16 16 16)
key for the city computer
*/
	{
		"key_data_spinner",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/spinner/tris.md2", EF_ROTATE,
		NULL,
		"textures/pics/k_dataspin.pcx",
		"Data Spinner",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_pass (0 .5 .8) (-16 -16 -16) (16 16 16)
security pass for the security level
*/
	{
		"key_pass",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2", EF_ROTATE,
		NULL,
		"textures/pics/k_security.pcx",
		"Security Pass",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_blue_key (0 .5 .8) (-16 -16 -16) (16 16 16)
normal door key - blue
*/
	{
		"key_blue_key",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/key/tris.md2", EF_ROTATE,
		NULL,
		"textures/pics/k_bluekey.pcx",
		"Blue Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_red_key (0 .5 .8) (-16 -16 -16) (16 16 16)
normal door key - red
*/
	{
		"key_red_key",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/red_key/tris.md2", EF_ROTATE,
		NULL,
		"textures/pics/k_redkey.pcx",
		"Red Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_commander_head (0 .5 .8) (-16 -16 -16) (16 16 16)
tank commander's head
*/
	{
		"key_commander_head",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/monsters/commandr/head/tris.md2", EF_GIB,
		NULL,
/* icon */	"textures/pics/k_comhead.pcx",
/* pickup */	"Commander's Head",
/* width */		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_airstrike_target (0 .5 .8) (-16 -16 -16) (16 16 16)
tank commander's head
*/
	{
		"key_airstrike_target",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/target/tris.md2", EF_ROTATE,
		NULL,
/* icon */	"textures/pics/i_airstrike.pcx",
/* pickup */	"Airstrike Marker",
/* width */		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

	{
		NULL,
		Pickup_Health,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		NULL, 0,
		NULL,
/* icon */	"textures/pics/i_health.pcx",
		//"gfx/hud/healthicon.tga",
/* pickup */	"Health",
/* width */		3,
		0,
		NULL,
		0,
		0,
		NULL,
		0,
/* precache */ "items/s_health.wav items/n_health.wav items/l_health.wav items/m_health.wav"
	},

	// end of list marker
	{NULL}
};

#endif


/*QUAKED item_health (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
/*
void SP_item_health (g_entity_c *self)
{
	if ( deathmatch->value && (dmflags->integer & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/items/healing/medium/tris.md2";
	self->count = 10;
	SpawnItem (self, G_FindItem ("Health"));
	trap_SV_SoundIndex ("items/n_health.wav");
}
*/

/*QUAKED item_health_small (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
/*
void SP_item_health_small (g_entity_c *self)
{
	if ( deathmatch->value && (dmflags->integer & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/items/healing/stimpack/tris.md2";
	self->count = 2;
	SpawnItem (self, G_FindItem ("Health"));
	self->style = HEALTH_IGNORE_MAX;
	trap_SV_SoundIndex ("items/s_health.wav");
}
*/

/*QUAKED item_health_large (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
/*
void SP_item_health_large (g_entity_c *self)
{
	if ( deathmatch->value && (dmflags->integer & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/items/healing/large/tris.md2";
	self->count = 25;
	SpawnItem (self, G_FindItem ("Health"));
	trap_SV_SoundIndex ("items/l_health.wav");
}
*/

/*QUAKED item_health_mega (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
/*
void SP_item_health_mega (g_entity_c *self)
{
	if ( deathmatch->value && (dmflags->integer & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/items/mega_h/tris.md2";
	self->count = 100;
	SpawnItem (self, G_FindItem ("Health"));
	trap_SV_SoundIndex ("items/m_health.wav");
	self->style = HEALTH_IGNORE_MAX|HEALTH_TIMED;
}
*/

void	G_InitItems()
{
	//
	// ARMOR
	//
	//g_items.push_back(new g_item_armor_body_c);
	//g_items.push_back(new g_item_armor_combat_c);
	//g_items.push_back(new g_item_armor_jacket_c);
	//g_items.push_back(new g_item_armor_shard_c);
	
	//g_items.push_back(new g_item_power_screen_c);
	//g_items.push_back(new g_item_power_shield_c);
	
	//
	// WEAPONS 
	//
	g_items.push_back(new g_item_weapon_blaster_c);
	g_items.push_back(new g_item_weapon_shotgun_c);
	//g_items.push_back(new g_item_weapon_super_shotgun_c);
	//g_items.push_back(new g_item_weapon_machinegun_c);
	//g_items.push_back(new g_item_weapon_chaingun_c);
	//g_items.push_back(new g_item_weapon_grenade_launcher_c);
	//g_items.push_back(new g_item_weapon_rocket_launcher_c);
	//g_items.push_back(new g_item_weapon_hyper_blaster_c);
	//g_items.push_back(new g_item_weapon_railgun_c);
	//g_items.push_back(new g_item_weapon_bfg_c);
	
	//g_items.push_back(new g_item_weapon_ionblaster_c);
	//g_items.push_back(new g_item_weapon_sidewinder_c);
	
	//g_items.push_back(new g_item_weapon_nailgun_c);
	
	//
	// AMMO
	//
	g_items.push_back(new g_item_ammo_shells_c);
	g_items.push_back(new g_item_ammo_bullets_c);
	g_items.push_back(new g_item_ammo_cells_c);
	g_items.push_back(new g_item_ammo_rockets_c);
//	g_items.push_back(new g_item_ammo_grenades_c);
	g_items.push_back(new g_item_ammo_slugs_c);
}



void	G_ShutdownItems()
{
	for(std::vector<g_item_c*>::const_iterator ir = g_items.begin(); ir != g_items.end(); ir++)
	{
		g_item_c *item = *ir;	
		
		delete item;
	}	
		
	g_items.clear();
}



void	G_SetItemNames()
{
	for(unsigned int i=0; i<g_items.size(); i++)
	{
		g_item_c *item = g_items[i];
		
		trap_SV_SetConfigString(CS_ITEMS+i, item->getPickupName());
	}

	/*
	jacket_armor_index = G_GetNumForItem(G_FindItem("Jacket Armor"));
	combat_armor_index = G_GetNumForItem(G_FindItem("Combat Armor"));
	body_armor_index   = G_GetNumForItem(G_FindItem("Body Armor"));
	power_screen_index = G_GetNumForItem(G_FindItem("Power Screen"));
	power_shield_index = G_GetNumForItem(G_FindItem("Power Shield"));
	*/
}
