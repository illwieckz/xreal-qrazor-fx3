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
#ifndef R_GL_H
#define R_GL_H

/// includes ===================================================================
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
// system -------------------------------------------------------------------
#include <stddef.h>

// qrazor-fx ----------------------------------------------------------------
#ifdef __linux__
#include "glimp_glx.h"
#endif

#ifdef _WIN32
#include "glimp_wgl.h"
#endif

#ifdef GL_H
# error Do not manually include gl.h or glu.h
#endif

#ifdef __GL_H__
# error Do not manually include gl.h or glu.h
#endif

#ifdef __gl_h__
# error Do not manually include gl.h or glu.h
#endif

#ifdef __gl_h_
# error Do not manually include gl.h or glu.h
#endif


#ifdef APIENTRY
# define GLAPIENTRY APIENTRY
#else
# define GLAPIENTRY
#endif



/// BASIC OPENGL TYPES =========================================================
typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef signed char GLbyte;
typedef short GLshort;
typedef int GLint;
typedef int GLsizei;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef unsigned int GLuint;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef double GLclampd;
typedef void GLvoid;




/// ENUMERATIONS ===============================================================
/* Version */
#define GL_VERSION_1_1                    1
//#define GL_VERSION_1_2                    1
//#define GL_VERSION_1_3                    1
//#define GL_VERSION_1_4                    1

/* AttribMask */
#define GL_CURRENT_BIT                    0x00000001
#define GL_POINT_BIT                      0x00000002
#define GL_LINE_BIT                       0x00000004
#define GL_POLYGON_BIT                    0x00000008
#define GL_POLYGON_STIPPLE_BIT            0x00000010
#define GL_PIXEL_MODE_BIT                 0x00000020
#define GL_LIGHTING_BIT                   0x00000040
#define GL_FOG_BIT                        0x00000080
#define GL_DEPTH_BUFFER_BIT               0x00000100
#define GL_ACCUM_BUFFER_BIT               0x00000200
#define GL_STENCIL_BUFFER_BIT             0x00000400
#define GL_VIEWPORT_BIT                   0x00000800
#define GL_TRANSFORM_BIT                  0x00001000
#define GL_ENABLE_BIT                     0x00002000
#define GL_COLOR_BUFFER_BIT               0x00004000
#define GL_HINT_BIT                       0x00008000
#define GL_EVAL_BIT                       0x00010000
#define GL_LIST_BIT                       0x00020000
#define GL_TEXTURE_BIT                    0x00040000
#define GL_SCISSOR_BIT                    0x00080000
#define GL_ALL_ATTRIB_BITS                0xFFFFFFFF

/* ClearBufferMask */
/*      GL_COLOR_BUFFER_BIT */
/*      GL_ACCUM_BUFFER_BIT */
/*      GL_STENCIL_BUFFER_BIT */
/*      GL_DEPTH_BUFFER_BIT */

/* ClientAttribMask */
#define GL_CLIENT_PIXEL_STORE_BIT         0x00000001
#define GL_CLIENT_VERTEX_ARRAY_BIT        0x00000002
#define GL_CLIENT_ALL_ATTRIB_BITS         0xFFFFFFFF

/* Boolean */
#define GL_FALSE                          0
#define GL_TRUE                           1

/* BeginMode */
#define GL_POINTS                         0x0000
#define GL_LINES                          0x0001
#define GL_LINE_LOOP                      0x0002
#define GL_LINE_STRIP                     0x0003
#define GL_TRIANGLES                      0x0004
#define GL_TRIANGLE_STRIP                 0x0005
#define GL_TRIANGLE_FAN                   0x0006
#define GL_QUADS                          0x0007
#define GL_QUAD_STRIP                     0x0008
#define GL_POLYGON                        0x0009

/* AccumOp */
#define GL_ACCUM                          0x0100
#define GL_LOAD                           0x0101
#define GL_RETURN                         0x0102
#define GL_MULT                           0x0103
#define GL_ADD                            0x0104

/* AlphaFunction */
#define GL_NEVER                          0x0200
#define GL_LESS                           0x0201
#define GL_EQUAL                          0x0202
#define GL_LEQUAL                         0x0203
#define GL_GREATER                        0x0204
#define GL_NOTEQUAL                       0x0205
#define GL_GEQUAL                         0x0206
#define GL_ALWAYS                         0x0207

/* BlendingFactorDest */
#define GL_ZERO                           0
#define GL_ONE                            1
#define GL_SRC_COLOR                      0x0300
#define GL_ONE_MINUS_SRC_COLOR            0x0301
#define GL_SRC_ALPHA                      0x0302
#define GL_ONE_MINUS_SRC_ALPHA            0x0303
#define GL_DST_ALPHA                      0x0304
#define GL_ONE_MINUS_DST_ALPHA            0x0305

/* BlendingFactorSrc */
/*      GL_ZERO */
/*      GL_ONE */
#define GL_DST_COLOR                      0x0306
#define GL_ONE_MINUS_DST_COLOR            0x0307
#define GL_SRC_ALPHA_SATURATE             0x0308
/*      GL_SRC_ALPHA */
/*      GL_ONE_MINUS_SRC_ALPHA */
/*      GL_DST_ALPHA */
/*      GL_ONE_MINUS_DST_ALPHA */

/* ColorMaterialFace */
/*      GL_FRONT */
/*      GL_BACK */
/*      GL_FRONT_AND_BACK */

/* ColorMaterialParameter */
/*      GL_AMBIENT */
/*      GL_DIFFUSE */
/*      GL_SPECULAR */
/*      GL_EMISSION */
/*      GL_AMBIENT_AND_DIFFUSE */

/* ColorPointerType */
/*      GL_BYTE */
/*      GL_UNSIGNED_BYTE */
/*      GL_SHORT */
/*      GL_UNSIGNED_SHORT */
/*      GL_INT */
/*      GL_UNSIGNED_INT */
/*      GL_FLOAT */
/*      GL_DOUBLE */

/* CullFaceMode */
/*      GL_FRONT */
/*      GL_BACK */
/*      GL_FRONT_AND_BACK */

/* DepthFunction */
/*      GL_NEVER */
/*      GL_LESS */
/*      GL_EQUAL */
/*      GL_LEQUAL */
/*      GL_GREATER */
/*      GL_NOTEQUAL */
/*      GL_GEQUAL */
/*      GL_ALWAYS */

/* DrawBufferMode */
#define GL_NONE                           0
#define GL_FRONT_LEFT                     0x0400
#define GL_FRONT_RIGHT                    0x0401
#define GL_BACK_LEFT                      0x0402
#define GL_BACK_RIGHT                     0x0403
#define GL_FRONT                          0x0404
#define GL_BACK                           0x0405
#define GL_LEFT                           0x0406
#define GL_RIGHT                          0x0407
#define GL_FRONT_AND_BACK                 0x0408
#define GL_AUX0                           0x0409
#define GL_AUX1                           0x040A
#define GL_AUX2                           0x040B
#define GL_AUX3                           0x040C

/* EnableCap */
/*      GL_FOG */
/*      GL_LIGHTING */
/*      GL_TEXTURE_1D */
/*      GL_TEXTURE_2D */
/*      GL_LINE_STIPPLE */
/*      GL_POLYGON_STIPPLE */
/*      GL_CULL_FACE */
/*      GL_ALPHA_TEST */
/*      GL_BLEND */
/*      GL_INDEX_LOGIC_OP */
/*      GL_COLOR_LOGIC_OP */
/*      GL_DITHER */
/*      GL_STENCIL_TEST */
/*      GL_DEPTH_TEST */
/*      GL_CLIP_PLANE0 */
/*      GL_CLIP_PLANE1 */
/*      GL_CLIP_PLANE2 */
/*      GL_CLIP_PLANE3 */
/*      GL_CLIP_PLANE4 */
/*      GL_CLIP_PLANE5 */
/*      GL_LIGHT0 */
/*      GL_LIGHT1 */
/*      GL_LIGHT2 */
/*      GL_LIGHT3 */
/*      GL_LIGHT4 */
/*      GL_LIGHT5 */
/*      GL_LIGHT6 */
/*      GL_LIGHT7 */
/*      GL_TEXTURE_GEN_S */
/*      GL_TEXTURE_GEN_T */
/*      GL_TEXTURE_GEN_R */
/*      GL_TEXTURE_GEN_Q */
/*      GL_MAP1_VERTEX_3 */
/*      GL_MAP1_VERTEX_4 */
/*      GL_MAP1_COLOR_4 */
/*      GL_MAP1_INDEX */
/*      GL_MAP1_NORMAL */
/*      GL_MAP1_TEXTURE_COORD_1 */
/*      GL_MAP1_TEXTURE_COORD_2 */
/*      GL_MAP1_TEXTURE_COORD_3 */
/*      GL_MAP1_TEXTURE_COORD_4 */
/*      GL_MAP2_VERTEX_3 */
/*      GL_MAP2_VERTEX_4 */
/*      GL_MAP2_COLOR_4 */
/*      GL_MAP2_INDEX */
/*      GL_MAP2_NORMAL */
/*      GL_MAP2_TEXTURE_COORD_1 */
/*      GL_MAP2_TEXTURE_COORD_2 */
/*      GL_MAP2_TEXTURE_COORD_3 */
/*      GL_MAP2_TEXTURE_COORD_4 */
/*      GL_POINT_SMOOTH */
/*      GL_LINE_SMOOTH */
/*      GL_POLYGON_SMOOTH */
/*      GL_SCISSOR_TEST */
/*      GL_COLOR_MATERIAL */
/*      GL_NORMALIZE */
/*      GL_AUTO_NORMAL */
/*      GL_POLYGON_OFFSET_POINT */
/*      GL_POLYGON_OFFSET_LINE */
/*      GL_POLYGON_OFFSET_FILL */
/*      GL_VERTEX_ARRAY */
/*      GL_NORMAL_ARRAY */
/*      GL_COLOR_ARRAY */
/*      GL_INDEX_ARRAY */
/*      GL_TEXTURE_COORD_ARRAY */
/*      GL_EDGE_FLAG_ARRAY */

/* ErrorCode */
#define GL_NO_ERROR                       0
#define GL_INVALID_ENUM                   0x0500
#define GL_INVALID_VALUE                  0x0501
#define GL_INVALID_OPERATION              0x0502
#define GL_STACK_OVERFLOW                 0x0503
#define GL_STACK_UNDERFLOW                0x0504
#define GL_OUT_OF_MEMORY                  0x0505
#define GL_TABLE_TOO_LARGE                0x8031

/* FeedbackType */
#define GL_2D                             0x0600
#define GL_3D                             0x0601
#define GL_3D_COLOR                       0x0602
#define GL_3D_COLOR_TEXTURE               0x0603
#define GL_4D_COLOR_TEXTURE               0x0604

/* FeedBackToken */
#define GL_PASS_THROUGH_TOKEN             0x0700
#define GL_POINT_TOKEN                    0x0701
#define GL_LINE_TOKEN                     0x0702
#define GL_POLYGON_TOKEN                  0x0703
#define GL_BITMAP_TOKEN                   0x0704
#define GL_DRAW_PIXEL_TOKEN               0x0705
#define GL_COPY_PIXEL_TOKEN               0x0706
#define GL_LINE_RESET_TOKEN               0x0707

/* FogMode */
/*      GL_LINEAR */
#define GL_EXP                            0x0800
#define GL_EXP2                           0x0801

/* FogParameter */
/*      GL_FOG_COLOR */
/*      GL_FOG_DENSITY */
/*      GL_FOG_END */
/*      GL_FOG_INDEX */
/*      GL_FOG_MODE */
/*      GL_FOG_START */

/* FrontFaceDirection */
#define GL_CW                             0x0900
#define GL_CCW                            0x0901

/* GetMapQuery */
#define GL_COEFF                          0x0A00
#define GL_ORDER                          0x0A01
#define GL_DOMAIN                         0x0A02

/* GetPixelMap */
#define GL_PIXEL_MAP_I_TO_I               0x0C70
#define GL_PIXEL_MAP_S_TO_S               0x0C71
#define GL_PIXEL_MAP_I_TO_R               0x0C72
#define GL_PIXEL_MAP_I_TO_G               0x0C73
#define GL_PIXEL_MAP_I_TO_B               0x0C74
#define GL_PIXEL_MAP_I_TO_A               0x0C75
#define GL_PIXEL_MAP_R_TO_R               0x0C76
#define GL_PIXEL_MAP_G_TO_G               0x0C77
#define GL_PIXEL_MAP_B_TO_B               0x0C78
#define GL_PIXEL_MAP_A_TO_A               0x0C79

/* GetPointervPName */
#define GL_VERTEX_ARRAY_POINTER           0x808E
#define GL_NORMAL_ARRAY_POINTER           0x808F
#define GL_COLOR_ARRAY_POINTER            0x8090
#define GL_INDEX_ARRAY_POINTER            0x8091
#define GL_TEXTURE_COORD_ARRAY_POINTER    0x8092
#define GL_EDGE_FLAG_ARRAY_POINTER        0x8093

