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


/// includes ===================================================================
// system -------------------------------------------------------------------
// xreal --------------------------------------------------------------------
#include "cg_local.h"



vrect_t		scr_vrect;



/*
===============================================================================

CENTER PRINTING

===============================================================================
*/

char		scr_centerstring[1024];
float		scr_centertime_start;	// for slow victory printing
float		scr_centertime_off;
int		scr_center_lines;
int		scr_erase_center;

/*
==============
CG_CenterPrint

Called for important messages that should stay in the center of the screen
for a few moments
==============
*/
void 	CG_CenterPrint (char *str)
{
	char	*s;
	char	line[64];
	int		i, j, l;

	strncpy (scr_centerstring, str, sizeof(scr_centerstring)-1);
	scr_centertime_off = cg_centertime->getValue();
	scr_centertime_start = trap_CL_GetTime();

	// count the number of lines for centering
	scr_center_lines = 1;
	s = str;
	while (*s)
	{
		if (*s == '\n')
			scr_center_lines++;
		s++;
	}

	// echo it to the console
	trap_Com_Printf("\n\n\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n\n");

	s = str;
	do	
	{
	// scan the width of the line
		for (l=0 ; l<40 ; l++)
			if (s[l] == '\n' || !s[l])
				break;
		for (i=0 ; i<(40-l)/2 ; i++)
			line[i] = ' ';

		for (j=0 ; j<l ; j++)
		{
			line[i++] = s[j];
		}

		line[i] = '\n';
		line[i+1] = 0;

		trap_Com_Printf ("%s", line);

		while (*s && *s != '\n')
			s++;

		if (!*s)
			break;
		s++;		// skip the \n
	} while (1);
	trap_Com_Printf("\n\n\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n\n");
	trap_Con_ClearNotify ();
}


void 	CG_DrawCenterString()
{
	char	*start;
	int		l;
	int		j;
	int		x, y;
	int		remaining;

	// the finale prints the characters one at a time
	remaining = 9999;

	scr_erase_center = 0;
	start = scr_centerstring;

	if (scr_center_lines <= 4)
		y = (int)(trap_VID_GetHeight()*0.35);
	else
		y = 48;

	do	
	{
		// scan the width of the line
		for (l=0 ; l<40 ; l++)
			if (start[l] == '\n' || !start[l])
				break;
		x = (trap_VID_GetWidth() - l*8)/2;
		for (j=0 ; j<l ; j++, x+=8)
		{
			CG_DrawChar(x, y, start[j], color_white, FONT_NONE);
			if (!remaining--)
				return;
		}
					
		y += 8;

		while (*start && *start != '\n')
			start++;

		if (!*start)
			break;
		start++;		// skip the \n
	} while (1);
}

void 	CG_CheckDrawCenterString (void)
{	
	scr_centertime_off -= trap_CLS_GetFrameTime();
	
	if (scr_centertime_off <= 0)
		return;

	CG_DrawCenterString ();
}



/*
=================
CG_CalcVrect

Sets scr_vrect, the coordinates of the rendered window
=================
*/
void 	CG_CalcVrect()
{
	int		size;

	// bound viewsize
	if(cg_viewsize->getInteger() < 40)
		trap_Cvar_Set("cg_viewsize","40");
		
	if(cg_viewsize->getInteger() > 100)
		trap_Cvar_Set("cg_viewsize","100");

	size = cg_viewsize->getInteger();

	scr_vrect.width = trap_VID_GetWidth()*size/100;
	scr_vrect.width &= ~7;

	scr_vrect.height = trap_VID_GetHeight()*size/100;
	scr_vrect.height &= ~1;

	scr_vrect.x = (trap_VID_GetWidth() - scr_vrect.width)/2;
	scr_vrect.y = (trap_VID_GetHeight() - scr_vrect.height)/2;
}


static void 	CG_SizeUp_f()
{
	trap_Cvar_SetValue("cg_viewsize", cg_viewsize->getInteger() + 10);
}


static void 	CG_SizeDown_f()
{
	trap_Cvar_SetValue("cg_viewsize", cg_viewsize->getInteger() - 10);
}


