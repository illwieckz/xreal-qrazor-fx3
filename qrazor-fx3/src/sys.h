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
#ifndef SYS_H
#define SYS_H

/// includes ===================================================================
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
// system -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
// shared -------------------------------------------------------------------
#include "x_shared.h"



// directory searching
enum
{
	SFF_ARCH   	= 0x01,
	SFF_HIDDEN	= 0x02,
	SFF_RDONLY	= 0x04,
	SFF_SUBDIR	= 0x08,
	SFF_SYSTEM	= 0x10
};


//
// misc
//
void	Sys_Init();
void	Sys_Shutdown();

void	Sys_AppActivate();

void*	Sys_GetAPI(const char *api_name, const char *api_main, void *api_parms, void **api_handle);
void	Sys_UnloadAPI(void **api_handle);

char*	Sys_ConsoleInput();
void	Sys_ConsoleOutput(char *string);
void	Sys_PushEvents();
void	Sys_Error(const char *error, ...);

#if defined(_WIN32)
// Windows only method to get the last Error. Similar to perror and errno.
std::string Sys_GetError();
#endif

char*	Sys_GetClipboardData();

char*	Sys_ExpandSquiggle(const char *in);

int	Sys_Milliseconds();
int	Sys_Microseconds();
void	Sys_Mkdir(const std::string &path);


// 
// network stuff
//
void	Sys_InitNet(bool server);
void	Sys_ShutdownNet();

void	Sys_SendPacket(const bitmessage_c &msg, const netadr_t &to);
int	Sys_GetPacket(void *data, int length, netadr_t &from);

bool	Sys_CompareAdr(const netadr_t &a, const netadr_t &b);
bool	Sys_CompareBaseAdr(const netadr_t &a, const netadr_t &b);
bool	Sys_IsLocalAddress(const netadr_t &adr);
char*	Sys_AdrToString(const netadr_t &a);
bool	Sys_StringToAdr(const char *s, netadr_t &a);
void	Sys_SleepNet(int msec);


//
// directory searching
//
std::string	Sys_FindFirst(const std::string &path, unsigned musthave, unsigned canthave);
std::string	Sys_FindNext(unsigned musthave, unsigned canthave);
void		Sys_FindClose();




#endif	//SYS_H