/* GetPName */
#define GL_CURRENT_COLOR                  0x0B00
#define GL_CURRENT_INDEX                  0x0B01
#define GL_CURRENT_NORMAL                 0x0B02
#define GL_CURRENT_TEXTURE_COORDS         0x0B03
#define GL_CURRENT_RASTER_COLOR           0x0B04
#define GL_CURRENT_RASTER_INDEX           0x0B05
#define GL_CURRENT_RASTER_TEXTURE_COORDS  0x0B06
#define GL_CURRENT_RASTER_POSITION        0x0B07
#define GL_CURRENT_RASTER_POSITION_VALID  0x0B08
#define GL_CURRENT_RASTER_DISTANCE        0x0B09
#define GL_POINT_SMOOTH                   0x0B10
#define GL_POINT_SIZE                     0x0B11
#define GL_SMOOTH_POINT_SIZE_RANGE        0x0B12
#define GL_SMOOTH_POINT_SIZE_GRANULARITY  0x0B13
#define GL_POINT_SIZE_RANGE               GL_SMOOTH_POINT_SIZE_RANGE
#define GL_POINT_SIZE_GRANULARITY         GL_SMOOTH_POINT_SIZE_GRANULARITY
#define GL_LINE_SMOOTH                    0x0B20
#define GL_LINE_WIDTH                     0x0B21
#define GL_SMOOTH_LINE_WIDTH_RANGE        0x0B22
#define GL_SMOOTH_LINE_WIDTH_GRANULARITY  0x0B23
#define GL_LINE_WIDTH_RANGE               GL_SMOOTH_LINE_WIDTH_RANGE
#define GL_LINE_WIDTH_GRANULARITY         GL_SMOOTH_LINE_WIDTH_GRANULARITY
#define GL_LINE_STIPPLE                   0x0B24
#define GL_LINE_STIPPLE_PATTERN           0x0B25
#define GL_LINE_STIPPLE_REPEAT            0x0B26
#define GL_LIST_MODE                      0x0B30
#define GL_MAX_LIST_NESTING               0x0B31
#define GL_LIST_BASE                      0x0B32
#define GL_LIST_INDEX                     0x0B33
#define GL_POLYGON_MODE                   0x0B40
#define GL_POLYGON_SMOOTH                 0x0B41
#define GL_POLYGON_STIPPLE                0x0B42
#define GL_EDGE_FLAG                      0x0B43
#define GL_CULL_FACE                      0x0B44
#define GL_CULL_FACE_MODE                 0x0B45
#define GL_FRONT_FACE                     0x0B46
#define GL_LIGHTING                       0x0B50
#define GL_LIGHT_MODEL_LOCAL_VIEWER       0x0B51
#define GL_LIGHT_MODEL_TWO_SIDE           0x0B52
#define GL_LIGHT_MODEL_AMBIENT            0x0B53
#define GL_SHADE_MODEL                    0x0B54
#define GL_COLOR_MATERIAL_FACE            0x0B55
#define GL_COLOR_MATERIAL_PARAMETER       0x0B56
#define GL_COLOR_MATERIAL                 0x0B57
#define GL_FOG                            0x0B60
#define GL_FOG_INDEX                      0x0B61
#define GL_FOG_DENSITY                    0x0B62
#define GL_FOG_START                      0x0B63
#define GL_FOG_END                        0x0B64
#define GL_FOG_MODE                       0x0B65
#define GL_FOG_COLOR                      0x0B66
#define GL_DEPTH_RANGE                    0x0B70
#define GL_DEPTH_TEST                     0x0B71
#define GL_DEPTH_WRITEMASK                0x0B72
#define GL_DEPTH_CLEAR_VALUE              0x0B73
#define GL_DEPTH_FUNC                     0x0B74
#define GL_ACCUM_CLEAR_VALUE              0x0B80
#define GL_STENCIL_TEST                   0x0B90
#define GL_STENCIL_CLEAR_VALUE            0x0B91
#define GL_STENCIL_FUNC                   0x0B92
#define GL_STENCIL_VALUE_MASK             0x0B93
#define GL_STENCIL_FAIL                   0x0B94
#define GL_STENCIL_PASS_DEPTH_FAIL        0x0B95
#define GL_STENCIL_PASS_DEPTH_PASS        0x0B96
#define GL_STENCIL_REF                    0x0B97
#define GL_STENCIL_WRITEMASK              0x0B98
#define GL_MATRIX_MODE                    0x0BA0
#define GL_NORMALIZE                      0x0BA1
#define GL_VIEWPORT                       0x0BA2
#define GL_MODELVIEW_STACK_DEPTH          0x0BA3
#define GL_PROJECTION_STACK_DEPTH         0x0BA4
#define GL_TEXTURE_STACK_DEPTH            0x0BA5
#define GL_MODELVIEW_MATRIX               0x0BA6
#define GL_PROJECTION_MATRIX              0x0BA7
#define GL_TEXTURE_MATRIX                 0x0BA8
#define GL_ATTRIB_STACK_DEPTH             0x0BB0
#define GL_CLIENT_ATTRIB_STACK_DEPTH      0x0BB1
#define GL_ALPHA_TEST                     0x0BC0
#define GL_ALPHA_TEST_FUNC                0x0BC1
#define GL_ALPHA_TEST_REF                 0x0BC2
#define GL_DITHER                         0x0BD0
#define GL_BLEND_DST                      0x0BE0
#define GL_BLEND_SRC                      0x0BE1
#define GL_BLEND                          0x0BE2
#define GL_LOGIC_OP_MODE                  0x0BF0
#define GL_INDEX_LOGIC_OP                 0x0BF1
#define GL_LOGIC_OP                       GL_INDEX_LOGIC_OP
#define GL_COLOR_LOGIC_OP                 0x0BF2
#define GL_AUX_BUFFERS                    0x0C00
#define GL_DRAW_BUFFER                    0x0C01
#define GL_READ_BUFFER                    0x0C02
#define GL_SCISSOR_BOX                    0x0C10
#define GL_SCISSOR_TEST                   0x0C11
#define GL_INDEX_CLEAR_VALUE              0x0C20
#define GL_INDEX_WRITEMASK                0x0C21
#define GL_COLOR_CLEAR_VALUE              0x0C22
#define GL_COLOR_WRITEMASK                0x0C23
#define GL_INDEX_MODE                     0x0C30
#define GL_RGBA_MODE                      0x0C31
#define GL_DOUBLEBUFFER                   0x0C32
#define GL_STEREO                         0x0C33
#define GL_RENDER_MODE                    0x0C40
#define GL_PERSPECTIVE_CORRECTION_HINT    0x0C50
#define GL_POINT_SMOOTH_HINT              0x0C51
#define GL_LINE_SMOOTH_HINT               0x0C52
#define GL_POLYGON_SMOOTH_HINT            0x0C53
#define GL_FOG_HINT                       0x0C54
#define GL_TEXTURE_GEN_S                  0x0C60
#define GL_TEXTURE_GEN_T                  0x0C61
#define GL_TEXTURE_GEN_R                  0x0C62
#define GL_TEXTURE_GEN_Q                  0x0C63
#define GL_PIXEL_MAP_I_TO_I_SIZE          0x0CB0
#define GL_PIXEL_MAP_S_TO_S_SIZE          0x0CB1
#define GL_PIXEL_MAP_I_TO_R_SIZE          0x0CB2
#define GL_PIXEL_MAP_I_TO_G_SIZE          0x0CB3
#define GL_PIXEL_MAP_I_TO_B_SIZE          0x0CB4
#define GL_PIXEL_MAP_I_TO_A_SIZE          0x0CB5
#define GL_PIXEL_MAP_R_TO_R_SIZE          0x0CB6
#define GL_PIXEL_MAP_G_TO_G_SIZE          0x0CB7
#define GL_PIXEL_MAP_B_TO_B_SIZE          0x0CB8
#define GL_PIXEL_MAP_A_TO_A_SIZE          0x0CB9
#define GL_UNPACK_SWAP_BYTES              0x0CF0
#define GL_UNPACK_LSB_FIRST               0x0CF1
#define GL_UNPACK_ROW_LENGTH              0x0CF2
#define GL_UNPACK_SKIP_ROWS               0x0CF3
#define GL_UNPACK_SKIP_PIXELS             0x0CF4
#define GL_UNPACK_ALIGNMENT               0x0CF5
#define GL_PACK_SWAP_BYTES                0x0D00
#define GL_PACK_LSB_FIRST                 0x0D01
#define GL_PACK_ROW_LENGTH                0x0D02
#define GL_PACK_SKIP_ROWS                 0x0D03
#define GL_PACK_SKIP_PIXELS               0x0D04
#define GL_PACK_ALIGNMENT                 0x0D05
#define GL_MAP_COLOR                      0x0D10
#define GL_MAP_STENCIL                    0x0D11
#define GL_INDEX_SHIFT                    0x0D12
#define GL_INDEX_OFFSET                   0x0D13
#define GL_RED_SCALE                      0x0D14
#define GL_RED_BIAS                       0x0D15
#define GL_ZOOM_X                         0x0D16
#define GL_ZOOM_Y                         0x0D17
#define GL_GREEN_SCALE                    0x0D18
#define GL_GREEN_BIAS                     0x0D19
#define GL_BLUE_SCALE                     0x0D1A
#define GL_BLUE_BIAS                      0x0D1B
#define GL_ALPHA_SCALE                    0x0D1C
#define GL_ALPHA_BIAS                     0x0D1D
#define GL_DEPTH_SCALE                    0x0D1E
#define GL_DEPTH_BIAS                     0x0D1F
#define GL_MAX_EVAL_ORDER                 0x0D30
#define GL_MAX_LIGHTS                     0x0D31
#define GL_MAX_CLIP_PLANES                0x0D32
#define GL_MAX_TEXTURE_SIZE               0x0D33
#define GL_MAX_PIXEL_MAP_TABLE            0x0D34
#define GL_MAX_ATTRIB_STACK_DEPTH         0x0D35
#define GL_MAX_MODELVIEW_STACK_DEPTH      0x0D36
#define GL_MAX_NAME_STACK_DEPTH           0x0D37
#define GL_MAX_PROJECTION_STACK_DEPTH     0x0D38
#define GL_MAX_TEXTURE_STACK_DEPTH        0x0D39
#define GL_MAX_VIEWPORT_DIMS              0x0D3A
#define GL_MAX_CLIENT_ATTRIB_STACK_DEPTH  0x0D3B
#define GL_SUBPIXEL_BITS                  0x0D50
#define GL_INDEX_BITS                     0x0D51
#define GL_RED_BITS                       0x0D52
#define GL_GREEN_BITS                     0x0D53
#define GL_BLUE_BITS                      0x0D54
#define GL_ALPHA_BITS                     0x0D55
#define GL_DEPTH_BITS                     0x0D56
#define GL_STENCIL_BITS                   0x0D57
#define GL_ACCUM_RED_BITS                 0x0D58
#define GL_ACCUM_GREEN_BITS               0x0D59
#define GL_ACCUM_BLUE_BITS                0x0D5A
#define GL_ACCUM_ALPHA_BITS               0x0D5B
#define GL_NAME_STACK_DEPTH               0x0D70
#define GL_AUTO_NORMAL                    0x0D80
#define GL_MAP1_COLOR_4                   0x0D90
#define GL_MAP1_INDEX                     0x0D91
#define GL_MAP1_NORMAL                    0x0D92
#define GL_MAP1_TEXTURE_COORD_1           0x0D93
#define GL_MAP1_TEXTURE_COORD_2           0x0D94
#define GL_MAP1_TEXTURE_COORD_3           0x0D95
#define GL_MAP1_TEXTURE_COORD_4           0x0D96
#define GL_MAP1_VERTEX_3                  0x0D97
#define GL_MAP1_VERTEX_4                  0x0D98
#define GL_MAP2_COLOR_4                   0x0DB0
#define GL_MAP2_INDEX                     0x0DB1
#define GL_MAP2_NORMAL                    0x0DB2
#define GL_MAP2_TEXTURE_COORD_1           0x0DB3
#define GL_MAP2_TEXTURE_COORD_2           0x0DB4
#define GL_MAP2_TEXTURE_COORD_3           0x0DB5
#define GL_MAP2_TEXTURE_COORD_4           0x0DB6
#define GL_MAP2_VERTEX_3                  0x0DB7
#define GL_MAP2_VERTEX_4                  0x0DB8
#define GL_MAP1_GRID_DOMAIN               0x0DD0
#define GL_MAP1_GRID_SEGMENTS             0x0DD1
#define GL_MAP2_GRID_DOMAIN               0x0DD2
#define GL_MAP2_GRID_SEGMENTS             0x0DD3
#define GL_TEXTURE_1D                     0x0DE0
#define GL_TEXTURE_2D                     0x0DE1
#define GL_FEEDBACK_BUFFER_POINTER        0x0DF0
#define GL_FEEDBACK_BUFFER_SIZE           0x0DF1
#define GL_FEEDBACK_BUFFER_TYPE           0x0DF2
#define GL_SELECTION_BUFFER_POINTER       0x0DF3
#define GL_SELECTION_BUFFER_SIZE          0x0DF4
#define GL_POLYGON_OFFSET_UNITS           0x2A00
#define GL_POLYGON_OFFSET_POINT           0x2A01
#define GL_POLYGON_OFFSET_LINE            0x2A02
#define GL_POLYGON_OFFSET_FILL            0x8037
#define GL_POLYGON_OFFSET_FACTOR          0x8038
#define GL_TEXTURE_BINDING_1D             0x8068
#define GL_TEXTURE_BINDING_2D             0x8069
#define GL_TEXTURE_BINDING_3D             0x806A
#define GL_VERTEX_ARRAY                   0x8074
#define GL_NORMAL_ARRAY                   0x8075
#define GL_COLOR_ARRAY                    0x8076
#define GL_INDEX_ARRAY                    0x8077
#define GL_TEXTURE_COORD_ARRAY            0x8078
#define GL_EDGE_FLAG_ARRAY                0x8079
#define GL_VERTEX_ARRAY_SIZE              0x807A
#define GL_VERTEX_ARRAY_TYPE              0x807B
#define GL_VERTEX_ARRAY_STRIDE            0x807C
#define GL_NORMAL_ARRAY_TYPE              0x807E
#define GL_NORMAL_ARRAY_STRIDE            0x807F
#define GL_COLOR_ARRAY_SIZE               0x8081
#define GL_COLOR_ARRAY_TYPE               0x8082
#define GL_COLOR_ARRAY_STRIDE             0x8083
#define GL_INDEX_ARRAY_TYPE               0x8085
#define GL_INDEX_ARRAY_STRIDE             0x8086
#define GL_TEXTURE_COORD_ARRAY_SIZE       0x8088
#define GL_TEXTURE_COORD_ARRAY_TYPE       0x8089
#define GL_TEXTURE_COORD_ARRAY_STRIDE     0x808A
#define GL_EDGE_FLAG_ARRAY_STRIDE         0x808C
/*      GL_VERTEX_ARRAY_COUNT_EXT */
/*      GL_NORMAL_ARRAY_COUNT_EXT */
/*      GL_COLOR_ARRAY_COUNT_EXT */
/*      GL_INDEX_ARRAY_COUNT_EXT */
/*      GL_TEXTURE_COORD_ARRAY_COUNT_EXT */
/*      GL_EDGE_FLAG_ARRAY_COUNT_EXT */

/* GetTextureParameter */
/*      GL_TEXTURE_MAG_FILTER */
/*      GL_TEXTURE_MIN_FILTER */
/*      GL_TEXTURE_WRAP_S */
/*      GL_TEXTURE_WRAP_T */
#define GL_TEXTURE_WIDTH                  0x1000
#define GL_TEXTURE_HEIGHT                 0x1001
#define GL_TEXTURE_INTERNAL_FORMAT        0x1003
#define GL_TEXTURE_COMPONENTS             GL_TEXTURE_INTERNAL_FORMAT
#define GL_TEXTURE_BORDER_COLOR           0x1004
#define GL_TEXTURE_BORDER                 0x1005
#define GL_TEXTURE_RED_SIZE               0x805C
#define GL_TEXTURE_GREEN_SIZE             0x805D
#define GL_TEXTURE_BLUE_SIZE              0x805E
#define GL_TEXTURE_ALPHA_SIZE             0x805F
#define GL_TEXTURE_LUMINANCE_SIZE         0x8060
#define GL_TEXTURE_INTENSITY_SIZE         0x8061
#define GL_TEXTURE_PRIORITY               0x8066
#define GL_TEXTURE_RESIDENT               0x8067

/* HintMode */
#define GL_DONT_CARE                      0x1100
#define GL_FASTEST                        0x1101
#define GL_NICEST                         0x1102

/* HintTarget */
/*      GL_PERSPECTIVE_CORRECTION_HINT */
/*      GL_POINT_SMOOTH_HINT */
/*      GL_LINE_SMOOTH_HINT */
/*      GL_POLYGON_SMOOTH_HINT */
/*      GL_FOG_HINT */

/* IndexMaterialParameterSGI */
/*      GL_INDEX_OFFSET */

/* IndexPointerType */
/*      GL_SHORT */
/*      GL_INT */
/*      GL_FLOAT */
/*      GL_DOUBLE */

/* IndexFunctionSGI */
/*      GL_NEVER */
/*      GL_LESS */
/*      GL_EQUAL */
/*      GL_LEQUAL */
/*      GL_GREATER */
/*      GL_NOTEQUAL */
/*      GL_GEQUAL */
/*      GL_ALWAYS */

/* LightModelParameter */
/*      GL_LIGHT_MODEL_AMBIENT */
/*      GL_LIGHT_MODEL_LOCAL_VIEWER */
/*      GL_LIGHT_MODEL_TWO_SIDE */

/* LightParameter */
#define GL_AMBIENT                        0x1200
#define GL_DIFFUSE                        0x1201
#define GL_SPECULAR                       0x1202
#define GL_POSITION                       0x1203
#define GL_SPOT_DIRECTION                 0x1204
#define GL_SPOT_EXPONENT                  0x1205
#define GL_SPOT_CUTOFF                    0x1206
#define GL_CONSTANT_ATTENUATION           0x1207
#define GL_LINEAR_ATTENUATION             0x1208
#define GL_QUADRATIC_ATTENUATION          0x1209

/* ListMode */
#define GL_COMPILE                        0x1300
#define GL_COMPILE_AND_EXECUTE            0x1301

/* DataType */
#define GL_BYTE                           0x1400
#define GL_UNSIGNED_BYTE                  0x1401
#define GL_SHORT                          0x1402
#define GL_UNSIGNED_SHORT                 0x1403
#define GL_INT                            0x1404
#define GL_UNSIGNED_INT                   0x1405
#define GL_FLOAT                          0x1406
#define GL_2_BYTES                        0x1407
#define GL_3_BYTES                        0x1408
#define GL_4_BYTES                        0x1409
#define GL_DOUBLE                         0x140A
#define GL_DOUBLE_EXT                     0x140A

/* ListNameType */
/*      GL_BYTE */
/*      GL_UNSIGNED_BYTE */
/*      GL_SHORT */
/*      GL_UNSIGNED_SHORT */
/*      GL_INT */
/*      GL_UNSIGNED_INT */
/*      GL_FLOAT */
/*      GL_2_BYTES */
/*      GL_3_BYTES */
/*      GL_4_BYTES */

/* LogicOp */
#define GL_CLEAR                          0x1500
#define GL_AND                            0x1501
#define GL_AND_REVERSE                    0x1502
#define GL_COPY                           0x1503
#define GL_AND_INVERTED                   0x1504
#define GL_NOOP                           0x1505
#define GL_XOR                            0x1506
#define GL_OR                             0x1507
#define GL_NOR                            0x1508
#define GL_EQUIV                          0x1509
#define GL_INVERT                         0x150A
#define GL_OR_REVERSE                     0x150B
#define GL_COPY_INVERTED                  0x150C
#define GL_OR_INVERTED                    0x150D
#define GL_NAND                           0x150E
#define GL_SET                            0x150F

/* MapTarget */
/*      GL_MAP1_COLOR_4 */
/*      GL_MAP1_INDEX */
/*      GL_MAP1_NORMAL */
/*      GL_MAP1_TEXTURE_COORD_1 */
/*      GL_MAP1_TEXTURE_COORD_2 */
/*      GL_MAP1_TEXTURE_COORD_3 */
/*      GL_MAP1_TEXTURE_COORD_4 */
/*      GL_MAP1_VERTEX_3 */
/*      GL_MAP1_VERTEX_4 */
/*      GL_MAP2_COLOR_4 */
/*      GL_MAP2_INDEX */
/*      GL_MAP2_NORMAL */
/*      GL_MAP2_TEXTURE_COORD_1 */
/*      GL_MAP2_TEXTURE_COORD_2 */
/*      GL_MAP2_TEXTURE_COORD_3 */
/*      GL_MAP2_TEXTURE_COORD_4 */
/*      GL_MAP2_VERTEX_3 */
/*      GL_MAP2_VERTEX_4 */

/* MaterialFace */
/*      GL_FRONT */
/*      GL_BACK */
/*      GL_FRONT_AND_BACK */

/* MaterialParameter */
#define GL_EMISSION                       0x1600
#define GL_SHININESS                      0x1601
#define GL_AMBIENT_AND_DIFFUSE            0x1602
#define GL_COLOR_INDEXES                  0x1603
/*      GL_AMBIENT */
/*      GL_DIFFUSE */
/*      GL_SPECULAR */

/* MatrixMode */
#define GL_MODELVIEW                      0x1700
#define GL_PROJECTION                     0x1701
#define GL_TEXTURE                        0x1702

/* MeshMode1 */
/*      GL_POINT */
/*      GL_LINE */

/* MeshMode2 */
/*      GL_POINT */
/*      GL_LINE */
/*      GL_FILL */

/* NormalPointerType */
/*      GL_BYTE */
/*      GL_SHORT */
/*      GL_INT */
/*      GL_FLOAT */
/*      GL_DOUBLE */

/* PixelCopyType */
#define GL_COLOR                          0x1800
#define GL_DEPTH                          0x1801
#define GL_STENCIL                        0x1802

/* PixelFormat */
#define GL_COLOR_INDEX                    0x1900
#define GL_STENCIL_INDEX                  0x1901
#define GL_DEPTH_COMPONENT                0x1902
#define GL_RED                            0x1903
#define GL_GREEN                          0x1904
#define GL_BLUE                           0x1905
#define GL_ALPHA                          0x1906
#define GL_RGB                            0x1907
#define GL_RGBA                           0x1908
#define GL_LUMINANCE                      0x1909
#define GL_LUMINANCE_ALPHA                0x190A
/*      GL_ABGR_EXT */

/* PixelMap */
/*      GL_PIXEL_MAP_I_TO_I */
/*      GL_PIXEL_MAP_S_TO_S */
/*      GL_PIXEL_MAP_I_TO_R */
/*      GL_PIXEL_MAP_I_TO_G */
/*      GL_PIXEL_MAP_I_TO_B */
/*      GL_PIXEL_MAP_I_TO_A */
/*      GL_PIXEL_MAP_R_TO_R */
/*      GL_PIXEL_MAP_G_TO_G */
/*      GL_PIXEL_MAP_B_TO_B */
/*      GL_PIXEL_MAP_A_TO_A */

/* PixelStoreParameter */
/*      GL_UNPACK_SWAP_BYTES */
/*      GL_UNPACK_LSB_FIRST */
/*      GL_UNPACK_ROW_LENGTH */
/*      GL_UNPACK_SKIP_ROWS */
/*      GL_UNPACK_SKIP_PIXELS */
/*      GL_UNPACK_ALIGNMENT */
/*      GL_PACK_SWAP_BYTES */
/*      GL_PACK_LSB_FIRST */
/*      GL_PACK_ROW_LENGTH */
/*      GL_PACK_SKIP_ROWS */
/*      GL_PACK_SKIP_PIXELS */
/*      GL_PACK_ALIGNMENT */

/* PixelTransferParameter */
/*      GL_MAP_COLOR */
/*      GL_MAP_STENCIL */
/*      GL_INDEX_SHIFT */
/*      GL_INDEX_OFFSET */
/*      GL_RED_SCALE */
/*      GL_RED_BIAS */
/*      GL_GREEN_SCALE */
/*      GL_GREEN_BIAS */
/*      GL_BLUE_SCALE */
/*      GL_BLUE_BIAS */
/*      GL_ALPHA_SCALE */
/*      GL_ALPHA_BIAS */
/*      GL_DEPTH_SCALE */
/*      GL_DEPTH_BIAS */

/* PixelType */
#define GL_BITMAP                         0x1A00
/*      GL_BYTE */
/*      GL_UNSIGNED_BYTE */
/*      GL_SHORT */
/*      GL_UNSIGNED_SHORT */
/*      GL_INT */
/*      GL_UNSIGNED_INT */
/*      GL_FLOAT */
/*      GL_UNSIGNED_BYTE_3_3_2_EXT */
/*      GL_UNSIGNED_SHORT_4_4_4_4_EXT */
/*      GL_UNSIGNED_SHORT_5_5_5_1_EXT */
/*      GL_UNSIGNED_INT_8_8_8_8_EXT */
/*      GL_UNSIGNED_INT_10_10_10_2_EXT */

/* PolygonMode */
#define GL_POINT                          0x1B00
#define GL_LINE                           0x1B01
#define GL_FILL                           0x1B02

/* ReadBufferMode */
/*      GL_FRONT_LEFT */
/*      GL_FRONT_RIGHT */
/*      GL_BACK_LEFT */
/*      GL_BACK_RIGHT */
/*      GL_FRONT */
/*      GL_BACK */
/*      GL_LEFT */
/*      GL_RIGHT */
/*      GL_AUX0 */
/*      GL_AUX1 */
/*      GL_AUX2 */
/*      GL_AUX3 */

/* RenderingMode */
#define GL_RENDER                         0x1C00
#define GL_FEEDBACK                       0x1C01
#define GL_SELECT                         0x1C02

/* ShadingModel */
#define GL_FLAT                           0x1D00
#define GL_SMOOTH                         0x1D01

