/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006 Robert Beckebans <trebor_7@users.sourceforge.net>
Copyright (C) 2006 Josef Soentgen <cnuke@users.sourceforge.net>

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
// cg_draw.c -- draw all of the graphical elements during
// active (after loading) gameplay

#include "cg_local.h"

#ifdef MISSIONPACK
#include "../ui/ui_shared.h"

// used for scoreboard
extern displayContextDef_t cgDC;
menuDef_t      *menuScoreboard = NULL;
#else
int             drawTeamOverlayModificationCount = -1;
#endif

int             sortedTeamPlayers[TEAM_MAXOVERLAY];
int             numSortedTeamPlayers;

char            systemChat[256];
char            teamChat1[256];
char            teamChat2[256];

#ifdef MISSIONPACK

int CG_Text_Width(const char *text, float scale, int limit)
{
	int             count, len;
	float           out;
	glyphInfo_t    *glyph;
	float           useScale;

// FIXME: see ui_main.c, same problem
//  const unsigned char *s = text;
	const char     *s = text;
	fontInfo_t     *font = &cgDC.Assets.textFont;

	if(scale <= cg_smallFont.value)
	{
		font = &cgDC.Assets.smallFont;
	}
	else if(scale > cg_bigFont.value)
	{
		font = &cgDC.Assets.bigFont;
	}
	useScale = scale * font->glyphScale;
	out = 0;
	if(text)
	{
		len = strlen(text);
		if(limit > 0 && len > limit)
		{
			len = limit;
		}
		count = 0;
		while(s && *s && count < len)
		{
			if(Q_IsColorString(s))
			{
				s += 2;
				continue;
			}
			else
			{
				glyph = &font->glyphs[(int)*s];	// TTimo: FIXME: getting nasty warnings without the cast, hopefully this doesn't break the VM build
				out += glyph->xSkip;
				s++;
				count++;
			}
		}
	}
	return out * useScale;
}

int CG_Text_Height(const char *text, float scale, int limit)
{
	int             len, count;
	float           max;
	glyphInfo_t    *glyph;
	float           useScale;

// TTimo: FIXME
//  const unsigned char *s = text;
	const char     *s = text;
	fontInfo_t     *font = &cgDC.Assets.textFont;

	if(scale <= cg_smallFont.value)
	{
		font = &cgDC.Assets.smallFont;
	}
	else if(scale > cg_bigFont.value)
	{
		font = &cgDC.Assets.bigFont;
	}
	useScale = scale * font->glyphScale;
	max = 0;
	if(text)
	{
		len = strlen(text);
		if(limit > 0 && len > limit)
		{
			len = limit;
		}
		count = 0;
		while(s && *s && count < len)
		{
			if(Q_IsColorString(s))
			{
				s += 2;
				continue;
			}
			else
			{
				glyph = &font->glyphs[(int)*s];	// TTimo: FIXME: getting nasty warnings without the cast, hopefully this doesn't break the VM build
				if(max < glyph->height)
				{
					max = glyph->height;
				}
				s++;
				count++;
			}
		}
	}
	return max * useScale;
}

void CG_Text_PaintChar(float x, float y, float width, float height, float scale, float s, float t, float s2, float t2,
					   qhandle_t hShader)
{
	float           w, h;

	w = width * scale;
	h = height * scale;
	CG_AdjustFrom640(&x, &y, &w, &h);
	trap_R_DrawStretchPic(x, y, w, h, s, t, s2, t2, hShader);
}

void CG_Text_Paint(float x, float y, float scale, vec4_t color, const char *text, float adjust, int limit, int style)
{
	int             len, count;
	vec4_t          newColor;
	glyphInfo_t    *glyph;
	float           useScale;
	fontInfo_t     *font = &cgDC.Assets.textFont;

	if(scale <= cg_smallFont.value)
	{
		font = &cgDC.Assets.smallFont;
	}
	else if(scale > cg_bigFont.value)
	{
		font = &cgDC.Assets.bigFont;
	}
	useScale = scale * font->glyphScale;
	if(text)
	{
// TTimo: FIXME
//      const unsigned char *s = text;
		const char     *s = text;

		trap_R_SetColor(color);
		memcpy(&newColor[0], &color[0], sizeof(vec4_t));
		len = strlen(text);
		if(limit > 0 && len > limit)
		{
			len = limit;
		}
		count = 0;
		while(s && *s && count < len)
		{
			glyph = &font->glyphs[(int)*s];	// TTimo: FIXME: getting nasty warnings without the cast, hopefully this doesn't break the VM build
			// int yadj = Assets.textFont.glyphs[text[i]].bottom + Assets.textFont.glyphs[text[i]].top;
			// float yadj = scale * (Assets.textFont.glyphs[text[i]].imageHeight - Assets.textFont.glyphs[text[i]].height);
			if(Q_IsColorString(s))
			{
				memcpy(newColor, g_color_table[ColorIndex(*(s + 1))], sizeof(newColor));
				newColor[3] = color[3];
				trap_R_SetColor(newColor);
				s += 2;
				continue;
			}
			else
			{
				float           yadj = useScale * glyph->top;

				if(style == ITEM_TEXTSTYLE_SHADOWED || style == ITEM_TEXTSTYLE_SHADOWEDMORE)
				{
					int             ofs = style == ITEM_TEXTSTYLE_SHADOWED ? 1 : 2;

					colorBlack[3] = newColor[3];
					trap_R_SetColor(colorBlack);
					CG_Text_PaintChar(x + ofs, y - yadj + ofs,
									  glyph->imageWidth,
									  glyph->imageHeight, useScale, glyph->s, glyph->t, glyph->s2, glyph->t2, glyph->glyph);
					colorBlack[3] = 1.0;
					trap_R_SetColor(newColor);
				}
				CG_Text_PaintChar(x, y - yadj,
								  glyph->imageWidth,
								  glyph->imageHeight, useScale, glyph->s, glyph->t, glyph->s2, glyph->t2, glyph->glyph);
				// CG_DrawPic(x, y - yadj, scale * cgDC.Assets.textFont.glyphs[text[i]].imageWidth, scale * cgDC.Assets.textFont.glyphs[text[i]].imageHeight, cgDC.Assets.textFont.glyphs[text[i]].glyph);
				x += (glyph->xSkip * useScale) + adjust;
				s++;
				count++;
			}
		}
		trap_R_SetColor(NULL);
	}
}
#endif


/*
==============
CG_DrawField

Draws large numbers for status bar and powerups
==============
*/
#ifndef MISSIONPACK
static void CG_DrawField(int x, int y, int width, int value, qboolean Digital)
{
	char            num[16], *ptr;
	int             l;
	int             frame;

	if(width < 1)
	{
		return;
	}

	// draw number string
	if(width > 5)
	{
		width = 5;
	}

	switch (width)
	{
		case 1:
			value = value > 9 ? 9 : value;
			value = value < 0 ? 0 : value;
			break;
		case 2:
			value = value > 99 ? 99 : value;
			value = value < -9 ? -9 : value;
			break;
		case 3:
			value = value > 999 ? 999 : value;
			value = value < -99 ? -99 : value;
			break;
		case 4:
			value = value > 9999 ? 9999 : value;
			value = value < -999 ? -999 : value;
			break;
	}

	Com_sprintf(num, sizeof(num), "%i", value);
	l = strlen(num);
	if(l > width)
		l = width;
	x += 2 + CHAR_WIDTH * (width - l) / 2;

	ptr = num;
	while(*ptr && l)
	{
		if(*ptr == '-')
			frame = STAT_MINUS;
		else
			frame = *ptr - '0';
		if(!Digital)
		{
			CG_DrawPic(x, y, CHAR_WIDTH / 2, CHAR_HEIGHT / 2, cgs.media.numberShaders[frame]);
		}
		else
		{
			CG_DrawPic(x, y, CHAR_WIDTH / 2, CHAR_HEIGHT / 2, cgs.media.digitalnumShaders[frame]);
		}
		x += CHAR_WIDTH / 2;
		ptr++;
		l--;
	}
}
#endif							// MISSIONPACK

/*
================
CG_Draw3DModel

================
*/
void CG_Draw3DModel(float x, float y, float w, float h, qhandle_t model, qhandle_t skin, vec3_t origin, vec3_t angles)
{
	refdef_t        refdef;
	refEntity_t     ent;
	refLight_t		light;

	if(!cg_draw3dIcons.integer || !cg_drawIcons.integer)
	{
		return;
	}

	CG_AdjustFrom640(&x, &y, &w, &h);

	memset(&refdef, 0, sizeof(refdef));

	memset(&ent, 0, sizeof(ent));
	AnglesToAxis(angles, ent.axis);
	VectorCopy(origin, ent.origin);
	ent.hModel = model;
	ent.customSkin = skin;
	ent.renderfx = RF_NOSHADOW;	// no stencil shadows

	refdef.rdflags = RDF_NOWORLDMODEL | RDF_NOSHADOWS;

	AxisClear(refdef.viewaxis);

	refdef.fov_x = 30;
	refdef.fov_y = 30;

	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;

	refdef.time = cg.time;

	trap_R_ClearScene();
	trap_R_AddRefEntityToScene(&ent);
	
	// add light
	memset(&light, 0, sizeof(refLight_t));
	
	light.rlType = RL_PROJ;
	
	VectorMA(refdef.vieworg, -30, refdef.viewaxis[0], light.origin);
	
	VectorCopy(refdef.viewaxis[0], light.axis[0]);
	VectorCopy(refdef.viewaxis[1], light.axis[1]);
	VectorCopy(refdef.viewaxis[2], light.axis[2]);
	
	light.color[0] = 1.0;
	light.color[1] = 1.0;
	light.color[2] = 1.0;
	
	light.fovX = 90;
	light.fovY = 90;
	light.distance = 800;
	
	trap_R_AddRefLightToScene(&light);
	
	trap_R_RenderScene(&refdef);
}


/*
================
CG_Draw3DWeaponModel
================
*/
void CG_Draw3DWeaponModel(float x, float y, float w, float h, qhandle_t weaponModel, qhandle_t barrelModel, qhandle_t skin,
						  vec3_t origin, vec3_t angles)
{
	refdef_t        refdef;
	refEntity_t     ent;
	refLight_t		light;

	if(!cg_draw3dIcons.integer || !cg_drawIcons.integer)
	{
		return;
	}

	CG_AdjustFrom640(&x, &y, &w, &h);

	memset(&refdef, 0, sizeof(refdef));

	memset(&ent, 0, sizeof(ent));
	AnglesToAxis(angles, ent.axis);
	VectorCopy(origin, ent.origin);
	ent.hModel = weaponModel;
	ent.customSkin = skin;
	ent.renderfx = RF_NOSHADOW;	// no stencil shadows

	refdef.rdflags = RDF_NOWORLDMODEL | RDF_NOSHADOWS;

	AxisClear(refdef.viewaxis);

	refdef.fov_x = 30;
	refdef.fov_y = 30;

	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;

	refdef.time = cg.time;

	trap_R_ClearScene();

	trap_R_AddRefEntityToScene(&ent);
	if(barrelModel)
	{
		refEntity_t     barrel;

		memset(&barrel, 0, sizeof(barrel));

		barrel.hModel = barrelModel;

		VectorCopy(ent.lightingOrigin, barrel.lightingOrigin);
		barrel.shadowPlane = ent.shadowPlane;
		barrel.renderfx = ent.renderfx;

		CG_PositionRotatedEntityOnTag(&barrel, &ent, weaponModel, "tag_barrel");

		AxisCopy(ent.axis, barrel.axis);
		barrel.nonNormalizedAxes = ent.nonNormalizedAxes;

		trap_R_AddRefEntityToScene(&barrel);
	}
	
	// add light
	memset(&light, 0, sizeof(refLight_t));
	
	light.rlType = RL_PROJ;
	
	VectorCopy(refdef.viewaxis[0], light.axis[0]);
	VectorCopy(refdef.viewaxis[1], light.axis[1]);
	VectorCopy(refdef.viewaxis[2], light.axis[2]);
	
	light.color[0] = 1.0;
	light.color[1] = 1.0;
	light.color[2] = 1.0;
	
	light.fovX = 90;
	light.fovY = 90;
	light.distance = 800;
	
	VectorMA(refdef.vieworg, -30, refdef.viewaxis[0], light.origin);
	
	//light.origin[1] -= 20;
	trap_R_AddRefLightToScene(&light);

	trap_R_RenderScene(&refdef);
}

/*
================
CG_DrawHead

Used for both the status bar and the scoreboard
================
*/
void CG_DrawHead(float x, float y, float w, float h, int clientNum, vec3_t headAngles)
{
	clipHandle_t    cm;
	clientInfo_t   *ci;
	float           len;
	vec3_t          origin;
	vec3_t          mins, maxs;

	ci = &cgs.clientinfo[clientNum];

	if(cg_draw3dIcons.integer)
	{
		cm = ci->headModel;
		if(!cm)
		{
			return;
		}

		// offset the origin y and z to center the head
		trap_R_ModelBounds(cm, mins, maxs);

		origin[2] = -0.5 * (mins[2] + maxs[2]);
		origin[1] = 0.5 * (mins[1] + maxs[1]);

		// calculate distance so the head nearly fills the box
		// assume heads are taller than wide
		len = 0.7 * (maxs[2] - mins[2]);
		origin[0] = len / 0.268;	// len / tan( fov/2 )

		// allow per-model tweaking
		VectorAdd(origin, ci->headOffset, origin);

		CG_Draw3DModel(x, y, w, h, ci->headModel, ci->headSkin, origin, headAngles);
	}
	else if(cg_drawIcons.integer)
	{
		CG_DrawPic(x, y, w, h, ci->modelIcon);
	}

	// if they are deferred, draw a cross out
	if(ci->deferred)
	{
		CG_DrawPic(x, y, w, h, cgs.media.deferShader);
	}
}

/*
================
CG_DrawFlagModel

Used for both the status bar and the scoreboard
================
*/
void CG_DrawFlagModel(float x, float y, float w, float h, int team, qboolean force2D)
{
	qhandle_t       cm;
	float           len;
	vec3_t          origin, angles;
	vec3_t          mins, maxs;

//  qhandle_t       handle;

	if(!force2D && cg_draw3dIcons.integer)
	{

		VectorClear(angles);

		cm = cgs.media.redFlagModel;

		// offset the origin y and z to center the flag
		trap_R_ModelBounds(cm, mins, maxs);

		origin[2] = -0.5 * (mins[2] + maxs[2]);
		origin[1] = 0.5 * (mins[1] + maxs[1]);

		// calculate distance so the flag nearly fills the box
		// assume heads are taller than wide
		len = 0.5 * (maxs[2] - mins[2]);
		origin[0] = len / 0.268;	// len / tan( fov/2 )

		angles[YAW] = 60 * sin(cg.time / 2000.0);;

		if(team == TEAM_RED)
		{
			CG_Draw3DModel(x, y, w, h, cgs.media.redFlagModel, 0, origin, angles);
		}
		else if(team == TEAM_BLUE)
		{
			CG_Draw3DModel(x, y, w, h, cgs.media.blueFlagModel, 0, origin, angles);
		}
		else if(team == TEAM_FREE)
		{
			CG_Draw3DModel(x, y, w, h, cgs.media.redFlagModel, 0, origin, angles);
			angles[YAW] += 180;
			CG_Draw3DModel(x, y, w, h, cgs.media.blueFlagModel, 0, origin, angles);
			return;
		}
	}
	else if(cg_drawIcons.integer)
	{
		gitem_t        *item;

		//NT - Changed this stuff a bit - a player can hold both flags when g_returnFlag is 1
		if(team == TEAM_RED)
		{
			item = BG_FindItemForPowerup(PW_REDFLAG);
		}
		else if(team == TEAM_BLUE)
		{
			item = BG_FindItemForPowerup(PW_BLUEFLAG);
		}
		else if(team == TEAM_FREE)
		{
			CG_DrawPic(x, y, w, h, cgs.media.redBlueFlagShader);
			return;
		}
		else
		{
			return;
		}

		if(item)
		{
			CG_DrawPic(x, y, w, h, cg_items[ITEM_INDEX(item)].icon);
		}
	}
}

/*
================
CG_DrawStatusBarHead

================
*/
#ifndef MISSIONPACK

static void CG_DrawStatusBarHead(float x, float y)
{
	vec3_t          angles;
	float           size;
	float           frac;

	VectorClear(angles);

/*	if ( cg.damageTime && cg.time - cg.damageTime < DAMAGE_TIME ) {
		frac = (float)(cg.time - cg.damageTime ) / DAMAGE_TIME;
		size = ICON_SIZE * 1.25 * ( 1.5 - frac * 0.5 );

		stretch = size - ICON_SIZE * 1.25;
		// kick in the direction of damage
		x -= stretch * 0.5 + cg.damageX * stretch * 0.5;

		cg.headStartYaw = 180 + cg.damageX * 45;

		cg.headEndYaw = 180 + 20 * cos( crandom()*M_PI );
		cg.headEndPitch = 5 * cos( crandom()*M_PI );

		cg.headStartTime = cg.time;
		cg.headEndTime = cg.time + 100 + random() * 2000;
	} else {*/
	if(cg.time >= cg.headEndTime)
	{
		// select a new head angle
		cg.headStartYaw = cg.headEndYaw;
		cg.headStartPitch = cg.headEndPitch;
		cg.headStartTime = cg.headEndTime;
		cg.headEndTime = cg.time + 100 + random() * 2000;

		cg.headEndYaw = 180 + 20 * cos(crandom() * M_PI);
		cg.headEndPitch = 5 * cos(crandom() * M_PI);
	}

	size = ICON_SIZE * 1.25;
//  }

	// if the server was frozen for a while we may have a bad head start time
	if(cg.headStartTime > cg.time)
	{
		cg.headStartTime = cg.time;
	}

	frac = (cg.time - cg.headStartTime) / (float)(cg.headEndTime - cg.headStartTime);
	frac = frac * frac * (3 - 2 * frac);
	angles[YAW] = cg.headStartYaw + (cg.headEndYaw - cg.headStartYaw) * frac;
	angles[PITCH] = cg.headStartPitch + (cg.headEndPitch - cg.headStartPitch) * frac;

	CG_DrawHead(x, y - size, size / 2, size / 2, cg.snap->ps.clientNum, angles);
}
#endif							// MISSIONPACK

