/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006-2008 Robert Beckebans <trebor_7@users.sourceforge.net>

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
/*
** QGL.C
**
** This file implements the operating system binding of GL to QGL function
** pointers.  When doing a port of XreaL you must implement the following
** two functions:
**
** QGL_Init() - loads libraries, assigns function pointers, etc.
** QGL_Shutdown() - unloads libraries, NULLs function pointers
*/

#include "SDL.h"
#include "SDL_loadso.h"

#include "qgl.h"
#include "tr_local.h"

void            (APIENTRY * qglBegin) (GLenum mode);
void            (APIENTRY * qglBindTexture) (GLenum target, GLuint texture);
void            (APIENTRY * qglBlendFunc) (GLenum sfactor, GLenum dfactor);
void            (APIENTRY * qglClear) (GLbitfield mask);
void            (APIENTRY * qglClearColor) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
void            (APIENTRY * qglClearDepth) (GLclampd depth);
void            (APIENTRY * qglClearStencil) (GLint s);
void            (APIENTRY * qglClipPlane) (GLenum plane, const GLdouble * equation);
void            (APIENTRY * qglColorMask) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
void            (APIENTRY * qglCopyPixels) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
void            (APIENTRY * qglCopyTexImage1D) (GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y,
												GLsizei width, GLint border);
void            (APIENTRY * qglCopyTexImage2D) (GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y,
												GLsizei width, GLsizei height, GLint border);
void            (APIENTRY * qglCopyTexSubImage1D) (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
void            (APIENTRY * qglCopyTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y,
												   GLsizei width, GLsizei height);
void            (APIENTRY * qglCullFace) (GLenum mode);
void            (APIENTRY * qglDeleteTextures) (GLsizei n, const GLuint * textures);
void            (APIENTRY * qglDepthFunc) (GLenum func);
void            (APIENTRY * qglDepthMask) (GLboolean flag);
void            (APIENTRY * qglDepthRange) (GLclampd zNear, GLclampd zFar);
void            (APIENTRY * qglDisable) (GLenum cap);
void            (APIENTRY * qglDrawArrays) (GLenum mode, GLint first, GLsizei count);
void            (APIENTRY * qglDrawBuffer) (GLenum mode);
void            (APIENTRY * qglDrawElements) (GLenum mode, GLsizei count, GLenum type, const GLvoid * indices);
void            (APIENTRY * qglEnable) (GLenum cap);
void            (APIENTRY * qglEnd) (void);
void            (APIENTRY * qglFinish) (void);
void            (APIENTRY * qglFlush) (void);
void            (APIENTRY * qglFrontFace) (GLenum mode);
void            (APIENTRY * qglGenTextures) (GLsizei n, GLuint * textures);
void            (APIENTRY * qglGetBooleanv) (GLenum pname, GLboolean * params);
GLenum(APIENTRY * qglGetError) (void);
void            (APIENTRY * qglGetFloatv) (GLenum pname, GLfloat * params);
void            (APIENTRY * qglGetIntegerv) (GLenum pname, GLint * params);
const GLubyte  *(APIENTRY * qglGetString) (GLenum name);
void            (APIENTRY * qglGetTexParameterfv) (GLenum target, GLenum pname, GLfloat * params);
void            (APIENTRY * qglGetTexParameteriv) (GLenum target, GLenum pname, GLint * params);
void            (APIENTRY * qglHint) (GLenum target, GLenum mode);
GLboolean(APIENTRY * qglIsEnabled) (GLenum cap);
GLboolean(APIENTRY * qglIsTexture) (GLuint texture);
void            (APIENTRY * qglLineWidth) (GLfloat width);
void            (APIENTRY * qglPolygonMode) (GLenum face, GLenum mode);
void            (APIENTRY * qglPolygonOffset) (GLfloat factor, GLfloat units);
void            (APIENTRY * qglPolygonStipple) (const GLubyte * mask);
void            (APIENTRY * qglPopAttrib) (void);
void            (APIENTRY * qglPopClientAttrib) (void);
void            (APIENTRY * qglPopName) (void);
void            (APIENTRY * qglPrioritizeTextures) (GLsizei n, const GLuint * textures, const GLclampf * priorities);
void            (APIENTRY * qglPushAttrib) (GLbitfield mask);
void            (APIENTRY * qglPushClientAttrib) (GLbitfield mask);
void            (APIENTRY * qglPushName) (GLuint name);
void            (APIENTRY * qglRasterPos2d) (GLdouble x, GLdouble y);
void            (APIENTRY * qglRasterPos2dv) (const GLdouble * v);
void            (APIENTRY * qglRasterPos2f) (GLfloat x, GLfloat y);
void            (APIENTRY * qglRasterPos2fv) (const GLfloat * v);
void            (APIENTRY * qglRasterPos2i) (GLint x, GLint y);
void            (APIENTRY * qglRasterPos2iv) (const GLint * v);
void            (APIENTRY * qglRasterPos2s) (GLshort x, GLshort y);
void            (APIENTRY * qglRasterPos2sv) (const GLshort * v);
void            (APIENTRY * qglRasterPos3d) (GLdouble x, GLdouble y, GLdouble z);
void            (APIENTRY * qglRasterPos3dv) (const GLdouble * v);
void            (APIENTRY * qglRasterPos3f) (GLfloat x, GLfloat y, GLfloat z);
void            (APIENTRY * qglRasterPos3fv) (const GLfloat * v);
void            (APIENTRY * qglRasterPos3i) (GLint x, GLint y, GLint z);
void            (APIENTRY * qglRasterPos3iv) (const GLint * v);
void            (APIENTRY * qglRasterPos3s) (GLshort x, GLshort y, GLshort z);
void            (APIENTRY * qglRasterPos3sv) (const GLshort * v);
void            (APIENTRY * qglRasterPos4d) (GLdouble x, GLdouble y, GLdouble z, GLdouble w);
void            (APIENTRY * qglRasterPos4dv) (const GLdouble * v);
void            (APIENTRY * qglRasterPos4f) (GLfloat x, GLfloat y, GLfloat z, GLfloat w);
void            (APIENTRY * qglRasterPos4fv) (const GLfloat * v);
void            (APIENTRY * qglRasterPos4i) (GLint x, GLint y, GLint z, GLint w);
void            (APIENTRY * qglRasterPos4iv) (const GLint * v);
void            (APIENTRY * qglRasterPos4s) (GLshort x, GLshort y, GLshort z, GLshort w);
void            (APIENTRY * qglRasterPos4sv) (const GLshort * v);
void            (APIENTRY * qglReadBuffer) (GLenum mode);
void            (APIENTRY * qglReadPixels) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type,
											GLvoid * pixels);