/* StencilFunction */
/*      GL_NEVER */
/*      GL_LESS */
/*      GL_EQUAL */
/*      GL_LEQUAL */
/*      GL_GREATER */
/*      GL_NOTEQUAL */
/*      GL_GEQUAL */
/*      GL_ALWAYS */

/* StencilOp */
/*      GL_ZERO */
#define GL_KEEP                           0x1E00
#define GL_REPLACE                        0x1E01
#define GL_INCR                           0x1E02
#define GL_DECR                           0x1E03
/*      GL_INVERT */

/* StringName */
#define GL_VENDOR                         0x1F00
#define GL_RENDERER                       0x1F01
#define GL_VERSION                        0x1F02
#define GL_EXTENSIONS                     0x1F03

/* TexCoordPointerType */
/*      GL_SHORT */
/*      GL_INT */
/*      GL_FLOAT */
/*      GL_DOUBLE */

/* TextureCoordName */
#define GL_S                              0x2000
#define GL_T                              0x2001
#define GL_R                              0x2002
#define GL_Q                              0x2003

/* TextureEnvMode */
#define GL_MODULATE                       0x2100
#define GL_DECAL                          0x2101
/*      GL_BLEND */
/*      GL_REPLACE */
/*      GL_ADD */

/* TextureEnvParameter */
#define GL_TEXTURE_ENV_MODE               0x2200
#define GL_TEXTURE_ENV_COLOR              0x2201

/* TextureEnvTarget */
#define GL_TEXTURE_ENV                    0x2300

/* TextureGenMode */
#define GL_EYE_LINEAR                     0x2400
#define GL_OBJECT_LINEAR                  0x2401
#define GL_SPHERE_MAP                     0x2402

/* TextureGenParameter */
#define GL_TEXTURE_GEN_MODE               0x2500
#define GL_OBJECT_PLANE                   0x2501
#define GL_EYE_PLANE                      0x2502

/* TextureMagFilter */
#define GL_NEAREST                        0x2600
#define GL_LINEAR                         0x2601

/* TextureMinFilter */
/*      GL_NEAREST */
/*      GL_LINEAR */
#define GL_NEAREST_MIPMAP_NEAREST         0x2700
#define GL_LINEAR_MIPMAP_NEAREST          0x2701
#define GL_NEAREST_MIPMAP_LINEAR          0x2702
#define GL_LINEAR_MIPMAP_LINEAR           0x2703

/* TextureParameterName */
#define GL_TEXTURE_MAG_FILTER             0x2800
#define GL_TEXTURE_MIN_FILTER             0x2801
#define GL_TEXTURE_WRAP_S                 0x2802
#define GL_TEXTURE_WRAP_T                 0x2803
/*      GL_TEXTURE_BORDER_COLOR */
/*      GL_TEXTURE_PRIORITY */

/* TextureTarget */
/*      GL_TEXTURE_1D */
/*      GL_TEXTURE_2D */
#define GL_PROXY_TEXTURE_1D               0x8063
#define GL_PROXY_TEXTURE_2D               0x8064

/* TextureWrapMode */
#define GL_CLAMP                          0x2900
#define GL_REPEAT                         0x2901

/* PixelInternalFormat */
#define GL_R3_G3_B2                       0x2A10
#define GL_ALPHA4                         0x803B
#define GL_ALPHA8                         0x803C
#define GL_ALPHA12                        0x803D
#define GL_ALPHA16                        0x803E
#define GL_LUMINANCE4                     0x803F
#define GL_LUMINANCE8                     0x8040
#define GL_LUMINANCE12                    0x8041
#define GL_LUMINANCE16                    0x8042
#define GL_LUMINANCE4_ALPHA4              0x8043
#define GL_LUMINANCE6_ALPHA2              0x8044
#define GL_LUMINANCE8_ALPHA8              0x8045
#define GL_LUMINANCE12_ALPHA4             0x8046
#define GL_LUMINANCE12_ALPHA12            0x8047
#define GL_LUMINANCE16_ALPHA16            0x8048
#define GL_INTENSITY                      0x8049
#define GL_INTENSITY4                     0x804A
#define GL_INTENSITY8                     0x804B
#define GL_INTENSITY12                    0x804C
#define GL_INTENSITY16                    0x804D
#define GL_RGB4                           0x804F
#define GL_RGB5                           0x8050
#define GL_RGB8                           0x8051
#define GL_RGB10                          0x8052
#define GL_RGB12                          0x8053
#define GL_RGB16                          0x8054
#define GL_RGBA2                          0x8055
#define GL_RGBA4                          0x8056
#define GL_RGB5_A1                        0x8057
#define GL_RGBA8                          0x8058
#define GL_RGB10_A2                       0x8059
#define GL_RGBA12                         0x805A
#define GL_RGBA16                         0x805B

/* InterleavedArrayFormat */
#define GL_V2F                            0x2A20
#define GL_V3F                            0x2A21
#define GL_C4UB_V2F                       0x2A22
#define GL_C4UB_V3F                       0x2A23
#define GL_C3F_V3F                        0x2A24
#define GL_N3F_V3F                        0x2A25
#define GL_C4F_N3F_V3F                    0x2A26
#define GL_T2F_V3F                        0x2A27
#define GL_T4F_V4F                        0x2A28
#define GL_T2F_C4UB_V3F                   0x2A29
#define GL_T2F_C3F_V3F                    0x2A2A
#define GL_T2F_N3F_V3F                    0x2A2B
#define GL_T2F_C4F_N3F_V3F                0x2A2C
#define GL_T4F_C4F_N3F_V4F                0x2A2D

/* VertexPointerType */
/*      GL_SHORT */
/*      GL_INT */
/*      GL_FLOAT */
/*      GL_DOUBLE */

/* ClipPlaneName */
#define GL_CLIP_PLANE0                    0x3000
#define GL_CLIP_PLANE1                    0x3001
#define GL_CLIP_PLANE2                    0x3002
#define GL_CLIP_PLANE3                    0x3003
#define GL_CLIP_PLANE4                    0x3004
#define GL_CLIP_PLANE5                    0x3005

/* LightName */
#define GL_LIGHT0                         0x4000
#define GL_LIGHT1                         0x4001
#define GL_LIGHT2                         0x4002
#define GL_LIGHT3                         0x4003
#define GL_LIGHT4                         0x4004
#define GL_LIGHT5                         0x4005
#define GL_LIGHT6                         0x4006
#define GL_LIGHT7                         0x4007

/* EXT_abgr */
//#define GL_ABGR_EXT                       0x8000

/* EXT_blend_subtract */
/*
#define GL_FUNC_SUBTRACT_EXT              0x800A
#define GL_FUNC_REVERSE_SUBTRACT_EXT      0x800B
*/

/* EXT_packed_pixels */
/*
#define GL_UNSIGNED_BYTE_3_3_2_EXT        0x8032
#define GL_UNSIGNED_SHORT_4_4_4_4_EXT     0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1_EXT     0x8034
#define GL_UNSIGNED_INT_8_8_8_8_EXT       0x8035
#define GL_UNSIGNED_INT_10_10_10_2_EXT    0x8036
*/

/* OpenGL12 */
/*
#define GL_PACK_SKIP_IMAGES               0x806B
#define GL_PACK_IMAGE_HEIGHT              0x806C
#define GL_UNPACK_SKIP_IMAGES             0x806D
#define GL_UNPACK_IMAGE_HEIGHT            0x806E
#define GL_TEXTURE_3D                     0x806F
#define GL_PROXY_TEXTURE_3D               0x8070
#define GL_TEXTURE_DEPTH                  0x8071
#define GL_TEXTURE_WRAP_R                 0x8072
#define GL_MAX_3D_TEXTURE_SIZE            0x8073
#define GL_BGR                            0x80E0
#define GL_BGRA                           0x80E1
#define GL_UNSIGNED_BYTE_3_3_2            0x8032
#define GL_UNSIGNED_BYTE_2_3_3_REV        0x8362
#define GL_UNSIGNED_SHORT_5_6_5           0x8363
#define GL_UNSIGNED_SHORT_5_6_5_REV       0x8364
#define GL_UNSIGNED_SHORT_4_4_4_4         0x8033
#define GL_UNSIGNED_SHORT_4_4_4_4_REV     0x8365
#define GL_UNSIGNED_SHORT_5_5_5_1         0x8034
#define GL_UNSIGNED_SHORT_1_5_5_5_REV     0x8366
#define GL_UNSIGNED_INT_8_8_8_8           0x8035
#define GL_UNSIGNED_INT_8_8_8_8_REV       0x8367
#define GL_UNSIGNED_INT_10_10_10_2        0x8036
#define GL_UNSIGNED_INT_2_10_10_10_REV    0x8368
#define GL_RESCALE_NORMAL                 0x803A
#define GL_LIGHT_MODEL_COLOR_CONTROL      0x81F8
#define GL_SINGLE_COLOR                   0x81F9
#define GL_SEPARATE_SPECULAR_COLOR        0x81FA
#define GL_CLAMP_TO_EDGE                  0x812F
#define GL_TEXTURE_MIN_LOD                0x813A
#define GL_TEXTURE_MAX_LOD                0x813B
#define GL_TEXTURE_BASE_LEVEL             0x813C
#define GL_TEXTURE_MAX_LEVEL              0x813D
#define GL_MAX_ELEMENTS_VERTICES          0x80E8
#define GL_MAX_ELEMENTS_INDICES           0x80E9
#define GL_ALIASED_POINT_SIZE_RANGE       0x846D
#define GL_ALIASED_LINE_WIDTH_RANGE       0x846E
*/

/* EXT_vertex_array */
/*
#define GL_VERTEX_ARRAY_EXT               0x8074
#define GL_NORMAL_ARRAY_EXT               0x8075
#define GL_COLOR_ARRAY_EXT                0x8076
#define GL_INDEX_ARRAY_EXT                0x8077
#define GL_TEXTURE_COORD_ARRAY_EXT        0x8078
#define GL_EDGE_FLAG_ARRAY_EXT            0x8079
#define GL_VERTEX_ARRAY_SIZE_EXT          0x807A
#define GL_VERTEX_ARRAY_TYPE_EXT          0x807B
#define GL_VERTEX_ARRAY_STRIDE_EXT        0x807C
#define GL_VERTEX_ARRAY_COUNT_EXT         0x807D
#define GL_NORMAL_ARRAY_TYPE_EXT          0x807E
#define GL_NORMAL_ARRAY_STRIDE_EXT        0x807F
#define GL_NORMAL_ARRAY_COUNT_EXT         0x8080
#define GL_COLOR_ARRAY_SIZE_EXT           0x8081
#define GL_COLOR_ARRAY_TYPE_EXT           0x8082
#define GL_COLOR_ARRAY_STRIDE_EXT         0x8083
#define GL_COLOR_ARRAY_COUNT_EXT          0x8084
#define GL_INDEX_ARRAY_TYPE_EXT           0x8085
#define GL_INDEX_ARRAY_STRIDE_EXT         0x8086
#define GL_INDEX_ARRAY_COUNT_EXT          0x8087
#define GL_TEXTURE_COORD_ARRAY_SIZE_EXT   0x8088
#define GL_TEXTURE_COORD_ARRAY_TYPE_EXT   0x8089
#define GL_TEXTURE_COORD_ARRAY_STRIDE_EXT 0x808A
#define GL_TEXTURE_COORD_ARRAY_COUNT_EXT  0x808B
#define GL_EDGE_FLAG_ARRAY_STRIDE_EXT     0x808C
#define GL_EDGE_FLAG_ARRAY_COUNT_EXT      0x808D
#define GL_VERTEX_ARRAY_POINTER_EXT       0x808E
#define GL_NORMAL_ARRAY_POINTER_EXT       0x808F
#define GL_COLOR_ARRAY_POINTER_EXT        0x8090
#define GL_INDEX_ARRAY_POINTER_EXT        0x8091
#define GL_TEXTURE_COORD_ARRAY_POINTER_EXT 0x8092
#define GL_EDGE_FLAG_ARRAY_POINTER_EXT    0x8093
*/

/* SGIS_texture_lod */
/*
#define GL_TEXTURE_MIN_LOD_SGIS           0x813A
#define GL_TEXTURE_MAX_LOD_SGIS           0x813B
#define GL_TEXTURE_BASE_LEVEL_SGIS        0x813C
#define GL_TEXTURE_MAX_LEVEL_SGIS         0x813D
*/

/* EXT_shared_texture_palette */
//#define GL_SHARED_TEXTURE_PALETTE_EXT     0x81FB

/* EXT_rescale_normal */
//#define GL_RESCALE_NORMAL_EXT             0x803A

/* SGIX_shadow */
/*
#define GL_TEXTURE_COMPARE_SGIX           0x819A
#define GL_TEXTURE_COMPARE_OPERATOR_SGIX  0x819B
#define GL_TEXTURE_LEQUAL_R_SGIX          0x819C
#define GL_TEXTURE_GEQUAL_R_SGIX          0x819D
*/

/* SGIX_depth_texture */
/*
#define GL_DEPTH_COMPONENT16_SGIX         0x81A5
#define GL_DEPTH_COMPONENT24_SGIX         0x81A6
#define GL_DEPTH_COMPONENT32_SGIX         0x81A7
*/

/* SGIS_generate_mipmap */
/*
#define GL_GENERATE_MIPMAP_SGIS           0x8191
#define GL_GENERATE_MIPMAP_HINT_SGIS      0x8192
*/

#ifndef GL_VERSION_1_2
#define GL_UNSIGNED_BYTE_3_3_2            0x8032
#define GL_UNSIGNED_SHORT_4_4_4_4         0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1         0x8034
#define GL_UNSIGNED_INT_8_8_8_8           0x8035
#define GL_UNSIGNED_INT_10_10_10_2        0x8036
#define GL_RESCALE_NORMAL                 0x803A
#define GL_TEXTURE_BINDING_3D             0x806A
#define GL_PACK_SKIP_IMAGES               0x806B
#define GL_PACK_IMAGE_HEIGHT              0x806C
#define GL_UNPACK_SKIP_IMAGES             0x806D
#define GL_UNPACK_IMAGE_HEIGHT            0x806E
#define GL_TEXTURE_3D                     0x806F
#define GL_PROXY_TEXTURE_3D               0x8070
#define GL_TEXTURE_DEPTH                  0x8071
#define GL_TEXTURE_WRAP_R                 0x8072
#define GL_MAX_3D_TEXTURE_SIZE            0x8073
#define GL_UNSIGNED_BYTE_2_3_3_REV        0x8362
#define GL_UNSIGNED_SHORT_5_6_5           0x8363
#define GL_UNSIGNED_SHORT_5_6_5_REV       0x8364
#define GL_UNSIGNED_SHORT_4_4_4_4_REV     0x8365
#define GL_UNSIGNED_SHORT_1_5_5_5_REV     0x8366
#define GL_UNSIGNED_INT_8_8_8_8_REV       0x8367
#define GL_UNSIGNED_INT_2_10_10_10_REV    0x8368
#define GL_BGR                            0x80E0
#define GL_BGRA                           0x80E1
#define GL_MAX_ELEMENTS_VERTICES          0x80E8
#define GL_MAX_ELEMENTS_INDICES           0x80E9
#define GL_CLAMP_TO_EDGE                  0x812F
#define GL_TEXTURE_MIN_LOD                0x813A
#define GL_TEXTURE_MAX_LOD                0x813B
#define GL_TEXTURE_BASE_LEVEL             0x813C
#define GL_TEXTURE_MAX_LEVEL              0x813D
#define GL_LIGHT_MODEL_COLOR_CONTROL      0x81F8
#define GL_SINGLE_COLOR                   0x81F9
#define GL_SEPARATE_SPECULAR_COLOR        0x81FA
#define GL_SMOOTH_POINT_SIZE_RANGE        0x0B12
#define GL_SMOOTH_POINT_SIZE_GRANULARITY  0x0B13
#define GL_SMOOTH_LINE_WIDTH_RANGE        0x0B22
#define GL_SMOOTH_LINE_WIDTH_GRANULARITY  0x0B23
#define GL_ALIASED_POINT_SIZE_RANGE       0x846D
#define GL_ALIASED_LINE_WIDTH_RANGE       0x846E
#endif

#ifndef GL_VERSION_1_3
#define GL_TEXTURE0                       0x84C0
#define GL_TEXTURE1                       0x84C1
#define GL_TEXTURE2                       0x84C2
#define GL_TEXTURE3                       0x84C3
#define GL_TEXTURE4                       0x84C4
#define GL_TEXTURE5                       0x84C5
#define GL_TEXTURE6                       0x84C6
#define GL_TEXTURE7                       0x84C7
#define GL_TEXTURE8                       0x84C8
#define GL_TEXTURE9                       0x84C9
#define GL_TEXTURE10                      0x84CA
#define GL_TEXTURE11                      0x84CB
#define GL_TEXTURE12                      0x84CC
#define GL_TEXTURE13                      0x84CD
#define GL_TEXTURE14                      0x84CE
#define GL_TEXTURE15                      0x84CF
#define GL_TEXTURE16                      0x84D0
#define GL_TEXTURE17                      0x84D1
#define GL_TEXTURE18                      0x84D2
#define GL_TEXTURE19                      0x84D3
#define GL_TEXTURE20                      0x84D4
#define GL_TEXTURE21                      0x84D5
#define GL_TEXTURE22                      0x84D6
#define GL_TEXTURE23                      0x84D7
#define GL_TEXTURE24                      0x84D8
#define GL_TEXTURE25                      0x84D9
#define GL_TEXTURE26                      0x84DA
#define GL_TEXTURE27                      0x84DB
#define GL_TEXTURE28                      0x84DC
#define GL_TEXTURE29                      0x84DD
#define GL_TEXTURE30                      0x84DE
#define GL_TEXTURE31                      0x84DF
#define GL_ACTIVE_TEXTURE                 0x84E0
#define GL_CLIENT_ACTIVE_TEXTURE          0x84E1
#define GL_MAX_TEXTURE_UNITS              0x84E2
#define GL_TRANSPOSE_MODELVIEW_MATRIX     0x84E3
#define GL_TRANSPOSE_PROJECTION_MATRIX    0x84E4
#define GL_TRANSPOSE_TEXTURE_MATRIX       0x84E5
#define GL_TRANSPOSE_COLOR_MATRIX         0x84E6
#define GL_MULTISAMPLE                    0x809D
#define GL_SAMPLE_ALPHA_TO_COVERAGE       0x809E
#define GL_SAMPLE_ALPHA_TO_ONE            0x809F
#define GL_SAMPLE_COVERAGE                0x80A0
#define GL_SAMPLE_BUFFERS                 0x80A8
#define GL_SAMPLES                        0x80A9
#define GL_SAMPLE_COVERAGE_VALUE          0x80AA
#define GL_SAMPLE_COVERAGE_INVERT         0x80AB
#define GL_MULTISAMPLE_BIT                0x20000000
#define GL_NORMAL_MAP                     0x8511
#define GL_REFLECTION_MAP                 0x8512
#define GL_TEXTURE_CUBE_MAP               0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP       0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X    0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X    0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y    0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y    0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z    0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z    0x851A
#define GL_PROXY_TEXTURE_CUBE_MAP         0x851B
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE      0x851C
#define GL_COMPRESSED_ALPHA               0x84E9
#define GL_COMPRESSED_LUMINANCE           0x84EA
#define GL_COMPRESSED_LUMINANCE_ALPHA     0x84EB
#define GL_COMPRESSED_INTENSITY           0x84EC
#define GL_COMPRESSED_RGB                 0x84ED
#define GL_COMPRESSED_RGBA                0x84EE
#define GL_TEXTURE_COMPRESSION_HINT       0x84EF
#define GL_TEXTURE_COMPRESSED_IMAGE_SIZE  0x86A0
#define GL_TEXTURE_COMPRESSED             0x86A1
#define GL_NUM_COMPRESSED_TEXTURE_FORMATS 0x86A2
#define GL_COMPRESSED_TEXTURE_FORMATS     0x86A3
#define GL_CLAMP_TO_BORDER                0x812D
#define GL_COMBINE                        0x8570
#define GL_COMBINE_RGB                    0x8571
#define GL_COMBINE_ALPHA                  0x8572
#define GL_SOURCE0_RGB                    0x8580
#define GL_SOURCE1_RGB                    0x8581
#define GL_SOURCE2_RGB                    0x8582
#define GL_SOURCE0_ALPHA                  0x8588
#define GL_SOURCE1_ALPHA                  0x8589
#define GL_SOURCE2_ALPHA                  0x858A
#define GL_OPERAND0_RGB                   0x8590
#define GL_OPERAND1_RGB                   0x8591
#define GL_OPERAND2_RGB                   0x8592
#define GL_OPERAND0_ALPHA                 0x8598
#define GL_OPERAND1_ALPHA                 0x8599
#define GL_OPERAND2_ALPHA                 0x859A
#define GL_RGB_SCALE                      0x8573
#define GL_ADD_SIGNED                     0x8574
#define GL_INTERPOLATE                    0x8575
#define GL_SUBTRACT                       0x84E7
#define GL_CONSTANT                       0x8576
#define GL_PRIMARY_COLOR                  0x8577
#define GL_PREVIOUS                       0x8578
#define GL_DOT3_RGB                       0x86AE
#define GL_DOT3_RGBA                      0x86AF
#endif

