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


/// includes ===================================================================
// system -------------------------------------------------------------------
// shared -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
// xreal --------------------------------------------------------------------
#include "gui_local.h"

gui_window_c::gui_window_c()
{
	_focus_type	= 0;
	
	_parent		= NULL;
	_previous	= NULL;
	_next		= NULL;
	
	_debug		= true;
	_enabled	= true;
	_focusable	= true;
	_orient		= true;
	
	_align		= GUI_ALIGN_LEFT;
	_pack		= GUI_PACK_LEFT;
	_xpos		= 0;
	_ypos		= 0;
	_xoffset	= 0;
	_yoffset	= 0;
//	_width		= 0;
//	_height		= 0;
	_accesskey	= 0;
	
//	_onCommand;
//	_onMouseOver;
//	_onMouseDown;
//	_onMouseUp;
	
//	data->string_table.str = "";
	_keycount = 0;
	_focused = NULL;
//	_stack = NULL;
}

void	gui_window_c::load(xmlNodePtr node)
{
	/* 
	 * --- supported attributes :

	 * --- unsupported :
	   
	   screenX    =  int : window x position (persistent ?)
	   screenY    =  int : window y position (persistent ?)
	   sizemode   =  {maximized|minimized|normal} : window size state
	   title      =  string
	   windowtype =  string	   
	*/
	
	// check window size
	if(_width.ratio != -1)
		_width.absolute = (int)(_width.ratio * trap_VID_GetWidth());
		
	if(_height.ratio != -1)
		_height.absolute = (int)(_height.ratio * trap_VID_GetHeight());
	
	if(_width.absolute == -1)
		_width.absolute = trap_VID_GetWidth();
		
	if(_height.absolute == -1)
		_height.absolute = trap_VID_GetHeight();
	
	

	// add newly created window to the list
	//TODO
//	M_AddWindowInList (data);
}

void	gui_window_c::draw(int x, int y)
{
	//TODO
}

bool	gui_window_c::handleKey(int key)
{
	//TODO
	return false;
}


