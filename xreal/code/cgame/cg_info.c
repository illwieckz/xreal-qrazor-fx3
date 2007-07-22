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

// cg_info.c -- display information while data is being loading
#include "cg_local.h"

/*
======================
CG_LoadingString
======================
*/
void CG_LoadingString(const char *s)
{
	Q_strncpyz(cg.infoScreenText, s, sizeof(cg.infoScreenText));

	trap_UpdateScreen();
}

/*
======================
CG_DrawLoadBar
======================
*/
void CG_DrawLoadBar(void)
{
	int			x,y,pad;

	// Round LCARS buttons
	y = 309;
	x = 10;
	pad = 22;

	/*
        if (cg.loadLCARSStage < 1)
        {
                trap_R_SetColor( colorTable[CT_VDKPURPLE3]);
        }
        else
        {
                trap_R_SetColor( colorTable[CT_VLTGOLD1]);
                CG_DrawPic( x + 222 - 20,y + 14, 16,  16,cgs.media.circle2);
                UI_DrawProportionalString( x + 222, y + 14, ingame_text[IGT_REPLICATION_MATRIX],UI_SMALLFONT, colorTable[CT_VLTGOLD1]);
                trap_R_SetColor( colorTable[CT_VLTPURPLE3]);
        }
        CG_DrawPic( x + 18,   y +102, 128,  64,cgs.media.loading1);


        if (cg.loadLCARSStage < 2)
        {
                trap_R_SetColor( colorTable[CT_VDKBLUE1]);
        }
        else
        {
                trap_R_SetColor( colorTable[CT_VLTGOLD1]);
                CG_DrawPic( x + 222 - 20,y + 14, 16,  16,cgs.media.circle);
                trap_R_SetColor( colorTable[CT_VLTBLUE1]);
        }
        CG_DrawPic(      x,   y + 37,  64,  64,cgs.media.loading2);


        if (cg.loadLCARSStage < 3)
        {
                trap_R_SetColor( colorTable[CT_VDKPURPLE1]);
        }
        else
        {
                trap_R_SetColor( colorTable[CT_VLTGOLD1]);
                CG_DrawPic( x + 222 - 20,y + 14+pad, 16,  16,cgs.media.circle2);
                UI_DrawProportionalString( x + 222, y + 14 + pad, ingame_text[IGT_HOLOGRAPHIC_PROJECTORS],UI_SMALLFONT, colorTable[CT_VLTGOLD1]);
                trap_R_SetColor( colorTable[CT_LTPURPLE1]);
        }
        CG_DrawPic( x + 17,        y, 128,  64,cgs.media.loading3);


        if (cg.loadLCARSStage < 4)
        {
                trap_R_SetColor( colorTable[CT_VDKPURPLE2]);
        }
        else
        {
                trap_R_SetColor( colorTable[CT_VLTGOLD1]);
                CG_DrawPic( x + 222 - 20,y + 14+pad, 16,  16,cgs.media.circle);
                trap_R_SetColor( colorTable[CT_LTPURPLE2]);
        }
        CG_DrawPic( x + 99,        y, 128, 128,cgs.media.loading4);


        if (cg.loadLCARSStage < 5)
        {
                trap_R_SetColor( colorTable[CT_VDKBLUE2]);
        }
        else
        {
                trap_R_SetColor( colorTable[CT_VLTGOLD1]);
                CG_DrawPic( x + 222 - 20,y + 14+pad+pad, 16,  16,cgs.media.circle2);
                UI_DrawProportionalString( x + 222, y + 14 + pad + pad, ingame_text[IGT_SIMULATION_DATA_BASE],UI_SMALLFONT, colorTable[CT_VLTGOLD1]);
                trap_R_SetColor( colorTable[CT_VLTBLUE2]);
        }
        CG_DrawPic( x +137,   y + 81,  64,  64,cgs.media.loading5);


        if (cg.loadLCARSStage < 6)
        {
                trap_R_SetColor( colorTable[CT_VDKORANGE]);
        }
        else
        {
                trap_R_SetColor( colorTable[CT_VLTGOLD1]);
                CG_DrawPic( x + 222 - 20,y + 14+pad+pad, 16,  16,cgs.media.circle);
                trap_R_SetColor( colorTable[CT_LTORANGE]);
        }
        CG_DrawPic( x + 45,   y + 99, 128,  64,cgs.media.loading6);


        if (cg.loadLCARSStage < 7)
        {
                trap_R_SetColor( colorTable[CT_VDKBLUE2]);
        }
        else
        {
                trap_R_SetColor( colorTable[CT_VLTGOLD1]);
                CG_DrawPic( x + 222 - 20,y + 14+pad+pad+pad, 16,  16,cgs.media.circle2);
                UI_DrawProportionalString( x + 222, y + 14 + pad + pad + pad, ingame_text[IGT_SAFETY_LOCKS],UI_SMALLFONT, colorTable[CT_VLTGOLD1]);
                trap_R_SetColor( colorTable[CT_LTBLUE2]);
        }
        CG_DrawPic( x + 38,   y + 24,  64, 128,cgs.media.loading7);

        if (cg.loadLCARSStage < 8)
        {
                trap_R_SetColor( colorTable[CT_VDKPURPLE1]);
        }
        else
        {
                trap_R_SetColor( colorTable[CT_VLTGOLD1]);
                CG_DrawPic( x + 222 - 20,y + 14+pad+pad+pad, 16,  16,cgs.media.circle);
                trap_R_SetColor( colorTable[CT_LTPURPLE1]);
        }
        CG_DrawPic( x + 78,   y + 20, 128,  64,cgs.media.loading8);

        if (cg.loadLCARSStage < 9)
        {
                trap_R_SetColor( colorTable[CT_VDKBROWN1]);
        }
        else
        {
                trap_R_SetColor( colorTable[CT_VLTBROWN1]);
        }
        CG_DrawPic( x +112,   y + 66,  64, 128,cgs.media.loading9);


        if (cg.loadLCARSStage < 9)
        {
                trap_R_SetColor( colorTable[CT_DKBLUE2]);
        }
        else
        {
                trap_R_SetColor( colorTable[CT_LTBLUE2]);
        }
        CG_DrawPic( x + 62,   y + 44, 128, 128,cgs.media.loadingcircle);	// Center arrows

        cg.loadLCARScnt++;
        if (cg.loadLCARScnt > 3)
        {
                cg.loadLCARScnt = 0;
        }

        trap_R_SetColor( colorTable[CT_DKPURPLE2]);
        CG_DrawPic( x +  61,   y + 43,  32,  32,cgs.media.loadingquarter);	// Quad UL
	CG_DrawPic( x + 135,   y + 43, -32,  32,cgs.media.loadingquarter);	// Quad UR
	CG_DrawPic( x + 135,   y +117, -32, -32,cgs.media.loadingquarter);	// Quad LR
	CG_DrawPic( x +  61,   y +117,  32, -32,cgs.media.loadingquarter);	// Quad LL

        trap_R_SetColor( colorTable[CT_LTPURPLE2]);
        switch (cg.loadLCARScnt)
        {
        case 0 :
                CG_DrawPic( x +  61,   y + 43,  32,  32,cgs.media.loadingquarter);	// Quad UL
		break;
        case 1 :
                CG_DrawPic( x + 135,   y + 43, -32,  32,cgs.media.loadingquarter);	// Quad UR
		break;
        case 2 :
                CG_DrawPic( x + 135,   y +117, -32, -32,cgs.media.loadingquarter);	// Quad LR
		break;
        case 3 :
                CG_DrawPic( x +  61,   y +117,  32, -32,cgs.media.loadingquarter);	// Quad LL
		break;
        } 

        UI_DrawProportionalString( x +  21, y + 150, "0987",UI_TINYFONT, colorTable[CT_BLACK]);
        UI_DrawProportionalString( x +   3, y +  90,   "18",UI_TINYFONT, colorTable[CT_BLACK]);
        UI_DrawProportionalString( x +  24, y +  20,    "7",UI_TINYFONT, colorTable[CT_BLACK]);
        UI_DrawProportionalString( x +  93, y +   5,   "51",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
        UI_DrawProportionalString( x + 103, y +   5,   "35",UI_TINYFONT, colorTable[CT_BLACK]);
        UI_DrawProportionalString( x + 165, y +  83,   "21",UI_TINYFONT, colorTable[CT_BLACK]);
        UI_DrawProportionalString( x + 101, y + 149,   "67",UI_TINYFONT, colorTable[CT_BLACK]);
        UI_DrawProportionalString( x + 123, y +  36,   "8",UI_TINYFONT, colorTable[CT_BLACK]);

        UI_DrawProportionalString( x +  90, y +  65, "1",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
        UI_DrawProportionalString( x + 105, y +  65, "2",UI_TINYFONT, colorTable[CT_BLACK]);
        UI_DrawProportionalString( x + 105, y +  87, "3",UI_TINYFONT, colorTable[CT_BLACK]);
        UI_DrawProportionalString( x +  91, y +  87, "4",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);

        trap_R_SetColor( colorTable[CT_DKBROWN1]);
        y +=10;
        CG_DrawPic( x + 130, y - 10 ,  64, 16,cgs.media.loadingtrim);	
        CG_DrawPic( x +  130, y + 150,  64, -16,cgs.media.loadingtrim);	

        CG_DrawPic( x +  150, y - 10,   432,  8, cgs.media.whiteShader);		// Top line
	CG_DrawPic( x +  150, y + 142, 432,  8, cgs.media.whiteShader);		// Bottom line
	CG_DrawPic( x +  583, y - 7,      16, 151, cgs.media.whiteShader);	// Side line

        CG_DrawPic( x +  580, y + 1,      32, -16,cgs.media.loadingcorner);	
        CG_DrawPic( x +  580, y + 139,    32, 16,cgs.media.loadingcorner);	
		*/
}

