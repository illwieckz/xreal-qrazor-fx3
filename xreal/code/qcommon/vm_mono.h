/*
===========================================================================
Copyright (C) 2011 Robert Beckebans <trebor_7@users.sourceforge.net>

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
#ifndef VM_MONO_H
#define VM_MONO_H

#if defined(USE_MONO)

#ifdef USE_LOCAL_HEADERS
#include "../libs/mono/jni.h"
#else
#include <glib.h>
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>
#include <mono/metadata/environment.h>
#include <mono/metadata/mono-config.h>
//#include <mono/utils/mono-publib.h>
#endif


MonoDomain     *mono_domain;
MonoImage      *mono_gameImage;
MonoImage      *mono_gameImage;

void			Mono_Init();
void			Mono_Shutdown();

#define CheckException() CheckException_(__FILE__, __LINE__)
qboolean		CheckException_(char *filename, int linenum);

MonoObject*		Mono_NewVector3f(const vec3_t v);
MonoObject*		Mono_NewAngle3f(float pitch, float yaw, float roll);
MonoObject*     Mono_NewQuat4f(const quat_t q);
MonoObject*		Mono_NewTrajectory(const trajectory_t * t);
MonoObject*		Mono_NewUserCommand(const usercmd_t * ucmd);
//jobjectArray	Mono_NewConsoleArgs();

#endif

#endif // VM_MONO_H
