/// ============================================================================
/*
Copyright (C) 2004 Didier Cassirame (Tenebrae Team)
Copyright (C) 2005 Robert Beckebans <trebor_7@users.sourceforge.net>
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
#ifndef GUI_LOCAL_H
#define GUI_LOCAL_H

/// includes ===================================================================
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
// system -------------------------------------------------------------------
#include <stack>
#include <libxml/parser.h>

// shared -------------------------------------------------------------------
#include "../x_shared.h"
#include "../x_keycodes.h"

// qrazor-fx ----------------------------------------------------------------
// xreal --------------------------------------------------------------------


enum gui_align_e
{
	GUI_ALIGN_LEFT,
	GUI_ALIGN_CENTER,
	GUI_ALIGN_RIGHT,
	GUI_ALIGN_BASELINE,
	GUI_ALIGN_STRETCH
};

enum gui_pack_e
{
	GUI_PACK_LEFT,
	GUI_PACK_CENTER,
	GUI_PACK_RIGHT
};

struct gui_dim_t
{
	inline gui_dim_t()
	{
		ratio = 0.0;
		absolute = 0;
	}
	
	float		ratio;
	int		absolute;
};

struct gui_script_t
{
	std::string	str;
};


class gui_widget_c
{
public:
	gui_widget_c();
	virtual ~gui_widget_c();
	
	virtual void	load(xmlNodePtr node)	{}
	virtual void	draw(int x, int y)	{}
	virtual void	focus()			{}
	virtual bool	handleKey(int key)	{return false;}
	virtual void	drag(int x, int y)	{}
	
	int		readXmlPropAsChar(xmlNodePtr node, const std::string &name, int defvalue);
	int		readXmlPropAsInt(xmlNodePtr node, const std::string &name, int defvalue);
	float		readXmlPropAsFloat(xmlNodePtr node, const std::string &name, float defvalue);
	const char*	readXmlPropAsString(xmlNodePtr node, const std::string&name);
//	char*		readXmlPropAsRefString(xmlNodePtr node, const char *name);
	void		readXmlDim(xmlNodePtr node, const std::string &name, gui_dim_t &dim);
	int		compareXmlProp(xmlNodePtr node, const std::string &name, const char **strings, int count);
	
	void		prepareWindow();
	
	void		loadXmlElement(xmlNodePtr node);
	
	void		insertWidget(gui_widget_c *child);

	std::string			_name;			// stored in a string table
	std::string			_id;			// idem
	std::string			_tag;

	int				_font;			//font to use for this item and all sub items
	
	int				_focus_shader;
	int				_focus_type;
	
	gui_widget_c*			_parent;		// bounding element
	gui_widget_c*			_previous;		// previous sibling
	gui_widget_c*			_next;			// next sibling
	std::list<gui_widget_c*>	_children;		// bounded element list
	
	bool				_debug;
	bool				_enabled;
	bool				_focusable;
	bool				_orient;
	gui_align_e			_align;
	gui_pack_e			_pack;
	int				_xpos;			// x position of this widget (absolute screen space)
	int				_ypos;			// y position of this widget (absolute screen space) 
	int				_xoffset;		// offset where to start drawing children
	int				_yoffset;
	gui_dim_t			_width;
	gui_dim_t			_height;
	int				_accesskey;
	
	gui_script_t			_onCommand;
	gui_script_t			_onMouseOver;
	gui_script_t			_onMouseDown;
	gui_script_t			_onMouseUp;
};

/*
class gui_template_c
{
public:
	
};
*/

class gui_window_c :
public gui_widget_c
{
public:
	gui_window_c();
	
	virtual void	load(xmlNodePtr node);
	virtual void	draw(int x, int y);
	virtual bool	handleKey(int key);
	
	int				_keycount;
	gui_widget_c*			_focused;
	std::stack<gui_window_c*>	_stack;
};


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

#endif // GUI_LOCAL_H
