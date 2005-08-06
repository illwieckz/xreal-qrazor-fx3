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
#ifndef G_TRIGGER_H
#define G_TRIGGER_H

/// includes ===================================================================
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
// system -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
// xreal --------------------------------------------------------------------
#include "g_entity.h"



class g_trigger_c : public g_entity_c
{
public:
//	g_trigger_c();

	void	init();
	
	virtual bool	touch(g_entity_c *other, const plane_c *plane, const csurface_c *surf);
	
};


class g_trigger_multiple_c : public g_trigger_c
{
public:
	g_trigger_multiple_c();
	
	virtual void	think();
	virtual bool	touch(g_entity_c *other, const plane_c *plane, const csurface_c *surf);
	virtual void	use(g_entity_c *other, g_entity_c *activator);
	
	virtual void	activate();
};


class g_trigger_once_c : public g_trigger_multiple_c
{
public:
	
	virtual void	activate();
};



class g_trigger_push_c : public g_trigger_c
{
public:
	g_trigger_push_c();
	
	virtual void	think();
	virtual bool	touch(g_entity_c *other, const plane_c *plane, const csurface_c *surf);
	
	virtual void	activate();
};


class g_trigger_hurt_c : public g_trigger_c
{
public:
	g_trigger_hurt_c();
	
	virtual bool	touch(g_entity_c *other, const plane_c *plane, const csurface_c *surf);
	virtual void	use(g_entity_c *other, g_entity_c *activator);
	
	virtual void	activate();

private:
	int		_timestamp;
};


class g_trigger_gravity_c : public g_trigger_c
{
public:
	g_trigger_gravity_c();
	
	virtual bool	touch(g_entity_c *other, const plane_c *plane, const csurface_c *surf);
	
	virtual void	activate();
};


class g_trigger_door_c : public g_trigger_c
{
public:
	g_trigger_door_c();	// helper entity for func_door*
	
	virtual bool	touch(g_entity_c *other, const plane_c *plane, const csurface_c *surf);
};



class g_trigger_teleport_c : public g_trigger_c
{
public:
	g_trigger_teleport_c();
	
	virtual void	think();
	virtual bool	touch(g_entity_c *other, const plane_c *plane, const csurface_c *surf);
	
	virtual void	activate();
};


#endif // G_TRIGGER_H
