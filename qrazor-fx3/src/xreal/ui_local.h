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
#include "gui_public.h"


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
	inline menu_framework_c()
	{
		_x	= 0;
		_y	= 0;
	}
	
	virtual ~menu_framework_c()
	{
	}

	virtual void		draw()
	{
		adjustCursor(1);
		drawGeneric();
	}
	virtual std::string	keyDown(int key)
	{
		return defaultKeyDown(key);
	}

	menu_common_c*		getItemAtCursor();
	void			addItem(menu_common_c *item);
	void			adjustCursor(int dir);
	void			center();
	void			drawGeneric();
	void			selectItem();
	void			setStatusBar(const std::string &string);
	void			slideItem(int dir);
	int			getTallySlotsNum();
	std::string		defaultKeyDown(int key);
	
private:
	void			drawStatusBar(const std::string &string);

public:
	int			_x;
	int			_y;
	int			_cursor;

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


extern const char*	menu_in_sound;
extern const char*	menu_move_sound;
extern const char*	menu_out_sound;


void	M_Menu_Main_f();
//	void M_Menu_Singleplayer_f();
//		void M_Menu_LoadGame_f();
//		void M_Menu_SaveGame_f();
	void M_Menu_Multiplayer_f();
		void M_Menu_JoinServer_f();
//			void M_Menu_AddressBook_f();
//		void M_Menu_StartServer_f();
//			void M_Menu_DMOptions_f();
//		void M_Menu_PlayerConfig_f();
			void M_Menu_DownloadOptions_f();
	void M_Menu_Audio_f();
	void M_Menu_Video_f();
	void M_Menu_Options_f();
	void M_Menu_Keys_f();
	void M_Menu_Credits_f();
	void M_Menu_Quit_f();


//
// ui_main.c
//virtual std::string	keyDown(int key)


//
// ui_menu.c
//
void		M_Banner(const std::string &name);
void		M_Init();
void		M_Keydown(int key);
void		M_Draw();
void		M_PushMenu(menu_framework_c *menu);
void		M_PopMenu();
void		M_ForceMenuOff();
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


//
// ui_syscalls.cxx
//
void 		trap_Com_Printf(const char *fmt, ...);
void 		trap_Com_DPrintf(const char *fmt, ...);
void 		trap_Com_Error(err_type_e type, const char *fmt, ...);
int		trap_Com_ServerState();

void 		trap_Cbuf_AddText(const std::string &text);
void 		trap_Cbuf_InsertText(const std::string &text);
void 		trap_Cbuf_ExecuteText(exec_type_e, const std::string &text);
void 		trap_Cbuf_Execute();
	
void		trap_Cmd_AddCommand(const std::string &name, void(*cmd)());
void		trap_Cmd_RemoveCommand(const std::string &name);
int		trap_Cmd_Argc();
const char*	trap_Cmd_Argv(int i);	

cvar_t*		trap_Cvar_Get(const std::string &name, const std::string &value, uint_t flags);
cvar_t*		trap_Cvar_Set(const std::string &name, const std::string &value);
cvar_t*		trap_Cvar_ForceSet(const std::string &name, const std::string &value);
void		trap_Cvar_SetValue(const std::string &name, float value);
void		trap_Cvar_SetModified(const std::string &name);
float		trap_Cvar_VariableValue(const std::string &name);
int		trap_Cvar_VariableInteger(const std::string &name);
const char*	trap_Cvar_VariableString(const std::string &name);
float		trap_Cvar_ClampVariable(const std::string &name, float min, float max);
	
std::string	trap_VFS_Gamedir();
std::vector<std::string>	trap_VFS_ListFiles(const std::string &dir, const std::string &extension);
	
int		trap_VFS_FLoad(const std::string &name, void **buf);
void		trap_VFS_FFree(void *buf);
		
int		trap_R_RegisterPic(const std::string &name);
void		trap_R_DrawPic(int x, int y, int w, int h, const vec4_c &color, int shader);
void		trap_R_DrawStretchPic(int x, int y, int w, int h, float s1, float t1, float s2, float t2, const vec4_c &color, int shader);
void		trap_R_DrawFill(int x, int y, int w, int h, const vec4_c &color);
	
char*		trap_Key_KeynumToString(int keynum);
char*		trap_Key_GetBinding(int keynum);
void 		trap_Key_SetBinding(int keynum, char *binding);
void 		trap_Key_ClearStates();
keydest_t	trap_Key_GetKeyDest();
void		trap_Key_SetKeyDest(keydest_t key_dest);
	
void		trap_Con_ClearNotify();
void		trap_Con_ClearTyping();

float		trap_CL_GetRealtime();
bool		trap_CL_GetAttractloop();
void 		trap_CL_Quit_f();
void 		trap_CL_PingServers_f();
void		trap_CL_SetServercount(int count);

void		trap_S_StartLocalSound(const std::string &name);

uint_t		trap_VID_GetWidth();
uint_t		trap_VID_GetHeight();
	
char*		trap_Sys_AdrToString(const netadr_t &a);
int		trap_Sys_Milliseconds();


#endif
