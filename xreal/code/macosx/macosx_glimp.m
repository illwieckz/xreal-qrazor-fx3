/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
#import "macosx_glimp.h"

#include "tr_local.h"
#import "macosx_local.h"
#import "macosx_display.h"
#import "macosx_timers.h"

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

#import <mach-o/dyld.h>
#import <mach/mach.h>
#import <mach/mach_error.h>

cvar_t	*r_allowSoftwareGL;		// don't abort out if the pixelformat claims software
cvar_t  *r_enablerender;                // Enable actual rendering
cvar_t  *r_appleTransformHint;          // Enable Apple transform hint

static void GLW_InitExtensions( void );
static qboolean CreateGameWindow( qboolean isSecondTry );
static unsigned long Sys_QueryVideoMemory();
static CGDisplayErr Sys_CaptureActiveDisplays(void);


glwstate_t glw_state;
qboolean Sys_IsHidden = qfalse;

#ifdef OMNI_TIMER
OTStampList glThreadStampList;
#endif

@interface NSOpenGLContext (CGLContextAccess)
- (CGLContextObj) cglContext;
@end

@implementation NSOpenGLContext (CGLContextAccess)
- (CGLContextObj) cglContext;
{
    return _contextAuxiliary;
}
@end

/*
============
CheckErrors
============
*/
void CheckErrors( void )
{		
    GLenum   err;

    err = qglGetError();
    if ( err != GL_NO_ERROR ) {
        ri.Error( ERR_FATAL, "glGetError: %s\n", qglGetString( err ) );
    }
}

#if !defined(NDEBUG) && defined(QGL_CHECK_GL_ERRORS)

unsigned int QGLBeginStarted = 0;

void QGLErrorBreak(void)
{
}

void QGLCheckError(const char *message)
{
    GLenum        error;
    static unsigned int errorCount = 0;
    
    error = _glGetError();
    if (error != GL_NO_ERROR) {
        if (errorCount == 100) {
            Com_Printf("100 GL errors printed ... disabling further error reporting.\n");
        } else if (errorCount < 100) {
            if (errorCount == 0) {
                fprintf(stderr, "BREAK ON QGLErrorBreak to stop at the GL errors\n");
            }
            fprintf(stderr, "OpenGL Error(%s): 0x%04x -- %s\n", message, (int)error,  gluErrorString(error));
            QGLErrorBreak();
        }
        errorCount++;
    }
}
#endif

/*
** GLimp_SetMode
*/

qboolean GLimp_SetMode( qboolean isSecondTry )
{
    if ( !CreateGameWindow(isSecondTry) ) {
        ri.Printf( PRINT_ALL, "GLimp_Init: window could not be created!\n" );
        return qfalse;
    }

    // draw something to show that GL is alive	
    if (r_enablerender->integer) {
        qglClearColor( 0.5, 0.5, 0.7, 0 );
        qglClear( GL_COLOR_BUFFER_BIT );
        GLimp_EndFrame();
        
        qglClearColor( 0.5, 0.5, 0.7, 0 );
        qglClear( GL_COLOR_BUFFER_BIT );
        GLimp_EndFrame();
    }

    Sys_UnfadeScreen(Sys_DisplayToUse(), NULL);
    
    CheckErrors();

    return qtrue;
}

/*
 =================
 GetPixelAttributes
 =================
 */

