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


class menu_allow_download_c : public menu_spincontrol_c
{
public:
	menu_allow_download_c()
	{
		_name		= "allow downloading";
		
		_itemnames.push_back("yes");
		_itemnames.push_back("no");
	}

	virtual void	callback()
	{
		trap_Cvar_SetValue("allow_download", _curvalue);
	}
};


class menu_allow_download_maps_c : public menu_spincontrol_c
{
public:
	menu_allow_download_maps_c()
	{
		_name		= "maps";
	
		_itemnames.push_back("yes");
		_itemnames.push_back("no");
	}

	virtual void	callback()
	{
		trap_Cvar_SetValue("allow_download_maps", _curvalue);
	}
};


class menu_allow_download_models_c : public menu_spincontrol_c
{
public:
	menu_allow_download_models_c()
	{
		_name		= "models";
		
		_itemnames.push_back("yes");
		_itemnames.push_back("no");
	}

	virtual void	callback()
	{
		trap_Cvar_SetValue("allow_download_models", _curvalue);
	}
};


class menu_allow_download_players_c : public menu_spincontrol_c
{
public:
	menu_allow_download_players_c()
	{
		_name		= "player models/skins";
	
		_itemnames.push_back("yes");
		_itemnames.push_back("no");
	}

	virtual void	callback()
	{
		trap_Cvar_SetValue("allow_download_players", _curvalue);
	}
};


class menu_allow_download_sounds_c : public menu_spincontrol_c
{
public:
	menu_allow_download_sounds_c()
	{
		_name		= "sounds";
	
		_itemnames.push_back("yes");
		_itemnames.push_back("no");
	}
	
	virtual void	callback()
	{
		trap_Cvar_SetValue("allow_download_sounds", _curvalue);
	}
};


static menu_framework_c			s_downloadoptions_menu;
static menu_separator_c				s_download_title("Download Options");
static menu_allow_download_c			s_allow_download_box;
static menu_allow_download_maps_c		s_allow_download_maps_box;
static menu_allow_download_models_c		s_allow_download_models_box;
static menu_allow_download_players_c		s_allow_download_players_box;
static menu_allow_download_sounds_c		s_allow_download_sounds_box;



static void	DownloadOptions_MenuInit()
{
	int y = 0;

	s_downloadoptions_menu._x = (int)(trap_VID_GetWidth() * 0.50);

	s_download_title._x    = 48;
	s_download_title._y	 = y;

	s_allow_download_box._x	= 0;
	s_allow_download_box._y	= y += 20;
	s_allow_download_box._curvalue = (trap_Cvar_VariableValue("allow_download") != 0);

	s_allow_download_maps_box._x	= 0;
	s_allow_download_maps_box._y	= y += 20;
	s_allow_download_maps_box._curvalue = (trap_Cvar_VariableValue("allow_download_maps") != 0);

	s_allow_download_players_box._x	= 0;
	s_allow_download_players_box._y	= y += 10;
	s_allow_download_players_box._curvalue = (trap_Cvar_VariableValue("allow_download_players") != 0);

	s_allow_download_models_box._x	= 0;
	s_allow_download_models_box._y	= y += 10;
	s_allow_download_models_box._curvalue = (trap_Cvar_VariableValue("allow_download_models") != 0);

	s_allow_download_sounds_box._x	= 0;
	s_allow_download_sounds_box._y	= y += 10;
	s_allow_download_sounds_box._curvalue = (trap_Cvar_VariableValue("allow_download_sounds") != 0);
	
	s_downloadoptions_menu.addItem(&s_download_title);
	s_downloadoptions_menu.addItem(&s_allow_download_box);
	s_downloadoptions_menu.addItem(&s_allow_download_maps_box);
	s_downloadoptions_menu.addItem(&s_allow_download_players_box);
	s_downloadoptions_menu.addItem(&s_allow_download_models_box);
	s_downloadoptions_menu.addItem(&s_allow_download_sounds_box);

	s_downloadoptions_menu.center();

	// skip over title
	if(s_downloadoptions_menu._cursor == 0)
		s_downloadoptions_menu._cursor = 1;
}

static void	DownloadOptions_MenuDraw()
{
	s_downloadoptions_menu.draw();
}

static const std::string	DownloadOptions_MenuKey(int key)
{
	return Default_MenuKey(&s_downloadoptions_menu, key);
}

void	M_Menu_DownloadOptions_f()
{
	DownloadOptions_MenuInit();
	M_PushMenu(DownloadOptions_MenuDraw, DownloadOptions_MenuKey);
}

