/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006 Robert Beckebans <trebor_7@users.sourceforge.net>

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

#include "g_local.h"

qboolean G_SpawnString(const char *key, const char *defaultString, char **out)
{
	int             i;

	if(!level.spawning)
	{
		*out = (char *)defaultString;
//      G_Error( "G_SpawnString() called while not spawning" );
	}

	for(i = 0; i < level.numSpawnVars; i++)
	{
		if(!Q_stricmp(key, level.spawnVars[i][0]))
		{
			*out = level.spawnVars[i][1];
			return qtrue;
		}
	}

	*out = (char *)defaultString;
	return qfalse;
}

qboolean G_SpawnFloat(const char *key, const char *defaultString, float *out)
{
	char           *s;
	qboolean        present;

	present = G_SpawnString(key, defaultString, &s);
	*out = atof(s);
	return present;
}

qboolean G_SpawnInt(const char *key, const char *defaultString, int *out)
{
	char           *s;
	qboolean        present;

	present = G_SpawnString(key, defaultString, &s);
	*out = atoi(s);
	return present;
}

qboolean G_SpawnVector(const char *key, const char *defaultString, float *out)
{
	char           *s;
	qboolean        present;

	present = G_SpawnString(key, defaultString, &s);
	sscanf(s, "%f %f %f", &out[0], &out[1], &out[2]);
	return present;
}



//
// fields are needed for spawning from the entity string
//
typedef enum
{
	F_INT,
	F_FLOAT,
	F_LSTRING,					// string on disk, pointer in memory, TAG_LEVEL
	F_GSTRING,					// string on disk, pointer in memory, TAG_GAME
	F_VECTOR,
	F_ANGLEHACK,
	F_MOVEDIRHACK,
	F_ROTATIONHACK,
	F_ENTITY,					// index on disk, pointer in memory
	F_ITEM,						// index on disk, pointer in memory
	F_CLIENT,					// index on disk, pointer in memory
	F_IGNORE
} fieldtype_t;

typedef struct
{
	char           *name;
	int             ofs;
	fieldtype_t     type;
	int             flags;
} field_t;

field_t         fields[] = {
	{"classname", FOFS(classname), F_LSTRING},
	{"origin", FOFS(s.origin), F_VECTOR},
	{"model", FOFS(model), F_LSTRING},
	{"model2", FOFS(model2), F_LSTRING},
	{"spawnflags", FOFS(spawnflags), F_INT},
	{"speed", FOFS(speed), F_FLOAT},
	{"target", FOFS(target), F_LSTRING},
//JH break q3/d3
	{"targetname", FOFS(targetname), F_LSTRING},
	{"message", FOFS(message), F_LSTRING},
	{"team", FOFS(team), F_LSTRING},
	{"wait", FOFS(wait), F_FLOAT},
	{"random", FOFS(random), F_FLOAT},
	{"count", FOFS(count), F_INT},
	{"health", FOFS(health), F_INT},
	{"light", 0, F_IGNORE},
	{"dmg", FOFS(damage), F_INT},
	{"angles", FOFS(s.angles), F_VECTOR},
	{"angle", FOFS(s.angles), F_ANGLEHACK},
	{"movedir", FOFS(movedir), F_MOVEDIRHACK},
	{"rotation", FOFS(s.angles), F_ROTATIONHACK},
	{"targetShaderName", FOFS(targetShaderName), F_LSTRING},
	{"targetShaderNewName", FOFS(targetShaderNewName), F_LSTRING},

#ifdef LUA
	{"luaThink", FOFS(luaThink), F_LSTRING},
	{"luaTouch", FOFS(luaTouch), F_LSTRING},
#endif

	{NULL}
};


typedef struct
{
	char           *name;
	void            (*spawn) (gentity_t * ent);
} spawn_t;

void            SP_info_player_start(gentity_t * ent);
void            SP_info_player_deathmatch(gentity_t * ent);
void            SP_info_player_intermission(gentity_t * ent);
//void            SP_info_player_teleport(gentity_t * ent);
void            SP_info_firstplace(gentity_t * ent);
void            SP_info_secondplace(gentity_t * ent);
void            SP_info_thirdplace(gentity_t * ent);
void            SP_info_podium(gentity_t * ent);

void            SP_func_plat(gentity_t * ent);
void            SP_func_static(gentity_t * ent);
void            SP_func_rotating(gentity_t * ent);
void            SP_func_bobbing(gentity_t * ent);
void            SP_func_pendulum(gentity_t * ent);
void            SP_func_button(gentity_t * ent);
void            SP_func_door(gentity_t * ent);
void            SP_func_train(gentity_t * ent);
void            SP_func_timer(gentity_t * self);
void            SP_func_mover(gentity_t * self);
void            SP_func_teleporter(gentity_t * self);

void            SP_trigger_always(gentity_t * ent);
void            SP_trigger_multiple(gentity_t * ent);
void            SP_trigger_push(gentity_t * ent);
void            SP_trigger_teleport(gentity_t * ent);
void            SP_trigger_hurt(gentity_t * ent);

void            SP_target_remove_powerups(gentity_t * ent);
void            SP_target_give(gentity_t * ent);
void            SP_target_delay(gentity_t * ent);
void            SP_target_speaker(gentity_t * ent);
void            SP_target_print(gentity_t * ent);
void            SP_target_laser(gentity_t * self);
void            SP_target_character(gentity_t * ent);
void            SP_target_score(gentity_t * ent);
void            SP_target_teleporter(gentity_t * ent);
void            SP_target_relay(gentity_t * ent);
void            SP_target_kill(gentity_t * ent);
void            SP_target_position(gentity_t * ent);
void            SP_target_null(gentity_t * ent);
void            SP_target_location(gentity_t * ent);
void            SP_target_push(gentity_t * ent);
void            SP_target_fx(gentity_t * ent);

void            SP_light(gentity_t * self);
void            SP_info_null(gentity_t * self);
void            SP_info_notnull(gentity_t * self);
void            SP_info_camp(gentity_t * self);
void            SP_path_corner(gentity_t * self);

void            SP_misc_teleporter_dest(gentity_t * self);
void            SP_misc_model(gentity_t * ent);
void            SP_misc_portal_camera(gentity_t * ent);
void            SP_misc_portal_surface(gentity_t * ent);
void            SP_misc_portalsky_surface(gentity_t * ent);

void            SP_shooter_rocket(gentity_t * ent);
void            SP_shooter_plasma(gentity_t * ent);
void            SP_shooter_grenade(gentity_t * ent);

void            SP_team_CTF_redplayer(gentity_t * ent);
void            SP_team_CTF_blueplayer(gentity_t * ent);

void            SP_team_CTF_redspawn(gentity_t * ent);
void            SP_team_CTF_bluespawn(gentity_t * ent);
void            SP_spark_emission(gentity_t * ent);

void            SP_propsFireColumn(gentity_t * ent);

