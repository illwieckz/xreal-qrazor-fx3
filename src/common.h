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
#ifndef COMMON_H
#define COMMON_H

/// includes ===================================================================
// system -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
// shared -------------------------------------------------------------------
#include "x_shared.h"


extern	cvar_t	*developer;
extern	cvar_t	*dedicated;
extern	cvar_t	*com_speeds;


// host_speeds times
extern	int		time_before_game;
extern	int		time_after_game;
extern	int		time_before_ref;
extern	int		time_after_ref;



void 		Com_Init(int argc, char **argv);
void 		Com_Frame(int msec);
void 		Com_Shutdown();

int		Com_Argc();
const char*	Com_Argv(int arg);	// range and null checked
void		Com_ClearArgv(int arg);
int		Com_CheckParm(const std::string &parm);
void		Com_AddParm(const std::string &parm);

void		Com_InitArgv(int argc, char **argv);

void 		Com_PrintInfo(const char *s);

void		Com_BeginRedirect(char *buffer, int buffer_size, void (*flush)(char*, const netadr_t&), const netadr_t &adr);
void		Com_EndRedirect();

void 		Com_Printf(const char *fmt, ...);
void 		Com_DPrintf(const char *fmt, ...);
void 		Com_Error(err_type_e type, const char *fmt, ...);

void		Com_PushEvent(sys_event_type_e type, int time, int parm1, int parm2, byte *data, uint_t size);

int		Com_ServerState();		// this should have just been a cvar...
void		Com_SetServerState(int state);


void		CL_Init();
void		CL_Shutdown();
void		CL_Frame(int msec);
void		CL_Drop();
void		Key_Init();
void		Key_Event(int key, bool down, int time);
void		CL_MouseEvent(int dx, int dy);
void		CL_PacketEvent(bitmessage_c &msg, const netadr_t &adr);
void		Con_Print(const char *text);

void		SV_Init();
void		SV_Shutdown(char *finalmsg, bool reconnect);
void		SV_Frame(int msec);
void		SV_PacketEvent(bitmessage_c &msg, const netadr_t &adr);

void		Map_Init();
void		Map_Shutdown();
void		Map_Frame(int msec);



#endif	// COMMON_H

