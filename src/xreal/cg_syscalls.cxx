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
#include "cg_public.h"
#include "cg_local.h"

cg_import_t	cgi;

void	trap_Com_Printf(const char *fmt, ...)
{
	char		msg[MAX_STRING_CHARS];
	va_list		argptr;
	
	va_start(argptr, fmt);
	vsprintf(msg, fmt, argptr);
	va_end(argptr);

	cgi.Com_Printf("%s", msg);
}

void	trap_Com_DPrintf(const char *fmt, ...)
{
	char		msg[MAX_STRING_CHARS];
	va_list		argptr;
	
	va_start(argptr, fmt);
	vsprintf(msg, fmt, argptr);
	va_end(argptr);
	
	cgi.Com_DPrintf("%s", msg);
}

void	trap_Com_Error(err_type_e type, const char *fmt, ...)
{
	char		msg[MAX_STRING_CHARS];
	va_list		argptr;
	
	va_start(argptr,fmt);
	vsprintf(msg, fmt, argptr);
	va_end(argptr);
	
	cgi.Com_Error(type, "%s", msg);
}

void		trap_Cmd_AddCommand(const std::string &name, void(*cmd)())
{
	cgi.Cmd_AddCommand(name, cmd);
}

void		trap_Cmd_RemoveCommand(const std::string &name)
{
	cgi.Cmd_RemoveCommand(name);
}

int		trap_Cmd_Argc()
{
	return cgi.Cmd_Argc();
}

const char*	trap_Cmd_Argv(int i)
{
	return cgi.Cmd_Argv(i);
}

cvar_t*		trap_Cvar_Get(const std::string &name, const std::string &value, uint_t flags)
{
	return cgi.Cvar_Get(name, value, flags);
}

cvar_t*		trap_Cvar_Set(const std::string &name, const std::string &value)
{
	return cgi.Cvar_Set(name, value);
}

void		trap_Cvar_SetValue(const std::string &name, float value)
{
	cgi.Cvar_SetValue(name, value);
}

float		trap_Cvar_VariableValue(const std::string &name)
{
	return cgi.Cvar_VariableValue(name);
}

int		trap_Cvar_VariableInteger(const std::string &name)
{
	return cgi.Cvar_VariableInteger(name);
}


void		trap_R_BeginRegistration(const std::string &map)
{
	cgi.R_BeginRegistration(map);
}

int		trap_R_RegisterModel(const std::string &name)
{
	return cgi.R_RegisterModel(name);
}

int		trap_R_RegisterAnimation(const std::string &name)
{
	return cgi.R_RegisterAnimation(name);
}

int		trap_R_RegisterSkin(const std::string &name)
{
	return cgi.R_RegisterSkin(name);
}

int		trap_R_RegisterShader(const std::string &name)
{
	return cgi.R_RegisterShader(name);
}

int		trap_R_RegisterPic(const std::string &name)
{
	return cgi.R_RegisterPic(name);
}

int		trap_R_RegisterParticle(const std::string &name)
{
	return cgi.R_RegisterParticle(name);
}

int		trap_R_RegisterLight(const std::string &name)
{
	return cgi.R_RegisterLight(name);
}

void		trap_R_EndRegistration()
{
	cgi.R_EndRegistration();
}


void		trap_R_BeginFrame()
{
	cgi.R_BeginFrame();
}

void		trap_R_RenderFrame(const r_refdef_t &fd)
{
	cgi.R_RenderFrame(fd);
}

void		trap_R_EndFrame()
{
	cgi.R_EndFrame();
}
	

void		trap_R_DrawPic(int x, int y, int w, int h, const vec4_c &color, int shader)
{
	cgi.R_DrawPic(x, y, w, h, color, shader);
}

void		trap_R_DrawStretchPic(int x, int y, int w, int h, float s1, float t1, float s2, float t2, const vec4_c &color, int shader)
{
	cgi.R_DrawStretchPic(x, y, w, h, s1, t1, s2, t2, color, shader);
}

void		trap_R_DrawFill(int x, int y, int w, int h, const vec4_c &color)
{
	cgi.R_DrawFill(x, y, w, h, color);
}

	
void		trap_R_ClearScene()
{
	cgi.R_ClearScene();
}

	
void		trap_R_AddDeltaEntity(int index, const r_entity_t &shared)
{
	cgi.R_AddDeltaEntity(index, shared);
}

void		trap_R_UpdateDeltaEntity(int index, const r_entity_t &shared)
{
	cgi.R_UpdateDeltaEntity(index, shared);
}

void		trap_R_RemoveDeltaEntity(int index)
{
	cgi.R_RemoveDeltaEntity(index);
}


void		trap_R_AddDeltaLight(int index, const r_light_t &shared)
{
	cgi.R_AddDeltaLight(index, shared);
}

void		trap_R_UpdateDeltaLight(int index, const r_light_t &shared)
{
	cgi.R_UpdateDeltaLight(index, shared);
}

void		trap_R_RemoveDeltaLight(int index)
{
	cgi.R_RemoveDeltaLight(index);
}


void		trap_R_AddParticle(const r_particle_t &part)
{
	cgi.R_AddParticle(part);
}

void		trap_R_AddPoly(const r_poly_t &poly)
{
	cgi.R_AddPoly(poly);
}

void		trap_R_AddContact(const r_contact_t &contact)
{
	cgi.R_AddContact(contact);
}

bool		trap_R_SetupTag(r_tag_t &tag, const r_entity_t &ent, const std::string &name)
{
	return cgi.R_SetupTag(tag, ent, name);
}

bool		trap_R_SetupAnimation(int model, int anim)
{
	return 	cgi.R_SetupAnimation(model, anim);
}

	// virtual fileystem access