#define ADD_ATTR(x) \
do { \
    if (attributeIndex >= attributeSize) { \
        attributeSize *= 2; \
        pixelAttributes = NSZoneRealloc(NULL, pixelAttributes, sizeof(*pixelAttributes) * attributeSize); \
    } \
    pixelAttributes[attributeIndex] = x; \
    attributeIndex++; \
    if (verbose) { \
        ri.Printf(PRINT_ALL, "Adding pixel attribute: %d (%s)\n", x, #x); \
    } \
} while(0)

static NSOpenGLPixelFormatAttribute *GetPixelAttributes()
{
    NSOpenGLPixelFormatAttribute *pixelAttributes;
    unsigned int attributeIndex = 0;
    unsigned int attributeSize = 128;
    int verbose = 0;
    unsigned int colorDepth;
    
    verbose = r_verbose->integer;
    
    pixelAttributes = NSZoneMalloc(NULL, sizeof(*pixelAttributes) * attributeSize);

    if (r_fullscreen->integer) {
        ADD_ATTR(NSOpenGLPFAFullScreen);

        // Since we are fullscreen, specify the screen that we need.
        ADD_ATTR(NSOpenGLPFAScreenMask);
        ADD_ATTR(CGDisplayIDToOpenGLDisplayMask(Sys_DisplayToUse()));
    }
    
    // Require hardware acceleration unless otherwise directed
    if (!r_allowSoftwareGL->integer) {
        ADD_ATTR(NSOpenGLPFAAccelerated);
    }

    // Require double-buffer
    ADD_ATTR(NSOpenGLPFADoubleBuffer);

    // Specify the number of color bits.  If we don't have a valid specified value or we are not full screen, use the current display mode's value.
    ADD_ATTR(NSOpenGLPFAColorSize);
    colorDepth = r_colorbits->integer;
    if (colorDepth < 16)
        colorDepth = 16;
    else if (colorDepth > 16)
        colorDepth = 32;
    if (!r_fullscreen->integer)
        colorDepth = [[glw_state.desktopMode objectForKey: (id)kCGDisplayBitsPerPixel] intValue];
    ADD_ATTR(colorDepth);

    // Specify the number of depth bits
    ADD_ATTR(NSOpenGLPFADepthSize);
    ADD_ATTR(r_depthbits->integer ? r_depthbits->integer : 16);

    // Specify the number of stencil bits
    if (r_stencilbits->integer) {
        ADD_ATTR(NSOpenGLPFAStencilSize);
        ADD_ATTR(r_stencilbits->integer);
    }

    // Terminate the list
    ADD_ATTR(0);
    
    return pixelAttributes;
}

// Needs to be visible to Q3Controller.m.
void Sys_UpdateWindowMouseInputRect(void)
{		
    NSRect           windowRect, screenRect;
    NSScreen        *screen;

    // It appears we need to flip the coordinate system here.  This means we need
    // to know the size of the screen.
    screen = [glw_state.window screen];
    screenRect = [screen frame];
    windowRect = [glw_state.window frame];
    windowRect.origin.y = screenRect.size.height - (windowRect.origin.y + windowRect.size.height);
    
    Sys_SetMouseInputRect(CGRectMake(windowRect.origin.x, windowRect.origin.y,
                                    windowRect.size.width, windowRect.size.height));
}									

// This is needed since CGReleaseAllDisplays() restores the gamma on the displays and we want to fade it up rather than just flickering all the displays
static void ReleaseAllDisplays()
{
    CGDisplayCount displayIndex;

    Com_Printf("Releasing displays\n");
    for (displayIndex = 0; displayIndex < glw_state.displayCount; displayIndex++) {
        CGDisplayRelease(glw_state.originalDisplayGammaTables[displayIndex].display);
    }
}

/*
=================
CreateGameWindow
=================
*/
static qboolean CreateGameWindow( qboolean isSecondTry )
{
    const char *windowed[] = { "Windowed", "Fullscreen" };
    int			current_mode;
    NSOpenGLPixelFormatAttribute *pixelAttributes;
    NSOpenGLPixelFormat *pixelFormat;
    CGDisplayErr err;
            

    // get mode info
    current_mode = r_mode->integer;
    glConfig.isFullscreen = (r_fullscreen->integer != 0);

    glw_state.desktopMode = (NSDictionary *)CGDisplayCurrentMode(glw_state.display);
    if (!glw_state.desktopMode) {
        ri.Error(ERR_FATAL, "Could not get current graphics mode for display 0x%08x\n", glw_state.display);
    }

#if 0
    ri.Printf( PRINT_ALL, "... desktop mode %d = %dx%d %s\n", glw_state.desktopMode,
               glw_state.desktopDesc.width, glw_state.desktopDesc.height,
               depthStrings[glw_state.desktopDesc.depth]);
#endif

    ri.Printf( PRINT_ALL, "...setting mode %d:\n", current_mode );
    if ( !R_GetModeInfo( &glConfig.vidWidth, &glConfig.vidHeight, &glConfig.windowAspect, current_mode ) )  {
        ri.Printf( PRINT_ALL, " invalid mode\n" );
        return qfalse;
    }
    ri.Printf( PRINT_ALL, " %d %d %s\n", glConfig.vidWidth, glConfig.vidHeight, windowed[glConfig.isFullscreen] );

    if (glConfig.isFullscreen) {
        
        // We'll set up the screen resolution first in case that effects the list of pixel
        // formats that are available (for example, a smaller frame buffer might mean more
        // bits for depth/stencil buffers).  Allow stretched video modes if we are in fallback mode.
        glw_state.gameMode = Sys_GetMatchingDisplayMode(isSecondTry);
        if (!glw_state.gameMode) {
            ri.Printf( PRINT_ALL, "Unable to find requested display mode.\n");
            return qfalse;
        }

        // Fade all screens to black
        Sys_FadeScreens();
        
        err = Sys_CaptureActiveDisplays();
        if ( err != CGDisplayNoErr ) {
            CGDisplayRestoreColorSyncSettings();
            ri.Printf( PRINT_ALL, " Unable to capture displays err = %d\n", err );
            return qfalse;
        }

        err = CGDisplaySwitchToMode(glw_state.display, (CFDictionaryRef)glw_state.gameMode);
        if ( err != CGDisplayNoErr ) {
            CGDisplayRestoreColorSyncSettings();
            ReleaseAllDisplays();
            ri.Printf( PRINT_ALL, " Unable to set display mode, err = %d\n", err );
            return qfalse;
        }
    } else {
        glw_state.gameMode = glw_state.desktopMode;
    }

    
    // Get the GL pixel format
    pixelAttributes = GetPixelAttributes();
    pixelFormat = [[[NSOpenGLPixelFormat alloc] initWithAttributes: pixelAttributes] autorelease];
    NSZoneFree(NULL, pixelAttributes);
    
    if (!pixelFormat) {
        CGDisplayRestoreColorSyncSettings();
        CGDisplaySwitchToMode(glw_state.display, (CFDictionaryRef)glw_state.desktopMode);
        ReleaseAllDisplays();
        ri.Printf( PRINT_ALL, " No pixel format found\n");
        return qfalse;
    }

    // Create a context with the desired pixel attributes
    OSX_SetGLContext([[NSOpenGLContext alloc] initWithFormat: pixelFormat shareContext: nil]);
    if (!OSX_GetNSGLContext()) {
        CGDisplayRestoreColorSyncSettings();
        CGDisplaySwitchToMode(glw_state.display, (CFDictionaryRef)glw_state.desktopMode);
        ReleaseAllDisplays();
        ri.Printf(PRINT_ALL, "... +[NSOpenGLContext createWithFormat:share:] failed.\n" );
        return qfalse;
    }

    if (!glConfig.isFullscreen) {
        cvar_t		*vid_xpos;
        cvar_t		*vid_ypos;
        NSRect           windowRect;
        
        vid_xpos = ri.Cvar_Get( "vid_xpos", "100", CVAR_ARCHIVE );
        vid_ypos = ri.Cvar_Get( "vid_ypos", "100", CVAR_ARCHIVE );

        // Create a window of the desired size
        windowRect.origin.x = vid_xpos->integer;
        windowRect.origin.y = vid_ypos->integer;
        windowRect.size.width = glConfig.vidWidth;
        windowRect.size.height = glConfig.vidHeight;
        
        glw_state.window = [[NSWindow alloc] initWithContentRect:windowRect
                                                       styleMask:NSTitledWindowMask
                                                         backing:NSBackingStoreRetained
                                                           defer:NO];
                                                           
        [glw_state.window setTitle: @"Quake3"];

        [glw_state.window orderFront: nil];

        // Always get mouse moved events (if mouse support is turned off (rare)
        // the event system will filter them out.
        [glw_state.window setAcceptsMouseMovedEvents: YES];
        
        // Direct the context to draw in this window
        [OSX_GetNSGLContext() setView: [glw_state.window contentView]];

        // Sync input rect with where the window actually is...
        Sys_UpdateWindowMouseInputRect();
    } else {
        CGLError err;
        
        err = CGLSetFullScreen(OSX_GetCGLContext());
        if (err) {
            CGDisplayRestoreColorSyncSettings();
            CGDisplaySwitchToMode(glw_state.display, (CFDictionaryRef)glw_state.desktopMode);
            ReleaseAllDisplays();
            Com_Printf("CGLSetFullScreen -> %d (%s)\n", err, CGLErrorString(err));
            return qfalse;
        }
        
        Sys_SetMouseInputRect(CGDisplayBounds(glw_state.display));
    }


#ifndef USE_CGLMACROS
    // Make this the current context
    OSX_GLContextSetCurrent();
#endif

    // Store off the pixel format attributes that we actually got
    [pixelFormat getValues: (long *) &glConfig.colorBits forAttribute: NSOpenGLPFAColorSize forVirtualScreen: 0];
    [pixelFormat getValues: (long *) &glConfig.depthBits forAttribute: NSOpenGLPFADepthSize forVirtualScreen: 0];
    [pixelFormat getValues: (long *) &glConfig.stencilBits forAttribute: NSOpenGLPFAStencilSize forVirtualScreen: 0];

    glConfig.displayFrequency = [[glw_state.gameMode objectForKey: (id)kCGDisplayRefreshRate] intValue];
    
    
    ri.Printf(PRINT_ALL, "ok\n" );

    return qtrue;
}

// This can be used to temporarily disassociate the GL context from the screen so that CoreGraphics can be used to draw to the screen.
void Sys_PauseGL ()
{
    if (!glw_state.glPauseCount) {
        qglFinish (); // must do this to ensure the queue is complete
        
        // Have to call both to actually deallocate kernel resources and free the NSSurface
        CGLClearDrawable(OSX_GetCGLContext());
        [OSX_GetNSGLContext() clearDrawable];
    }
    glw_state.glPauseCount++;
}

// This can be used to reverse the pausing caused by Sys_PauseGL()
void Sys_ResumeGL ()
{
    if (glw_state.glPauseCount) {
        glw_state.glPauseCount--;
        if (!glw_state.glPauseCount) {
            if (!glConfig.isFullscreen) {
                [OSX_GetNSGLContext() setView: [glw_state.window contentView]];
            } else {
                CGLError err;
                
                err = CGLSetFullScreen(OSX_GetCGLContext());
                if (err)
                    Com_Printf("CGLSetFullScreen -> %d (%s)\n", err, CGLErrorString(err));
            }
        }
    }
}

/*
===================
GLimp_Init

Don't return unless OpenGL has been properly initialized
===================
*/

static void GLImp_Toggle_Renderer_f(void)
{
    ri.Cvar_Set("r_enablerender", r_enablerender->integer ? "0" : "1");
}

#ifdef OMNI_TIMER
static void GLImp_Dump_Stamp_List_f(void)
{
    OTStampListDumpToFile(glThreadStampList, "/tmp/gl_stamps");
}
#endif

void GLimp_Init( void )
{
    static BOOL addedCommands = NO;
    cvar_t *lastValidRenderer = ri.Cvar_Get( "r_lastValidRenderer", "(uninitialized)", CVAR_ARCHIVE );
    char *buf;

    if (!addedCommands) {
        addedCommands = YES;

#ifdef OMNI_TIMER
        glThreadStampList = OTStampListCreate(64);
        Cmd_AddCommand ("dump_stamp_list", GLImp_Dump_Stamp_List_f);
#endif
        Cmd_AddCommand ("toggle_renderer", GLImp_Toggle_Renderer_f);
    }

    ri.Printf( PRINT_ALL, "Initializing OpenGL subsystem\n" );
    ri.Printf( PRINT_ALL, "  Last renderer was '%s'\n", lastValidRenderer->string);
    ri.Printf( PRINT_ALL, "  r_fullscreen = %d\n", r_fullscreen->integer);

    memset( &glConfig, 0, sizeof( glConfig ) );
    memset( &glConfig2, 0, sizeof( glConfig2 ) );

    // We only allow changing the gamma if we are full screen
    glConfig.deviceSupportsGamma = (r_fullscreen->integer != 0);
    if (glConfig.deviceSupportsGamma) {
        Sys_StoreGammaTables();
    }
    
    r_allowSoftwareGL = ri.Cvar_Get( "r_allowSoftwareGL", "0", CVAR_LATCH );
    r_enablerender = ri.Cvar_Get("r_enablerender", "1", 0 );

    if (Sys_QueryVideoMemory() == 0 && !r_allowSoftwareGL->integer) {
        ri.Error( ERR_FATAL, "Could not initialize OpenGL.  There does not appear to be an OpenGL-supported video card in your system.\n" );
    }
    
    if ( ! GLimp_SetMode(qfalse) ) {
        // fall back to the known-good mode
        ri.Cvar_Set( "r_fullscreen", "1" );
        ri.Cvar_Set( "r_mode", "3" );
        ri.Cvar_Set( "r_stereo", "0" );
        ri.Cvar_Set( "r_depthBits", "16" );
        ri.Cvar_Set( "r_colorBits", "16" );
        ri.Cvar_Set( "r_stencilBits", "8" );
        if ( GLimp_SetMode(qtrue) ) {
            ri.Printf( PRINT_ALL, "------------------\n" );
            return;
        }

        ri.Error( ERR_FATAL, "Could not initialize OpenGL\n" );
        return;
    }

    ri.Printf( PRINT_ALL, "------------------\n" );

    // get our config strings
    Q_strncpyz( glConfig.vendor_string, (const char *)qglGetString (GL_VENDOR), sizeof( glConfig.vendor_string ) );
    Q_strncpyz( glConfig.renderer_string, (const char *)qglGetString (GL_RENDERER), sizeof( glConfig.renderer_string ) );
    Q_strncpyz( glConfig.version_string, (const char *)qglGetString (GL_VERSION), sizeof( glConfig.version_string ) );
    Q_strncpyz( glConfig.extensions_string, (const char *)qglGetString (GL_EXTENSIONS), sizeof( glConfig.extensions_string ) );

    //
    // chipset specific configuration
    //
    buf = malloc(strlen(glConfig.renderer_string) + 1);
    strcpy( buf, glConfig.renderer_string );
    Q_strlwr( buf );

    ri.Cvar_Set( "r_lastValidRenderer", glConfig.renderer_string );
    free(buf);

    GLW_InitExtensions();
    
#ifndef USE_CGLMACROS
    if (!r_enablerender->integer)
        OSX_GLContextClearCurrent();
#endif
}


/*
** GLimp_EndFrame
** 
** Responsible for doing a swapbuffers and possibly for other stuff
** as yet to be determined.  Probably better not to make this a GLimp
** function and instead do a call to GLimp_SwapBuffers.
*/
void GLimp_EndFrame (void)
{
    GLSTAMP("GLimp_EndFrame start", 0);
    
    //
    // swapinterval stuff
    //
    if ( r_swapInterval->modified ) {
        r_swapInterval->modified = qfalse;

        if ( !glConfig.stereoEnabled ) {	// why?
            [[NSOpenGLContext currentContext] setValues: (long *)&r_swapInterval->integer
            forParameter: NSOpenGLCPSwapInterval];
        }
    }

#if !defined(NDEBUG) && defined(QGL_CHECK_GL_ERRORS)
    QGLCheckError("GLimp_EndFrame");
#endif

    if (!glw_state.glPauseCount && !Sys_IsHidden) {
        glw_state.bufferSwapCount++;
        [OSX_GetNSGLContext() flushBuffer];
    }
    
    // Enable turning off GL at any point for performance testing
    if (OSX_GLContextIsCurrent() != r_enablerender->integer) {
        if (r_enablerender->integer) {
            Com_Printf("--- Enabling Renderer ---\n");
            OSX_GLContextSetCurrent();
        } else {
            Com_Printf("--- Disabling Renderer ---\n");
            OSX_GLContextClearCurrent();
        }
    }

    GLSTAMP("GLimp_EndFrame end", 0);
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

static void _GLimp_RestoreOriginalVideoSettings()
{
    CGDisplayErr err;
    
    // CGDisplayCurrentMode lies because we've captured the display and thus we won't
    // get any notifications about what the current display mode really is.  For now,
    // we just always force it back to what mode we remember the desktop being in.
    if (glConfig.isFullscreen) {
        err = CGDisplaySwitchToMode(glw_state.display, (CFDictionaryRef)glw_state.desktopMode);
        if ( err != CGDisplayNoErr )
            ri.Printf( PRINT_ALL, " Unable to restore display mode!\n" );

        ReleaseAllDisplays();
    }
}

void GLimp_Shutdown( void )
{
    CGDisplayCount displayIndex;

    Com_Printf("----- Shutting down GL -----\n");

    Sys_FadeScreen(Sys_DisplayToUse());
    
    if (OSX_GetNSGLContext()) {
#ifndef USE_CGLMACROS
        OSX_GLContextClearCurrent();
#endif
        // Have to call both to actually deallocate kernel resources and free the NSSurface
        CGLClearDrawable(OSX_GetCGLContext());
        [OSX_GetNSGLContext() clearDrawable];
        
        [OSX_GetNSGLContext() release];
        OSX_SetGLContext((id)nil);
    }

    _GLimp_RestoreOriginalVideoSettings();
    
    Sys_UnfadeScreens();

    // Restore the original gamma if needed.
    if (glConfig.deviceSupportsGamma) {
        Com_Printf("Restoring ColorSync settings\n");
        CGDisplayRestoreColorSyncSettings();
    }

    if (glw_state.window) {
        [glw_state.window release];
        glw_state.window = nil;
    }

    if (glw_state.log_fp) {
        fclose(glw_state.log_fp);
        glw_state.log_fp = 0;
    }

    for (displayIndex = 0; displayIndex < glw_state.displayCount; displayIndex++) {
        free(glw_state.originalDisplayGammaTables[displayIndex].red);
        free(glw_state.originalDisplayGammaTables[displayIndex].blue);
        free(glw_state.originalDisplayGammaTables[displayIndex].green);
    }
    free(glw_state.originalDisplayGammaTables);
    if (glw_state.tempTable.red) {
        free(glw_state.tempTable.red);
        free(glw_state.tempTable.blue);
        free(glw_state.tempTable.green);
    }
    if (glw_state.inGameTable.red) {
        free(glw_state.inGameTable.red);
        free(glw_state.inGameTable.blue);
        free(glw_state.inGameTable.green);
    }
    
    memset(&glConfig, 0, sizeof(glConfig));
    memset(&glConfig2, 0, sizeof(glConfig2));
    memset(&glState, 0, sizeof(glState));
    memset(&glw_state, 0, sizeof(glw_state));

    Com_Printf("----- Done shutting down GL -----\n");
}

/*
===============
GLimp_LogComment

===============
*/
void	GLimp_LogComment( char *comment ) {
        }

/*
===============
GLimp_SetGamma

===============
*/
void GLimp_SetGamma(unsigned char red[256],
                    unsigned char green[256],
                    unsigned char blue[256])
{
    CGGammaValue redGamma[256], greenGamma[256], blueGamma[256];
    CGTableCount i;
    CGDisplayErr err;
    
    if (!glConfig.deviceSupportsGamma)
        return;
        
    for (i = 0; i < 256; i++) {
        redGamma[i]   = red[i]   / 255.0;
        greenGamma[i] = green[i] / 255.0;
        blueGamma[i]  = blue[i]  / 255.0;
    }
    
    err = CGSetDisplayTransferByTable(glw_state.display, 256, redGamma, greenGamma, blueGamma);
    if (err != CGDisplayNoErr) {
        Com_Printf("GLimp_SetGamma: CGSetDisplayTransferByByteTable returned %d.\n", err);
    }
    
    // Store the gamma table that we ended up using so we can reapply it later when unhiding or to work around the bug where if you leave the game sitting and the monitor sleeps, when it wakes, the gamma isn't reset.
    glw_state.inGameTable.display = glw_state.display;
    Sys_GetGammaTable(&glw_state.inGameTable);
}

qboolean GLimp_ChangeMode( int mode )
{
    qboolean result;
    int oldvalue = r_mode->integer;

    Com_Printf("*** GLimp_ChangeMode\n");
    r_mode->integer = mode;
    if (!(result = GLimp_SetMode(qfalse)))
        r_mode->integer = oldvalue;
    
   return result;
}

/*****************************************************************************/

void *qwglGetProcAddress(const char *name)
{
    NSSymbol symbol;
    char *symbolName;

    // Prepend a '_' for the Unix C symbol mangling convention
    symbolName = malloc(strlen(name) + 2);
    strcpy(symbolName + 1, name);
    symbolName[0] = '_';

    if (NSIsSymbolNameDefined(symbolName))
        symbol = NSLookupAndBindSymbol(symbolName);
    else
        symbol = NULL;
    
    free(symbolName);
    
    if (!symbol)
        // shouldn't happen ...
        return NULL;

    return NSAddressOfSymbol(symbol);
}

/*
** GLW_InitExtensions
*/
static void GLW_InitExtensions( void )
{
        if ( !r_allowExtensions->integer )
        {
                ri.Printf( PRINT_ALL, "*** IGNORING OPENGL EXTENSIONS ***\n" );
                return;
        }

        ri.Printf( PRINT_ALL, "Initializing OpenGL extensions\n" );
        ri.Printf( PRINT_ALL, "... Supported extensions are %s\n", glConfig.extensions_string);
        
        // GL_S3_s3tc
        glConfig.textureCompression = TC_NONE;
        if ( strstr( glConfig.extensions_string, "GL_S3_s3tc" ) )
        {
                if ( r_ext_compressed_textures->integer == 1 )
                {
                        glConfig.textureCompression = TC_S3TC;
                        ri.Printf( PRINT_ALL, "...using GL_S3_s3tc\n" );
                }
                else
                {
                        glConfig.textureCompression = TC_NONE;
                        ri.Printf( PRINT_ALL, "...ignoring GL_S3_s3tc\n" );
                }
        }
        else
        {
                ri.Printf( PRINT_ALL, "...GL_S3_s3tc not found\n" );
        }


#ifdef GL_EXT_texture_env_add
        // GL_EXT_texture_env_add
        glConfig.textureEnvAddAvailable = qfalse;
        if ( strstr( glConfig.extensions_string, "GL_EXT_texture_env_add" ) )
        {
                if ( r_ext_texture_env_add->integer )
                {
                        glConfig.textureEnvAddAvailable = qtrue;
                        ri.Printf( PRINT_ALL, "...using GL_EXT_texture_env_add\n" );
                }
                else
                {
                        glConfig.textureEnvAddAvailable = qfalse;
                        ri.Printf( PRINT_ALL, "...ignoring GL_EXT_texture_env_add\n" );
                }
        }
        else
        {
                ri.Printf( PRINT_ALL, "...GL_EXT_texture_env_add not found\n" );
        }
#endif

#ifdef GL_ARB_texture_env_add
        // GL_ARB_texture_env_add -- only if we didn't find GL_EXT_texture_env_add
        if (!glConfig.textureEnvAddAvailable) {
            if ( strstr( glConfig.extensions_string, "GL_ARB_texture_env_add" ) )
            {
                    if ( r_ext_texture_env_add->integer )
                    {
                            glConfig.textureEnvAddAvailable = qtrue;
                            ri.Printf( PRINT_ALL, "...using GL_ARB_texture_env_add\n" );
                    }
                    else
                    {
                            glConfig.textureEnvAddAvailable = qfalse;
                            ri.Printf( PRINT_ALL, "...ignoring GL_ARB_texture_env_add\n" );
                    }
            }
            else
            {
                    ri.Printf( PRINT_ALL, "...GL_ARB_texture_env_add not found\n" );
            }
        }
#endif


#if 0   // Win32 does this differently than we do -- I'll provide a C function that looks the same
        // that will do the correct ObjC stuff
        // WGL_EXT_swap_control
        qwglSwapIntervalEXT = ( BOOL (WINAPI *)(int)) qwglGetProcAddress( "wglSwapIntervalEXT" );
        if ( qwglSwapIntervalEXT )
        {
                ri.Printf( PRINT_ALL, "...using WGL_EXT_swap_control\n" );
                r_swapInterval->modified = qtrue;	// force a set next frame
        }
        else
        {
                ri.Printf( PRINT_ALL, "...WGL_EXT_swap_control not found\n" );
        }
#else
        if (r_swapInterval) {
            ri.Printf( PRINT_ALL, "...using +[NSOpenGLContext setParameter:] for qwglSwapIntervalEXT\n" );
            r_swapInterval->modified = qtrue;	// force a set next frame
        }
#endif
        
        // GL_ARB_multitexture
        qglMultiTexCoord2fARB = NULL;
        qglActiveTextureARB = NULL;
        qglClientActiveTextureARB = NULL;
        if ( strstr( glConfig.extensions_string, "GL_ARB_multitexture" )  )
        {
                if ( r_ext_multitexture->integer )
                {
                        qglMultiTexCoord2fARB = ( PFNGLMULTITEXCOORD2FARBPROC ) qwglGetProcAddress( "glMultiTexCoord2fARB" );
                        qglActiveTextureARB = ( PFNGLACTIVETEXTUREARBPROC ) qwglGetProcAddress( "glActiveTextureARB" );
                        qglClientActiveTextureARB = ( PFNGLCLIENTACTIVETEXTUREARBPROC ) qwglGetProcAddress( "glClientActiveTextureARB" );

                        if ( qglActiveTextureARB )
                        {
                                qglGetIntegerv( GL_MAX_TEXTURE_UNITS_ARB, (GLint *)&glConfig.maxTextureUnits );

                                if ( glConfig.maxTextureUnits > 1 )
                                {
                                        ri.Printf( PRINT_ALL, "...using GL_ARB_multitexture\n" );
                                }
                                else
                                {
                                        qglMultiTexCoord2fARB = NULL;
                                        qglActiveTextureARB = NULL;
                                        qglClientActiveTextureARB = NULL;
                                        ri.Printf( PRINT_ALL, "...not using GL_ARB_multitexture, < 2 texture units\n" );
                                }
                        }
                }
                else
                {
                        ri.Printf( PRINT_ALL, "...ignoring GL_ARB_multitexture\n" );
                }
        }
        else
        {
                ri.Printf( PRINT_ALL, "...GL_ARB_multitexture not found\n" );
        }

	// GL_ARB_transpose_matrix
	qglLoadTransposeMatrixfARB = NULL;
	if ( strstr( glConfig.extensions_string, "GL_ARB_transpose_matrix" ) ) {
		if ( r_ext_transpose_matrix->value ) {
			qglLoadTransposeMatrixfARB = ( PFNGLLOADTRANSPOSEMATRIXFARBPROC ) qwglGetProcAddress( "glLoadTransposeMatrixfARB" );
		  ri.Printf( PRINT_ALL, "...using GL_ARB_transpose_matrix\n" );
		} else {
		  ri.Printf( PRINT_ALL, "...ignoring GL_ARB_transpose_matrix\n" );
		}
	} else {
		ri.Printf( PRINT_ALL, "...GL_ARB_transpose_matrix not found\n" );
	}
	
	// GL_ARB_texture_cube_map
	glConfig2.textureCubeAvailable = qfalse;
	if(strstr(glConfig.extensions_string, "GL_ARB_texture_cube_map"))
	{
		if(r_ext_texture_cube_map->integer)
		{
			qglGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB, &glConfig2.maxCubeMapTextureSize);
			glConfig2.textureCubeAvailable = qtrue;
			ri.Printf(PRINT_ALL, "...using GL_ARB_texture_cube_map\n");
		}
		else
		{
			ri.Printf(PRINT_ALL, "...ignoring GL_ARB_texture_cube_map\n");
		}
	}
	else
	{
		ri.Printf(PRINT_ALL, "...GL_ARB_texture_cube_map\n");
	}
	
	// GL_ARB_depth_texture
	glConfig2.depthTextureAvailable = qfalse;
	if(strstr(glConfig.extensions_string, "GL_ARB_depth_texture"))
	{
		if(r_ext_depth_texture->integer)
		{
			glConfig2.depthTextureAvailable = qtrue;
			ri.Printf(PRINT_ALL, "...using GL_ARB_depth_texture\n");
		}
		else
		{
			ri.Printf(PRINT_ALL, "...ignoring GL_ARB_depth_texture\n");
		}
	}
	else
	{
		ri.Printf(PRINT_ALL, "...GL_ARB_depth_texture\n");
	}
	
	// GL_ARB_vertex_program
	glConfig2.vertexProgramAvailable = qfalse;
	qglVertexAttribPointerARB = NULL;
	qglEnableVertexAttribArrayARB = NULL;
	qglDisableVertexAttribArrayARB = NULL;
	if ( strstr( glConfig.extensions_string, "GL_ARB_vertex_program" ) ) {
		if ( r_ext_vertex_program->value ) {
			qglVertexAttribPointerARB = ( PFNGLVERTEXATTRIBPOINTERARBPROC ) qwglGetProcAddress( "glVertexAttribPointerARB" );
			qglEnableVertexAttribArrayARB = ( PFNGLENABLEVERTEXATTRIBARRAYARBPROC ) qwglGetProcAddress( "glEnableVertexAttribArrayARB" );
			qglDisableVertexAttribArrayARB = ( PFNGLDISABLEVERTEXATTRIBARRAYARBPROC ) qwglGetProcAddress( "glDisableVertexAttribArrayARB" );
			glConfig2.vertexProgramAvailable = qtrue;
			ri.Printf( PRINT_ALL, "...using GL_ARB_vertex_program\n" );
			} else {
				ri.Printf( PRINT_ALL, "...ignoring GL_ARB_vertex_program\n" );
		}
	} else {
		ri.Printf( PRINT_ALL, "...GL_ARB_vertex_program not found\n" );
	}
	
	// GL_ARB_vertex_buffer_object
	glConfig2.vertexBufferObjectAvailable = qfalse;
	qglBindBufferARB = NULL;
	qglDeleteBuffersARB = NULL;
	qglGenBuffersARB = NULL;
	qglIsBufferARB = NULL;
	qglBufferDataARB = NULL;
	qglBufferSubDataARB = NULL;
	qglGetBufferSubDataARB = NULL;
	qglMapBufferARB = NULL;
	qglUnmapBufferARB = NULL;
	qglGetBufferParameterivARB = NULL;
	qglGetBufferPointervARB = NULL;
	if(strstr(glConfig.extensions_string, "GL_ARB_vertex_buffer_object"))
	{
		if(r_ext_vertex_buffer_object->value)
		{
			qglBindBufferARB = (PFNGLBINDBUFFERARBPROC) qwglGetProcAddress("glBindBufferARB");
			qglDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC) qwglGetProcAddress("glDeleteBuffersARB");
			qglGenBuffersARB = (PFNGLGENBUFFERSARBPROC) qwglGetProcAddress("glGenBuffersARB");
			qglIsBufferARB = (PFNGLISBUFFERARBPROC) qwglGetProcAddress("glIsBufferARB");
			qglBufferDataARB = (PFNGLBUFFERDATAARBPROC) qwglGetProcAddress("glBufferDataARB");
			qglBufferSubDataARB = (PFNGLBUFFERSUBDATAARBPROC) qwglGetProcAddress("glBufferSubDataARB");
			qglGetBufferSubDataARB = (PFNGLGETBUFFERSUBDATAARBPROC) qwglGetProcAddress("glGetBufferSubDataARB");
			qglMapBufferARB = (PFNGLMAPBUFFERARBPROC) qwglGetProcAddress("glMapBufferARB");
			qglUnmapBufferARB = (PFNGLUNMAPBUFFERARBPROC) qwglGetProcAddress("glUnmapBufferARB");
			qglGetBufferParameterivARB = (PFNGLGETBUFFERPARAMETERIVARBPROC) qwglGetProcAddress("glGetBufferParameterivARB");
			qglGetBufferPointervARB = (PFNGLGETBUFFERPOINTERVARBPROC) qwglGetProcAddress("glGetBufferPointervARB");
			glConfig2.vertexBufferObjectAvailable = qtrue;
			ri.Printf(PRINT_ALL, "...using GL_ARB_vertex_buffer_object\n");
		}
		else
		{
			ri.Printf(PRINT_ALL, "...ignoring GL_ARB_vertex_buffer_object\n");
		}
	}
	else
	{
		ri.Printf(PRINT_ALL, "...GL_ARB_vertex_buffer_object not found\n");
	}

	// GL_ARB_occlusion_query
	glConfig2.occlusionQueryAvailable = qfalse;
	glConfig2.occlusionQueryBits = 0;
	qglGenQueriesARB = NULL;
	qglDeleteQueriesARB = NULL;
	qglIsQueryARB = NULL;
	qglBeginQueryARB = NULL;
	qglEndQueryARB = NULL;
	qglGetQueryivARB = NULL;
	qglGetQueryObjectivARB = NULL;
	qglGetQueryObjectuivARB = NULL;
	if(strstr(glConfig.extensions_string, "GL_ARB_occlusion_query"))
	{
		if(r_ext_occlusion_query->value)
		{
			qglGenQueriesARB = (PFNGLGENQUERIESARBPROC) qwglGetProcAddress("glGenQueriesARB");
			qglDeleteQueriesARB = (PFNGLDELETEQUERIESARBPROC) qwglGetProcAddress("glDeleteQueriesARB");
			qglIsQueryARB = (PFNGLISQUERYARBPROC) qwglGetProcAddress("glIsQueryARB");
			qglBeginQueryARB = (PFNGLBEGINQUERYARBPROC) qwglGetProcAddress("glBeginQueryARB");
			qglEndQueryARB = (PFNGLENDQUERYARBPROC) qwglGetProcAddress("glEndQueryARB");
			qglGetQueryivARB = (PFNGLGETQUERYIVARBPROC) qwglGetProcAddress("glGetQueryivARB");
			qglGetQueryObjectivARB = (PFNGLGETQUERYOBJECTIVARBPROC) qwglGetProcAddress("glGetQueryObjectivARB");
			qglGetQueryObjectuivARB = (PFNGLGETQUERYOBJECTUIVARBPROC) qwglGetProcAddress("glGetQueryObjectuivARB");
			glConfig2.occlusionQueryAvailable = qtrue;
			qglGetQueryivARB(GL_SAMPLES_PASSED, GL_QUERY_COUNTER_BITS, &glConfig2.occlusionQueryBits); 
			ri.Printf(PRINT_ALL, "...using GL_ARB_occlusion_query\n");
		}
		else
		{
			ri.Printf(PRINT_ALL, "...ignoring GL_ARB_occlusion_query\n");
		}
	}
	else
	{
		ri.Printf(PRINT_ALL, "...GL_ARB_occlusion_query not found\n");
	}
	
	// GL_ARB_shader_objects
	glConfig2.shaderObjectsAvailable = qfalse;
	qglDeleteObjectARB = NULL;
	qglGetHandleARB = NULL;
	qglDetachObjectARB = NULL;
	qglCreateShaderObjectARB = NULL;
	qglShaderSourceARB = NULL;
	qglCompileShaderARB = NULL;
	qglCreateProgramObjectARB = NULL;
	qglAttachObjectARB = NULL;
	qglLinkProgramARB = NULL;
	qglUseProgramObjectARB = NULL;
	qglValidateProgramARB = NULL;
	qglUniform1fARB = NULL;
	qglUniform2fARB = NULL;
	qglUniform3fARB = NULL;
	qglUniform4fARB = NULL;
	qglUniform1iARB = NULL;
	qglUniform2iARB = NULL;
	qglUniform3iARB = NULL;
	qglUniform4iARB = NULL;
	qglUniform2fvARB = NULL;
	qglUniform3fvARB = NULL;
	qglUniform4fvARB = NULL;
	qglUniform2ivARB = NULL;
	qglUniform3ivARB = NULL;
	qglUniform4ivARB = NULL;
	qglUniformMatrix2fvARB = NULL;
	qglUniformMatrix3fvARB = NULL;
	qglUniformMatrix4fvARB = NULL;
	qglGetObjectParameterfvARB = NULL;
	qglGetObjectParameterivARB = NULL;
	qglGetInfoLogARB = NULL;
	qglGetAttachedObjectsARB = NULL;
	qglGetUniformLocationARB = NULL;
	qglGetActiveUniformARB = NULL;
	qglGetUniformfvARB = NULL;
	qglGetUniformivARB = NULL;
	qglGetShaderSourceARB = NULL;
	if ( strstr( glConfig.extensions_string, "GL_ARB_shader_objects" ) ) {
		if ( r_ext_shader_objects->value ) {
			qglDeleteObjectARB = ( PFNGLDELETEOBJECTARBPROC ) qwglGetProcAddress( "glDeleteObjectARB" );
			qglGetHandleARB = ( PFNGLGETHANDLEARBPROC ) qwglGetProcAddress( "glGetHandleARB" );
			qglDetachObjectARB = ( PFNGLDETACHOBJECTARBPROC ) qwglGetProcAddress( "glDetachObjectARB" );
			qglCreateShaderObjectARB = ( PFNGLCREATESHADEROBJECTARBPROC ) qwglGetProcAddress( "glCreateShaderObjectARB" );
			qglShaderSourceARB = ( PFNGLSHADERSOURCEARBPROC ) qwglGetProcAddress( "glShaderSourceARB" );
			qglCompileShaderARB = ( PFNGLCOMPILESHADERARBPROC ) qwglGetProcAddress( "glCompileShaderARB" );
			qglCreateProgramObjectARB = ( PFNGLCREATEPROGRAMOBJECTARBPROC ) qwglGetProcAddress( "glCreateProgramObjectARB" );
			qglAttachObjectARB = ( PFNGLATTACHOBJECTARBPROC ) qwglGetProcAddress( "glAttachObjectARB" );
			qglLinkProgramARB = ( PFNGLLINKPROGRAMARBPROC ) qwglGetProcAddress( "glLinkProgramARB" );
			qglUseProgramObjectARB = ( PFNGLUSEPROGRAMOBJECTARBPROC ) qwglGetProcAddress( "glUseProgramObjectARB" );
			qglValidateProgramARB = ( PFNGLVALIDATEPROGRAMARBPROC ) qwglGetProcAddress( "glValidateProgramARB" );
			qglUniform1fARB = ( PFNGLUNIFORM1FARBPROC ) qwglGetProcAddress( "glUniform1fARB" );
			qglUniform2fARB = ( PFNGLUNIFORM2FARBPROC ) qwglGetProcAddress( "glUniform2fARB" );
			qglUniform3fARB = ( PFNGLUNIFORM3FARBPROC ) qwglGetProcAddress( "glUniform3fARB" );
			qglUniform4fARB = ( PFNGLUNIFORM4FARBPROC ) qwglGetProcAddress( "glUniform4fARB" );
			qglUniform1iARB = ( PFNGLUNIFORM1IARBPROC ) qwglGetProcAddress( "glUniform1iARB" );
			qglUniform2iARB = ( PFNGLUNIFORM2IARBPROC ) qwglGetProcAddress( "glUniform2iARB" );
			qglUniform3iARB = ( PFNGLUNIFORM3IARBPROC ) qwglGetProcAddress( "glUniform3iARB" );
			qglUniform4iARB = ( PFNGLUNIFORM4IARBPROC ) qwglGetProcAddress( "glUniform4iARB" );
			qglUniform2fvARB = ( PFNGLUNIFORM2FVARBPROC ) qwglGetProcAddress( "glUniform2fvARB" );
			qglUniform3fvARB = ( PFNGLUNIFORM3FVARBPROC ) qwglGetProcAddress( "glUniform3fvARB" );
			qglUniform4fvARB = ( PFNGLUNIFORM4FVARBPROC ) qwglGetProcAddress( "glUniform4fvARB" );
			qglUniform2ivARB = ( PFNGLUNIFORM2IVARBPROC ) qwglGetProcAddress( "glUniform2ivARB" );
			qglUniform3ivARB = ( PFNGLUNIFORM3IVARBPROC ) qwglGetProcAddress( "glUniform3ivARB" );
			qglUniform4ivARB = ( PFNGLUNIFORM4IVARBPROC ) qwglGetProcAddress( "glUniform4ivARB" );
			qglUniformMatrix2fvARB = ( PFNGLUNIFORMMATRIX2FVARBPROC ) qwglGetProcAddress( "glUniformMatrix2fvARB" );
			qglUniformMatrix3fvARB = ( PFNGLUNIFORMMATRIX3FVARBPROC ) qwglGetProcAddress( "glUniformMatrix3fvARB" );
			qglUniformMatrix4fvARB = ( PFNGLUNIFORMMATRIX4FVARBPROC ) qwglGetProcAddress( "glUniformMatrix4fvARB" );
			qglGetObjectParameterfvARB = ( PFNGLGETOBJECTPARAMETERFVARBPROC ) qwglGetProcAddress( "glGetObjectParameterfvARB" );
			qglGetObjectParameterivARB = ( PFNGLGETOBJECTPARAMETERIVARBPROC ) qwglGetProcAddress( "glGetObjectParameterivARB" );
			qglGetInfoLogARB = ( PFNGLGETINFOLOGARBPROC ) qwglGetProcAddress( "glGetInfoLogARB" );
			qglGetAttachedObjectsARB = ( PFNGLGETATTACHEDOBJECTSARBPROC ) qwglGetProcAddress( "glGetAttachedObjectsARB" );
			qglGetUniformLocationARB = ( PFNGLGETUNIFORMLOCATIONARBPROC ) qwglGetProcAddress( "glGetUniformLocationARB" );
			qglGetActiveUniformARB = ( PFNGLGETACTIVEUNIFORMARBPROC ) qwglGetProcAddress( "glGetActiveUniformARB" );
			qglGetUniformfvARB = ( PFNGLGETUNIFORMFVARBPROC ) qwglGetProcAddress( "glGetUniformfvARB" );
			qglGetUniformivARB = ( PFNGLGETUNIFORMIVARBPROC ) qwglGetProcAddress( "glGetUniformivARB" );
			qglGetShaderSourceARB = ( PFNGLGETSHADERSOURCEARBPROC ) qwglGetProcAddress( "glGetShaderSourceARB" );
			glConfig2.shaderObjectsAvailable = qtrue;
			ri.Printf( PRINT_ALL, "...using GL_ARB_shader_objects\n" );
			} else {
				ri.Printf( PRINT_ALL, "...ignoring GL_ARB_shader_objects\n" );
		}
	} else {
		ri.Printf( PRINT_ALL, "...GL_ARB_shader_objects not found\n" );
	}
	
	// GL_ARB_vertex_shader
	glConfig2.vertexShaderAvailable = qfalse;
	qglBindAttribLocationARB = NULL;
	qglGetActiveAttribARB = NULL;
	qglGetAttribLocationARB = NULL;
	if ( strstr( glConfig.extensions_string, "GL_ARB_vertex_shader" ) ) {
		if ( r_ext_vertex_shader->value ) {
			qglBindAttribLocationARB = ( PFNGLBINDATTRIBLOCATIONARBPROC ) qwglGetProcAddress( "glBindAttribLocationARB" );
			qglGetActiveAttribARB = ( PFNGLGETACTIVEATTRIBARBPROC ) qwglGetProcAddress( "glGetActiveAttribARB" );
			qglGetAttribLocationARB = ( PFNGLGETATTRIBLOCATIONARBPROC ) qwglGetProcAddress( "glGetAttribLocationARB" );
			glConfig2.vertexShaderAvailable = qtrue;
			ri.Printf( PRINT_ALL, "...using GL_ARB_vertex_shader\n" );
			} else {
				ri.Printf( PRINT_ALL, "...ignoring GL_ARB_vertex_shader\n" );
		}
	} else {
		ri.Printf( PRINT_ALL, "...GL_ARB_vertex_shader not found\n" );
	}
	
	// GL_ARB_fragment_shader
	glConfig2.fragmentShaderAvailable = qfalse;
	if ( strstr( glConfig.extensions_string, "GL_ARB_fragment_shader" ) ) {
		if ( r_ext_fragment_shader->value ) {
			glConfig2.fragmentShaderAvailable = qtrue;
			ri.Printf( PRINT_ALL, "...using GL_ARB_fragment_shader\n" );
			} else {
				ri.Printf( PRINT_ALL, "...ignoring GL_ARB_fragment_shader\n" );
		}
	} else {
		ri.Printf( PRINT_ALL, "...GL_ARB_fragment_shader not found\n" );
	}
	
	// GL_ARB_shading_language_100
	glConfig2.shadingLanguage100Available = qfalse;
	if ( strstr( glConfig.extensions_string, "GL_ARB_shading_language_100" ) ) {
		if ( r_ext_shading_language_100->value ) {
			glConfig2.shadingLanguage100Available = qtrue;
			ri.Printf( PRINT_ALL, "...using GL_ARB_shading_language_100\n" );
			} else {
				ri.Printf( PRINT_ALL, "...ignoring GL_ARB_shading_language_100\n" );
		}
	} else {
		ri.Printf( PRINT_ALL, "...GL_ARB_shading_language_100 not found\n" );
	}

        // GL_EXT_compiled_vertex_array
        qglLockArraysEXT = NULL;
        qglUnlockArraysEXT = NULL;
        if ( strstr( glConfig.extensions_string, "GL_EXT_compiled_vertex_array" ) && ( glConfig.hardwareType != GLHW_RIVA128 ) )
        {
                if ( r_ext_compiled_vertex_array->integer )
                {
                        ri.Printf( PRINT_ALL, "...using GL_EXT_compiled_vertex_array\n" );
                        qglLockArraysEXT = ( void ( APIENTRY * )( GLint, GLint ) ) qwglGetProcAddress( "glLockArraysEXT" );
                        qglUnlockArraysEXT = ( void ( APIENTRY * )( void ) ) qwglGetProcAddress( "glUnlockArraysEXT" );
                        if (!qglLockArraysEXT || !qglUnlockArraysEXT) {
                                ri.Error (ERR_FATAL, "bad getprocaddress\n");
                        }
                }
                else
                {
                        ri.Printf( PRINT_ALL, "...ignoring GL_EXT_compiled_vertex_array\n" );
                }
        }
        else
        {
                ri.Printf( PRINT_ALL, "...GL_EXT_compiled_vertex_array not found\n" );
        }

#ifdef GL_APPLE_transform_hint
        if ( strstr( glConfig.extensions_string, "GL_APPLE_transform_hint" )  ) {
            r_appleTransformHint = ri.Cvar_Get("r_appleTransformHint", "1", CVAR_ARCHIVE );
            if (r_appleTransformHint->value) {
                ri.Printf( PRINT_ALL, "...using GL_APPLE_transform_hint\n");
                qglHint(GL_TRANSFORM_HINT_APPLE, GL_FASTEST);
                CheckErrors();
            } else {
                ri.Printf( PRINT_ALL, "...ignoring using GL_APPLE_transform_hint\n");
            }
        } else {
            ri.Printf( PRINT_ALL, "...GL_APPLE_transform_hint not found\n" );
        }
#endif
	
	// GL_EXT_stencil_wrap
	glConfig2.stencilWrapAvailable = qfalse;
	if(strstr(glConfig.extensions_string, "GL_EXT_stencil_wrap"))
	{
		if(r_ext_stencil_wrap->value)
		{
			glConfig2.stencilWrapAvailable = qtrue;
			ri.Printf(PRINT_ALL, "...using GL_EXT_stencil_wrap\n");
		}
		else
		{
			ri.Printf(PRINT_ALL, "...ignoring GL_EXT_stencil_wrap\n");
		}
	}
	else
	{
		ri.Printf(PRINT_ALL, "...GL_EXT_stencil_wrap not found\n");
	}

	// GL_EXT_texture_filter_anisotropic
	glConfig2.textureAnisotropyAvailable = qfalse;
	if(strstr(glConfig.extensions_string, "GL_EXT_texture_filter_anisotropic"))
	{
		qglGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &glConfig2.maxTextureAnisotropy);
	
		if(r_ext_texture_filter_anisotropic->value)
		{
			glConfig2.textureAnisotropyAvailable = qtrue;
			ri.Printf(PRINT_ALL, "...using GL_EXT_texture_filter_anisotropic\n");
		}
		else
		{
			ri.Printf(PRINT_ALL, "...ignoring GL_EXT_texture_filter_anisotropic\n");
		}
	}
	else
	{
		ri.Printf(PRINT_ALL, "...GL_EXT_texture_filter_anisotropic not found\n");
	}
	
	// GL_EXT_stencil_two_side
	qglActiveStencilFaceEXT = NULL;
	if(strstr(glConfig.extensions_string, "GL_EXT_stencil_two_side"))
	{
		if(r_ext_stencil_two_side->value)
		{
			qglActiveStencilFaceEXT = (void (APIENTRY *) (GLenum))qwglGetProcAddress("glActiveStencilFaceEXT");
			ri.Printf(PRINT_ALL, "...using GL_EXT_stencil_two_side\n");
		}
		else
		{
			ri.Printf(PRINT_ALL, "...ignoring GL_EXT_stencil_two_side\n");
		}
	}
	else
	{
		ri.Printf(PRINT_ALL, "...GL_EXT_stencil_two_side not found\n");
	}
	
	// GL_EXT_depth_bounds_test
	qglDepthBoundsEXT = NULL;
	if(strstr(glConfig.extensions_string, "GL_EXT_depth_bounds_test"))
	{
		if(r_ext_depth_bounds_test->value)
		{
			qglDepthBoundsEXT = (PFNGLDEPTHBOUNDSEXTPROC)qwglGetProcAddress("glDepthBoundsEXT");
			ri.Printf(PRINT_ALL, "...using GL_EXT_depth_bounds_test\n");
		}
		else
		{
			ri.Printf(PRINT_ALL, "...ignoring GL_EXT_depth_bounds_test\n");
		}
	}
	else
	{
		ri.Printf(PRINT_ALL, "...GL_EXT_depth_bounds_test not found\n");
	}

	// GL_EXT_framebuffer_object
	glConfig2.framebufferObjectAvailable = qfalse;
	qglIsRenderbufferEXT = NULL;
	qglBindRenderbufferEXT = NULL;
	qglDeleteRenderbuffersEXT = NULL;
	qglGenRenderbuffersEXT = NULL;
	qglRenderbufferStorageEXT = NULL;
	qglGetRenderbufferParameterivEXT = NULL;
	qglIsFramebufferEXT = NULL;
	qglBindFramebufferEXT = NULL;
	qglDeleteFramebuffersEXT = NULL;
	qglGenFramebuffersEXT = NULL;
	qglCheckFramebufferStatusEXT = NULL;
	qglFramebufferTexture1DEXT = NULL;
	qglFramebufferTexture2DEXT = NULL;
	qglFramebufferTexture3DEXT = NULL;
	qglFramebufferRenderbufferEXT = NULL;
	qglGetFramebufferAttachmentParameterivEXT = NULL;
	qglGenerateMipmapEXT = NULL;
	if ( strstr( glConfig.extensions_string, "GL_EXT_framebuffer_object" ) ) {
		if ( r_ext_framebuffer_object->value ) {
			qglIsRenderbufferEXT = ( PFNGLISRENDERBUFFEREXTPROC ) qwglGetProcAddress( "glIsRenderbufferEXT" );
			qglBindRenderbufferEXT = ( PFNGLBINDRENDERBUFFEREXTPROC ) qwglGetProcAddress( "glBindRenderbufferEXT" );
			qglDeleteRenderbuffersEXT = ( PFNGLDELETERENDERBUFFERSEXTPROC ) qwglGetProcAddress( "glDeleteRenderbuffersEXT" );
			qglGenRenderbuffersEXT = ( PFNGLGENRENDERBUFFERSEXTPROC ) qwglGetProcAddress( "glGenRenderbuffersEXT" );
			qglRenderbufferStorageEXT = ( PFNGLRENDERBUFFERSTORAGEEXTPROC ) qwglGetProcAddress( "glRenderbufferStorageEXT" );
			qglGetRenderbufferParameterivEXT = ( PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC ) qwglGetProcAddress( "glGetRenderbufferParameterivEXT" );
			qglIsFramebufferEXT = ( PFNGLISFRAMEBUFFEREXTPROC ) qwglGetProcAddress( "glIsFramebufferEXT" );
			qglBindFramebufferEXT = ( PFNGLBINDFRAMEBUFFEREXTPROC ) qwglGetProcAddress( "glBindFramebufferEXT" );
			qglDeleteFramebuffersEXT = ( PFNGLDELETEFRAMEBUFFERSEXTPROC ) qwglGetProcAddress( "glDeleteFramebuffersEXT" );
			qglGenFramebuffersEXT = ( PFNGLGENFRAMEBUFFERSEXTPROC ) qwglGetProcAddress( "glGenFramebuffersEXT" );
			qglCheckFramebufferStatusEXT = ( PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC ) qwglGetProcAddress( "glCheckFramebufferStatusEXT" );
			qglFramebufferTexture1DEXT = ( PFNGLFRAMEBUFFERTEXTURE1DEXTPROC ) qwglGetProcAddress( "glFramebufferTexture1DEXT" );
			qglFramebufferTexture2DEXT = ( PFNGLFRAMEBUFFERTEXTURE2DEXTPROC ) qwglGetProcAddress( "glFramebufferTexture2DEXT" );
			qglFramebufferTexture3DEXT = ( PFNGLFRAMEBUFFERTEXTURE3DEXTPROC ) qwglGetProcAddress( "glFramebufferTexture3DEXT" );
			qglFramebufferRenderbufferEXT = ( PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC ) qwglGetProcAddress( "glFramebufferRenderbufferEXT" );
			qglGetFramebufferAttachmentParameterivEXT = ( PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC ) qwglGetProcAddress( "glGetFramebufferAttachmentParameterivEXT" );
			qglGenerateMipmapEXT = ( PFNGLGENERATEMIPMAPEXTPROC ) qwglGetProcAddress( "glGenerateMipmapEXT" );
			glConfig2.framebufferObjectAvailable = qtrue;
			ri.Printf( PRINT_ALL, "...using GL_EXT_framebuffer_object\n" );
		} else {
			ri.Printf( PRINT_ALL, "...ignoring GL_EXT_framebuffer_object\n" );
		}
	} else {
		ri.Printf( PRINT_ALL, "...GL_EXT_framebuffer_object not found\n" );
	}
}