void            (APIENTRY * qglRectd) (GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
void            (APIENTRY * qglRectdv) (const GLdouble * v1, const GLdouble * v2);
void            (APIENTRY * qglRectf) (GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
void            (APIENTRY * qglRectfv) (const GLfloat * v1, const GLfloat * v2);
void            (APIENTRY * qglRecti) (GLint x1, GLint y1, GLint x2, GLint y2);
void            (APIENTRY * qglRectiv) (const GLint * v1, const GLint * v2);
void            (APIENTRY * qglRects) (GLshort x1, GLshort y1, GLshort x2, GLshort y2);
void            (APIENTRY * qglRectsv) (const GLshort * v1, const GLshort * v2);

GLint(APIENTRY * qglRenderMode) (GLenum mode);
void            (APIENTRY * qglRotated) (GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
void            (APIENTRY * qglRotatef) (GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
void            (APIENTRY * qglScaled) (GLdouble x, GLdouble y, GLdouble z);
void            (APIENTRY * qglScalef) (GLfloat x, GLfloat y, GLfloat z);
void            (APIENTRY * qglScissor) (GLint x, GLint y, GLsizei width, GLsizei height);
void            (APIENTRY * qglSelectBuffer) (GLsizei size, GLuint * buffer);
void            (APIENTRY * qglShadeModel) (GLenum mode);
void            (APIENTRY * qglStencilFunc) (GLenum func, GLint ref, GLuint mask);
void            (APIENTRY * qglStencilMask) (GLuint mask);
void            (APIENTRY * qglStencilOp) (GLenum fail, GLenum zfail, GLenum zpass);
void            (APIENTRY * qglTexImage1D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border,
											GLenum format, GLenum type, const GLvoid * pixels);
void            (APIENTRY * qglTexImage2D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height,
											GLint border, GLenum format, GLenum type, const GLvoid * pixels);
void            (APIENTRY * qglTexParameterf) (GLenum target, GLenum pname, GLfloat param);
void            (APIENTRY * qglTexParameterfv) (GLenum target, GLenum pname, const GLfloat * params);
void            (APIENTRY * qglTexParameteri) (GLenum target, GLenum pname, GLint param);
void            (APIENTRY * qglTexParameteriv) (GLenum target, GLenum pname, const GLint * params);
void            (APIENTRY * qglTexSubImage1D) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format,
											   GLenum type, const GLvoid * pixels);
void            (APIENTRY * qglTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width,
											   GLsizei height, GLenum format, GLenum type, const GLvoid * pixels);
void            (APIENTRY * qglTranslated) (GLdouble x, GLdouble y, GLdouble z);
void            (APIENTRY * qglTranslatef) (GLfloat x, GLfloat y, GLfloat z);
void            (APIENTRY * qglViewport) (GLint x, GLint y, GLsizei width, GLsizei height);




static void     (APIENTRY * dllBegin) (GLenum mode);
static void     (APIENTRY * dllBindTexture) (GLenum target, GLuint texture);
static void     (APIENTRY * dllBlendFunc) (GLenum sfactor, GLenum dfactor);
static void     (APIENTRY * dllClear) (GLbitfield mask);
static void     (APIENTRY * dllClearColor) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
static void     (APIENTRY * dllClearDepth) (GLclampd depth);
static void     (APIENTRY * dllClearStencil) (GLint s);
static void     (APIENTRY * dllClipPlane) (GLenum plane, const GLdouble * equation);
static void     (APIENTRY * dllColorMask) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
static void     (APIENTRY * dllCopyPixels) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
static void     (APIENTRY * dllCopyTexImage1D) (GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y,
												GLsizei width, GLint border);
static void     (APIENTRY * dllCopyTexImage2D) (GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y,
												GLsizei width, GLsizei height, GLint border);
static void     (APIENTRY * dllCopyTexSubImage1D) (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
static void     (APIENTRY * dllCopyTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y,
												   GLsizei width, GLsizei height);
static void     (APIENTRY * dllCullFace) (GLenum mode);
static void     (APIENTRY * dllDeleteTextures) (GLsizei n, const GLuint * textures);
static void     (APIENTRY * dllDepthFunc) (GLenum func);
static void     (APIENTRY * dllDepthMask) (GLboolean flag);
static void     (APIENTRY * dllDepthRange) (GLclampd zNear, GLclampd zFar);
static void     (APIENTRY * dllDisable) (GLenum cap);
static void     (APIENTRY * dllDrawArrays) (GLenum mode, GLint first, GLsizei count);
static void     (APIENTRY * dllDrawBuffer) (GLenum mode);
static void     (APIENTRY * dllDrawElements) (GLenum mode, GLsizei count, GLenum type, const GLvoid * indices);
static void     (APIENTRY * dllEnable) (GLenum cap);
static void     (APIENTRY * dllEnd) (void);
static void     (APIENTRY * dllFinish) (void);
static void     (APIENTRY * dllFlush) (void);
static void     (APIENTRY * dllFrontFace) (GLenum mode);
static void     (APIENTRY * dllGenTextures) (GLsizei n, GLuint * textures);
static void     (APIENTRY * dllGetBooleanv) (GLenum pname, GLboolean * params);
static void     (APIENTRY * dllGetClipPlane) (GLenum plane, GLdouble * equation);
static void     (APIENTRY * dllGetDoublev) (GLenum pname, GLdouble * params);

GLenum(APIENTRY * dllGetError) (void);
static void     (APIENTRY * dllGetFloatv) (GLenum pname, GLfloat * params);
static void     (APIENTRY * dllGetIntegerv) (GLenum pname, GLint * params);
static void     (APIENTRY * dllGetLightfv) (GLenum light, GLenum pname, GLfloat * params);
static void     (APIENTRY * dllGetLightiv) (GLenum light, GLenum pname, GLint * params);
static void     (APIENTRY * dllGetMapdv) (GLenum target, GLenum query, GLdouble * v);
static void     (APIENTRY * dllGetMapfv) (GLenum target, GLenum query, GLfloat * v);
static void     (APIENTRY * dllGetMapiv) (GLenum target, GLenum query, GLint * v);
static void     (APIENTRY * dllGetMaterialfv) (GLenum face, GLenum pname, GLfloat * params);
static void     (APIENTRY * dllGetMaterialiv) (GLenum face, GLenum pname, GLint * params);
static void     (APIENTRY * dllGetPixelMapfv) (GLenum map, GLfloat * values);
static void     (APIENTRY * dllGetPixelMapuiv) (GLenum map, GLuint * values);
static void     (APIENTRY * dllGetPixelMapusv) (GLenum map, GLushort * values);
static void     (APIENTRY * dllGetPointerv) (GLenum pname, GLvoid * *params);
static void     (APIENTRY * dllGetPolygonStipple) (GLubyte * mask);
const GLubyte  *(APIENTRY * dllGetString) (GLenum name);
static void     (APIENTRY * dllGetTexEnvfv) (GLenum target, GLenum pname, GLfloat * params);
static void     (APIENTRY * dllGetTexEnviv) (GLenum target, GLenum pname, GLint * params);
static void     (APIENTRY * dllGetTexGendv) (GLenum coord, GLenum pname, GLdouble * params);
static void     (APIENTRY * dllGetTexGenfv) (GLenum coord, GLenum pname, GLfloat * params);
static void     (APIENTRY * dllGetTexGeniv) (GLenum coord, GLenum pname, GLint * params);
static void     (APIENTRY * dllGetTexImage) (GLenum target, GLint level, GLenum format, GLenum type, GLvoid * pixels);
static void     (APIENTRY * dllGetTexLevelParameterfv) (GLenum target, GLint level, GLenum pname, GLfloat * params);
static void     (APIENTRY * dllGetTexLevelParameteriv) (GLenum target, GLint level, GLenum pname, GLint * params);
static void     (APIENTRY * dllGetTexParameterfv) (GLenum target, GLenum pname, GLfloat * params);
static void     (APIENTRY * dllGetTexParameteriv) (GLenum target, GLenum pname, GLint * params);
static void     (APIENTRY * dllHint) (GLenum target, GLenum mode);
static void     (APIENTRY * dllInitNames) (void);
static void     (APIENTRY * dllInterleavedArrays) (GLenum format, GLsizei stride, const GLvoid * pointer);
GLboolean(APIENTRY * dllIsEnabled) (GLenum cap);
GLboolean(APIENTRY * dllIsTexture) (GLuint texture);
static void     (APIENTRY * dllLineWidth) (GLfloat width);
static void     (APIENTRY * dllListBase) (GLuint base);
static void     (APIENTRY * dllLoadIdentity) (void);
static void     (APIENTRY * dllLoadMatrixd) (const GLdouble * m);
static void     (APIENTRY * dllLoadMatrixf) (const GLfloat * m);
static void     (APIENTRY * dllLoadName) (GLuint name);
static void     (APIENTRY * dllLogicOp) (GLenum opcode);
static void     (APIENTRY * dllMap1d) (GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order,
									   const GLdouble * points);
static void     (APIENTRY * dllMap1f) (GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat * points);
static void     (APIENTRY * dllMap2d) (GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1,
									   GLdouble v2, GLint vstride, GLint vorder, const GLdouble * points);
static void     (APIENTRY * dllMap2f) (GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2,
									   GLint vstride, GLint vorder, const GLfloat * points);
static void     (APIENTRY * dllMapGrid1d) (GLint un, GLdouble u1, GLdouble u2);
static void     (APIENTRY * dllMapGrid1f) (GLint un, GLfloat u1, GLfloat u2);
static void     (APIENTRY * dllMapGrid2d) (GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
static void     (APIENTRY * dllMapGrid2f) (GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
static void     (APIENTRY * dllMaterialf) (GLenum face, GLenum pname, GLfloat param);
static void     (APIENTRY * dllMaterialfv) (GLenum face, GLenum pname, const GLfloat * params);
static void     (APIENTRY * dllMateriali) (GLenum face, GLenum pname, GLint param);
static void     (APIENTRY * dllMaterialiv) (GLenum face, GLenum pname, const GLint * params);
static void     (APIENTRY * dllMatrixMode) (GLenum mode);
static void     (APIENTRY * dllMultMatrixd) (const GLdouble * m);
static void     (APIENTRY * dllMultMatrixf) (const GLfloat * m);
static void     (APIENTRY * dllNewList) (GLuint list, GLenum mode);
static void     (APIENTRY * dllNormal3b) (GLbyte nx, GLbyte ny, GLbyte nz);
static void     (APIENTRY * dllNormal3bv) (const GLbyte * v);
static void     (APIENTRY * dllNormal3d) (GLdouble nx, GLdouble ny, GLdouble nz);
static void     (APIENTRY * dllNormal3dv) (const GLdouble * v);
static void     (APIENTRY * dllNormal3f) (GLfloat nx, GLfloat ny, GLfloat nz);
static void     (APIENTRY * dllNormal3fv) (const GLfloat * v);
static void     (APIENTRY * dllNormal3i) (GLint nx, GLint ny, GLint nz);
static void     (APIENTRY * dllNormal3iv) (const GLint * v);
static void     (APIENTRY * dllNormal3s) (GLshort nx, GLshort ny, GLshort nz);
static void     (APIENTRY * dllNormal3sv) (const GLshort * v);
static void     (APIENTRY * dllPassThrough) (GLfloat token);
static void     (APIENTRY * dllPixelMapfv) (GLenum map, GLsizei mapsize, const GLfloat * values);
static void     (APIENTRY * dllPixelMapuiv) (GLenum map, GLsizei mapsize, const GLuint * values);
static void     (APIENTRY * dllPixelMapusv) (GLenum map, GLsizei mapsize, const GLushort * values);
static void     (APIENTRY * dllPixelStoref) (GLenum pname, GLfloat param);
static void     (APIENTRY * dllPixelStorei) (GLenum pname, GLint param);
static void     (APIENTRY * dllPixelTransferf) (GLenum pname, GLfloat param);
static void     (APIENTRY * dllPixelTransferi) (GLenum pname, GLint param);
static void     (APIENTRY * dllPixelZoom) (GLfloat xfactor, GLfloat yfactor);
static void     (APIENTRY * dllPointSize) (GLfloat size);
static void     (APIENTRY * dllPolygonMode) (GLenum face, GLenum mode);
static void     (APIENTRY * dllPolygonOffset) (GLfloat factor, GLfloat units);
static void     (APIENTRY * dllPolygonStipple) (const GLubyte * mask);
static void     (APIENTRY * dllPopAttrib) (void);
static void     (APIENTRY * dllPopClientAttrib) (void);
static void     (APIENTRY * dllPopName) (void);
static void     (APIENTRY * dllPrioritizeTextures) (GLsizei n, const GLuint * textures, const GLclampf * priorities);
static void     (APIENTRY * dllPushAttrib) (GLbitfield mask);
static void     (APIENTRY * dllPushClientAttrib) (GLbitfield mask);
static void     (APIENTRY * dllPushName) (GLuint name);
static void     (APIENTRY * dllRasterPos2d) (GLdouble x, GLdouble y);
static void     (APIENTRY * dllRasterPos2dv) (const GLdouble * v);
static void     (APIENTRY * dllRasterPos2f) (GLfloat x, GLfloat y);
static void     (APIENTRY * dllRasterPos2fv) (const GLfloat * v);
static void     (APIENTRY * dllRasterPos2i) (GLint x, GLint y);
static void     (APIENTRY * dllRasterPos2iv) (const GLint * v);
static void     (APIENTRY * dllRasterPos2s) (GLshort x, GLshort y);
static void     (APIENTRY * dllRasterPos2sv) (const GLshort * v);
static void     (APIENTRY * dllRasterPos3d) (GLdouble x, GLdouble y, GLdouble z);
static void     (APIENTRY * dllRasterPos3dv) (const GLdouble * v);
static void     (APIENTRY * dllRasterPos3f) (GLfloat x, GLfloat y, GLfloat z);
static void     (APIENTRY * dllRasterPos3fv) (const GLfloat * v);
static void     (APIENTRY * dllRasterPos3i) (GLint x, GLint y, GLint z);
static void     (APIENTRY * dllRasterPos3iv) (const GLint * v);
static void     (APIENTRY * dllRasterPos3s) (GLshort x, GLshort y, GLshort z);
static void     (APIENTRY * dllRasterPos3sv) (const GLshort * v);
static void     (APIENTRY * dllRasterPos4d) (GLdouble x, GLdouble y, GLdouble z, GLdouble w);
static void     (APIENTRY * dllRasterPos4dv) (const GLdouble * v);
static void     (APIENTRY * dllRasterPos4f) (GLfloat x, GLfloat y, GLfloat z, GLfloat w);
static void     (APIENTRY * dllRasterPos4fv) (const GLfloat * v);
static void     (APIENTRY * dllRasterPos4i) (GLint x, GLint y, GLint z, GLint w);
static void     (APIENTRY * dllRasterPos4iv) (const GLint * v);
static void     (APIENTRY * dllRasterPos4s) (GLshort x, GLshort y, GLshort z, GLshort w);
static void     (APIENTRY * dllRasterPos4sv) (const GLshort * v);
static void     (APIENTRY * dllReadBuffer) (GLenum mode);
static void     (APIENTRY * dllReadPixels) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type,
											GLvoid * pixels);
static void     (APIENTRY * dllRectd) (GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
static void     (APIENTRY * dllRectdv) (const GLdouble * v1, const GLdouble * v2);
static void     (APIENTRY * dllRectf) (GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
static void     (APIENTRY * dllRectfv) (const GLfloat * v1, const GLfloat * v2);
static void     (APIENTRY * dllRecti) (GLint x1, GLint y1, GLint x2, GLint y2);
static void     (APIENTRY * dllRectiv) (const GLint * v1, const GLint * v2);
static void     (APIENTRY * dllRects) (GLshort x1, GLshort y1, GLshort x2, GLshort y2);
static void     (APIENTRY * dllRectsv) (const GLshort * v1, const GLshort * v2);

GLint(APIENTRY * dllRenderMode) (GLenum mode);
static void     (APIENTRY * dllRotated) (GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
static void     (APIENTRY * dllRotatef) (GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
static void     (APIENTRY * dllScaled) (GLdouble x, GLdouble y, GLdouble z);
static void     (APIENTRY * dllScalef) (GLfloat x, GLfloat y, GLfloat z);
static void     (APIENTRY * dllScissor) (GLint x, GLint y, GLsizei width, GLsizei height);
static void     (APIENTRY * dllSelectBuffer) (GLsizei size, GLuint * buffer);
static void     (APIENTRY * dllShadeModel) (GLenum mode);
static void     (APIENTRY * dllStencilFunc) (GLenum func, GLint ref, GLuint mask);
static void     (APIENTRY * dllStencilMask) (GLuint mask);
static void     (APIENTRY * dllStencilOp) (GLenum fail, GLenum zfail, GLenum zpass);
static void     (APIENTRY * dllTexImage1D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border,
											GLenum format, GLenum type, const GLvoid * pixels);
static void     (APIENTRY * dllTexImage2D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height,
											GLint border, GLenum format, GLenum type, const GLvoid * pixels);
static void     (APIENTRY * dllTexParameterf) (GLenum target, GLenum pname, GLfloat param);
static void     (APIENTRY * dllTexParameterfv) (GLenum target, GLenum pname, const GLfloat * params);
static void     (APIENTRY * dllTexParameteri) (GLenum target, GLenum pname, GLint param);
static void     (APIENTRY * dllTexParameteriv) (GLenum target, GLenum pname, const GLint * params);
static void     (APIENTRY * dllTexSubImage1D) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format,
											   GLenum type, const GLvoid * pixels);
static void     (APIENTRY * dllTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width,
											   GLsizei height, GLenum format, GLenum type, const GLvoid * pixels);
static void     (APIENTRY * dllTranslated) (GLdouble x, GLdouble y, GLdouble z);
static void     (APIENTRY * dllTranslatef) (GLfloat x, GLfloat y, GLfloat z);
static void     (APIENTRY * dllViewport) (GLint x, GLint y, GLsizei width, GLsizei height);

static FILE    *log_fp = NULL;

static const char *BooleanToString(GLboolean b)
{
	if(b == GL_FALSE)
		return "GL_FALSE";
	else if(b == GL_TRUE)
		return "GL_TRUE";
	else
		return "OUT OF RANGE FOR BOOLEAN";
}

static const char *FuncToString(GLenum f)
{
	switch (f)
	{
		case GL_ALWAYS:
			return "GL_ALWAYS";
		case GL_NEVER:
			return "GL_NEVER";
		case GL_LEQUAL:
			return "GL_LEQUAL";
		case GL_LESS:
			return "GL_LESS";
		case GL_EQUAL:
			return "GL_EQUAL";
		case GL_GREATER:
			return "GL_GREATER";
		case GL_GEQUAL:
			return "GL_GEQUAL";
		case GL_NOTEQUAL:
			return "GL_NOTEQUAL";
		default:
			return "!!! UNKNOWN !!!";
	}
}

static const char *PrimToString(GLenum mode)
{
	static char     prim[1024];

	if(mode == GL_TRIANGLES)
		strcpy(prim, "GL_TRIANGLES");
	else if(mode == GL_TRIANGLE_STRIP)
		strcpy(prim, "GL_TRIANGLE_STRIP");
	else if(mode == GL_TRIANGLE_FAN)
		strcpy(prim, "GL_TRIANGLE_FAN");
	else if(mode == GL_QUADS)
		strcpy(prim, "GL_QUADS");
	else if(mode == GL_QUAD_STRIP)
		strcpy(prim, "GL_QUAD_STRIP");
	else if(mode == GL_POLYGON)
		strcpy(prim, "GL_POLYGON");
	else if(mode == GL_POINTS)
		strcpy(prim, "GL_POINTS");
	else if(mode == GL_LINES)
		strcpy(prim, "GL_LINES");
	else if(mode == GL_LINE_STRIP)
		strcpy(prim, "GL_LINE_STRIP");
	else if(mode == GL_LINE_LOOP)
		strcpy(prim, "GL_LINE_LOOP");
	else
		sprintf(prim, "0x%x", mode);

	return prim;
}

static const char *CapToString(GLenum cap)
{
	static char     buffer[1024];

	switch (cap)
	{
		case GL_TEXTURE_2D:
			return "GL_TEXTURE_2D";
		case GL_BLEND:
			return "GL_BLEND";
		case GL_DEPTH_TEST:
			return "GL_DEPTH_TEST";
		case GL_CULL_FACE:
			return "GL_CULL_FACE";
		case GL_CLIP_PLANE0:
			return "GL_CLIP_PLANE0";
		case GL_COLOR_ARRAY:
			return "GL_COLOR_ARRAY";
		case GL_TEXTURE_COORD_ARRAY:
			return "GL_TEXTURE_COORD_ARRAY";
		case GL_VERTEX_ARRAY:
			return "GL_VERTEX_ARRAY";
		case GL_ALPHA_TEST:
			return "GL_ALPHA_TEST";
		case GL_STENCIL_TEST:
			return "GL_STENCIL_TEST";
		default:
			sprintf(buffer, "0x%x", cap);
	}

	return buffer;
}

static const char *TypeToString(GLenum t)
{
	switch (t)
	{
		case GL_BYTE:
			return "GL_BYTE";
		case GL_UNSIGNED_BYTE:
			return "GL_UNSIGNED_BYTE";
		case GL_SHORT:
			return "GL_SHORT";
		case GL_UNSIGNED_SHORT:
			return "GL_UNSIGNED_SHORT";
		case GL_INT:
			return "GL_INT";
		case GL_UNSIGNED_INT:
			return "GL_UNSIGNED_INT";
		case GL_FLOAT:
			return "GL_FLOAT";
		case GL_DOUBLE:
			return "GL_DOUBLE";
		default:
			return "!!! UNKNOWN !!!";
	}
}

static void APIENTRY logBegin(GLenum mode)
{
	fprintf(log_fp, "glBegin( %s )\n", PrimToString(mode));
	dllBegin(mode);
}

static void APIENTRY logBindTexture(GLenum target, GLuint texture)
{
	fprintf(log_fp, "glBindTexture( 0x%x, %u )\n", target, texture);
	dllBindTexture(target, texture);
}

static void BlendToName(char *n, GLenum f)
{
	switch (f)
	{
		case GL_ONE:
			strcpy(n, "GL_ONE");
			break;
		case GL_ZERO:
			strcpy(n, "GL_ZERO");
			break;
		case GL_SRC_ALPHA:
			strcpy(n, "GL_SRC_ALPHA");
			break;
		case GL_ONE_MINUS_SRC_ALPHA:
			strcpy(n, "GL_ONE_MINUS_SRC_ALPHA");
			break;
		case GL_DST_COLOR:
			strcpy(n, "GL_DST_COLOR");
			break;
		case GL_ONE_MINUS_DST_COLOR:
			strcpy(n, "GL_ONE_MINUS_DST_COLOR");
			break;
		case GL_DST_ALPHA:
			strcpy(n, "GL_DST_ALPHA");
			break;
		default:
			sprintf(n, "0x%x", f);
	}
}
static void APIENTRY logBlendFunc(GLenum sfactor, GLenum dfactor)
{
	char            sf[128], df[128];

	BlendToName(sf, sfactor);
	BlendToName(df, dfactor);

	fprintf(log_fp, "glBlendFunc( %s, %s )\n", sf, df);
	dllBlendFunc(sfactor, dfactor);
}

static void APIENTRY logClear(GLbitfield mask)
{
	fprintf(log_fp, "glClear( 0x%x = ", mask);

	if(mask & GL_COLOR_BUFFER_BIT)
		fprintf(log_fp, "GL_COLOR_BUFFER_BIT ");
	if(mask & GL_DEPTH_BUFFER_BIT)
		fprintf(log_fp, "GL_DEPTH_BUFFER_BIT ");
	if(mask & GL_STENCIL_BUFFER_BIT)
		fprintf(log_fp, "GL_STENCIL_BUFFER_BIT ");
	if(mask & GL_ACCUM_BUFFER_BIT)
		fprintf(log_fp, "GL_ACCUM_BUFFER_BIT ");

	fprintf(log_fp, ")\n");
	dllClear(mask);
}

static void APIENTRY logClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
	fprintf(log_fp, "glClearColor\n");
	dllClearColor(red, green, blue, alpha);
}

static void APIENTRY logClearDepth(GLclampd depth)
{
	fprintf(log_fp, "glClearDepth( %f )\n", (float)depth);
	dllClearDepth(depth);
}

static void APIENTRY logClearStencil(GLint s)
{
	fprintf(log_fp, "glClearStencil( %d )\n", s);
	dllClearStencil(s);
}

static void APIENTRY logClipPlane(GLenum plane, const GLdouble * equation)
{
	fprintf(log_fp, "glClipPlane\n");
	dllClipPlane(plane, equation);
}



#define SIG( x ) fprintf( log_fp, x "\n" )


static void APIENTRY logColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
	SIG("glColorMask");
	dllColorMask(red, green, blue, alpha);
}

static void APIENTRY logCopyPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type)
{
	SIG("glCopyPixels");
	dllCopyPixels(x, y, width, height, type);
}

static void APIENTRY logCopyTexImage1D(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width,
									   GLint border)
{
	SIG("glCopyTexImage1D");
	dllCopyTexImage1D(target, level, internalFormat, x, y, width, border);
}

static void APIENTRY logCopyTexImage2D(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width,
									   GLsizei height, GLint border)
{
	SIG("glCopyTexImage2D");
	dllCopyTexImage2D(target, level, internalFormat, x, y, width, height, border);
}

static void APIENTRY logCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
	SIG("glCopyTexSubImage1D");
	dllCopyTexSubImage1D(target, level, xoffset, x, y, width);
}

static void APIENTRY logCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y,
										  GLsizei width, GLsizei height)
{
	SIG("glCopyTexSubImage2D");
	dllCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
}

static void APIENTRY logCullFace(GLenum mode)
{
	fprintf(log_fp, "glCullFace( %s )\n", (mode == GL_FRONT) ? "GL_FRONT" : "GL_BACK");
	dllCullFace(mode);
}

static void APIENTRY logDeleteTextures(GLsizei n, const GLuint * textures)
{
	SIG("glDeleteTextures");
	dllDeleteTextures(n, textures);
}

static void APIENTRY logDepthFunc(GLenum func)
{
	fprintf(log_fp, "glDepthFunc( %s )\n", FuncToString(func));
	dllDepthFunc(func);
}

static void APIENTRY logDepthMask(GLboolean flag)
{
	fprintf(log_fp, "glDepthMask( %s )\n", BooleanToString(flag));
	dllDepthMask(flag);
}

static void APIENTRY logDepthRange(GLclampd zNear, GLclampd zFar)
{
	fprintf(log_fp, "glDepthRange( %f, %f )\n", (float)zNear, (float)zFar);
	dllDepthRange(zNear, zFar);
}

static void APIENTRY logDisable(GLenum cap)
{
	fprintf(log_fp, "glDisable( %s )\n", CapToString(cap));
	dllDisable(cap);
}

static void APIENTRY logDrawArrays(GLenum mode, GLint first, GLsizei count)
{
	SIG("glDrawArrays");
	dllDrawArrays(mode, first, count);
}

static void APIENTRY logDrawBuffer(GLenum mode)
{
	SIG("glDrawBuffer");
	dllDrawBuffer(mode);
}

static void APIENTRY logDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices)
{
	fprintf(log_fp, "glDrawElements( %s, %d, %s, MEM )\n", PrimToString(mode), count, TypeToString(type));
	dllDrawElements(mode, count, type, indices);
}

static void APIENTRY logEnable(GLenum cap)
{
	fprintf(log_fp, "glEnable( %s )\n", CapToString(cap));
	dllEnable(cap);
}

static void APIENTRY logEnd(void)
{
	SIG("glEnd");
	dllEnd();
}

static void APIENTRY logFinish(void)
{
	SIG("glFinish");
	dllFinish();
}

static void APIENTRY logFlush(void)
{
	SIG("glFlush");
	dllFlush();
}

static void APIENTRY logFrontFace(GLenum mode)
{
	SIG("glFrontFace");
	dllFrontFace(mode);
}

static void APIENTRY logGenTextures(GLsizei n, GLuint * textures)
{
	SIG("glGenTextures");
	dllGenTextures(n, textures);
}

static void APIENTRY logGetBooleanv(GLenum pname, GLboolean * params)
{
	SIG("glGetBooleanv");
	dllGetBooleanv(pname, params);
}

static void APIENTRY logGetClipPlane(GLenum plane, GLdouble * equation)
{
	SIG("glGetClipPlane");
	dllGetClipPlane(plane, equation);
}

static void APIENTRY logGetDoublev(GLenum pname, GLdouble * params)
{
	SIG("glGetDoublev");
	dllGetDoublev(pname, params);
}

static GLenum APIENTRY logGetError(void)
{
	SIG("glGetError");
	return dllGetError();
}

static void APIENTRY logGetFloatv(GLenum pname, GLfloat * params)
{
	SIG("glGetFloatv");
	dllGetFloatv(pname, params);
}

static void APIENTRY logGetIntegerv(GLenum pname, GLint * params)
{
	SIG("glGetIntegerv");
	dllGetIntegerv(pname, params);
}

static void APIENTRY logGetLightfv(GLenum light, GLenum pname, GLfloat * params)
{
	SIG("glGetLightfv");
	dllGetLightfv(light, pname, params);
}

static void APIENTRY logGetLightiv(GLenum light, GLenum pname, GLint * params)
{
	SIG("glGetLightiv");
	dllGetLightiv(light, pname, params);
}

static void APIENTRY logGetMapdv(GLenum target, GLenum query, GLdouble * v)
{
	SIG("glGetMapdv");
	dllGetMapdv(target, query, v);
}

static void APIENTRY logGetMapfv(GLenum target, GLenum query, GLfloat * v)
{
	SIG("glGetMapfv");
	dllGetMapfv(target, query, v);
}

static void APIENTRY logGetMapiv(GLenum target, GLenum query, GLint * v)
{
	SIG("glGetMapiv");
	dllGetMapiv(target, query, v);
}

static void APIENTRY logGetMaterialfv(GLenum face, GLenum pname, GLfloat * params)
{
	SIG("glGetMaterialfv");
	dllGetMaterialfv(face, pname, params);
}

static void APIENTRY logGetMaterialiv(GLenum face, GLenum pname, GLint * params)
{
	SIG("glGetMaterialiv");
	dllGetMaterialiv(face, pname, params);
}

static void APIENTRY logGetPixelMapfv(GLenum map, GLfloat * values)
{
	SIG("glGetPixelMapfv");
	dllGetPixelMapfv(map, values);
}

static void APIENTRY logGetPixelMapuiv(GLenum map, GLuint * values)
{
	SIG("glGetPixelMapuiv");
	dllGetPixelMapuiv(map, values);
}

static void APIENTRY logGetPixelMapusv(GLenum map, GLushort * values)
{
	SIG("glGetPixelMapusv");
	dllGetPixelMapusv(map, values);
}

static void APIENTRY logGetPointerv(GLenum pname, GLvoid * *params)
{
	SIG("glGetPointerv");
	dllGetPointerv(pname, params);
}

static void APIENTRY logGetPolygonStipple(GLubyte * mask)
{
	SIG("glGetPolygonStipple");
	dllGetPolygonStipple(mask);
}

static const GLubyte *APIENTRY logGetString(GLenum name)
{
	SIG("glGetString");
	return dllGetString(name);
}

static void APIENTRY logGetTexEnvfv(GLenum target, GLenum pname, GLfloat * params)
{
	SIG("glGetTexEnvfv");
	dllGetTexEnvfv(target, pname, params);
}

static void APIENTRY logGetTexEnviv(GLenum target, GLenum pname, GLint * params)
{
	SIG("glGetTexEnviv");
	dllGetTexEnviv(target, pname, params);
}

static void APIENTRY logGetTexGendv(GLenum coord, GLenum pname, GLdouble * params)
{
	SIG("glGetTexGendv");
	dllGetTexGendv(coord, pname, params);
}

static void APIENTRY logGetTexGenfv(GLenum coord, GLenum pname, GLfloat * params)
{
	SIG("glGetTexGenfv");
	dllGetTexGenfv(coord, pname, params);
}

static void APIENTRY logGetTexGeniv(GLenum coord, GLenum pname, GLint * params)
{
	SIG("glGetTexGeniv");
	dllGetTexGeniv(coord, pname, params);
}

static void APIENTRY logGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, void *pixels)
{
	SIG("glGetTexImage");
	dllGetTexImage(target, level, format, type, pixels);
}
static void APIENTRY logGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat * params)
{
	SIG("glGetTexLevelParameterfv");
	dllGetTexLevelParameterfv(target, level, pname, params);
}

