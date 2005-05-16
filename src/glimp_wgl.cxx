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


/// includes ===================================================================
#include <assert.h>
#include <windows.h>
#include "glimp_wgl.h"
#include "winquake.h"
#include "r_local.h"

// system -------------------------------------------------------------------

/*
** GLW_IMP.C
**
** This file contains ALL Win32 specific stuff having to do with the
** OpenGL refresh.  When a port is being made the following functions
** must be implemented by the port:
**
** GLimp_EndFrame
** GLimp_Init
** GLimp_Shutdown
** GLimp_SwitchFullscreen
**
*/
#define min(x, y) (x < y ? x : y)
#define max(x, y) (x > y ? x : y)

//static bool	GLimp_SwitchFullscreen(int width, int height);
bool	GLimp_InitGL();

sys_gl_t sys_gl;

extern cvar_t*	vid_fullscreen;
extern cvar_t*	vid_ref;

extern BOOL	(*xglMakeCurrent)(HDC, HGLRC);
extern BOOL	(*xglDeleteContext)(HGLRC);
extern HGLRC	(*xglCreateContext)(HDC);

static bool	VerifyDriver()
{
	char buffer[1024];

	strcpy(buffer, (char*)xglGetString(GL_RENDERER));
	strlwr(buffer);
	
	if(strcmp(buffer, "gdi generic") == 0)
		if(!sys_gl.mcd_accelerated)
			return false;
	return true;
}

/*
** VID_CreateWindow
*/
#define	WINDOW_CLASS_NAME	"QRazor_Win"
#define	WINDOW_NAME	        "QRazor FX III"

bool	VID_CreateWindow(int width, int height, bool fullscreen)
{
	WNDCLASS		wc;
	RECT			r;
	cvar_t			*vid_xpos, *vid_ypos;
	int				stylebits;
	int				x, y, w, h;
	int				exstyle;

	
	/* Register the frame class */
	wc.style         = 0;
	wc.lpfnWndProc   = (WNDPROC)sys_gl.wndproc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = sys_gl.hInstance;
	wc.hIcon         = 0;
	wc.hCursor       = LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_GRAYTEXT;
	wc.lpszMenuName  = 0;
	wc.lpszClassName = WINDOW_CLASS_NAME;

	if(!RegisterClass(&wc))
		ri.Com_Error(ERR_FATAL, "Couldn't register window class");

	if(fullscreen)
	{
		exstyle = WS_EX_TOPMOST;
		stylebits = WS_POPUP|WS_VISIBLE;
	}
	else
	{
		exstyle = 0;
		stylebits = WS_CAPTION;
	}

	r.left = 0;
	r.top = 0;
	r.right  = width;
	r.bottom = height;

	AdjustWindowRect(&r, stylebits, FALSE);

	w = r.right - r.left;
	h = r.bottom - r.top;

	if(fullscreen)
	{
		x = 0;
		y = 0;
	}
	else
	{
		vid_xpos = ri.Cvar_Get("vid_xpos", "0", 0);
		vid_ypos = ri.Cvar_Get("vid_ypos", "0", 0);
		x = vid_xpos->getInteger();
		y = vid_ypos->getInteger();
	}

	// Create Window
	sys_gl.hWnd = CreateWindow(WINDOW_CLASS_NAME, WINDOW_NAME, stylebits,
		 x, y, w, h, NULL, NULL, sys_gl.hInstance, NULL);

	if(!sys_gl.hWnd)
		ri.Com_Error(ERR_FATAL, "Couldn't create window");
	
	ShowWindow(sys_gl.hWnd, SW_SHOW);
	UpdateWindow(sys_gl.hWnd);

	// init all the gl stuff for the window
	if(!GLimp_InitGL())
	{
		ri.Com_Printf("VID_CreateWindow() - GLimp_InitGL failed\n");
		return false;
	}

	SetForegroundWindow(sys_gl.hWnd);
	SetFocus(sys_gl.hWnd);

	// let the sound and input subsystems know about the new window
	ri.VID_NewWindow(width, height);

	return true;
}


