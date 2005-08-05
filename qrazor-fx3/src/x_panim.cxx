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
// shared -------------------------------------------------------------------
#include "x_panim.h"

// qrazor-fx ----------------------------------------------------------------
// xreal --------------------------------------------------------------------

const player_anim_t player_anims[] = 
{	
// 	bfg_aim
// 	bfg_aim_down
// 	bfg_aim_up
// 	bfg_fire
// 	bfg_fire_down
// 	bfg_fire_up
// 	bfg_idle
// 	bfg_idle_down
// 	bfg_idle_up
// 	bfg_reload
// 	bfg_startfire
// 	bfg_startfire_down
// 	bfg_startfire_up
// 	chaingun_fire
// 	chaingun_fire_down
// 	chaingun_fire_up
// 	chaingun_idle
// 	chaingun_idle_down
// 	chaingun_idle_up
// 	chaingun_reload
// 	chainsaw_fire
// 	chainsaw_fire_down
// 	chainsaw_fire_up
// 	chainsaw_idle
// 	chainsaw_idle_down
// 	chainsaw_idle_up
// 	chainsaw_raise
	{"crouch",			PLAYER_ANIM_CROUCH},
	{"crouch_down",			PLAYER_ANIM_CROUCH_DOWN},
	{"crouch_up",			PLAYER_ANIM_CROUCH_UP},
	{"crouch_walk",			PLAYER_ANIM_CROUCH_WALK},
	{"crouch_walk_back",		PLAYER_ANIM_CROUCH_WALK_BACK},
// 	exit_teleporter
	{"fall",			PLAYER_ANIM_FALL},
// 	fists_aim
// 	fists_aim_down
// 	fists_aim_up
	{"fists_idle",			PLAYER_ANIM_FISTS_IDLE},
	{"fists_idle_up",		PLAYER_ANIM_FISTS_IDLE_UP},
	{"fists_idle_down",		PLAYER_ANIM_FISTS_IDLE_DOWN},
// 	fists_punch1
// 	fists_punch2
// 	fists_punch_down1
// 	fists_punch_down2
// 	fists_punch_up1
// 	fists_punch_up2
// 	flashlight_aim_down
// 	flashlight_aim_up
// 	flashlight_idle
// 	flashlight_swing1
// 	flashlight_swing1_down
// 	flashlight_swing1_up
// 	flashlight_swing2
// 	flashlight_swing2_down
// 	flashlight_swing2_up
// 	hard_land
// 	initial
// 	jog
// 	jog_back
// 	lowerpda
// 	lowerwep
// 	lowerwep_left
// 	machinegun_aim
// 	machinegun_aim_down
// 	machinegun_aim_up
// 	machinegun_fire1
// 	machinegun_fire_down
// 	machinegun_fire_up
// 	machinegun_idle
// 	machinegun_idle_down
// 	machinegun_idle_up
// 	machinegun_reload
// 	nade_idle
// 	nade_idle_down
// 	nade_idle_up
// 	nade_quickthrow
// 	nade_quickthrow_down
// 	nade_quickthrow_up
// 	nade_throw
// 	nade_throw_down
// 	nade_throw_up
// 	nade_throwstart
// 	nade_throwstart_down
// 	nade_throwstart_up
// 	painchest
// 	painhead
// 	painleftarm
// 	painrightarm
// 	pda_idle
// 	pistol_aim
// 	pistol_aim_down
// 	pistol_aim_up
// 	pistol_fire1
// 	pistol_fire_down
// 	pistol_fire_up
// 	pistol_idle
// 	pistol_idle_down
// 	pistol_idle_up
// 	pistol_reload
// 	plasmagun_aim
// 	plasmagun_aim_down
// 	plasmagun_aim_up
// 	plasmagun_fire1
// 	plasmagun_fire_down
// 	plasmagun_fire_up
// 	plasmagun_idle
// 	plasmagun_idle_down
// 	plasmagun_idle_up
// 	raisepda
// 	raisewep
// 	raisewep_left
// 	rocketlauncher_aim
// 	rocketlauncher_aim_down
// 	rocketlauncher_aim_up
// 	rocketlauncher_fire
// 	rocketlauncher_fire_down
// 	rocketlauncher_fire_up
// 	rocketlauncher_idle
// 	rocketlauncher_idle_down
// 	rocketlauncher_idle_up
// 	rocketlauncher_reload
// 	run
// 	run_270
// 	run_backwards_270
// 	run_strafe_left
// 	run_strafe_right
// 	shotgun_aim
// 	shotgun_aim_down
// 	shotgun_aim_up
// 	shotgun_fire1
// 	shotgun_fire_down
// 	shotgun_fire_up
// 	shotgun_idle
// 	shotgun_idle_down
// 	shotgun_idle_up
// 	shotgun_reload_end
// 	shotgun_reload_loop1
// 	shotgun_reload_start
// 	soft_land
// 	soulcube_fire
// 	soulcube_idle
// 	soulcube_idle_down
// 	soulcube_idle_up
// 	teleportin
// 	{"turn_left",			PLAYER_ANIM_TURN_LEFT},
// 	{"turn_right",			PLAYER_ANIM_TURN_RIGHT},
	{"walk_strafe_left",		PLAYER_ANIM_WALK_STRAFE_LEFT},
	{"walk_strafe_right",		PLAYER_ANIM_WALK_STRAFE_RIGHT},
	{"walkjump1",			PLAYER_ANIM_WALKJUMP1},
	
	{NULL,		0}
};

