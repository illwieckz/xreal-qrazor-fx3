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
// cg_drawtools.c -- helper functions called by cg_draw, cg_scoreboard, cg_info, etc
#include "cg_local.h"

/*
================
CG_AdjustFrom640

Adjusted for resolution and screen aspect ratio
================
*/
void CG_AdjustFrom640(float *x, float *y, float *w, float *h)
{
#if 0
	// adjust for wide screens
	if(cgs.glconfig.vidWidth * 480 > cgs.glconfig.vidHeight2 * 640)
	{
		*x += 0.5 * (cgs.glconfig.vidWidth - (cgs.glconfig.vidHeight2 * 640 / 480));
	}
#endif
	// scale for screen sizes
	*x *= cgs.screenXScale;
	*y *= cgs.screenYScale;
	*w *= cgs.screenXScale;
	*h *= cgs.screenYScale;
}

/*
================
CG_FillRect

Coordinates are 640*480 virtual values
=================
*/
void CG_FillRect(float x, float y, float width, float height, const float *color)
{
	trap_R_SetColor(color);

	CG_AdjustFrom640(&x, &y, &width, &height);
	trap_R_DrawStretchPic(x, y, width, height, 0, 0, 0, 0, cgs.media.whiteShader);

	trap_R_SetColor(NULL);
}

/*
================
CG_DrawSides

Coords are virtual 640x480
================
*/
void CG_DrawSides(float x, float y, float w, float h, float size)
{
	CG_AdjustFrom640(&x, &y, &w, &h);
	size *= cgs.screenXScale;
	trap_R_DrawStretchPic(x, y, size, h, 0, 0, 0, 0, cgs.media.whiteShader);
	trap_R_DrawStretchPic(x + w - size, y, size, h, 0, 0, 0, 0, cgs.media.whiteShader);
}

void CG_DrawTopBottom(float x, float y, float w, float h, float size)
{
	CG_AdjustFrom640(&x, &y, &w, &h);
	size *= cgs.screenYScale;
	trap_R_DrawStretchPic(x, y, w, size, 0, 0, 0, 0, cgs.media.whiteShader);
	trap_R_DrawStretchPic(x, y + h - size, w, size, 0, 0, 0, 0, cgs.media.whiteShader);
}

/*
================
UI_DrawRect

Coordinates are 640*480 virtual values
=================
*/
void CG_DrawRect(float x, float y, float width, float height, float size, const float *color)
{
	trap_R_SetColor(color);

	CG_DrawTopBottom(x, y, width, height, size);
	CG_DrawSides(x, y, width, height, size);

	trap_R_SetColor(NULL);
}



/*
================
CG_DrawPic

Coordinates are 640*480 virtual values
=================
*/
void CG_DrawPic(float x, float y, float width, float height, qhandle_t hShader)
{
	CG_AdjustFrom640(&x, &y, &width, &height);
	trap_R_DrawStretchPic(x, y, width, height, 0, 0, 1, 1, hShader);
}






/*
================
CG_FadeColor
================
*/
float          *CG_FadeColor(int startMsec, int totalMsec)
{
	static vec4_t   color;
	int             t;

	if(startMsec == 0)
	{
		return NULL;
	}

	t = cg.time - startMsec;

	if(t >= totalMsec)
	{
		return NULL;
	}

	// fade out
	if(totalMsec - t < FADE_TIME)
	{
		color[3] = (totalMsec - t) * 1.0 / FADE_TIME;
	}
	else
	{
		color[3] = 1.0;
	}
	color[0] = color[1] = color[2] = 1;

	return color;
}


/*
================
CG_TeamColor
================
*/
float          *CG_TeamColor(int team)
{
	static vec4_t   red = { 1, 0.2f, 0.2f, 1 };
	static vec4_t   blue = { 0.2f, 0.2f, 1, 1 };
	static vec4_t   other = { 1, 1, 1, 1 };
	static vec4_t   spectator = { 0.7f, 0.7f, 0.7f, 1 };

	switch (team)
	{
		case TEAM_RED:
			return red;
		case TEAM_BLUE:
			return blue;
		case TEAM_SPECTATOR:
			return spectator;
		default:
			return other;
	}
}



/*
=================
CG_GetColorForHealth
=================
*/
void CG_GetColorForHealth(int health, int armor, vec4_t hcolor)
{
	int             count;
	int             max;

	// calculate the total points of damage that can
	// be sustained at the current health / armor level
	if(health <= 0)
	{
		VectorClear(hcolor);	// black
		hcolor[3] = 1;
		return;
	}
	count = armor;
	max = health * ARMOR_PROTECTION;
	if(max < count)
	{
		count = max;
	}
	health += count;

	// set the color based on health
	hcolor[0] = 1.0;
	hcolor[3] = 1.0;
	if(health >= 100)
	{
		hcolor[2] = 1.0;
	}
	else if(health < 66)
	{
		hcolor[2] = 0;
	}
	else
	{
		hcolor[2] = (health - 66) / 33.0;
	}

	if(health > 60)
	{
		hcolor[1] = 1.0;
	}
	else if(health < 30)
	{
		hcolor[1] = 0;
	}
	else
	{
		hcolor[1] = (health - 30) / 30.0;
	}
}

