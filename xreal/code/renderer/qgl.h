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
/*
** QGL.H
*/

#ifndef __QGL_H__
#define __QGL_H__

#if defined( __LINT__ )

#include <GL/gl.h>

#elif defined( _WIN32 )

#if _MSC_VER
#pragma warning (disable: 4201)
#pragma warning (disable: 4214)
#pragma warning (disable: 4514)
#pragma warning (disable: 4032)
#pragma warning (disable: 4201)
#pragma warning (disable: 4214)
#endif

#include <windows.h>
#include <GL/gl.h>
#include <GL/glext.h>
#elif defined(MACOS_X)

#include "macosx_glimp.h"

#elif defined( __linux__ )

#include <GL/gl.h>
#include <GL/glx.h>
// bk001129 - from cvs1.17 (mkv)
#if defined(__FX__)
#include <GL/fxmesa.h>
#endif

#elif defined( __FreeBSD__ )	// rb010123

#include <GL/gl.h>
#include <GL/glx.h>
#if defined(__FX__)
#include <GL/fxmesa.h>
#endif

#else

#include <gl.h>

#endif

#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef WINAPI
#define WINAPI
#endif


#ifndef GL_ARB_multitexture
#define GL_TEXTURE0_ARB                   0x84C0
#define GL_TEXTURE1_ARB                   0x84C1
#define GL_TEXTURE2_ARB                   0x84C2
#define GL_TEXTURE3_ARB                   0x84C3
#define GL_TEXTURE4_ARB                   0x84C4
#define GL_TEXTURE5_ARB                   0x84C5
#define GL_TEXTURE6_ARB                   0x84C6
#define GL_TEXTURE7_ARB                   0x84C7
#define GL_TEXTURE8_ARB                   0x84C8
#define GL_TEXTURE9_ARB                   0x84C9
#define GL_TEXTURE10_ARB                  0x84CA
#define GL_TEXTURE11_ARB                  0x84CB
#define GL_TEXTURE12_ARB                  0x84CC
#define GL_TEXTURE13_ARB                  0x84CD
#define GL_TEXTURE14_ARB                  0x84CE
#define GL_TEXTURE15_ARB                  0x84CF
#define GL_TEXTURE16_ARB                  0x84D0
#define GL_TEXTURE17_ARB                  0x84D1
#define GL_TEXTURE18_ARB                  0x84D2
#define GL_TEXTURE19_ARB                  0x84D3
#define GL_TEXTURE20_ARB                  0x84D4
#define GL_TEXTURE21_ARB                  0x84D5
#define GL_TEXTURE22_ARB                  0x84D6
#define GL_TEXTURE23_ARB                  0x84D7
#define GL_TEXTURE24_ARB                  0x84D8
#define GL_TEXTURE25_ARB                  0x84D9
#define GL_TEXTURE26_ARB                  0x84DA
#define GL_TEXTURE27_ARB                  0x84DB
#define GL_TEXTURE28_ARB                  0x84DC
#define GL_TEXTURE29_ARB                  0x84DD
#define GL_TEXTURE30_ARB                  0x84DE
#define GL_TEXTURE31_ARB                  0x84DF
#define GL_ACTIVE_TEXTURE_ARB             0x84E0
#define GL_CLIENT_ACTIVE_TEXTURE_ARB      0x84E1
#define GL_MAX_TEXTURE_UNITS_ARB          0x84E2
#endif

#ifndef GL_ARB_transpose_matrix
#define GL_TRANSPOSE_MODELVIEW_MATRIX_ARB 0x84E3
#define GL_TRANSPOSE_PROJECTION_MATRIX_ARB 0x84E4
#define GL_TRANSPOSE_TEXTURE_MATRIX_ARB   0x84E5
#define GL_TRANSPOSE_COLOR_MATRIX_ARB     0x84E6
#endif

#ifndef GL_ARB_texture_cube_map
#define GL_NORMAL_MAP_ARB                 0x8511
#define GL_REFLECTION_MAP_ARB             0x8512
#define GL_TEXTURE_CUBE_MAP_ARB           0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP_ARB   0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB 0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB 0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB 0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB 0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB 0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB 0x851A
#define GL_PROXY_TEXTURE_CUBE_MAP_ARB     0x851B
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB  0x851C
#endif

#ifndef GL_ARB_texture_compression
#define GL_COMPRESSED_ALPHA_ARB           0x84E9
#define GL_COMPRESSED_LUMINANCE_ARB       0x84EA
#define GL_COMPRESSED_LUMINANCE_ALPHA_ARB 0x84EB
#define GL_COMPRESSED_INTENSITY_ARB       0x84EC
#define GL_COMPRESSED_RGB_ARB             0x84ED
#define GL_COMPRESSED_RGBA_ARB            0x84EE
#define GL_TEXTURE_COMPRESSION_HINT_ARB   0x84EF
#define GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB 0x86A0
#define GL_TEXTURE_COMPRESSED_ARB         0x86A1
#define GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB 0x86A2
#define GL_COMPRESSED_TEXTURE_FORMATS_ARB 0x86A3
#endif

#ifndef GL_S3_s3tc
#define GL_RGB_S3TC                       0x83A0
#define GL_RGB4_S3TC                      0x83A1
#define GL_RGBA_S3TC                      0x83A2
#define GL_RGBA4_S3TC                     0x83A3
#endif


