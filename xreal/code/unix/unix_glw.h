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
#if !( defined __linux__ || defined __FreeBSD__ || defined MACOS_X )
#error You should include this file only on Linux/FreeBSD/Mac OS X platforms
#endif

#ifndef __GLW_LINUX_H__
#define __GLW_LINUX_H__

typedef struct
{
	void           *OpenGLLib;	// instance of OpenGL library

	FILE           *log_fp;
} glwstate_t;

extern glwstate_t glw_state;

#endif
