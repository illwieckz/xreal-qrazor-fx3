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
#ifndef UI_LOCAL_H
#define UI_LOCAL_H

/// includes ===================================================================
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
// system -------------------------------------------------------------------
// shared --------------------------------------------------------------------
#include "../x_shared.h"
#include "../x_keycodes.h"

// xreal --------------------------------------------------------------------
#include "ui_public.h"


#define MAXMENUITEMS	64


enum menu_type_t
{
	MTYPE_SLIDER,
	MTYPE_LIST,
	MTYPE_ACTION,
	MTYPE_SPINCONTROL,
	MTYPE_SEPARATOR,
	MTYPE_FIELD
};



enum
{
	QMF_LEFT_JUSTIFY	= (1<<0),
	//QMF_GRAYED		= (1<<1),
	QMF_NUMBERSONLY		= (1<<2),
	QMF_NOITEMNAMES		= (1<<3)
	//QMF_CENTERED		= (1<<4),
	//QMF_MEDIUM		= (1<<5),
	//QMF_BIG		= (1<<6)
};

#define SLIDER_RANGE		10


class menu_common_c;

class menu_framework_c
{
public:
	menu_common_c*		getItemAtCursor();
	void			addItem(menu_common_c *item);
	void			adjustCursor(int dir);
	void			center();
	void			draw();
	void			selectItem();
	void			setStatusBar(const std::string &string);
	void			slideItem(int dir);
	int			getTallySlotsNum();
	
private:
	void			drawStatusBar(const std::string &string);

public:
	int			_x;
	int			_y;
	int			_cursor;

	//int			nitems;
	int			_nslots;
	

	std::string		_statusbar;
	
	std::vector<menu_common_c*>	_items;

	void		(*_cursordraw)(menu_framework_c *m);

	

};

class menu_common_c
{
public:
	menu_common_c()
	{
		_name		= "";
	}
	
	virtual ~menu_common_c()
	{
		//TODO
	}
	
	virtual void	doEnter()		{}
	virtual void	doSlide(int dir)	{}
	virtual void	draw() = 0;
	
	virtual void	callback()		{}
		
	menu_type_t	getType() const		{return _type;}
//	const char*	getName() const		{return _name.c_str();}

//protected:
//	void		setName(const std::string &name)	{_name = name;}
	
protected:
	menu_type_t		_type;	
public:
	std::string		_name;

	int 			_x;
	int			_y;
//	int			_mins[2];
//	int			_maxs[2];
	
	menu_framework_c*	_parent;
	int			_cursor_offset;
	int			_localdata[4];
	unsigned		_flags;
	unsigned		_fontflags;

	std::string		_statusbar;

//	void 		(*_callback)(void *self);
	void 		(*_statusbarfunc)(void *self);
	void 		(*_ownerdraw)(void *self);
	void 		(*_cursordraw)(void *self);
};

class menu_field_c : public menu_common_c
{
public:
	menu_field_c()
	{
		_type		= MTYPE_FIELD;
	}


	virtual void	doEnter();
	virtual void	draw();

	bool			key(int key);

	std::string		_buffer;
	int			_cursor;
	int			_length;
	//int			_visible_length;
	//int			_visible_offset;
};

class menu_slider_c : public menu_common_c
{
public:
	menu_slider_c()
	{
		_type		= MTYPE_SLIDER;
	}

	menu_slider_c(const std::string &name, float minvalue, float maxvalue)
	{
		_type		= MTYPE_SLIDER;
		_name		= name;
		
		_minvalue	= minvalue;
		_maxvalue	= maxvalue;
	}

	menu_slider_c(float minvalue, float maxvalue)
	{
		_type		= MTYPE_SLIDER;
		
		_minvalue	= minvalue;
		_maxvalue	= maxvalue;
	}
	
	virtual void	doSlide(int dir);
	virtual void	draw();

protected:
	float			_minvalue;
	float			_maxvalue;
public:
	float			_curvalue;
};

class menu_list_c : public menu_common_c
{
public:
	menu_list_c()
	{
		_type		= MTYPE_LIST;
	}

	virtual void	draw();

//protected:
	std::vector<std::string>	_itemnames;
public:
	int				_curvalue;
};

class menu_spincontrol_c : public menu_list_c
{
public:
	menu_spincontrol_c()
	{
		_type		= MTYPE_SPINCONTROL;
	}
	
	virtual void	doEnter();
	virtual void	doSlide(int dir);
	virtual void	draw();
		
};

class menu_action_c : public menu_common_c
{
public:
	menu_action_c()
	{
		_type		= MTYPE_ACTION;
	}
	
	virtual void	doEnter();
	virtual void	draw();
};

class menu_separator_c : public menu_common_c
{
public:
	menu_separator_c()
	{
		_type		= MTYPE_SEPARATOR;
	}

	menu_separator_c(const std::string &name)
	{
		_type		= MTYPE_SEPARATOR;
		_name		= name;
	}
	
	virtual void	draw();
};


extern ui_import_t	uii;
extern ui_export_t	ui_globals;

extern const char*	menu_in_sound;
extern const char*	menu_move_sound;
extern const char*	menu_out_sound;



extern menu_framework_c	s_multiplayer_menu;


void	M_Menu_Main_f();
//	void M_Menu_Singleplayer_f();
//		void M_Menu_LoadGame_f();
//		void M_Menu_SaveGame_f();
	void M_Menu_Multiplayer_f();
		void M_Menu_JoinServer_f();
//			void M_Menu_AddressBook_f();
//		void M_Menu_StartServer_f();
//			void M_Menu_DMOptions_f();
		void M_Menu_PlayerConfig_f();
			void M_Menu_DownloadOptions_f();
	void M_Menu_Audio_f();
	void M_Menu_Video_f();
	void M_Menu_Options_f();
		void M_Menu_Keys_f();
	void M_Menu_Credits_f();
	void M_Menu_Quit_f();


//
// ui_main.c
//


//
// ui_menu.c
//
void		M_Banner(const std::string &name);
void		M_Init();
void		M_Keydown(int key);
void		M_Draw();
void		M_PushMenu(void (*draw)(), const std::string (*key)(int k));
void		M_PopMenu();
void		M_ForceMenuOff();
const std::string	Default_MenuKey(menu_framework_c *m, int key);
void		M_AddToServerList(const netadr_t &adr, const char *info);
void		M_Print(int cx, int cy, char *str);
void		M_DrawPic(int x, int y, char *pic);
void		M_DrawCursor(int x, int y, int f);
void		M_DrawTextBox(int x, int y, int width, int lines);

void		M_VIDMenuInit();
void		M_VIDMenuDraw();



//
// ui_utils.c
//
int	Menu_GetCharWidth(int flags);
void	Menu_DrawChar(int x, int y, int num, const vec4_c &color, int flags);
void	Menu_DrawString(int, int, const std::string &string, int flags);


#endif