void 	CG_InitScreen()
{	
	trap_Cmd_AddCommand("sizeup",	CG_SizeUp_f);
	trap_Cmd_AddCommand("sizedown",	CG_SizeDown_f);

//	scr_initialized = true;
}



/*
================
CG_DrawChar

Draws one 8*8 graphics character with 0 being transparent.
It can be clipped to the top of the screen to allow the console to be
smoothly scrolled off.
================
*/
void	CG_DrawChar(int x, int y, int num, const vec4_c &color, int flags)
{
	int			row, col;
	int			width, height;
	float			frow, fcol, size;
	int			shader;

		
	num &= 255;
	
	if((num&127) == 32)
		return;		// space

	if(flags & FONT_MEDIUM)
	{
		width = CHAR_MEDIUM_WIDTH;
		height = CHAR_MEDIUM_HEIGHT;
	}
	else if(flags & FONT_BIG)
	{
		width = CHAR_BIG_WIDTH;
		height = CHAR_BIG_HEIGHT;
	}
	else
	{
		width = CHAR_SMALL_WIDTH;
		height = CHAR_SMALL_HEIGHT;
	}


	if(y <= -height)
		return;			// totally off screen
		
	if(flags & FONT_ALT)
		num = num ^ 0x80;

	row = num>>4;
	col = num&15;

	frow = row*0.0625;
	fcol = col*0.0625;
	size = 0.0625;
	
	if(flags & FONT_CHROME)
		shader = trap_R_RegisterPic("fonts/chromechars");
	else
		shader = trap_R_RegisterPic("textures/bigchars");
		//shader = trap_R_RegisterPic("fonts/conchars");

	
	if(flags & FONT_SHADOWED)
		trap_R_DrawStretchPic(x+2, y+2, width, height, fcol, frow, fcol+size, frow+size, color, shader);
	else
		trap_R_DrawStretchPic(x, y, width, height, fcol, frow, fcol+size, frow+size, color, shader);
}


void	CG_DrawString(int x, int y, const vec4_c &color, int flags, const char *s)
{
	int width;
	
	if(flags & FONT_MEDIUM)
	{
		width = CHAR_MEDIUM_WIDTH;
	}
	else if(flags & FONT_BIG)
	{
		width = CHAR_BIG_WIDTH;
	}
	else
	{
		width = CHAR_SMALL_WIDTH;
	}


	while(*s)
	{
		CG_DrawChar(x, y, *s, color, flags);
		x += width;
		s++;
	}
}

void 	CG_DrawNet()
{
	int incoming_ack, outgoing_seq;
	
	trap_CLS_GetCurrentNetState(incoming_ack, outgoing_seq);

	if(outgoing_seq - incoming_ack < (CMD_BACKUP-1))
		return;

	trap_R_DrawPic(scr_vrect.x+64, scr_vrect.y, 32, 32, color_white, trap_R_RegisterPic("gfx/2d/net"));
}


void 	CG_DrawFPS()
{
	static int fps;
	static double oldtime;
	static int oldframecount;
	double t;
	char s[32];
	int x, width;

	if(!trap_CLS_GetFrameTime() || trap_CLS_GetConnectionState() < CA_CONNECTED)
		return;
		
	if(!cg_showfps->getInteger())
		return;

	if(cg_showfps->getInteger() == 2)
	{
		t = trap_CLS_GetRealTime();
		
		if ((t - oldtime) >= 0.25)
		{
			// updates 4 times a second
			fps = (int)((trap_CLS_GetFrameCount() - oldframecount) / (t - oldtime) + 0.5);
			oldframecount = trap_CLS_GetFrameCount();
			oldtime = t;
		}
	}
	else
		fps = (int)(1.0 / (trap_CLS_GetFrameTime() / 1000));

	Com_sprintf ( s, sizeof( s ), "%3dfps", fps );
	width = strlen(s) * CHAR_MEDIUM_WIDTH;
	x = trap_VID_GetWidth() - 5 - width;
	
	CG_DrawString(x, 2, color_white, FONT_MEDIUM, s);
}



