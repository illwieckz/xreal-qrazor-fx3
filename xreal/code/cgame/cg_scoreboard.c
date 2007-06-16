/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006 Robert Beckebans <trebor_7@users.sourceforge.net>
Copyright (C) 2007 Jeremy Hughes <Encryption767@msn.com>

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
// cg_scoreboard -- draw the scoreboard on top of the game screen
#include "cg_local.h"


#define	SCOREBOARD_X		(0)

#define SB_HEADER			86
#define SB_TOP				(SB_HEADER+32)

// Where the status bar starts, so we don't overwrite it
#define SB_STATUSBAR		420

#define SB_NORMAL_HEIGHT	40
#define SB_INTER_HEIGHT		16	// interleaved height

#define SB_MAXCLIENTS_NORMAL  ((SB_STATUSBAR - SB_TOP) / SB_NORMAL_HEIGHT)
#define SB_MAXCLIENTS_INTER   ((SB_STATUSBAR - SB_TOP) / SB_INTER_HEIGHT - 1)

// Used when interleaved



#define SB_LEFT_BOTICON_X	(SCOREBOARD_X+0)
#define SB_LEFT_HEAD_X		(SCOREBOARD_X+32)
#define SB_RIGHT_BOTICON_X	(SCOREBOARD_X+64)
#define SB_RIGHT_HEAD_X		(SCOREBOARD_X+96)
// Normal
#define SB_BOTICON_X		(SCOREBOARD_X+32)
#define SB_HEAD_X			(SCOREBOARD_X+64)

#define SB_SCORELINE_X		112

#define SB_RATING_WIDTH	    (6 * BIGCHAR_WIDTH)	// width 6
#define SB_SCORE_X			(SB_SCORELINE_X + BIGCHAR_WIDTH)	// width 6
#define SB_RATING_X			(SB_SCORELINE_X + 6 * BIGCHAR_WIDTH)	// width 6
#define SB_PING_X			(SB_SCORELINE_X + 12 * BIGCHAR_WIDTH + 8)	// width 5
#define SB_TIME_X			(SB_SCORELINE_X + 17 * BIGCHAR_WIDTH + 8)	// width 5
#define SB_NAME_X			(SB_SCORELINE_X + 22 * BIGCHAR_WIDTH)	// width 15

// The new and improved score board
//
// In cases where the number of clients is high, the score board heads are interleaved
// here's the layout

//
//  0   32   80  112  144   240  320  400   <-- pixel position
//  bot head bot head score ping time name
//  
//  wins/losses are drawn on bot icon now

static qboolean localClient;	// true if local client has been displayed

#define	SCOREBOARD_XB		(0)

#define	SCOREBOARD_XR		(0)


#define SB_SCORELINE_XR		0
#define SB_SCORELINE_XB		(0)

#define SB_HEAD_XR			(SCOREBOARD_XR+5)


#define SB_SCORE_XR			(SB_SCORELINE_XR)	// width 6
#define SB_SCORE_XB			(SB_SCORELINE_XB)	// width 6
#define SB_NAME_XB			(SB_SCORELINE_XB  )	// width 15
#define SB_NAME_XR			(SB_SCORELINE_XR  )	// width 15
#define SB_PING_XR			(SB_SCORELINE_XR  )	// width 5
#define SB_PING_XB			(SB_SCORELINE_XB  )	// width 5

static qboolean localZClient2;	// true if local client has been displayed



							 /*
							    =================
							    CG_DrawScoreboard
							    =================
							  */