static void APIENTRY logGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint * params)
{
	SIG("glGetTexLevelParameteriv");
	dllGetTexLevelParameteriv(target, level, pname, params);
}

static void APIENTRY logGetTexParameterfv(GLenum target, GLenum pname, GLfloat * params)
{
	SIG("glGetTexParameterfv");
	dllGetTexParameterfv(target, pname, params);
}

static void APIENTRY logGetTexParameteriv(GLenum target, GLenum pname, GLint * params)
{
	SIG("glGetTexParameteriv");
	dllGetTexParameteriv(target, pname, params);
}

static void APIENTRY logHint(GLenum target, GLenum mode)
{
	fprintf(log_fp, "glHint( 0x%x, 0x%x )\n", target, mode);
	dllHint(target, mode);
}

static void APIENTRY logInitNames(void)
{
	SIG("glInitNames");
	dllInitNames();
}

static void APIENTRY logInterleavedArrays(GLenum format, GLsizei stride, const void *pointer)
{
	SIG("glInterleavedArrays");
	dllInterleavedArrays(format, stride, pointer);
}

static GLboolean APIENTRY logIsEnabled(GLenum cap)
{
	SIG("glIsEnabled");
	return dllIsEnabled(cap);
}

static GLboolean APIENTRY logIsTexture(GLuint texture)
{
	SIG("glIsTexture");
	return dllIsTexture(texture);
}

