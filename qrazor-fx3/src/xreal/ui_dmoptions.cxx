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
// xreal --------------------------------------------------------------------
#include "ui_local.h"


static char dmoptions_statusbar[128];


class menu_dmoptions_c : public menu_spincontrol_c
{
protected:
	menu_dmoptions_c()	{}

	void	getDMFlags()
	{
		_dmflags = trap_Cvar_VariableInteger("dmflags");
	}

	void	setDMFlags()
	{
		trap_Cvar_SetValue("dmflags", _dmflags);

		Com_sprintf(dmoptions_statusbar, sizeof(dmoptions_statusbar), "dmflags = %d", _dmflags);
	}
	
	void	addDMFlags(uint_t flags)
	{
		_dmflags &= ~flags;
	}
	
	void	delDMFlags(uint_t flags)
	{
		_dmflags |= flags;
	}

private:
	uint_t	_dmflags;
};


class menu_friendly_fire_c : public menu_dmoptions_c
{
public:
	menu_friendly_fire_c()
	{
		_name	= "friendly fire";
	
		_itemnames.push_back("no");
		_itemnames.push_back("yes");
	}

	virtual void	callback()
	{
		getDMFlags();
	
		if(_curvalue)
			delDMFlags(DF_NO_FRIENDLY_FIRE);
		else
			addDMFlags(DF_NO_FRIENDLY_FIRE);
		
		setDMFlags();
	}
};

class menu_falls_c : public menu_dmoptions_c
{
public:
	menu_falls_c()
	{
		_name	= "falling damage";
		
		_itemnames.push_back("no");
		_itemnames.push_back("yes");
	}

	virtual void	callback()
	{
		getDMFlags();
	
		if(_curvalue)
			delDMFlags(DF_NO_FALLING);
		else
			addDMFlags(DF_NO_FALLING);
		
		setDMFlags();
	}
};

class menu_weapons_stay_c : public menu_dmoptions_c
{
public:
	menu_weapons_stay_c()
	{
		_name	= "weapons stay";
		
		_itemnames.push_back("no");
		_itemnames.push_back("yes");
	}

	virtual void	callback()
	{
		getDMFlags();
	
		if(_curvalue)
			addDMFlags(DF_WEAPONS_STAY);
		else
			delDMFlags(DF_WEAPONS_STAY);
		
		setDMFlags();
	}
};

class menu_instant_powerups_c : public menu_dmoptions_c
{
public:
	menu_instant_powerups_c()
	{
		_name	= "instant powerups";
		
		_itemnames.push_back("no");
		_itemnames.push_back("yes");
	}
	
	virtual void	callback()
	{
		getDMFlags();
	
		if(_curvalue)
			addDMFlags(DF_INSTANT_ITEMS);
		else
			delDMFlags(DF_INSTANT_ITEMS);
		
		setDMFlags();
	}
};

class menu_powerups_c : public menu_dmoptions_c
{
public:
	menu_powerups_c()
	{
		_name	= "allow powerups";
		
		_itemnames.push_back("no");
		_itemnames.push_back("yes");
	}
	
	virtual void	callback()
	{
		getDMFlags();
	
		if(_curvalue)
			delDMFlags(DF_NO_ITEMS);
		else
			addDMFlags(DF_NO_ITEMS);
		
		setDMFlags();
	}
};

class menu_health_c : public menu_dmoptions_c
{
public:
	menu_health_c()
	{
		_name	= "allow health";
		
		_itemnames.push_back("no");
		_itemnames.push_back("yes");
	}

	virtual void	callback()
	{
		getDMFlags();
	
		if(_curvalue)
			delDMFlags(DF_NO_HEALTH);
		else
			addDMFlags(DF_NO_HEALTH);
		
		setDMFlags();
	}
};

class menu_spawn_farthest_c : public menu_dmoptions_c
{
public:
	menu_spawn_farthest_c()
	{
		_name	= "spawn farthest";
		
		_itemnames.push_back("no");
		_itemnames.push_back("yes");
	}

	virtual void	callback()
	{
		getDMFlags();
	
		if(_curvalue)
			addDMFlags(DF_SPAWN_FARTHEST);
		else
			delDMFlags(DF_SPAWN_FARTHEST);
		
		setDMFlags();
	}
};

class menu_teamplay_c : public menu_dmoptions_c
{
public:
	menu_teamplay_c()
	{
		_name	= "teamplay";
		
		_itemnames.push_back("disabled");
		_itemnames.push_back("by skin");
		_itemnames.push_back("by model");
	}

