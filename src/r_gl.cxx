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

#ifdef _WIN32
BOOL	(*xglMakeCurrent)(HDC, HGLRC);
BOOL	(*xglDeleteContext)(HGLRC);
HGLRC	(*xglCreateContext)(HDC);
#endif


/// OpenGL 1.1 functions =======================================================
void (GLAPIENTRY* qglBegin)(GLenum mode, const char *filename, int line);
void (GLAPIENTRY* qglColor4f)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha, const char *filename, int line);
void (GLAPIENTRY* qglColor4fv)(const GLfloat *v, const char *filename, int line);
void (GLAPIENTRY* qglEnd)(const char *filename, int line);
void (GLAPIENTRY* qglTexCoord1f)(GLfloat s, const char *filename, int line);
void (GLAPIENTRY* qglTexCoord1fv)(const GLfloat *v, const char *filename, int line);
void (GLAPIENTRY* qglTexCoord2f)(GLfloat s, GLfloat t, const char *filename, int line);
void (GLAPIENTRY* qglTexCoord2fv)(const GLfloat *v, const char *filename, int line);
void (GLAPIENTRY* qglTexCoord3f)(GLfloat s, GLfloat t, GLfloat r, const char *filename, int line);
void (GLAPIENTRY* qglTexCoord3fv)(const GLfloat *v, const char *filename, int line);
void (GLAPIENTRY* qglVertex3f)(GLfloat x, GLfloat y, GLfloat z, const char *filename, int line);
void (GLAPIENTRY* qglVertex3fv)(const GLfloat *v, const char *filename, int line);
void (GLAPIENTRY* qglClipPlane)(GLenum plane, const GLdouble *equation, const char *filename, int line);
void (GLAPIENTRY* qglCullFace)(GLenum mode, const char *filename, int line);
void (GLAPIENTRY* qglFrontFace)(GLenum mode, const char *filename, int line);
void (GLAPIENTRY* qglPolygonMode)(GLenum face, GLenum mode, const char *filename, int line);
void (GLAPIENTRY* qglScissor)(GLint x, GLint y, GLsizei width, GLsizei height, const char *filename, int line);
void (GLAPIENTRY* qglTexParameterfv)(GLenum target, GLenum pname, const GLfloat *params, const char *filename, int line);
void (GLAPIENTRY* qglTexParameteri)(GLenum target, GLenum pname, GLint param, const char *filename, int line);
void (GLAPIENTRY* qglTexParameteriv)(GLenum target, GLenum pname, const GLint *params, const char *filename, int line);
void (GLAPIENTRY* qglTexImage1D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels, const char *filename, int line);
void (GLAPIENTRY* qglTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels, const char *filename, int line);
void (GLAPIENTRY* qglTexEnvi)(GLenum target, GLenum pname, GLint param, const char *filename, int line);
void (GLAPIENTRY* qglTexGenf)(GLenum coord, GLenum pname, GLfloat param, const char *filename, int line);
void (GLAPIENTRY* qglTexGenfv)(GLenum coord, GLenum pname, const GLfloat *params, const char *filename, int line);
void (GLAPIENTRY* qglTexGeni)(GLenum coord, GLenum pname, GLint param, const char *filename, int line);
void (GLAPIENTRY* qglTexGeniv)(GLenum coord, GLenum pname, const GLint *params, const char *filename, int line);
void (GLAPIENTRY* qglDrawBuffer)(GLenum mode, const char *filename, int line);
void (GLAPIENTRY* qglClear)(GLbitfield mask, const char *filename, int line);
void (GLAPIENTRY* qglClearColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha, const char *filename, int line);
void (GLAPIENTRY* qglClearStencil)(GLint s, const char *filename, int line);
void (GLAPIENTRY* qglColorMask)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha, const char *filename, int line);
void (GLAPIENTRY* qglDepthMask)(GLboolean flag, const char *filename, int line);
void (GLAPIENTRY* qglDisable)(GLenum cap, const char *filename, int line);
void (GLAPIENTRY* qglEnable)(GLenum cap, const char *filename, int line);
void (GLAPIENTRY* qglFlush)(const char *filename, int line);
void (GLAPIENTRY* qglPopAttrib)(const char *filename, int line);
void (GLAPIENTRY* qglPushAttrib)(GLbitfield mask, const char *filename, int line);
void (GLAPIENTRY* qglAlphaFunc)(GLenum func, GLclampf ref, const char *filename, int line);
void (GLAPIENTRY* qglBlendFunc)(GLenum sfactor, GLenum dfactor, const char *filename, int line);
void (GLAPIENTRY* qglStencilFunc)(GLenum func, GLint ref, GLuint mask, const char *filename, int line);
void (GLAPIENTRY* qglStencilOp)(GLenum fail, GLenum zfail, GLenum zpass, const char *filename, int line);
void (GLAPIENTRY* qglDepthFunc)(GLenum func, const char *filename, int line);
void (GLAPIENTRY* qglReadBuffer)(GLenum mode, const char *filename, int line);
void (GLAPIENTRY* qglReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels, const char *filename, int line);
void (GLAPIENTRY* qglGetFloatv)(GLenum pname, GLfloat *params, const char *filename, int line);
void (GLAPIENTRY* qglGetIntegerv)(GLenum pname, GLint *params, const char *filename, int line);
const GLubyte* (GLAPIENTRY* qglGetString)(GLenum name, const char *filename, int line);
void (GLAPIENTRY* qglDepthRange)(GLclampd zNear, GLclampd zFar, const char *filename, int line);
void (GLAPIENTRY* qglLoadIdentity)(const char *filename, int line);
void (GLAPIENTRY* qglLoadMatrixf)(const GLfloat *m, const char *filename, int line);
void (GLAPIENTRY* qglMatrixMode)(GLenum mode, const char *filename, int line);
void (GLAPIENTRY* qglViewport) (GLint x, GLint y, GLsizei width, GLsizei height, const char *filename, int line);
void (GLAPIENTRY* qglColorPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer, const char *filename, int line);
void (GLAPIENTRY* qglDisableClientState)(GLenum array, const char *filename, int line);
void (GLAPIENTRY* qglDrawElements)(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, const char *filename, int line);
void (GLAPIENTRY* qglEnableClientState)(GLenum array, const char *filename, int line);
void (GLAPIENTRY* qglIndexPointer)(GLenum type, GLsizei stride, const GLvoid *pointer, const char *filename, int line);
void (GLAPIENTRY* qglNormalPointer)(GLenum type, GLsizei stride, const GLvoid *pointer, const char *filename, int line);
void (GLAPIENTRY* qglTexCoordPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer, const char *filename, int line);
void (GLAPIENTRY* qglVertexPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer, const char *filename, int line);
void (GLAPIENTRY* qglPolygonOffset)(GLfloat factor, GLfloat units, const char *filename, int line);
void (GLAPIENTRY* qglCopyTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border, const char *filename, int line);
void (GLAPIENTRY* qglCopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height, const char *filename, int line);
void (GLAPIENTRY* qglBindTexture)(GLenum target, GLuint texture, const char *filename, int line);
void (GLAPIENTRY* qglDeleteTextures)(GLsizei n, const GLuint *textures, const char *filename, int line);
void (GLAPIENTRY* qglGenTextures)(GLsizei n, GLuint *textures, const char *filename, int line);

static void (GLAPIENTRY* dllBegin)(GLenum mode);
static void (GLAPIENTRY* dllColor4f)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
static void (GLAPIENTRY* dllColor4fv)(const GLfloat *v);
static void (GLAPIENTRY* dllEnd)();
static void (GLAPIENTRY* dllTexCoord1f)(GLfloat s);
static void (GLAPIENTRY* dllTexCoord1fv)(const GLfloat *v);
static void (GLAPIENTRY* dllTexCoord2f)(GLfloat s, GLfloat t);
static void (GLAPIENTRY* dllTexCoord2fv)(const GLfloat *v);
static void (GLAPIENTRY* dllTexCoord3f)(GLfloat s, GLfloat t, GLfloat r);
static void (GLAPIENTRY* dllTexCoord3fv)(const GLfloat *v);
static void (GLAPIENTRY* dllVertex3f)(GLfloat x, GLfloat y, GLfloat z);
static void (GLAPIENTRY* dllVertex3fv)(const GLfloat *v);
static void (GLAPIENTRY* dllClipPlane)(GLenum plane, const GLdouble *equation);
static void (GLAPIENTRY* dllCullFace)(GLenum mode);
static void (GLAPIENTRY* dllFrontFace)(GLenum mode);
static void (GLAPIENTRY* dllPolygonMode)(GLenum face, GLenum mode);
static void (GLAPIENTRY* dllScissor)(GLint x, GLint y, GLsizei width, GLsizei height);
static void (GLAPIENTRY* dllTexParameterfv)(GLenum target, GLenum pname, const GLfloat *params);
static void (GLAPIENTRY* dllTexParameteri)(GLenum target, GLenum pname, GLint param);
static void (GLAPIENTRY* dllTexParameteriv)(GLenum target, GLenum pname, const GLint *params);
static void (GLAPIENTRY* dllTexImage1D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
static void (GLAPIENTRY* dllTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
static void (GLAPIENTRY* dllTexEnvi)(GLenum target, GLenum pname, GLint param);
static void (GLAPIENTRY* dllTexGenf)(GLenum coord, GLenum pname, GLfloat param);
static void (GLAPIENTRY* dllTexGenfv)(GLenum coord, GLenum pname, const GLfloat *params);
static void (GLAPIENTRY* dllTexGeni)(GLenum coord, GLenum pname, GLint param);
static void (GLAPIENTRY* dllTexGeniv)(GLenum coord, GLenum pname, const GLint *params);
static void (GLAPIENTRY* dllDrawBuffer)(GLenum mode);
static void (GLAPIENTRY* dllClear)(GLbitfield mask);
static void (GLAPIENTRY* dllClearColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
static void (GLAPIENTRY* dllClearStencil)(GLint s);
static void (GLAPIENTRY* dllColorMask)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
static void (GLAPIENTRY* dllDepthMask)(GLboolean flag);
static void (GLAPIENTRY* dllDisable)(GLenum cap);
static void (GLAPIENTRY* dllEnable)(GLenum cap);
static void (GLAPIENTRY* dllFlush)();
static void (GLAPIENTRY* dllPopAttrib)();
static void (GLAPIENTRY* dllPushAttrib)(GLbitfield mask);
static void (GLAPIENTRY* dllAlphaFunc)(GLenum func, GLclampf ref);
static void (GLAPIENTRY* dllBlendFunc)(GLenum sfactor, GLenum dfactor);
static void (GLAPIENTRY* dllStencilFunc)(GLenum func, GLint ref, GLuint mask);
static void (GLAPIENTRY* dllStencilOp)(GLenum fail, GLenum zfail, GLenum zpass);
static void (GLAPIENTRY* dllDepthFunc)(GLenum func);
static void (GLAPIENTRY* dllReadBuffer)(GLenum mode);
static void (GLAPIENTRY* dllReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
static GLenum (GLAPIENTRY* dllGetError)();
static void (GLAPIENTRY* dllGetFloatv)(GLenum pname, GLfloat *params);
static void (GLAPIENTRY* dllGetIntegerv)(GLenum pname, GLint *params);
static const GLubyte* (GLAPIENTRY* dllGetString)(GLenum name);
static void (GLAPIENTRY* dllDepthRange)(GLclampd zNear, GLclampd zFar);
static void (GLAPIENTRY* dllLoadIdentity)(void);
static void (GLAPIENTRY* dllLoadMatrixf)(const GLfloat *m);
static void (GLAPIENTRY* dllMatrixMode)(GLenum mode);
static void (GLAPIENTRY* dllViewport)(GLint x, GLint y, GLsizei width, GLsizei height);
static void (GLAPIENTRY* dllColorPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
static void (GLAPIENTRY* dllDisableClientState)(GLenum array);
static void (GLAPIENTRY* dllDrawElements)(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
static void (GLAPIENTRY* dllEnableClientState)(GLenum array);
static void (GLAPIENTRY* dllIndexPointer)(GLenum type, GLsizei stride, const GLvoid *pointer);
static void (GLAPIENTRY* dllNormalPointer)(GLenum type, GLsizei stride, const GLvoid *pointer);
static void (GLAPIENTRY* dllTexCoordPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
static void (GLAPIENTRY* dllVertexPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
static void (GLAPIENTRY* dllPolygonOffset)(GLfloat factor, GLfloat units);
static void (GLAPIENTRY* dllCopyTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
static void (GLAPIENTRY* dllCopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
static void (GLAPIENTRY* dllBindTexture)(GLenum target, GLuint texture);
static void (GLAPIENTRY* dllDeleteTextures)(GLsizei n, const GLuint *textures);
static void (GLAPIENTRY* dllGenTextures)(GLsizei n, GLuint *textures);

static void GLAPIENTRY stdBegin(GLenum mode, const char *filename, int line)
{
	dllBegin(mode);
}
static void GLAPIENTRY stdColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha, const char *filename, int line)
{
	dllColor4f(red, green, blue, alpha);
}
static void GLAPIENTRY stdColor4fv(const GLfloat *v, const char *filename, int line)
{
	dllColor4fv(v);
}
static void GLAPIENTRY stdEnd(const char *filename, int line)
{
	dllEnd();
}
static void GLAPIENTRY stdTexCoord1f(GLfloat s, const char *filename, int line)
{
	dllTexCoord1f(s);
}
static void GLAPIENTRY stdTexCoord1fv(const GLfloat *v, const char *filename, int line)
{
	dllTexCoord1fv(v);
}
static void GLAPIENTRY stdTexCoord2f(GLfloat s, GLfloat t, const char *filename, int line)
{
	dllTexCoord2f(s, t);
}
static void GLAPIENTRY stdTexCoord2fv(const GLfloat *v, const char *filename, int line)
{
	dllTexCoord2fv(v);
}
static void GLAPIENTRY stdTexCoord3f(GLfloat s, GLfloat t, GLfloat r, const char *filename, int line)
{
	dllTexCoord3f(s, t, r);
}
static void GLAPIENTRY stdTexCoord3fv(const GLfloat *v, const char *filename, int line)
{
	dllTexCoord3fv(v);
}
static void GLAPIENTRY stdVertex3f(GLfloat x, GLfloat y, GLfloat z, const char *filename, int line)
{
	dllVertex3f(x, y, z);
}
static void GLAPIENTRY stdVertex3fv(const GLfloat *v, const char *filename, int line)
{
	dllVertex3fv(v);
}
static void GLAPIENTRY stdClipPlane(GLenum plane, const GLdouble *equation, const char *filename, int line)
{
	dllClipPlane(plane, equation);
}
static void GLAPIENTRY stdCullFace(GLenum mode, const char *filename, int line)
{
	dllCullFace(mode);
}
static void GLAPIENTRY stdFrontFace(GLenum mode, const char *filename, int line)
{
	dllFrontFace(mode);
}
static void GLAPIENTRY stdPolygonMode(GLenum face, GLenum mode, const char *filename, int line)
{
	dllPolygonMode(face, mode);
}
static void GLAPIENTRY stdScissor(GLint x, GLint y, GLsizei width, GLsizei height, const char *filename, int line)
{
	dllScissor(x, y, width, height);
}
static void GLAPIENTRY stdTexParameterfv(GLenum target, GLenum pname, const GLfloat *params, const char *filename, int line)
{
	dllTexParameterfv(target, pname, params);
}
static void GLAPIENTRY stdTexParameteri(GLenum target, GLenum pname, GLint param, const char *filename, int line)
{
	dllTexParameteri(target, pname, param);
}
static void GLAPIENTRY stdTexParameteriv(GLenum target, GLenum pname, const GLint *params, const char *filename, int line)
{
	dllTexParameteriv(target, pname, params);
}
static void GLAPIENTRY stdTexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels, const char *filename, int line)
{
	dllTexImage1D(target, level, internalformat, width, border, format, type, pixels);
}
static void GLAPIENTRY stdTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels, const char *filename, int line)
{
	dllTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
}
static void GLAPIENTRY stdTexEnvi(GLenum target, GLenum pname, GLint param, const char *filename, int line)
{
	dllTexEnvi(target, pname, param);
}
static void GLAPIENTRY stdTexGenf(GLenum coord, GLenum pname, GLfloat param, const char *filename, int line)
{
	dllTexGenf(coord, pname, param);
}
static void GLAPIENTRY stdTexGenfv(GLenum coord, GLenum pname, const GLfloat *params, const char *filename, int line)
{
	dllTexGenfv(coord, pname, params);
}
static void GLAPIENTRY stdTexGeni(GLenum coord, GLenum pname, GLint param, const char *filename, int line)
{
	dllTexGeni(coord, pname, param);
}
static void GLAPIENTRY stdTexGeniv(GLenum coord, GLenum pname, const GLint *params, const char *filename, int line)
{
	dllTexGeniv(coord, pname, params);
}
static void GLAPIENTRY stdDrawBuffer(GLenum mode, const char *filename, int line)
{
	dllDrawBuffer(mode);
}
static void GLAPIENTRY stdClear(GLbitfield mask, const char *filename, int line)
{
	dllClear(mask);
}
static void GLAPIENTRY stdClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha, const char *filename, int line)
{
	dllClearColor(red, green, blue, alpha);
}
static void GLAPIENTRY stdClearStencil(GLint s, const char *filename, int line)
{
	dllClearStencil(s);
}
static void GLAPIENTRY stdColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha, const char *filename, int line)
{
	dllColorMask(red, green, blue, alpha);
}
static void GLAPIENTRY stdDepthMask(GLboolean flag, const char *filename, int line)
{
	dllDepthMask(flag);
}
static void GLAPIENTRY stdDisable(GLenum cap, const char *filename, int line)
{
	dllDisable(cap);
}
static void GLAPIENTRY stdEnable(GLenum cap, const char *filename, int line)
{
	dllEnable(cap);
}
static void GLAPIENTRY stdFlush(const char *filename, int line)
{
	dllFlush();
}
static void GLAPIENTRY stdPopAttrib(const char *filename, int line)
{
	dllPopAttrib();
}
static void GLAPIENTRY stdPushAttrib(GLbitfield mask, const char *filename, int line)
{
	dllPushAttrib(mask);
}
static void GLAPIENTRY stdAlphaFunc(GLenum func, GLclampf ref, const char *filename, int line)
{
	dllAlphaFunc(func, ref);
}
static void GLAPIENTRY stdBlendFunc(GLenum sfactor, GLenum dfactor, const char *filename, int line)
{
	dllBlendFunc(sfactor, dfactor);
}
static void GLAPIENTRY stdStencilFunc(GLenum func, GLint ref, GLuint mask, const char *filename, int line)
{
	dllStencilFunc(func, ref, mask);
}
static void GLAPIENTRY stdStencilOp(GLenum fail, GLenum zfail, GLenum zpass, const char *filename, int line)
{
	dllStencilOp(fail, zfail, zpass);
}
static void GLAPIENTRY stdDepthFunc(GLenum func, const char *filename, int line)
{
	dllDepthFunc(func);
}
static void GLAPIENTRY stdReadBuffer(GLenum mode, const char *filename, int line)
{
	dllReadBuffer(mode);
}
static void GLAPIENTRY stdReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels, const char *filename, int line)
{
	dllReadPixels(x, y, width, height, format, type, pixels);
}
static void GLAPIENTRY stdGetFloatv(GLenum pname, GLfloat *params, const char *filename, int line)
{
	dllGetFloatv(pname, params);
}
static void GLAPIENTRY stdGetIntegerv(GLenum pname, GLint *params, const char *filename, int line)
{
	dllGetIntegerv(pname, params);
}
static const GLubyte* GLAPIENTRY stdGetString(GLenum name, const char *filename, int line)
{
	return dllGetString(name);
}
static void GLAPIENTRY stdDepthRange(GLclampd zNear, GLclampd zFar, const char *filename, int line)
{
	dllDepthRange(zNear, zFar);
}
static void GLAPIENTRY stdLoadIdentity(const char *filename, int line)
{
	dllLoadIdentity();
}
static void GLAPIENTRY stdLoadMatrixf(const GLfloat *m, const char *filename, int line)
{
	dllLoadMatrixf(m);
}
static void GLAPIENTRY stdMatrixMode(GLenum mode, const char *filename, int line)
{
	dllMatrixMode(mode);
}
static void GLAPIENTRY stdViewport(GLint x, GLint y, GLsizei width, GLsizei height, const char *filename, int line)
{
	dllViewport(x, y, width, height);
}
static void GLAPIENTRY stdColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer, const char *filename, int line)
{
	dllColorPointer(size, type, stride, pointer);
}
static void GLAPIENTRY stdDisableClientState(GLenum array, const char *filename, int line)
{
	dllDisableClientState(array);
}
static void GLAPIENTRY stdDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, const char *filename, int line)
{
	dllDrawElements(mode, count, type, indices);
}
static void GLAPIENTRY stdEnableClientState(GLenum array, const char *filename, int line)
{
	dllEnableClientState(array);
}
static void GLAPIENTRY stdIndexPointer(GLenum type, GLsizei stride, const GLvoid *pointer, const char *filename, int line)
{
	dllIndexPointer(type, stride, pointer);
}
static void GLAPIENTRY stdNormalPointer(GLenum type, GLsizei stride, const GLvoid *pointer, const char *filename, int line)
{
	dllNormalPointer(type, stride, pointer);
}
static void GLAPIENTRY stdTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer, const char *filename, int line)
{
	dllTexCoordPointer(size, type, stride, pointer);
}
static void GLAPIENTRY stdVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer, const char *filename, int line)
{
	dllVertexPointer(size, type, stride, pointer);
}
static void GLAPIENTRY stdPolygonOffset(GLfloat factor, GLfloat units, const char *filename, int line)
{
	dllPolygonOffset(factor, units);
}
static void GLAPIENTRY stdCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border, const char *filename, int line)
{
	dllCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
}
static void GLAPIENTRY stdCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height, const char *filename, int line)
{
	dllCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
}
static void GLAPIENTRY stdBindTexture(GLenum target, GLuint texture, const char *filename, int line)
{
	dllBindTexture(target, texture);
}
static void GLAPIENTRY stdDeleteTextures(GLsizei n, const GLuint *textures, const char *filename, int line)
{
	dllDeleteTextures(n, textures);
}
static void GLAPIENTRY stdGenTextures(GLsizei n, GLuint *textures, const char *filename, int line)
{
	dllGenTextures(n, textures);
}