#ifndef GL_VERSION_1_4
#define GL_BLEND_DST_RGB                  0x80C8
#define GL_BLEND_SRC_RGB                  0x80C9
#define GL_BLEND_DST_ALPHA                0x80CA
#define GL_BLEND_SRC_ALPHA                0x80CB
#define GL_POINT_SIZE_MIN                 0x8126
#define GL_POINT_SIZE_MAX                 0x8127
#define GL_POINT_FADE_THRESHOLD_SIZE      0x8128
#define GL_POINT_DISTANCE_ATTENUATION     0x8129
#define GL_GENERATE_MIPMAP                0x8191
#define GL_GENERATE_MIPMAP_HINT           0x8192
#define GL_DEPTH_COMPONENT16              0x81A5
#define GL_DEPTH_COMPONENT24              0x81A6
#define GL_DEPTH_COMPONENT32              0x81A7
#define GL_MIRRORED_REPEAT                0x8370
#define GL_FOG_COORDINATE_SOURCE          0x8450
#define GL_FOG_COORDINATE                 0x8451
#define GL_FRAGMENT_DEPTH                 0x8452
#define GL_CURRENT_FOG_COORDINATE         0x8453
#define GL_FOG_COORDINATE_ARRAY_TYPE      0x8454
#define GL_FOG_COORDINATE_ARRAY_STRIDE    0x8455
#define GL_FOG_COORDINATE_ARRAY_POINTER   0x8456
#define GL_FOG_COORDINATE_ARRAY           0x8457
#define GL_COLOR_SUM                      0x8458
#define GL_CURRENT_SECONDARY_COLOR        0x8459
#define GL_SECONDARY_COLOR_ARRAY_SIZE     0x845A
#define GL_SECONDARY_COLOR_ARRAY_TYPE     0x845B
#define GL_SECONDARY_COLOR_ARRAY_STRIDE   0x845C
#define GL_SECONDARY_COLOR_ARRAY_POINTER  0x845D
#define GL_SECONDARY_COLOR_ARRAY          0x845E
#define GL_MAX_TEXTURE_LOD_BIAS           0x84FD
#define GL_TEXTURE_FILTER_CONTROL         0x8500
#define GL_TEXTURE_LOD_BIAS               0x8501
#define GL_INCR_WRAP                      0x8507
#define GL_DECR_WRAP                      0x8508
#define GL_TEXTURE_DEPTH_SIZE             0x884A
#define GL_DEPTH_TEXTURE_MODE             0x884B
#define GL_TEXTURE_COMPARE_MODE           0x884C
#define GL_TEXTURE_COMPARE_FUNC           0x884D
#define GL_COMPARE_R_TO_TEXTURE           0x884E
#endif

#ifndef GL_VERSION_1_5
#define GL_BUFFER_SIZE                    0x8764
#define GL_BUFFER_USAGE                   0x8765
#define GL_QUERY_COUNTER_BITS             0x8864
#define GL_CURRENT_QUERY                  0x8865
#define GL_QUERY_RESULT                   0x8866
#define GL_QUERY_RESULT_AVAILABLE         0x8867
#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_ARRAY_BUFFER_BINDING           0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING   0x8895
#define GL_VERTEX_ARRAY_BUFFER_BINDING    0x8896
#define GL_NORMAL_ARRAY_BUFFER_BINDING    0x8897
#define GL_COLOR_ARRAY_BUFFER_BINDING     0x8898
#define GL_INDEX_ARRAY_BUFFER_BINDING     0x8899
#define GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING 0x889A
#define GL_EDGE_FLAG_ARRAY_BUFFER_BINDING 0x889B
#define GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING 0x889C
#define GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING 0x889D
#define GL_WEIGHT_ARRAY_BUFFER_BINDING    0x889E
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING 0x889F
#define GL_READ_ONLY                      0x88B8
#define GL_WRITE_ONLY                     0x88B9
#define GL_READ_WRITE                     0x88BA
#define GL_BUFFER_ACCESS                  0x88BB
#define GL_BUFFER_MAPPED                  0x88BC
#define GL_BUFFER_MAP_POINTER             0x88BD
#define GL_STREAM_DRAW                    0x88E0
#define GL_STREAM_READ                    0x88E1
#define GL_STREAM_COPY                    0x88E2
#define GL_STATIC_DRAW                    0x88E4
#define GL_STATIC_READ                    0x88E5
#define GL_STATIC_COPY                    0x88E6
#define GL_DYNAMIC_DRAW                   0x88E8
#define GL_DYNAMIC_READ                   0x88E9
#define GL_DYNAMIC_COPY                   0x88EA
#define GL_SAMPLES_PASSED                 0x8914
#define GL_FOG_COORD_SRC                  GL_FOG_COORDINATE_SOURCE
#define GL_FOG_COORD                      GL_FOG_COORDINATE
#define GL_CURRENT_FOG_COORD              GL_CURRENT_FOG_COORDINATE
#define GL_FOG_COORD_ARRAY_TYPE           GL_FOG_COORDINATE_ARRAY_TYPE
#define GL_FOG_COORD_ARRAY_STRIDE         GL_FOG_COORDINATE_ARRAY_STRIDE
#define GL_FOG_COORD_ARRAY_POINTER        GL_FOG_COORDINATE_ARRAY_POINTER
#define GL_FOG_COORD_ARRAY                GL_FOG_COORDINATE_ARRAY
#define GL_FOG_COORD_ARRAY_BUFFER_BINDING GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING
#define GL_SRC0_RGB                       GL_SOURCE0_RGB
#define GL_SRC1_RGB                       GL_SOURCE1_RGB
#define GL_SRC2_RGB                       GL_SOURCE2_RGB
#define GL_SRC0_ALPHA                     GL_SOURCE0_ALPHA
#define GL_SRC1_ALPHA                     GL_SOURCE1_ALPHA
#define GL_SRC2_ALPHA                     GL_SOURCE2_ALPHA
#endif

#ifndef GL_VERSION_2_0
#define GL_BLEND_EQUATION_RGB             GL_BLEND_EQUATION
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED    0x8622
#define GL_VERTEX_ATTRIB_ARRAY_SIZE       0x8623
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE     0x8624
#define GL_VERTEX_ATTRIB_ARRAY_TYPE       0x8625
#define GL_CURRENT_VERTEX_ATTRIB          0x8626
#define GL_VERTEX_PROGRAM_POINT_SIZE      0x8642
#define GL_VERTEX_PROGRAM_TWO_SIDE        0x8643
#define GL_VERTEX_ATTRIB_ARRAY_POINTER    0x8645
#define GL_STENCIL_BACK_FUNC              0x8800
#define GL_STENCIL_BACK_FAIL              0x8801
#define GL_STENCIL_BACK_PASS_DEPTH_FAIL   0x8802
#define GL_STENCIL_BACK_PASS_DEPTH_PASS   0x8803
#define GL_MAX_DRAW_BUFFERS               0x8824
#define GL_DRAW_BUFFER0                   0x8825
#define GL_DRAW_BUFFER1                   0x8826
#define GL_DRAW_BUFFER2                   0x8827
#define GL_DRAW_BUFFER3                   0x8828
#define GL_DRAW_BUFFER4                   0x8829
#define GL_DRAW_BUFFER5                   0x882A
#define GL_DRAW_BUFFER6                   0x882B
#define GL_DRAW_BUFFER7                   0x882C
#define GL_DRAW_BUFFER8                   0x882D
#define GL_DRAW_BUFFER9                   0x882E
#define GL_DRAW_BUFFER10                  0x882F
#define GL_DRAW_BUFFER11                  0x8830
#define GL_DRAW_BUFFER12                  0x8831
#define GL_DRAW_BUFFER13                  0x8832
#define GL_DRAW_BUFFER14                  0x8833
#define GL_DRAW_BUFFER15                  0x8834
#define GL_BLEND_EQUATION_ALPHA           0x883D
#define GL_POINT_SPRITE                   0x8861
#define GL_COORD_REPLACE                  0x8862
#define GL_MAX_VERTEX_ATTRIBS             0x8869
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED 0x886A
#define GL_MAX_TEXTURE_COORDS             0x8871
#define GL_MAX_TEXTURE_IMAGE_UNITS        0x8872
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS 0x8B49
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS  0x8B4A
#define GL_MAX_VARYING_FLOATS             0x8B4B
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS 0x8B4C
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS 0x8B4D
#define GL_SHADER_TYPE                    0x8B4F
#define GL_FLOAT_VEC2                     0x8B50
#define GL_FLOAT_VEC3                     0x8B51
#define GL_FLOAT_VEC4                     0x8B52
#define GL_INT_VEC2                       0x8B53
#define GL_INT_VEC3                       0x8B54
#define GL_INT_VEC4                       0x8B55
#define GL_BOOL                           0x8B56
#define GL_BOOL_VEC2                      0x8B57
#define GL_BOOL_VEC3                      0x8B58
#define GL_BOOL_VEC4                      0x8B59
#define GL_FLOAT_MAT2                     0x8B5A
#define GL_FLOAT_MAT3                     0x8B5B
#define GL_FLOAT_MAT4                     0x8B5C
#define GL_SAMPLER_1D                     0x8B5D
#define GL_SAMPLER_2D                     0x8B5E
#define GL_SAMPLER_3D                     0x8B5F
#define GL_SAMPLER_CUBE                   0x8B60
#define GL_SAMPLER_1D_SHADOW              0x8B61
#define GL_SAMPLER_2D_SHADOW              0x8B62
#define GL_DELETE_STATUS                  0x8B80
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_VALIDATE_STATUS                0x8B83
#define GL_INFO_LOG_LENGTH                0x8B84
#define GL_ATTACHED_SHADERS               0x8B85
#define GL_ACTIVE_UNIFORMS                0x8B86
#define GL_ACTIVE_UNIFORM_MAX_LENGTH      0x8B87
#define GL_SHADER_SOURCE_LENGTH           0x8B88
#define GL_ACTIVE_ATTRIBUTES              0x8B89
#define GL_ACTIVE_ATTRIBUTE_MAX_LENGTH    0x8B8A
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT 0x8B8B
#define GL_SHADING_LANGUAGE_VERSION       0x8B8C
#define GL_CURRENT_PROGRAM                0x8B8D
#define GL_POINT_SPRITE_COORD_ORIGIN      0x8CA0
#define GL_LOWER_LEFT                     0x8CA1
#define GL_UPPER_LEFT                     0x8CA2
#define GL_STENCIL_BACK_REF               0x8CA3
#define GL_STENCIL_BACK_VALUE_MASK        0x8CA4
#define GL_STENCIL_BACK_WRITEMASK         0x8CA5
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

#ifndef GL_ARB_depth_texture
#define GL_DEPTH_COMPONENT16_ARB          0x81A5
#define GL_DEPTH_COMPONENT24_ARB          0x81A6
#define GL_DEPTH_COMPONENT32_ARB          0x81A7
#define GL_TEXTURE_DEPTH_SIZE_ARB         0x884A
#define GL_DEPTH_TEXTURE_MODE_ARB         0x884B
#endif

#ifndef GL_ARB_shadow
#define GL_TEXTURE_COMPARE_MODE_ARB       0x884C
#define GL_TEXTURE_COMPARE_FUNC_ARB       0x884D
#define GL_COMPARE_R_TO_TEXTURE_ARB       0x884E
#endif

#ifndef GL_ARB_vertex_program
#define GL_COLOR_SUM_ARB                  0x8458
#define GL_VERTEX_PROGRAM_ARB             0x8620
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED_ARB 0x8622
#define GL_VERTEX_ATTRIB_ARRAY_SIZE_ARB   0x8623
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE_ARB 0x8624
#define GL_VERTEX_ATTRIB_ARRAY_TYPE_ARB   0x8625
#define GL_CURRENT_VERTEX_ATTRIB_ARB      0x8626
#define GL_PROGRAM_LENGTH_ARB             0x8627
#define GL_PROGRAM_STRING_ARB             0x8628
#define GL_MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB 0x862E
#define GL_MAX_PROGRAM_MATRICES_ARB       0x862F
#define GL_CURRENT_MATRIX_STACK_DEPTH_ARB 0x8640
#define GL_CURRENT_MATRIX_ARB             0x8641
#define GL_VERTEX_PROGRAM_POINT_SIZE_ARB  0x8642
#define GL_VERTEX_PROGRAM_TWO_SIDE_ARB    0x8643
#define GL_VERTEX_ATTRIB_ARRAY_POINTER_ARB 0x8645
#define GL_PROGRAM_ERROR_POSITION_ARB     0x864B
#define GL_PROGRAM_BINDING_ARB            0x8677
#define GL_MAX_VERTEX_ATTRIBS_ARB         0x8869
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED_ARB 0x886A
#define GL_PROGRAM_ERROR_STRING_ARB       0x8874
#define GL_PROGRAM_FORMAT_ASCII_ARB       0x8875
#define GL_PROGRAM_FORMAT_ARB             0x8876
#define GL_PROGRAM_INSTRUCTIONS_ARB       0x88A0
#define GL_MAX_PROGRAM_INSTRUCTIONS_ARB   0x88A1
#define GL_PROGRAM_NATIVE_INSTRUCTIONS_ARB 0x88A2
#define GL_MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB 0x88A3
#define GL_PROGRAM_TEMPORARIES_ARB        0x88A4
#define GL_MAX_PROGRAM_TEMPORARIES_ARB    0x88A5
#define GL_PROGRAM_NATIVE_TEMPORARIES_ARB 0x88A6
#define GL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB 0x88A7
#define GL_PROGRAM_PARAMETERS_ARB         0x88A8
#define GL_MAX_PROGRAM_PARAMETERS_ARB     0x88A9
#define GL_PROGRAM_NATIVE_PARAMETERS_ARB  0x88AA
#define GL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB 0x88AB
#define GL_PROGRAM_ATTRIBS_ARB            0x88AC
#define GL_MAX_PROGRAM_ATTRIBS_ARB        0x88AD
#define GL_PROGRAM_NATIVE_ATTRIBS_ARB     0x88AE
#define GL_MAX_PROGRAM_NATIVE_ATTRIBS_ARB 0x88AF
#define GL_PROGRAM_ADDRESS_REGISTERS_ARB  0x88B0
#define GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB 0x88B1
#define GL_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB 0x88B2
#define GL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB 0x88B3
#define GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB 0x88B4
#define GL_MAX_PROGRAM_ENV_PARAMETERS_ARB 0x88B5
#define GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB 0x88B6
#define GL_TRANSPOSE_CURRENT_MATRIX_ARB   0x88B7
#define GL_MATRIX0_ARB                    0x88C0
#define GL_MATRIX1_ARB                    0x88C1
#define GL_MATRIX2_ARB                    0x88C2
#define GL_MATRIX3_ARB                    0x88C3
#define GL_MATRIX4_ARB                    0x88C4
#define GL_MATRIX5_ARB                    0x88C5
#define GL_MATRIX6_ARB                    0x88C6
#define GL_MATRIX7_ARB                    0x88C7
#define GL_MATRIX8_ARB                    0x88C8
#define GL_MATRIX9_ARB                    0x88C9
#define GL_MATRIX10_ARB                   0x88CA
#define GL_MATRIX11_ARB                   0x88CB
#define GL_MATRIX12_ARB                   0x88CC
#define GL_MATRIX13_ARB                   0x88CD
#define GL_MATRIX14_ARB                   0x88CE
#define GL_MATRIX15_ARB                   0x88CF
#define GL_MATRIX16_ARB                   0x88D0
#define GL_MATRIX17_ARB                   0x88D1
#define GL_MATRIX18_ARB                   0x88D2
#define GL_MATRIX19_ARB                   0x88D3
#define GL_MATRIX20_ARB                   0x88D4
#define GL_MATRIX21_ARB                   0x88D5
#define GL_MATRIX22_ARB                   0x88D6
#define GL_MATRIX23_ARB                   0x88D7
#define GL_MATRIX24_ARB                   0x88D8
#define GL_MATRIX25_ARB                   0x88D9
#define GL_MATRIX26_ARB                   0x88DA
#define GL_MATRIX27_ARB                   0x88DB
#define GL_MATRIX28_ARB                   0x88DC
#define GL_MATRIX29_ARB                   0x88DD
#define GL_MATRIX30_ARB                   0x88DE
#define GL_MATRIX31_ARB                   0x88DF
#endif

#ifndef GL_ARB_fragment_program
#define GL_FRAGMENT_PROGRAM_ARB           0x8804
#define GL_PROGRAM_ALU_INSTRUCTIONS_ARB   0x8805
#define GL_PROGRAM_TEX_INSTRUCTIONS_ARB   0x8806
#define GL_PROGRAM_TEX_INDIRECTIONS_ARB   0x8807
#define GL_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB 0x8808
#define GL_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB 0x8809
#define GL_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB 0x880A
#define GL_MAX_PROGRAM_ALU_INSTRUCTIONS_ARB 0x880B
#define GL_MAX_PROGRAM_TEX_INSTRUCTIONS_ARB 0x880C
#define GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB 0x880D
#define GL_MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB 0x880E
#define GL_MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB 0x880F
#define GL_MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB 0x8810
#define GL_MAX_TEXTURE_COORDS_ARB         0x8871
#define GL_MAX_TEXTURE_IMAGE_UNITS_ARB    0x8872
#endif