static void APIENTRY logLineWidth(GLfloat width)
{
	SIG("glLineWidth");
	dllLineWidth(width);
}

static void APIENTRY logPolygonMode(GLenum face, GLenum mode)
{
	fprintf(log_fp, "glPolygonMode( 0x%x, 0x%x )\n", face, mode);
	dllPolygonMode(face, mode);
}

static void APIENTRY logPolygonOffset(GLfloat factor, GLfloat units)
{
	SIG("glPolygonOffset");
	dllPolygonOffset(factor, units);
}
static void APIENTRY logPolygonStipple(const GLubyte * mask)
{
	SIG("glPolygonStipple");
	dllPolygonStipple(mask);
}
static void APIENTRY logPopAttrib(void)
{
	SIG("glPopAttrib");
	dllPopAttrib();
}

static void APIENTRY logPopClientAttrib(void)
{
	SIG("glPopClientAttrib");
	dllPopClientAttrib();
}

static void APIENTRY logPopName(void)
{
	SIG("glPopName");
	dllPopName();
}

static void APIENTRY logPrioritizeTextures(GLsizei n, const GLuint * textures, const GLclampf * priorities)
{
	SIG("glPrioritizeTextures");
	dllPrioritizeTextures(n, textures, priorities);
}

static void APIENTRY logPushAttrib(GLbitfield mask)
{
	SIG("glPushAttrib");
	dllPushAttrib(mask);
}

