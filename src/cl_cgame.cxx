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
// qrazor-fx ----------------------------------------------------------------
#include "cl_local.h"

#include "cmd.h"
#include "vfs.h"
#include "cvar.h"
#include "cm.h"
#include "sys.h"

// xreal --------------------------------------------------------------------
#include "xreal/cg_public.h"


cg_export_t*	cge = NULL;

static void*	cge_handle = NULL;


static int	CL_GetTime()
{
	return cl.time;
}

static void	CL_SetTime(int time)
{
	cl.time = time;
}

static const char*	CL_GetConfigString(int index)
{
	return cl.configstrings[index];
}

static void	CL_GetUserCommand(int frame, usercmd_t &cmd)
{
	cmd = cl.cmds[frame & CMD_MASK];
}

static void	CL_GetCurrentUserCommand(usercmd_t &cmd)
{
	cmd = cl.cmds[cl.cmds_num & CMD_MASK];
}

static bool	CL_GetRefreshPrepped()
{
	return cl.refresh_prepped;
}

static void	CL_SetRefreshPrepped(bool val)
{
	cl.refresh_prepped = val;
}

static bool	CL_GetForceRefdef()
{
	return cl.force_refdef;
}

static void	CL_SetForceRefdef(bool val)
{
	cl.force_refdef = val;
}

static int	CL_GetPlayerNum()
{
	return cl.playernum;
}


static connection_state_t	CLS_GetConnectionState()
{
	return cls.state;
}

static float	CLS_GetRealTime()
{
	return cls.realtime;
}

static float	CLS_GetFrameTime()
{
	return cls.frametime;
}

static int	CLS_GetFrameCount()
{
	return cls.framecount;
}

static void	CLS_GetCurrentNetState(int &incoming_acknowledged, int &outgoing_sequence)
{
	incoming_acknowledged = cls.netchan.getIncomingAcknowledged();
	
	outgoing_sequence = cls.netchan.getOutgoingSequence();
}

