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
#include <ctype.h>
#ifdef _WIN32
#include <io.h>
#endif

#include <stack>

// xreal --------------------------------------------------------------------
#include "ui_local.h"


static std::stack<menu_framework_c*>	m_layers;


void	M_Banner(const std::string &name)
{
	// center the string on the top
	int x = trap_VID_GetWidth()/2 - (CHAR_BIG_WIDTH * (name.length()/2));
	int y = trap_VID_GetHeight()/2 - 160;
	
	Menu_DrawString(x, y, name, FONT_BIG | FONT_CHROME);
}

void	M_PushMenu(menu_framework_c *menu)
{
	m_layers.push(menu);
	
	trap_Key_SetKeyDest(KEY_MENU);
	
	trap_S_StartLocalSound(menu_in_sound);
	
//	trap_Com_DPrintf("M_PushMenu: stack size: %i\n", m_layers.size());
}

void	M_PopMenu()
{
	trap_S_StartLocalSound(menu_out_sound);
	
	if(m_layers.empty())
		trap_Com_Error(ERR_FATAL, "M_PopMenu: depth < 1");
	
	m_layers.pop();
	
	if(m_layers.empty())
	{
		trap_Key_SetKeyDest(KEY_GAME);
		trap_Key_ClearStates();
	}
	
//	trap_Com_DPrintf("M_PopMenu: stack size: %i\n", m_layers.size());
}

void	M_ForceMenuOff()
{
	while(!m_layers.empty())
		m_layers.pop();
		
//	trap_Com_DPrintf("M_ForceMenuOff: stack size: %i\n", m_layers.size());
	
	trap_Key_SetKeyDest(KEY_GAME);
	trap_Key_ClearStates();
}

std::string	menu_framework_c::defaultKeyDown(int key)
{
	std::string sound = "";
	menu_common_c *item;

	if((item = getItemAtCursor()) != 0)
	{
		if(item->getType() == MTYPE_FIELD)
		{
			if(((menu_field_c*)item)->key(key))
				return "";
		}
	}

	switch(key)
	{
		case K_ESCAPE:
			M_PopMenu();
			return menu_out_sound;
			
		case K_KP_UPARROW:
		case K_UPARROW:
			_cursor--;
			adjustCursor(-1);
			sound = menu_move_sound;
			break;
			
		case K_TAB:
			_cursor++;
			adjustCursor(1);
			sound = menu_move_sound;
			break;
			
		case K_KP_DOWNARROW:
		case K_DOWNARROW:
			_cursor++;
			adjustCursor(1);
			sound = menu_move_sound;
			break;
		
		case K_KP_LEFTARROW:
		case K_LEFTARROW:
			slideItem(-1);
			sound = menu_move_sound;
			break;
		
		case K_KP_RIGHTARROW:
		case K_RIGHTARROW:
			slideItem(1);
			sound = menu_move_sound;
			break;
		
		case K_MOUSE1:
		case K_MOUSE2:
		case K_MOUSE3:
		case K_JOY1:
		case K_JOY2:
		case K_JOY3:
		case K_JOY4:
		case K_AUX1:
		case K_AUX2:
		case K_AUX3:
		case K_AUX4:
		case K_AUX5:
		case K_AUX6:
		case K_AUX7:
		case K_AUX8:
		case K_AUX9:
		case K_AUX10:
		case K_AUX11:
		case K_AUX12:
		case K_AUX13:
		case K_AUX14:
		case K_AUX15:
		case K_AUX16:
		case K_AUX17:
		case K_AUX18:
		case K_AUX19:
		case K_AUX20:
		case K_AUX21:
		case K_AUX22:
		case K_AUX23:
		case K_AUX24:
		case K_AUX25:
		case K_AUX26:
		case K_AUX27:
		case K_AUX28:
		case K_AUX29:
		case K_AUX30:
		case K_AUX31:
		case K_AUX32:
		
		case K_KP_ENTER:
		case K_ENTER:
			selectItem();
			sound = menu_move_sound;
			break;
	}

	return sound;
}



/*
================
M_DrawCharacter

Draws one solid graphics character
cx and cy are in 320*240 coordinates, and will be centered on
higher res screens.
================
*/
static void	M_DrawCharacter(int cx, int cy, int num)
{
	Menu_DrawChar( cx + ((trap_VID_GetWidth() - 640)>>1), cy + ((trap_VID_GetHeight() - 480)>>1), num, color_white, FONT_NONE);
}

void	M_Print(int cx, int cy, char *str)
{
	while(*str)
	{
		M_DrawCharacter(cx, cy, (*str)+128);
		str++;
		cx += 8;
	}
}

void	M_PrintWhite(int cx, int cy, char *str)
{
	while(*str)
	{
		M_DrawCharacter(cx, cy, *str);
		str++;
		cx += 8;
	}
}

