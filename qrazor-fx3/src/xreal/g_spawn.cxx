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
#include <string>
#include <map>
#include <algorithm>
#include <utility>
// xreal --------------------------------------------------------------------
#include "g_local.h"
#include "g_item.h"



struct spawn_t
{
	char	*name;
	void	(*spawn)(g_entity_c **ent);
	//void	(*activate)(g_entity_c *ent);
};


void SP_item_health(g_entity_c *self);
void SP_item_health_small(g_entity_c *self);
void SP_item_health_large(g_entity_c *self);
void SP_item_health_mega(g_entity_c *self);

void SP_info_player_start(g_entity_c **entity);
void SP_info_player_deathmatch(g_entity_c **entity);
void SP_info_player_coop(g_entity_c *ent);

void SP_func_plat(g_entity_c *ent);
void SP_func_rotating(g_entity_c **entity);
void SP_func_button(g_entity_c **entity);
void SP_func_door(g_entity_c **entity);
void SP_func_door_secret(g_entity_c *ent);
void SP_func_door_rotating(g_entity_c *ent);
void SP_func_water(g_entity_c *ent);
void SP_func_train(g_entity_c *ent);
void SP_func_conveyor(g_entity_c *self);
void SP_func_wall(g_entity_c *self);
void SP_func_object(g_entity_c *self);
void SP_func_explosive(g_entity_c *self);
void SP_func_timer(g_entity_c *self);
void SP_func_clock(g_entity_c *ent);
void SP_func_killbox(g_entity_c **entity);
void SP_func_static(g_entity_c **entity);
void SP_func_flare(g_entity_c **entity);

void SP_trigger_always(g_entity_c *ent);
void SP_trigger_once(g_entity_c **entity);
void SP_trigger_multiple(g_entity_c **entity);
void SP_trigger_relay(g_entity_c *ent);
void SP_trigger_push(g_entity_c **entity);
void SP_trigger_teleport(g_entity_c *ent);
void SP_trigger_hurt(g_entity_c **entity);
void SP_trigger_key(g_entity_c *ent);
void SP_trigger_counter(g_entity_c *ent);
void SP_trigger_elevator(g_entity_c *ent);
void SP_trigger_gravity(g_entity_c **entity);
void SP_trigger_teleport(g_entity_c **entity);

void SP_target_temp_entity(g_entity_c *ent);
void SP_target_speaker(g_entity_c **entity);
void SP_target_explosion(g_entity_c **entity);
void SP_target_changelevel(g_entity_c *ent);
void SP_target_splash(g_entity_c *ent);
void SP_target_spawner(g_entity_c *ent);
void SP_target_blaster(g_entity_c *ent);
void SP_target_crosslevel_trigger(g_entity_c *ent);
void SP_target_crosslevel_target(g_entity_c *ent);
void SP_target_laser(g_entity_c *self);
void SP_target_help(g_entity_c **entity);
void SP_target_actor(g_entity_c *ent);
void SP_target_lightramp(g_entity_c *self);
void SP_target_earthquake(g_entity_c *ent);
void SP_target_character(g_entity_c *ent);
void SP_target_string(g_entity_c *ent);
void SP_target_position(g_entity_c **entity);
void SP_target_teleport(g_entity_c **entity);


void SP_worldspawn(g_entity_c **entity);

void SP_light(g_entity_c **entity);

void	SP_ode_box(g_entity_c **entity);



spawn_t	spawns[] = {
//	{"item_health",			SP_item_health},
//	{"item_health_small",		SP_item_health_small},
//	{"item_health_large",		SP_item_health_large},
//	{"item_health_mega",		SP_item_health_mega},

	{"info_player_start",		SP_info_player_start},
	{"info_player_deathmatch",	SP_info_player_deathmatch},
//	{"info_player_coop",		SP_info_player_coop},
	{"info_notnull",		SP_target_position},
	{"info_null",			SP_target_position},
	{"info_location",		SP_target_position},

//	{"func_plat",			SP_func_plat},
//	{"func_button",			SP_func_button},
//	{"func_door",			SP_func_door},
//	{"func_door_secret",		SP_func_door_secret},
//	{"func_door_rotating",		SP_func_door_rotating},
//	{"func_rotating",		SP_func_rotating},
//	{"func_train",			SP_func_train},
//	{"func_water",			SP_func_water},
//	{"func_conveyor",		SP_func_conveyor},
//	{"func_clock",			SP_func_clock},
//	{"func_wall",			SP_func_wall},
//	{"func_object",			SP_func_object},
//	{"func_timer",			SP_func_timer},
//	{"func_explosive",		SP_func_explosive},
	{"func_killbox",		SP_func_killbox},
	{"func_static", 		SP_func_static},
//	{"func_flare",			SP_func_flare},

//	{"trigger_always",		SP_trigger_always},
	{"trigger_once",		SP_trigger_once},
	{"trigger_multiple",		SP_trigger_multiple},
//	{"trigger_relay",		SP_trigger_relay},
	{"trigger_push",		SP_trigger_push},
	{"trigger_hurt",		SP_trigger_hurt},
//	{"trigger_key",			SP_trigger_key},
//	{"trigger_counter",		SP_trigger_counter},
//	{"trigger_elevator",		SP_trigger_elevator},
	{"trigger_gravity",		SP_trigger_gravity},
	{"trigger_teleport",		SP_trigger_teleport},

//	{"target_speaker",		SP_target_speaker},
	{"target_explosion",		SP_target_explosion},
//	{"target_changelevel",		SP_target_changelevel},
//	{"target_splash",		SP_target_splash},
//	{"target_spawner",		SP_target_spawner},
//	{"target_blaster",		SP_target_blaster},
//	{"target_crosslevel_trigger",	SP_target_crosslevel_trigger},
//	{"target_crosslevel_target",	SP_target_crosslevel_target},
//	{"target_laser",		SP_target_laser},
//	{"target_help",			SP_target_help},
//	{"target_lightramp",		SP_target_lightramp},
//	{"target_earthquake",		SP_target_earthquake},
//	{"target_character",		SP_target_character},
//	{"target_string",		SP_target_string},
	{"target_location",		SP_target_position},
	{"target_position",		SP_target_position},
//	{"target_teleport",		SP_target_teleport},

	{"worldspawn",			SP_worldspawn},

	{"light",			SP_light},
//	{"rt_light",			SP_light},

	{"misc_teleporter_dest",	SP_target_teleport},
//	{"speaker",			SP_target_speaker},
	
//	{"ode_box",			SP_ode_box},

	{NULL, NULL}
};

