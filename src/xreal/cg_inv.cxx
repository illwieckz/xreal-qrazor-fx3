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
#include "cg_local.h"


void	CG_ParseInventory(bitmessage_c &msg)
{
	for(int i=0; i<MAX_ITEMS; i++)
		cg.inventory[i] = msg.readShort();
}


static void	CG_DrawInvString(int x, int y, const vec4_c &color, char *string)
{
	while(*string)
	{
		CG_DrawChar(x, y, *string, color, FONT_NONE);
		x+=8;
		string++;
	}
}


void	CG_SetStringHighBit(char *s)
{
	while (*s)
		*s++ |= 128;
}

#define	DISPLAY_ITEMS	17

void	CG_DrawInventory()
{
	int		i, j;
	int		num, selected_num, item;
	int		index[MAX_ITEMS];
	char	string[1024];
	int		x, y;
	char	binding[1024];
	char	*bind;
	int		selected;
	int		top;

	if(!(cg.frame.playerstate.stats[STAT_LAYOUTS] & 2))
		return;

	
	selected = cg.frame.playerstate.stats[STAT_SELECTED_ITEM];

	num = 0;
	selected_num = 0;
	for (i=0 ; i<MAX_ITEMS ; i++)
	{
		if(i==selected)
			selected_num = num;
			
		if(cg.inventory[i])
		{
			index[num] = i;
			num++;
		}
	}

	// determine scroll point
	top = selected_num - DISPLAY_ITEMS/2;
	if (num - top < DISPLAY_ITEMS)
		top = num - DISPLAY_ITEMS;
	if (top < 0)
		top = 0;

	x = (trap_VID_GetWidth()-256)/2;
	y = (trap_VID_GetHeight()-240)/2;

	
	//trap_R_DrawPic(x, y+8, color_white, trap_R_RegisterPic("textures/pics/inventory"));

	y += 24;
	x += 24;
	CG_DrawInvString(x, y, color_blue, "hotkey ### item");
	CG_DrawInvString(x, y+8, color_blue, "------ --- ----");
	y += 16;
	for (i=top ; i<num && i < top+DISPLAY_ITEMS ; i++)
	{
		item = index[i];
		// search for a binding
		Com_sprintf(binding, sizeof(binding), "use %s", trap_CL_GetConfigString(CS_ITEMS+item));
		
		bind = "";
		for(j=0; j<256; j++)
		{
			if(trap_Key_GetBinding(j) && !X_stricmp(trap_Key_GetBinding(j), binding))
			{
				bind = trap_Key_KeynumToString(j);
				break;
			}
		}

		Com_sprintf(string, sizeof(string), "%6s %3i %s", bind, cg.inventory[item], trap_CL_GetConfigString(CS_ITEMS+item));
		
		if(item != selected)
		{
			CG_SetStringHighBit(string);
		}		
		else	
		{
			// draw a blinky cursor by the selected item
			if((int)(trap_CLS_GetRealTime()*10) & 1)
				CG_DrawChar(x-8, y, 15, color_red, FONT_NONE);
		}
		
		CG_DrawInvString(x, y, color_white, string);
		y += 8;
	}
}


