/// ============================================================================
/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2003 Robert Beckebans <trebor_7@users.sourceforge.net>
Copyright (C) 2003, 2004  contributors of the XreaL project
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
#ifndef G_TARGET_H
#define G_TARGET_H

/// includes ===================================================================
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
// system -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
// xreal --------------------------------------------------------------------
#include "g_entity.h"




class g_target_c : public g_entity_c
{
public:
	g_target_c();
	
};


class g_target_speaker_c : public g_target_c
{
public:	
	g_target_speaker_c();
	
	virtual void	use(g_entity_c *other, g_entity_c *activator);
	
	virtual void	activate();

private:
	std::string	_s_shader;
	int		_s_looping;
};


class g_target_explosion_c : public g_target_c
{
public:
	g_target_explosion_c();
	
	virtual void	think();
	virtual void	use(g_entity_c *other, g_entity_c *activator);

	virtual void	activate();
};


class g_target_position_c : public g_target_c
{
public:

	virtual void	activate();
};


class g_target_teleport_c : public g_target_c
{
public:	
	
	virtual void	activate();
};


class g_target_delay_c : public g_target_c
{
public:
	// delay temporary target, this one will not have a spawn function
	g_target_delay_c(g_entity_c *ent, g_entity_c *activator);
	
	virtual void	think();
};


class g_target_changelevel_c : public g_target_c
{
public:
	g_target_changelevel_c();
	
	inline const char*	getMapName() const	{return _map.c_str();}

private:
	std::string	_map;
};






#endif // G_TARGET_H
