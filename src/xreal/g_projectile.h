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
#ifndef G_PROJECTILE_H
#define G_PROJECTILE_H

/// includes ===================================================================
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
// system -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
// xreal --------------------------------------------------------------------
#include "g_entity.h"




class g_projectile_c : public g_entity_c
{
public:
	//TODO
};


class g_projectile_bolt_c : public g_projectile_c
{
public:
	g_projectile_bolt_c(g_entity_c *activator, const vec3_c &start, const vec3_c &dir, int damage, vec_t speed);
	
	virtual void	think();
	virtual bool	touch(g_entity_c *other, const plane_c &plane, csurface_c *surf);
};


class g_projectile_grenade_c : public g_projectile_c
{
public:
	g_projectile_grenade_c(g_entity_c *activator, const vec3_c &start, const vec3_c &dir, int damage, int speed, float timer, float damage_radius);
	
	virtual void	think();
	virtual bool	touch(g_entity_c *other, const plane_c &plane, csurface_c *surf);
};


class g_projectile_rocket_c : public g_projectile_c
{
public:
	g_projectile_rocket_c(g_entity_c *activator, const vec3_c &origin, const quaternion_c &quat, int damage, int speed, float damage_radius, float radius_damage);
	
	virtual void	think();
	virtual bool	touch(g_entity_c *other, const plane_c &plane, csurface_c *surf);
	
private:
	void		explode();
};



#endif // G_PROJECTILE_H
