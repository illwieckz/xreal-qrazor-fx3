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
#include "common.h"
#include "cmd.h"

// this file can stub out the entire client system
// for pure dedicated servers



void	CL_Init()
{
	//DO NOTHING
}

void	CL_Drop()
{
	//DO NOTHING
}

void	CL_Shutdown()
{
	//DO NOTHING
}

void	CL_Frame(int msec)
{
	//DO NOTHING
}


void	Cmd_ForwardToServer()
{
	const char *cmd;

	cmd = Cmd_Argv(0);
	Com_Printf("Unknown command \"%s\"\n", cmd);
}


static void	Key_Bind_Null_f()
{
	//DO NOTHING
}

static void	Key_Unbindall_Null_f()
{
	//DO NOTHING
}

void	Key_Init()
{
	Cmd_AddCommand("bind",		Key_Bind_Null_f);
	Cmd_AddCommand("unbindall",	Key_Unbindall_Null_f);
}

void	Key_Event(int key, bool down, int time)
{
	//DO NOTHING
}

void	CL_MouseEvent(int dx, int dy)
{
	//DO NOTHING
}

void 	CL_PacketEvent(message_c &msg, const netadr_t &adr)
{
	//DO NOTHING
}

void	Con_Print(const char *text)
{
	//DO NOTHING
}


