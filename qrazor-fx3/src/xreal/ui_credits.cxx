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



static int credits_start_time;
static const char **credits;
static char *creditsIndex[256];
static char *creditsBuffer;

static const char *xrealcredits[] =
{
	"+XreaL",
	"",
	"+PROGRAMMING",
	"Robert 'Tr3B' Beckebans",
	"",
	"+TECH GURUS WHO HELPED",
	"Mathias 'Skynet' Heyer",
	"Dawid 'rockfor' Kozlowski",
	"",
	"+LEVEL DESIGN",
	"-",
	"",
	"+HP DESIGN",
	"Christian Ballsieper",
	"",
	"+ART",
	"-",
	"",
	"+ANIMATION SEQUENCES",
	"-",
	"",
	"+ADDITIONAL LEVEL DESIGN",
	"-",
	"",
	"+SOUND",
	"-",
	"",
	"+MUSIC",
	"-",
	0
};



static void	M_Credits_MenuDraw()
{
	int i, y;

	//
	// draw the credits
	//
	for(i=0, y=(int)(uii.VID_GetHeight() - ((uii.CL_GetRealtime() - credits_start_time ) / 40.0F)); credits[i] && y < (int)uii.VID_GetHeight(); i++)
	{
		int j, stringoffset = 0;
		
		bool bold = false;

		//if(y <= -8)
		//	continue;
		
		if(y <= -CHAR_MEDIUM_HEIGHT)
			continue;


		if(credits[i][0] == '+')
		{
			bold = true;
			stringoffset = 1;
			
			//if(y <= -CHAR_MEDIUM_HEIGHT)
			//	continue;
		}
		else
		{
			bold = false;
			stringoffset = 0;
			
			//if(y <= -CHAR_SMALL_HEIGHT)
			//	continue;
		}

		for(j=0; credits[i][j+stringoffset]; j++)
		{
			int x;

			//x =(uii.VID_GetWidth() - strlen(credits[i]) * 8 - stringoffset * 8 ) / 2 + (j + stringoffset) * 8;

			if(bold)
			{
				x =(uii.VID_GetWidth() - strlen(credits[i]) * CHAR_MEDIUM_WIDTH - stringoffset * CHAR_MEDIUM_WIDTH ) / 2 + (j + stringoffset) * CHAR_MEDIUM_WIDTH;
				
				Menu_DrawChar(x, y, credits[i][j+stringoffset], color_white, FONT_MEDIUM | FONT_ALT | FONT_CHROME);
			}	
			else
			{
				x =(uii.VID_GetWidth() - strlen(credits[i]) * CHAR_MEDIUM_WIDTH - stringoffset * CHAR_MEDIUM_WIDTH ) / 2 + (j + stringoffset) * CHAR_MEDIUM_WIDTH;
				
				Menu_DrawChar(x, y, credits[i][j+stringoffset], color_white, FONT_MEDIUM | FONT_CHROME);
			}
		}
		
		//if(bold)
		//	y += CHAR_MEDIUM_WIDTH + 5;
		//else
			y += CHAR_MEDIUM_WIDTH + 5;
	}

	if(y < 0)
		credits_start_time = (int)uii.CL_GetRealtime();
}

static const std::string	M_Credits_Key(int key)
{
	switch (key)
	{
		case K_ESCAPE:
			if (creditsBuffer)
				uii.VFS_FFree (creditsBuffer);
			M_PopMenu ();
		break;
	}

	return menu_out_sound;

}

void	M_Menu_Credits_f()
{
	int		n;
	int		count;
	char	*p;

	creditsBuffer = NULL;
	count = uii.VFS_FLoad("credits", (void**)&creditsBuffer);
	if(count != -1)
	{
		p = creditsBuffer;
		for(n = 0; n < 255; n++)
		{
			creditsIndex[n] = p;
			while (*p != '\r' && *p != '\n')
			{
				p++;
				if (--count == 0)
					break;
			}
			if (*p == '\r')
			{
				*p++ = 0;
				if (--count == 0)
					break;
			}
			*p++ = 0;
			if (--count == 0)
				break;
		}
		creditsIndex[++n] = 0;
		credits = (const char**)creditsIndex;
	}
	else
	{
		credits = xrealcredits;		//XreaL project
	}

	credits_start_time = (int)uii.CL_GetRealtime();
	
	M_PushMenu(M_Credits_MenuDraw, M_Credits_Key);
}

