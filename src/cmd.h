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
#ifndef CMD_H
#define CMD_H

/// includes ===================================================================
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
// system -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
#include "common.h"




//void 	Cbuf_Init();
// allocates an initial text buffer that will grow as needed

void 	Cbuf_AddText(const std::string &text);
// as new commands are generated from the console or keybindings,
// the text is added to the end of the command buffer.

void 	Cbuf_InsertText(const std::string &text);
// when a command wants to issue other commands immediately, the text is
// inserted at the beginning of the buffer, before any remaining unexecuted
// commands.

void 	Cbuf_ExecuteText(exec_type_e, const std::string &text);
// this can be used in place of either Cbuf_AddText or Cbuf_InsertText

void 	Cbuf_AddEarlyCommands(bool clear);
// adds all the +set commands from the command line

bool 	Cbuf_AddLateCommands();
// adds all the remaining + commands from the command line
// Returns true if any late commands were added, which
// will keep the demoloop from immediately starting

void 	Cbuf_Execute();
// Pulls off \n terminated lines of text from the command buffer and sends
// them through Cmd_ExecuteString.  Stops when the buffer is empty.
// Normally called once per frame, but may be explicitly invoked.
// Do not call inside a command function!

void 	Cbuf_CopyToDefer();
void 	Cbuf_InsertFromDefer();
// These two functions are used to defer any pending commands while a map
// is being loaded


/*

Command execution takes a null terminated string, breaks it into tokens,
then searches for a command or variable that matches the first token.

*/


void	Cmd_Init();

void	Cmd_AddCommand(const std::string &name, void (*)());
// called by the init functions of other parts of the program to
// register commands and functions to call for them.
// The cmd_name is referenced later, so it should not be in temp memory
// if function is NULL, the command will be forwarded to the server
// as a clc_stringcmd instead of executed locally
void	Cmd_RemoveCommand(const std::string &name);

bool	Cmd_Exists(const std::string &name);
// used by the cvar code to check for cvar / command name overlap

const char*	Cmd_CompleteCommand(const std::string &partial);
// attempts to match a partial command for automatic command line completion
// returns NULL if nothing fits

int		Cmd_Argc();
const char*	Cmd_Argv(int arg);

inline int	Cmd_ArgvInt(int arg)
{
	return atoi(Cmd_Argv(arg));
}

inline float	Cmd_ArgvFloat(int arg)
{
	return atof(Cmd_Argv(arg));
}

const char*	Cmd_Args();
// The functions that execute commands get their parameters with these
// functions. Cmd_Argv () will return an empty string, not a NULL
// if arg > argc, so string operations are always safe.

void  	Cmd_TokenizeString(const std::string &text);
// Takes a null terminated string.  Does not need to be /n terminated.
// breaks the string up into arg tokens.

void	Cmd_ExecuteString(const std::string &text);
// Parses a single line of text into arguments and tries to execute it
// as if it was typed at the console

void	Cmd_ForwardToServer();
// adds the current command line as a clc_stringcmd to the client message.
// things like godmode, noclip, etc, are commands directed to the server,
// so when they are typed in at the console, they will need to be forwarded.



#endif // CMD_H

