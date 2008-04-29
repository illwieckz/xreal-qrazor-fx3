/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006 Robert Beckebans <trebor_7@users.sourceforge.net>
Copyright (C) 2008 Pat Raynor <raynorpat@gmail.com>

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

/*
=======================================================================

CREDITS

=======================================================================
*/

#include "ui_local.h"

#define SCROLLSPEED	3.50

typedef struct
{
	menuframework_s menu;
} creditsmenu_t;

static creditsmenu_t s_credits;

int             starttime;		// game time at which credits are started
float           mvolume;		// records the original music volume level

qhandle_t       BackgroundShader;

typedef struct
{
	char           *string;
	int             style;
	vec4_t         *colour;
} cr_line;

cr_line         credits[] = {
	{"XreaL", UI_CENTER | UI_GIANTFONT | UI_PULSE, &colorRed},
	{"", UI_CENTER | UI_SMALLFONT, &colorBlue},

	//{ "Project Leader", UI_CENTER|UI_SMALLFONT, &colorLtGrey },
	//{ "Robert 'Tr3B' Beckebans", UI_CENTER|UI_SMALLFONT, &colorWhite },
	//{ "", UI_CENTER|UI_SMALLFONT, &colorBlue },

	{"Programming", UI_CENTER | UI_SMALLFONT, &colorLtGrey},
	{"Robert Beckebans", UI_CENTER | UI_SMALLFONT, &colorWhite},
	{"Pat Raynor", UI_CENTER | UI_SMALLFONT, &colorWhite},
	{"", UI_CENTER | UI_SMALLFONT, &colorBlue},

	{"Development Assistance", UI_CENTER | UI_SMALLFONT, &colorLtGrey},
	//{ "Mathias Heyer", UI_CENTER|UI_SMALLFONT, &colorWhite },
	{"Josef Soentgen", UI_CENTER | UI_SMALLFONT, &colorWhite},
	{"", UI_CENTER | UI_SMALLFONT, &colorBlue},

	{"Art", UI_CENTER | UI_SMALLFONT, &colorLtGrey},
	{"XreaL Team", UI_CENTER | UI_SMALLFONT, &colorWhite},
	{"Quake II: Lost Marine Team", UI_CENTER | UI_SMALLFONT, &colorWhite},
	{"OpenArena Team", UI_CENTER | UI_SMALLFONT, &colorWhite},
	{"Tenebrae Team", UI_CENTER | UI_SMALLFONT, &colorWhite},
	{"Sapphire Scar Team", UI_CENTER | UI_SMALLFONT, &colorWhite},
	//{ "Paul 'JTR' Steffens, Lee David Ash,", UI_CENTER|UI_SMALLFONT, &colorWhite },
	//{ "James 'HarlequiN' Taylor,", UI_CENTER|UI_SMALLFONT, &colorWhite },
	//{ "Michael 'mic' Denno", UI_CENTER|UI_SMALLFONT, &colorWhite },
	{"", UI_CENTER | UI_SMALLFONT, &colorBlue},

	//{ "Level Design", UI_CENTER|UI_SMALLFONT, &colorLtGrey },
	//{ "Michael 'mic' Denno", UI_CENTER|UI_SMALLFONT, &colorWhite },
	//{ "'Dominic 'cha0s' Szablewski", UI_CENTER|UI_SMALLFONT, &colorWhite },
	//{ "", UI_CENTER|UI_SMALLFONT, &colorBlue },

	{"Special Thanks To:", UI_CENTER | UI_SMALLFONT, &colorLtGrey},
	{"iD Software", UI_CENTER | UI_SMALLFONT, &colorWhite},
	{"ioquake3 project", UI_CENTER | UI_SMALLFONT, &colorWhite},
	{"", UI_CENTER | UI_SMALLFONT, &colorBlue},

	{"Contributors", UI_CENTER | UI_SMALLFONT, &colorLtGrey},
	{"For a list of contributors,", UI_CENTER | UI_SMALLFONT, &colorWhite},
	{"see the accompanying CONTRIBUTORS.txt", UI_CENTER | UI_SMALLFONT, &colorWhite},
	{"", UI_CENTER | UI_SMALLFONT, &colorBlue},

	{"Websites:", UI_CENTER | UI_SMALLFONT, &colorLtGrey},
	{"www.sourceforge.net/projects/xreal", UI_CENTER | UI_SMALLFONT, &colorBlue},
	{"xreal.sourceforge.net", UI_CENTER | UI_SMALLFONT, &colorBlue},
	{"", UI_CENTER | UI_SMALLFONT, &colorBlue},

	{"XreaL(c) 2005-2008, XreaL Team and Contributors", UI_CENTER | UI_SMALLFONT, &colorRed},

	{NULL}
};

