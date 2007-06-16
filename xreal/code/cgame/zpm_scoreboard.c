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
// cg_scoreboard -- draw the scoreboard on top of the game screen
#include "cg_local.h"
#include "../game/q_shared.h"

#define	SCOREBOARD_XB		(0)

#define	SCOREBOARD_XR		320

#define SB_HEADER			86
#define SB_TOP				(SB_HEADER+32)

// Where the status bar starts, so we don't overwrite it
#define SB_STATUSBAR		420

#define SB_INTER_HEIGHT		16	// interleaved height

#define SB_MAXCLIENTS_INTER   ((SB_STATUSBAR - SB_TOP) / SB_INTER_HEIGHT - 1)

// Used when interleaved



#define SB_LEFT_BOTICON_XR	(SCOREBOARD_XR+0)
#define SB_LEFT_HEAD_XR		(SCOREBOARD_XR+32)
#define SB_RIGHT_BOTICON_XR	(SCOREBOARD_XR+64)
#define SB_RIGHT_HEAD_XR		(SCOREBOARD_XR+96)
// Normal
#define SB_BOTICON_XR		(SCOREBOARD_XR+15)
#define SB_HEAD_XR			(SCOREBOARD_XR+5)

#define SB_LEFT_BOTICON_XB	(SCOREBOARD_XB+0)
#define SB_LEFT_HEAD_XB		(SCOREBOARD_XB+32)
#define SB_RIGHT_BOTICON_XB	(SCOREBOARD_XB+64)
#define SB_RIGHT_HEAD_XB		(SCOREBOARD_XB+96)
// Normal
#define SB_BOTICON_XB		(SCOREBOARD_XB+15)
#define SB_HEAD_XB			(SCOREBOARD_XB+5)


#define SB_SCORELINE_XR		320
#define SB_SCORELINE_XB		(0)

#define SB_RATING_WIDTH	    (6 * BIGCHAR_WIDTH)	// width 6

#define SB_SCORE_XR			(SB_SCORELINE_XR)	// width 6
#define SB_SCORE_XB			(SB_SCORELINE_XB)	// width 6


#define SB_RATING_XR			(SB_SCORELINE_XR + 6 * BIGCHAR_WIDTH)	// width 6
#define SB_PING_XR			(SB_SCORELINE_XR  )	// width 5
#define SB_NAME_XR			(SB_SCORELINE_XR  )	// width 15

#define SB_RATING_XB			(SB_SCORELINE_XB + 6 * BIGCHAR_WIDTH)	// width 6
#define SB_PING_XB			(SB_SCORELINE_XB  )	// width 5
#define SB_NAME_XB			(SB_SCORELINE_XB  )	// width 15


// The new and improved score board
//
// In cases where the number of clients is high, the score board heads are interleaved
// here's the layout

//
//  0   32   80  112  144   240  320  400   <-- pixel position
//  bot head bot head score ping time name
//  
//  wins/losses are drawn on bot icon now

static qboolean localZClient;	// true if local client has been displayed