static void APIENTRY logPushClientAttrib(GLbitfield mask)
{
	SIG("glPushClientAttrib");
	dllPushClientAttrib(mask);
}

static void APIENTRY logPushName(GLuint name)
{
	SIG("glPushName");
	dllPushName(name);
}

static void APIENTRY logRasterPos2d(GLdouble x, GLdouble y)
{
	SIG("glRasterPot2d");
	dllRasterPos2d(x, y);
}

static void APIENTRY logRasterPos2dv(const GLdouble * v)
{
	SIG("glRasterPos2dv");
	dllRasterPos2dv(v);
}

static void APIENTRY logRasterPos2f(GLfloat x, GLfloat y)
{
	SIG("glRasterPos2f");
	dllRasterPos2f(x, y);
}
static void APIENTRY logRasterPos2fv(const GLfloat * v)
{
	SIG("glRasterPos2dv");
	dllRasterPos2fv(v);
}
static void APIENTRY logRasterPos2i(GLint x, GLint y)
{
	SIG("glRasterPos2if");
	dllRasterPos2i(x, y);
}
static void APIENTRY logRasterPos2iv(const GLint * v)
{
	SIG("glRasterPos2iv");
	dllRasterPos2iv(v);
}
static void APIENTRY logRasterPos2s(GLshort x, GLshort y)
{
	SIG("glRasterPos2s");
	dllRasterPos2s(x, y);
}
static void APIENTRY logRasterPos2sv(const GLshort * v)
{
	SIG("glRasterPos2sv");
	dllRasterPos2sv(v);
}
static void APIENTRY logRasterPos3d(GLdouble x, GLdouble y, GLdouble z)
{
	SIG("glRasterPos3d");
	dllRasterPos3d(x, y, z);
}
static void APIENTRY logRasterPos3dv(const GLdouble * v)
{
	SIG("glRasterPos3dv");
	dllRasterPos3dv(v);
}
static void APIENTRY logRasterPos3f(GLfloat x, GLfloat y, GLfloat z)
{
	SIG("glRasterPos3f");
	dllRasterPos3f(x, y, z);
}
static void APIENTRY logRasterPos3fv(const GLfloat * v)
{
	SIG("glRasterPos3fv");
	dllRasterPos3fv(v);
}
static void APIENTRY logRasterPos3i(GLint x, GLint y, GLint z)
{
	SIG("glRasterPos3i");
	dllRasterPos3i(x, y, z);
}
static void APIENTRY logRasterPos3iv(const GLint * v)
{
	SIG("glRasterPos3iv");
	dllRasterPos3iv(v);
}
static void APIENTRY logRasterPos3s(GLshort x, GLshort y, GLshort z)
{
	SIG("glRasterPos3s");
	dllRasterPos3s(x, y, z);
}
static void APIENTRY logRasterPos3sv(const GLshort * v)
{
	SIG("glRasterPos3sv");
	dllRasterPos3sv(v);
}
static void APIENTRY logRasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	SIG("glRasterPos4d");
	dllRasterPos4d(x, y, z, w);
}
static void APIENTRY logRasterPos4dv(const GLdouble * v)
{
	SIG("glRasterPos4dv");
	dllRasterPos4dv(v);
}
static void APIENTRY logRasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	SIG("glRasterPos4f");
	dllRasterPos4f(x, y, z, w);
}
static void APIENTRY logRasterPos4fv(const GLfloat * v)
{
	SIG("glRasterPos4fv");
	dllRasterPos4fv(v);
}
static void APIENTRY logRasterPos4i(GLint x, GLint y, GLint z, GLint w)
{
	SIG("glRasterPos4i");
	dllRasterPos4i(x, y, z, w);
}
static void APIENTRY logRasterPos4iv(const GLint * v)
{
	SIG("glRasterPos4iv");
	dllRasterPos4iv(v);
}
static void APIENTRY logRasterPos4s(GLshort x, GLshort y, GLshort z, GLshort w)
{
	SIG("glRasterPos4s");
	dllRasterPos4s(x, y, z, w);
}
static void APIENTRY logRasterPos4sv(const GLshort * v)
{
	SIG("glRasterPos4sv");
	dllRasterPos4sv(v);
}
static void APIENTRY logReadBuffer(GLenum mode)
{
	SIG("glReadBuffer");
	dllReadBuffer(mode);
}
static void APIENTRY logReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels)
{
	SIG("glReadPixels");
	dllReadPixels(x, y, width, height, format, type, pixels);
}

static void APIENTRY logRectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2)
{
	SIG("glRectd");
	dllRectd(x1, y1, x2, y2);
}

static void APIENTRY logRectdv(const GLdouble * v1, const GLdouble * v2)
{
	SIG("glRectdv");
	dllRectdv(v1, v2);
}

static void APIENTRY logRectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
	SIG("glRectf");
	dllRectf(x1, y1, x2, y2);
}

static void APIENTRY logRectfv(const GLfloat * v1, const GLfloat * v2)
{
	SIG("glRectfv");
	dllRectfv(v1, v2);
}
static void APIENTRY logRecti(GLint x1, GLint y1, GLint x2, GLint y2)
{
	SIG("glRecti");
	dllRecti(x1, y1, x2, y2);
}
static void APIENTRY logRectiv(const GLint * v1, const GLint * v2)
{
	SIG("glRectiv");
	dllRectiv(v1, v2);
}
static void APIENTRY logRects(GLshort x1, GLshort y1, GLshort x2, GLshort y2)
{
	SIG("glRects");
	dllRects(x1, y1, x2, y2);
}
static void APIENTRY logRectsv(const GLshort * v1, const GLshort * v2)
{
	SIG("glRectsv");
	dllRectsv(v1, v2);
}
static GLint APIENTRY logRenderMode(GLenum mode)
{
	SIG("glRenderMode");
	return dllRenderMode(mode);
}
static void APIENTRY logRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
	SIG("glRotated");
	dllRotated(angle, x, y, z);
}

static void APIENTRY logRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
	SIG("glRotatef");
	dllRotatef(angle, x, y, z);
}

static void APIENTRY logScaled(GLdouble x, GLdouble y, GLdouble z)
{
	SIG("glScaled");
	dllScaled(x, y, z);
}

static void APIENTRY logScalef(GLfloat x, GLfloat y, GLfloat z)
{
	SIG("glScalef");
	dllScalef(x, y, z);
}

static void APIENTRY logScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
	fprintf(log_fp, "glScissor( %d, %d, %d, %d )\n", x, y, width, height);
	dllScissor(x, y, width, height);
}

static void APIENTRY logSelectBuffer(GLsizei size, GLuint * buffer)
{
	SIG("glSelectBuffer");
	dllSelectBuffer(size, buffer);
}

static void APIENTRY logShadeModel(GLenum mode)
{
	SIG("glShadeModel");
	dllShadeModel(mode);
}

static void APIENTRY logStencilFunc(GLenum func, GLint ref, GLuint mask)
{
	SIG("glStencilFunc");
	dllStencilFunc(func, ref, mask);
}

static void APIENTRY logStencilMask(GLuint mask)
{
	SIG("glStencilMask");
	dllStencilMask(mask);
}

static void APIENTRY logStencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
	SIG("glStencilOp");
	dllStencilOp(fail, zfail, zpass);
}

static void APIENTRY logTexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format,
								   GLenum type, const void *pixels)
{
	SIG("glTexImage1D");
	dllTexImage1D(target, level, internalformat, width, border, format, type, pixels);
}
static void APIENTRY logTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border,
								   GLenum format, GLenum type, const void *pixels)
{
	SIG("glTexImage2D");
	dllTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
}

static void APIENTRY logTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
	fprintf(log_fp, "glTexParameterf( 0x%x, 0x%x, %f )\n", target, pname, param);
	dllTexParameterf(target, pname, param);
}

static void APIENTRY logTexParameterfv(GLenum target, GLenum pname, const GLfloat * params)
{
	SIG("glTexParameterfv");
	dllTexParameterfv(target, pname, params);
}
static void APIENTRY logTexParameteri(GLenum target, GLenum pname, GLint param)
{
	fprintf(log_fp, "glTexParameteri( 0x%x, 0x%x, 0x%x )\n", target, pname, param);
	dllTexParameteri(target, pname, param);
}
static void APIENTRY logTexParameteriv(GLenum target, GLenum pname, const GLint * params)
{
	SIG("glTexParameteriv");
	dllTexParameteriv(target, pname, params);
}
static void APIENTRY logTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type,
									  const void *pixels)
{
	SIG("glTexSubImage1D");
	dllTexSubImage1D(target, level, xoffset, width, format, type, pixels);
}
static void APIENTRY logTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height,
									  GLenum format, GLenum type, const void *pixels)
{
	SIG("glTexSubImage2D");
	dllTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
}
static void APIENTRY logTranslated(GLdouble x, GLdouble y, GLdouble z)
{
	SIG("glTranslated");
	dllTranslated(x, y, z);
}

