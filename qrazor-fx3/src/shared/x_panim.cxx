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
	{"idle",		PLAYER_ANIM_IDLE},
	{"idle_up",		PLAYER_ANIM_IDLE_UP},
	{"idle_down",		PLAYER_ANIM_IDLE_DOWN},
	
	
	{"crouch_idle",		PLAYER_ANIM_CROUCH_IDLE},
//	{"crouch_down",		PLAYER_ANIM_CROUCH_DOWN},
//	{"crouch_up",		PLAYER_ANIM_CROUCH_UP},
//	{"crouch_walk",		PLAYER_ANIM_CROUCH_WALK},
	
	{"walk_forward",	PLAYER_ANIM_WALK_FORWARD},
	{"walk_backward",	PLAYER_ANIM_WALK_BACKWARD},
	{"walk_right",		PLAYER_ANIM_WALK_RIGHT},
	{"walk_left",		PLAYER_ANIM_WALK_LEFT},
	
//	{"jump",		PLAYER_ANIM_JUMP},
	
	{"land_soft",		PLAYER_ANIM_LAND_SOFT},
	{"land_hard",		PLAYER_ANIM_LAND_HARD},
	
	{"run",			PLAYER_ANIM_RUN},
	
//	{"idle",		PLAYER_ANIM_SWIM},
	
	{"attack1",		PLAYER_ANIM_ATTACK1},			
//	{"attack2",		PLAYER_ANIM_ATTACK2},
	
//	{"gesture",		PLAYER_ANIM_GESTURE},
	
	{"pain1",		PLAYER_ANIM_PAIN1},
//	{"pain2",		PLAYER_ANIM_PAIN2},
//	{"pain3",		PLAYER_ANIM_PAIN3},
	
	{"death1",		PLAYER_ANIM_DEATH1},
//	{"death2",		PLAYER_ANIM_DEATH2},
//	{"death3",		PLAYER_ANIM_DEATH3},
	
	{NULL,		0}
};