/*
=================
CG_DrawScoreboard
=================
*/
static void CG_DrawClientZPMScoreR(int y, score_t * score, float *color, float fade, qboolean largeFormat)
{
	char            string[1024];
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

	if(score->client == cg.snap->ps.clientNum)
	{
		float           hcolor[4];
		int             rank;

		localZClient = qtrue;

		if(cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR)
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
		CG_FillRect(SB_SCORELINE_XR + 10, y, 640, BIGCHAR_HEIGHT + 1, hcolor);
	}


	iconx = SB_HEAD_XR - 50 + (SB_RATING_WIDTH / 2);
	headx = SB_HEAD_XR + 17 + (SB_RATING_WIDTH / 2);

	// draw the handicap or bot skill marker (unless player has flag)
	if(ci->powerups & (1 << PW_REDFLAG) && ci->powerups & (1 << PW_BLUEFLAG))
	{
		CG_DrawFlagModel(iconx, y, 16, 16, TEAM_FREE, qfalse);
		haveflag = qtrue;
	}
	else
	{
		if(ci->powerups & (1 << PW_REDFLAG))
		{
			CG_DrawFlagModel(iconx, y, 16, 16, TEAM_RED, qfalse);
			haveflag = qtrue;
		}
		if(ci->powerups & (1 << PW_BLUEFLAG))
		{
			CG_DrawFlagModel(iconx, y, 16, 16, TEAM_BLUE, qfalse);
			haveflag = qtrue;
		}
	}

	if(!haveflag)
	{
		if(ci->botSkill > 0 && ci->botSkill <= 5)
		{
			if(cg_drawIcons.integer)
			{
				CG_DrawPic(iconx, y, 16, 16, cgs.media.botSkillShaders[ci->botSkill - 1]);
			}
		}
		else if(ci->handicap < 100)
		{
			Com_sprintf(string, sizeof(string), "%i", ci->handicap);
			CG_DrawSmallStringColor(iconx, y, string, color);
		}
	}

	// draw the face
	VectorClear(headAngles);
	headAngles[YAW] = 180;
	VectorClear(headAngles2);
	headAngles2[YAW] = (cg.time & 2047) * 360 / 2048.0;
	if(score->ref == 0)
	{
		if(score->client == cg.snap->ps.clientNum)
		{
			CG_DrawHead(headx, y, 16, 16, score->client, headAngles2);
		}
		else
		{
			CG_DrawHead(headx, y, 16, 16, score->client, headAngles);
		}
	}
	else
	{
		CH_DrawStringExt(headx, y, "^3REF", TINYCHAR_WIDTH, TINYCHAR_HEIGHT + 3, colorWhite, 27, qfalse);
	}

	// draw the score line

	if(score->ping == -1)
	{
		Com_sprintf(string, sizeof(string), " connecting    %s", ci->name);
	}
	else if(ci->team == TEAM_SPECTATOR)
	{
		Com_sprintf(string, sizeof(string), "%3i %s", score->ping, ci->name);
	}
	else
	{
		Com_sprintf(string, sizeof(string), "%5i %4i %s", score->score, score->ping, ci->name);
	}


	// highlight your position


	CH_DrawStringExt(SB_SCORELINE_XR + (SB_RATING_WIDTH / 2) - 50 + 15, y + 2, string, TINYCHAR_WIDTH + 3, TINYCHAR_HEIGHT + 3,
					 colorWhite, 64, qfalse);
	//      CG_DrawBigString( SB_SCORELINE_XR + (SB_RATING_WIDTH / 2)-100, y, string, fade );


//  CG_DrawRect( SB_SCORELINE_XR + BIGCHAR_WIDTH + (SB_RATING_WIDTH / 2), y+BIGCHAR_HEIGHT+1, 640 - SB_SCORELINE_XR - BIGCHAR_WIDTH, 1, 1, colorWhite );
//  CG_DrawRect( SB_SCORELINE_XR + BIGCHAR_WIDTH + (SB_RATING_WIDTH / 2), y, 1, BIGCHAR_HEIGHT+1, 1, colorWhite );

	// add the "ready" marker for intermission exiting
	if(cg.snap->ps.stats[STAT_CLIENTS_READY] & (1 << score->client))
	{
		CG_DrawBigStringColor(iconx, y, "READY", color);
	}
}

							 /*
							    =================
							    CG_DrawScoreboard
							    =================
							  */