#define MAX_RENDERER_INFO_COUNT 128

// Returns zero if there are no hardware renderers.  Otherwise, returns the max memory across all renderers (on the presumption that the screen that we'll use has the most memory).
static unsigned long Sys_QueryVideoMemory()
{
    CGLError err;
    CGLRendererInfoObj rendererInfo, rendererInfos[MAX_RENDERER_INFO_COUNT];
    long rendererInfoIndex, rendererInfoCount = MAX_RENDERER_INFO_COUNT;
    long rendererIndex, rendererCount;
    long maxVRAM = 0, vram;
    long accelerated;
    long rendererID;
    long totalRenderers = 0;
    
    err = CGLQueryRendererInfo(CGDisplayIDToOpenGLDisplayMask(Sys_DisplayToUse()), rendererInfos, &rendererInfoCount);
    if (err) {
        Com_Printf("CGLQueryRendererInfo -> %d\n", err);
        return vram;
    }
    
    //Com_Printf("rendererInfoCount = %d\n", rendererInfoCount);
    for (rendererInfoIndex = 0; rendererInfoIndex < rendererInfoCount && totalRenderers < rendererInfoCount; rendererInfoIndex++) {
        rendererInfo = rendererInfos[rendererInfoIndex];
        //Com_Printf("rendererInfo: 0x%08x\n", rendererInfo);
        

        err = CGLDescribeRenderer(rendererInfo, 0, kCGLRPRendererCount, &rendererCount);
        if (err) {
            Com_Printf("CGLDescribeRenderer(kCGLRPRendererID) -> %d\n", err);
            continue;
        }
        //Com_Printf("  rendererCount: %d\n", rendererCount);

        for (rendererIndex = 0; rendererIndex < rendererCount; rendererIndex++) {
            totalRenderers++;
            //Com_Printf("  rendererIndex: %d\n", rendererIndex);
            
            rendererID = 0xffffffff;
            err = CGLDescribeRenderer(rendererInfo, rendererIndex, kCGLRPRendererID, &rendererID);
            if (err) {
                Com_Printf("CGLDescribeRenderer(kCGLRPRendererID) -> %d\n", err);
                continue;
            }
            //Com_Printf("    rendererID: 0x%08x\n", rendererID);
            
            accelerated = 0;
            err = CGLDescribeRenderer(rendererInfo, rendererIndex, kCGLRPAccelerated, &accelerated);
            if (err) {
                Com_Printf("CGLDescribeRenderer(kCGLRPAccelerated) -> %d\n", err);
                continue;
            }
            //Com_Printf("    accelerated: %d\n", accelerated);
            if (!accelerated)
                continue;
            
            vram = 0;
            err = CGLDescribeRenderer(rendererInfo, rendererIndex, kCGLRPVideoMemory, &vram);
            if (err) {
                Com_Printf("CGLDescribeRenderer -> %d\n", err);
                continue;
            }
            //Com_Printf("    vram: 0x%08x\n", vram);
            
            // presumably we'll be running on the best card, so we'll take the max of the vrams
            if (vram > maxVRAM)
                maxVRAM = vram;
        }
        
#if 0
        err = CGLDestroyRendererInfo(rendererInfo);
        if (err) {
            Com_Printf("CGLDestroyRendererInfo -> %d\n", err);
        }
#endif
    }

    return maxVRAM;
}