void	M_DrawPic(int x, int y, char *pic)
{
	/*
	int	shader;
	
	shader = trap_R_RegisterPic(pic);
	trap_R_DrawPic(x + ((trap_VID_GetWidth() -640)>>1), y + ((trap_VID_GetHeight() - 480)>>1), color_white, shader);
	*/
}


/*
=============
M_DrawCursor

Draws an animating cursor with the point at
x,y.  The pic will extend to the left of x,
and both above and below y.
=============
*/
void	M_DrawCursor(int x, int y, int f)
{
	/*
	char	cursorname[80];
	static bool cached;
	//struct r_shader_s*	shader;

	if ( !cached )
	{
		int i;

		for ( i = 0; i < NUM_CURSOR_FRAMES; i++ )
		{
			Com_sprintf( cursorname, sizeof( cursorname ), "textures/pics/m_cursor%d.pcx", i );

			trap_R_RegisterPic( cursorname );
		}
		cached = true;
	}

	Com_sprintf( cursorname, sizeof(cursorname), "textures/pics/m_cursor%d.pcx", f );
	
	trap_R_DrawPic( x, y, color_white, trap_R_RegisterPic(cursorname) );
	*/
}

void M_DrawTextBox(int x, int y, int width, int lines)
{
	int		cx, cy;
	int		n;

	// draw left side
	cx = x;
	cy = y;
	M_DrawCharacter (cx, cy, 1);
	for (n = 0; n < lines; n++)
	{
		cy += 8;
		M_DrawCharacter (cx, cy, 4);
	}
	M_DrawCharacter (cx, cy+8, 7);

	// draw middle
	cx += 8;
	while (width > 0)
	{
		cy = y;
		M_DrawCharacter (cx, cy, 2);
		for (n = 0; n < lines; n++)
		{
			cy += 8;
			M_DrawCharacter (cx, cy, 5);
		}
		M_DrawCharacter (cx, cy+8, 8);
		width -= 1;
		cx += 8;
	}

	// draw right side
	cy = y;
	M_DrawCharacter (cx, cy, 3);
	for (n = 0; n < lines; n++)
	{
		cy += 8;
		M_DrawCharacter (cx, cy, 6);
	}
	M_DrawCharacter (cx, cy+8, 9);
}

		





void	M_Init()
{
//	GUI_Init();	XUL menu

	trap_Cmd_AddCommand("menu_main", M_Menu_Main_f);
//	trap_Cmd_AddCommand("menu_singleplayer", M_Menu_Singleplayer_f);
//		trap_Cmd_AddCommand("menu_loadgame", M_Menu_LoadGame_f);
//		trap_Cmd_AddCommand("menu_savegame", M_Menu_SaveGame_f);
		
	trap_Cmd_AddCommand("menu_multiplayer", M_Menu_Multiplayer_f);
		trap_Cmd_AddCommand("menu_joinserver", M_Menu_JoinServer_f);
//			trap_Cmd_AddCommand("menu_addressbook", M_Menu_AddressBook_f);
			
//		trap_Cmd_AddCommand("menu_startserver", M_Menu_StartServer_f);
//			trap_Cmd_AddCommand("menu_dmoptions", M_Menu_DMOptions_f);
			
//		trap_Cmd_AddCommand("menu_playerconfig", M_Menu_PlayerConfig_f);
//			trap_Cmd_AddCommand("menu_downloadoptions", M_Menu_DownloadOptions_f);
			
	trap_Cmd_AddCommand("menu_options", M_Menu_Options_f);
		trap_Cmd_AddCommand("menu_keys", M_Menu_Keys_f);
		
	trap_Cmd_AddCommand("menu_video", M_Menu_Video_f);
	trap_Cmd_AddCommand("menu_credits", M_Menu_Credits_f);
	trap_Cmd_AddCommand("menu_quit", M_Menu_Quit_f);
}


void	M_Draw()
{
	if(trap_Key_GetKeyDest() != KEY_MENU)
		return;
	
	vec4_c color(0, 0, 0, 0.8);
	trap_R_DrawFill(0, 0, trap_VID_GetWidth(), trap_VID_GetHeight(), color);
	
	if(!m_layers.empty())
	{
		m_layers.top()->draw();
	}
}

void	M_Keydown(int key)
{
	if(trap_Key_GetKeyDest() != KEY_MENU)
		return;

	if(!m_layers.empty())
	{
		// keyDown may clear menu stack so remember current menu
		menu_framework_c* menu = m_layers.top();
	
		std::string s;
		
		if((s = menu->keyDown(key)).length() != 0)
			trap_S_StartLocalSound(s);
	
		menu->draw();
	}
}


