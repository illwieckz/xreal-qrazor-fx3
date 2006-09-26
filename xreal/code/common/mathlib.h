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
#ifndef __MATHLIB__
#define __MATHLIB__

// mathlib.h

#include <math.h>

#ifdef DOUBLEVEC_T
typedef double  vec_t;
#else
typedef float   vec_t;
#endif
typedef vec_t   vec2_t[3];
typedef vec_t   vec3_t[3];
typedef vec_t   vec4_t[4];

typedef vec_t   matrix_t[16];

// angle indexes
#define	PITCH				0	// up / down
#define	YAW					1	// left / right
#define	ROLL				2	// fall over

// plane sides
#define	SIDE_FRONT		0
#define	SIDE_ON			2
#define	SIDE_BACK		1
#define	SIDE_CROSS		-2

#define	Q_PI	3.14159265358979323846
#define DEG2RAD( a ) ( ( (a) * Q_PI ) / 180.0F )
#define RAD2DEG( a ) ( ( (a) * 180.0f ) / Q_PI )

extern vec3_t   vec3_origin;

#define	EQUAL_EPSILON	0.001

// plane types are used to speed some tests
// 0-2 are axial planes
#define	PLANE_X			0
#define	PLANE_Y			1
#define	PLANE_Z			2
#define	PLANE_NON_AXIAL	3

int             VectorCompare(const vec3_t v1, const vec3_t v2);

#define VectorSet(v,x,y,z) {v[0]=x;v[1]=y;v[2]=z;}
#define DotProduct(x,y) (x[0]*y[0]+x[1]*y[1]+x[2]*y[2])
#define VectorSubtract(a,b,c) {c[0]=a[0]-b[0];c[1]=a[1]-b[1];c[2]=a[2]-b[2];}
#define VectorAdd(a,b,c) {c[0]=a[0]+b[0];c[1]=a[1]+b[1];c[2]=a[2]+b[2];}
#define VectorCopy(a,b) {b[0]=a[0];b[1]=a[1];b[2]=a[2];}
#define VectorScale(a,b,c) {c[0]=b*a[0];c[1]=b*a[1];c[2]=b*a[2];}
#define VectorClear(x) {x[0] = x[1] = x[2] = 0;}
#define	VectorNegate(x) {x[0]=-x[0];x[1]=-x[1];x[2]=-x[2];}
void            Vec10Copy(vec_t * in, vec_t * out);

vec_t           Q_rint(vec_t in);
vec_t           _DotProduct(vec3_t v1, vec3_t v2);
void            _VectorSubtract(vec3_t va, vec3_t vb, vec3_t out);
void            _VectorAdd(vec3_t va, vec3_t vb, vec3_t out);
void            _VectorCopy(vec3_t in, vec3_t out);
void            _VectorScale(vec3_t v, vec_t scale, vec3_t out);

double          VectorLength(const vec3_t v);

void            VectorMA(const vec3_t va, double scale, const vec3_t vb, vec3_t vc);

void            CrossProduct(const vec3_t v1, const vec3_t v2, vec3_t cross);
vec_t           VectorNormalize(const vec3_t in, vec3_t out);
vec_t           ColorNormalize(const vec3_t in, vec3_t out);
void            VectorInverse(vec3_t v);

void            ClearBounds(vec3_t mins, vec3_t maxs);
void            AddPointToBounds(const vec3_t v, vec3_t mins, vec3_t maxs);

qboolean        PlaneFromPoints(vec4_t plane, const vec3_t a, const vec3_t b, const vec3_t c, qboolean cw);

void            NormalToLatLong(const vec3_t normal, byte bytes[2]);

int             PlaneTypeForNormal(vec3_t normal);
void            AxisMultiply(float in1[3][3], float in2[3][3], float out[3][3]);
void            RotatePointAroundVector(vec3_t dst, const vec3_t dir, const vec3_t point, float degrees);

void            MatrixIdentity(matrix_t m);
void            MatrixClear(matrix_t m);
void            MatrixCopy(const matrix_t in, matrix_t out);
void            MatrixTranspose(const matrix_t in, matrix_t out);
void            MatrixSetupXRotation(matrix_t m, vec_t degrees);
void            MatrixSetupYRotation(matrix_t m, vec_t degrees);
void            MatrixSetupZRotation(matrix_t m, vec_t degrees);
void            MatrixSetupRotation(matrix_t m, vec_t x, vec_t y, vec_t z, vec_t degrees);
void            MatrixSetupTranslation(matrix_t m, vec_t x, vec_t y, vec_t z);
void            MatrixsetupScale(matrix_t m, vec_t x, vec_t y, vec_t z);
void            MatrixMultiply(const matrix_t a, const matrix_t b, matrix_t out);
void            MatrixMultiplyRotation(matrix_t m, vec_t pitch, vec_t yaw, vec_t roll);
void            MatrixMultiplyTranslation(matrix_t m, vec_t x, vec_t y, vec_t z);
void            MatrixMultiplyScale(matrix_t m, vec_t x, vec_t y, vec_t z);
void            MatrixFromAngles(matrix_t m, vec_t pitch, vec_t yaw, vec_t roll);
void            MatrixFromVectorsFLU(matrix_t m, const vec3_t forward, const vec3_t left, const vec3_t up);
void            MatrixFromVectorsFRU(matrix_t m, const vec3_t forward, const vec3_t right, const vec3_t up);
void            MatrixToVectorsFLU(const matrix_t m, vec3_t forward, vec3_t left, vec3_t up);
void            MatrixToVectorsFRU(const matrix_t m, vec3_t forward, vec3_t right, vec3_t up);

void            MatrixSetupTransform(matrix_t m, const vec3_t forward, const vec3_t left, const vec3_t up, const vec3_t origin);
void            MatrixSetupTransformFromRotation(matrix_t m, const matrix_t rot, const vec3_t origin);
void            MatrixAffineInverse(const matrix_t in, matrix_t out);
void            MatrixTransformNormal(const matrix_t m, const vec3_t in, vec3_t out);
void            MatrixTransformPoint(const matrix_t m, const vec3_t in, vec3_t out);

#endif