/*
=============
CG_DrawTileClear


This repeats a 64*64 tile graphic to fill the screen around a sized down
refresh window.
=============
*/
static void	CG_DrawTileClearRect(int x, int y, int w, int h, int shader)
{
	float iw, ih;

	iw = 1.0f / 64.0;
	ih = 1.0f / 64.0;

	trap_R_DrawStretchPic(x, y, w, h, x*iw, y*ih, (x+w)*iw, (y+h)*ih, color_white, shader);
}


/*
==============
CG_TileClear

Clear any parts of the tiled background that were drawn on last frame
==============
*/
void 	CG_TileClear()
{
	int		w, h;
	int		top, bottom, left, right;
	int		shader;


	if(cg_viewsize->getInteger() == 100)
		return;		// full screen rendering
		
	
	w = trap_VID_GetWidth();
	h = trap_VID_GetHeight();

	top = scr_vrect.y;
	bottom = top + scr_vrect.height-1;
	left = scr_vrect.x;
	right = left + scr_vrect.width-1;
	
	shader = trap_R_RegisterPic("gfx/2d/backtile");

	// clear above view screen
	CG_DrawTileClearRect(0, 0, w, top, shader);

	// clear below view screen
	CG_DrawTileClearRect(0, bottom, w, h - bottom, shader);

	// clear left of view screen
	CG_DrawTileClearRect(0, top, left, bottom - top + 1, shader);

	// clear left of view screen
	CG_DrawTileClearRect(right, top, w - right, bottom - top + 1, shader);
}





#define STAT_MINUS		10	// num frame for '-' stats digit
char		*sb_nums[11] = 
{
	"numbers/digital/zero_32b",
	"numbers/digital/one_32b",
	"numbers/digital/two_32b",
	"numbers/digital/three_32b",
	"numbers/digital/four_32b",
	"numbers/digital/five_32b",
	"numbers/digital/six_32b",
	"numbers/digital/seven_32b",
	"numbers/digital/eight_32b",
	"numbers/digital/nine_32b",
	"numbers/digital/minus_32b"
};

#define	ICON_WIDTH	24
#define	ICON_HEIGHT	24
//#define	CHAR_WIDTH	16
#define	ICON_SPACE	8



/*
================
SizeHUDString

Allow embedded \n in the string
================
*/
void 	CG_SizeHUDString(char *string, int *w, int *h)
{
	int		lines, width, current;

	lines = 1;
	width = 0;

	current = 0;
	while (*string)
	{
		if (*string == '\n')
		{
			lines++;
			current = 0;
		}
		else
		{
			current++;
			if (current > width)
				width = current;
		}
		string++;
	}

	*w = width * 8;
	*h = lines * 8;
}

static void 	CG_DrawHUDString(char *str, int x, int y, int centerwidth, int Xor)
{
	int		margin;
	char	line[1024];
	int		width;
	int		i;

	margin = x;

	while (*str)
	{
		// scan out one line of text from the string
		width = 0;
		while (*str && *str != '\n')
			line[width++] = *str++;
		line[width] = 0;

		if (centerwidth)
			x = margin + (centerwidth - width*8)/2;
		else
			x = margin;
			
		for (i=0 ; i<width ; i++)
		{
			CG_DrawChar(x, y, line[i]^Xor, color_white, FONT_NONE);
			x += 8;
		}
		
		if (*str)
		{
			str++;	// skip the \n
			x = margin;
			y += 8;
		}
	}
}


static void 	CG_DrawField(int x, int y, const vec4_c &color, int width, int value)
{
	char	num[16], *ptr;
	int		l;
	int		frame;

	if (width < 1)
		return;

	// draw number string
	if (width > 5)
		width = 5;

	Com_sprintf (num, sizeof(num), "%i", value);
	l = strlen(num);
	if (l > width)
		l = width;
	x += 2 + CHAR_MEDIUM_WIDTH*(width - l);

	ptr = num;
	while (*ptr && l)
	{
		if (*ptr == '-')
			frame = STAT_MINUS;
		else
			frame = *ptr -'0';

		trap_R_DrawStretchPic(x, y, CHAR_MEDIUM_WIDTH, CHAR_MEDIUM_WIDTH, 0, 0, 1, 1, color, trap_R_RegisterPic(sb_nums[frame]));
		x += CHAR_MEDIUM_WIDTH;
		ptr++;
		l--;
	}
}

