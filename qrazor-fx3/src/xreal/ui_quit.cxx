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


class menu_quit_framework_c : public menu_framework_c
{
public:
	virtual std::string	keyDown(int key)
	{
		switch(key)
		{
			case K_ESCAPE:
			case 'n':
			case 'N':
				M_PopMenu();
				break;

			case 'Y':
			case 'y':
				trap_Key_SetKeyDest(KEY_CONSOLE);
				trap_CL_Quit_f();
				break;
				
			case K_UPARROW:
				_cursor--;
				adjustCursor(-1);
				break;
			
			case K_DOWNARROW:
				_cursor++;
				adjustCursor(1);
				break;
		
			case K_ENTER:
				selectItem();
				break;
			
			default:
				break;
		}

		return "";
	}
};


class menu_quit_yes_c : public menu_action_c
{
public:
	virtual void	callback()
	{
		trap_Key_SetKeyDest(KEY_CONSOLE);
		trap_CL_Quit_f();
	}
};

class menu_quit_no_c : public menu_action_c
{
public:
	virtual void	callback()
	{
		M_PopMenu();
	}
};



static menu_quit_framework_c	s_quit_menu;
static menu_separator_c			s_quit_question;
static menu_quit_yes_c			s_quit_yes_action;
static menu_quit_no_c			s_quit_no_action;

static void	M_QuitInit()
{
	s_quit_menu._x = trap_VID_GetWidth() / 2;
	
	s_quit_question._flags		= QMF_LEFT_JUSTIFY;
	s_quit_question._fontflags	= FONT_MEDIUM | FONT_ALT | FONT_CHROME;
	s_quit_question._x		= -100;
	s_quit_question._y		= 0;
	s_quit_question._name		= "Are you sure?";
	
	s_quit_yes_action._flags	= QMF_LEFT_JUSTIFY;
	s_quit_yes_action._fontflags	= FONT_MEDIUM | FONT_CHROME;
	s_quit_yes_action._x		= 0;
	s_quit_yes_action._y		= 20;
	s_quit_yes_action._name		= "yes";
	
	s_quit_no_action._flags		= QMF_LEFT_JUSTIFY;
	s_quit_no_action._fontflags	= FONT_MEDIUM | FONT_CHROME;
	s_quit_no_action._x		= 0;
	s_quit_no_action._y		= 40;
	s_quit_no_action._name		= "no";
	
	s_quit_menu.addItem(&s_quit_question);
	s_quit_menu.addItem(&s_quit_yes_action);
	s_quit_menu.addItem(&s_quit_no_action);
	
	s_quit_menu.center();
}

void	M_Menu_Quit_f()
{
	M_QuitInit();
	M_PushMenu(&s_quit_menu);
}

