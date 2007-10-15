/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006 Robert Beckebans <trebor_7@users.sourceforge.net>

This file is part of XreaL source code.

XreaL source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

XreaL source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XreaL source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

// win_local.h: Win32-specific XreaL header file
#include <windows.h>

// raynorpat: use directinput 8
#define DIRECTINPUT_VERSION 0x0800

#include <dinput.h>
#include <dsound.h>
#include <winsock.h>

void            Sys_QueEvent(int time, sysEventType_t type, int value, int value2, int ptrLength, void *ptr);

void            Sys_CreateConsole(void);
void            Sys_DestroyConsole(void);

char           *Sys_ConsoleInput(void);

qboolean        Sys_GetPacket(netadr_t * net_from, msg_t * net_message);

// Input subsystem
void            IN_Init(void);
void            IN_Shutdown(void);
void            IN_JoystickCommands(void);

void            IN_Move(usercmd_t *cmd);

void            IN_MouseEvent(int mstate);

// add additional non keyboard / non mouse movement on top of the keyboard move cmd
void            IN_DeactivateWin32Mouse(void);

void            IN_Activate(qboolean active);
void            IN_Frame(void);

// window procedure
LONG WINAPI     MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void            Conbuf_AppendText(const char *msg);

void            SNDDMA_Activate(void);
int             SNDDMA_InitDS(void);

void            QGL_EnableLogging(qboolean enable);
qboolean        QGL_Init(const char *dllname);
void            QGL_Shutdown(void);

typedef struct
{

	HINSTANCE       reflib_library;	// Handle to refresh DLL 
	qboolean        reflib_active;

	HWND            hWnd;
	HINSTANCE       hInstance;
	qboolean        activeApp;
	qboolean        isMinimized;
	OSVERSIONINFO   osversion;

	// when we get a windows message, we store the time off so keyboard processing
	// can know the exact time of an event
	unsigned        sysMsgTime;
} WinVars_t;

extern WinVars_t g_wv;
