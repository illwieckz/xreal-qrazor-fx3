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
#include "sys.h"

// xreal --------------------------------------------------------------------
#include "xreal/ui_public.h"


ui_export_t*	uie = NULL;
static void*	uie_handle = NULL;

static float	CL_GetRealtime()
{
	return cls.realtime;
}

static bool	CL_GetAttractloop()
{
	return cl.attractloop;
}

static void	CL_SetServercount(int count)
{
	cl.servercount = count;
}

void	CL_InitUserInterface()
{
	ui_import_t	import;

	Com_Printf("------- UI_Init -------\n");
	
	// unload anything we have now
	if(uie)
		CL_ShutdownUserInterface();

	import.Com_Printf		= Com_Printf;
	import.Com_DPrintf		= Com_DPrintf;
	import.Com_Error		= Com_Error;
	import.Com_ServerState		= Com_ServerState;
	
	import.Cbuf_AddText		= Cbuf_AddText;
	import.Cbuf_InsertText		= Cbuf_InsertText;
	import.Cbuf_ExecuteText		= Cbuf_ExecuteText;
	import.Cbuf_Execute		= Cbuf_Execute;
	
	import.Cmd_AddCommand		= Cmd_AddCommand;
	import.Cmd_RemoveCommand	= Cmd_RemoveCommand;
	import.Cmd_Argc			= Cmd_Argc;
	import.Cmd_Argv			= Cmd_Argv;
	
	import.VFS_Gamedir		= VFS_Gamedir;
	import.VFS_ListFiles		= VFS_ListFiles;
	
	import.VFS_FLoad		= VFS_FLoad;
	import.VFS_FFree		= VFS_FFree;

	import.Cvar_Get			= Cvar_Get;
	import.Cvar_Set			= Cvar_Set;
	import.Cvar_ForceSet		= Cvar_ForceSet;
	import.Cvar_SetValue		= Cvar_SetValue;
	import.Cvar_SetModified		= Cvar_SetModified;
	import.Cvar_VariableValue	= Cvar_VariableValue;
	import.Cvar_VariableInteger	= Cvar_VariableInteger;
	import.Cvar_VariableString	= Cvar_VariableString;
	import.Cvar_ClampVariable	= Cvar_ClampVariable;
	
//	import.R_BeginRegistration	= re.R_BeginRegistration;
//	import.R_RegisterModel		= re.R_RegisterModel;
//	import.R_RegisterAnimation	= re.R_RegisterAnimation;
//	import.R_RegisterSkin		= re.R_RegisterSkin;
	import.R_RegisterPic		= re.R_RegisterPic;
//	import.R_RegisterLight		= re.R_RegisterLight;
//	import.R_EndRegistration	= re.R_EndRegistration;
	
//	import.R_BeginFrame		= re.R_BeginFrame;
//	import.R_RenderFrame		= re.R_RenderFrame;
//	import.R_EndFrame		= re.R_EndFrame;

	import.R_DrawPic		= re.R_DrawPic;
	import.R_DrawStretchPic		= re.R_DrawStretchPic;
	import.R_DrawFill		= re.R_DrawFill;
	
//	import.R_ClearScene		= re.R_ClearScene;
	
//	import.R_AddEntity		= re.R_AddEntity;	
	
//	import.R_AddLight		= re.R_AddLight;
//	import.R_UpdateLight		= re.R_UpdateLight;
//	import.R_RemoveLight		= re.R_RemoveLight;
	
//	import.R_AddParticle		= re.R_AddParticle;
//	import.R_AddPoly		= re.R_AddPoly;
	
//	import.R_SetupTag		= re.R_SetupTag;
//	import.R_SetupAnimation		= re.R_SetupAnimation;
	
	import.Key_KeynumToString	= Key_KeynumToString;
	import.Key_GetBinding		= Key_GetBinding;
	import.Key_SetBinding		= Key_SetBinding;
	import.Key_ClearStates		= Key_ClearStates;
	import.Key_GetKeyDest		= Key_GetKeyDest;
	import.Key_SetKeyDest		= Key_SetKeyDest;
	
	import.Con_ClearNotify		= Con_ClearNotify;
	import.Con_ClearTyping		= Con_ClearTyping;

	import.CL_GetRealtime		= CL_GetRealtime;
	import.CL_GetAttractloop	= CL_GetAttractloop;
	import.CL_Quit_f		= CL_Quit_f;
	import.CL_PingServers_f		= CL_PingServers_f;
	import.CL_SetServercount	= CL_SetServercount;
	
	import.S_StartLocalSound	= S_StartLocalSound;
	
	import.VID_GetWidth		= VID_GetWidth;
	import.VID_GetHeight		= VID_GetHeight;
		
	import.Sys_AdrToString		= Sys_AdrToString;
	import.Sys_Milliseconds		= Sys_Milliseconds;

	
	uie = (ui_export_t *)Sys_GetAPI("ui", "GetUIAPI", &import, &uie_handle);

	if(!uie)
		Com_Error(ERR_DROP, "failed to load ui DLL");
	
	if(uie->apiversion != UI_API_VERSION)
		Com_Error(ERR_DROP, "ui is version %i, not %i", uie->apiversion, UI_API_VERSION);

	uie->Init();
}

void	CL_ShutdownUserInterface()
{
	if(!uie)
		return;
	
	uie->Shutdown();
	
	Sys_UnloadAPI(&uie_handle);
}
