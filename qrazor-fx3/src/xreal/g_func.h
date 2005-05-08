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
#ifndef G_FUNC_H
#define G_FUNC_H

/// includes ===================================================================
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
// system -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
// xreal --------------------------------------------------------------------
#include "g_entity.h"




struct moveinfo_t
{
	// fixed data
	vec3_c		start_origin;
	quaternion_c	start_quat;
	vec3_c		end_origin;
	quaternion_c	end_quat;

	int		sound_start;
	int		sound_middle;
	int		sound_end;

	float		accel;
	float		speed;
	float		decel;
	float		distance;

	float		wait;

	// state data
	int		state;
	vec3_c		dir;
	float		current_speed;
	float		move_speed;
	float		next_speed;
	float		remaining_distance;
	float		decel_distance;
	int		endfunc;
};



class g_func_c : public g_entity_c
{
public:
	g_func_c();
	
	void	calcMove(const vec3_c &dest, int endfunc);
	void	beginMove();
	void	finishMove();
	void	clearMove();
	
protected:
	int		_thinktype;
	int		_lip;
	
	vec3_c		_pos1;
	vec3_c		_pos2;
public:
	moveinfo_t	_moveinfo;
};


class g_func_rotating_c : public g_func_c
{
public:
	g_func_rotating_c();
	
	virtual void	blocked(g_entity_c *other);
	virtual bool	touch(g_entity_c *other, const plane_c &plane, csurface_c *surf);
	virtual void	use(g_entity_c *other, g_entity_c *activator);
	
	virtual void	activate();
};


class g_func_button_c : public g_func_c
{
public:
	g_func_button_c();
	
	virtual void	think();
	virtual bool	touch(g_entity_c *other, const plane_c &plane, csurface_c *surf);
	virtual void	use(g_entity_c *other, g_entity_c *activator);
	virtual void	die(g_entity_c *inflictor, g_entity_c *attacker, int damage, vec3_t point);
	
	virtual void	activate();
	
private:
	void	finish();
	void	returnButton();
	void	wait();
	void	fire();
};


class g_func_door_c : public g_func_c
{
public:
	g_func_door_c();
	
	virtual void	think();
	virtual void	blocked(g_entity_c *other);
	virtual bool	touch(g_entity_c *other, const plane_c &plane, csurface_c *surf);
	virtual void	use(g_entity_c *other, g_entity_c *activator);
	virtual void	die(g_entity_c *inflictor, g_entity_c *attacker, int damage, vec3_t point);
	
	virtual void	activate();
	
private:
	void	hitTop();
	void	hitBottom();
	void	calcMoveSpeed();
	void	spawnDoorTrigger();
	void	goDown();
	void	goUp(g_entity_c *activator);
	
	void	useAreaportals(bool open);
	
	std::string	_snd_open;
	std::string	_snd_close;
};


class g_func_killbox_c : public g_func_c
{
public:
	g_func_killbox_c();
	
	virtual void	use(g_entity_c *other, g_entity_c *activator);
	
	virtual void	activate();
};


class g_func_static_c : public g_entity_c
{
public:
	g_func_static_c();
	
	virtual void	activate();
};

/*
class g_func_flare_c : public g_entity_c
{
public:
	g_func_flare_c();
	
	virtual void	activate();
};
*/


/*
class g_ode_box_c : public g_entity_c
{
public:
	g_ode_box_c();
	
	virtual void	activate();
};
*/

#endif // G_FUNC_H