static void CG_DrawClientZPMScoreB(int y, score_t * score, float *color, float fade, qboolean largeFormat)
{
	char            string[1024];
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

	// highlight your position
	if(score->client == cg.snap->ps.clientNum)
	{
		float           hcolor[4];
		int             rank;

		localZClient = qtrue;

		if(cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR)
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
		CG_FillRect(0, y, 315, BIGCHAR_HEIGHT + 1, hcolor);
	}


	iconx = SB_HEAD_XB - 50 + (SB_RATING_WIDTH / 2);
	headx = SB_HEAD_XB + 17 + (SB_RATING_WIDTH / 2);

	// draw the handicap or bot skill marker (unless player has flag)
	if(ci->powerups & (1 << PW_REDFLAG) && ci->powerups & (1 << PW_BLUEFLAG))
	{
		CG_DrawFlagModel(iconx, y, 16, 16, TEAM_FREE, qfalse);
		haveflag = qtrue;
	}
	else
	{
		if(ci->powerups & (1 << PW_REDFLAG))
		{
			CG_DrawFlagModel(iconx, y, 16, 16, TEAM_RED, qfalse);
			haveflag = qtrue;
		}
		if(ci->powerups & (1 << PW_BLUEFLAG))
		{
			CG_DrawFlagModel(iconx, y, 16, 16, TEAM_BLUE, qfalse);
			haveflag = qtrue;
		}
	}

	if(!haveflag)
	{
		if(ci->botSkill > 0 && ci->botSkill <= 5)
		{
			if(cg_drawIcons.integer)
			{
				CG_DrawPic(iconx, y, 16, 16, cgs.media.botSkillShaders[ci->botSkill - 1]);
			}
		}
		else if(ci->handicap < 100)
		{
			Com_sprintf(string, sizeof(string), "%i", ci->handicap);
			CG_DrawSmallStringColor(iconx, y, string, color);
		}
	}

	// draw the face
	VectorClear(headAngles);
	headAngles[YAW] = 180;
	VectorClear(headAngles2);
	headAngles2[YAW] = (cg.time & 2047) * 360 / 2048.0;

	if(score->client == cg.snap->ps.clientNum)
	{
		CG_DrawHead(headx, y, 16, 16, score->client, headAngles2);
	}
	else
	{
		CG_DrawHead(headx, y, 16, 16, score->client, headAngles);
	}

	// draw the score line

	if(score->ping == -1)
	{
		Com_sprintf(string, sizeof(string), " connecting    %s", ci->name);
	}
	else if(ci->team == TEAM_SPECTATOR)
	{
		Com_sprintf(string, sizeof(string), "%3i %s", score->ping, ci->name);
	}
	else
	{
		Com_sprintf(string, sizeof(string), "%5i %4i %s", score->score, score->ping, ci->name);
	}




	CH_DrawStringExt(SB_SCORELINE_XB + (SB_RATING_WIDTH / 2) - 50 + 15, y + 2, string, TINYCHAR_WIDTH + 3, TINYCHAR_HEIGHT + 3,
					 colorWhite, 64, qfalse);
	//  CG_DrawBigString( SB_SCORELINE_XB + (SB_RATING_WIDTH / 2)-100, y, string, fade );


//  CG_DrawRect( SB_SCORELINE_XB + BIGCHAR_WIDTH + (SB_RATING_WIDTH / 2), y, 1, BIGCHAR_HEIGHT+1, 1, colorWhite );

	// add the "ready" marker for intermission exiting
	if(cg.snap->ps.stats[STAT_CLIENTS_READY] & (1 << score->client))
	{
		CG_DrawBigStringColor(iconx, y, "READY", color);
	}
}

							 /*
							    =================
							    CG_DrawScoreboard
							    =================
							  */
static void CG_DrawClientZPMScoreS(int y, score_t * score, float *color, float fade, qboolean largeFormat)
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

		localZClient = qtrue;
		hcolor[0] = 0.7f;
		hcolor[1] = 0.7f;
		hcolor[2] = 0.7f;
		hcolor[3] = fade * 0.7;
		CG_FillRect(0, y, 640, BIGCHAR_HEIGHT + 1, hcolor);
	}

	CH_DrawStringExt(220, y + 2, string, TINYCHAR_WIDTH + 3, TINYCHAR_HEIGHT + 3, colorWhite, 64, qfalse);

	// add the "ready" marker for intermission exiting
}