/*
================
CG_DrawStatusBarFlag

================
*/
#ifndef MISSIONPACK
static void CG_DrawStatusBarFlag(float x, int team)
{
	CG_DrawFlagModel(x, 480 - ICON_SIZE, ICON_SIZE, ICON_SIZE, team, qfalse);
}
#endif							// MISSIONPACK

/*
================
CG_DrawTeamBackground

================
*/
void CG_DrawTeamBackground(int x, int y, int w, int h, float alpha, int team)
{
	vec4_t          hcolor;

	hcolor[3] = alpha;
	if(team == TEAM_RED)
	{
		hcolor[0] = 1;
		hcolor[1] = 0;
		hcolor[2] = 0;
	}
	else if(team == TEAM_BLUE)
	{
		hcolor[0] = 0;
		hcolor[1] = 0;
		hcolor[2] = 1;
	}
	else
	{
		return;
	}
	trap_R_SetColor(hcolor);
	CG_DrawPic(x, y, w, h, cgs.media.teamStatusBar);
	trap_R_SetColor(NULL);
}

#define BLINKMASK2 0x000003FF	// 2047

/*
================
CG_DrawStatusBar

================
*/
#ifndef MISSIONPACK
static void CG_DrawStatusBar(void)
{
	int             color;
	centity_t      *cent;
	playerState_t  *ps;
	int             value, value2;
	int             charge;
	int             recharge;
	vec4_t          hcolor;
	vec4_t          color1, color2;
	vec3_t          angles;
	vec3_t          origin;
	int             t;
	static float    colors[4][4] = {
		{1.0f, 0.69f, 0.0f, 1.0f},	// normal
		{1.0f, 0.2f, 0.2f, 1.0f},	// low health
		{0.5f, 0.5f, 0.5f, 1.0f},	// weapon firing
		{1.0f, 1.0f, 1.0f, 1.0f}
	};							// health > 100

	if(cg_drawStatus.integer == 0)
	{
		return;
	}

	// draw the team background
	CG_DrawTeamBackground(0, 420, 640, 60, 0.33f, cg.snap->ps.persistant[PERS_TEAM]);

	cent = &cg_entities[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;

	VectorClear(angles);



	CG_DrawStatusBarHead(250 + 18, 483 - 1);


	if(cg.predictedPlayerState.powerups[PW_REDFLAG])
	{
		CG_DrawStatusBarFlag(10, TEAM_RED);
	}
	if(cg.predictedPlayerState.powerups[PW_BLUEFLAG])
	{
		CG_DrawStatusBarFlag(42, TEAM_BLUE);
	}


	origin[0] = 90;
	origin[1] = 0;
	origin[2] = -10;
	angles[YAW] = (cg.time & 2047) * 360 / 2048.0;
	CG_Draw3DModel(345, 428, ICON_SIZE / 2, ICON_SIZE / 2, cgs.media.armorModel, 0, origin, angles);

	//
	// health
	//

	value = ps->stats[STAT_HEALTH];
	t = cg.time & BLINKMASK2;
	t = -t + 2047;


	if(value <= 100)
	{
		if(value <= 100 && value > 50)
		{
			color2[0] = 0.5f;
			color2[1] = 0.8f;
			color2[2] = 0.1f;
			color2[3] = 0.5f;
			trap_R_SetColor(color2);
			CG_DrawRect(240 + 12 - value * 1.5f, 456 - 2, value * 1.5f, 1, 1, colorWhite);
			CG_FillRect(240 + 12 - value * 1.5f, 456, value * 1.5f, 17, color2);
			CG_DrawRect(240 + 12 - value * 1.5f, 473 + 1, value * 1.5f, 1, 1, colorWhite);
			trap_R_SetColor(NULL);
		}
		else if(value <= 50)
		{
			color2[0] = 1.0f;
			color2[1] = 0.0f;
			color2[2] = 0.0f;
			color2[3] = 0.8f;
			trap_R_SetColor(color2);
			CG_DrawRect(240 + 12 - value * 1.5f, 456 - 2, value * 1.5f, 1, 1, color2);
			CG_DrawRect(240 + 12 - value * 1.5f, 473 + 1, value * 1.5f, 1, 1, color2);
			trap_R_SetColor(NULL);
			color2[0] = 0.5f;
			color2[1] = 0.9f;
			color2[2] = 0.0f;
			color2[3] = 0.5f;
			trap_R_SetColor(color2);
			CG_FillRect(240 + 12 - value * 1.5f, 456, value * 1.5f, 17, color2);
			trap_R_SetColor(NULL);
			color1[0] = 0.9f;
			color1[1] = 0.0f;
			color1[2] = 0.0f;
			color1[3] = ((float)t / (value * 20));
			trap_R_SetColor(color1);
			CG_FillRect(240 + 12 - value * 1.5f, 456, value * 1.5f, 17, color1);
			trap_R_SetColor(NULL);
		}

	}
	else if(value > 100)
	{
		value2 = value + 1 / 2;
		color1[0] = 0.8f;
		color1[1] = 0.7f;
		color1[2] = 0.7f;
		color1[3] = 0.5f;
		trap_R_SetColor(color1);
		CG_DrawRect(240 + 12 - value2 * 1.5f + 150, 430 - 3, value * 1.5f - 150, 1, 1, colorYellow);
		CG_FillRect(240 + 12 - value2 * 1.5f + 150, 429, value * 1.5f - 150, 17, color1);
		CG_DrawRect(240 + 12 - value2 * 1.5f + 150, 447, value * 1.5f - 150, 1, 1, colorYellow);

		CG_DrawRect(240 + 12 - 100 * 1.5f, 456 - 2, 100 * 1.5f, 1, 1, colorYellow);
		CG_FillRect(240 + 12 - 100 * 1.5f, 456, 100 * 1.5f, 17, color1);
		CG_DrawRect(240 + 12 - 100 * 1.5f, 473 + 1, 100 * 1.5f, 1, 1, colorYellow);
		trap_R_SetColor(NULL);
	}


	if(value > 100)
	{
		trap_R_SetColor(colors[3]);	// white
	}
	else if(value > 25)
	{
		trap_R_SetColor(colors[0]);	// green
	}
	else if(value > 0)
	{
		color = (cg.time >> 8) & 1;	// flash
		trap_R_SetColor(colors[color]);
	}
	else
	{
		trap_R_SetColor(colors[1]);	// red
	}


	CG_DrawField(245 + 12, 454, 3, value, qfalse);
	CG_ColorForHealth(hcolor);
	trap_R_SetColor(hcolor);



	//
	// armor
	//

	value = ps->stats[STAT_ARMOR];
	if(value > 0)
	{
		if(value <= 100)
		{
			if(value <= 100 && value > 50)
			{
				color2[0] = 0.5f;
				color2[1] = 0.8f;
				color2[2] = 0.1f;
				color2[3] = 0.5f;
				trap_R_SetColor(color2);
				CG_DrawRect(390, 456 - 2, value * 1.5f, 1, 1, colorWhite);
				CG_FillRect(390, 456, value * 1.5f, 17, color2);
				CG_DrawRect(390, 473 + 1, value * 1.5f, 1, 1, colorWhite);
				trap_R_SetColor(NULL);
			}
			else if(value <= 50)
			{
				color2[0] = 0.9f;
				color2[1] = 0.9f;
				color2[2] = 0.0f;
				color2[3] = 0.8f;
				trap_R_SetColor(color2);
				CG_DrawRect(390, 456 - 2, value * 1.5f, 1, 1, color2);
				CG_DrawRect(390, 473 + 1, value * 1.5f, 1, 1, color2);
				trap_R_SetColor(NULL);
				color2[0] = 0.5f;
				color2[1] = 0.5f;
				color2[2] = 0.0f;
				color2[3] = 0.5f;
				trap_R_SetColor(color2);
				CG_FillRect(390, 456, value * 1.5f, 17, color2);
				trap_R_SetColor(NULL);
				color1[0] = 0.8f;
				color1[1] = 0.8f;
				color1[2] = 0.0f;
				color1[3] = ((float)t / (value * 20));
				trap_R_SetColor(color1);
				CG_FillRect(390, 456, value * 1.5f, 17, color1);
				trap_R_SetColor(NULL);
			}

		}
		else if(value > 100)
		{
			color1[0] = 0.8f;
			color1[1] = 0.7f;
			color1[2] = 0.7f;
			color1[3] = 0.5f;
			trap_R_SetColor(color1);
			CG_DrawRect(390, 430 - 3, value * 1.5f - 150, 1, 1, colorYellow);
			CG_FillRect(390, 429, value * 1.5f - 150, 17, color1);
			CG_DrawRect(390, 447, value * 1.5f - 150, 1, 1, colorYellow);

			CG_DrawRect(390, 456 - 2, 100 * 1.5f, 1, 1, colorYellow);
			CG_FillRect(390, 456, 100 * 1.5f, 17, color1);
			CG_DrawRect(390, 473 + 1, 100 * 1.5f, 1, 1, colorYellow);
			trap_R_SetColor(NULL);
		}
	}


	trap_R_SetColor(colors[0]);
	CG_DrawField(330, 454, 3, value, qfalse);
	trap_R_SetColor(NULL);
	CG_DrawRect(320, 425, 1, 50, 1, colorWhite);

	if(ps->weapon == WP_IRAILGUN)
	{


		charge = ps->stats[STAT_RAIL_CHARGE] / 70;
		if(charge < -111)
		{
			charge = -111;
		}
		if(charge > 0)
		{
			charge = -111;
		}
		color1[0] = 0.1f;
		color1[1] = 0.1f;
		color1[2] = 0.1f;
		color1[3] = 0.7f;
		trap_R_SetColor(color1);
		CG_FillRect(594, 369, 20, -116, color1);
		CG_FillRect(573, 396, 12, -55, color1);
		CG_FillRect(623, 396, 12, -55, color1);
		trap_R_SetColor(NULL);
		color1[0] = 0.1f;
		color1[1] = 0.1f;
		color1[2] = 0.1f;
		color1[3] = 0.7f;
		trap_R_SetColor(color1);
		CG_FillRect(597, 380, 16, 16, color1);
		trap_R_SetColor(NULL);
		if(ps->weaponstate == WEAPON_PREFIRING)
		{

			color1[0] = 0.9f;
			color1[1] = 0.0f;
			color1[2] = 0.1f;
			color1[3] = 0.7f;
			trap_R_SetColor(color1);
			CG_FillRect(597, 365, 16, charge, color1);
			trap_R_SetColor(NULL);
			//  CG_DrawPic( 601,365,8 , charge, cgs.media.chargefxMeterShader );
			CG_FillRect(597, 365 + charge, 16, 1, colorYellow);
		}
		if(ps->weaponstate != WEAPON_PREFIRING)
		{
			recharge = ps->stats[STAT_RAIL_READY] / 23;
			color1[0] = 0.9f;
			color1[1] = 0.8f;
			color1[2] = 0.1f;
			color1[3] = 0.7f;
			trap_R_SetColor(color1);
			CG_FillRect(573, 396, 12, -recharge, color1);
			trap_R_SetColor(NULL);
			color1[0] = 0.9f;
			color1[1] = 0.8f;
			color1[2] = 0.1f;
			color1[3] = 0.7f;
			trap_R_SetColor(color1);
			CG_FillRect(623, 396, 12, -recharge, color1);
			trap_R_SetColor(NULL);
		}
		//  CG_DrawPic( 570, 250,70 , 150, cgs.media.chargeMeterShader );

		if(ps->weaponstate != WEAPON_READY && ps->weaponstate != WEAPON_PREFIRING)
		{
			vec3_t          angles;
			vec3_t          origin;

			origin[0] = 90;
			origin[1] = 0;
			origin[2] = -4;
			angles[YAW] = 180;
			//  CG_Draw3DModel( 593,371, 24, 24,
			//   trap_R_RegisterModel( "models/zpm/scorerm.md3" ), trap_R_RegisterShader( "scorefxr01"), origin, angles );
		}
		else if(ps->weaponstate == WEAPON_READY)
		{
			origin[0] = 90;
			origin[1] = 0;
			origin[2] = -4;
			angles[YAW] = 180;
			//  CG_Draw3DModel( 593,371, 24,24,
			//  trap_R_RegisterModel( "models/zpm/scorebm.md3" ), trap_R_RegisterShader( "scorefxb01"), origin, angles );
		}
		else if(ps->weaponstate == WEAPON_PREFIRING)
		{
			origin[0] = 90;
			origin[1] = 0;
			origin[2] = -4;
			angles[YAW] = 180;
			//  CG_Draw3DModel( 593,371, 24,24,
			//  trap_R_RegisterModel( "models/zpm/scorebm.md3" ), trap_R_RegisterShader( "scorefxb01"), origin, angles );
		}

	}
	if(ps->weapon == WP_GRENADE_LAUNCHER)
	{
		charge = ps->stats[STAT_GREN_CHARGE] / 18.5f;
		if(charge < -111)
		{
			charge = -111;
		}
		if(charge > 0)
		{
			charge = -111;
		}
		color1[0] = 0.1f;
		color1[1] = 0.1f;
		color1[2] = 0.1f;
		color1[3] = 0.7f;
		trap_R_SetColor(color1);
		CG_FillRect(594, 369, 20, -116, color1);
		trap_R_SetColor(NULL);
		if(ps->weaponstate == WEAPON_PREFIRING)
		{

			color1[0] = 0.3f;
			color1[1] = 0.9f;
			color1[2] = 0.1f;
			color1[3] = 0.7f;
			trap_R_SetColor(color1);
			CG_FillRect(597, 365, 16, charge, color1);
			trap_R_SetColor(NULL);
			CG_FillRect(601, 365, 8, charge, colorBlue);
			CG_FillRect(597, 365 + charge, 16, 1, colorYellow);
		}
		CG_DrawPic(570, 250, 70, 150, cgs.media.chargeMetergShader);

	}
}
#endif

/*
===========================================================================================

  UPPER RIGHT CORNER

===========================================================================================
*/

/*
================
CG_DrawAttacker

================
*/
static float CG_DrawAttacker(float y)
{
	int             t;
	float           size;
	vec3_t          angles;
	const char     *info;
	const char     *name;
	int             clientNum;

	if(cg.predictedPlayerState.stats[STAT_HEALTH] <= 0)
	{
		return y;
	}

	if(!cg.attackerTime)
	{
		return y;
	}

	clientNum = cg.predictedPlayerState.persistant[PERS_ATTACKER];
	if(clientNum < 0 || clientNum >= MAX_CLIENTS || clientNum == cg.snap->ps.clientNum)
	{
		return y;
	}

	t = cg.time - cg.attackerTime;
	if(t > ATTACKER_HEAD_TIME)
	{
		cg.attackerTime = 0;
		return y;
	}

	size = ICON_SIZE * 1.25;

	angles[PITCH] = 0;
	angles[YAW] = 180;
	angles[ROLL] = 0;
	CG_DrawHead(640 - size, y, size, size, clientNum, angles);

	info = CG_ConfigString(CS_PLAYERS + clientNum);
	name = Info_ValueForKey(info, "n");
	y += size;
	CG_DrawBigString(640 - (Q_PrintStrlen(name) * BIGCHAR_WIDTH), y, name, 0.5);

	return y + BIGCHAR_HEIGHT + 2;
}

/*
==================
CG_DrawSnapshot
==================
*/
static float CG_DrawSnapshot(float y)
{
	char           *s;
	int             w;

	s = va("time:%i snap:%i cmd:%i", cg.snap->serverTime, cg.latestSnapshotNum, cgs.serverCommandSequence);
	w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;

	CG_DrawBigString(635 - w, y + 2, s, 1.0F);

	return y + BIGCHAR_HEIGHT + 4;
}

/*
==================
CG_DrawFPS
==================
*/
#define	FPS_FRAMES	4
static float CG_DrawFPS(float y)
{
	char           *s;
	int             w;
	static int      previousTimes[FPS_FRAMES];
	static int      index;
	int             i, total;
	int             fps;
	static int      previous;
	int             t, frameTime;

	// don't use serverTime, because that will be drifting to
	// correct for internet lag changes, timescales, timedemos, etc
	t = trap_Milliseconds();
	frameTime = t - previous;
	previous = t;

	previousTimes[index % FPS_FRAMES] = frameTime;
	index++;
	if(index > FPS_FRAMES)
	{
		// average multiple frames together to smooth changes out a bit
		total = 0;
		for(i = 0; i < FPS_FRAMES; i++)
		{
			total += previousTimes[i];
		}
		if(!total)
		{
			total = 1;
		}
		fps = 1000 * FPS_FRAMES / total;

		s = va("^a%ifps", fps);
		w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;

		CG_DrawBigString(635 - w, y + 2, s, 1.0F);
	}

	return y + BIGCHAR_HEIGHT + 4;
}

/*
=================
CG_DrawTimer
=================
*/
static float CG_DrawTimer(float y)
{
	char           *s;
	int             w;
	int             mins, seconds, tens;
	int             msec;

	msec = cg.time - cgs.levelStartTime;

	seconds = msec / 1000;
	mins = seconds / 60;
	seconds -= mins * 60;
	tens = seconds / 10;
	seconds -= tens * 10;

	s = va("%i:%i%i", mins, tens, seconds);
	w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;

	CG_DrawBigString(635 - w, y + 2, s, 1.0F);

	return y + BIGCHAR_HEIGHT + 4;
}


/*
=================
CG_DrawTeamOverlay
=================
*/

static float CG_DrawTeamOverlay(float y, qboolean right, qboolean upper)
{
	int             x, w, h, xx;
	int             i, j, len;
	const char     *p;
	vec4_t          hcolor;
	int             pwidth, lwidth;
	int             plyrs;
	char            st[16];
	clientInfo_t   *ci;
	gitem_t        *item;
	int             ret_y, count;

	if(!cg_drawTeamOverlay.integer)
	{
		return y;
	}

	if(cg.snap->ps.persistant[PERS_TEAM] != TEAM_RED && cg.snap->ps.persistant[PERS_TEAM] != TEAM_BLUE)
	{
		return y;				// Not on any team
	}

	plyrs = 0;

	// max player name width
	pwidth = 0;
	count = (numSortedTeamPlayers > 8) ? 8 : numSortedTeamPlayers;
	for(i = 0; i < count; i++)
	{
		ci = cgs.clientinfo + sortedTeamPlayers[i];
		if(ci->infoValid && ci->team == cg.snap->ps.persistant[PERS_TEAM])
		{
			plyrs++;
			len = CG_DrawStrlen(ci->name);
			if(len > pwidth)
				pwidth = len;
		}
	}

	if(!plyrs)
		return y;

	if(pwidth > TEAM_OVERLAY_MAXNAME_WIDTH)
		pwidth = TEAM_OVERLAY_MAXNAME_WIDTH;

	// max location name width
	lwidth = 0;
	for(i = 1; i < MAX_LOCATIONS; i++)
	{
		p = CG_ConfigString(CS_LOCATIONS + i);
		if(p && *p)
		{
			len = CG_DrawStrlen(p);
			if(len > lwidth)
				lwidth = len;
		}
	}

	if(lwidth > TEAM_OVERLAY_MAXLOCATION_WIDTH)
		lwidth = TEAM_OVERLAY_MAXLOCATION_WIDTH;

	w = (pwidth + lwidth + 4 + 7) * TINYCHAR_WIDTH;

	if(right)
		x = 640 - w;
	else
		x = 0;

	h = plyrs * TINYCHAR_HEIGHT;

	if(upper)
	{
		ret_y = y + h;
	}
	else
	{
		y -= h;
		ret_y = y;
	}

	if(cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED)
	{
		hcolor[0] = 1.0f;
		hcolor[1] = 0.0f;
		hcolor[2] = 0.0f;
		hcolor[3] = 0.33f;
	}
	else
	{							// if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE )
		hcolor[0] = 0.0f;
		hcolor[1] = 0.0f;
		hcolor[2] = 1.0f;
		hcolor[3] = 0.33f;
	}
	trap_R_SetColor(hcolor);
	CG_DrawPic(x, y, w, h, cgs.media.teamStatusBar);
	trap_R_SetColor(NULL);

	for(i = 0; i < count; i++)
	{
		ci = cgs.clientinfo + sortedTeamPlayers[i];
		if(ci->infoValid && ci->team == cg.snap->ps.persistant[PERS_TEAM])
		{

			hcolor[0] = hcolor[1] = hcolor[2] = hcolor[3] = 1.0;

			xx = x + TINYCHAR_WIDTH;

			CH_DrawStringExt(xx, y, ci->name, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, hcolor, TEAM_OVERLAY_MAXNAME_WIDTH, qfalse);


			if(lwidth)
			{
				p = CG_ConfigString(CS_LOCATIONS + ci->location);
				if(!p || !*p)
					p = "unknown";
				len = CG_DrawStrlen(p);
				if(len > lwidth)
					len = lwidth;

				xx = x + TINYCHAR_WIDTH * 2 + TINYCHAR_WIDTH * pwidth;

				CH_DrawStringExt(xx, y, p, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, hcolor, TEAM_OVERLAY_MAXLOCATION_WIDTH, qfalse);

			}

			CG_GetColorForHealth(ci->health, ci->armor, hcolor);

			Com_sprintf(st, sizeof(st), "%3i %3i", ci->health, ci->armor);

			xx = x + TINYCHAR_WIDTH * 3 + TINYCHAR_WIDTH * pwidth + TINYCHAR_WIDTH * lwidth;

			CH_DrawStringExt(xx, y, st, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, hcolor, 0, qfalse);


			// draw weapon icon
			xx += TINYCHAR_WIDTH * 3;

			if(cg_weapons[ci->curWeapon].weaponIcon)
			{
				CG_DrawPic(xx, y, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, cg_weapons[ci->curWeapon].weaponIcon);
			}
			else
			{
				CG_DrawPic(xx, y, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, cgs.media.deferShader);
			}

			// Draw powerup icons
			if(right)
			{
				xx = x;
			}
			else
			{
				xx = x + w - TINYCHAR_WIDTH;
			}
			for(j = 0; j <= PW_NUM_POWERUPS; j++)
			{
				if(ci->powerups & (1 << j))
				{

					item = BG_FindItemForPowerup(j);

					if(item)
					{
						CG_DrawPic(xx, y, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, trap_R_RegisterShader(item->icon));
						if(right)
						{
							xx -= TINYCHAR_WIDTH;
						}
						else
						{
							xx += TINYCHAR_WIDTH;
						}
					}
				}
			}

			y += TINYCHAR_HEIGHT;
		}
	}

	return ret_y;
}


/*
=====================
CG_DrawUpperRight

=====================
*/
static void CG_DrawUpperRight(void)
{
	float           y;

	y = 0;

	if(cgs.gametype >= GT_TEAM && cg_drawTeamOverlay.integer == 1)
	{
		y = CG_DrawTeamOverlay(y, qtrue, qtrue);
	}
	if(cg_drawSnapshot.integer)
	{
		y = CG_DrawSnapshot(y);
	}
	if(cg_drawFPS.integer)
	{
		y = CG_DrawFPS(y);
	}
	if(cg_drawTimer.integer)
	{
		y = CG_DrawTimer(y);
	}
	if(cg_drawAttacker.integer)
	{
		y = CG_DrawAttacker(y);
	}

}

/*
===========================================================================================

  LOWER RIGHT CORNER

===========================================================================================
*/

/*
=================
CG_DrawScores

Draw the small two score display
=================
*/
#ifndef MISSIONPACK
static float CG_DrawScores(float y)
{
	const char     *s;
	int             s1, s2, score;
	int             x, w;
	int             v;
	vec4_t          color;
	float           y1;
	gitem_t        *item;

	s1 = cgs.scores1;
	s2 = cgs.scores2;

	y -= SMALLSCORE_SIZE;

	y1 = y;

	// draw from the right side to left
	if(cgs.gametype >= GT_TEAM)
	{
		x = 640;
		color[0] = 0.0f;
		color[1] = 0.0f;
		color[2] = 1.0f;
		color[3] = 0.33f;
		s = va("%2i", s2);
		w = CG_DrawStrlen(s) * SMALLSCORE_SIZE;
		x -= w;
		CG_FillRect(x, y - 4, w, SMALLSCORE_SIZE, color);
		if(cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE)
		{
			CG_DrawPic(x, y - 4, w, SMALLSCORE_SIZE, cgs.media.selectShader);
		}
		if(w >= 2)
		{
			CH_DrawStringExt(x + 2, y - 3, s, SMALLSCORE_SIZE - 2, SMALLSCORE_SIZE - 2, colorWhite, 0, qfalse);
		}
		else
		{
			CH_DrawStringExt(x - 2, y - 3, s, SMALLSCORE_SIZE - 2, SMALLSCORE_SIZE - 2, colorWhite, 0, qfalse);
		}

		if(cgs.gametype == GT_CTF)
		{
			// Display flag status
			item = BG_FindItemForPowerup(PW_BLUEFLAG);

			if(item)
			{
				y1 = y - SMALLSCORE_SIZE;
				//NT - cgs.blueflag can be 3 - the flag is being returned
				if(cgs.blueflag >= 0 && cgs.blueflag <= 3)
				{
					CG_DrawPic(x, y1 - 4, w, SMALLSCORE_SIZE, cgs.media.blueFlagShader[cgs.blueflag]);
				}
			}
		}
		color[0] = 1.0f;
		color[1] = 0.0f;
		color[2] = 0.0f;
		color[3] = 0.33f;
		s = va("%2i", s1);
		w = CG_DrawStrlen(s) * SMALLSCORE_SIZE;
		x -= w;
		CG_FillRect(x, y - 4, w, SMALLSCORE_SIZE, color);
		if(cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED)
		{
			CG_DrawPic(x, y - 4, w, SMALLSCORE_SIZE, cgs.media.selectShader);
		}
		if(w >= 2)
		{
			CH_DrawStringExt(x + 2, y - 3, s, SMALLSCORE_SIZE - 2, SMALLSCORE_SIZE - 2, colorWhite, 0, qfalse);
		}
		else
		{
			CH_DrawStringExt(x - 2, y - 3, s, SMALLSCORE_SIZE - 2, SMALLSCORE_SIZE - 2, colorWhite, 0, qfalse);
		}

		if(cgs.gametype == GT_CTF)
		{
			// Display flag status
			item = BG_FindItemForPowerup(PW_REDFLAG);

			if(item)
			{
				y1 = y - SMALLSCORE_SIZE;
				//NT - cgs.redflag can be 3 - the flag is being returned
				if(cgs.redflag >= 0 && cgs.redflag <= 3)
				{
					CG_DrawPic(x, y1 - 4, w, SMALLSCORE_SIZE, cgs.media.redFlagShader[cgs.redflag]);
				}
			}
		}

		if(cgs.gametype >= GT_CTF)
		{
			v = cgs.capturelimit;
		}
		else
		{
			v = cgs.fraglimit;
		}
		if(v)
		{
			s = va("%2i", v);
			w = CG_DrawStrlen(s) * SMALLSCORE_SIZE;
			x -= w;
			if(w >= 2)
			{
				CH_DrawStringExt(x + 2, y - 3, s, SMALLSCORE_SIZE - 2, SMALLSCORE_SIZE - 2, colorWhite, 0, qfalse);
			}
			else
			{
				CH_DrawStringExt(x - 2, y - 3, s, SMALLSCORE_SIZE - 2, SMALLSCORE_SIZE - 2, colorWhite, 0, qfalse);
			}
		}

	}
	else
	{
		qboolean        spectator;

		x = 640;
		score = cg.snap->ps.persistant[PERS_SCORE];
		spectator = (cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR);

		// always show your score in the second box if not in first place
		if(s1 != score)
		{
			s2 = score;
		}
		if(s2 != SCORE_NOT_PRESENT)
		{
			s = va("%2i", s2);
			w = CG_DrawStrlen(s) * SMALLSCORE_SIZE;
			x -= w;
			if(!spectator && score == s2 && score != s1)
			{
				color[0] = 1.0f;
				color[1] = 0.0f;
				color[2] = 0.0f;
				color[3] = 0.33f;
				CG_FillRect(x, y - 4, w, SMALLSCORE_SIZE, color);
				CG_DrawPic(x, y - 4, w, SMALLSCORE_SIZE, cgs.media.selectShader);
			}
			else
			{
				color[0] = 0.5f;
				color[1] = 0.5f;
				color[2] = 0.5f;
				color[3] = 0.33f;
				CG_FillRect(x, y - 4, w, SMALLSCORE_SIZE, color);
			}
			if(w >= 2)
			{
				CH_DrawStringExt(x + 2, y - 3, s, SMALLSCORE_SIZE - 2, SMALLSCORE_SIZE - 2, colorWhite, 0, qfalse);
			}
			else
			{
				CH_DrawStringExt(x - 2, y - 3, s, SMALLSCORE_SIZE - 2, SMALLSCORE_SIZE - 2, colorWhite, 0, qfalse);
			}
		}

		// first place
		if(s1 != SCORE_NOT_PRESENT)
		{
			s = va("%2i", s1);
			w = CG_DrawStrlen(s) * SMALLSCORE_SIZE;
			x -= w;
			if(!spectator && score == s1)
			{
				color[0] = 0.0f;
				color[1] = 0.0f;
				color[2] = 1.0f;
				color[3] = 0.33f;
				CG_FillRect(x, y - 4, w, SMALLSCORE_SIZE, color);
				CG_DrawPic(x, y - 4, w, SMALLSCORE_SIZE, cgs.media.selectShader);
			}
			else
			{
				color[0] = 0.5f;
				color[1] = 0.5f;
				color[2] = 0.5f;
				color[3] = 0.33f;
				CG_FillRect(x, y - 4, w, SMALLSCORE_SIZE, color);
			}
			if(w >= 2)
			{
				CH_DrawStringExt(x + 2, y - 3, s, SMALLSCORE_SIZE - 2, SMALLSCORE_SIZE - 2, colorWhite, 0, qfalse);
			}
			else
			{
				CH_DrawStringExt(x - 2, y - 3, s, SMALLSCORE_SIZE - 2, SMALLSCORE_SIZE - 2, colorWhite, 0, qfalse);
			}
		}

		if(cgs.fraglimit)
		{
			s = va("%2i", cgs.fraglimit);
			w = CG_DrawStrlen(s) * SMALLSCORE_SIZE;
			x -= w;
			if(w >= 2)
			{
				CH_DrawStringExt(x + 2, y - 3, s, SMALLSCORE_SIZE - 2, SMALLSCORE_SIZE - 2, colorWhite, 0, qfalse);
			}
			else
			{
				CH_DrawStringExt(x - 2, y - 3, s, SMALLSCORE_SIZE - 2, SMALLSCORE_SIZE - 2, colorWhite, 0, qfalse);
			}
		}

	}

	return y1;
}
#endif							// MISSIONPACK

/*
================
CG_DrawPowerups
================
*/
#ifndef MISSIONPACK
static float CG_DrawPowerups(float y)
{
	int             sorted[MAX_POWERUPS];
	int             sortedTime[MAX_POWERUPS];
	int             i, j, k;
	int             active;
	playerState_t  *ps;
	int             t;
	gitem_t        *item;
	int             x;
	int             color;
	float           size;
	float           f;
	static float    colors[2][4] = {
		{0.2f, 1.0f, 0.2f, 1.0f},
		{1.0f, 0.2f, 0.2f, 1.0f}
	};

	ps = &cg.snap->ps;

	if(ps->stats[STAT_HEALTH] <= 0)
	{
		return y;
	}

	// sort the list by time remaining
	active = 0;
	for(i = 0; i < MAX_POWERUPS; i++)
	{
		if(!ps->powerups[i])
		{
			continue;
		}
		t = ps->powerups[i] - cg.time;
		// ZOID--don't draw if the power up has unlimited time (999 seconds)
		// This is true of the CTF flags
		if(t < 0 || t > 999000)
		{
			continue;
		}

		// insert into the list
		for(j = 0; j < active; j++)
		{
			if(sortedTime[j] >= t)
			{
				for(k = active - 1; k >= j; k--)
				{
					sorted[k + 1] = sorted[k];
					sortedTime[k + 1] = sortedTime[k];
				}
				break;
			}
		}
		sorted[j] = i;
		sortedTime[j] = t;
		active++;
	}

	// draw the icons and timers
	x = 640 - ICON_SIZE - CHAR_WIDTH * 2;
	for(i = 0; i < active; i++)
	{
		item = BG_FindItemForPowerup(sorted[i]);
		//  CG_RegisterItemVisuals( item->giTag );

		if(item)
		{

			color = 1;

			y -= ICON_SIZE;

			trap_R_SetColor(colors[color]);
			CG_DrawField(x, y, 2, sortedTime[i] / 1000, qtrue);

			t = ps->powerups[sorted[i]];
			if(t - cg.time >= POWERUP_BLINKS * POWERUP_BLINK_TIME)
			{
				trap_R_SetColor(NULL);
			}
			else
			{
				vec4_t          modulate;

				f = (float)(t - cg.time) / POWERUP_BLINK_TIME;
				f -= (int)f;
				modulate[0] = modulate[1] = modulate[2] = modulate[3] = f;
				trap_R_SetColor(modulate);
			}

			if(cg.powerupActive == sorted[i] && cg.time - cg.powerupTime < PULSE_TIME)
			{
				f = 1.0 - (((float)cg.time - cg.powerupTime) / PULSE_TIME);
				size = ICON_SIZE * (1.0 + (PULSE_SCALE - 1.0) * f);
			}
			else
			{
				size = ICON_SIZE;
			}
			//      CG_Draw3DModel( x, y, w, h, IT_POWERUP, 0, origin, angles );

			CG_DrawPic(640 - size, y + ICON_SIZE / 2 - size / 2, size, size, trap_R_RegisterShader(item->icon));
		}
	}
	trap_R_SetColor(NULL);

	return y;
}
#endif							// MISSIONPACK

/*
=====================
CG_DrawLowerRight

=====================
*/
#ifndef MISSIONPACK
static void CG_DrawLowerRight(void)
{
	float           y;

	y = 480 - ICON_SIZE;

	if(cgs.gametype >= GT_TEAM && cg_drawTeamOverlay.integer == 2)
	{
		y = CG_DrawTeamOverlay(y, qtrue, qfalse);
	}

	y = CG_DrawScores(y);
	y = CG_DrawPowerups(y);


}
#endif							// MISSIONPACK

/*
===================
CG_DrawPickupItem
===================
*/
#ifndef MISSIONPACK
static int CG_DrawPickupItem(int y)
{
	int             value;
	float          *fadeColor;

	if(cg.snap->ps.stats[STAT_HEALTH] <= 0)
	{
		return y;
	}

	y -= ICON_SIZE;

	value = cg.itemPickup;
	if(value)
	{
		fadeColor = CG_FadeColor(cg.itemPickupTime, 3000);
		if(fadeColor)
		{
			CG_RegisterItemVisuals(value);
			trap_R_SetColor(fadeColor);
			CG_DrawPic(8, y, ICON_SIZE, ICON_SIZE, cg_items[value].icon);
			CG_DrawBigString(ICON_SIZE + 16, y + (ICON_SIZE / 2 - BIGCHAR_HEIGHT / 2), bg_itemlist[value].pickup_name,
							 fadeColor[0]);
			trap_R_SetColor(NULL);
		}
	}

	return y;
}
#endif							// MISSIONPACK

/*
=====================
CG_DrawLowerLeft

=====================
*/
#ifndef MISSIONPACK
static void CG_DrawLowerLeft(void)
{
	float           y;

	y = 480 - ICON_SIZE;

	if(cgs.gametype >= GT_TEAM && cg_drawTeamOverlay.integer == 3)
	{
		y = CG_DrawTeamOverlay(y, qfalse, qfalse);
	}
//  CG_CalcVrect2();


	y = CG_DrawPickupItem(y);
}
#endif							// MISSIONPACK


//===========================================================================================

/*
=================
CG_DrawTeamInfo
=================
*/
#ifndef MISSIONPACK
static void CG_DrawTeamInfo(void)
{
	int             w, h, y;
	int             i, len;
	vec4_t          hcolor;
	int             chatHeight;

#define CHATLOC_Y 410			// bottom end
#define CHATLOC_X 0
	y = 0;


	if(cg.predictedPlayerState.pm_type == PM_INTERMISSION)
	{
		y += 32;
	}
	else
	{
		y += 410;
	}

//  if (cg_teamChatHeight.integer < TEAMCHAT_HEIGHT)
//      chatHeight = cg_teamChatHeight.integer;
//  else
	chatHeight = 6;
//  if (chatHeight <= 0)
//      return; // disabled

	if(cgs.teamLastChatPos != cgs.teamChatPos)
	{
		if(cg.time - cgs.teamChatMsgTimes[cgs.teamLastChatPos % chatHeight] > 3)
		{
			cgs.teamLastChatPos++;
		}

		h = (cgs.teamChatPos - cgs.teamLastChatPos) * 8;

		w = 0;

		for(i = cgs.teamLastChatPos; i < cgs.teamChatPos; i++)
		{
			len = CG_DrawStrlen(cgs.teamChatMsgs[i % chatHeight]);
			if(len > w)
				w = len;
		}
		w *= TINYCHAR_WIDTH;
		w += TINYCHAR_WIDTH * 2;

		if(cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED)
		{
			hcolor[0] = 1.0f;
			hcolor[1] = 0.0f;
			hcolor[2] = 0.0f;
			hcolor[3] = 0.33f;
		}
		else if(cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE)
		{
			hcolor[0] = 0.0f;
			hcolor[1] = 0.0f;
			hcolor[2] = 1.0f;
			hcolor[3] = 0.33f;
		}
		else
		{
			hcolor[0] = 0.0f;
			hcolor[1] = 1.0f;
			hcolor[2] = 0.0f;
			hcolor[3] = 0.33f;
		}

		trap_R_SetColor(hcolor);
		//  CG_DrawPic( CHATLOC_X, CHATLOC_Y - h, 640, h, cgs.media.teamStatusBar );
		trap_R_SetColor(NULL);

		hcolor[0] = hcolor[1] = hcolor[2] = 1.0f;
		hcolor[3] = 1.0f;

		for(i = cgs.teamChatPos - 1; i >= cgs.teamLastChatPos; i--)
		{
			CH_DrawStringExt(CHATLOC_X + 8, y - (cgs.teamChatPos - i) * 8,
							 cgs.teamChatMsgs[i % chatHeight], 8, 8, hcolor, 0, qfalse);

		}
	}
}
#endif							// MISSIONPACK

/*
===================
CG_DrawHoldableItem
===================

#ifndef MISSIONPACK
static void CG_DrawHoldableItem( void ) { 
	int		value;

	value = cg.snap->ps.stats[STAT_HOLDABLE_ITEM];
	if ( value ) {
		CG_RegisterItemVisuals( value );
		CG_DrawPic( 640-ICON_SIZE, (SCREEN_HEIGHT-ICON_SIZE)/2, ICON_SIZE, ICON_SIZE, cg_items[ value ].icon );
	}

}
#endif // MISSIONPACK
*/
#ifdef MISSIONPACK
/*
===================
CG_DrawPersistantPowerup
===================
*/
#if 0							// sos001208 - DEAD
static void CG_DrawPersistantPowerup(void)
{
	int             value;

	value = cg.snap->ps.stats[STAT_PERSISTANT_POWERUP];
	if(value)
	{
		CG_RegisterItemVisuals(value);
		CG_DrawPic(640 - ICON_SIZE, (SCREEN_HEIGHT - ICON_SIZE) / 2 - ICON_SIZE, ICON_SIZE, ICON_SIZE, cg_items[value].icon);
	}
}
#endif
#endif							// MISSIONPACK


/*
===================
CG_DrawReward
===================
*/
static void CG_DrawReward(void)
{
	float          *color;
	int             i, count;
	float           x, y;
	char            buf[32];
	vec3_t          angles;
	vec3_t          origin;


	VectorClear(angles);
	origin[0] = 90;
	origin[1] = 0;
	origin[2] = -10;
	angles[YAW] = (cg.time & 2047) * 360 / 2048.0;

	if(!cg_drawRewards.integer)
	{
		return;
	}

	color = CG_FadeColor(cg.rewardTime, REWARD_TIME);
	if(!color)
	{
		if(cg.rewardStack > 0)
		{
			for(i = 0; i < cg.rewardStack; i++)
			{
				cg.rewardSound[i] = cg.rewardSound[i + 1];
				cg.rewardShader[i] = cg.rewardShader[i + 1];
				cg.rewardCount[i] = cg.rewardCount[i + 1];
				cg.rewardModel[i] = cg.rewardModel[i + 1];
			}
			cg.rewardTime = cg.time;
			cg.rewardStack--;
			color = CG_FadeColor(cg.rewardTime, REWARD_TIME);
			trap_S_StartLocalSound(cg.rewardSound[0], CHAN_ANNOUNCER);
		}
		else
		{
			return;
		}
	}

	trap_R_SetColor(color);

	/*
	   count = cg.rewardCount[0]/10;                // number of big rewards to draw

	   if (count) {
	   y = 4;
	   x = 320 - count * ICON_SIZE;
	   for ( i = 0 ; i < count ; i++ ) {
	   CG_DrawPic( x, y, (ICON_SIZE*2)-4, (ICON_SIZE*2)-4, cg.rewardShader[0] );
	   x += (ICON_SIZE*2);
	   }
	   }

	   count = cg.rewardCount[0] - count*10;        // number of small rewards to draw
	 */

	if(cg.rewardCount[0] >= 10)
	{
		y = 56;
		x = 320 - ICON_SIZE / 2;
		//  CG_DrawPic( x, y, ICON_SIZE-4, ICON_SIZE-4, cg.rewardShader[0] );

		CG_Draw3DModel(x, y, ICON_SIZE, ICON_SIZE, cg.rewardModel[0], 0, origin, angles);

		Com_sprintf(buf, sizeof(buf), "%d", cg.rewardCount[0]);
		x = (SCREEN_WIDTH - SMALLCHAR_WIDTH * CG_DrawStrlen(buf)) / 2;
		CH_DrawStringExt(x, y + ICON_SIZE, buf, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, color, 0, qfalse);

	}
	else
	{

		count = cg.rewardCount[0];

		y = 56;
		x = 320 - count * ICON_SIZE / 2;
		for(i = 0; i < count; i++)
		{
			//      CG_DrawPic( x, y, ICON_SIZE-4, ICON_SIZE-4, cg.rewardShader[0] );
			CG_Draw3DModel(x, y, ICON_SIZE, ICON_SIZE, cg.rewardModel[0], 0, origin, angles);
			x += ICON_SIZE;
		}
	}
	trap_R_SetColor(NULL);
}

/*
==============
CG_DrawBloom
==============
*/
static void CG_DrawBloom(void)
{
	if(cg_drawBloom.integer == 1)
	{
		CG_DrawPic(0, 0, 640, 480, cgs.media.bloomShader);
	}
	else if(cg_drawBloom.integer == 2)
	{
		CG_DrawPic(0, 0, 640, 480, cgs.media.bloom2Shader);
	}
}

/*
==============
CG_DrawRotoscope
==============
*/
static void CG_DrawRotoscope(void)
{
	if(cg_drawRotoscope.integer == 1)
	{
		CG_DrawPic(0, 0, 640, 480, cgs.media.rotoscopeShader);
	}
}


/*
===============================================================================

LAGOMETER

===============================================================================
*/

#define	LAG_SAMPLES		128


typedef struct
{
	int             frameSamples[LAG_SAMPLES];
	int             frameCount;
	int             snapshotFlags[LAG_SAMPLES];
	int             snapshotSamples[LAG_SAMPLES];
	int             snapshotCount;
} lagometer_t;

lagometer_t     lagometer;

/*
==============
CG_AddLagometerFrameInfo

Adds the current interpolate / extrapolate bar for this frame
==============
*/
void CG_AddLagometerFrameInfo(void)
{
	int             offset;

	offset = cg.time - cg.latestSnapshotTime;
	lagometer.frameSamples[lagometer.frameCount & (LAG_SAMPLES - 1)] = offset;
	lagometer.frameCount++;
}

/*
==============
CG_AddLagometerSnapshotInfo

Each time a snapshot is received, log its ping time and
the number of snapshots that were dropped before it.

Pass NULL for a dropped packet.
==============
*/
void CG_AddLagometerSnapshotInfo(snapshot_t * snap)
{
	// dropped packet
	if(!snap)
	{
		lagometer.snapshotSamples[lagometer.snapshotCount & (LAG_SAMPLES - 1)] = -1;
		lagometer.snapshotCount++;
		return;
	}

	// add this snapshot's info
	lagometer.snapshotSamples[lagometer.snapshotCount & (LAG_SAMPLES - 1)] = snap->ping;
	lagometer.snapshotFlags[lagometer.snapshotCount & (LAG_SAMPLES - 1)] = snap->snapFlags;
	lagometer.snapshotCount++;
}

/*
==============
CG_DrawDisconnect

Should we draw something differnet for long lag vs no packets?
==============
*/
static void CG_DrawDisconnect(void)
{
	float           x, y;
	int             cmdNum;
	usercmd_t       cmd;
	const char     *s;
	int             w;			// bk010215 - FIXME char message[1024];

	// draw the phone jack if we are completely past our buffers
	cmdNum = trap_GetCurrentCmdNumber() - CMD_BACKUP + 1;
	trap_GetUserCmd(cmdNum, &cmd);
	if(cmd.serverTime <= cg.snap->ps.commandTime || cmd.serverTime > cg.time)
	{							// special check for map_restart // bk 0102165 - FIXME
		return;
	}

	// also add text in center of screen
	s = "Connection Interrupted";	// bk 010215 - FIXME
	w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;
	CG_DrawBigString(320 - w / 2, 100, s, 1.0F);

	// blink the icon
	if((cg.time >> 9) & 1)
	{
		return;
	}

	x = 640 - 48;
	y = 480 - 48;

	CG_DrawPic(x, y, 48, 48, trap_R_RegisterShader("gfx/2d/net.tga"));
}


#define	MAX_LAGOMETER_PING	900
#define	MAX_LAGOMETER_RANGE	300

/*
==============
CG_DrawLagometer
==============
*/
static void CG_DrawLagometer(void)
{
	int             a, x, y, i;
	float           v;
	float           ax, ay, aw, ah, mid, range;
	int             color;
	float           vscale;

	if(!cg_lagometer.integer || cgs.localServer)
	{
		CG_DrawDisconnect();
		return;
	}

	//
	// draw the graph
	//
#ifdef MISSIONPACK
	x = 640 - 48;
	y = 480 - 144;
#else
	x = 640 - 48;
	y = 480 - 48;
#endif

	trap_R_SetColor(NULL);
	CG_DrawPic(x, y, 48, 48, cgs.media.lagometerShader);

	ax = x;
	ay = y;
	aw = 48;
	ah = 48;
	CG_AdjustFrom640(&ax, &ay, &aw, &ah);

	color = -1;
	range = ah / 3;
	mid = ay + range;

	vscale = range / MAX_LAGOMETER_RANGE;

	// draw the frame interpoalte / extrapolate graph
	for(a = 0; a < aw; a++)
	{
		i = (lagometer.frameCount - 1 - a) & (LAG_SAMPLES - 1);
		v = lagometer.frameSamples[i];
		v *= vscale;
		if(v > 0)
		{
			if(color != 1)
			{
				color = 1;
				trap_R_SetColor(g_color_table[ColorIndex(COLOR_YELLOW)]);
			}
			if(v > range)
			{
				v = range;
			}
			trap_R_DrawStretchPic(ax + aw - a, mid - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader);
		}
		else if(v < 0)
		{
			if(color != 2)
			{
				color = 2;
				trap_R_SetColor(g_color_table[ColorIndex(COLOR_BLUE)]);
			}
			v = -v;
			if(v > range)
			{
				v = range;
			}
			trap_R_DrawStretchPic(ax + aw - a, mid, 1, v, 0, 0, 0, 0, cgs.media.whiteShader);
		}
	}

	// draw the snapshot latency / drop graph
	range = ah / 2;
	vscale = range / MAX_LAGOMETER_PING;

	for(a = 0; a < aw; a++)
	{
		i = (lagometer.snapshotCount - 1 - a) & (LAG_SAMPLES - 1);
		v = lagometer.snapshotSamples[i];
		if(v > 0)
		{
			if(lagometer.snapshotFlags[i] & SNAPFLAG_RATE_DELAYED)
			{
				if(color != 5)
				{
					color = 5;	// YELLOW for rate delay
					trap_R_SetColor(g_color_table[ColorIndex(COLOR_YELLOW)]);
				}
			}
			else
			{
				if(color != 3)
				{
					color = 3;
					trap_R_SetColor(g_color_table[ColorIndex(COLOR_GREEN)]);
				}
			}
			v = v * vscale;
			if(v > range)
			{
				v = range;
			}
			trap_R_DrawStretchPic(ax + aw - a, ay + ah - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader);
		}
		else if(v < 0)
		{
			if(color != 4)
			{
				color = 4;		// RED for dropped snapshots
				trap_R_SetColor(g_color_table[ColorIndex(COLOR_RED)]);
			}
			trap_R_DrawStretchPic(ax + aw - a, ay + ah - range, 1, range, 0, 0, 0, 0, cgs.media.whiteShader);
		}
	}

	trap_R_SetColor(NULL);

	if(cg_nopredict.integer || cg_synchronousClients.integer)
	{
		CG_DrawBigString(ax, ay, "snc", 1.0);
	}

	CG_DrawDisconnect();
}



/*
===============================================================================

CENTER PRINTING

===============================================================================
*/


/*
==============
CG_CenterPrint

Called for important messages that should stay in the center of the screen
for a few moments
==============
*/
void CG_CenterPrint(const char *str, int y, int charWidth)
{
	char           *s;

	Q_strncpyz(cg.centerPrint, str, sizeof(cg.centerPrint));

	cg.centerPrintTime = cg.time;
	cg.centerPrintY = y;
	cg.centerPrintCharWidth = charWidth;

	// count the number of lines for centering
	cg.centerPrintLines = 1;
	s = cg.centerPrint;
	while(*s)
	{
		if(*s == '\n')
			cg.centerPrintLines++;
		s++;
	}
}


/*
===================
CG_DrawCenterString
===================
*/
static void CG_DrawCenterString(void)
{
	char           *start;
	int             l;
	int             x, y, w;

#ifdef MISSIONPACK				// bk010221 - unused else
	int             h;
#endif
	float          *color;

	if(!cg.centerPrintTime)
	{
		return;
	}

	color = CG_FadeColor(cg.centerPrintTime, 1000 * cg_centertime.value);
	if(!color)
	{
		return;
	}

	trap_R_SetColor(color);

	start = cg.centerPrint;

	y = cg.centerPrintY - cg.centerPrintLines * BIGCHAR_HEIGHT / 2;

	while(1)
	{
		char            linebuffer[1024];

		for(l = 0; l < 50; l++)
		{
			if(!start[l] || start[l] == '\n')
			{
				break;
			}
			linebuffer[l] = start[l];
		}
		linebuffer[l] = 0;

#ifdef MISSIONPACK
		w = CG_Text_Width(linebuffer, 0.5, 0);
		h = CG_Text_Height(linebuffer, 0.5, 0);
		x = (SCREEN_WIDTH - w) / 2;
		CG_Text_Paint(x, y + h, 0.5, color, linebuffer, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE);
		y += h + 6;
#else
		w = cg.centerPrintCharWidth * CG_DrawStrlen(linebuffer);

		x = (SCREEN_WIDTH - w) / 2;

//      CG_DrawStringExt( x, y, linebuffer, color, qfalse, qtrue,
//          cg.centerPrintCharWidth, (int)(cg.centerPrintCharWidth * 1.5), 0 );
		CH_DrawStringExt(x, y, linebuffer, cg.centerPrintCharWidth, (int)(cg.centerPrintCharWidth * 1.5), colorWhite, 0, qfalse);


		y += cg.centerPrintCharWidth * 1.5;
#endif
		while(*start && (*start != '\n'))
		{
			start++;
		}
		if(!*start)
		{
			break;
		}
		start++;
	}

	trap_R_SetColor(NULL);
}



/*
================================================================================

CROSSHAIR

================================================================================
*/


/*
=================
CG_DrawCrosshair
=================
*/
void CG_DrawCrosshair(void)
{
	float           w, h;
	qhandle_t       hShader;
	float           f;
	float           x, y;
	int             ca;

	if(cg_drawCrosshair.integer < 0)
	{
		return;
	}

	if(cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR)
	{
		return;
	}

	if(cg.renderingThirdPerson)
	{
		return;
	}

	// set color based on health
	if(cg_crosshairHealth.integer)
	{
		vec4_t          hcolor;

		CG_ColorForHealth(hcolor);
		trap_R_SetColor(hcolor);
	}
	else
	{
		trap_R_SetColor(NULL);
	}

	w = h = cg_crosshairSize.value;

	// pulse the size of the crosshair when picking up items
	f = cg.time - cg.itemPickupBlendTime;
	if(f > 0 && f < ITEM_BLOB_TIME)
	{
		f /= ITEM_BLOB_TIME;
		w *= (1 + f);
		h *= (1 + f);
	}

	x = cg_crosshairX.integer;
	y = cg_crosshairY.integer;
	CG_AdjustFrom640(&x, &y, &w, &h);

	ca = cg_drawCrosshair.integer;
	if(ca < 0)
	{
		ca = 0;
	}
	hShader = cgs.media.crosshairShader[ca % NUM_CROSSHAIRS];

	trap_R_DrawStretchPic(x + cg.refdef[0].x + 0.5 * (cg.refdef[0].width - w),
						  y + cg.refdef[0].y + 0.5 * (cg.refdef[0].height - h), w, h, 0, 0, 1, 1, hShader);
}



/*
=================
CG_ScanForCrosshairEntity
=================
*/
static void CG_ScanForCrosshairEntity(void)
{
	trace_t         trace;
	vec3_t          start, end;
	int             content;

	VectorCopy(cg.refdef[0].vieworg, start);
	VectorMA(start, 131072, cg.refdef[0].viewaxis[0], end);

	CG_Trace(&trace, start, vec3_origin, vec3_origin, end, cg.snap->ps.clientNum, CONTENTS_SOLID | CONTENTS_BODY);
	if(trace.entityNum >= MAX_CLIENTS)
	{
		return;
	}

	// if the player is in fog, don't show it
	content = trap_CM_PointContents(trace.endpos, 0);
	if(content & CONTENTS_FOG)
	{
		return;
	}

	// if the player is invisible, don't show it
	if(cg_entities[trace.entityNum].currentState.powerups & (1 << PW_INVIS))
	{
		return;
	}

	// update the fade timer
	cg.crosshairClientNum = trace.entityNum;
	cg.crosshairClientTime = cg.time;
}


/*
=====================
CG_DrawCrosshairNames
=====================
*/
static void CG_DrawCrosshairNames(void)
{
	float          *color;
	char           *name;
	float           w;

	if(!cg_drawCrosshair.integer)
	{
		return;
	}
	if(!cg_drawCrosshairNames.integer)
	{
		return;
	}
	if(cg.renderingThirdPerson)
	{
		return;
	}

	// scan the known entities to see if the crosshair is sighted on one
	CG_ScanForCrosshairEntity();

	// draw the name of the player being looked at
	color = CG_FadeColor(cg.crosshairClientTime, 1000);
	if(!color)
	{
		trap_R_SetColor(NULL);
		return;
	}

	name = cgs.clientinfo[cg.crosshairClientNum].name;
#ifdef MISSIONPACK
	color[3] *= 0.5f;
	w = CG_Text_Width(name, 0.3f, 0);
	CG_Text_Paint(320 - w / 2, 190, 0.3f, color, name, 0, 0, ITEM_TEXTSTYLE_SHADOWED);
#else
	w = CG_DrawStrlen(name) * BIGCHAR_WIDTH;
	CG_DrawBigString(320 - w / 2, 170, name, color[3] * 0.5f);
#endif
	trap_R_SetColor(NULL);
}


//==============================================================================

/*
=================
CG_DrawSpectator
=================
*/
static void CG_DrawSpectator(void)
{
//  CG_DrawBigString(320 - 9 * 8, 440, "SPECTATOR", 1.0F);
	CH_DrawStringExt(320 - 9 * 8, 440, "^xff9900^0^BSPECTATOR^N", 16, 24, colorWhite, 0, qfalse);

//  if ( cgs.gametype == GT_TOURNAMENT ) {
//      CG_DrawBigString(320 - 15 * 8, 460, "waiting to play", 1.0F);
//  }
	if(cgs.gametype >= GT_TEAM)
	{
		//  CG_DrawBigString(320 - 39 * 8, 460, "press ESC and use the JOIN menu to play", 1.0F);
// CH_DrawStringExt( 320 - 39 * 8, 450, "^x000000^3^Bpress ESC and use the JOIN menu to play", 16, 24, colorWhite, 0, qfalse);

	}
}

/*
=================
CG_DrawVote
=================
*/
static void CG_DrawVote(void)
{
	char           *s;
	int             sec;

	if(!cgs.voteTime)
	{
		return;
	}

	// play a talk beep whenever it is modified
	if(cgs.voteModified)
	{
		cgs.voteModified = qfalse;
		trap_S_StartLocalSound(cgs.media.votecallSound, CHAN_LOCAL_SOUND);
	}

	sec = (VOTE_TIME - (cg.time - cgs.voteTime)) / 1000;
	if(sec < 0)
	{
		sec = 0;
	}
#ifdef MISSIONPACK
	s = va("^3VOTE(%i):%s yes:%i no:%i", sec, cgs.voteString, cgs.voteYes, cgs.voteNo);
	CG_DrawSmallString(0, 58, s, 1.0F);
	s = "or press ESC then click Vote";
	CG_DrawSmallString(0, 58 + BIGCHAR_HEIGHT + 2, s, 1.0F);
#else
	s = va("^3VOTE(%i):%s yes:%i no:%i", sec, cgs.voteString, cgs.voteYes, cgs.voteNo);
	CG_DrawSmallString(0, 58, s, 1.0F);
#endif
}

/*
=================
CG_DrawTeamVote
=================
*/
static void CG_DrawTeamVote(void)
{
	char           *s;
	int             sec, cs_offset;

	if(cgs.clientinfo->team == TEAM_RED)
		cs_offset = 0;
	else if(cgs.clientinfo->team == TEAM_BLUE)
		cs_offset = 1;
	else
		return;

	if(!cgs.teamVoteTime[cs_offset])
	{
		return;
	}

	// play a talk beep whenever it is modified
	if(cgs.teamVoteModified[cs_offset])
	{
		cgs.teamVoteModified[cs_offset] = qfalse;
		trap_S_StartLocalSound(cgs.media.talkSound, CHAN_LOCAL_SOUND);
	}

	sec = (VOTE_TIME - (cg.time - cgs.teamVoteTime[cs_offset])) / 1000;
	if(sec < 0)
	{
		sec = 0;
	}
	s = va("TEAMVOTE(%i):%s yes:%i no:%i", sec, cgs.teamVoteString[cs_offset],
		   cgs.teamVoteYes[cs_offset], cgs.teamVoteNo[cs_offset]);
	CG_DrawSmallString(0, 90, s, 1.0F);
}


static qboolean CG_DrawScoreboard()
{

	if(cgs.gametype < GT_TEAM)
	{
		return CG_DrawOldScoreboard();
	}
	return CG_DrawZPMScoreboard();



}

/*
=================
CG_DrawIntermission
=================
*/
static void CG_DrawIntermission(void)
{

	if(cgs.gametype == GT_SINGLE_PLAYER)
	{
		CG_DrawCenterString();
		return;
	}

	cg.scoreFadeTime = cg.time;
	cg.scoreBoardShowing = CG_DrawScoreboard();


}

/*
=================
CG_DrawFollow
=================
*/
static qboolean CG_DrawFollow(void)
{
	float           x;
	vec4_t          color;
	const char     *name;

	if(!(cg.snap->ps.pm_flags & PMF_FOLLOW))
	{
		return qfalse;
	}
	color[0] = 1;
	color[1] = 1;
	color[2] = 1;
	color[3] = 1;


	CG_DrawBigString(320 - 9 * 8, 24, "^1following", 1.0F);

	name = cgs.clientinfo[cg.snap->ps.clientNum].name;

	x = 0.5 * (640 - GIANT_WIDTH * CG_DrawStrlen(name));

	CH_DrawStringExt(x, 40, name, GIANT_WIDTH, GIANT_HEIGHT, color, 0, qfalse);

	return qtrue;
}



/*
=================
CG_DrawAmmoWarning
=================
*/
static void CG_DrawAmmoWarning(void)
{
	const char     *s;
	int             w;

	if(cg_drawAmmoWarning.integer == 0)
	{
		return;
	}

	if(!cg.lowAmmoWarning)
	{
		return;
	}

	if(cg.lowAmmoWarning == 2)
	{
		s = "^1OUT OF AMMO";
	}
	else
	{
		s = "^3LOW AMMO WARNING";
	}
	w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;
	CG_DrawBigString(320 - w / 2, 64, s, 1.0F);
}


#ifdef MISSIONPACK
/*
=================
CG_DrawProxWarning
=================
*/
static void CG_DrawProxWarning(void)
{
	char            s[32];
	int             w;
	static int      proxTime;
	static int      proxCounter;
	static int      proxTick;

	if(!(cg.snap->ps.eFlags & EF_TICKING))
	{
		proxTime = 0;
		return;
	}

	if(proxTime == 0)
	{
		proxTime = cg.time + 5000;
		proxCounter = 5;
		proxTick = 0;
	}

	if(cg.time > proxTime)
	{
		proxTick = proxCounter--;
		proxTime = cg.time + 1000;
	}

	if(proxTick != 0)
	{
		Com_sprintf(s, sizeof(s), "INTERNAL COMBUSTION IN: %i", proxTick);
	}
	else
	{
		Com_sprintf(s, sizeof(s), "YOU HAVE BEEN MINED");
	}

	w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;
	CG_DrawBigStringColor(320 - w / 2, 64 + BIGCHAR_HEIGHT, s, g_color_table[ColorIndex(COLOR_RED)]);
}
#endif


/*
=================
CG_DrawWarmup
=================
*/
static void CG_DrawWarmup(void)
{
	int             w, w2, w3;
	int             sec, sec2, sec3;
	int             i;
	float           scale, w4, y, x, h;
	clientInfo_t   *ci1, *ci2;
	int             cw, cw2, cw3;
	const char     *s, *s1;
	const char     *s2, *s3;

//  const char  *sc,*sc2;
//  char    *name,*name2;

	sec = cg.warmup;
	sec2 = cg.warmup;
	sec3 = cg.warmup;
	if(!sec || !sec2 || !sec3)
	{
		return;
	}

	if(sec < 0)
	{
		s = "^A^xffff00^2Waiting for players";
		w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;
		CG_DrawBigString(320 - w / 2, 24, s, 1.0F);
		cg.warmupCount = 0;
		return;
	}

	if(cgs.gametype == GT_TOURNAMENT)
	{
		// find the two active players
		ci1 = NULL;
		ci2 = NULL;
		for(i = 0; i < cgs.maxclients; i++)
		{
			if(cgs.clientinfo[i].infoValid && cgs.clientinfo[i].team == TEAM_FREE)
			{
				if(!ci1)
				{
					ci1 = &cgs.clientinfo[i];
				}
				else
				{
					ci2 = &cgs.clientinfo[i];
				}
			}
		}

		if(ci1 && ci2)
		{
			if(cgs.InstaGib == 1)
			{
				if(cgs.InstaWeapon == 0)
				{
					s = va("^AInstagib Rail Tournament");
					w = CG_DrawStrlen(s);
					if(w > 640 / 12)
					{
						cw = 640 / w;
					}
					else
					{
						cw = 12;
					}
					CH_DrawStringExt(320 - w * cw / 2, 6, s, 12, 12, colorWhite, 0, qfalse);
				}
				else if(cgs.InstaWeapon == 1)
				{
					s = va("^AInstagib Rocket Tournament");
					w = CG_DrawStrlen(s);
					if(w > 640 / 12)
					{
						cw = 640 / w;
					}
					else
					{
						cw = 12;
					}
					CH_DrawStringExt(320 - w * cw / 2, 6, s, 12, 12, colorWhite, 0, qfalse);
				}
				else if(cgs.InstaWeapon == 2)
				{
					s = va("^AInstagib Rocket & Rail Tournament");
					w = CG_DrawStrlen(s);
					if(w > 640 / 12)
					{
						cw = 640 / w;
					}
					else
					{
						cw = 12;
					}
					CH_DrawStringExt(320 - w * cw / 2, 6, s, 12, 12, colorWhite, 0, qfalse);
				}
			}
			else
			{
				s = va("^ATournament");
				w = CG_DrawStrlen(s);
				if(w > 640 / 12)
				{
					cw = 640 / w;
				}
				else
				{
					cw = 12;
				}
				CH_DrawStringExt(320 - w * cw / 2, 6, s, 12, 12, colorWhite, 0, qfalse);
			}

			s = va("%s", ci1->name);
			w = CG_DrawStrlen(s);
			if(w > 640 / 16)
			{
				cw = 640 / w;
			}
			else
			{
				cw = 16;
			}
			CH_DrawStringExt(320 - w * cw / 2, 20, s, 16, 16, colorWhite, 0, qfalse);

			s1 = va("^A^7vs");
			w2 = CG_DrawStrlen(s1);
			if(w2 > 640 / 16)
			{
				cw2 = 640 / w2;
			}
			else
			{
				cw2 = 16;
			}
			CH_DrawStringExt(320 - w2 * cw2 / 2, 38, s1, 16, 16, colorWhite, 0, qfalse);

			s3 = va("%s", ci2->name);
			w3 = CG_DrawStrlen(s3);
			if(w3 > 640 / 16)
			{
				cw3 = 640 / w3;
			}
			else
			{
				cw3 = 16;
			}
			CH_DrawStringExt(320 - w3 * cw3 / 2, 56, s3, 16, 16, colorWhite, 0, qfalse);

		}
	}
	else
	{
		if(cgs.gametype == GT_FFA)
		{
			s = "Free For All";
		}
		else if(cgs.gametype == GT_TEAM)
		{
			s = "Team Deathmatch";
		}
		else if(cgs.gametype == GT_CTF)
		{
			s = "Capture the Flag";
		}
		else
		{
			s = "";
		}
		w = CG_DrawStrlen(s);
		if(w > 640 / GIANT_WIDTH)
		{
			cw = 640 / w;
		}
		else
		{
			cw = GIANT_WIDTH;
		}
		CG_DrawStringExt(320 - w * cw / 2, 25, s, colorWhite, qfalse, qtrue, cw, (int)(cw * 1.1f), 0);
	}

	sec = (sec - cg.time) / 1000;
	sec2 = (sec2 - cg.time) * 10 / 1000;
	sec3 = (sec3 - cg.time) * 10 / 100;
	if(sec < 0)
	{
		cg.warmup = 0;
		sec = 0;
	}
	if(sec2 < 0)
	{
		sec2 = 0;
	}

	if(sec3 < 0)
	{
		sec3 = 0;
	}
	if(sec != cg.warmupCount)
	{
		cg.warmupCount = sec;
		switch (sec)
		{
			case 0:
				trap_S_StartLocalSound(cgs.media.wearOffSound, CHAN_ANNOUNCER);
				trap_S_StartLocalSound(cgs.media.wearOffSound, CHAN_ANNOUNCER);
				break;
			case 1:
				trap_S_StartLocalSound(cgs.media.wearOffSound, CHAN_ANNOUNCER);
				trap_S_StartLocalSound(cgs.media.wearOffSound, CHAN_ANNOUNCER);
				break;
			case 2:
				trap_S_StartLocalSound(cgs.media.wearOffSound, CHAN_ANNOUNCER);
				trap_S_StartLocalSound(cgs.media.wearOffSound, CHAN_ANNOUNCER);
				break;
			case 3:
				trap_S_StartLocalSound(cgs.media.wearOffSound, CHAN_ANNOUNCER);
				trap_S_StartLocalSound(cgs.media.wearOffSound, CHAN_ANNOUNCER);
				break;
			default:
				break;
		}
	}

	scale = 0.45f;
	switch (cg.warmupCount)
	{
		case 0:
			cw = 16;
			scale = 0.54f;
			break;
		case 1:
			cw = 16;
			scale = 0.51f;
			break;
		case 2:
			cw = 16;
			scale = 0.48f;
			break;
		default:
			cw = 16;
			scale = 0.45f;
			break;
	}
	x = 320;
	h = (int)(cw * 1.5);
	y = 72;
	w4 = cw;
//  CG_AdjustFrom640( &x, &y, &w4, &h );
	s2 = va("%i", sec3);
	if(cg.warmupCount <= 9999)
	{
		if(CG_DrawStrlen(s2) < 6 && CG_DrawStrlen(s2) > 4)
		{
			s = va("0%i", sec3);
			w = CG_DrawStrlen(s) * 5;
			CG_DrawStringExtTimer(260 /*- w * cw/2 -cw*/ , 72, s, colorGreen,
								  qfalse, qtrue, cw, h, 0);
		}
		else if(CG_DrawStrlen(s2) < 5 && CG_DrawStrlen(s2) > 3)
		{
			s = va("00%i", sec3);
			w = CG_DrawStrlen(s) * 5;
			CG_DrawStringExtTimer(260 /*- w * cw/2 -cw*/ , 72, s, colorGreen,
								  qfalse, qtrue, cw, h, 0);
		}
		else if(CG_DrawStrlen(s2) < 4 && CG_DrawStrlen(s2) > 2)
		{
			s = va("000%i", sec3);
			w = CG_DrawStrlen(s) * 5;
			CG_DrawStringExtTimer(260 /*- w * cw/2 -cw*/ , 72, s, colorRed,
								  qfalse, qtrue, cw, h, 0);
		}
		else if(CG_DrawStrlen(s2) < 3 && CG_DrawStrlen(s2) > 1)
		{
			s = va("0000%i", sec3);
			w = CG_DrawStrlen(s) * 5;
			CG_DrawStringExtTimer(260 /*- w * cw/2 -cw*/ , 72, s, colorRed,
								  qfalse, qtrue, cw, h, 0);
		}
		else if(CG_DrawStrlen(s2) < 2 && CG_DrawStrlen(s2) > 0)
		{
			s = va("00000%i", sec3);
			w = CG_DrawStrlen(s) * 5;
			CG_DrawStringExtTimer(260 /*- w * cw/2 -cw*/ , 72, s, colorRed,
								  qfalse, qtrue, cw, h, 0);
		}
		else
		{
			s = va("%i", sec3);
			w = CG_DrawStrlen(s) * 5;
			CG_DrawStringExtTimer(260 /*- w * cw/2 -cw*/ , 72, s, colorGreen,
								  qfalse, qtrue, cw, h, 0);
		}
	}

}

//==================================================================================
#ifdef MISSIONPACK
/* 
=================
CG_DrawTimedMenus
=================
*/
void CG_DrawTimedMenus()
{
	if(cg.voiceTime)
	{
		int             t = cg.time - cg.voiceTime;

		if(t > 2500)
		{
			Menus_CloseByName("voiceMenu");
			trap_Cvar_Set("cl_conXOffset", "0");
			cg.voiceTime = 0;
		}
	}
}
#endif

/*
=================
CG_DrawMOTD
=================
*/
void CG_Drawtest()
{
	vec4_t          hcolor;
	vec_t           alpha;
	int             x;

//  vec_t       alpha2;

	x = 10;


	if(cg.logoTime1 >= 6000)
	{
		x = (cg.logoTime1 - 6000) * -300 / 1000 + 10;
	}
	else if(cg.logoTime1 <= 3000)
	{
		x = (cg.logoTime1 - 3000) * 300 / 1000 + 10;
	}

	alpha = 0.3f;
	hcolor[0] = 0.0f;
	hcolor[1] = 0.0f;
	hcolor[2] = 0.0f;
	hcolor[3] = 1.0f;
	trap_R_SetColor(NULL);
	CG_DrawRect(x + 1, 322 + 1, 559, 70, 3, hcolor);
	trap_R_SetColor(NULL);

	hcolor[0] = 0.9f;
	hcolor[1] = 0.0f;
	hcolor[2] = 0.0f;
	hcolor[3] = 1.0f;
	trap_R_SetColor(NULL);
	CG_DrawRect(x + 1, 322 + 1, 559, 70, 1, hcolor);
	trap_R_SetColor(NULL);

	hcolor[0] = 0.1f;
	hcolor[1] = 0.1f;
	hcolor[2] = 0.1f;
	hcolor[3] = alpha;
	trap_R_SetColor(NULL);
	CG_FillRect(x + 2, 322 + 2, 559 - 2, 70 - 2, hcolor);
	trap_R_SetColor(NULL);



	CH_DrawStringExt(x + 75, 326, "^b^0**** ^1Pro-ZPM Version ^41.0b ^0****", 6, 10, colorWhite, 85, qfalse);

	CH_DrawStringExt(x + 15, 338, "", 6, 10, colorWhite, 85, qfalse);


	// motd #1
	CH_DrawStringExt(x + 15, 350, cgs.motd1, 6, 10, colorWhite, 85, qfalse);

	// motd #2
	CH_DrawStringExt(x + 15, 362, cgs.motd2, 6, 10, colorWhite, 85, qfalse);

	// motd #3
	CH_DrawStringExt(x + 15, 374, cgs.motd3, 6, 10, colorWhite, 85, qfalse);

}



/*
=================
CG_DrawSTATS
=================
*/
void CG_DrawStats(void)
{
	vec4_t          hcolor;
	vec_t           alpha;
	int             x;
	int             accuracycmg, accuracycrl, accuracycrg, accuracycgl, accuracycsg;
	int             accuracycpg, accuracycbfg, accuracyclg;
	const char     *hmg, *smg, *amg;
	const char     *hrl, *srl, *arl;
	const char     *hrg, *srg, *arg;
	const char     *hgl, *sgl, *agl;
	const char     *hsg, *ssg, *asg;
	const char     *hpg, *spg, *apg;
	const char     *hbfg, *sbfg, *abfg;
	const char     *hlg, *slg, *alg;
	const char     *name;
	clientInfo_t   *ci;
	playerState_t  *ps;
	clientInfo_t   *c2;



	ps = &cg.snap->ps;

	ci = &cgs.clientinfo[ps->clientNum];

	c2 = &cgs.clientinfo[ci->statnum];

	x = 10;


	if(cg.statTime >= 7000)
	{
		x = (cg.statTime - 7000) * -500 / 1000 + 10;
	}
	else if(cg.statTime <= 2000)
	{
		x = (cg.statTime - 2000) * 500 / 1000 + 10;
	}

	alpha = 0.3f;
	hcolor[0] = 0.0f;
	hcolor[1] = 0.0f;
	hcolor[2] = 0.0f;
	hcolor[3] = 1.0f;
	trap_R_SetColor(NULL);
	CG_DrawRect(x + 1, 322 + 1, 319, 90, 3, hcolor);
	trap_R_SetColor(NULL);

	hcolor[0] = 0.9f;
	hcolor[1] = 0.0f;
	hcolor[2] = 0.0f;
	hcolor[3] = 1.0f;
	trap_R_SetColor(NULL);
	CG_DrawRect(x + 1, 322 + 1, 319, 90, 1, hcolor);
	trap_R_SetColor(NULL);

	hcolor[0] = 0.1f;
	hcolor[1] = 0.1f;
	hcolor[2] = 0.1f;
	hcolor[3] = alpha;
	trap_R_SetColor(NULL);
	CG_FillRect(x + 2, 322 + 2, 319 - 2, 90 - 2, hcolor);
	trap_R_SetColor(NULL);


	CH_DrawStringExt(x + 10, 326, "^3^FCurrent Statistics^f^1Current Statistics^N ^1For", 6, 7, colorWhite, 85, qfalse);
	name = va("%s ", c2->name);
	CH_DrawStringExt(x + 150, 326, name, 6, 7, colorWhite, 85, qfalse);

	hcolor[0] = 0.9f;
	hcolor[1] = 0.9f;
	hcolor[2] = 0.9f;
	hcolor[3] = alpha;
	trap_R_SetColor(NULL);
	CG_FillRect(x + 9, 339, 300, 9, hcolor);
	trap_R_SetColor(NULL);

	CH_DrawStringExt(x + 10, 339, "^x55ff33^4Weapon", 6, 7, colorWhite, 85, qfalse);
	CH_DrawStringExt(x + 150, 339, "^x55ff33^4Hits^1/^4Shots", 6, 7, colorWhite, 85, qfalse);

	CH_DrawStringExt(x + 250, 339, "^x55ff33^4Accuracy", 6, 7, colorWhite, 85, qfalse);

	hcolor[0] = 0.9f;
	hcolor[1] = 0.0f;
	hcolor[2] = 0.0f;
	hcolor[3] = 1.0f;
	trap_R_SetColor(NULL);
	CG_DrawRect(x + 9, 348, 300, 1, 1, hcolor);
	trap_R_SetColor(NULL);

	//Machinegun
	CH_DrawStringExt(x + 10, 351, "^3MachineGun^N", 6, 7, colorWhite, 85, qfalse);
	hmg = va("%i ", c2->accuracy_hitmg);
	CH_DrawStringExt(x + 150, 351, hmg, 6, 7, colorWhite, 85, qfalse);
	CH_DrawStringExt(x + 174, 349, "^1/", 6, 10, colorWhite, 85, qfalse);
	smg = va("%i ", c2->accuracy_shotmg);
	CH_DrawStringExt(x + 181, 351, smg, 6, 7, colorWhite, 85, qfalse);
	if(c2->accuracy_shotmg > 0)
	{
		accuracycmg = c2->accuracy_hitmg * 100 / c2->accuracy_shotmg;
	}
	else
	{
		accuracycmg = 0;
	}
	amg = va("^x000000^B^3%i ", accuracycmg);
	CH_DrawStringExt(x + 250, 351, amg, 6, 7, colorWhite, 85, qfalse);

	// Rocket Launcher
	CH_DrawStringExt(x + 10, 358, "^3Rocket Launcher^N", 6, 7, colorWhite, 85, qfalse);
	hrl = va("%i", c2->accuracy_hitrl);
	CH_DrawStringExt(x + 150, 358, hrl, 6, 7, colorWhite, 85, qfalse);
	CH_DrawStringExt(x + 174, 356, "^1/", 6, 10, colorWhite, 85, qfalse);
	srl = va("%i", c2->accuracy_shotrl);
	CH_DrawStringExt(x + 181, 358, srl, 6, 7, colorWhite, 85, qfalse);
	if(c2->accuracy_shotrl > 0)
	{
		accuracycrl = c2->accuracy_hitrl * 100 / c2->accuracy_shotrl;
	}
	else
	{
		accuracycrl = 0;
	}
	arl = va("^x000000^B^3%i ", accuracycrl);
	CH_DrawStringExt(x + 250, 358, arl, 6, 7, colorWhite, 85, qfalse);

	// RailGun
	CH_DrawStringExt(x + 10, 365, "^3RailGun^N", 6, 7, colorWhite, 85, qfalse);
	hrg = va("%i", c2->accuracy_hitrg);
	CH_DrawStringExt(x + 150, 365, hrg, 6, 7, colorWhite, 85, qfalse);
	CH_DrawStringExt(x + 174, 363, "^1/", 6, 10, colorWhite, 85, qfalse);
	srg = va("%i", c2->accuracy_shotrg);
	CH_DrawStringExt(x + 181, 365, srg, 6, 7, colorWhite, 85, qfalse);
	if(c2->accuracy_shotrg > 0)
	{
		accuracycrg = c2->accuracy_hitrg * 100 / c2->accuracy_shotrg;
	}
	else
	{
		accuracycrg = 0;
	}
	arg = va("^x000000^B^3%i ", accuracycrg);
	CH_DrawStringExt(x + 250, 365, arg, 6, 7, colorWhite, 85, qfalse);

	// ShotGun
	CH_DrawStringExt(x + 10, 372, "^3ShotGun^N", 6, 7, colorWhite, 85, qfalse);
	hsg = va("%i", c2->accuracy_hitsg);
	CH_DrawStringExt(x + 150, 372, hsg, 6, 7, colorWhite, 85, qfalse);
	CH_DrawStringExt(x + 174, 370, "^1/", 6, 10, colorWhite, 85, qfalse);
	ssg = va("%i", c2->accuracy_shotsg);
	CH_DrawStringExt(x + 181, 372, ssg, 6, 7, colorWhite, 85, qfalse);
	if(c2->accuracy_shotsg > 0)
	{
		accuracycsg = c2->accuracy_hitsg * 100 / c2->accuracy_shotsg;
	}
	else
	{
		accuracycsg = 0;
	}
	asg = va("^x000000^B^3%i ", accuracycsg);
	CH_DrawStringExt(x + 250, 372, asg, 6, 7, colorWhite, 85, qfalse);

	// PlasmaGun
	CH_DrawStringExt(x + 10, 379, "^3PlasmaGun^N", 6, 7, colorWhite, 85, qfalse);
	hpg = va("%i", c2->accuracy_hitpg);
	CH_DrawStringExt(x + 150, 379, hpg, 6, 7, colorWhite, 85, qfalse);
	CH_DrawStringExt(x + 174, 377, "^1/", 6, 10, colorWhite, 85, qfalse);
	spg = va("%i", c2->accuracy_shotpg);
	CH_DrawStringExt(x + 181, 379, spg, 6, 7, colorWhite, 85, qfalse);
	if(c2->accuracy_shotpg > 0)
	{
		accuracycpg = c2->accuracy_hitpg * 100 / c2->accuracy_shotpg;
	}
	else
	{
		accuracycpg = 0;
	}
	apg = va("^x000000^B^3%i ", accuracycpg);
	CH_DrawStringExt(x + 250, 379, apg, 6, 7, colorWhite, 85, qfalse);

	// Grenade Launcher
	CH_DrawStringExt(x + 10, 386, "^3Grenade Launcher^N", 6, 7, colorWhite, 85, qfalse);
	hgl = va("%i", c2->accuracy_hitgl);
	CH_DrawStringExt(x + 150, 386, hgl, 6, 7, colorWhite, 85, qfalse);
	CH_DrawStringExt(x + 174, 384, "^1/", 6, 10, colorWhite, 85, qfalse);
	sgl = va("%i", c2->accuracy_shotgl);
	CH_DrawStringExt(x + 181, 386, sgl, 6, 7, colorWhite, 85, qfalse);
	if(cgs.clientinfo->accuracy_shotgl > 0)
	{
		accuracycgl = c2->accuracy_hitgl * 100 / c2->accuracy_shotgl;
	}
	else
	{
		accuracycgl = 0;
	}
	agl = va("^x000000^B^3%i ", accuracycgl);
	CH_DrawStringExt(x + 250, 386, agl, 6, 7, colorWhite, 85, qfalse);

	// BFG
	CH_DrawStringExt(x + 10, 393, "^3BFG^N", 6, 7, colorWhite, 85, qfalse);
	hbfg = va("%i", c2->accuracy_hitbfg);
	CH_DrawStringExt(x + 150, 393, hbfg, 6, 7, colorWhite, 85, qfalse);
	CH_DrawStringExt(x + 174, 391, "^1/", 6, 10, colorWhite, 85, qfalse);
	sbfg = va("%i", c2->accuracy_shotbfg);
	CH_DrawStringExt(x + 181, 393, sbfg, 6, 7, colorWhite, 85, qfalse);
	if(c2->accuracy_shotbfg > 0)
	{
		accuracycbfg = c2->accuracy_hitbfg * 100 / c2->accuracy_shotbfg;
	}
	else
	{
		accuracycbfg = 0;
	}
	abfg = va("^x000000^B^3%i ", accuracycbfg);
	CH_DrawStringExt(x + 250, 393, abfg, 6, 7, colorWhite, 85, qfalse);

	// LightningGun
	CH_DrawStringExt(x + 10, 400, "^3LightningGun^N", 6, 7, colorWhite, 85, qfalse);
	hlg = va("%i", c2->accuracy_hitlg);
	CH_DrawStringExt(x + 150, 400, hlg, 6, 7, colorWhite, 85, qfalse);
	CH_DrawStringExt(x + 174, 398, "^1/", 6, 10, colorWhite, 85, qfalse);
	slg = va("%i", c2->accuracy_shotlg);
	CH_DrawStringExt(x + 181, 400, slg, 6, 7, colorWhite, 85, qfalse);
	if(c2->accuracy_shotlg > 0)
	{
		accuracyclg = c2->accuracy_hitlg * 100 / c2->accuracy_shotlg;
	}
	else
	{
		accuracyclg = 0;
	}
	alg = va("^x000000^B^3%i ", accuracyclg);
	CH_DrawStringExt(x + 250, 400, alg, 6, 7, colorWhite, 85, qfalse);

}

/*
=================
CG_DrawSTATS
=================
*/
void CG_DrawStatsFinal(void)
{
	vec4_t          hcolor;
	vec_t           alpha;
	int             x;
	int             accuracycmg, accuracycrl, accuracycrg, accuracycgl, accuracycsg;
	int             accuracycpg, accuracycbfg, accuracyclg;
	const char     *hmg, *smg, *amg;
	const char     *hrl, *srl, *arl;
	const char     *hrg, *srg, *arg;
	const char     *hgl, *sgl, *agl;
	const char     *hsg, *ssg, *asg;
	const char     *hpg, *spg, *apg;
	const char     *hbfg, *sbfg, *abfg;
	const char     *hlg, *slg, *alg;
	const char     *pmega, *ph5;
	const char     *pmg, *ph20, *ph40;
	const char     *prl, *prarmor, *pyarmor;
	const char     *prg, *pshard, *pquad;
	const char     *pgl, *pinvis, *pregen;
	const char     *psg, *psuit, *pfly;
	const char     *ppg, *prflag, *pbflag;
	const char     *pbfg, *plg, *phaste;
	const char     *pht, *pat, *dgt, *drt;
	const char     *name;
	clientInfo_t   *ci;
	playerState_t  *ps;
	clientInfo_t   *c2;




	ps = &cg.snap->ps;

	ci = &cgs.clientinfo[ps->clientNum];

	c2 = &cgs.clientinfo[ci->statnum];

	x = 20;


	if(cg.statTime >= 6000)
	{
		x = (cg.statTime - 6000) * -300 / 2000 + 10;
	}
	CG_DrawPic(x + 565, 325 + 50, 35, 85, trap_R_RegisterShaderNoMip("gfx/stats/q3zpmlogo01"));

	alpha = 0.3f;
	hcolor[0] = 0.0f;
	hcolor[1] = 0.0f;
	hcolor[2] = 0.0f;
	hcolor[3] = 1.0f;
	trap_R_SetColor(NULL);
	CG_DrawRect(x + 1, 322 + 1 + 50, 600, 90, 3, hcolor);
	trap_R_SetColor(NULL);

	hcolor[0] = 0.9f;
	hcolor[1] = 0.0f;
	hcolor[2] = 0.0f;
	hcolor[3] = 1.0f;
	trap_R_SetColor(NULL);
	CG_DrawRect(x + 1, 322 + 1 + 50, 600, 90, 1, hcolor);
	trap_R_SetColor(NULL);


	hcolor[0] = 0.1f;
	hcolor[1] = 0.1f;
	hcolor[2] = 0.1f;
	hcolor[3] = alpha + 0.5;
	trap_R_SetColor(NULL);
	CG_FillRect(x + 2, 322 + 2 + 50, 565, 90 - 2, hcolor);
	trap_R_SetColor(NULL);

	hcolor[0] = 0.9f;
	hcolor[1] = 0.0f;
	hcolor[2] = 0.0f;
	hcolor[3] = 1.0f;
	trap_R_SetColor(NULL);
	CG_DrawRect(x + 310, 389 + 50, 97.5f, 1, 1, hcolor);
	trap_R_SetColor(NULL);

	hcolor[0] = 0.9f;
	hcolor[1] = 0.0f;
	hcolor[2] = 0.0f;
	hcolor[3] = 1.0f;
	trap_R_SetColor(NULL);
	CG_DrawRect(x + 467.5f, 389 + 50, 96.5f, 1, 1, hcolor);
	trap_R_SetColor(NULL);

	hcolor[0] = 0.9f;
	hcolor[1] = 0.0f;
	hcolor[2] = 0.0f;
	hcolor[3] = 1.0f;
	trap_R_SetColor(NULL);
	CG_DrawRect(x + 310, 375 + 50.5f, 97.5f, 1, 1, hcolor);
	trap_R_SetColor(NULL);

	hcolor[0] = 0.9f;
	hcolor[1] = 0.0f;
	hcolor[2] = 0.0f;
	hcolor[3] = 1.0f;
	trap_R_SetColor(NULL);
	CG_DrawRect(x + 467.5f, 375.5f + 50, 96.5f, 1, 1, hcolor);
	trap_R_SetColor(NULL);

	CH_DrawStringExt(x + 10, 326 + 50, "^3^FMatch Statistics^f^1Match Statistics^N ^1For", 6, 7, colorWhite, 85, qfalse);
	name = va("%s ", c2->name);
	CH_DrawStringExt(x + 150, 326 + 50, name, 6, 7, colorWhite, 85, qfalse);

	hcolor[0] = 0.9f;
	hcolor[1] = 0.9f;
	hcolor[2] = 0.9f;
	hcolor[3] = alpha;
	trap_R_SetColor(NULL);
	CG_FillRect(x + 3, 339 + 50, 560, 9, hcolor);
	trap_R_SetColor(NULL);

	CH_DrawStringExt(x + 10, 339 + 50, "^x55ff33^4Weapon", 6, 7, colorWhite, 85, qfalse);
	CH_DrawStringExt(x + 125, 339 + 50, "^x55ff33^4Hits^1/^4Shots", 6, 7, colorWhite, 85, qfalse);

	CH_DrawStringExt(x + 260, 339 + 50, "^x55ff33^4Accuracy", 6, 7, colorWhite, 85, qfalse);

	CH_DrawStringExt(x + 205, 339 + 50, "^x55ff33^4Pickups", 6, 7, colorWhite, 85, qfalse);

	hcolor[0] = 0.9f;
	hcolor[1] = 0.0f;
	hcolor[2] = 0.0f;
	hcolor[3] = 1.0f;
	trap_R_SetColor(NULL);
	CG_DrawRect(x + 2, 349 + 50, 562, 1, 1, hcolor);
	trap_R_SetColor(NULL);

	hcolor[0] = 0.9f;
	hcolor[1] = 0.0f;
	hcolor[2] = 0.0f;
	hcolor[3] = 1.0f;
	trap_R_SetColor(NULL);
	CG_DrawRect(x + 2, 337 + 50, 562, 1, 1, hcolor);
	trap_R_SetColor(NULL);

	hcolor[0] = 0.9f;
	hcolor[1] = 0.0f;
	hcolor[2] = 0.0f;
	hcolor[3] = 1.0f;
	trap_R_SetColor(NULL);
	CG_DrawRect(x + 309, 339 + 50, 1, 69, 1, hcolor);
	trap_R_SetColor(NULL);

	hcolor[0] = 0.0f;
	hcolor[1] = 0.0f;
	hcolor[2] = 0.6f;
	hcolor[3] = alpha + 0.3;
	trap_R_SetColor(NULL);
	CG_FillRect(x + 10, 358 + 50, 298, 7, hcolor);
	trap_R_SetColor(NULL);

	hcolor[0] = 0.0f;
	hcolor[1] = 0.0f;
	hcolor[2] = 0.6f;
	hcolor[3] = alpha + 0.3;
	trap_R_SetColor(NULL);
	CG_FillRect(x + 10, 372 + 50, 298, 7, hcolor);
	trap_R_SetColor(NULL);

	hcolor[0] = 0.0f;
	hcolor[1] = 0.0f;
	hcolor[2] = 0.6f;
	hcolor[3] = alpha + 0.3;
	trap_R_SetColor(NULL);
	CG_FillRect(x + 10, 386 + 50, 298, 7, hcolor);
	trap_R_SetColor(NULL);

	hcolor[0] = 0.0f;
	hcolor[1] = 0.0f;
	hcolor[2] = 0.6f;
	hcolor[3] = alpha + 0.3;
	trap_R_SetColor(NULL);
	CG_FillRect(x + 10, 400 + 50, 298, 7, hcolor);
	trap_R_SetColor(NULL);

	CH_DrawStringExtTiny(x + 422.5f, 374.5f + 50, "^F^1POWERUPS^f^xff0011^4POWERUPS", 4, 4, colorWhite, 85, qfalse);

	CH_DrawStringExtTiny(x + 425.5f, 387 + 50, "^F^4DAMAGE^f^xff0011^3DAMAGE^N", 4, 4, colorWhite, 85, qfalse);

	hcolor[0] = 0.0f;
	hcolor[1] = 0.0f;
	hcolor[2] = 0.0f;
	hcolor[3] = alpha + 0.4;
	trap_R_SetColor(NULL);
	CG_FillRect(x + 311, 379 + 50, 181, 7, hcolor);
	trap_R_SetColor(NULL);

	hcolor[0] = 0.0f;
	hcolor[1] = 0.0f;
	hcolor[2] = 0.0f;
	hcolor[3] = alpha + 0.4;
	trap_R_SetColor(NULL);
	CG_FillRect(x + 311, 351 + 50, 253, 7, hcolor);
	trap_R_SetColor(NULL);

	hcolor[0] = 0.0f;
	hcolor[1] = 0.0f;
	hcolor[2] = 0.6f;
	hcolor[3] = alpha + 0.3;
	trap_R_SetColor(NULL);
	CG_FillRect(x + 311, 358 + 50, 253, 14, hcolor);
	trap_R_SetColor(NULL);

	hcolor[0] = 0.0f;
	hcolor[1] = 0.0f;
	hcolor[2] = 0.6f;
	hcolor[3] = alpha + 0.3;
	trap_R_SetColor(NULL);
	CG_FillRect(x + 311, 393 + 50, 253, 14, hcolor);
	trap_R_SetColor(NULL);

	hcolor[0] = 0.9f;
	hcolor[1] = 0.0f;
	hcolor[2] = 0.0f;
	hcolor[3] = 1.0f;
	trap_R_SetColor(NULL);
	CG_FillRect(x + 564, 323 + 50, 1, 90, hcolor);
	trap_R_SetColor(NULL);

	hcolor[0] = 0.5f;
	hcolor[1] = 0.0f;
	hcolor[2] = 0.0f;
	hcolor[3] = alpha + 0.3;
	trap_R_SetColor(NULL);
	CG_FillRect(x + 402, 379 + 50, 7, 7, hcolor);
	trap_R_SetColor(NULL);

	hcolor[0] = 1.0f;
	hcolor[1] = 0.0f;
	hcolor[2] = 0.5f;
	hcolor[3] = 1.0f;
	trap_R_SetColor(NULL);
	CG_DrawRect(x + 490, 376.5f + 50, 1, 12.5f, 1, hcolor);
	trap_R_SetColor(NULL);


	hcolor[0] = 0.5f;
	hcolor[1] = 0.0f;
	hcolor[2] = 0.0f;
	hcolor[3] = alpha + 0.3;
	trap_R_SetColor(NULL);
	CG_FillRect(x + 491, 376.5f + 50, 36, 12.5f, hcolor);
	trap_R_SetColor(NULL);

	hcolor[0] = 0.0f;
	hcolor[1] = 0.0f;
	hcolor[2] = 0.5f;
	hcolor[3] = alpha + 0.3;
	trap_R_SetColor(NULL);
	CG_FillRect(x + 527, 376.5f + 50, 37, 12.5f, hcolor);
	trap_R_SetColor(NULL);

	//Machinegun
	CH_DrawStringExt(x + 10, 351 + 50, "^3MachineGun^N", 6, 7, colorWhite, 85, qfalse);
	hmg = va("%i ", c2->accuracy_hitmg);
	CH_DrawStringExt(x + 125, 351 + 50, hmg, 6, 7, colorWhite, 85, qfalse);
	CH_DrawStringExt(x + 149, 349 + 50, "^1/", 6, 10, colorWhite, 85, qfalse);
	smg = va("%i ", c2->accuracy_shotmg);
	CH_DrawStringExt(x + 155, 351 + 50, smg, 6, 7, colorWhite, 85, qfalse);
	if(c2->accuracy_shotmg > 0)
	{
		accuracycmg = c2->accuracy_hitmg * 100 / c2->accuracy_shotmg;
	}
	else
	{
		accuracycmg = 0;
	}
	amg = va("^x000000^B^3%i ", accuracycmg);
	CH_DrawStringExt(x + 280, 351 + 50, amg, 6, 7, colorWhite, 85, qfalse);
	pmg = va("%i ", c2->pickupmg);
	CH_DrawStringExt(x + 224, 351 + 50, pmg, 6, 7, colorWhite, 85, qfalse);

	// Rocket Launcher
	CH_DrawStringExt(x + 10, 358 + 50, "^3Rocket Launcher^N", 6, 7, colorWhite, 85, qfalse);
	hrl = va("%i", c2->accuracy_hitrl);
	CH_DrawStringExt(x + 125, 358 + 50, hrl, 6, 7, colorWhite, 85, qfalse);
	CH_DrawStringExt(x + 149, 356 + 50, "^1/", 6, 10, colorWhite, 85, qfalse);
	srl = va("%i", c2->accuracy_shotrl);
	CH_DrawStringExt(x + 155, 358 + 50, srl, 6, 7, colorWhite, 85, qfalse);
	if(c2->accuracy_shotrl > 0)
	{
		accuracycrl = c2->accuracy_hitrl * 100 / c2->accuracy_shotrl;
	}
	else
	{
		accuracycrl = 0;
	}
	arl = va("^x000000^B^3%i ", accuracycrl);
	CH_DrawStringExt(x + 280, 358 + 50, arl, 6, 7, colorWhite, 85, qfalse);
	prl = va("%i ", c2->pickuprl);
	CH_DrawStringExt(x + 224, 358 + 50, prl, 6, 7, colorWhite, 85, qfalse);

	// RailGun
	CH_DrawStringExt(x + 10, 365 + 50, "^3RailGun^N", 6, 7, colorWhite, 85, qfalse);
	hrg = va("%i", c2->accuracy_hitrg);
	CH_DrawStringExt(x + 125, 365 + 50, hrg, 6, 7, colorWhite, 85, qfalse);
	CH_DrawStringExt(x + 149, 363 + 50, "^1/", 6, 10, colorWhite, 85, qfalse);
	srg = va("%i", c2->accuracy_shotrg);
	CH_DrawStringExt(x + 155, 365 + 50, srg, 6, 7, colorWhite, 85, qfalse);
	if(c2->accuracy_shotrg > 0)
	{
		accuracycrg = c2->accuracy_hitrg * 100 / c2->accuracy_shotrg;
	}
	else
	{
		accuracycrg = 0;
	}
	arg = va("^x000000^B^3%i ", accuracycrg);
	CH_DrawStringExt(x + 280, 365 + 50, arg, 6, 7, colorWhite, 85, qfalse);
	prg = va("%i ", c2->pickuprg);
	CH_DrawStringExt(x + 224, 365 + 50, prg, 6, 7, colorWhite, 85, qfalse);

	// ShotGun
	CH_DrawStringExt(x + 10, 372 + 50, "^3ShotGun^B(Per Pellet)", 6, 7, colorWhite, 85, qfalse);
	hsg = va("%i", c2->accuracy_hitsg);
	CH_DrawStringExt(x + 125, 372 + 50, hsg, 6, 7, colorWhite, 85, qfalse);
	CH_DrawStringExt(x + 149, 370 + 50, "^1/", 6, 10, colorWhite, 85, qfalse);
	ssg = va("%i", c2->accuracy_shotsg);
	CH_DrawStringExt(x + 155, 372 + 50, ssg, 6, 7, colorWhite, 85, qfalse);
	if(c2->accuracy_shotsg > 0)
	{
		accuracycsg = c2->accuracy_hitsg * 100 / c2->accuracy_shotsg;
	}
	else
	{
		accuracycsg = 0;
	}
	asg = va("^x000000^B^3%i ", accuracycsg);
	CH_DrawStringExt(x + 280, 372 + 50, asg, 6, 7, colorWhite, 85, qfalse);
	psg = va("%i ", c2->pickupsg);
	CH_DrawStringExt(x + 224, 372 + 50, psg, 6, 7, colorWhite, 85, qfalse);

	// PlasmaGun
	CH_DrawStringExt(x + 10, 379 + 50, "^3PlasmaGun^N", 6, 7, colorWhite, 85, qfalse);
	hpg = va("%i", c2->accuracy_hitpg);
	CH_DrawStringExt(x + 125, 379 + 50, hpg, 6, 7, colorWhite, 85, qfalse);
	CH_DrawStringExt(x + 149, 377 + 50, "^1/", 6, 10, colorWhite, 85, qfalse);
	spg = va("%i", c2->accuracy_shotpg);
	CH_DrawStringExt(x + 155, 379 + 50, spg, 6, 7, colorWhite, 85, qfalse);
	if(c2->accuracy_shotpg > 0)
	{
		accuracycpg = c2->accuracy_hitpg * 100 / c2->accuracy_shotpg;
	}
	else
	{
		accuracycpg = 0;
	}
	apg = va("^x000000^B^3%i ", accuracycpg);
	CH_DrawStringExt(x + 280, 379 + 50, apg, 6, 7, colorWhite, 85, qfalse);
	ppg = va("%i ", c2->pickuppg);
	CH_DrawStringExt(x + 224, 379 + 50, ppg, 6, 7, colorWhite, 85, qfalse);

	// Grenade Launcher
	CH_DrawStringExt(x + 10, 386 + 50, "^3Grenade Launcher^N", 6, 7, colorWhite, 85, qfalse);
	hgl = va("%i", c2->accuracy_hitgl);
	CH_DrawStringExt(x + 125, 386 + 50, hgl, 6, 7, colorWhite, 85, qfalse);
	CH_DrawStringExt(x + 149, 384 + 50, "^1/", 6, 10, colorWhite, 85, qfalse);
	sgl = va("%i", c2->accuracy_shotgl);
	CH_DrawStringExt(x + 155, 386 + 50, sgl, 6, 7, colorWhite, 85, qfalse);
	if(cgs.clientinfo->accuracy_shotgl > 0)
	{
		accuracycgl = c2->accuracy_hitgl * 100 / c2->accuracy_shotgl;
	}
	else
	{
		accuracycgl = 0;
	}
	agl = va("^x000000^B^3%i ", accuracycgl);
	CH_DrawStringExt(x + 280, 386 + 50, agl, 6, 7, colorWhite, 85, qfalse);
	pgl = va("%i ", c2->pickupgl);
	CH_DrawStringExt(x + 224, 386 + 50, pgl, 6, 7, colorWhite, 85, qfalse);

	// BFG
	CH_DrawStringExt(x + 10, 393 + 50, "^3BFG^N", 6, 7, colorWhite, 85, qfalse);
	hbfg = va("%i", c2->accuracy_hitbfg);
	CH_DrawStringExt(x + 125, 393 + 50, hbfg, 6, 7, colorWhite, 85, qfalse);
	CH_DrawStringExt(x + 149, 391 + 50, "^1/", 6, 10, colorWhite, 85, qfalse);
	sbfg = va("%i", c2->accuracy_shotbfg);
	CH_DrawStringExt(x + 155, 393 + 50, sbfg, 6, 7, colorWhite, 85, qfalse);
	if(c2->accuracy_shotbfg > 0)
	{
		accuracycbfg = c2->accuracy_hitbfg * 100 / c2->accuracy_shotbfg;
	}
	else
	{
		accuracycbfg = 0;
	}
	abfg = va("^x000000^B^3%i ", accuracycbfg);
	CH_DrawStringExt(x + 280, 393 + 50, abfg, 6, 7, colorWhite, 85, qfalse);
	pbfg = va("%i ", c2->pickupbfg);
	CH_DrawStringExt(x + 224, 393 + 50, pbfg, 6, 7, colorWhite, 85, qfalse);

	// LightningGun
	CH_DrawStringExt(x + 10, 400 + 50, "^3LightningGun^N", 6, 7, colorWhite, 85, qfalse);
	hlg = va("%i", c2->accuracy_hitlg);
	CH_DrawStringExt(x + 125, 400 + 50, hlg, 6, 7, colorWhite, 85, qfalse);
	CH_DrawStringExt(x + 149, 398 + 50, "^1/", 6, 10, colorWhite, 85, qfalse);
	slg = va("%i", c2->accuracy_shotlg);
	CH_DrawStringExt(x + 155, 400 + 50, slg, 6, 7, colorWhite, 85, qfalse);
	if(c2->accuracy_shotlg > 0)
	{
		accuracyclg = c2->accuracy_hitlg * 100 / c2->accuracy_shotlg;
	}
	else
	{
		accuracyclg = 0;
	}
	alg = va("^x000000^B^3%i ", accuracyclg);
	CH_DrawStringExt(x + 280, 400 + 50, alg, 6, 7, colorWhite, 85, qfalse);
	plg = va("%i ", c2->pickuplg);
	CH_DrawStringExt(x + 224, 400 + 50, plg, 6, 7, colorWhite, 85, qfalse);

	// armor
	CG_DrawPic(x + 312, 351 + 50, 7, 7, trap_R_RegisterShaderNoMip("icons/iconr_red"));
	prarmor = va("^1%i ", c2->pickuprarmor);
	CH_DrawStringExt(x + 324, 351 + 50, prarmor, 6, 7, colorWhite, 85, qfalse);

	CG_DrawPic(x + 348, 351 + 50, 7, 7, trap_R_RegisterShaderNoMip("icons/iconr_yellow"));
	pyarmor = va("^1%i ", c2->pickupyarmor);
	CH_DrawStringExt(x + 358, 351 + 50, pyarmor, 6, 7, colorWhite, 85, qfalse);

	CG_DrawPic(x + 384, 351 + 50, 7, 7, trap_R_RegisterShaderNoMip("icons/iconr_shard"));
	pshard = va("^1%i ", c2->pickupshard);
	CH_DrawStringExt(x + 396, 351 + 50, pshard, 6, 7, colorWhite, 85, qfalse);
	// health
	CG_DrawPic(x + 420, 351 + 50, 7, 7, trap_R_RegisterShaderNoMip("icons/iconh_red"));
	ph40 = va("^1%i ", c2->pickup40h);
	CH_DrawStringExt(x + 432, 351 + 50, ph40, 6, 7, colorWhite, 85, qfalse);

	CG_DrawPic(x + 456, 351 + 50, 7, 7, trap_R_RegisterShaderNoMip("icons/iconh_yellow"));
	ph20 = va("^1%i ", c2->pickup20h);
	CH_DrawStringExt(x + 468, 351 + 50, ph20, 6, 7, colorWhite, 85, qfalse);

	CG_DrawPic(x + 492, 351 + 50, 7, 7, trap_R_RegisterShaderNoMip("icons/iconh_green"));
	ph5 = va("^1%i ", c2->pickup5h);
	CH_DrawStringExt(x + 504, 351 + 50, ph5, 6, 7, colorWhite, 85, qfalse);

	CG_DrawPic(x + 528, 351 + 50, 7, 7, trap_R_RegisterShaderNoMip("icons/iconh_mega"));
	pmega = va("^1%i ", c2->pickupmega);
	CH_DrawStringExt(x + 540, 351 + 50, pmega, 6, 7, colorWhite, 85, qfalse);


	// Total Health + Armor Pickup
	CH_DrawStringExt(x + 312, 358 + 50, "^F^1Total Health Recieved^f^3^xff0011Total Health Recieved^N:", 6, 7, colorWhite, 85,
					 qfalse);
	pat = va("^B^xff0011^3%i ", c2->pickupArmorT);
	CH_DrawStringExt(x + 444, 358 + 50, pat, 6, 7, colorWhite, 85, qfalse);

	CH_DrawStringExt(x + 312, 365 + 50, "^F^1Total Armor Recieved^f^3^xff0011Total Armor Recieved^N:", 6, 7, colorWhite, 85,
					 qfalse);
	pht = va("^B^xff0011^3%i ", c2->pickupHealthT);
	CH_DrawStringExt(x + 438, 365 + 50, pht, 6, 7, colorWhite, 85, qfalse);

	// powerup pickups
	CG_DrawPic(x + 312, 379 + 50, 7, 7, trap_R_RegisterShaderNoMip("icons/quad"));
	pquad = va("^1%i ", c2->pickupquad);
	CH_DrawStringExt(x + 324, 379 + 50, pquad, 6, 7, colorWhite, 85, qfalse);

	CG_DrawPic(x + 342, 379 + 50, 7, 7, trap_R_RegisterShaderNoMip("icons/envirosuit"));
	psuit = va("^1%i ", c2->pickupsuit);
	CH_DrawStringExt(x + 354, 379 + 50, psuit, 6, 7, colorWhite, 85, qfalse);

	CG_DrawPic(x + 372, 379 + 50, 7, 7, trap_R_RegisterShaderNoMip("icons/haste"));
	phaste = va("^1%i ", c2->pickuphaste);
	CH_DrawStringExt(x + 384, 379 + 50, phaste, 6, 7, colorWhite, 85, qfalse);

	CG_DrawPic(x + 402, 379 + 50, 7, 7, trap_R_RegisterShaderNoMip("icons/invis"));
	pinvis = va("^1%i ", c2->pickupinvis);
	CH_DrawStringExt(x + 414, 379 + 50, pinvis, 6, 7, colorWhite, 85, qfalse);

	CG_DrawPic(x + 432, 379 + 50, 7, 7, trap_R_RegisterShaderNoMip("icons/regen"));
	pregen = va("^1%i ", c2->pickupregen);
	CH_DrawStringExt(x + 444, 379 + 50, pregen, 6, 7, colorWhite, 85, qfalse);

	CG_DrawPic(x + 462, 379 + 50, 7, 7, trap_R_RegisterShaderNoMip("icons/flight"));
	pfly = va("^1%i ", c2->pickupflight);
	CH_DrawStringExt(x + 474, 379 + 50, pfly, 6, 7, colorWhite, 85, qfalse);

	// CTF flags
	CG_DrawPic(x + 493, 379 + 50, 7, 7, trap_R_RegisterShaderNoMip("icons/iconf_blu1"));
	pbflag = va("^4%i ", c2->pickupbflag);
	CH_DrawStringExt(x + 505, 379 + 50, pbflag, 6, 7, colorWhite, 85, qfalse);

	CG_DrawPic(x + 529, 379 + 50, 7, 7, trap_R_RegisterShaderNoMip("icons/iconf_red1"));
	prflag = va("^1%i ", c2->pickuprflag);
	CH_DrawStringExt(x + 541, 379 + 50, prflag, 6, 7, colorWhite, 85, qfalse);

	// Damage
	CH_DrawStringExt(x + 312, 393 + 50, "^F^1Total Damage Recieved^f^3^xff0011Total Damage Recieved^N:", 6, 7, colorWhite, 85,
					 qfalse);
	drt = va("^B^xff0011^3%i ", c2->damageRT);
	CH_DrawStringExt(x + 444, 393 + 50, drt, 6, 7, colorWhite, 85, qfalse);

	CH_DrawStringExt(x + 312, 400 + 50, "^F^1Total Damage Given^f^3^xff0011Total Damage Given^N:", 6, 7, colorWhite, 85, qfalse);
	dgt = va("^B^xff0011^3%i ", c2->damageGT);
	CH_DrawStringExt(x + 426, 400 + 50, dgt, 6, 7, colorWhite, 85, qfalse);


}

/*
=================================
	CG_ScrollStats
small helper to scroll stats
=================================
*/

void CG_ScrollStat(void)
{

	if(cg.statTime == 999999)
		cg.statTime = 8000;


	if(cg.statTime > 0)
	{
		CG_DrawStats();

		//NT - don't change if the frame takes more than 1/5 second (evil hack)
		if(cg.frametime < 100)
		{
			cg.laststatTime = cg.statTime;
			cg.statTime -= cg.frametime;
		}
	}
}

void CG_ScrollStatFinal(void)
{

	if(cg.statTime1 == 999999)
		cg.statTime1 = 8000;


	if(cg.statTime1 > 0)
	{
		CG_DrawStatsFinal();

		//NT - don't change if the frame takes more than 1/5 second (evil hack)
		if(cg.frametime < 100)
		{
			cg.laststatTime1 = cg.statTime1;
			//  cg.statTime1 -= cg.frametime;
		}
	}
}

/*
=================================
	CG_ScrollMOTD
small helper to scroll motd 
=================================
*/

void CG_Scrolltest(void)
{

	if(cg.logoTime1 == 999999)
		cg.logoTime1 = 8000;


	if(cg.logoTime1 > 0)
	{
		CG_Drawtest();
		//  CG_DrawMOTDText();

		//NT - don't change if the frame takes more than 1/5 second (evil hack)
		if(cg.frametime < 75)
		{
			cg.lastLogoTime1 = cg.logoTime1;
			cg.logoTime1 -= cg.frametime;
		}
	}
}

/*********************************
CameraFXView_REFDEF
A special refdef just for lens effects.

  ~ENC
*********************************/
void CameraFXView_REFDEF(void)
{

	memset(&cg.CameraFXView, 0, sizeof(cg.CameraFXView));

	VectorCopy(cg.refdef[0].vieworg, cg.CameraFXView.vieworg);
//  AnglesToAxis(cg.predictedPlayerEntity.lerpAngles,cg.CameraFXView.viewaxis);
	AxisCopy(cg.refdef[0].viewaxis, cg.CameraFXView.viewaxis);
	cg.CameraFXView.rdflags = RDF_NOWORLDMODEL;

	cg.CameraFXView.fov_x = cg.refdef[0].fov_x;
	cg.CameraFXView.fov_y = cg.refdef[0].fov_y;
	cg.CameraFXView.x = cg.refdef[0].x;
	cg.CameraFXView.y = cg.refdef[0].y;
	cg.CameraFXView.width = cg.refdef[0].width;
	cg.CameraFXView.height = cg.refdef[0].height;


	trap_R_ClearScene();

	CG_AddCameraFX();
	CG_AddMapFX();
	// added the view weapon here so the lens effects don't draw over the gun

	CG_AddViewWeapon(&cg.predictedPlayerState);

	cg.CameraFXView.time = cg.time;
	memcpy(cg.CameraFXView.areamask, cg.snap->areamask, sizeof(cg.refdef[0].areamask));
	if(cg.predictedPlayerState.stats[STAT_HEALTH] > 0)
	{
		trap_R_RenderScene(&cg.CameraFXView);
	}
}

/*
=================
CG_Draw2D
=================
*/
void CG_Draw2D(void)
{
#ifdef MISSIONPACK
	if(cgs.orderPending && cg.time > cgs.orderTime)
	{
		CG_CheckOrderPending();
	}
#endif
	// if we are taking a levelshot for the menu, don't draw anything
	if(cg.levelShot)
	{
		return;
	}

	CameraFXView_REFDEF();

	if(cg_draw2D.integer == 0)
	{
		return;
	}

	if(cg.snap->ps.pm_type == PM_INTERMISSION)
	{
		cg.statTime1 = cg.laststatTime1 = 999999;
		CG_ScrollStatFinal();
		CG_DrawIntermission();
		CG_DrawTeamInfo();
		return;
	}
	CG_ScrollStat();
/*
	if (cg.cameraMode) {
		return;
	}
*/
	CG_Scrolltest();
//  CG_ScrollMOTD();

	if(cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR)
	{
		CG_DrawSpectator();
		CG_DrawCrosshair();
		CG_DrawCrosshairNames();
		CG_DrawTeamInfo();
	}
	else
	{
		// don't draw any status if dead or the scoreboard is being explicitly shown
		if(!cg.showScores && cg.snap->ps.stats[STAT_HEALTH] > 0)
		{

#ifdef MISSIONPACK
			if(cg_drawStatus.integer)
			{
				Menu_PaintAll();
				CG_DrawTimedMenus();
			}
#else
			CG_DrawStatusBar();
#endif

			CG_DrawAmmoWarning();

#ifdef MISSIONPACK
			CG_DrawProxWarning();
#endif
			CG_DrawCrosshair();
			CG_DrawCrosshairNames();
			CG_DrawWeaponSelect();


			CG_DrawReward();
		}

//      if ( cgs.gametype >= GT_TEAM ) {
		//  if(cgs.teamChatMsgTimes){
		CG_DrawTeamInfo();
		//  }

//      }
	}

	CG_DrawVote();
	CG_DrawTeamVote();

	CG_DrawLagometer();

#ifdef MISSIONPACK
	if(!cg_paused.integer)
	{
		CG_DrawUpperRight();
	}
#else
	CG_DrawUpperRight();
#endif

#ifndef MISSIONPACK
	CG_DrawLowerRight();
	CG_DrawLowerLeft();
#endif

	if(!CG_DrawFollow())
	{
		CG_DrawWarmup();
	}

	// don't draw center string if scoreboard is up
	cg.scoreBoardShowing = CG_DrawScoreboard();
	if(!cg.scoreBoardShowing)
	{
		CG_DrawCenterString();
	}

}


static void CG_DrawTourneyScoreboard()
{
#ifdef MISSIONPACK
//#else
	CG_DrawOldTourneyScoreboard();
#endif
}

/*
===============
CG_ZoomView

===============
*/
static void CG_ZoomView(void)
{
	refdef_t        refdef3;

//  refEntity_t     ent;
//  refEntity_t     ent2;
	vec3_t          angles;
	float           x, y, h, w;

	if(cg.renderingThirdPerson)
	{
		return;
	}

	if(cg.predictedPlayerState.pm_type == PM_INTERMISSION)
	{
		return;
	}

	if(cg.predictedPlayerState.stats[STAT_HEALTH] <= 0)
	{
		return;
	}

	if(cg.VFlags & VFL_ZOOM && cg.predictedPlayerState.weapon == WP_RAILGUN ||
	   cg.VFlags & VFL_ZOOM && cg.predictedPlayerState.weapon == WP_IRAILGUN)
	{
		int             size;

		// the intermission should allways be full screen
		if(cg.snap->ps.pm_type == PM_INTERMISSION)
		{
			size = 100;
		}
		else
		{
			// bound normal viewsize
			if(cg_viewsize.integer < 30)
			{
				trap_Cvar_Set("cg_viewsize", "30");
				size = 30;
			}
			else if(cg_viewsize.integer > 100)
			{
				trap_Cvar_Set("cg_viewsize", "100");
				size = 100;
			}
			else
			{
				size = cg_viewsize.integer;
			}

		}

		x = 0;
		y = 0;
		w = 640;
		h = 480;


		angles[0] = 0;
		angles[1] = 90;
		angles[2] = 0;

		CG_AdjustFrom640(&x, &y, &w, &h);

		memset(&refdef3, 0, sizeof(refdef3));

		AxisClear(refdef3.viewaxis);
		refdef3.rdflags = RDF_NOWORLDMODEL;

		refdef3.fov_x = 90;		//cg.refdef.fov_x;
		refdef3.fov_y = 90;		//cg.refdef.fov_y;

		refdef3.width = w;
		refdef3.height = h;
		refdef3.x = x;
		refdef3.y = y;

		refdef3.time = cg.time;

		trap_R_ClearScene();

		/*  memset( &ent, 0, sizeof( ent ) );
		   AnglesToAxis(angles, ent.axis );
		   VectorCopy(vec3_origin,ent.oldorigin);
		   ent.origin[0] = 70;
		   ent.origin[1] = -5.5;
		   ent.origin[2] = 13;
		   ent.hModel = cgs.media.zoomtopModel;
		   ent.reType = RT_PORTALSURFACE;
		   //   ent.renderfx = RF_NOSHADOW;
		   trap_R_AddRefEntityToScene( &ent );

		   /*   memset( &ent2, 0, sizeof( ent2 ) );
		   AnglesToAxis(angles, ent2.axis );
		   ent2.origin[0] = 70;
		   ent2.origin[1] = -5.5;
		   ent2.origin[2] = -13;
		   ent2.hModel = cgs.media.zoombotModel;
		   ent2.renderfx = RF_NOSHADOW;
		   trap_R_AddRefEntityToScene( &ent2 ); */

		trap_R_RenderScene(&refdef3);

		//  CG_DrawPic(-29, -9,700 , 500, cgs.media.zoomringShader);
	}
}

/*
=====================
CG_DrawActive

Perform all drawing needed to completely fill the screen
=====================
*/
void CG_DrawActive(stereoFrame_t stereoView, int view)
{
	float           separation;
	vec3_t          baseOrg;

	// optionally draw the info screen instead
	if(!cg.snap)
	{
		CG_DrawInformation();
		return;
	}

	// optionally draw the tournement scoreboard instead
	if(cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR && (cg.snap->ps.pm_flags & PMF_SCOREBOARD))
	{
		CG_DrawTourneyScoreboard();
		return;
	}

	switch (stereoView)
	{
		case STEREO_CENTER:
			separation = 0;
			break;
		case STEREO_LEFT:
			separation = -cg_stereoSeparation.value / 2;
			break;
		case STEREO_RIGHT:
			separation = cg_stereoSeparation.value / 2;
			break;
		default:
			separation = 0;
			CG_Error("CG_DrawActive: Undefined stereoView");
	}


	// clear around the rendered view if sized down
	CG_TileClear();

	// offset vieworg appropriately if we're doing stereo separation
	VectorCopy(cg.refdef[view].vieworg, baseOrg);
	if(separation != 0)
	{
		VectorMA(cg.refdef[view].vieworg, -separation, cg.refdef[view].viewaxis[1], cg.refdef[view].vieworg);
	}

	// draw 3D view

	if(cg.snap->ps.pm_flags & PMF_FOLLOW)
	{
		if(!(cg.snap->ps.generic1 & GNF_DEADVIEW))
		{
			cg.DeadSet = qfalse;
			trap_R_RenderScene(&cg.refdef[view]);

		}
	}
	else
	{
		if(!(cg.predictedPlayerState.generic1 & GNF_DEADVIEW))
		{
			cg.DeadSet = qfalse;
			trap_R_RenderScene(&cg.refdef[view]);

		}
	}

	// restore original viewpoint if running stereo
	if(separation != 0)
	{
		VectorCopy(baseOrg, cg.refdef[view].vieworg);
	}
	CG_ZoomView();
	// draw status bar and other floating elements
	CG_Draw2D();
}