static void CG_DrawClientScore(int y, score_t * score, float *color, float fade, qboolean largeFormat)
{
	char            string[1024];
	char            string2[1024];
	vec3_t          headAngles;
	vec3_t          headAngles2;
	clientInfo_t   *ci;
	int             iconx, headx;
	qboolean        haveflag = qfalse;

	if(score->client < 0 || score->client >= cgs.maxclients)
	{
		Com_Printf("Bad score->client: %i\n", score->client);
		return;
	}

	ci = &cgs.clientinfo[score->client];

	iconx = SB_HEAD_XR - 50 + (SB_RATING_WIDTH / 2);
	headx = SB_HEAD_XR - 50 + (SB_RATING_WIDTH / 2);

	// highlight your position
	if(score->client == cg.snap->ps.clientNum)
	{
		float           hcolor[4];
		int             rank;

		localClient = qtrue;

		if(cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR || cgs.gametype >= GT_TEAM)
		{
			rank = -1;
		}
		else
		{
			rank = cg.snap->ps.persistant[PERS_RANK] & ~RANK_TIED_FLAG;
		}
		if(rank == 0)
		{
			hcolor[0] = 0;
			hcolor[1] = 0;
			hcolor[2] = 0.7f;
		}
		else if(rank == 1)
		{
			hcolor[0] = 0.7f;
			hcolor[1] = 0;
			hcolor[2] = 0;
		}
		else if(rank == 2)
		{
			hcolor[0] = 0.7f;
			hcolor[1] = 0.7f;
			hcolor[2] = 0;
		}
		else
		{
			hcolor[0] = 0.7f;
			hcolor[1] = 0.7f;
			hcolor[2] = 0.7f;
		}

		hcolor[3] = fade * 0.7;
		CG_FillRect(0, y, 640, BIGCHAR_HEIGHT + 1, hcolor);
	}

	if(!haveflag)
	{
		if(ci->botSkill > 0 && ci->botSkill <= 5)
		{
			if(cg_drawIcons.integer)
			{
				if(largeFormat)
				{
					CG_DrawPic(iconx, y - (32 - BIGCHAR_HEIGHT) / 2, 32, 32, cgs.media.botSkillShaders[ci->botSkill - 1]);
				}
				else
				{
					CG_DrawPic(iconx, y, 16, 16, cgs.media.botSkillShaders[ci->botSkill - 1]);
				}
			}
		}
		else if(ci->handicap < 100)
		{
			Com_sprintf(string, sizeof(string), "%i", ci->handicap);
			if(cgs.gametype == GT_TOURNAMENT)
				CG_DrawSmallStringColor(0, y - SMALLCHAR_HEIGHT / 2, string, color);
			else
				CG_DrawSmallStringColor(0, y, string, color);
		}

		// draw the wins / losses
		if(cgs.gametype == GT_TOURNAMENT)
		{
			Com_sprintf(string, sizeof(string), "%i/%i", ci->wins, ci->losses);
			if(ci->handicap < 100 && !ci->botSkill)
			{
				CG_DrawSmallStringColor(iconx, y + SMALLCHAR_HEIGHT / 2, string, color);
			}
			else
			{
				CG_DrawSmallStringColor(iconx, y, string, color);
			}
		}
	}

	// draw the face
	VectorClear(headAngles);
	headAngles[YAW] = 180;
	VectorClear(headAngles2);
	headAngles2[YAW] = (cg.time & 2047) * 360 / 2048.0;
	if(largeFormat)
	{
		if(score->client == cg.snap->ps.clientNum)
		{
			CG_DrawHead(headx, y - (ICON_SIZE - BIGCHAR_HEIGHT) / 2, ICON_SIZE, ICON_SIZE, score->client, headAngles2);
		}
		else
		{
			CG_DrawHead(headx, y - (ICON_SIZE - BIGCHAR_HEIGHT) / 2, ICON_SIZE, ICON_SIZE, score->client, headAngles);
		}
	}
	else
	{
		if(score->client == cg.snap->ps.clientNum)
		{
			CG_DrawHead(headx + 35, y, 16, 16, score->client, headAngles2);
		}
		else
		{
			CG_DrawHead(headx + 35, y, 16, 16, score->client, headAngles);
		}
	}

	if(score->ref != 0)
	{
		CH_DrawStringExt(50, y, "^f^3REF^F^0REF^n", BIGCHAR_WIDTH, BIGCHAR_HEIGHT, colorWhite, 0, qfalse);
	}

	// draw the score line
	if(score->ping == -1)
	{
		Com_sprintf(string, sizeof(string), " connecting    %s", ci->name);
		Com_sprintf(string2, sizeof(string2), "%s", ci->name);
	}
	else if(ci->team == TEAM_SPECTATOR)
	{
		Com_sprintf(string, sizeof(string), " SPECT %3i %4i %s", score->ping, score->time, ci->name);
		Com_sprintf(string2, sizeof(string2), "%s", ci->name);
	}
	else
	{
		Com_sprintf(string, sizeof(string), "%5i %4i %4i", score->score, score->ping, score->time);
		Com_sprintf(string2, sizeof(string2), "%s", ci->name);
	}

	CH_DrawStringExt(SB_SCORELINE_XB + (SB_RATING_WIDTH / 2) + 20, y + 2, string, TINYCHAR_WIDTH + 3, TINYCHAR_HEIGHT + 3,
					 colorWhite, 64, qfalse);

	CH_DrawStringExt(SB_NAME_XB + 400, y + 2, string2, TINYCHAR_WIDTH + 3, TINYCHAR_HEIGHT + 3, colorWhite, 64, qfalse);


//  CG_DrawBigString( SB_SCORELINE_X + (SB_RATING_WIDTH / 2), y, string, fade );
	//CG_DrawRect( SB_SCORELINE_X + BIGCHAR_WIDTH + (SB_RATING_WIDTH / 2), y+BIGCHAR_HEIGHT+1, 640 - SB_SCORELINE_X - BIGCHAR_WIDTH, 1, 1, colorWhite );
	//CG_DrawRect( SB_SCORELINE_X + BIGCHAR_WIDTH + (SB_RATING_WIDTH / 2), y, 1, BIGCHAR_HEIGHT+1, 1, colorWhite );

	// add the "ready" marker for intermission exiting
	if(cg.snap->ps.stats[STAT_CLIENTS_READY] & (1 << score->client))
	{
		CG_DrawBigStringColor(iconx, y, "READY", color);
	}
}