/*
=================
CG_TeamScoreboard
=================
*/
static int CG_TeamZPMScoreboardR(int y, team_t team, float fade, int maxClients, int lineHeight)
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

		CG_DrawClientZPMScoreR(y + lineHeight * count, score, color, fade, lineHeight == SB_INTER_HEIGHT + 5);

		count++;
	}

	return count;
}

/*
=================
CG_TeamScoreboard
=================
*/
static int CG_TeamZPMScoreboardB(int y, team_t team, float fade, int maxClients, int lineHeight)
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

		CG_DrawClientZPMScoreB(y + lineHeight * count, score, color, fade, lineHeight == SB_INTER_HEIGHT + 5);

		count++;
	}

	return count;
}

/*
=================
CG_TeamScoreboard
=================
*/
static int CG_TeamZPMScoreboardS(int y, team_t team, float fade, int maxClients, int lineHeight)
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

		CG_DrawClientZPMScoreS(y + lineHeight * count, score, color, fade, lineHeight == SB_INTER_HEIGHT + 5);

		count++;
	}

	return count;
}

static char    *monthStr[12] = {
	"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
};

static char    *weekStr[7] = {
	"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
};

/*
=================
CG_DrawScoreboard

Draw the normal in-game scoreboard
=================
*/
qboolean CG_DrawZPMScoreboard(void)
{
	int             x, y, w, i, n1, y2, y3;
	float           fade;
	float          *fadeColor;
	vec3_t          angles;
	vec3_t          origin;
	const char     *s;
	int             maxClients;
	int             lineHeight;
	int             topBorderSize, bottomBorderSize;
	qtime_t         tm;
	const char     *st;



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
	if(cg.killerName)
	{
		s = va("Fragged by %s", cg.killerName);
		w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;
		x = (SCREEN_WIDTH - w) / 2;
		y = 40 + 50;
		CG_DrawBigString(x, y, s, fade);
	}

	// current rank
	if(cg.teamScores[0] == cg.teamScores[1])
	{
		s = va("Teams are tied at %i", cg.teamScores[0]);
	}
	else if(cg.teamScores[0] >= cg.teamScores[1])
	{
		s = va("Red leads %i to %i", cg.teamScores[0], cg.teamScores[1]);
	}
	else
	{
		s = va("Blue leads %i to %i", cg.teamScores[1], cg.teamScores[0]);
	}

	w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;
	x = (SCREEN_WIDTH - w) / 2;
	y = 60 + 50;
	CG_DrawBigString(x, y, s, fade);


	// scoreboard
	y = SB_HEADER + 50;
	CG_DrawPic(SB_SCORE_XR + 50, y + 5, 32, 16, cgs.media.scoreboardScore);
	CG_DrawPic(SB_PING_XR + 100, y + 5, 32, 16, cgs.media.scoreboardPing);
	CG_DrawPic(SB_NAME_XR + 200, y + 5, 32, 16, cgs.media.scoreboardName);
	CG_DrawPic(SB_SCORE_XB + 50, y + 5, 32, 16, cgs.media.scoreboardScore);
	CG_DrawPic(SB_PING_XB + 100, y + 5, 32, 16, cgs.media.scoreboardPing);
	CG_DrawPic(SB_NAME_XB + 200, y + 5, 32, 16, cgs.media.scoreboardName);
	CG_DrawRect(320, y + 1, 1, 29, 1, colorWhite);
	CG_DrawRect(0, y, 640, 1, 1, colorWhite);
	CG_DrawRect(0, y + 30, 120, 1, 1, colorWhite);
	CG_DrawRect(200, y + 30, 240, 1, 1, colorWhite);
	CG_DrawRect(520, y + 30, 640, 1, 1, colorWhite);

	origin[0] = 90;
	origin[1] = 0;
	origin[2] = -10;
	angles[YAW] = 180;
	CG_Draw3DModel(455 - 35, y + 17.5f, ICON_SIZE * 2.4f, ICON_SIZE / 2.5f,
				   trap_R_RegisterModel("models/zpm/scorerm.md3"), trap_R_RegisterShader("scorefxr01"), origin, angles);
	CG_Draw3DModel(132 - 31, y + 17.5f, ICON_SIZE * 2.4f, ICON_SIZE / 2.5f,
				   trap_R_RegisterModel("models/zpm/scorebm.md3"), trap_R_RegisterShader("scorefxb01"), origin, angles);


	CH_DrawStringExt(132, y + 27, "^x000000^4^BBLUE TEAM", 6, 6, colorWhite, 200, qfalse);
	CH_DrawStringExt(455, y + 27, "^x000000^1^BRED TEAM", 6, 6, colorWhite, 200, qfalse);

	y = SB_TOP + 50;
	y2 = SB_TOP + 50;
	maxClients = SB_MAXCLIENTS_INTER + 2;
	lineHeight = SB_INTER_HEIGHT + 2;
	topBorderSize = 9;
	bottomBorderSize = 8;

	localZClient = qfalse;

	//
	// teamplay scoreboard
	//
	y += lineHeight / 2;
	y2 += lineHeight / 2;



	n1 = CG_TeamZPMScoreboardR(y, TEAM_RED, fade, maxClients, lineHeight);
	CG_DrawTeamBackground(320 + 2, y - topBorderSize, 320, n1 * lineHeight + bottomBorderSize, 0.33f, TEAM_RED);
	y += (n1 * lineHeight) + BIGCHAR_HEIGHT;
	maxClients -= n1;
	n1 = CG_TeamZPMScoreboardB(y2, TEAM_BLUE, fade, maxClients, lineHeight);
	CG_DrawTeamBackground(0, y2 - topBorderSize, 320 - 2, n1 * lineHeight + bottomBorderSize, 0.33f, TEAM_BLUE);
	y2 += (n1 * lineHeight) + BIGCHAR_HEIGHT;
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
	CG_DrawRect(320, SB_HEADER + 30 + 50, 1, y3 - 125 - 50, 1, colorWhite);
	n1 = CG_TeamZPMScoreboardS(y3 + 20, TEAM_SPECTATOR, fade, maxClients, lineHeight);
	CG_DrawRect(260, y3 - 10, 120, 1, 1, colorWhite);
	CG_FillRect(0, y3 - 10, 260, 7, colorBlue);
	CG_DrawRect(260, y3 - 10, 1, 5 + 3, 1, colorWhite);
	CG_DrawRect(380, y3 - 10, 1, 5 + 3, 1, colorWhite);
	CG_FillRect(381, y3 - 10, 640, 7, colorRed);

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
//      CG_DrawRect( 0, y3-30, 640, 1, 1, colorWhite );

	if(!localZClient)
	{
		// draw local client at the bottom
		for(i = 0; i < cg.numScores; i++)
		{
			if(cg.scores[i].client == cg.snap->ps.clientNum)
			{
				CG_DrawClientZPMScoreR(y, &cg.scores[i], fadeColor, fade, lineHeight == SB_INTER_HEIGHT);
				break;
			}
		}
	}

	// load any models that have been deferred
	if(++cg.deferredPlayerLoading > 10)
	{
		CG_LoadDeferredPlayers();
	}

/*
	mins = tm_min;
	secs =	tm_sec;
	hours =	tm_hour;
	mdays =	tm_mday;
	mons =	tm_mon;
	years =	tm_year;
	wdays =	tm_wday;
*/

	trap_RealTime(&tm);

	st = va("^3%2i:%s%i:%s%i %s  %s %s %i, %i", (1 + (tm.tm_hour + 11) % 12),	// 12 hour format
			(tm.tm_min > 9 ? "" : "0"),	// minute padding
			tm.tm_min, (tm.tm_sec > 9 ? "" : "0"),	// second padding
			tm.tm_sec, (tm.tm_hour < 12 ? "am" : "pm"), weekStr[tm.tm_wday], monthStr[tm.tm_mon], tm.tm_mday, 1900 + tm.tm_year);
	CH_DrawStringExt(10, 470, st, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, colorWhite, 200, qfalse);

	return qtrue;
}

//================================================================================