/*
=================
UI_CreditMenu_Key
=================
*/
static sfxHandle_t UI_CreditMenu_Key(int key)
{
	if(key & K_CHAR_FLAG)
		return 0;

	// pressing the escape key or clicking the mouse will exit
	// we also reset the music volume to the user's original
	// choice here,  by setting s_musicvolume to the stored var
	trap_Cmd_ExecuteText(EXEC_APPEND, va("s_musicvolume %f; quit\n", mvolume));
	return 0;
}

/*
=================
ScrollingCredits_Draw

Main drawing function
=================
*/
static void ScrollingCredits_Draw(void)
{
	int             x = 320, y, n, ysize = 0, fadetime = 0;
	vec4_t          fadecolour = { 0.00, 0.00, 0.00, 0.00 };

	// ysize is used to determine the entire length of the credits in pixels. 
	if(!ysize)
	{
		// loop through entire credits array
		for(n = 0; n <= sizeof(credits) - 1; n++)
		{
			if(credits[n].style & UI_SMALLFONT)
			{
				// add small character height
				ysize += PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
			}
			else if(credits[n].style & UI_BIGFONT)
			{
				// add big character size
				ysize += PROP_HEIGHT;
			}
			else if(credits[n].style & UI_GIANTFONT)
			{
				// add giant character size.
				ysize += PROP_HEIGHT * (1 / PROP_SMALL_SIZE_SCALE);
			}
		}
	}

	// first, fill the background with the specified shader
	UI_DrawHandlePic(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BackgroundShader);

	// draw the stuff by settting the initial y location
	y = 480 - SCROLLSPEED * (float)(uis.realtime - starttime) / 100;

	// loop through the entire credits sequence
	for(n = 0; n <= sizeof(credits) - 1; n++)
	{
		// this NULL string marks the end of the credits struct
		if(credits[n].string == NULL)
		{
			// credits sequence is completely off screen
			if(y < -16)
			{
				// TODO: bring up XreaL plaque and fade-in and wait for keypress?
				break;
			}
			break;
		}

		if(strlen(credits[n].string) == 1)	// spacer string, no need to draw
			continue;

		if(y > -(PROP_HEIGHT * (1 / PROP_SMALL_SIZE_SCALE)))
		{
			// the line is within the visible range of the screen
			UI_DrawProportionalString(x, y, credits[n].string, credits[n].style, *credits[n].colour);
		}

		// re-adjust y for next line
		if(credits[n].style & UI_SMALLFONT)
		{
			y += PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
		}
		else if(credits[n].style & UI_BIGFONT)
		{
			y += PROP_HEIGHT;
		}
		else if(credits[n].style & UI_GIANTFONT)
		{
			y += PROP_HEIGHT * (1 / PROP_SMALL_SIZE_SCALE);
		}

		// if y is off the screen, break out of loop
		if(y > 480)
			break;
	}
}

/*
===============
UI_CreditMenu
===============
*/
void UI_CreditMenu(void)
{
	memset(&s_credits, 0, sizeof(s_credits));

	s_credits.menu.draw = ScrollingCredits_Draw;
	s_credits.menu.key = UI_CreditMenu_Key;
	s_credits.menu.fullscreen = qtrue;
	UI_PushMenu(&s_credits.menu);

	starttime = uis.realtime; // record start time for credits to scroll properly
	mvolume = trap_Cvar_VariableValue("s_musicvolume");
	if(mvolume < 0.5)
		trap_Cmd_ExecuteText(EXEC_APPEND, "s_musicvolume 0.5\n");
	trap_Cmd_ExecuteText(EXEC_APPEND, "music music/credits\n");

	// load the background shader
	BackgroundShader = trap_R_RegisterShaderNoMip("menubackcredits");
}
