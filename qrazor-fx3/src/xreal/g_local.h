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
#ifndef G_LOCAL_H
#define G_LOCAL_H

/// includes ===================================================================
// system -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
// xreal --------------------------------------------------------------------
// shared -------------------------------------------------------------------
#include "../shared/x_shared.h"
#include "../shared/x_ode.h"
#include "../shared/x_protocol.h"


// define GAME_INCLUDE so that game.h does not define the
// short, server-visible g_client_c and g_entity_c structures,
// because we define the full size ones in this file
#define	GAME_INCLUDE
#include "g_public.h"


// xg_local.h -- local definitions for the XeaL game module


// the "gameversion" client command will print this plus compile date
#define	GAMEVERSION	"XreaL"




// view pitching times
#define DAMAGE_TIME		0.5
#define	FALL_TIME		0.3


// edict->spawnflags
// these are set with checkboxes on each entity in the map editor
#define	SPAWNFLAG_NOT_EASY		0x00000100
#define	SPAWNFLAG_NOT_MEDIUM		0x00000200
#define	SPAWNFLAG_NOT_HARD		0x00000400
#define	SPAWNFLAG_NOT_DEATHMATCH	0x00000800
#define	SPAWNFLAG_NOT_COOP		0x00001000

// edict->flags
#define	FL_FLY				0x00000001
#define	FL_SWIM				0x00000002	// implied immunity to drowining
#define FL_IMMUNE_LASER			0x00000004
#define	FL_INWATER			0x00000008
#define	FL_ULTRAMANMODE			0x00000010
#define	FL_NOTARGET			0x00000020
#define FL_IMMUNE_SLIME			0x00000040
#define FL_IMMUNE_LAVA			0x00000080
#define	FL_PARTIALGROUND		0x00000100	// not all corners are valid
#define	FL_WATERJUMP			0x00000200	// player jumping out of water
#define	FL_TEAMSLAVE			0x00000400	// not the first on the team
#define FL_NO_KNOCKBACK			0x00000800
#define FL_POWER_ARMOR			0x00001000	// power armor (if any) is active
#define FL_RESPAWN			0x80000000	// used for item respawning


#if 1
#define	FRAMETIME		0.02
#else
#define FRAMETIME		0.1
#endif



enum
{
	DAMAGE_NO,
	DAMAGE_YES,			// will take damage if hit
	DAMAGE_AIM			// auto targeting recognizes this
};

enum
{
	AMMO_BULLETS,
	AMMO_SHELLS,
	AMMO_ROCKETS,
	AMMO_GRENADES,
	AMMO_CELLS,
	AMMO_SLUGS
};


//deadflag
#define DEAD_NO				0
#define DEAD_DYING			1
#define DEAD_DEAD			2
#define DEAD_RESPAWNABLE		3

//range
#define RANGE_MELEE			0
#define RANGE_NEAR			1
#define RANGE_MID			2
#define RANGE_FAR			3

//gib types
#define GIB_ORGANIC			0
#define GIB_METALLIC			1

// armor types
#define ARMOR_NONE			0
#define ARMOR_JACKET			1
#define ARMOR_COMBAT			2
#define ARMOR_BODY			3
#define ARMOR_SHARD			4

// handedness values
#define RIGHT_HANDED			0
#define LEFT_HANDED			1
#define CENTER_HANDED			2


// noise types for PlayerNoise
//#define PNOISE_SELF			0
//#define PNOISE_WEAPON			1
//#define PNOISE_IMPACT			2









// damage flags
#define DAMAGE_RADIUS			0x00000001	// damage was indirect
#define DAMAGE_NO_ARMOR			0x00000002	// armour does not protect from this damage
#define DAMAGE_ENERGY			0x00000004	// damage is from an energy based weapon
#define DAMAGE_NO_KNOCKBACK		0x00000008	// do not affect velocity, just view angles
#define DAMAGE_BULLET			0x00000010 	// damage is from a bullet (used for ricochets)
#define DAMAGE_NO_PROTECTION		0x00000020	// armor, shields, invulnerability, and godmode have no effect

#define DEFAULT_BULLET_HSPREAD			300
#define DEFAULT_BULLET_VSPREAD			500
#define DEFAULT_SHOTGUN_HSPREAD			1000
#define DEFAULT_SHOTGUN_VSPREAD			500
#define DEFAULT_DEATHMATCH_SHOTGUN_COUNT	12
#define DEFAULT_SHOTGUN_COUNT			12
#define DEFAULT_SSHOTGUN_COUNT			20