static void APIENTRY logTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
	SIG("glTranslatef");
	dllTranslatef(x, y, z);
}

static void APIENTRY logViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
	fprintf(log_fp, "glViewport( %d, %d, %d, %d )\n", x, y, width, height);
	dllViewport(x, y, width, height);
}

/*
** QGL_Shutdown
**
** Unloads the specified DLL then nulls out all the proc pointers.  This
** is only called during a hard shutdown of the OGL subsystem (e.g. vid_restart).
*/
// *INDENT-OFF*
void QGL_Shutdown(void)
{
	ri.Printf(PRINT_ALL, "...shutting down QGL\n");

	qglBegin                     = NULL;
	qglBindTexture               = NULL;
	qglBlendFunc                 = NULL;
	qglClear                     = NULL;
	qglClearColor                = NULL;
	qglClearDepth                = NULL;
	qglClearStencil              = NULL;
	qglClipPlane                 = NULL;
	qglColorMask                 = NULL;
	qglCopyPixels                = NULL;
	qglCopyTexImage1D            = NULL;
	qglCopyTexImage2D            = NULL;
	qglCopyTexSubImage1D         = NULL;
	qglCopyTexSubImage2D         = NULL;
	qglCullFace                  = NULL;
	qglDeleteTextures            = NULL;
	qglDepthFunc                 = NULL;
	qglDepthMask                 = NULL;
	qglDepthRange                = NULL;
	qglDisable                   = NULL;
	qglDrawArrays                = NULL;
	qglDrawBuffer                = NULL;
	qglDrawElements              = NULL;
	qglEnable                    = NULL;
	qglEnd                       = NULL;
	qglFinish                    = NULL;
	qglFlush                     = NULL;
	qglFrontFace                 = NULL;
	qglGenTextures               = NULL;
	qglGetBooleanv               = NULL;
	qglGetError                  = NULL;
	qglGetFloatv                 = NULL;
	qglGetIntegerv               = NULL;
	qglGetString                 = NULL;
	qglGetTexParameterfv         = NULL;
	qglGetTexParameteriv         = NULL;
	qglHint                      = NULL;
	qglIsEnabled                 = NULL;
	qglIsTexture                 = NULL;
	qglLineWidth                 = NULL;
	qglPolygonMode               = NULL;
	qglPolygonOffset             = NULL;
	qglPolygonStipple            = NULL;
	qglPopAttrib                 = NULL;
	qglPopClientAttrib           = NULL;
	qglPopName                   = NULL;
	qglPrioritizeTextures        = NULL;
	qglPushAttrib                = NULL;
	qglPushClientAttrib          = NULL;
	qglPushName                  = NULL;
	qglRasterPos2d               = NULL;
	qglRasterPos2dv              = NULL;
	qglRasterPos2f               = NULL;
	qglRasterPos2fv              = NULL;
	qglRasterPos2i               = NULL;
	qglRasterPos2iv              = NULL;
	qglRasterPos2s               = NULL;
	qglRasterPos2sv              = NULL;
	qglRasterPos3d               = NULL;
	qglRasterPos3dv              = NULL;
	qglRasterPos3f               = NULL;
	qglRasterPos3fv              = NULL;
	qglRasterPos3i               = NULL;
	qglRasterPos3iv              = NULL;
	qglRasterPos3s               = NULL;
	qglRasterPos3sv              = NULL;
	qglRasterPos4d               = NULL;
	qglRasterPos4dv              = NULL;
	qglRasterPos4f               = NULL;
	qglRasterPos4fv              = NULL;
	qglRasterPos4i               = NULL;
	qglRasterPos4iv              = NULL;
	qglRasterPos4s               = NULL;
	qglRasterPos4sv              = NULL;
	qglReadBuffer                = NULL;
	qglReadPixels                = NULL;
	qglRectd                     = NULL;
	qglRectdv                    = NULL;
	qglRectf                     = NULL;
	qglRectfv                    = NULL;
	qglRecti                     = NULL;
	qglRectiv                    = NULL;
	qglRects                     = NULL;
	qglRectsv                    = NULL;
	qglRenderMode                = NULL;
	qglRotated                   = NULL;
	qglRotatef                   = NULL;
	qglScaled                    = NULL;
	qglScalef                    = NULL;
	qglScissor                   = NULL;
	qglSelectBuffer              = NULL;
	qglShadeModel                = NULL;
	qglStencilFunc               = NULL;
	qglStencilMask               = NULL;
	qglStencilOp                 = NULL;
	qglTexImage1D                = NULL;
	qglTexImage2D                = NULL;
	qglTexParameterf             = NULL;
	qglTexParameterfv            = NULL;
	qglTexParameteri             = NULL;
	qglTexParameteriv            = NULL;
	qglTexSubImage1D             = NULL;
	qglTexSubImage2D             = NULL;
	qglTranslated                = NULL;
	qglTranslatef                = NULL;
	qglViewport                  = NULL;
}
// *INDENT-ON*

#define GPA(a) SDL_GL_GetProcAddress(a)

qboolean GLimp_sdl_init_video(void)
{
	if(!SDL_WasInit(SDL_INIT_VIDEO))
	{
		ri.Printf(PRINT_ALL, "Calling SDL_Init(SDL_INIT_VIDEO)...\n");
		if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) == -1)
		{
			ri.Printf(PRINT_ALL, "SDL_Init(SDL_INIT_VIDEO) failed: %s\n", SDL_GetError());
			return qfalse;
		}
		ri.Printf(PRINT_ALL, "SDL_Init(SDL_INIT_VIDEO) passed.\n");
	}

	return qtrue;
}