// We will set the Sys_IsHidden global to cause input to be handle differently (we'll just let NSApp handle events in this case).  We also will unbind the GL context and restore the video mode.
qboolean Sys_Hide()
{
    if (Sys_IsHidden)
        // Eh?
        return qfalse;
    
    if (!r_fullscreen->integer)
        // We only support hiding in fullscreen mode right now
        return qfalse;
    
    Sys_IsHidden = qtrue;

    // Don't need to store the current gamma since we always keep it around in glw_state.inGameTable.

    Sys_FadeScreen(Sys_DisplayToUse());

    // Disassociate the GL context from the screen
    // Have to call both to actually deallocate kernel resources and free the NSSurface
    CGLClearDrawable(OSX_GetCGLContext());
    [OSX_GetNSGLContext() clearDrawable];
    
    // Restore the original video mode
    _GLimp_RestoreOriginalVideoSettings();

    // Restore the original gamma if needed.
    if (glConfig.deviceSupportsGamma) {
        CGDisplayRestoreColorSyncSettings();
    }

    // Release the screen(s)
    ReleaseAllDisplays();
    
    Sys_UnfadeScreens();
    
    // Shut down the input system so the mouse and keyboard settings are restore to normal
    Sys_ShutdownInput();
    
    // Hide the application so that when the user clicks on our app icon, we'll get an unhide notification
    [NSApp hide: nil];
    
    return qtrue;
}

