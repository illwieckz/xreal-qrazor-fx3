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
#ifndef BG_LOCAL_H
#define BG_LOCAL_H

/// includes ===================================================================
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
// system -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
#include "../x_shared.h"

// xreal --------------------------------------------------------------------

enum
{
	
	MINS_Z			= -24,
	VIEWHEIGHT_DEFAULT	= 36,
	VIEWHEIGHT_CROUCH	= 6,
	VIEWHEIGHT_DEAD		= -12
};

//
// console variables
//
extern cvar_t*	bg_maxvelocity;
extern cvar_t*	bg_gravity;


//
// trap functions to provide engine and game functionality
//

// common printing and error handling
void 		trap_Com_Printf(const char *fmt, ...);
void 		trap_Com_DPrintf(const char *fmt, ...);

void 		trap_Com_Error(err_type_e type, const char *fmt, ...);

// console variable interaction
cvar_t*		trap_Cvar_Get(const std::string &name, const std::string &value, uint_t flags);
cvar_t*		trap_Cvar_Set(const std::string &name, const std::string &value);



#endif // BG_LOCAL_H