#ifdef MISSIONPACK
void            SP_team_blueobelisk(gentity_t * ent);
void            SP_team_redobelisk(gentity_t * ent);
void            SP_team_neutralobelisk(gentity_t * ent);
#endif
void SP_item_botroam(gentity_t * ent)
{
}

spawn_t         spawns[] = {
	// info entities don't do anything at all, but provide positional
	// information for things controlled by other processes
	{"info_player_start", SP_info_player_start},
	{"info_player_deathmatch", SP_info_player_deathmatch},
	{"info_player_intermission", SP_info_player_intermission},
	//{"info_player_teleport", SP_info_player_teleport},
	{"info_null", SP_info_null},
	{"info_notnull", SP_info_notnull},	// use target_position instead
	{"info_camp", SP_info_camp},

	{"func_plat", SP_func_plat},
	{"func_button", SP_func_button},
	{"func_door", SP_func_door},
	{"func_static", SP_func_static},
	{"func_rotating", SP_func_rotating},
	{"func_bobbing", SP_func_bobbing},
	{"func_pendulum", SP_func_pendulum},
	{"func_train", SP_func_train},
	{"func_group", SP_info_null},
	{"func_timer", SP_func_timer},	// rename trigger_timer?
	{"func_mover", SP_func_mover},
	{"func_teleporter", SP_func_teleporter},

	// Triggers are brush objects that cause an effect when contacted
	// by a living player, usually involving firing targets.
	// While almost everything could be done with
	// a single trigger class and different targets, triggered effects
	// could not be client side predicted (push and teleport).
	{"trigger_always", SP_trigger_always},
	{"trigger_multiple", SP_trigger_multiple},
	{"trigger_push", SP_trigger_push},
	{"trigger_teleport", SP_trigger_teleport},
	{"trigger_hurt", SP_trigger_hurt},

	// targets perform no action by themselves, but must be triggered
	// by another entity
	{"target_give", SP_target_give},
	{"target_remove_powerups", SP_target_remove_powerups},
	{"target_delay", SP_target_delay},
	{"target_speaker", SP_target_speaker},
	{"target_print", SP_target_print},
	{"target_laser", SP_target_laser},
	{"target_score", SP_target_score},
	{"target_teleporter", SP_target_teleporter},
	{"target_relay", SP_target_relay},
	{"target_kill", SP_target_kill},
	{"target_position", SP_target_position},
	{"target_null", SP_target_null},
	{"target_location", SP_target_location},
	{"target_push", SP_target_push},
	{"target_fx", SP_target_fx},

	{"light", SP_light},
	{"path_corner", SP_path_corner},

	{"misc_teleporter_dest", SP_misc_teleporter_dest},
	{"misc_model", SP_misc_model},
	{"misc_portal_surface", SP_misc_portal_surface},
	{"misc_portalsky_surface", SP_misc_portalsky_surface},
	{"misc_portal_camera", SP_misc_portal_camera},

	{"shooter_rocket", SP_shooter_rocket},
	{"shooter_grenade", SP_shooter_grenade},
	{"shooter_plasma", SP_shooter_plasma},

	{"props_FireColumn", SP_propsFireColumn},

	{"team_CTF_redplayer", SP_team_CTF_redplayer},
	{"team_CTF_blueplayer", SP_team_CTF_blueplayer},

	{"team_CTF_redspawn", SP_team_CTF_redspawn},
	{"team_CTF_bluespawn", SP_team_CTF_bluespawn},

#ifdef MISSIONPACK
	{"team_redobelisk", SP_team_redobelisk},
	{"team_blueobelisk", SP_team_blueobelisk},
	{"team_neutralobelisk", SP_team_neutralobelisk},
#endif
	{"item_botroam", SP_item_botroam},

	{0, 0}
};