#ifndef GL_ARB_vertex_buffer_object
#define GL_BUFFER_SIZE_ARB                0x8764
#define GL_BUFFER_USAGE_ARB               0x8765
#define GL_ARRAY_BUFFER_ARB               0x8892
#define GL_ELEMENT_ARRAY_BUFFER_ARB       0x8893
#define GL_ARRAY_BUFFER_BINDING_ARB       0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING_ARB 0x8895
#define GL_VERTEX_ARRAY_BUFFER_BINDING_ARB 0x8896
#define GL_NORMAL_ARRAY_BUFFER_BINDING_ARB 0x8897
#define GL_COLOR_ARRAY_BUFFER_BINDING_ARB 0x8898
#define GL_INDEX_ARRAY_BUFFER_BINDING_ARB 0x8899
#define GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING_ARB 0x889A
#define GL_EDGE_FLAG_ARRAY_BUFFER_BINDING_ARB 0x889B
#define GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING_ARB 0x889C
#define GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING_ARB 0x889D
#define GL_WEIGHT_ARRAY_BUFFER_BINDING_ARB 0x889E
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING_ARB 0x889F
#define GL_READ_ONLY_ARB                  0x88B8
#define GL_WRITE_ONLY_ARB                 0x88B9
#define GL_READ_WRITE_ARB                 0x88BA
#define GL_BUFFER_ACCESS_ARB              0x88BB
#define GL_BUFFER_MAPPED_ARB              0x88BC
#define GL_BUFFER_MAP_POINTER_ARB         0x88BD
#define GL_STREAM_DRAW_ARB                0x88E0
#define GL_STREAM_READ_ARB                0x88E1
#define GL_STREAM_COPY_ARB                0x88E2
#define GL_STATIC_DRAW_ARB                0x88E4
#define GL_STATIC_READ_ARB                0x88E5
#define GL_STATIC_COPY_ARB                0x88E6
#define GL_DYNAMIC_DRAW_ARB               0x88E8
#define GL_DYNAMIC_READ_ARB               0x88E9
#define GL_DYNAMIC_COPY_ARB               0x88EA
#endif

#ifndef GL_ARB_occlusion_query
#define GL_QUERY_COUNTER_BITS_ARB         0x8864
#define GL_CURRENT_QUERY_ARB              0x8865
#define GL_QUERY_RESULT_ARB               0x8866
#define GL_QUERY_RESULT_AVAILABLE_ARB     0x8867
#define GL_SAMPLES_PASSED_ARB             0x8914
#endif

#ifndef GL_ARB_shader_objects
#define GL_PROGRAM_OBJECT_ARB             0x8B40
#define GL_SHADER_OBJECT_ARB              0x8B48
#define GL_OBJECT_TYPE_ARB                0x8B4E
#define GL_OBJECT_SUBTYPE_ARB             0x8B4F
#define GL_FLOAT_VEC2_ARB                 0x8B50
#define GL_FLOAT_VEC3_ARB                 0x8B51
#define GL_FLOAT_VEC4_ARB                 0x8B52
#define GL_INT_VEC2_ARB                   0x8B53
#define GL_INT_VEC3_ARB                   0x8B54
#define GL_INT_VEC4_ARB                   0x8B55
#define GL_BOOL_ARB                       0x8B56
#define GL_BOOL_VEC2_ARB                  0x8B57
#define GL_BOOL_VEC3_ARB                  0x8B58
#define GL_BOOL_VEC4_ARB                  0x8B59
#define GL_FLOAT_MAT2_ARB                 0x8B5A
#define GL_FLOAT_MAT3_ARB                 0x8B5B
#define GL_FLOAT_MAT4_ARB                 0x8B5C
#define GL_SAMPLER_1D_ARB                 0x8B5D
#define GL_SAMPLER_2D_ARB                 0x8B5E
#define GL_SAMPLER_3D_ARB                 0x8B5F
#define GL_SAMPLER_CUBE_ARB               0x8B60
#define GL_SAMPLER_1D_SHADOW_ARB          0x8B61
#define GL_SAMPLER_2D_SHADOW_ARB          0x8B62
#define GL_SAMPLER_2D_RECT_ARB            0x8B63
#define GL_SAMPLER_2D_RECT_SHADOW_ARB     0x8B64
#define GL_OBJECT_DELETE_STATUS_ARB       0x8B80
#define GL_OBJECT_COMPILE_STATUS_ARB      0x8B81
#define GL_OBJECT_LINK_STATUS_ARB         0x8B82
#define GL_OBJECT_VALIDATE_STATUS_ARB     0x8B83
#define GL_OBJECT_INFO_LOG_LENGTH_ARB     0x8B84
#define GL_OBJECT_ATTACHED_OBJECTS_ARB    0x8B85
#define GL_OBJECT_ACTIVE_UNIFORMS_ARB     0x8B86
#define GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH_ARB 0x8B87
#define GL_OBJECT_SHADER_SOURCE_LENGTH_ARB 0x8B88
#endif

#ifndef GL_ARB_vertex_shader
#define GL_VERTEX_SHADER_ARB              0x8B31
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB 0x8B4A
#define GL_MAX_VARYING_FLOATS_ARB         0x8B4B
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB 0x8B4C
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB 0x8B4D
#define GL_OBJECT_ACTIVE_ATTRIBUTES_ARB   0x8B89
#define GL_OBJECT_ACTIVE_ATTRIBUTE_MAX_LENGTH_ARB 0x8B8A
#endif

#ifndef GL_ARB_fragment_shader
#define GL_FRAGMENT_SHADER_ARB            0x8B30
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB 0x8B49
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT_ARB 0x8B8B
#endif

#ifndef GL_ARB_shading_language_100
#define GL_SHADING_LANGUAGE_VERSION_ARB   0x8B8C
#endif

#ifndef GL_ARB_texture_rectangle
#define GL_TEXTURE_RECTANGLE_ARB          0x84F5
#define GL_TEXTURE_BINDING_RECTANGLE_ARB  0x84F6
#define GL_PROXY_TEXTURE_RECTANGLE_ARB    0x84F7
#define GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB 0x84F8
#endif

#ifndef GL_EXT_texture3D
#define GL_PACK_SKIP_IMAGES_EXT           0x806B
#define GL_PACK_IMAGE_HEIGHT_EXT          0x806C
#define GL_UNPACK_SKIP_IMAGES_EXT         0x806D
#define GL_UNPACK_IMAGE_HEIGHT_EXT        0x806E
#define GL_TEXTURE_3D_EXT                 0x806F
#define GL_PROXY_TEXTURE_3D_EXT           0x8070
#define GL_TEXTURE_DEPTH_EXT              0x8071
#define GL_TEXTURE_WRAP_R_EXT             0x8072
#define GL_MAX_3D_TEXTURE_SIZE_EXT        0x8073
#endif

#ifndef GL_EXT_draw_range_elements
#define GL_MAX_ELEMENTS_VERTICES_EXT      0x80E8
#define GL_MAX_ELEMENTS_INDICES_EXT       0x80E9
#endif

#ifndef GL_EXT_texture_filter_anisotropic
#define GL_TEXTURE_MAX_ANISOTROPY_EXT     0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif

#ifndef GL_NV_vertex_program
#define GL_VERTEX_PROGRAM_NV              0x8620
#define GL_VERTEX_STATE_PROGRAM_NV        0x8621
#define GL_ATTRIB_ARRAY_SIZE_NV           0x8623
#define GL_ATTRIB_ARRAY_STRIDE_NV         0x8624
#define GL_ATTRIB_ARRAY_TYPE_NV           0x8625
#define GL_CURRENT_ATTRIB_NV              0x8626
#define GL_PROGRAM_LENGTH_NV              0x8627
#define GL_PROGRAM_STRING_NV              0x8628
#define GL_MODELVIEW_PROJECTION_NV        0x8629
#define GL_IDENTITY_NV                    0x862A
#define GL_INVERSE_NV                     0x862B
#define GL_TRANSPOSE_NV                   0x862C
#define GL_INVERSE_TRANSPOSE_NV           0x862D
#define GL_MAX_TRACK_MATRIX_STACK_DEPTH_NV 0x862E
#define GL_MAX_TRACK_MATRICES_NV          0x862F
#define GL_MATRIX0_NV                     0x8630
#define GL_MATRIX1_NV                     0x8631
#define GL_MATRIX2_NV                     0x8632
#define GL_MATRIX3_NV                     0x8633
#define GL_MATRIX4_NV                     0x8634
#define GL_MATRIX5_NV                     0x8635
#define GL_MATRIX6_NV                     0x8636
#define GL_MATRIX7_NV                     0x8637
#define GL_CURRENT_MATRIX_STACK_DEPTH_NV  0x8640
#define GL_CURRENT_MATRIX_NV              0x8641
#define GL_VERTEX_PROGRAM_POINT_SIZE_NV   0x8642
#define GL_VERTEX_PROGRAM_TWO_SIDE_NV     0x8643
#define GL_PROGRAM_PARAMETER_NV           0x8644
#define GL_ATTRIB_ARRAY_POINTER_NV        0x8645
#define GL_PROGRAM_TARGET_NV              0x8646
#define GL_PROGRAM_RESIDENT_NV            0x8647
#define GL_TRACK_MATRIX_NV                0x8648
#define GL_TRACK_MATRIX_TRANSFORM_NV      0x8649
#define GL_VERTEX_PROGRAM_BINDING_NV      0x864A
#define GL_PROGRAM_ERROR_POSITION_NV      0x864B
#define GL_VERTEX_ATTRIB_ARRAY0_NV        0x8650
#define GL_VERTEX_ATTRIB_ARRAY1_NV        0x8651
#define GL_VERTEX_ATTRIB_ARRAY2_NV        0x8652
#define GL_VERTEX_ATTRIB_ARRAY3_NV        0x8653
#define GL_VERTEX_ATTRIB_ARRAY4_NV        0x8654
#define GL_VERTEX_ATTRIB_ARRAY5_NV        0x8655
#define GL_VERTEX_ATTRIB_ARRAY6_NV        0x8656
#define GL_VERTEX_ATTRIB_ARRAY7_NV        0x8657
#define GL_VERTEX_ATTRIB_ARRAY8_NV        0x8658
#define GL_VERTEX_ATTRIB_ARRAY9_NV        0x8659
#define GL_VERTEX_ATTRIB_ARRAY10_NV       0x865A
#define GL_VERTEX_ATTRIB_ARRAY11_NV       0x865B
#define GL_VERTEX_ATTRIB_ARRAY12_NV       0x865C
#define GL_VERTEX_ATTRIB_ARRAY13_NV       0x865D
#define GL_VERTEX_ATTRIB_ARRAY14_NV       0x865E
#define GL_VERTEX_ATTRIB_ARRAY15_NV       0x865F
#define GL_MAP1_VERTEX_ATTRIB0_4_NV       0x8660
#define GL_MAP1_VERTEX_ATTRIB1_4_NV       0x8661
#define GL_MAP1_VERTEX_ATTRIB2_4_NV       0x8662
#define GL_MAP1_VERTEX_ATTRIB3_4_NV       0x8663
#define GL_MAP1_VERTEX_ATTRIB4_4_NV       0x8664
#define GL_MAP1_VERTEX_ATTRIB5_4_NV       0x8665
#define GL_MAP1_VERTEX_ATTRIB6_4_NV       0x8666
#define GL_MAP1_VERTEX_ATTRIB7_4_NV       0x8667
#define GL_MAP1_VERTEX_ATTRIB8_4_NV       0x8668
#define GL_MAP1_VERTEX_ATTRIB9_4_NV       0x8669
#define GL_MAP1_VERTEX_ATTRIB10_4_NV      0x866A
#define GL_MAP1_VERTEX_ATTRIB11_4_NV      0x866B
#define GL_MAP1_VERTEX_ATTRIB12_4_NV      0x866C
#define GL_MAP1_VERTEX_ATTRIB13_4_NV      0x866D
#define GL_MAP1_VERTEX_ATTRIB14_4_NV      0x866E
#define GL_MAP1_VERTEX_ATTRIB15_4_NV      0x866F
#define GL_MAP2_VERTEX_ATTRIB0_4_NV       0x8670
#define GL_MAP2_VERTEX_ATTRIB1_4_NV       0x8671
#define GL_MAP2_VERTEX_ATTRIB2_4_NV       0x8672
#define GL_MAP2_VERTEX_ATTRIB3_4_NV       0x8673
#define GL_MAP2_VERTEX_ATTRIB4_4_NV       0x8674
#define GL_MAP2_VERTEX_ATTRIB5_4_NV       0x8675
#define GL_MAP2_VERTEX_ATTRIB6_4_NV       0x8676
#define GL_MAP2_VERTEX_ATTRIB7_4_NV       0x8677
#define GL_MAP2_VERTEX_ATTRIB8_4_NV       0x8678
#define GL_MAP2_VERTEX_ATTRIB9_4_NV       0x8679
#define GL_MAP2_VERTEX_ATTRIB10_4_NV      0x867A
#define GL_MAP2_VERTEX_ATTRIB11_4_NV      0x867B
#define GL_MAP2_VERTEX_ATTRIB12_4_NV      0x867C
#define GL_MAP2_VERTEX_ATTRIB13_4_NV      0x867D
#define GL_MAP2_VERTEX_ATTRIB14_4_NV      0x867E
#define GL_MAP2_VERTEX_ATTRIB15_4_NV      0x867F
#endif

#ifndef GL_NV_fragment_program
#define GL_MAX_FRAGMENT_PROGRAM_LOCAL_PARAMETERS_NV 0x8868
#define GL_FRAGMENT_PROGRAM_NV            0x8870
#define GL_MAX_TEXTURE_COORDS_NV          0x8871
#define GL_MAX_TEXTURE_IMAGE_UNITS_NV     0x8872
#define GL_FRAGMENT_PROGRAM_BINDING_NV    0x8873
#define GL_PROGRAM_ERROR_STRING_NV        0x8874
#endif




/// OpenGL 1.1 functions =======================================================
/// display-list functions
//void GLAPI glNewList (GLuint list, GLenum mode);
//void GLAPI glEndList (void);
//void GLAPI glCallList (GLuint list);
//void GLAPI glCallLists (GLsizei n, GLenum type, const GLvoid *lists);
//void GLAPI glDeleteLists (GLuint list, GLsizei range);
//GLuint GLAPI glGenLists (GLsizei range);
//void GLAPI glListBase (GLuint base);
/// drawing functions
extern void (GLAPIENTRY* qglBegin)(GLenum mode, const char *filename, int line);
#define			 xglBegin(mode) \
			 qglBegin(mode, __FILE__, __LINE__)
//void GLAPI glBitmap (GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
//void GLAPI glColor3b (GLbyte red, GLbyte green, GLbyte blue);
//void GLAPI glColor3bv (const GLbyte *v);
//void GLAPI glColor3d (GLdouble red, GLdouble green, GLdouble blue);
//void GLAPI glColor3dv (const GLdouble *v);
//void GLAPI glColor3f (GLfloat red, GLfloat green, GLfloat blue);
//void GLAPI glColor3fv (const GLfloat *v);
//void GLAPI glColor3i (GLint red, GLint green, GLint blue);
//void GLAPI glColor3iv (const GLint *v);
//void GLAPI glColor3s (GLshort red, GLshort green, GLshort blue);
//void GLAPI glColor3sv (const GLshort *v);
//void GLAPI glColor3ub (GLubyte red, GLubyte green, GLubyte blue);
//void GLAPI glColor3ubv (const GLubyte *v);
//void GLAPI glColor3ui (GLuint red, GLuint green, GLuint blue);
//void GLAPI glColor3uiv (const GLuint *v);
//void GLAPI glColor3us (GLushort red, GLushort green, GLushort blue);
//void GLAPI glColor3usv (const GLushort *v);
//void GLAPI glColor4b (GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
//void GLAPI glColor4bv (const GLbyte *v);
//void GLAPI glColor4d (GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
//void GLAPI glColor4dv (const GLdouble *v);
extern void (GLAPIENTRY* qglColor4f)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha, const char *filename, int line);
#define			 xglColor4f(red, green, blue, alpha) \
			 qglColor4f(red, green, blue, alpha, __FILE__, __LINE__)
			 
extern void (GLAPIENTRY* qglColor4fv)(const GLfloat *v, const char *filename, int line);
#define			 xglColor4fv(v) \
			 qglColor4fv(v, __FILE__, __LINE__)
//void GLAPI glColor4i (GLint red, GLint green, GLint blue, GLint alpha);
//void GLAPI glColor4iv (const GLint *v);
//void GLAPI glColor4s (GLshort red, GLshort green, GLshort blue, GLshort alpha);
//void GLAPI glColor4sv (const GLshort *v);
//void GLAPI glColor4ub (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
//void GLAPI glColor4ubv (const GLubyte *v);
//void GLAPI glColor4ui (GLuint red, GLuint green, GLuint blue, GLuint alpha);
//void GLAPI glColor4uiv (const GLuint *v);
//void GLAPI glColor4us (GLushort red, GLushort green, GLushort blue, GLushort alpha);
//void GLAPI glColor4usv (const GLushort *v);
//void GLAPI glEdgeFlag (GLboolean flag);
//void GLAPI glEdgeFlagv (const GLboolean *flag);
extern void (GLAPIENTRY* qglEnd)(const char *filename, int line);
#define			 xglEnd() \
			 qglEnd(__FILE__, __LINE__)
//void GLAPI glIndexd (GLdouble c);
//void GLAPI glIndexdv (const GLdouble *c);
//void GLAPI glIndexf (GLfloat c);
//void GLAPI glIndexfv (const GLfloat *c);
//void GLAPI glIndexi (GLint c);
//void GLAPI glIndexiv (const GLint *c);
//void GLAPI glIndexs (GLshort c);
//void GLAPI glIndexsv (const GLshort *c);
//void GLAPI glNormal3b (GLbyte nx, GLbyte ny, GLbyte nz);
//void GLAPI glNormal3bv (const GLbyte *v);
//void GLAPI glNormal3d (GLdouble nx, GLdouble ny, GLdouble nz);
//void GLAPI glNormal3dv (const GLdouble *v);
//extern void GLAPI glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz);
//extern void GLAPI glNormal3fv (const GLfloat *v);
//void GLAPI glNormal3i (GLint nx, GLint ny, GLint nz);
//void GLAPI glNormal3iv (const GLint *v);
//void GLAPI glNormal3s (GLshort nx, GLshort ny, GLshort nz);
//void GLAPI glNormal3sv (const GLshort *v);
//void GLAPI glRasterPos2d (GLdouble x, GLdouble y);
//void GLAPI glRasterPos2dv (const GLdouble *v);
//void GLAPI glRasterPos2f (GLfloat x, GLfloat y);
//void GLAPI glRasterPos2fv (const GLfloat *v);
//void GLAPI glRasterPos2i (GLint x, GLint y);
//void GLAPI glRasterPos2iv (const GLint *v);
//void GLAPI glRasterPos2s (GLshort x, GLshort y);
//void GLAPI glRasterPos2sv (const GLshort *v);
//void GLAPI glRasterPos3d (GLdouble x, GLdouble y, GLdouble z);
//void GLAPI glRasterPos3dv (const GLdouble *v);
//void GLAPI glRasterPos3f (GLfloat x, GLfloat y, GLfloat z);
//void GLAPI glRasterPos3fv (const GLfloat *v);
//void GLAPI glRasterPos3i (GLint x, GLint y, GLint z);
//void GLAPI glRasterPos3iv (const GLint *v);
//void GLAPI glRasterPos3s (GLshort x, GLshort y, GLshort z);
//void GLAPI glRasterPos3sv (const GLshort *v);
//void GLAPI glRasterPos4d (GLdouble x, GLdouble y, GLdouble z, GLdouble w);
//void GLAPI glRasterPos4dv (const GLdouble *v);
//void GLAPI glRasterPos4f (GLfloat x, GLfloat y, GLfloat z, GLfloat w);
//void GLAPI glRasterPos4fv (const GLfloat *v);
//void GLAPI glRasterPos4i (GLint x, GLint y, GLint z, GLint w);
//void GLAPI glRasterPos4iv (const GLint *v);
//void GLAPI glRasterPos4s (GLshort x, GLshort y, GLshort z, GLshort w);
//void GLAPI glRasterPos4sv (const GLshort *v);
//void GLAPI glRectd (GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
//void GLAPI glRectdv (const GLdouble *v1, const GLdouble *v2);
//void GLAPI glRectf (GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
//void GLAPI glRectfv (const GLfloat *v1, const GLfloat *v2);
//void GLAPI glRecti (GLint x1, GLint y1, GLint x2, GLint y2);
//void GLAPI glRectiv (const GLint *v1, const GLint *v2);
//void GLAPI glRects (GLshort x1, GLshort y1, GLshort x2, GLshort y2);
//void GLAPI glRectsv (const GLshort *v1, const GLshort *v2);
//void GLAPI glTexCoord1d (GLdouble s);
//void GLAPI glTexCoord1dv (const GLdouble *v);
extern void (GLAPIENTRY* qglTexCoord1f)(GLfloat s, const char *filename, int line);
#define			 xglTexCoord1f(s) \
			 qglTexCoord1f(s, __FILE__, __LINE__)

