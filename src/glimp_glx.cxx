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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
// system -------------------------------------------------------------------
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/vt.h>
#include <stdarg.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>		//Tr3B - _exit(int status);

#include <X11/Xlib.h>		/* Window, Display */
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include <dlfcn.h>

#include <X11/keysym.h>
#include <X11/cursorfont.h>

#ifdef HAVE_XF86_DGA
#include <X11/extensions/xf86dga.h>
#endif

#ifdef HAVE_XF86_VIDMODE
#include <X11/extensions/xf86vmode.h>
#endif

//#include <GL/glx.h>
// shared -------------------------------------------------------------------
#include "x_keycodes.h"

// qrazor-fx ----------------------------------------------------------------
#include "r_local.h"
#include "rw_linux.h"
#include "glimp_glx.h"

sys_gl_t	sys_gl;
sys_pbuffer_t	sys_pbuffer;



#define KEY_MASK (KeyPressMask | KeyReleaseMask)
#define MOUSE_MASK (ButtonPressMask | ButtonReleaseMask | \
		    PointerMotionMask | ButtonMotionMask )
#define X_MASK (KEY_MASK | MOUSE_MASK | VisibilityChangeMask | StructureNotifyMask )

/*****************************************************************************/
/* MOUSE                                                                     */
/*****************************************************************************/

// this is inside the renderer shared lib, so these are called from vid_so

static bool        	mouse_avail;


static int 			win_x, win_y;

static cvar_t	*in_mouse;
static cvar_t	*in_dgamouse;



static bool mlooking;

static bool mouse_active = false;
static bool dgamouse = false;
static bool vidmode_ext = false;

// state struct passed in Init
static in_state_t	*in_state;

static Cursor	CreateNullCursor(Display *display, Window root)
{
    Pixmap cursormask; 
    XGCValues xgc;
    GC gc;
    XColor dummycolour;
    Cursor cursor;

    cursormask = XCreatePixmap(display, root, 1, 1, 1/*depth*/);
    xgc.function = GXclear;
    gc =  XCreateGC(display, cursormask, GCFunction, &xgc);
    XFillRectangle(display, cursormask, gc, 0, 0, 1, 1);
    dummycolour.pixel = 0;
    dummycolour.red = 0;
    dummycolour.flags = 04;
    cursor = XCreatePixmapCursor(display, cursormask, cursormask, &dummycolour, &dummycolour, 0, 0);
    XFreePixmap(display,cursormask);
    XFreeGC(display,gc);
    
    return cursor;
}

static void	install_grabs()
{

// inviso cursor
	XDefineCursor(sys_gl.dpy, sys_gl.win, CreateNullCursor(sys_gl.dpy, sys_gl.win));

	XGrabPointer(sys_gl.dpy, sys_gl.win,
				 True,
				 0,
				 GrabModeAsync, GrabModeAsync,
				 sys_gl.win,
				 None,
				 CurrentTime);

#ifdef HAVE_XF86_DGA
	if(in_dgamouse->getValue())
	{
		int MajorVersion, MinorVersion;

		if(!XF86DGAQueryVersion(sys_gl.dpy, &MajorVersion, &MinorVersion))
		{ 
			// unable to query, probalby not supported
			ri.Com_Printf("Failed to detect XF86DGA Mouse\n" );
			ri.Cvar_Set("in_dgamouse", "0" );
		}
		else
		{
			dgamouse = true;
			XF86DGADirectVideo(sys_gl.dpy, DefaultScreen(sys_gl.dpy), XF86DGADirectMouse);
			XWarpPointer(sys_gl.dpy, None, sys_gl.win, 0, 0, 0, 0, 0, 0);
		}
	}
	else
#endif
	{
		XWarpPointer(sys_gl.dpy, None, sys_gl.win,
					 0, 0, 0, 0,
					 vid.width / 2, vid.height / 2);
	}

	XGrabKeyboard(sys_gl.dpy, sys_gl.win,
				  False,
				  GrabModeAsync, GrabModeAsync,
				  CurrentTime);

	mouse_active = true;

//	XSync(sys_gl.dpy, True);
}

static void	uninstall_grabs()
{
	if(!sys_gl.dpy || !sys_gl.win)
		return;

#ifdef HAVE_XF86_DGA
	if(dgamouse)
	{
		dgamouse = false;
		XF86DGADirectVideo(sys_gl.dpy, DefaultScreen(sys_gl.dpy), 0);
	}
#endif

	XUngrabPointer(sys_gl.dpy, CurrentTime);
	XUngrabKeyboard(sys_gl.dpy, CurrentTime);

// inviso cursor
	XUndefineCursor(sys_gl.dpy, sys_gl.win);

	mouse_active = false;
}


