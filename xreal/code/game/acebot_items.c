/*
===========================================================================
Copyright (C) 1998 Steve Yeager
Copyright (C) 2008 Robert Beckebans <trebor_7@users.sourceforge.net>

This file is part of XreaL source code.

XreaL source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

XreaL source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XreaL source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
//  acebot_items.c - This file contains all of the 
//                   item handling routines for the 
//                   ACE bot, including fact table support


#include "g_local.h"
#include "acebot.h"

#if defined(ACEBOT)




// Can we get there?
qboolean ACEIT_IsReachable(gentity_t * self, vec3_t goal)
{
	trace_t         trace;
	vec3_t          v;

	VectorCopy(self->r.mins, v);
	v[2] += STEPSIZE;
	
	trap_Trace(&trace, self->client->ps.origin, v, self->r.maxs, goal, self->s.number, MASK_PLAYERSOLID);

	// Yes we can see it
	if(trace.fraction == 1)
		return qtrue;
	else
		return qfalse;
}

// Visiblilty check 
qboolean ACEIT_IsVisible(gentity_t * self, vec3_t goal)
{
	trace_t         trace;
	
	trap_Trace(&trace, self->client->ps.origin, NULL, NULL, goal, self->s.number, MASK_PLAYERSOLID);

	// Yes we can see it
	if(trace.fraction == 1)
		return qtrue;
	else
		return qfalse;
}

//  Weapon changing support
qboolean ACEIT_ChangeWeapon(gentity_t * self, weapon_t weapon)
{
	// see if we're already using it
//	if(self->client->ps.weapon == weapon)
//		return qtrue;

	// has not picked up weapon yet
	if(!(self->client->ps.stats[STAT_WEAPONS] & BIT(weapon)))
		return qfalse;

	// do we have ammo for it?
	if(weapon != WP_GAUNTLET && weapon != WP_GRAPPLING_HOOK && !self->client->ps.ammo[weapon])
		return qfalse;

	// change to this weapon
	self->client->pers.cmd.weapon = weapon;

	return qtrue;
}


// Check if we can use the armor
qboolean ACEIT_CanUseArmor(gitem_t * item, gentity_t * other)
{
#if 0
	int             old_armor_index;
	gitem_armor_t  *oldinfo;
	gitem_armor_t  *newinfo;
	int             newcount;
	float           salvage;
	int             salvagecount;

	// get info on new armor
	newinfo = (gitem_armor_t *) item->info;

	old_armor_index = ArmorIndex(other);

	// handle armor shards specially
	if(item->tag == ARMOR_SHARD)
		return true;

	// get info on old armor
	if(old_armor_index == ITEM_INDEX(FindItem("Jacket Armor")))
		oldinfo = &jacketarmor_info;
	else if(old_armor_index == ITEM_INDEX(FindItem("Combat Armor")))
		oldinfo = &combatarmor_info;
	else						// (old_armor_index == body_armor_index)
		oldinfo = &bodyarmor_info;

	if(newinfo->normal_protection <= oldinfo->normal_protection)
	{
		// calc new armor values
		salvage = newinfo->normal_protection / oldinfo->normal_protection;
		salvagecount = salvage * newinfo->base_count;
		newcount = other->client->pers.inventory[old_armor_index] + salvagecount;

		if(newcount > oldinfo->max_count)
			newcount = oldinfo->max_count;

		// if we're already maxed out then we don't need the new armor
		if(other->client->pers.inventory[old_armor_index] >= newcount)
			return false;

	}

	return true;
#else
	return qfalse;
#endif
}


float ACEIT_ItemNeed(gentity_t * self, gitem_t * item)
{
	// Tr3B: logic based on BG_CanItemBeGrabbed
	
	switch (item->giType)
	{
		case IT_WEAPON:
		{
			// weapons are always picked up
			return 0.7;
		}

		case IT_AMMO:
		{
			if(self->client->ps.ammo[item->giTag] >= 200)
			{
				return 0.0;	// can't hold any more
			}
			return 0.3;
		}

		case IT_ARMOR:
		{
#ifdef MISSIONPACK
			if(bg_itemlist[ps->stats[STAT_PERSISTANT_POWERUP]].giTag == PW_SCOUT)
			{
				return 0.0;
			}

			// we also clamp armor to the maxhealth for handicapping
			if(bg_itemlist[ps->stats[STAT_PERSISTANT_POWERUP]].giTag == PW_GUARD)
			{
				upperBound = ps->stats[STAT_MAX_HEALTH];
			}
			else
			{
				upperBound = ps->stats[STAT_MAX_HEALTH] * 2;
			}

			if(ps->stats[STAT_ARMOR] >= upperBound)
			{
				return 0.0;
			}
#else
			if(self->client->ps.stats[STAT_ARMOR] >= self->client->ps.stats[STAT_MAX_HEALTH] * 2)
			{
				return 0.0;
			}
#endif
			return 0.6;
		}

		case IT_HEALTH:
		{
			// small and mega healths will go over the max, otherwise
			// don't pick up if already at max
#ifdef MISSIONPACK
			if(bg_itemlist[self->client->ps->stats[STAT_PERSISTANT_POWERUP]].giTag == PW_GUARD)
			{
				upperBound = self->client->ps->stats[STAT_MAX_HEALTH];
			}
			else
#endif
			
			if(item->quantity == 5 || item->quantity == 100)
			{
				if(self->client->ps.stats[STAT_HEALTH] >= self->client->ps.stats[STAT_MAX_HEALTH] * 2)
				{
					return 0.0;
				}
				
				return 1.0 - (float)self->health / (self->client->ps.stats[STAT_MAX_HEALTH] * 2);	// worse off, higher priority;
			}

			if(self->client->ps.stats[STAT_HEALTH] >= self->client->ps.stats[STAT_MAX_HEALTH])
			{
				return 0.0;
			}
			
			return 1.0 - (float)self->health / 100.0f;	// worse off, higher priority
		}

		case IT_POWERUP:
		{
			// powerups are always picked up
			return 0.8;
		}
		
		
		default:
			return 0.0;
	}
}


///////////////////////////////////////////////////////////////////////
// Only called once per level, when saved will not be called again
//
// Downside of the routine is that items can not move about. If the level
// has been saved before and reloaded, it could cause a problem if there
// are items that spawn at random locations.
//
//#define DEBUG // uncomment to write out items to a file.
///////////////////////////////////////////////////////////////////////
void ACEIT_BuildItemNodeTable(qboolean rebuild)
{
	int             i;
	gentity_t        *ent;
	vec3_t          v, v1, v2;
	int				nodeType;

	for(i = 0, ent = &g_entities[0]; i < level.numEntities; i++, ent++)
	{
		if(!ent->inuse)
		{
			continue;
		}

		

		/*
		// special node dropping for platforms
		if(!Q_stricmp(ent->item->classname, "func_plat"))
		{
			if(!rebuild)
				ACEND_AddNode(ent, NODE_PLATFORM);
			item_index = 99;	// to allow to pass the item index test
		}

		// special node dropping for teleporters
		if(!Q_stricmp(ent->item->classname, "misc_teleporter_dest") || !Q_stricmp(ent->item->classname, "trigger_teleport"))
		{
			if(!rebuild)
				ACEND_AddNode(ent, NODE_TELEPORTER);
			item_index = 99;
		}
		*/
		
		
		if(ent->item)
		{
			nodeType = NODE_ITEM;
		}
		else if(!Q_stricmp(ent->classname, "trigger_teleport"))
		{
			nodeType = NODE_TRIGGER_TELEPORT;
		}
		else if(!Q_stricmp(ent->classname, "misc_teleporter_dest"))
		{
			nodeType = NODE_TARGET_TELEPORT;
		}
		else if(!Q_stricmp(ent->classname, "trigger_push"))
		{
			nodeType = NODE_JUMPPAD;
		}
		else
		{
			// entity does not matter
			continue;
		}

		// ff new, add nodes for items
		if(!rebuild)
		{
			// add a new node at the item's location.
			ent->node = ACEND_AddNode(ent, nodeType);
		}
		else					// Now if rebuilding, just relink ent structures 
		{
			// Find stored location
			for(i = 0; i < numNodes; i++)
			{
				if(	nodes[i].type == NODE_ITEM ||
					nodes[i].type == NODE_PLATFORM ||
					nodes[i].type == NODE_TRIGGER_TELEPORT || 
					nodes[i].type == NODE_TARGET_TELEPORT)	// valid types
				{
					VectorCopy(ent->s.origin, v);
					
					if(nodes[i].type == NODE_ITEM)
					{
						v[2] += 16;
					}
					else if(nodes[i].type == NODE_TRIGGER_TELEPORT)
					{
						VectorAdd(ent->r.absmin, ent->r.absmax, v);
						VectorScale(v, 0.5, v);
					}
					else if(nodes[i].type == NODE_TARGET_TELEPORT)
					{
						v[2] += 32;
					}
					else if(nodes[i].type == NODE_JUMPPAD)
					{
						VectorAdd(ent->r.absmin, ent->r.absmax, v);
						VectorScale(v, 0.5, v);
					}
					else if(nodes[i].type == NODE_PLATFORM)
					{
						VectorCopy(ent->r.maxs, v1);
						VectorCopy(ent->r.mins, v2);

						// to get the center
						v[0] = (v1[0] - v2[0]) / 2 + v2[0];
						v[1] = (v1[1] - v2[1]) / 2 + v2[1];
						v[2] = ent->r.mins[2] + 64;
					}
					
					SnapVector(v);
					
					if(VectorCompare(v, nodes[i].origin))
					{
						// found a match now link to facts
						ent->node = i;

#ifdef DEBUG
						if(ent->item)
						{
							G_Printf("relink item: %s node: %d pos: %f %f %f\n", ent->item->classname, ent->node,
									ent->s.origin[0], ent->s.origin[1], ent->s.origin[2]);
						}
						else
						{
							G_Printf("relink entity: %s node: %d pos: %f %f %f\n", ent->classname, ent->node,
									ent->s.origin[0], ent->s.origin[1], ent->s.origin[2]);
						}
#endif
					}
				}
			}
		}
		
#ifdef _DEBUG
		//if(item_index == INVALID)
		//	fprintf(pOut, "Rejected item: %s node: %d pos: %f %f %f\n", ent->item->classname, ent->node,
		//			ent->s.origin[0], ent->s.origin[1], ent->s.origin[2]);
		//else
		if(ent->item)
		{
			G_Printf("accepted item: %s node: %d pos: %f %f %f\n", ent->item->classname, ent->node, ent->s.origin[0],
					ent->s.origin[1], ent->s.origin[2]);
		}
		else
		{
			G_Printf("accepted entity: %s node: %d pos: %f %f %f\n", ent->classname, ent->node, ent->s.origin[0],
					ent->s.origin[1], ent->s.origin[2]);
		}
#endif

	}

}

#endif