	virtual void	callback()
	{
		getDMFlags();
	
		switch(_curvalue)
		{
			case 1:
			case 2:
			{
				addDMFlags(DF_SKINTEAMS);
				delDMFlags(DF_MODELTEAMS);
				break;
			}
			
			default:
			{
				delDMFlags(DF_MODELTEAMS | DF_SKINTEAMS);
				break;
			}
		}
		
		setDMFlags();
	}
};

class menu_samelevel_c : public menu_dmoptions_c
{
public:
	menu_samelevel_c()
	{
		_name	= "same map";
	
		_itemnames.push_back("no");
		_itemnames.push_back("yes");
	}

	virtual void	callback()
	{
		getDMFlags();
	
		if(_curvalue)
			addDMFlags(DF_SAME_LEVEL);
		else
			delDMFlags(DF_SAME_LEVEL);
		
		setDMFlags();
	}
};

class menu_force_respawn_c : public menu_dmoptions_c
{
public:
	menu_force_respawn_c()
	{
		_name	= "force respawn";
		
		_itemnames.push_back("no");
		_itemnames.push_back("yes");
	}

	virtual void	callback()
	{
		getDMFlags();
	
		if(_curvalue)
			addDMFlags(DF_FORCE_RESPAWN);
		else
			delDMFlags(DF_FORCE_RESPAWN);
		
		setDMFlags();
	}
};

class menu_armor_c : public menu_dmoptions_c
{
public:
	menu_armor_c()
	{
		_name	= "allow armor";
		
		_itemnames.push_back("no");
		_itemnames.push_back("yes");
	}

	virtual void	callback()
	{
		getDMFlags();
	
		if(_curvalue)
			delDMFlags(DF_NO_ARMOR);
		else
			addDMFlags(DF_NO_ARMOR);
		
		setDMFlags();
	}
};

class menu_allow_exit_c : public menu_dmoptions_c
{
public:
	menu_allow_exit_c()
	{
		_name	= "allow exit";
		
		_itemnames.push_back("no");
		_itemnames.push_back("yes");
	}

	virtual void	callback()
	{
		getDMFlags();
	
		if(_curvalue)
			addDMFlags(DF_ALLOW_EXIT);
		else
			delDMFlags(DF_ALLOW_EXIT);
		
		setDMFlags();
	}
};

class menu_infinite_ammo_c : public menu_dmoptions_c
{
public:
	menu_infinite_ammo_c()
	{
		_name	= "infinite ammo";
	
		_itemnames.push_back("no");
		_itemnames.push_back("yes");
	}

	virtual void	callback()
	{
		getDMFlags();
	
		if(_curvalue)
			addDMFlags(DF_INFINITE_AMMO);
		else
			delDMFlags(DF_INFINITE_AMMO);
		
		setDMFlags();
	}
};

class menu_fixed_fov_c : public menu_dmoptions_c
{
public:
	menu_fixed_fov_c()
	{
		_name	= "fixed FOV";
		
		_itemnames.push_back("no");
		_itemnames.push_back("yes");
	}

	virtual void	callback()
	{
		getDMFlags();
	
		if(_curvalue)
			addDMFlags(DF_FIXED_FOV);
		else
			delDMFlags(DF_FIXED_FOV);
		
		setDMFlags();
	}
};

class menu_quad_drop_c : public menu_dmoptions_c
{
public:

	menu_quad_drop_c()
	{
		_name	= "quad drop";
		
		_itemnames.push_back("no");
		_itemnames.push_back("yes");
	}

	virtual void	callback()
	{
		getDMFlags();
	
		if(_curvalue)
			addDMFlags(DF_QUAD_DROP);
		else
			delDMFlags(DF_QUAD_DROP);
		
		setDMFlags();
	}
};

static menu_framework_c		s_dmoptions_menu;
static menu_friendly_fire_c		s_friendlyfire_box;
static menu_falls_c			s_falls_box;
static menu_weapons_stay_c		s_weapons_stay_box;
static menu_instant_powerups_c		s_instant_powerups_box;
static menu_powerups_c			s_powerups_box;
static menu_health_c			s_health_box;
static menu_spawn_farthest_c		s_spawn_farthest_box;
static menu_teamplay_c			s_teamplay_box;
static menu_samelevel_c			s_samelevel_box;
static menu_force_respawn_c		s_force_respawn_box;
static menu_armor_c			s_armor_box;
static menu_allow_exit_c		s_allow_exit_box;
static menu_infinite_ammo_c		s_infinite_ammo_box;
static menu_fixed_fov_c			s_fixed_fov_box;
static menu_quad_drop_c			s_quad_drop_box;