#ifdef __cplusplus
extern "C" {
#endif

static void	RW_IN_MLookDown() 
{ 
	mlooking = true; 
}

static void	RW_IN_MLookUp() 
{
	mlooking = false;
	
//	std::string cmd = "centerview";
//	in_state->Com_PushEvent_fp(SE_CONSOLE, ri.Sys_Milliseconds(), 0, 0, cmd.c_str(), cmd.length());
}

void	RW_IN_Init(in_state_t *in_state_p)
{
	in_state = in_state_p;

	// mouse variables
    	in_mouse 	= ri.Cvar_Get("in_mouse", "1", CVAR_ARCHIVE);
    	in_dgamouse 	= ri.Cvar_Get("in_dgamouse", "1", CVAR_ARCHIVE);

	ri.Cmd_AddCommand("+mlook", RW_IN_MLookDown);
	ri.Cmd_AddCommand("-mlook", RW_IN_MLookUp);

	mouse_avail = true;
}

void	RW_IN_Shutdown()
{
	mouse_avail = false;

	//ri.Cmd_RemoveCommand("-mlook");
	//ri.Cmd_RemoveCommand("+mlook");
}

void	RW_IN_Commands()
{
}

void	RW_IN_Move(usercmd_t &cmd)
{
	if(!mouse_avail)
		return;
   
	
}

static void	IN_DeactivateMouse() 
{
	if(!mouse_avail || !sys_gl.dpy || !sys_gl.win)
		return;

	if(mouse_active)
	{
		uninstall_grabs();
		mouse_active = false;
	}
}

static void	IN_ActivateMouse() 
{
	if(!mouse_avail || !sys_gl.dpy || !sys_gl.win)
		return;

	if(!mouse_active && in_mouse->getValue())
	{
		install_grabs();
		mouse_active = true;
	}
}

void	RW_IN_Activate(bool active)
{
	if(active || sys_gl.vidmodes_active)
		IN_ActivateMouse();
	else
		IN_DeactivateMouse();
}

static int	XLateKey(XKeyEvent *ev)
{

	int key;
	char buf[64];
	KeySym keysym;

	key = 0;

	XLookupString(ev, buf, sizeof buf, &keysym, 0);

	switch(keysym)
	{
		case XK_KP_Page_Up:	 key = K_KP_PGUP; break;
		case XK_Page_Up:	 key = K_PGUP; break;

		case XK_KP_Page_Down: key = K_KP_PGDN; break;
		case XK_Page_Down:	 key = K_PGDN; break;

		case XK_KP_Home: key = K_KP_HOME; break;
		case XK_Home:	 key = K_HOME; break;

		case XK_KP_End:  key = K_KP_END; break;
		case XK_End:	 key = K_END; break;

		case XK_KP_Left: key = K_KP_LEFTARROW; break;
		case XK_Left:	 key = K_LEFTARROW; break;

		case XK_KP_Right: key = K_KP_RIGHTARROW; break;
		case XK_Right:	key = K_RIGHTARROW;		break;

		case XK_KP_Down: key = K_KP_DOWNARROW; break;
		case XK_Down:	 key = K_DOWNARROW; break;

		case XK_KP_Up:   key = K_KP_UPARROW; break;
		case XK_Up:		 key = K_UPARROW;	 break;

		case XK_Escape: key = K_ESCAPE;		break;

		case XK_KP_Enter: key = K_KP_ENTER;	break;
		case XK_Return: key = K_ENTER;		 break;

		case XK_Tab:		key = K_TAB;			 break;

		case XK_F1:		 key = K_F1;				break;

		case XK_F2:		 key = K_F2;				break;

		case XK_F3:		 key = K_F3;				break;

		case XK_F4:		 key = K_F4;				break;

		case XK_F5:		 key = K_F5;				break;

		case XK_F6:		 key = K_F6;				break;

		case XK_F7:		 key = K_F7;				break;

		case XK_F8:		 key = K_F8;				break;

		case XK_F9:		 key = K_F9;				break;

		case XK_F10:		key = K_F10;			 break;

		case XK_F11:		key = K_F11;			 break;

		case XK_F12:		key = K_F12;			 break;

		case XK_BackSpace: key = K_BACKSPACE; break;

		case XK_KP_Delete: key = K_KP_DEL; break;
		case XK_Delete: key = K_DEL; break;

		case XK_Pause:	key = K_PAUSE;		 break;

		case XK_Shift_L:
		case XK_Shift_R:	key = K_SHIFT;		break;

		case XK_Execute: 
		case XK_Control_L: 
		case XK_Control_R:	key = K_CTRL;		 break;

		case XK_Alt_L:	
		case XK_Meta_L: 
		case XK_Alt_R:	
		case XK_Meta_R: key = K_ALT;			break;

		case XK_KP_Begin: key = K_KP_5;	break;

		case XK_Insert:key = K_INS; break;
		case XK_KP_Insert: key = K_KP_INS; break;

		case XK_KP_Multiply: key = '*'; break;
		case XK_KP_Add:  key = K_KP_PLUS; break;
		case XK_KP_Subtract: key = K_KP_MINUS; break;
		case XK_KP_Divide: key = K_KP_SLASH; break;

#if 0
		case 0x021: key = '1';break;/* [!] */
		case 0x040: key = '2';break;/* [@] */
		case 0x023: key = '3';break;/* [#] */
		case 0x024: key = '4';break;/* [$] */
		case 0x025: key = '5';break;/* [%] */
		case 0x05e: key = '6';break;/* [^] */
		case 0x026: key = '7';break;/* [&] */
		case 0x02a: key = '8';break;/* [*] */
		case 0x028: key = '9';;break;/* [(] */
		case 0x029: key = '0';break;/* [)] */
		case 0x05f: key = '-';break;/* [_] */
		case 0x02b: key = '=';break;/* [+] */
		case 0x07c: key = '\'';break;/* [|] */
		case 0x07d: key = '[';break;/* [}] */
		case 0x07b: key = ']';break;/* [{] */
		case 0x022: key = '\'';break;/* ["] */
		case 0x03a: key = ';';break;/* [:] */
		case 0x03f: key = '/';break;/* [?] */
		case 0x03e: key = '.';break;/* [>] */
		case 0x03c: key = ',';break;/* [<] */
#endif

		default:
			key = *(unsigned char*)buf;
			if (key >= 'A' && key <= 'Z')
				key = key - 'A' + 'a';
			break;
	} 

	return key;
}


static void	HandleEvents()
{
	XEvent event;
	int b;
	bool dowarp = false;
	int mwx = vid.width/2;
	int mwy = vid.height/2;
   
	if (!sys_gl.dpy)
		return;

	while (XPending(sys_gl.dpy))
	{

		XNextEvent(sys_gl.dpy, &event);

		switch(event.type)
		{
		case KeyPress:
		case KeyRelease:
			if(in_state && in_state->Com_PushEvent_fp)
				in_state->Com_PushEvent_fp(SE_KEY, ri.Sys_Milliseconds(), XLateKey(&event.xkey), event.type == KeyPress, NULL, 0);
			break;

		case MotionNotify:
			if(mouse_active)
			{
				if(dgamouse)
				{
					in_state->Com_PushEvent_fp(SE_MOUSE, -1, (event.xmotion.x + win_x)*2, (event.xmotion.y + win_y)*2, NULL, 0);
				} 
				else 
				{
					in_state->Com_PushEvent_fp(SE_MOUSE, -1, (event.xmotion.x - mwx)*2, (event.xmotion.y - mwy)*2, NULL, 0);
					
					//mx += ((int)event.xmotion.x - mwx) * 2;
					//my += ((int)event.xmotion.y - mwy) * 2;
					
					//mwx = event.xmotion.x;
					//mwy = event.xmotion.y;

					//if(mx || my)
					dowarp = true;
				}
			}
			break;


		case ButtonPress:
			b=-1;
			if (event.xbutton.button == 1)
				b = 0;
			else if (event.xbutton.button == 2)
				b = 2;
			else if (event.xbutton.button == 3)
				b = 1;
			else if (event.xbutton.button == 4)
				b = 3;
			else if (event.xbutton.button == 5)
				b = 4;
			else if (event.xbutton.button == 6)
				b = 5;
			else if (event.xbutton.button == 7)
				b = 6;
			
			if (b>=0 && in_state && in_state->Com_PushEvent_fp)
				in_state->Com_PushEvent_fp(SE_KEY, ri.Sys_Milliseconds(), K_MOUSE1 + b, true, NULL, 0);
			break;

		case ButtonRelease:
			b=-1;
			if (event.xbutton.button == 1)
				b = 0;
			else if (event.xbutton.button == 2)
				b = 2;
			else if (event.xbutton.button == 3)
				b = 1;
			else if (event.xbutton.button == 4)
				b = 3;
			else if (event.xbutton.button == 5)
				b = 4;
			else if (event.xbutton.button == 6)
				b = 5;
			else if (event.xbutton.button == 7)
				b = 6;
			
			if (b>=0 && in_state && in_state->Com_PushEvent_fp)
				in_state->Com_PushEvent_fp(SE_KEY, ri.Sys_Milliseconds(), K_MOUSE1 + b, false, NULL, 0);
			break;

		case CreateNotify :
			win_x = event.xcreatewindow.x;
			win_y = event.xcreatewindow.y;
			break;

		case ConfigureNotify :
			win_x = event.xconfigure.x;
			win_y = event.xconfigure.y;
			break;

		case ClientMessage:
			//TODO
			break;

		case MapNotify:
			//TODO
 			break;

		case UnmapNotify:
			//TODO
			break;
		}
	}
	   
	if (dowarp)
	{
		/* move the mouse to the window center again */
		XWarpPointer(sys_gl.dpy,None,sys_gl.win,0,0,0,0, vid.width/2,vid.height/2);
	}
}

void	RW_IN_Frame()
{
}


void	KBD_Init()
{
}

void	KBD_Update()
{
	// get events from x server
	HandleEvents();
}

void	KBD_Close()
{
}


#ifdef __cplusplus
}
#endif



