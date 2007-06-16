/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006 Robert Beckebans <trebor_7@users.sourceforge.net>
Copyright (C) 2007 Jeremy Hughes <Encryption767@msn.com>

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
// bg_misc.c -- both games misc functions, all completely stateless

#include "q_shared.h"
#include "bg_public.h"

/*QUAKED item_***** ( 0 0 0 ) (-16 -16 -16) (16 16 16) suspended
DO NOT USE THIS CLASS, IT JUST HOLDS GENERAL INFORMATION.
The suspended flag will allow items to hang in the air, otherwise they are dropped to the next surface.

If an item is the target of another entity, it will not spawn in until fired.

An item fires all of its targets when it is picked up.  If the toucher can't carry it, the targets won't be fired.

"notfree" if set to 1, don't spawn in free for all games
"notteam" if set to 1, don't spawn in team games
"notsingle" if set to 1, don't spawn in single player games
"wait"	override the default wait before respawning.  -1 = never respawn automatically, which can be used with targeted spawning.
"random" random number of plus or minus seconds varied from the respawn time
"count" override quantity or duration on most items.
*/

gitem_t         bg_itemlist[] = {
	{
	 NULL,
	 NULL,
	 {NULL,
	  NULL,
	  0, 0}
	 ,
/* icon */ NULL,
/* pickup */ NULL,
	 0,
	 0,
	 0,
/* precache */ "",
/* sounds */ ""
	 }
	,							// leave index 0 alone

	//
	// ARMOR
	//

/*QUAKED item_armor_shard (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "item_armor_shard",
	 "sound/misc/ar1_pkup.ogg",
	 {"models/powerups/armor/shard.md3",
	  "models/powerups/armor/shard_sphere.md3",
	  0, 0}
	 ,
/* icon */ "icons/iconr_shard",
/* pickup */ "Armor Shard",
	 0,
	 IT_ARMOR,
	 IA_SHARD,
/* precache */ "",
/* sounds */ ""
	 }
	,

/*QUAKED item_armor_combat (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "item_armor_combat",
	 "sound/misc/ar2_pkup.ogg",
	 {"models/powerups/armor/armor_yel.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/iconr_yellow",
/* pickup */ "Armor",
	 0,
	 IT_ARMOR,
	 IA_YARMOR,
/* precache */ "",
/* sounds */ ""
	 }
	,

/*QUAKED item_armor_body (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "item_armor_body",
	 "sound/misc/ar3_pkup.ogg",
	 {"models/powerups/armor/armor_red.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/iconr_red",
/* pickup */ "Heavy Armor",
	 0,
	 IT_ARMOR,
	 IA_RARMOR,
/* precache */ "",
/* sounds */ ""
	 }
	,

	//
	// health
	//
/*QUAKED item_health_small (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "item_health_small",
	 "sound/items/s_health.ogg",
	 {"models/powerups/health/small_cross.md3",
	  "models/powerups/health/small_sphere.md3",
	  0, 0}
	 ,
/* icon */ "icons/iconh_green",
/* pickup */ "5 Health",
	 5,
	 IT_HEALTH,
	 IH_5HEALTH,
/* precache */ "",
/* sounds */ ""
	 }
	,

/*QUAKED item_health (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "item_health",
	 "sound/items/n_health.ogg",
	 {"models/powerups/health/medium_cross.md3",
	  "models/powerups/health/medium_sphere.md3",
	  0, 0}
	 ,
/* icon */ "icons/iconh_yellow",
/* pickup */ "20 Health",
	 20,
	 IT_HEALTH,
	 IH_20HEALTH,
/* precache */ "",
/* sounds */ ""
	 }
	,

/*QUAKED item_health_large (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "item_health_large",
	 "sound/items/l_health.ogg",
	 {"models/powerups/health/large_cross.md3",
	  "models/powerups/health/large_sphere.md3",
	  0, 0}
	 ,
/* icon */ "icons/iconh_red",
/* pickup */ "40 Health",
	 40,
	 IT_HEALTH,
	 IH_40HEALTH,
/* precache */ "",
/* sounds */ ""
	 }
	,

/*QUAKED item_health_mega (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "item_health_mega",
	 "sound/items/m_health.ogg",
	 {"models/powerups/health/mega_cross.md3",
	  "models/powerups/health/mega_sphere.md3",
	  0, 0}
	 ,
/* icon */ "icons/iconh_mega",
/* pickup */ "Mega Health",
	 100,
	 IT_HEALTH,
	 IH_MHEALTH,
/* precache */ "",
/* sounds */ ""
	 }
	,


	//
	// WEAPONS 
	//

/*QUAKED weapon_gauntlet (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "weapon_gauntlet",
	 "sound/misc/w_pkup.ogg",
	 {"models/weapons/gauntlet/gauntlet.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/iconw_gauntlet",
/* pickup */ "Gauntlet",
	 0,
	 IT_WEAPON,
	 WP_GAUNTLET,
/* precache */ "",
/* sounds */ ""
	 }
	,

/*QUAKED weapon_shotgun (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "weapon_shotgun",
	 "sound/misc/w_pkup.ogg",
	 {"models/weapons/shotgun/shotgun.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/iconw_shotgun",
/* pickup */ "Shotgun",
	 10,
	 IT_WEAPON,
	 WP_SHOTGUN,
/* precache */ "",
/* sounds */ ""
	 }
	,

/*QUAKED weapon_machinegun (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "weapon_machinegun",
	 "sound/misc/w_pkup.ogg",
	 {"models/weapons/machinegun/machinegun.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/iconw_machinegun",
/* pickup */ "Machinegun",
	 75,
	 IT_WEAPON,
	 WP_MACHINEGUN,
/* precache */ "",
/* sounds */ ""
	 }
	,

/*QUAKED weapon_grenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "weapon_grenadelauncher",
	 "sound/misc/w_pkup.ogg",
	 {"models/weapons2/grenadel/grenadel.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/iconw_grenade",
/* pickup */ "Grenade Launcher",
	 10,
	 IT_WEAPON,
	 WP_GRENADE_LAUNCHER,
/* precache */ "",
/* sounds */ "sound/weapons/grenade/hgrenb1a.wav sound/weapons/grenade/hgrenb2a.wav"
	 }
	,

/*QUAKED weapon_rocketlauncher (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "weapon_rocketlauncher",
	 "sound/misc/w_pkup.ogg",
	 {"models/weapons/rocketl/rocketl.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/iconw_rocket",
/* pickup */ "Rocket Launcher",
	 12,
	 IT_WEAPON,
	 WP_ROCKET_LAUNCHER,
/* precache */ "",
/* sounds */ ""
	 }
	,

/*QUAKED weapon_lightning (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "weapon_lightning",
	 "sound/misc/w_pkup.ogg",
	 {"models/weapons/lightning/lightning.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/iconw_lightning",
/* pickup */ "Lightning Gun",
	 100,
	 IT_WEAPON,
	 WP_LIGHTNING,
/* precache */ "",
/* sounds */ ""
	 }
	,

/*QUAKED weapon_railgun (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "weapon_railgun",
	 "sound/misc/w_pkup.ogg",
	 {"models/weapons2/railgun/railgun.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/iconw_railgun",
/* pickup */ "Railgun",
	 10,
	 IT_WEAPON,
	 WP_RAILGUN,
/* precache */ "",
/* sounds */ ""
	 }
	,

