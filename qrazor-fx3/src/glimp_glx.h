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
#ifndef GLIMP_GLX_H
#define GLIMP_GLX_H


/// includes ===================================================================
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
// system -------------------------------------------------------------------
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

// qrazor-fx ----------------------------------------------------------------



//#ifndef __linux__
//#error You shouldnt be including this file on non-Linux platforms
//#endif


/*
** Names for attributes to glXGetConfig.
*/
#define GLX_USE_GL		1	/* support GLX rendering */
#define GLX_BUFFER_SIZE		2	/* depth of the color buffer */
#define GLX_LEVEL		3	/* level in plane stacking */
#define GLX_RGBA		4	/* true if RGBA mode */
#define GLX_DOUBLEBUFFER	5	/* double buffering supported */
#define GLX_STEREO		6	/* stereo buffering supported */
#define GLX_AUX_BUFFERS 	7	/* number of aux buffers */
#define GLX_RED_SIZE		8	/* number of red component bits */
#define GLX_GREEN_SIZE		9	/* number of green component bits */
#define GLX_BLUE_SIZE		10	/* number of blue component bits */
#define GLX_ALPHA_SIZE		11	/* number of alpha component bits */
#define GLX_DEPTH_SIZE		12	/* number of depth bits */
#define GLX_STENCIL_SIZE	13	/* number of stencil bits */
#define GLX_ACCUM_RED_SIZE	14	/* number of red accum bits */
#define GLX_ACCUM_GREEN_SIZE	15	/* number of green accum bits */
#define GLX_ACCUM_BLUE_SIZE	16	/* number of blue accum bits */
#define GLX_ACCUM_ALPHA_SIZE	17	/* number of alpha accum bits */

#define GLX_SAMPLE_BUFFERS_ARB  100000  /* number of multisample buffers */
#define GLX_SAMPLES_ARB         100001  /* number of multisample samples */

/*
 * FBConfig-specific attributes
 */
#define GLX_X_VISUAL_TYPE               0x22
#define GLX_CONFIG_CAVEAT               0x20    /* Like visual_info VISUAL_CAVEAT */
#define GLX_TRANSPARENT_TYPE            0x23
#define GLX_TRANSPARENT_INDEX_VALUE     0x24
#define GLX_TRANSPARENT_RED_VALUE       0x25
#define GLX_TRANSPARENT_GREEN_VALUE     0x26
#define GLX_TRANSPARENT_BLUE_VALUE      0x27
#define GLX_TRANSPARENT_ALPHA_VALUE     0x28
#define GLX_DRAWABLE_TYPE               0x8010
#define GLX_RENDER_TYPE                 0x8011
#define GLX_X_RENDERABLE                0x8012
#define GLX_FBCONFIG_ID                 0x8013
#define GLX_MAX_PBUFFER_WIDTH           0x8016
#define GLX_MAX_PBUFFER_HEIGHT          0x8017
#define GLX_MAX_PBUFFER_PIXELS          0x8018
#define GLX_VISUAL_ID                   0x800B
                                                                                
#define GLX_DRAWABLE_TYPE_SGIX          GLX_DRAWABLE_TYPE
#define GLX_RENDER_TYPE_SGIX            GLX_RENDER_TYPE
#define GLX_X_RENDERABLE_SGIX           GLX_X_RENDERABLE
#define GLX_FBCONFIG_ID_SGIX            GLX_FBCONFIG_ID
#define GLX_MAX_PBUFFER_WIDTH_SGIX      GLX_MAX_PBUFFER_WIDTH 
#define GLX_MAX_PBUFFER_HEIGHT_SGIX     GLX_MAX_PBUFFER_HEIGHT
#define GLX_MAX_PBUFFER_PIXELS_SGIX     GLX_MAX_PBUFFER_PIXELS
#define GLX_OPTIMAL_PBUFFER_WIDTH_SGIX  0x8019
#define GLX_OPTIMAL_PBUFFER_HEIGHT_SGIX 0x801A

