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
#ifndef CVAR_H
#define CVAR_H

/// includes ===================================================================
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
// system -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
// xreal --------------------------------------------------------------------
// shared -------------------------------------------------------------------
#include "x_shared.h"

/*
cvar_t variables are used to hold scalar or string variables that can be changed or displayed at the console or prog code as well as accessed directly
in C code.

The user can access cvars from the console in three ways:
r_draworder			prints the current value
r_draworder 0		sets the current value to 0
set r_draworder 0	as above, but creates the cvar if not present
Cvars are restricted from having the same names as commands to keep this
interface from being ambiguous.
*/


extern bool			cvar_userinfo_modified;
// this is set each time a CVAR_USERINFO variable is changed
// so that the client knows to send it to the server


cvar_t*	Cvar_Get(const std::string &name, const std::string &value, uint_t flags);
// creates the variable if it doesn't exist, or returns the existing one
// if it exists, the value will not be changed, but flags will be ORed in
// that allows variables to be unarchived without needing bitflags

cvar_t*	Cvar_Set(const std::string &name, const std::string &value);
// will create the variable if it doesn't exist

cvar_t*	Cvar_Set2(const std::string &name, const std::string &value, uint_t flags, bool force);

cvar_t*	Cvar_ForceSet(const std::string &name, const std::string &value);
// will set the variable even if NOSET or LATCH

void	Cvar_SetValue(const std::string &name, float value);
// expands value to a string and calls Cvar_Set

void	Cvar_SetInteger(const std::string &name, int integer);

void	Cvar_SetModified(const std::string &name);

float	Cvar_VariableValue(const std::string &name);
// returns 0 if not defined or non numeric

int	Cvar_VariableInteger(const std::string &name);
// returns 0 if not defined or non numeric

const char*	Cvar_VariableString(const std::string &name);
// returns an empty string if not defined

const char*	Cvar_CompleteVariable(const std::string &partial);
// attempts to match a partial variable name for command line completion
// returns NULL if nothing fits

float	Cvar_ClampVariable(const std::string &name, float min, float max);


void	Cvar_SetLatchedVars();
// any CVAR_LATCHED variables that have been set will now take effect

bool 	Cvar_Command();
// called by Cmd_ExecuteString when Cmd_Argv(0) doesn't match a known
// command.  Returns true if the command was a variable reference that
// was handled. (print or change)

void 	Cvar_WriteVars(VFILE *stream);
// appends lines containing "set variable value" for all variables
// with the archive flag set to true.

void 	Cvar_WriteLatchedVars(VFILE *stream);
// write all CVAR_LATCH cvars
// these will be things like coop, skill, deathmatch, etc

void	Cvar_Init();

const char*	Cvar_Userinfo();
// returns an info string containing all the CVAR_USERINFO cvars

const char*	Cvar_Serverinfo();
// returns an info string containing all the CVAR_SERVERINFO cvars




#endif // CVAR_H
