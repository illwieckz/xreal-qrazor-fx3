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
static menu_framework_c s_startserver_menu;

static menu_action_c		s_startserver_start_action;
static menu_action_c		s_startserver_dmoptions_action;
static menu_field_c		s_timelimit_field;
static menu_field_c		s_fraglimit_field;
static menu_field_c		s_maxclients_field;
static menu_field_c		s_hostname_field;
static menu_spincontrol_c	s_startmap_list;
static menu_spincontrol_c	s_rules_box;

static std::vector<std::string>	s_mapnames;
//static int			s_nummaps = 0;


static void	DMOptionsFunc(void *self)
{
	if(s_rules_box._curvalue == 1)
		return;
		
	M_Menu_DMOptions_f();
}

static void	RulesChangeFunc(void *self)
{
	if (s_rules_box._curvalue == 0)
	{
		s_maxclients_field._statusbar = "";
		s_startserver_dmoptions_action._statusbar = "";
	}
	else if(s_rules_box._curvalue == 1)		// coop
	{
		s_maxclients_field._statusbar = "4 maximum for cooperative";
		if (atoi(s_maxclients_field._buffer.c_str()) > 4)
			s_maxclients_field._buffer = "4";
		s_startserver_dmoptions_action._statusbar = "N/A for cooperative";
	}
}
*/

/*
static void	MapChangeFunc(void *self)
{
	//TODO
}
*/

/*
static void	StartServerActionFunc(void *self)
{
	char	startmap[1024];
	int		timelimit;
	int		fraglimit;
	int		maxclients;
	char	*spot;

	strcpy(startmap, strchr(s_mapnames[s_startmap_list._curvalue].c_str(), '\n') + 1);

	maxclients  	= atoi(s_maxclients_field._buffer.c_str());
	timelimit	= atoi(s_timelimit_field._buffer.c_str());
	fraglimit	= atoi(s_fraglimit_field._buffer.c_str());

	uii.Cvar_SetValue("maxclients", uii.Cvar_ClampVariable(0, maxclients, maxclients));
	uii.Cvar_SetValue("timelimit", uii.Cvar_ClampVariable(0, timelimit, timelimit));
	uii.Cvar_SetValue("fraglimit", uii.Cvar_ClampVariable(0, fraglimit, fraglimit));
	uii.Cvar_Set("hostname", s_hostname_field._buffer);
//	Cvar_SetValue ("deathmatch", !s_rules_box_curvalue );
//	Cvar_SetValue ("coop", s_rules_box_curvalue );

	uii.Cvar_SetValue ("deathmatch", 1 );	// deathmatch is always true for rogue games, right?
	uii.Cvar_SetValue ("coop", 0 );			// FIXME - this might need to depend on which game we're running
	uii.Cvar_SetValue ("gamerules", s_rules_box._curvalue );
	

	spot = NULL;
	
	if(s_rules_box._curvalue == 1)		// PGM
	{
 		if(X_stricmp(startmap, "bunk1") == 0)
  			spot = "start";
 		else if(X_stricmp(startmap, "mintro") == 0)
  			spot = "start";
 		else if(X_stricmp(startmap, "fact1") == 0)
  			spot = "start";
 		else if(X_stricmp(startmap, "power1") == 0)
  			spot = "pstart";
 		else if(X_stricmp(startmap, "biggun") == 0)
  			spot = "bstart";
 		else if(X_stricmp(startmap, "hangar1") == 0)
  			spot = "unitstart";
 		else if(X_stricmp(startmap, "city1") == 0)
  			spot = "unitstart";
 		else if(X_stricmp(startmap, "boss1") == 0)
			spot = "bosstart";
	}

	if(spot)
	{
		if (uii.Com_ServerState())
			uii.Cbuf_AddText("disconnect\n");
			
		uii.Cbuf_AddText(va("gamemap \"*%s$%s\"\n", startmap, spot));
	}
	else
	{
		uii.Cbuf_AddText(va("map %s\n", startmap));
	}

	M_ForceMenuOff();
}
*/