int		trap_VFS_FOpenRead(const std::string &filename, VFILE **stream)
{
	return cgi.VFS_FOpenRead(filename, stream);
}

int		trap_VFS_FOpenWrite(const std::string &filename, VFILE **stream)
{
	return cgi.VFS_FOpenWrite(filename, stream);
}

void		trap_VFS_FClose(VFILE **stream)
{
	cgi.VFS_FClose(stream);
}

	
int		trap_VFS_FLoad(const std::string &name, void **buf)
{
	return cgi.VFS_FLoad(name, buf);
}

void		trap_VFS_FSave(const std::string &path, void *buffer, int len)
{
	cgi.VFS_FSave(path, buffer, len);
}

void		trap_VFS_FFree(void *buf)
{
	cgi.VFS_FFree(buf);
}

int		trap_VFS_FRead(void *buffer, int len, VFILE *stream)
{
	return cgi.VFS_FRead(buffer, len, stream);
}

int		trap_VFS_FWrite(const void *buffer, int len, VFILE *stream)
{
	return cgi.VFS_FWrite(buffer, len, stream);
}
	
		
char*		trap_Key_KeynumToString(int keynum)
{
	return cgi.Key_KeynumToString(keynum);
}

char*		trap_Key_GetBinding(int keynum)
{
	return cgi.Key_GetBinding(keynum);
}
//void 		(*Key_SetBinding)(int keynum, char *binding);
//void 		(*Key_ClearStates)(void);
//keydest_t	(*Key_GetKeyDest)(void);
//void		(*Key_SetKeyDest)( keydest_t key_dest );
	
void		trap_Con_ClearNotify()
{
	cgi.Con_ClearNotify();
}

	
void		trap_S_Init()
{
	cgi.S_Init();
}

void		trap_S_Shutdown()
{
	cgi.S_Shutdown();
}
	
void		trap_S_StartSound(const vec3_c &origin, int ent_num, int ent_channel, int sound)
{
	cgi.S_StartSound(origin, ent_num, ent_channel, sound);
}

void		trap_S_StartLoopSound(const vec3_c &origin, const vec3_c &velocity, int entity_num, int entity_channel, int sound)
{
	cgi.S_StartLoopSound(origin, velocity, entity_num, entity_channel, sound);
}

void		trap_S_UpdateLoopSound(const vec3_c &origin, const vec3_c &velocity, int entity_num, int entity_channel, int sound)
{
	cgi.S_UpdateLoopSound(origin, velocity, entity_num, entity_channel, sound);
}

void		trap_S_StopLoopSound(int entity_num)
{
	cgi.S_StopLoopSound(entity_num);
}

void		trap_S_StopAllSounds()
{
	cgi.S_StopAllSounds();
}

void		trap_S_Update(const vec3_c &origin, const vec3_c &velocity, const vec3_c &v_forward, const vec3_c &v_right, const vec3_c &v_up)
{
	cgi.S_Update(origin, velocity, v_forward, v_right, v_up);
}
	
void		trap_S_BeginRegistration()
{
	cgi.S_BeginRegistration();
}

int		trap_S_RegisterSound(const std::string &name)
{
	return cgi.S_RegisterSound(name);
}

void		trap_S_EndRegistration()
{
	cgi.S_EndRegistration();
}


cmodel_c*	trap_CM_BeginRegistration(const std::string &name, bool clientload, unsigned *checksum)
{
	return cgi.CM_BeginRegistration(name, clientload, checksum);
}

cmodel_c*	trap_CM_RegisterModel(const std::string &name)
{
	return cgi.CM_RegisterModel(name);
}
void		trap_CM_EndRegistration()
{
	cgi.CM_EndRegistration();
}

int		trap_CL_GetTime()
{
	return cgi.CL_GetTime();
}

void		trap_CL_SetTime(int time)
{
	cgi.CL_SetTime(time);
}

const char*	trap_CL_GetConfigString(int index)
{
	return cgi.CL_GetConfigString(index);
}

void		trap_CL_GetUserCommand(int frame, usercmd_t &cmd)
{
	cgi.CL_GetUserCommand(frame, cmd);
}

void		trap_CL_GetCurrentUserCommand(usercmd_t &cmd)
{
	cgi.CL_GetCurrentUserCommand(cmd);
}

bool		trap_CL_GetRefreshPrepped()
{
	return cgi.CL_GetRefreshPrepped();
}

void		trap_CL_SetRefreshPrepped(bool val)
{
	cgi.CL_SetRefreshPrepped(val);
}

bool		trap_CL_GetForceRefdef()
{
	return cgi.CL_GetForceRefdef();
}

void		trap_CL_SetForceRefdef(bool val)
{
	cgi.CL_SetForceRefdef(val);
}

int		trap_CL_GetPlayerNum()
{
	return cgi.CL_GetPlayerNum();
}


connection_state_t	trap_CLS_GetConnectionState()
{
	return cgi.CLS_GetConnectionState();
}

float		trap_CLS_GetRealTime()
{
	return cgi.CLS_GetRealTime();
}

float		trap_CLS_GetFrameTime()
{
	return cgi.CLS_GetFrameTime();
}

int		trap_CLS_GetFrameCount()
{
	return cgi.CLS_GetFrameCount();
}

void		trap_CLS_GetCurrentNetState(int &incoming_acknowledged, int &outgoing_sequence)
{
	cgi.CLS_GetCurrentNetState(incoming_acknowledged, outgoing_sequence);
}


uint_t		trap_VID_GetWidth()
{
	return cgi.VID_GetWidth();
}

uint_t		trap_VID_GetHeight()
{
	return cgi.VID_GetHeight();
}


int		trap_Sys_Milliseconds()
{
	return cgi.Sys_Milliseconds();
}