/*
** GLimp_SetMode
*/
int	GLimp_SetMode(int *pwidth, int *pheight, int mode, bool fullscreen)
{
	int width, height;
	const char *win_fs[] = { "W", "FS" };

	ri.Com_Printf("Initializing OpenGL display\n");

	ri.Com_Printf("...setting mode %d:", mode);

	if(!ri.VID_GetModeInfo(&width, &height, mode))
	{
		ri.Com_Printf("Invalid mode!\n");
		return RSERR_INVALID_MODE;
	}

	ri.Com_Printf(" %d %d %s\n", width, height, win_fs[fullscreen]);

	// destroy the existing window
	if(sys_gl.hWnd)
	{
		GLimp_Shutdown();
	}

	// do a CDS if needed
	if(fullscreen)
	{
		DEVMODE dm;

		ri.Com_Printf("...attempting fullscreen\n");

		memset(&dm, 0, sizeof(dm));

		dm.dmSize = sizeof(dm);

		dm.dmPelsWidth  = width;
		dm.dmPelsHeight = height;
		dm.dmFields     = DM_PELSWIDTH | DM_PELSHEIGHT;

		if(vid_colorbits->getInteger() != 0 )
		{
			dm.dmBitsPerPel = vid_colorbits->getInteger();
			dm.dmFields |= DM_BITSPERPEL;
			ri.Com_Printf("...using gl_bitdepth of %d\n", vid_colorbits->getInteger());
		}
		else
		{
			HDC hdc = GetDC(NULL);
			int bitspixel = GetDeviceCaps(hdc, BITSPIXEL);

			ri.Com_Printf("...using desktop display depth of %d\n", bitspixel);

			ReleaseDC(0, hdc);
		}

		ri.Com_Printf("...calling CDS: ");
		if(ChangeDisplaySettings(&dm, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL)
		{
			*pwidth = width;
			*pheight = height;

			gl_state.fullscreen = true;

			ri.Com_Printf("ok\n");

			if(!VID_CreateWindow(width, height, true))
				return RSERR_INVALID_MODE;

			return RSERR_OK;
		}
		else
		{
			*pwidth = width;
			*pheight = height;

			ri.Com_Printf("failed\n");

			ri.Com_Printf("...calling CDS assuming dual monitors:");

			dm.dmPelsWidth = width * 2;
			dm.dmPelsHeight = height;
			dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;

			if(vid_colorbits->getInteger() != 0 )
			{
				dm.dmBitsPerPel = vid_colorbits->getInteger();
				dm.dmFields |= DM_BITSPERPEL;
			}

			/*
			** our first CDS failed, so maybe we're running on some weird dual monitor
			** system
			*/
			if(ChangeDisplaySettings(&dm, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
			{
				ri.Com_Printf(" failed\n");

				ri.Com_Printf("...setting windowed mode\n");

				ChangeDisplaySettings(0, 0);

				*pwidth = width;
				*pheight = height;
				
				gl_state.fullscreen = false;
				
				if(!VID_CreateWindow(width, height, false))
					return RSERR_INVALID_MODE;
					
				return RSERR_INVALID_FULLSCREEN;
			}
			else
			{
				ri.Com_Printf(" ok\n");
				
				if(!VID_CreateWindow(width, height, true))
					return RSERR_INVALID_MODE;

				gl_state.fullscreen = true;
				return RSERR_OK;
			}
		}
	}
	else
	{
		ri.Com_Printf("...setting windowed mode\n");

		ChangeDisplaySettings(0, 0);

		*pwidth = width;
		*pheight = height;
		
		gl_state.fullscreen = false;
		
		if(!VID_CreateWindow(width, height, false))
		{
			ri.Com_Printf("... failed!\n");
			return RSERR_INVALID_MODE;
		}
	}

	return RSERR_OK;
}

/*
** GLimp_Shutdown
**
** This routine does all OS specific shutdown procedures for the OpenGL
** subsystem.  Under OpenGL this means NULLing out the current DC and
** HGLRC, deleting the rendering context, and releasing the DC acquired
** for the window.  The state structure is also nulled out.
**
*/
void	GLimp_Shutdown()
{
	ri.Com_Printf("------- GLimp_Shutdown -------\n");
	
	if(xglMakeCurrent && !xglMakeCurrent(NULL, NULL))
		ri.Com_Printf("ref_gl::R_Shutdown() - wglMakeCurrent failed\n");
		
	if(sys_gl.hGLRC)
	{
		if(xglDeleteContext && !xglDeleteContext(sys_gl.hGLRC))
			ri.Com_Printf("ref_gl::R_Shutdown() - wglDeleteContext failed\n");
			
		sys_gl.hGLRC = NULL;
	}
	
	if(sys_gl.hDC)
	{
		if(!ReleaseDC(sys_gl.hWnd, sys_gl.hDC))
			ri.Com_Printf("ref_gl::R_Shutdown() - ReleaseDC failed\n");
			
		sys_gl.hDC   = NULL;
	}
	
	if(sys_gl.hWnd)
	{
		DestroyWindow(sys_gl.hWnd);
		sys_gl.hWnd = NULL;
	}

	UnregisterClass(WINDOW_CLASS_NAME, sys_gl.hInstance);

	if(gl_state.fullscreen)
	{
		ChangeDisplaySettings(0, 0);
		gl_state.fullscreen = false;
	}
}


/*
** GLimp_Init
**
** This routine is responsible for initializing the OS specific portions
** of OpenGL.  Under Win32 this means dealing with the pixelformats and
** doing the wgl interface stuff.
*/
int	GLimp_Init(void *hinstance, void *wndproc)
{
#define OSR2_BUILD_NUMBER 1111

	OSVERSIONINFO	vinfo;

	vinfo.dwOSVersionInfoSize = sizeof(vinfo);

	sys_gl.allowdisplaydepthchange = false;

	if(GetVersionEx(&vinfo))
	{
		if(vinfo.dwMajorVersion > 4)
		{
			sys_gl.allowdisplaydepthchange = true;
		}
		else if(vinfo.dwMajorVersion == 4)
		{
			if(vinfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
			{
				sys_gl.allowdisplaydepthchange = true;
			}
			else if(vinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
			{
				if(LOWORD(vinfo.dwBuildNumber) >= OSR2_BUILD_NUMBER)
				{
					sys_gl.allowdisplaydepthchange = true;
				}
			}
		}
	}
	else
	{
		ri.Com_Printf("GLimp_Init() - GetVersionEx failed\n");
		return false;
	}

	sys_gl.hInstance = ( HINSTANCE ) hinstance;
	sys_gl.wndproc = wndproc;

	return true;
}

bool	GLimp_InitGL()
{
	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),	// size of this pfd
		1,				// version number
		PFD_DRAW_TO_WINDOW |		// support window
		PFD_SUPPORT_OPENGL |		// support OpenGL
		PFD_DOUBLEBUFFER,		// double buffered
		PFD_TYPE_RGBA,			// RGBA type
		24,				// 24-bit color depth
		0, 0, 0, 0, 0, 0,		// color bits ignored
		0,				// no alpha buffer
		0,				// shift bit ignored
		0,				// no accumulation buffer
		0, 0, 0, 0, 			// accum bits ignored
		32,				// 32-bit z-buffer	
		0,				// no stencil buffer
		0,				// no auxiliary buffer
		PFD_MAIN_PLANE,			// main layer
		0,				// reserved
		0, 0, 0				// layer masks ignored
	};
	
	int  pixelformat;
	
	/*
	** set PFD_STEREO if necessary
	*/
	/*
	cvar_t *stereo;
	stereo = ri.Cvar_Get( "cl_stereo", "0", 0 );
	if(stereo->value != 0 )
	{
		ri.Com_Printf( PRINT_ALL, "...attempting to use stereo\n" );
		pfd.dwFlags |= PFD_STEREO;
		gl_state.stereo_enabled = true;
	}
	else
	{
		gl_state.stereo_enabled = false;
	}
	*/

	/*
	** Get a DC for the specified window
	*/
	if(sys_gl.hDC != NULL)
		ri.Com_Printf("GLimp_Init() - non-NULL DC exists\n");

	if((sys_gl.hDC = GetDC(sys_gl.hWnd)) == NULL)
	{
		ri.Com_Printf("GLimp_Init() - GetDC failed\n");
		return false;
	}

	if((pixelformat = ChoosePixelFormat(sys_gl.hDC, &pfd)) == 0)
	{
		ri.Com_Printf("GLimp_Init() - ChoosePixelFormat failed\n");
		return false;
	}
	
	if(SetPixelFormat(sys_gl.hDC, pixelformat, &pfd) == FALSE)
	{
		ri.Com_Printf("GLimp_Init() - SetPixelFormat failed\n");
		return false;
	}
	
	DescribePixelFormat(sys_gl.hDC, pixelformat, sizeof(pfd), &pfd);

	if(!(pfd.dwFlags & PFD_GENERIC_ACCELERATED))
	{
		sys_gl.mcd_accelerated = false;
	}
	else
	{
		sys_gl.mcd_accelerated = true;
	}

	/*
	** report if stereo is desired but unavailable
	*/
	/*
	if(!(pfd.dwFlags & PFD_STEREO) && (stereo->value != 0))
	{
		ri.Com_Printf( PRINT_ALL, "...failed to select stereo pixel format\n" );
		ri.Cvar_SetValue( "cl_stereo", 0 );
		gl_state.stereo_enabled = false;
	}
	*/

	/*
	** startup the OpenGL subsystem by creating a context and making
	** it current
	*/
	if(!xglCreateContext || ((sys_gl.hGLRC = xglCreateContext(sys_gl.hDC)) == 0))
	{
		ri.Com_Printf("GLimp_Init() - qwglCreateContext failed\n");

		goto glimp_init_gl_fail;
	}
	
	if(!xglMakeCurrent || !xglMakeCurrent(sys_gl.hDC, sys_gl.hGLRC))
	{
		ri.Com_Printf("GLimp_Init() - qwglMakeCurrent failed\n");

		goto glimp_init_gl_fail;
	}
	
	if(!VerifyDriver())
	{
		ri.Com_Printf("GLimp_Init() - no hardware acceleration detected\n" );
		goto glimp_init_gl_fail;
	}

	/*
	** print out PFD specifics
	*/
	ri.Com_Printf("GL PFD: color(%d-bits) Z(%d-bit)\n", (int)pfd.cColorBits, (int)pfd.cDepthBits);

	return true;

glimp_init_gl_fail:
	if(sys_gl.hGLRC)
	{
		xglDeleteContext(sys_gl.hGLRC);
		sys_gl.hGLRC = NULL;
	}

	if(sys_gl.hDC)
	{
		ReleaseDC(sys_gl.hWnd, sys_gl.hDC);
		sys_gl.hDC = NULL;
	}
	return false;
}

/*
** GLimp_BeginFrame
*/
void	GLimp_BeginFrame()
{
	if(vid_colorbits->isModified())
	{
		if(vid_colorbits->getInteger() != 0 && !sys_gl.allowdisplaydepthchange)
		{
			ri.Cvar_SetValue("vid_colorbits", 0);
			ri.Com_Printf("vid_colorbits requires Win95 OSR2.x or WinNT 4.x\n");
		}
		
		vid_colorbits->isModified(false);
	}

	/*
	if ( camera_separation < 0 && gl_state.stereo_enabled )
	{
		qglDrawBuffer( GL_BACK_LEFT );
	}
	else if ( camera_separation > 0 && gl_state.stereo_enabled )
	{
		qglDrawBuffer( GL_BACK_RIGHT );
	}
	else
	*/
	{
		xglDrawBuffer(GL_BACK);
	}
}

/*
** GLimp_EndFrame
**
** Responsible for doing a swapbuffers and possibly for other stuff
** as yet to be determined.  Probably better not to make this a GLimp
** function and instead do a call to GLimp_SwapBuffers.
*/
void	GLimp_EndFrame()
{
	RB_CheckForError();
	//int err = qglGetError();
	
	//assert(err == GL_NO_ERROR);

	if(stricmp(r_drawbuffer->getString(), "GL_BACK") == 0)
	{
		if(!SwapBuffers(sys_gl.hDC))
			ri.Com_Error(ERR_FATAL, "GLimp_EndFrame() - SwapBuffers() failed!\n");
	}
}

/*
** GLimp_AppActivate
*/
void GLimp_AppActivate(bool active)
{
	if(active)
	{
		SetForegroundWindow(sys_gl.hWnd);
		ShowWindow(sys_gl.hWnd, SW_RESTORE);
	}
	else
	{
		if(vid_fullscreen->getInteger())
			ShowWindow(sys_gl.hWnd, SW_MINIMIZE);
	}
}


void	GLimp_Gamma()
{
	HDC dc = GetDC(NULL);
	WORD ramp[256*3];

	for(int i=0; i<256; i++)
	{
		ramp[i+0] = ramp[i+256] = ramp[i+512] =
			(WORD)(min(65535, max(0,
				pow((i+1) / 256.0, vid_gamma->getValue())
					   * 65535 + 0.5)));
	}
	
	SetDeviceGammaRamp(dc, ramp);
}

void	GLimp_DeactivatePbuffer()
{
	//TODO
}

void	GLimp_ActivatePbuffer()
{
	//TODO
}

void	GLimp_InitPbuffer(bool shared_context, bool shared_objects)
{
	//TODO
}

void	GLimp_ShutdownPbuffer()
{
	//TODO
}
