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
// qrazor-fx ----------------------------------------------------------------
// xreal --------------------------------------------------------------------
#include "cl_local.h"

#include "cmd.h"
#include "cvar.h"
#include "sys.h"

/*

  full screen console
  put up loading plaque
  blanked background with loading plaque
  blanked background with menu
  cinematics
  full screen image for quit and victory

  end of unit intermissions

  */



float		scr_con_current;	// aproaches scr_conlines at scr_conspeed
float		scr_conlines;		// 0.0 to 1.0 lines of console to display

bool		scr_initialized;	// ready to draw

int		scr_draw_loading;

vrect_t		scr_vrect;		// position of render window on screen



static cvar_t*	scr_conspeed;
static cvar_t*	scr_netgraph;
static cvar_t*	scr_timegraph;
static cvar_t*	scr_debuggraph;
static cvar_t*	scr_graphheight;
static cvar_t*	scr_graphscale;
static cvar_t*	scr_graphshift;
static cvar_t*	scr_viewsize;


void CG_TimeRefresh_f();
void CG_Loading_f();




struct graphsamp_t
{
	int		value;
	vec4_c		color;
};

static	int		current = 0;
static	graphsamp_t	values[1024];

static void 	SCR_DebugGraph(int value, const vec4_c &color)
{
	values[current].value = value;
	values[current].color = color;
	
	current++;
	current &= 1023;
}


void 	SCR_AddNetgraph()
{
	int		i;
	int		in;
	int		ping;

	// if using the debuggraph for something else, don't
	// add the net lines
	if(scr_debuggraph->getValue() || scr_timegraph->getValue())
		return;

	vec4_c color;
	color.set(0.655, 0.231, 0.169, 1.0);

	for(i=0; i<cls.netchan.getDropped(); i++)
		SCR_DebugGraph(30, color);

	for(i=0; i<cl.surpress_count; i++)
		SCR_DebugGraph(30, color_green);

	// see what the latency was on this packet
	in = cls.netchan.getIncomingAcknowledged() & (CMD_BACKUP-1);
	ping = cls.realtime - cl.cmd_time[in];
	ping /= 30;
	if(ping > 30)
		ping = 30;
	
	color.set(1.0, 0.75, 0.06, 1.0);
	
	SCR_DebugGraph(ping, color);
}


void	SCR_DrawDebugGraph()
{
	int		x, y, w, h;

	if(scr_timegraph->getValue())
		SCR_DebugGraph((int)(cls.frametime/3), color_black);


	if(!scr_debuggraph->getValue() || !scr_timegraph->getValue() || !scr_netgraph->getValue())
		return;

	//
	// draw the graph
	//
	w = scr_vrect.width;

	x = scr_vrect.x;
	y = scr_vrect.y+scr_vrect.height;
	
	re.R_DrawFill(x, y-scr_graphheight->getInteger(), w, scr_graphheight->getInteger(), color_grey_med);

	for(int a=0; a<w; a++)
	{
		int i = (current-1-a+1024) & 1023;
		int v = values[i].value;
		v = v * scr_graphscale->getInteger() + scr_graphshift->getInteger();
		
		if(v < 0)
			v += scr_graphheight->getInteger() * (1 + (-v/scr_graphheight->getInteger()));
		h = v % scr_graphheight->getInteger();
		
		re.R_DrawFill(x+w-1-a, y - h, 1, h, values[i].color);
	}
}


/*
==================
CG_RunConsole

Scroll it up or down
==================
*/
void 	SCR_RunConsole()
{
	// decide on the height of the console
	if(cls.key_dest == KEY_CONSOLE)
		scr_conlines = 0.5;		// half screen
	else
		scr_conlines = 0;				// none visible
	
	if(scr_conlines < scr_con_current)
	{
		scr_con_current -= scr_conspeed->getValue() * (cls.frametime * 0.001);
		
		if(scr_conlines > scr_con_current)
			scr_con_current = scr_conlines;

	}
	else if(scr_conlines > scr_con_current)
	{
		scr_con_current += scr_conspeed->getValue() * (cls.frametime * 0.001);
		
		if(scr_conlines < scr_con_current)
			scr_con_current = scr_conlines;
	}
}

void 	SCR_DrawConsole()
{
	Con_CheckResize();
	
	if(cls.state == CA_DISCONNECTED || cls.state == CA_CONNECTING)
	{	
		// forced full screen console
		Con_DrawConsole(1.0);
		return;
	}

	if(cls.state != CA_ACTIVE || !cl.refresh_prepped)
	{
		vec4_c color;
			
		// connected, but can't render
		Con_DrawConsole(0.5);
		
		re.R_DrawFill(0, VID_GetHeight()/2, VID_GetWidth(), VID_GetHeight()/2, color);
		return;
	}

	if(scr_con_current)
	{
		Con_DrawConsole(scr_con_current);
	}
	else
	{
		if(cls.key_dest == KEY_GAME || cls.key_dest == KEY_MESSAGE)
			Con_DrawNotify();	// only draw notify in game
	}
}



