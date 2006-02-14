/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006 Robert Beckebans <trebor_7@users.sourceforge.net>

This file is part of XreaL source code.

XreaL source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

XreaL source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XreaL source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
/*
=======================================================================

CREDITS

=======================================================================
*/


#include "ui_local.h"


typedef struct
{
	menuframework_s menu;
} creditsmenu_t;

static creditsmenu_t s_credits;


/*
=================
UI_CreditMenu_Key
=================
*/
static sfxHandle_t UI_CreditMenu_Key(int key)
{
	if(key & K_CHAR_FLAG)
	{
		return 0;
	}

	trap_Cmd_ExecuteText(EXEC_APPEND, "quit\n");
	return 0;
}


/*
===============
UI_CreditMenu_Draw
===============
*/
static void UI_CreditMenu_Draw(void)
{
	int             y;

	y = 12;
	UI_DrawProportionalString(320, y, "Credits", UI_CENTER | UI_SMALLFONT, colorRed);

	y += 1.42 * PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
	UI_DrawProportionalString(320, y, "Programming", UI_CENTER | UI_SMALLFONT, colorLtGrey);
	y += PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
	UI_DrawProportionalString(320, y, "Robert 'Tr3B' Beckebans", UI_CENTER | UI_SMALLFONT, colorWhite);

	y += 1.42 * PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
	UI_DrawProportionalString(320, y, "Art", UI_CENTER | UI_SMALLFONT, colorLtGrey);
	y += PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
	UI_DrawProportionalString(320, y, "Robert 'Tr3B' Beckebans,", UI_CENTER | UI_SMALLFONT, colorWhite);
	y += PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
	UI_DrawProportionalString(320, y, "Paul 'JTR' Steffens, Lee David Ash,", UI_CENTER | UI_SMALLFONT, colorWhite);
	y += PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
	UI_DrawProportionalString(320, y, "James 'HarlequiN' Taylor,", UI_CENTER | UI_SMALLFONT, colorWhite);
	y += PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
	UI_DrawProportionalString(320, y, "Michael 'mic' Denno", UI_CENTER | UI_SMALLFONT, colorWhite);

	/*
	y += 1.42 * PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
	UI_DrawProportionalString(320, y, "Game Designer", UI_CENTER | UI_SMALLFONT, colorLtGrey);
	y += PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
	UI_DrawProportionalString(320, y, "Graeme Devine", UI_CENTER | UI_SMALLFONT, colorWhite);
	*/

	y += 1.42 * PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
	UI_DrawProportionalString(320, y, "Level Design", UI_CENTER | UI_SMALLFONT, colorLtGrey);
	y += PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
	UI_DrawProportionalString(320, y, "Michael 'mic' Denno", UI_CENTER | UI_SMALLFONT, colorWhite);
	//y += PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
	//UI_DrawProportionalString(320, y, "'Dominic 'cha0s' Szablewski", UI_CENTER | UI_SMALLFONT, colorWhite);

	y += 1.42 * PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
	UI_DrawProportionalString(320, y, "Project Leader", UI_CENTER | UI_SMALLFONT, colorLtGrey);
	y += PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
	UI_DrawProportionalString(320, y, "Robert 'Tr3B' Beckebans", UI_CENTER | UI_SMALLFONT, colorWhite);

	/*
	y += 1.42 * PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
	UI_DrawProportionalString(320, y, "Director of Business Development", UI_CENTER | UI_SMALLFONT, colorWhite);
	y += PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
	UI_DrawProportionalString(320, y, "-", UI_CENTER | UI_SMALLFONT, colorWhite);

	y += 1.42 * PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
	UI_DrawProportionalString( 320, y, "Biz Assist and id Mom", UI_CENTER|UI_SMALLFONT, colorWhite );
	y += PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
	UI_DrawProportionalString( 320, y, "Donna Jackson", UI_CENTER|UI_SMALLFONT, colorWhite );
	*/
	
	y += 1.42 * PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
	UI_DrawProportionalString(320, y, "Development Assistance", UI_CENTER | UI_SMALLFONT, colorLtGrey);
	y += PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
	UI_DrawProportionalString(320, y, "Mathias 'Skynet' Heyer", UI_CENTER | UI_SMALLFONT, colorWhite);
	y += PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
	UI_DrawProportionalString(320, y, "Josef 'cnuke' Soentgen", UI_CENTER | UI_SMALLFONT, colorWhite);
	y += PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
	UI_DrawProportionalString(320, y, "Bjoern 'bj0ern' Paschen", UI_CENTER | UI_SMALLFONT, colorWhite);

	y += 1.35 * PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
//	UI_DrawString(320, y, "To order: 1-800-idgames     www.quake3arena.com     www.idsoftware.com", UI_CENTER | UI_SMALLFONT, colorRed);
	UI_DrawString(320, y, "websites:   www.sourceforge.net/projects/xreal    xreal.sourceforge.net", UI_CENTER | UI_SMALLFONT, colorBlue);
	y += 1.35 * PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
	//y += SMALLCHAR_HEIGHT;
	UI_DrawString(320, y, "Quake III Arena(c) 1999-2005, Id Software, Inc.", UI_CENTER | UI_SMALLFONT, colorRed);
	y += SMALLCHAR_HEIGHT;
	UI_DrawString(320, y, "XreaL(c) 2005-2006, XreaL team and contributors", UI_CENTER | UI_SMALLFONT, colorRed);
}


/*
===============
UI_CreditMenu
===============
*/
void UI_CreditMenu(void)
{
	memset(&s_credits, 0, sizeof(s_credits));

	s_credits.menu.draw = UI_CreditMenu_Draw;
	s_credits.menu.key = UI_CreditMenu_Key;
	s_credits.menu.fullscreen = qtrue;
	UI_PushMenu(&s_credits.menu);
}
