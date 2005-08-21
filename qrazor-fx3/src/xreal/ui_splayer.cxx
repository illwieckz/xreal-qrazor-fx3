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

/*
static menu_framework_c	s_singleplayer_menu;
static menu_action_c		s_easy_game_action;
static menu_action_c		s_medium_game_action;
static menu_action_c		s_hard_game_action;
static menu_action_c		s_load_game_action;
static menu_action_c		s_save_game_action;
//static menu_action_c		s_credits_action;
static menu_separator_c		s_blankline;

#define NOSINGLEPLAYER

static void	StartGame()
{
#ifdef NOSINGLEPLAYER
	s_singleplayer_menu.setStatusBar("No single player support in this version");
	return;
#endif


	// disable updates and start the cinematic going
	uii.CL_SetServercount(-1);
	M_ForceMenuOff();
	uii.Cvar_SetValue("deathmatch", 0);
	uii.Cvar_SetValue("coop", 0);

	uii.Cvar_SetValue("gamerules", 0);		//PGM

	uii.Cbuf_AddText("loading ; killserver ; wait ; newgame\n");
	uii.Key_SetKeyDest(KEY_GAME);
}

static void	EasyGameFunc(void *data)
{
	uii.Cvar_ForceSet("skill", "0");
	StartGame();
}

static void	MediumGameFunc(void *data)
{
	uii.Cvar_ForceSet("skill", "1");
	StartGame();
}

static void	HardGameFunc(void *data)
{
	uii.Cvar_ForceSet("skill", "2");
	StartGame();
}

static void	LoadGameFunc(void *unused)
{
#ifdef NOSINGLEPLAYER
	s_singleplayer_menu.setStatusBar("No single player support in this version");
	return;
#endif

	M_Menu_LoadGame_f();
}

static void	SaveGameFunc(void *unused)
{
#ifdef NOSINGLEPLAYER
	s_singleplayer_menu.setStatusBar("No single player support in this version");
	return;
#endif
	
	M_Menu_SaveGame_f();
}


static void	CreditsFunc(void *unused)
{
	M_Menu_Credits_f();
}


static void	Singleplayer_MenuInit()
{
	int	y;
	int	y_offset = CHAR_MEDIUM_HEIGHT + 5;

	s_singleplayer_menu._x = (int)(uii.viddef->width * 0.50 - (CHAR_MEDIUM_WIDTH * 5));


	s_easy_game_action._flags	= QMF_LEFT_JUSTIFY;
	s_easy_game_action._fontflags	= FONT_MEDIUM | FONT_CHROME;
	s_easy_game_action._x		= 0;
	s_easy_game_action._y		= y = 0;
	s_easy_game_action._name	= "easy";
	s_easy_game_action._callback = EasyGameFunc;

	s_medium_game_action._flags	= QMF_LEFT_JUSTIFY;
	s_medium_game_action._fontflags	= FONT_MEDIUM | FONT_CHROME;
	s_medium_game_action._x		= 0;
	s_medium_game_action._y		= y += y_offset;
	s_medium_game_action._name	= "medium";
	s_medium_game_action._callback = MediumGameFunc;

	s_hard_game_action._flags	= QMF_LEFT_JUSTIFY;
	s_hard_game_action._fontflags	= FONT_MEDIUM | FONT_CHROME;
	s_hard_game_action._x		= 0;
	s_hard_game_action._y		= y += y_offset;
	s_hard_game_action._name	= "hard";
	s_hard_game_action._callback = HardGameFunc;

	//s_blankline._type = MTYPE_SEPARATOR;

	s_load_game_action._flags	= QMF_LEFT_JUSTIFY;
	s_load_game_action._fontflags	= FONT_MEDIUM | FONT_CHROME;
	s_load_game_action._x		= 0;
	s_load_game_action._y		= y += y_offset;
	s_load_game_action._name	= "load game";
	s_load_game_action._callback = LoadGameFunc;

	s_save_game_action._flags	= QMF_LEFT_JUSTIFY;
	s_save_game_action._fontflags	= FONT_MEDIUM | FONT_CHROME;
	s_save_game_action._x		= 0;
	s_save_game_action._y		= y += y_offset;
	s_save_game_action._name	= "save game";
	s_save_game_action._callback = SaveGameFunc;

	
	s_singleplayer_menu.addItem(&s_easy_game_action);
	s_singleplayer_menu.addItem(&s_medium_game_action);
	s_singleplayer_menu.addItem(&s_hard_game_action);
	s_singleplayer_menu.addItem(&s_blankline);
	s_singleplayer_menu.addItem(&s_load_game_action);
	s_singleplayer_menu.addItem(&s_save_game_action);
	s_singleplayer_menu.addItem(&s_blankline);

	s_singleplayer_menu.center();
}

static void	Singleplayer_MenuDraw()
{
	M_Banner("SINGLEPLAYER");
	
	s_singleplayer_menu.adjustCursor(1);
	s_singleplayer_menu.draw();
}

static const std::string	Singleplayer_MenuKey(int key)
{
	return Default_MenuKey(&s_singleplayer_menu, key);
}



void	M_Menu_Singleplayer_f()
{
	Singleplayer_MenuInit();
	M_PushMenu(Singleplayer_MenuDraw, Singleplayer_MenuKey);
}

*/