/*
static void	StartServer_MenuInit()
{
	//char *buffer;
	//char *s;
	//int length;
	//int i;

	//
	// load the list of map names
	//
		
	//if((length = uii.VFS_FLoad( "maps.lst", (void**) &buffer)) == -1)
	//		uii.Com_Error( ERR_DROP, "couldn't find maps.lst\n");
	
	//s = buffer;
	
	i = 0;
	while ( i < length )
	{
		if ( s[i] == '\r' )
			s_nummaps++;
		i++;
	}

	if ( s_nummaps == 0 )
		uii.Com_Error( ERR_DROP, "no maps in maps.lst\n" );

	s_mapnames.clear();

	s = buffer;

	for ( i = 0; i < s_nummaps; i++ )
	{
    		char  shortname[MAX_TOKEN_CHARS];
    		char  longname[MAX_TOKEN_CHARS];
		char  scratch[200];
		int		j, l;

		strcpy( shortname, Com_Parse( &s ) );
		l = strlen(shortname);
		for (j=0 ; j<l ; j++)
			shortname[j] = toupper(shortname[j]);
		strcpy( longname, Com_Parse( &s ) );
		Com_sprintf( scratch, sizeof( scratch ), "%s\n%s", longname, shortname );

		s_mapnames.push_back(scratch);
	}

	uii.VFS_FFree(buffer);
	

	//
	// initialize the menu stuff
	//
	s_startserver_menu._x = (int)(uii.viddef->width * 0.50);

	//s_startmap_list._type = MTYPE_SPINCONTROL;
	s_startmap_list._x	= 0;
	s_startmap_list._y	= 0;
	s_startmap_list._name	= "initial map";
	s_startmap_list._itemnames = s_mapnames;

	//s_rules_box._type = MTYPE_SPINCONTROL;
	s_rules_box._x	= 0;
	s_rules_box._y	= 20;
	s_rules_box._name	= "rules";

	s_rules_box._itemnames.push_back("deathmatch");
	s_rules_box._itemnames.push_back("cooperative");


	if (uii.Cvar_VariableValue("coop"))
		s_rules_box._curvalue = 1;
	else
		s_rules_box._curvalue = 0;
	s_rules_box._callback = RulesChangeFunc;

	//s_timelimit_field._type = MTYPE_FIELD;
	s_timelimit_field._name = "time limit";
	s_timelimit_field._flags = QMF_NUMBERSONLY;
	s_timelimit_field._x	= 0;
	s_timelimit_field._y	= 36;
	s_timelimit_field._statusbar = "0 = no limit";
	s_timelimit_field._length = 3;
	//s_timelimit_field._visible_length = 3;
	s_timelimit_field._buffer = uii.Cvar_VariableString("timelimit");

	//s_fraglimit_field._type = MTYPE_FIELD;
	s_fraglimit_field._name = "frag limit";
	s_fraglimit_field._flags = QMF_NUMBERSONLY;
	s_fraglimit_field._x	= 0;
	s_fraglimit_field._y	= 54;
	s_fraglimit_field._statusbar = "0 = no limit";
	s_fraglimit_field._length = 3;
	//s_fraglimit_field._visible_length = 3;
	s_fraglimit_field._buffer = uii.Cvar_VariableString("fraglimit");

	//
	// maxclients determines the maximum number of players that can join
	// the game.  If maxclients is only "1" then we should default the menu
	// option to 8 players, otherwise use whatever its current value is. 
	// Clamping will be done when the server is actually started.
	//
	//s_maxclients_field._type = MTYPE_FIELD;
	s_maxclients_field._name = "max players";
	s_maxclients_field._flags = QMF_NUMBERSONLY;
	s_maxclients_field._x	= 0;
	s_maxclients_field._y	= 72;
	s_maxclients_field._statusbar = "";
	s_maxclients_field._length = 3;
	//s_maxclients_field._visible_length = 3;
	if ( uii.Cvar_VariableValue( "maxclients" ) == 1 )
		 s_maxclients_field._buffer = "8";
	else 
		s_maxclients_field._buffer = uii.Cvar_VariableString("maxclients");

	//s_hostname_field._type = MTYPE_FIELD;
	s_hostname_field._name = "hostname";
	s_hostname_field._flags = 0;
	s_hostname_field._x	= 0;
	s_hostname_field._y	= 90;
	s_hostname_field._statusbar = "";
	s_hostname_field._length = 12;
	//s_hostname_field._visible_length = 12;
	s_hostname_field._buffer = uii.Cvar_VariableString("hostname");

	//s_startserver_dmoptions_action._type = MTYPE_ACTION;
	s_startserver_dmoptions_action._name	= " deathmatch flags";
	s_startserver_dmoptions_action._flags= QMF_LEFT_JUSTIFY;
	s_startserver_dmoptions_action._x	= 24;
	s_startserver_dmoptions_action._y	= 108;
	s_startserver_dmoptions_action._statusbar = "";
	s_startserver_dmoptions_action._callback = DMOptionsFunc;

	//s_startserver_start_action._type = MTYPE_ACTION;
	s_startserver_start_action._name	= " begin";
	s_startserver_start_action._flags= QMF_LEFT_JUSTIFY;
	s_startserver_start_action._x	= 24;
	s_startserver_start_action._y	= 128;
	s_startserver_start_action._callback = StartServerActionFunc;

	s_startserver_menu.addItem(&s_startmap_list);
	s_startserver_menu.addItem(&s_rules_box);
	s_startserver_menu.addItem(&s_timelimit_field);
	s_startserver_menu.addItem(&s_fraglimit_field);
	s_startserver_menu.addItem(&s_maxclients_field);
	s_startserver_menu.addItem(&s_hostname_field);
	s_startserver_menu.addItem(&s_startserver_dmoptions_action);
	s_startserver_menu.addItem(&s_startserver_start_action);

	s_startserver_menu.center();

	// call this now to set proper inital state
	RulesChangeFunc(NULL);
}

static void	StartServer_MenuDraw()
{
	s_startserver_menu.draw();
}

const std::string	StartServer_MenuKey(int key)
{
	if(key == K_ESCAPE)
	{
		// clean up
		s_mapnames.clear();
	}

	return Default_MenuKey(&s_startserver_menu, key);
}

void	M_Menu_StartServer_f()
{
	StartServer_MenuInit();
	M_PushMenu(StartServer_MenuDraw, StartServer_MenuKey);
}
*/