extern void (GLAPIENTRY* xglTexCoord1fv)(const GLfloat *v, const char *filename, int line);
#define			 xglTexCoord1fv(v) \
			 qglTexCoord1fv(v, __FILE__, __LINE__)
//void GLAPI glTexCoord1i (GLint s);
//void GLAPI glTexCoord1iv (const GLint *v);
//void GLAPI glTexCoord1s (GLshort s);
//void GLAPI glTexCoord1sv (const GLshort *v);
//void GLAPI glTexCoord2d (GLdouble s, GLdouble t);
//void GLAPI glTexCoord2dv (const GLdouble *v);
extern void (GLAPIENTRY* qglTexCoord2f)(GLfloat s, GLfloat t, const char *filename, int line);
#define			 xglTexCoord2f(s, t) \
			 qglTexCoord2f(s, t, __FILE__, __LINE__)
			 
extern void (GLAPIENTRY* qglTexCoord2fv)(const GLfloat *v, const char *filename, int line);
#define			 xglTexCoord2fv(v) \
			 qglTexCoord2fv(v, __FILE__, __LINE__)
//void GLAPI glTexCoord2i (GLint s, GLint t);
//void GLAPI glTexCoord2iv (const GLint *v);
//void GLAPI glTexCoord2s (GLshort s, GLshort t);
//void GLAPI glTexCoord2sv (const GLshort *v);
//void GLAPI glTexCoord3d (GLdouble s, GLdouble t, GLdouble r);
//void GLAPI glTexCoord3dv (const GLdouble *v);
extern void (GLAPIENTRY* xglTexCoord3f)(GLfloat s, GLfloat t, GLfloat r, const char *filename, int line);
#define			 xglTexCoord3f(s, t, r) \
			 qglTexCoord3f(s, t, r, __FILE__, __LINE__)

extern void (GLAPIENTRY* qglTexCoord3fv)(const GLfloat *v, const char *filename, int line);
#define			 xglTexCoord3fv(v) \
			 qglTexCoord3fv(v, __FILE__, __LINE__)
//void GLAPI glTexCoord3i (GLint s, GLint t, GLint r);
//void GLAPI glTexCoord3iv (const GLint *v);
//void GLAPI glTexCoord3s (GLshort s, GLshort t, GLshort r);
//void GLAPI glTexCoord3sv (const GLshort *v);
//void GLAPI glTexCoord4d (GLdouble s, GLdouble t, GLdouble r, GLdouble q);
//void GLAPI glTexCoord4dv (const GLdouble *v);
//void GLAPI glTexCoord4f (GLfloat s, GLfloat t, GLfloat r, GLfloat q);
//void GLAPI glTexCoord4fv (const GLfloat *v);
//void GLAPI glTexCoord4i (GLint s, GLint t, GLint r, GLint q);
//void GLAPI glTexCoord4iv (const GLint *v);
//void GLAPI glTexCoord4s (GLshort s, GLshort t, GLshort r, GLshort q);
//void GLAPI glTexCoord4sv (const GLshort *v);
//void GLAPI glVertex2d (GLdouble x, GLdouble y);
//void GLAPI glVertex2dv (const GLdouble *v);
//void GLAPI glVertex2f (GLfloat x, GLfloat y);
//void GLAPI glVertex2fv (const GLfloat *v);
//void GLAPI glVertex2i (GLint x, GLint y);
//void GLAPI glVertex2iv (const GLint *v);
//void GLAPI glVertex2s (GLshort x, GLshort y);
//void GLAPI glVertex2sv (const GLshort *v);
//void GLAPI glVertex3d (GLdouble x, GLdouble y, GLdouble z);
//void GLAPI glVertex3dv (const GLdouble *v); 
extern void (GLAPIENTRY* qglVertex3f)(GLfloat x, GLfloat y, GLfloat z, const char *filename, int line);
#define			 xglVertex3f(x, y, z) \
			 qglVertex3f(x, y, z, __FILE__, __LINE__)

extern void (GLAPIENTRY* qglVertex3fv)(const GLfloat *v, const char *filename, int line);
#define			 xglVertex3fv(v) \
			 qglVertex3fv(v, __FILE__, __LINE__)
//void GLAPI glVertex3i (GLint x, GLint y, GLint z);
//void GLAPI glVertex3iv (const GLint *v);
//void GLAPI glVertex3s (GLshort x, GLshort y, GLshort z);
//void GLAPI glVertex3sv (const GLshort *v);
//void GLAPI glVertex4d (GLdouble x, GLdouble y, GLdouble z, GLdouble w);
//void GLAPI glVertex4dv (const GLdouble *v);
//extern void (GLAPIENTRY* glVertex4f) (GLfloat x, GLfloat y, GLfloat z, GLfloat w);
//extern void (GLAPIENTRY* glVertex4fv) (const GLfloat *v);
//void GLAPI glVertex4i (GLint x, GLint y, GLint z, GLint w);
//void GLAPI glVertex4iv (const GLint *v);
//void GLAPI glVertex4s (GLshort x, GLshort y, GLshort z, GLshort w);
//void GLAPI glVertex4sv (const GLshort *v);
/// drawing-control functions
//void GLAPI glClipPlane (GLenum plane, const GLdouble *equation);
//void GLAPI glColorMaterial (GLenum face, GLenum mode);
extern void (GLAPIENTRY* qglCullFace)(GLenum mode, const char *filename, int line);
#define			 xglCullFace(mode) \
			 qglCullFace(mode, __FILE__, __LINE__)
//void GLAPI glFogf (GLenum pname, GLfloat param);
//void GLAPI glFogfv (GLenum pname, const GLfloat *params);
//void GLAPI glFogi (GLenum pname, GLint param);
//void GLAPI glFogiv (GLenum pname, const GLint *params);
extern void (GLAPIENTRY* qglFrontFace)(GLenum mode, const char *filename, int line);
#define			 xglFrontFace(mode) \
			 qglFrontFace(mode, __FILE__, __LINE__)
//void GLAPI glHint (GLenum target, GLenum mode);
//void GLAPI glLightf (GLenum light, GLenum pname, GLfloat param);
//void GLAPI glLightfv (GLenum light, GLenum pname, const GLfloat *params);
//void GLAPI glLighti (GLenum light, GLenum pname, GLint param);
//void GLAPI glLightiv (GLenum light, GLenum pname, const GLint *params);
//void GLAPI glLightModelf (GLenum pname, GLfloat param);
//void GLAPI glLightModelfv (GLenum pname, const GLfloat *params);
//void GLAPI glLightModeli (GLenum pname, GLint param);
//void GLAPI glLightModeliv (GLenum pname, const GLint *params);
//void GLAPI glLineStipple (GLint factor, GLushort pattern);
//void GLAPI glLineWidth (GLfloat width);
//void GLAPI glMaterialf (GLenum face, GLenum pname, GLfloat param);
//void GLAPI glMaterialfv (GLenum face, GLenum pname, const GLfloat *params);
//void GLAPI glMateriali (GLenum face, GLenum pname, GLint param);
//void GLAPI glMaterialiv (GLenum face, GLenum pname, const GLint *params);
//void GLAPI glPointSize (GLfloat size);
extern void (GLAPIENTRY* qglPolygonMode)(GLenum face, GLenum mode, const char *filename, int line);
#define			 xglPolygonMode(face, mode) \
			 qglPolygonMode(face, mode, __FILE__, __LINE__)
//void GLAPI glPolygonStipple (const GLubyte *mask);
extern void (GLAPIENTRY* qglScissor)(GLint x, GLint y, GLsizei width, GLsizei height, const char *filename, int line);
#define			 xglScissor(x, y, width, height) \
			 qglScissor(x, y, width, height, __FILE__, __LINE__)
//void GLAPI glShadeModel (GLenum mode);
//void GLAPI glTexParameterf (GLenum target, GLenum pname, GLfloat param);
extern void (GLAPIENTRY* qglTexParameterfv)(GLenum target, GLenum pname, const GLfloat *params, const char *filename, int line);
#define			 xglTexParameterfv(target, pname, params) \
			 qglTexParameterfv(target, pname, params, __FILE__, __LINE__)
			 
extern void (GLAPIENTRY* qglTexParameteri)(GLenum target, GLenum pname, GLint param, const char *filename, int line);
#define			 xglTexParameteri(target, pname, param) \
			 qglTexParameteri(target, pname, param, __FILE__, __LINE__)
			 
extern void (GLAPIENTRY* qglTexParameteriv)(GLenum target, GLenum pname, const GLint *params, const char *filename, int line);
#define			 xglTexParameteriv(target, pname, params) \
			 qglTexParameteriv(target, pname, params, __FILE__, __LINE__)
			 
extern void (GLAPIENTRY* qglTexImage1D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels, const char *filename, int line);
#define			 xglTexImage1D(target, level, internalformat, width, border, format, type, pixels) \
			 qglTexImage1D(target, level, internalformat, width, border, format, type, pixels, __FILE__, __LINE__)

extern void (GLAPIENTRY* qglTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels, const char *filename, int line);
#define			 xglTexImage2D(target, level, internalformat, width, height, border, format, type, pixels) \
			 qglTexImage2D(target, level, internalformat, width, height, border, format, type, pixels, __FILE__, __LINE__)
//void GLAPI glTexEnvf (GLenum target, GLenum pname, GLfloat param);
//void GLAPI glTexEnvfv (GLenum target, GLenum pname, const GLfloat *params);
extern void (GLAPIENTRY* qglTexEnvi)(GLenum target, GLenum pname, GLint param, const char *filename, int line);
#define			 xglTexEnvi(target, pname, param) \
			 qglTexEnvi(target, pname, param, __FILE__, __LINE__)
//void GLAPI glTexEnviv (GLenum target, GLenum pname, const GLint *params);
//void GLAPI glTexGend (GLenum coord, GLenum pname, GLdouble param);
//void GLAPI glTexGendv (GLenum coord, GLenum pname, const GLdouble *params);
extern void (GLAPIENTRY* qglTexGenf)(GLenum coord, GLenum pname, GLfloat param, const char *filename, int line);
#define			 xglTexGenf(coord, pname, param) \
			 qglTexGenf(coord, pname, param)
			 
extern void (GLAPIENTRY* qglTexGenfv)(GLenum coord, GLenum pname, const GLfloat *params, const char *filename, int line);
#define			 xglTexGenfv(coord, pname, params) \
			 qglTexGenfv(coord, pname, params, __FILE__, __LINE__)
			 
extern void (GLAPIENTRY* qglTexGeni)(GLenum coord, GLenum pname, GLint param, const char *filename, int line);
#define			 xglTexGeni(coord, pname, param) \
			 qglTexGeni(coord, pname, param, __FILE__, __LINE__)
			 
extern void (GLAPIENTRY* qglTexGeniv)(GLenum coord, GLenum pname, const GLint *params, const char *filename, int line);
#define			 xglTexGeniv(coord, pname, params) \
			 qglTexGeniv(coord, pname, params, __FILE__, __LINE__)
/// feedback functions
//void GLAPI glFeedbackBuffer (GLsizei size, GLenum type, GLfloat *buffer);
//void GLAPI glSelectBuffer (GLsizei size, GLuint *buffer);
//GLint GLAPI glRenderMode (GLenum mode);
//void GLAPI glInitNames (void);
//void GLAPI glLoadName (GLuint name);
//void GLAPI glPassThrough (GLfloat token);
//void GLAPI glPopName (void);
//void GLAPI glPushName (GLuint name);
/// framebuf functions
extern void (GLAPIENTRY* qglDrawBuffer)(GLenum mode, const char *filename, int line);
#define			 xglDrawBuffer(mode) \
			 qglDrawBuffer(mode, __FILE__, __LINE__)
			 
extern void (GLAPIENTRY* qglClear)(GLbitfield mask, const char *filename, int line);
#define			 xglClear(mask) \
			 qglClear(mask, __FILE__, __LINE__)
//void GLAPI glClearAccum (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
//void GLAPI glClearIndex (GLfloat c);
extern void (GLAPIENTRY* qglClearColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha, const char *filename, int line);
#define			 xglClearColor(red, green, blue, alpha) \
			 qglClearColor(red, green, blue, alpha, __FILE__, __LINE__)
			 
extern void (GLAPIENTRY* qglClearStencil)(GLint s, const char *filename, int line);
#define			 xglClearStencil(s) \
			 qglClearStencil(s, __FILE__, __LINE__)
//extern void (GLAPIENTRY* xglClearDepth) (GLclampd depth);
//void GLAPI glStencilMask (GLuint mask);
extern void (GLAPIENTRY* qglColorMask)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha, const char *filename, int line);
#define			 xglColorMask(red, green, blue, alpha) \
			 qglColorMask(red, green, blue, alpha, __FILE__, __LINE__)
			 
extern void (GLAPIENTRY* qglDepthMask)(GLboolean flag, const char *filename, int line);
#define			 xglDepthMask(flag) \
			 qglDepthMask(flag, __FILE__, __LINE__)
//void GLAPI glIndexMask (GLuint mask);
/// misc functions
//void GLAPI glAccum (GLenum op, GLfloat value);
extern void (GLAPIENTRY* qglDisable)(GLenum cap, const char *filename, int line);
#define			 xglDisable(cap) \
			 qglDisable(cap, __FILE__, __LINE__)
			 
extern void (GLAPIENTRY* qglEnable)(GLenum cap, const char *filename, int line);
#define			 xglEnable(cap) \
			 qglEnable(cap, __FILE__, __LINE__)
//extern void (GLAPIENTRY* glFinish)();
extern void (GLAPIENTRY* qglFlush)(const char *filename, int line);
#define			 xglFlush() \
			 qglFlush(__FILE__, __LINE__)
			 
extern void (GLAPIENTRY* qglPopAttrib)(const char *filename, int line);
#define			 xglPopAttrib() \
			 qglPopAttrib(__FILE__, __LINE__)
			 
extern void (GLAPIENTRY* qglPushAttrib)(GLbitfield mask, const char *filename, int line);
#define			 xglPushAttrib(mask) \
			 qglPushAttrib(mask, __FILE__, __LINE__)
/// modelling functions
//void GLAPI glMap1d (GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points);
//void GLAPI glMap1f (GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);
//void GLAPI glMap2d (GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points);
//void GLAPI glMap2f (GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points);
//void GLAPI glMapGrid1d (GLint un, GLdouble u1, GLdouble u2);
//void GLAPI glMapGrid1f (GLint un, GLfloat u1, GLfloat u2);
//void GLAPI glMapGrid2d (GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
//void GLAPI glMapGrid2f (GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
//void GLAPI glEvalCoord1d (GLdouble u);
//void GLAPI glEvalCoord1dv (const GLdouble *u);
//void GLAPI glEvalCoord1f (GLfloat u);
//void GLAPI glEvalCoord1fv (const GLfloat *u);
//void GLAPI glEvalCoord2d (GLdouble u, GLdouble v);
//void GLAPI glEvalCoord2dv (const GLdouble *u);
//void GLAPI glEvalCoord2f (GLfloat u, GLfloat v);
//void GLAPI glEvalCoord2fv (const GLfloat *u);
//void GLAPI glEvalMesh1 (GLenum mode, GLint i1, GLint i2);
//void GLAPI glEvalPoint1 (GLint i);
//void GLAPI glEvalMesh2 (GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
//void GLAPI glEvalPoint2 (GLint i, GLint j);
/// pixel-op functions
extern void (GLAPIENTRY* qglAlphaFunc)(GLenum func, GLclampf ref, const char *filename, int line);
#define			 xglAlphaFunc(func, ref) \
			 qglAlphaFunc(func, ref, __FILE__, __LINE__)
			 
extern void (GLAPIENTRY* qglBlendFunc)(GLenum sfactor, GLenum dfactor, const char *filename, int line);
#define			 xglBlendFunc(sfactor, dfactor) \
			 qglBlendFunc(sfactor, dfactor, __FILE__, __LINE__)

//void GLAPI glLogicOp (GLenum opcode);
extern void (GLAPIENTRY* qglStencilFunc)(GLenum func, GLint ref, GLuint mask, const char *filename, int line);
#define			 xglStencilFunc(func, ref, mask) \
			 qglStencilFunc(func, ref, mask, __FILE__, __LINE__)

extern void (GLAPIENTRY* qglStencilOp)(GLenum fail, GLenum zfail, GLenum zpass, const char *filename, int line);
#define			 xglStencilOp(fail, zfail, zpass) \
			 qglStencilOp(fail, zfail, zpass, __FILE__, __LINE__)

extern void (GLAPIENTRY* qglDepthFunc)(GLenum func, const char *filename, int line);
#define			 xglDepthFunc(func) \
			 qglDepthFunc(func, __FILE__, __LINE__)
/// pixel-rw functions
//void GLAPI glPixelZoom (GLfloat xfactor, GLfloat yfactor);
//void GLAPI glPixelTransferf (GLenum pname, GLfloat param);
//void GLAPI glPixelTransferi (GLenum pname, GLint param);
//void GLAPI glPixelStoref (GLenum pname, GLfloat param);
//void GLAPI glPixelStorei (GLenum pname, GLint param);
//void GLAPI glPixelMapfv (GLenum map, GLint mapsize, const GLfloat *values);
//void GLAPI glPixelMapuiv (GLenum map, GLint mapsize, const GLuint *values);
//void GLAPI glPixelMapusv (GLenum map, GLint mapsize, const GLushort *values);
extern void (GLAPIENTRY* qglReadBuffer)(GLenum mode, const char *filename, int line);
#define			 xglReadBuffer(mode) \
			 qglReadBuffer(mode, __FILE__, __LINE__)

//void GLAPI glCopyPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
extern void (GLAPIENTRY* qglReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels, const char *filename, int line);
#define			 xglReadPixels(x, y, width, height, format, type, pixels) \
			 qglReadPixels(x, y, width, height, format, type, pixels, __FILE__, __LINE__)

//void GLAPI glDrawPixels (GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
/// state-req functions
//void GLAPI glGetBooleanv (GLenum pname, GLboolean *params);
//void GLAPI glGetClipPlane (GLenum plane, GLdouble *equation);
//void GLAPI glGetDoublev (GLenum pname, GLdouble *params);
//extern GLenum (GLAPIENTRY* qglGetError)(const char *filename, int line);
//define		   xglGetError() 
//			   qglGetError(__FILE__, __LINE__)