static void 	CG_DrawField(int x, int y, int w, int h, const vec4_c &color, int width, int value)
{
	char	num[16], *ptr;
	int		l;
	int		frame;

	if (width < 1)
		return;

	// draw number string
	if (width > 5)
		width = 5;

	Com_sprintf (num, sizeof(num), "%i", value);
	l = strlen(num);
	if (l > width)
		l = width;
	x += 2 + w*(width - l);

	ptr = num;
	while (*ptr && l)
	{
		if (*ptr == '-')
			frame = STAT_MINUS;
		else
			frame = *ptr -'0';

		trap_R_DrawStretchPic(x, y, w, h, 0, 0, 1, 1, color, trap_R_RegisterPic(sb_nums[frame]));
		x += w;
		ptr++;
		l--;
	}
}



/*
===============
CG_TouchPics

Allows rendering code to cache all needed sbar graphics
===============
*/
void 	CG_RegisterPics()
{
	for(int j=0; j<11; j++)
		trap_R_RegisterPic(sb_nums[j]);

	
	if(cg_crosshair->getInteger())
	{
		//if(cg_crosshair->getInteger() > 3 || cg_crosshair->getInteger() < 0)
		//	cg_crosshair->getInteger() = 3;

		std::string filename = va("crosshairs/crosshair_%i", cg_crosshair->getInteger());
		
		cg.media.shader_crosshair = trap_R_RegisterPic(filename);
	}
}


void	CG_DrawModel(int x, int y, int w, int h, int model, int shader, const vec3_c &origin, const vec3_c &angles)
{
/*
	r_refdef_t refdef;
	r_entity_t entity;

	if(!model)
		return;

	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;
	refdef.setFOV(40);
	refdef.time = trap_CL_GetTime() * 0.001;
	refdef.rdflags = RDF_NOWORLDMODEL;

	entity.model = model;
	entity.custom_shader = shader;
	entity.scale = 1.0f;
	entity.flags = RF_FULLBRIGHT | RF_NOSHADOW | RF_DEPTHHACK; //TODO RF_NODEPTHCOMPARE
	entity.origin = origin;
	entity.origin2 = entity.origin;
	entity.quat.fromAngles(angles);
	
	trap_R_ClearScene();
//	trap_R_AddEntityToScene(entity);

	trap_R_RenderFrame(refdef);
*/
}

void	CG_DrawHUDModel(int x, int y, int w, int h, int model, int shader, float yawspeed)
{
	float autorotate = anglemod(trap_CL_GetTime()/10);

	vec3_c origin, angles;

	origin.set(90, 0, 0);
	//origin.set(90, 0, -10);
	//angles.set(0, anglemod(yawspeed * (trap_CL_GetTime() & 2047) * (360.0 / 2048.0)), 0);
	angles.set(0, autorotate, 0);
	
	CG_DrawModel(x, y, w, h, model, shader, origin, angles);
}