/*QUAKED weapon_irailgun (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "weapon_irailgun",
	 "sound/misc/w_pkup.wav",
	 {"models/weapons2/irailgun/irailgun.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/iconw_irailgun",
/* pickup */ "Particle Accelerator",
	 10,
	 IT_WEAPON,
	 WP_IRAILGUN,
/* precache */ "",
/* sounds */ ""
	 }
	,

/*QUAKED weapon_plasmagun (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "weapon_plasmagun",
	 "sound/misc/w_pkup.ogg",
	 {"models/weapons2/plasma/plasma.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/iconw_plasma",
/* pickup */ "Plasma Gun",
	 50,
	 IT_WEAPON,
	 WP_PLASMAGUN,
/* precache */ "",
/* sounds */ ""
	 }
	,

/*QUAKED weapon_bfg (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "weapon_bfg",
	 "sound/misc/w_pkup.ogg",
	 {"models/weapons2/bfg/bfg.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/iconw_bfg",
/* pickup */ "BFG10K",
	 4,
	 IT_WEAPON,
	 WP_BFG,
/* precache */ "",
/* sounds */ ""
	 }
	,

/*QUAKED weapon_flamethrower (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "weapon_ft",
	 "sound/misc/w_pkup.wav",
	 {"models/weapons2/ft/ft.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/iconw_ft",
/* pickup */ "FlameThrower",
	 25,
	 IT_WEAPON,
	 WP_FLAMETHROWER,
/* precache */ "",
/* sounds */ ""
	 }
	,

/*QUAKED weapon_grapplinghook (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "weapon_grapplinghook",
	 "sound/misc/w_pkup.ogg",
	 {"models/weapons2/grapple/grapple.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/iconw_grapple",
/* pickup */ "Grappling Hook",
	 0,
	 IT_WEAPON,
	 WP_GRAPPLING_HOOK,
/* precache */ "",
/* sounds */ ""
	 }
	,

	//
	// AMMO ITEMS
	//

/*QUAKED ammo_shells (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "ammo_shells",
	 "sound/misc/am_pkup.ogg",
	 {"models/powerups/ammo/shotgunam.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/icona_shotgun",
/* pickup */ "Shells",
	 10,
	 IT_AMMO,
	 WP_SHOTGUN,
/* precache */ "",
/* sounds */ ""
	 }
	,

/*QUAKED ammo_bullets (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "ammo_bullets",
	 "sound/misc/am_pkup.ogg",
	 {"models/powerups/ammo/machinegunam.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/icona_machinegun",
/* pickup */ "Bullets",
	 100,
	 IT_AMMO,
	 WP_MACHINEGUN,
/* precache */ "",
/* sounds */ ""
	 }
	,

/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "ammo_grenades",
	 "sound/misc/am_pkup.ogg",
	 {"models/powerups/ammo/grenadeam.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/icona_grenade",
/* pickup */ "Grenades",
	 5,
	 IT_AMMO,
	 WP_GRENADE_LAUNCHER,
/* precache */ "",
/* sounds */ ""
	 }
	,

/*QUAKED ammo_cells (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "ammo_cells",
	 "sound/misc/am_pkup.ogg",
	 {"models/powerups/ammo/plasmaam.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/icona_plasma",
/* pickup */ "Cells",
	 30,
	 IT_AMMO,
	 WP_PLASMAGUN,
/* precache */ "",
/* sounds */ ""
	 }
	,

/*QUAKED ammo_lightning (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "ammo_lightning",
	 "sound/misc/am_pkup.ogg",
	 {"models/powerups/ammo/lightningam.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/icona_lightning",
/* pickup */ "Lightning",
	 60,
	 IT_AMMO,
	 WP_LIGHTNING,
/* precache */ "",
/* sounds */ ""
	 }
	,

/*QUAKED ammo_rockets (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "ammo_rockets",
	 "sound/misc/am_pkup.ogg",
	 {"models/powerups/ammo/rocketam.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/icona_rocket",
/* pickup */ "Rockets",
	 5,
	 IT_AMMO,
	 WP_ROCKET_LAUNCHER,
/* precache */ "",
/* sounds */ ""
	 }
	,

/*QUAKED ammo_slugs (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "ammo_slugs",
	 "sound/misc/am_pkup.ogg",
	 {"models/powerups/ammo/railgunam.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/icona_railgun",
/* pickup */ "Slugs",
	 5,
	 IT_AMMO,
	 WP_RAILGUN,
/* precache */ "",
/* sounds */ ""
	 }
	,

/*QUAKED ammo_fuel (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "ammo_fuel",
	 "sound/misc/am_pkup.wav",
	 {"models/powerups/ammo/bfgam.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/icona_ft",
/* pickup */ "Fuel",
	 15,
	 IT_AMMO,
	 WP_FLAMETHROWER,
/* precache */ "",
/* sounds */ ""
	 }
	,

/*QUAKED ammo_slugs (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "ammo_islugs",
	 "sound/misc/am_pkup.wav",
	 {"models/powerups/ammo/railgunam.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/icona_irailgun",
/* pickup */ "Charge Slugs",
	 5,
	 IT_AMMO,
	 WP_IRAILGUN,
/* precache */ "",
/* sounds */ ""
	 }
	,

/*QUAKED ammo_bfg (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "ammo_bfg",
	 "sound/misc/am_pkup.ogg",
	 {"models/powerups/ammo/bfgam.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/icona_bfg",
/* pickup */ "Bfg Ammo",
	 2,
	 IT_AMMO,
	 WP_BFG,
/* precache */ "",
/* sounds */ ""
	 }
	,

	//
	// HOLDABLE ITEMS
	//
/*QUAKED holdable_teleporter (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "holdable_teleporter",
	 "sound/items/holdable.wav",
	 {"models/powerups/holdable/teleporter.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/teleporter",
/* pickup */ "Personal Teleporter",
	 60,
	 IT_HOLDABLE,
	 HI_TELEPORTER,
/* precache */ "",
/* sounds */ ""
	 }
	,
/*QUAKED holdable_medkit (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "holdable_medkit",
	 "sound/items/holdable.wav",
	 {
	  "models/powerups/holdable/medkit.md3",
	  "models/powerups/holdable/medkit_sphere.md3",
	  0, 0}
	 ,
/* icon */ "icons/medkit",
/* pickup */ "Medkit",
	 60,
	 IT_HOLDABLE,
	 HI_MEDKIT,
/* precache */ "",
/* sounds */ "sound/items/use_medkit.wav"
	 }
	,

	//
	// POWERUP ITEMS
	//
/*QUAKED item_quad (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "item_quad",
	 "sound/items/quaddamage.wav",
	 {"models/powerups/instant/quad.md3",
	  "models/powerups/instant/quad_ring.md3",
	  0, 0}
	 ,
/* icon */ "icons/quad",
/* pickup */ "Quad Damage",
	 30,
	 IT_POWERUP,
	 PW_QUAD,
/* precache */ "",
/* sounds */ "sound/items/damage2.wav sound/items/damage3.wav"
	 }
	,

	{
	 "item_enviro",
	 "sound/items/protect.wav",
	 {"models/powerups/instant/enviro.md3",
	  "models/powerups/instant/enviro_ring.md3",
	  0, 0}
	 ,
/* icon */ "icons/envirosuit",
/* pickup */ "Battle Suit",
	 30,
	 IT_POWERUP,
	 PW_BATTLESUIT,
/* precache */ "",
/* sounds */ "sound/items/airout.wav sound/items/protect3.wav"
	 }
	,

	{
	 "item_spawnp",
	 "sound/items/protect.wav",
	 {"models/powerups/instant/enviro.md3",
	  "models/powerups/instant/enviro_ring.md3",
	  0, 0}
	 ,
/* icon */ "icons/envirosuit",
/* pickup */ "Spawn Protection",
	 1,
	 IT_POWERUP,
	 PW_SPAWNPROT,
/* precache */ "",
/* sounds */ "sound/items/airout.wav sound/items/protect3.wav"
	 }
	,

