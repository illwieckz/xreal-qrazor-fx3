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


enum
{
	REF_ARB_GLX,
	REF_NV30_GLX
};


class menu_video_mode_c : public menu_spincontrol_c
{
public:
	virtual void	callback()
	{
		trap_Cvar_SetValue("vid_mode", _curvalue);
	}
};

class menu_video_ref_c : public menu_spincontrol_c
{
public:
	virtual void	callback()
	{
		switch(_curvalue)
		{
			case REF_ARB_GLX :
				trap_Cvar_Set("vid_ref", "arb_glx");
				break;
			
			case REF_NV30_GLX :
				trap_Cvar_Set("vid_ref", "nv30_glx");
				break;
		}
	}
};

class menu_video_screensize_c : public menu_slider_c
{
public:
	menu_video_screensize_c()
	{
		_name	= "screen size";
		
		_minvalue	= 3;
		_maxvalue	= 12;
	}

	virtual void	callback()
	{
		trap_Cvar_SetValue("cg_viewsize", _curvalue * 10);
	}
};

class menu_video_gamma_c : public menu_slider_c
{
public:
	menu_video_gamma_c()
	{
		_name	= "gamma";
		
		_minvalue	= 5;
		_maxvalue	= 13;
	}

	virtual void	callback()
	{
		// invert sense so greater = brighter, and scale to a range of 0.5 to 1.3
		float gamma = (0.8 - (_curvalue/10.0 - 0.5)) + 0.5;

		trap_Cvar_SetValue("vid_gamma", gamma);
	}
};

class menu_video_fullscreen_c : public menu_spincontrol_c
{
public:
	virtual void	callback()
	{
		trap_Cvar_SetValue("vid_fullscreen", _curvalue);
	}
};

/*
class menu_video_apply_c : public menu_action_c
{
public:
	virtual void	callback()
	{		
		//trap_Cvar_SetValue("r_picmip", 3 - s_tq_slider._curvalue);
		M_ForceMenuOff();
	}
};
*/

class menu_video_defaults_c : public menu_action_c
{
public:
	virtual void	callback()
	{
		M_VIDMenuInit();
	}
};



static menu_framework_c	s_video_menu;
static menu_video_mode_c	s_mode_list;
static menu_video_ref_c		s_ref_list;
static menu_video_screensize_c	s_screensize_slider;
static menu_video_gamma_c	s_gamma_slider;
static menu_video_fullscreen_c	s_fs_box;
//static menu_slider_c		s_tq_slider;
//static menu_spincontrol_c	s_windowed_mouse;
//static menu_video_apply_c	s_apply_action;
static menu_video_defaults_c	s_defaults_action;




