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


#define MAX_LOCAL_SERVERS 8

int		m_num_servers;
#define	NO_SERVER_STRING	"<no server>"

// user readable information
static char local_server_names[MAX_LOCAL_SERVERS][80];

// network address
static netadr_t local_server_netadr[MAX_LOCAL_SERVERS];


static void	SearchLocalGames()
{
	m_num_servers = 0;
	
	for(int i=0; i<MAX_LOCAL_SERVERS; i++)
		strcpy(local_server_names[i], NO_SERVER_STRING);

	M_DrawTextBox( 8, 120 - 48, 36, 3 );
	M_Print( 16 + 16, 120 - 48 + 8,  "Searching for local servers, this" );
	M_Print( 16 + 16, 120 - 48 + 16, "could take up to a minute, so" );
	M_Print( 16 + 16, 120 - 48 + 24, "please be patient." );

	// the text box won't show up unless we do a buffer swap
	//uii.R_EndFrame();

	// send out info packets
	uii.CL_PingServers_f();
}

class menu_joinserver_address_book_c : public menu_action_c
{
public:
	menu_joinserver_address_book_c()
	{
		_name		= "address book";
	}

	virtual void	callback()
	{
//		M_Menu_AddressBook_f();
	}	
};


class menu_joinserver_search_c : public menu_action_c
{
public:
	menu_joinserver_search_c()
	{
		_name		= "refresh server list";
	}

	virtual void	callback()
	{
		SearchLocalGames();
	}	
};


class menu_joinserver_connect_c : public menu_action_c
{
public:
	menu_joinserver_connect_c(int index)
	{
		_name		= "<no server>";
		_index		= index;
	}

	virtual void	callback()
	{
		char	buffer[128];

		if(X_stricmp(local_server_names[_index], NO_SERVER_STRING ) == 0)
			return;

		if(_index >= m_num_servers)
			return;

		Com_sprintf(buffer, sizeof(buffer), "connect %s\n", uii.Sys_AdrToString(local_server_netadr[_index]));
		uii.Cbuf_AddText(buffer);
		M_ForceMenuOff();
	}
private:
	int	_index;
};

static menu_framework_c			s_joinserver_menu;
static menu_separator_c			s_joinserver_server_title("connect to...");
						
static menu_joinserver_search_c		s_joinserver_search_action;

static menu_joinserver_address_book_c	s_joinserver_address_book_action;
static menu_joinserver_connect_c		s_joinserver_connect0(0);
static menu_joinserver_connect_c		s_joinserver_connect1(1);
static menu_joinserver_connect_c		s_joinserver_connect2(2);
static menu_joinserver_connect_c		s_joinserver_connect3(3);
static menu_joinserver_connect_c		s_joinserver_connect4(4);
static menu_joinserver_connect_c		s_joinserver_connect5(5);
static menu_joinserver_connect_c		s_joinserver_connect6(6);
static menu_joinserver_connect_c		s_joinserver_connect7(7);



void	M_AddToServerList(const netadr_t &adr, const char *info)
{
	int		i;

	if (m_num_servers == MAX_LOCAL_SERVERS)
		return;
	while ( *info == ' ' )
		info++;

	// ignore if duplicated
	for (i=0 ; i<m_num_servers ; i++)
		if (!strcmp(info, local_server_names[i]))
			return;

	local_server_netadr[m_num_servers] = adr;
	strncpy (local_server_names[m_num_servers], info, sizeof(local_server_names[0])-1);
	m_num_servers++;
}