/*
=================
CG_TeamScoreboard
=================
*/
static int CG_TeamScoreboard(int y, team_t team, float fade, int maxClients, int lineHeight)
{
	int             i;
	score_t        *score;
	float           color[4];
	int             count;
	clientInfo_t   *ci;

	color[0] = color[1] = color[2] = 1.0;
	color[3] = fade;

	count = 0;
	for(i = 0; i < cg.numScores && count < maxClients; i++)
	{
		score = &cg.scores[i];
		ci = &cgs.clientinfo[score->client];

		if(team != ci->team)
		{
			continue;
		}

		CG_DrawClientScore(y + lineHeight * count, score, color, fade, lineHeight == SB_NORMAL_HEIGHT);

		count++;
	}

	return count;
}

char           *monthStr2[12] = {
	"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
};

char           *weekStr2[7] = {
	"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
};

static void CG_DrawClientZPMScoreS2(int y, score_t * score, float *color, float fade, qboolean largeFormat)
{
	char            string[1024];
	clientInfo_t   *ci;

//  qboolean haveflag = qfalse;

	if(score->client < 0 || score->client >= cgs.maxclients)
	{
		Com_Printf("Bad score->client: %i\n", score->client);
		return;
	}

	ci = &cgs.clientinfo[score->client];


	// draw the score line

	if(score->ping == -1)
	{
		Com_sprintf(string, sizeof(string), " connecting    %s", ci->name);
	}
	else if(ci->team == TEAM_SPECTATOR)
	{
		Com_sprintf(string, sizeof(string), "%3i %s", score->ping, ci->name);
	}


	// highlight your position
	if(score->client == cg.snap->ps.clientNum)
	{
		float           hcolor[4];

		localZClient2 = qtrue;
		hcolor[0] = 0.7f;
		hcolor[1] = 0.7f;
		hcolor[2] = 0.7f;
		hcolor[3] = fade * 0.7;
		CG_FillRect(0, y, 640, BIGCHAR_HEIGHT + 1, hcolor);
	}

	CH_DrawStringExt(220, y + 2, string, TINYCHAR_WIDTH + 3, TINYCHAR_HEIGHT + 3, colorWhite, 64, qfalse);

	// add the "ready" marker for intermission exiting
}