/*
===============
ED_CallSpawn

Finds the spawn function for the entity and calls it
===============
*/
static g_entity_c*	G_CallEntitySpawn(const std::string &classname)
{
	spawn_t		*s;
	g_entity_c	*ent = NULL;
	
	
	if(!classname.length())
	{
		gi.Com_Printf("G_CallEntitySpawn: NULL classname\n");
		return NULL;
	}
	
	//gi.Com_Printf ("G_CallEntitySpawn: %s\n", classname);

	//
	// check item spawn functions
	//
	for(std::vector<g_item_c*>::const_iterator ir = g_items.begin(); ir != g_items.end(); ir++)
	{
		g_item_c *item = *ir;
		
		if(!item)
			continue;
	
		if(!item->getClassname())
			continue;
		
		
		if(X_strcaseequal(item->getClassname(), classname.c_str()))
		{
			// found it
			G_SpawnItem(&ent, item);
			return ent;
		}
	}

	//
	// check normal spawn functions
	//
	for(s=spawns; s->name; s++)
	{
		if(!X_stricmp(s->name, classname.c_str()))
		{
			// found it
			s->spawn(&ent);
			return ent;
		}
	}
	
	return NULL;
}






/*
====================
ED_ParseEdict

Parses an edict out of the given string, returning the new position
ed should be a properly initialized empty edict.
====================
*/
static char*	G_ParseEntity(char *data, std::map<std::string, std::string> &map)
{
	std::string		key;
	std::string		value;
			
	char	*token;
	

	// go through all the dictionary pairs
	while(true)
	{	
		//
		// parse key
		//
		token = Com_Parse(&data);
		
		if(token[0] == '}')
			break;
		
		if(!data)
			gi.Com_Error(ERR_DROP, "G_ParseEntity: EOF without closing brace");

		key = token;
	
		//
		// parse value	
		//
		token = Com_Parse(&data);
		
		if(!data)
			gi.Com_Error(ERR_DROP, "G_ParseEntity: EOF without closing brace");

		if(token[0] == '}')
			gi.Com_Error(ERR_DROP, "G_ParseEntity: closing brace without data");



		// keynames with a leading underscore are used for utility comments,
		// and are immediately discarded by quake
		//if (key[0] == '_')
		//	continue;
			
		value = token;

		map.insert(std::make_pair(key, value));
	}

	return data;
}






/*
================
G_FindTeams

Chain together all entities with a matching team field.

All but the first will have the FL_TEAMSLAVE flag set.
All but the last will have the teamchain field set to the next one
================
*/
static void	G_FindTeams()
{
	g_entity_c	*e, *e2, *chain;
	int		i, j;
	int		c, c2;

	c = 0;
	c2 = 0;
	for(i=1; i<(int)g_entities.size(); i++)
	{
		e = (g_entity_c*)g_entities[i];
		
		if(!e)
			continue;
	
		if(!e->_r.inuse)
			continue;
			
		if(!e->_team.length())
			continue;
			
		if(e->_flags & FL_TEAMSLAVE)
			continue;
			
		chain = e;
		e->_teammaster = e;
		c++;
		c2++;
		for(j=i+1; j<(int)g_entities.size(); j++)
		{
			e2 = (g_entity_c*)g_entities[j];
			
			if(!e2)
				continue;
		
			if(!e2->_r.inuse)
				continue;
				
			if(!e2->_team.length())
				continue;
				
			if(e2->_flags & FL_TEAMSLAVE)
				continue;
			
			//gi.Com_Printf("G_FindTeams: '%i' '%i'\n", i, j);
			//gi.Com_Printf("G_FindTeams: '%s'\n", e->classname);
			
			if(e->_team == e2->_team)
			{
				c2++;
				chain->_teamchain = e2;
				e2->_teammaster = e;
				chain = e2;
				e2->_flags |= FL_TEAMSLAVE;
				
				gi.Com_Printf("G_FindTeams: '%s' '%s' are in team '%s'\n", e->getClassName(), e2->getClassName(), e->_team.c_str());
			}
		}
	}

	gi.Com_Printf("%i teams with %i entities\n", c, c2);
}