extern void (GLAPIENTRY* qglGetFloatv)(GLenum pname, GLfloat *params, const char *filename, int line);
#define			 xglGetFloatv(pname, params) \
			 qglGetFloatv(pname, params, __FILE__, __LINE__)

extern void (GLAPIENTRY* qglGetIntegerv)(GLenum pname, GLint *params, const char *filename, int line);
#define			 xglGetIntegerv(pname, params) \
			 qglGetIntegerv(pname, params, __FILE__, __LINE__)

//void GLAPI glGetLightfv (GLenum light, GLenum pname, GLfloat *params);
//void GLAPI glGetLightiv (GLenum light, GLenum pname, GLint *params);
//void GLAPI glGetMapdv (GLenum target, GLenum query, GLdouble *v);
//void GLAPI glGetMapfv (GLenum target, GLenum query, GLfloat *v);
//void GLAPI glGetMapiv (GLenum target, GLenum query, GLint *v);
//void GLAPI glGetMaterialfv (GLenum face, GLenum pname, GLfloat *params);
//void GLAPI glGetMaterialiv (GLenum face, GLenum pname, GLint *params);
//void GLAPI glGetPixelMapfv (GLenum map, GLfloat *values);
//void GLAPI glGetPixelMapuiv (GLenum map, GLuint *values);
//void GLAPI glGetPixelMapusv (GLenum map, GLushort *values);
//void GLAPI glGetPolygonStipple (GLubyte *mask);
extern const GLubyte* (GLAPIENTRY* qglGetString)(GLenum name, const char *filename, int line);
#define				   xglGetString(name) \
				   qglGetString(name, __FILE__, __LINE__)

//void GLAPI glGetTexEnvfv (GLenum target, GLenum pname, GLfloat *params);
//void GLAPI glGetTexEnviv (GLenum target, GLenum pname, GLint *params);
//void GLAPI glGetTexGendv (GLenum coord, GLenum pname, GLdouble *params);
//void GLAPI glGetTexGenfv (GLenum coord, GLenum pname, GLfloat *params);
//void GLAPI glGetTexGeniv (GLenum coord, GLenum pname, GLint *params);
//void GLAPI glGetTexImage (GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
//void GLAPI glGetTexParameterfv (GLenum target, GLenum pname, GLfloat *params);
//void GLAPI glGetTexParameteriv (GLenum target, GLenum pname, GLint *params);
//void GLAPI glGetTexLevelParameterfv (GLenum target, GLint level, GLenum pname, GLfloat *params);
//void GLAPI glGetTexLevelParameteriv (GLenum target, GLint level, GLenum pname, GLint *params);
//GLboolean GLAPI glIsEnabled (GLenum cap);
//GLboolean GLAPI glIsList (GLuint list);
/// xform functions
extern void (GLAPIENTRY* qglDepthRange)(GLclampd zNear, GLclampd zFar, const char *filename, int line);
#define			 xglDepthRange(zNear, zFar) \
			 qglDepthRange(zNear, zFar, __FILE__, __LINE__)

//extern void (GLAPIENTRY* xglFrustum) (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
extern void (GLAPIENTRY* qglLoadIdentity)(const char *filename, int line);
#define			 xglLoadIdentity() \
			 qglLoadIdentity(__FILE__, __LINE__)

extern void (GLAPIENTRY* qglLoadMatrixf)(const GLfloat *m, const char *filename, int line);
#define			 xglLoadMatrixf(m) \
			 qglLoadMatrixf(m, __FILE__, __LINE__)

//void GLAPI glLoadMatrixd (const GLdouble *m);
extern void (GLAPIENTRY* qglMatrixMode)(GLenum mode, const char *filename, int line);
#define			 xglMatrixMode(mode) \
			 qglMatrixMode(mode, __FILE__, __LINE__)

//void GLAPI glMultMatrixf (const GLfloat *m);
//void GLAPI glMultMatrixd (const GLdouble *m);
//extern void (GLAPIENTRY* qglOrtho)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
//define		 xglOrtho(left, right, bottom, top, zNear, zFar) 
//			 qglOrtho(left, right, bottom, top, zNear, zFar, __FILE__, __LINE__)

//extern void (GLAPIENTRY* qglPopMatrix)();
//define			 xglPopMatrix() 
//			 xglPopMatrix(__FILE__, __LINE__)

//extern void (GLAPIENTRY* qglPushMatrix)();
//define		 xglPushMatrix() 
//			 qglPushMatrix(__FILE__, __LINE__)

//void GLAPI glRotated (GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
//extern void (GLAPIENTRY* xglRotatef) (GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
//void GLAPI glScaled (GLdouble x, GLdouble y, GLdouble z);
//extern void (GLAPIENTRY* xglScalef) (GLfloat x, GLfloat y, GLfloat z);
//void GLAPI glTranslated (GLdouble x, GLdouble y, GLdouble z);
//extern void (GLAPIENTRY* xglTranslatef) (GLfloat x, GLfloat y, GLfloat z);
extern void (GLAPIENTRY* qglViewport)(GLint x, GLint y, GLsizei width, GLsizei height, const char *filename, int line);
#define			 xglViewport(x, y, width, height) \
			 qglViewport(x, y, width, height, __FILE__, __LINE__)
/// 1.1 functions
//void GLAPI glArrayElement (GLint i);
//void GLAPI glColorPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
extern void (GLAPIENTRY* qglDisableClientState)(GLenum array, const char *filename, int line);
#define			 xglDisableClientState(array) \
			 qglDisableClientState(array, __FILE__, __LINE__)

//void GLAPI glDrawArrays (GLenum mode, GLint first, GLsizei count);
extern void (GLAPIENTRY* qglDrawElements)(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, const char *filename, int line);
#define			 xglDrawElements(mode, count, type, indices) \
			 qglDrawElements(mode, count, type, indices, __FILE__, __LINE__)

//void GLAPI glEdgeFlagPointer (GLsizei stride, const GLboolean *pointer);
extern void (GLAPIENTRY* qglEnableClientState)(GLenum array, const char *filename, int line);
#define			 xglEnableClientState(array) \
			 qglEnableClientState(array, __FILE__, __LINE__)
			 
//void GLAPI glGetPointerv (GLenum pname, GLvoid* *params);
extern void (GLAPIENTRY* qglIndexPointer)(GLenum type, GLsizei stride, const GLvoid *pointer, const char *filename, int line);
#define			 xglIndexPointer(type, stride, pointer) \
			 qglIndexPointer(type, stride, pointer, __FILE__, __LINE__)

//void GLAPI glInterleavedArrays (GLenum format, GLsizei stride, const GLvoid *pointer);
extern void (GLAPIENTRY* qglNormalPointer)(GLenum type, GLsizei stride, const GLvoid *pointer, const char *filename, int line);
#define			 xglNormalPointer(type, stride, pointer) \
			 qglNormalPointer(type, stride, pointer, __FILE__, __LINE__)

extern void (GLAPIENTRY* qglTexCoordPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer, const char *filename, int line);
#define			 xglTexCoordPointer(size, type, stride, pointer) \
			 xglTexCoordPointer(size, type, stride, pointer, __FILE__, __LINE__)

extern void (GLAPIENTRY* qglVertexPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer, const char *filename, int line);
#define			 xglVertexPointer(size, type, stride, pointer) \
			 qglVertexPointer(size, type, stride, pointer, __FILE__, __LINE__)

extern void (GLAPIENTRY* qglPolygonOffset)(GLfloat factor, GLfloat units, const char *filename, int line);
#define			 xglPolygonOffset(factor, units) \
			 qglPolygonOffset(factor, units, __FILE__, __LINE__)

//void GLAPI glCopyTexImage1D (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
extern void (GLAPIENTRY* qglCopyTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border, const char *filename, int line);
#define			 xglCopyTexImage2D(target, level, internalformat, x, y, width, height, border) \
			 qglCopyTexImage2D(target, level, internalformat, x, y, width, height, border, __FILE__, __LINE__)

//void GLAPI glCopyTexSubImage1D (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
extern void (GLAPIENTRY* qglCopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height, const char *filename, int line);
#define			 xglCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height) \
			 qglCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height, __FILE__, __LINE__)
			 
//void GLAPI glTexSubImage1D (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
//void GLAPI glTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
//GLboolean GLAPI glAreTexturesResident (GLsizei n, const GLuint *textures, GLboolean *residences);
extern void (GLAPIENTRY* qglBindTexture)(GLenum target, GLuint texture, const char *filename, int line);
#define			 xglBindTexture(target, texture) \
			 qglBindTexture(target, texture, __FILE__, __LINE__)

extern void (GLAPIENTRY* qglDeleteTextures)(GLsizei n, const GLuint *textures, const char *filename, int line);
#define			 xglDeleteTextures(n, textures) \
			 qglDeleteTextures(n, textures, __FILE__, __LINE__)

extern void (GLAPIENTRY* qglGenTextures)(GLsizei n, GLuint *textures, const char *filename, int line);
#define			 xglGenTextures(n, textures) \
			 qglGenTextures(n, textures, __FILE__, __LINE__)

//GLboolean GLAPI glIsTexture (GLuint texture);
//void GLAPI glPrioritizeTextures (GLsizei n, const GLuint *textures, const GLclampf *priorities);
//void GLAPI glIndexub (GLubyte c);
//void GLAPI glIndexubv (const GLubyte *c);
//void GLAPI glPopClientAttrib (void);
//void GLAPI glPushClientAttrib (GLbitfield mask);


/// OpenGL 1.2 functions =======================================================
//void GLAPI glBlendColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
//void GLAPI glBlendEquation (GLenum mode);
//extern void (GLAPIENTRY* qglDrawRangeElements)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices, const char *filename, int line);
//define			 xglDrawRangeElements(mode, start, end, count, type, indices) 
//			 qglDrawRangeElements(mode, start, end, count, type, indices, __FILE__, __LINE__)
			 
//void GLAPI glColorTable (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table);
//void GLAPI glColorTableParameterfv (GLenum target, GLenum pname, const GLfloat *params);
//void GLAPI glColorTableParameteriv (GLenum target, GLenum pname, const GLint *params);
//void GLAPI glCopyColorTable (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
//void GLAPI glGetColorTable (GLenum target, GLenum format, GLenum type, GLvoid *table);
//void GLAPI glGetColorTableParameterfv (GLenum target, GLenum pname, GLfloat *params);
//void GLAPI glGetColorTableParameteriv (GLenum target, GLenum pname, GLint *params);
//void GLAPI glColorSubTable (GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data);
//void GLAPI glCopyColorSubTable (GLenum target, GLsizei start, GLint x, GLint y, GLsizei width);
//void GLAPI glConvolutionFilter1D (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *image);
//void GLAPI glConvolutionFilter2D (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image);
//void GLAPI glConvolutionParameterf (GLenum target, GLenum pname, GLfloat params);
//void GLAPI glConvolutionParameterfv (GLenum target, GLenum pname, const GLfloat *params);
//void GLAPI glConvolutionParameteri (GLenum target, GLenum pname, GLint params);
//void GLAPI glConvolutionParameteriv (GLenum target, GLenum pname, const GLint *params);
//void GLAPI glCopyConvolutionFilter1D (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
//void GLAPI glCopyConvolutionFilter2D (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height);
//void GLAPI glGetConvolutionFilter (GLenum target, GLenum format, GLenum type, GLvoid *image);
//void GLAPI glGetConvolutionParameterfv (GLenum target, GLenum pname, GLfloat *params);
//void GLAPI glGetConvolutionParameteriv (GLenum target, GLenum pname, GLint *params);
//void GLAPI glGetSeparableFilter (GLenum target, GLenum format, GLenum type, GLvoid *row, GLvoid *column, GLvoid *span);
//void GLAPI glSeparableFilter2D (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *row, const GLvoid *column);
//void GLAPI glGetHistogram (GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values);
//void GLAPI glGetHistogramParameterfv (GLenum target, GLenum pname, GLfloat *params);
//void GLAPI glGetHistogramParameteriv (GLenum target, GLenum pname, GLint *params);
//void GLAPI glGetMinmax (GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values);
//void GLAPI glGetMinmaxParameterfv (GLenum target, GLenum pname, GLfloat *params);
//void GLAPI glGetMinmaxParameteriv (GLenum target, GLenum pname, GLint *params);
//void GLAPI glHistogram (GLenum target, GLsizei width, GLenum internalformat, GLboolean sink);
//void GLAPI glMinmax (GLenum target, GLenum internalformat, GLboolean sink);
//void GLAPI glResetHistogram (GLenum target);
//void GLAPI glResetMinmax (GLenum target);
//extern void (GLAPIENTRY* qglTexImage3D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels, const char *filename, int line);
//define			 xglTexImage3D(target, level, internalformat, width, height, depth, border, format, type, pixels) 
//			 qglTexImage3D(target, level, internalformat, width, height, depth, border, format, type, pixels, __FILE__, __LINE__)
			 
//void GLAPI glTexSubImage3D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
//void GLAPI glCopyTexSubImage3D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);


/// OpenGL 1.3 functions =======================================================
//extern void (GLAPIENTRY* qglActiveTexture)(GLenum texture, const char *filename, int line);
//define			 xglActiveTexture(texture) 
//			 qglActiveTexture(texture, __FILE__, __LINE__)

//extern void (GLAPIENTRY* qglClientActiveTexture)(GLenum texture, const char *filename, int line);
//define			 xglClientActiveTexture(texture) 
//			 qglClientActiveTexture(texture, __FILE__, __LINE__)
			 
//void GLAPI glMultiTexCoord1d (GLenum target, GLdouble s);
//void GLAPI glMultiTexCoord1dv (GLenum target, const GLdouble *v);
//void GLAPI glMultiTexCoord1f (GLenum target, GLfloat s);
//void GLAPI glMultiTexCoord1fv (GLenum target, const GLfloat *v);
//void GLAPI glMultiTexCoord1i (GLenum target, GLint s);
//void GLAPI glMultiTexCoord1iv (GLenum target, const GLint *v);
//void GLAPI glMultiTexCoord1s (GLenum target, GLshort s);
//void GLAPI glMultiTexCoord1sv (GLenum target, const GLshort *v);
//void GLAPI glMultiTexCoord2d (GLenum target, GLdouble s, GLdouble t);
//void GLAPI glMultiTexCoord2dv (GLenum target, const GLdouble *v);
//void GLAPI glMultiTexCoord2f (GLenum target, GLfloat s, GLfloat t);
//void GLAPI glMultiTexCoord2fv (GLenum target, const GLfloat *v);
//void GLAPI glMultiTexCoord2i (GLenum target, GLint s, GLint t);
//void GLAPI glMultiTexCoord2iv (GLenum target, const GLint *v);
//void GLAPI glMultiTexCoord2s (GLenum target, GLshort s, GLshort t);
//void GLAPI glMultiTexCoord2sv (GLenum target, const GLshort *v);
//void GLAPI glMultiTexCoord3d (GLenum target, GLdouble s, GLdouble t, GLdouble r);
//void GLAPI glMultiTexCoord3dv (GLenum target, const GLdouble *v);
//void GLAPI glMultiTexCoord3f (GLenum target, GLfloat s, GLfloat t, GLfloat r);
//void GLAPI glMultiTexCoord3fv (GLenum target, const GLfloat *v);
//void GLAPI glMultiTexCoord3i (GLenum target, GLint s, GLint t, GLint r);
//void GLAPI glMultiTexCoord3iv (GLenum target, const GLint *v);
//void GLAPI glMultiTexCoord3s (GLenum target, GLshort s, GLshort t, GLshort r);
//void GLAPI glMultiTexCoord3sv (GLenum target, const GLshort *v);
//void GLAPI glMultiTexCoord4d (GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
//void GLAPI glMultiTexCoord4dv (GLenum target, const GLdouble *v);
//void GLAPI glMultiTexCoord4f (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
//void GLAPI glMultiTexCoord4fv (GLenum target, const GLfloat *v);
//void GLAPI glMultiTexCoord4i (GLenum target, GLint s, GLint t, GLint r, GLint q);
//void GLAPI glMultiTexCoord4iv (GLenum target, const GLint *v);
//void GLAPI glMultiTexCoord4s (GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
//void GLAPI glMultiTexCoord4sv (GLenum target, const GLshort *v);
//extern void (GLAPIENTRY* qglLoadTransposeMatrixf)(const GLfloat *m, const char *filename, int line);
//define			 xglLoadTransposeMatrixf(m) 
//			 qglLoadTransposeMatrixf(m, __FILE__, __LINE__)
			 
//void GLAPI glLoadTransposeMatrixd (const GLdouble *m);
//void GLAPI glMultTransposeMatrixf (const GLfloat *m);
//void GLAPI glMultTransposeMatrixd (const GLdouble *m);
//void GLAPI glCompressedTexImage3D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data);
//void GLAPI glCompressedTexImage2D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
//void GLAPI glCompressedTexImage1D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data);
//void GLAPI glCompressedTexSubImage3D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data);
//void GLAPI glCompressedTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
//void GLAPI glCompressedTexSubImage1D (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data);
//void GLAPI glGetCompressedTexImage (GLenum target, GLint lod, GLvoid *img);


/// OpenGL 1.4 functions =======================================================
//void GLAPI glSampleCoverage (GLclampf value, GLboolean invert);
//void GLAPI glPointParameterf (GLenum pname, GLfloat param);
//void GLAPI glPointParameterfv (GLenum pname, const GLfloat *params);
//void GLAPI glPointParameteri (GLenum pname, GLint param);
//void GLAPI glPointParameteriv (GLenum pname, const GLint *params);
//void GLAPI glWindowPos2d (GLdouble x, GLdouble y);
//void GLAPI glWindowPos2f (GLfloat x, GLfloat y);
//void GLAPI glWindowPos2i (GLint x, GLint y);
//void GLAPI glWindowPos2s (GLshort x, GLshort y);
//void GLAPI glWindowPos2dv (const GLdouble *p);
//void GLAPI glWindowPos2fv (const GLfloat *p);
//void GLAPI glWindowPos2iv (const GLint *p);
//void GLAPI glWindowPos2sv (const GLshort *p);
//void GLAPI glWindowPos3d (GLdouble x, GLdouble y, GLdouble z);
//void GLAPI glWindowPos3f (GLfloat x, GLfloat y, GLfloat z);
//void GLAPI glWindowPos3i (GLint x, GLint y, GLint z);
//void GLAPI glWindowPos3s (GLshort x, GLshort y, GLshort z);
//void GLAPI glWindowPos3dv (const GLdouble *p);
//void GLAPI glWindowPos3fv (const GLfloat *p);
//void GLAPI glWindowPos3iv (const GLint *p);
//void GLAPI glWindowPos3sv (const GLshort *p);
//void GLAPI glBlendFuncSeparate (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
//void GLAPI glFogCoordd (GLdouble fog);
//void GLAPI glFogCoorddv (const GLdouble *fog);
//void GLAPI glFogCoordf (GLfloat fog);
//void GLAPI glFogCoordfv (const GLfloat *fog);
//void GLAPI glFogCoordPointer (GLenum type, GLsizei stride, const GLvoid *pointer);
//void GLAPI glMultiDrawArrays (GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount);
//void GLAPI glMultiDrawElements (GLenum mode, const GLsizei *count, GLenum type, const GLvoid* *indices, GLsizei primcount);
//void GLAPI glSecondaryColor3b (GLbyte red, GLbyte green, GLbyte blue);
//void GLAPI glSecondaryColor3bv (const GLbyte *v);
//void GLAPI glSecondaryColor3d (GLdouble red, GLdouble green, GLdouble blue);
//void GLAPI glSecondaryColor3dv (const GLdouble *v);
//void GLAPI glSecondaryColor3f (GLfloat red, GLfloat green, GLfloat blue);
//void GLAPI glSecondaryColor3fv (const GLfloat *v);
//void GLAPI glSecondaryColor3i (GLint red, GLint green, GLint blue);
//void GLAPI glSecondaryColor3iv (const GLint *v);
//void GLAPI glSecondaryColor3s (GLshort red, GLshort green, GLshort blue);
//void GLAPI glSecondaryColor3sv (const GLshort *v);
//void GLAPI glSecondaryColor3ub (GLubyte red, GLubyte green, GLubyte blue);
//void GLAPI glSecondaryColor3ubv (const GLubyte *v);
//void GLAPI glSecondaryColor3ui (GLuint red, GLuint green, GLuint blue);
//void GLAPI glSecondaryColor3uiv (const GLuint *v);
//void GLAPI glSecondaryColor3us (GLushort red, GLushort green, GLushort blue);
//void GLAPI glSecondaryColor3usv (const GLushort *v);
//void GLAPI glSecondaryColorPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);


