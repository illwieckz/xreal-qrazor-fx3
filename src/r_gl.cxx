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
// system -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
#include "r_local.h"



/// OpenGL 1.1 functions =======================================================
void (GLAPIENTRY* xglBegin) (GLenum mode);
void (GLAPIENTRY* xglColor4f) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void (GLAPIENTRY* xglColor4fv) (const GLfloat *v);
void (GLAPIENTRY* xglEnd) (void);
void (GLAPIENTRY* xglTexCoord1f) (GLfloat s);
void (GLAPIENTRY* xglTexCoord1fv) (const GLfloat *v);
void (GLAPIENTRY* xglTexCoord2f) (GLfloat s, GLfloat t);
void (GLAPIENTRY* xglTexCoord2fv) (const GLfloat *v);
void (GLAPIENTRY* xglTexCoord3f) (GLfloat s, GLfloat t, GLfloat r);
void (GLAPIENTRY* xglTexCoord3fv) (const GLfloat *v);
void (GLAPIENTRY* xglVertex3f) (GLfloat x, GLfloat y, GLfloat z);
void (GLAPIENTRY* xglVertex3fv) (const GLfloat *v);
void (GLAPIENTRY* xglCullFace) (GLenum mode);
void (GLAPIENTRY* xglFrontFace) (GLenum mode);
void (GLAPIENTRY* xglPolygonMode) (GLenum face, GLenum mode);
void (GLAPIENTRY* xglScissor) (GLint x, GLint y, GLsizei width, GLsizei height);
void (GLAPIENTRY* xglTexParameterfv) (GLenum target, GLenum pname, const GLfloat *params);
void (GLAPIENTRY* xglTexParameteri) (GLenum target, GLenum pname, GLint param);
void (GLAPIENTRY* xglTexParameteriv) (GLenum target, GLenum pname, const GLint *params);
void (GLAPIENTRY* xglTexImage1D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void (GLAPIENTRY* xglTexImage2D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void (GLAPIENTRY* xglTexEnvi) (GLenum target, GLenum pname, GLint param);
void (GLAPIENTRY* xglTexGenf) (GLenum coord, GLenum pname, GLfloat param);
void (GLAPIENTRY* xglTexGenfv) (GLenum coord, GLenum pname, const GLfloat *params);
void (GLAPIENTRY* xglTexGeni) (GLenum coord, GLenum pname, GLint param);
void (GLAPIENTRY* xglTexGeniv) (GLenum coord, GLenum pname, const GLint *params);
void (GLAPIENTRY* xglDrawBuffer) (GLenum mode);
void (GLAPIENTRY* xglClear) (GLbitfield mask);
void (GLAPIENTRY* xglClearColor) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
void (GLAPIENTRY* xglClearStencil) (GLint s);
void (GLAPIENTRY* xglColorMask) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
void (GLAPIENTRY* xglDepthMask) (GLboolean flag);
void (GLAPIENTRY* xglDisable) (GLenum cap);
void (GLAPIENTRY* xglEnable) (GLenum cap);
void (GLAPIENTRY* xglFlush) (void);
void (GLAPIENTRY* xglPopAttrib) (void);
void (GLAPIENTRY* xglPushAttrib) (GLbitfield mask);
void (GLAPIENTRY* xglAlphaFunc) (GLenum func, GLclampf ref);
void (GLAPIENTRY* xglBlendFunc) (GLenum sfactor, GLenum dfactor);
void (GLAPIENTRY* xglStencilFunc) (GLenum func, GLint ref, GLuint mask);
void (GLAPIENTRY* xglStencilOp) (GLenum fail, GLenum zfail, GLenum zpass);
void (GLAPIENTRY* xglDepthFunc) (GLenum func);
void (GLAPIENTRY* xglReadPixels) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
GLenum (GLAPIENTRY* xglGetError) (void);
void (GLAPIENTRY* xglGetFloatv) (GLenum pname, GLfloat *params);
void (GLAPIENTRY* xglGetIntegerv) (GLenum pname, GLint *params);
const GLubyte* (GLAPIENTRY* xglGetString) (GLenum name);
void (GLAPIENTRY* xglDepthRange) (GLclampd zNear, GLclampd zFar);
//void (GLAPIENTRY* xglFrustum) (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
void (GLAPIENTRY* xglLoadIdentity) (void);
//void (GLAPIENTRY* xglLoadMatrixf) (const GLfloat *m);
void (GLAPIENTRY* xglMatrixMode) (GLenum mode);
void (GLAPIENTRY* xglOrtho) (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
void (GLAPIENTRY* xglPopMatrix) (void);
void (GLAPIENTRY* xglPushMatrix) (void);
//void (GLAPIENTRY* xglRotatef) (GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
//void (GLAPIENTRY* xglScalef) (GLfloat x, GLfloat y, GLfloat z);
//void (GLAPIENTRY* xglTranslatef) (GLfloat x, GLfloat y, GLfloat z);
void (GLAPIENTRY* xglViewport) (GLint x, GLint y, GLsizei width, GLsizei height);
void (GLAPIENTRY* xglDisableClientState) (GLenum array);
void (GLAPIENTRY* xglDrawElements) (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
void (GLAPIENTRY* xglEnableClientState) (GLenum array);
void (GLAPIENTRY* xglIndexPointer) (GLenum type, GLsizei stride, const GLvoid *pointer);
void (GLAPIENTRY* xglNormalPointer) (GLenum type, GLsizei stride, const GLvoid *pointer);
void (GLAPIENTRY* xglTexCoordPointer) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void (GLAPIENTRY* xglVertexPointer) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void (GLAPIENTRY* xglPolygonOffset) (GLfloat factor, GLfloat units);
void (GLAPIENTRY* xglBindTexture) (GLenum target, GLuint texture);
void (GLAPIENTRY* xglDeleteTextures) (GLsizei n, const GLuint *textures);
void (GLAPIENTRY* xglGenTextures) (GLsizei n, GLuint *textures);

/// OpenGL 1.2 functions =======================================================
void (GLAPIENTRY* xglDrawRangeElements) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);
void (GLAPIENTRY* xglTexImage3D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);

/// OpenGL 1.3 functions =======================================================
void (GLAPIENTRY* xglActiveTexture) (GLenum texture);
void (GLAPIENTRY* xglClientActiveTexture) (GLenum texture);
void (GLAPIENTRY* xglLoadTransposeMatrixf) (const GLfloat *m);


/// GL_ARB_vertex_program ======================================================
void (GLAPIENTRY* xglVertexAttribPointerARB) (GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid *);
void (GLAPIENTRY* xglEnableVertexAttribArrayARB) (GLuint);
void (GLAPIENTRY* xglDisableVertexAttribArrayARB) (GLuint);
void (GLAPIENTRY* xglProgramStringARB) (GLenum, GLenum, GLsizei, const GLvoid *);
void (GLAPIENTRY* xglBindProgramARB) (GLenum, GLuint);
void (GLAPIENTRY* xglDeleteProgramsARB) (GLsizei, const GLuint *);
void (GLAPIENTRY* xglGenProgramsARB) (GLsizei, GLuint *);
void (GLAPIENTRY* xglProgramEnvParameter4fARB) (GLenum, GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
void (GLAPIENTRY* xglProgramEnvParameter4fvARB) (GLenum, GLuint, const GLfloat *);
void (GLAPIENTRY* xglProgramLocalParameter4fARB) (GLenum, GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
void (GLAPIENTRY* xglProgramLocalParameter4fvARB) (GLenum, GLuint, const GLfloat *);
void (GLAPIENTRY* xglGetProgramEnvParameterfvARB) (GLenum, GLuint, GLfloat *);
void (GLAPIENTRY* xglGetProgramLocalParameterfvARB) (GLenum, GLuint, GLfloat *);
void (GLAPIENTRY* xglGetProgramivARB) (GLenum, GLenum, GLint *);
void (GLAPIENTRY* xglGetProgramStringARB) (GLenum, GLenum, GLvoid *);
void (GLAPIENTRY* xglGetVertexAttribfvARB) (GLuint, GLenum, GLfloat *);
void (GLAPIENTRY* xglGetVertexAttribPointervARB) (GLuint, GLenum, GLvoid* *);
GLboolean (GLAPIENTRY* xglIsProgramARB) (GLuint);


/// GL_ARB_vertex_buffer_object ================================================
void (GLAPIENTRY* xglBindBufferARB) (GLenum, GLuint);
void (GLAPIENTRY* xglDeleteBuffersARB) (GLsizei, const GLuint *);
void (GLAPIENTRY* xglGenBuffersARB) (GLsizei, GLuint *);
GLboolean (GLAPIENTRY* xglIsBufferARB) (GLuint);
void (GLAPIENTRY* xglBufferDataARB) (GLenum, GLsizeiptrARB, const GLvoid *, GLenum);
void (GLAPIENTRY* xglBufferSubDataARB) (GLenum, GLintptrARB, GLsizeiptrARB, const GLvoid *);
void (GLAPIENTRY* xglGetBufferSubDataARB) (GLenum, GLintptrARB, GLsizeiptrARB, GLvoid *);
GLvoid* (GLAPIENTRY* xglMapBufferARB) (GLenum, GLenum);
GLboolean (GLAPIENTRY* xglUnmapBufferARB) (GLenum);
void (GLAPIENTRY* xglGetBufferParameterivARB) (GLenum, GLenum, GLint *);
void (GLAPIENTRY* xglGetBufferPointervARB) (GLenum, GLenum, GLvoid* *);


/// GL_ARB_shader_objects ======================================================
typedef char GLcharARB;			/* native character */
typedef unsigned int GLhandleARB;	/* shader object handle */

void (GLAPIENTRY* xglDeleteObjectARB) (GLhandleARB);
GLhandleARB (GLAPIENTRY* xglGetHandleARB) (GLenum);
void (GLAPIENTRY* xglDetachObjectARB) (GLhandleARB, GLhandleARB);
GLhandleARB (GLAPIENTRY* xglCreateShaderObjectARB) (GLenum);
void (GLAPIENTRY* xglShaderSourceARB) (GLhandleARB, GLsizei, const GLcharARB* *, const GLint *);
void (GLAPIENTRY* xglCompileShaderARB) (GLhandleARB);
GLhandleARB (GLAPIENTRY* xglCreateProgramObjectARB) (void);
void (GLAPIENTRY* xglAttachObjectARB) (GLhandleARB, GLhandleARB);
void (GLAPIENTRY* xglLinkProgramARB) (GLhandleARB);
void (GLAPIENTRY* xglUseProgramObjectARB) (GLhandleARB);
void (GLAPIENTRY* xglValidateProgramARB) (GLhandleARB);
void (GLAPIENTRY* xglUniform1fARB) (GLint, GLfloat);
void (GLAPIENTRY* xglUniform2fARB) (GLint, GLfloat, GLfloat);
void (GLAPIENTRY* xglUniform3fARB) (GLint, GLfloat, GLfloat, GLfloat);
void (GLAPIENTRY* xglUniform4fARB) (GLint, GLfloat, GLfloat, GLfloat, GLfloat);
void (GLAPIENTRY* xglUniform1iARB) (GLint, GLint);
void (GLAPIENTRY* xglUniform2iARB) (GLint, GLint, GLint);
void (GLAPIENTRY* xglUniform3iARB) (GLint, GLint, GLint, GLint);
void (GLAPIENTRY* xglUniform4iARB) (GLint, GLint, GLint, GLint, GLint);
void (GLAPIENTRY* xglUniform1fvARB) (GLint, GLsizei, const GLfloat *);
void (GLAPIENTRY* xglUniform2fvARB) (GLint, GLsizei, const GLfloat *);
void (GLAPIENTRY* xglUniform3fvARB) (GLint, GLsizei, const GLfloat *);
void (GLAPIENTRY* xglUniform4fvARB) (GLint, GLsizei, const GLfloat *);
void (GLAPIENTRY* xglUniform1ivARB) (GLint, GLsizei, const GLint *);
void (GLAPIENTRY* xglUniform2ivARB) (GLint, GLsizei, const GLint *);
void (GLAPIENTRY* xglUniform3ivARB) (GLint, GLsizei, const GLint *);
void (GLAPIENTRY* xglUniform4ivARB) (GLint, GLsizei, const GLint *);
void (GLAPIENTRY* xglUniformMatrix2fvARB) (GLint, GLsizei, GLboolean, const GLfloat *);
void (GLAPIENTRY* xglUniformMatrix3fvARB) (GLint, GLsizei, GLboolean, const GLfloat *);
void (GLAPIENTRY* xglUniformMatrix4fvARB) (GLint, GLsizei, GLboolean, const GLfloat *);
void (GLAPIENTRY* xglGetObjectParameterfvARB) (GLhandleARB, GLenum, GLfloat *);
void (GLAPIENTRY* xglGetObjectParameterivARB) (GLhandleARB, GLenum, GLint *);
void (GLAPIENTRY* xglGetInfoLogARB) (GLhandleARB, GLsizei, GLsizei *, GLcharARB *);
void (GLAPIENTRY* xglGetAttachedObjectsARB) (GLhandleARB, GLsizei, GLsizei *, GLhandleARB *);
GLint (GLAPIENTRY* xglGetUniformLocationARB) (GLhandleARB, const GLcharARB *);
void (GLAPIENTRY* xglGetActiveUniformARB) (GLhandleARB, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLcharARB *);
void (GLAPIENTRY* xglGetUniformfvARB) (GLhandleARB, GLint, GLfloat *);
void (GLAPIENTRY* xglGetUniformivARB) (GLhandleARB, GLint, GLint *);
void (GLAPIENTRY* xglGetShaderSourceARB) (GLhandleARB, GLsizei, GLsizei *, GLcharARB *);


/// GL_ARB_vertex_shader =======================================================
void (GLAPIENTRY* xglBindAttribLocationARB) (GLhandleARB, GLuint, const GLcharARB *);
void (GLAPIENTRY* xglGetActiveAttribARB) (GLhandleARB, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLcharARB *);
GLint (GLAPIENTRY* xglGetAttribLocationARB) (GLhandleARB, const GLcharARB *);


/// GL_EXT_compiled_vertex_array ===============================================
void (GLAPIENTRY* xglLockArraysEXT) (GLint, GLsizei);
void (GLAPIENTRY* xglUnlockArraysEXT) (void);


/// GL_NV_vertex_program2 ======================================================
GLboolean (GLAPIENTRY* xglAreProgramsResidentNV) (GLsizei, const GLuint *, GLboolean *);
void (GLAPIENTRY* xglBindProgramNV) (GLenum, GLuint);
void (GLAPIENTRY* xglDeleteProgramsNV) (GLsizei, const GLuint *);
void (GLAPIENTRY* xglExecuteProgramNV) (GLenum, GLuint, const GLfloat *);
void (GLAPIENTRY* xglGenProgramsNV) (GLsizei, GLuint *);
void (GLAPIENTRY* xglGetProgramParameterdvNV) (GLenum, GLuint, GLenum, GLdouble *);
void (GLAPIENTRY* xglGetProgramParameterfvNV) (GLenum, GLuint, GLenum, GLfloat *);
void (GLAPIENTRY* xglGetProgramivNV) (GLuint, GLenum, GLint *);
void (GLAPIENTRY* xglGetProgramStringNV) (GLuint, GLenum, GLubyte *);
void (GLAPIENTRY* xglGetTrackMatrixivNV) (GLenum, GLuint, GLenum, GLint *);
void (GLAPIENTRY* xglGetVertexAttribdvNV) (GLuint, GLenum, GLdouble *);
void (GLAPIENTRY* xglGetVertexAttribfvNV) (GLuint, GLenum, GLfloat *);
void (GLAPIENTRY* xglGetVertexAttribivNV) (GLuint, GLenum, GLint *);
void (GLAPIENTRY* xglGetVertexAttribPointervNV) (GLuint, GLenum, GLvoid* *);
GLboolean (GLAPIENTRY* xglIsProgramNV) (GLuint);
void (GLAPIENTRY* xglLoadProgramNV) (GLenum, GLuint, GLsizei, const GLubyte *);
void (GLAPIENTRY* xglProgramParameter4fNV) (GLenum, GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
void (GLAPIENTRY* xglProgramParameter4fvNV) (GLenum, GLuint, const GLfloat *);
void (GLAPIENTRY* xglProgramParameters4fvNV) (GLenum, GLuint, GLuint, const GLfloat *);
void (GLAPIENTRY* xglRequestResidentProgramsNV) (GLsizei, const GLuint *);
void (GLAPIENTRY* xglTrackMatrixNV) (GLenum, GLuint, GLenum, GLenum);
void (GLAPIENTRY* xglVertexAttribPointerNV) (GLuint, GLint, GLenum, GLsizei, const GLvoid *);
void (GLAPIENTRY* xglVertexAttrib1fNV) (GLuint, GLfloat);
void (GLAPIENTRY* xglVertexAttrib1fvNV) (GLuint, const GLfloat *);
void (GLAPIENTRY* xglVertexAttrib2fNV) (GLuint, GLfloat, GLfloat);
void (GLAPIENTRY* xglVertexAttrib2fvNV) (GLuint, const GLfloat *);
void (GLAPIENTRY* xglVertexAttrib3fNV) (GLuint, GLfloat, GLfloat, GLfloat);
void (GLAPIENTRY* xglVertexAttrib3fvNV) (GLuint, const GLfloat *);
void (GLAPIENTRY* xglVertexAttrib4fNV) (GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
void (GLAPIENTRY* xglVertexAttrib4fvNV) (GLuint, const GLfloat *);
void (GLAPIENTRY* xglVertexAttribs1fvNV) (GLuint, GLsizei, const GLfloat *);
void (GLAPIENTRY* xglVertexAttribs2fvNV) (GLuint, GLsizei, const GLfloat *);
void (GLAPIENTRY* xglVertexAttribs3fvNV) (GLuint, GLsizei, const GLfloat *);
void (GLAPIENTRY* xglVertexAttribs4fvNV) (GLuint, GLsizei, const GLfloat *);


/// GL_NV_fragment_program =====================================================
void (GLAPIENTRY* xglProgramNamedParameter4fNV) (GLuint, GLsizei, const GLubyte *, GLfloat, GLfloat, GLfloat, GLfloat);
void (GLAPIENTRY* xglProgramNamedParameter4fvNV) (GLuint, GLsizei, const GLubyte *, const GLfloat *);
void (GLAPIENTRY* xglGetProgramNamedParameterfvNV) (GLuint, GLsizei, const GLubyte *, GLfloat *);


#ifdef __linux__
/// GLX 1.0 functions ==========================================================
XVisualInfo* 		(*xglXChooseVisual)(Display *dpy, int screen, int *attribList);
void 			(*xglXCopyContext)(Display *dpy, GLXContext src, GLXContext dst, GLuint mask);
GLXContext		(*xglXCreateContext)(Display *dpy, XVisualInfo *vis, GLXContext shareList, Bool direct);
void 			(*xglXDestroyContext)(Display *dpy, GLXContext ctx);
GLXDrawable		(*xglXGetCurrentDrawable)(void);
Bool 			(*xglXMakeCurrent)(Display *dpy, GLXDrawable drawable, GLXContext ctx);
int 			(*xglXGetConfig)(Display *dpy, XVisualInfo *vis, int attrib, int *value);
GLXContext		(*xglXGetCurrentContext)(void);
Bool 			(*xglXQueryExtension)(Display *dpy, int *error_base, int *event_base);
Bool			(*xglXQueryVersion)(Display *dpy, int *major, int *minor);
void 			(*xglXSwapBuffers)(Display *dpy, GLXDrawable drawable);

/// GLX 1.1 functions ==========================================================
const char*		(*xglXQueryExtensionsString)(Display *dpy, int screen);

/// GLX 1.2 functions ==========================================================
Display*		(*xglXGetCurrentDisplay)(void);

/// GLX 1.3 functions ==========================================================
GLXFBConfig*		(*xglXGetFBConfigs)(Display *dpy, int screen, int *nelements);

/// SGIX_fbconfig functions ====================================================
GLXFBConfigSGIX*	(*xglXChooseFBConfigSGIX)(Display *dpy, int screen, const int *attrib_list, int *nelements);
GLXContext		(*xglXCreateContextWithConfigSGIX)(Display *dpy, GLXFBConfigSGIX config, int render_type, GLXContext share_list, Bool direct);

/// SGIX_pbuffer functions =====================================================
GLXPbuffer		(*xglXCreateGLXPbufferSGIX)(Display *dpy, GLXFBConfig config, unsigned int width, unsigned int height, const int *attrib_list);
void			(*xglXQueryGLXPbufferSGIX)(Display *dpy, GLXPbuffer pbuf, int attribute, unsigned int *value);

#endif	// __linux__

#ifdef _WIN32
/// WGL functions =============================================================
extern BOOL		(*xglMakeCurrent)(HDC, HGLRC);
extern BOOL		(*xglDeleteContext)(HGLRC);
extern HGLRC		(*xglCreateContext)(HDC);
#endif // _WIN32

/*
** XGL_Shutdown
**
** Unloads the specified DLL then nulls out all the proc pointers.
*/
void	XGL_Shutdown()
{
	ri.Com_Printf("------- XGL_Shutdown -------\n");
	
#ifdef __linux__
	if(sys_gl.OpenGLLib)
	{
		dlclose(sys_gl.OpenGLLib);
	}
#endif

#ifdef _WIN32
	if(sys_gl.OpengGLLib)
	{
		FreeLibrary(sys_gl.OpenGLLib);
	}
#endif

	sys_gl.OpenGLLib 					= NULL;
	
	xglBegin						= NULL;
	xglColor4f						= NULL;
	xglColor4fv						= NULL;
	xglEnd							= NULL;
	xglTexCoord1f						= NULL;
	xglTexCoord1fv						= NULL;
	xglTexCoord2f						= NULL;
	xglTexCoord2fv						= NULL;
	xglTexCoord3f						= NULL;
	xglTexCoord3fv						= NULL;
	xglVertex3f						= NULL;
	xglVertex3fv						= NULL;
	xglCullFace						= NULL;
	xglFrontFace						= NULL;
	xglPolygonMode						= NULL;
	xglScissor						= NULL;
	xglTexParameterfv					= NULL;
	xglTexParameteri					= NULL;
	xglTexParameteriv					= NULL;
	xglTexImage1D						= NULL;
	xglTexImage2D						= NULL;
	xglTexEnvi						= NULL;
	xglTexGenf						= NULL;
	xglTexGenfv						= NULL;
	xglTexGeni						= NULL;
	xglTexGeniv						= NULL;
	xglDrawBuffer						= NULL;
	xglClear						= NULL;
	xglClearColor						= NULL;
	xglClearStencil						= NULL;
	xglColorMask						= NULL;
	xglDepthMask						= NULL;
	xglDisable						= NULL;
	xglEnable						= NULL;
	xglFlush						= NULL;
	xglPopAttrib						= NULL;
	xglPushAttrib						= NULL;
	xglAlphaFunc						= NULL;
	xglBlendFunc						= NULL;
	xglStencilFunc						= NULL;
	xglStencilOp						= NULL;
	xglDepthFunc						= NULL;
	xglReadPixels						= NULL;
	xglGetError						= NULL;
	xglGetFloatv						= NULL;
	xglGetIntegerv						= NULL;
	xglGetString						= NULL;
	xglDepthRange						= NULL;
//	xglFrustum						= NULL;
	xglLoadIdentity						= NULL;
//	xglLoadMatrixf						= NULL;
	xglMatrixMode						= NULL;
	xglOrtho						= NULL;
	xglPopMatrix						= NULL;
	xglPushMatrix						= NULL;
//	xglRotatef						= NULL;
//	xglScalef						= NULL;
//	xglTranslatef						= NULL;
	xglViewport						= NULL;
	xglDisableClientState					= NULL;
	xglDrawElements						= NULL;
	xglEnableClientState					= NULL;
	xglIndexPointer						= NULL;
	xglNormalPointer					= NULL;
	xglTexCoordPointer					= NULL;
	xglVertexPointer					= NULL;
	xglPolygonOffset					= NULL;
	xglBindTexture						= NULL;
	xglDeleteTextures					= NULL;
	xglGenTextures						= NULL;
	
	xglDrawRangeElements					= NULL;
	xglTexImage3D						= NULL;
	
	xglActiveTexture					= NULL;
	xglClientActiveTexture					= NULL;
	xglLoadTransposeMatrixf					= NULL;
	
	xglVertexAttribPointerARB				= NULL;
	xglEnableVertexAttribArrayARB				= NULL;
	xglDisableVertexAttribArrayARB				= NULL;
	xglProgramStringARB					= NULL;
	xglBindProgramARB					= NULL;
	xglDeleteProgramsARB					= NULL;
	xglGenProgramsARB					= NULL;
	xglProgramEnvParameter4fARB				= NULL;
	xglProgramEnvParameter4fvARB				= NULL;
	xglProgramLocalParameter4fARB				= NULL;
	xglProgramLocalParameter4fvARB				= NULL;
	xglGetProgramEnvParameterfvARB				= NULL;
	xglGetProgramLocalParameterfvARB			= NULL;
	xglGetProgramivARB					= NULL;
	xglGetProgramStringARB					= NULL;
	xglGetVertexAttribfvARB					= NULL;
	xglGetVertexAttribPointervARB				= NULL;

	xglBindBufferARB					= NULL;
	xglDeleteBuffersARB					= NULL;
	xglGenBuffersARB					= NULL;
	xglIsBufferARB						= NULL;
	xglBufferDataARB					= NULL;
	xglBufferSubDataARB					= NULL;
	xglGetBufferSubDataARB					= NULL;
	xglMapBufferARB						= NULL;
	xglUnmapBufferARB					= NULL;
	xglGetBufferParameterivARB				= NULL;
	xglGetBufferPointervARB					= NULL;
	
	xglDeleteObjectARB					= NULL;
	xglGetHandleARB						= NULL;
	xglDetachObjectARB					= NULL;
	xglCreateShaderObjectARB				= NULL;
	xglShaderSourceARB					= NULL;
	xglCompileShaderARB					= NULL;
	xglCreateProgramObjectARB				= NULL;
	xglAttachObjectARB					= NULL;
	xglLinkProgramARB					= NULL;
	xglUseProgramObjectARB					= NULL;
	xglValidateProgramARB					= NULL;
	xglUniform1fARB						= NULL;
	xglUniform2fARB						= NULL;
	xglUniform3fARB						= NULL;
	xglUniform4fARB						= NULL;
	xglUniform1iARB						= NULL;
	xglUniform2iARB						= NULL;
	xglUniform3iARB						= NULL;
	xglUniform4iARB						= NULL;
	xglUniform1fvARB					= NULL;
	xglUniform2fvARB					= NULL;
	xglUniform3fvARB					= NULL;
	xglUniform4fvARB					= NULL;
	xglUniform1ivARB					= NULL;
	xglUniform2ivARB					= NULL;
	xglUniform3ivARB					= NULL;
	xglUniform4ivARB					= NULL;
	xglUniformMatrix2fvARB					= NULL;
	xglUniformMatrix3fvARB					= NULL;
	xglUniformMatrix4fvARB					= NULL;
	xglGetObjectParameterfvARB				= NULL;
	xglGetObjectParameterivARB				= NULL;
	xglGetInfoLogARB					= NULL;
	xglGetAttachedObjectsARB				= NULL;
	xglGetUniformLocationARB				= NULL;
	xglGetActiveUniformARB					= NULL;
	xglGetUniformfvARB					= NULL;
	xglGetUniformivARB					= NULL;
	xglGetShaderSourceARB					= NULL;

	xglBindAttribLocationARB				= NULL;
	xglGetActiveAttribARB					= NULL;
	xglGetAttribLocationARB					= NULL;

	xglLockArraysEXT					= NULL;
	xglUnlockArraysEXT					= NULL;
	
	xglAreProgramsResidentNV				= NULL;
	xglBindProgramNV					= NULL;
	xglDeleteProgramsNV					= NULL;
	xglExecuteProgramNV					= NULL;
	xglGenProgramsNV					= NULL;
	xglGetProgramParameterdvNV				= NULL;
	xglGetProgramParameterfvNV				= NULL;
	xglGetProgramivNV					= NULL;
	xglGetProgramStringNV					= NULL;
	xglGetTrackMatrixivNV					= NULL;
	xglGetVertexAttribdvNV					= NULL;
	xglGetVertexAttribfvNV					= NULL;
	xglGetVertexAttribivNV					= NULL;
	xglGetVertexAttribPointervNV				= NULL;
	xglIsProgramNV						= NULL;
	xglLoadProgramNV					= NULL;
	xglProgramParameter4fNV					= NULL;
	xglProgramParameter4fvNV				= NULL;
	xglProgramParameters4fvNV				= NULL;
	xglRequestResidentProgramsNV				= NULL;
	xglTrackMatrixNV					= NULL;
	xglVertexAttribPointerNV				= NULL;
	xglVertexAttrib1fNV					= NULL;
	xglVertexAttrib1fvNV					= NULL;
	xglVertexAttrib2fNV					= NULL;
	xglVertexAttrib2fvNV					= NULL;
	xglVertexAttrib3fNV					= NULL;
	xglVertexAttrib3fvNV					= NULL;
	xglVertexAttrib4fNV					= NULL;
	xglVertexAttrib4fvNV					= NULL;
	xglVertexAttribs1fvNV					= NULL;
	xglVertexAttribs2fvNV					= NULL;
	xglVertexAttribs3fvNV					= NULL;
	xglVertexAttribs4fvNV					= NULL;
	
	xglProgramNamedParameter4fNV				= NULL;
	xglProgramNamedParameter4fvNV				= NULL;
	xglGetProgramNamedParameterfvNV				= NULL;
	
#ifdef __linux__
	xglXChooseVisual					= NULL;
	xglXCopyContext						= NULL;
	xglXCreateContext					= NULL;
	xglXDestroyContext					= NULL;
	xglXGetCurrentDrawable					= NULL;
	xglXMakeCurrent						= NULL;
	xglXGetConfig						= NULL;
	xglXGetCurrentContext					= NULL;
	xglXQueryExtension					= NULL;
	xglXQueryVersion					= NULL;
	xglXSwapBuffers						= NULL;
	
	xglXQueryExtensionsString				= NULL;
	
	xglXGetCurrentDisplay					= NULL;
	
	xglXGetFBConfigs					= NULL;
	
	xglXChooseFBConfigSGIX					= NULL;
	xglXCreateContextWithConfigSGIX				= NULL;

	xglXCreateGLXPbufferSGIX				= NULL;
	xglXQueryGLXPbufferSGIX					= NULL;
#endif // __linux__

#ifdef _WIN32
	xglMakeCurrent						= NULL;
	xglDeleteContext					= NULL;
	xglCreateContext					= NULL;
#endif // _WIN32
}


void*	XGL_GetSymbol(const char *symbolname)
{
	void	*sym = NULL;

#ifdef __linux__
	char 	*err = NULL;
	
	if((sym = dlsym(sys_gl.OpenGLLib, symbolname)) == NULL)
	{
		err = dlerror();
	}
	else
	{
//		ri.Com_DPrintf("XGL_GetSymbol: '%s'  '%p'\n", symbolname, sym);
		return sym;
	}
	
	if(err)
		ri.Com_Error(ERR_FATAL, "XGL_GetSymbol: %s by searching symbol '%s'", err, symbolname);
	else
		ri.Com_Error(ERR_FATAL, "XGL_GetSymbol: unknown error by searching symbol '%s'", symbolname);

	return NULL;
	
#elif _WIN32
	if((sym = GetProcAddress(sys_gl.OpenGLLib, symbolname)) == NULL)
	{
		ri.Com_Error(ERR_FATAL, "XGL_GetSymbol: GetProcAddress failed on '%s'", symbolname);
		return NULL;
	}

	return sym;
	
#endif
}

void*	xglGetProcAddress(const char *symbol)
{
	if(sys_gl.OpenGLLib)
		return XGL_GetSymbol(symbol);

	return NULL;
}

/*
===================
XGL_Init

This is responsible for binding our xgl function pointers to
the appropriate GL stuff.  In Windows this means doing a 
LoadLibrary and a bunch of calls to GetProcAddress.  On other
operating systems we need to do the right thing, whatever that
might be.
=================== 
*/

bool	XGL_Init(const char *dllname)
{
	ri.Com_Printf("------- XGL_Init -------\n");
	
	if(sys_gl.OpenGLLib)
		XGL_Shutdown();
	
#ifdef __linux__
	if((sys_gl.OpenGLLib = dlopen(dllname, RTLD_NOW/* | RTLD_GLOBAL*/)) == NULL)
	{
		ri.Com_Printf("%s\n", dlerror());
		return false;
	}
#elif _WIN32
	if ((sys_gl.OpenGLLib = LoadLibrary(dllname)) == NULL)
	{
		ri.Com_Printf("LoadLibrary(\"%s\") failed\n", dllname);
		return false;
	}
#else
	return false;
#endif

	xglBegin = (void (GLAPIENTRY*) (GLenum mode)) XGL_GetSymbol("glBegin");
	xglColor4f = (void (GLAPIENTRY*) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)) XGL_GetSymbol("glColor4f");
	xglColor4fv = (void (GLAPIENTRY*) (const GLfloat *v)) XGL_GetSymbol("glColor4fv");
	xglEnd = (void (GLAPIENTRY*) (void)) XGL_GetSymbol("glEnd");
	
	xglTexCoord1f = (void (GLAPIENTRY*) (GLfloat s)) XGL_GetSymbol("glTexCoord1f");
	xglTexCoord1fv = (void (GLAPIENTRY*) (const GLfloat *v)) XGL_GetSymbol("glTexCoord1fv");
	xglTexCoord2f = (void (GLAPIENTRY*) (GLfloat s, GLfloat t)) XGL_GetSymbol("glTexCoord2f");
	xglTexCoord2fv = (void (GLAPIENTRY*) (const GLfloat *v)) XGL_GetSymbol("glTexCoord2fv");
	xglTexCoord3f = (void (GLAPIENTRY*) (GLfloat s, GLfloat t, GLfloat r)) XGL_GetSymbol("glTexCoord3f");
	xglTexCoord3fv = (void (GLAPIENTRY*) (const GLfloat *v)) XGL_GetSymbol("glTexCoord3fv");
	
	xglVertex3f = (void (GLAPIENTRY*) (GLfloat x, GLfloat y, GLfloat z)) XGL_GetSymbol("glVertex3f");
	xglVertex3fv = (void (GLAPIENTRY*) (const GLfloat *v)) XGL_GetSymbol("glVertex3fv");
	
	xglCullFace = (void (GLAPIENTRY*) (GLenum mode)) XGL_GetSymbol("glCullFace");
	xglFrontFace = (void (GLAPIENTRY*) (GLenum mode)) XGL_GetSymbol("glFrontFace");
	xglPolygonMode = (void (GLAPIENTRY*) (GLenum face, GLenum mode)) XGL_GetSymbol("glPolygonMode");
	xglScissor = (void (GLAPIENTRY*) (GLint x, GLint y, GLsizei width, GLsizei height)) XGL_GetSymbol("glScissor");
	
	xglTexParameterfv = (void (GLAPIENTRY*) (GLenum target, GLenum pname, const GLfloat *params)) XGL_GetSymbol("glTexParameterfv");
	xglTexParameteri = (void (GLAPIENTRY*) (GLenum target, GLenum pname, GLint param)) XGL_GetSymbol("glTexParameteri");
	xglTexParameteriv = (void (GLAPIENTRY*) (GLenum target, GLenum pname, const GLint *params)) XGL_GetSymbol("glTexParameteriv");
	xglTexImage1D = (void (GLAPIENTRY*) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels)) XGL_GetSymbol("glTexImage1D");
	xglTexImage2D = (void (GLAPIENTRY*) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels)) XGL_GetSymbol("glTexImage2D");
	xglTexEnvi = (void (GLAPIENTRY*) (GLenum target, GLenum pname, GLint param)) XGL_GetSymbol("glTexEnvi");
	xglTexGenf = (void (GLAPIENTRY*) (GLenum coord, GLenum pname, GLfloat param)) XGL_GetSymbol("glTexGenf");
	xglTexGenfv = (void (GLAPIENTRY*) (GLenum coord, GLenum pname, const GLfloat *params)) XGL_GetSymbol("glTexGenfv");
	xglTexGeni = (void (GLAPIENTRY*) (GLenum coord, GLenum pname, GLint param)) XGL_GetSymbol("glTexGeni");
	xglTexGeniv = (void (GLAPIENTRY*) (GLenum coord, GLenum pname, const GLint *params)) XGL_GetSymbol("glTexGeniv");
	
	xglDrawBuffer = (void (GLAPIENTRY*) (GLenum mode)) XGL_GetSymbol("glDrawBuffer");
	
	xglClear = (void (GLAPIENTRY*) (GLbitfield mask)) XGL_GetSymbol("glClear");
        xglClearColor = (void (GLAPIENTRY*) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)) XGL_GetSymbol("glClearColor");
	xglClearStencil = (void (GLAPIENTRY*) (GLint s)) XGL_GetSymbol ("glClearStencil");

	xglColorMask = (void (GLAPIENTRY*) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)) XGL_GetSymbol("glColorMask");
	xglDepthMask = (void (GLAPIENTRY*) (GLboolean flag)) XGL_GetSymbol("glDepthMask");
	
	xglDisable = (void (GLAPIENTRY*) (GLenum cap)) XGL_GetSymbol("glDisable");
	xglEnable = (void (GLAPIENTRY*) (GLenum cap)) XGL_GetSymbol("glEnable");
	
	xglFlush = (void (GLAPIENTRY*) (void)) XGL_GetSymbol("glFlush");
	
	xglPopAttrib = (void (GLAPIENTRY*) (void)) XGL_GetSymbol("glPopAttrib");
	xglPushAttrib = (void (GLAPIENTRY*) (GLbitfield mask)) XGL_GetSymbol("glPushAttrib");
	
	xglAlphaFunc = (void (GLAPIENTRY*) (GLenum func, GLclampf ref)) XGL_GetSymbol("glAlphaFunc");
	xglBlendFunc = (void (GLAPIENTRY*) (GLenum sfactor, GLenum dfactor)) XGL_GetSymbol("glBlendFunc");
	xglStencilFunc = (void (GLAPIENTRY*) (GLenum func, GLint ref, GLuint mask)) XGL_GetSymbol("glStencilFunc");
	xglStencilOp = (void (GLAPIENTRY*) (GLenum fail, GLenum zfail, GLenum zpass)) XGL_GetSymbol("glStencilOp");
	xglDepthFunc = (void (GLAPIENTRY*) (GLenum func)) XGL_GetSymbol("glDepthFunc");
	
	xglReadPixels = (void (GLAPIENTRY*) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels)) XGL_GetSymbol("glReadPixels");
	
	xglGetError = (GLenum	(GLAPIENTRY*) (void)) XGL_GetSymbol("glGetError");
	xglGetFloatv = (void (GLAPIENTRY*) (GLenum pname, GLfloat *params)) XGL_GetSymbol("glGetFloatv");
	xglGetIntegerv = (void (GLAPIENTRY*) (GLenum pname, GLint *params)) XGL_GetSymbol("glGetIntegerv");
	xglGetString = (const GLubyte*	(GLAPIENTRY*) (GLenum name)) XGL_GetSymbol("glGetString");
	
	xglDepthRange = (void (GLAPIENTRY*) (GLclampd zNear, GLclampd zFar)) XGL_GetSymbol("glDepthRange");
//	xglFrustum = (void (GLAPIENTRY*) (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)) XGL_GetSymbol("glFrustum");
	xglLoadIdentity = (void (GLAPIENTRY*) (void)) XGL_GetSymbol("glLoadIdentity");
//	xglLoadMatrixf = (void (GLAPIENTRY*) (const GLfloat *m)) XGL_GetSymbol("glLoadMatrixf");
	xglMatrixMode = (void (GLAPIENTRY*) (GLenum mode)) XGL_GetSymbol ("glMatrixMode");
	xglOrtho = (void (GLAPIENTRY*) (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)) XGL_GetSymbol("glOrtho");
	xglPopMatrix = (void (GLAPIENTRY*) (void)) XGL_GetSymbol("glPopMatrix");
	xglPushMatrix = (void (GLAPIENTRY*) (void)) XGL_GetSymbol("glPushMatrix");
//	xglRotatef = (void (GLAPIENTRY*) (GLfloat angle, GLfloat x, GLfloat y, GLfloat z)) XGL_GetSymbol("glRotatef");
//	xglTranslatef = (void (GLAPIENTRY*) (GLfloat x, GLfloat y, GLfloat z)) XGL_GetSymbol("glTranslatef");
//	xglScalef = (void (GLAPIENTRY*) (GLfloat x, GLfloat y, GLfloat z)) XGL_GetSymbol("glScalef");
	xglViewport = (void (GLAPIENTRY*) (GLint x, GLint y, GLsizei width, GLsizei height)) XGL_GetSymbol("glViewport");
	
	xglDisableClientState = (void (GLAPIENTRY*) (GLenum array)) XGL_GetSymbol("glDisableClientState");
	xglDrawElements = (void (GLAPIENTRY*) (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)) XGL_GetSymbol("glDrawElements");
	xglEnableClientState = (void (GLAPIENTRY*) (GLenum array)) XGL_GetSymbol("glEnableClientState");
	xglIndexPointer = (void (GLAPIENTRY*) (GLenum type, GLsizei stride, const GLvoid *pointer)) XGL_GetSymbol("glIndexPointer");
	xglNormalPointer = (void (GLAPIENTRY*) (GLenum type, GLsizei stride, const GLvoid *pointer)) XGL_GetSymbol("glNormalPointer");
	xglTexCoordPointer = (void (GLAPIENTRY*) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)) XGL_GetSymbol("glTexCoordPointer");
	xglVertexPointer = (void (GLAPIENTRY*) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)) XGL_GetSymbol("glVertexPointer");
	
	xglPolygonOffset = (void (GLAPIENTRY*) (GLfloat factor, GLfloat units)) XGL_GetSymbol("glPolygonOffset");
	
	xglBindTexture = (void (GLAPIENTRY*) (GLenum target, GLuint texture)) XGL_GetSymbol("glBindTexture");
	xglDeleteTextures = (void (GLAPIENTRY*) (GLsizei n, const GLuint *textures)) XGL_GetSymbol("glDeleteTextures");
	xglGenTextures = (void (GLAPIENTRY*) (GLsizei n, GLuint *textures)) XGL_GetSymbol("glGenTextures");
	
	xglDrawRangeElements = (void (GLAPIENTRY*) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices)) XGL_GetSymbol("glDrawRangeElements");
	
	xglTexImage3D = (void (GLAPIENTRY*) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels)) XGL_GetSymbol("glTexImage3D");
	
	xglActiveTexture = (void (GLAPIENTRY*) (GLenum texture)) XGL_GetSymbol("glActiveTexture");
	xglClientActiveTexture = (void (GLAPIENTRY*) (GLenum texture)) XGL_GetSymbol("glClientActiveTexture");
	
	xglLoadTransposeMatrixf = (void (GLAPIENTRY*) (const GLfloat *m)) XGL_GetSymbol("glLoadTransposeMatrixf");

	/*
	xglArrayElement = (void (GLAPIENTRY*) (GLint i)) XGL_GetSymbol("glArrayElement");
        xglClipPlane = (void (GLAPIENTRY*) (GLenum plane, const GLdouble *equation)) XGL_GetSymbol("glClipPlane");
        xglColorPointer = (void (GLAPIENTRY*) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)) XGL_GetSymbol("glColorPointer");
        xglGetBooleanv = (void (GLAPIENTRY*) (GLenum pname, GLboolean *params)) XGL_GetSymbol("glGetBooleanv");
	xglGetDoublev = (void (GLAPIENTRY*) (GLenum pname, GLdouble *params)) XGL_GetSymbol("glGetDoublev");
	xglIsEnabled = (GLboolean (GLAPIENTRY*) (GLenum cap)) XGL_GetSymbol("glIsEnabled");
	xglShadeModel = (void (GLAPIENTRY*) (GLenum mode)) XGL_GetSymbol("glShadeModel");
	*/
	
