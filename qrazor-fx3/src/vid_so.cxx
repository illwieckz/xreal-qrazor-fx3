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
#include <assert.h>
#include <dlfcn.h> // ELF dl loader
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>		//Tr3B -  gcc3 -Wall -isspace fix
// qrazor-fx ----------------------------------------------------------------
#include "cl_local.h"

#include "cmd.h"
#include "cvar.h"
#include "vfs.h"
#include "sys.h"
#include "roq.h"

#include "rw_linux.h"

// Structure containing functions exported from refresh DLL
ref_export_t	re;

// Console variables that we need to access from this module
cvar_t		*vid_gamma;
cvar_t		*vid_ref;			// Name of Refresh DLL loaded
cvar_t		*vid_xpos;			// X coordinate of window position
cvar_t		*vid_ypos;			// Y coordinate of window position
cvar_t		*vid_fullscreen;

// Global variables used internally by this module
viddef_t	viddef;				// global video state; used by other modules

void*		reflib_library = NULL;		// Handle to refresh DLL 
bool		reflib_active = false;

/** KEYBOARD **************************************************************/

void (*KBD_Update_fp)();
void (*KBD_Init_fp)();
void (*KBD_Close_fp)();

/** MOUSE *****************************************************************/

in_state_t in_state;

void (*RW_IN_Init_fp)(in_state_t *in_state_p);
void (*RW_IN_Shutdown_fp)(void);
void (*RW_IN_Activate_fp)(bool active);
void (*RW_IN_Commands_fp)(void);
void (*RW_IN_Move_fp)(usercmd_t &cmd);
void (*RW_IN_Frame_fp)(void);

void	Real_IN_Init();


/*
============
VID_Restart_f

Console command to re-start the video mode and refresh DLL. We do this
simply by setting the modified flag for the vid_ref variable, which will
cause the entire video mode and refresh DLL to be reset on the next frame.
============
*/
void 	VID_Restart_f()
{
	vid_ref->isModified(true);
}