void	CL_InitClientGame()
{
	cg_import_t	import;

	Com_Printf("------- CL_InitClientGame -------\n");
	
	// unload anything we have now
	if(cge)
		CL_ShutdownClientGame();

	
	import.Com_Printf		= Com_Printf;
	import.Com_DPrintf		= Com_DPrintf;
	import.Com_Error		= Com_Error;
		
	//import.Cbuf_AddText		= Cbuf_AddText;
	//import.Cbuf_InsertText		= Cbuf_InsertText;
	//import.Cbuf_ExecuteText		= Cbuf_ExecuteText;
	//import.Cbuf_Execute		= Cbuf_Execute;
	
	import.Cmd_AddCommand		= Cmd_AddCommand;
	import.Cmd_RemoveCommand	= Cmd_RemoveCommand;
	import.Cmd_Argc			= Cmd_Argc;
	import.Cmd_Argv			= Cmd_Argv;

	import.Cvar_Get			= Cvar_Get;
	import.Cvar_Set			= Cvar_Set;
	//import.Cvar_ForceSet		= Cvar_ForceSet;
	import.Cvar_SetValue		= Cvar_SetValue;
	//import.Cvar_SetModified		= Cvar_SetModified;
	import.Cvar_VariableValue	= Cvar_VariableValue;
	import.Cvar_VariableInteger	= Cvar_VariableInteger;
	//import.Cvar_VariableString	= Cvar_VariableString;
	//import.Cvar_ClampVariable	= Cvar_ClampVariable;
	
	import.R_BeginRegistration	= re.R_BeginRegistration;
	import.R_RegisterModel		= re.R_RegisterModel;
	import.R_RegisterAnimation	= re.R_RegisterAnimation;
	import.R_RegisterSkin		= re.R_RegisterSkin;
	import.R_RegisterPic		= re.R_RegisterPic;
	import.R_RegisterParticle	= re.R_RegisterParticle;
	import.R_RegisterLight		= re.R_RegisterLight;
	import.R_EndRegistration	= re.R_EndRegistration;
	
	import.R_BeginFrame		= re.R_BeginFrame;
	import.R_RenderFrame		= re.R_RenderFrame;
	import.R_EndFrame		= re.R_EndFrame;

	import.R_DrawPic		= re.R_DrawPic;
	import.R_DrawStretchPic		= re.R_DrawStretchPic;
	import.R_DrawFill		= re.R_DrawFill;
	
	import.R_ClearScene		= re.R_ClearScene;
	
	import.R_AddEntity		= re.R_AddEntity;
	import.R_UpdateEntity		= re.R_UpdateEntity;
	import.R_RemoveEntity		= re.R_RemoveEntity;
	
	import.R_AddLight		= re.R_AddLight;
	import.R_UpdateLight		= re.R_UpdateLight;
	import.R_RemoveLight		= re.R_RemoveLight;
	
	import.R_AddParticle		= re.R_AddParticle;
	import.R_AddPoly		= re.R_AddPoly;
	import.R_AddContact		= re.R_AddContact;
	
	import.R_SetupTag		= re.R_SetupTag;
	import.R_SetupAnimation		= re.R_SetupAnimation;
				
	import.VFS_FOpenRead		= VFS_FOpenRead;
	import.VFS_FOpenWrite		= VFS_FOpenWrite;
	import.VFS_FClose		= VFS_FClose;
	
	import.VFS_FLoad		= VFS_FLoad;
	import.VFS_FSave		= VFS_FSave;
	import.VFS_FFree		= VFS_FFree;

	import.VFS_FRead		= VFS_FRead;
	import.VFS_FWrite		= VFS_FWrite;
			
	import.Key_KeynumToString	= Key_KeynumToString;
	import.Key_GetBinding		= Key_GetBinding;
		
	import.Con_ClearNotify		= Con_ClearNotify;
			
	import.S_Init			= S_Init;
	import.S_Shutdown		= S_Shutdown;
	
	import.S_StartSound		= S_StartSound;
	
	import.S_StartLoopSound		= S_StartLoopSound;
	import.S_UpdateLoopSound	= S_UpdateLoopSound;
	import.S_StopLoopSound		= S_StopLoopSound;
	
	import.S_StopAllSounds		= S_StopAllSounds;
	import.S_Update			= S_Update;
	
	import.S_BeginRegistration	= S_BeginRegistration;
	import.S_RegisterSound		= S_RegisterSound;
	import.S_EndRegistration	= S_EndRegistration;
				
	import.CM_BeginRegistration		= CM_BeginRegistration;
	import.CM_RegisterModel			= CM_RegisterModel;
	import.CM_EndRegistration		= CM_EndRegistration;
	
	import.CL_GetTime		= CL_GetTime;
	import.CL_SetTime		= CL_SetTime;
	import.CL_GetConfigString	= CL_GetConfigString;
	import.CL_GetUserCommand	= CL_GetUserCommand;
	import.CL_GetCurrentUserCommand	= CL_GetCurrentUserCommand;
	import.CL_GetRefreshPrepped	= CL_GetRefreshPrepped;
	import.CL_SetRefreshPrepped	= CL_SetRefreshPrepped;
	import.CL_GetForceRefdef	= CL_GetForceRefdef;
	import.CL_SetForceRefdef	= CL_SetForceRefdef;
	import.CL_GetPlayerNum		= CL_GetPlayerNum;
	
	import.CLS_GetConnectionState	= CLS_GetConnectionState;
	import.CLS_GetRealTime		= CLS_GetRealTime;
	import.CLS_GetFrameTime		= CLS_GetFrameTime;
	import.CLS_GetFrameCount	= CLS_GetFrameCount;
	import.CLS_GetCurrentNetState	= CLS_GetCurrentNetState;
	
	import.VID_GetWidth		= VID_GetWidth;
	import.VID_GetHeight		= VID_GetHeight;
	
	import.Sys_Milliseconds		= Sys_Milliseconds;


	cge = (cg_export_t *)Sys_GetAPI("cgame", "GetCGameAPI", &import, &cge_handle);

	if(!cge)
		Com_Error(ERR_DROP, "failed to load cgame DLL");
	
	if(cge->apiversion != CG_API_VERSION)
		Com_Error(ERR_DROP, "cgame is version %i, not %i", cge->apiversion, CG_API_VERSION);

	cge->Init();
}

void	CL_ShutdownClientGame()
{
	if(!cge)
		return;
	
	cge->Shutdown();
	
	Sys_UnloadAPI(&cge_handle);
}