#ifdef __linux__
	xglXChooseVisual = (XVisualInfo* (*)( Display *dpy, int screen, int *attribList )) XGL_GetSymbol("glXChooseVisual");
	xglXCopyContext = (void	(*)(Display *dpy, GLXContext src, GLXContext dst, GLuint mask)) XGL_GetSymbol("glXCopyContext");
	xglXCreateContext = (GLXContext (*)( Display *dpy, XVisualInfo *vis, GLXContext shareList, Bool direct )) XGL_GetSymbol("glXCreateContext");
	xglXDestroyContext = (void (*)( Display *dpy, GLXContext ctx )) XGL_GetSymbol("glXDestroyContext");
	xglXGetCurrentDrawable = (GLXDrawable (*)(void)) XGL_GetSymbol("glXGetCurrentDrawable");
	xglXMakeCurrent = (Bool (*)( Display *dpy, GLXDrawable drawable, GLXContext ctx)) XGL_GetSymbol("glXMakeCurrent");
	xglXGetConfig = (int (*)(Display *dpy, XVisualInfo *vis, int attrib, int *value)) XGL_GetSymbol("glXGetConfig");
	xglXGetCurrentContext = (GLXContext (*)(void)) XGL_GetSymbol("glXGetCurrentContext");
	xglXQueryExtension = (Bool (*)(Display *dpy, int *error_base, int *event_base)) XGL_GetSymbol("glXQueryExtension");
	xglXQueryVersion = (Bool (*)(Display *dpy, int *major, int *minor)) XGL_GetSymbol("glXQueryVersion");
	xglXSwapBuffers = (void (*)( Display *dpy, GLXDrawable drawable )) XGL_GetSymbol("glXSwapBuffers");
	
	xglXQueryExtensionsString = (const char* (*)(Display *dpy, int screen)) XGL_GetSymbol("glXQueryExtensionsString");
	
	xglXGetCurrentDisplay = (Display* (*)(void)) XGL_GetSymbol("glXGetCurrentDisplay");

	xglXGetFBConfigs = (GLXFBConfig* (*)(Display *dpy, int screen, int *nelements)) XGL_GetSymbol("glXGetFBConfigs");
	
#elif _WIN32
	xwglMakeCurrent = (BOOL (*)(HDC, HGLRC)) XGL_GetSymbol("wglMakeCurrent");
	xwglDeleteContext = (BOOL (*)(HGLRC)) XGL_GetSymbol("wglDeleteContext");
	xwglCreateContext = (HGLRC (*)(HDC)) XGL_GetSymbol("wglCreateContext");
#endif

	return true;
}

