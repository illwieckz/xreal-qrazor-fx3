/// ============================================================================
/*
Copyright (C) 1997-2001 Id Software, Inc.
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
#ifndef BG_PUBLIC_H
#define BG_PUBLIC_H

/// includes ===================================================================
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
// system -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
#include "../x_shared.h"

// xreal --------------------------------------------------------------------

// the "gameversion" client command will print this plus compile date
#define	GAMEVERSION	"XreaL"

enum
{
	
	MINS_Z			= 0,
	VIEWHEIGHT_DEFAULT	= 64,
	VIEWHEIGHT_CROUCH	= 24,
	VIEWHEIGHT_DEAD		= 12
};


enum
{
	// can accelerate and turn
	PM_NORMAL,
	PM_SPECTATOR,
	// no acceleration or turning
	PM_DEAD,
	PM_GIB,		// different bounding box
	PM_FREEZE
};

// pmove->pm_flags
enum
{
	PMF_DUCKED		= (1<<0),
	PMF_JUMP_HELD		= (1<<1),
	PMF_ON_GROUND		= (1<<2),
	PMF_TIME_WATERJUMP	= (1<<3),	// pm_time is waterjump
	PMF_TIME_LAND		= (1<<4),	// pm_time is time before rejump
	PMF_TIME_TELEPORT	= (1<<5),	// pm_time is non-moving time
	PMF_NO_PREDICTION	= (1<<6)	// temporarily disables prediction (used for grappling hook)
};

struct pmove_t
{
	inline void	clear()
	{
		s.clear();
		
		cmd.clear();
		snapinitial	 = false;
		
		touchents.clear();
		
		viewangles.clear();
		viewheight	= 0;
		
		bbox.clear();
		
		groundentity	= NULL;
		
		watertype	= 0;
		waterlevel	= 0;
		
		rayTrace	= NULL;
		boxTrace	= NULL;
		pointContents	= NULL;
	}

	// state (in / out)
	pmove_state_t	s;

	// command (in)
	usercmd_t	cmd;
	bool		snapinitial;	// if s has been changed outside pmove

	// results (out)
	std::deque<entity_c*>	touchents;
	
	vec3_c		viewangles;			// clamped
	float		viewheight;

	aabb_c		bbox;

	entity_c*	groundentity;
	
	int		watertype;
	int		waterlevel;

	// callbacks to test the world
	trace_t		(*rayTrace)(const vec3_c &start, const vec3_c &dir, vec_t length);
	trace_t		(*boxTrace)(const vec3_c &start, const aabb_c &bbox, const vec3_c &end);
	int		(*pointContents)(const vec3_c &point);
};

// player movement code common with client prediction
void	BG_PMove(pmove_t *pmove);


// entity_state_t->effects
// Effects are things handled on the client side (lights, particles, frame animations)
// that happen constantly on the given entity.
// An entity that has effects will be sent to the client
// even if it has a zero index model.
enum
{
	EF_NONE				= 0,
	EF_ROTATE			= (1<<0),		// rotate (bonus items)
	EF_GIB				= (1<<1),		// leave a trail
	EF_BLASTER			= (1<<2),		// redlight + trail
	EF_ROCKET			= (1<<3),		// redlight + trail
	EF_GRENADE			= (1<<4),
	EF_HYPERBLASTER			= (1<<5),
	EF_BFG				= (1<<6),
	EF_COLOR_SHELL			= (1<<7),
	EF_AUTOANIM_TOGGLE_01_2		= (1<<8),		// automatically cycle between frames 0 and 1 at 2 hz
	EF_AUTOANIM_TOGGLE_23_2		= (1<<9),		// automatically cycle between frames 2 and 3 at 2 hz
	EF_AUTOANIM_1			= (1<<10),		// automatically cycle through all frames at 1hz
	EF_AUTOANIM_2			= (1<<11),		// automatically cycle through all frames at 2hz
	EF_AUTOANIM_10			= (1<<12),		// automatically cycle through all frames at 10hz
	EF_AUTOANIM_24			= (1<<13),		// automatically cycle through all frames at 24hz
	EF_QUAD				= (1<<14),
	EF_DLIGHT			= (1<<15)
};

// splash types
enum
{
	SPLASH_UNKNOWN,
	SPLASH_SPARKS,

	SPLASH_BLUE_WATER,
	SPLASH_BROWN_WATER,
	SPLASH_SLIME,
	SPLASH_LAVA,
	SPLASH_BLOOD
};

// muzzle flashes / player effects
enum
{
	MZ_BLASTER,
	MZ_MACHINEGUN,
	MZ_SHOTGUN,
	MZ_CHAINGUN1,
	MZ_CHAINGUN2,
	MZ_CHAINGUN3,
	MZ_RAILGUN,
	MZ_ROCKET,
	MZ_GRENADE,
	MZ_LOGIN,
	MZ_LOGOUT,
	MZ_RESPAWN,
	MZ_BFG,
	MZ_SSHOTGUN,
	MZ_HYPERBLASTER,
	MZ_ITEMRESPAWN,
	
	MZ_SIDEWINDER//,
	
	//MZ_SILENCED		= 128		// bit flag ORed with one of the above numbers
};

enum entity_type_e
{
	ET_GENERIC,
	
	ET_PLAYER,
	
	ET_FUNC_STATIC,
	
	ET_LIGHT_OMNI,
	ET_LIGHT_PROJ,
	
	ET_PROJECTILE_BOLT,
	ET_PROJECTILE_GRENADE,
	ET_PROJECTILE_ROCKET,
	
	ET_TARGET_SPEAKER
};


// entity_state_t->event values
// ertity events are for effects that take place reletive
// to an existing entities origin.  Very network efficient.
// All muzzle flashes really should be converted to events...
enum entity_event_e
{
	EV_NONE,
	
	EV_ITEM_RESPAWN,
	
	EV_PLAYER_TELEPORT,
	
	EV_PLAYER_FOOTSTEP,
	EV_PLAYER_FOOTSTEP_METAL,
	EV_PLAYER_FOOTSTEP_SPLASH,
	EV_PLAYER_SWIM,
	EV_PLAYER_JUMP,
	
	EV_PLAYER_FALL_SHORT,
	EV_PLAYER_FALL_MEDIUM,
	EV_PLAYER_FALL_FAR,
	
	EV_PLAYER_PAIN_100,
	EV_PLAYER_PAIN_75,
	EV_PLAYER_PAIN_50,
	EV_PLAYER_PAIN_25,
	
	EV_PLAYER_DIE,
	EV_PLAYER_GIB,
	
	EV_PLAYER_WATER_TOUCH,
	EV_PLAYER_WATER_LEAVE,
	EV_PLAYER_WATER_UNDER,
	EV_PLAYER_WATER_CLEAR,
	
	EV_GRENADE_BOUNCE,
	EV_GRENADE_EXPLODE,

	EV_ROCKET_SCRATCH,
	EV_ROCKET_EXPLOSION,
	EV_ROCKET_EXPLOSION_UNDERWATER
};

#endif // BG_PUBLIC_H