static void	DMOptions_MenuInit()
{
	int dmflags = trap_Cvar_VariableInteger("dmflags");
	int y = 0;

	s_dmoptions_menu._x = (int)(trap_VID_GetWidth() * 0.50);

	s_falls_box._x	= 0;
	s_falls_box._y	= y;	
	s_falls_box._curvalue = ( dmflags & DF_NO_FALLING ) == 0;

	s_weapons_stay_box._x	= 0;
	s_weapons_stay_box._y	= y += 10;
	s_weapons_stay_box._curvalue = ( dmflags & DF_WEAPONS_STAY ) != 0;

	s_instant_powerups_box._x	= 0;
	s_instant_powerups_box._y	= y += 10;
	s_instant_powerups_box._curvalue = ( dmflags & DF_INSTANT_ITEMS ) != 0;

	s_powerups_box._x	= 0;
	s_powerups_box._y	= y += 10;
	s_powerups_box._curvalue = ( dmflags & DF_NO_ITEMS ) == 0;

	s_health_box._x	= 0;
	s_health_box._y	= y += 10;
	s_health_box._curvalue = ( dmflags & DF_NO_HEALTH ) == 0;

	s_armor_box._x	= 0;
	s_armor_box._y	= y += 10;
	s_armor_box._curvalue = ( dmflags & DF_NO_ARMOR ) == 0;

	s_spawn_farthest_box._x	= 0;
	s_spawn_farthest_box._y	= y += 10;
	s_spawn_farthest_box._curvalue = ( dmflags & DF_SPAWN_FARTHEST ) != 0;

	s_samelevel_box._x	= 0;
	s_samelevel_box._y	= y += 10;
	s_samelevel_box._curvalue = ( dmflags & DF_SAME_LEVEL ) != 0;

	s_force_respawn_box._x	= 0;
	s_force_respawn_box._y	= y += 10;
	s_force_respawn_box._curvalue = ( dmflags & DF_FORCE_RESPAWN ) != 0;

	s_teamplay_box._x	= 0;
	s_teamplay_box._y	= y += 10;

	s_allow_exit_box._x	= 0;
	s_allow_exit_box._y	= y += 10;
	s_allow_exit_box._curvalue = ( dmflags & DF_ALLOW_EXIT ) != 0;

	s_infinite_ammo_box._x	= 0;
	s_infinite_ammo_box._y	= y += 10;
	s_infinite_ammo_box._curvalue = ( dmflags & DF_INFINITE_AMMO ) != 0;

	s_fixed_fov_box._x	= 0;
	s_fixed_fov_box._y	= y += 10;
	s_fixed_fov_box._curvalue = ( dmflags & DF_FIXED_FOV ) != 0;

	s_quad_drop_box._x	= 0;
	s_quad_drop_box._y	= y += 10;
	s_quad_drop_box._curvalue = ( dmflags & DF_QUAD_DROP ) != 0;

	s_friendlyfire_box._x	= 0;
	s_friendlyfire_box._y	= y += 10;
	s_friendlyfire_box._curvalue = ( dmflags & DF_NO_FRIENDLY_FIRE ) == 0;


	s_dmoptions_menu.addItem(&s_falls_box);
	s_dmoptions_menu.addItem(&s_weapons_stay_box);
	s_dmoptions_menu.addItem(&s_instant_powerups_box);
	s_dmoptions_menu.addItem(&s_powerups_box);
	s_dmoptions_menu.addItem(&s_health_box);
	s_dmoptions_menu.addItem(&s_armor_box);
	s_dmoptions_menu.addItem(&s_spawn_farthest_box);
	s_dmoptions_menu.addItem(&s_samelevel_box);
	s_dmoptions_menu.addItem(&s_force_respawn_box);
	s_dmoptions_menu.addItem(&s_teamplay_box);
	s_dmoptions_menu.addItem(&s_allow_exit_box);
	s_dmoptions_menu.addItem(&s_infinite_ammo_box);
	s_dmoptions_menu.addItem(&s_fixed_fov_box);
	s_dmoptions_menu.addItem(&s_quad_drop_box);
	s_dmoptions_menu.addItem(&s_friendlyfire_box);

	s_dmoptions_menu.center();

	// set the original dmflags statusbar
	//DMFlagCallback(0);
	
	s_dmoptions_menu.setStatusBar(dmoptions_statusbar);
}

static void	DMOptions_MenuDraw()
{
	s_dmoptions_menu.draw();
}

static const std::string	DMOptions_MenuKey(int key)
{
	return Default_MenuKey(&s_dmoptions_menu, key);
}

void	M_Menu_DMOptions_f()
{
	DMOptions_MenuInit();
	M_PushMenu(DMOptions_MenuDraw, DMOptions_MenuKey);
}