/*
=================
CG_ColorForHealth
=================
*/
void CG_ColorForHealth(vec4_t hcolor)
{

	CG_GetColorForHealth(cg.snap->ps.stats[STAT_HEALTH], cg.snap->ps.stats[STAT_ARMOR], hcolor);
}




// bk001205 - code below duplicated in q3_ui/ui-atoms.c
// bk001205 - FIXME: does this belong in ui_shared.c?
// bk001205 - FIXME: HARD_LINKED flags not visible here
#ifndef Q3_STATIC				// bk001205 - q_shared defines not visible here
/*
=================
UI_DrawProportionalString2
=================
*/
static int      propMap[128][3] = {
	{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
	{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},

	{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
	{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},

	{0, 0, PROP_SPACE_WIDTH},	// SPACE
	{11, 122, 7},				// !
	{154, 181, 14},				// "
	{55, 122, 17},				// #
	{79, 122, 18},				// $
	{101, 122, 23},				// %
	{153, 122, 18},				// &
	{9, 93, 7},					// '
	{207, 122, 8},				// (
	{230, 122, 9},				// )
	{177, 122, 18},				// *
	{30, 152, 18},				// +
	{85, 181, 7},				// ,
	{34, 93, 11},				// -
	{110, 181, 6},				// .
	{130, 152, 14},				// /

	{22, 64, 17},				// 0
	{41, 64, 12},				// 1
	{58, 64, 17},				// 2
	{78, 64, 18},				// 3
	{98, 64, 19},				// 4
	{120, 64, 18},				// 5
	{141, 64, 18},				// 6
	{204, 64, 16},				// 7
	{162, 64, 17},				// 8
	{182, 64, 18},				// 9
	{59, 181, 7},				// :
	{35, 181, 7},				// ;
	{203, 152, 14},				// <
	{56, 93, 14},				// =
	{228, 152, 14},				// >
	{177, 181, 18},				// ?

	{28, 122, 22},				// @
	{5, 4, 18},					// A
	{27, 4, 18},				// B
	{48, 4, 18},				// C
	{69, 4, 17},				// D
	{90, 4, 13},				// E
	{106, 4, 13},				// F
	{121, 4, 18},				// G
	{143, 4, 17},				// H
	{164, 4, 8},				// I
	{175, 4, 16},				// J
	{195, 4, 18},				// K
	{216, 4, 12},				// L
	{230, 4, 23},				// M
	{6, 34, 18},				// N
	{27, 34, 18},				// O

	{48, 34, 18},				// P
	{68, 34, 18},				// Q
	{90, 34, 17},				// R
	{110, 34, 18},				// S
	{130, 34, 14},				// T
	{146, 34, 18},				// U
	{166, 34, 19},				// V
	{185, 34, 29},				// W
	{215, 34, 18},				// X
	{234, 34, 18},				// Y
	{5, 64, 14},				// Z
	{60, 152, 7},				// [
	{106, 151, 13},				// '\'
	{83, 152, 7},				// ]
	{128, 122, 17},				// ^
	{4, 152, 21},				// _

	{134, 181, 5},				// '
	{5, 4, 18},					// A
	{27, 4, 18},				// B
	{48, 4, 18},				// C
	{69, 4, 17},				// D
	{90, 4, 13},				// E
	{106, 4, 13},				// F
	{121, 4, 18},				// G
	{143, 4, 17},				// H
	{164, 4, 8},				// I
	{175, 4, 16},				// J
	{195, 4, 18},				// K
	{216, 4, 12},				// L
	{230, 4, 23},				// M
	{6, 34, 18},				// N
	{27, 34, 18},				// O

	{48, 34, 18},				// P
	{68, 34, 18},				// Q
	{90, 34, 17},				// R
	{110, 34, 18},				// S
	{130, 34, 14},				// T
	{146, 34, 18},				// U
	{166, 34, 19},				// V
	{185, 34, 29},				// W
	{215, 34, 18},				// X
	{234, 34, 18},				// Y
	{5, 64, 14},				// Z
	{153, 152, 13},				// {
	{11, 181, 5},				// |
	{180, 152, 13},				// }
	{79, 93, 17},				// ~
	{0, 0, -1}					// DEL
};

static int      propMapB[26][3] = {
	{11, 12, 33},
	{49, 12, 31},
	{85, 12, 31},
	{120, 12, 30},
	{156, 12, 21},
	{183, 12, 21},
	{207, 12, 32},

	{13, 55, 30},
	{49, 55, 13},
	{66, 55, 29},
	{101, 55, 31},
	{135, 55, 21},
	{158, 55, 40},
	{204, 55, 32},

	{12, 97, 31},
	{48, 97, 31},
	{82, 97, 30},
	{118, 97, 30},
	{153, 97, 30},
	{185, 97, 25},
	{213, 97, 30},

	{11, 139, 32},
	{42, 139, 51},
	{93, 139, 32},
	{126, 139, 31},
	{158, 139, 25},
};

#define PROPB_GAP_WIDTH		4
#define PROPB_SPACE_WIDTH	12
#define PROPB_HEIGHT		36

/*
=================
UI_DrawBannerString
=================
*/
static void UI_DrawBannerString2(int x, int y, const char *str, vec4_t color)
{
	const char     *s;
	unsigned char   ch;			// bk001204 : array subscript
	float           ax;
	float           ay;
	float           aw;
	float           ah;
	float           frow;
	float           fcol;
	float           fwidth;
	float           fheight;

	// draw the colored text
	trap_R_SetColor(color);

	ax = x * cgs.screenXScale + cgs.screenXBias;
	ay = y * cgs.screenXScale;

	s = str;
	while(*s)
	{
		ch = *s & 127;
		if(ch == ' ')
		{
			ax += ((float)PROPB_SPACE_WIDTH + (float)PROPB_GAP_WIDTH) * cgs.screenXScale;
		}
		else if(ch >= 'A' && ch <= 'Z')
		{
			ch -= 'A';
			fcol = (float)propMapB[ch][0] / 256.0f;
			frow = (float)propMapB[ch][1] / 256.0f;
			fwidth = (float)propMapB[ch][2] / 256.0f;
			fheight = (float)PROPB_HEIGHT / 256.0f;
			aw = (float)propMapB[ch][2] * cgs.screenXScale;
			ah = (float)PROPB_HEIGHT *cgs.screenXScale;

			trap_R_DrawStretchPic(ax, ay, aw, ah, fcol, frow, fcol + fwidth, frow + fheight, cgs.media.charsetPropB);
			ax += (aw + (float)PROPB_GAP_WIDTH * cgs.screenXScale);
		}
		s++;
	}

	trap_R_SetColor(NULL);
}

void UI_DrawBannerString(int x, int y, const char *str, int style, vec4_t color)
{
	const char     *s;
	int             ch;
	int             width;
	vec4_t          drawcolor;

	// find the width of the drawn text
	s = str;
	width = 0;
	while(*s)
	{
		ch = *s;
		if(ch == ' ')
		{
			width += PROPB_SPACE_WIDTH;
		}
		else if(ch >= 'A' && ch <= 'Z')
		{
			width += propMapB[ch - 'A'][2] + PROPB_GAP_WIDTH;
		}
		s++;
	}
	width -= PROPB_GAP_WIDTH;

	switch (style & UI_FORMATMASK)
	{
		case UI_CENTER:
			x -= width / 2;
			break;

		case UI_RIGHT:
			x -= width;
			break;

		case UI_LEFT:
		default:
			break;
	}

	if(style & UI_DROPSHADOW)
	{
		drawcolor[0] = drawcolor[1] = drawcolor[2] = 0;
		drawcolor[3] = color[3];
		UI_DrawBannerString2(x + 2, y + 2, str, drawcolor);
	}

	UI_DrawBannerString2(x, y, str, color);
}


int UI_ProportionalStringWidth(const char *str)
{
	const char     *s;
	int             ch;
	int             charWidth;
	int             width;

	s = str;
	width = 0;
	while(*s)
	{
		ch = *s & 127;
		charWidth = propMap[ch][2];
		if(charWidth != -1)
		{
			width += charWidth;
			width += PROP_GAP_WIDTH;
		}
		s++;
	}

	width -= PROP_GAP_WIDTH;
	return width;
}

static void UI_DrawProportionalString2(int x, int y, const char *str, vec4_t color, float sizeScale, qhandle_t charset)
{
	const char     *s;
	unsigned char   ch;			// bk001204 - unsigned
	float           ax;
	float           ay;
	float           aw;
	float           ah;
	float           frow;
	float           fcol;
	float           fwidth;
	float           fheight;

	// draw the colored text
	trap_R_SetColor(color);

	ax = x * cgs.screenXScale + cgs.screenXBias;
	ay = y * cgs.screenXScale;

	s = str;
	while(*s)
	{
		ch = *s & 127;
		if(ch == ' ')
		{
			aw = (float)PROP_SPACE_WIDTH *cgs.screenXScale * sizeScale;
		}
		else if(propMap[ch][2] != -1)
		{
			fcol = (float)propMap[ch][0] / 256.0f;
			frow = (float)propMap[ch][1] / 256.0f;
			fwidth = (float)propMap[ch][2] / 256.0f;
			fheight = (float)PROP_HEIGHT / 256.0f;
			aw = (float)propMap[ch][2] * cgs.screenXScale * sizeScale;
			ah = (float)PROP_HEIGHT *cgs.screenXScale * sizeScale;

			trap_R_DrawStretchPic(ax, ay, aw, ah, fcol, frow, fcol + fwidth, frow + fheight, charset);
		}
		else
		{
			aw = 0;
		}

		ax += (aw + (float)PROP_GAP_WIDTH * cgs.screenXScale * sizeScale);
		s++;
	}

	trap_R_SetColor(NULL);
}

/*
=================
UI_ProportionalSizeScale
=================
*/
float UI_ProportionalSizeScale(int style)
{
	if(style & UI_SMALLFONT)
	{
		return 0.75;
	}

	return 1.00;
}

/*
===============
CG_DrawChar

Coordinates and size in 640*480 virtual screen size
===============
*/
void CG_DrawChar(int x, int y, int width, int height, int ch, vec4_t color)
{
	int             row, col;
	float           frow, fcol;
	float           size;
	float           ax, ay, aw, ah;

	ch &= 255;

	if(ch == ' ')
	{
		return;
	}


	ax = x;
	ay = y;
	aw = width;
	ah = height;
	CG_AdjustFrom640(&ax, &ay, &aw, &ah);

	row = ch >> 4;
	col = ch & 15;

	frow = row * 0.0625;
	fcol = col * 0.0625;
	size = 0.0625;

	trap_R_DrawStretchPic(ax, ay, aw, ah, fcol, frow, fcol + size, frow + size, cgs.media.charsetShader);
}

void CG_DrawMultiFontChar(int x, int y, int width, int height, int ch, vec4_t color, qhandle_t ChShader)
{
	int             row, col;
	float           frow, fcol;
	float           size;
	float           ax, ay, aw, ah;

	ch &= 255;

	if(ch == ' ')
	{
		return;
	}


	ax = x;
	ay = y;
	aw = width;
	ah = height;
	CG_AdjustFrom640(&ax, &ay, &aw, &ah);

	row = ch >> 4;
	col = ch & 15;

	frow = row * 0.0625;
	fcol = col * 0.0625;
	size = 0.0625;

	trap_R_DrawStretchPic(ax, ay, aw, ah, fcol, frow, fcol + size, frow + size, ChShader);
}

/*
==================
CG_DrawStringExt

Draws a multi-colored string with a drop shadow, optionally forcing
to a fixed color.

Coordinates are at 640 by 480 virtual resolution
==================
*/
void CG_DrawStringExtTimer(int x, int y, const char *string, const float *setColor,
						   qboolean forceColor, qboolean shadow, int charWidth, int charHeight, int maxChars)
{
	vec4_t          color;
	const char     *s;
	const char     *s1;
	int             xx;
	int             cnt;



	if(maxChars <= 0)
		maxChars = 32767;		// do them all!

	// draw the drop shadow
	if(shadow)
	{

	}



	// draw the colored text
	s = string;
	s1 = ":";
	xx = x;
	cnt = 0;
	trap_R_SetColor(setColor);
	while(*s && cnt < maxChars)
	{
		if(Q_IsColorString(s))
		{
			if(!forceColor)
			{
				memcpy(color, g_color_table[ColorIndex(*(s + 1))], sizeof(color));
				color[3] = setColor[3];
				trap_R_SetColor(color);
			}
			s += 2;
			continue;
		}
		if(cnt == 2 || cnt == 5)
		{
			CG_DrawMultiFontChar(xx, y, charWidth, charHeight, *s1, color, cgs.media.ZcharsetShader);
			xx += charWidth;
			cnt++;
		}
		//  if(cnt == 5){
		//      CG_DrawChar( xx, y, charWidth, charHeight, *s1 ,color);
		//      xx += charWidth;
		//      cnt++;

		//  }
		if(cnt != 2 || cnt != 5)
		{
			CG_DrawMultiFontChar(xx, y, charWidth, charHeight, *s, color, cgs.media.ZcharsetShader);
			xx += charWidth;
			cnt++;
			s++;
		}
	}
	trap_R_SetColor(NULL);
}

void CG_DrawStringExt(int x, int y, const char *string, const float *setColor,
					  qboolean forceColor, qboolean shadow, int charWidth, int charHeight, int maxChars)
{
	vec4_t          color;
	const char     *s;
	int             xx;
	int             cnt;



	if(maxChars <= 0)
		maxChars = 32767;		// do them all!

	// draw the drop shadow
	if(shadow)
	{
		color[0] = color[1] = color[2] = 0;
		color[3] = setColor[3];
		trap_R_SetColor(color);
		s = string;
		xx = x;
		cnt = 0;
		while(*s && cnt < maxChars)
		{
			if(Q_IsColorString(s))
			{
				s += 2;
				continue;
			}
			CG_DrawChar(xx + 2, y + 2, charWidth, charHeight, *s, color);
			cnt++;
			xx += charWidth;
			s++;
		}
	}



	// draw the colored text
	s = string;
	xx = x;
	cnt = 0;
	trap_R_SetColor(setColor);
	while(*s && cnt < maxChars)
	{
		if(Q_IsColorString(s))
		{
			if(!forceColor)
			{
				memcpy(color, g_color_table[ColorIndex(*(s + 1))], sizeof(color));
				color[3] = setColor[3];
				trap_R_SetColor(color);
			}
			s += 2;
			continue;
		}
		CG_DrawChar(xx, y, charWidth, charHeight, *s, color);

		xx += charWidth;
		cnt++;
		s++;
	}
	trap_R_SetColor(NULL);
}

void CG_DrawMultiFontStringExt(int x, int y, const char *string, const float *setColor,
							   qboolean forceColor, qboolean shadow, int charWidth, int charHeight, int maxChars,
							   qhandle_t ChShader)
{
	vec4_t          color;
	const char     *s;
	int             xx;
	int             cnt;



	if(maxChars <= 0)
		maxChars = 32767;		// do them all!

	// draw the drop shadow
	if(shadow)
	{
		color[0] = color[1] = color[2] = 0;
		color[3] = setColor[3];
		trap_R_SetColor(color);
		s = string;
		xx = x;
		cnt = 0;
		while(*s && cnt < maxChars)
		{
			if(Q_IsColorString(s))
			{
				s += 2;
				continue;
			}
			CG_DrawMultiFontChar(xx + 2, y + 2, charWidth, charHeight, *s, color, ChShader);
			cnt++;
			xx += charWidth;
			s++;
		}
	}



	// draw the colored text
	s = string;
	xx = x;
	cnt = 0;
	trap_R_SetColor(setColor);
	while(*s && cnt < maxChars)
	{
		if(Q_IsColorString(s))
		{
			if(!forceColor)
			{
				memcpy(color, g_color_table[ColorIndex(*(s + 1))], sizeof(color));
				color[3] = setColor[3];
				trap_R_SetColor(color);
			}
			s += 2;
			continue;
		}
		CG_DrawMultiFontChar(xx, y, charWidth, charHeight, *s, color, ChShader);

		xx += charWidth;
		cnt++;
		s++;
	}
	trap_R_SetColor(NULL);
}

#define BLINKMASK 0x000007FF	// 2047
#define FLASHMASK 0x000003FF
/*
==================
CH_DrawStringExt

Simple & fast drawing of stuff including new escapes as ^B, ^f ^F and ^8
            Coordinates are at 640 by 480 virtual resolution
==================
*/

int CH_DrawStringExt(int x, int y, const char *string, int charWidth, int charHeight, const float *hcolor, int count,
					 qboolean forcecolor)
{
	vec4_t          color = { 1.0, 1.0, 1.0, 1.0 };
	vec4_t          color1 = { 1.0, 1.0, 1.0, 1.0 };
	const char     *s, *ss;
	int             xx, xxx;
	int             t;
	int             ch;
	float           chw, chh;
	float           row, col, frow, fcol;
	int             n;
	float           defalpha;	// default alpha value to switch backtowhenusing ^N or something;
	qboolean        shadow;
	qhandle_t       ChShader;

//  char    p[1024];

	// draw the colored text
	chw = charWidth * cgs.screenXScale;
	chh = charHeight * cgs.screenYScale;
	s = string;
	xx = x;
	n = 0;
	ss = string;
	xxx = x;
	shadow = qfalse;

	ChShader = cgs.media.charsetShader;

	if(hcolor)
	{
		color[0] = hcolor[0];
		color[1] = hcolor[1];
		color[2] = hcolor[2];
		color[3] = hcolor[3];
	}
	trap_R_SetColor(color);
	defalpha = color[3];

	if(!count)
	{
		count = 32767;
	}


	while(*s)
	{

		if(Q_IsColorString(s))
		{
			// advance 2 chars already
			s += 2;
			// macro has also checked if next char is \0
			switch (*(s - 1))
			{
				case 'A':
				case 'a':
					ChShader = cgs.media.ZcharsetShader;
					continue;
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '8':
				case '7':
				case '9':
					if(forcecolor == qtrue)
					{
						continue;
					}
					t = (*(s - 1) - '0') % 10;	//15;
					color[0] = g_color_table[t][0];
					color[1] = g_color_table[t][1];
					color[2] = g_color_table[t][2];
					trap_R_SetColor(color);
					continue;

					/* invisible, can only be broken by an ^N */
				case 'i':
				case 'I':
//                      parse_I:
					while(*s)
					{
						if(*s == Q_COLOR_ESCAPE && (*(s + 1) == 'n' || *(s + 1) == 'N'))
						{
							s += 2;
							break;
						}
						s++;
					}
					continue;
				case 'n':
				case 'N':
				  parse_N:
					color[3] = defalpha;
					trap_R_SetColor(color);
					ChShader = cgs.media.charsetShader;
					//  Q_CleanAbsoluteColorStr( strcpy( p, s ) );
					continue;
				case 'b':
//                  parse_b:
					t = cg.time & BLINKMASK;

					if(t > 1024)
						t = -t + 2047;
					color[3] = ((float)t) / 1024;
					trap_R_SetColor(color);
					continue;
				case 'B':
//                  parse_B:
					t = cg.time & BLINKMASK;
					if(t > 1024)
						t = -t + 2047;
					color[3] = ((float)t / 1463) + 0.3;
					trap_R_SetColor(color);
					continue;
				case 'F':
				  parse_F:

					if((cg.time & FLASHMASK) >= 512)
					{
						// we're in timeframe 512 - 1023 msec
						//skip all characters until ^N or ^f found
						while(*s)
						{
							if(*s == Q_COLOR_ESCAPE && *(s + 1))
							{
								if(*(s + 1) == 'N')
									goto parse_N;
								else if(*(s + 1) == 'f')
									goto parse_f;

								// can't absorb characters for their could be a pending ^B or something
							}
							s++;
						}
					}
					continue;
				case 'f':
				  parse_f:
					if((cg.time & FLASHMASK) < 512)
					{
						// we're in timeframe 0 - 511 msec
						//skip all characters until ^N or ^F found
						while(*s)
						{
							if(*s == Q_COLOR_ESCAPE && *(s + 1))
							{
								if(*(s + 1) == 'N')
									goto parse_N;
								else if(*(s + 1) == 'F')
									goto parse_F;

							}
							s++;
						}
					}
					continue;
				case 'X':
				case 'x':		// ^xrrggbb r,g,b e {0..9} U {A..F}
					//  parse_X:
					if(Q_IsAbsoluteColorString(s))
					{
						ss = s;
						xxx = xx;
						while(*ss)
						{
							if(Q_IsAbsoluteColorString(ss))
							{
								color1[0] = ((float)(gethex(*(ss)) * 16 + gethex(*(ss + 1)))) / 255.00;
								color1[1] = ((float)(gethex(*(ss + 2)) * 16 + gethex(*(ss + 3)))) / 255.00;
								color1[2] = ((float)(gethex(*(ss + 4)) * 16 + gethex(*(ss + 5)))) / 255.00;
								ss += 6;
							}
							if(*ss == Q_COLOR_ESCAPE && *(ss + 1))
							{
								if(*(ss + 1) == 'N' || *(ss + 1) == 'n')
								{
									break;
								}
								if(*(ss + 1) == 'F')
								{
									goto parse_F;
								}
								if(*(ss + 1) == 'f')
								{
									goto parse_f;
								}
								if(*(ss + 1) == 'b')
								{
									ss += 2;
								}
								if(*(ss + 1) == 'B')
								{
									ss += 2;
								}
								if(*(ss + 1) == '0')
								{
									ss += 2;
								}
								if(*(ss + 1) == 'I' || *(ss + 1) == 'i')
								{
									ss += 2;
								}
							}
							if(Q_IsColorString(ss))
							{
								ss += 2;
							}
							trap_R_SetColor(color1);
							CG_DrawMultiFontChar(xxx + 1, y + 1, charWidth, charHeight, *ss, color1, ChShader);
							xxx += charWidth;
							ss++;
						}
						s += 6;
					}
					continue;
				default:
					continue;
			}
		}


		if(++n > count)
		{
			return xx - x;
		}
		ch = *s;
		// Test #1
		if(ch != ' ')
		{
			row = ch >> 4;
			col = ch & 15;
			frow = row * 0.0625;
			fcol = col * 0.0625;

			trap_R_DrawStretchPic(xx * cgs.screenXScale, y * cgs.screenYScale,
								  chw, chh, fcol, frow, fcol + 0.0625, frow + 0.0625, ChShader);

		}
		trap_R_SetColor(color);
		CG_DrawMultiFontChar(xx, y, charWidth, charHeight, *s, color, ChShader);

		xx += charWidth;
		s++;

	}
	trap_R_SetColor(NULL);
	return xx - x;
}

int CH_DrawStringExtTiny(int x, int y, const char *string, int charWidth, int charHeight, const float *hcolor, int count,
						 qboolean forcecolor)
{
	vec4_t          color = { 1.0, 1.0, 1.0, 1.0 };
	vec4_t          color1 = { 1.0, 1.0, 1.0, 1.0 };
	const char     *s;
	int             xx;
	int             t;
	int             ch;
	float           chw, chh;
	float           row, col, frow, fcol;
	int             n;
	float           defalpha;	// default alpha value to switch backtowhenusing ^N or something;

	// draw the colored text
	chw = charWidth * cgs.screenXScale;
	chh = charHeight * cgs.screenYScale;
	s = string;
	xx = x;
	n = 0;

	if(hcolor)
	{
		color[0] = hcolor[0];
		color[1] = hcolor[1];
		color[2] = hcolor[2];
		color[3] = hcolor[3];
	}
	trap_R_SetColor(color);
	defalpha = color[3];

	if(!count)
		count = 32767;

	while(*s)
	{

		if(Q_IsColorString(s))
		{
			// advance 2 chars already
			s += 2;
			// macro has also checked if next char is \0
			switch (*(s - 1))
			{
				case '7':
					// to also force the color of the 'neutral' white color
					/*if (hcolor) {
					   trap_R_SetColor(hcolor);
					   break;
					   } */

				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '8':
				case '9':
					if(forcecolor == qtrue)
						continue;
					t = (*(s - 1) - '0') % 10;	//15;
					color[0] = g_color_table[t][0];
					color[1] = g_color_table[t][1];
					color[2] = g_color_table[t][2];
					trap_R_SetColor(color);
					continue;
					/* invisible, can only be broken by an ^N */
				case 'i':
				case 'I':
					while(*s)
					{
						if(*s == Q_COLOR_ESCAPE && (*(s + 1) == 'n' || *(s + 1) == 'N'))
						{
							s += 2;
							break;
						}
						s++;
					}
					continue;
				case 'n':
				case 'N':
				  parse_N:
					color[3] = defalpha;
					trap_R_SetColor(color);
					continue;
				case 'b':

					t = cg.time & BLINKMASK;

					if(t > 1024)
						t = -t + 2047;
					color[3] = ((float)t) / 1024;
					trap_R_SetColor(color);
					continue;
				case 'B':
					t = cg.time & BLINKMASK;
					if(t > 1024)
						t = -t + 2047;
					color[3] = ((float)t / 1463) + 0.3;
					trap_R_SetColor(color);
					continue;
				case 'F':
				  parse_F:

					if((cg.time & FLASHMASK) >= 512)
					{
						// we're in timeframe 512 - 1023 msec
						//skip all characters until ^N or ^f found
						while(*s)
						{
							if(*s == Q_COLOR_ESCAPE && *(s + 1))
							{
								if(*(s + 1) == 'N')
									goto parse_N;
								else if(*(s + 1) == 'f')
									goto parse_f;
								// can't absorb characters for their could be a pending ^B or something
							}
							s++;
						}
					}
					continue;
				case 'f':
				  parse_f:
					if((cg.time & FLASHMASK) < 512)
					{
						// we're in timeframe 0 - 511 msec
						//skip all characters until ^N or ^F found
						while(*s)
						{
							if(*s == Q_COLOR_ESCAPE && *(s + 1))
							{
								if(*(s + 1) == 'N')
									goto parse_N;
								else if(*(s + 1) == 'F')
									goto parse_F;
								// can't absorb characters for their could be a pending ^B or something
							}
							s++;
						}

						continue;
				case 'X':
				case 'x':		// ^xrrggbb r,g,b e {0..9} U {A..F}
						//  parse_X:            
						if(Q_IsAbsoluteColorString(s))
						{
							color1[0] = ((float)(gethex(*(s)) * 16 + gethex(*(s + 1)))) / 255.00;
							color1[1] = ((float)(gethex(*(s + 2)) * 16 + gethex(*(s + 3)))) / 255.00;
							color1[2] = ((float)(gethex(*(s + 4)) * 16 + gethex(*(s + 5)))) / 255.00;
							s += 6;

							trap_R_SetColor(color1);
							CG_DrawStringExt(xx + 0.5f, y + 0.5f, s, color1, qtrue, qfalse, charWidth, charHeight, count);

						}

					}
			}
			continue;
		}
		if(++n > count)
		{
			return xx - x;
		}
		ch = *s;
		// Test #1
		if(ch != ' ')
		{
			row = ch >> 4;
			col = ch & 15;
			frow = row * 0.0625;
			fcol = col * 0.0625;

			trap_R_DrawStretchPic(xx * cgs.screenXScale, y * cgs.screenYScale,
								  chw, chh, fcol, frow, fcol + 0.0625, frow + 0.0625, cgs.media.charsetShader);

		}
		trap_R_SetColor(color);
		CG_DrawChar(xx, y, charWidth, charHeight, *s, color);
		xx += charWidth;
		s++;
	}
	trap_R_SetColor(NULL);
	return xx - x;				//useful for concatenation (in menu system f.i.)
}

void CG_DrawBigString(int x, int y, const char *s, float alpha)
{
	float           color[4];

	color[0] = color[1] = color[2] = 1.0;
	color[3] = alpha;
//  CG_DrawStringExt( x, y, s, color, qfalse, qtrue, BIGCHAR_WIDTH, BIGCHAR_HEIGHT, 0 );
	CH_DrawStringExt(x, y, s, BIGCHAR_WIDTH, BIGCHAR_HEIGHT, color, 0, qfalse);

}

void CG_DrawBigStringColor(int x, int y, const char *s, vec4_t color)
{
//  CG_DrawStringExt( x, y, s, color, qtrue, qtrue, BIGCHAR_WIDTH, BIGCHAR_HEIGHT, 0 );
	CH_DrawStringExt(x, y, s, BIGCHAR_WIDTH, BIGCHAR_HEIGHT, color, 0, qfalse);

}

void CG_DrawSmallString(int x, int y, const char *s, float alpha)
{
	float           color[4];

	color[0] = color[1] = color[2] = 1.0;
	color[3] = alpha;
//  CG_DrawStringExt( x, y, s, color, qfalse, qfalse, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0 );
	CH_DrawStringExt(x, y, s, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, color, 0, qfalse);

}

void CG_DrawSmallStringColor(int x, int y, const char *s, vec4_t color)
{
//  CG_DrawStringExt( x, y, s, color, qtrue, qfalse, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0 );
	CH_DrawStringExt(x, y, s, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, color, 0, qfalse);

}

/*
=================
CG_DrawStrlen

Returns character count, skiping color escape codes
=================
*/
int CG_DrawStrlen(const char *str)
{
	const char     *s = str;
	int             count = 0;

	while(*s)
	{
		if(Q_IsAbsoluteColorString(s))
		{
			s += 8;
		}
		else if(Q_IsColorString(s))
		{

			s += 2;
		}
		else
		{
			count++;
			s++;
		}
	}

	return count;
}

/*
=============
CG_TileClearBox

This repeats a 64*64 tile graphic to fill the screen around a sized down
refresh window.
=============
*/
static void CG_TileClearBox(int x, int y, int w, int h, qhandle_t hShader)
{
	float           s1, t1, s2, t2;

	s1 = x / 64.0;
	t1 = y / 64.0;
	s2 = (x + w) / 64.0;
	t2 = (y + h) / 64.0;
	trap_R_DrawStretchPic(x, y, w, h, s1, t1, s2, t2, hShader);
}



/*
==============
CG_TileClear

Clear around a sized down screen
==============
*/
void CG_TileClear(void)
{
	int             top, bottom, left, right;
	int             w, h;

	w = cgs.glconfig.vidWidth;
	h = cgs.glconfig.vidHeight;

	if(cg.refdef[0].x == 0 && cg.refdef[0].y == 0 && cg.refdef[0].width == w && cg.refdef[0].height == h)
	{
		return;					// full screen rendering
	}

	top = cg.refdef[0].y;
	bottom = top + cg.refdef[0].height - 1;
	left = cg.refdef[0].x;
	right = left + cg.refdef[0].width - 1;

	// clear above view screen
	CG_TileClearBox(0, 0, w, top, cgs.media.backTileShader);

	// clear below view screen
	CG_TileClearBox(0, bottom, w, h - bottom, cgs.media.backTileShader);

	// clear left of view screen
	CG_TileClearBox(0, top, left, bottom - top + 1, cgs.media.backTileShader);

	// clear right of view screen
	CG_TileClearBox(right, top, w - right, bottom - top + 1, cgs.media.backTileShader);
}



/*
=================
UI_DrawProportionalString
=================
*/
void UI_DrawProportionalString(int x, int y, const char *str, int style, vec4_t color)
{
	vec4_t          drawcolor;
	int             width;
	float           sizeScale;

	sizeScale = UI_ProportionalSizeScale(style);

	switch (style & UI_FORMATMASK)
	{
		case UI_CENTER:
			width = UI_ProportionalStringWidth(str) * sizeScale;
			x -= width / 2;
			break;

		case UI_RIGHT:
			width = UI_ProportionalStringWidth(str) * sizeScale;
			x -= width;
			break;

		case UI_LEFT:
		default:
			break;
	}

	if(style & UI_DROPSHADOW)
	{
		drawcolor[0] = drawcolor[1] = drawcolor[2] = 0;
		drawcolor[3] = color[3];
		UI_DrawProportionalString2(x + 2, y + 2, str, drawcolor, sizeScale, cgs.media.charsetProp);
	}

	if(style & UI_INVERSE)
	{
		drawcolor[0] = color[0] * 0.8;
		drawcolor[1] = color[1] * 0.8;
		drawcolor[2] = color[2] * 0.8;
		drawcolor[3] = color[3];
		UI_DrawProportionalString2(x, y, str, drawcolor, sizeScale, cgs.media.charsetProp);
		return;
	}

	if(style & UI_PULSE)
	{
		drawcolor[0] = color[0] * 0.8;
		drawcolor[1] = color[1] * 0.8;
		drawcolor[2] = color[2] * 0.8;
		drawcolor[3] = color[3];
		UI_DrawProportionalString2(x, y, str, color, sizeScale, cgs.media.charsetProp);

		drawcolor[0] = color[0];
		drawcolor[1] = color[1];
		drawcolor[2] = color[2];
		drawcolor[3] = 0.5 + 0.5 * sin(cg.time / PULSE_DIVISOR);
		UI_DrawProportionalString2(x, y, str, drawcolor, sizeScale, cgs.media.charsetPropGlow);
		return;
	}

	UI_DrawProportionalString2(x, y, str, color, sizeScale, cgs.media.charsetProp);
}


#endif							// Q3STATIC
