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
// shared -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
// xreal --------------------------------------------------------------------
#include "g_public.h"
#include "g_local.h"

extern game_import_t	gi;

void	trap_Com_Printf(const char *fmt, ...)
{
	char		msg[MAX_STRING_CHARS];
	va_list		argptr;
	
	va_start(argptr, fmt);
	vsprintf(msg, fmt, argptr);
	va_end(argptr);

	gi.Com_Printf("%s", msg);
}

void	trap_Com_DPrintf(const char *fmt, ...)
{
	char		msg[MAX_STRING_CHARS];
	va_list		argptr;
	
	va_start(argptr, fmt);
	vsprintf(msg, fmt, argptr);
	va_end(argptr);
	
	gi.Com_DPrintf("%s", msg);
}

void	trap_Com_Error(err_type_e type, const char *fmt, ...)
{
	char		msg[MAX_STRING_CHARS];
	va_list		argptr;
	
	va_start(argptr,fmt);
	vsprintf(msg, fmt, argptr);
	va_end(argptr);
	
	gi.Com_Error(type, "%s", msg);
}


void	trap_Cbuf_AddText(const std::string &text)
{
	gi.Cbuf_AddText(text);
}


void	trap_Cmd_AddCommand(const std::string &name, void(*cmd)())
{
	gi.Cmd_AddCommand(name, cmd);
}

void	trap_Cmd_RemoveCommand(const std::string &name)
{
	gi.Cmd_RemoveCommand(name);
}

int	trap_Cmd_Argc()
{
	return gi.Cmd_Argc();
}

const char*	trap_Cmd_Argv(int i)
{
	return gi.Cmd_Argv(i);
}

const char*	trap_Cmd_Args()
{
	return gi.Cmd_Args();
}


cvar_t*	trap_Cvar_Get(const std::string &name, const std::string &value, uint_t flags)
{
	return gi.Cvar_Get(name, value, flags);
}

cvar_t*	trap_Cvar_Set(const std::string &name, const std::string &value)
{
	return gi.Cvar_Set(name, value);
}

cvar_t*	trap_Cvar_ForceSet(const std::string &name, const std::string &value)
{
	return gi.Cvar_ForceSet(name, value);
}

void	trap_Cvar_SetValue(const std::string &name, float value)
{
	gi.Cvar_SetValue(name, value);
}


void	trap_SV_BPrintf(g_print_level_e level, const char *fmt, ...)
{
	char		msg[MAX_STRING_CHARS];
	va_list		argptr;
	
	va_start(argptr, fmt);
	vsprintf(msg, fmt, argptr);
	va_end(argptr);
	
	gi.SV_BPrintf(level, "%s", msg);
}

void	trap_SV_CPrintf(sv_entity_c *ent, g_print_level_e level, const char *fmt, ...)
{
	char		msg[MAX_STRING_CHARS];
	va_list		argptr;
	
	va_start(argptr, fmt);
	vsprintf(msg, fmt, argptr);
	va_end(argptr);
	
	gi.SV_CPrintf(ent, level, "%s", msg);
}

void	trap_SV_CenterPrintf(sv_entity_c *ent, const char *fmt, ...)
{
	char		msg[MAX_STRING_CHARS];
	va_list		argptr;
	
	va_start(argptr, fmt);
	vsprintf(msg, fmt, argptr);
	va_end(argptr);
	
	gi.SV_CenterPrintf(ent, "%s", msg);
}

void	trap_SV_StartSound(vec3_t origin, sv_entity_c *ent, int channel, int soundindex, float volume, float attenuation, float timeofs)
{
	gi.SV_StartSound(origin, ent, channel, soundindex, volume, attenuation, timeofs);
}

// config strings hold all the index strings, the lightstyles,
// and misc data like the sky definition and cdtrack.
// All of the current configstrings are sent to clients when
// they connect, and changes are sent to all connected clients.
void	trap_SV_SetConfigString(int index, const std::string &value)
{
	gi.SV_SetConfigString(index, value);
}

// the *index functions create configstrings and some internal server state
int	trap_SV_ModelIndex(const std::string &name)
{
	return gi.SV_ModelIndex(name);
}

int	trap_SV_ShaderIndex(const std::string &name)
{
	return gi.SV_ShaderIndex(name);
}

int	trap_SV_AnimationIndex(const std::string &name)
{
	return gi.SV_AnimationIndex(name);
}

int	trap_SV_SoundIndex(const std::string &name)
{
	return gi.SV_SoundIndex(name);
}

int	trap_SV_LightIndex(const std::string &name)
{
	return gi.SV_LightIndex(name);
}

// collision detection
d_bsp_c*	trap_CM_BeginRegistration(const std::string &name, bool clientload, unsigned *checksum, dSpaceID space)
{
	return gi.CM_BeginRegistration(name, clientload, checksum, space);
}

cmodel_c*	trap_CM_RegisterModel(const std::string &name)
{
	return gi.CM_RegisterModel(name);
}

cskel_animation_c*	trap_CM_RegisterAnimation(const std::string &name)
{
	return gi.CM_RegisterAnimation(name);
}