// NOTE: some Linux platforms would need those prototypes
#if defined(MACOS_X)
typedef void    (APIENTRY * PFNGLMULTITEXCOORD1DARBPROC) (GLenum target, GLdouble s);
typedef void    (APIENTRY * PFNGLMULTITEXCOORD1DVARBPROC) (GLenum target, const GLdouble * v);
typedef void    (APIENTRY * PFNGLMULTITEXCOORD1FARBPROC) (GLenum target, GLfloat s);
typedef void    (APIENTRY * PFNGLMULTITEXCOORD1FVARBPROC) (GLenum target, const GLfloat * v);
typedef void    (APIENTRY * PFNGLMULTITEXCOORD1IARBPROC) (GLenum target, GLint s);
typedef void    (APIENTRY * PFNGLMULTITEXCOORD1IVARBPROC) (GLenum target, const GLint * v);
typedef void    (APIENTRY * PFNGLMULTITEXCOORD1SARBPROC) (GLenum target, GLshort s);
typedef void    (APIENTRY * PFNGLMULTITEXCOORD1SVARBPROC) (GLenum target, const GLshort * v);
typedef void    (APIENTRY * PFNGLMULTITEXCOORD2DARBPROC) (GLenum target, GLdouble s, GLdouble t);
typedef void    (APIENTRY * PFNGLMULTITEXCOORD2DVARBPROC) (GLenum target, const GLdouble * v);
typedef void    (APIENTRY * PFNGLMULTITEXCOORD2FARBPROC) (GLenum target, GLfloat s, GLfloat t);
typedef void    (APIENTRY * PFNGLMULTITEXCOORD2FVARBPROC) (GLenum target, const GLfloat * v);
typedef void    (APIENTRY * PFNGLMULTITEXCOORD2IARBPROC) (GLenum target, GLint s, GLint t);
typedef void    (APIENTRY * PFNGLMULTITEXCOORD2IVARBPROC) (GLenum target, const GLint * v);
typedef void    (APIENTRY * PFNGLMULTITEXCOORD2SARBPROC) (GLenum target, GLshort s, GLshort t);
typedef void    (APIENTRY * PFNGLMULTITEXCOORD2SVARBPROC) (GLenum target, const GLshort * v);
typedef void    (APIENTRY * PFNGLMULTITEXCOORD3DARBPROC) (GLenum target, GLdouble s, GLdouble t, GLdouble r);
typedef void    (APIENTRY * PFNGLMULTITEXCOORD3DVARBPROC) (GLenum target, const GLdouble * v);
typedef void    (APIENTRY * PFNGLMULTITEXCOORD3FARBPROC) (GLenum target, GLfloat s, GLfloat t, GLfloat r);
typedef void    (APIENTRY * PFNGLMULTITEXCOORD3FVARBPROC) (GLenum target, const GLfloat * v);
typedef void    (APIENTRY * PFNGLMULTITEXCOORD3IARBPROC) (GLenum target, GLint s, GLint t, GLint r);
typedef void    (APIENTRY * PFNGLMULTITEXCOORD3IVARBPROC) (GLenum target, const GLint * v);
typedef void    (APIENTRY * PFNGLMULTITEXCOORD3SARBPROC) (GLenum target, GLshort s, GLshort t, GLshort r);
typedef void    (APIENTRY * PFNGLMULTITEXCOORD3SVARBPROC) (GLenum target, const GLshort * v);
typedef void    (APIENTRY * PFNGLMULTITEXCOORD4DARBPROC) (GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
typedef void    (APIENTRY * PFNGLMULTITEXCOORD4DVARBPROC) (GLenum target, const GLdouble * v);
typedef void    (APIENTRY * PFNGLMULTITEXCOORD4FARBPROC) (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
typedef void    (APIENTRY * PFNGLMULTITEXCOORD4FVARBPROC) (GLenum target, const GLfloat * v);
typedef void    (APIENTRY * PFNGLMULTITEXCOORD4IARBPROC) (GLenum target, GLint s, GLint t, GLint r, GLint q);
typedef void    (APIENTRY * PFNGLMULTITEXCOORD4IVARBPROC) (GLenum target, const GLint * v);
typedef void    (APIENTRY * PFNGLMULTITEXCOORD4SARBPROC) (GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
typedef void    (APIENTRY * PFNGLMULTITEXCOORD4SVARBPROC) (GLenum target, const GLshort * v);
typedef void    (APIENTRY * PFNGLACTIVETEXTUREARBPROC) (GLenum target);
typedef void    (APIENTRY * PFNGLCLIENTACTIVETEXTUREARBPROC) (GLenum target);
#endif

// TTimo - VC7 / XP ?
#if defined(WIN32) && ! defined(__MINGW32__) 
typedef void    (APIENTRY * PFNGLMULTITEXCOORD2FARBPROC) (GLenum target, GLfloat s, GLfloat t);
typedef void    (APIENTRY * PFNGLACTIVETEXTUREARBPROC) (GLenum target);
typedef void    (APIENTRY * PFNGLCLIENTACTIVETEXTUREARBPROC) (GLenum target);
#endif

// extensions will be function pointers on all platforms

// GL_ARB_multitexture
extern void     (APIENTRY * qglMultiTexCoord2fARB) (GLenum texture, GLfloat s, GLfloat t);
extern void     (APIENTRY * qglActiveTextureARB) (GLenum texture);
extern void     (APIENTRY * qglClientActiveTextureARB) (GLenum texture);

// GL_ARB_transpose_matrix
extern void     (APIENTRY * qglLoadTransposeMatrixfARB) (const GLfloat * m);

// GL_ARB_vertex_program
extern void     (APIENTRY * qglVertexAttribPointerARB) (GLuint index, GLint size, GLenum type, GLboolean normalized,
														GLsizei stride, const GLvoid * pointer);
extern void     (APIENTRY * qglEnableVertexAttribArrayARB) (GLuint index);
extern void     (APIENTRY * qglDisableVertexAttribArrayARB) (GLuint index);

// GL_ARB_vertex_buffer_object
//typedef ptrdiff_t GLintptrARB;
//typedef ptrdiff_t GLsizeiptrARB;
extern void     (APIENTRY * qglBindBufferARB) (GLenum target, GLuint buffer);
extern void     (APIENTRY * qglDeleteBuffersARB) (GLsizei n, const GLuint * buffers);
extern void     (APIENTRY * qglGenBuffersARB) (GLsizei n, GLuint * buffers);
extern          GLboolean(APIENTRY * qglIsBufferARB) (GLuint buffer);
extern void     (APIENTRY * qglBufferDataARB) (GLenum target, GLsizeiptrARB size, const GLvoid * data, GLenum usage);
extern void     (APIENTRY * qglBufferSubDataARB) (GLenum target, GLintptrARB offset, GLsizeiptrARB size, const GLvoid * data);
extern void     (APIENTRY * qglGetBufferSubDataARB) (GLenum target, GLintptrARB offset, GLsizeiptrARB size, GLvoid * data);
extern GLvoid  *(APIENTRY * qglMapBufferARB) (GLenum target, GLenum access);
extern          GLboolean(APIENTRY * qglUnmapBufferARB) (GLenum target);
extern void     (APIENTRY * qglGetBufferParameterivARB) (GLenum target, GLenum pname, GLint * params);
extern void     (APIENTRY * qglGetBufferPointervARB) (GLenum target, GLenum pname, GLvoid * *params);

// GL_ARB_occlusion_query
extern void     (APIENTRY * qglGenQueriesARB) (GLsizei n, GLuint * ids);
extern void     (APIENTRY * qglDeleteQueriesARB) (GLsizei n, const GLuint * ids);
extern GLboolean (APIENTRY * qglIsQueryARB) (GLuint id);
extern void     (APIENTRY * qglBeginQueryARB) (GLenum target, GLuint id);
extern void     (APIENTRY * qglEndQueryARB) (GLenum target);
extern void     (APIENTRY * qglGetQueryivARB) (GLenum target, GLenum pname, GLint * params);
extern void     (APIENTRY * qglGetQueryObjectivARB) (GLuint id, GLenum pname, GLint * params);
extern void     (APIENTRY * qglGetQueryObjectuivARB) (GLuint id, GLenum pname, GLuint * params);

// GL_ARB_shader_objects
//typedef char GLcharARB;           /* native character */
//typedef unsigned int GLhandleARB; /* shader object handle */

extern void     (APIENTRY * qglDeleteObjectARB) (GLhandleARB obj);
extern          GLhandleARB(APIENTRY * qglGetHandleARB) (GLenum pname);
extern void     (APIENTRY * qglDetachObjectARB) (GLhandleARB containerObj, GLhandleARB attachedObj);
extern          GLhandleARB(APIENTRY * qglCreateShaderObjectARB) (GLenum shaderType);
extern void     (APIENTRY * qglShaderSourceARB) (GLhandleARB shaderObj, GLsizei count, const GLcharARB * *string,
												 const GLint * length);
extern void     (APIENTRY * qglCompileShaderARB) (GLhandleARB shaderObj);
extern          GLhandleARB(APIENTRY * qglCreateProgramObjectARB) (void);
extern void     (APIENTRY * qglAttachObjectARB) (GLhandleARB containerObj, GLhandleARB obj);
extern void     (APIENTRY * qglLinkProgramARB) (GLhandleARB programObj);
extern void     (APIENTRY * qglUseProgramObjectARB) (GLhandleARB programObj);
extern void     (APIENTRY * qglValidateProgramARB) (GLhandleARB programObj);
extern void     (APIENTRY * qglUniform1fARB) (GLint location, GLfloat v0);
extern void     (APIENTRY * qglUniform2fARB) (GLint location, GLfloat v0, GLfloat v1);
extern void     (APIENTRY * qglUniform3fARB) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
extern void     (APIENTRY * qglUniform4fARB) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
extern void     (APIENTRY * qglUniform1iARB) (GLint location, GLint v0);
extern void     (APIENTRY * qglUniform2iARB) (GLint location, GLint v0, GLint v1);
extern void     (APIENTRY * qglUniform3iARB) (GLint location, GLint v0, GLint v1, GLint v2);
extern void     (APIENTRY * qglUniform4iARB) (GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
extern void     (APIENTRY * qglUniform2fvARB) (GLint location, GLsizei count, const GLfloat * value);
extern void     (APIENTRY * qglUniform3fvARB) (GLint location, GLsizei count, const GLfloat * value);
extern void     (APIENTRY * qglUniform4fvARB) (GLint location, GLsizei count, const GLfloat * value);
extern void     (APIENTRY * qglUniform2ivARB) (GLint location, GLsizei count, const GLint * value);
extern void     (APIENTRY * qglUniform3ivARB) (GLint location, GLsizei count, const GLint * value);
extern void     (APIENTRY * qglUniform4ivARB) (GLint location, GLsizei count, const GLint * value);
extern void     (APIENTRY * qglUniformMatrix2fvARB) (GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
extern void     (APIENTRY * qglUniformMatrix3fvARB) (GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
extern void     (APIENTRY * qglUniformMatrix4fvARB) (GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
extern void     (APIENTRY * qglGetObjectParameterfvARB) (GLhandleARB obj, GLenum pname, GLfloat * params);
extern void     (APIENTRY * qglGetObjectParameterivARB) (GLhandleARB obj, GLenum pname, GLint * params);
extern void     (APIENTRY * qglGetInfoLogARB) (GLhandleARB obj, GLsizei maxLength, GLsizei * length, GLcharARB * infoLog);
extern void     (APIENTRY * qglGetAttachedObjectsARB) (GLhandleARB containerObj, GLsizei maxCount, GLsizei * count,
													   GLhandleARB * obj);
extern          GLint(APIENTRY * qglGetUniformLocationARB) (GLhandleARB programObj, const GLcharARB * name);
extern void     (APIENTRY * qglGetActiveUniformARB) (GLhandleARB programObj, GLuint index, GLsizei maxIndex, GLsizei * length,
													 GLint * size, GLenum * type, GLcharARB * name);
extern void     (APIENTRY * qglGetUniformfvARB) (GLhandleARB programObj, GLint location, GLfloat * params);
extern void     (APIENTRY * qglGetUniformivARB) (GLhandleARB programObj, GLint location, GLint * params);
extern void     (APIENTRY * qglGetShaderSourceARB) (GLhandleARB obj, GLsizei maxLength, GLsizei * length, GLcharARB * source);

// GL_ARB_vertex_shader
extern void     (APIENTRY * qglBindAttribLocationARB) (GLhandleARB programObj, GLuint index, const GLcharARB * name);
extern void     (APIENTRY * qglGetActiveAttribARB) (GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei * length,
													GLint * size, GLenum * type, GLcharARB * name);
extern          GLint(APIENTRY * qglGetAttribLocationARB) (GLhandleARB programObj, const GLcharARB * name);

// GL_EXT_compiled_vertex_array
extern void     (APIENTRY * qglLockArraysEXT) (GLint, GLint);
extern void     (APIENTRY * qglUnlockArraysEXT) (void);

// GL_EXT_stencil_two_side
extern void     (APIENTRY * qglActiveStencilFaceEXT) (GLenum face);

// GL_EXT_depth_bounds_test
extern void     (APIENTRY * qglDepthBoundsEXT) (GLclampd zmin, GLclampd zmax);


// GL_EXT_framebuffer_object
extern          GLboolean(APIENTRY * qglIsRenderbufferEXT) (GLuint renderbuffer);
extern void     (APIENTRY * qglBindRenderbufferEXT) (GLenum target, GLuint renderbuffer);
extern void     (APIENTRY * qglDeleteRenderbuffersEXT) (GLsizei n, const GLuint * renderbuffers);
extern void     (APIENTRY * qglGenRenderbuffersEXT) (GLsizei n, GLuint * renderbuffers);
extern void     (APIENTRY * qglRenderbufferStorageEXT) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
extern void     (APIENTRY * qglGetRenderbufferParameterivEXT) (GLenum target, GLenum pname, GLint * params);
extern          GLboolean(APIENTRY * qglIsFramebufferEXT) (GLuint framebuffer);
extern void     (APIENTRY * qglBindFramebufferEXT) (GLenum target, GLuint framebuffer);
extern void     (APIENTRY * qglDeleteFramebuffersEXT) (GLsizei n, const GLuint * framebuffers);
extern void     (APIENTRY * qglGenFramebuffersEXT) (GLsizei n, GLuint * framebuffers);
extern          GLenum(APIENTRY * qglCheckFramebufferStatusEXT) (GLenum target);
extern void     (APIENTRY * qglFramebufferTexture1DEXT) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture,
														 GLint level);
extern void     (APIENTRY * qglFramebufferTexture2DEXT) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture,
														 GLint level);
extern void     (APIENTRY * qglFramebufferTexture3DEXT) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture,
														 GLint level, GLint zoffset);
extern void     (APIENTRY * qglFramebufferRenderbufferEXT) (GLenum target, GLenum attachment, GLenum renderbuffertarget,
															GLuint renderbuffer);
extern void     (APIENTRY * qglGetFramebufferAttachmentParameterivEXT) (GLenum target, GLenum attachment, GLenum pname,
																		GLint * params);
extern void     (APIENTRY * qglGenerateMipmapEXT) (GLenum target);


//===========================================================================

// non-windows systems will just redefine qgl* to gl*
#if !defined( _WIN32 ) && !defined(MACOS_X) && !defined( __linux__ ) && !defined( __FreeBSD__ )	// rb010123

#include "qgl_linked.h"

#elif defined(MACOS_X)
// This includes #ifdefs for optional logging and GL error checking after every GL call as well as #defines to prevent incorrect usage of the non-'qgl' versions of the GL API.
#include "macosx_qgl.h"

#else

// windows systems use a function pointer for each call so we can load minidrivers

extern void     (APIENTRY * qglAccum) (GLenum op, GLfloat value);
extern void     (APIENTRY * qglAlphaFunc) (GLenum func, GLclampf ref);
extern          GLboolean(APIENTRY * qglAreTexturesResident) (GLsizei n, const GLuint * textures, GLboolean * residences);
extern void     (APIENTRY * qglArrayElement) (GLint i);
extern void     (APIENTRY * qglBegin) (GLenum mode);
extern void     (APIENTRY * qglBindTexture) (GLenum target, GLuint texture);
extern void     (APIENTRY * qglBitmap) (GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove,
										const GLubyte * bitmap);
extern void     (APIENTRY * qglBlendFunc) (GLenum sfactor, GLenum dfactor);
extern void     (APIENTRY * qglCallList) (GLuint list);
extern void     (APIENTRY * qglCallLists) (GLsizei n, GLenum type, const GLvoid * lists);
extern void     (APIENTRY * qglClear) (GLbitfield mask);
extern void     (APIENTRY * qglClearAccum) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
extern void     (APIENTRY * qglClearColor) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
extern void     (APIENTRY * qglClearDepth) (GLclampd depth);
extern void     (APIENTRY * qglClearIndex) (GLfloat c);
extern void     (APIENTRY * qglClearStencil) (GLint s);
extern void     (APIENTRY * qglClipPlane) (GLenum plane, const GLdouble * equation);
extern void     (APIENTRY * qglColor3b) (GLbyte red, GLbyte green, GLbyte blue);
extern void     (APIENTRY * qglColor3bv) (const GLbyte * v);
extern void     (APIENTRY * qglColor3d) (GLdouble red, GLdouble green, GLdouble blue);
extern void     (APIENTRY * qglColor3dv) (const GLdouble * v);
extern void     (APIENTRY * qglColor3f) (GLfloat red, GLfloat green, GLfloat blue);
extern void     (APIENTRY * qglColor3fv) (const GLfloat * v);
extern void     (APIENTRY * qglColor3i) (GLint red, GLint green, GLint blue);
extern void     (APIENTRY * qglColor3iv) (const GLint * v);
extern void     (APIENTRY * qglColor3s) (GLshort red, GLshort green, GLshort blue);
extern void     (APIENTRY * qglColor3sv) (const GLshort * v);
extern void     (APIENTRY * qglColor3ub) (GLubyte red, GLubyte green, GLubyte blue);
extern void     (APIENTRY * qglColor3ubv) (const GLubyte * v);
extern void     (APIENTRY * qglColor3ui) (GLuint red, GLuint green, GLuint blue);
extern void     (APIENTRY * qglColor3uiv) (const GLuint * v);
extern void     (APIENTRY * qglColor3us) (GLushort red, GLushort green, GLushort blue);
extern void     (APIENTRY * qglColor3usv) (const GLushort * v);
extern void     (APIENTRY * qglColor4b) (GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
extern void     (APIENTRY * qglColor4bv) (const GLbyte * v);
extern void     (APIENTRY * qglColor4d) (GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
extern void     (APIENTRY * qglColor4dv) (const GLdouble * v);
extern void     (APIENTRY * qglColor4f) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
extern void     (APIENTRY * qglColor4fv) (const GLfloat * v);
extern void     (APIENTRY * qglColor4i) (GLint red, GLint green, GLint blue, GLint alpha);
extern void     (APIENTRY * qglColor4iv) (const GLint * v);
extern void     (APIENTRY * qglColor4s) (GLshort red, GLshort green, GLshort blue, GLshort alpha);
extern void     (APIENTRY * qglColor4sv) (const GLshort * v);
extern void     (APIENTRY * qglColor4ub) (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
extern void     (APIENTRY * qglColor4ubv) (const GLubyte * v);
extern void     (APIENTRY * qglColor4ui) (GLuint red, GLuint green, GLuint blue, GLuint alpha);
extern void     (APIENTRY * qglColor4uiv) (const GLuint * v);
extern void     (APIENTRY * qglColor4us) (GLushort red, GLushort green, GLushort blue, GLushort alpha);
extern void     (APIENTRY * qglColor4usv) (const GLushort * v);
extern void     (APIENTRY * qglColorMask) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
extern void     (APIENTRY * qglColorMaterial) (GLenum face, GLenum mode);
extern void     (APIENTRY * qglColorPointer) (GLint size, GLenum type, GLsizei stride, const GLvoid * pointer);
extern void     (APIENTRY * qglCopyPixels) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
extern void     (APIENTRY * qglCopyTexImage1D) (GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y,
												GLsizei width, GLint border);
extern void     (APIENTRY * qglCopyTexImage2D) (GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y,
												GLsizei width, GLsizei height, GLint border);
extern void     (APIENTRY * qglCopyTexSubImage1D) (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
extern void     (APIENTRY * qglCopyTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y,
												   GLsizei width, GLsizei height);
extern void     (APIENTRY * qglCullFace) (GLenum mode);
extern void     (APIENTRY * qglDeleteLists) (GLuint list, GLsizei range);
extern void     (APIENTRY * qglDeleteTextures) (GLsizei n, const GLuint * textures);
extern void     (APIENTRY * qglDepthFunc) (GLenum func);
extern void     (APIENTRY * qglDepthMask) (GLboolean flag);
extern void     (APIENTRY * qglDepthRange) (GLclampd zNear, GLclampd zFar);
extern void     (APIENTRY * qglDisable) (GLenum cap);
extern void     (APIENTRY * qglDisableClientState) (GLenum array);
extern void     (APIENTRY * qglDrawArrays) (GLenum mode, GLint first, GLsizei count);
extern void     (APIENTRY * qglDrawBuffer) (GLenum mode);
extern void     (APIENTRY * qglDrawElements) (GLenum mode, GLsizei count, GLenum type, const GLvoid * indices);
extern void     (APIENTRY * qglDrawPixels) (GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * pixels);
extern void     (APIENTRY * qglEdgeFlag) (GLboolean flag);
extern void     (APIENTRY * qglEdgeFlagPointer) (GLsizei stride, const GLvoid * pointer);
extern void     (APIENTRY * qglEdgeFlagv) (const GLboolean * flag);
extern void     (APIENTRY * qglEnable) (GLenum cap);
extern void     (APIENTRY * qglEnableClientState) (GLenum array);
extern void     (APIENTRY * qglEnd) (void);
extern void     (APIENTRY * qglEndList) (void);
extern void     (APIENTRY * qglEvalCoord1d) (GLdouble u);
extern void     (APIENTRY * qglEvalCoord1dv) (const GLdouble * u);
extern void     (APIENTRY * qglEvalCoord1f) (GLfloat u);
extern void     (APIENTRY * qglEvalCoord1fv) (const GLfloat * u);
extern void     (APIENTRY * qglEvalCoord2d) (GLdouble u, GLdouble v);
extern void     (APIENTRY * qglEvalCoord2dv) (const GLdouble * u);
extern void     (APIENTRY * qglEvalCoord2f) (GLfloat u, GLfloat v);
extern void     (APIENTRY * qglEvalCoord2fv) (const GLfloat * u);
extern void     (APIENTRY * qglEvalMesh1) (GLenum mode, GLint i1, GLint i2);
extern void     (APIENTRY * qglEvalMesh2) (GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
extern void     (APIENTRY * qglEvalPoint1) (GLint i);
extern void     (APIENTRY * qglEvalPoint2) (GLint i, GLint j);
extern void     (APIENTRY * qglFeedbackBuffer) (GLsizei size, GLenum type, GLfloat * buffer);
extern void     (APIENTRY * qglFinish) (void);
extern void     (APIENTRY * qglFlush) (void);
extern void     (APIENTRY * qglFogf) (GLenum pname, GLfloat param);
extern void     (APIENTRY * qglFogfv) (GLenum pname, const GLfloat * params);
extern void     (APIENTRY * qglFogi) (GLenum pname, GLint param);
extern void     (APIENTRY * qglFogiv) (GLenum pname, const GLint * params);
extern void     (APIENTRY * qglFrontFace) (GLenum mode);
extern void     (APIENTRY * qglFrustum) (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear,
										 GLdouble zFar);
extern          GLuint(APIENTRY * qglGenLists) (GLsizei range);
extern void     (APIENTRY * qglGenTextures) (GLsizei n, GLuint * textures);
extern void     (APIENTRY * qglGetBooleanv) (GLenum pname, GLboolean * params);
extern void     (APIENTRY * qglGetClipPlane) (GLenum plane, GLdouble * equation);
extern void     (APIENTRY * qglGetDoublev) (GLenum pname, GLdouble * params);
extern          GLenum(APIENTRY * qglGetError) (void);
extern void     (APIENTRY * qglGetFloatv) (GLenum pname, GLfloat * params);
extern void     (APIENTRY * qglGetIntegerv) (GLenum pname, GLint * params);
extern void     (APIENTRY * qglGetLightfv) (GLenum light, GLenum pname, GLfloat * params);
extern void     (APIENTRY * qglGetLightiv) (GLenum light, GLenum pname, GLint * params);
extern void     (APIENTRY * qglGetMapdv) (GLenum target, GLenum query, GLdouble * v);
extern void     (APIENTRY * qglGetMapfv) (GLenum target, GLenum query, GLfloat * v);
extern void     (APIENTRY * qglGetMapiv) (GLenum target, GLenum query, GLint * v);
extern void     (APIENTRY * qglGetMaterialfv) (GLenum face, GLenum pname, GLfloat * params);
extern void     (APIENTRY * qglGetMaterialiv) (GLenum face, GLenum pname, GLint * params);
extern void     (APIENTRY * qglGetPixelMapfv) (GLenum map, GLfloat * values);
extern void     (APIENTRY * qglGetPixelMapuiv) (GLenum map, GLuint * values);
extern void     (APIENTRY * qglGetPixelMapusv) (GLenum map, GLushort * values);
extern void     (APIENTRY * qglGetPointerv) (GLenum pname, GLvoid * *params);
extern void     (APIENTRY * qglGetPolygonStipple) (GLubyte * mask);
extern const GLubyte *(APIENTRY * qglGetString) (GLenum name);
extern void     (APIENTRY * qglGetTexEnvfv) (GLenum target, GLenum pname, GLfloat * params);
extern void     (APIENTRY * qglGetTexEnviv) (GLenum target, GLenum pname, GLint * params);
extern void     (APIENTRY * qglGetTexGendv) (GLenum coord, GLenum pname, GLdouble * params);
extern void     (APIENTRY * qglGetTexGenfv) (GLenum coord, GLenum pname, GLfloat * params);
extern void     (APIENTRY * qglGetTexGeniv) (GLenum coord, GLenum pname, GLint * params);
extern void     (APIENTRY * qglGetTexImage) (GLenum target, GLint level, GLenum format, GLenum type, GLvoid * pixels);
extern void     (APIENTRY * qglGetTexLevelParameterfv) (GLenum target, GLint level, GLenum pname, GLfloat * params);
extern void     (APIENTRY * qglGetTexLevelParameteriv) (GLenum target, GLint level, GLenum pname, GLint * params);
extern void     (APIENTRY * qglGetTexParameterfv) (GLenum target, GLenum pname, GLfloat * params);
extern void     (APIENTRY * qglGetTexParameteriv) (GLenum target, GLenum pname, GLint * params);
extern void     (APIENTRY * qglHint) (GLenum target, GLenum mode);
extern void     (APIENTRY * qglIndexMask) (GLuint mask);
extern void     (APIENTRY * qglIndexPointer) (GLenum type, GLsizei stride, const GLvoid * pointer);
extern void     (APIENTRY * qglIndexd) (GLdouble c);
extern void     (APIENTRY * qglIndexdv) (const GLdouble * c);
extern void     (APIENTRY * qglIndexf) (GLfloat c);
extern void     (APIENTRY * qglIndexfv) (const GLfloat * c);
extern void     (APIENTRY * qglIndexi) (GLint c);
extern void     (APIENTRY * qglIndexiv) (const GLint * c);
extern void     (APIENTRY * qglIndexs) (GLshort c);
extern void     (APIENTRY * qglIndexsv) (const GLshort * c);
extern void     (APIENTRY * qglIndexub) (GLubyte c);
extern void     (APIENTRY * qglIndexubv) (const GLubyte * c);
extern void     (APIENTRY * qglInitNames) (void);
extern void     (APIENTRY * qglInterleavedArrays) (GLenum format, GLsizei stride, const GLvoid * pointer);
extern          GLboolean(APIENTRY * qglIsEnabled) (GLenum cap);
extern          GLboolean(APIENTRY * qglIsList) (GLuint list);
extern          GLboolean(APIENTRY * qglIsTexture) (GLuint texture);
extern void     (APIENTRY * qglLightModelf) (GLenum pname, GLfloat param);
extern void     (APIENTRY * qglLightModelfv) (GLenum pname, const GLfloat * params);
extern void     (APIENTRY * qglLightModeli) (GLenum pname, GLint param);
extern void     (APIENTRY * qglLightModeliv) (GLenum pname, const GLint * params);
extern void     (APIENTRY * qglLightf) (GLenum light, GLenum pname, GLfloat param);
extern void     (APIENTRY * qglLightfv) (GLenum light, GLenum pname, const GLfloat * params);
extern void     (APIENTRY * qglLighti) (GLenum light, GLenum pname, GLint param);
extern void     (APIENTRY * qglLightiv) (GLenum light, GLenum pname, const GLint * params);
extern void     (APIENTRY * qglLineStipple) (GLint factor, GLushort pattern);
extern void     (APIENTRY * qglLineWidth) (GLfloat width);
extern void     (APIENTRY * qglListBase) (GLuint base);
extern void     (APIENTRY * qglLoadIdentity) (void);
extern void     (APIENTRY * qglLoadMatrixd) (const GLdouble * m);
extern void     (APIENTRY * qglLoadMatrixf) (const GLfloat * m);
extern void     (APIENTRY * qglLoadName) (GLuint name);
extern void     (APIENTRY * qglLogicOp) (GLenum opcode);
extern void     (APIENTRY * qglMap1d) (GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order,
									   const GLdouble * points);
extern void     (APIENTRY * qglMap1f) (GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat * points);
extern void     (APIENTRY * qglMap2d) (GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1,
									   GLdouble v2, GLint vstride, GLint vorder, const GLdouble * points);
extern void     (APIENTRY * qglMap2f) (GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2,
									   GLint vstride, GLint vorder, const GLfloat * points);
extern void     (APIENTRY * qglMapGrid1d) (GLint un, GLdouble u1, GLdouble u2);
extern void     (APIENTRY * qglMapGrid1f) (GLint un, GLfloat u1, GLfloat u2);
extern void     (APIENTRY * qglMapGrid2d) (GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
extern void     (APIENTRY * qglMapGrid2f) (GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
extern void     (APIENTRY * qglMaterialf) (GLenum face, GLenum pname, GLfloat param);
extern void     (APIENTRY * qglMaterialfv) (GLenum face, GLenum pname, const GLfloat * params);
extern void     (APIENTRY * qglMateriali) (GLenum face, GLenum pname, GLint param);
extern void     (APIENTRY * qglMaterialiv) (GLenum face, GLenum pname, const GLint * params);
extern void     (APIENTRY * qglMatrixMode) (GLenum mode);
extern void     (APIENTRY * qglMultMatrixd) (const GLdouble * m);
extern void     (APIENTRY * qglMultMatrixf) (const GLfloat * m);
extern void     (APIENTRY * qglNewList) (GLuint list, GLenum mode);
extern void     (APIENTRY * qglNormal3b) (GLbyte nx, GLbyte ny, GLbyte nz);
extern void     (APIENTRY * qglNormal3bv) (const GLbyte * v);
extern void     (APIENTRY * qglNormal3d) (GLdouble nx, GLdouble ny, GLdouble nz);
extern void     (APIENTRY * qglNormal3dv) (const GLdouble * v);
extern void     (APIENTRY * qglNormal3f) (GLfloat nx, GLfloat ny, GLfloat nz);
extern void     (APIENTRY * qglNormal3fv) (const GLfloat * v);
extern void     (APIENTRY * qglNormal3i) (GLint nx, GLint ny, GLint nz);
extern void     (APIENTRY * qglNormal3iv) (const GLint * v);
extern void     (APIENTRY * qglNormal3s) (GLshort nx, GLshort ny, GLshort nz);
extern void     (APIENTRY * qglNormal3sv) (const GLshort * v);
extern void     (APIENTRY * qglNormalPointer) (GLenum type, GLsizei stride, const GLvoid * pointer);
extern void     (APIENTRY * qglOrtho) (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear,
									   GLdouble zFar);
extern void     (APIENTRY * qglPassThrough) (GLfloat token);
extern void     (APIENTRY * qglPixelMapfv) (GLenum map, GLsizei mapsize, const GLfloat * values);
extern void     (APIENTRY * qglPixelMapuiv) (GLenum map, GLsizei mapsize, const GLuint * values);
extern void     (APIENTRY * qglPixelMapusv) (GLenum map, GLsizei mapsize, const GLushort * values);
extern void     (APIENTRY * qglPixelStoref) (GLenum pname, GLfloat param);
extern void     (APIENTRY * qglPixelStorei) (GLenum pname, GLint param);
extern void     (APIENTRY * qglPixelTransferf) (GLenum pname, GLfloat param);
extern void     (APIENTRY * qglPixelTransferi) (GLenum pname, GLint param);
extern void     (APIENTRY * qglPixelZoom) (GLfloat xfactor, GLfloat yfactor);
extern void     (APIENTRY * qglPointSize) (GLfloat size);
extern void     (APIENTRY * qglPolygonMode) (GLenum face, GLenum mode);
extern void     (APIENTRY * qglPolygonOffset) (GLfloat factor, GLfloat units);
extern void     (APIENTRY * qglPolygonStipple) (const GLubyte * mask);
extern void     (APIENTRY * qglPopAttrib) (void);
extern void     (APIENTRY * qglPopClientAttrib) (void);
extern void     (APIENTRY * qglPopMatrix) (void);
extern void     (APIENTRY * qglPopName) (void);
extern void     (APIENTRY * qglPrioritizeTextures) (GLsizei n, const GLuint * textures, const GLclampf * priorities);
extern void     (APIENTRY * qglPushAttrib) (GLbitfield mask);
extern void     (APIENTRY * qglPushClientAttrib) (GLbitfield mask);
extern void     (APIENTRY * qglPushMatrix) (void);
extern void     (APIENTRY * qglPushName) (GLuint name);
extern void     (APIENTRY * qglRasterPos2d) (GLdouble x, GLdouble y);
extern void     (APIENTRY * qglRasterPos2dv) (const GLdouble * v);
extern void     (APIENTRY * qglRasterPos2f) (GLfloat x, GLfloat y);
extern void     (APIENTRY * qglRasterPos2fv) (const GLfloat * v);
extern void     (APIENTRY * qglRasterPos2i) (GLint x, GLint y);
extern void     (APIENTRY * qglRasterPos2iv) (const GLint * v);
extern void     (APIENTRY * qglRasterPos2s) (GLshort x, GLshort y);
extern void     (APIENTRY * qglRasterPos2sv) (const GLshort * v);
extern void     (APIENTRY * qglRasterPos3d) (GLdouble x, GLdouble y, GLdouble z);
extern void     (APIENTRY * qglRasterPos3dv) (const GLdouble * v);
extern void     (APIENTRY * qglRasterPos3f) (GLfloat x, GLfloat y, GLfloat z);
extern void     (APIENTRY * qglRasterPos3fv) (const GLfloat * v);
extern void     (APIENTRY * qglRasterPos3i) (GLint x, GLint y, GLint z);
extern void     (APIENTRY * qglRasterPos3iv) (const GLint * v);
extern void     (APIENTRY * qglRasterPos3s) (GLshort x, GLshort y, GLshort z);
extern void     (APIENTRY * qglRasterPos3sv) (const GLshort * v);
extern void     (APIENTRY * qglRasterPos4d) (GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern void     (APIENTRY * qglRasterPos4dv) (const GLdouble * v);
extern void     (APIENTRY * qglRasterPos4f) (GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void     (APIENTRY * qglRasterPos4fv) (const GLfloat * v);
extern void     (APIENTRY * qglRasterPos4i) (GLint x, GLint y, GLint z, GLint w);
extern void     (APIENTRY * qglRasterPos4iv) (const GLint * v);
extern void     (APIENTRY * qglRasterPos4s) (GLshort x, GLshort y, GLshort z, GLshort w);
extern void     (APIENTRY * qglRasterPos4sv) (const GLshort * v);
extern void     (APIENTRY * qglReadBuffer) (GLenum mode);
extern void     (APIENTRY * qglReadPixels) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type,
											GLvoid * pixels);
extern void     (APIENTRY * qglRectd) (GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
extern void     (APIENTRY * qglRectdv) (const GLdouble * v1, const GLdouble * v2);
extern void     (APIENTRY * qglRectf) (GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
extern void     (APIENTRY * qglRectfv) (const GLfloat * v1, const GLfloat * v2);
extern void     (APIENTRY * qglRecti) (GLint x1, GLint y1, GLint x2, GLint y2);
extern void     (APIENTRY * qglRectiv) (const GLint * v1, const GLint * v2);
extern void     (APIENTRY * qglRects) (GLshort x1, GLshort y1, GLshort x2, GLshort y2);
extern void     (APIENTRY * qglRectsv) (const GLshort * v1, const GLshort * v2);
extern          GLint(APIENTRY * qglRenderMode) (GLenum mode);
extern void     (APIENTRY * qglRotated) (GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
extern void     (APIENTRY * qglRotatef) (GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
extern void     (APIENTRY * qglScaled) (GLdouble x, GLdouble y, GLdouble z);
extern void     (APIENTRY * qglScalef) (GLfloat x, GLfloat y, GLfloat z);
extern void     (APIENTRY * qglScissor) (GLint x, GLint y, GLsizei width, GLsizei height);
extern void     (APIENTRY * qglSelectBuffer) (GLsizei size, GLuint * buffer);
extern void     (APIENTRY * qglShadeModel) (GLenum mode);
extern void     (APIENTRY * qglStencilFunc) (GLenum func, GLint ref, GLuint mask);
extern void     (APIENTRY * qglStencilMask) (GLuint mask);
extern void     (APIENTRY * qglStencilOp) (GLenum fail, GLenum zfail, GLenum zpass);
extern void     (APIENTRY * qglTexCoord1d) (GLdouble s);
extern void     (APIENTRY * qglTexCoord1dv) (const GLdouble * v);
extern void     (APIENTRY * qglTexCoord1f) (GLfloat s);
extern void     (APIENTRY * qglTexCoord1fv) (const GLfloat * v);
extern void     (APIENTRY * qglTexCoord1i) (GLint s);
extern void     (APIENTRY * qglTexCoord1iv) (const GLint * v);
extern void     (APIENTRY * qglTexCoord1s) (GLshort s);
extern void     (APIENTRY * qglTexCoord1sv) (const GLshort * v);
extern void     (APIENTRY * qglTexCoord2d) (GLdouble s, GLdouble t);
extern void     (APIENTRY * qglTexCoord2dv) (const GLdouble * v);
extern void     (APIENTRY * qglTexCoord2f) (GLfloat s, GLfloat t);
extern void     (APIENTRY * qglTexCoord2fv) (const GLfloat * v);
extern void     (APIENTRY * qglTexCoord2i) (GLint s, GLint t);
extern void     (APIENTRY * qglTexCoord2iv) (const GLint * v);
extern void     (APIENTRY * qglTexCoord2s) (GLshort s, GLshort t);
extern void     (APIENTRY * qglTexCoord2sv) (const GLshort * v);
extern void     (APIENTRY * qglTexCoord3d) (GLdouble s, GLdouble t, GLdouble r);
extern void     (APIENTRY * qglTexCoord3dv) (const GLdouble * v);
extern void     (APIENTRY * qglTexCoord3f) (GLfloat s, GLfloat t, GLfloat r);
extern void     (APIENTRY * qglTexCoord3fv) (const GLfloat * v);
extern void     (APIENTRY * qglTexCoord3i) (GLint s, GLint t, GLint r);
extern void     (APIENTRY * qglTexCoord3iv) (const GLint * v);
extern void     (APIENTRY * qglTexCoord3s) (GLshort s, GLshort t, GLshort r);
extern void     (APIENTRY * qglTexCoord3sv) (const GLshort * v);
extern void     (APIENTRY * qglTexCoord4d) (GLdouble s, GLdouble t, GLdouble r, GLdouble q);
extern void     (APIENTRY * qglTexCoord4dv) (const GLdouble * v);
extern void     (APIENTRY * qglTexCoord4f) (GLfloat s, GLfloat t, GLfloat r, GLfloat q);
extern void     (APIENTRY * qglTexCoord4fv) (const GLfloat * v);
extern void     (APIENTRY * qglTexCoord4i) (GLint s, GLint t, GLint r, GLint q);
extern void     (APIENTRY * qglTexCoord4iv) (const GLint * v);
extern void     (APIENTRY * qglTexCoord4s) (GLshort s, GLshort t, GLshort r, GLshort q);
extern void     (APIENTRY * qglTexCoord4sv) (const GLshort * v);
extern void     (APIENTRY * qglTexCoordPointer) (GLint size, GLenum type, GLsizei stride, const GLvoid * pointer);
extern void     (APIENTRY * qglTexEnvf) (GLenum target, GLenum pname, GLfloat param);
extern void     (APIENTRY * qglTexEnvfv) (GLenum target, GLenum pname, const GLfloat * params);
extern void     (APIENTRY * qglTexEnvi) (GLenum target, GLenum pname, GLint param);
extern void     (APIENTRY * qglTexEnviv) (GLenum target, GLenum pname, const GLint * params);
extern void     (APIENTRY * qglTexGend) (GLenum coord, GLenum pname, GLdouble param);
extern void     (APIENTRY * qglTexGendv) (GLenum coord, GLenum pname, const GLdouble * params);
extern void     (APIENTRY * qglTexGenf) (GLenum coord, GLenum pname, GLfloat param);
extern void     (APIENTRY * qglTexGenfv) (GLenum coord, GLenum pname, const GLfloat * params);
extern void     (APIENTRY * qglTexGeni) (GLenum coord, GLenum pname, GLint param);
extern void     (APIENTRY * qglTexGeniv) (GLenum coord, GLenum pname, const GLint * params);
extern void     (APIENTRY * qglTexImage1D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border,
											GLenum format, GLenum type, const GLvoid * pixels);
extern void     (APIENTRY * qglTexImage2D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height,
											GLint border, GLenum format, GLenum type, const GLvoid * pixels);
extern void     (APIENTRY * qglTexParameterf) (GLenum target, GLenum pname, GLfloat param);
extern void     (APIENTRY * qglTexParameterfv) (GLenum target, GLenum pname, const GLfloat * params);
extern void     (APIENTRY * qglTexParameteri) (GLenum target, GLenum pname, GLint param);
extern void     (APIENTRY * qglTexParameteriv) (GLenum target, GLenum pname, const GLint * params);
extern void     (APIENTRY * qglTexSubImage1D) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format,
											   GLenum type, const GLvoid * pixels);
extern void     (APIENTRY * qglTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width,
											   GLsizei height, GLenum format, GLenum type, const GLvoid * pixels);
extern void     (APIENTRY * qglTranslated) (GLdouble x, GLdouble y, GLdouble z);
extern void     (APIENTRY * qglTranslatef) (GLfloat x, GLfloat y, GLfloat z);
extern void     (APIENTRY * qglVertex2d) (GLdouble x, GLdouble y);
extern void     (APIENTRY * qglVertex2dv) (const GLdouble * v);
extern void     (APIENTRY * qglVertex2f) (GLfloat x, GLfloat y);
extern void     (APIENTRY * qglVertex2fv) (const GLfloat * v);
extern void     (APIENTRY * qglVertex2i) (GLint x, GLint y);
extern void     (APIENTRY * qglVertex2iv) (const GLint * v);
extern void     (APIENTRY * qglVertex2s) (GLshort x, GLshort y);
extern void     (APIENTRY * qglVertex2sv) (const GLshort * v);
extern void     (APIENTRY * qglVertex3d) (GLdouble x, GLdouble y, GLdouble z);
extern void     (APIENTRY * qglVertex3dv) (const GLdouble * v);
extern void     (APIENTRY * qglVertex3f) (GLfloat x, GLfloat y, GLfloat z);
extern void     (APIENTRY * qglVertex3fv) (const GLfloat * v);
extern void     (APIENTRY * qglVertex3i) (GLint x, GLint y, GLint z);
extern void     (APIENTRY * qglVertex3iv) (const GLint * v);
extern void     (APIENTRY * qglVertex3s) (GLshort x, GLshort y, GLshort z);
extern void     (APIENTRY * qglVertex3sv) (const GLshort * v);
extern void     (APIENTRY * qglVertex4d) (GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern void     (APIENTRY * qglVertex4dv) (const GLdouble * v);
extern void     (APIENTRY * qglVertex4f) (GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void     (APIENTRY * qglVertex4fv) (const GLfloat * v);
extern void     (APIENTRY * qglVertex4i) (GLint x, GLint y, GLint z, GLint w);
extern void     (APIENTRY * qglVertex4iv) (const GLint * v);
extern void     (APIENTRY * qglVertex4s) (GLshort x, GLshort y, GLshort z, GLshort w);
extern void     (APIENTRY * qglVertex4sv) (const GLshort * v);
extern void     (APIENTRY * qglVertexPointer) (GLint size, GLenum type, GLsizei stride, const GLvoid * pointer);
extern void     (APIENTRY * qglViewport) (GLint x, GLint y, GLsizei width, GLsizei height);

#if defined( _WIN32 )

extern int      (WINAPI * qwglChoosePixelFormat) (HDC, CONST PIXELFORMATDESCRIPTOR *);
extern int      (WINAPI * qwglDescribePixelFormat) (HDC, int, UINT, LPPIXELFORMATDESCRIPTOR);
extern int      (WINAPI * qwglGetPixelFormat) (HDC);
extern          BOOL(WINAPI * qwglSetPixelFormat) (HDC, int, CONST PIXELFORMATDESCRIPTOR *);
extern          BOOL(WINAPI * qwglSwapBuffers) (HDC);

extern          BOOL(WINAPI * qwglGetDeviceGammaRamp3DFX) (HDC, LPVOID);
extern          BOOL(WINAPI * qwglSetDeviceGammaRamp3DFX) (HDC, LPVOID);

extern          BOOL(WINAPI * qwglCopyContext) (HGLRC, HGLRC, UINT);
extern          HGLRC(WINAPI * qwglCreateContext) (HDC);
extern          HGLRC(WINAPI * qwglCreateLayerContext) (HDC, int);
extern          BOOL(WINAPI * qwglDeleteContext) (HGLRC);
extern          HGLRC(WINAPI * qwglGetCurrentContext) (VOID);
extern          HDC(WINAPI * qwglGetCurrentDC) (VOID);
extern          PROC(WINAPI * qwglGetProcAddress) (LPCSTR);
extern          BOOL(WINAPI * qwglMakeCurrent) (HDC, HGLRC);
extern          BOOL(WINAPI * qwglShareLists) (HGLRC, HGLRC);
extern          BOOL(WINAPI * qwglUseFontBitmaps) (HDC, DWORD, DWORD, DWORD);

extern          BOOL(WINAPI * qwglUseFontOutlines) (HDC, DWORD, DWORD, DWORD, FLOAT, FLOAT, int, LPGLYPHMETRICSFLOAT);

extern          BOOL(WINAPI * qwglDescribeLayerPlane) (HDC, int, int, UINT, LPLAYERPLANEDESCRIPTOR);
extern int      (WINAPI * qwglSetLayerPaletteEntries) (HDC, int, int, int, CONST COLORREF *);
extern int      (WINAPI * qwglGetLayerPaletteEntries) (HDC, int, int, int, COLORREF *);
extern          BOOL(WINAPI * qwglRealizeLayerPalette) (HDC, int, BOOL);
extern          BOOL(WINAPI * qwglSwapLayerBuffers) (HDC, UINT);

extern          BOOL(WINAPI * qwglSwapIntervalEXT) (int interval);

#endif							// _WIN32

#if ( (defined __linux__ )  || (defined __FreeBSD__ ) )	// rb010123

//FX Mesa Functions
// bk001129 - from cvs1.17 (mkv)
#if defined (__FX__)
extern          fxMesaContext(*qfxMesaCreateContext) (GLuint win, GrScreenResolution_t, GrScreenRefresh_t,
													  const GLint attribList[]);
extern          fxMesaContext(*qfxMesaCreateBestContext) (GLuint win, GLint width, GLint height, const GLint attribList[]);
extern void     (*qfxMesaDestroyContext) (fxMesaContext ctx);
extern void     (*qfxMesaMakeCurrent) (fxMesaContext ctx);
extern          fxMesaContext(*qfxMesaGetCurrentContext) (void);
extern void     (*qfxMesaSwapBuffers) (void);
#endif

//GLX Functions
extern XVisualInfo *(*qglXChooseVisual) (Display * dpy, int screen, int *attribList);
extern          GLXContext(*qglXCreateContext) (Display * dpy, XVisualInfo * vis, GLXContext shareList, Bool direct);
extern void     (*qglXDestroyContext) (Display * dpy, GLXContext ctx);
extern          Bool(*qglXMakeCurrent) (Display * dpy, GLXDrawable drawable, GLXContext ctx);
extern void     (*qglXCopyContext) (Display * dpy, GLXContext src, GLXContext dst, GLuint mask);
extern void     (*qglXSwapBuffers) (Display * dpy, GLXDrawable drawable);

#endif							// __linux__ || __FreeBSD__ // rb010123

#endif							// _WIN32 && __linux__

#endif							// __QGL_H__