/*QUAKED item_haste (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "item_haste",
	 "sound/items/haste.wav",
	 {"models/powerups/instant/haste.md3",
	  "models/powerups/instant/haste_ring.md3",
	  0, 0}
	 ,
/* icon */ "icons/haste",
/* pickup */ "Speed",
	 30,
	 IT_POWERUP,
	 PW_HASTE,
/* precache */ "",
/* sounds */ ""
	 }
	,

/*QUAKED item_invis (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "item_invis",
	 "sound/items/invisibility.wav",
	 {"models/powerups/instant/invis.md3",
	  "models/powerups/instant/invis_ring.md3",
	  0, 0}
	 ,
/* icon */ "icons/invis",
/* pickup */ "Invisibility",
	 30,
	 IT_POWERUP,
	 PW_INVIS,
/* precache */ "",
/* sounds */ ""
	 }
	,

/*QUAKED item_regen (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "item_regen",
	 "sound/items/regeneration.wav",
	 {"models/powerups/instant/regen.md3",
	  "models/powerups/instant/regen_ring.md3",
	  0, 0}
	 ,
/* icon */ "icons/regen",
/* pickup */ "Regeneration",
	 30,
	 IT_POWERUP,
	 PW_REGEN,
/* precache */ "",
/* sounds */ "sound/items/regen.wav"
	 }
	,

/*QUAKED item_flight (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "item_flight",
	 "sound/items/flight.wav",
	 {"models/powerups/instant/flight.md3",
	  "models/powerups/instant/flight_ring.md3",
	  0, 0}
	 ,
/* icon */ "icons/flight",
/* pickup */ "Flight",
	 60,
	 IT_POWERUP,
	 PW_FLIGHT,
/* precache */ "",
/* sounds */ "sound/items/flight.wav"
	 }
	,

/*QUAKED team_CTF_redflag (1 0 0) (-16 -16 -16) (16 16 16)
Only in CTF games
*/
	{
	 "team_CTF_redflag",
	 NULL,
	 {"models/flags/r_flag.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/iconf_red1",
/* pickup */ "Red Flag",
	 0,
	 IT_TEAM,
	 PW_REDFLAG,
/* precache */ "",
/* sounds */ ""
	 }
	,

/*QUAKED team_CTF_blueflag (0 0 1) (-16 -16 -16) (16 16 16)
Only in CTF games
*/
	{
	 "team_CTF_blueflag",
	 NULL,
	 {"models/flags/b_flag.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/iconf_blu1",
/* pickup */ "Blue Flag",
	 0,
	 IT_TEAM,
	 PW_BLUEFLAG,
/* precache */ "",
/* sounds */ ""
	 }
	,

#ifdef MISSIONPACK
/*QUAKED holdable_kamikaze (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "holdable_kamikaze",
	 "sound/items/holdable.wav",
	 {"models/powerups/kamikazi.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/kamikaze",
/* pickup */ "Kamikaze",
	 10,
	 IT_HOLDABLE,
	 HI_KAMIKAZE,
/* precache */ "",
/* sounds */ "sound/items/kamikazerespawn.wav"
	 }
	,

/*QUAKED holdable_portal (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "holdable_portal",
	 "sound/items/holdable.wav",
	 {"models/powerups/holdable/porter.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/portal",
/* pickup */ "Portal",
	 60,
	 IT_HOLDABLE,
	 HI_PORTAL,
/* precache */ "",
/* sounds */ ""
	 }
	,

/*QUAKED holdable_invulnerability (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "holdable_invulnerability",
	 "sound/items/holdable.wav",
	 {"models/powerups/holdable/invulnerability.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/invulnerability",
/* pickup */ "Invulnerability",
	 10,
	 IT_HOLDABLE,
	 HI_INVULNERABILITY,
/* precache */ "",
/* sounds */ ""
	 }
	,

/*QUAKED ammo_nails (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "ammo_nails",
	 "sound/misc/am_pkup.ogg",
	 {"models/powerups/ammo/nailgunam.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/icona_nailgun",
/* pickup */ "Nails",
	 20,
	 IT_AMMO,
	 WP_NAILGUN,
/* precache */ "",
/* sounds */ ""
	 }
	,

/*QUAKED ammo_mines (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "ammo_mines",
	 "sound/misc/am_pkup.ogg",
	 {"models/powerups/ammo/proxmineam.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/icona_proxlauncher",
/* pickup */ "Proximity Mines",
	 10,
	 IT_AMMO,
	 WP_PROX_LAUNCHER,
/* precache */ "",
/* sounds */ ""
	 }
	,

/*QUAKED ammo_belt (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "ammo_belt",
	 "sound/misc/am_pkup.ogg",
	 {"models/powerups/ammo/chaingunam.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/icona_chaingun",
/* pickup */ "Chaingun Belt",
	 100,
	 IT_AMMO,
	 WP_CHAINGUN,
/* precache */ "",
/* sounds */ ""
	 }
	,

	//
	// PERSISTANT POWERUP ITEMS
	//