/*
** QGL_Init
**
** This is responsible for binding our qgl function pointers to 
** the appropriate GL stuff.  In Windows this means doing a 
** LoadLibrary and a bunch of calls to GetProcAddress.  On other
** operating systems we need to do the right thing, whatever that
** might be.
*/
// *INDENT-OFF*
int QGL_Init()
{
	ri.Printf(PRINT_ALL, "...initializing QGL\n");

	if(GLimp_sdl_init_video() == qfalse)
		return qfalse;

	qglBegin                     = dllBegin = GPA( "glBegin" );
	qglBindTexture               = dllBindTexture = GPA( "glBindTexture" );
	qglBlendFunc                 = dllBlendFunc = GPA( "glBlendFunc" );
	qglClear                     = dllClear = GPA( "glClear" );
	qglClearColor                = dllClearColor = GPA( "glClearColor" );
	qglClearDepth                = dllClearDepth = GPA( "glClearDepth" );
	qglClearStencil              = dllClearStencil = GPA( "glClearStencil" );
	qglClipPlane                 = dllClipPlane = GPA( "glClipPlane" );
	qglColorMask                 = dllColorMask = GPA( "glColorMask" );
	qglCopyPixels                = dllCopyPixels = GPA( "glCopyPixels" );
	qglCopyTexImage1D            = dllCopyTexImage1D = GPA( "glCopyTexImage1D" );
	qglCopyTexImage2D            = dllCopyTexImage2D = GPA( "glCopyTexImage2D" );
	qglCopyTexSubImage1D         = dllCopyTexSubImage1D = GPA( "glCopyTexSubImage1D" );
	qglCopyTexSubImage2D         = dllCopyTexSubImage2D = GPA( "glCopyTexSubImage2D" );
	qglCullFace                  = dllCullFace = GPA( "glCullFace" );
	qglDeleteTextures            = dllDeleteTextures = GPA( "glDeleteTextures" );
	qglDepthFunc                 = dllDepthFunc = GPA( "glDepthFunc" );
	qglDepthMask                 = dllDepthMask = GPA( "glDepthMask" );
	qglDepthRange                = dllDepthRange = GPA( "glDepthRange" );
	qglDisable                   = dllDisable = GPA( "glDisable" );
	qglDrawArrays                = dllDrawArrays = GPA( "glDrawArrays" );
	qglDrawBuffer                = dllDrawBuffer = GPA( "glDrawBuffer" );
	qglDrawElements              = dllDrawElements = GPA( "glDrawElements" );
	qglEnable                    = 	dllEnable                    = GPA( "glEnable" );
	qglEnd                       = 	dllEnd                       = GPA( "glEnd" );
	qglFinish                    = 	dllFinish                    = GPA( "glFinish" );
	qglFlush                     = 	dllFlush                     = GPA( "glFlush" );
	qglFrontFace                 = 	dllFrontFace                 = GPA( "glFrontFace" );
	qglGenTextures               = 	dllGenTextures               = GPA( "glGenTextures" );
	qglGetBooleanv               = 	dllGetBooleanv               = GPA( "glGetBooleanv" );
	qglGetError                  = 	dllGetError                  = GPA( "glGetError" );
	qglGetFloatv                 = 	dllGetFloatv                 = GPA( "glGetFloatv" );
	qglGetIntegerv               = 	dllGetIntegerv               = GPA( "glGetIntegerv" );
	qglGetString                 = 	dllGetString                 = GPA( "glGetString" );
	qglGetTexParameterfv         = 	dllGetTexParameterfv         = GPA( "glGetTexParameterfv" );
	qglGetTexParameteriv         = 	dllGetTexParameteriv         = GPA( "glGetTexParameteriv" );
	qglHint                      = 	dllHint                      = GPA( "glHint" );
	qglIsEnabled                 = 	dllIsEnabled                 = GPA( "glIsEnabled" );
	qglIsTexture                 = 	dllIsTexture                 = GPA( "glIsTexture" );
	qglLineWidth                 = 	dllLineWidth                 = GPA( "glLineWidth" );
	qglPolygonMode               = 	dllPolygonMode               = GPA( "glPolygonMode" );
	qglPolygonOffset             = 	dllPolygonOffset             = GPA( "glPolygonOffset" );
	qglPolygonStipple            = 	dllPolygonStipple            = GPA( "glPolygonStipple" );
	qglPopAttrib                 = 	dllPopAttrib                 = GPA( "glPopAttrib" );
	qglPopClientAttrib           = 	dllPopClientAttrib           = GPA( "glPopClientAttrib" );
	qglPopName                   = 	dllPopName                   = GPA( "glPopName" );
	qglPrioritizeTextures        = 	dllPrioritizeTextures        = GPA( "glPrioritizeTextures" );
	qglPushAttrib                = 	dllPushAttrib                = GPA( "glPushAttrib" );
	qglPushClientAttrib          = 	dllPushClientAttrib          = GPA( "glPushClientAttrib" );
	qglPushName                  = 	dllPushName                  = GPA( "glPushName" );
	qglRasterPos2d               = 	dllRasterPos2d               = GPA( "glRasterPos2d" );
	qglRasterPos2dv              = 	dllRasterPos2dv              = GPA( "glRasterPos2dv" );
	qglRasterPos2f               = 	dllRasterPos2f               = GPA( "glRasterPos2f" );
	qglRasterPos2fv              = 	dllRasterPos2fv              = GPA( "glRasterPos2fv" );
	qglRasterPos2i               = 	dllRasterPos2i               = GPA( "glRasterPos2i" );
	qglRasterPos2iv              = 	dllRasterPos2iv              = GPA( "glRasterPos2iv" );
	qglRasterPos2s               = 	dllRasterPos2s               = GPA( "glRasterPos2s" );
	qglRasterPos2sv              = 	dllRasterPos2sv              = GPA( "glRasterPos2sv" );
	qglRasterPos3d               = 	dllRasterPos3d               = GPA( "glRasterPos3d" );
	qglRasterPos3dv              = 	dllRasterPos3dv              = GPA( "glRasterPos3dv" );
	qglRasterPos3f               = 	dllRasterPos3f               = GPA( "glRasterPos3f" );
	qglRasterPos3fv              = 	dllRasterPos3fv              = GPA( "glRasterPos3fv" );
	qglRasterPos3i               = 	dllRasterPos3i               = GPA( "glRasterPos3i" );
	qglRasterPos3iv              = 	dllRasterPos3iv              = GPA( "glRasterPos3iv" );
	qglRasterPos3s               = 	dllRasterPos3s               = GPA( "glRasterPos3s" );
	qglRasterPos3sv              = 	dllRasterPos3sv              = GPA( "glRasterPos3sv" );
	qglRasterPos4d               = 	dllRasterPos4d               = GPA( "glRasterPos4d" );
	qglRasterPos4dv              = 	dllRasterPos4dv              = GPA( "glRasterPos4dv" );
	qglRasterPos4f               = 	dllRasterPos4f               = GPA( "glRasterPos4f" );
	qglRasterPos4fv              = 	dllRasterPos4fv              = GPA( "glRasterPos4fv" );
	qglRasterPos4i               = 	dllRasterPos4i               = GPA( "glRasterPos4i" );
	qglRasterPos4iv              = 	dllRasterPos4iv              = GPA( "glRasterPos4iv" );
	qglRasterPos4s               = 	dllRasterPos4s               = GPA( "glRasterPos4s" );
	qglRasterPos4sv              = 	dllRasterPos4sv              = GPA( "glRasterPos4sv" );
	qglReadBuffer                = 	dllReadBuffer                = GPA( "glReadBuffer" );
	qglReadPixels                = 	dllReadPixels                = GPA( "glReadPixels" );
	qglRectd                     = 	dllRectd                     = GPA( "glRectd" );
	qglRectdv                    = 	dllRectdv                    = GPA( "glRectdv" );
	qglRectf                     = 	dllRectf                     = GPA( "glRectf" );
	qglRectfv                    = 	dllRectfv                    = GPA( "glRectfv" );
	qglRecti                     = 	dllRecti                     = GPA( "glRecti" );
	qglRectiv                    = 	dllRectiv                    = GPA( "glRectiv" );
	qglRects                     = 	dllRects                     = GPA( "glRects" );
	qglRectsv                    = 	dllRectsv                    = GPA( "glRectsv" );
	qglRenderMode                = 	dllRenderMode                = GPA( "glRenderMode" );
	qglRotated                   = 	dllRotated                   = GPA( "glRotated" );
	qglRotatef                   = 	dllRotatef                   = GPA( "glRotatef" );
	qglScaled                    = 	dllScaled                    = GPA( "glScaled" );
	qglScalef                    = 	dllScalef                    = GPA( "glScalef" );
	qglScissor                   = 	dllScissor                   = GPA( "glScissor" );
	qglSelectBuffer              = 	dllSelectBuffer              = GPA( "glSelectBuffer" );
	qglShadeModel                = 	dllShadeModel                = GPA( "glShadeModel" );
	qglStencilFunc               = 	dllStencilFunc               = GPA( "glStencilFunc" );
	qglStencilMask               = 	dllStencilMask               = GPA( "glStencilMask" );
	qglStencilOp                 = 	dllStencilOp                 = GPA( "glStencilOp" );
	qglTexImage1D                = 	dllTexImage1D                = GPA( "glTexImage1D" );
	qglTexImage2D                = 	dllTexImage2D                = GPA( "glTexImage2D" );
	qglTexParameterf             = 	dllTexParameterf             = GPA( "glTexParameterf" );
	qglTexParameterfv            = 	dllTexParameterfv            = GPA( "glTexParameterfv" );
	qglTexParameteri             = 	dllTexParameteri             = GPA( "glTexParameteri" );
	qglTexParameteriv            = 	dllTexParameteriv            = GPA( "glTexParameteriv" );
	qglTexSubImage1D             = 	dllTexSubImage1D             = GPA( "glTexSubImage1D" );
	qglTexSubImage2D             = 	dllTexSubImage2D             = GPA( "glTexSubImage2D" );
	qglTranslated                = 	dllTranslated                = GPA( "glTranslated" );
	qglTranslatef                = 	dllTranslatef                = GPA( "glTranslatef" );
	qglViewport                  = 	dllViewport                  = GPA( "glViewport" );
	
	// check logging
	QGL_EnableLogging(r_logFile->integer);

	return qtrue;
}
// *INDENT-ON*

