/// ============================================================================
/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2003 Robert Beckebans <trebor_7@users.sourceforge.net>
Copyright (C) 2003, 2004  contributors of the XreaL project
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
#ifndef CONSOLE_H
#define CONSOLE_H

/// includes ===================================================================
// system -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
// shared -------------------------------------------------------------------
#include "x_shared.h"


#define		NUM_CON_TIMES 	4
#define		CON_TEXTSIZE	32768

struct console_t
{
	bool	initialized;

	char	text[CON_TEXTSIZE];
	int	current;		// line where next message will be printed
	int	x;			// offset in current line for next print
	int	display;		// bottom of console displays this line

	int	ormask;			// high bit mask for colored characters

	int 	linewidth;		// characters across screen
	int	totallines;		// total lines in console scrollback

	float	cursorspeed;

	int	vislines;

	float	times[NUM_CON_TIMES];	// cls.realtime time the line was generated
								// for transparent notify lines
};

extern	console_t	con;


void	Con_Init();
void	Con_CheckResize();
void	Con_DrawConsole(float frac);
void	Con_Print(const char *txt);
void	Con_CenteredPrint(const char *text);
void	Con_ServerPrint(bitmessage_c &msg);
void	Con_Clear_f();
void	Con_DrawNotify();
void	Con_ClearNotify();
void	Con_ClearTyping();
void	Con_ToggleConsole_f();
bool	Con_IsInitialized();



#endif // CONSOLE_H