/*QUAKED item_scout (.3 .3 1) (-16 -16 -16) (16 16 16) suspended redTeam blueTeam
*/
	{
	 "item_scout",
	 "sound/items/scout.wav",
	 {"models/powerups/scout.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/scout",
/* pickup */ "Scout",
	 30,
	 IT_PERSISTANT_POWERUP,
	 PW_SCOUT,
/* precache */ "",
/* sounds */ ""
	 }
	,

/*QUAKED item_guard (.3 .3 1) (-16 -16 -16) (16 16 16) suspended redTeam blueTeam
*/
	{
	 "item_guard",
	 "sound/items/guard.wav",
	 {"models/powerups/guard.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/guard",
/* pickup */ "Guard",
	 30,
	 IT_PERSISTANT_POWERUP,
	 PW_GUARD,
/* precache */ "",
/* sounds */ ""
	 }
	,

/*QUAKED item_doubler (.3 .3 1) (-16 -16 -16) (16 16 16) suspended redTeam blueTeam
*/
	{
	 "item_doubler",
	 "sound/items/doubler.wav",
	 {"models/powerups/doubler.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/doubler",
/* pickup */ "Doubler",
	 30,
	 IT_PERSISTANT_POWERUP,
	 PW_DOUBLER,
/* precache */ "",
/* sounds */ ""
	 }
	,

/*QUAKED item_doubler (.3 .3 1) (-16 -16 -16) (16 16 16) suspended redTeam blueTeam
*/
	{
	 "item_ammoregen",
	 "sound/items/ammoregen.wav",
	 {"models/powerups/ammo.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/ammo_regen",
/* pickup */ "Ammo Regen",
	 30,
	 IT_PERSISTANT_POWERUP,
	 PW_AMMOREGEN,
/* precache */ "",
/* sounds */ ""
	 }
	,

	{
	 "item_redcube",
	 "sound/misc/am_pkup.ogg",
	 {"models/powerups/orb/r_orb.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/iconh_rorb",
/* pickup */ "Red Cube",
	 0,
	 IT_TEAM,
	 0,
/* precache */ "",
/* sounds */ ""
	 }
	,

	{
	 "item_bluecube",
	 "sound/misc/am_pkup.ogg",
	 {"models/powerups/orb/b_orb.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/iconh_borb",
/* pickup */ "Blue Cube",
	 0,
	 IT_TEAM,
	 0,
/* precache */ "",
/* sounds */ ""
	 }
	,
/*QUAKED weapon_nailgun (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "weapon_nailgun",
	 "sound/misc/w_pkup.ogg",
	 {"models/weapons/nailgun/nailgun.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/iconw_nailgun",
/* pickup */ "Nailgun",
	 10,
	 IT_WEAPON,
	 WP_NAILGUN,
/* precache */ "",
/* sounds */ ""
	 }
	,

/*QUAKED weapon_prox_launcher (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "weapon_prox_launcher",
	 "sound/misc/w_pkup.ogg",
	 {"models/weapons/proxmine/proxmine.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/iconw_proxlauncher",
/* pickup */ "Prox Launcher",
	 5,
	 IT_WEAPON,
	 WP_PROX_LAUNCHER,
/* precache */ "",
/* sounds */ "sound/weapons/proxmine/wstbtick.wav "
	 "sound/weapons/proxmine/wstbactv.wav "
	 "sound/weapons/proxmine/wstbimpl.wav "
	 "sound/weapons/proxmine/wstbimpm.wav " "sound/weapons/proxmine/wstbimpd.wav " "sound/weapons/proxmine/wstbactv.wav"}
	,

/*QUAKED weapon_chaingun (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
	 "weapon_chaingun",
	 "sound/misc/w_pkup.ogg",
	 {"models/weapons/vulcan/vulcan.md3",
	  0, 0, 0}
	 ,
/* icon */ "icons/iconw_chaingun",
/* pickup */ "Chaingun",
	 80,
	 IT_WEAPON,
	 WP_CHAINGUN,
/* precache */ "",
/* sounds */ "sound/weapons/vulcan/wvulwind.wav"
	 }
	,
#endif

	// end of list marker
	{NULL}
};

int             bg_numItems = sizeof(bg_itemlist) / sizeof(bg_itemlist[0]) - 1;


/*
==============
BG_FindItemForPowerup
==============
*/
gitem_t        *BG_FindItemForPowerup(powerup_t pw)
{
	int             i;

	for(i = 0; i < bg_numItems; i++)
	{
		if((bg_itemlist[i].giType == IT_POWERUP ||
			bg_itemlist[i].giType == IT_TEAM || bg_itemlist[i].giType == IT_PERSISTANT_POWERUP) && bg_itemlist[i].giTag == pw)
		{
			return &bg_itemlist[i];
		}
	}

	return NULL;
}


/*
==============
BG_FindItemForHoldable
==============
*/
gitem_t        *BG_FindItemForHoldable(holdable_t pw)
{
	int             i;

	for(i = 0; i < bg_numItems; i++)
	{
		if(bg_itemlist[i].giType == IT_HOLDABLE && bg_itemlist[i].giTag == pw)
		{
			return &bg_itemlist[i];
		}
	}

	Com_Error(ERR_DROP, "HoldableItem not found");

	return NULL;
}


/*
===============
BG_FindItemForWeapon

===============
*/
gitem_t        *BG_FindItemForWeapon(weapon_t weapon)
{
	gitem_t        *it;

	for(it = bg_itemlist + 1; it->classname; it++)
	{
		if(it->giType == IT_WEAPON && it->giTag == weapon)
		{
			return it;
		}
	}

	Com_Error(ERR_DROP, "Couldn't find item for weapon %i", weapon);
	return NULL;
}

/*
===============
BG_FindAmmoForWeapon

===============
*/
gitem_t        *BG_FindAmmoForWeapon(weapon_t weapon)
{
	gitem_t        *it;

	for(it = bg_itemlist + 1; it->classname; it++)
	{
		if(it->giType == IT_AMMO && it->giTag == weapon)
		{
			return it;
		}
	}

	Com_Error(ERR_DROP, "Couldn't find ammo for weapon %i", weapon);
	return NULL;
}

/*
===============
BG_FindItem

===============
*/
gitem_t        *BG_FindItem(const char *pickupName)
{
	gitem_t        *it;

	for(it = bg_itemlist + 1; it->classname; it++)
	{
		if(!Q_stricmp(it->pickup_name, pickupName))
			return it;
	}

	return NULL;
}

/*
============
BG_PlayerTouchesItem

Items can be picked up without actually touching their physical bounds to make
grabbing them easier
============
*/
qboolean BG_PlayerTouchesItem(playerState_t * ps, entityState_t * item, int atTime)
{
	vec3_t          origin;

	BG_EvaluateTrajectory(&item->pos, atTime, origin);

	// we are ignoring ducked differences here
	if(ps->origin[0] - origin[0] > 44
	   || ps->origin[0] - origin[0] < -50
	   || ps->origin[1] - origin[1] > 36
	   || ps->origin[1] - origin[1] < -36 || ps->origin[2] - origin[2] > 36 || ps->origin[2] - origin[2] < -36)
	{
		return qfalse;
	}

	return qtrue;
}



/*
================
BG_CanItemBeGrabbed

Returns false if the item should not be picked up.
This needs to be the same for client side prediction and server use.
================
*/
qboolean BG_CanItemBeGrabbed(int gametype, const entityState_t * ent, const playerState_t * ps)
{
	gitem_t        *item;

#ifdef MISSIONPACK
	int             upperBound;
#endif

	if(ent->modelindex < 1 || ent->modelindex >= bg_numItems)
	{
		Com_Error(ERR_DROP, "BG_CanItemBeGrabbed: index out of range");
	}

	item = &bg_itemlist[ent->modelindex];

	switch (item->giType)
	{
		case IT_WEAPON:
			return qtrue;		// weapons are always picked up

		case IT_AMMO:

			if(ps->ammo[ /*item->giTag */ WP_RAILGUN] && item->giTag == WP_RAILGUN)
			{
				if(ps->ammo[ /*item->giTag */ WP_RAILGUN] >= 40)
				{
					return qfalse;
				}
			}

			if(ps->ammo[ /*item->giTag */ WP_FLAMETHROWER] && item->giTag == WP_FLAMETHROWER)
			{
				if(ps->ammo[ /*item->giTag */ WP_FLAMETHROWER] >= 80)
				{
					return qfalse;
				}
			}
			if(ps->ammo[ /*item->giTag */ WP_IRAILGUN] && item->giTag == WP_IRAILGUN)
			{
				if(ps->ammo[ /*item->giTag */ WP_IRAILGUN] >= 40)
				{
					return qfalse;
				}
			}
			if(ps->ammo[WP_ROCKET_LAUNCHER] && item->giTag == WP_ROCKET_LAUNCHER)
			{
				if(ps->ammo[ /*item->giTag */ WP_ROCKET_LAUNCHER] >= 35)
				{
					return qfalse;
				}
			}
			if(ps->ammo[WP_MACHINEGUN] && item->giTag == WP_MACHINEGUN)
			{
				if(ps->ammo[WP_MACHINEGUN] >= 320)
				{
					return qfalse;
				}
			}
			if(ps->ammo[WP_PLASMAGUN] && item->giTag == WP_PLASMAGUN)
			{
				if(ps->ammo[WP_PLASMAGUN] >= 240)
				{
					return qfalse;
				}
			}
			if(ps->ammo[WP_SHOTGUN] && item->giTag == WP_SHOTGUN)
			{
				if(ps->ammo[WP_SHOTGUN] >= 60)
				{
					return qfalse;
				}
			}
			if(ps->ammo[WP_BFG] && item->giTag == WP_BFG)
			{
				if(ps->ammo[WP_BFG] >= 6)
				{
					return qfalse;
				}
			}
			if(ps->ammo[WP_GRENADE_LAUNCHER] && item->giTag == WP_GRENADE_LAUNCHER)
			{
				if(ps->ammo[WP_GRENADE_LAUNCHER] >= 60)
				{
					return qfalse;
				}
			}
			if(ps->ammo[WP_LIGHTNING] && item->giTag == WP_LIGHTNING)
			{
				if(ps->ammo[WP_LIGHTNING] >= 260)
				{
					return qfalse;
				}
			}
			return qtrue;

		case IT_ARMOR:

			if(ps->stats[STAT_ARMOR] >= ps->stats[STAT_MAX_HEALTH] * 2)
			{
				return qfalse;
			}
			return qtrue;

		case IT_HEALTH:
			// small and mega healths will go over the max, otherwise
			// don't pick up if already at max
#ifdef MISSIONPACK
			if(bg_itemlist[ps->stats[STAT_PERSISTANT_POWERUP]].giTag == PW_GUARD)
			{
				upperBound = ps->stats[STAT_MAX_HEALTH];
			}
			else
#endif
			if(item->quantity == 5 || item->quantity == 100)
			{
				if(ps->stats[STAT_HEALTH] >= ps->stats[STAT_MAX_HEALTH] * 2)
				{
					return qfalse;
				}
				return qtrue;
			}

			if(ps->stats[STAT_HEALTH] >= ps->stats[STAT_MAX_HEALTH])
			{
				return qfalse;
			}
			return qtrue;

		case IT_POWERUP:
			return qtrue;		// powerups are always picked up

#ifdef MISSIONPACK
		case IT_PERSISTANT_POWERUP:
			// can only hold one item at a time
			if(ps->stats[STAT_PERSISTANT_POWERUP])
			{
				return qfalse;
			}

			// check team only
			if((ent->generic1 & 2) && (ps->persistant[PERS_TEAM] != TEAM_RED))
			{
				return qfalse;
			}
			if((ent->generic1 & 4) && (ps->persistant[PERS_TEAM] != TEAM_BLUE))
			{
				return qfalse;
			}

			return qtrue;
#endif

		case IT_TEAM:			// team items, such as flags
#ifdef MISSIONPACK
			if(gametype == GT_1FCTF)
			{
				// neutral flag can always be picked up
				if(item->giTag == PW_NEUTRALFLAG)
				{
					return qtrue;
				}
				if(ps->persistant[PERS_TEAM] == TEAM_RED)
				{
					if(item->giTag == PW_BLUEFLAG && ps->powerups[PW_NEUTRALFLAG])
					{
						return qtrue;
					}
				}
				else if(ps->persistant[PERS_TEAM] == TEAM_BLUE)
				{
					if(item->giTag == PW_REDFLAG && ps->powerups[PW_NEUTRALFLAG])
					{
						return qtrue;
					}
				}
			}
#endif
			if(gametype == GT_CTF)
			{
				// ent->modelindex2 is non-zero on items if they are dropped
				// we need to know this because we can pick up our dropped flag (and return it)
				// but we can't pick up our flag at base
				if(ps->persistant[PERS_TEAM] == TEAM_RED)
				{
					if(item->giTag == PW_BLUEFLAG ||
					   (item->giTag == PW_REDFLAG && ent->modelindex2) ||
					   (item->giTag == PW_REDFLAG && ps->powerups[PW_BLUEFLAG]))
						return qtrue;
				}
				else if(ps->persistant[PERS_TEAM] == TEAM_BLUE)
				{
					if(item->giTag == PW_REDFLAG ||
					   (item->giTag == PW_BLUEFLAG && ent->modelindex2) ||
					   (item->giTag == PW_BLUEFLAG && ps->powerups[PW_REDFLAG]))
						return qtrue;
				}
				//NT - you can pick up your own flag
				if(ps->persistant[PERS_TEAM] == TEAM_RED)
				{
					if(ps->powerups[PW_REDFLAG] && item->giTag == PW_REDFLAG)
						return qtrue;
				}
				else if(ps->persistant[PERS_TEAM] == TEAM_BLUE)
				{
					if(ps->powerups[PW_BLUEFLAG] && item->giTag == PW_BLUEFLAG)
						return qtrue;
				}
			}

#ifdef MISSIONPACK
			if(gametype == GT_HARVESTER)
			{
				return qtrue;
			}
#endif
			return qfalse;

		case IT_HOLDABLE:
			// can only hold one item at a time
			if(ps->stats[STAT_HOLDABLE_ITEM])
			{
				return qfalse;
			}
			return qtrue;

		case IT_BAD:
			Com_Error(ERR_DROP, "BG_CanItemBeGrabbed: IT_BAD");
		default:
#ifndef Q3_VM
#ifndef NDEBUG					// bk0001204
			Com_Printf("BG_CanItemBeGrabbed: unknown enum %d\n", item->giType);
#endif
#endif
			break;
	}

	return qfalse;
}

//======================================================================

/*
================
BG_EvaluateTrajectory

================
*/
void BG_EvaluateTrajectory(const trajectory_t * tr, int atTime, vec3_t result)
{
	float           deltaTime;
	float           phase;
	vec3_t          v;

	switch (tr->trType)
	{
		case TR_STATIONARY:
		case TR_INTERPOLATE:
			VectorCopy(tr->trBase, result);
			break;
		case TR_LINEAR:
			deltaTime = (atTime - tr->trTime) * 0.001;	// milliseconds to seconds
			VectorMA(tr->trBase, deltaTime, tr->trDelta, result);
			break;
		case TR_SINE:
			deltaTime = (atTime - tr->trTime) / (float)tr->trDuration;
			phase = sin(deltaTime * M_PI * 2);
			VectorMA(tr->trBase, phase, tr->trDelta, result);
			break;
		case TR_LINEAR_STOP:
			if(atTime > tr->trTime + tr->trDuration)
			{
				atTime = tr->trTime + tr->trDuration;
			}
			deltaTime = (atTime - tr->trTime) * 0.001;	// milliseconds to seconds
			if(deltaTime < 0)
			{
				deltaTime = 0;
			}
			VectorMA(tr->trBase, deltaTime, tr->trDelta, result);
			break;
		case TR_GRAVITY_LOW:
			deltaTime = (atTime - tr->trTime) * 0.001;	// milliseconds to seconds
			VectorMA(tr->trBase, deltaTime, tr->trDelta, result);
			result[2] -= 0.5 * (DEFAULT_GRAVITY * 0.3) * deltaTime * deltaTime;	// FIXME: local gravity...
			break;
		case TR_GRAVITY:
			deltaTime = (atTime - tr->trTime) * 0.001;	// milliseconds to seconds
			VectorMA(tr->trBase, deltaTime, tr->trDelta, result);
			result[2] -= 0.5 * DEFAULT_GRAVITY * deltaTime * deltaTime;	// FIXME: local gravity...
			break;
		case TR_GRAVITY_FLOAT:
			deltaTime = (atTime - tr->trTime) * 0.001;	// milliseconds to seconds
			VectorMA(tr->trBase, deltaTime, tr->trDelta, result);
			result[2] -= 0.5 * (DEFAULT_GRAVITY * 0.2) * deltaTime;
			break;
		case TR_ACCELERATE:	// trDelta is the ultimate speed
			if(atTime > tr->trTime + tr->trDuration)
			{
				atTime = tr->trTime + tr->trDuration;
			}
			deltaTime = (atTime - tr->trTime) * 0.001;	// milliseconds to seconds
			// phase is the acceleration constant
			phase = VectorLength(tr->trDelta) / (tr->trDuration * 0.001);
			// trDelta at least gives us the acceleration direction
			VectorNormalize2(tr->trDelta, result);
			// get distance travelled at current time
			VectorMA(tr->trBase, phase * 0.5 * deltaTime * deltaTime, result, result);
			break;
		case TR_DECCELERATE:	// trDelta is the starting speed
			if(atTime > tr->trTime + tr->trDuration)
			{
				atTime = tr->trTime + tr->trDuration;
			}
			deltaTime = (atTime - tr->trTime) * 0.001;	// milliseconds to seconds
			// phase is the breaking constant
			phase = VectorLength(tr->trDelta) / (tr->trDuration * 0.001);
			// trDelta at least gives us the acceleration direction
			VectorNormalize2(tr->trDelta, result);
			// get distance travelled at current time (without breaking)
			VectorMA(tr->trBase, deltaTime, tr->trDelta, v);
			// subtract breaking force
			VectorMA(v, -phase * 0.5 * deltaTime * deltaTime, result, result);
			break;
		default:
			Com_Error(ERR_DROP, "BG_EvaluateTrajectory: unknown trType: %i", tr->trTime);
			break;
	}
}

/*
================
BG_EvaluateTrajectoryDelta

For determining velocity at a given time
================
*/
void BG_EvaluateTrajectoryDelta(const trajectory_t * tr, int atTime, vec3_t result)
{
	float           deltaTime;
	float           phase;

	switch (tr->trType)
	{
		case TR_STATIONARY:
		case TR_INTERPOLATE:
			VectorClear(result);
			break;
		case TR_LINEAR:
			VectorCopy(tr->trDelta, result);
			break;
		case TR_SINE:
			deltaTime = (atTime - tr->trTime) / (float)tr->trDuration;
			phase = cos(deltaTime * M_PI * 2);	// derivative of sin = cos
			phase *= 0.5;
			VectorScale(tr->trDelta, phase, result);
			break;
		case TR_LINEAR_STOP:
			if(atTime > tr->trTime + tr->trDuration)
			{
				VectorClear(result);
				return;
			}
			VectorCopy(tr->trDelta, result);
			break;
		case TR_GRAVITY_LOW:
			deltaTime = (atTime - tr->trTime) * 0.001;	// milliseconds to seconds
			VectorCopy(tr->trDelta, result);
			result[2] -= (DEFAULT_GRAVITY * 0.3) * deltaTime;	// FIXME: local gravity...
			break;
		case TR_GRAVITY:
			deltaTime = (atTime - tr->trTime) * 0.001;	// milliseconds to seconds
			VectorCopy(tr->trDelta, result);
			result[2] -= DEFAULT_GRAVITY * deltaTime;	// FIXME: local gravity...
			break;
		case TR_GRAVITY_FLOAT:
			deltaTime = (atTime - tr->trTime) * 0.001;	// milliseconds to seconds
			VectorCopy(tr->trDelta, result);
			result[2] -= (DEFAULT_GRAVITY * 0.2) * deltaTime;
			break;
		case TR_ACCELERATE:	// trDelta is eventual speed
			if(atTime > tr->trTime + tr->trDuration)
			{
				VectorClear(result);
				return;
			}
			deltaTime = (atTime - tr->trTime) * 0.001;	// milliseconds to seconds
			phase = deltaTime / (float)tr->trDuration;
			VectorScale(tr->trDelta, deltaTime * deltaTime, result);
			break;
		case TR_DECCELERATE:	// trDelta is breaking force
			if(atTime > tr->trTime + tr->trDuration)
			{
				VectorClear(result);
				return;
			}
			deltaTime = (atTime - tr->trTime) * 0.001;	// milliseconds to seconds
			VectorScale(tr->trDelta, deltaTime, result);
			break;
		default:
			Com_Error(ERR_DROP, "BG_EvaluateTrajectoryDelta: unknown trType: %i", tr->trTime);
			break;
	}
}

char           *eventnames[] = {
	"EV_NONE",

	"EV_FOOTSTEP",
	"EV_FOOTSTEP_METAL",
	"EV_FOOTSPLASH",
	"EV_FOOTWADE",
	"EV_SWIM",

	"EV_STEP_4",
	"EV_STEP_8",
	"EV_STEP_12",
	"EV_STEP_16",

	"EV_FALL_SHORT",
	"EV_FALL_MEDIUM",
	"EV_FALL_FAR",

	"EV_JUMP_PAD",				// boing sound at origin", jump sound on player

	"EV_JUMP",
	"EV_WALLJUMP",
	"EV_WATER_TOUCH",			// foot touches
	"EV_WATER_LEAVE",			// foot leaves
	"EV_WATER_UNDER",			// head touches
	"EV_WATER_CLEAR",			// head leaves

	"EV_ITEM_PICKUP",			// normal item pickups are predictable
	"EV_GLOBAL_ITEM_PICKUP",	// powerup / team sounds are broadcast to everyone

	"EV_NOAMMO",
	"EV_CHANGE_WEAPON",
	"EV_FIRE_WEAPON",

	"EV_USE_ITEM0",
	"EV_USE_ITEM1",
	"EV_USE_ITEM2",
	"EV_USE_ITEM3",
	"EV_USE_ITEM4",
	"EV_USE_ITEM5",
	"EV_USE_ITEM6",
	"EV_USE_ITEM7",
	"EV_USE_ITEM8",
	"EV_USE_ITEM9",
	"EV_USE_ITEM10",
	"EV_USE_ITEM11",
	"EV_USE_ITEM12",
	"EV_USE_ITEM13",
	"EV_USE_ITEM14",
	"EV_USE_ITEM15",

	"EV_ITEM_RESPAWN",
	"EV_ITEM_POP",
	"EV_PLAYER_TELEPORT_IN",
	"EV_PLAYER_TELEPORT_OUT",

	"EV_GRENADE_BOUNCE",		// eventParm will be the soundindex

	"EV_GENERAL_SOUND",
	"EV_GLOBAL_SOUND",			// no attenuation
	"EV_GLOBAL_TEAM_SOUND",

	"EV_BULLET_HIT_FLESH",
	"EV_BULLET_HIT_WALL",

	"EV_RAIL_HIT_FLESH",

	"EV_LG_HIT",
	"EV_LG_MISS",

	"EV_MISSILE_HIT",
	"EV_MISSILE_MISS",
	"EV_MISSILE_MISS_METAL",
	"EV_RAILTRAIL",
	"EV_RAILTRAIL2",
	"EV_SHOTGUN",
	"EV_BULLET",				// otherEntity is the shooter

	"EV_PAIN",
	"EV_EV_ARMORHIT",
	"EV_DEATH1",
	"EV_DEATH2",
	"EV_DEATH3",
	"EV_OBITUARY",

	"EV_POWERUP_QUAD",
	"EV_POWERUP_BATTLESUIT",
	"EV_POWERUP_REGEN",
	"EV_SPAWN_PROTECT",

	"EV_GIB_PLAYER",			// gib a previously living player
	"EV_SCOREPLUM",				// score plum
	"EV_GIB_PLAYERQ",			// gib a previously living player with quad overlay

	"EV_GIB_PLAYERLEGS",		// gib a previously living player           
	"EV_GIB_PLAYERQLEGS",		// gib a previously living player with quad overlay

	"EV_GIB_PLAYERHEAD",		// gib a previously living player           
	"EV_GIB_PLAYERQHEAD",		// gib a previously living player with quad overlay

	"EV_GIB_PLAYERQFIRE20",		// gib a previously living player with a 20% chance of being on fire with quad overlay
	"EV_GIB_PLAYERQFIRE50",		// gib a previously living player with a 50% chance of being on fire with quad overlay
	"EV_GIB_PLAYERQFIRE80",		// gib a previously living player with a 80% chance of being on fire with quad overlay

	"EV_GIB_PLAYERFIRE20",		// gib a previously living player with a 20% chance of being on fire
	"EV_GIB_PLAYERFIRE50",		// gib a previously living player with a 50% chance of being on fire
	"EV_GIB_PLAYERFIRE80",		// gib a previously living player with a 80% chance of being on fire

	"EV_GIB_PLAYERQFIRE20LEGS",	// gib a previously living player with a 20% chance of being on fire with quad overlay
	"EV_GIB_PLAYERQFIRE50LEGS",	// gib a previously living player with a 50% chance of being on fire with quad overlay
	"EV_GIB_PLAYERQFIRE80LEGS",	// gib a previously living player with a 80% chance of being on fire with quad overlay

	"EV_GIB_PLAYERFIRE20LEGS",	// gib a previously living player with a 20% chance of being on fire
	"EV_GIB_PLAYERFIRE50LEGS",	// gib a previously living player with a 50% chance of being on fire
	"EV_GIB_PLAYERFIRE80LEGS",	// gib a previously living player with a 80% chance of being on fire

	"EV_GIB_PLAYERQFIRE20HEAD",	// gib a previously living player with a 20% chance of being on fire with quad overlay
	"EV_GIB_PLAYERQFIRE50HEAD",	// gib a previously living player with a 50% chance of being on fire with quad overlay
	"EV_GIB_PLAYERQFIRE80HEAD",	// gib a previously living player with a 80% chance of being on fire with quad overlay

	"EV_GIB_PLAYERFIRE20HEAD",	// gib a previously living player with a 20% chance of being on fire
	"EV_GIB_PLAYERFIRE50HEAD",	// gib a previously living player with a 50% chance of being on fire
	"EV_GIB_PLAYERFIRE80HEAD",	// gib a previously living player with a 80% chance of being on fire


//#ifdef MISSIONPACK
	"EV_PROXIMITY_MINE_STICK",
	"EV_PROXIMITY_MINE_TRIGGER",
	"EV_KAMIKAZE",				// kamikaze explodes
	"EV_OBELISKEXPLODE",		// obelisk explodes
	"EV_INVUL_IMPACT",			// invulnerability sphere impact
	"EV_JUICED",				// invulnerability juiced effect
	"EV_LIGHTNINGBOLT",			// lightning bolt bounced of invulnerability sphere
//#endif

	"EV_DEBUG_LINE",
	"EV_STOPLOOPINGSOUND",
	"EV_TAUNT",
	"EV_TAUNT_YES",
	"EV_TAUNT_NO",
	"EV_TAUNT_FOLLOWME",
	"EV_TAUNT_GETFLAG",
	"EV_TAUNT_GUARDBASE",
	"EV_RAILGUN_PREFIRE",
	"EV_RAILGUN_READY",
	"EV_FLAMETHROWER_EFFECT",
	"EV_TAUNT_PATROL"
};

/*
===============
BG_AddPredictableEventToPlayerstate

Handles the sequence numbers
===============
*/

void            trap_Cvar_VariableStringBuffer(const char *var_name, char *buffer, int bufsize);

void BG_AddPredictableEventToPlayerstate(int newEvent, int eventParm, playerState_t * ps)
{

#ifdef _DEBUG
	{
		char            buf[256];

		trap_Cvar_VariableStringBuffer("showevents", buf, sizeof(buf));
		if(atof(buf) != 0)
		{
#ifdef QAGAME
			Com_Printf(" game event svt %5d -> %5d: num = %20s parm %d\n", ps->pmove_framecount /*ps->commandTime */ ,
					   ps->eventSequence, eventnames[newEvent], eventParm);
#else
			Com_Printf("Cgame event svt %5d -> %5d: num = %20s parm %d\n", ps->pmove_framecount /*ps->commandTime */ ,
					   ps->eventSequence, eventnames[newEvent], eventParm);
#endif
		}
	}
#endif
	ps->events[ps->eventSequence & (MAX_PS_EVENTS - 1)] = newEvent;
	ps->eventParms[ps->eventSequence & (MAX_PS_EVENTS - 1)] = eventParm;
	ps->eventSequence++;
}

/*
========================
BG_TouchJumpPad
========================
*/
void BG_TouchJumpPad(playerState_t * ps, entityState_t * jumppad)
{
	vec3_t          angles;
	float           p;
	int             effectNum;

	// spectators don't use jump pads
	if(ps->pm_type != PM_NORMAL)
	{
		return;
	}

	// flying characters don't hit bounce pads
	if(ps->powerups[PW_FLIGHT])
	{
		return;
	}

	// if we didn't hit this same jumppad the previous frame
	// then don't play the event sound again if we are in a fat trigger
	if(ps->jumppad_ent != jumppad->number)
	{

		vectoangles(jumppad->origin2, angles);
		p = fabs(AngleNormalize180(angles[PITCH]));
		if(p < 45)
		{
			effectNum = 0;
		}
		else
		{
			effectNum = 1;
		}
		BG_AddPredictableEventToPlayerstate(EV_JUMP_PAD, effectNum, ps);
	}
	// remember hitting this jumppad this frame
	ps->jumppad_ent = jumppad->number;
	ps->jumppad_frame = ps->pmove_framecount;
	// give the player the velocity from the jumppad
	VectorCopy(jumppad->origin2, ps->velocity);
}

/*
========================
BG_PlayerStateToEntityState

This is done after each set of usercmd_t on the server,
and after local prediction on the client
========================
*/
void BG_PlayerStateToEntityState(playerState_t * ps, entityState_t * s, qboolean snap, qboolean quad)
{
	int             i;

	if(ps->pm_type == PM_INTERMISSION || ps->pm_type == PM_SPECTATOR || ps->pm_type == PM_NOCLIP)
	{
		s->eType = ET_INVISIBLE;
	}
	else if(ps->stats[STAT_HEALTH] <= GIB_HEALTH)
	{
		if(quad)
		{
			if(ps->stats[STAT_DEAD] == 1)
			{
				s->eType = ET_DEADQPLAYERLEGS;
			}
			else if(ps->stats[STAT_DEAD] == 2)
			{
				s->eType = ET_DEADQPLAYERHEAD;
			}
			else
			{
				s->eType = ET_DEADQPLAYER;
			}
		}
		else
		{
			if(ps->stats[STAT_DEAD] == 1)
			{
				s->eType = ET_DEADPLAYERLEGS;
			}
			else if(ps->stats[STAT_DEAD] == 2)
			{
				s->eType = ET_DEADPLAYERHEAD;
			}
			else
			{
				s->eType = ET_DEADPLAYER;
			}
		}
	}
	else
	{
		s->eType = ET_PLAYER;
	}

	s->number = ps->clientNum;

	s->pos.trType = TR_INTERPOLATE;
	VectorCopy(ps->origin, s->pos.trBase);
	if(snap)
	{
		SnapVector(s->pos.trBase);
	}
	// set the trDelta for flag direction
	VectorCopy(ps->velocity, s->pos.trDelta);

	s->apos.trType = TR_INTERPOLATE;
	VectorCopy(ps->viewangles, s->apos.trBase);
	if(snap)
	{
		SnapVector(s->apos.trBase);
	}

	s->angles2[YAW] = ps->movementDir;
	s->legsAnim = ps->legsAnim;
	s->torsoAnim = ps->torsoAnim;
	s->clientNum = ps->clientNum;	// ET_PLAYER looks here instead of at number
	// so corpses can also reference the proper config
	s->eFlags = ps->eFlags;
	if(ps->stats[STAT_HEALTH] <= 0)
	{
		s->eFlags |= EF_DEAD;
	}
	else
	{
		s->eFlags &= ~EF_DEAD;
	}

	if(ps->externalEvent)
	{
		s->event = ps->externalEvent;
		s->eventParm = ps->externalEventParm;
	}
	else if(ps->entityEventSequence < ps->eventSequence)
	{
		int             seq;

		if(ps->entityEventSequence < ps->eventSequence - MAX_PS_EVENTS)
		{
			ps->entityEventSequence = ps->eventSequence - MAX_PS_EVENTS;
		}
		seq = ps->entityEventSequence & (MAX_PS_EVENTS - 1);
		s->event = ps->events[seq] | ((ps->entityEventSequence & 3) << 8);
		s->eventParm = ps->eventParms[seq];
		ps->entityEventSequence++;
	}

	s->weapon = ps->weapon;
	s->groundEntityNum = ps->groundEntityNum;

	s->powerups = 0;
	for(i = 0; i < MAX_POWERUPS; i++)
	{
		if(ps->powerups[i])
		{
			s->powerups |= 1 << i;
		}
	}

	s->loopSound = ps->loopSound;
	s->generic1 = ps->generic1;
}

void BG_EntityStateToPlayerState(playerState_t * ps, entityState_t * s, qboolean snap, qboolean quad)
{
	int             i;

//  if ( ps->pm_type == PM_INTERMISSION || ps->pm_type == PM_SPECTATOR || ps->pm_type == PM_NOCLIP) {
//      s->eType = ET_INVISIBLE;
//  } else if ( ps->stats[STAT_HEALTH] <= GIB_HEALTH ) {
//      if(quad){
//          s->eType = ET_DEADQPLAYER;
//      }else{
//          s->eType = ET_DEADPLAYER;
//      }
//  } else {
//      s->eType = ET_PLAYER;
//  }

	ps->clientNum = s->number;

//  s->pos.trType = TR_INTERPOLATE;
	VectorCopy(s->pos.trBase, ps->origin);
	if(snap)
	{
		SnapVector(ps->origin);
	}
	// set the trDelta for flag direction
	VectorCopy(s->pos.trDelta, ps->velocity);

	s->apos.trType = TR_INTERPOLATE;
	VectorCopy(s->apos.trBase, ps->viewangles);
	if(snap)
	{
		SnapVector(ps->viewangles);
	}

	ps->movementDir = s->angles2[YAW];
	ps->legsAnim = s->legsAnim;
	ps->torsoAnim = s->torsoAnim;
	ps->clientNum = s->clientNum;	// ET_PLAYER looks here instead of at number
	// so corpses can also reference the proper config
	ps->eFlags = s->eFlags;
	if(s->eFlags |= EF_DEAD)
	{
		ps->stats[STAT_HEALTH] = 0;
	}
	else
	{
		ps->stats[STAT_HEALTH] = 100;
	}

	if(s->event)
	{
		ps->externalEvent = s->event;
		ps->externalEventParm = s->eventParm;
	}							//else if ( ps->entityEventSequence < ps->eventSequence ) {
	//  int     seq;

	//  if ( ps->entityEventSequence < ps->eventSequence - MAX_PS_EVENTS) {
	//      ps->entityEventSequence = ps->eventSequence - MAX_PS_EVENTS;
	//  }
	//  seq = ps->entityEventSequence & (MAX_PS_EVENTS-1);
	//  s->event = ps->events[ seq ] | ( ( ps->entityEventSequence & 3 ) << 8 );
	//  s->eventParm = ps->eventParms[ seq ];
	//  ps->entityEventSequence++;
//  }

	ps->weapon = s->weapon;
	ps->groundEntityNum = s->groundEntityNum;

	ps->powerups[MAX_POWERUPS] = 0;
	for(i = 0; i < MAX_POWERUPS; i++)
	{
		if(s->powerups)
		{
			ps->powerups[i] |= 1 << i;
		}
	}

	ps->loopSound = s->loopSound;
	ps->generic1 = s->generic1;
}


/*
========================
BG_PlayerStateToEntityStateExtraPolate

This is done after each set of usercmd_t on the server,
and after local prediction on the client
========================
*/
void BG_PlayerStateToEntityStateExtraPolate(playerState_t * ps, entityState_t * s, int time, qboolean snap, qboolean quad)
{
	int             i;

	if(ps->pm_type == PM_INTERMISSION || ps->pm_type == PM_SPECTATOR || ps->pm_type == PM_NOCLIP)
	{
		s->eType = ET_INVISIBLE;
	}
	else if(ps->stats[STAT_HEALTH] <= GIB_HEALTH)
	{
		s->eType = ET_INVISIBLE;
	}
	else
	{
		s->eType = ET_PLAYER;
	}

	s->number = ps->clientNum;

	s->pos.trType = TR_LINEAR_STOP;
	VectorCopy(ps->origin, s->pos.trBase);
	if(snap)
	{
		SnapVector(s->pos.trBase);
	}
	// set the trDelta for flag direction and linear prediction
	VectorCopy(ps->velocity, s->pos.trDelta);
	// set the time for linear prediction
	s->pos.trTime = time;
	// set maximum extra polation time
	s->pos.trDuration = 50;		// 1000 / sv_fps (default = 20)

	s->apos.trType = TR_INTERPOLATE;
	VectorCopy(ps->viewangles, s->apos.trBase);
	if(snap)
	{
		SnapVector(s->apos.trBase);
	}

	s->angles2[YAW] = ps->movementDir;
	s->legsAnim = ps->legsAnim;
	s->torsoAnim = ps->torsoAnim;
	s->clientNum = ps->clientNum;	// ET_PLAYER looks here instead of at number
	// so corpses can also reference the proper config
	s->eFlags = ps->eFlags;
	if(ps->stats[STAT_HEALTH] <= 0)
	{
		s->eFlags |= EF_DEAD;
	}
	else
	{
		s->eFlags &= ~EF_DEAD;
	}

	if(ps->externalEvent)
	{
		s->event = ps->externalEvent;
		s->eventParm = ps->externalEventParm;
	}
	else if(ps->entityEventSequence < ps->eventSequence)
	{
		int             seq;

		if(ps->entityEventSequence < ps->eventSequence - MAX_PS_EVENTS)
		{
			ps->entityEventSequence = ps->eventSequence - MAX_PS_EVENTS;
		}
		seq = ps->entityEventSequence & (MAX_PS_EVENTS - 1);
		s->event = ps->events[seq] | ((ps->entityEventSequence & 3) << 8);
		s->eventParm = ps->eventParms[seq];
		ps->entityEventSequence++;
	}

	s->weapon = ps->weapon;
	s->groundEntityNum = ps->groundEntityNum;

	s->powerups = 0;
	for(i = 0; i < MAX_POWERUPS; i++)
	{
		if(ps->powerups[i])
		{
			s->powerups |= 1 << i;
		}
	}

	s->loopSound = ps->loopSound;
	s->generic1 = ps->generic1;
}