// *INDENT-OFF*
void QGL_EnableLogging(int enable)
{
	// fixed for new countdown
	static qboolean isEnabled = qfalse;	// init

	// return if we're already active
	if(isEnabled && enable)
	{
		// decrement log counter and stop if it has reached 0
		ri.Cvar_Set("r_logFile", va("%d", r_logFile->integer - 1));
		if(r_logFile->integer)
		{
			return;
		}
		enable = qfalse;
	}

	// return if we're already disabled
	if(!enable && !isEnabled)
		return;

	isEnabled = enable;

	// old code starts here
	if(enable)
	{
		if(!log_fp)
		{
			struct tm      *newtime;
			time_t          aclock;
			char            buffer[1024];
			cvar_t         *basedir;

			time(&aclock);
			newtime = localtime(&aclock);

			asctime(newtime);

			basedir = ri.Cvar_Get("fs_basepath", "", 0);	// FIXME: userdir?
			assert(basedir);
			Com_sprintf(buffer, sizeof(buffer), "%s/gl.log", basedir->string);
			log_fp = fopen(buffer, "wt");
			assert(log_fp);
			ri.Printf(PRINT_ALL, "QGL_EnableLogging(%d): writing %s\n", r_logFile->integer, buffer);

			fprintf(log_fp, "%s\n", asctime(newtime));
		}

		qglBegin                     = logBegin;
		qglBindTexture               = logBindTexture;
		qglBlendFunc                 = logBlendFunc;
		qglClear                     = logClear;
		qglClearColor                = logClearColor;
		qglClearDepth                = logClearDepth;
		qglClearStencil              = logClearStencil;
		qglClipPlane                 = logClipPlane;
		qglColorMask                 = logColorMask;
		qglCopyPixels                = logCopyPixels;
		qglCopyTexImage1D            = logCopyTexImage1D;
		qglCopyTexImage2D            = logCopyTexImage2D;
		qglCopyTexSubImage1D         = logCopyTexSubImage1D;
		qglCopyTexSubImage2D         = logCopyTexSubImage2D;
		qglCullFace                  = logCullFace;
		qglDeleteTextures            = logDeleteTextures;
		qglDepthFunc                 = logDepthFunc;
		qglDepthMask                 = logDepthMask;
		qglDepthRange                = logDepthRange;
		qglDisable                   = logDisable;
		qglDrawArrays                = logDrawArrays;
		qglDrawBuffer                = logDrawBuffer;
		qglDrawElements              = logDrawElements;
		qglEnable                    = logEnable;
		qglEnd                       = logEnd;
		qglFinish                    = logFinish;
		qglFlush                     = logFlush;
		qglFrontFace                 = logFrontFace;
		qglGenTextures               = 	logGenTextures               ;
		qglGetBooleanv               = 	logGetBooleanv               ;
		qglGetError                  = 	logGetError                  ;
		qglGetFloatv                 = 	logGetFloatv                 ;
		qglGetIntegerv               = 	logGetIntegerv               ;
		qglGetString                 = 	logGetString                 ;
		qglGetTexParameterfv         = 	logGetTexParameterfv         ;
		qglGetTexParameteriv         = 	logGetTexParameteriv         ;
		qglHint                      = 	logHint                      ;
		qglIsEnabled                 = 	logIsEnabled                 ;
		qglIsTexture                 = 	logIsTexture                 ;
		qglLineWidth                 = 	logLineWidth                 ;
		qglPolygonMode               = 	logPolygonMode               ;
		qglPolygonOffset             = 	logPolygonOffset             ;
		qglPolygonStipple            = 	logPolygonStipple            ;
		qglPopAttrib                 = 	logPopAttrib                 ;
		qglPopClientAttrib           = 	logPopClientAttrib           ;
		qglPopName                   = 	logPopName                   ;
		qglPrioritizeTextures        = 	logPrioritizeTextures        ;
		qglPushAttrib                = 	logPushAttrib                ;
		qglPushClientAttrib          = 	logPushClientAttrib          ;
		qglPushName                  = 	logPushName                  ;
		qglRasterPos2d               = 	logRasterPos2d               ;
		qglRasterPos2dv              = 	logRasterPos2dv              ;
		qglRasterPos2f               = 	logRasterPos2f               ;
		qglRasterPos2fv              = 	logRasterPos2fv              ;
		qglRasterPos2i               = 	logRasterPos2i               ;
		qglRasterPos2iv              = 	logRasterPos2iv              ;
		qglRasterPos2s               = 	logRasterPos2s               ;
		qglRasterPos2sv              = 	logRasterPos2sv              ;
		qglRasterPos3d               = 	logRasterPos3d               ;
		qglRasterPos3dv              = 	logRasterPos3dv              ;
		qglRasterPos3f               = 	logRasterPos3f               ;
		qglRasterPos3fv              = 	logRasterPos3fv              ;
		qglRasterPos3i               = 	logRasterPos3i               ;
		qglRasterPos3iv              = 	logRasterPos3iv              ;
		qglRasterPos3s               = 	logRasterPos3s               ;
		qglRasterPos3sv              = 	logRasterPos3sv              ;
		qglRasterPos4d               = 	logRasterPos4d               ;
		qglRasterPos4dv              = 	logRasterPos4dv              ;
		qglRasterPos4f               = 	logRasterPos4f               ;
		qglRasterPos4fv              = 	logRasterPos4fv              ;
		qglRasterPos4i               = 	logRasterPos4i               ;
		qglRasterPos4iv              = 	logRasterPos4iv              ;
		qglRasterPos4s               = 	logRasterPos4s               ;
		qglRasterPos4sv              = 	logRasterPos4sv              ;
		qglReadBuffer                = 	logReadBuffer                ;
		qglReadPixels                = 	logReadPixels                ;
		qglRectd                     = 	logRectd                     ;
		qglRectdv                    = 	logRectdv                    ;
		qglRectf                     = 	logRectf                     ;
		qglRectfv                    = 	logRectfv                    ;
		qglRecti                     = 	logRecti                     ;
		qglRectiv                    = 	logRectiv                    ;
		qglRects                     = 	logRects                     ;
		qglRectsv                    = 	logRectsv                    ;
		qglRenderMode                = 	logRenderMode                ;
		qglRotated                   = 	logRotated                   ;
		qglRotatef                   = 	logRotatef                   ;
		qglScaled                    = 	logScaled                    ;
		qglScalef                    = 	logScalef                    ;
		qglScissor                   = 	logScissor                   ;
		qglSelectBuffer              = 	logSelectBuffer              ;
		qglShadeModel                = 	logShadeModel                ;
		qglStencilFunc               = 	logStencilFunc               ;
		qglStencilMask               = 	logStencilMask               ;
		qglStencilOp                 = 	logStencilOp                 ;
		qglTexImage1D                = 	logTexImage1D                ;
		qglTexImage2D                = 	logTexImage2D                ;
		qglTexParameterf             = 	logTexParameterf             ;
		qglTexParameterfv            = 	logTexParameterfv            ;
		qglTexParameteri             = 	logTexParameteri             ;
		qglTexParameteriv            = 	logTexParameteriv            ;
		qglTexSubImage1D             = 	logTexSubImage1D             ;
		qglTexSubImage2D             = 	logTexSubImage2D             ;
		qglTranslated                = 	logTranslated                ;
		qglTranslatef                = 	logTranslatef                ;
		qglViewport                  = 	logViewport                  ;
	}
	else
	{
		if(log_fp)
		{
			fprintf(log_fp, "*** CLOSING LOG ***\n");
			fclose(log_fp);
			log_fp = NULL;
		}
		qglBegin                     = dllBegin;
		qglBindTexture               = dllBindTexture;
		qglBlendFunc                 = dllBlendFunc;
		qglClear                     = dllClear;
		qglClearColor                = dllClearColor;
		qglClearDepth                = dllClearDepth;
		qglClearStencil              = dllClearStencil;
		qglClipPlane                 = dllClipPlane;
		qglColorMask                 = dllColorMask;
		qglCopyPixels                = dllCopyPixels;
		qglCopyTexImage1D            = dllCopyTexImage1D;
		qglCopyTexImage2D            = dllCopyTexImage2D;
		qglCopyTexSubImage1D         = dllCopyTexSubImage1D;
		qglCopyTexSubImage2D         = dllCopyTexSubImage2D;
		qglCullFace                  = dllCullFace;
		qglDeleteTextures            = dllDeleteTextures;
		qglDepthFunc                 = dllDepthFunc;
		qglDepthMask                 = dllDepthMask;
		qglDepthRange                = dllDepthRange;
		qglDisable                   = dllDisable;
		qglDrawArrays                = dllDrawArrays;
		qglDrawBuffer                = dllDrawBuffer;
		qglDrawElements              = dllDrawElements;
		qglEnable                    = dllEnable;
		qglEnd                       = dllEnd;
		qglFinish                    = dllFinish;
		qglFlush                     = dllFlush;
		qglFrontFace                 = dllFrontFace;
		qglGenTextures               = 	dllGenTextures               ;
		qglGetBooleanv               = 	dllGetBooleanv               ;
		qglGetError                  = 	dllGetError                  ;
		qglGetFloatv                 = 	dllGetFloatv                 ;
		qglGetIntegerv               = 	dllGetIntegerv               ;
		qglGetString                 = 	dllGetString                 ;
		qglGetTexParameterfv         = 	dllGetTexParameterfv         ;
		qglGetTexParameteriv         = 	dllGetTexParameteriv         ;
		qglHint                      = 	dllHint                      ;
		qglIsEnabled                 = 	dllIsEnabled                 ;
		qglIsTexture                 = 	dllIsTexture                 ;
		qglLineWidth                 = 	dllLineWidth                 ;
		qglPolygonMode               = 	dllPolygonMode               ;
		qglPolygonOffset             = 	dllPolygonOffset             ;
		qglPolygonStipple            = 	dllPolygonStipple            ;
		qglPopAttrib                 = 	dllPopAttrib                 ;
		qglPopClientAttrib           = 	dllPopClientAttrib           ;
		qglPopName                   = 	dllPopName                   ;
		qglPrioritizeTextures        = 	dllPrioritizeTextures        ;
		qglPushAttrib                = 	dllPushAttrib                ;
		qglPushClientAttrib          = 	dllPushClientAttrib          ;
		qglPushName                  = 	dllPushName                  ;
		qglRasterPos2d               = 	dllRasterPos2d               ;
		qglRasterPos2dv              = 	dllRasterPos2dv              ;
		qglRasterPos2f               = 	dllRasterPos2f               ;
		qglRasterPos2fv              = 	dllRasterPos2fv              ;
		qglRasterPos2i               = 	dllRasterPos2i               ;
		qglRasterPos2iv              = 	dllRasterPos2iv              ;
		qglRasterPos2s               = 	dllRasterPos2s               ;
		qglRasterPos2sv              = 	dllRasterPos2sv              ;
		qglRasterPos3d               = 	dllRasterPos3d               ;
		qglRasterPos3dv              = 	dllRasterPos3dv              ;
		qglRasterPos3f               = 	dllRasterPos3f               ;
		qglRasterPos3fv              = 	dllRasterPos3fv              ;
		qglRasterPos3i               = 	dllRasterPos3i               ;
		qglRasterPos3iv              = 	dllRasterPos3iv              ;
		qglRasterPos3s               = 	dllRasterPos3s               ;
		qglRasterPos3sv              = 	dllRasterPos3sv              ;
		qglRasterPos4d               = 	dllRasterPos4d               ;
		qglRasterPos4dv              = 	dllRasterPos4dv              ;
		qglRasterPos4f               = 	dllRasterPos4f               ;
		qglRasterPos4fv              = 	dllRasterPos4fv              ;
		qglRasterPos4i               = 	dllRasterPos4i               ;
		qglRasterPos4iv              = 	dllRasterPos4iv              ;
		qglRasterPos4s               = 	dllRasterPos4s               ;
		qglRasterPos4sv              = 	dllRasterPos4sv              ;
		qglReadBuffer                = 	dllReadBuffer                ;
		qglReadPixels                = 	dllReadPixels                ;
		qglRectd                     = 	dllRectd                     ;
		qglRectdv                    = 	dllRectdv                    ;
		qglRectf                     = 	dllRectf                     ;
		qglRectfv                    = 	dllRectfv                    ;
		qglRecti                     = 	dllRecti                     ;
		qglRectiv                    = 	dllRectiv                    ;
		qglRects                     = 	dllRects                     ;
		qglRectsv                    = 	dllRectsv                    ;
		qglRenderMode                = 	dllRenderMode                ;
		qglRotated                   = 	dllRotated                   ;
		qglRotatef                   = 	dllRotatef                   ;
		qglScaled                    = 	dllScaled                    ;
		qglScalef                    = 	dllScalef                    ;
		qglScissor                   = 	dllScissor                   ;
		qglSelectBuffer              = 	dllSelectBuffer              ;
		qglShadeModel                = 	dllShadeModel                ;
		qglStencilFunc               = 	dllStencilFunc               ;
		qglStencilMask               = 	dllStencilMask               ;
		qglStencilOp                 = 	dllStencilOp                 ;
		qglTexImage1D                = 	dllTexImage1D                ;
		qglTexImage2D                = 	dllTexImage2D                ;
		qglTexParameterf             = 	dllTexParameterf             ;
		qglTexParameterfv            = 	dllTexParameterfv            ;
		qglTexParameteri             = 	dllTexParameteri             ;
		qglTexParameteriv            = 	dllTexParameteriv            ;
		qglTexSubImage1D             = 	dllTexSubImage1D             ;
		qglTexSubImage2D             = 	dllTexSubImage2D             ;
		qglTranslated                = 	dllTranslated                ;
		qglTranslatef                = 	dllTranslatef                ;
		qglViewport                  = 	dllViewport                  ;
	}
}
// *INDENT-ON*


/*
===============
GLimp_LogComment
===============
*/
void GLimp_LogComment(char *comment)
{
	if(log_fp)
	{
		fprintf(log_fp, "%s", comment);
	}
}

#ifdef _MSC_VER
#pragma warning (default : 4113 4133 4047 )
#endif