static int CG_TeamZPMScoreboardS2(int y, team_t team, float fade, int maxClients, int lineHeight)
{
	int             i;
	score_t        *score;
	float           color[4];
	int             count;
	clientInfo_t   *ci;

	color[0] = color[1] = color[2] = 1.0;
	color[3] = fade;

	count = 0;
	for(i = 0; i < cg.numScores && count < maxClients; i++)
	{
		score = &cg.scores[i];
		ci = &cgs.clientinfo[score->client];

		if(team != ci->team)
		{
			continue;
		}

		CG_DrawClientZPMScoreS2(y + lineHeight * count, score, color, fade, lineHeight == SB_INTER_HEIGHT + 5);

		count++;
	}

	return count;
}

/*
=================
CG_DrawScoreboard

Draw the normal in-game scoreboard
=================
*/
qboolean CG_DrawOldScoreboard(void)
{
	int             x, y, w, n1, y2, y3;
	float           fade;
	float          *fadeColor;
	char           *s;
	int             maxClients;
	int             lineHeight;
	int             topBorderSize, bottomBorderSize;
	qtime_t         tm;
	char            st[1024];



	// don't draw amuthing if the menu or console is up
	if(cg_paused.integer)
	{
		cg.deferredPlayerLoading = 0;
		return qfalse;
	}

	// don't draw scoreboard during death while warmup up
	if(cg.warmup && !cg.showScores)
	{
		return qfalse;
	}

	if(cg.showScores || cg.predictedPlayerState.pm_type == PM_DEAD || cg.predictedPlayerState.pm_type == PM_INTERMISSION)
	{
		fade = 1.0;
		fadeColor = colorWhite;
	}
	else
	{
		fadeColor = CG_FadeColor(cg.scoreFadeTime, FADE_TIME);

		if(!fadeColor)
		{
			// next time scoreboard comes up, don't print killer
			cg.deferredPlayerLoading = 0;
			cg.killerName[0] = 0;
			return qfalse;
		}
		fade = *fadeColor;
	}


	// fragged by ... line
	if(cg.killerName[0] && cg.predictedPlayerState.pm_type != PM_INTERMISSION && cg.predictedPlayerState.pm_type == PM_DEAD)
	{
		s = va("Fragged by %s", cg.killerName);
//      Com_sprintf(s, sizeof(s),
//          "Fragged by %s",cg.killerName);
		w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;
		x = (SCREEN_WIDTH - w) / 2;
		y = 40 + 50;
//      CG_DrawBigString( x, y, s, fade );
		CH_DrawStringExt(x, y, s, BIGCHAR_WIDTH, BIGCHAR_HEIGHT, colorWhite, 900, qfalse);

	}
	else
	{
		s = va("%s place with %i", CG_PlaceString(cg.snap->ps.persistant[PERS_RANK] + 1), cg.snap->ps.persistant[PERS_SCORE]);
//      Com_sprintf(s, sizeof(s),
//          "Fragged by %s",cg.killerName);
		w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;
		x = (SCREEN_WIDTH - w) / 2;
		y = 40 + 50;
//      CG_DrawBigString( x, y, s, fade );
		CH_DrawStringExt(x, y, s, BIGCHAR_WIDTH, BIGCHAR_HEIGHT, colorWhite, 900, qfalse);
	}

	// scoreboard
	y = SB_HEADER + 50;
	CG_DrawPic(SB_SCORE_XB + 92, y + 5, 32, 16, cgs.media.scoreboardScore);
	CG_DrawPic(SB_PING_XB + 148, y + 5, 32, 16, cgs.media.scoreboardPing);
	CG_DrawPic(SB_PING_XB + 205, y + 5, 32, 16, cgs.media.scoreboardTime);
	CG_DrawPic(SB_NAME_XB + 400, y + 5, 32, 16, cgs.media.scoreboardName);
	CG_DrawRect(0, y, 640, 1, 1, colorWhite);
	CG_DrawRect(0, y + 30, 640, 1, 1, colorWhite);

	y = SB_TOP + 50;
	y2 = SB_TOP + 50;
	maxClients = SB_MAXCLIENTS_INTER + 2;
	lineHeight = SB_INTER_HEIGHT + 2;
	topBorderSize = 9;
	bottomBorderSize = 8;

	localZClient2 = qfalse;

	//
	// teamplay scoreboard
	//
	y += lineHeight / 2;
	y2 += lineHeight / 2;



	n1 = CG_TeamScoreboard(y, TEAM_FREE, fade, maxClients, lineHeight);
	y += (n1 * lineHeight) + BIGCHAR_HEIGHT;
	maxClients -= n1;
	y3 = 0;
	if(y2 >= y)
	{
		y3 = y2;
	}
	else if(y > y2)
	{
		y3 = y;
	}
//      CG_DrawRect( 320, SB_HEADER+30+50, 1, y3 -125 -50, 1, colorWhite );
	n1 = CG_TeamZPMScoreboardS2(y3 + 20, TEAM_SPECTATOR, fade, maxClients, lineHeight);
	CG_DrawRect(260, y3 - 10, 120, 1, 1, colorWhite);
	CG_FillRect(0, y3 - 10, 260, 7, colorGreen);
	CG_DrawRect(260, y3 - 10, 1, 5 + 3, 1, colorWhite);
	CG_DrawRect(380, y3 - 10, 1, 5 + 3, 1, colorWhite);
	CG_FillRect(381, y3 - 10, 640, 7, colorGreen);

	CG_DrawRect(0, y3 - 3, 260, 1, 1, colorWhite);
	CG_DrawRect(380, y3 - 3, 640, 1, 1, colorWhite);
	CH_DrawStringExt(265, y3 - 3, "^xff0011^B^0SPECTATORS", TINYCHAR_WIDTH + 3, TINYCHAR_HEIGHT + 3, colorWhite, 200, qfalse);
	CG_DrawRect(260, y3 + 15, 120, 1, 1, colorWhite);
	CG_DrawRect(260, y3 + 8, 1, 5 + 3, 1, colorWhite);
	CG_DrawRect(380, y3 + 8, 1, 5 + 3, 1, colorWhite);
	CG_DrawRect(0, y3 + 8, 260, 1, 1, colorWhite);
	CG_DrawRect(380, y3 + 8, 640, 1, 1, colorWhite);
	y3 += (n1 * lineHeight) + BIGCHAR_HEIGHT;


	y3 += (n1 * lineHeight) + BIGCHAR_HEIGHT;

	// load any models that have been deferred
	if(++cg.deferredPlayerLoading > 10)
	{
		CG_LoadDeferredPlayers();
	}
	trap_RealTime(&tm);

/*	st = va("^3%2i:%s%i:%s%i %s  %s %s %i, %i",
				(1 + (tm.tm_hour+11) % 12),		// 12 hour format
				(tm.tm_min > 9 ? "" : "0"),		// minute padding
				tm.tm_min,
				(tm.tm_sec > 9 ? "" : "0"),		// second padding
				tm.tm_sec,
				(tm.tm_hour < 12 ? "am" : "pm"),weekStr2[tm.tm_wday],monthStr2[tm.tm_mon],
				tm.tm_mday,
				1900 + tm.tm_year );*/
	Com_sprintf(st, sizeof(st), "^3%2i:%s%i:%s%i %s  %s %s %i, %i", (1 + (tm.tm_hour + 11) % 12),	// 12 hour format
				(tm.tm_min > 9 ? "" : "0"),	// minute padding
				tm.tm_min, (tm.tm_sec > 9 ? "" : "0"),	// second padding
				tm.tm_sec,
				(tm.tm_hour < 12 ? "am" : "pm"), weekStr2[tm.tm_wday], monthStr2[tm.tm_mon], tm.tm_mday, 1900 + tm.tm_year);
	CH_DrawStringExt(10, 470, st, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, colorWhite, 200, qfalse);

	return qtrue;
}