/* GLX_RENDER_TYPE bits */
#define GLX_RGBA_BIT                    0x00000001
#define GLX_COLOR_INDEX_BIT             0x00000002
#define GLX_RGBA_BIT_SGIX               GLX_RGBA_BIT
#define GLX_COLOR_INDEX_BIT_SGIX        GLX_COLOR_INDEX_BIT

/* GLX_DRAWABLE_TYPE bits */
#define GLX_WINDOW_BIT                  0x00000001
#define GLX_PIXMAP_BIT                  0x00000002
#define GLX_PBUFFER_BIT                 0x00000004
#define GLX_WINDOW_BIT_SGIX             GLX_WINDOW_BIT
#define GLX_PIXMAP_BIT_SGIX             GLX_PIXMAP_BIT
#define GLX_PBUFFER_BIT_SGIX            GLX_PBUFFER_BIT

/* glXCreateGLXPbuffer attributes */
#define GLX_PRESERVED_CONTENTS          0x801B
#define GLX_LARGEST_PBUFFER             0x801C
#define GLX_PBUFFER_HEIGHT              0x8040  /* New for GLX 1.3 */
#define GLX_PBUFFER_WIDTH               0x8041  /* New for GLX 1.3 */
#define GLX_PRESERVED_CONTENTS_SGIX     GLX_PRESERVED_CONTENTS
#define GLX_LARGEST_PBUFFER_SGIX        GLX_LARGEST_PBUFFER

/* glXQueryGLXPBuffer attributes */
#define GLX_WIDTH                       0x801D
#define GLX_HEIGHT                      0x801E
#define GLX_EVENT_MASK                  0x801F
#define GLX_WIDTH_SGIX                  GLX_WIDTH
#define GLX_HEIGHT_SGIX                 GLX_HEIGHT
#define GLX_EVENT_MASK_SGIX             GLX_EVENT_MASK

/* glXCreateNewContext render_type attribute values */
#define GLX_RGBA_TYPE                   0x8014
#define GLX_COLOR_INDEX_TYPE            0x8015
#define GLX_RGBA_TYPE_SGIX              GLX_RGBA_TYPE
#define GLX_COLOR_INDEX_TYPE_SGIX       GLX_COLOR_INDEX_TYPE

/*
** GLX resources.
*/
typedef XID GLXContextID;
typedef XID GLXPixmap;
typedef XID GLXDrawable;
typedef XID GLXPbuffer;
typedef XID GLXWindow;
typedef XID GLXFBConfigID;
typedef XID GLXPbufferSGIX;

typedef struct __GLXcontextRec *GLXContext;

typedef struct __GLXFBConfigRec *GLXFBConfig;
typedef struct __GLXFBConfigRec *GLXFBConfigSGIX;


struct sys_gl_t
{
	sys_gl_t()
	{
		dpy		= NULL;
		scr		= 0;
		win		= 0;
		
		ctx		= 0;
		OpenGLLib	= NULL;
	}

	Display*		dpy;
	int 			scr;
	GLXContext 		ctx;
	
	Window 			win;
	void*			OpenGLLib; 	// instance of OpenGL library e.g. libGL.so.1
	
#ifdef HAVE_XF86_VIDMODE
	bool			vidmodes_available;
	XF86VidModeModeInfo**	vidmodes;
	int 			vidmodes_num;
	bool 			vidmodes_active;
	XF86VidModeGamma	gamma;
#endif

	const char*		extensions_string;
	bool			arb_get_proc_address;
	bool			sgix_fbconfig;
	bool			sgix_pbuffer;
};


struct sys_pbuffer_t
{
	Display*		dpy;
	GLXPbuffer		pbuffer;
	GLXContext 		ctx;
	int			scr;
	
	std::vector<int>	fattribs;
	std::vector<int>	battribs; 

	uint_t			width;
	uint_t			height;
	
	bool			shared_ctx;
	bool			shared_objects;
	
	Display*		dpy_old;
	GLXPbuffer		pbuffer_old;
	GLXContext 		ctx_old;
};


extern sys_gl_t		sys_gl;
extern sys_pbuffer_t	sys_pbuffer;

#endif //GLIMP_GLX_H