static CGDisplayErr Sys_CaptureActiveDisplays(void)
{
    CGDisplayErr err;
    CGDisplayCount displayIndex;
    for (displayIndex = 0; displayIndex < glw_state.displayCount; displayIndex++) {
	const glwgamma_t *table;
	table = &glw_state.originalDisplayGammaTables[displayIndex];
	err = CGDisplayCapture(table->display);
	if (err != CGDisplayNoErr)
	    return err;
    }
    return CGDisplayNoErr;
}

qboolean Sys_Unhide()
{
    CGDisplayErr err;
    CGLError glErr;
    
    if (!Sys_IsHidden)
        // Eh?
        return qfalse;
        
    Sys_FadeScreens();

    // Capture the screen(s)
    err = Sys_CaptureActiveDisplays();
    if (err != CGDisplayNoErr) {
        Sys_UnfadeScreens();
        ri.Printf( PRINT_ALL, "Unhide failed -- cannot capture the display again.\n" );
        return qfalse;
    }
    
    // Restore the game mode
    err = CGDisplaySwitchToMode(glw_state.display, (CFDictionaryRef)glw_state.gameMode);
    if ( err != CGDisplayNoErr ) {
        ReleaseAllDisplays();
        Sys_UnfadeScreens();
        ri.Printf( PRINT_ALL, "Unhide failed -- Unable to set display mode\n" );
        return qfalse;
    }

    // Reassociate the GL context and the screen
    glErr = CGLSetFullScreen(OSX_GetCGLContext());
    if (err) {
        ReleaseAllDisplays();
        Sys_UnfadeScreens();
        ri.Printf( PRINT_ALL, "Unhide failed: CGLSetFullScreen -> %d (%s)\n", err, CGLErrorString(err));
        return qfalse;
    }

    // Restore the current context
    [OSX_GetNSGLContext() makeCurrentContext];
    
    // Restore the gamma that the game had set
    Sys_UnfadeScreen(Sys_DisplayToUse(), &glw_state.inGameTable);
    
    // Restore the input system (last so if something goes wrong we don't eat the mouse)
    Sys_InitInput();
    
    Sys_IsHidden = qfalse;
    return qtrue;
}