/*
====================
CG_DrawInformation

Draw all the status / pacifier stuff during level loading
====================
*/
void CG_DrawInformation(void)
{
	const char     *s = NULL;
	const char     *info;
	const char     *sysInfo;
	int             y;
	int             value;
	qhandle_t       levelshot;
	qhandle_t       detail;
	char            buf[1024];

	info = CG_ConfigString(CS_SERVERINFO);
	sysInfo = CG_ConfigString(CS_SYSTEMINFO);

	s = Info_ValueForKey(info, "mapname");
	levelshot = trap_R_RegisterShaderNoMip(va("levelshots/%s.tga", s));
	if(!levelshot)
		levelshot = trap_R_RegisterShaderNoMip("menu/art/unknownmap");

	trap_R_SetColor(NULL);
	CG_DrawPic(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, levelshot);

	// blend a detail texture over it
	detail = trap_R_RegisterShader("levelShotDetail");
	trap_R_DrawStretchPic(0, 0, cgs.glconfig.vidWidth, cgs.glconfig.vidHeight, 0, 0, 2.5, 2, detail);

	// draw the loading bar
	CG_DrawLoadBar();

	// the first 150 rows are reserved for the client connection
	// screen to write into
	if(cg.infoScreenText[0])
	{
		UI_DrawProportionalString(320, 128 - 16, va("Loading... %s", cg.infoScreenText),
								  UI_CENTER | UI_SMALLFONT | UI_DROPSHADOW, colorWhite);
	}
	else
	{
		UI_DrawProportionalString(320, 128 - 16, "Awaiting snapshot...", UI_CENTER | UI_SMALLFONT | UI_DROPSHADOW, colorWhite);
	}

	// draw info string information
	y = 180 - 32;

	// don't print server lines if playing a local game
	trap_Cvar_VariableStringBuffer("sv_running", buf, sizeof(buf));
	if(!atoi(buf))
	{
		// server hostname
		Q_strncpyz(buf, Info_ValueForKey(info, "sv_hostname"), 1024);
		Q_CleanStr(buf);
		UI_DrawProportionalString(320, y, buf, UI_CENTER | UI_SMALLFONT | UI_DROPSHADOW, colorWhite);
		y += PROP_HEIGHT;

		// pure server
		s = Info_ValueForKey(sysInfo, "sv_pure");
		if(s[0] == '1')
		{
			UI_DrawProportionalString(320, y, "Pure Server", UI_CENTER | UI_SMALLFONT | UI_DROPSHADOW, colorWhite);
			y += PROP_HEIGHT;
		}

		// server-specific message of the day
		s = CG_ConfigString(CS_MOTD);
		if(s[0])
		{
			UI_DrawProportionalString(320, y, s, UI_CENTER | UI_SMALLFONT | UI_DROPSHADOW, colorWhite);
			y += PROP_HEIGHT;
		}

		// some extra space after hostname and motd
		y += 10;
	}

	// map-specific message (long map name)
	s = CG_ConfigString(CS_MESSAGE);
	if(s[0])
	{
		UI_DrawProportionalString(320, y, s, UI_CENTER | UI_SMALLFONT | UI_DROPSHADOW, colorWhite);
		y += PROP_HEIGHT;
	}

	// cheats warning
	s = Info_ValueForKey(sysInfo, "sv_cheats");
	if(s[0] == '1')
	{
		UI_DrawProportionalString(320, y, "CHEATS ARE ENABLED", UI_CENTER | UI_SMALLFONT | UI_DROPSHADOW, colorRed);
		y += PROP_HEIGHT;
	}

	// game type
	switch (cgs.gametype)
	{
		case GT_FFA:
			s = "Free For All";
			break;
		case GT_SINGLE_PLAYER:
			s = "Single Player";
			break;
		case GT_TOURNAMENT:
			s = "Tournament";
			break;
		case GT_TEAM:
			s = "Team Deathmatch";
			break;
		case GT_CTF:
			s = "Capture The Flag";
			break;
#ifdef MISSIONPACK
		case GT_1FCTF:
			s = "One Flag CTF";
			break;
		case GT_OBELISK:
			s = "Overload";
			break;
		case GT_HARVESTER:
			s = "Harvester";
			break;
#endif
		default:
			s = "Unknown Gametype";
			break;
	}
	UI_DrawProportionalString(320, y, s, UI_CENTER | UI_SMALLFONT | UI_DROPSHADOW, colorWhite);
	y += PROP_HEIGHT / PROP_SMALL_SIZE_SCALE;

	value = atoi(Info_ValueForKey(info, "timelimit"));
	if(value)
	{
		UI_DrawProportionalString(320, y, va("timelimit %i", value), UI_CENTER | UI_SMALLFONT | UI_DROPSHADOW, colorWhite);
		y += PROP_HEIGHT;
	}

	if(cgs.gametype < GT_CTF)
	{
		value = atoi(Info_ValueForKey(info, "fraglimit"));
		if(value)
		{
			UI_DrawProportionalString(320, y, va("fraglimit %i", value), UI_CENTER | UI_SMALLFONT | UI_DROPSHADOW, colorWhite);
			y += PROP_HEIGHT;
		}
	}
	else if(cgs.gametype >= GT_CTF)
	{
		value = atoi(Info_ValueForKey(info, "capturelimit"));
		if(value)
		{
			UI_DrawProportionalString(320, y, va("capturelimit %i", value), UI_CENTER | UI_SMALLFONT | UI_DROPSHADOW, colorWhite);
			y += PROP_HEIGHT;
		}
	}
}