struct vidmode_t
{
	const char *description;
	int         width, height;
	int         mode;
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


bool 	VID_GetModeInfo(int *width, int *height, int mode)
{
	if(mode < 0 || mode >= X_asz(vid_modes))
		return false;

	*width  = vid_modes[mode].width;
	*height = vid_modes[mode].height;

	return true;
}

void 	VID_NewWindow(int width, int height)
{
	viddef.width  = width;
	viddef.height = height;
}

void 	VID_FreeReflib()
{
	if (reflib_library)
	{
		if (KBD_Close_fp)
			KBD_Close_fp();
			
		if (RW_IN_Shutdown_fp)
			RW_IN_Shutdown_fp();
			
		dlclose(reflib_library);
	}

	KBD_Init_fp = NULL;
	KBD_Update_fp = NULL;
	KBD_Close_fp = NULL;
	RW_IN_Init_fp = NULL;
	RW_IN_Shutdown_fp = NULL;
	RW_IN_Activate_fp = NULL;
	RW_IN_Commands_fp = NULL;
	RW_IN_Move_fp = NULL;
	RW_IN_Frame_fp = NULL;

	memset (&re, 0, sizeof(re));
	reflib_library = NULL;
	reflib_active  = false;


}

bool 	VID_LoadRefresh(const char *name)
{
	ref_import_t	ri;
	GetRefAPI_t	GetRefAPI;
	char	fn[MAX_OSPATH];
		
	if(reflib_active)
	{
		if (KBD_Close_fp)
			KBD_Close_fp();
		if (RW_IN_Shutdown_fp)
			RW_IN_Shutdown_fp();
		KBD_Close_fp = NULL;
		RW_IN_Shutdown_fp = NULL;
		re.Shutdown();
		VID_FreeReflib ();
	}

	Com_Printf( "------- Loading %s -------\n", name );

	strncpy(fn, VFS_PKGLIBDIR, sizeof(VFS_PKGLIBDIR));
	
	while(*fn && isspace(fn[strlen(fn) - 1]))
		fn[strlen(fn) - 1] = 0;

	strcat(fn, "/");
	strcat(fn, name);

	if((reflib_library = dlopen(fn, RTLD_NOW)) == 0) // | RTLD_GLOBAL)) == 0)
	{
		Com_Printf( "LoadLibrary(\"%s\") failed: %s\n", name , dlerror());
		return false;
	}

  	Com_Printf("LoadLibrary(\"%s\")\n", fn);


	//
	// setup import interface
	// 
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
	ri.Sys_Microseconds	= Sys_Microseconds;
	

	if((GetRefAPI =  (ref_export_t(*)(ref_import_t))dlsym(reflib_library, "GetRefAPI")) == 0)
		Com_Error(ERR_FATAL, "dlsym failed on %s", name);

	re = GetRefAPI(ri);

	if(re.api_version != REF_API_VERSION)
	{
		VID_FreeReflib();
		Com_Error(ERR_FATAL, "%s has incompatible api_version", name);
	}

	// Init IN (Mouse)
	in_state.Com_PushEvent_fp = Com_PushEvent;

	if(	(RW_IN_Init_fp = (void(*)(in_state_t*))dlsym(reflib_library, "RW_IN_Init")) == NULL ||
		(RW_IN_Shutdown_fp = (void(*)())dlsym(reflib_library, "RW_IN_Shutdown")) == NULL ||
		(RW_IN_Activate_fp = (void(*)(bool))dlsym(reflib_library, "RW_IN_Activate")) == NULL ||
		(RW_IN_Commands_fp = (void(*)())dlsym(reflib_library, "RW_IN_Commands")) == NULL ||
		(RW_IN_Move_fp = (void(*)(usercmd_t&))dlsym(reflib_library, "RW_IN_Move")) == NULL ||
		(RW_IN_Frame_fp = (void(*)())dlsym(reflib_library, "RW_IN_Frame")) == NULL
	)
		Com_Error(ERR_FATAL, "No RW_IN functions in REF.\n");

	Real_IN_Init();

	if(re.Init(0, 0) == false)
	{
		re.Shutdown();
		VID_FreeReflib();
		return false;
	}

	// Init KBD
	if(	(KBD_Init_fp = (void(*)())dlsym(reflib_library, "KBD_Init")) == NULL ||
		(KBD_Update_fp = (void(*)())dlsym(reflib_library, "KBD_Update")) == NULL ||
		(KBD_Close_fp = (void(*)())dlsym(reflib_library, "KBD_Close")) == NULL
	)
		Com_Error(ERR_FATAL, "No KBD functions in REF.\n");

	KBD_Init_fp();

	Com_Printf( "------------------------------------\n");
	reflib_active = true;
	return true;
}

/*
============
VID_CheckChanges

This function gets called once just before drawing each frame, and it's sole purpose in life
is to check to see if any of the video mode parameters have changed, and if they have to 
update the rendering DLL and/or video mode to match.
============
*/
void 	VID_CheckChanges()
{
	std::string	name;
	cvar_t *vid_mode;

	if(vid_ref->isModified())
	{
		S_StopAllSounds();
	}

	while(vid_ref->isModified())
	{
		//
		// refresh has changed
		//
		vid_ref->isModified(false);
		vid_fullscreen->isModified(true);
		cl.refresh_prepped = false;
		cls.disable_screen = true;

		name = "ref_" + std::string(vid_ref->getString()) + ".so";
		
		if(!VID_LoadRefresh(name.c_str()))
		{
			if(X_strequal(vid_ref->getString(), "glsl_glx"))
			{
				Com_Printf("Refresh failed\n");
				
				vid_mode = Cvar_Get("vid_mode", "0", 0);
				
				if(vid_mode->getInteger() != 0)
				{
					Com_Printf("Trying mode 0\n");
					
					Cvar_SetValue("vid_mode", 0);
					
					if(!VID_LoadRefresh( name.c_str()))
						Com_Error(ERR_FATAL, "Couldn't initialize renderer plugin!");
				} 
				else
					Com_Error(ERR_FATAL, "Couldn't initialize renderer plugin!");
			}
			
			Cvar_Set("vid_ref", "glsl_glx");

			//
			// drop the console if we fail to load a refresh
			//
			if(cls.key_dest != KEY_CONSOLE)
			{
				Con_ToggleConsole_f();
			}
		}
		cls.disable_screen = false;
	}
}


void 	VID_Init()
{
	
	Com_Printf ("------- VID_Init -------\n");
	
	/* Create the video variables so we know how to start the graphics drivers */
	// if DISPLAY is defined, try X
	vid_ref		= Cvar_Get("vid_ref", "glsl_glx", CVAR_ARCHIVE);
	vid_xpos	= Cvar_Get("vid_xpos", "3", CVAR_ARCHIVE);
	vid_ypos	= Cvar_Get("vid_ypos", "22", CVAR_ARCHIVE);
	vid_fullscreen	= Cvar_Get("vid_fullscreen", "0", CVAR_ARCHIVE);
	vid_gamma	= Cvar_Get( "vid_gamma", "1", CVAR_ARCHIVE );

	/* Add some console commands that we want to handle */
	Cmd_AddCommand("vid_restart", VID_Restart_f);

		
	/* Start the graphics mode and load refresh DLL */
	VID_CheckChanges();
}

void 	VID_Shutdown()
{
	Com_Printf ("------- VID_Shutdown -------\n");
	
	if(reflib_active)
	{
		if (KBD_Close_fp)
			KBD_Close_fp();
			
		if (RW_IN_Shutdown_fp)
			RW_IN_Shutdown_fp();
			
		KBD_Close_fp = NULL;
		RW_IN_Shutdown_fp = NULL;
		
		re.Shutdown();
		
		VID_FreeReflib();
	}
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

void	 Real_IN_Init()
{
	if(RW_IN_Init_fp)
		RW_IN_Init_fp(&in_state);
}

void 	IN_Shutdown()
{
	if(RW_IN_Shutdown_fp)
		RW_IN_Shutdown_fp();
}

void 	IN_Commands()
{
	if(RW_IN_Commands_fp)
		RW_IN_Commands_fp();
}

void 	IN_Move(usercmd_t &cmd)
{
	if(RW_IN_Move_fp)
		RW_IN_Move_fp(cmd);
}

void 	IN_Frame()
{
	if(RW_IN_Activate_fp) 
	{
		if (!cl.refresh_prepped || cls.key_dest == KEY_CONSOLE || cls.key_dest == KEY_MENU)
			RW_IN_Activate_fp(false);
		else
			RW_IN_Activate_fp(true);
	}

	if(RW_IN_Frame_fp)
		RW_IN_Frame_fp();
}

void 	IN_Activate(bool active)
{
}