static void	JoinServer_MenuInit()
{
	s_joinserver_menu._x = (int)(uii.viddef->width * 0.50 - 120);
	
	s_joinserver_address_book_action._flags	= QMF_LEFT_JUSTIFY;
	s_joinserver_address_book_action._x		= 0;
	s_joinserver_address_book_action._y		= 0;

	s_joinserver_search_action._flags	= QMF_LEFT_JUSTIFY;
	s_joinserver_search_action._x	= 0;
	s_joinserver_search_action._y	= 10;
	s_joinserver_search_action._statusbar = "search for servers";

	s_joinserver_server_title._x    = 80;
	s_joinserver_server_title._y	= 30;

	for(int i=0; i<8; i++)
		strcpy(local_server_names[i], NO_SERVER_STRING);

	s_joinserver_connect0._flags	= QMF_LEFT_JUSTIFY;
	s_joinserver_connect0._x		= 0;
	s_joinserver_connect0._y		= 40 + 0*10;
	s_joinserver_connect0._statusbar = "press ENTER to connect";
	
	s_joinserver_connect1._flags	= QMF_LEFT_JUSTIFY;
	s_joinserver_connect1._x		= 0;
	s_joinserver_connect1._y		= 40 + 1*10;
	s_joinserver_connect1._statusbar = "press ENTER to connect";
	
	s_joinserver_connect2._flags	= QMF_LEFT_JUSTIFY;
	s_joinserver_connect2._x		= 0;
	s_joinserver_connect2._y		= 40 + 2*10;
	s_joinserver_connect2._statusbar = "press ENTER to connect";
	
	s_joinserver_connect3._flags	= QMF_LEFT_JUSTIFY;
	s_joinserver_connect3._x		= 0;
	s_joinserver_connect3._y		= 40 + 3*10;
	s_joinserver_connect3._statusbar = "press ENTER to connect";
	
	s_joinserver_connect4._flags	= QMF_LEFT_JUSTIFY;
	s_joinserver_connect4._x		= 0;
	s_joinserver_connect4._y		= 40 + 4*10;
	s_joinserver_connect4._statusbar = "press ENTER to connect";
	
	s_joinserver_connect5._flags	= QMF_LEFT_JUSTIFY;
	s_joinserver_connect5._x		= 0;
	s_joinserver_connect5._y		= 40 + 5*10;
	s_joinserver_connect5._statusbar = "press ENTER to connect";
	
	s_joinserver_connect6._flags	= QMF_LEFT_JUSTIFY;
	s_joinserver_connect6._x		= 0;
	s_joinserver_connect6._y		= 40 + 6*10;
	s_joinserver_connect6._statusbar = "press ENTER to connect";
	
	s_joinserver_connect7._flags	= QMF_LEFT_JUSTIFY;
	s_joinserver_connect7._x		= 0;
	s_joinserver_connect7._y		= 40 + 7*10;
	s_joinserver_connect7._statusbar = "press ENTER to connect";

	s_joinserver_menu.addItem(&s_joinserver_address_book_action);
	s_joinserver_menu.addItem(&s_joinserver_server_title);
	s_joinserver_menu.addItem(&s_joinserver_search_action);

	s_joinserver_menu.addItem(&s_joinserver_connect0);
	s_joinserver_menu.addItem(&s_joinserver_connect1);
	s_joinserver_menu.addItem(&s_joinserver_connect2);
	s_joinserver_menu.addItem(&s_joinserver_connect3);
	s_joinserver_menu.addItem(&s_joinserver_connect4);
	s_joinserver_menu.addItem(&s_joinserver_connect5);
	s_joinserver_menu.addItem(&s_joinserver_connect6);
	s_joinserver_menu.addItem(&s_joinserver_connect7);

	s_joinserver_menu.center();

	SearchLocalGames();
}

static void	JoinServer_MenuDraw()
{
	M_Banner("textues/pics/m_banner_join_server.pcx");
	
	s_joinserver_menu.draw();
}


static const std::string	JoinServer_MenuKey(int key)
{
	return Default_MenuKey(&s_joinserver_menu, key);
}

void	M_Menu_JoinServer_f ()
{
	JoinServer_MenuInit();
	M_PushMenu(JoinServer_MenuDraw, JoinServer_MenuKey);
}