void	M_VIDMenuInit()
{
	int	y;
	int	y_offset = 20;

	s_mode_list._curvalue = trap_Cvar_VariableInteger("vid_mode");

	s_screensize_slider._curvalue = trap_Cvar_VariableValue("cg_viewsize") / 10;


	if(strcmp(trap_Cvar_VariableString("vid_ref"), "arb_glx") == 0)
	{
		s_ref_list._curvalue = REF_ARB_GLX;
	}
	else if(strcmp(trap_Cvar_VariableString("vid_ref"), "nv30_glx") == 0)
	{
		s_ref_list._curvalue = REF_NV30_GLX;
	}
	
	s_video_menu._x = (int)(trap_VID_GetWidth() * 0.50) - CHAR_MEDIUM_WIDTH * 10;


	//s_ref_list[i].generic.type = MTYPE_SPINCONTROL;
	s_ref_list._flags = QMF_LEFT_JUSTIFY;
	s_ref_list._fontflags = FONT_MEDIUM;
	s_ref_list._name = "driver";
	s_ref_list._x = 0;
	s_ref_list._y = y = 0;
	s_ref_list._itemnames.push_back("[OpenGL ARB GLX]");
	s_ref_list._itemnames.push_back("[OpenGL NV30 GLX]");
	

	//s_mode_list[i].generic.type = MTYPE_SPINCONTROL;
	s_mode_list._flags = QMF_LEFT_JUSTIFY;
	s_mode_list._fontflags = FONT_MEDIUM;
	s_mode_list._name = "video mode";
	s_mode_list._x = 0;
	s_mode_list._y = y += y_offset;
	s_mode_list._itemnames.push_back("[320 240  ]");
	s_mode_list._itemnames.push_back("[300 400  ]");
	s_mode_list._itemnames.push_back("[512 384  ]");
	s_mode_list._itemnames.push_back("[640 480  ]");
	s_mode_list._itemnames.push_back("[800 600  ]");
	s_mode_list._itemnames.push_back("[960 720  ]");
	s_mode_list._itemnames.push_back("[1024 768 ]");
	s_mode_list._itemnames.push_back("[1152 864 ]");
	s_mode_list._itemnames.push_back("[1280 1024]");
	s_mode_list._itemnames.push_back("[1600 1200]");
	s_mode_list._itemnames.push_back("[2048 1536]");

	s_screensize_slider._fontflags	= FONT_MEDIUM | FONT_ALT | FONT_CHROME;
	s_screensize_slider._x		= 0;
	s_screensize_slider._y		= y += y_offset;

	s_gamma_slider._fontflags	= FONT_MEDIUM | FONT_ALT | FONT_CHROME;
	s_gamma_slider._x		= 0;
	s_gamma_slider._y		= y += y_offset;
	s_gamma_slider._curvalue	= (1.3 - trap_Cvar_VariableValue("vid_gamma") + 0.5) * 10;

	s_fs_box._fontflags	= FONT_MEDIUM;
	s_fs_box._x		= 0;
	s_fs_box._y		= y += y_offset;
	s_fs_box._name		= "fullscreen";
	s_fs_box._itemnames.push_back("no");
	s_fs_box._itemnames.push_back("yes");
	s_fs_box._curvalue = trap_Cvar_VariableInteger("vid_fullscreen");
	
	/*
	s_tq_slider._fontflags	= FONT_MEDIUM | FONT_ALT | FONT_CHROME;
	s_tq_slider._x		= 0;
	s_tq_slider._y		= y += y_offset;
	s_tq_slider._name	= "texture quality";
	s_tq_slider._minvalue = 0;
	s_tq_slider._maxvalue = 3;
	s_tq_slider._curvalue = 3 - trap_Cvar_VariableInteger("r_picmip");
	*/

	s_defaults_action._fontflags	= FONT_MEDIUM | FONT_CHROME;
	s_defaults_action._name		= "reset to default";
	s_defaults_action._x    	= 0;
	s_defaults_action._y    	= y += y_offset;

	/*
	s_apply_action._fontflags	= FONT_MEDIUM | FONT_CHROME;
	s_apply_action._name		= "apply";
	s_apply_action._x   		= 0;
	s_apply_action._y   		= y += y_offset;
	*/
	
	/*
	s_windowed_mouse._x  = 0;
	s_windowed_mouse._y  = y += y_offset;
	s_windowed_mouse._name   = "windowed mouse";
	s_windowed_mouse._curvalue = trap_Cvar_VariableInteger("_windowed_mouse");
	s_windowed_mouse._itemnames.push_back("no");
	s_windowed_mouse._itemnames.push_back("yes");
	*/
	
	s_video_menu.addItem(&s_ref_list);
	s_video_menu.addItem(&s_mode_list);
	s_video_menu.addItem(&s_screensize_slider);
	s_video_menu.addItem(&s_gamma_slider);
	s_video_menu.addItem(&s_fs_box);
//	s_video_menu.addItem(&s_tq_slider);
	s_video_menu.addItem(&s_defaults_action);
//	s_video_menu.addItem(&s_apply_action);

	s_video_menu.center();
}

void	M_VIDMenuDraw()
{
	//
	// draw the banner
	//
	M_Banner("VIDEO");

	//
	// move cursor to a reasonable starting position
	//
	s_video_menu.adjustCursor(1);

	//
	// draw the menu
	//
	s_video_menu.draw();
}

const std::string	M_VIDMenuKey(int key)
{
	
	menu_framework_c *m = &s_video_menu;

	switch(key)
	{
		case K_ESCAPE:
			M_PopMenu();
			return "";
			
		case K_UPARROW:
			m->_cursor--;
			m->adjustCursor(-1);
			break;
			
		case K_DOWNARROW:
			m->_cursor++;
			m->adjustCursor(1);
			break;
			
		case K_LEFTARROW:
			m->slideItem(-1);
			break;
			
		case K_RIGHTARROW:
			m->slideItem(1);
			break;
			
		case K_ENTER:
			m->selectItem();
			break;
	}

	return "sound/misc/menu1.wav";
}



/*
=======================================================================
				VIDEO MENU
=======================================================================
*/

void	M_Menu_Video_f()
{
	M_VIDMenuInit();
	M_PushMenu(M_VIDMenuDraw, M_VIDMenuKey);
}