void 	CG_ExecuteLayoutString(char *s)
{
	int		x, y, w, h;
	int		value;
	char	*token;
	int		width;
	int		index;
	client_info_t	*ci;
	vec4_c		color(1, 1, 1, 1);

	if(trap_CLS_GetConnectionState() != CA_ACTIVE)
		return;

	if(!s[0])
		return;
		
	if(!cg_showlayout->getInteger())
		return;

	x = 0;
	y = 0;
	w = 0;
	h = 0;
	width = 3;

	while(s)
	{
		token = Com_Parse (&s);
		if(!strcmp(token, "xl"))
		{
			token = Com_Parse(&s);
			x = atoi(token);
			continue;
		}
		
		if(!strcmp(token, "xr"))
		{
			token = Com_Parse(&s);
			x = trap_VID_GetWidth() + atoi(token);
			continue;
		}
		
		if(!strcmp(token, "xv"))
		{
			token = Com_Parse(&s);
			x = trap_VID_GetWidth()/2 - 160 + atoi(token);
			continue;
		}

		if(!strcmp(token, "yt"))
		{
			token = Com_Parse(&s);
			y = atoi(token);
			continue;
		}
		if(!strcmp(token, "yb"))
		{
			token = Com_Parse(&s);
			y = trap_VID_GetHeight() + atoi(token);
			continue;
		}
		
		if(!strcmp(token, "yv"))
		{
			token = Com_Parse (&s);
			y = trap_VID_GetHeight()/2 - 120 + atoi(token);
			continue;
		}
		
		if(!strcmp(token, "size"))
		{
			token = Com_Parse (&s);
			w = atoi(token);
			
			token = Com_Parse (&s);
			h = atoi(token);
		}
		
		if(!strcmp(token, "color"))
		{
			float	r, g, b, a;
			
			token = Com_Parse (&s);
			r = atof(token);
			
			token = Com_Parse (&s);
			g = atof(token);
			
			token = Com_Parse (&s);
			b = atof(token);
			
			token = Com_Parse (&s);
			a = atof(token);
			
			color.set(r, g, b, a);
		}

		if(!strcmp(token, "pic"))
		{	
			// draw a pic from a stat number
			token = Com_Parse (&s);
			value = cg.frame.playerstate.stats[atoi(token)];
			
			if(value >= MAX_SHADERS)
				trap_Com_Error(ERR_DROP, "Pic >= MAX_SHADERS");
				
			if(trap_CL_GetConfigString(CS_SHADERS+value)[0])
			{
				trap_R_DrawPic(x, y, w, h, color, trap_R_RegisterPic(trap_CL_GetConfigString(CS_SHADERS+value)));
			}
			continue;
		}

		if(!strcmp(token, "client"))
		{	
			// draw a deathmatch client block
			int		score, ping, time;

			token = Com_Parse(&s);
			x = trap_VID_GetWidth()/2 - 160 + atoi(token);
			
			token = Com_Parse(&s);
			y = trap_VID_GetHeight()/2 - 120 + atoi(token);
			
			token = Com_Parse(&s);
			value = atoi(token);
			if(value < 0 || value >= MAX_CLIENTS)
				trap_Com_Error(ERR_DROP, "client >= MAX_CLIENTS");
			ci = &cg.clientinfo[value];

			token = Com_Parse(&s);
			score = atoi(token);

			token = Com_Parse(&s);
			ping = atoi(token);

			token = Com_Parse(&s);
			time = atoi(token);

			CG_DrawString(x+32, y, color_white, FONT_NONE, ci->name.c_str());
			CG_DrawString(x+32, y+8, color_white, FONT_NONE, "Score: ");
			CG_DrawString(x+32+7*8, y+8, color_white, FONT_ALT, va("%i", score));
			CG_DrawString(x+32, y+16, color_white, FONT_NONE, va("Ping:  %i", ping));
			CG_DrawString(x+32, y+24, color_white, FONT_NONE, va("Time:  %i", time));
			
			continue;
		}

		if(!strcmp(token, "ctf"))
		{
			// draw a ctf client block
			int		score, ping;
			char	block[80];

			token = Com_Parse (&s);
			x = trap_VID_GetWidth()/2 - 160 + atoi(token);
			token = Com_Parse (&s);
			y = trap_VID_GetHeight()/2 - 120 + atoi(token);
			

			token = Com_Parse (&s);
			value = atoi(token);
			if (value >= MAX_CLIENTS || value < 0)
				trap_Com_Error (ERR_DROP, "client >= MAX_CLIENTS");
			ci = &cg.clientinfo[value];

			token = Com_Parse (&s);
			score = atoi(token);

			token = Com_Parse (&s);
			ping = atoi(token);
			if (ping > 999)
				ping = 999;

			sprintf(block, "%3d %3d %-12.12s", score, ping, ci->name.c_str());

			if(value == trap_CL_GetPlayerNum())
				CG_DrawString (x, y, color_red, FONT_ALT, block);
			else
				CG_DrawString (x, y, color_white, FONT_NONE, block);
			continue;
		}

		if(!strcmp(token, "picn"))
		{
			// draw a pic from a name
			int	shader;	
			
			token = Com_Parse (&s);
			shader = trap_R_RegisterPic(token);
			
			trap_R_DrawPic(x, y, w, h, color, shader);				
			continue;
		}
		
		if(!strcmp(token, "model"))
		{
			int	model = -1;
			float	yawspeed;
			
			// draw a model from a stat number
			token = Com_Parse (&s);
			value = cg.frame.playerstate.stats[atoi(token)];
			
			//if(value < 0 || value >= MAX_MODELS)
			//	trap_Com_Error(ERR_DROP, "model >= MAX_MODELS");
			
			//if(value)
						
			token = Com_Parse (&s);
			yawspeed = atof(token);
			
			if(trap_CL_GetConfigString(CS_MODELS+value)[0])
			{
				model = trap_R_RegisterModel(trap_CL_GetConfigString(CS_MODELS+value));
				
				CG_DrawHUDModel(x, y, w, h, model, -1, yawspeed);
			}
		}
		
		if(!strcmp(token, "modeln"))
		{
			int	model;
			int	shader;
			float	yawspeed;
			
			// draw a model from a stat number
			token = Com_Parse (&s);
			model = trap_R_RegisterModel(token);
			
			token = Com_Parse (&s);
			shader = trap_R_RegisterShader(token);
			
			token = Com_Parse (&s);
			yawspeed = atof(token);
			
			CG_DrawHUDModel(x, y, w, h, model, shader, yawspeed);
		}

		if(!strcmp(token, "num"))
		{	
			// draw a number
			token = Com_Parse (&s);
			width = atoi(token);
			
			token = Com_Parse (&s);
			value = cg.frame.playerstate.stats[atoi(token)];
			
			CG_DrawField(x, y, color, width, value);
			continue;
		}

		if(!strcmp(token, "hnum"))
		{
			// health number
			width = 3;
			value = cg.frame.playerstate.stats[STAT_HEALTH];
			
			if (value > 25)
				color = color_green;
			else if (value > 0)
				color = (cg.frame.serverframe>>2) & 1 ? color_red : color_white;		// flash
			else
				color = color_red;

			CG_DrawField(x, y, CHAR_MEDIUM_WIDTH, CHAR_MEDIUM_HEIGHT, color, width, value);
			continue;
		}

		if(!strcmp(token, "anum"))
		{
			// ammo number
			width = 3;
			value = cg.frame.playerstate.stats[STAT_AMMO];
			
			if (value > 5)
				color = color_green;
			else if (value >= 0)
				color = (cg.frame.serverframe>>2) & 1 ? color_red : color_white;		// flash
			else
				continue;	// negative number = don't show
	
			CG_DrawField(x, y, CHAR_MEDIUM_WIDTH, CHAR_MEDIUM_HEIGHT, color, width, value);
			continue;
		}
		
		if(!strcmp(token, "fnum"))
		{
			// frags number
			width = 3;
			value = cg.frame.playerstate.stats[STAT_FRAGS];
			
			if (value > 5)
				color = color_green;
			else if (value >= 0)
				color = (cg.frame.serverframe>>2) & 1 ? color_red : color_white;		// flash
			else
				color = color_red;
	
			CG_DrawField(x, y, CHAR_MEDIUM_WIDTH, CHAR_MEDIUM_HEIGHT, color, width, value);
			continue;
		}

		if(!strcmp(token, "rnum"))
		{
			// armor number
			width = 3;
			value = cg.frame.playerstate.stats[STAT_ARMOR];
			if (value < 1)
				continue;

			color = color_white;

			CG_DrawField(x, y, color, width, value);
			continue;
		}
		
		if(!strcmp(token, "wnum"))
		{	
			// weapon window number, index holds ammo value for certain weapon
			token = Com_Parse (&s);
			index = atoi (token);
			
			width = 3;
			value = cg.frame.playerstate.stats[index];
			
			if (value > 5)
				color = color_green;
			else if (value >= 1)
				color = (cg.frame.serverframe>>2) & 1 ? color_red : color_white;
			else
				color = color_red;

			CG_DrawField (x, y, CHAR_SMALL_WIDTH, CHAR_SMALL_HEIGHT, color, width, value);
			continue;
		}



		if(!strcmp(token, "stat_string"))
		{
			token = Com_Parse(&s);
			index = atoi(token);
			if(index < 0 || index >= MAX_CONFIGSTRINGS)
				trap_Com_Error (ERR_DROP, "Bad stat_string index");
			
			index = cg.frame.playerstate.stats[index];
			if(index < 0 || index >= MAX_CONFIGSTRINGS)
				trap_Com_Error (ERR_DROP, "Bad stat_string index");
				
			CG_DrawString(x, y, color, FONT_NONE, trap_CL_GetConfigString(index));
			continue;
		}

		if(!strcmp(token, "cstring"))
		{
			token = Com_Parse(&s);
			CG_DrawHUDString(token, x, y, 640, 0);
			continue;
		}

		if(!strcmp(token, "string"))
		{
			token = Com_Parse(&s);
			CG_DrawString(x, y, color, FONT_NONE, token);
			continue;
		}

		if(!strcmp(token, "cstring2"))
		{
			token = Com_Parse(&s);
			CG_DrawHUDString(token, x, y, 640, 0x80);
			continue;
		}

		if(!strcmp(token, "string2"))
		{
			token = Com_Parse(&s);
			CG_DrawString(x, y, color, FONT_ALT, token);
			continue;
		}

		if(!strcmp(token, "if"))
		{	
			// draw a number
			token = Com_Parse(&s);
			value = cg.frame.playerstate.stats[atoi(token)];
			
			if(!value)
			{	
				// skip to endif
				while(s && strcmp(token, "endif") )
				{
					token = Com_Parse(&s);
				}
			}
			continue;
		}


	}
}

