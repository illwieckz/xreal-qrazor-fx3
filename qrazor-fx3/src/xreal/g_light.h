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
#ifndef G_LIGHT_H
#define G_LIGHT_H

/// includes ===================================================================
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
// system -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
// xreal --------------------------------------------------------------------
#include "g_entity.h"



class g_light_c : public g_entity_c
{
public:
	g_light_c();
	
	virtual void	think();
	//virtual void	touch(g_entity_c *other, cplane_c *plane, csurface_c *surf);
	virtual void	die(g_entity_c *inflictor, g_entity_c *attacker, int damage, vec3_t point);
	
	virtual void	activate();
	
private:
	std::string	_texture;
	
	vec3_c		_light_target;
	vec3_c		_light_right;
	vec3_c		_light_up;
	
	int		_falloff;
	
	int		_nodiffuse;
	int		_nospecular;
	int		_noshadows;
};



#endif // G_LIGHT_H