cmodel_c*	trap_CM_GetModelByNum(int num)
{
	return gi.CM_GetModelByNum(num);
}

int	trap_CM_LeafContents(int leafnum)
{
	return gi.CM_LeafContents(leafnum);
}

int	trap_CM_LeafCluster(int leafnum)
{
	return gi.CM_LeafCluster(leafnum);
}

int	trap_CM_LeafArea(int leafnum)
{
	return gi.CM_LeafArea(leafnum);
}

int	trap_CM_NumModels()
{
	return gi.CM_NumModels();
}

int	trap_CM_HeadnodeForBox(const cbbox_c& bbox)
{
	return gi.CM_HeadnodeForBox(bbox);
}

int	trap_CM_PointContents(const vec3_c &p, int headnode)
{
	return gi.CM_PointContents(p, headnode);
}

int	trap_CM_TransformedPointContents(const vec3_c &p, int headnode, const vec3_c &origin, const quaternion_c &quat)
{
	return gi.CM_TransformedPointContents(p, headnode, origin, quat);
}

trace_t	trap_CM_BoxTrace(const vec3_c &start, const vec3_c &end, const cbbox_c &bbox, int headnode, int brushmask)
{
	return gi.CM_BoxTrace(start, end, bbox, headnode, brushmask);
}

trace_t	trap_CM_TransformedBoxTrace(const vec3_c &start, const vec3_c &end,
					const cbbox_c &bbox,
					int headnode, int brushmask, 
					const vec3_c &origin, const quaternion_c &quat)
{
	return gi.CM_TransformedBoxTrace(start, end, bbox, headnode, brushmask, origin, quat);
}

int	trap_CM_PointLeafnum(const vec3_c &p)
{
	return gi.CM_PointLeafnum(p);
}

int	trap_CM_PointAreanum(const vec3_c &p)
{
	return gi.CM_PointAreanum(p);
}

int	trap_CM_BoxLeafnums(const cbbox_c &bbox, std::deque<int> &list, int headnode)
{
	return gi.CM_BoxLeafnums(bbox, list, headnode);
}

int	trap_CM_GetClosestAreaPortal(const vec3_c &p)
{
	return gi.CM_GetClosestAreaPortal(p);
}

bool	trap_CM_GetAreaPortalState(int portal)
{
	return gi.CM_GetAreaPortalState(portal);
}

void	trap_CM_SetAreaPortalState(int portal, bool open)
{
	gi.CM_SetAreaPortalState(portal, open);
}

bool	trap_CM_AreasConnected(int area1, int area2)
{
	return gi.CM_AreasConnected(area1, area2);
}


void	trap_SV_Multicast(const vec3_c &origin, multicast_type_e to)
{
	gi.SV_Multicast(origin, to);
}

void	trap_SV_Unicast(sv_entity_c *ent, bool reliable)
{
	gi.SV_Unicast(ent, reliable);
}

void	trap_SV_WriteBit(bool bit)
{
	gi.SV_WriteBit(bit);
}

void	trap_SV_WriteBits(int bits, int bits_num)
{
	gi.SV_WriteBits(bits, bits_num);
}

void	trap_SV_WriteByte(int c)
{
	gi.SV_WriteByte(c);
}

void	trap_SV_WriteShort(int c)
{
	gi.SV_WriteShort(c);
}

void	trap_SV_WriteLong(int c)
{
	gi.SV_WriteLong(c);
}

void	trap_SV_WriteFloat(float f)
{
	gi.SV_WriteFloat(f);
}

void	trap_SV_WriteString(const char *s)
{
	gi.SV_WriteString(s);
}

void	trap_SV_WritePosition(const vec3_c &pos)
{
	gi.SV_WritePosition(pos);
}

void	trap_SV_WriteDir(const vec3_c &dir)
{
	gi.SV_WriteDir(dir);
}

void	trap_SV_WriteAngle(float f)
{
	gi.SV_WriteAngle(f);
}

void	trap_SV_WriteColor(vec4_t color)
{
	gi.SV_WriteColor(color);
}

// virtual fileystem access
int	trap_VFS_FOpenRead(const std::string &filename, VFILE **stream)
{
	return gi.VFS_FOpenRead(filename, stream);
}

int	trap_VFS_FOpenWrite(const std::string &filename, VFILE **stream)
{
	return gi.VFS_FOpenWrite(filename, stream);
}

void	trap_VFS_FClose(VFILE **stream)
{
	gi.VFS_FClose(stream);
}

int	trap_VFS_FLoad(const std::string &name, void **buf)
{
	return gi.VFS_FLoad(name, buf);
}

void	trap_VFS_FSave(const std::string &path, void *buffer, int len)
{
	gi.VFS_FSave(path, buffer, len);
}

void	trap_VFS_FFree(void *buf)
{
	gi.VFS_FFree(buf);
}

int	trap_VFS_FRead(void *buffer, int len, VFILE *stream)
{
	return gi.VFS_FRead(buffer, len, stream);
}

int	trap_VFS_FWrite(const void *buffer, int len, VFILE *stream)
{
	return gi.VFS_FWrite(buffer, len, stream);
}

