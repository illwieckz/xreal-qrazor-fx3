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
#ifndef GLIMP_WGL_H
#define GLIMP_WGL_H

/// includes ===================================================================
// system -------------------------------------------------------------------
#include <stdio.h>
// qrazor-fx ----------------------------------------------------------------

#ifndef _WIN32
#  error You should not be including this file on this platform
#endif


typedef struct
{
	HINSTANCE	hInstance;
	void*		wndproc;

	HDC     	hDC;			// handle to device context
	HWND    	hWnd;			// handle to window
	HGLRC   	hGLRC;			// handle to GL rendering context

//	HINSTANCE	hinstOpenGL;	// HINSTANCE for the OpenGL library
	HMODULE		OpenGLLib;

	bool		allowdisplaydepthchange;
	bool		mcd_accelerated;
} sys_gl_t;

extern sys_gl_t sys_gl;

#endif // GLIMP_WGL_H