/*
===============
G_CallSpawn

Finds the spawn function for the entity and calls it,
returning qfalse if not found
===============
*/
qboolean G_CallSpawn(gentity_t * ent)
{
	spawn_t        *s;
	gitem_t        *item;
	gitem_t        *ritem;



	if(DisGaunt.integer)
	{
		if(((int)(DisGaunt.integer == 1) && (Q_stricmp(ent->classname, "weapon_gauntlet") == 0)))
		{
			return qfalse;
		}
	}
	if(DisableSG.integer)
	{
		if(((int)(DisableSG.integer == 1) && (Q_stricmp(ent->classname, "weapon_shotgun") == 0)))
		{
			return qfalse;
		}
	}

	if(DisableGL.integer)
	{
		if(((int)(DisableGL.integer == 1) && (Q_stricmp(ent->classname, "weapon_grenadelauncher") == 0)))
		{
			return qfalse;
		}
	}
	if(DisableRL.integer)
	{
		if(((int)(DisableRL.integer == 1) && (Q_stricmp(ent->classname, "weapon_rocketlauncher") == 0)))
		{
			return qfalse;
		}
	}
	if(DisableLG.integer)
	{
		if(((int)(DisableLG.integer == 1) && (Q_stricmp(ent->classname, "weapon_lightning") == 0)))
		{
			return qfalse;
		}
	}
	if(DisableRG.integer)
	{
		if(((int)(DisableRG.integer == 1) && (Q_stricmp(ent->classname, "weapon_railgun") == 0)))
		{
			return qfalse;
		}
	}
	if(DisablePG.integer)
	{
		if(((int)(DisablePG.integer == 1) && (Q_stricmp(ent->classname, "weapon_plasmagun") == 0)))
		{
			return qfalse;
		}
	}
	if(DisableBFG.integer)
	{
		if(((int)(DisableBFG.integer == 1) && (Q_stricmp(ent->classname, "weapon_bfg") == 0)))
		{
			return qfalse;
		}
	}
	if(DisableMG.integer)
	{
		if(((int)(DisableMG.integer == 1) && (Q_stricmp(ent->classname, "weapon_machinegun") == 0)))
		{
			return qfalse;
		}
	}
	if(Disshard.integer)
	{
		if(((int)(Disshard.integer == 1) && (Q_stricmp(ent->classname, "item_armor_shard") == 0)))
		{
			return qfalse;
		}
	}
	if(DisableYA.integer)
	{
		if(((int)(DisableYA.integer == 1) && (Q_stricmp(ent->classname, "item_armor_combat") == 0)))
		{
			return qfalse;
		}
	}
	if(DisableRA.integer)
	{
		if(((int)(DisableRA.integer == 1) && (Q_stricmp(ent->classname, "item_armor_body") == 0)))
		{
			return qfalse;
		}
	}
	if(Dis5health.integer)
	{
		if(((int)(Dis5health.integer == 1) && (Q_stricmp(ent->classname, "item_health_small") == 0)))
		{
			return qfalse;
		}
	}
	if(Dis25health.integer)
	{
		if(((int)(Dis25health.integer == 1) && (Q_stricmp(ent->classname, "item_health") == 0)))
		{
			return qfalse;
		}
	}
	if(Dis50health.integer)
	{
		if(((int)(Dis50health.integer == 1) && (Q_stricmp(ent->classname, "item_health_large") == 0)))
		{
			return qfalse;
		}
	}
	if(DisMhealth.integer)
	{
		if(((int)(DisMhealth.integer == 1) && (Q_stricmp(ent->classname, "item_health_mega") == 0)))
		{
			return qfalse;
		}
	}
	if(Disteleporter.integer)
	{
		if(((int)(Disteleporter.integer == 1) && (Q_stricmp(ent->classname, "holdable_teleporter") == 0)))
		{
			return qfalse;
		}
	}
	if(Dismedkit.integer)
	{
		if(((int)(Dismedkit.integer == 1) && (Q_stricmp(ent->classname, "holdable_medkit") == 0)))
		{
			return qfalse;
		}
	}
	if(DisableQuad.integer)
	{
		if(((int)(DisableQuad.integer == 1) && (Q_stricmp(ent->classname, "item_quad") == 0)))
		{
			return qfalse;
		}
	}
	if(Disenviro.integer)
	{
		if(((int)(Disenviro.integer == 1) && (Q_stricmp(ent->classname, "item_enviro") == 0)))
		{
			return qfalse;
		}
	}
	if(DisableSG.integer)
	{
		if(((int)(DisableSG.integer == 1) && (Q_stricmp(ent->classname, "ammo_shells") == 0)))
		{
			return qfalse;
		}
	}
	if(DisableMG.integer)
	{
		if(((int)(DisableMG.integer == 1) && (Q_stricmp(ent->classname, "ammo_bullets") == 0)))
		{
			return qfalse;
		}
	}
	if(DisableGL.integer)
	{
		if(((int)(DisableGL.integer == 1) && (Q_stricmp(ent->classname, "ammo_grenades") == 0)))
		{
			return qfalse;
		}
	}
	if(DisablePG.integer)
	{
		if(((int)(DisablePG.integer == 1) && (Q_stricmp(ent->classname, "ammo_cells") == 0)))
		{
			return qfalse;
		}
	}
	if(DisableLG.integer)
	{
		if(((int)(DisableLG.integer == 1) && (Q_stricmp(ent->classname, "ammo_lightning") == 0)))
		{
			return qfalse;
		}
	}
	if(DisableRL.integer)
	{
		if(((int)(DisableRL.integer == 1) && (Q_stricmp(ent->classname, "ammo_rockets") == 0)))
		{
			return qfalse;
		}
	}
	if(DisableBFG.integer)
	{
		if(((int)(DisableBFG.integer == 1) && (Q_stricmp(ent->classname, "ammo_bfg") == 0)))
		{
			return qfalse;
		}
	}
	if(DisableRG.integer)
	{
		if(((int)(DisableRG.integer == 1) && (Q_stricmp(ent->classname, "ammo_slugs") == 0)))
		{
			return qfalse;
		}
	}
	if(DisableHaste.integer)
	{
		if(((int)(DisableHaste.integer == 1) && (Q_stricmp(ent->classname, "item_haste") == 0)))
		{
			return qfalse;
		}
	}
	if(DisableInvis.integer)
	{
		if(((int)(DisableInvis.integer == 1) && (Q_stricmp(ent->classname, "item_invis") == 0)))
		{
			return qfalse;
		}
	}
	if(DisableRegen.integer)
	{
		if(((int)(DisableRegen.integer == 1) && (Q_stricmp(ent->classname, "item_regen") == 0)))
		{
			return qfalse;
		}
	}
	if(DisableFlight.integer)
	{
		if(((int)(DisableFlight.integer == 1) && (Q_stricmp(ent->classname, "item_flight") == 0)))
		{
			return qfalse;
		}
	}



	if(!ent->classname)
	{
		G_Printf("G_CallSpawn: NULL classname\n");
		return qfalse;
	}

	// check item spawn functions
	for(item = bg_itemlist + 1; item->classname; item++)
	{

		//shotgun replacement
		if(ReplaceSG.integer == 1)
		{
			if(!strcmp(ent->classname, "weapon_shotgun"))
			{
				ritem = BG_FindItemForWeapon(WP_MACHINEGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_shells"))
			{
				ritem = BG_FindAmmoForWeapon(WP_MACHINEGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceSG.integer == 2)
		{
			if(!strcmp(ent->classname, "weapon_shotgun"))
			{
				ritem = BG_FindItemForWeapon(WP_GRENADE_LAUNCHER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_shells"))
			{
				ritem = BG_FindAmmoForWeapon(WP_GRENADE_LAUNCHER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceSG.integer == 3)
		{
			if(!strcmp(ent->classname, "weapon_shotgun"))
			{
				ritem = BG_FindItemForWeapon(WP_ROCKET_LAUNCHER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_shells"))
			{
				ritem = BG_FindAmmoForWeapon(WP_ROCKET_LAUNCHER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceSG.integer == 4)
		{
			if(!strcmp(ent->classname, "weapon_shotgun"))
			{
				ritem = BG_FindItemForWeapon(WP_LIGHTNING);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_shells"))
			{
				ritem = BG_FindAmmoForWeapon(WP_LIGHTNING);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceSG.integer == 5)
		{
			if(!strcmp(ent->classname, "weapon_shotgun"))
			{
				ritem = BG_FindItemForWeapon(WP_RAILGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_shells"))
			{
				ritem = BG_FindAmmoForWeapon(WP_RAILGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceSG.integer == 6)
		{
			if(!strcmp(ent->classname, "weapon_shotgun"))
			{
				ritem = BG_FindItemForWeapon(WP_PLASMAGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_shells"))
			{
				ritem = BG_FindAmmoForWeapon(WP_PLASMAGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceSG.integer == 7)
		{
			if(!strcmp(ent->classname, "weapon_shotgun"))
			{
				ritem = BG_FindItemForWeapon(WP_BFG);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_shells"))
			{
				ritem = BG_FindAmmoForWeapon(WP_BFG);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceSG.integer == 8)
		{
			if(!strcmp(ent->classname, "weapon_shotgun"))
			{
				ritem = BG_FindItemForWeapon(WP_IRAILGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_shells"))
			{
				ritem = BG_FindAmmoForWeapon(WP_IRAILGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceSG.integer == 9)
		{
			if(!strcmp(ent->classname, "weapon_shotgun"))
			{
				ritem = BG_FindItemForWeapon(WP_FLAMETHROWER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_shells"))
			{
				ritem = BG_FindAmmoForWeapon(WP_FLAMETHROWER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceSG.integer == 10)
		{
			if(!strcmp(ent->classname, "weapon_shotgun"))
			{
				ritem = BG_FindItemForWeapon(WP_SHOTGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_shells"))
			{
				ritem = BG_FindAmmoForWeapon(WP_SHOTGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		//shotgun replacement

		//grenade launcher replacement
		if(ReplaceGL.integer == 1)
		{
			if(!strcmp(ent->classname, "weapon_grenadelauncher"))
			{
				ritem = BG_FindItemForWeapon(WP_MACHINEGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_grenades"))
			{
				ritem = BG_FindAmmoForWeapon(WP_MACHINEGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceGL.integer == 2)
		{
			if(!strcmp(ent->classname, "weapon_grenadelauncher"))
			{
				ritem = BG_FindItemForWeapon(WP_GRENADE_LAUNCHER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_grenades"))
			{
				ritem = BG_FindAmmoForWeapon(WP_GRENADE_LAUNCHER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceGL.integer == 3)
		{
			if(!strcmp(ent->classname, "weapon_grenadelauncher"))
			{
				ritem = BG_FindItemForWeapon(WP_ROCKET_LAUNCHER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_grenades"))
			{
				ritem = BG_FindAmmoForWeapon(WP_ROCKET_LAUNCHER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceGL.integer == 4)
		{
			if(!strcmp(ent->classname, "weapon_grenadelauncher"))
			{
				ritem = BG_FindItemForWeapon(WP_LIGHTNING);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_grenades"))
			{
				ritem = BG_FindAmmoForWeapon(WP_LIGHTNING);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceGL.integer == 5)
		{
			if(!strcmp(ent->classname, "weapon_grenadelauncher"))
			{
				ritem = BG_FindItemForWeapon(WP_RAILGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_grenades"))
			{
				ritem = BG_FindAmmoForWeapon(WP_RAILGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceGL.integer == 6)
		{
			if(!strcmp(ent->classname, "weapon_grenadelauncher"))
			{
				ritem = BG_FindItemForWeapon(WP_PLASMAGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_grenades"))
			{
				ritem = BG_FindAmmoForWeapon(WP_PLASMAGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceGL.integer == 7)
		{
			if(!strcmp(ent->classname, "weapon_grenadelauncher"))
			{
				ritem = BG_FindItemForWeapon(WP_BFG);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_grenades"))
			{
				ritem = BG_FindAmmoForWeapon(WP_BFG);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceGL.integer == 8)
		{
			if(!strcmp(ent->classname, "weapon_grenadelauncher"))
			{
				ritem = BG_FindItemForWeapon(WP_IRAILGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_grenades"))
			{
				ritem = BG_FindAmmoForWeapon(WP_IRAILGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceGL.integer == 9)
		{
			if(!strcmp(ent->classname, "weapon_grenadelauncher"))
			{
				ritem = BG_FindItemForWeapon(WP_FLAMETHROWER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_grenades"))
			{
				ritem = BG_FindAmmoForWeapon(WP_FLAMETHROWER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceGL.integer == 10)
		{
			if(!strcmp(ent->classname, "weapon_grenadelauncher"))
			{
				ritem = BG_FindItemForWeapon(WP_SHOTGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_grenades"))
			{
				ritem = BG_FindAmmoForWeapon(WP_SHOTGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		//grenade launcher replacement

		//rocket launcher replacement
		if(ReplaceRL.integer == 1)
		{
			if(!strcmp(ent->classname, "weapon_rocketlauncher"))
			{
				ritem = BG_FindItemForWeapon(WP_MACHINEGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_rockets"))
			{
				ritem = BG_FindAmmoForWeapon(WP_MACHINEGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceRL.integer == 2)
		{
			if(!strcmp(ent->classname, "weapon_rocketlauncher"))
			{
				ritem = BG_FindItemForWeapon(WP_GRENADE_LAUNCHER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_rockets"))
			{
				ritem = BG_FindAmmoForWeapon(WP_GRENADE_LAUNCHER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceRL.integer == 3)
		{
			if(!strcmp(ent->classname, "weapon_rocketlauncher"))
			{
				ritem = BG_FindItemForWeapon(WP_ROCKET_LAUNCHER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_rockets"))
			{
				ritem = BG_FindAmmoForWeapon(WP_ROCKET_LAUNCHER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceRL.integer == 4)
		{
			if(!strcmp(ent->classname, "weapon_rocketlauncher"))
			{
				ritem = BG_FindItemForWeapon(WP_LIGHTNING);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_rockets"))
			{
				ritem = BG_FindAmmoForWeapon(WP_LIGHTNING);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceRL.integer == 5)
		{
			if(!strcmp(ent->classname, "weapon_rocketlauncher"))
			{
				ritem = BG_FindItemForWeapon(WP_RAILGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_rockets"))
			{
				ritem = BG_FindAmmoForWeapon(WP_RAILGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceRL.integer == 6)
		{
			if(!strcmp(ent->classname, "weapon_rocketlauncher"))
			{
				ritem = BG_FindItemForWeapon(WP_PLASMAGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_rockets"))
			{
				ritem = BG_FindAmmoForWeapon(WP_PLASMAGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceRL.integer == 7)
		{
			if(!strcmp(ent->classname, "weapon_rocketlauncher"))
			{
				ritem = BG_FindItemForWeapon(WP_BFG);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_rockets"))
			{
				ritem = BG_FindAmmoForWeapon(WP_BFG);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceRL.integer == 8)
		{
			if(!strcmp(ent->classname, "weapon_rocketlauncher"))
			{
				ritem = BG_FindItemForWeapon(WP_IRAILGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_rockets"))
			{
				ritem = BG_FindAmmoForWeapon(WP_IRAILGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceRL.integer == 9)
		{
			if(!strcmp(ent->classname, "weapon_rocketlauncher"))
			{
				ritem = BG_FindItemForWeapon(WP_FLAMETHROWER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_rockets"))
			{
				ritem = BG_FindAmmoForWeapon(WP_FLAMETHROWER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceRL.integer == 10)
		{
			if(!strcmp(ent->classname, "weapon_rocketlauncher"))
			{
				ritem = BG_FindItemForWeapon(WP_SHOTGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_rockets"))
			{
				ritem = BG_FindAmmoForWeapon(WP_SHOTGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		//rocket launcher replacement

		//plasma gun replacement
		if(ReplacePG.integer == 1)
		{
			if(!strcmp(ent->classname, "weapon_plasmagun"))
			{
				ritem = BG_FindItemForWeapon(WP_MACHINEGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_cells"))
			{
				ritem = BG_FindAmmoForWeapon(WP_MACHINEGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplacePG.integer == 2)
		{
			if(!strcmp(ent->classname, "weapon_plasmagun"))
			{
				ritem = BG_FindItemForWeapon(WP_GRENADE_LAUNCHER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_cells"))
			{
				ritem = BG_FindAmmoForWeapon(WP_GRENADE_LAUNCHER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplacePG.integer == 3)
		{
			if(!strcmp(ent->classname, "weapon_plasmagun"))
			{
				ritem = BG_FindItemForWeapon(WP_ROCKET_LAUNCHER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_cells"))
			{
				ritem = BG_FindAmmoForWeapon(WP_ROCKET_LAUNCHER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplacePG.integer == 4)
		{
			if(!strcmp(ent->classname, "weapon_plasmagun"))
			{
				ritem = BG_FindItemForWeapon(WP_LIGHTNING);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_cells"))
			{
				ritem = BG_FindAmmoForWeapon(WP_LIGHTNING);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplacePG.integer == 5)
		{
			if(!strcmp(ent->classname, "weapon_plasmagun"))
			{
				ritem = BG_FindItemForWeapon(WP_RAILGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_cells"))
			{
				ritem = BG_FindAmmoForWeapon(WP_RAILGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplacePG.integer == 6)
		{
			if(!strcmp(ent->classname, "weapon_plasmagun"))
			{
				ritem = BG_FindItemForWeapon(WP_PLASMAGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_cells"))
			{
				ritem = BG_FindAmmoForWeapon(WP_PLASMAGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplacePG.integer == 7)
		{
			if(!strcmp(ent->classname, "weapon_plasmagun"))
			{
				ritem = BG_FindItemForWeapon(WP_BFG);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_cells"))
			{
				ritem = BG_FindAmmoForWeapon(WP_BFG);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplacePG.integer == 8)
		{
			if(!strcmp(ent->classname, "weapon_plasmagun"))
			{
				ritem = BG_FindItemForWeapon(WP_IRAILGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_cells"))
			{
				ritem = BG_FindAmmoForWeapon(WP_IRAILGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplacePG.integer == 9)
		{
			if(!strcmp(ent->classname, "weapon_plasmagun"))
			{
				ritem = BG_FindItemForWeapon(WP_FLAMETHROWER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_cells"))
			{
				ritem = BG_FindAmmoForWeapon(WP_FLAMETHROWER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplacePG.integer == 10)
		{
			if(!strcmp(ent->classname, "weapon_plasmagun"))
			{
				ritem = BG_FindItemForWeapon(WP_SHOTGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_cells"))
			{
				ritem = BG_FindAmmoForWeapon(WP_SHOTGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		//plasma gun replacement

		//particle accelerator replacement
		if(ReplacePA.integer == 1)
		{
			if(!strcmp(ent->classname, "weapon_irailgun"))
			{
				ritem = BG_FindItemForWeapon(WP_MACHINEGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_islugs"))
			{
				ritem = BG_FindAmmoForWeapon(WP_MACHINEGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplacePA.integer == 2)
		{
			if(!strcmp(ent->classname, "weapon_irailgun"))
			{
				ritem = BG_FindItemForWeapon(WP_GRENADE_LAUNCHER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_islugs"))
			{
				ritem = BG_FindAmmoForWeapon(WP_GRENADE_LAUNCHER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplacePA.integer == 3)
		{
			if(!strcmp(ent->classname, "weapon_irailgun"))
			{
				ritem = BG_FindItemForWeapon(WP_ROCKET_LAUNCHER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_islugs"))
			{
				ritem = BG_FindAmmoForWeapon(WP_ROCKET_LAUNCHER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplacePA.integer == 4)
		{
			if(!strcmp(ent->classname, "weapon_irailgun"))
			{
				ritem = BG_FindItemForWeapon(WP_LIGHTNING);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_islugs"))
			{
				ritem = BG_FindAmmoForWeapon(WP_LIGHTNING);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplacePA.integer == 5)
		{
			if(!strcmp(ent->classname, "weapon_irailgun"))
			{
				ritem = BG_FindItemForWeapon(WP_RAILGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_islugs"))
			{
				ritem = BG_FindAmmoForWeapon(WP_RAILGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplacePA.integer == 6)
		{
			if(!strcmp(ent->classname, "weapon_irailgun"))
			{
				ritem = BG_FindItemForWeapon(WP_PLASMAGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_islugs"))
			{
				ritem = BG_FindAmmoForWeapon(WP_PLASMAGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplacePA.integer == 7)
		{
			if(!strcmp(ent->classname, "weapon_irailgun"))
			{
				ritem = BG_FindItemForWeapon(WP_BFG);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_islugs"))
			{
				ritem = BG_FindAmmoForWeapon(WP_BFG);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplacePA.integer == 8)
		{
			if(!strcmp(ent->classname, "weapon_irailgun"))
			{
				ritem = BG_FindItemForWeapon(WP_IRAILGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_islugs"))
			{
				ritem = BG_FindAmmoForWeapon(WP_IRAILGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplacePA.integer == 9)
		{
			if(!strcmp(ent->classname, "weapon_irailgun"))
			{
				ritem = BG_FindItemForWeapon(WP_FLAMETHROWER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_islugs"))
			{
				ritem = BG_FindAmmoForWeapon(WP_FLAMETHROWER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplacePA.integer == 10)
		{
			if(!strcmp(ent->classname, "weapon_irailgun"))
			{
				ritem = BG_FindItemForWeapon(WP_SHOTGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_islugs"))
			{
				ritem = BG_FindAmmoForWeapon(WP_SHOTGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		//particle accelerator replacement

		//lightning gun replacement
		if(ReplaceLG.integer == 1)
		{
			if(!strcmp(ent->classname, "weapon_lightning"))
			{
				ritem = BG_FindItemForWeapon(WP_MACHINEGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_lightning"))
			{
				ritem = BG_FindAmmoForWeapon(WP_MACHINEGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceLG.integer == 2)
		{
			if(!strcmp(ent->classname, "weapon_lightning"))
			{
				ritem = BG_FindItemForWeapon(WP_GRENADE_LAUNCHER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_lightning"))
			{
				ritem = BG_FindAmmoForWeapon(WP_GRENADE_LAUNCHER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceLG.integer == 3)
		{
			if(!strcmp(ent->classname, "weapon_lightning"))
			{
				ritem = BG_FindItemForWeapon(WP_ROCKET_LAUNCHER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_lightning"))
			{
				ritem = BG_FindAmmoForWeapon(WP_ROCKET_LAUNCHER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceLG.integer == 4)
		{
			if(!strcmp(ent->classname, "weapon_lightning"))
			{
				ritem = BG_FindItemForWeapon(WP_LIGHTNING);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_lightning"))
			{
				ritem = BG_FindAmmoForWeapon(WP_LIGHTNING);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceLG.integer == 5)
		{
			if(!strcmp(ent->classname, "weapon_lightning"))
			{
				ritem = BG_FindItemForWeapon(WP_RAILGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_lightning"))
			{
				ritem = BG_FindAmmoForWeapon(WP_RAILGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceLG.integer == 6)
		{
			if(!strcmp(ent->classname, "weapon_lightning"))
			{
				ritem = BG_FindItemForWeapon(WP_PLASMAGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_lightning"))
			{
				ritem = BG_FindAmmoForWeapon(WP_PLASMAGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceLG.integer == 7)
		{
			if(!strcmp(ent->classname, "weapon_lightning"))
			{
				ritem = BG_FindItemForWeapon(WP_BFG);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_lightning"))
			{
				ritem = BG_FindAmmoForWeapon(WP_BFG);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceLG.integer == 8)
		{
			if(!strcmp(ent->classname, "weapon_lightning"))
			{
				ritem = BG_FindItemForWeapon(WP_IRAILGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_lightning"))
			{
				ritem = BG_FindAmmoForWeapon(WP_IRAILGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceLG.integer == 9)
		{
			if(!strcmp(ent->classname, "weapon_lightning"))
			{
				ritem = BG_FindItemForWeapon(WP_FLAMETHROWER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_lightning"))
			{
				ritem = BG_FindAmmoForWeapon(WP_FLAMETHROWER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceLG.integer == 10)
		{
			if(!strcmp(ent->classname, "weapon_lightning"))
			{
				ritem = BG_FindItemForWeapon(WP_SHOTGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_lightning"))
			{
				ritem = BG_FindAmmoForWeapon(WP_SHOTGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		//lightning gun replacement

		//rail gun replacement
		if(ReplaceRG.integer == 1)
		{
			if(!strcmp(ent->classname, "weapon_railgun"))
			{
				ritem = BG_FindItemForWeapon(WP_MACHINEGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_slugs"))
			{
				ritem = BG_FindAmmoForWeapon(WP_MACHINEGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceRG.integer == 2)
		{
			if(!strcmp(ent->classname, "weapon_railgun"))
			{
				ritem = BG_FindItemForWeapon(WP_GRENADE_LAUNCHER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_slugs"))
			{
				ritem = BG_FindAmmoForWeapon(WP_GRENADE_LAUNCHER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceRG.integer == 3)
		{
			if(!strcmp(ent->classname, "weapon_railgun"))
			{
				ritem = BG_FindItemForWeapon(WP_ROCKET_LAUNCHER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_slugs"))
			{
				ritem = BG_FindAmmoForWeapon(WP_ROCKET_LAUNCHER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceRG.integer == 4)
		{
			if(!strcmp(ent->classname, "weapon_railgun"))
			{
				ritem = BG_FindItemForWeapon(WP_LIGHTNING);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_slugs"))
			{
				ritem = BG_FindAmmoForWeapon(WP_LIGHTNING);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceRG.integer == 5)
		{
			if(!strcmp(ent->classname, "weapon_railgun"))
			{
				ritem = BG_FindItemForWeapon(WP_RAILGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_slugs"))
			{
				ritem = BG_FindAmmoForWeapon(WP_RAILGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceRG.integer == 6)
		{
			if(!strcmp(ent->classname, "weapon_railgun"))
			{
				ritem = BG_FindItemForWeapon(WP_PLASMAGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_slugs"))
			{
				ritem = BG_FindAmmoForWeapon(WP_PLASMAGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceRG.integer == 7)
		{
			if(!strcmp(ent->classname, "weapon_railgun"))
			{
				ritem = BG_FindItemForWeapon(WP_BFG);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_slugs"))
			{
				ritem = BG_FindAmmoForWeapon(WP_BFG);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceRG.integer == 8)
		{
			if(!strcmp(ent->classname, "weapon_railgun"))
			{
				ritem = BG_FindItemForWeapon(WP_IRAILGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_slugs"))
			{
				ritem = BG_FindAmmoForWeapon(WP_IRAILGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceRG.integer == 9)
		{
			if(!strcmp(ent->classname, "weapon_railgun"))
			{
				ritem = BG_FindItemForWeapon(WP_FLAMETHROWER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_slugs"))
			{
				ritem = BG_FindAmmoForWeapon(WP_FLAMETHROWER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceRG.integer == 10)
		{
			if(!strcmp(ent->classname, "weapon_railgun"))
			{
				ritem = BG_FindItemForWeapon(WP_SHOTGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_slugs"))
			{
				ritem = BG_FindAmmoForWeapon(WP_SHOTGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		//rail gun replacement

		//flame thrower replacement
		if(ReplaceFT.integer == 1)
		{
			if(!strcmp(ent->classname, "weapon_ft"))
			{
				ritem = BG_FindItemForWeapon(WP_MACHINEGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_fuel"))
			{
				ritem = BG_FindAmmoForWeapon(WP_MACHINEGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceFT.integer == 2)
		{
			if(!strcmp(ent->classname, "weapon_ft"))
			{
				ritem = BG_FindItemForWeapon(WP_GRENADE_LAUNCHER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_fuel"))
			{
				ritem = BG_FindAmmoForWeapon(WP_GRENADE_LAUNCHER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceFT.integer == 3)
		{
			if(!strcmp(ent->classname, "weapon_ft"))
			{
				ritem = BG_FindItemForWeapon(WP_ROCKET_LAUNCHER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_fuel"))
			{
				ritem = BG_FindAmmoForWeapon(WP_ROCKET_LAUNCHER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceFT.integer == 4)
		{
			if(!strcmp(ent->classname, "weapon_ft"))
			{
				ritem = BG_FindItemForWeapon(WP_LIGHTNING);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_fuel"))
			{
				ritem = BG_FindAmmoForWeapon(WP_LIGHTNING);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceFT.integer == 5)
		{
			if(!strcmp(ent->classname, "weapon_ft"))
			{
				ritem = BG_FindItemForWeapon(WP_RAILGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_fuel"))
			{
				ritem = BG_FindAmmoForWeapon(WP_RAILGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceFT.integer == 6)
		{
			if(!strcmp(ent->classname, "weapon_ft"))
			{
				ritem = BG_FindItemForWeapon(WP_PLASMAGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_fuel"))
			{
				ritem = BG_FindAmmoForWeapon(WP_PLASMAGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceFT.integer == 7)
		{
			if(!strcmp(ent->classname, "weapon_ft"))
			{
				ritem = BG_FindItemForWeapon(WP_BFG);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_fuel"))
			{
				ritem = BG_FindAmmoForWeapon(WP_BFG);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceFT.integer == 8)
		{
			if(!strcmp(ent->classname, "weapon_ft"))
			{
				ritem = BG_FindItemForWeapon(WP_IRAILGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_fuel"))
			{
				ritem = BG_FindAmmoForWeapon(WP_IRAILGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceFT.integer == 9)
		{
			if(!strcmp(ent->classname, "weapon_ft"))
			{
				ritem = BG_FindItemForWeapon(WP_FLAMETHROWER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_fuel"))
			{
				ritem = BG_FindAmmoForWeapon(WP_FLAMETHROWER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceFT.integer == 10)
		{
			if(!strcmp(ent->classname, "weapon_ft"))
			{
				ritem = BG_FindItemForWeapon(WP_SHOTGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_fuel"))
			{
				ritem = BG_FindAmmoForWeapon(WP_SHOTGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		//flame thrower replacement

		//BFG replacement
		if(ReplaceBFG.integer == 1)
		{
			if(!strcmp(ent->classname, "weapon_bfg"))
			{
				ritem = BG_FindItemForWeapon(WP_MACHINEGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_bfg"))
			{
				ritem = BG_FindAmmoForWeapon(WP_MACHINEGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceBFG.integer == 2)
		{
			if(!strcmp(ent->classname, "weapon_bfg"))
			{
				ritem = BG_FindItemForWeapon(WP_GRENADE_LAUNCHER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_bfg"))
			{
				ritem = BG_FindAmmoForWeapon(WP_GRENADE_LAUNCHER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceBFG.integer == 3)
		{
			if(!strcmp(ent->classname, "weapon_bfg"))
			{
				ritem = BG_FindItemForWeapon(WP_ROCKET_LAUNCHER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_bfg"))
			{
				ritem = BG_FindAmmoForWeapon(WP_ROCKET_LAUNCHER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceBFG.integer == 4)
		{
			if(!strcmp(ent->classname, "weapon_bfg"))
			{
				ritem = BG_FindItemForWeapon(WP_LIGHTNING);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_bfg"))
			{
				ritem = BG_FindAmmoForWeapon(WP_LIGHTNING);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceBFG.integer == 5)
		{
			if(!strcmp(ent->classname, "weapon_bfg"))
			{
				ritem = BG_FindItemForWeapon(WP_RAILGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_bfg"))
			{
				ritem = BG_FindAmmoForWeapon(WP_RAILGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceBFG.integer == 6)
		{
			if(!strcmp(ent->classname, "weapon_bfg"))
			{
				ritem = BG_FindItemForWeapon(WP_PLASMAGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_bfg"))
			{
				ritem = BG_FindAmmoForWeapon(WP_PLASMAGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceBFG.integer == 7)
		{
			if(!strcmp(ent->classname, "weapon_bfg"))
			{
				ritem = BG_FindItemForWeapon(WP_BFG);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_bfg"))
			{
				ritem = BG_FindAmmoForWeapon(WP_BFG);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceBFG.integer == 8)
		{
			if(!strcmp(ent->classname, "weapon_bfg"))
			{
				ritem = BG_FindItemForWeapon(WP_IRAILGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_bfg"))
			{
				ritem = BG_FindAmmoForWeapon(WP_IRAILGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceBFG.integer == 9)
		{
			if(!strcmp(ent->classname, "weapon_bfg"))
			{
				ritem = BG_FindItemForWeapon(WP_FLAMETHROWER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_bfg"))
			{
				ritem = BG_FindAmmoForWeapon(WP_FLAMETHROWER);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		if(ReplaceBFG.integer == 10)
		{
			if(!strcmp(ent->classname, "weapon_bfg"))
			{
				ritem = BG_FindItemForWeapon(WP_SHOTGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
			if(!strcmp(ent->classname, "ammo_bfg"))
			{
				ritem = BG_FindAmmoForWeapon(WP_SHOTGUN);
				G_SpawnItem(ent, ritem);
				return qtrue;
			}
		}
		//BFG replacement

		if(!strcmp(item->classname, ent->classname))
		{
			G_SpawnItem(ent, item);
			return qtrue;
		}



	}

	// check normal spawn functions
	for(s = spawns; s->name; s++)
	{
		if(!strcmp(s->name, ent->classname))
		{
			// found it
			s->spawn(ent);
			return qtrue;
		}
	}
	G_Printf("%s doesn't have a spawn function\n", ent->classname);
	return qfalse;
}

/*
=============
G_NewString

Builds a copy of the string, translating \n to real linefeeds
so message texts can be multi-line
=============
*/
char           *G_NewString(const char *string)
{
	char           *newb, *new_p;
	int             i, l;

	l = strlen(string) + 1;

	newb = G_Alloc(l);

	new_p = newb;

	// turn \n into a real linefeed
	for(i = 0; i < l; i++)
	{
		if(string[i] == '\\' && i < l - 1)
		{
			i++;
			if(string[i] == 'n')
			{
				*new_p++ = '\n';
			}
			else
			{
				*new_p++ = '\\';
			}
		}
		else
		{
			*new_p++ = string[i];
		}
	}

	return newb;
}




/*
===============
G_ParseField

Takes a key/value pair and sets the binary values
in a gentity
===============
*/
void G_ParseField(const char *key, const char *value, gentity_t * ent)
{
	field_t        *f;
	byte           *b;
	float           v;
	vec3_t          vec;
	vec3_t          angles;
	matrix_t		rotation;
	int             i;
	char           *p;
	char           *token;

	for(f = fields; f->name; f++)
	{
		if(!Q_stricmp(f->name, key))
		{
			// found it
			b = (byte *) ent;

			switch (f->type)
			{
				case F_LSTRING:
					*(char **)(b + f->ofs) = G_NewString(value);
					break;
				case F_VECTOR:
					sscanf(value, "%f %f %f", &vec[0], &vec[1], &vec[2]);
					((float *)(b + f->ofs))[0] = vec[0];
					((float *)(b + f->ofs))[1] = vec[1];
					((float *)(b + f->ofs))[2] = vec[2];
					break;
				case F_INT:
					*(int *)(b + f->ofs) = atoi(value);
					break;
				case F_FLOAT:
					*(float *)(b + f->ofs) = atof(value);
					break;
				case F_ANGLEHACK:
					v = atof(value);
					((float *)(b + f->ofs))[0] = 0;
					((float *)(b + f->ofs))[1] = v;
					((float *)(b + f->ofs))[2] = 0;
					break;
					
				case F_MOVEDIRHACK:
					v = atof(value);
					angles[0] = 0;
					angles[1] = v;
					angles[2] = 0;
					
					G_SetMovedir(angles, vec);
					
					((float *)(b + f->ofs))[0] = vec[0];
					((float *)(b + f->ofs))[1] = vec[1];
					((float *)(b + f->ofs))[2] = vec[2];
					break;
					
				case F_ROTATIONHACK:
					MatrixIdentity(rotation);
					#if 0
					sscanf(value, "%f %f %f %f %f %f %f %f %f",	&rotation[ 0], &rotation[ 1], &rotation[ 2],
						   										&rotation[ 4], &rotation[ 5], &rotation[ 6],
						   										&rotation[ 8], &rotation[ 9], &rotation[10]);
					#else
					p = (char *)value;
					for(i = 0; i < 9; i++)
					{
						token = Com_Parse(&p);
						rotation[i] = atof(token);
					}
					#endif
					MatrixToAngles(rotation, vec);
					((float *)(b + f->ofs))[0] = vec[0];
					((float *)(b + f->ofs))[1] = vec[1];
					((float *)(b + f->ofs))[2] = vec[2];
					break;
					
				default:
				case F_IGNORE:
					break;
			}
			return;
		}
	}
}




/*
===================
G_SpawnGEntityFromSpawnVars

Spawn an entity and fill in all of the level fields from
level.spawnVars[], then call the class specfic spawn function
===================
*/
void G_SpawnGEntityFromSpawnVars(void)
{
	int             i;
	gentity_t      *ent;
	char           *s, *value, *gametypeName;
	static char    *gametypeNames[] =
		{ "ffa", "tournament", "single", "team", "ctf", "oneflag", "obelisk", "harvester", "teamtournament" };

	// get the next free entity
	ent = G_Spawn();

	for(i = 0; i < level.numSpawnVars; i++)
	{
		G_ParseField(level.spawnVars[i][0], level.spawnVars[i][1], ent);
	}

	// check for "notsingle" flag
	if(g_gametype.integer == GT_SINGLE_PLAYER)
	{
		G_SpawnInt("notsingle", "0", &i);
		if(i)
		{
			G_FreeEntity(ent);
			return;
		}
	}
	// check for "notteam" flag (GT_FFA, GT_TOURNAMENT, GT_SINGLE_PLAYER)
	if(g_gametype.integer >= GT_TEAM)
	{
		G_SpawnInt("notteam", "0", &i);
		if(i)
		{
			G_FreeEntity(ent);
			return;
		}
	}
	else
	{
		G_SpawnInt("notfree", "0", &i);
		if(i)
		{
			G_FreeEntity(ent);
			return;
		}
	}

#ifdef MISSIONPACK
	G_SpawnInt("notta", "0", &i);
	if(i)
	{
		G_FreeEntity(ent);
		return;
	}
#else
	G_SpawnInt("notq3a", "0", &i);
	if(i)
	{
		G_FreeEntity(ent);
		return;
	}
#endif

	if(G_SpawnString("gametype", NULL, &value))
	{
		if(g_gametype.integer >= GT_FFA && g_gametype.integer < GT_MAX_GAME_TYPE)
		{
			gametypeName = gametypeNames[g_gametype.integer];

			s = strstr(value, gametypeName);
			if(!s)
			{
				G_FreeEntity(ent);
				return;
			}
		}
	}

	// move editor origin to pos
	VectorCopy(ent->s.origin, ent->s.pos.trBase);
	VectorCopy(ent->s.origin, ent->r.currentOrigin);

	// if we didn't get a classname, don't bother spawning anything
	if(!G_CallSpawn(ent))
	{
		G_FreeEntity(ent);
	}
}



/*
====================
G_AddSpawnVarToken
====================
*/
char           *G_AddSpawnVarToken(const char *string)
{
	int             l;
	char           *dest;

	l = strlen(string);
	if(level.numSpawnVarChars + l + 1 > MAX_SPAWN_VARS_CHARS)
	{
		G_Error("G_AddSpawnVarToken: MAX_SPAWN_CHARS");
	}

	dest = level.spawnVarChars + level.numSpawnVarChars;
	memcpy(dest, string, l + 1);

	level.numSpawnVarChars += l + 1;

	return dest;
}

/*
====================
G_ParseSpawnVars

Parses a brace bounded set of key / value pairs out of the
level's entity strings into level.spawnVars[]

This does not actually spawn an entity.
====================
*/
qboolean G_ParseSpawnVars(void)
{
	char            keyname[MAX_TOKEN_CHARS];
	char            Com_token[MAX_TOKEN_CHARS];

	level.numSpawnVars = 0;
	level.numSpawnVarChars = 0;

	// parse the opening brace
	if(!trap_GetEntityToken(Com_token, sizeof(Com_token)))
	{
		// end of spawn string
		return qfalse;
	}
	if(Com_token[0] != '{')
	{
		G_Error("G_ParseSpawnVars: found %s when expecting {", Com_token);
	}

	// go through all the key / value pairs
	while(1)
	{
		// parse key
		if(!trap_GetEntityToken(keyname, sizeof(keyname)))
		{
			G_Error("G_ParseSpawnVars: EOF without closing brace");
		}

		if(keyname[0] == '}')
		{
			break;
		}

		// parse value  
		if(!trap_GetEntityToken(Com_token, sizeof(Com_token)))
		{
			G_Error("G_ParseSpawnVars: EOF without closing brace");
		}

		if(Com_token[0] == '}')
		{
			G_Error("G_ParseSpawnVars: closing brace without data");
		}
		if(level.numSpawnVars == MAX_SPAWN_VARS)
		{
			G_Error("G_ParseSpawnVars: MAX_SPAWN_VARS");
		}
		level.spawnVars[level.numSpawnVars][0] = G_AddSpawnVarToken(keyname);
		level.spawnVars[level.numSpawnVars][1] = G_AddSpawnVarToken(Com_token);
		level.numSpawnVars++;
	}

	return qtrue;
}



/*QUAKED worldspawn (0 0 0) ?

Every map should have exactly one worldspawn.
"music"		music wav file
"gravity"	800 is default gravity
"message"	Text to print during connection process
*/
void SP_worldspawn(void)
{
	char           *s;

	G_SpawnString("classname", "", &s);
	if(Q_stricmp(s, "worldspawn"))
	{
		G_Error("SP_worldspawn: The first entity isn't 'worldspawn'");
	}

	// make some data visible to connecting client
	trap_SetConfigstring(CS_GAME_VERSION, GAME_VERSION);

	trap_SetConfigstring(CS_LEVEL_START_TIME, va("%i", level.startTime));

	G_SpawnString("music", "", &s);
	trap_SetConfigstring(CS_MUSIC, s);

	G_SpawnString("message", "", &s);
	trap_SetConfigstring(CS_MESSAGE, s);	// map specific message

	trap_SetConfigstring(CS_MOTD, g_motd.string);	// message of the day

	G_SpawnString("gravity", "800", &s);
	trap_Cvar_Set("g_gravity", s);

	G_SpawnString("enableDust", "0", &s);
	trap_Cvar_Set("g_enableDust", s);

	G_SpawnString("enableBreath", "0", &s);
	trap_Cvar_Set("g_enableBreath", s);

	g_entities[ENTITYNUM_WORLD].s.number = ENTITYNUM_WORLD;
	g_entities[ENTITYNUM_WORLD].classname = "worldspawn";

	// see if we want a warmup time
	trap_SetConfigstring(CS_WARMUP, "");
	if(g_restarted.integer)
	{
		trap_Cvar_Set("g_restarted", "0");
		level.warmupTime = 0;
	}
	else if(g_doWarmup.integer)
	{							// Turn it on
		level.warmupTime = -1;
		trap_SetConfigstring(CS_WARMUP, va("%i", level.warmupTime));
		G_LogPrintf("Warmup:\n");
	}

}


/*
==============
G_SpawnEntitiesFromString

Parses textual entity definitions out of an entstring and spawns gentities.
==============
*/
void G_SpawnEntitiesFromString(void)
{
	// allow calls to G_Spawn*()
	level.spawning = qtrue;
	level.numSpawnVars = 0;

	// the worldspawn is not an actual entity, but it still
	// has a "spawn" function to perform any global setup
	// needed by a level (setting configstrings or cvars, etc)
	if(!G_ParseSpawnVars())
	{
		G_Error("SpawnEntities: no entities");
	}
	SP_worldspawn();

	// parse ents
	while(G_ParseSpawnVars())
	{
		G_SpawnGEntityFromSpawnVars();
	}

	level.spawning = qfalse;	// any future calls to G_Spawn*() will be errors
}