extern char*	strsignal(int sig);

static void	signal_handler(int sig)
{
	char *sigstr;

	switch(sig)
	{
		case SIGSEGV:
			sigstr = "SIGSEGV";
			break;
			
		default:
			sigstr = strerror(sig);
	}
	
	
	
	printf("Received signal %d (%s), exiting...\n", sig, strsignal(sig));
	
	GLimp_Shutdown();
	
	_exit(0);
}

static void	InitSig()
{
	signal(SIGHUP, signal_handler);
	signal(SIGQUIT, signal_handler);
	signal(SIGILL, signal_handler);
	signal(SIGTRAP, signal_handler);
	signal(SIGIOT, signal_handler);
	signal(SIGBUS, signal_handler);
	signal(SIGFPE, signal_handler);
	signal(SIGSEGV, signal_handler);
	signal(SIGTERM, signal_handler);
}


int	GLimp_SetMode(int *pwidth, int *pheight, int mode, bool fullscreen)
{
	int width, height;
	std::vector<int> attribs;
	Window root;
	XVisualInfo *visinfo;
	XSetWindowAttributes attr;
	unsigned long mask;
	int MajorVersion, MinorVersion;
	int actualWidth, actualHeight;
	int i;
	int value;
	
	ri.Com_Printf("------- GLimp_SetMode -------\n");

	ri.Com_Printf("GLimp_SetMode: Initializing OpenGL display\n");
	
	if(!ri.VID_GetModeInfo(&width, &height, mode))
	{
		ri.Com_Printf("GLimp_SetMode: invalid mode\n");
		return RSERR_INVALID_MODE;
	}

	if(fullscreen)
		ri.Com_Printf("GLimp_SetMode: setting fullscreen mode %d: %d %d\n", mode, width, height);
	else
		ri.Com_Printf("GLimp_SetMode: setting mode %d: %d %d\n", mode, width, height);

	// destroy the existing window
	GLimp_Shutdown();
	
	char *dpy_name = getenv("DISPLAY");
	
	ri.Com_Printf("GLimp_SetMode: trying to open '%s'\n", dpy_name);

	if((sys_gl.dpy = XOpenDisplay(dpy_name)) == NULL)
	{
		ri.Com_Error(ERR_FATAL, "GLimp_SetMode: couldn't open the X display\n");
		return RSERR_INVALID_MODE;
	}

	sys_gl.scr = DefaultScreen(sys_gl.dpy);
	root = RootWindow(sys_gl.dpy, sys_gl.scr);

	// get video mode list
	MajorVersion = MinorVersion = 0;
#ifdef HAVE_XF86_VIDMODE
	if (!XF86VidModeQueryVersion(sys_gl.dpy, &MajorVersion, &MinorVersion))
	{
		ri.Com_Printf("GLimp_SetMode: no XFree86-VidModeExtension available\n");
		vidmode_ext = false;
	}
	else
	{
		ri.Com_Printf("GLimp_SetMode: using XFree86-VidModeExtension Version %d.%d\n", MajorVersion, MinorVersion);
		vidmode_ext = true;
	}
#endif

	// set attrib list
	attribs.push_back(GLX_RGBA);
	attribs.push_back(GLX_DOUBLEBUFFER);

	if(vid_colorbits->getInteger() >= 16)
	{
		attribs.push_back(GLX_RED_SIZE);
		attribs.push_back(8);
		attribs.push_back(GLX_GREEN_SIZE);
		attribs.push_back(8);
		attribs.push_back(GLX_BLUE_SIZE);
		attribs.push_back(8);
		attribs.push_back(GLX_ALPHA_SIZE);
		attribs.push_back(8);
	}
	else
	{
		attribs.push_back(GLX_RED_SIZE);
		attribs.push_back(5);
		attribs.push_back(GLX_GREEN_SIZE);
		attribs.push_back(5);
		attribs.push_back(GLX_BLUE_SIZE);
		attribs.push_back(5);
		attribs.push_back(GLX_ALPHA_SIZE);
		attribs.push_back(5);
	}

	attribs.push_back(GLX_DEPTH_SIZE);
	if(vid_depthbits->getInteger())
		attribs.push_back(vid_depthbits->getInteger());
	else
		attribs.push_back(24);

	attribs.push_back(GLX_STENCIL_SIZE);
	if(vid_stencilbits->getInteger())
		attribs.push_back(vid_stencilbits->getInteger());
	else
		attribs.push_back(8);

	// end of attrib list
	attribs.push_back(0);

	
	// try attrib list
	visinfo = xglXChooseVisual(sys_gl.dpy, sys_gl.scr, &attribs[0]);
	if(!visinfo)
	{
		ri.Com_Error(ERR_FATAL, "GLimp_SetMode: couldn't get visual\n");
		return RSERR_INVALID_MODE;
	}

        gl_state.hwgamma = false;
	
#ifdef HAVE_XF86_VIDMODE
	if(vidmode_ext)
	{
		int best_fit, best_dist, dist, x, y;
		
		XF86VidModeGetAllModeLines(sys_gl.dpy, sys_gl.scr, &sys_gl.vidmodes_num, &sys_gl.vidmodes);

		// Are we going fullscreen?  If so, let's change video mode
		if(fullscreen)
		{
			best_dist = 9999999;
			best_fit = -1;

			for(i=0; i<sys_gl.vidmodes_num; i++)
			{
				if(width > sys_gl.vidmodes[i]->hdisplay || height > sys_gl.vidmodes[i]->vdisplay)
					continue;

				x = width - sys_gl.vidmodes[i]->hdisplay;
				y = height - sys_gl.vidmodes[i]->vdisplay;
				dist = (x * x) + (y * y);
				if(dist < best_dist)
				{
					best_dist = dist;
					best_fit = i;
				}
			}

			if(best_fit != -1)
			{
				actualWidth = sys_gl.vidmodes[best_fit]->hdisplay;
				actualHeight = sys_gl.vidmodes[best_fit]->vdisplay;

				// change to the mode
				XF86VidModeSwitchToMode(sys_gl.dpy, sys_gl.scr, sys_gl.vidmodes[best_fit]);
				sys_gl.vidmodes_active = true;

				// grab old gamma
				if(XF86VidModeGetGamma(sys_gl.dpy, sys_gl.scr, &sys_gl.gamma))
				{
					gl_state.hwgamma = true;
					
					vid_gamma->isModified(true);

					ri.Com_Printf("GLimp_SetMode: using hardware gamma\n");
				}
				

				// Move the viewport to top left
				XF86VidModeSetViewPort(sys_gl.dpy, sys_gl.scr, 0, 0);
			}
			else
			{
				fullscreen = 0;
			}
		}
	}
#endif //HAVE_XF86_VIDMODE

	/* window attributes */
	attr.background_pixel = 0;
	attr.border_pixel = 0;
	attr.colormap = XCreateColormap(sys_gl.dpy, root, visinfo->visual, AllocNone);
	attr.event_mask = X_MASK;
	if (sys_gl.vidmodes_active)
	{
		mask = CWBackPixel | CWColormap | CWSaveUnder | CWBackingStore | CWEventMask | CWOverrideRedirect;
		attr.override_redirect = True;
		attr.backing_store = NotUseful;
		attr.save_under = False;
	} else
		mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

	sys_gl.win = XCreateWindow(sys_gl.dpy, root, 0, 0, width, height,
						0, visinfo->depth, InputOutput,
						visinfo->visual, mask, &attr);

        XStoreName (sys_gl.dpy, sys_gl.win, "QRazor FX III");
	
	XMapWindow(sys_gl.dpy, sys_gl.win);

#ifdef HAVE_XF86_VIDMODE
	if (sys_gl.vidmodes_active)
	{
		XMoveWindow(sys_gl.dpy, sys_gl.win, 0, 0);
		XRaiseWindow(sys_gl.dpy, sys_gl.win);
		XWarpPointer(sys_gl.dpy, None, sys_gl.win, 0, 0, 0, 0, 0, 0);
		XFlush(sys_gl.dpy);

		// Move the viewport to top left
		XF86VidModeSetViewPort(sys_gl.dpy, sys_gl.scr, 0, 0);
	}
#endif

	XFlush(sys_gl.dpy);

	sys_gl.ctx = xglXCreateContext(sys_gl.dpy, visinfo, NULL, True);

	xglXMakeCurrent(sys_gl.dpy, sys_gl.win, sys_gl.ctx);

	*pwidth = width;
	*pheight = height;

	// let the sound and input subsystems know about the new window
	ri.VID_NewWindow (width, height);

	xglXMakeCurrent(sys_gl.dpy, sys_gl.win, sys_gl.ctx);
	
	
	// check GLX infos
	xglXQueryVersion(sys_gl.dpy, &MajorVersion, &MinorVersion);
        Com_Printf("GLX_VERSION:        %i.%i\n", MajorVersion, MinorVersion);
	
	xglXGetConfig(sys_gl.dpy, visinfo, GLX_USE_GL, &value);
        Com_Printf("GLX_USE_GL:         %s\n", (value ? "Yes" : "No"));
	
        xglXGetConfig(sys_gl.dpy, visinfo, GLX_BUFFER_SIZE, &value);
        Com_Printf("GLX_BUFFER_SIZE:    %i\n", value);
	
        xglXGetConfig(sys_gl.dpy, visinfo, GLX_RGBA, &value);
        Com_Printf("GLX_RGBA :          %s\n", (value ? "Yes" : "No"));
	
        xglXGetConfig(sys_gl.dpy, visinfo, GLX_DOUBLEBUFFER, &value);
        Com_Printf("GLX_DOUBLEBUFFER:   %s\n", (value ? "Yes" : "No"));
	
        xglXGetConfig(sys_gl.dpy, visinfo, GLX_STEREO, &value);
        Com_Printf("GLX_STEREO:         %s\n", (value ? "Yes" : "No"));
	
        xglXGetConfig(sys_gl.dpy, visinfo, GLX_RED_SIZE, &value);
        Com_Printf("GLX_RED_SIZE:       %i\n", value);
	
        xglXGetConfig(sys_gl.dpy, visinfo, GLX_GREEN_SIZE, &value);
        Com_Printf("GLX_GREEN_SIZE:     %i\n", value);
	
        xglXGetConfig(sys_gl.dpy, visinfo, GLX_BLUE_SIZE, &value);
        Com_Printf("GLX_BLUE_SIZE:      %i\n", value);
	
        xglXGetConfig(sys_gl.dpy, visinfo, GLX_ALPHA_SIZE, &value);
        Com_Printf("GLX_ALPHA_SIZE:     %i\n", value);
	
        xglXGetConfig(sys_gl.dpy, visinfo, GLX_DEPTH_SIZE, &value);
        Com_Printf("GLX_DEPTH_SIZE:     %i\n", value);
	
        xglXGetConfig(sys_gl.dpy, visinfo, GLX_STENCIL_SIZE, &value);
        Com_Printf("GLX_STENCIL_SIZE:   %i\n", value);
	
	// check glx extensions
	sys_gl.extensions_string = (const char*)xglXQueryExtensionsString(sys_gl.dpy, sys_gl.scr);
	ri.Com_Printf("GLX_EXTENSIONS: %s\n", sys_gl.extensions_string);
	
	sys_gl.sgix_fbconfig	= false;
	sys_gl.sgix_pbuffer	= false;

	if(strstr(sys_gl.extensions_string, "SGIX_fbconfig"))
	{
		if(r_sgix_fbconfig->getValue())
		{
			ri.Com_Printf("...using SGIX_fbconfig\n");
			xglXChooseFBConfigSGIX = (GLXFBConfigSGIX* (*)(Display *dpy, int screen, const int *attrib_list, int *nelements)) XGL_GetSymbol("glXChooseFBConfigSGIX");
			xglXCreateContextWithConfigSGIX = (GLXContext (*)(Display *dpy, GLXFBConfigSGIX config, int render_type, GLXContext share_list, Bool direct)) XGL_GetSymbol("glXCreateContextWithConfigSGIX");
			sys_gl.sgix_fbconfig = true;
		}
		else
		{
			ri.Com_Printf("...ignoring SGIX_fbconfig\n");
		}
	}
	else
	{
		ri.Com_Printf("...SGIX_fbconfig not found\n" );
	}

	if(strstr(sys_gl.extensions_string, "SGIX_pbuffer"))
	{
		if(r_sgix_pbuffer->getValue())
		{
			ri.Com_Printf("...using SGIX_pbuffer\n");
			
			xglXCreateGLXPbufferSGIX = (GLXPbuffer (*)(Display *dpy, GLXFBConfig config, unsigned int width, unsigned int height, const int *attrib_list)) XGL_GetSymbol("glXCreateGLXPbufferSGIX");
			xglXQueryGLXPbufferSGIX = (void (*)(Display *dpy, GLXPbuffer pbuf, int attribute, unsigned int *value)) XGL_GetSymbol("glXQueryGLXPbufferSGIX");
			sys_gl.sgix_pbuffer = true;
		}
		else
		{
			ri.Com_Printf("...ignoring SGIX_pbuffer\n");
		}
	}
	else
	{
		ri.Com_Printf("...SGIX_fbconfig not found\n" );
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

	uninstall_grabs();
	mouse_active = false;
	dgamouse = false;

	if(sys_gl.dpy)
	{
		if(sys_gl.ctx)
			xglXDestroyContext(sys_gl.dpy, sys_gl.ctx);
			
		if(sys_gl.win)
			XDestroyWindow(sys_gl.dpy, sys_gl.win);

#ifdef HAVE_XF86_VIDMODE
		if(gl_state.hwgamma)
                          XF86VidModeSetGamma(sys_gl.dpy, sys_gl.scr, &sys_gl.gamma);
			
		if(sys_gl.vidmodes_active)
			XF86VidModeSwitchToMode(sys_gl.dpy, sys_gl.scr, sys_gl.vidmodes[0]);
#endif
		XUngrabKeyboard(sys_gl.dpy, CurrentTime);
		XCloseDisplay(sys_gl.dpy);
	}
	
	sys_gl.ctx = NULL;
	sys_gl.dpy = NULL;
	sys_gl.win = 0;
	sys_gl.ctx = NULL;
}

/*
** GLimp_Init
**
** This routine is responsible for initializing the OS specific portions
** of OpenGL.  
*/
int	GLimp_Init(void *hinstance, void *wndproc)
{
	ri.Com_Printf("------- GLimp_Init -------\n");

	InitSig();
	
	return true;
}

void	GLimp_BeginFrame()
{
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
	//ri.Com_Printf("GLimp_EndFrame:\n");
	
	RB_CheckForError();
	
	xglFlush();
	xglXSwapBuffers(sys_gl.dpy, sys_gl.win);
	
	RB_CheckForError();
}

void	GLimp_AppActivate(bool active)
{
}



void	GLimp_Gamma()
{
#ifdef HAVE_XF86_VIDMODE
	XF86VidModeGamma gamma;
	float	g;

	g = (1.3 - vid_gamma->getValue() + 1);
	g = (g > 1) ? g : 1;
	
	gamma.red = sys_gl.gamma.red * g;
	gamma.green = sys_gl.gamma.green * g;
	gamma.blue = sys_gl.gamma.blue * g;

	ri.Com_Printf("Setting gamma to: %f\n", g);
	XF86VidModeSetGamma(sys_gl.dpy, sys_gl.scr, &gamma);
#endif
}


#if 0
static void	GLimp_ParsePbufferModeString(const std::string &mode_string, std::vector<int> *attribs)
{
	if (!mode_string.length())
		return;

	//bool bIsFloatBuffer = false; 
	//bool bNeedAlpha = false;
 	
	std::string token;   
	char *mode = (char*)mode_string.c_str();

	while(true)
	{
		token = Com_Parse(&mode);
		
		if(!mode)
			break;
		
		ri.Com_Printf("GLimp_ParsePbufferModeString: '%s'\n", token.c_str());
		
		/*
		if(token.find("float") != token.npos)
		{
			bIsFloatBuffer = true;
			continue;
		}
		*/
		
		/*
		if(token == "alpha")
		{
			bNeedAlpha = true;
			continue;
		}
		*/
        
		if(token.find("rgb") != token.npos /* && !bIsFloatBuffer*/)
		{
			attribs->push_back(GLX_RED_SIZE);
			attribs->push_back(8);
			attribs->push_back(GLX_GREEN_SIZE);
			attribs->push_back(8);
			attribs->push_back(GLX_BLUE_SIZE);
			attribs->push_back(8);
			continue;
		}

		if(token.find("alpha") != token.npos)
		{
			attribs->push_back(GLX_ALPHA_SIZE);
			attribs->push_back(GetIntegerValue(token));
			continue;
		}

		if(token.find("depth") != token.npos)
		{
			attribs->push_back(GLX_DEPTH_SIZE);
			attribs->push_back(GetIntegerValue(token));
			continue;
		}

		if(token.find("stencil") != token.npos)
		{
			attribs->push_back(GLX_STENCIL_SIZE);
			attribs->push_back(GetIntegerValue(token));
            		continue;
		}
      
		if(token.find("samples") != token.npos)
		{
			attribs->push_back(GLX_SAMPLE_BUFFERS_ARB);
			attribs->push_back(1);
			attribs->push_back(GLX_SAMPLES_ARB);
			attribs->push_back(GetIntegerValue(token));
			continue;
		}
		
		if(token.find("double") != token.npos)
		{
			attribs->push_back(GLX_DOUBLEBUFFER);
			attribs->push_back(true);
			continue;
		}
		
		/*
		if(X_strcaseequal(token, "float"))
		{
			token = Com_Parse(&mode);
			int precision = atoi(token);
			
			attribs->push_back(GLX_RED_SIZE);
			attribs->push_back(precision);
			attribs->push_back(GLX_GREEN_SIZE);
			attribs->push_back(precision);
			attribs->push_back(GLX_BLUE_SIZE);
			attribs->push_back(precision);
			
			if(bNeedAlpha)
			{
				attribs->push_back(GLX_ALPHA_SIZE);
				attribs->push_back(precision);
			}
			
			attribs->push_back(GLX_FLOAT_COMPONENTS_NV);
			attribs->push_back(true);
			continue;
		}
		*/       

		
		ri.Com_Error(ERR_FATAL, "GLimp_ParsePbufferModeString: unknown pbuffer attribute: %s", token.c_str());
	}
} 
#endif

void	GLimp_InitPbuffer(bool shared_context, bool shared_objects)
{
	if(!sys_gl.sgix_fbconfig || !sys_gl.sgix_pbuffer)
		return;
	
	Display *dpy		= xglXGetCurrentDisplay();
	int screen		= DefaultScreen(dpy);
	GLXContext ctx		= xglXGetCurrentContext();
	
	GLXFBConfig*	config;
	int		config_count;
	
	// setup pixel buffer attributes
	sys_pbuffer.fattribs.push_back(GLX_DRAWABLE_TYPE);
	sys_pbuffer.fattribs.push_back(GLX_PBUFFER_BIT);
	sys_pbuffer.fattribs.push_back(GLX_RENDER_TYPE);
	sys_pbuffer.fattribs.push_back(GLX_RGBA_BIT);

	sys_pbuffer.battribs.push_back(GLX_LARGEST_PBUFFER);
	sys_pbuffer.battribs.push_back(true);
	sys_pbuffer.battribs.push_back(GLX_PRESERVED_CONTENTS);
	sys_pbuffer.battribs.push_back(true);

		
	if(vid_pbuffer_colorbits->getInteger() >= 16)
	{
		sys_pbuffer.fattribs.push_back(GLX_RED_SIZE);
		sys_pbuffer.fattribs.push_back(8);
		sys_pbuffer.fattribs.push_back(GLX_GREEN_SIZE);
		sys_pbuffer.fattribs.push_back(8);
		sys_pbuffer.fattribs.push_back(GLX_BLUE_SIZE);
		sys_pbuffer.fattribs.push_back(8);
		sys_pbuffer.fattribs.push_back(GLX_ALPHA_SIZE);
		sys_pbuffer.fattribs.push_back(8);
	}
	else
	{
		sys_pbuffer.fattribs.push_back(GLX_RED_SIZE);
		sys_pbuffer.fattribs.push_back(5);
		sys_pbuffer.fattribs.push_back(GLX_GREEN_SIZE);
		sys_pbuffer.fattribs.push_back(5);
		sys_pbuffer.fattribs.push_back(GLX_BLUE_SIZE);
		sys_pbuffer.fattribs.push_back(5);
		sys_pbuffer.fattribs.push_back(GLX_ALPHA_SIZE);
		sys_pbuffer.fattribs.push_back(5);
	}
	
	
	sys_pbuffer.fattribs.push_back(GLX_DEPTH_SIZE);
	if(vid_pbuffer_depthbits->getInteger())
		sys_pbuffer.fattribs.push_back(vid_pbuffer_depthbits->getInteger());
	else
		sys_pbuffer.fattribs.push_back(24);
		
	
	sys_pbuffer.fattribs.push_back(GLX_STENCIL_SIZE);
	if(vid_pbuffer_stencilbits->getInteger())
		sys_pbuffer.fattribs.push_back(vid_pbuffer_stencilbits->getInteger());
	else
		sys_pbuffer.fattribs.push_back(8);


	// end of attrib lists
	sys_pbuffer.fattribs.push_back(0);
	sys_pbuffer.battribs.push_back(0);
	
	
	if(shared_context)
	{
		config = xglXGetFBConfigs(dpy, screen, &config_count);
		if(!config)
		{
			ri.Com_Error(ERR_FATAL, "GLimp_InitPbuffer: glXGetFBConfigs() failed");
		}
	}
	else
	{
#if 0
		for(unsigned i=0; i<sys_pbuffer.fattribs.size(); i++)
			ri.Com_Printf("GLimp_InitPbuffer: %i\n", sys_pbuffer.fattribs[i]);
		ri.Com_Printf("\n");
#endif
		
		config = xglXChooseFBConfigSGIX(dpy, screen, &sys_pbuffer.fattribs[0], &config_count);
		if(!config)
		{
			ri.Com_Error(ERR_FATAL, "GLimp_InitPbuffer: glXChooseFBConfigSGIX() failed");
		}
	}
	
#if 0
	int width = 0, height = 0;
	ri.VID_GetModeInfo(&width, &height, vid_mode->getInteger());
	
	ri.Cvar_SetValue("vid_pbuffer_width", width);
	ri.Cvar_SetValue("vid_pbuffer_height", height);
#endif
	
	sys_pbuffer.pbuffer = xglXCreateGLXPbufferSGIX(dpy, config[0], vid_pbuffer_width->getInteger(), vid_pbuffer_height->getInteger(), &sys_pbuffer.battribs[0]);
	
	if(!sys_pbuffer.pbuffer)
	{
        	ri.Com_Error(ERR_FATAL, "GLimp_InitPuffer: glXCreatePbuffer() failed");
	}
	
	if(shared_context)
	{
        	sys_pbuffer.ctx = ctx;
	}
	else
	{
		if(shared_objects)
			sys_pbuffer.ctx = xglXCreateContextWithConfigSGIX(dpy, config[0], GLX_RGBA_TYPE, ctx, true);
		else
			sys_pbuffer.ctx = xglXCreateContextWithConfigSGIX(dpy, config[0], GLX_RGBA_TYPE, NULL, true);
		
		if(!config)
		{
			ri.Com_Error(ERR_FATAL, "GLimp_InitPbuffer: glXCreateNewContext() failed");
		}
	}
	
	sys_pbuffer.dpy = dpy;

	xglXQueryGLXPbufferSGIX(sys_pbuffer.dpy, sys_pbuffer.pbuffer, GLX_WIDTH, &sys_pbuffer.width);
	xglXQueryGLXPbufferSGIX(sys_pbuffer.dpy, sys_pbuffer.pbuffer, GLX_HEIGHT, &sys_pbuffer.height);
	    
	ri.Com_Printf("GLimp_InitPbuffer: created a %d x %d pbuffer\n", sys_pbuffer.width, sys_pbuffer.height);
}

void	GLimp_ActivatePbuffer()
{
	if(!sys_gl.sgix_fbconfig || !sys_gl.sgix_pbuffer)
	{
		gl_state.active_pbuffer = false;
		return;
	}
		
	sys_pbuffer.dpy_old = xglXGetCurrentDisplay();
	sys_pbuffer.pbuffer_old = xglXGetCurrentDrawable();
	sys_pbuffer.ctx_old = xglXGetCurrentContext();

	if(!xglXMakeCurrent(sys_pbuffer.dpy, sys_pbuffer.pbuffer, sys_pbuffer.ctx))
		ri.Com_Error(ERR_FATAL, "GLimp_ActivatePbuffer: glXMakeCurrent failed");
		
	gl_state.active_pbuffer = true;
}


void	GLimp_DeactivatePbuffer()
{
	if(!sys_gl.sgix_fbconfig || !sys_gl.sgix_pbuffer)
	{
		gl_state.active_pbuffer = false;
		return;
	}

	if(!xglXMakeCurrent(sys_pbuffer.dpy_old, sys_pbuffer.pbuffer_old, sys_pbuffer.ctx_old))
		ri.Com_Error(ERR_FATAL, "GLimp_DeactivatePbuffer: glXMakeCurrent failed");
	
	sys_pbuffer.dpy_old = NULL;
	sys_pbuffer.pbuffer_old = 0;
	sys_pbuffer.ctx_old = NULL;
	
	gl_state.active_pbuffer = false;
}



