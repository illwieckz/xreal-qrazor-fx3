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
#ifndef G_WORLD_H
#define G_WOLRD_H

/// includes ===================================================================
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
// system -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
// xreal --------------------------------------------------------------------
#include "g_entity.h"



class g_world_c : public g_entity_c
{
public:
	//
	// constructor / destructor
	//
	g_world_c();
	
	~g_world_c();
	
	//
	// virtual functions
	//
	virtual void	activate();
	
	//
	// access
	//
	
	
	//
	// members
	//
	std::string	_sky;
	float		_skyrotate;
	vec3_c		_skyaxis;
	
	std::string	_nextmap;

	int		_lip;
	int		_distance;
	int		_height;
	std::string	_noise;
	float		_pausetime;
	std::string	_item;

	float		_minyaw;
	float		_maxyaw;
	float		_minpitch;
	float		_maxpitch;
};


#endif // G_WORLD_H
