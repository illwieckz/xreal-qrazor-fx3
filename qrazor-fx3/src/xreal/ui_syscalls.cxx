/// ============================================================================
/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2005 Robert Beckebans <trebor_7@users.sourceforge.net>
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
// shared -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
// xreal --------------------------------------------------------------------
#include "ui_public.h"
#include "ui_local.h"

ui_import_t	uii;

void 		trap_Com_Printf(const char *fmt, ...)
{
	char		msg[MAX_STRING_CHARS];
	va_list		argptr;
	
	va_start(argptr, fmt);
	vsprintf(msg, fmt, argptr);
	va_end(argptr);

	uii.Com_Printf("%s", msg);
}
void	trap_Com_DPrintf(const char *fmt, ...)
{
	char		msg[MAX_STRING_CHARS];
	va_list		argptr;
	
	va_start(argptr, fmt);
	vsprintf(msg, fmt, argptr);
	va_end(argptr);
	
	uii.Com_DPrintf("%s", msg);
}

void	trap_Com_Error(err_type_e type, const char *fmt, ...)
{
	char		msg[MAX_STRING_CHARS];
	va_list		argptr;
	
	va_start(argptr,fmt);
	vsprintf(msg, fmt, argptr);
	va_end(argptr);
	
	uii.Com_Error(type, "%s", msg);
}

int		trap_Com_ServerState()
{
	return uii.Com_ServerState();
}

void 		trap_Cbuf_AddText(const std::string &text)
{
	uii.Cbuf_AddText(text);
}

void 		trap_Cbuf_InsertText(const std::string &text)
{
	uii.Cbuf_InsertText(text);
}

void 		trap_Cbuf_ExecuteText(exec_type_e type, const std::string &text)
{
	uii.Cbuf_ExecuteText(type, text); 
}

void 		trap_Cbuf_Execute()
{
	uii.Cbuf_Execute();
}
	
void		trap_Cmd_AddCommand(const std::string &name, void(*cmd)())
{
	uii.Cmd_AddCommand(name, cmd);
}

void		trap_Cmd_RemoveCommand(const std::string &name)
{
	uii.Cmd_RemoveCommand(name);
}

int		trap_Cmd_Argc()
{
	return uii.Cmd_Argc();
}

const char*	trap_Cmd_Argv(int i)
{
	return uii.Cmd_Argv(i);
}

cvar_t*		trap_Cvar_Get(const std::string &name, const std::string &value, uint_t flags)
{
	return uii.Cvar_Get(name, value, flags);
}

cvar_t*		trap_Cvar_Set(const std::string &name, const std::string &value)
{
	return uii.Cvar_Set(name, value);
}

cvar_t*		trap_Cvar_ForceSet(const std::string &name, const std::string &value)
{
	return uii.Cvar_ForceSet(name, value);
}

void		trap_Cvar_SetValue(const std::string &name, float value)
{
	uii.Cvar_SetValue(name, value);
}

void		trap_Cvar_SetModified(const std::string &name)
{
	uii.Cvar_SetModified(name);
}

float		trap_Cvar_VariableValue(const std::string &name)
{
	return uii.Cvar_VariableValue(name);
}

int		trap_Cvar_VariableInteger(const std::string &name)
{
	return uii.Cvar_VariableInteger(name);
}

const char*	trap_Cvar_VariableString(const std::string &name)
{
	return uii.Cvar_VariableString(name);
}

float		trap_Cvar_ClampVariable(const std::string &name, float min, float max)
{
	return uii.Cvar_ClampVariable(name, min, max);
}
	
std::string	trap_VFS_Gamedir()
{
	return uii.VFS_Gamedir();
}

std::vector<std::string>	trap_VFS_ListFiles(const std::string &dir, const std::string &extension)
{
	return uii.VFS_ListFiles(dir, extension);
}
	
int		trap_VFS_FLoad(const std::string &name, void **buf)
{
	return uii.VFS_FLoad(name, buf);
}

void		trap_VFS_FFree(void *buf)
{
	uii.VFS_FFree(buf);
}
		
int		trap_R_RegisterPic(const std::string &name)
{
	return uii.R_RegisterPic(name);
}

void		trap_R_DrawPic(int x, int y, int w, int h, const vec4_c &color, int shader)
{
	return uii.R_DrawPic(x, y, w, h, color, shader);
}

void		trap_R_DrawStretchPic(int x, int y, int w, int h, float s1, float t1, float s2, float t2, const vec4_c &color, int shader)
{
	uii.R_DrawStretchPic(x, y, w, h, s1, t1, s2, t2, color, shader);
}

void		trap_R_DrawFill(int x, int y, int w, int h, const vec4_c &color)
{
	uii.R_DrawFill(x, y, w, h, color);
}
	
char*		trap_Key_KeynumToString(int keynum)
{
	return uii.Key_KeynumToString(keynum);
}

char*		trap_Key_GetBinding(int keynum)
{
	return uii.Key_GetBinding(keynum);
}

void 		trap_Key_SetBinding(int keynum, char *binding)
{
	uii.Key_SetBinding(keynum, binding);
}

void 		trap_Key_ClearStates()
{
	uii.Key_ClearStates();
}

keydest_t	trap_Key_GetKeyDest()
{
	return uii.Key_GetKeyDest();
}

void		trap_Key_SetKeyDest(keydest_t key_dest)
{
	uii.Key_SetKeyDest(key_dest);
}
	
void		trap_Con_ClearNotify()
{
	uii.Con_ClearNotify();
}

void		trap_Con_ClearTyping()
{
	uii.Con_ClearTyping();
}

float		trap_CL_GetRealtime()
{
	return uii.CL_GetRealtime();
}

bool		trap_CL_GetAttractloop()
{
	return uii.CL_GetAttractloop();
}

void 		trap_CL_Quit_f()
{
	uii.CL_Quit_f();
}

void 		trap_CL_PingServers_f()
{
	uii.CL_PingServers_f();
}

void		trap_CL_SetServercount(int count)
{
	uii.CL_SetServercount(count);
}

void		trap_S_StartLocalSound(const std::string &name)
{
	uii.S_StartLocalSound(name);
}

uint_t		trap_VID_GetWidth()
{
	return uii.VID_GetWidth();
}

uint_t		trap_VID_GetHeight()
{
	return uii.VID_GetHeight();
}
	
char*		trap_Sys_AdrToString(const netadr_t &a)
{
	return uii.Sys_AdrToString(a);
}

int		trap_Sys_Milliseconds()
{
	return uii.Sys_Milliseconds();
}