const char*	G_ValueForKey(std::map<std::string, std::string> &epairs, const std::string &key)
{
	std::map<std::string, std::string>::iterator ir = epairs.find(key);
			
	if(ir != epairs.end())
	{
		return ir->second.c_str();
	}
	else
	{
		return "";
	}
}


/*
==============
SpawnEntities

Creates a server's entity / program execution context by
parsing textual entity definitions out of an ent file.
==============
*/
void	G_SpawnEntities(const std::string &mapname, char *entities, const std::string &spawnpoint)
{
	g_entity_c	*ent;
	char		*token;
	const char	*classname = NULL, *value = NULL;
	
	int		inhibit;
	int		spawned;
	float		skill_level;
	
	std::map<std::string, std::string>	epairs;
	
	//gi.Com_Printf("G_SpawnEntities:\n");
	skill_level = floorf(skill->getInteger());
	
	if(skill_level < 0)
		skill_level = 0;
		
	if(skill_level > 3)
		skill_level = 3;
		
	if(skill->getInteger() != skill_level)
		gi.Cvar_ForceSet("skill", va("%f", skill_level));

	G_SaveClientData();

	//gi.Z_FreeTags (TAG_LEVEL);

	//memset (&level, 0, sizeof(level));
			
	level.mapname = mapname;
	game.spawnpoint = spawnpoint;


	ent = NULL;
	inhibit = 0;
	spawned = 0;
	
	// parse ents
	while(true)
	{
		epairs.clear();
		
		// parse the opening brace	
		token = Com_Parse(&entities);
		
		if(!entities)
			break;
			
		if(token[0] != '{')
			gi.Com_Error(ERR_DROP, "G_SpawnEntities: found %s when expecting {", token);


		// spawn custom entity
		ent = NULL;
			
		entities = G_ParseEntity(entities, epairs);
			
		std::map<std::string, std::string>::const_iterator ir = epairs.find("classname");
			
		if(ir != epairs.end())
			classname = ir->second.c_str();
		else
			classname = NULL;
			
			
		ent = G_CallEntitySpawn(classname);
						
		if(!ent)
		{
			gi.Com_Printf("G_SpawnEntities: could not spawn %s\n", classname);
			continue;
		}
		else
		{
			gi.Com_Printf("G_SpawnEntities: spawned %s\n", classname);
			spawned++;
		}
		
		ent->setEPairs(epairs);	
		ent->setFields(epairs);
		
		// HACK
		/*
		name = G_ValueForKey( epairs, "name" );
		model = G_ValueForKey( epairs, "model" );
		if( name[ 0 ] != '\0' && model[ 0 ] != '\0' && strcmp( name, model ) == 0 )
			ent->_s.origin.clear();
		*/
		
		value = ent->valueForKey("rotation");
		if(value[0] != '\0')
			ent->setField("rotation", value);
		
		ent->activate();
			

		// remove things (except the world) from different skill levels or deathmatch
		if(ent != g_entities[0])
		{
			if(deathmatch->getInteger())
			{
				if(ent->_spawnflags & SPAWNFLAG_NOT_DEATHMATCH)
				{
					delete ent;
					inhibit++;
					continue;
				}
			}
			else
			{
				if( /* ((coop->getInteger()) && (ent->spawnflags & SPAWNFLAG_NOT_COOP)) || */
					((skill->getInteger() == 0) && (ent->_spawnflags & SPAWNFLAG_NOT_EASY)) ||
					((skill->getInteger() == 1) && (ent->_spawnflags & SPAWNFLAG_NOT_MEDIUM)) ||
					(((skill->getInteger() == 2) || (skill->getInteger() == 3)) && (ent->_spawnflags & SPAWNFLAG_NOT_HARD))
					)
					{
						delete ent;
						inhibit++;
						continue;
					}
			}

			ent->_spawnflags &= ~(SPAWNFLAG_NOT_EASY|SPAWNFLAG_NOT_MEDIUM|SPAWNFLAG_NOT_HARD|SPAWNFLAG_NOT_COOP|SPAWNFLAG_NOT_DEATHMATCH);
		}
	}	


	gi.Com_Printf("%i entities inhibited\n", inhibit);
	gi.Com_Printf("%i entities spawned\n", spawned);

	// create team links
	G_FindTeams ();
}