void	CG_DrawCrosshair()
{
	if(trap_CLS_GetConnectionState() != CA_ACTIVE)
		return;

	if(!cg_crosshair->getInteger())
		return;

	if(cg_crosshair_size->getInteger() <= 0)
		return;

	if(cg_crosshair->isModified())
	{
		cg_crosshair->isModified(false);
		//CG_RegisterPics();
		
		std::string filename = va("crosshairs/crosshair_%i", cg_crosshair->getInteger());
		
		cg.media.shader_crosshair = trap_R_RegisterPic(filename);
	}


	trap_R_DrawStretchPic(	scr_vrect.x + ((scr_vrect.width - cg_crosshair_size->getInteger())>>1),
				scr_vrect.y + ((scr_vrect.height - cg_crosshair_size->getInteger())>>1),
				cg_crosshair_size->getInteger(),
				cg_crosshair_size->getInteger(),
				0, 0, 1, 1,
				color_white,
				cg.media.shader_crosshair	);
}


/*
================
CG_DrawStats

The status bar is a small layout program that
is based on the stats array
================
*/
void 	CG_DrawStats()
{
	CG_ExecuteLayoutString((char*)trap_CL_GetConfigString(CS_STATUSBAR));
}


#define	STAT_LAYOUTS		13

void 	CG_DrawLayout()
{
	if(!(cg.frame.playerstate.stats[STAT_LAYOUTS] & 1))
		return;

	if(!cg.frame.playerstate.stats[STAT_LAYOUTS])
		return;
		
	CG_ExecuteLayoutString(cg.layout);
}


void	CG_DrawBlend()
{
	//if(!cg_blend->getValue())
	//	return;
			
	if(cg.v_blend[3] < 0.01f)
		return;
	
	trap_R_DrawFill(scr_vrect.x, scr_vrect.y, scr_vrect.width, scr_vrect.height, cg.v_blend);
}

