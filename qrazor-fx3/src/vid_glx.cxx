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
#include "cl_local.h"

#include "cmd.h"
#include "cvar.h"
#include "vfs.h"
#include "sys.h"
#include "roq.h"

#include "rw_linux.h"

#include "r_local.h"

//
// vid_null.cxx -- null video driver to aid porting efforts
// this assumes that one of the refs is statically linked to the executable

viddef_t	viddef;				// global video state

ref_export_t	re;

extern "C" ref_export_t 	GetRefAPI(ref_import_t rimp);

extern "C"
{
void	KBD_Init();
void	KBD_Update();
void	KBD_Close();
	
void	RW_IN_Init(in_state_t *in_state_p);
void	RW_IN_Shutdown();
void	RW_IN_Activate(bool active);
void	RW_IN_Commands();
void	RW_IN_Move(usercmd_t *cmd);
void	RW_IN_Frame();
}

in_state_t in_state;

void	Real_IN_Init();



void	VID_NewWindow(int width, int height)
{
        viddef.width = width;
        viddef.height = height;
}

struct vidmode_t
{
	const char*	description;
	int		width, height;
	int		mode;
};

vidmode_t vid_modes[] =
{
	{ "Mode 0: 320x240",   320, 240,   0 },
	{ "Mode 1: 400x300",   400, 300,   1 },
	{ "Mode 2: 512x384",   512, 384,   2 },
	{ "Mode 3: 640x480",   640, 480,   3 },
	{ "Mode 4: 800x600",   800, 600,   4 },
	{ "Mode 5: 960x720",   960, 720,   5 },
	{ "Mode 6: 1024x768",  1024, 768,  6 },
	{ "Mode 7: 1152x864",  1152, 864,  7 },
	{ "Mode 8: 1280x1024",  1280, 1024, 8 },
	{ "Mode 9: 1600x1200", 1600, 1200, 9 },
	{ "Mode 10: 2048x1536", 2048, 1536, 10 }
};

#define VID_NUM_MODES ( sizeof( vid_modes ) / sizeof( vid_modes[0] ) )

bool	VID_GetModeInfo(int *width, int *height, int mode)
{
    	if(mode < 3 || mode >= (int)VID_NUM_MODES)
		return false;

	*width  = vid_modes[mode].width;
	*height = vid_modes[mode].height;

	return true;
}


void	VID_Init()
{
	ref_import_t	ri;

	viddef.width = 320;
	viddef.height = 240;

	ri.Com_Printf		= Com_Printf;
	ri.Com_DPrintf		= Com_DPrintf;
	ri.Com_Error		= Com_Error;

	ri.Cbuf_ExecuteText	= Cbuf_ExecuteText;
	
	ri.Cmd_AddCommand	= Cmd_AddCommand;
	ri.Cmd_RemoveCommand	= Cmd_RemoveCommand;
	ri.Cmd_Argc		= Cmd_Argc;
	ri.Cmd_Argv		= Cmd_Argv;
	ri.Cmd_Args		= Cmd_Args;
		
	ri.VFS_FLoad		= VFS_FLoad;
	ri.VFS_FSave		= VFS_FSave;
	ri.VFS_FFree		= VFS_FFree;
	ri.VFS_ListFiles	= VFS_ListFiles;

	ri.Cvar_Get		= Cvar_Get;
	ri.Cvar_Set		= Cvar_Set;
	ri.Cvar_SetValue	= Cvar_SetValue;
	
	ri.Roq_Open		= Roq_Open;
	ri.Roq_Close		= Roq_Close;
	ri.Roq_ReadVideo	= Roq_ReadVideo;
	ri.Roq_ResetStream	= Roq_ResetStream;

	ri.VID_GetModeInfo	= VID_GetModeInfo;
	ri.VID_NewWindow	= VID_NewWindow;
	
	ri.Sys_Milliseconds	= Sys_Milliseconds;

	re = GetRefAPI(ri);

	if(re.api_version != REF_API_VERSION)
		Com_Error(ERR_FATAL, "Re has incompatible api_version");
    
	// call the init function
	if(re.Init(NULL, NULL) == false)
		Com_Error(ERR_FATAL, "Couldn't start refresh");
	
	
	// Init IN (Mouse)
	in_state.Com_PushEvent_fp = Com_PushEvent;
		
	Real_IN_Init();
	
	// Init KBD
	KBD_Init();
}

void	VID_Shutdown()
{
	if(re.Shutdown)
		re.Shutdown();
}

void	VID_CheckChanges()
{
}

uint_t	VID_GetWidth()
{
	return vid.width;
}

uint_t	VID_GetHeight()
{
	return vid.height;
}


/*
================================================================================
				INPUT
================================================================================
*/


cvar_t	*in_joystick;

// This is fake, it's acutally done by the Refresh load
void 	IN_Init()
{
	in_joystick	= Cvar_Get("in_joystick", "0", CVAR_ARCHIVE);
}

void	Real_IN_Init()
{
	RW_IN_Init(&in_state);
}

void 	IN_Shutdown()
{
	RW_IN_Shutdown();
}

void 	IN_Commands()
{
	RW_IN_Commands();
}

void 	IN_Move(usercmd_t *cmd)
{
	RW_IN_Move(cmd);
}

void 	IN_Frame()
{
	if(!cl.refresh_prepped || cls.key_dest == KEY_CONSOLE || cls.key_dest == KEY_MENU)
		RW_IN_Activate(false);
	else
		RW_IN_Activate(true);
	
	RW_IN_Frame();
}

void 	IN_Activate(bool active)
{
}


