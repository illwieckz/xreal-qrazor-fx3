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


class menu_mplayer_framework_c : public menu_framework_c
{
public:
	virtual void	draw()
	{
		M_Banner("MULTIPLAYER");
	
		adjustCursor(1);
		drawGeneric();
	}
	
	virtual std::string	keyDown(int key)
	{
		return defaultKeyDown(key);
	}
};

class menu_mplayer_join_network_c : public menu_action_c
{
public:
	virtual void	callback()
	{
		M_Menu_JoinServer_f();
	}
};

/*
class menu_mplayer_player_setup_c : public menu_action_c
{
public:
	virtual void	callback()
	{
		M_Menu_PlayerConfig_f();
	}
};
*/

static menu_mplayer_framework_c			s_multiplayer_menu;
static menu_mplayer_join_network_c		s_join_network_server_action;
//static menu_mplayer_player_setup_c		s_player_setup_action;


static void	Multiplayer_MenuInit()
{
	int	y;
//	int	y_offset = CHAR_MEDIUM_HEIGHT + 5;

	s_multiplayer_menu._x = (int)(trap_VID_GetWidth() * 0.50 - (CHAR_MEDIUM_WIDTH * 10));

	s_join_network_server_action._flags 	= QMF_LEFT_JUSTIFY;
	s_join_network_server_action._fontflags	= FONT_MEDIUM | FONT_CHROME;
	s_join_network_server_action._x		= 0;
	s_join_network_server_action._y		= y = 0;
	s_join_network_server_action._name	= "join network server";

//	s_player_setup_action._flags		= QMF_LEFT_JUSTIFY;
//	s_player_setup_action._fontflags	= FONT_MEDIUM | FONT_CHROME;
//	s_player_setup_action._x		= 0;
//	s_player_setup_action._y		= y += y_offset;
//	s_player_setup_action._name		= "player setup";

	s_multiplayer_menu.addItem(&s_join_network_server_action);
//	s_multiplayer_menu.addItem(&s_player_setup_action);

	s_multiplayer_menu.center();
}

void	M_Menu_Multiplayer_f()
{
	Multiplayer_MenuInit();
	M_PushMenu(&s_multiplayer_menu);
}

