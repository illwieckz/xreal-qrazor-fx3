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
#define	MAX_SAVEGAMES	15

static menu_framework_c	s_savegame_menu;
static menu_action_c	s_savegame_actions[MAX_SAVEGAMES];

static menu_framework_c	s_loadgame_menu;
static menu_action_c	s_loadgame_actions[MAX_SAVEGAMES];

char	m_savestrings[MAX_SAVEGAMES][32];
bool	m_savevalid[MAX_SAVEGAMES];

static void	Create_Savestrings()
{
	int		i;
	FILE	*f;
	std::string		name;

	for (i=0 ; i<MAX_SAVEGAMES ; i++)
	{
		name = uii.VFS_Gamedir() + "/save/save" + va("%i", i) + "/server.ssv";
		f = fopen(name.c_str(), "rb");
		if (!f)
		{
			strcpy (m_savestrings[i], "<EMPTY>");
			m_savevalid[i] = false;
		}
		else
		{
			//FS_Read (m_savestrings[i], sizeof(m_savestrings[i]), f);
			fread(m_savestrings[i], 1, sizeof(m_savestrings[i]), f);
			fclose (f);
			m_savevalid[i] = true;
		}
	}
}

static void	LoadGameCallback(void *self)
{
	menu_action_c *a = ( menu_action_c * ) self;

	if ( m_savevalid[ a->_localdata[0] ] )
		uii.Cbuf_AddText (va("load save%i\n",  a->_localdata[0] ) );
	M_ForceMenuOff ();
}

static void	LoadGame_MenuInit()	
{
	int i;

	s_loadgame_menu._x = uii.viddef->width / 2 - 120;
	s_loadgame_menu._y = uii.viddef->height / 2 - 58;
	//s_loadgame_menu.nitems = 0;

	Create_Savestrings();

	for ( i = 0; i < MAX_SAVEGAMES; i++ )
	{
		s_loadgame_actions[i]._name			= m_savestrings[i];
		s_loadgame_actions[i]._flags			= QMF_LEFT_JUSTIFY;
		s_loadgame_actions[i]._localdata[0]	= i;
		s_loadgame_actions[i]._callback		= LoadGameCallback;

		s_loadgame_actions[i]._x = 0;
		s_loadgame_actions[i]._y = ( i ) * 10;
		if (i>0)	// separate from autosave
			s_loadgame_actions[i]._y += 10;

		//s_loadgame_actions[i]._type = MTYPE_ACTION;

		s_loadgame_menu.addItem(&s_loadgame_actions[i]);
	}
}

static void	LoadGame_MenuDraw()
{
	M_Banner( "textures/pics/m_banner_load_game.pcx" );

	s_loadgame_menu.draw();
}

static const std::string	LoadGame_MenuKey(int key)
{
	if ( key == K_ESCAPE || key == K_ENTER )
	{
		s_savegame_menu._cursor = s_loadgame_menu._cursor - 1;
		if ( s_savegame_menu._cursor < 0 )
			s_savegame_menu._cursor = 0;
	}
	return Default_MenuKey( &s_loadgame_menu, key );
}

void	M_Menu_LoadGame_f()
{
	LoadGame_MenuInit();
	M_PushMenu(LoadGame_MenuDraw, LoadGame_MenuKey);
}





static void	SaveGameCallback(void *self)
{
	menu_action_c *a = ( menu_action_c * ) self;

	uii.Cbuf_AddText (va("save save%i\n", a->_localdata[0] ));
	M_ForceMenuOff();
}

static void	SaveGame_MenuDraw()
{
	M_Banner( "textures/pics/m_banner_save_game.pcx" );
	
	s_savegame_menu.adjustCursor(1);
	s_savegame_menu.draw();
}

static void	SaveGame_MenuInit()
{
	int i;

	s_savegame_menu._x = uii.viddef->width / 2 - 120;
	s_savegame_menu._y = uii.viddef->height / 2 - 58;

	Create_Savestrings();

	// don't include the autosave slot
	for ( i = 0; i < MAX_SAVEGAMES-1; i++ )
	{
		s_savegame_actions[i]._name = m_savestrings[i+1];
		s_savegame_actions[i]._localdata[0] = i+1;
		s_savegame_actions[i]._flags = QMF_LEFT_JUSTIFY;
		s_savegame_actions[i]._callback = SaveGameCallback;

		s_savegame_actions[i]._x = 0;
		s_savegame_actions[i]._y = ( i ) * 10;

		//s_savegame_actions[i]._type = MTYPE_ACTION;

		s_savegame_menu.addItem(&s_savegame_actions[i]);
	}
}

static const std::string	SaveGame_MenuKey(int key)
{
	if ( key == K_ENTER || key == K_ESCAPE )
	{
		s_loadgame_menu._cursor = s_savegame_menu._cursor - 1;
		if ( s_loadgame_menu._cursor < 0 )
			s_loadgame_menu._cursor = 0;
	}
	return Default_MenuKey( &s_savegame_menu, key );
}

void	M_Menu_SaveGame_f()
{
	if(!uii.Com_ServerState())
		return;		// not playing a game

	SaveGame_MenuInit();
	M_PushMenu(SaveGame_MenuDraw, SaveGame_MenuKey);
	Create_Savestrings();
}
*/