static void 	SCR_TimeRefresh_f()
{
	int		i;
	int		start, stop;
	float		time;

	if(cls.state != CA_ACTIVE)
		return;
		
	r_refdef_t refdef;
	refdef.width = VID_GetWidth();
	refdef.height = VID_GetHeight();
	refdef.setFOV(90);
	refdef.view_origin = cl.frame.playerstate.view_offset + (cl.frame.playerstate.pmove.origin * (1.0/16.0));

	start = Sys_Milliseconds();

	if(Cmd_Argc() == 2)
	{
		// run without page flipping
		re.R_BeginFrame();
		for(i=0; i<128; i++)
		{
			refdef.view_angles[1] = i/128.0*360.0;
			re.R_RenderFrame(refdef);
		}
		re.R_EndFrame();
	}
	else
	{
		for(i=0; i<128; i++)
		{
			refdef.view_angles[1] = i/128.0*360.0;

			re.R_BeginFrame();
			re.R_RenderFrame(refdef);
			re.R_EndFrame();
		}
	}

	stop = Sys_Milliseconds();
	time = (stop-start)/1000.0;
	
	Com_Printf("%f seconds (%f fps)\n", time, 128/time);
}

static void 	SCR_Loading_f()
{
	SCR_BeginLoadingPlaque();
}


void 	SCR_Init()
{	
	scr_conspeed		= Cvar_Get("scr_conspeed", "3", 0);

	scr_netgraph		= Cvar_Get("scr_netgraph", "0", CVAR_ARCHIVE);
	scr_timegraph		= Cvar_Get("scr_timegraph", "0", 0);
	scr_debuggraph		= Cvar_Get("scr_debuggraph", "0", 0);
	scr_graphheight		= Cvar_Get("scr_graphheight", "32", 0);
	scr_graphscale		= Cvar_Get("scr_graphscale", "1", 0);
	scr_graphshift		= Cvar_Get("scr_graphshift", "0", 0);
	
	scr_viewsize		= Cvar_Get("viewsize", "100", CVAR_ARCHIVE);
	
	Cmd_AddCommand("timerefresh",	SCR_TimeRefresh_f);
	Cmd_AddCommand("loading",	SCR_Loading_f);
//	Cmd_AddCommand("sizeup",	CG_SizeUp_f);
//	Cmd_AddCommand("sizedown",	CG_SizeDown_f);

	scr_initialized = true;
}

/*
=================
SCR_CalcVrect

Sets scr_vrect, the coordinates of the rendered window
=================
*/
static void 	SCR_CalcVrect()
{
	int		size;

	// bound viewsize
	if(scr_viewsize->getInteger() < 40)
		Cvar_Set("viewsize","40");
		
	if(scr_viewsize->getInteger() > 100)
		Cvar_Set("viewsize","100");

	size = scr_viewsize->getInteger();

	scr_vrect.width = VID_GetWidth()*size/100;
	scr_vrect.width &= ~7;

	scr_vrect.height = VID_GetHeight()*size/100;
	scr_vrect.height &= ~1;

	scr_vrect.x = (VID_GetWidth() - scr_vrect.width)/2;
	scr_vrect.y = (VID_GetHeight() - scr_vrect.height)/2;
}


void	SCR_DrawLoading()
{
	//int		w, h;
		
	if(!scr_draw_loading)
		return;

	scr_draw_loading = false;
	
	//cgi.R_DrawGetPicSize(&w, &h, cgi.R_RegisterPic("loading"));
	//cgi.R_DrawPic((cgi.viddef->width-w)/2, (cgi.viddef->height-h)/2, color_white, cgi.R_RegisterPic("loading"));
}


void	SCR_DrawMenu()
{
	uie->M_Draw();
}


/*
==================
CG_UpdateScreen

This is called every frame, and can also be called explicitly to flush
text to the screen.
==================
*/
void 	SCR_UpdateScreen()
{

	// if the screen is disabled (loading plaque is up, or vid mode changing)
	// do nothing at all
	if(cls.disable_screen)
	{
		if(Sys_Milliseconds() - cls.disable_screen > 120000)
		{
			cls.disable_screen = 0;
			Com_Printf("Loading plaque timed out.\n");
		}
		return;
	}

	if(!scr_initialized || !Con_IsInitialized())
		return;				// not initialized yet

	
	re.R_BeginFrame();

	if(scr_draw_loading == 2)
	{	
		//  loading plaque over black screen
		//int		w, h;

		scr_draw_loading = false;
		//cgi.R_DrawGetPicSize(&w, &h, cgi.R_RegisterPic("loading"));
		//cgi.R_DrawPic((cgi.viddef->width-w)/2, (cgi.viddef->height-h)/2, color_white, cgi.R_RegisterPic("loading"));
	}
	else 
	{
		// do 3D refresh drawing, and then update the screen
		SCR_CalcVrect();
		
		cge->CG_RunFrame();
		
		SCR_DrawDebugGraph();
		
		SCR_DrawConsole();
		
		SCR_DrawLoading();
		
		SCR_DrawMenu();
	}
	
	re.R_EndFrame();
}


void 	SCR_BeginLoadingPlaque()
{
	S_StopAllSounds();
	cl.sound_prepped = false;		// don't play ambients
	
	if(cls.disable_screen)
		return;
		
	if(developer->getValue())
		return;
		
	if(cls.state == CA_DISCONNECTED)
		return;	// if at console, don't bring up the plaque
		
	if(cls.key_dest == KEY_CONSOLE)
		return;
		
	scr_draw_loading = 1;
		
	SCR_UpdateScreen();
	
	cls.disable_screen = Sys_Milliseconds();
	cls.disable_servercount = cl.servercount;
}

void	SCR_EndLoadingPlaque()
{
	cls.disable_screen = 0;
	Con_ClearNotify();
}