/// GL_ARB_multitexture ========================================================
extern void (GLAPIENTRY* qglActiveTextureARB)(GLenum texture, const char *filename, int line);
#define			 xglActiveTextureARB(texture) \
			 qglActiveTextureARB(texture, __FILE__, __LINE__)

extern void (GLAPIENTRY* qglClientActiveTextureARB)(GLenum texture, const char *filename, int line);
#define			 xglClientActiveTextureARB(texture) \
			 qglClientActiveTextureARB(texture, __FILE__, __LINE__)


/// GL_ARB_transpose_matrix ====================================================
extern void (GLAPIENTRY* qglLoadTransposeMatrixfARB)(const GLfloat *m, const char *filename, int line);
#define			 xglLoadTransposeMatrixfARB(m) \
			 qglLoadTransposeMatrixfARB(m, __FILE__, __LINE__)


/// GL_ARB_vertex_program ======================================================
extern void (GLAPIENTRY* xglVertexAttribPointerARB) (GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid *);
extern void (GLAPIENTRY* xglEnableVertexAttribArrayARB) (GLuint);
extern void (GLAPIENTRY* xglDisableVertexAttribArrayARB) (GLuint);
extern void (GLAPIENTRY* xglProgramStringARB) (GLenum, GLenum, GLsizei, const GLvoid *);
extern void (GLAPIENTRY* xglBindProgramARB) (GLenum, GLuint);
extern void (GLAPIENTRY* xglDeleteProgramsARB) (GLsizei, const GLuint *);
extern void (GLAPIENTRY* xglGenProgramsARB) (GLsizei, GLuint *);
extern void (GLAPIENTRY* xglProgramEnvParameter4fARB) (GLenum, GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
extern void (GLAPIENTRY* xglProgramEnvParameter4fvARB) (GLenum, GLuint, const GLfloat *);
extern void (GLAPIENTRY* xglProgramLocalParameter4fARB) (GLenum, GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
extern void (GLAPIENTRY* xglProgramLocalParameter4fvARB) (GLenum, GLuint, const GLfloat *);
extern void (GLAPIENTRY* xglGetProgramEnvParameterfvARB) (GLenum, GLuint, GLfloat *);
extern void (GLAPIENTRY* xglGetProgramLocalParameterfvARB) (GLenum, GLuint, GLfloat *);
extern void (GLAPIENTRY* xglGetProgramivARB) (GLenum, GLenum, GLint *);
extern void (GLAPIENTRY* xglGetProgramStringARB) (GLenum, GLenum, GLvoid *);
extern void (GLAPIENTRY* xglGetVertexAttribfvARB) (GLuint, GLenum, GLfloat *);
extern void (GLAPIENTRY* xglGetVertexAttribPointervARB) (GLuint, GLenum, GLvoid* *);
extern GLboolean (GLAPIENTRY* xglIsProgramARB) (GLuint);


/// GL_ARB_vertex_buffer_object ================================================
typedef ptrdiff_t GLintptrARB;
typedef ptrdiff_t GLsizeiptrARB;

extern void (GLAPIENTRY* xglBindBufferARB) (GLenum, GLuint);
extern void (GLAPIENTRY* xglDeleteBuffersARB) (GLsizei, const GLuint *);
extern void (GLAPIENTRY* xglGenBuffersARB) (GLsizei, GLuint *);
extern GLboolean (GLAPIENTRY* xglIsBufferARB) (GLuint);
extern void (GLAPIENTRY* xglBufferDataARB) (GLenum, GLsizeiptrARB, const GLvoid *, GLenum);
extern void (GLAPIENTRY* xglBufferSubDataARB) (GLenum, GLintptrARB, GLsizeiptrARB, const GLvoid *);
extern void (GLAPIENTRY* xglGetBufferSubDataARB) (GLenum, GLintptrARB, GLsizeiptrARB, GLvoid *);
extern GLvoid* (GLAPIENTRY* xglMapBufferARB) (GLenum, GLenum);
extern GLboolean (GLAPIENTRY* xglUnmapBufferARB) (GLenum);
extern void (GLAPIENTRY* xglGetBufferParameterivARB) (GLenum, GLenum, GLint *);
extern void (GLAPIENTRY* xglGetBufferPointervARB) (GLenum, GLenum, GLvoid* *);


/// GL_ARB_occlusion_query =====================================================
extern void (GLAPIENTRY* xglGenQueriesARB) (GLsizei, GLuint *);
extern void (GLAPIENTRY* xglDeleteQueriesARB) (GLsizei, const GLuint *);
extern GLboolean (GLAPIENTRY* xglIsQueryARB) (GLuint);
extern void (GLAPIENTRY* xglBeginQueryARB) (GLenum, GLuint);
extern void (GLAPIENTRY* xglEndQueryARB) (GLenum);
extern void (GLAPIENTRY* xglGetQueryivARB) (GLenum, GLenum, GLint *);
extern void (GLAPIENTRY* xglGetQueryObjectivARB) (GLuint, GLenum, GLint *);
extern void (GLAPIENTRY* xglGetQueryObjectuivARB) (GLuint, GLenum, GLuint *);


/// GL_ARB_shader_objects ======================================================
typedef char GLcharARB;			/* native character */
typedef unsigned int GLhandleARB;	/* shader object handle */

extern void (GLAPIENTRY* xglDeleteObjectARB) (GLhandleARB);
extern GLhandleARB (GLAPIENTRY* xglGetHandleARB) (GLenum);
extern void (GLAPIENTRY* xglDetachObjectARB) (GLhandleARB, GLhandleARB);
extern GLhandleARB (GLAPIENTRY* xglCreateShaderObjectARB) (GLenum);
extern void (GLAPIENTRY* xglShaderSourceARB) (GLhandleARB, GLsizei, const GLcharARB* *, const GLint *);
extern void (GLAPIENTRY* xglCompileShaderARB) (GLhandleARB);
extern GLhandleARB (GLAPIENTRY* xglCreateProgramObjectARB) (void);
extern void (GLAPIENTRY* xglAttachObjectARB) (GLhandleARB, GLhandleARB);
extern void (GLAPIENTRY* xglLinkProgramARB) (GLhandleARB);
extern void (GLAPIENTRY* xglUseProgramObjectARB) (GLhandleARB);
extern void (GLAPIENTRY* xglValidateProgramARB) (GLhandleARB);
extern void (GLAPIENTRY* xglUniform1fARB) (GLint, GLfloat);
extern void (GLAPIENTRY* xglUniform2fARB) (GLint, GLfloat, GLfloat);
extern void (GLAPIENTRY* xglUniform3fARB) (GLint, GLfloat, GLfloat, GLfloat);
extern void (GLAPIENTRY* xglUniform4fARB) (GLint, GLfloat, GLfloat, GLfloat, GLfloat);
extern void (GLAPIENTRY* xglUniform1iARB) (GLint, GLint);
extern void (GLAPIENTRY* xglUniform2iARB) (GLint, GLint, GLint);
extern void (GLAPIENTRY* xglUniform3iARB) (GLint, GLint, GLint, GLint);
extern void (GLAPIENTRY* xglUniform4iARB) (GLint, GLint, GLint, GLint, GLint);
extern void (GLAPIENTRY* xglUniform1fvARB) (GLint, GLsizei, const GLfloat *);
extern void (GLAPIENTRY* xglUniform2fvARB) (GLint, GLsizei, const GLfloat *);
extern void (GLAPIENTRY* xglUniform3fvARB) (GLint, GLsizei, const GLfloat *);
extern void (GLAPIENTRY* xglUniform4fvARB) (GLint, GLsizei, const GLfloat *);
extern void (GLAPIENTRY* xglUniform1ivARB) (GLint, GLsizei, const GLint *);
extern void (GLAPIENTRY* xglUniform2ivARB) (GLint, GLsizei, const GLint *);
extern void (GLAPIENTRY* xglUniform3ivARB) (GLint, GLsizei, const GLint *);
extern void (GLAPIENTRY* xglUniform4ivARB) (GLint, GLsizei, const GLint *);
extern void (GLAPIENTRY* xglUniformMatrix2fvARB) (GLint, GLsizei, GLboolean, const GLfloat *);
extern void (GLAPIENTRY* xglUniformMatrix3fvARB) (GLint, GLsizei, GLboolean, const GLfloat *);
extern void (GLAPIENTRY* xglUniformMatrix4fvARB) (GLint, GLsizei, GLboolean, const GLfloat *);
extern void (GLAPIENTRY* xglGetObjectParameterfvARB) (GLhandleARB, GLenum, GLfloat *);
extern void (GLAPIENTRY* xglGetObjectParameterivARB) (GLhandleARB, GLenum, GLint *);
extern void (GLAPIENTRY* xglGetInfoLogARB) (GLhandleARB, GLsizei, GLsizei *, GLcharARB *);
extern void (GLAPIENTRY* xglGetAttachedObjectsARB) (GLhandleARB, GLsizei, GLsizei *, GLhandleARB *);
extern GLint (GLAPIENTRY* xglGetUniformLocationARB) (GLhandleARB, const GLcharARB *);
extern void (GLAPIENTRY* xglGetActiveUniformARB) (GLhandleARB, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLcharARB *);
extern void (GLAPIENTRY* xglGetUniformfvARB) (GLhandleARB, GLint, GLfloat *);
extern void (GLAPIENTRY* xglGetUniformivARB) (GLhandleARB, GLint, GLint *);
extern void (GLAPIENTRY* xglGetShaderSourceARB) (GLhandleARB, GLsizei, GLsizei *, GLcharARB *);


/// GL_ARB_vertex_shader =======================================================
extern void (GLAPIENTRY* xglBindAttribLocationARB) (GLhandleARB, GLuint, const GLcharARB *);
extern void (GLAPIENTRY* xglGetActiveAttribARB) (GLhandleARB, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLcharARB *);
extern GLint (GLAPIENTRY* xglGetAttribLocationARB) (GLhandleARB, const GLcharARB *);


/// GL_EXT_texture3D ===========================================================
extern void (GLAPIENTRY* qglTexImage3DEXT)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels, const char *filename, int line);
#define			 xglTexImage3D(target, level, internalformat, width, height, depth, border, format, type, pixels) \
			 qglTexImage3D(target, level, internalformat, width, height, depth, border, format, type, pixels, __FILE__, __LINE__)


/// GL_EXT_draw_range_elements =================================================
extern void (GLAPIENTRY* qglDrawRangeElementsEXT)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices, const char *filename, int line);
#define			 xglDrawRangeElementsEXT(mode, start, end, count, type, indices) \
			 qglDrawRangeElementsEXT(mode, start, end, count, type, indices, __FILE__, __LINE__)


/// GL_EXT_compiled_vertex_array ===============================================
extern void (GLAPIENTRY* xglLockArraysEXT) (GLint, GLsizei);
extern void (GLAPIENTRY* xglUnlockArraysEXT) (void);


/// GL_NV_vertex_program2 ======================================================
extern GLboolean (GLAPIENTRY* xglAreProgramsResidentNV) (GLsizei, const GLuint *, GLboolean *);
extern void (GLAPIENTRY* xglBindProgramNV) (GLenum, GLuint);
extern void (GLAPIENTRY* xglDeleteProgramsNV) (GLsizei, const GLuint *);
extern void (GLAPIENTRY* xglExecuteProgramNV) (GLenum, GLuint, const GLfloat *);
extern void (GLAPIENTRY* xglGenProgramsNV) (GLsizei, GLuint *);
extern void (GLAPIENTRY* xglGetProgramParameterdvNV) (GLenum, GLuint, GLenum, GLdouble *);
extern void (GLAPIENTRY* xglGetProgramParameterfvNV) (GLenum, GLuint, GLenum, GLfloat *);
extern void (GLAPIENTRY* xglGetProgramivNV) (GLuint, GLenum, GLint *);
extern void (GLAPIENTRY* xglGetProgramStringNV) (GLuint, GLenum, GLubyte *);
extern void (GLAPIENTRY* xglGetTrackMatrixivNV) (GLenum, GLuint, GLenum, GLint *);
extern void (GLAPIENTRY* xglGetVertexAttribdvNV) (GLuint, GLenum, GLdouble *);
extern void (GLAPIENTRY* xglGetVertexAttribfvNV) (GLuint, GLenum, GLfloat *);
extern void (GLAPIENTRY* xglGetVertexAttribivNV) (GLuint, GLenum, GLint *);
extern void (GLAPIENTRY* xglGetVertexAttribPointervNV) (GLuint, GLenum, GLvoid* *);
extern GLboolean (GLAPIENTRY* xglIsProgramNV) (GLuint);
extern void (GLAPIENTRY* xglLoadProgramNV) (GLenum, GLuint, GLsizei, const GLubyte *);
extern void (GLAPIENTRY* xglProgramParameter4fNV) (GLenum, GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
extern void (GLAPIENTRY* xglProgramParameter4fvNV) (GLenum, GLuint, const GLfloat *);
extern void (GLAPIENTRY* xglProgramParameters4fvNV) (GLenum, GLuint, GLuint, const GLfloat *);
extern void (GLAPIENTRY* xglRequestResidentProgramsNV) (GLsizei, const GLuint *);
extern void (GLAPIENTRY* xglTrackMatrixNV) (GLenum, GLuint, GLenum, GLenum);
extern void (GLAPIENTRY* xglVertexAttribPointerNV) (GLuint, GLint, GLenum, GLsizei, const GLvoid *);
extern void (GLAPIENTRY* xglVertexAttrib1fNV) (GLuint, GLfloat);
extern void (GLAPIENTRY* xglVertexAttrib1fvNV) (GLuint, const GLfloat *);
extern void (GLAPIENTRY* xglVertexAttrib2fNV) (GLuint, GLfloat, GLfloat);
extern void (GLAPIENTRY* xglVertexAttrib2fvNV) (GLuint, const GLfloat *);
extern void (GLAPIENTRY* xglVertexAttrib3fNV) (GLuint, GLfloat, GLfloat, GLfloat);
extern void (GLAPIENTRY* xglVertexAttrib3fvNV) (GLuint, const GLfloat *);
extern void (GLAPIENTRY* xglVertexAttrib4fNV) (GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
extern void (GLAPIENTRY* xglVertexAttrib4fvNV) (GLuint, const GLfloat *);
extern void (GLAPIENTRY* xglVertexAttribs1fvNV) (GLuint, GLsizei, const GLfloat *);
extern void (GLAPIENTRY* xglVertexAttribs2fvNV) (GLuint, GLsizei, const GLfloat *);
extern void (GLAPIENTRY* xglVertexAttribs3fvNV) (GLuint, GLsizei, const GLfloat *);
extern void (GLAPIENTRY* xglVertexAttribs4fvNV) (GLuint, GLsizei, const GLfloat *);


/// GL_NV_fragment_program =====================================================
extern void (GLAPIENTRY* xglProgramNamedParameter4fNV) (GLuint, GLsizei, const GLubyte *, GLfloat, GLfloat, GLfloat, GLfloat);
extern void (GLAPIENTRY* xglProgramNamedParameter4fvNV) (GLuint, GLsizei, const GLubyte *, const GLfloat *);
extern void (GLAPIENTRY* xglGetProgramNamedParameterfvNV) (GLuint, GLsizei, const GLubyte *, GLfloat *);


#ifdef __linux__
/// GLX 1.0 functions ==========================================================
extern XVisualInfo* 	(*xglXChooseVisual)(Display *dpy, int screen, int *attribList);
extern void 		(*xglXCopyContext)(Display *dpy, GLXContext src, GLXContext dst, GLuint mask);
extern GLXContext	(*xglXCreateContext)(Display *dpy, XVisualInfo *vis, GLXContext shareList, Bool direct);
extern void 		(*xglXDestroyContext)(Display *dpy, GLXContext ctx);
extern GLXDrawable	(*xglXGetCurrentDrawable)(void);
extern Bool 		(*xglXMakeCurrent)(Display *dpy, GLXDrawable drawable, GLXContext ctx);
extern int		(*xglXGetConfig)(Display *dpy, XVisualInfo *vis, int attrib, int *value);
extern GLXContext	(*xglXGetCurrentContext)(void);
extern Bool 		(*xglXQueryExtension)(Display *dpy, int *error_base, int *event_base);
extern Bool		(*xglXQueryVersion)(Display *dpy, int *major, int *minor);
extern void 		(*xglXSwapBuffers)(Display *dpy, GLXDrawable drawable);

/// GLX 1.1 functions ==========================================================
extern const char*	(*xglXQueryExtensionsString)(Display *dpy, int screen);

/// GLX 1.2 functions ==========================================================
extern Display*		(*xglXGetCurrentDisplay)(void);

/// GLX 1.3 functions ==========================================================
extern GLXFBConfig*	(*xglXGetFBConfigs)(Display *dpy, int screen, int *nelements);

/// ARB_get_proc_address functions =============================================
extern void*		(*xglXGetProcAddressARB)(const GLubyte *);

/// SGIX_fbconfig functions ====================================================
extern GLXFBConfigSGIX*	(*xglXChooseFBConfigSGIX)(Display *dpy, int screen, const int *attrib_list, int *nelements);
extern GLXContext	(*xglXCreateContextWithConfigSGIX)(Display *dpy, GLXFBConfigSGIX config, int render_type, GLXContext share_list, Bool direct);

/// SGIX_pbuffer functions =====================================================
extern GLXPbuffer	(*xglXCreateGLXPbufferSGIX)(Display *dpy, GLXFBConfig config, unsigned int width, unsigned int height, const int *attrib_list);
extern void		(*xglXQueryGLXPbufferSGIX)(Display *dpy, GLXPbuffer pbuf, int attribute, unsigned int *value);

#endif	// __linux__

#ifdef _WIN32
/// WGL functions =============================================================
extern BOOL		(*xwglMakeCurrent)(HDC, HGLRC);
extern BOOL		(*xwglDeleteContext)(HGLRC);
extern HGLRC		(*xwglCreateContext)(HDC);
#endif // _WIN32

/// GL Tools ==================================================================
bool		XGL_Init(const char *dllname);
void		XGL_InitExtensions();
void    	XGL_Shutdown();
void*		XGL_GetSymbol(const char *symbolname);
void		XGL_EnableDebugging(bool enable);

void		XGL_CheckForError_(const std::string &file, int line);

#if 1 //DEBUG
#define		XGL_CheckForError()	XGL_CheckForError_(__FILE__, __LINE__)
#else
#define		XGL_CheckForError()
#endif

#define		RB_CheckForError()

// interface for wgl/glxGetProcAddress, use this to load OpenGL extensions
void*		xglGetProcAddress(const char *symbolname);


#endif	// R_GL_H




