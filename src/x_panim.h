/// ============================================================================
/*
Copyright (C) 2005 Robert Beckebans <trebor_7@users.sourceforge.net>
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
#ifndef X_PANIM_H
#define X_PANIM_H

/// includes ===================================================================
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
// system -------------------------------------------------------------------
// shared -------------------------------------------------------------------
#include "x_shared.h"

// qrazor-fx ----------------------------------------------------------------
// xreal --------------------------------------------------------------------

enum //player_anim_type_e
{
	PLAYER_ANIM_FISTS_IDLE,

	PLAYER_ANIM_FIRST			= PLAYER_ANIM_FISTS_IDLE,

	PLAYER_ANIM_FISTS_IDLE_UP,
	PLAYER_ANIM_FISTS_IDLE_DOWN,
	
//	PLAYER_ANIM_CROUCH_IDLE,
//	PLAYER_ANIM_CROUCH_DOWN,
//	PLAYER_ANIM_CROUCH_UP,
//	PLAYER_ANIM_CROUCH_WALK,
	
//	PLAYER_ANIM_WALK_FORWARD,
//	PLAYER_ANIM_WALK_BACKWARD,
//	PLAYER_ANIM_WALK_RIGHT,
//	PLAYER_ANIM_WALK_LEFT,
	
//	PLAYER_ANIM_JUMP,
	
//	PLAYER_ANIM_LAND_SOFT,
//	PLAYER_ANIM_LAND_HARD,
	
//	PLAYER_ANIM_RUN,
	
//	PLAYER_ANIM_SWIM,
	
//	PLAYER_ANIM_ATTACK1,
//	PLAYER_ANIM_ATTACK2,
	
//	PLAYER_ANIM_GESTURE,
	
//	PLAYER_ANIM_PAIN1,
//	PLAYER_ANIM_PAIN2,
//	PLAYER_ANIM_PAIN3,
	
//	PLAYER_ANIM_DEATH1,
//	PLAYER_ANIM_DEATH2,
//	PLAYER_ANIM_DEATH3

	PLAYER_ANIM_LAST			= PLAYER_ANIM_FISTS_IDLE_DOWN,

	PLAYER_ANIMS_NUM
};


struct player_anim_t
{
	const char*		name;
	int			type;
};

extern const player_anim_t player_anims[];

#endif // X_PANIM_H
