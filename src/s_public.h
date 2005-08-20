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
#ifndef S_PUBLIC_H
#define S_PUBLIC_H

/// includes ===================================================================
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
// system -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
// xreal --------------------------------------------------------------------
// shared -------------------------------------------------------------------
#include "x_shared.h"


void	S_Init();
void	S_Shutdown();

void	S_StartSound(const vec3_c &origin, int sound);
void	S_StartLocalSound(const std::string &name);

void	S_StartLoopSound(const vec3_c &origin, const vec3_c &velocity, int entity, int sound);
void	S_UpdateLoopSound(const vec3_c &origin, const vec3_c &velocity, int entity, int sound);
void	S_StopLoopSound(int entity);

void	S_StopAllSounds();
void	S_Update(const vec3_c &origin, const vec3_c &velocity, const vec3_c &v_forward, const vec3_c &v_right, const vec3_c &v_up);

void	S_BeginRegistration();
int	S_RegisterSound(const std::string &name);	// returns number of the sound shader
void 	S_EndRegistration();

#endif // S_PUBLIC_H