//
// interface
//
extern	game_import_t	gi;
extern	game_export_t	globals;


//
// forward declarations
//
class g_entity_c;
class g_item_c;
class g_player_c;
class g_target_changelevel_c;



//
// this structure is left intact through an entire game
// it should be initialized at dll load time, and read/written to
// the server.ssv file for savegames
//
class g_game_locals_c
{
public:
	// can't store spawnpoint in level, because
	// it would get overwritten by the savegame restore
	std::string	spawnpoint;		// needed for coop respawns

	// store latched cvars here that we want to get at often
	int		maxclients;
	int		maxentities;

	// cross level triggers
	int		serverflags;

	bool		autosaved;
};



//
// this structure is cleared as each map is entered
// it is read/written to the level.sav file for savegames
//
class g_level_locals_c //: public g_field_interface_c
{
public:			
	int		framenum;
	float		time;

	std::string	level_name;			// the descriptive name (Outer Base, etc)
	std::string	mapname;			// the server name (base1, etc)
	std::string	nextmap;			// go here when fraglimit is hit

	// intermission state
	std::string	changemap;
	float		intermission_time;		// time the intermission was started
	bool		intermission_exit;		// go immediately to the next level
	vec3_c		intermission_origin;
	vec3_c		intermission_angles;
};





extern	g_game_locals_c		game;
extern	g_level_locals_c	level;




// means of death
#define MOD_UNKNOWN			0
#define MOD_BLASTER			1
#define MOD_SHOTGUN			2
#define MOD_SSHOTGUN			3
#define MOD_MACHINEGUN			4
#define MOD_CHAINGUN			5
#define MOD_GRENADE			6
#define MOD_G_SPLASH			7
#define MOD_ROCKET			8
#define MOD_R_SPLASH			9
#define MOD_HYPERBLASTER		10
#define MOD_RAILGUN			11
#define MOD_BFG_LASER			12
#define MOD_BFG_BLAST			13
#define MOD_BFG_EFFECT			14
#define MOD_HANDGRENADE			15
#define MOD_HG_SPLASH			16
#define MOD_WATER			17
#define MOD_SLIME			18
#define MOD_LAVA			19
#define MOD_CRUSH			20
#define MOD_TELEFRAG			21
#define MOD_FALLING			22
#define MOD_SUICIDE			23
#define MOD_HELD_GRENADE		24
#define MOD_EXPLOSIVE			25
#define MOD_BARREL			26
#define MOD_BOMB			27
#define MOD_EXIT			28
#define MOD_SPLASH			29
#define MOD_TARGET_LASER		30
#define MOD_TRIGGER_HURT		31
#define MOD_HIT				32
#define MOD_TARGET_BLASTER		33
#define MOD_FRIENDLY_FIRE	0x8000000

extern	int	meansOfDeath;

extern g_entity_c*			g_world;
extern std::vector<sv_entity_c*>	g_entities;
extern std::vector<g_item_c*>		g_items;

extern d_world_c*		g_ode_world;
extern d_space_c*		g_ode_space;
extern d_joint_group_c*		g_ode_contact_group;


#define random()	((rand () & 0x7fff) / ((float)0x7fff))
#define crandom()	(2.0 * (random() - 0.5))

extern	cvar_t	*maxentities;
extern	cvar_t	*deathmatch;
extern	cvar_t	*coop;
extern	cvar_t	*dmflags;
extern	cvar_t	*skill;
extern	cvar_t	*fraglimit;
extern	cvar_t	*timelimit;
extern	cvar_t	*password;
extern	cvar_t	*spectator_password;
extern	cvar_t	*needpass;
extern	cvar_t	*g_select_empty;
extern	cvar_t	*dedicated;

extern	cvar_t	*filterban;

extern	cvar_t	*g_gravity;
extern	cvar_t	*sv_maxvelocity;

extern	cvar_t	*gun_x, *gun_y, *gun_z;
extern	cvar_t	*sv_rollspeed;
extern	cvar_t	*sv_rollangle;

extern	cvar_t	*run_pitch;
extern	cvar_t	*run_roll;
extern	cvar_t	*bob_up;
extern	cvar_t	*bob_pitch;
extern	cvar_t	*bob_roll;

extern	cvar_t	*sv_cheats;
extern	cvar_t	*maxclients;
extern	cvar_t	*maxspectators;

extern	cvar_t	*flood_msgs;
extern	cvar_t	*flood_persecond;
extern	cvar_t	*flood_waitdelay;