//================================================================================

/*
================
CG_CenterGiantLine
================
*/
static void CG_CenterGiantLine(float y, const char *string)
{
	float           x;
	vec4_t          color;

	color[0] = 1;
	color[1] = 1;
	color[2] = 1;
	color[3] = 1;

	x = 0.5 * (640 - GIANT_WIDTH * CG_DrawStrlen(string));

//  CG_DrawStringExt( x, y, string, color, qtrue, qtrue, GIANT_WIDTH, GIANT_HEIGHT, 0 );
	CH_DrawStringExt(x, y, string, GIANT_WIDTH, GIANT_HEIGHT, color, 0, qfalse);

}

/*
=================
CG_DrawTourneyScoreboard

Draw the oversize scoreboard for tournements
=================
*/
qboolean CG_DrawOldTourneyScoreboard(void)
{
	const char     *s;
	vec4_t          color;
	int             min, tens, ones;
	clientInfo_t   *ci;
	int             y;
	int             i;

	// request more scores regularly
	if(cg.scoresRequestTime + 2000 < cg.time)
	{
		cg.scoresRequestTime = cg.time;
		trap_SendClientCommand("score");
	}

	color[0] = 1;
	color[1] = 1;
	color[2] = 1;
	color[3] = 1;

	// draw the dialog background
	color[0] = color[1] = color[2] = 0;
	color[3] = 1;
	CG_FillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, color);

	// print the mesage of the day
	s = CG_ConfigString(CS_MOTD);
	if(!s[0])
	{
		s = "Scoreboard";
	}

	// print optional title
	CG_CenterGiantLine(8, s);

	// print server time
	ones = cg.time / 1000;
	min = ones / 60;
	ones %= 60;
	tens = ones / 10;
	ones %= 10;
	s = va("%i:%i%i", min, tens, ones);

	CG_CenterGiantLine(64, s);


	// print the two scores

	y = 160;
	if(cgs.gametype >= GT_TEAM)
	{
		//
		// teamplay scoreboard
		//
		CH_DrawStringExt(8, y, "Red Team", GIANT_WIDTH, GIANT_HEIGHT, color, 0, qfalse);

		s = va("%i", cg.teamScores[0]);
		CH_DrawStringExt(632 - GIANT_WIDTH * strlen(s), y, s, GIANT_WIDTH, GIANT_HEIGHT, color, 0, qfalse);


		y += 64;

		CH_DrawStringExt(8, y, "Blue Team", GIANT_WIDTH, GIANT_HEIGHT, color, 0, qfalse);

		s = va("%i", cg.teamScores[1]);
		CH_DrawStringExt(632 - GIANT_WIDTH * strlen(s), y, s, GIANT_WIDTH, GIANT_HEIGHT, color, 0, qfalse);

	}
	else
	{
		//
		// free for all scoreboard
		//
		for(i = 0; i < MAX_CLIENTS; i++)
		{
			ci = &cgs.clientinfo[i];
			if(!ci->infoValid)
			{
				continue;
			}
			if(ci->team != TEAM_FREE)
			{
				continue;
			}

			CH_DrawStringExt(8, y, ci->name, GIANT_WIDTH, GIANT_HEIGHT, color, 0, qfalse);

			s = va("%i", ci->score);
			CH_DrawStringExt(632 - GIANT_WIDTH * strlen(s), y, s, GIANT_WIDTH, GIANT_HEIGHT, color, 0, qfalse);

			y += 64;
		}
	}
	return qtrue;

}