static void GLAPIENTRY dbgBegin(GLenum mode, const char *filename, int line)
{
	dllBegin(mode);
//	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha, const char *filename, int line)
{
	dllColor4f(red, green, blue, alpha);
//	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgColor4fv(const GLfloat *v, const char *filename, int line)
{
	dllColor4fv(v);
//	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgEnd(const char *filename, int line)
{
	dllEnd();
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgTexCoord1f(GLfloat s, const char *filename, int line)
{
	dllTexCoord1f(s);
//	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgTexCoord1fv(const GLfloat *v, const char *filename, int line)
{
	dllTexCoord1fv(v);
//	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgTexCoord2f(GLfloat s, GLfloat t, const char *filename, int line)
{
	dllTexCoord2f(s, t);
//	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgTexCoord2fv(const GLfloat *v, const char *filename, int line)
{
	dllTexCoord2fv(v);
//	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgTexCoord3f(GLfloat s, GLfloat t, GLfloat r, const char *filename, int line)
{
	dllTexCoord3f(s, t, r);
//	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgTexCoord3fv(const GLfloat *v, const char *filename, int line)
{
	dllTexCoord3fv(v);
//	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgVertex3f(GLfloat x, GLfloat y, GLfloat z, const char *filename, int line)
{
	dllVertex3f(x, y, z);
//	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgVertex3fv(const GLfloat *v, const char *filename, int line)
{
	dllVertex3fv(v);
//	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgClipPlane(GLenum plane, const GLdouble *equation, const char *filename, int line)
{
	dllClipPlane(plane, equation);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgCullFace(GLenum mode, const char *filename, int line)
{
	dllCullFace(mode);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgFrontFace(GLenum mode, const char *filename, int line)
{
	dllFrontFace(mode);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgPolygonMode(GLenum face, GLenum mode, const char *filename, int line)
{
	dllPolygonMode(face, mode);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgScissor(GLint x, GLint y, GLsizei width, GLsizei height, const char *filename, int line)
{
	dllScissor(x, y, width, height);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgTexParameterfv(GLenum target, GLenum pname, const GLfloat *params, const char *filename, int line)
{
	dllTexParameterfv(target, pname, params);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgTexParameteri(GLenum target, GLenum pname, GLint param, const char *filename, int line)
{
	dllTexParameteri(target, pname, param);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgTexParameteriv(GLenum target, GLenum pname, const GLint *params, const char *filename, int line)
{
	dllTexParameteriv(target, pname, params);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgTexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels, const char *filename, int line)
{
	dllTexImage1D(target, level, internalformat, width, border, format, type, pixels);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels, const char *filename, int line)
{
	dllTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgTexEnvi(GLenum target, GLenum pname, GLint param, const char *filename, int line)
{
	dllTexEnvi(target, pname, param);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgTexGenf(GLenum coord, GLenum pname, GLfloat param, const char *filename, int line)
{
	dllTexGenf(coord, pname, param);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgTexGenfv(GLenum coord, GLenum pname, const GLfloat *params, const char *filename, int line)
{
	dllTexGenfv(coord, pname, params);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgTexGeni(GLenum coord, GLenum pname, GLint param, const char *filename, int line)
{
	dllTexGeni(coord, pname, param);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgTexGeniv(GLenum coord, GLenum pname, const GLint *params, const char *filename, int line)
{
	dllTexGeniv(coord, pname, params);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgDrawBuffer(GLenum mode, const char *filename, int line)
{
	dllDrawBuffer(mode);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgClear(GLbitfield mask, const char *filename, int line)
{
	dllClear(mask);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha, const char *filename, int line)
{
	dllClearColor(red, green, blue, alpha);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgClearStencil(GLint s, const char *filename, int line)
{
	dllClearStencil(s);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha, const char *filename, int line)
{
	dllColorMask(red, green, blue, alpha);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgDepthMask(GLboolean flag, const char *filename, int line)
{
	dllDepthMask(flag);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgDisable(GLenum cap, const char *filename, int line)
{
	dllDisable(cap);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgEnable(GLenum cap, const char *filename, int line)
{
	dllEnable(cap);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgFlush(const char *filename, int line)
{
	dllFlush();
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgPopAttrib(const char *filename, int line)
{
	dllPopAttrib();
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgPushAttrib(GLbitfield mask, const char *filename, int line)
{
	dllPushAttrib(mask);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgAlphaFunc(GLenum func, GLclampf ref, const char *filename, int line)
{
	dllAlphaFunc(func, ref);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgBlendFunc(GLenum sfactor, GLenum dfactor, const char *filename, int line)
{
	dllBlendFunc(sfactor, dfactor);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgStencilFunc(GLenum func, GLint ref, GLuint mask, const char *filename, int line)
{
	dllStencilFunc(func, ref, mask);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgStencilOp(GLenum fail, GLenum zfail, GLenum zpass, const char *filename, int line)
{
	dllStencilOp(fail, zfail, zpass);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgDepthFunc(GLenum func, const char *filename, int line)
{
	dllDepthFunc(func);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgReadBuffer(GLenum mode, const char *filename, int line)
{
	dllReadBuffer(mode);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels, const char *filename, int line)
{
	dllReadPixels(x, y, width, height, format, type, pixels);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgGetFloatv(GLenum pname, GLfloat *params, const char *filename, int line)
{
	dllGetFloatv(pname, params);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgGetIntegerv(GLenum pname, GLint *params, const char *filename, int line)
{
	dllGetIntegerv(pname, params);
	XGL_CheckForError_(filename, line);
}
static const GLubyte* GLAPIENTRY dbgGetString(GLenum name, const char *filename, int line)
{
	const GLubyte*s = dllGetString(name);
	XGL_CheckForError_(filename, line);
	return s;
}
static void GLAPIENTRY dbgDepthRange(GLclampd zNear, GLclampd zFar, const char *filename, int line)
{
	dllDepthRange(zNear, zFar);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgLoadIdentity(const char *filename, int line)
{
	dllLoadIdentity();
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgLoadMatrixf(const GLfloat *m, const char *filename, int line)
{
	dllLoadMatrixf(m);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgMatrixMode(GLenum mode, const char *filename, int line)
{
	dllMatrixMode(mode);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgViewport(GLint x, GLint y, GLsizei width, GLsizei height, const char *filename, int line)
{
	dllViewport(x, y, width, height);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer, const char *filename, int line)
{
	dllColorPointer(size, type, stride, pointer);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgDisableClientState(GLenum array, const char *filename, int line)
{
	dllDisableClientState(array);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, const char *filename, int line)
{
	dllDrawElements(mode, count, type, indices);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgEnableClientState(GLenum array, const char *filename, int line)
{
	dllEnableClientState(array);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgIndexPointer(GLenum type, GLsizei stride, const GLvoid *pointer, const char *filename, int line)
{
	dllIndexPointer(type, stride, pointer);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgNormalPointer(GLenum type, GLsizei stride, const GLvoid *pointer, const char *filename, int line)
{
	dllNormalPointer(type, stride, pointer);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer, const char *filename, int line)
{
	dllTexCoordPointer(size, type, stride, pointer);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer, const char *filename, int line)
{
	dllVertexPointer(size, type, stride, pointer);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgPolygonOffset(GLfloat factor, GLfloat units, const char *filename, int line)
{
	dllPolygonOffset(factor, units);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border, const char *filename, int line)
{
	dllCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height, const char *filename, int line)
{
	dllCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgBindTexture(GLenum target, GLuint texture, const char *filename, int line)
{
	dllBindTexture(target, texture);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgDeleteTextures(GLsizei n, const GLuint *textures, const char *filename, int line)
{
	dllDeleteTextures(n, textures);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgGenTextures(GLsizei n, GLuint *textures, const char *filename, int line)
{
	dllGenTextures(n, textures);
	XGL_CheckForError_(filename, line);
}

/*
static void GLAPIENTRY logViewport (GLint x, GLint y, GLsizei width, GLsizei height)
{
	SIG( "glViewport" );
	dllViewport( x, y, width, height );
}
*/



/// GL_ARB_multitexture ========================================================
void (GLAPIENTRY* qglActiveTextureARB)(GLenum texture, const char *filename, int line);
void (GLAPIENTRY* qglClientActiveTextureARB)(GLenum texture, const char *filename, int line);

static void (GLAPIENTRY* dllActiveTextureARB)(GLenum texture);
static void (GLAPIENTRY* dllClientActiveTextureARB)(GLenum texture);

void GLAPIENTRY stdActiveTextureARB(GLenum texture, const char *filename, int line)
{
	dllActiveTextureARB(texture);
}
void GLAPIENTRY stdClientActiveTextureARB(GLenum texture, const char *filename, int line)
{
	dllClientActiveTextureARB(texture);
}

void GLAPIENTRY dbgActiveTextureARB(GLenum texture, const char *filename, int line)
{
	dllActiveTextureARB(texture);
	XGL_CheckForError_(filename, line);
}
void GLAPIENTRY dbgClientActiveTextureARB(GLenum texture, const char *filename, int line)
{
	dllClientActiveTextureARB(texture);
	XGL_CheckForError_(filename, line);
}


/// GL_ARB_transpose_matrix ====================================================
void (GLAPIENTRY* qglLoadTransposeMatrixfARB)(const GLfloat *m, const char *filename, int line);

static void (GLAPIENTRY* dllLoadTransposeMatrixfARB)(const GLfloat *m);

static void GLAPIENTRY stdLoadTransposeMatrixfARB(const GLfloat *m, const char *filename, int line)
{
	dllLoadTransposeMatrixfARB(m);
}

static void GLAPIENTRY dbgLoadTransposeMatrixfARB(const GLfloat *m, const char *filename, int line)
{
	dllLoadTransposeMatrixfARB(m);
	XGL_CheckForError_(filename, line);
}


/// GL_ARB_vertex_program ======================================================
void (GLAPIENTRY* qglVertexAttribPointerARB)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer, const char *filename, int line);
void (GLAPIENTRY* qglEnableVertexAttribArrayARB)(GLuint index, const char *filename, int line);
void (GLAPIENTRY* qglDisableVertexAttribArrayARB)(GLuint index, const char *filename, int line);

static void (GLAPIENTRY* dllVertexAttribPointerARB)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
static void (GLAPIENTRY* dllEnableVertexAttribArrayARB)(GLuint index);
static void (GLAPIENTRY* dllDisableVertexAttribArrayARB)(GLuint index);

static void GLAPIENTRY stdVertexAttribPointerARB(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer, const char *filename, int line)
{
	dllVertexAttribPointerARB(index, size, type, normalized, stride, pointer);
}
static void GLAPIENTRY stdEnableVertexAttribArrayARB(GLuint index, const char *filename, int line)
{
	dllEnableVertexAttribArrayARB(index);
}
static void GLAPIENTRY stdDisableVertexAttribArrayARB(GLuint index, const char *filename, int line)
{
	dllDisableVertexAttribArrayARB(index);
}

static void GLAPIENTRY dbgVertexAttribPointerARB(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer, const char *filename, int line)
{
	dllVertexAttribPointerARB(index, size, type, normalized, stride, pointer);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgEnableVertexAttribArrayARB(GLuint index, const char *filename, int line)
{
	dllEnableVertexAttribArrayARB(index);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgDisableVertexAttribArrayARB(GLuint index, const char *filename, int line)
{
	dllDisableVertexAttribArrayARB(index);
	XGL_CheckForError_(filename, line);
}


/*
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
*/

/// GL_ARB_vertex_buffer_object ================================================
void (GLAPIENTRY* qglBindBufferARB)(GLenum target, GLuint buffer, const char *filename, int line);
void (GLAPIENTRY* qglDeleteBuffersARB)(GLsizei n, const GLuint *buffers, const char *filename, int line);
void (GLAPIENTRY* qglGenBuffersARB)(GLsizei n, GLuint *buffers, const char *filename, int line);
GLboolean (GLAPIENTRY* qglIsBufferARB)(GLuint buffer, const char *filename, int line);
void (GLAPIENTRY* qglBufferDataARB)(GLenum target, GLsizeiptrARB size, const GLvoid *data, GLenum usage, const char *filename, int line);
void (GLAPIENTRY* qglBufferSubDataARB)(GLenum target, GLintptrARB offset, GLsizeiptrARB size, const GLvoid *data, const char *filename, int line);
void (GLAPIENTRY* qglGetBufferSubDataARB)(GLenum target, GLintptrARB offset, GLsizeiptrARB size, GLvoid *data, const char *filename, int line);
GLvoid* (GLAPIENTRY* qglMapBufferARB)(GLenum target, GLenum access, const char *filename, int line);
GLboolean (GLAPIENTRY* qglUnmapBufferARB)(GLenum target, const char *filename, int line);
void (GLAPIENTRY* qglGetBufferParameterivARB)(GLenum target, GLenum pname, GLint *params, const char *filename, int line);
void (GLAPIENTRY* qglGetBufferPointervARB)(GLenum target, GLenum pname, GLvoid* *params, const char *filename, int line);

static void (GLAPIENTRY* dllBindBufferARB)(GLenum target, GLuint buffer);
static void (GLAPIENTRY* dllDeleteBuffersARB)(GLsizei n, const GLuint *buffers);
static void (GLAPIENTRY* dllGenBuffersARB)(GLsizei n, GLuint *buffers);
static GLboolean (GLAPIENTRY* dllIsBufferARB)(GLuint buffer);
static void (GLAPIENTRY* dllBufferDataARB)(GLenum target, GLsizeiptrARB size, const GLvoid *data, GLenum usage);
static void (GLAPIENTRY* dllBufferSubDataARB)(GLenum target, GLintptrARB offset, GLsizeiptrARB size, const GLvoid *data);
static void (GLAPIENTRY* dllGetBufferSubDataARB)(GLenum target, GLintptrARB offset, GLsizeiptrARB size, GLvoid *data);
static GLvoid* (GLAPIENTRY* dllMapBufferARB)(GLenum target, GLenum access);
static GLboolean (GLAPIENTRY* dllUnmapBufferARB)(GLenum target);
static void (GLAPIENTRY* dllGetBufferParameterivARB)(GLenum target, GLenum pname, GLint *params);
static void (GLAPIENTRY* dllGetBufferPointervARB)(GLenum target, GLenum pname, GLvoid* *params);

static void GLAPIENTRY stdBindBufferARB(GLenum target, GLuint buffer, const char *filename, int line)
{
	dllBindBufferARB(target, buffer);
}
static void GLAPIENTRY stdDeleteBuffersARB(GLsizei n, const GLuint *buffers, const char *filename, int line)
{
	dllDeleteBuffersARB(n, buffers);
}
static void GLAPIENTRY stdGenBuffersARB(GLsizei n, GLuint *buffers, const char *filename, int line)
{
	dllGenBuffersARB(n, buffers);
}
static GLboolean GLAPIENTRY stdIsBufferARB(GLuint buffer, const char *filename, int line)
{
	return dllIsBufferARB(buffer);
}
static void GLAPIENTRY stdBufferDataARB(GLenum target, GLsizeiptrARB size, const GLvoid *data, GLenum usage, const char *filename, int line)
{
	dllBufferDataARB(target, size, data, usage);
}
static void GLAPIENTRY stdBufferSubDataARB(GLenum target, GLintptrARB offset, GLsizeiptrARB size, const GLvoid *data, const char *filename, int line)
{
	dllBufferSubDataARB(target, offset, size, data);
}
static void GLAPIENTRY stdGetBufferSubDataARB(GLenum target, GLintptrARB offset, GLsizeiptrARB size, GLvoid *data, const char *filename, int line)
{
	dllGetBufferSubDataARB(target, offset, size, data);
}
static GLvoid* GLAPIENTRY stdMapBufferARB(GLenum target, GLenum access, const char *filename, int line)
{
	return dllMapBufferARB(target, access);
}
static GLboolean GLAPIENTRY stdUnmapBufferARB(GLenum target, const char *filename, int line)
{
	return dllUnmapBufferARB(target);
}
static void GLAPIENTRY stdGetBufferParameterivARB(GLenum target, GLenum pname, GLint *params, const char *filename, int line)
{
	dllGetBufferParameterivARB(target, pname, params);
}
static void GLAPIENTRY stdGetBufferPointervARB(GLenum target, GLenum pname, GLvoid* *params, const char *filename, int line)
{
	dllGetBufferPointervARB(target, pname, params);
}

static void GLAPIENTRY dbgBindBufferARB(GLenum target, GLuint buffer, const char *filename, int line)
{
	dllBindBufferARB(target, buffer);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgDeleteBuffersARB(GLsizei n, const GLuint *buffers, const char *filename, int line)
{
	dllDeleteBuffersARB(n, buffers);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgGenBuffersARB(GLsizei n, GLuint *buffers, const char *filename, int line)
{
	dllGenBuffersARB(n, buffers);
	XGL_CheckForError_(filename, line);
}
static GLboolean GLAPIENTRY dbgIsBufferARB(GLuint buffer, const char *filename, int line)
{
	GLboolean b = dllIsBufferARB(buffer);
	XGL_CheckForError_(filename, line);
	return b;
}
static void GLAPIENTRY dbgBufferDataARB(GLenum target, GLsizeiptrARB size, const GLvoid *data, GLenum usage, const char *filename, int line)
{
	dllBufferDataARB(target, size, data, usage);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgBufferSubDataARB(GLenum target, GLintptrARB offset, GLsizeiptrARB size, const GLvoid *data, const char *filename, int line)
{
	dllBufferSubDataARB(target, offset, size, data);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgGetBufferSubDataARB(GLenum target, GLintptrARB offset, GLsizeiptrARB size, GLvoid *data, const char *filename, int line)
{
	dllGetBufferSubDataARB(target, offset, size, data);
	XGL_CheckForError_(filename, line);
}
static GLvoid* GLAPIENTRY dbgMapBufferARB(GLenum target, GLenum access, const char *filename, int line)
{
	GLvoid* p = dllMapBufferARB(target, access);
	XGL_CheckForError_(filename, line);
	return p;
}
static GLboolean GLAPIENTRY dbgUnmapBufferARB(GLenum target, const char *filename, int line)
{
	GLboolean b = dllUnmapBufferARB(target);
	XGL_CheckForError_(filename, line);
	return b;
}
static void GLAPIENTRY dbgGetBufferParameterivARB(GLenum target, GLenum pname, GLint *params, const char *filename, int line)
{
	dllGetBufferParameterivARB(target, pname, params);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgGetBufferPointervARB(GLenum target, GLenum pname, GLvoid* *params, const char *filename, int line)
{
	dllGetBufferPointervARB(target, pname, params);
	XGL_CheckForError_(filename, line);
}

/*
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
*/


/// GL_ARB_occlusion_query =====================================================
void (GLAPIENTRY* xglGenQueriesARB) (GLsizei, GLuint *);
void (GLAPIENTRY* xglDeleteQueriesARB) (GLsizei, const GLuint *);
GLboolean (GLAPIENTRY* xglIsQueryARB) (GLuint);
void (GLAPIENTRY* xglBeginQueryARB) (GLenum, GLuint);
void (GLAPIENTRY* xglEndQueryARB) (GLenum);
void (GLAPIENTRY* xglGetQueryivARB) (GLenum, GLenum, GLint *);
void (GLAPIENTRY* xglGetQueryObjectivARB) (GLuint, GLenum, GLint *);
void (GLAPIENTRY* xglGetQueryObjectuivARB) (GLuint, GLenum, GLuint *);


/// GL_ARB_shader_objects ======================================================
typedef char GLcharARB;			/* native character */
typedef unsigned int GLhandleARB;	/* shader object handle */

void (GLAPIENTRY* qglDeleteObjectARB)(GLhandleARB obj, const char *filename, int line);
GLhandleARB (GLAPIENTRY* qglGetHandleARB)(GLenum pname, const char *filename, int line);
void (GLAPIENTRY* qglDetachObjectARB)(GLhandleARB containerObj, GLhandleARB attachedObj, const char *filename, int line);
GLhandleARB (GLAPIENTRY* qglCreateShaderObjectARB)(GLenum shaderType, const char *filename, int line);
void (GLAPIENTRY* qglShaderSourceARB)(GLhandleARB shaderObj, GLsizei count, const GLcharARB* *string, const GLint *length, const char *filename, int line);
void (GLAPIENTRY* qglCompileShaderARB)(GLhandleARB shaderObj, const char *filename, int line);
GLhandleARB (GLAPIENTRY* qglCreateProgramObjectARB)(const char *filename, int line);
void (GLAPIENTRY* qglAttachObjectARB)(GLhandleARB containerObj, GLhandleARB obj, const char *filename, int line);
void (GLAPIENTRY* qglLinkProgramARB)(GLhandleARB programObj, const char *filename, int line);
void (GLAPIENTRY* qglUseProgramObjectARB)(GLhandleARB programObj, const char *filename, int line);
void (GLAPIENTRY* qglValidateProgramARB)(GLhandleARB programObj, const char *filename, int line);
void (GLAPIENTRY* qglUniform1fARB)(GLint location, GLfloat v0, const char *filename, int line);
void (GLAPIENTRY* qglUniform2fARB)(GLint location, GLfloat v0, GLfloat v1, const char *filename, int line);
void (GLAPIENTRY* qglUniform3fARB)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, const char *filename, int line);
void (GLAPIENTRY* qglUniform4fARB)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3, const char *filename, int line);
void (GLAPIENTRY* qglUniform1iARB)(GLint location, GLint v0, const char *filename, int line);
void (GLAPIENTRY* qglUniform2iARB)(GLint location, GLint v0, GLint v1, const char *filename, int line);
void (GLAPIENTRY* qglUniform3iARB)(GLint location, GLint v0, GLint v1, GLint v2, const char *filename, int line);
void (GLAPIENTRY* qglUniform4iARB)(GLint location, GLint v0, GLint v1, GLint v2, GLint v3, const char *filename, int line);
void (GLAPIENTRY* qglUniform2fvARB)(GLint location, GLsizei count, const GLfloat *value, const char *filename, int line);
void (GLAPIENTRY* qglUniform3fvARB)(GLint location, GLsizei count, const GLfloat *value, const char *filename, int line);
void (GLAPIENTRY* qglUniform4fvARB)(GLint location, GLsizei count, const GLfloat *value, const char *filename, int line);
void (GLAPIENTRY* qglUniform2ivARB)(GLint location, GLsizei count, const GLint *value, const char *filename, int line);
void (GLAPIENTRY* qglUniform3ivARB)(GLint location, GLsizei count, const GLint *value, const char *filename, int line);
void (GLAPIENTRY* qglUniform4ivARB)(GLint location, GLsizei count, const GLint *value, const char *filename, int line);
void (GLAPIENTRY* qglUniformMatrix2fvARB)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value, const char *filename, int line);
void (GLAPIENTRY* qglUniformMatrix3fvARB)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value, const char *filename, int line);
void (GLAPIENTRY* qglUniformMatrix4fvARB)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value, const char *filename, int line);
void (GLAPIENTRY* qglGetObjectParameterfvARB)(GLhandleARB obj, GLenum pname, GLfloat *params, const char *filename, int line);
void (GLAPIENTRY* qglGetObjectParameterivARB)(GLhandleARB obj, GLenum pname, GLint *params, const char *filename, int line);
void (GLAPIENTRY* qglGetInfoLogARB)(GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *infoLog, const char *filename, int line);
void (GLAPIENTRY* qglGetAttachedObjectsARB)(GLhandleARB containerObj, GLsizei maxCount, GLsizei *count, GLhandleARB *obj, const char *filename, int line);
GLint (GLAPIENTRY* qglGetUniformLocationARB)(GLhandleARB programObj, const GLcharARB *name, const char *filename, int line);
void (GLAPIENTRY* qglGetActiveUniformARB)(GLhandleARB programObj, GLuint index, GLsizei maxIndex, GLsizei *length, GLint *size, GLenum *type, GLcharARB *name, const char *filename, int line);
void (GLAPIENTRY* qglGetUniformfvARB)(GLhandleARB programObj, GLint location, GLfloat *params, const char *filename, int line);
void (GLAPIENTRY* qglGetUniformivARB)(GLhandleARB programObj, GLint location, GLint *params, const char *filename, int line);
void (GLAPIENTRY* qglGetShaderSourceARB)(GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *source, const char *filename, int line);

static void (GLAPIENTRY* dllDeleteObjectARB)(GLhandleARB obj);
GLhandleARB (GLAPIENTRY* dllGetHandleARB)(GLenum pname);
static void (GLAPIENTRY* dllDetachObjectARB)(GLhandleARB containerObj, GLhandleARB attachedObj);
GLhandleARB (GLAPIENTRY* dllCreateShaderObjectARB)(GLenum shaderType);
static void (GLAPIENTRY* dllShaderSourceARB)(GLhandleARB shaderObj, GLsizei count, const GLcharARB* *string, const GLint *length);
static void (GLAPIENTRY* dllCompileShaderARB)(GLhandleARB shaderObj);
GLhandleARB (GLAPIENTRY* dllCreateProgramObjectARB)();
static void (GLAPIENTRY* dllAttachObjectARB)(GLhandleARB containerObj, GLhandleARB obj);
static void (GLAPIENTRY* dllLinkProgramARB)(GLhandleARB programObj);
static void (GLAPIENTRY* dllUseProgramObjectARB)(GLhandleARB programObj);
static void (GLAPIENTRY* dllValidateProgramARB)(GLhandleARB programObj);
static void (GLAPIENTRY* dllUniform1fARB)(GLint, GLfloat);
static void (GLAPIENTRY* dllUniform2fARB)(GLint, GLfloat, GLfloat);
static void (GLAPIENTRY* dllUniform3fARB)(GLint, GLfloat, GLfloat, GLfloat);
static void (GLAPIENTRY* dllUniform4fARB)(GLint, GLfloat, GLfloat, GLfloat, GLfloat);
static void (GLAPIENTRY* dllUniform1iARB)(GLint, GLint);
static void (GLAPIENTRY* dllUniform2iARB)(GLint, GLint, GLint);
static void (GLAPIENTRY* dllUniform3iARB)(GLint, GLint, GLint, GLint);
static void (GLAPIENTRY* dllUniform4iARB)(GLint, GLint, GLint, GLint, GLint);
static void (GLAPIENTRY* dllUniform2fvARB)(GLint, GLsizei, const GLfloat *);
static void (GLAPIENTRY* dllUniform3fvARB)(GLint, GLsizei, const GLfloat *);
static void (GLAPIENTRY* dllUniform4fvARB)(GLint, GLsizei, const GLfloat *);
static void (GLAPIENTRY* dllUniform2ivARB)(GLint, GLsizei, const GLint *);
static void (GLAPIENTRY* dllUniform3ivARB)(GLint, GLsizei, const GLint *);
static void (GLAPIENTRY* dllUniform4ivARB)(GLint, GLsizei, const GLint *);
static void (GLAPIENTRY* dllUniformMatrix2fvARB)(GLint, GLsizei, GLboolean, const GLfloat *);
static void (GLAPIENTRY* dllUniformMatrix3fvARB)(GLint, GLsizei, GLboolean, const GLfloat *);
static void (GLAPIENTRY* dllUniformMatrix4fvARB)(GLint, GLsizei, GLboolean, const GLfloat *);
static void (GLAPIENTRY* dllGetObjectParameterfvARB)(GLhandleARB, GLenum, GLfloat *);
static void (GLAPIENTRY* dllGetObjectParameterivARB)(GLhandleARB, GLenum, GLint *);
static void (GLAPIENTRY* dllGetInfoLogARB)(GLhandleARB, GLsizei, GLsizei *, GLcharARB *);
static void (GLAPIENTRY* dllGetAttachedObjectsARB)(GLhandleARB, GLsizei, GLsizei *, GLhandleARB *);
static GLint (GLAPIENTRY* dllGetUniformLocationARB)(GLhandleARB, const GLcharARB *);
static void (GLAPIENTRY* dllGetActiveUniformARB)(GLhandleARB, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLcharARB *);
static void (GLAPIENTRY* dllGetUniformfvARB)(GLhandleARB, GLint, GLfloat *);
static void (GLAPIENTRY* dllGetUniformivARB)(GLhandleARB, GLint, GLint *);
static void (GLAPIENTRY* dllGetShaderSourceARB)(GLhandleARB, GLsizei, GLsizei *, GLcharARB *);

static void GLAPIENTRY stdDeleteObjectARB(GLhandleARB obj, const char *filename, int line)
{
	dllDeleteObjectARB(obj);
}
GLhandleARB GLAPIENTRY stdGetHandleARB(GLenum pname, const char *filename, int line)
{
	return dllGetHandleARB(pname);
}
static void GLAPIENTRY stdDetachObjectARB(GLhandleARB containerObj, GLhandleARB attachedObj, const char *filename, int line)
{
	dllDetachObjectARB(containerObj, attachedObj);
}
GLhandleARB GLAPIENTRY stdCreateShaderObjectARB(GLenum shaderType, const char *filename, int line)
{
	return dllCreateShaderObjectARB(shaderType);
}
static void GLAPIENTRY stdShaderSourceARB(GLhandleARB shaderObj, GLsizei count, const GLcharARB* *string, const GLint *length, const char *filename, int line)
{
	dllShaderSourceARB(shaderObj, count, string, length);
}
static void GLAPIENTRY stdCompileShaderARB(GLhandleARB shaderObj, const char *filename, int line)
{
	dllCompileShaderARB(shaderObj);
}
GLhandleARB GLAPIENTRY stdCreateProgramObjectARB(const char *filename, int line)
{
	return dllCreateProgramObjectARB();
}
static void GLAPIENTRY stdAttachObjectARB(GLhandleARB containerObj, GLhandleARB obj, const char *filename, int line)
{
	dllAttachObjectARB(containerObj, obj);
}
static void GLAPIENTRY stdLinkProgramARB(GLhandleARB programObj, const char *filename, int line)
{
	dllLinkProgramARB(programObj);
}
static void GLAPIENTRY stdUseProgramObjectARB(GLhandleARB programObj, const char *filename, int line)
{
	dllUseProgramObjectARB(programObj);
}
static void GLAPIENTRY stdValidateProgramARB(GLhandleARB programObj, const char *filename, int line)
{
	dllValidateProgramARB(programObj);
}
static void GLAPIENTRY stdUniform1fARB(GLint location, GLfloat v0, const char *filename, int line)
{
	dllUniform1fARB(location, v0);
}
static void GLAPIENTRY stdUniform2fARB(GLint location, GLfloat v0, GLfloat v1, const char *filename, int line)
{
	dllUniform2fARB(location, v0, v1);
}
static void GLAPIENTRY stdUniform3fARB(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, const char *filename, int line)
{
	dllUniform3fARB(location, v0, v1, v2);
}
static void GLAPIENTRY stdUniform4fARB(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3, const char *filename, int line)
{
	dllUniform4fARB(location, v0, v1, v2, v3);
}
static void GLAPIENTRY stdUniform1iARB(GLint location, GLint v0, const char *filename, int line)
{
	dllUniform1iARB(location, v0);
}
static void GLAPIENTRY stdUniform2iARB(GLint location, GLint v0, GLint v1, const char *filename, int line)
{
	dllUniform2iARB(location, v0, v1);
}
static void GLAPIENTRY stdUniform3iARB(GLint location, GLint v0, GLint v1, GLint v2, const char *filename, int line)
{
	dllUniform3iARB(location, v0, v1, v2);
}
static void GLAPIENTRY stdUniform4iARB(GLint location, GLint v0, GLint v1, GLint v2, GLint v3, const char *filename, int line)
{
	dllUniform4iARB(location, v0, v1, v2, v3);
}
static void GLAPIENTRY stdUniform2fvARB(GLint location, GLsizei count, const GLfloat *value, const char *filename, int line)
{
	dllUniform2fvARB(location, count, value);
}
static void GLAPIENTRY stdUniform3fvARB(GLint location, GLsizei count, const GLfloat *value, const char *filename, int line)
{
	dllUniform3fvARB(location, count, value);
}
static void GLAPIENTRY stdUniform4fvARB(GLint location, GLsizei count, const GLfloat *value, const char *filename, int line)
{
	dllUniform4fvARB(location, count, value);
}
static void GLAPIENTRY stdUniform2ivARB(GLint location, GLsizei count, const GLint *value, const char *filename, int line)
{
	dllUniform2ivARB(location, count, value);
}
static void GLAPIENTRY stdUniform3ivARB(GLint location, GLsizei count, const GLint *value, const char *filename, int line)
{
	dllUniform3ivARB(location, count, value);
}
static void GLAPIENTRY stdUniform4ivARB(GLint location, GLsizei count, const GLint *value, const char *filename, int line)
{
	dllUniform4ivARB(location, count, value);
}
static void GLAPIENTRY stdUniformMatrix2fvARB(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value, const char *filename, int line)
{
	dllUniformMatrix2fvARB(location, count, transpose, value);
}
static void GLAPIENTRY stdUniformMatrix3fvARB(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value, const char *filename, int line)
{
	dllUniformMatrix3fvARB(location, count, transpose, value);
}
static void GLAPIENTRY stdUniformMatrix4fvARB(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value, const char *filename, int line)
{
	dllUniformMatrix4fvARB(location, count, transpose, value);
}
static void GLAPIENTRY stdGetObjectParameterfvARB(GLhandleARB obj, GLenum pname, GLfloat *params, const char *filename, int line)
{
	dllGetObjectParameterfvARB(obj, pname, params);
}
static void GLAPIENTRY stdGetObjectParameterivARB(GLhandleARB obj, GLenum pname, GLint *params, const char *filename, int line)
{
	dllGetObjectParameterivARB(obj, pname, params);
}
static void GLAPIENTRY stdGetInfoLogARB(GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *infoLog, const char *filename, int line)
{
	dllGetInfoLogARB(obj, maxLength, length, infoLog);
}
static void GLAPIENTRY stdGetAttachedObjectsARB(GLhandleARB containerObj, GLsizei maxCount, GLsizei *count, GLhandleARB *obj, const char *filename, int line)
{
	dllGetAttachedObjectsARB(containerObj, maxCount, count, obj);
}
static GLint GLAPIENTRY stdGetUniformLocationARB(GLhandleARB programObj, const GLcharARB *name, const char *filename, int line)
{
	return dllGetUniformLocationARB(programObj, name);
}
static void GLAPIENTRY stdGetActiveUniformARB(GLhandleARB programObj, GLuint index, GLsizei maxIndex, GLsizei *length, GLint *size, GLenum *type, GLcharARB *name, const char *filename, int line)
{
	dllGetActiveUniformARB(programObj, index, maxIndex, length, size, type, name);
}
static void GLAPIENTRY stdGetUniformfvARB(GLhandleARB programObj, GLint location, GLfloat *params, const char *filename, int line)
{
	dllGetUniformfvARB(programObj, location, params);
}
static void GLAPIENTRY stdGetUniformivARB(GLhandleARB programObj, GLint location, GLint *params, const char *filename, int line)
{
	dllGetUniformivARB(programObj, location, params);
}
static void GLAPIENTRY stdGetShaderSourceARB(GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *source, const char *filename, int line)
{
	dllGetShaderSourceARB(obj, maxLength, length, source);
}

static void GLAPIENTRY dbgDeleteObjectARB(GLhandleARB obj, const char *filename, int line)
{
	dllDeleteObjectARB(obj);
	XGL_CheckForError_(filename, line);
}
GLhandleARB GLAPIENTRY dbgGetHandleARB(GLenum pname, const char *filename, int line)
{
	GLhandleARB h = dllGetHandleARB(pname);
	XGL_CheckForError_(filename, line);
	return h;
}
static void GLAPIENTRY dbgDetachObjectARB(GLhandleARB containerObj, GLhandleARB attachedObj, const char *filename, int line)
{
	dllDetachObjectARB(containerObj, attachedObj);
	XGL_CheckForError_(filename, line);
}
GLhandleARB GLAPIENTRY dbgCreateShaderObjectARB(GLenum shaderType, const char *filename, int line)
{
	GLhandleARB h = dllCreateShaderObjectARB(shaderType);
	XGL_CheckForError_(filename, line);
	return h;
}
static void GLAPIENTRY dbgShaderSourceARB(GLhandleARB shaderObj, GLsizei count, const GLcharARB* *string, const GLint *length, const char *filename, int line)
{
	dllShaderSourceARB(shaderObj, count, string, length);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgCompileShaderARB(GLhandleARB shaderObj, const char *filename, int line)
{
	dllCompileShaderARB(shaderObj);
	XGL_CheckForError_(filename, line);
}
GLhandleARB GLAPIENTRY dbgCreateProgramObjectARB(const char *filename, int line)
{
	GLhandleARB h = dllCreateProgramObjectARB();
	XGL_CheckForError_(filename, line);
	return h;
}
static void GLAPIENTRY dbgAttachObjectARB(GLhandleARB containerObj, GLhandleARB obj, const char *filename, int line)
{
	dllAttachObjectARB(containerObj, obj);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgLinkProgramARB(GLhandleARB programObj, const char *filename, int line)
{
	dllLinkProgramARB(programObj);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgUseProgramObjectARB(GLhandleARB programObj, const char *filename, int line)
{
	dllUseProgramObjectARB(programObj);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgValidateProgramARB(GLhandleARB programObj, const char *filename, int line)
{
	dllValidateProgramARB(programObj);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgUniform1fARB(GLint location, GLfloat v0, const char *filename, int line)
{
	dllUniform1fARB(location, v0);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgUniform2fARB(GLint location, GLfloat v0, GLfloat v1, const char *filename, int line)
{
	dllUniform2fARB(location, v0, v1);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgUniform3fARB(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, const char *filename, int line)
{
	dllUniform3fARB(location, v0, v1, v2);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgUniform4fARB(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3, const char *filename, int line)
{
	dllUniform4fARB(location, v0, v1, v2, v3);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgUniform1iARB(GLint location, GLint v0, const char *filename, int line)
{
	dllUniform1iARB(location, v0);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgUniform2iARB(GLint location, GLint v0, GLint v1, const char *filename, int line)
{
	dllUniform2iARB(location, v0, v1);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgUniform3iARB(GLint location, GLint v0, GLint v1, GLint v2, const char *filename, int line)
{
	dllUniform3iARB(location, v0, v1, v2);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgUniform4iARB(GLint location, GLint v0, GLint v1, GLint v2, GLint v3, const char *filename, int line)
{
	dllUniform4iARB(location, v0, v1, v2, v3);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgUniform2fvARB(GLint location, GLsizei count, const GLfloat *value, const char *filename, int line)
{
	dllUniform2fvARB(location, count, value);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgUniform3fvARB(GLint location, GLsizei count, const GLfloat *value, const char *filename, int line)
{
	dllUniform3fvARB(location, count, value);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgUniform4fvARB(GLint location, GLsizei count, const GLfloat *value, const char *filename, int line)
{
	dllUniform4fvARB(location, count, value);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgUniform2ivARB(GLint location, GLsizei count, const GLint *value, const char *filename, int line)
{
	dllUniform2ivARB(location, count, value);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgUniform3ivARB(GLint location, GLsizei count, const GLint *value, const char *filename, int line)
{
	dllUniform3ivARB(location, count, value);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgUniform4ivARB(GLint location, GLsizei count, const GLint *value, const char *filename, int line)
{
	dllUniform4ivARB(location, count, value);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgUniformMatrix2fvARB(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value, const char *filename, int line)
{
	dllUniformMatrix2fvARB(location, count, transpose, value);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgUniformMatrix3fvARB(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value, const char *filename, int line)
{
	dllUniformMatrix3fvARB(location, count, transpose, value);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgUniformMatrix4fvARB(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value, const char *filename, int line)
{
	dllUniformMatrix4fvARB(location, count, transpose, value);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgGetObjectParameterfvARB(GLhandleARB obj, GLenum pname, GLfloat *params, const char *filename, int line)
{
	dllGetObjectParameterfvARB(obj, pname, params);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgGetObjectParameterivARB(GLhandleARB obj, GLenum pname, GLint *params, const char *filename, int line)
{
	dllGetObjectParameterivARB(obj, pname, params);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgGetInfoLogARB(GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *infoLog, const char *filename, int line)
{
	dllGetInfoLogARB(obj, maxLength, length, infoLog);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgGetAttachedObjectsARB(GLhandleARB containerObj, GLsizei maxCount, GLsizei *count, GLhandleARB *obj, const char *filename, int line)
{
	dllGetAttachedObjectsARB(containerObj, maxCount, count, obj);
	XGL_CheckForError_(filename, line);
}
static GLint GLAPIENTRY dbgGetUniformLocationARB(GLhandleARB programObj, const GLcharARB *name, const char *filename, int line)
{
	GLint i = dllGetUniformLocationARB(programObj, name);
	XGL_CheckForError_(filename, line);
	return i;
}
static void GLAPIENTRY dbgGetActiveUniformARB(GLhandleARB programObj, GLuint index, GLsizei maxIndex, GLsizei *length, GLint *size, GLenum *type, GLcharARB *name, const char *filename, int line)
{
	dllGetActiveUniformARB(programObj, index, maxIndex, length, size, type, name);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgGetUniformfvARB(GLhandleARB programObj, GLint location, GLfloat *params, const char *filename, int line)
{
	dllGetUniformfvARB(programObj, location, params);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgGetUniformivARB(GLhandleARB programObj, GLint location, GLint *params, const char *filename, int line)
{
	dllGetUniformivARB(programObj, location, params);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgGetShaderSourceARB(GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *source, const char *filename, int line)
{
	dllGetShaderSourceARB(obj, maxLength, length, source);
	XGL_CheckForError_(filename, line);
}


/// GL_ARB_vertex_shader =======================================================
void (GLAPIENTRY* qglBindAttribLocationARB)(GLhandleARB programObj, GLuint index, const GLcharARB *name, const char *filename, int line);
void (GLAPIENTRY* qglGetActiveAttribARB)(GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, GLenum *type, GLcharARB *name, const char *filename, int line);
GLint (GLAPIENTRY* qglGetAttribLocationARB)(GLhandleARB programObj, const GLcharARB *name, const char *filename, int line);

static void (GLAPIENTRY* dllBindAttribLocationARB)(GLhandleARB, GLuint, const GLcharARB *);
static void (GLAPIENTRY* dllGetActiveAttribARB)(GLhandleARB, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLcharARB *);
static GLint (GLAPIENTRY* dllGetAttribLocationARB)(GLhandleARB, const GLcharARB *);

static void GLAPIENTRY stdBindAttribLocationARB(GLhandleARB programObj, GLuint index, const GLcharARB *name, const char *filename, int line)
{
	dllBindAttribLocationARB(programObj, index, name);
}
static void GLAPIENTRY stdGetActiveAttribARB(GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, GLenum *type, GLcharARB *name, const char *filename, int line)
{
	dllGetActiveAttribARB(programObj, index, maxLength, length, size, type, name);
}
static GLint GLAPIENTRY stdGetAttribLocationARB(GLhandleARB programObj, const GLcharARB *name, const char *filename, int line)
{
	return dllGetAttribLocationARB(programObj, name);
}

static void GLAPIENTRY dbgBindAttribLocationARB(GLhandleARB programObj, GLuint index, const GLcharARB *name, const char *filename, int line)
{
	dllBindAttribLocationARB(programObj, index, name);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgGetActiveAttribARB(GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, GLenum *type, GLcharARB *name, const char *filename, int line)
{
	dllGetActiveAttribARB(programObj, index, maxLength, length, size, type, name);
	XGL_CheckForError_(filename, line);
}
static GLint GLAPIENTRY dbgGetAttribLocationARB(GLhandleARB programObj, const GLcharARB *name, const char *filename, int line)
{
	GLint i = dllGetAttribLocationARB(programObj, name);
	XGL_CheckForError_(filename, line);
	return i;
}


/// GL_EXT_texture3D ===========================================================
void (GLAPIENTRY* qglTexImage3DEXT)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels, const char *filename, int line);

static void (GLAPIENTRY* dllTexImage3DEXT)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);

static void GLAPIENTRY stdTexImage3DEXT(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels, const char *filename, int line)
{
	if(dllTexImage3DEXT)
		dllTexImage3DEXT(target, level, internalformat, width, height, depth, border, format, type, pixels);
	else
		ri.Com_Error(ERR_FATAL, "stdTexImage3DEXT: function pointer not available in file %s, line %i", filename, line);
}

static void GLAPIENTRY dbgTexImage3DEXT(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels, const char *filename, int line)
{
	if(dllTexImage3DEXT)
		dllTexImage3DEXT(target, level, internalformat, width, height, depth, border, format, type, pixels);
	else
		ri.Com_Error(ERR_FATAL, "dbgTexImage3DEXT: function pointer not available in file %s, line %i", filename, line);
		
	XGL_CheckForError_(filename, line);
}


/// GL_EXT_compiled_vertex_array ===============================================
/*
void (GLAPIENTRY* xglLockArraysEXT) (GLint, GLsizei);
void (GLAPIENTRY* xglUnlockArraysEXT) (void);
*/

/// GL_EXT_draw_range_elements =================================================
void (GLAPIENTRY* qglDrawRangeElementsEXT)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices, const char *filename, int line);

static void (GLAPIENTRY* dllDrawRangeElementsEXT)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);

static void GLAPIENTRY stdDrawRangeElementsEXT(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices, const char *filename, int line)
{
	if(dllDrawRangeElementsEXT)
		dllDrawRangeElementsEXT(mode, start, end, count, type, indices);
	else
		ri.Com_Error(ERR_FATAL, "stdDrawRangeElementsEXT: function pointer not available in file %s, line %i", filename, line);
}

static void GLAPIENTRY dbgDrawRangeElementsEXT(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices, const char *filename, int line)
{
	if(dllDrawRangeElementsEXT)
		dllDrawRangeElementsEXT(mode, start, end, count, type, indices);
	else
		ri.Com_Error(ERR_FATAL, "dbgDrawRangeElementsEXT: function pointer not available in file %s, line %i", filename, line);
		
	XGL_CheckForError_(filename, line);
}


/// GL_NV_vertex_program2 ======================================================
/*
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
*/

/// GL_NV_fragment_program =====================================================
/*
void (GLAPIENTRY* xglProgramNamedParameter4fNV) (GLuint, GLsizei, const GLubyte *, GLfloat, GLfloat, GLfloat, GLfloat);
void (GLAPIENTRY* xglProgramNamedParameter4fvNV) (GLuint, GLsizei, const GLubyte *, const GLfloat *);
void (GLAPIENTRY* xglGetProgramNamedParameterfvNV) (GLuint, GLsizei, const GLubyte *, GLfloat *);
*/

/// GL_EXT_framebuffer_object ==================================================
GLboolean (GLAPIENTRY* qglIsRenderbufferEXT)(GLuint renderbuffer, const char *filename, int line);
void (GLAPIENTRY* qglBindRenderbufferEXT)(GLenum target, GLuint renderbuffer, const char *filename, int line);
void (GLAPIENTRY* qglDeleteRenderbuffersEXT)(GLsizei n, const GLuint *renderbuffers, const char *filename, int line);
void (GLAPIENTRY* qglGenRenderbuffersEXT)(GLsizei n, GLuint *renderbuffers, const char *filename, int line);
void (GLAPIENTRY* qglRenderbufferStorageEXT)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, const char *filename, int line);
void (GLAPIENTRY* qglGetRenderbufferParameterivEXT)(GLenum target, GLenum pname, GLint *params, const char *filename, int line);
GLboolean (GLAPIENTRY* qglIsFramebufferEXT)(GLuint framebuffer, const char *filename, int line);
void (GLAPIENTRY* qglBindFramebufferEXT)(GLenum target, GLuint framebuffer, const char *filename, int line);
void (GLAPIENTRY* qglDeleteFramebuffersEXT)(GLsizei n, const GLuint *framebuffers, const char *filename, int line);
void (GLAPIENTRY* qglGenFramebuffersEXT)(GLsizei n, GLuint *framebuffers, const char *filename, int line);
GLenum (GLAPIENTRY* qglCheckFramebufferStatusEXT)(GLenum target, const char *filename, int line);
void (GLAPIENTRY* qglFramebufferTexture1DEXT)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, const char *filename, int line);
void (GLAPIENTRY* qglFramebufferTexture2DEXT)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, const char *filename, int line);
void (GLAPIENTRY* qglFramebufferTexture3DEXT)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset, const char *filename, int line);
void (GLAPIENTRY* qglFramebufferRenderbufferEXT)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer, const char *filename, int line);
void (GLAPIENTRY* qglGetFramebufferAttachmentParameterivEXT)(GLenum target, GLenum attachment, GLenum pname, GLint *params, const char *filename, int line);
void (GLAPIENTRY* qglGenerateMipmapEXT)(GLenum target, const char *filename, int line);

static GLboolean (GLAPIENTRY* dllIsRenderbufferEXT)(GLuint renderbuffer);
static void (GLAPIENTRY* dllBindRenderbufferEXT)(GLenum target, GLuint renderbuffer);
static void (GLAPIENTRY* dllDeleteRenderbuffersEXT)(GLsizei n, const GLuint *renderbuffers);
static void (GLAPIENTRY* dllGenRenderbuffersEXT)(GLsizei n, GLuint *renderbuffers);
static void (GLAPIENTRY* dllRenderbufferStorageEXT)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
static void (GLAPIENTRY* dllGetRenderbufferParameterivEXT)(GLenum target, GLenum pname, GLint *params);
static GLboolean (GLAPIENTRY* dllIsFramebufferEXT)(GLuint framebuffer);
static void (GLAPIENTRY* dllBindFramebufferEXT)(GLenum target, GLuint framebuffer);
static void (GLAPIENTRY* dllDeleteFramebuffersEXT)(GLsizei n, const GLuint *framebuffers);
static void (GLAPIENTRY* dllGenFramebuffersEXT)(GLsizei n, GLuint *framebuffers);
static GLenum (GLAPIENTRY* dllCheckFramebufferStatusEXT)(GLenum target);
static void (GLAPIENTRY* dllFramebufferTexture1DEXT)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
static void (GLAPIENTRY* dllFramebufferTexture2DEXT)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
static void (GLAPIENTRY* dllFramebufferTexture3DEXT)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
static void (GLAPIENTRY* dllFramebufferRenderbufferEXT)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
static void (GLAPIENTRY* dllGetFramebufferAttachmentParameterivEXT)(GLenum target, GLenum attachment, GLenum pname, GLint *params);
static void (GLAPIENTRY* dllGenerateMipmapEXT)(GLenum target);

static GLboolean GLAPIENTRY stdIsRenderbufferEXT(GLuint renderbuffer, const char *filename, int line)
{
	return dllIsRenderbufferEXT(renderbuffer);
}
static void GLAPIENTRY stdBindRenderbufferEXT(GLenum target, GLuint renderbuffer, const char *filename, int line)
{
	dllBindRenderbufferEXT(target, renderbuffer);
}
static void GLAPIENTRY stdDeleteRenderbuffersEXT(GLsizei n, const GLuint *renderbuffers, const char *filename, int line)
{
	dllDeleteRenderbuffersEXT(n, renderbuffers);
}
static void GLAPIENTRY stdGenRenderbuffersEXT(GLsizei n, GLuint *renderbuffers, const char *filename, int line)
{
	dllGenRenderbuffersEXT(n, renderbuffers);
}
static void GLAPIENTRY stdRenderbufferStorageEXT(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, const char *filename, int line)
{
	dllRenderbufferStorageEXT(target, internalformat, width, height);
}
static void GLAPIENTRY stdGetRenderbufferParameterivEXT(GLenum target, GLenum pname, GLint *params, const char *filename, int line)
{
	dllGetRenderbufferParameterivEXT(target, pname, params);
}
static GLboolean GLAPIENTRY stdIsFramebufferEXT(GLuint framebuffer, const char *filename, int line)
{
	return dllIsFramebufferEXT(framebuffer);
}
static void GLAPIENTRY stdBindFramebufferEXT(GLenum target, GLuint framebuffer, const char *filename, int line)
{
	dllBindFramebufferEXT(target, framebuffer);
}
static void GLAPIENTRY stdDeleteFramebuffersEXT(GLsizei n, const GLuint *framebuffers, const char *filename, int line)
{
	dllDeleteFramebuffersEXT(n, framebuffers);
}
static void GLAPIENTRY stdGenFramebuffersEXT(GLsizei n, GLuint *framebuffers, const char *filename, int line)
{
	dllGenFramebuffersEXT(n, framebuffers);
}
static GLenum GLAPIENTRY stdCheckFramebufferStatusEXT(GLenum target, const char *filename, int line)
{
	return dllCheckFramebufferStatusEXT(target);
}
static void GLAPIENTRY stdFramebufferTexture1DEXT(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, const char *filename, int line)
{
	dllFramebufferTexture1DEXT(target, attachment, textarget, texture, level);
}
static void GLAPIENTRY stdFramebufferTexture2DEXT(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, const char *filename, int line)
{
	dllFramebufferTexture2DEXT(target, attachment, textarget, texture, level);
}
static void GLAPIENTRY stdFramebufferTexture3DEXT(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset, const char *filename, int line)
{
	dllFramebufferTexture3DEXT(target, attachment, textarget, texture, level, zoffset);
}
static void GLAPIENTRY stdFramebufferRenderbufferEXT(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer, const char *filename, int line)
{
	dllFramebufferRenderbufferEXT(target, attachment, renderbuffertarget, renderbuffer);
}
static void GLAPIENTRY stdGetFramebufferAttachmentParameterivEXT(GLenum target, GLenum attachment, GLenum pname, GLint *params, const char *filename, int line)
{
	dllGetFramebufferAttachmentParameterivEXT(target, attachment, pname, params);
}
static void GLAPIENTRY stdGenerateMipmapEXT(GLenum target, const char *filename, int line)
{
	dllGenerateMipmapEXT(target);
}

static GLboolean GLAPIENTRY dbgIsRenderbufferEXT(GLuint renderbuffer, const char *filename, int line)
{
	GLboolean b = dllIsRenderbufferEXT(renderbuffer);
	XGL_CheckForError_(filename, line);
	return b;
}
static void GLAPIENTRY dbgBindRenderbufferEXT(GLenum target, GLuint renderbuffer, const char *filename, int line)
{
	dllBindRenderbufferEXT(target, renderbuffer);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgDeleteRenderbuffersEXT(GLsizei n, const GLuint *renderbuffers, const char *filename, int line)
{
	dllDeleteRenderbuffersEXT(n, renderbuffers);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgGenRenderbuffersEXT(GLsizei n, GLuint *renderbuffers, const char *filename, int line)
{
	dllGenRenderbuffersEXT(n, renderbuffers);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgRenderbufferStorageEXT(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, const char *filename, int line)
{
	dllRenderbufferStorageEXT(target, internalformat, width, height);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgGetRenderbufferParameterivEXT(GLenum target, GLenum pname, GLint *params, const char *filename, int line)
{
	dllGetRenderbufferParameterivEXT(target, pname, params);
	XGL_CheckForError_(filename, line);
}
static GLboolean GLAPIENTRY dbgIsFramebufferEXT(GLuint framebuffer, const char *filename, int line)
{
	GLboolean b = dllIsFramebufferEXT(framebuffer);
	XGL_CheckForError_(filename, line);
	return b;
}
static void GLAPIENTRY dbgBindFramebufferEXT(GLenum target, GLuint framebuffer, const char *filename, int line)
{
	dllBindFramebufferEXT(target, framebuffer);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgDeleteFramebuffersEXT(GLsizei n, const GLuint *framebuffers, const char *filename, int line)
{
	dllDeleteFramebuffersEXT(n, framebuffers);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgGenFramebuffersEXT(GLsizei n, GLuint *framebuffers, const char *filename, int line)
{
	dllGenFramebuffersEXT(n, framebuffers);
	XGL_CheckForError_(filename, line);
}
static GLenum GLAPIENTRY dbgCheckFramebufferStatusEXT(GLenum target, const char *filename, int line)
{
	GLenum e = dllCheckFramebufferStatusEXT(target);
	XGL_CheckForError_(filename, line);
	return e;
}
static void GLAPIENTRY dbgFramebufferTexture1DEXT(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, const char *filename, int line)
{
	dllFramebufferTexture1DEXT(target, attachment, textarget, texture, level);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgFramebufferTexture2DEXT(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, const char *filename, int line)
{
	dllFramebufferTexture2DEXT(target, attachment, textarget, texture, level);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgFramebufferTexture3DEXT(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset, const char *filename, int line)
{
	dllFramebufferTexture3DEXT(target, attachment, textarget, texture, level, zoffset);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgFramebufferRenderbufferEXT(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer, const char *filename, int line)
{
	dllFramebufferRenderbufferEXT(target, attachment, renderbuffertarget, renderbuffer);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgGetFramebufferAttachmentParameterivEXT(GLenum target, GLenum attachment, GLenum pname, GLint *params, const char *filename, int line)
{
	dllGetFramebufferAttachmentParameterivEXT(target, attachment, pname, params);
	XGL_CheckForError_(filename, line);
}
static void GLAPIENTRY dbgGenerateMipmapEXT(GLenum target, const char *filename, int line)
{
	dllGenerateMipmapEXT(target);
	XGL_CheckForError_(filename, line);
}


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

/// ARB_get_proc_address functions =============================================
void*			(*xglXGetProcAddressARB)(const GLubyte *);

/// SGIX_fbconfig functions ====================================================
GLXFBConfigSGIX*	(*xglXChooseFBConfigSGIX)(Display *dpy, int screen, const int *attrib_list, int *nelements);
GLXContext		(*xglXCreateContextWithConfigSGIX)(Display *dpy, GLXFBConfigSGIX config, int render_type, GLXContext share_list, Bool direct);

/// SGIX_pbuffer functions =====================================================
GLXPbuffer		(*xglXCreateGLXPbufferSGIX)(Display *dpy, GLXFBConfig config, unsigned int width, unsigned int height, const int *attrib_list);
void			(*xglXDestroyGLXPbufferSGIX)(Display *, GLXPbufferSGIX);
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
	if(sys_gl.OpenGLLib)
	{
		FreeLibrary(sys_gl.OpenGLLib);
	}
#endif

	sys_gl.OpenGLLib 					= NULL;
	
	qglBegin						= NULL;
	qglColor4f						= NULL;
	qglColor4fv						= NULL;
	qglEnd							= NULL;
	qglTexCoord1f						= NULL;
	qglTexCoord1fv						= NULL;
	qglTexCoord2f						= NULL;
	qglTexCoord2fv						= NULL;
	qglTexCoord3f						= NULL;
	qglTexCoord3fv						= NULL;
	qglVertex3f						= NULL;
	qglVertex3fv						= NULL;
	qglClipPlane						= NULL;
	qglCullFace						= NULL;
	qglFrontFace						= NULL;
	qglPolygonMode						= NULL;
	qglScissor						= NULL;
	qglTexParameterfv					= NULL;
	qglTexParameteri					= NULL;
	qglTexParameteriv					= NULL;
	qglTexImage1D						= NULL;
	qglTexImage2D						= NULL;
	qglTexEnvi						= NULL;
	qglTexGenf						= NULL;
	qglTexGenfv						= NULL;
	qglTexGeni						= NULL;
	qglTexGeniv						= NULL;
	qglDrawBuffer						= NULL;
	qglClear						= NULL;
	qglClearColor						= NULL;
	qglClearStencil						= NULL;
	qglColorMask						= NULL;
	qglDepthMask						= NULL;
	qglDisable						= NULL;
	qglEnable						= NULL;
	qglFlush						= NULL;
	qglPopAttrib						= NULL;
	qglPushAttrib						= NULL;
	qglAlphaFunc						= NULL;
	qglBlendFunc						= NULL;
	qglStencilFunc						= NULL;
	qglStencilOp						= NULL;
	qglDepthFunc						= NULL;
	qglReadBuffer						= NULL;
	qglReadPixels						= NULL;
	qglGetFloatv						= NULL;
	qglGetIntegerv						= NULL;
	qglGetString						= NULL;
	qglDepthRange						= NULL;
	qglLoadIdentity						= NULL;
	qglLoadMatrixf						= NULL;
	qglMatrixMode						= NULL;
	qglViewport						= NULL;
	qglColorPointer						= NULL;
	qglDisableClientState					= NULL;
	qglDrawElements						= NULL;
	qglEnableClientState					= NULL;
	qglIndexPointer						= NULL;
	qglNormalPointer					= NULL;
	qglTexCoordPointer					= NULL;
	qglVertexPointer					= NULL;
	qglPolygonOffset					= NULL;
	qglCopyTexImage2D					= NULL;
	qglCopyTexSubImage2D					= NULL;
	qglBindTexture						= NULL;
	qglDeleteTextures					= NULL;
	qglGenTextures						= NULL;
	
	qglActiveTextureARB					= NULL;
	qglClientActiveTextureARB				= NULL;
	dllActiveTextureARB					= NULL;
	dllClientActiveTextureARB				= NULL;
	
	qglLoadTransposeMatrixfARB				= NULL;
	dllLoadTransposeMatrixfARB				= NULL;
	
	qglVertexAttribPointerARB				= NULL;
	qglEnableVertexAttribArrayARB				= NULL;
	qglDisableVertexAttribArrayARB				= NULL;
	dllVertexAttribPointerARB				= NULL;
	dllEnableVertexAttribArrayARB				= NULL;
	dllDisableVertexAttribArrayARB				= NULL;

	qglBindBufferARB					= NULL;
	qglDeleteBuffersARB					= NULL;
	qglGenBuffersARB					= NULL;
	qglIsBufferARB						= NULL;
	qglBufferDataARB					= NULL;
	qglBufferSubDataARB					= NULL;
	qglGetBufferSubDataARB					= NULL;
	qglMapBufferARB						= NULL;
	qglUnmapBufferARB					= NULL;
	qglGetBufferParameterivARB				= NULL;
	qglGetBufferPointervARB					= NULL;
	dllBindBufferARB					= NULL;
	dllDeleteBuffersARB					= NULL;
	dllGenBuffersARB					= NULL;
	dllIsBufferARB						= NULL;
	dllBufferDataARB					= NULL;
	dllBufferSubDataARB					= NULL;
	dllGetBufferSubDataARB					= NULL;
	dllMapBufferARB						= NULL;
	dllUnmapBufferARB					= NULL;
	dllGetBufferParameterivARB				= NULL;
	dllGetBufferPointervARB					= NULL;
	
	xglGenQueriesARB					= NULL;
	xglDeleteQueriesARB					= NULL;
	xglIsQueryARB						= NULL;
	xglBeginQueryARB					= NULL;
	xglEndQueryARB						= NULL;
	xglGetQueryivARB					= NULL;
	xglGetQueryObjectivARB					= NULL;
	xglGetQueryObjectuivARB					= NULL;
	
	qglDeleteObjectARB					= NULL;
	qglGetHandleARB						= NULL;
	qglDetachObjectARB					= NULL;
	qglCreateShaderObjectARB				= NULL;
	qglShaderSourceARB					= NULL;
	qglCompileShaderARB					= NULL;
	qglCreateProgramObjectARB				= NULL;
	qglAttachObjectARB					= NULL;
	qglLinkProgramARB					= NULL;
	qglUseProgramObjectARB					= NULL;
	qglValidateProgramARB					= NULL;
	qglUniform1fARB						= NULL;
	qglUniform2fARB						= NULL;
	qglUniform3fARB						= NULL;
	qglUniform4fARB						= NULL;
	qglUniform1iARB						= NULL;
	qglUniform2iARB						= NULL;
	qglUniform3iARB						= NULL;
	qglUniform4iARB						= NULL;
	qglUniform2fvARB					= NULL;
	qglUniform3fvARB					= NULL;
	qglUniform4fvARB					= NULL;
	qglUniform2ivARB					= NULL;
	qglUniform3ivARB					= NULL;
	qglUniform4ivARB					= NULL;
	qglUniformMatrix2fvARB					= NULL;
	qglUniformMatrix3fvARB					= NULL;
	qglUniformMatrix4fvARB					= NULL;
	qglGetObjectParameterfvARB				= NULL;
	qglGetObjectParameterivARB				= NULL;
	qglGetInfoLogARB					= NULL;
	qglGetAttachedObjectsARB				= NULL;
	qglGetUniformLocationARB				= NULL;
	qglGetActiveUniformARB					= NULL;
	qglGetUniformfvARB					= NULL;
	qglGetUniformivARB					= NULL;
	qglGetShaderSourceARB					= NULL;
	dllDeleteObjectARB					= NULL;
	dllGetHandleARB						= NULL;
	dllDetachObjectARB					= NULL;
	dllCreateShaderObjectARB				= NULL;
	dllShaderSourceARB					= NULL;
	dllCompileShaderARB					= NULL;
	dllCreateProgramObjectARB				= NULL;
	dllAttachObjectARB					= NULL;
	dllLinkProgramARB					= NULL;
	dllUseProgramObjectARB					= NULL;
	dllValidateProgramARB					= NULL;
	dllUniform1fARB						= NULL;
	dllUniform2fARB						= NULL;
	dllUniform3fARB						= NULL;
	dllUniform4fARB						= NULL;
	dllUniform1iARB						= NULL;
	dllUniform2iARB						= NULL;
	dllUniform3iARB						= NULL;
	dllUniform4iARB						= NULL;
	dllUniform2fvARB					= NULL;
	dllUniform3fvARB					= NULL;
	dllUniform4fvARB					= NULL;
	dllUniform2ivARB					= NULL;
	dllUniform3ivARB					= NULL;
	dllUniform4ivARB					= NULL;
	dllUniformMatrix2fvARB					= NULL;
	dllUniformMatrix3fvARB					= NULL;
	dllUniformMatrix4fvARB					= NULL;
	dllGetObjectParameterfvARB				= NULL;
	dllGetObjectParameterivARB				= NULL;
	dllGetInfoLogARB					= NULL;
	dllGetAttachedObjectsARB				= NULL;
	dllGetUniformLocationARB				= NULL;
	dllGetActiveUniformARB					= NULL;
	dllGetUniformfvARB					= NULL;
	dllGetUniformivARB					= NULL;
	dllGetShaderSourceARB					= NULL;

	qglBindAttribLocationARB				= NULL;
	qglGetActiveAttribARB					= NULL;
	qglGetAttribLocationARB					= NULL;
	dllBindAttribLocationARB				= NULL;
	dllGetActiveAttribARB					= NULL;
	dllGetAttribLocationARB					= NULL;
	
	qglTexImage3DEXT					= NULL;
	dllTexImage3DEXT					= NULL;

	/*
	xglLockArraysEXT					= NULL;
	xglUnlockArraysEXT					= NULL;
	*/
	
	qglDrawRangeElementsEXT					= NULL;
	dllDrawRangeElementsEXT					= NULL;
	
	/*
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
	*/
	
	/*
	xglProgramNamedParameter4fNV				= NULL;
	xglProgramNamedParameter4fvNV				= NULL;
	xglGetProgramNamedParameterfvNV				= NULL;
	*/

	qglIsRenderbufferEXT					= NULL;
	qglBindRenderbufferEXT					= NULL;
	qglDeleteRenderbuffersEXT				= NULL;
	qglGenRenderbuffersEXT					= NULL;
	qglRenderbufferStorageEXT				= NULL;
	qglGetRenderbufferParameterivEXT			= NULL;
	qglIsFramebufferEXT					= NULL;
	qglBindFramebufferEXT					= NULL;
	qglDeleteFramebuffersEXT				= NULL;
	qglGenFramebuffersEXT					= NULL;
	qglCheckFramebufferStatusEXT				= NULL;
	qglFramebufferTexture1DEXT				= NULL;
	qglFramebufferTexture2DEXT				= NULL;
	qglFramebufferTexture3DEXT				= NULL;
	qglFramebufferRenderbufferEXT				= NULL;
	qglGetFramebufferAttachmentParameterivEXT		= NULL;
	qglGenerateMipmapEXT					= NULL;
	dllIsRenderbufferEXT					= NULL;
	dllBindRenderbufferEXT					= NULL;
	dllDeleteRenderbuffersEXT				= NULL;
	dllGenRenderbuffersEXT					= NULL;
	dllRenderbufferStorageEXT				= NULL;
	dllGetRenderbufferParameterivEXT			= NULL;
	dllIsFramebufferEXT					= NULL;
	dllBindFramebufferEXT					= NULL;
	dllDeleteFramebuffersEXT				= NULL;
	dllGenFramebuffersEXT					= NULL;
	dllCheckFramebufferStatusEXT				= NULL;
	dllFramebufferTexture1DEXT				= NULL;
	dllFramebufferTexture2DEXT				= NULL;
	dllFramebufferTexture3DEXT				= NULL;
	dllFramebufferRenderbufferEXT				= NULL;
	dllGetFramebufferAttachmentParameterivEXT		= NULL;
	dllGenerateMipmapEXT					= NULL;
	
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
	
	xglXGetProcAddressARB					= NULL;
	
	xglXQueryExtensionsString				= NULL;
	
	xglXGetCurrentDisplay					= NULL;
	
	xglXGetFBConfigs					= NULL;
	
	xglXChooseFBConfigSGIX					= NULL;
	xglXCreateContextWithConfigSGIX				= NULL;

	xglXCreateGLXPbufferSGIX				= NULL;
	xglXDestroyGLXPbufferSGIX				= NULL;
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
		
		if(err)
			ri.Com_Error(ERR_FATAL, "XGL_GetSymbol: %s by searching symbol '%s'", err, symbolname);
		else
			ri.Com_Error(ERR_FATAL, "XGL_GetSymbol: unknown error by searching symbol '%s'", symbolname);
	}
	else
	{
//		ri.Com_DPrintf("XGL_GetSymbol: '%s'  '%p'\n", symbolname, sym);
	}
	
	return sym;
	
#elif _WIN32
	sym = (void*)GetProcAddress(sys_gl.OpenGLLib, symbolname);
	
    if( ! sym)
	{
		ri.Com_Error(ERR_FATAL, "XGL_GetSymbol: GetProcAddress failed on '%s'", symbolname);
	}

	return sym;	
#endif
}

void*	xglGetProcAddress(const char *symbol)
{
#ifdef __linux__
	return XGL_GetSymbol(symbol);
	
#elif _WIN32
	void* sym = (void*)wglGetProcAddress(symbol);
	
    if(!sym)
		ri.Com_Error(ERR_FATAL, "xglGetProcAddress: failed on '%s'", symbol);
	
	return sym;
#endif
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

	dllBegin = (void (GLAPIENTRY*) (GLenum mode)) XGL_GetSymbol("glBegin");
	dllColor4f = (void (GLAPIENTRY*) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)) XGL_GetSymbol("glColor4f");
	dllColor4fv = (void (GLAPIENTRY*) (const GLfloat *v)) XGL_GetSymbol("glColor4fv");
	dllEnd = (void (GLAPIENTRY*) (void)) XGL_GetSymbol("glEnd");
	dllTexCoord1f = (void (GLAPIENTRY*) (GLfloat s)) XGL_GetSymbol("glTexCoord1f");
	dllTexCoord1fv = (void (GLAPIENTRY*) (const GLfloat *v)) XGL_GetSymbol("glTexCoord1fv");
	dllTexCoord2f = (void (GLAPIENTRY*) (GLfloat s, GLfloat t)) XGL_GetSymbol("glTexCoord2f");
	dllTexCoord2fv = (void (GLAPIENTRY*) (const GLfloat *v)) XGL_GetSymbol("glTexCoord2fv");
	dllTexCoord3f = (void (GLAPIENTRY*) (GLfloat s, GLfloat t, GLfloat r)) XGL_GetSymbol("glTexCoord3f");
	dllTexCoord3fv = (void (GLAPIENTRY*) (const GLfloat *v)) XGL_GetSymbol("glTexCoord3fv");
	dllVertex3f = (void (GLAPIENTRY*) (GLfloat x, GLfloat y, GLfloat z)) XGL_GetSymbol("glVertex3f");
	dllVertex3fv = (void (GLAPIENTRY*) (const GLfloat *v)) XGL_GetSymbol("glVertex3fv");
	dllClipPlane = (void (GLAPIENTRY*)(GLenum plane, const GLdouble *equation)) XGL_GetSymbol("glClipPlane");
	dllCullFace = (void (GLAPIENTRY*) (GLenum mode)) XGL_GetSymbol("glCullFace");
	dllFrontFace = (void (GLAPIENTRY*) (GLenum mode)) XGL_GetSymbol("glFrontFace");
	dllPolygonMode = (void (GLAPIENTRY*) (GLenum face, GLenum mode)) XGL_GetSymbol("glPolygonMode");
	dllScissor = (void (GLAPIENTRY*) (GLint x, GLint y, GLsizei width, GLsizei height)) XGL_GetSymbol("glScissor");
	dllTexParameterfv = (void (GLAPIENTRY*) (GLenum target, GLenum pname, const GLfloat *params)) XGL_GetSymbol("glTexParameterfv");
	dllTexParameteri = (void (GLAPIENTRY*) (GLenum target, GLenum pname, GLint param)) XGL_GetSymbol("glTexParameteri");
	dllTexParameteriv = (void (GLAPIENTRY*) (GLenum target, GLenum pname, const GLint *params)) XGL_GetSymbol("glTexParameteriv");
	dllTexImage1D = (void (GLAPIENTRY*) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels)) XGL_GetSymbol("glTexImage1D");
	dllTexImage2D = (void (GLAPIENTRY*) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels)) XGL_GetSymbol("glTexImage2D");
	dllTexEnvi = (void (GLAPIENTRY*) (GLenum target, GLenum pname, GLint param)) XGL_GetSymbol("glTexEnvi");
	dllTexGenf = (void (GLAPIENTRY*) (GLenum coord, GLenum pname, GLfloat param)) XGL_GetSymbol("glTexGenf");
	dllTexGenfv = (void (GLAPIENTRY*) (GLenum coord, GLenum pname, const GLfloat *params)) XGL_GetSymbol("glTexGenfv");
	dllTexGeni = (void (GLAPIENTRY*) (GLenum coord, GLenum pname, GLint param)) XGL_GetSymbol("glTexGeni");
	dllTexGeniv = (void (GLAPIENTRY*) (GLenum coord, GLenum pname, const GLint *params)) XGL_GetSymbol("glTexGeniv");
	dllDrawBuffer = (void (GLAPIENTRY*) (GLenum mode)) XGL_GetSymbol("glDrawBuffer");
	dllClear = (void (GLAPIENTRY*) (GLbitfield mask)) XGL_GetSymbol("glClear");
        dllClearColor = (void (GLAPIENTRY*) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)) XGL_GetSymbol("glClearColor");
	dllClearStencil = (void (GLAPIENTRY*) (GLint s)) XGL_GetSymbol ("glClearStencil");
	dllColorMask = (void (GLAPIENTRY*) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)) XGL_GetSymbol("glColorMask");
	dllDepthMask = (void (GLAPIENTRY*) (GLboolean flag)) XGL_GetSymbol("glDepthMask");
	dllDisable = (void (GLAPIENTRY*) (GLenum cap)) XGL_GetSymbol("glDisable");
	dllEnable = (void (GLAPIENTRY*) (GLenum cap)) XGL_GetSymbol("glEnable");
	dllFlush = (void (GLAPIENTRY*) (void)) XGL_GetSymbol("glFlush");
	dllPopAttrib = (void (GLAPIENTRY*) (void)) XGL_GetSymbol("glPopAttrib");
	dllPushAttrib = (void (GLAPIENTRY*) (GLbitfield mask)) XGL_GetSymbol("glPushAttrib");
	dllAlphaFunc = (void (GLAPIENTRY*) (GLenum func, GLclampf ref)) XGL_GetSymbol("glAlphaFunc");
	dllBlendFunc = (void (GLAPIENTRY*) (GLenum sfactor, GLenum dfactor)) XGL_GetSymbol("glBlendFunc");
	dllStencilFunc = (void (GLAPIENTRY*) (GLenum func, GLint ref, GLuint mask)) XGL_GetSymbol("glStencilFunc");
	dllStencilOp = (void (GLAPIENTRY*) (GLenum fail, GLenum zfail, GLenum zpass)) XGL_GetSymbol("glStencilOp");
	dllDepthFunc = (void (GLAPIENTRY*) (GLenum func)) XGL_GetSymbol("glDepthFunc");
	dllReadBuffer = (void (GLAPIENTRY*) (GLenum mode)) XGL_GetSymbol("glReadBuffer");
	dllReadPixels = (void (GLAPIENTRY*) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels)) XGL_GetSymbol("glReadPixels");
	dllGetError = (GLenum	(GLAPIENTRY*) (void)) XGL_GetSymbol("glGetError");
	dllGetFloatv = (void (GLAPIENTRY*) (GLenum pname, GLfloat *params)) XGL_GetSymbol("glGetFloatv");
	dllGetIntegerv = (void (GLAPIENTRY*) (GLenum pname, GLint *params)) XGL_GetSymbol("glGetIntegerv");
	dllGetString = (const GLubyte*	(GLAPIENTRY*) (GLenum name)) XGL_GetSymbol("glGetString");
	dllDepthRange = (void (GLAPIENTRY*) (GLclampd zNear, GLclampd zFar)) XGL_GetSymbol("glDepthRange");
	dllLoadIdentity = (void (GLAPIENTRY*) (void)) XGL_GetSymbol("glLoadIdentity");
	dllLoadMatrixf = (void (GLAPIENTRY*) (const GLfloat *m)) XGL_GetSymbol("glLoadMatrixf");
	dllMatrixMode = (void (GLAPIENTRY*) (GLenum mode)) XGL_GetSymbol ("glMatrixMode");
	dllViewport = (void (GLAPIENTRY*) (GLint x, GLint y, GLsizei width, GLsizei height)) XGL_GetSymbol("glViewport");
	dllColorPointer = (void (GLAPIENTRY*)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)) XGL_GetSymbol("glColorPointer");
	dllDisableClientState = (void (GLAPIENTRY*) (GLenum array)) XGL_GetSymbol("glDisableClientState");
	dllDrawElements = (void (GLAPIENTRY*) (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)) XGL_GetSymbol("glDrawElements");
	dllEnableClientState = (void (GLAPIENTRY*) (GLenum array)) XGL_GetSymbol("glEnableClientState");
	dllIndexPointer = (void (GLAPIENTRY*) (GLenum type, GLsizei stride, const GLvoid *pointer)) XGL_GetSymbol("glIndexPointer");
	dllNormalPointer = (void (GLAPIENTRY*) (GLenum type, GLsizei stride, const GLvoid *pointer)) XGL_GetSymbol("glNormalPointer");
	dllTexCoordPointer = (void (GLAPIENTRY*) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)) XGL_GetSymbol("glTexCoordPointer");
	dllVertexPointer = (void (GLAPIENTRY*) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)) XGL_GetSymbol("glVertexPointer");
	dllPolygonOffset = (void (GLAPIENTRY*) (GLfloat factor, GLfloat units)) XGL_GetSymbol("glPolygonOffset");
	dllCopyTexImage2D = (void (GLAPIENTRY*) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)) XGL_GetSymbol("glCopyTexImage2D");
	dllCopyTexSubImage2D = (void (GLAPIENTRY*) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)) XGL_GetSymbol("glCopyTexSubImage2D");
	dllBindTexture = (void (GLAPIENTRY*) (GLenum target, GLuint texture)) XGL_GetSymbol("glBindTexture");
	dllDeleteTextures = (void (GLAPIENTRY*) (GLsizei n, const GLuint *textures)) XGL_GetSymbol("glDeleteTextures");
	dllGenTextures = (void (GLAPIENTRY*) (GLsizei n, GLuint *textures)) XGL_GetSymbol("glGenTextures");

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
	xglMakeCurrent = (BOOL (*)(HDC, HGLRC)) XGL_GetSymbol("wglMakeCurrent");
	xglDeleteContext = (BOOL (*)(HGLRC)) XGL_GetSymbol("wglDeleteContext");
	xglCreateContext = (HGLRC (*)(HDC)) XGL_GetSymbol("wglCreateContext");
#endif

	return true;
}

void	XGL_InitExtensions()
{
	gl_config.arb_multitexture = false;
	gl_config.arb_transpose_matrix = false;
	gl_config.arb_texture_compression = false;
	gl_config.arb_vertex_program = false;
	gl_config.arb_vertex_buffer_object = false;
	gl_config.arb_occlusion_query = false;
	gl_config.arb_shader_objects = false;
	gl_config.arb_vertex_shader = false;
	gl_config.arb_fragment_shader = false;
	gl_config.arb_shading_language_100 = false;
	gl_config.arb_texture_rectangle = false;
	
	gl_config.ext_texture3D = false;
	gl_config.ext_compiled_vertex_array = false;
	gl_config.ext_draw_range_elements = false;
	gl_config.ext_texture_filter_anisotropic = false;
	gl_config.ext_framebuffer_object = false;	


	if(strstr(gl_config.extensions_string, "GL_ARB_multitexture"))
	{
		ri.Com_Printf("...using GL_ARB_multitexture\n");
		dllActiveTextureARB = (void (GLAPIENTRY*) (GLenum texture)) xglGetProcAddress("glActiveTextureARB");
		dllClientActiveTextureARB = (void (GLAPIENTRY*) (GLenum texture)) xglGetProcAddress("glClientActiveTextureARB");
		gl_config.arb_multitexture = true;
	}
	else
	{
		ri.Com_Error(ERR_FATAL, "...GL_ARB_multitexture not found");
	}
	
	if(strstr(gl_config.extensions_string, "GL_ARB_transpose_matrix"))
	{
		ri.Com_Printf("...using GL_ARB_tranpose_matrix\n");
		dllLoadTransposeMatrixfARB = (void (GLAPIENTRY*) (const GLfloat *m)) xglGetProcAddress("glLoadTransposeMatrixf");
		gl_config.arb_transpose_matrix = true;
	}
	else
	{
		ri.Com_Error(ERR_FATAL, "...GL_ARB_transpose_matrix not found");
	}
	
	if(strstr(gl_config.extensions_string, "GL_ARB_texture_compression"))
	{
		if(r_arb_texture_compression->getInteger())
		{
			Com_Printf("...using GL_ARB_texture_compression\n");
			gl_config.arb_texture_compression = true;
		}
		else
		{
			ri.Com_Printf("...ignoring GL_ARB_texture_compression\n");
		}
	}
	else
	{
		ri.Com_Printf("...GL_ARB_texture_compression not found\n");
	}
	
	if(strstr(gl_config.extensions_string, "GL_ARB_vertex_program"))
	{
		ri.Com_Printf("...using GL_ARB_vertex_program\n");
		/// GL_ARB_vertex_program
		dllVertexAttribPointerARB = (void (GLAPIENTRY*) (GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid *)) xglGetProcAddress("glVertexAttribPointerARB");
		dllEnableVertexAttribArrayARB = (void (GLAPIENTRY*) (GLuint)) xglGetProcAddress("glEnableVertexAttribArrayARB");
		dllDisableVertexAttribArrayARB = (void (GLAPIENTRY*) (GLuint)) xglGetProcAddress("glDisableVertexAttribArrayARB");
		
//	xglProgramStringARB = (void (GLAPIENTRY*) (GLenum, GLenum, GLsizei, const GLvoid *)) XGL_GetSymbol("glProgramStringARB");
//	xglBindProgramARB = (void (GLAPIENTRY*) (GLenum, GLuint)) XGL_GetSymbol("glBindProgramARB");
//	xglDeleteProgramsARB = (void (GLAPIENTRY*) (GLsizei, const GLuint *)) XGL_GetSymbol("glDeleteProgramsARB");
//	xglGenProgramsARB = (void (GLAPIENTRY*) (GLsizei, GLuint *)) XGL_GetSymbol("glGenProgramsARB");
//	xglProgramEnvParameter4fARB = (void (GLAPIENTRY*) (GLenum, GLuint, GLfloat, GLfloat, GLfloat, GLfloat)) XGL_GetSymbol("glProgramEnvParameter4fARB");
//	xglProgramEnvParameter4fvARB = (void (GLAPIENTRY*) (GLenum, GLuint, const GLfloat *)) XGL_GetSymbol("glProgramEnvParameter4fvARB");
//	xglProgramLocalParameter4fARB = (void (GLAPIENTRY*) (GLenum, GLuint, GLfloat, GLfloat, GLfloat, GLfloat)) XGL_GetSymbol("glProgramLocalParameter4fARB");
//	xglProgramLocalParameter4fvARB = (void (GLAPIENTRY*) (GLenum, GLuint, const GLfloat *)) XGL_GetSymbol("glProgramLocalParameter4fvARB");
//	xglGetProgramEnvParameterfvARB = (void (GLAPIENTRY*) (GLenum, GLuint, GLfloat *)) XGL_GetSymbol("glGetProgramEnvParameterfvARB");
//	xglGetProgramLocalParameterfvARB = (void (GLAPIENTRY*) (GLenum, GLuint, GLfloat *)) XGL_GetSymbol("glGetProgramLocalParameterfvARB");
//	xglGetProgramivARB = (void (GLAPIENTRY*) (GLenum, GLenum, GLint *)) XGL_GetSymbol("glGetProgramivARB");
//	xglGetProgramStringARB = (void (GLAPIENTRY*) (GLenum, GLenum, GLvoid *)) XGL_GetSymbol("glGetProgramStringARB");
//	xglGetVertexAttribfvARB = (void (GLAPIENTRY*) (GLuint, GLenum, GLfloat *)) XGL_GetSymbol("glGetVertexAttribfvARB");
//	xglGetVertexAttribPointervARB = (void (GLAPIENTRY*) (GLuint, GLenum, GLvoid* *)) XGL_GetSymbol("glGetVertexAttribPointervARB");
//	xglIsProgramARB = (GLboolean (GLAPIENTRY*) (GLuint)) XGL_GetSymbol("glIsProgramARB");
		gl_config.arb_vertex_program = true;
	}
	else
	{
		ri.Com_Error(ERR_FATAL, "...GL_ARB_vertex_program not found");
	}
	
	if(strstr(gl_config.extensions_string, "GL_ARB_vertex_buffer_object"))
	{
		if(r_arb_vertex_buffer_object->getInteger())
		{
			ri.Com_Printf("...using GL_ARB_vertex_buffer_object\n");
			gl_config.arb_vertex_buffer_object = true;
			
			dllBindBufferARB = (void (GLAPIENTRY*) (GLenum, GLuint)) xglGetProcAddress("glBindBufferARB");
			dllDeleteBuffersARB = (void (GLAPIENTRY*) (GLsizei, const GLuint *)) xglGetProcAddress("glDeleteBuffersARB");
			dllGenBuffersARB = (void (GLAPIENTRY*) (GLsizei, GLuint *)) xglGetProcAddress("glGenBuffersARB");
			dllIsBufferARB = (GLboolean (GLAPIENTRY*) (GLuint)) xglGetProcAddress("glIsBufferARB");
			dllBufferDataARB = (void (GLAPIENTRY*) (GLenum, GLsizeiptrARB, const GLvoid *, GLenum)) xglGetProcAddress("glBufferDataARB");
			dllBufferSubDataARB = (void (GLAPIENTRY*) (GLenum, GLintptrARB, GLsizeiptrARB, const GLvoid *)) xglGetProcAddress("glBufferSubDataARB");
			dllGetBufferSubDataARB = (void (GLAPIENTRY*) (GLenum, GLintptrARB, GLsizeiptrARB, GLvoid *)) xglGetProcAddress("glGetBufferSubDataARB");
			dllMapBufferARB = (GLvoid* (GLAPIENTRY*) (GLenum, GLenum)) xglGetProcAddress("glMapBufferARB");
			dllUnmapBufferARB = (GLboolean (GLAPIENTRY*) (GLenum)) xglGetProcAddress("glUnmapBufferARB");
			dllGetBufferParameterivARB = (void (GLAPIENTRY*) (GLenum, GLenum, GLint *)) xglGetProcAddress("glGetBufferParameterivARB");
			dllGetBufferPointervARB = (void (GLAPIENTRY*) (GLenum, GLenum, GLvoid* *)) xglGetProcAddress("glGetBufferPointervARB");
		}
		else
		{
			ri.Com_Printf("...ignoring GL_ARB_vertex_buffer_object\n");
		}
	}
	else
	{
		ri.Com_Printf("...GL_ARB_vertex_buffer_object not found\n");
	}
	
	if(strstr(gl_config.extensions_string, "GL_ARB_occlusion_query"))
	{
		if(r_arb_occlusion_query->getInteger())
		{
			ri.Com_Printf("...using GL_ARB_occlusion_query\n");
			gl_config.arb_occlusion_query = true;
			
			xglGenQueriesARB = (void (GLAPIENTRY*) (GLsizei, GLuint *)) xglGetProcAddress("glGenQueriesARB");
			xglDeleteQueriesARB = (void (GLAPIENTRY*) (GLsizei, const GLuint *)) xglGetProcAddress("glDeleteQueriesARB");
			xglIsQueryARB = (GLboolean (GLAPIENTRY*) (GLuint)) xglGetProcAddress("glIsQueryARB");
			xglBeginQueryARB = (void (GLAPIENTRY*) (GLenum, GLuint)) xglGetProcAddress("glBeginQueryARB");
			xglEndQueryARB = (void (GLAPIENTRY*) (GLenum)) xglGetProcAddress("glEndQueryARB");
			xglGetQueryivARB = (void (GLAPIENTRY*) (GLenum, GLenum, GLint *)) xglGetProcAddress("glGetQueryivARB");
			xglGetQueryObjectivARB = (void (GLAPIENTRY*) (GLuint, GLenum, GLint *)) xglGetProcAddress("glGetQueryObjectivARB");
			xglGetQueryObjectuivARB = (void (GLAPIENTRY*) (GLuint, GLenum, GLuint *)) xglGetProcAddress("glGetQueryObjectuivARB");
		}
		else
		{
			ri.Com_Printf("...ignoring GL_ARB_oclussion_query\n");
		}
	}
	else
	{
		ri.Com_Printf("...GL_ARB_occlusion_query not found\n");
	}

	if(strstr(gl_config.extensions_string, "GL_ARB_shader_objects"))
	{
		ri.Com_Printf("...using GL_ARB_shader_objects\n");
		dllDeleteObjectARB = (void (GLAPIENTRY*) (GLhandleARB)) xglGetProcAddress("glDeleteObjectARB");
		dllGetHandleARB = (GLhandleARB (GLAPIENTRY*) (GLenum)) xglGetProcAddress("glGetHandleARB");
		dllDetachObjectARB = (void (GLAPIENTRY*) (GLhandleARB, GLhandleARB)) xglGetProcAddress("glDetachObjectARB");
		dllCreateShaderObjectARB = (GLhandleARB (GLAPIENTRY*) (GLenum)) xglGetProcAddress("glCreateShaderObjectARB");
		dllShaderSourceARB = (void (GLAPIENTRY*) (GLhandleARB, GLsizei, const GLcharARB* *, const GLint *)) xglGetProcAddress("glShaderSourceARB");
		dllCompileShaderARB = (void (GLAPIENTRY*) (GLhandleARB)) xglGetProcAddress("glCompileShaderARB");
		dllCreateProgramObjectARB = (GLhandleARB (GLAPIENTRY*) (void)) xglGetProcAddress("glCreateProgramObjectARB");
		dllAttachObjectARB = (void (GLAPIENTRY*) (GLhandleARB, GLhandleARB)) xglGetProcAddress("glAttachObjectARB");
		dllLinkProgramARB = (void (GLAPIENTRY*) (GLhandleARB)) xglGetProcAddress("glLinkProgramARB");
		dllUseProgramObjectARB = (void (GLAPIENTRY*) (GLhandleARB)) xglGetProcAddress("glUseProgramObjectARB");
		dllValidateProgramARB = (void (GLAPIENTRY*) (GLhandleARB)) xglGetProcAddress("glValidateProgramARB");
		dllUniform1fARB = (void (GLAPIENTRY*) (GLint, GLfloat)) xglGetProcAddress("glUniform1fARB");
		dllUniform2fARB = (void (GLAPIENTRY*) (GLint, GLfloat, GLfloat)) xglGetProcAddress("glUniform2fARB");
		dllUniform3fARB = (void (GLAPIENTRY*) (GLint, GLfloat, GLfloat, GLfloat)) xglGetProcAddress("glUniform3fARB");
		dllUniform4fARB = (void (GLAPIENTRY*) (GLint, GLfloat, GLfloat, GLfloat, GLfloat)) xglGetProcAddress("glUniform4fARB");
		dllUniform1iARB = (void (GLAPIENTRY*) (GLint, GLint)) xglGetProcAddress("glUniform1iARB");
		dllUniform2iARB = (void (GLAPIENTRY*) (GLint, GLint, GLint)) xglGetProcAddress("glUniform2iARB");
		dllUniform3iARB = (void (GLAPIENTRY*) (GLint, GLint, GLint, GLint)) xglGetProcAddress("glUniform3iARB");
		dllUniform4iARB = (void (GLAPIENTRY*) (GLint, GLint, GLint, GLint, GLint)) xglGetProcAddress("glUniform4iARB");
		dllUniform2fvARB = (void (GLAPIENTRY*) (GLint, GLsizei, const GLfloat *)) xglGetProcAddress("glUniform2fvARB");
		dllUniform3fvARB = (void (GLAPIENTRY*) (GLint, GLsizei, const GLfloat *)) xglGetProcAddress("glUniform3fvARB");
		dllUniform4fvARB = (void (GLAPIENTRY*) (GLint, GLsizei, const GLfloat *)) xglGetProcAddress("glUniform4fvARB");
		dllUniform2ivARB = (void (GLAPIENTRY*) (GLint, GLsizei, const GLint *)) xglGetProcAddress("glUniform2ivARB");
		dllUniform3ivARB = (void (GLAPIENTRY*) (GLint, GLsizei, const GLint *)) xglGetProcAddress("glUniform3ivARB");
		dllUniform4ivARB = (void (GLAPIENTRY*) (GLint, GLsizei, const GLint *)) xglGetProcAddress("glUniform4ivARB");
		dllUniformMatrix2fvARB = (void (GLAPIENTRY*) (GLint, GLsizei, GLboolean, const GLfloat *)) xglGetProcAddress("glUniformMatrix2fvARB");
		dllUniformMatrix3fvARB = (void (GLAPIENTRY*) (GLint, GLsizei, GLboolean, const GLfloat *)) xglGetProcAddress("glUniformMatrix3fvARB");
		dllUniformMatrix4fvARB = (void (GLAPIENTRY*) (GLint, GLsizei, GLboolean, const GLfloat *)) xglGetProcAddress("glUniformMatrix4fvARB");
		dllGetObjectParameterfvARB = (void (GLAPIENTRY*) (GLhandleARB, GLenum, GLfloat *)) xglGetProcAddress("glGetObjectParameterfvARB");
		dllGetObjectParameterivARB = (void (GLAPIENTRY*) (GLhandleARB, GLenum, GLint *)) xglGetProcAddress("glGetObjectParameterivARB");
		dllGetInfoLogARB = (void (GLAPIENTRY*) (GLhandleARB, GLsizei, GLsizei *, GLcharARB *)) xglGetProcAddress("glGetInfoLogARB");
		dllGetAttachedObjectsARB = (void (GLAPIENTRY*) (GLhandleARB, GLsizei, GLsizei *, GLhandleARB *)) xglGetProcAddress("glGetAttachedObjectsARB");
		dllGetUniformLocationARB = (GLint (GLAPIENTRY*) (GLhandleARB, const GLcharARB *)) xglGetProcAddress("glGetUniformLocationARB");
		dllGetActiveUniformARB = (void (GLAPIENTRY*) (GLhandleARB, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLcharARB *)) 	xglGetProcAddress("glGetActiveUniformARB");
		dllGetUniformfvARB = (void (GLAPIENTRY*) (GLhandleARB, GLint, GLfloat *)) xglGetProcAddress("glGetUniformfvARB");
		dllGetUniformivARB = (void (GLAPIENTRY*) (GLhandleARB, GLint, GLint *)) xglGetProcAddress("glGetUniformivARB");
		dllGetShaderSourceARB = (void (GLAPIENTRY*) (GLhandleARB, GLsizei, GLsizei *, GLcharARB *)) xglGetProcAddress("glGetShaderSourceARB");
		gl_config.arb_shader_objects = true;
	}
	else
	{
		ri.Com_Error(ERR_FATAL, "...GL_ARB_shader_objects not found");
	}

	if(strstr(gl_config.extensions_string, "GL_ARB_vertex_shader"))
	{
		ri.Com_Printf("...using GL_ARB_vertex_shader\n");
		dllBindAttribLocationARB = (void (GLAPIENTRY*) (GLhandleARB, GLuint, const GLcharARB *)) xglGetProcAddress("glBindAttribLocationARB");
		dllGetActiveAttribARB = (void (GLAPIENTRY*) (GLhandleARB, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLcharARB *)) 	xglGetProcAddress("glGetActiveAttribARB");
		dllGetAttribLocationARB = (GLint (GLAPIENTRY*) (GLhandleARB, const GLcharARB *)) xglGetProcAddress("glGetAttribLocationARB");
		gl_config.arb_vertex_shader = true;
	}
	else
	{
		ri.Com_Error(ERR_FATAL, "...GL_ARB_vertex_shader not found");
	}

	if(strstr(gl_config.extensions_string, "GL_ARB_fragment_shader"))
	{
		ri.Com_Printf("...using GL_ARB_fragment_shader\n");
		gl_config.arb_fragment_shader = true;
	}
	else
	{
		ri.Com_Error(ERR_FATAL, "...GL_ARB_fragment_shader not found");
	}

	if(strstr(gl_config.extensions_string, "GL_ARB_shading_language_100"))
	{
		ri.Com_Printf("...using GL_ARB_shading_language_100\n");
		gl_config.arb_shading_language_100 = true;
	}
	else
	{
		ri.Com_Error(ERR_FATAL, "...GL_ARB_shading_language_100");
	}
	
	if(strstr(gl_config.extensions_string, "GL_ARB_texture_rectangle"))
	{
		if(r_arb_texture_rectangle->getInteger())
		{
			Com_Printf("...using GL_ARB_texture_rectangle\n");
			gl_config.arb_texture_rectangle = true;
		}
		else
		{
			ri.Com_Printf("...ignoring GL_ARB_texture_rectangle\n");
		}
	}
	else
	{
		ri.Com_Printf("...GL_ARB_texture_rectangle not found\n");
	}
	
	if(strstr(gl_config.extensions_string, "GL_EXT_texture3D"))
	{
		if(r_ext_texture3D->getInteger())
		{
			ri.Com_Printf("...using GL_EXT_texture3D\n");
			dllTexImage3DEXT = (void (GLAPIENTRY*) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels)) xglGetProcAddress("glTexImage3DEXT");
			gl_config.ext_texture3D = true;
		}
		else
		{
			ri.Com_Printf("...ignoring GL_EXT_texture3D\n");
		}
	}
	else
	{
		ri.Com_Printf("...GL_EXT_texture3D not found\n");
	}
	
	/*
	if(strstr(gl_config.extensions_string, "GL_EXT_compiled_vertex_array"))
	{
		if(r_ext_compiled_vertex_array->getInteger())
		{
			ri.Com_Printf("...using GL_EXT_compiled_vertex_array\n");
			xglLockArraysEXT = (void (GLAPIENTRY*) (GLint first, GLsizei count)) xglGetProcAddress("glLockArraysEXT");
			xglUnlockArraysEXT = (void (GLAPIENTRY*) (void)) xglGetProcAddress("glUnlockArraysEXT");
			gl_config.ext_compiled_vertex_array = true;
		}
		else
		{
			ri.Com_Printf("...ignoring GL_EXT_compiled_vertex_array\n");
		}
	}
	else
	{
		ri.Com_Printf("...GL_EXT_compiled_vertex_array not found\n");
	}
	*/
	
	if(strstr(gl_config.extensions_string, "GL_EXT_draw_range_elements"))
	{
		if(r_ext_draw_range_elements->getInteger())
		{
			ri.Com_Printf("...using GL_EXT_draw_range_elements\n");
			dllDrawRangeElementsEXT = (void (GLAPIENTRY*) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices)) xglGetProcAddress("glDrawRangeElementsEXT");
			gl_config.ext_draw_range_elements = true;
		}
		else
		{
			ri.Com_Printf("...ignoring GL_EXT_draw_range_elements\n");
		}
	}
	else
	{
		ri.Com_Printf("...GL_EXT_draw_range_elements not found\n");
	}
	
	if(strstr(gl_config.extensions_string, "GL_EXT_texture_filter_anisotropic"))
	{
		if(r_ext_texture_filter_anisotropic->getInteger())
		{
			ri.Com_Printf("...using GL_EXT_texture_filter_anisotropic\n");
			gl_config.ext_texture_filter_anisotropic = true;
		}
		else
		{
			ri.Com_Printf("...ignoring GL_EXT_texture_filter_anisotropic\n");
		}
	}
	else
	{
		ri.Com_Printf("...GL_EXT_texture_filter_anisotropic not found\n");
	}

	if(strstr(gl_config.extensions_string, "GL_EXT_framebuffer_object"))
	{
		//if(r_ext_framebuffer_object->getInteger())
		{
			ri.Com_Printf("...using GL_EXT_framebuffer_object\n");
			dllIsRenderbufferEXT = (GLboolean (GLAPIENTRY*)(GLuint renderbuffer)) xglGetProcAddress("glIsRenderbufferEXT");
			dllBindRenderbufferEXT = (void (GLAPIENTRY*)(GLenum target, GLuint renderbuffer)) xglGetProcAddress("glBindRenderbufferEXT");
			dllDeleteRenderbuffersEXT = (void (GLAPIENTRY*)(GLsizei n, const GLuint *renderbuffers)) xglGetProcAddress("glDeleteRenderbuffersEXT");
			dllGenRenderbuffersEXT = (void (GLAPIENTRY*)(GLsizei n, GLuint *renderbuffers)) xglGetProcAddress("glGenRenderbuffersEXT");
			dllRenderbufferStorageEXT = (void (GLAPIENTRY*)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)) xglGetProcAddress("glRenderbufferStorageEXT");
			dllGetRenderbufferParameterivEXT = (void (GLAPIENTRY*)(GLenum target, GLenum pname, GLint *params)) xglGetProcAddress("glGetRenderbufferParameterivEXT");
			dllIsFramebufferEXT = (GLboolean (GLAPIENTRY*)(GLuint framebuffer)) xglGetProcAddress("glIsFramebufferEXT");
			dllBindFramebufferEXT = (void (GLAPIENTRY*)(GLenum target, GLuint framebuffer)) xglGetProcAddress("glBindFramebufferEXT");
			dllDeleteFramebuffersEXT = (void (GLAPIENTRY*)(GLsizei n, const GLuint *framebuffers)) xglGetProcAddress("glDeleteFramebuffersEXT");
			dllGenFramebuffersEXT = (void (GLAPIENTRY*)(GLsizei n, GLuint *framebuffers)) xglGetProcAddress("glGenFramebuffersEXT");
			dllCheckFramebufferStatusEXT = (GLenum (GLAPIENTRY*)(GLenum target)) xglGetProcAddress("glCheckFramebufferStatusEXT");
			dllFramebufferTexture1DEXT = (void (GLAPIENTRY*)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)) xglGetProcAddress("glFramebufferTexture1DEXT");
			dllFramebufferTexture2DEXT = (void (GLAPIENTRY*)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)) xglGetProcAddress("glFramebufferTexture2DEXT");
			dllFramebufferTexture3DEXT = (void (GLAPIENTRY*)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)) xglGetProcAddress("glFramebufferTexture3DEXT");
			dllFramebufferRenderbufferEXT = (void (GLAPIENTRY*)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)) xglGetProcAddress("glFramebufferRenderbufferEXT");
			dllGetFramebufferAttachmentParameterivEXT = (void (GLAPIENTRY*)(GLenum target, GLenum attachment, GLenum pname, GLint *params)) xglGetProcAddress("glGetFramebufferAttachmentParameterivEXT");
			dllGenerateMipmapEXT = (void (GLAPIENTRY*)(GLenum target)) xglGetProcAddress("glGenerateMipmapEXT");
			gl_config.ext_framebuffer_object = true;
		}
		//else
		//{
		//	ri.Com_Printf("...ignoring GL_EXT_framebuffer_object\n");
		//}
	}
	else
	{
		ri.Com_Error(ERR_FATAL, "...GL_EXT_framebuffer_object not found");
	}
}

void	XGL_EnableDebugging(bool enable)
{
	if(enable)
	{
		qglBegin						= dbgBegin;
		qglColor4f						= dbgColor4f;
		qglColor4fv						= dbgColor4fv;
		qglEnd							= dbgEnd;
		qglTexCoord1f						= dbgTexCoord1f;
		qglTexCoord1fv						= dbgTexCoord1fv;
		qglTexCoord2f						= dbgTexCoord2f;
		qglTexCoord2fv						= dbgTexCoord2fv;
		qglTexCoord3f						= dbgTexCoord3f;
		qglTexCoord3fv						= dbgTexCoord3fv;
		qglVertex3f						= dbgVertex3f;
		qglVertex3fv						= dbgVertex3fv;
		qglClipPlane						= dbgClipPlane;
		qglCullFace						= dbgCullFace;
		qglFrontFace						= dbgFrontFace;
		qglPolygonMode						= dbgPolygonMode;
		qglScissor						= dbgScissor;
		qglTexParameterfv					= dbgTexParameterfv;
		qglTexParameteri					= dbgTexParameteri;
		qglTexParameteriv					= dbgTexParameteriv;
		qglTexImage1D						= dbgTexImage1D;
		qglTexImage2D						= dbgTexImage2D;
		qglTexEnvi						= dbgTexEnvi;
		qglTexGenf						= dbgTexGenf;
		qglTexGenfv						= dbgTexGenfv;
		qglTexGeni						= dbgTexGeni;
		qglTexGeniv						= dbgTexGeniv;
		qglDrawBuffer						= dbgDrawBuffer;
		qglClear						= dbgClear;
		qglClearColor						= dbgClearColor;
		qglClearStencil						= dbgClearStencil;
		qglColorMask						= dbgColorMask;
		qglDepthMask						= dbgDepthMask;
		qglDisable						= dbgDisable;
		qglEnable						= dbgEnable;
		qglFlush						= dbgFlush;
		qglPopAttrib						= dbgPopAttrib;
		qglPushAttrib						= dbgPushAttrib;
		qglAlphaFunc						= dbgAlphaFunc;
		qglBlendFunc						= dbgBlendFunc;
		qglStencilFunc						= dbgStencilFunc;
		qglStencilOp						= dbgStencilOp;
		qglDepthFunc						= dbgDepthFunc;
		qglReadBuffer						= dbgReadBuffer;
		qglReadPixels						= dbgReadPixels;
		qglGetFloatv						= dbgGetFloatv;
		qglGetIntegerv						= dbgGetIntegerv;
		qglGetString						= dbgGetString;
		qglDepthRange						= dbgDepthRange;
		qglLoadIdentity						= dbgLoadIdentity;
		qglLoadMatrixf						= dbgLoadMatrixf;
		qglMatrixMode						= dbgMatrixMode;
		qglViewport						= dbgViewport;
		qglColorPointer						= dbgColorPointer;
		qglDisableClientState					= dbgDisableClientState;
		qglDrawElements						= dbgDrawElements;
		qglEnableClientState					= dbgEnableClientState;
		qglIndexPointer						= dbgIndexPointer;
		qglNormalPointer					= dbgNormalPointer;
		qglTexCoordPointer					= dbgTexCoordPointer;
		qglVertexPointer					= dbgVertexPointer;
		qglPolygonOffset					= dbgPolygonOffset;
		qglCopyTexImage2D					= dbgCopyTexImage2D;
		qglCopyTexSubImage2D					= dbgCopyTexSubImage2D;
		qglBindTexture						= dbgBindTexture;
		qglDeleteTextures					= dbgDeleteTextures;
		qglGenTextures						= dbgGenTextures;
		
		qglActiveTextureARB					= dbgActiveTextureARB;
		qglClientActiveTextureARB				= dbgClientActiveTextureARB;
		
		qglLoadTransposeMatrixfARB				= dbgLoadTransposeMatrixfARB;
		
		qglVertexAttribPointerARB				= dbgVertexAttribPointerARB;
		qglEnableVertexAttribArrayARB				= dbgEnableVertexAttribArrayARB;
		qglDisableVertexAttribArrayARB				= dbgDisableVertexAttribArrayARB;
		
		qglBindBufferARB					= dbgBindBufferARB;
		qglDeleteBuffersARB					= dbgDeleteBuffersARB;
		qglGenBuffersARB					= dbgGenBuffersARB;
		qglIsBufferARB						= dbgIsBufferARB;
		qglBufferDataARB					= dbgBufferDataARB;
		qglBufferSubDataARB					= dbgBufferSubDataARB;
		qglGetBufferSubDataARB					= dbgGetBufferSubDataARB;
		qglMapBufferARB						= dbgMapBufferARB;
		qglUnmapBufferARB					= dbgUnmapBufferARB;
		qglGetBufferParameterivARB				= dbgGetBufferParameterivARB;
		qglGetBufferPointervARB					= dbgGetBufferPointervARB;

		qglDeleteObjectARB					= dbgDeleteObjectARB;
		qglGetHandleARB						= dbgGetHandleARB;
		qglDetachObjectARB					= dbgDetachObjectARB;
		qglCreateShaderObjectARB				= dbgCreateShaderObjectARB;
		qglShaderSourceARB					= dbgShaderSourceARB;
		qglCompileShaderARB					= dbgCompileShaderARB;
		qglCreateProgramObjectARB				= dbgCreateProgramObjectARB;
		qglAttachObjectARB					= dbgAttachObjectARB;
		qglLinkProgramARB					= dbgLinkProgramARB;
		qglUseProgramObjectARB					= dbgUseProgramObjectARB;
		qglValidateProgramARB					= dbgValidateProgramARB;
		qglUniform1fARB						= dbgUniform1fARB;
		qglUniform2fARB						= dbgUniform2fARB;
		qglUniform3fARB						= dbgUniform3fARB;
		qglUniform4fARB						= dbgUniform4fARB;
		qglUniform1iARB						= dbgUniform1iARB;
		qglUniform2iARB						= dbgUniform2iARB;
		qglUniform3iARB						= dbgUniform3iARB;
		qglUniform4iARB						= dbgUniform4iARB;
		qglUniform2fvARB					= dbgUniform2fvARB;
		qglUniform3fvARB					= dbgUniform3fvARB;
		qglUniform4fvARB					= dbgUniform4fvARB;
		qglUniform2ivARB					= dbgUniform2ivARB;
		qglUniform3ivARB					= dbgUniform3ivARB;
		qglUniform4ivARB					= dbgUniform4ivARB;
		qglUniformMatrix2fvARB					= dbgUniformMatrix2fvARB;
		qglUniformMatrix3fvARB					= dbgUniformMatrix3fvARB;
		qglUniformMatrix4fvARB					= dbgUniformMatrix4fvARB;
		qglGetObjectParameterfvARB				= dbgGetObjectParameterfvARB;
		qglGetObjectParameterivARB				= dbgGetObjectParameterivARB;
		qglGetInfoLogARB					= dbgGetInfoLogARB;
		qglGetAttachedObjectsARB				= dbgGetAttachedObjectsARB;
		qglGetUniformLocationARB				= dbgGetUniformLocationARB;
		qglGetActiveUniformARB					= dbgGetActiveUniformARB;
		qglGetUniformfvARB					= dbgGetUniformfvARB;
		qglGetUniformivARB					= dbgGetUniformivARB;
		qglGetShaderSourceARB					= dbgGetShaderSourceARB;

		qglBindAttribLocationARB				= dbgBindAttribLocationARB;
		qglGetActiveAttribARB					= dbgGetActiveAttribARB;
		qglGetAttribLocationARB					= dbgGetAttribLocationARB;
		
		qglTexImage3DEXT					= dbgTexImage3DEXT;
		
		qglDrawRangeElementsEXT					= dbgDrawRangeElementsEXT;

		qglIsRenderbufferEXT					= dbgIsRenderbufferEXT;
		qglBindRenderbufferEXT					= dbgBindRenderbufferEXT;
		qglDeleteRenderbuffersEXT				= dbgDeleteRenderbuffersEXT;
		qglGenRenderbuffersEXT					= dbgGenRenderbuffersEXT;
		qglRenderbufferStorageEXT				= dbgRenderbufferStorageEXT;
		qglGetRenderbufferParameterivEXT			= dbgGetRenderbufferParameterivEXT;
		qglIsFramebufferEXT					= dbgIsFramebufferEXT;
		qglBindFramebufferEXT					= dbgBindFramebufferEXT;
		qglDeleteFramebuffersEXT				= dbgDeleteFramebuffersEXT;
		qglGenFramebuffersEXT					= dbgGenFramebuffersEXT;
		qglCheckFramebufferStatusEXT				= dbgCheckFramebufferStatusEXT;
		qglFramebufferTexture1DEXT				= dbgFramebufferTexture1DEXT;
		qglFramebufferTexture2DEXT				= dbgFramebufferTexture2DEXT;
		qglFramebufferTexture3DEXT				= dbgFramebufferTexture3DEXT;
		qglFramebufferRenderbufferEXT				= dbgFramebufferRenderbufferEXT;
		qglGetFramebufferAttachmentParameterivEXT		= dbgGetFramebufferAttachmentParameterivEXT;
		qglGenerateMipmapEXT					= dbgGenerateMipmapEXT;
	}
	else
	{
		qglBegin						= stdBegin;
		qglColor4f						= stdColor4f;
		qglColor4fv						= stdColor4fv;
		qglEnd							= stdEnd;
		qglTexCoord1f						= stdTexCoord1f;
		qglTexCoord1fv						= stdTexCoord1fv;
		qglTexCoord2f						= stdTexCoord2f;
		qglTexCoord2fv						= stdTexCoord2fv;
		qglTexCoord3f						= stdTexCoord3f;
		qglTexCoord3fv						= stdTexCoord3fv;
		qglVertex3f						= stdVertex3f;
		qglVertex3fv						= stdVertex3fv;
		qglClipPlane						= stdClipPlane;
		qglCullFace						= stdCullFace;
		qglFrontFace						= stdFrontFace;
		qglPolygonMode						= stdPolygonMode;
		qglScissor						= stdScissor;
		qglTexParameterfv					= stdTexParameterfv;
		qglTexParameteri					= stdTexParameteri;
		qglTexParameteriv					= stdTexParameteriv;
		qglTexImage1D						= stdTexImage1D;
		qglTexImage2D						= stdTexImage2D;
		qglTexEnvi						= stdTexEnvi;
		qglTexGenf						= stdTexGenf;
		qglTexGenfv						= stdTexGenfv;
		qglTexGeni						= stdTexGeni;
		qglTexGeniv						= stdTexGeniv;
		qglDrawBuffer						= stdDrawBuffer;
		qglClear						= stdClear;
		qglClearColor						= stdClearColor;
		qglClearStencil						= stdClearStencil;
		qglColorMask						= stdColorMask;
		qglDepthMask						= stdDepthMask;
		qglDisable						= stdDisable;
		qglEnable						= stdEnable;
		qglFlush						= stdFlush;
		qglPopAttrib						= stdPopAttrib;
		qglPushAttrib						= stdPushAttrib;
		qglAlphaFunc						= stdAlphaFunc;
		qglBlendFunc						= stdBlendFunc;
		qglStencilFunc						= stdStencilFunc;
		qglStencilOp						= stdStencilOp;
		qglDepthFunc						= stdDepthFunc;
		qglReadBuffer						= stdReadBuffer;
		qglReadPixels						= stdReadPixels;
		qglGetFloatv						= stdGetFloatv;
		qglGetIntegerv						= stdGetIntegerv;
		qglGetString						= stdGetString;
		qglDepthRange						= stdDepthRange;
		qglLoadIdentity						= stdLoadIdentity;
		qglLoadMatrixf						= stdLoadMatrixf;
		qglMatrixMode						= stdMatrixMode;
		qglViewport						= stdViewport;
		qglColorPointer						= stdColorPointer;
		qglDisableClientState					= stdDisableClientState;
		qglDrawElements						= stdDrawElements;
		qglEnableClientState					= stdEnableClientState;
		qglIndexPointer						= stdIndexPointer;
		qglNormalPointer					= stdNormalPointer;
		qglTexCoordPointer					= stdTexCoordPointer;
		qglVertexPointer					= stdVertexPointer;
		qglPolygonOffset					= stdPolygonOffset;
		qglCopyTexImage2D					= stdCopyTexImage2D;
		qglCopyTexSubImage2D					= stdCopyTexSubImage2D;
		qglBindTexture						= stdBindTexture;
		qglDeleteTextures					= stdDeleteTextures;
		qglGenTextures						= stdGenTextures;
		
		qglActiveTextureARB					= stdActiveTextureARB;
		qglClientActiveTextureARB				= stdClientActiveTextureARB;
		
		qglLoadTransposeMatrixfARB				= stdLoadTransposeMatrixfARB;
		
		qglVertexAttribPointerARB				= stdVertexAttribPointerARB;
		qglEnableVertexAttribArrayARB				= stdEnableVertexAttribArrayARB;
		qglDisableVertexAttribArrayARB				= stdDisableVertexAttribArrayARB;
		
		qglBindBufferARB					= stdBindBufferARB;
		qglDeleteBuffersARB					= stdDeleteBuffersARB;
		qglGenBuffersARB					= stdGenBuffersARB;
		qglIsBufferARB						= stdIsBufferARB;
		qglBufferDataARB					= stdBufferDataARB;
		qglBufferSubDataARB					= stdBufferSubDataARB;
		qglGetBufferSubDataARB					= stdGetBufferSubDataARB;
		qglMapBufferARB						= stdMapBufferARB;
		qglUnmapBufferARB					= stdUnmapBufferARB;
		qglGetBufferParameterivARB				= stdGetBufferParameterivARB;
		qglGetBufferPointervARB					= stdGetBufferPointervARB;

		qglDeleteObjectARB					= stdDeleteObjectARB;
		qglGetHandleARB						= stdGetHandleARB;
		qglDetachObjectARB					= stdDetachObjectARB;
		qglCreateShaderObjectARB				= stdCreateShaderObjectARB;
		qglShaderSourceARB					= stdShaderSourceARB;
		qglCompileShaderARB					= stdCompileShaderARB;
		qglCreateProgramObjectARB				= stdCreateProgramObjectARB;
		qglAttachObjectARB					= stdAttachObjectARB;
		qglLinkProgramARB					= stdLinkProgramARB;
		qglUseProgramObjectARB					= stdUseProgramObjectARB;
		qglValidateProgramARB					= stdValidateProgramARB;
		qglUniform1fARB						= stdUniform1fARB;
		qglUniform2fARB						= stdUniform2fARB;
		qglUniform3fARB						= stdUniform3fARB;
		qglUniform4fARB						= stdUniform4fARB;
		qglUniform1iARB						= stdUniform1iARB;
		qglUniform2iARB						= stdUniform2iARB;
		qglUniform3iARB						= stdUniform3iARB;
		qglUniform4iARB						= stdUniform4iARB;
		qglUniform2fvARB					= stdUniform2fvARB;
		qglUniform3fvARB					= stdUniform3fvARB;
		qglUniform4fvARB					= stdUniform4fvARB;
		qglUniform2ivARB					= stdUniform2ivARB;
		qglUniform3ivARB					= stdUniform3ivARB;
		qglUniform4ivARB					= stdUniform4ivARB;
		qglUniformMatrix2fvARB					= stdUniformMatrix2fvARB;
		qglUniformMatrix3fvARB					= stdUniformMatrix3fvARB;
		qglUniformMatrix4fvARB					= stdUniformMatrix4fvARB;
		qglGetObjectParameterfvARB				= stdGetObjectParameterfvARB;
		qglGetObjectParameterivARB				= stdGetObjectParameterivARB;
		qglGetInfoLogARB					= stdGetInfoLogARB;
		qglGetAttachedObjectsARB				= stdGetAttachedObjectsARB;
		qglGetUniformLocationARB				= stdGetUniformLocationARB;
		qglGetActiveUniformARB					= stdGetActiveUniformARB;
		qglGetUniformfvARB					= stdGetUniformfvARB;
		qglGetUniformivARB					= stdGetUniformivARB;
		qglGetShaderSourceARB					= stdGetShaderSourceARB;

		qglBindAttribLocationARB				= stdBindAttribLocationARB;
		qglGetActiveAttribARB					= stdGetActiveAttribARB;
		qglGetAttribLocationARB					= stdGetAttribLocationARB;
		
		qglTexImage3DEXT					= stdTexImage3DEXT;
		
		qglDrawRangeElementsEXT					= stdDrawRangeElementsEXT;

		qglIsRenderbufferEXT					= stdIsRenderbufferEXT;
		qglBindRenderbufferEXT					= stdBindRenderbufferEXT;
		qglDeleteRenderbuffersEXT				= stdDeleteRenderbuffersEXT;
		qglGenRenderbuffersEXT					= stdGenRenderbuffersEXT;
		qglRenderbufferStorageEXT				= stdRenderbufferStorageEXT;
		qglGetRenderbufferParameterivEXT			= stdGetRenderbufferParameterivEXT;
		qglIsFramebufferEXT					= stdIsFramebufferEXT;
		qglBindFramebufferEXT					= stdBindFramebufferEXT;
		qglDeleteFramebuffersEXT				= stdDeleteFramebuffersEXT;
		qglGenFramebuffersEXT					= stdGenFramebuffersEXT;
		qglCheckFramebufferStatusEXT				= stdCheckFramebufferStatusEXT;
		qglFramebufferTexture1DEXT				= stdFramebufferTexture1DEXT;
		qglFramebufferTexture2DEXT				= stdFramebufferTexture2DEXT;
		qglFramebufferTexture3DEXT				= stdFramebufferTexture3DEXT;
		qglFramebufferRenderbufferEXT				= stdFramebufferRenderbufferEXT;
		qglGetFramebufferAttachmentParameterivEXT		= stdGetFramebufferAttachmentParameterivEXT;
		qglGenerateMipmapEXT					= stdGenerateMipmapEXT;
	}
}


void	XGL_CheckForError_(const std::string &file, int line)
{
	int err = dllGetError();
	char* errstr;
	
	if(err != GL_NO_ERROR)
	{
		switch(err)
		{
			case GL_INVALID_ENUM:
				errstr = "GL_INVALID_ENUM";
				break;
			
			case GL_INVALID_VALUE:
				errstr = "GL_INVALID_VALUE";
				break;
			
			case GL_INVALID_OPERATION:
				errstr = "GL_INVALID_OPERATION";
				break;
			
			case GL_STACK_OVERFLOW:
				errstr = "GL_STACK_OVERFLOW";
				break;
			
			case GL_STACK_UNDERFLOW:
				errstr = "GL_STACK_UNDERFLOW";
				break;
			
			case GL_OUT_OF_MEMORY:
				errstr = "GL_OUT_OF_MEMORY";
				break;
			
			case GL_TABLE_TOO_LARGE:
				errstr = "GL_TABLE_TOO_LARGE";
				break;
			
			default:
				errstr = "unknown error";
		}
		
#if 0
		ri.Com_Error(ERR_FATAL, "XGL_CheckForError: %s in file %s, line %i", errstr, file.c_str(), line);
#else
		ri.Com_Printf("OpenGL error: %s in file %s, line %i\n", errstr, file.c_str(), line);
#endif
	}
}