extern	cvar_t	*sv_maplist;


// item spawnflags
#define ITEM_TRIGGER_SPAWN		0x00000001
#define ITEM_NO_TOUCH			0x00000002
// 6 bits reserved for editor flags
// 8 bits used as power cube id bits for coop games
#define DROPPED_ITEM			0x00010000
#define	DROPPED_PLAYER_ITEM		0x00020000
#define ITEM_TARGETS_USED		0x00040000





//
// g_entity.cxx
//
void		G_ShutdownEntities();


//
// g_item.cxx
//
int		G_GetNumForItem(g_item_c *item);
g_item_c*	G_GetItemByNum (int num);

void		G_PrecacheItem (g_item_c *it);
void		G_InitItems();
void		G_ShutdownItems();
void		G_SetItemNames();

g_item_c*	G_FindItem(const std::string &pickup_name);
g_item_c*	G_FindItemByClassname(const std::string &classname);

void 		SetRespawn(g_entity_c *ent, float delay);

void 		G_SpawnItem(g_entity_c **ent, g_item_c *item);
//int 		ArmorIndex(g_entity_c *ent);
//int 		PowerArmorType(g_entity_c *ent);


//
// g_utils.cxx
//
bool		G_KillBox(g_entity_c *ent);
void		G_ProjectSource(const vec3_c &point, const vec3_c &distance, const vec3_c &forward, const vec3_c &right, vec3_c &result);
bool		G_InFront(g_entity_c *self, g_entity_c *other);
bool		G_IsVisible(g_entity_c *self, g_entity_c *other);
g_entity_c*	G_FindByClassName (g_entity_c *from, const std::string &match);
g_entity_c*	G_FindByTargetName (g_entity_c *from, const std::string &match);
g_entity_c*	G_FindByRadius (g_entity_c *from, vec3_t org, float rad);
g_entity_c*	G_PickTarget(const std::string &targetname);
void		G_UseTargets (g_entity_c *ent, g_entity_c *activator);
void		G_SetMovedir(quaternion_c &quat, vec3_c &movedir);


vec_t*		tv (float x, float y, float z);


float 		vectoyaw (vec3_t vec);
void 		vectoangles (vec3_t vec, vec3_t angles);

g_entity_c*	G_GetEntityByNum(int num);
int		G_GetNumForEntity(g_entity_c *ent);


//
// g_combat.cxx
//
bool 		G_OnSameTeam(g_entity_c *ent1, g_entity_c *ent2);
bool 		G_CanDamage(g_entity_c *targ, g_entity_c *inflictor);
void 		G_Killed(g_entity_c *targ, g_entity_c *inflictor, g_entity_c *attacker, int damage, vec3_t point);
void		G_SpawnDamage(int type, vec3_t origin, vec3_t normal, int damage);
void 		G_RadiusDamage(g_entity_c *inflictor, g_entity_c *attacker, float damage, g_entity_c *ignore, float radius, int mod);



//
// g_misc.cxx
//

void 		G_ThrowDebris(g_entity_c *self, const std::string &modelname, float speed, const vec3_c &origin);

//void 		G_ThrowHead (g_entity_c *self, char *gibname, int damage, int type);
//void 		G_ThrowClientHead (g_entity_c *self, int damage);
//void 		G_ThrowGib (g_entity_c *self, char *gibname, int damage, int type);


void 		G_BecomeExplosion1(g_entity_c *self);
void 		G_BecomeExplosion2(g_entity_c *self);


//
// g_svcmds.cxx
//
void		G_ServerCommand();
bool 		SV_FilterPacket(const char *from);



//
// g_hud.cxx
//
void 		G_BeginIntermission(const g_target_changelevel_c *target);
void 		DeathmatchScoreboardMessage();
void		DeathmatchScoreboard(g_entity_c *ent);



//
// g_phys.cxx
//
bool		G_RunThink(g_entity_c *ent);
void 		G_RunEntity(g_entity_c *ent);

void		G_InitDynamics();
void		G_ShutdownDynamics();

void		G_RunDynamics();

trace_t		G_RayTrace(const vec3_c &origin, const vec3_c &dir, vec_t length);

cmodel_c*	G_SetModel(g_entity_c *ent, const std::string &name);
void		G_SetWorldModel(g_entity_c *ent, const std::string &name);


//
// g_main.cxx
//
void 		G_SaveClientData();


//
// g_chase.cxx
//




#endif // G_LOCAL_H

