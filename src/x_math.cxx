/// ============================================================================
/*
Copyright (C) 1997-2001 Id Software, Inc.				(Quake2)
Copyright (C) 2000 Vasiliou Nikos email <nbasili@ceid.upatras.gr>	(U3D)
Copyright (C) 2000-2002 by Iain Nicholson <iain@delphinus.demon.co.uk>	(libSIMD)
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
// shared -------------------------------------------------------------------
#include "x_shared.h"
// qrazor-fx ----------------------------------------------------------------
// xreal --------------------------------------------------------------------

#ifdef DOUBLEVEC_T
const vec_t		X_infinity = std::numeric_limits<double>::infinity();
#else
const vec_t		X_infinity = std::numeric_limits<float>::infinity();
#endif

const vec3_c		vec3_origin(REAL(0.0), REAL(0.0), REAL(0.0));


const matrix_c		matrix_identity(	REAL(1.0), REAL(0.0), REAL(0.0), REAL(0.0), 
						REAL(0.0), REAL(1.0), REAL(0.0), REAL(0.0),
						REAL(0.0), REAL(0.0), REAL(1.0), REAL(0.0),
						REAL(0.0), REAL(0.0), REAL(0.0), REAL(1.0)	);
						
const quaternion_c	quat_identity(0, 0, 0, 1);


const vec4_c	color_black	(0, 0, 0, 1);
const vec4_c	color_red	(1, 0, 0, 1);
const vec4_c	color_green	(0, 1, 0, 1);
const vec4_c	color_blue	(0, 0, 1, 1);
const vec4_c	color_yellow	(1, 1, 0, 1);
const vec4_c	color_magenta	(1, 0, 1, 1);
const vec4_c	color_cyan	(0, 1, 1, 1);
const vec4_c	color_white	(1, 1, 1, 1);
const vec4_c	color_grey_lite	(0.75, 0.75, 0.75, 1);
const vec4_c	color_grey_med	(0.5, 0.5, 0.5, 1);
const vec4_c	color_grey_dark	(0.25, 0.25, 0.25, 1);


#if defined(__GNUC__) && defined(SIMD_3DNOW)
inline void	femms()
{
	asm volatile("femms");
}
#endif

// reciprocal
vec_t	X_recip(vec_t in)
{
#if defined(__GNUC__) && !defined(DOUBLEVEC_T) && defined(SIMD_3DNOW)
	vec_t out;
	femms();
	asm volatile
	(
	"movd		(%%eax),	%%mm0\n"
	
	"pfrcp		%%mm0,		%%mm1\n"	// (approx)
	"pfrcpit1	%%mm1,		%%mm0\n"	// (intermediate)
	"pfrcpit2	%%mm1,		%%mm0\n"	// (full 24-bit)
	// out = mm0[low]
	"movd		%%mm0,		(%%edx)\n"
	:
	: "a"(&in), "d"(&out)
	: "memory"
	);
	femms();
	return out;
#elif defined(DOUBLEVEC_T)
	return (1.0/(in));
#else
	return ((float)(1.0f/(in)));
#endif
}

// square root
vec_t	X_sqrt(vec_t in)
{
#if defined(__GNUC__) && !defined(DOUBLEVEC_T) && defined(SIMD_3DNOW)
// Tr3B - see 3DNow! Technology Manual p.62
	vec_t out;
	femms();
	asm volatile
	(						// lo				| hi
	"movd		(%%eax),	%%mm0\n"	// in				| -
	
	"pfrsqrt	%%mm0,		%%mm1\n"	// 1/sqrt(in)			| 1/sqrt(in)	(approx)
	"movq		%%mm1,		%%mm2\n"	// 1/sqrt(in)			| 1/sqrt(in)	(approx)
	"pfmul		%%mm1,		%%mm1\n"	// (1/sqrt(in))�		| (1/sqrt(in))�	step 1
//	"punpckldq	%%mm0,		%%mm0\n"	// in				| in		(MMX instruction)
	"pfrsqit1	%%mm0,		%%mm1\n"	// intermediate					step 2
	"pfrcpit2	%%mm2,		%%mm1\n"	// 1/sqrt(in) (full 24-bit precision)		step 3
	"pfmul		%%mm1,		%%mm0\n"	// sqrt(in)
	
	"movd		%%mm0,		(%%edx)\n"
	:
	: "a"(&in), "d"(&out)
	: "memory"
	);
	femms();
	return out;
#elif defined(DOUBLEVEC_T)
	return sqrt(in);
#else
	return ((float)sqrtf(float(in)));
#endif
}

// reciprocal square root
vec_t	X_recipsqrt(vec_t in)
{
#if defined(__GNUC__) && !defined(DOUBLEVEC_T) && defined(SIMD_3DNOW)
	vec_t out;
	femms();
	asm volatile
	(
	"movd		(%%eax),	%%mm0\n"	// 0		|	a
	
	"pfrsqrt	%%mm0,		%%mm1\n"	// 1/sqrt(in)			| 1/sqrt(in)	(approx)
	"movq		%%mm1,		%%mm2\n"	// 1/sqrt(in)			| 1/sqrt(in)	(approx)
	"pfmul		%%mm1,		%%mm1\n"	// (1/sqrt(in))�		| (1/sqrt(in))�	step 1
	"pfrsqit1	%%mm0,		%%mm1\n"	// intermediate					step 2
	"pfrcpit2	%%mm2,		%%mm1\n"	// 1/sqrt(in) (full 24-bit precision)		step 3
	
	"movd		%%mm1,		(%%edx)\n"
	:
	: "a"(&in), "d"(&out)
	: "memory"
	);
	femms();
	return out;
#elif defined(DOUBLEVEC_T)
	return (1.0/sqrt(in));
#else
	return ((float)(1.0f/sqrtf(float(in))));
#endif
}

vec_t	vec3_c::length() const
{
#if defined(__GNUC__) && !defined(DOUBLEVEC_T) && defined(SIMD_3DNOW)
	vec_t out;
	femms();
	asm volatile
	(						// lo						| hi
	"movq		(%%eax),	%%mm0\n"	// _v[0]					| _v[1]
	"movd		8(%%eax),	%%mm1\n"	// _v[2]					| -
	// mm0[lo] = dot product(this)
	"pfmul		%%mm0,		%%mm0\n"	// _v[0]*_v[0]					| _v[1]*_v[1]
	"pfmul		%%mm1,		%%mm1\n"	// _v[2]*_v[2]					| -
	"pfacc		%%mm0,		%%mm0\n"	// _v[0]*v._v[0]+_v[1]*v._v[1]			| -
	"pfadd		%%mm1,		%%mm0\n"	// _v[0]*v._v[0]+_v[1]*v._v[1]+_v[2]*v._v[2]	| -
	// mm0[lo] = sqrt(mm0[lo])
	"pfrsqrt	%%mm0,		%%mm1\n"	// 1/sqrt(dot)					| 1/sqrt(dot)		(approx)
	"movq		%%mm1,		%%mm2\n"	// 1/sqrt(dot)					| 1/sqrt(dot)		(approx)
	"pfmul		%%mm1,		%%mm1\n"	// (1/sqrt(dot))�				| (1/sqrt(dot))�	step 1
	"pfrsqit1	%%mm0,		%%mm1\n"	// intermediate								step 2
	"pfrcpit2	%%mm2,		%%mm1\n"	// 1/sqrt(dot) (full 24-bit precision)					step 3
	"pfmul		%%mm1,		%%mm0\n"	// sqrt(dot)
	// out = mm0[lo]
	"movd		%%mm0,		(%%edx)\n"
	:
	: "a"(&_v[0]), "d"(&out)
	: "memory"
	);
	femms();
	return out;
#else
	return X_sqrt(_v[0]*_v[0] + _v[1]*_v[1] + _v[2]*_v[2]);
#endif
}

vec_t	vec3_c::normalize()
{
#if defined(__GNUC__) && !defined(DOUBLEVEC_T) && defined(SIMD_3DNOW)
	vec_t len;
	femms();
	asm volatile
	(						// lo						| hi
	"movq		(%%eax),	%%mm0\n"	// _v[0]					| _v[1]
	"movd		8(%%eax),	%%mm1\n"	// _v[2]					| -
	// mm0[lo] = dot product(this)
	"pfmul		%%mm0,		%%mm0\n"	// _v[0]*_v[0]					| _v[1]*_v[1]
	"pfmul		%%mm1,		%%mm1\n"	// _v[2]*_v[2]					| -
	"pfacc		%%mm0,		%%mm0\n"	// _v[0]*v._v[0]+_v[1]*v._v[1]			| -
	"pfadd		%%mm1,		%%mm0\n"	// _v[0]*v._v[0]+_v[1]*v._v[1]+_v[2]*v._v[2]	| -
	// mm0[lo] = sqrt(mm0[lo])
	"pfrsqrt	%%mm0,		%%mm1\n"	// 1/sqrt(dot)					| 1/sqrt(dot)		(approx)
	"movq		%%mm1,		%%mm2\n"	// 1/sqrt(dot)					| 1/sqrt(dot)		(approx)
	"pfmul		%%mm1,		%%mm1\n"	// (1/sqrt(dot))�				| (1/sqrt(dot))�	step 1
	"punpckldq	%%mm0,		%%mm0\n"	// dot						| dot			(MMX instruction)
	"pfrsqit1	%%mm0,		%%mm1\n"	// intermediate					| intermediate		step 2
	"pfrcpit2	%%mm2,		%%mm1\n"	// 1/sqrt(dot) (full 24-bit precision)		| 1/sqrt(dot)		step 3
	"pfmul		%%mm1,		%%mm0\n"	// sqrt(dot)					| sqrt(dot)
	// len = mm0[lo]
	"movd		%%mm0,		(%%edx)\n"
	// load this into registers
	"movq		(%%eax),	%%mm2\n"	// _v[0]					| _v[1]
	"movd		8(%%eax),	%%mm3\n"	// _v[2]					| -
	// scale this by the reciprocal square root
	"pfmul		%%mm1,		%%mm2\n"	// _v[0]*1/sqrt(dot)				| _v[1]*1/sqrt(dot)
	"pfmul		%%mm1,		%%mm3\n"	// _v[2]*1/sqrt(dot)				| -
	// store scaled vector
	"movq		%%mm2,		(%%eax)\n"
	"movd		%%mm3,		8(%%eax)\n"
	:
	: "a"(&_v[0]), "d"(&len)
	: "memory"
	);
	femms();
	return len;
#else
	vec_t len = length();
	
	if(len)
	{
		vec_t ilen = X_recip(len);
		
		_v[0] *= ilen;
		_v[1] *= ilen;
		_v[2] *= ilen;
	}
	
	return len;
#endif
}

vec_t	vec3_c::dotProduct(const vec3_c &v) const
{
#if defined(__GNUC__) && !defined(DOUBLEVEC_T) && defined(SIMD_3DNOW)
	vec_t out;
	femms();
	asm volatile
	(						// lo						| hi
	"movq		(%%eax),	%%mm0\n"	// _v[0]					| _v[1]
	"movq		(%%edx),	%%mm2\n"	// v._v[0]					| v._v[1]
	"movd		8(%%eax),	%%mm1\n"	// _v[2]					| -
	"movd		8(%%edx),	%%mm3\n"	// v._v[2]					| -
		
	"pfmul		%%mm2,		%%mm0\n"	// _v[0]*v._v[0]				| _v[1]*v._v[1]
	"pfmul		%%mm3,		%%mm1\n"	// _v[2]*v._v[2]				| -
	"pfacc		%%mm0,		%%mm0\n"	// _v[0]*v._v[0]+_v[1]*v._v[1]			| -
	"pfadd		%%mm1,		%%mm0\n"	// _v[0]*v._v[0]+_v[1]*v._v[1]+_v[2]*v._v[2]	| -
	
	"movd		%%mm0,		(%%ecx)\n"	// out = mm2[lo]
	:
	: "a"(&_v[0]), "d"(&v._v[0]), "c"(&out)
	: "memory"
	);
	femms();
	return out;
#else
	return  (_v[0]*v._v[0] + _v[1]*v._v[1] + _v[2]*v._v[2]);
#endif
}

vec3_c&	vec3_c::crossProduct(const vec3_c &v1, const vec3_c &v2)
{
	_v[0] = v1._v[1]*v2._v[2] - v1._v[2]*v2._v[1];
	_v[1] = v1._v[2]*v2._v[0] - v1._v[0]*v2._v[2];
	_v[2] = v1._v[0]*v2._v[1] - v1._v[1]*v2._v[0];
	
	return *this;
}

void	vec3_c::snap()
{
	for(int i=0; i<3; i++)
	{
		if(X_fabs(_v[i] - 1) < NORMAL_EPSILON)
		{
			clear();
			_v[i] = 1;
			break;
		}
			
		if(X_fabs(_v[i] - -1) < NORMAL_EPSILON)
		{
			clear();
			_v[i] = -1;
			break;
		}
	}
}
	
const char*	vec3_c::toString() const
{
	return va("(%i %i %i)", (int)_v[0], (int)_v[1], (int)_v[2]);
}

vec3_c&	vec3_c::operator = (const vec3_c &v)
{
#if defined(__GNUC__) && !defined(DOUBLEVEC_T) && defined(SIMD_3DNOW)
	femms();
	asm volatile
	(						// lo						| hi
	"movq		(%%edx),	%%mm0\n"	// v._v[0]					| v._v[1]
	"movd		8(%%edx),	%%mm1\n"	// v._v[2]					| -
	
	"movq		%%mm0,		(%%eax)\n"
	"movd		%%mm1,		8(%%eax)\n"
	:
	: "a"(&_v[0]), "d"(&v._v[0])
	: "memory"
	);
	femms();
	return *this;
#else
	_v[0] = v._v[0];
	_v[1] = v._v[1];
	_v[2] = v._v[2];
	
	return *this;
#endif
}
		
vec3_c&	vec3_c::operator = (const vec_t *v)
{
#if defined(__GNUC__) && !defined(DOUBLEVEC_T) && defined(SIMD_3DNOW)
	femms();
	asm volatile
	(						// lo						| hi
	"movq		(%%edx),	%%mm0\n"	// v._v[0]					| v._v[1]
	"movd		8(%%edx),	%%mm1\n"	// v._v[2]					| -
	
	"movq		%%mm0,		(%%eax)\n"
	"movd		%%mm1,		8(%%eax)\n"
	:
	: "a"(&_v[0]), "d"(v)
	: "memory"
	);
	femms();
	return *this;
#else
	_v[0] = v[0];
	_v[1] = v[1];
	_v[2] = v[2];
	
	return *this;
#endif
}




matrix_c::matrix_c()
{
	// DO NOTHING AND SAVE SPEED
}
	
matrix_c::matrix_c(	vec_t m00, vec_t m01, vec_t m02, vec_t m03,
			vec_t m10, vec_t m11, vec_t m12, vec_t m13,
			vec_t m20, vec_t m21, vec_t m22, vec_t m23,
			vec_t m30, vec_t m31, vec_t m32, vec_t m33)
{
	_m[0][0]=m00;	_m[0][1]=m01;	_m[0][2]=m02;	_m[0][3]=m03;
	_m[1][0]=m10;	_m[1][1]=m11;	_m[1][2]=m12;	_m[1][3]=m13;
	_m[2][0]=m20;	_m[2][1]=m21;	_m[2][2]=m22;	_m[2][3]=m23;
	_m[3][0]=m30;	_m[3][1]=m31;	_m[3][2]=m32;	_m[3][3]=m33;
}
	
void	matrix_c::identity()
{
	_m[0][0]=1.0f;	_m[0][1]=0.0f;	_m[0][2]=0.0f;	_m[0][3]=0.0f;
	_m[1][0]=0.0f;	_m[1][1]=1.0f;	_m[1][2]=0.0f;	_m[1][3]=0.0f;
	_m[2][0]=0.0f;	_m[2][1]=0.0f;	_m[2][2]=1.0f;	_m[2][3]=0.0f;
	_m[3][0]=0.0f;	_m[3][1]=0.0f;	_m[3][2]=0.0f;	_m[3][3]=1.0f;
}
	
void	matrix_c::zero()
{
	_m[0][0]=0.0f;	_m[0][1]=0.0f;	_m[0][2]=0.0f;	_m[0][3]=0.0f;
	_m[1][0]=0.0f;	_m[1][1]=0.0f;	_m[1][2]=0.0f;	_m[1][3]=0.0f;
	_m[2][0]=0.0f;	_m[2][1]=0.0f;	_m[2][2]=0.0f;	_m[2][3]=0.0f;
	_m[3][0]=0.0f;	_m[3][1]=0.0f;	_m[3][2]=0.0f;	_m[3][3]=0.0f;
}
	
void	matrix_c::copyTo(matrix_c &out) const
{
	out._m[0][0]=_m[0][0];	out._m[0][1]=_m[0][1];	out._m[0][2]=_m[0][2];	out._m[0][3]=_m[0][3];
	out._m[1][0]=_m[1][0];	out._m[1][1]=_m[1][1];	out._m[1][2]=_m[1][2];	out._m[1][3]=_m[1][3];
	out._m[2][0]=_m[2][0];	out._m[2][1]=_m[2][1];	out._m[2][2]=_m[2][2];	out._m[2][3]=_m[2][3];
	out._m[3][0]=_m[3][0];	out._m[3][1]=_m[3][1];	out._m[3][2]=_m[3][2];	out._m[3][3]=_m[3][3];
}
	
void	matrix_c::copyRotateOnly(matrix_c &out) const
{
	out._m[0][0]=_m[0][0];	out._m[0][1]=_m[0][1];	out._m[0][2]=_m[0][2];	out._m[0][3]=0.0f;
	out._m[1][0]=_m[1][0];	out._m[1][1]=_m[1][1];	out._m[1][2]=_m[1][2];	out._m[1][3]=0.0f;
	out._m[2][0]=_m[2][0];	out._m[2][1]=_m[2][1];	out._m[2][2]=_m[2][2];	out._m[2][3]=0.0f;
	out._m[3][0]=_m[3][0];	out._m[3][1]=_m[3][1];	out._m[3][2]=_m[3][2];	out._m[3][3]=1.0f;
}
	
void	matrix_c::copyTranslateOnly(matrix_c &out) const
{
	out._m[0][0]=1.0f;	out._m[0][1]=0.0f;	out._m[0][2]=0.0f;	out._m[0][3]=_m[0][3];
	out._m[1][0]=0.0f;	out._m[1][1]=1.0f;	out._m[1][2]=0.0f;	out._m[1][3]=_m[1][3];
	out._m[2][0]=0.0f;	out._m[2][1]=0.0f;	out._m[2][2]=1.0f;	out._m[2][3]=_m[2][3];
	out._m[3][0]=0.0f;	out._m[3][1]=0.0f;	out._m[3][2]=0.0f;	out._m[3][3]=1.0f;
}
	
void	matrix_c::copyTranspose(matrix_c &out) const
{
	out._m[0][0]=_m[0][0];	out._m[0][1]=_m[1][0];	out._m[0][2]=_m[2][0];	out._m[0][3]=_m[3][0];
	out._m[1][0]=_m[0][1];	out._m[1][1]=_m[1][1];	out._m[1][2]=_m[2][1];	out._m[1][3]=_m[3][1];
	out._m[2][0]=_m[0][2];	out._m[2][1]=_m[1][2];	out._m[2][2]=_m[2][2];	out._m[2][3]=_m[3][2];
	out._m[3][0]=_m[0][3];	out._m[3][1]=_m[1][3];	out._m[3][2]=_m[2][3];	out._m[3][3]=_m[3][3];
}

void	matrix_c::transpose()
{
	/*
					std::swap(_m[0][1], _m[1][0]);	std::swap(_m[0][2], _m[2][0]);	std::swap(_m[0][3], _m[3][0]);
	std::swap(_m[1][0], _m[0][1]);					std::swap(_m[1][2], _m[2][1]);	std::swap(_m[1][3], _m[3][1]);
	std::swap(_m[2][0], _m[0][2]);	std::swap(_m[2][1], _m[1][2]);					std::swap(_m[2][3], _m[3][2]);
	std::swap(_m[3][0], _m[0][3]);	std::swap(_m[3][1], _m[1][3]);	std::swap(_m[3][2], _m[2][3]);
	*/
	
					std::swap(_m[0][1], _m[1][0]);	std::swap(_m[0][2], _m[2][0]);	std::swap(_m[0][3], _m[3][0]);
									std::swap(_m[1][2], _m[2][1]);	std::swap(_m[1][3], _m[3][1]);
													std::swap(_m[2][3], _m[3][2]);
}
	
void	matrix_c::setupXRotation(vec_t deg)
{
	vec_t a = DEGTORAD(deg);
	
	_m[0][0]=1.0f;	_m[0][1]=0.0f;		_m[0][2]=0.0f;		_m[0][3]=0.0f;
	_m[1][0]=0.0f;	_m[1][1]=cos(a);	_m[1][2]=-sin(a);	_m[1][3]=0.0f;
	_m[2][0]=0.0f;	_m[2][1]=sin(a);	_m[2][2]= cos(a);	_m[2][3]=0.0f;
	_m[3][0]=0.0f;	_m[3][1]=0.0f;		_m[3][2]=0.0f;		_m[3][3]=1.0f;
}

void	matrix_c::setupYRotation(vec_t deg)
{
	vec_t a = DEGTORAD(deg);

	_m[0][0]=cos(a);	_m[0][1]=0.0f;		_m[0][2]=sin(a);	_m[0][3]=0.0f;	
	_m[1][0]=0.0f;		_m[1][1]=1.0f;		_m[1][2]=0.0f;		_m[1][3]=0.0f;
	_m[2][0]=-sin(a);	_m[2][1]=0.0f;		_m[2][2]=cos(a);	_m[2][3]=0.0f;
	_m[3][0]=0.0f;		_m[3][1]=0.0f;		_m[3][2]=0.0f;		_m[3][3]=1.0f;
}

void	matrix_c::setupZRotation(vec_t deg)
{
	vec_t a = DEGTORAD(deg);
	
	_m[0][0]=cos(a);	_m[0][1]=-sin(a);	_m[0][2]=0.0f;		_m[0][3]=0.0f;
	_m[1][0]=sin(a);	_m[1][1]= cos(a);	_m[1][2]=0.0f;		_m[1][3]=0.0f;
	_m[2][0]=0.0f;		_m[2][1]=0.0f;		_m[2][2]=1.0f;		_m[2][3]=0.0f;
	_m[3][0]=0.0f;		_m[3][1]=0.0f;		_m[3][2]=0.0f;		_m[3][3]=1.0f;
}

void	matrix_c::setupRotation(vec_t x, vec_t y, vec_t z, vec_t deg)
{
	vec_t len, c, s;
	
	len = x*x+y*y+z*z;
	if(len != 0.0f)
		len = 1.0f / (vec_t)X_sqrt(len);
	x *= len;
	y *= len;
	z *= len;

	deg *= (vec_t)(-M_PI / 180.0);
	c = (vec_t)cos(deg);
	s = (vec_t)sin(deg);

	_m[0][0]=x*x + c*(1 - x*x);	_m[0][1]=x*y * (1 - c) + z*s;	_m[0][2]=z*x * (1 - c) - y*s;	_m[0][3]=0.0f;
	_m[1][0]=x*y * (1 - c) - z*s;	_m[1][1]=y*y + c * (1 - y*y);	_m[1][2]=y*z * (1 - c) + x*s;	_m[1][3]=0.0f;
	_m[2][0]=z*x * (1 - c) + y*s;	_m[2][1]=y*z * (1 - c) - x*s;	_m[2][2]=z*z + c * (1 - z*z);	_m[2][3]=0.0f;
	_m[3][0]=0.0f;			_m[3][1]=0.0f;			_m[3][2]=0.0f;			_m[3][3]=1.0f;
}
	
void	matrix_c::setupTranslation(vec_t x, vec_t y, vec_t z)
{
	_m[0][0]=1;	_m[0][1]=0;	_m[0][2]=0;	_m[0][3]=x;
	_m[1][0]=0;	_m[1][1]=1;	_m[1][2]=0;	_m[1][3]=y;
	_m[2][0]=0;	_m[2][1]=0;	_m[2][2]=1;	_m[2][3]=z;
	_m[3][0]=0;	_m[3][1]=0;	_m[3][2]=0;	_m[3][3]=1;
}
	
void	matrix_c::setupScale(vec_t x, vec_t y, vec_t z)
{
	_m[0][0]=x;	_m[0][1]=0;	_m[0][2]=0;	_m[0][3]=0;
	_m[1][0]=0;	_m[1][1]=y;	_m[1][2]=0;	_m[1][3]=0;
	_m[2][0]=0;	_m[2][1]=0;	_m[2][2]=z;	_m[2][3]=0;
	_m[3][0]=0;	_m[3][1]=0;	_m[3][2]=0;	_m[3][3]=1;
}

void	matrix_c::fromAngles(vec_t pitch, vec_t yaw, vec_t roll)
{
	double	sr, sp, sy, cr, cp, cy;
		
	sp = X_sin(DEGTORAD(pitch));
	cp = X_cos(DEGTORAD(pitch));
	
	sy = X_sin(DEGTORAD(yaw));
	cy = X_cos(DEGTORAD(yaw));
	
	sr = X_sin(DEGTORAD(roll));
	cr = X_cos(DEGTORAD(roll));

	_m[0][0]=(vec_t)(cp*cy);	_m[0][1]=(vec_t)(sr*sp*cy+cr*-sy);	_m[0][2]=(vec_t)(cr*sp*cy+-sr*-sy);	_m[0][3]= 0;
	_m[1][0]=(vec_t)(cp*sy);	_m[1][1]=(vec_t)(sr*sp*sy+cr*cy);	_m[1][2]=(vec_t)(cr*sp*sy+-sr*cy);	_m[1][3]= 0;
	_m[2][0]=(vec_t)(-sp);		_m[2][1]=(vec_t)(sr*cp);		_m[2][2]=(vec_t)(cr*cp);		_m[2][3]= 0;
	_m[3][0]= 0;			_m[3][1]= 0;				_m[3][2]= 0;				_m[3][3]= 1;
}

/*
void	matrix_c::fromEulerAngles(vec_t phi, vec_t theta, vec_t psi)
{
	vec_t sphi,cphi,stheta,ctheta,spsi,cpsi;
	
	sphi = X_sin(phi);
	cphi = X_cos(phi);
	
	stheta = X_sin(theta);
	ctheta = X_cos(theta);
	
	spsi = X_sin(psi);
	cpsi = X_cos(psi);
	
	_m[0][0]=cpsi*ctheta;			_m[0][1]=spsi*ctheta;			_m[0][2]=-stheta;		_m[0][3]=0.0f;
	_m[1][0]=cpsi*stheta*sphi - spsi*cphi;	_m[1][1]=spsi*stheta*sphi + cpsi*cphi;	_m[1][2]=ctheta*sphi;		_m[1][3]=0.0f;
	_m[2][0]=cpsi*stheta*cphi + spsi*sphi;	_m[2][1]=spsi*stheta*cphi - cpsi*sphi;	_m[2][2]=ctheta*cphi;		_m[2][3]=0.0f;
	_m[3][0]=0.0f;				_m[3][1]=0.0f;				_m[3][2]=0.0f;			_m[3][3]=1.0f;
}
*/
	
void	matrix_c::fromVectorsFLU(const vec3_c &forward, const vec3_c &left, const vec3_c &up)
{
	_m[0][0]=forward[0];	_m[0][1]=left[0];	_m[0][2]=up[0];	_m[0][3]=0; 
	_m[1][0]=forward[0];	_m[1][1]=left[0];	_m[1][2]=up[0];	_m[1][3]=0;
	_m[2][0]=forward[0];	_m[2][1]=left[0];	_m[2][2]=up[0];	_m[2][3]=0;
	_m[3][0]=0;		_m[3][1]=0;		_m[3][2]=0;	_m[3][3]=1;
}
	
void	matrix_c::toVectorsFLU(vec3_c &forward, vec3_c &left, vec3_c &up) const
{
	forward[0] = _m[0][0];	// cp*cy;
	forward[1] = _m[1][0];	// cp*sy;
	forward[2] = _m[2][0];	//-sp;
		
	left[0] = _m[0][1];	// sr*sp*cy+cr*-sy;
	left[1] = _m[1][1];	// sr*sp*sy+cr*cy;
	left[2] = _m[2][1];	// sr*cp;

	up[0] = _m[0][2];	// cr*sp*cy+-sr*-sy;
	up[1] = _m[1][2];	// cr*sp*sy+-sr*cy;
	up[2] = _m[2][2];	// cr*cp;
}
	
void	matrix_c::fromVectorsFRU(const vec3_c &forward, const vec3_c &right, const vec3_c &up)
{
	_m[0][0]=forward[0];	_m[0][1]=-right[0];	_m[0][2]=up[0];	_m[0][3]=0;
	_m[1][0]=forward[0];	_m[1][1]=-right[0];	_m[1][2]=up[0];	_m[1][3]=0;
	_m[2][0]=forward[0];	_m[2][1]=-right[0];	_m[2][2]=up[0];	_m[2][3]=0;
	_m[3][0]=0;		_m[3][1]=0;		_m[3][2]=0;	_m[3][3]=1;
}
	
void	matrix_c::toVectorsFRU(vec3_c &forward, vec3_c &right, vec3_c &up) const
{
	forward[0] = _m[0][0];	// cp*cy;
	forward[1] = _m[1][0];	// cp*sy;
	forward[2] = _m[2][0];	//-sp;
		
	right[0] = -_m[0][1];	//-sr*sp*cy+cr*-sy;
	right[1] = -_m[1][1];	//-sr*sp*sy+cr*cy;
	right[2] = -_m[2][1];	//-sr*cp;

	up[0] = _m[0][2];	// cr*sp*cy+-sr*-sy;
	up[1] = _m[1][2];	// cr*sp*sy+-sr*cy;
	up[2] = _m[2][2];	// cr*cp;
}
	
void	matrix_c::fromQuaternion(const quaternion_c &q)
{
	/*
	Assuming that a quaternion has been created in the form:

	Q = |X Y Z W|
	
	Then the quaternion can then be converted into a 4x4 row major rotation
	matrix using the following expression
	
	
         �        2     2                                      �

         � 1 - (2Y  + 2Z )   2XY - 2ZW         2XZ + 2YW       �

         �                                                     �

         �                          2     2                    �

     M = � 2XY + 2ZW         1 - (2X  + 2Z )   2YZ - 2XW       �

         �                                                     �

         �                                            2     2  �

         � 2XZ - 2YW         2YZ + 2XW         1 - (2X  + 2Y ) �

         �                                                     �

	If a 4x4 matrix is required, then the bottom row and right-most column
	may be added.
	*/
	
	/*
	dReal qq1 = 2*q[0]*q[0];
	dReal qq2 = 2*q[1]*q[1];
	dReal qq3 = 2*q[2]*q[2];
	
	_R(0,0) = 1 - qq2 - qq3;		_R(0,1) = 2*(q[0]*q[1] - q[3]*q[2]);	_R(0,2) = 2*(q[0]*q[2] + q[3]*q[1]);
	_R(1,0) = 2*(q[0]*q[1] + q[3]*q[2]);	_R(1,1) = 1 - qq1 - qq3;		_R(1,2) = 2*(q[1]*q[2] - q[3]*q[0]);
	_R(2,0) = 2*(q[0]*q[2] - q[3]*q[1]);	_R(2,1) = 2*(q[1]*q[2] + q[3]*q[0]);	_R(2,2) = 1 - qq1 - qq2;
	*/
	
	double xx = q._q[0] * q._q[0];
	double xy = q._q[0] * q._q[1];
	double xz = q._q[0] * q._q[2];
	double xw = q._q[0] * q._q[3];
	
	double yy = q._q[1] * q._q[1];
	double yz = q._q[1] * q._q[2];
	double yw = q._q[1] * q._q[3];
	
	double zz = q._q[2] * q._q[2];
	double zw = q._q[2] * q._q[3];

	_m[0][0]=1-2*(yy+zz);	_m[0][1]=  2*(xy-zw);	_m[0][2]=  2*(xz+yw);	_m[0][3]=0;
	_m[1][0]=  2*(xy+zw);	_m[1][1]=1-2*(xx+zz);	_m[1][2]=  2*(yz-xw);	_m[1][3]=0;
	_m[2][0]=  2*(xz-yw);	_m[2][1]=  2*(yz+xw);	_m[2][2]=1-2*(xx+yy);	_m[2][3]=0;
	_m[3][0]=0;		_m[3][1]=0;		_m[3][2]=0;		_m[3][3]=1;
}

void	matrix_c::from2Axes(vec_t ax, vec_t ay, vec_t az, vec_t bx, vec_t by, vec_t bz)
{
	vec_t	l,k;
	  
	l = X_sqrt(ax*ax + ay*ay + az*az);
	if(l <= REAL(0.0))
	{
		Com_Printf("matrix_c::from2Axes: zero length vector");
		return;
	}
	
	l = X_recip(l);
	ax *= l;
	ay *= l;
	az *= l;
	
	k = ax*bx + ay*by + az*bz;
	
	bx -= k*ax;
	by -= k*ay;
	bz -= k*az;
	
	l = X_sqrt(bx*bx + by*by + bz*bz);
	if(l <= REAL(0.0))
	{
		Com_Printf("matrix_c::from2Axes: zero length vector");
		return;
	}
	
	l = X_recip(l);
	bx *= l;
	by *= l;
	bz *= l;
	
	_m[0][0] = ax;		_m[0][1] = bx;		_m[0][2] = - by*az + ay*bz;	_m[0][3] = REAL(0.0);
	_m[1][0] = ay;		_m[1][1] = by;		_m[1][2] = - bz*ax + az*bx;	_m[1][3] = REAL(0.0);
	_m[2][0] = az;		_m[2][1] = bz;		_m[2][2] = - bx*ay + ax*by;	_m[2][3] = REAL(0.0);
	_m[3][0] = REAL(0.0);	_m[3][1] = REAL(0.0);	_m[3][2] = REAL(0.0);		_m[3][3] = REAL(1.0);
}
	
void	matrix_c::fromZAxis(vec_t ax, vec_t ay, vec_t az)
{
	vec3_c n(ax, ay, az), p, q;
	
	n.normalize();
	
	Vector3_PlaneSpace(n, p, q);
	
	_m[0][0] = p[0];	_m[0][1] = q[0];	_m[0][2] = n[0];	_m[0][3] = REAL(0.0);
	_m[1][0] = p[1];	_m[1][1] = q[1];	_m[1][2] = n[1];	_m[1][3] = REAL(0.0);
	_m[2][0] = p[2];	_m[2][1] = q[2];	_m[2][2] = n[2];	_m[2][3] = REAL(0.0);
	_m[3][0] = REAL(0.0);	_m[3][1] = REAL(0.0);	_m[3][2] = REAL(0.0);	_m[3][3] = REAL(1.0);
}
	
void	matrix_c::lerp(const matrix_c &old, const matrix_c &nu, vec_t f)
{
	_m[0][0] = nu._m[0][0] + (old._m[0][0] - nu._m[0][0]) * f;
	_m[0][1] = nu._m[0][1] + (old._m[0][1] - nu._m[0][1]) * f;
	_m[0][2] = nu._m[0][2] + (old._m[0][2] - nu._m[0][2]) * f;
	_m[0][3] = nu._m[0][3] + (old._m[0][3] - nu._m[0][3]) * f;
		
	_m[1][0] = nu._m[1][0] + (old._m[1][0] - nu._m[1][0]) * f;
	_m[1][1] = nu._m[1][1] + (old._m[1][1] - nu._m[1][1]) * f;
	_m[1][2] = nu._m[1][2] + (old._m[1][2] - nu._m[1][2]) * f;
	_m[1][3] = nu._m[1][3] + (old._m[1][3] - nu._m[1][3]) * f;
		
	_m[2][0] = nu._m[2][0] + (old._m[2][0] - nu._m[2][0]) * f;
	_m[2][1] = nu._m[2][1] + (old._m[2][1] - nu._m[2][1]) * f;
	_m[2][2] = nu._m[2][2] + (old._m[2][2] - nu._m[2][2]) * f;
	_m[2][3] = nu._m[2][3] + (old._m[2][3] - nu._m[2][3]) * f;
		
	_m[3][0] = nu._m[3][0] + (old._m[3][0] - nu._m[3][0]) * f;
	_m[3][1] = nu._m[3][1] + (old._m[3][1] - nu._m[3][1]) * f;
	_m[3][2] = nu._m[3][2] + (old._m[3][2] - nu._m[3][2]) * f;
	_m[3][3] = nu._m[3][3] + (old._m[3][3] - nu._m[3][3]) * f;
}
	
// Tr3B - recoded from Tenebrae2 into row major style
matrix_c	matrix_c::affineInverse() const
{
	matrix_c out;
		
	// The rotational part of the matrix is simply the transpose of the original matrix.
	out[0][0]=_m[0][0];	out[0][1]=_m[1][0];	out[0][2]=_m[2][0];
	out[1][0]=_m[0][1];	out[1][1]=_m[1][1];	out[1][2]=_m[2][1];
	out[2][0]=_m[0][2];	out[2][1]=_m[1][2];	out[2][2]=_m[2][2];
	out[3][0] = 0;		out[3][1] = 0;		out[3][2] = 0;		out[3][3] = 1;
		
	// The translation components of the original matrix.
	vec_t tx = _m[0][3];
	vec_t ty = _m[1][3];
	vec_t tz = _m[2][3];
		
	// Rresult = -(Tm * Rm) to get the translation part of the inverse
	out[0][3] = -(_m[0][0]*tx + _m[1][0]*ty + _m[2][0]*tz);
	out[1][3] = -(_m[0][1]*tx + _m[1][1]*ty + _m[2][1]*tz);
	out[2][3] = -(_m[0][2]*tx + _m[1][2]*ty + _m[2][2]*tz);
	
	return out;
}
	
	
const char*	matrix_c::toString() const
{
#if 0
		std::string s;
		s  = _m[0].toString(); s += '\n';
		s += _m[1].toString(); s += '\n';
		s += _m[2].toString(); s += '\n';
		s += _m[3].toString();
		return s.c_str();
#else
		std::string s;
		s  = va("(%i %i %i %i)\n", (int)_m[0][0], (int)_m[0][1], (int)_m[0][2], (int)_m[0][3]);
		s += va("(%i %i %i %i)\n", (int)_m[1][0], (int)_m[1][1], (int)_m[1][2], (int)_m[1][3]);
		s += va("(%i %i %i %i)\n", (int)_m[2][0], (int)_m[2][1], (int)_m[2][2], (int)_m[2][3]);
		s += va("(%i %i %i %i)",   (int)_m[3][0], (int)_m[3][1], (int)_m[3][2], (int)_m[3][3]);
		return s.c_str();
#endif
}
		
bool	matrix_c::operator == (const matrix_c &m) const
{
#if 1
	return	(	_m[0][0]==m._m[0][0] && _m[0][1]==m._m[0][1] && _m[0][2]==m._m[0][2] && _m[0][3]==m._m[0][3] &&
			_m[1][0]==m._m[1][0] && _m[1][1]==m._m[1][1] && _m[1][2]==m._m[1][2] && _m[1][3]==m._m[1][3] &&
			_m[2][0]==m._m[2][0] && _m[2][1]==m._m[2][1] && _m[2][2]==m._m[2][2] && _m[2][3]==m._m[2][3] &&
			_m[3][0]==m._m[3][0] && _m[3][1]==m._m[3][1] && _m[3][2]==m._m[3][2] && _m[3][3]==m._m[3][3]	);
#else
	return (memcmp(&_m[0][0], &m._m[0][0], sizeof(m)) == 0);
#endif
}

matrix_c	matrix_c::operator + (const matrix_c &m) const
{
	return matrix_c(	_m[0][0]+m._m[0][0],	_m[0][1]+m._m[0][1],	_m[0][2]+m._m[0][2],	_m[0][3]+m._m[0][3],
				_m[1][0]+m._m[1][0],	_m[1][1]+m._m[1][1],	_m[1][2]+m._m[1][2],	_m[1][3]+m._m[1][3],
				_m[2][0]+m._m[2][0],	_m[2][1]+m._m[2][1],	_m[2][2]+m._m[2][2],	_m[2][3]+m._m[2][3],
				_m[3][0]+m._m[3][0],	_m[3][1]+m._m[3][1],	_m[3][2]+m._m[3][2],	_m[3][3]+m._m[3][3]	);
}

matrix_c	matrix_c::operator - (const matrix_c &m) const
{
	return matrix_c(	_m[0][0]-m._m[0][0],	_m[0][1]-m._m[0][1],	_m[0][2]-m._m[0][2],	_m[0][3]-m._m[0][3],
				_m[1][0]-m._m[1][0],	_m[1][1]-m._m[1][1],	_m[1][2]-m._m[1][2],	_m[1][3]-m._m[1][3],
				_m[2][0]-m._m[2][0],	_m[2][1]-m._m[2][1],	_m[2][2]-m._m[2][2],	_m[2][3]-m._m[2][3],
				_m[3][0]-m._m[3][0],	_m[3][1]-m._m[3][1],	_m[3][2]-m._m[3][2],	_m[3][3]-m._m[3][3]	);
}
	
matrix_c	matrix_c::operator * (const matrix_c &m) const
{
	/*
					std::swap(_m[0][1], _m[1][0]);	std::swap(_m[0][2], _m[2][0]);	std::swap(_m[0][3], _m[3][0]);
									std::swap(_m[1][2], _m[2][1]);	std::swap(_m[1][3], _m[3][1]);
													std::swap(_m[2][3], _m[3][2]);
	*/
	
#if defined(__GNUC__) && !defined(DOUBLEVEC_T) && defined(SIMD_SSE) && 0
	matrix_c out;
	asm volatile
	(						// reg[0]			| reg[1]		| reg[2]		| reg[3]
	// load transpose into XMM registers
	"movlps		(%%eax),	%%xmm4\n"	// _m[0][0]			| _m[0][1]		| -			| -
	"movhps		16(%%eax),	%%xmm4\n"	// _m[0][0]			| _m[0][1]		| _m[1][0]		| _m[1][1]
	"movlps		32(%%eax),	%%xmm3\n"	// _m[2][0]			| _m[2][1]		| -			| -
	"movhps		48(%%eax),	%%xmm3\n"	// _m[2][0]			| _m[2][1]		| _m[3][0]		| _m[3][1]
	
	"movups		%%xmm4,		%%xmm5\n"	// _m[0][0]			| _m[0][1]		| _m[1][0]		| _m[1][1]
	
	"shufps		$0x88, %%xmm3,	%%xmm4\n"	// _m[2][0]			| _m[2][1]		| _m[3][0]		| _m[3][1]
	
	#error TODO matrix_c	matrix_c::operator * (const matrix_c &m) const for SSE
	
	"movups		(%%eax),	%%xmm0\n"	// _m[0][0]			| _m[0][1]		| _m[0][2]		| _m[0][3]
	"movups		16(%%eax),	%%xmm1\n"	// _m[1][0]			| _m[1][1]		| _m[1][2]		| _m[1][3]
	"movups		32(%%eax),	%%xmm2\n"	// _m[2][0]			| _m[2][1]		| _m[2][2]		| _m[2][3]
	"movups		48(%%eax),	%%xmm3\n"	// _m[3][0]			| _m[3][1]		| _m[3][2]		| _m[3][3]

//	"movups		(%%edx),	%%xmm4\n"	// v[0]				| v[1]			| v[2]			| v[3]
	"movss		(%%edx),	%%xmm4\n"	// v[0]				| -			| -			| -
	"movss		4(%%edx),	%%xmm5\n"	// v[1]				| -			| -			| -
	"movss		8(%%edx),	%%xmm6\n"	// v[2]				| -			| -			| -
	"movss		12(%%edx),	%%xmm6\n"	// v[3]				| -			| -			| -
	
//	"shufps		$0x00, %%xmm4,	%%xmm4\n"	// v[0]				| v[0]			| v[0]			| v[0]
//	"shufps		$0x00, %%xmm5,	%%xmm5\n"	// v[1]				| v[1]			| v[1]			| v[1]
//	"shufps		$0x00, %%xmm6,	%%xmm6\n"	// v[2]				| v[2]			| v[2]			| v[2]
	
//	"xorps		%%xmm5,		%%xmm6\n"	// -				| -			| -			| -
	
	"mulps		%%xmm4,		%%xmm0\n"	// _m[0][0]*v[0]		| _m[0][1]*v[1]		| _m[0][2]*v[2]		| _m[0][3]*v[3]
	"mulps		%%xmm5,		%%xmm1\n"	// _m[1][0]*v[0]		| _m[1][1]*v[1]		| _m[1][2]*v[2]		| _m[1][3]*v[3]
	"mulps		%%xmm6,		%%xmm2\n"	// _m[2][0]*v[0]		| _m[2][1]*v[1]		| _m[2][2]*v[2]		| _m[2][3]*v[3]
	"mulps		%%xmm7,		%%xmm3\n"	// _m[3][0]*v[0]		| _m[3][1]*v[1]		| _m[3][2]*v[2]		| _m[3][3]*v[3]
	
	"addps		%%xmm1,		%%xmm0\n"	// _m[0][0]*v[0]+_m[1][0]*v[1]	| _m[0][1]*v[0]+_m[1][1]*v[1]	| _m[0][2]*v[0]+_m[1][2]*v[1]	| _m[0][3]*v[0]+_m[1][3]*v[1]
	"addps		%%xmm2,		%%xmm0\n"	// _m[0][0]*v[0]+_m[1][0]*v[1]+_m[2][0]*v[2]	| _m[0][1]*v[0]+_m[1][1]*v[1]	| _m[0][2]*v[0]+_m[1][2]*v[1]	| _m[0][3]*v[0]+_m[1][3]*v[1]
	"addps		%%xmm3,		%%xmm0\n"
	
	"movups		%%xmm0,		(%%ecx)\n"
	:
	: "a"(&_m[0][0]), "d"(&m._m[0][0]), "c"(&out._m[0][0])
	: "memory"
	);
	return out;
#elif defined(__GNUC__) && !defined(DOUBLEVEC_T) && defined(SIMD_3DNOW)
	matrix_c out;
	femms();
	asm volatile
	(						// lo									| hi
	//
	// calculate first row of out
	//
	
	// copy row 0 of this into registers mm0-mm3
	"movq		(%%eax),	%%mm0\n"	// _m[0][0]								| _m[0][1]
	"movq		%%mm0,		%%mm1\n"	// _m[0][0]								| _m[0][1]
	
	"movq		8(%%eax),	%%mm2\n"	// _m[0][2]								| _m[0][3]
	"movq		%%mm2,		%%mm3\n"	// _m[0][2]								| _m[0][3]
	
	"punpckldq	%%mm0,		%%mm0\n"	// _m[0][0]								| _m[0][0]
	"punpckhdq	%%mm1,		%%mm1\n"	// _m[0][1]								| _m[0][1]
	
	"punpckldq	%%mm2,		%%mm2\n"	// _m[0][2]								| _m[0][2]
	"punpckhdq	%%mm3,		%%mm3\n"	// _m[0][3]								| _m[0][3]
	// copy column 0 and column 1 of m into registers mm4-mm7
	"movq		(%%edx),	%%mm4\n"	// m[0][0]								| m[0][1]
	"movq		16(%%edx),	%%mm5\n"	// m[1][0]								| m[1][1]
	"movq		32(%%edx),	%%mm6\n"	// m[2][0]								| m[2][1]
	"movq		48(%%edx),	%%mm7\n"	// m[3][0]								| m[3][1]
	// calc out[0][0] and out[0][1]	
	"pfmul		%%mm0,		%%mm4\n"	// _m[0][0]*m[0][0]							| _m[0][0]*m[0][1]
	"pfmul		%%mm1,		%%mm5\n"	// _m[0][1]*m[1][0]							| _m[0][1]*m[1][1]
	"pfmul		%%mm2,		%%mm6\n"	// _m[0][2]*m[2][0]							| _m[0][2]*m[2][1]
	"pfmul		%%mm3,		%%mm7\n"	// _m[0][3]*m[3][0]							| _m[0][3]*m[3][1]
	
	"pfadd		%%mm4,		%%mm5\n"	// _m[0][0]*m[0][0]+_m[0][1]*m[1][0]					| _m[0][0]*m[0][1]+_m[0][1]*m[1][1]
	"pfadd		%%mm5,		%%mm6\n"	// _m[0][0]*m[0][0]+_m[0][1]*m[1][0]+_m[0][2]*m[2][0]			| _m[0][0]*m[0][1]+_m[0][1]*m[1][1]+_m[0][2]*m[2][1]
	"pfadd		%%mm6,		%%mm7\n"	// _m[0][0]*m[0][0]+_m[0][1]*m[1][0]+_m[0][2]*m[2][0]+_m[0][3]*m[3][0]	| _m[0][0]*m[0][1]+_m[0][1]*m[1][1]+_m[0][2]*m[2][1]+_m[0][3]*m[3][1]
	// write out[0][0] and out[0][1]
	"movq		%%mm7,		(%%ecx)\n"
	// copy column 2 and column 3 of m into registers mm4-mm7
	"movq		8(%%edx),	%%mm4\n"	// m[0][2]								| m[0][3]
	"movq		24(%%edx),	%%mm5\n"	// m[1][2]								| m[1][3]
	"movq		40(%%edx),	%%mm6\n"	// m[2][2]								| m[2][3]
	"movq		56(%%edx),	%%mm7\n"	// m[3][2]								| m[3][3]
	// calc out[0][2] and out[0][3]	
	"pfmul		%%mm0,		%%mm4\n"	// _m[0][0]*m[0][2]							| _m[0][0]*m[0][3]
	"pfmul		%%mm1,		%%mm5\n"	// _m[0][1]*m[1][2]							| _m[0][1]*m[1][3]
	"pfmul		%%mm2,		%%mm6\n"	// _m[0][2]*m[2][2]							| _m[0][2]*m[2][3]
	"pfmul		%%mm3,		%%mm7\n"	// _m[0][3]*m[3][2]							| _m[0][3]*m[3][3]
	
	"pfadd		%%mm4,		%%mm5\n"	// _m[0][0]*m[0][2]+_m[0][1]*m[1][2]					| _m[0][0]*m[0][3]+_m[0][1]*m[1][3]
	"pfadd		%%mm5,		%%mm6\n"	// _m[0][0]*m[0][2]+_m[0][1]*m[1][2]+_m[0][2]*m[2][2]			| _m[0][0]*m[0][3]+_m[0][1]*m[1][3]+_m[0][2]*m[2][3]
	"pfadd		%%mm6,		%%mm7\n"	// _m[0][0]*m[0][2]+_m[0][1]*m[1][2]+_m[0][2]*m[2][2]+_m[0][3]*m[3][2]	| _m[0][0]*m[0][3]+_m[0][1]*m[1][3]+_m[0][2]*m[2][3]+_m[0][3]*m[3][3]
	// write out[0][2] and out[0][3]
	"movq		%%mm7,		8(%%ecx)\n"
	
	//
	// calculate second row of out
	//
	
	// copy row 1 of this into registers mm0-mm3
	"movq		16(%%eax),	%%mm0\n"	// _m[1][0]								| _m[1][1]
	"movq		%%mm0,		%%mm1\n"	// _m[1][0]								| _m[1][1]
	
	"movq		24(%%eax),	%%mm2\n"	// _m[1][2]								| _m[1][3]
	"movq		%%mm2,		%%mm3\n"	// _m[1][2]								| _m[1][3]
	
	"punpckldq	%%mm0,		%%mm0\n"	// _m[1][0]								| _m[1][0]
	"punpckhdq	%%mm1,		%%mm1\n"	// _m[1][1]								| _m[1][1]
	
	"punpckldq	%%mm2,		%%mm2\n"	// _m[1][2]								| _m[1][2]
	"punpckhdq	%%mm3,		%%mm3\n"	// _m[1][3]								| _m[1][3]
	// copy column 0 and column 1 of m into registers mm4-mm7
	"movq		(%%edx),	%%mm4\n"	// m[0][0]								| m[0][1]
	"movq		16(%%edx),	%%mm5\n"	// m[1][0]								| m[1][1]
	"movq		32(%%edx),	%%mm6\n"	// m[2][0]								| m[2][1]
	"movq		48(%%edx),	%%mm7\n"	// m[3][0]								| m[3][1]
	// calc out[1][0] and out[1][1]	
	"pfmul		%%mm0,		%%mm4\n"	// _m[1][0]*m[0][0]							| _m[1][0]*m[0][1]
	"pfmul		%%mm1,		%%mm5\n"	// _m[1][1]*m[1][0]							| _m[1][1]*m[1][1]
	"pfmul		%%mm2,		%%mm6\n"	// _m[1][2]*m[2][0]							| _m[1][2]*m[2][1]
	"pfmul		%%mm3,		%%mm7\n"	// _m[1][3]*m[3][0]							| _m[1][3]*m[3][1]
	
	"pfadd		%%mm4,		%%mm5\n"	// _m[1][0]*m[0][0]+_m[1][1]*m[1][0]					| _m[1][0]*m[0][1]+_m[1][1]*m[1][1]
	"pfadd		%%mm5,		%%mm6\n"	// _m[1][0]*m[0][0]+_m[1][1]*m[1][0]+_m[1][2]*m[2][0]			| _m[1][0]*m[0][1]+_m[1][1]*m[1][1]+_m[1][2]*m[2][1]
	"pfadd		%%mm6,		%%mm7\n"	// _m[1][0]*m[0][0]+_m[1][1]*m[1][0]+_m[1][2]*m[2][0]+_m[1][3]*m[3][0]	| _m[1][0]*m[0][1]+_m[1][1]*m[1][1]+_m[1][2]*m[2][1]+_m[1][3]*m[3][1]
	// write out[0][0] and out[0][1]
	"movq		%%mm7,		16(%%ecx)\n"
	// copy column 2 and column 3 of m into registers mm4-mm7
	"movq		8(%%edx),	%%mm4\n"	// m[0][2]								| m[0][3]
	"movq		24(%%edx),	%%mm5\n"	// m[1][2]								| m[1][3]
	"movq		40(%%edx),	%%mm6\n"	// m[2][2]								| m[2][3]
	"movq		56(%%edx),	%%mm7\n"	// m[3][2]								| m[3][3]
	// calc out[1][2] and out[1][3]	
	"pfmul		%%mm0,		%%mm4\n"	// _m[1][0]*m[0][2]							| _m[1][0]*m[0][3]
	"pfmul		%%mm1,		%%mm5\n"	// _m[1][1]*m[1][2]							| _m[1][1]*m[1][3]
	"pfmul		%%mm2,		%%mm6\n"	// _m[1][2]*m[2][2]							| _m[1][2]*m[2][3]
	"pfmul		%%mm3,		%%mm7\n"	// _m[1][3]*m[3][2]							| _m[1][3]*m[3][3]
	
	"pfadd		%%mm4,		%%mm5\n"	// _m[1][0]*m[0][2]+_m[1][1]*m[1][2]					| _m[1][0]*m[0][3]+_m[1][1]*m[1][3]
	"pfadd		%%mm5,		%%mm6\n"	// _m[1][0]*m[0][2]+_m[1][1]*m[1][2]+_m[1][2]*m[2][2]			| _m[1][0]*m[0][3]+_m[1][1]*m[1][3]+_m[1][2]*m[2][3]
	"pfadd		%%mm6,		%%mm7\n"	// _m[1][0]*m[0][2]+_m[1][1]*m[1][2]+_m[1][2]*m[2][2]+_m[1][3]*m[3][2]	| _m[1][0]*m[0][3]+_m[1][1]*m[1][3]+_m[1][2]*m[2][3]+_m[1][3]*m[3][3]
	// write out[1][2] and out[1][3]
	"movq		%%mm7,		24(%%ecx)\n"
	
	//
	// calculate third row of out
	//
	
	// copy row 2 of this into registers mm0-mm3
	"movq		32(%%eax),	%%mm0\n"	// _m[2][0]								| _m[2][1]
	"movq		%%mm0,		%%mm1\n"	// _m[2][0]								| _m[2][1]
	
	"movq		40(%%eax),	%%mm2\n"	// _m[2][2]								| _m[2][3]
	"movq		%%mm2,		%%mm3\n"	// _m[2][2]								| _m[2][3]
	
	"punpckldq	%%mm0,		%%mm0\n"	// _m[2][0]								| _m[2][0]
	"punpckhdq	%%mm1,		%%mm1\n"	// _m[2][1]								| _m[2][1]
	
	"punpckldq	%%mm2,		%%mm2\n"	// _m[2][2]								| _m[2][2]
	"punpckhdq	%%mm3,		%%mm3\n"	// _m[2][3]								| _m[2][3]
	// copy column 0 and column 1 of m into registers mm4-mm7
	"movq		(%%edx),	%%mm4\n"	// m[0][0]								| m[0][1]
	"movq		16(%%edx),	%%mm5\n"	// m[1][0]								| m[1][1]
	"movq		32(%%edx),	%%mm6\n"	// m[2][0]								| m[2][1]
	"movq		48(%%edx),	%%mm7\n"	// m[3][0]								| m[3][1]
	// calc out[2][0] and out[2][1]	
	"pfmul		%%mm0,		%%mm4\n"	// _m[2][0]*m[0][0]							| _m[2][0]*m[0][1]
	"pfmul		%%mm1,		%%mm5\n"	// _m[2][1]*m[1][0]							| _m[2][1]*m[1][1]
	"pfmul		%%mm2,		%%mm6\n"	// _m[2][2]*m[2][0]							| _m[2][2]*m[2][1]
	"pfmul		%%mm3,		%%mm7\n"	// _m[2][3]*m[3][0]							| _m[2][3]*m[3][1]
	
	"pfadd		%%mm4,		%%mm5\n"	// _m[2][0]*m[0][0]+_m[2][1]*m[1][0]					| _m[2][0]*m[0][1]+_m[2][1]*m[1][1]
	"pfadd		%%mm5,		%%mm6\n"	// _m[2][0]*m[0][0]+_m[2][1]*m[1][0]+_m[2][2]*m[2][0]			| _m[2][0]*m[0][1]+_m[2][1]*m[1][1]+_m[2][2]*m[2][1]
	"pfadd		%%mm6,		%%mm7\n"	// _m[2][0]*m[0][0]+_m[2][1]*m[1][0]+_m[2][2]*m[2][0]+_m[2][3]*m[3][0]	| _m[2][0]*m[0][1]+_m[2][1]*m[1][1]+_m[2][2]*m[2][1]+_m[2][3]*m[3][1]
	// write out[2][0] and out[2][1]
	"movq		%%mm7,		32(%%ecx)\n"
	// copy column 2 and column 3 of m into registers mm4-mm7
	"movq		8(%%edx),	%%mm4\n"	// m[0][2]								| m[0][3]
	"movq		24(%%edx),	%%mm5\n"	// m[1][2]								| m[1][3]
	"movq		40(%%edx),	%%mm6\n"	// m[2][2]								| m[2][3]
	"movq		56(%%edx),	%%mm7\n"	// m[3][2]								| m[3][3]
	// calc out[2][2] and out[2][3]	
	"pfmul		%%mm0,		%%mm4\n"	// _m[2][0]*m[0][2]							| _m[2][0]*m[0][3]
	"pfmul		%%mm1,		%%mm5\n"	// _m[2][1]*m[1][2]							| _m[2][1]*m[1][3]
	"pfmul		%%mm2,		%%mm6\n"	// _m[2][2]*m[2][2]							| _m[2][2]*m[2][3]
	"pfmul		%%mm3,		%%mm7\n"	// _m[2][3]*m[3][2]							| _m[2][3]*m[3][3]
	
	"pfadd		%%mm4,		%%mm5\n"	// _m[2][0]*m[0][2]+_m[2][1]*m[1][2]					| _m[2][0]*m[0][3]+_m[2][1]*m[1][3]
	"pfadd		%%mm5,		%%mm6\n"	// _m[2][0]*m[0][2]+_m[2][1]*m[1][2]+_m[2][2]*m[2][2]			| _m[2][0]*m[0][3]+_m[2][1]*m[1][3]+_m[2][2]*m[2][3]
	"pfadd		%%mm6,		%%mm7\n"	// _m[2][0]*m[0][2]+_m[2][1]*m[1][2]+_m[2][2]*m[2][2]+_m[2][3]*m[3][2]	| _m[2][0]*m[0][3]+_m[2][1]*m[1][3]+_m[2][2]*m[2][3]+_m[2][3]*m[3][3]
	// write out[2][2] and out[2][3]
	"movq		%%mm7,		40(%%ecx)\n"
	
	//
	// calculate fourth row of out
	//
	
	// copy row 3 of this into registers mm0-mm3
	"movq		48(%%eax),	%%mm0\n"	// _m[3][0]								| _m[3][1]
	"movq		%%mm0,		%%mm1\n"	// _m[3][0]								| _m[3][1]
	
	"movq		56(%%eax),	%%mm2\n"	// _m[3][2]								| _m[3][3]
	"movq		%%mm2,		%%mm3\n"	// _m[3][2]								| _m[3][3]
	
	"punpckldq	%%mm0,		%%mm0\n"	// _m[3][0]								| _m[3][0]
	"punpckhdq	%%mm1,		%%mm1\n"	// _m[3][1]								| _m[3][1]
	
	"punpckldq	%%mm2,		%%mm2\n"	// _m[3][2]								| _m[3][2]
	"punpckhdq	%%mm3,		%%mm3\n"	// _m[3][3]								| _m[3][3]
	// copy column 0 and column 1 of m into registers mm4-mm7
	"movq		(%%edx),	%%mm4\n"	// m[0][0]								| m[0][1]
	"movq		16(%%edx),	%%mm5\n"	// m[1][0]								| m[1][1]
	"movq		32(%%edx),	%%mm6\n"	// m[2][0]								| m[2][1]
	"movq		48(%%edx),	%%mm7\n"	// m[3][0]								| m[3][1]
	// calc out[3][0] and out[3][1]	
	"pfmul		%%mm0,		%%mm4\n"	// _m[3][0]*m[0][0]							| _m[3][0]*m[0][1]
	"pfmul		%%mm1,		%%mm5\n"	// _m[3][1]*m[1][0]							| _m[3][1]*m[1][1]
	"pfmul		%%mm2,		%%mm6\n"	// _m[3][2]*m[2][0]							| _m[3][2]*m[2][1]
	"pfmul		%%mm3,		%%mm7\n"	// _m[3][3]*m[3][0]							| _m[3][3]*m[3][1]
	
	"pfadd		%%mm4,		%%mm5\n"	// _m[3][0]*m[0][0]+_m[3][1]*m[1][0]					| _m[3][0]*m[0][1]+_m[3][1]*m[1][1]
	"pfadd		%%mm5,		%%mm6\n"	// _m[3][0]*m[0][0]+_m[3][1]*m[1][0]+_m[3][2]*m[2][0]			| _m[3][0]*m[0][1]+_m[3][1]*m[1][1]+_m[3][2]*m[2][1]
	"pfadd		%%mm6,		%%mm7\n"	// _m[3][0]*m[0][0]+_m[3][1]*m[1][0]+_m[3][2]*m[2][0]+_m[3][3]*m[3][0]	| _m[3][0]*m[0][1]+_m[3][1]*m[1][1]+_m[3][2]*m[2][1]+_m[3][3]*m[3][1]
	// write out[3][0] and out[3][1]
	"movq		%%mm7,		48(%%ecx)\n"
	// copy column 2 and column 3 of m into registers mm4-mm7
	"movq		8(%%edx),	%%mm4\n"	// m[0][2]								| m[0][3]
	"movq		24(%%edx),	%%mm5\n"	// m[1][2]								| m[1][3]
	"movq		40(%%edx),	%%mm6\n"	// m[2][2]								| m[2][3]
	"movq		56(%%edx),	%%mm7\n"	// m[3][2]								| m[3][3]
	// calc out[3][2] and out[3][3]	
	"pfmul		%%mm0,		%%mm4\n"	// _m[3][0]*m[0][2]							| _m[3][0]*m[0][3]
	"pfmul		%%mm1,		%%mm5\n"	// _m[3][1]*m[1][2]							| _m[3][1]*m[1][3]
	"pfmul		%%mm2,		%%mm6\n"	// _m[3][2]*m[2][2]							| _m[3][2]*m[2][3]
	"pfmul		%%mm3,		%%mm7\n"	// _m[3][3]*m[3][2]							| _m[3][3]*m[3][3]
	
	"pfadd		%%mm4,		%%mm5\n"	// _m[3][0]*m[0][2]+_m[3][1]*m[1][2]					| _m[3][0]*m[0][3]+_m[3][1]*m[1][3]
	"pfadd		%%mm5,		%%mm6\n"	// _m[3][0]*m[0][2]+_m[3][1]*m[1][2]+_m[3][2]*m[2][2]			| _m[3][0]*m[0][3]+_m[3][1]*m[1][3]+_m[3][2]*m[2][3]
	"pfadd		%%mm6,		%%mm7\n"	// _m[3][0]*m[0][2]+_m[3][1]*m[1][2]+_m[3][2]*m[2][2]+_m[3][3]*m[3][2]	| _m[3][0]*m[0][3]+_m[3][1]*m[1][3]+_m[3][2]*m[2][3]+_m[3][3]*m[3][3]
	// write out[3][2] and out[3][3]
	"movq		%%mm7,		56(%%ecx)\n"
	:
	: "a"(&_m[0][0]), "d"(&m._m[0][0]), "c"(&out._m[0][0])
	: "memory"
	);
	femms();
	return out;
#else
	return matrix_c(	_m[0][0]*m._m[0][0] + _m[0][1]*m._m[1][0] + _m[0][2]*m._m[2][0] + _m[0][3]*m._m[3][0]	,
				_m[0][0]*m._m[0][1] + _m[0][1]*m._m[1][1] + _m[0][2]*m._m[2][1] + _m[0][3]*m._m[3][1]	,
				_m[0][0]*m._m[0][2] + _m[0][1]*m._m[1][2] + _m[0][2]*m._m[2][2] + _m[0][3]*m._m[3][2]	,
				_m[0][0]*m._m[0][3] + _m[0][1]*m._m[1][3] + _m[0][2]*m._m[2][3] + _m[0][3]*m._m[3][3]	,
				
				_m[1][0]*m._m[0][0] + _m[1][1]*m._m[1][0] + _m[1][2]*m._m[2][0] + _m[1][3]*m._m[3][0]	,
				_m[1][0]*m._m[0][1] + _m[1][1]*m._m[1][1] + _m[1][2]*m._m[2][1] + _m[1][3]*m._m[3][1]	,
				_m[1][0]*m._m[0][2] + _m[1][1]*m._m[1][2] + _m[1][2]*m._m[2][2] + _m[1][3]*m._m[3][2]	,
				_m[1][0]*m._m[0][3] + _m[1][1]*m._m[1][3] + _m[1][2]*m._m[2][3] + _m[1][3]*m._m[3][3]	,
				
				_m[2][0]*m._m[0][0] + _m[2][1]*m._m[1][0] + _m[2][2]*m._m[2][0] + _m[2][3]*m._m[3][0]	,
				_m[2][0]*m._m[0][1] + _m[2][1]*m._m[1][1] + _m[2][2]*m._m[2][1] + _m[2][3]*m._m[3][1]	,
				_m[2][0]*m._m[0][2] + _m[2][1]*m._m[1][2] + _m[2][2]*m._m[2][2] + _m[2][3]*m._m[3][2]	,
				_m[2][0]*m._m[0][3] + _m[2][1]*m._m[1][3] + _m[2][2]*m._m[2][3] + _m[2][3]*m._m[3][3]	,
					
				_m[3][0]*m._m[0][0] + _m[3][1]*m._m[1][0] + _m[3][2]*m._m[2][0] + _m[3][3]*m._m[3][0]	,
				_m[3][0]*m._m[0][1] + _m[3][1]*m._m[1][1] + _m[3][2]*m._m[2][1] + _m[3][3]*m._m[3][1]	,
				_m[3][0]*m._m[0][2] + _m[3][1]*m._m[1][2] + _m[3][2]*m._m[2][2] + _m[3][3]*m._m[3][2]	,
				_m[3][0]*m._m[0][3] + _m[3][1]*m._m[1][3] + _m[3][2]*m._m[2][3] + _m[3][3]*m._m[3][3]	);
#endif
}

vec3_c	matrix_c::operator * (const vec3_c &v) const
{
#if defined(__GNUC__) && !defined(DOUBLEVEC_T) && defined(SIMD_3DNOW)
	vec3_c out(false);
	femms();
	asm volatile
	(
	"movq		(%%edx),	%%mm0\n"	// mm0	v[0]	|	v[1]
	"movd		8(%%edx),	%%mm1\n"	// mm1	v[2]
	// row 0
	"movq		(%%eax),	%%mm2\n"	// mm2	_m[0][0]	|	_m[0][1]
	"movd		8(%%eax),	%%mm3\n"	// mm3	_m[0][2]
	
	"pfmul		%%mm0,		%%mm2\n"	// mm2	_m[0][0]*v[0]	|	_m[0][1]*v[1]
	"pfmul		%%mm1,		%%mm3\n"	// mm3	_m[0][2]*v[2]
	"pfadd		%%mm3,		%%mm2\n"	// mm2	_m[0][0]*v[0] + _m[0][2]*v[2]	| _m[0][1]*v[1]
	// row 1
	"movq		16(%%eax),	%%mm3\n"	// mm3	_m[1][0]	|	_m[1][1]
	"movd		24(%%eax),	%%mm4\n"	// mm4	_m[1][2]
	
	"pfmul		%%mm0,		%%mm3\n"	// mm3	_m[1][0]*v[0]	|	_m[1][1]*v[1]
	"pfmul		%%mm1,		%%mm4\n"	// mm4	_m[1][2]*v[2]
	"pfadd		%%mm4,		%%mm3\n"	// mm3	_m[1][0]*v[0] + _m[1][2]*v[2]	| _m[0][1]*v[1]
	// save
	"pfacc		%%mm3,		%%mm2\n"
	"movq		%%mm2,		(%%ecx)\n"
	// row 2
	"movq		32(%%eax),	%%mm2\n"	// mm2	_m[2][0]	|	_m[2][1]
	"movd		40(%%eax),	%%mm3\n"	// mm3	_m[2][2]
	
	"pfmul		%%mm0,		%%mm2\n"	// mm2	_m[2][0]*v[0]	|	_m[2][1]*v[1]
	"pfmul		%%mm1,		%%mm3\n"	// mm3	_m[2][2]*v[2]
	"pfadd		%%mm3,		%%mm2\n"	// mm2	_m[2][0]*v[0] + _m[2][2]*v[2]	| _m[2][1]*v[1]
	"pfacc		%%mm2,		%%mm2\n"
	"movq		%%mm2,		8(%%ecx)\n"
	:
	: "a"(&_m[0][0]), "d"((vec_t*)v), "c"((vec_t*)out)
	: "memory"
	);
	femms();
	return out;
#else
	// ~ 50 ASM instructions
	return vec3_c(	_m[0][0]*v[0] + _m[0][1]*v[1] + _m[0][2]*v[2],
			_m[1][0]*v[0] + _m[1][1]*v[1] + _m[1][2]*v[2],
			_m[2][0]*v[0] + _m[2][1]*v[1] + _m[2][2]*v[2]	);
#endif
}
	
vec4_c	matrix_c::operator * (const vec4_c &v) const
{
#if 0
//#if defined(__GNUC__) && !defined(DOUBLEVEC_T) && defined(SIMD_SSE)
	vec4_c out(false);
	asm volatile
	(						// reg[0]			| reg[1]		| reg[2]		| reg[3]
	"movups		(%%eax),	%%xmm0\n"	// _m[0][0]			| _m[0][1]		| _m[0][2]		| _m[0][3]
	"movups		16(%%eax),	%%xmm1\n"	// _m[1][0]			| _m[1][1]		| _m[1][2]		| _m[1][3]
	"movups		32(%%eax),	%%xmm2\n"	// _m[2][0]			| _m[2][1]		| _m[2][2]		| _m[2][3]
//	"movups		48(%%eax),	%%xmm3\n"	// _m[3][0]			| _m[3][1]		| _m[3][2]		| _m[3][3]

//	"movups		(%%edx),	%%xmm4\n"	// v[0]				| v[1]			| v[2]			| v[3]
	"movss		(%%edx),	%%xmm4\n"	// v[0]				| -			| -			| -
	"movss		4(%%edx),	%%xmm5\n"	// v[1]				| -			| -			| -
	"movss		8(%%edx),	%%xmm6\n"	// v[2]				| -			| -			| -
	"movss		12(%%edx),	%%xmm6\n"	// v[3]				| -			| -			| -
	
//	"shufps		$0x00, %%xmm4,	%%xmm4\n"	// v[0]				| v[0]			| v[0]			| v[0]
//	"shufps		$0x00, %%xmm5,	%%xmm5\n"	// v[1]				| v[1]			| v[1]			| v[1]
//	"shufps		$0x00, %%xmm6,	%%xmm6\n"	// v[2]				| v[2]			| v[2]			| v[2]
	
//	"xorps		%%xmm5,		%%xmm6\n"	// -				| -			| -			| -
	
	"mulps		%%xmm4,		%%xmm0\n"	// _m[0][0]*v[0]		| _m[0][1]*v[1]		| _m[0][2]*v[2]		| _m[0][3]*v[3]
	"mulps		%%xmm5,		%%xmm1\n"	// _m[1][0]*v[0]		| _m[1][1]*v[1]		| _m[1][2]*v[2]		| _m[1][3]*v[3]
	"mulps		%%xmm6,		%%xmm2\n"	// _m[2][0]*v[0]		| _m[2][1]*v[1]		| _m[2][2]*v[2]		| _m[2][3]*v[3]
	"mulps		%%xmm7,		%%xmm3\n"	// _m[3][0]*v[0]		| _m[3][1]*v[1]		| _m[3][2]*v[2]		| _m[3][3]*v[3]
	
	"addps		%%xmm1,		%%xmm0\n"	// _m[0][0]*v[0]+_m[1][0]*v[1]	| _m[0][1]*v[0]+_m[1][1]*v[1]	| _m[0][2]*v[0]+_m[1][2]*v[1]	| _m[0][3]*v[0]+_m[1][3]*v[1]
	"addps		%%xmm2,		%%xmm0\n"	// _m[0][0]*v[0]+_m[1][0]*v[1]+_m[2][0]*v[2]	| _m[0][1]*v[0]+_m[1][1]*v[1]	| _m[0][2]*v[0]+_m[1][2]*v[1]	| _m[0][3]*v[0]+_m[1][3]*v[1]
	"addps		%%xmm3,		%%xmm0\n"
	
	"movups		%%xmm0,		(%%ecx)\n"
	:
	: "a"(&_m[0][0]), "d"((vec_t*)v), "c"((vec_t*)out)
	: "memory"
	);
	return out;
#elif defined(__GNUC__) && !defined(DOUBLEVEC_T) && defined(SIMD_3DNOW)
	vec4_c out(false);
	femms();
	asm volatile
	(						//	LOW				|	HIGH
	"movq		(%%edx),	%%mm0\n"	//	[0]				|	v[1]
	"movq		8(%%edx),	%%mm1\n"	//	v[2]				|	v[3]
	
	"movq		(%%eax),	%%mm2\n"	//	_m[0][0]			|	_m[0][1]
	"movq		8(%%eax),	%%mm3\n"	//	_m[0][2]			|	_m[0][3]
	"pfmul		%%mm0,		%%mm2\n"	//	_m[0][0]*v[0]			|	_m[0][1]*v[1]
	"pfmul		%%mm1,		%%mm3\n"	//	_m[0][2]*v[2]			|	_m[0][3]*v[3]
	"pfadd		%%mm2,		%%mm3\n"	//	_m[0][2]*v[2] + _m[0][0]*v[0]	| _m[0][3]*v[3] + _m[0][1]*v[1]
	
	"movq		16(%%eax),	%%mm4\n"	//	_m[1][0]			|	_m[1][1]
	"movq		24(%%eax),	%%mm5\n"	//	_m[1][2]			|	_m[1][3]
	"pfmul		%%mm0,		%%mm4\n"	//	_m[1][0]*v[0]			|	_m[1][1]*v[1]
	"pfmul		%%mm1,		%%mm5\n"	//	_m[1][2]*v[2]			|	_m[1][3]*v[3]
	"pfadd		%%mm4,		%%mm5\n"	//	_m[1][2]*v[2] + _m[1][0]*v[0]	| _m[1][3]*v[3] + _m[1][1]*v[1]
	
	"pfacc		%%mm5,		%%mm3\n"	// _m[0][2]*v[2] + _m[0][0]*v[0] + _m[0][3]*v[3] + _m[0][1]*v[1]	|	_m[1][2]*v[2] + _m[1][0]*v[0] + _m[1][3]*v[3] + _m[1][1]*v[1]
	"movq		%%mm3,		(%%ecx)\n"	// out[0] = mm3[low], out[1] = mm3[high]
	
	"movq		32(%%eax),	%%mm2\n"	//	_m[2][0]			|	_m[2][1]
	"movq		40(%%eax),	%%mm3\n"	//	_m[2][2]			|	_m[2][3]
	"pfmul		%%mm0,		%%mm2\n"	//	_m[2][0]*v[0]			|	_m[2][1]*v[1]
	"pfmul		%%mm1,		%%mm3\n"	//	_m[2][2]*v[2]			|	_m[2][3]*v[3]
	"pfadd		%%mm2,		%%mm3\n"	//	_m[2][2]*v[2] + _m[2][0]*v[0]	| _m[2][3]*v[3] + _m[2][1]*v[1]
	
	"movq		48(%%eax),	%%mm4\n"	//	_m[3][0]			|	_m[3][1]
	"movq		56(%%eax),	%%mm5\n"	//	_m[3][2]			|	_m[3][3]
	"pfmul		%%mm0,		%%mm4\n"	//	_m[3][0]*v[0]			|	_m[3][1]*v[1]
	"pfmul		%%mm1,		%%mm5\n"	//	_m[3][2]*v[2]			|	_m[3][3]*v[3]
	"pfadd		%%mm4,		%%mm5\n"	//	_m[3][2]*v[2] + _m[3][0]*v[0]	| _m[3][3]*v[3] + _m[3][1]*v[1]
	
	"pfacc		%%mm5,		%%mm3\n"	// _m[2][2]*v[2] + _m[2][0]*v[0] + _m[2][3]*v[3] + _m[2][1]*v[1]	|	_m[3][2]*v[2] + _m[3][0]*v[0] + _m[3][3]*v[3] + _m[3][1]*v[1]
	"movq		%%mm3,		8(%%ecx)\n"	// out[2] = mm3[low], out[3] = mm3[high]
	:
	: "a"(&_m[0][0]), "d"((vec_t*)v), "c"((vec_t*)out)
	: "memory"
	);
	femms();
	return out;
#else
	// ~ 80 ASM instructions
	return vec4_c(	_m[0][0]*v[0] + _m[0][1]*v[1] + _m[0][2]*v[2] + _m[0][3]*v[3],
			_m[1][0]*v[0] + _m[1][1]*v[1] + _m[1][2]*v[2] + _m[1][3]*v[3],
			_m[2][0]*v[0] + _m[2][1]*v[1] + _m[2][2]*v[2] + _m[2][3]*v[3],
			_m[3][0]*v[0] + _m[3][1]*v[1] + _m[3][2]*v[2] + _m[3][3]*v[3]	);
#endif
}

matrix_c&	matrix_c::operator = (const matrix_c &m)
{
#if defined(__GNUC__) && !defined(DOUBLEVEC_T) && defined(SIMD_SSE)
	asm volatile
	(
	//FIXME figure out why movaps breaks at runtime
	"movups		(%%edx),	%%xmm0\n"
	"movups		0x10(%%edx),	%%xmm1\n"
	"movups		0x20(%%edx),	%%xmm2\n"
	"movups		0x30(%%edx),	%%xmm3\n"
	
	"movups		%%xmm0,		(%%eax)\n"
	"movups		%%xmm1,		0x10(%%eax)\n"
	"movups		%%xmm2,		0x20(%%eax)\n"
	"movups		%%xmm3,		0x30(%%eax)\n"
	:
	: "a"(&_m[0][0]), "d"(&m._m[0][0])
	: "memory"
	);
#elif defined(__GNUC__) && !defined(DOUBLEVEC_T) && defined(SIMD_3DNOW)
	femms();
	asm volatile
	(
	//
	"movq		(%%edx),	%%mm0\n"	// mm0	_m[0][0]	| _m[0][1]
	"movq		8(%%edx),	%%mm1\n"	// mm1	_m[0][2]	| _m[0][3]
	"movq		16(%%edx),	%%mm2\n"	// mm2	_m[1][0]	| _m[1][1]
	"movq		24(%%edx),	%%mm3\n"	// mm3	_m[1][2]	| _m[1][3]
	"movq		32(%%edx),	%%mm4\n"	// mm4	_m[2][0]	| _m[2][1]
	"movq		40(%%edx),	%%mm5\n"	// mm5	_m[2][2]	| _m[2][3]
	"movq		48(%%edx),	%%mm6\n"	// mm6	_m[3][0]	| _m[3][1]
	"movq		56(%%edx),	%%mm7\n"	// mm7	_m[3][2]	| _m[3][3]
	
	"movq		%%mm0,		(%%eax)\n"
	"movq		%%mm1,		8(%%eax)\n"
	"movq		%%mm2,		16(%%eax)\n"
	"movq		%%mm3,		24(%%eax)\n"
	"movq		%%mm4,		32(%%eax)\n"
	"movq		%%mm5,		40(%%eax)\n"
	"movq		%%mm6,		48(%%eax)\n"
	"movq		%%mm7,		56(%%eax)\n"
	:
	: "a"(&_m[0][0]), "d"(&m._m[0][0])
	: "memory"
	);
	femms();
#else
	_m[0][0]=m._m[0][0];	_m[0][1]=m._m[0][1];	_m[0][2]=m._m[0][2];	_m[0][3]=m._m[0][3];
	_m[1][0]=m._m[1][0];	_m[1][1]=m._m[1][1];	_m[1][2]=m._m[1][2];	_m[1][3]=m._m[1][3];
	_m[2][0]=m._m[2][0];	_m[2][1]=m._m[2][1];	_m[2][2]=m._m[2][2];	_m[2][3]=m._m[2][3];
	_m[3][0]=m._m[3][0];	_m[3][1]=m._m[3][1];	_m[3][2]=m._m[3][2];	_m[3][3]=m._m[3][3];
#endif
	return *this;
}


	
vec_t	quaternion_c::normalize()
{
#if defined(__GNUC__) && !defined(DOUBLEVEC_T) && defined(SIMD_3DNOW)
	vec_t len;
	femms();
	asm volatile
	(						// lo							| hi
	"movq		(%%eax),	%%mm0\n"	// _q[0]						| _q[1]
	"movq		8(%%eax),	%%mm1\n"	// _q[2]						| _q[3]
	// mm0[lo] = dot product(this)
	"pfmul		%%mm0,		%%mm0\n"	// _q[0]*_q[0]						| _q[1]*_q[1]
	"pfmul		%%mm1,		%%mm1\n"	// _q[2]*_q[2]						| _q[3]*_q[3]
	"pfacc		%%mm0,		%%mm0\n"	// _q[0]*_q[0]+_q[1]*_q[1]				| -
	"pfacc		%%mm1,		%%mm1\n"	// _q[2]*_q[2]+_q[3]*_q[3]				| -
	"pfadd		%%mm1,		%%mm0\n"	// _q[0]*_q[0]+_q[1]*_q[1]+_q[2]*_q[2]+_q[3]*_q[3]	| -
	// mm0[lo] = sqrt(mm0[lo])
	"pfrsqrt	%%mm0,		%%mm1\n"	// 1/sqrt(dot)						| 1/sqrt(dot)		(approx)
	"movq		%%mm1,		%%mm2\n"	// 1/sqrt(dot)						| 1/sqrt(dot)		(approx)
	"pfmul		%%mm1,		%%mm1\n"	// (1/sqrt(dot))�					| (1/sqrt(dot))�	step 1
	"punpckldq	%%mm0,		%%mm0\n"	// dot							| dot			(MMX instruction)
	"pfrsqit1	%%mm0,		%%mm1\n"	// intermediate						| intermediate		step 2
	"pfrcpit2	%%mm2,		%%mm1\n"	// 1/sqrt(dot) (full 24-bit precision)			| 1/sqrt(dot)		step 3
	"pfmul		%%mm1,		%%mm0\n"	// sqrt(dot)						| sqrt(dot)
	// len = mm0[lo]
	"movd		%%mm0,		(%%edx)\n"
	// load this into registers
	"movq		(%%eax),	%%mm2\n"	// _q[0]						| _q[1]
	"movq		8(%%eax),	%%mm3\n"	// _q[2]						| _q[3]
	// scale this by the reciprocal square root
	"pfmul		%%mm1,		%%mm2\n"	// _q[0]*1/sqrt(dot)					| _q[1]*1/sqrt(dot)
	"pfmul		%%mm1,		%%mm3\n"	// _q[2]*1/sqrt(dot)					| _q[3]*1/sqrt(dot)
	// store scaled vector
	"movq		%%mm2,		(%%eax)\n"
	"movq		%%mm3,		8(%%eax)\n"
	:
	: "a"(&_q[0]), "d"(&len)
	: "memory"
	);
	femms();
	return len;
#else
	vec_t len = length();
	
	if(len)
	{
		vec_t ilen = X_recip(len);
		
		_q[0] *= ilen;
		_q[1] *= ilen;
		_q[2] *= ilen;
		_q[3] *= ilen;
	}
	
	return len;
#endif
}
	
void	quaternion_c::fromAngles(vec_t pitch, vec_t yaw, vec_t roll)
{
#if 1
	matrix_c m;
	m.fromAngles(pitch, yaw, roll);
	fromMatrix(m);
#else
	identity();
		
	multiplyRotation(0, 0, 1, yaw);
	multiplyRotation(0, 1, 0, pitch);
	multiplyRotation(1, 0, 0, roll);
		
	normalize();
#endif
}
	
void	quaternion_c::fromMatrix(const matrix_c &m)
{
	vec_t trace = 1.0f + m[0][0] + m[1][1] + m[2][2];
		
	if(trace > 0)
	{
		vec_t s = 0.5f / X_sqrt(trace);
		
		_q[0] = (m[2][1] - m[1][2]) * s;
		_q[1] = (m[0][2] - m[2][0]) * s;
		_q[2] = (m[1][0] - m[0][1]) * s;
		_q[3] = 0.25f / s;
	}
	else
	{
		if(m[0][0] > m[1][1] && m[0][0] > m[2][2])
		{	
			// Column 0: 
			float s = X_sqrt(1.0f + m[0][0] - m[1][1] - m[2][2]) * 2.0f;
				
			_q[0] = 0.25f * s;
			_q[1] = (m[0][1] + m[1][0]) / s;
			_q[2] = (m[0][2] + m[2][0]) / s;
			_q[3] = (m[1][2] - m[2][1]) / s;
		}
		else if(m[1][1] > m[2][2])
		{
			// Column 1:
			float s = X_sqrt(1.0f + m[1][1] - m[0][0] - m[2][2]) * 2.0f;
			
			_q[0] = (m[0][1] + m[1][0]) / s;
			_q[1] = 0.25f * s;
			_q[2] = (m[1][2] + m[2][1]) / s;
			_q[3] = (m[0][2] - m[2][0]) / s;
		}
		else
		{
			// Column 2:
			float s = X_sqrt(1.0f + m[2][2] - m[0][0] - m[1][1]) * 2.0f;
			
			_q[0] = (m[0][2] + m[2][0]) / s;
			_q[1] = (m[1][2] + m[2][1]) / s;
			_q[2] = 0.25f * s;
			_q[3] = (m[0][1] - m[1][0]) / s;
		}
	}
	
	normalize();
}
	
void	quaternion_c::fromAxisAngle(vec_t ax, vec_t ay, vec_t az, vec_t angle)
{
#if 0
	vec3_c	naxis(x, y, z);
	double	sin_a;
	double	cos_a;
	
	naxis.normalize();
	
	sin_a = X_sin(DEGTORAD(deg/2.0f));
	cos_a = X_cos(DEGTORAD(deg/2.0f));
	
	_q[0] = naxis[0] * sin_a;
	_q[1] = naxis[1] * sin_a;
	_q[2] = naxis[2] * sin_a;
	_q[3] = cos_a;
#else

	vec_t l = ax*ax + ay*ay + az*az;
	if (l > REAL(0.0))
	{
		angle *= REAL(0.5);
		_q[3] = X_cos(angle);
		l = X_sin(angle) * X_recipsqrt(l);
		_q[0] = ax*l;
		_q[1] = ay*l;
		_q[2] = az*l;
	}
	else
	{
		_q[3] = 1;
		_q[0] = 0;
		_q[1] = 0;
		_q[2] = 0;
	}
#endif
}
	
void	quaternion_c::toAxisAngle(vec3_c &axis, float *deg)
{
	float	angle;
	float	scale;
	float	length;
	
	angle = acos(_q[3]);

	scale = Vector3_Length(_q);

	if(scale == 0)
	{
		axis.clear();
		*deg = 0;
	}
	else
	{
		*deg = angle*2;
		axis[0] = _q[0] / scale;
		axis[1] = _q[1] / scale;
		axis[2] = _q[2] / scale;
		length = axis.length();
		
		if(length != 0)
		{
			axis[0] /= length;
			axis[1] /= length;
			axis[2] /= length;
		}
		
		*deg = RADTODEG (*deg);			//give us angle in degrees
	}
}
	
void	quaternion_c::toVectorsFLU(vec3_c &forward, vec3_c &left, vec3_c &up) const
{
	vec_t xx = _q[0] * _q[0];
	vec_t xy = _q[0] * _q[1];
	vec_t xz = _q[0] * _q[2];
	vec_t xw = _q[0] * _q[3];
	
	vec_t yy = _q[1] * _q[1];
	vec_t yz = _q[1] * _q[2];
	vec_t yw = _q[1] * _q[3];
	
	vec_t zz = _q[2] * _q[2];
	vec_t zw = _q[2] * _q[3];

	
	forward[0] = 1-2*(yy+zz);	//_m[0][0];	// cp*cy;
	forward[1] =   2*(xy+zw);	//_m[1][0];	// cp*sy;
	forward[2] =   2*(xz-yw);	//_m[2][0];	//-sp;
	
	left[0] =   2*(xy-zw);		//_m[0][1];	// sr*sp*cy+cr*-sy;
	left[1] = 1-2*(xx+zz);		//_m[1][1];	// sr*sp*sy+cr*cy;
	left[2] =   2*(yz+xw);		//_m[2][1];	// sr*cp;

	up[0] =   2*(xz+yw);		//_m[0][2];	// cr*sp*cy+-sr*-sy;
	up[1] =   2*(yz-xw);		//_m[1][2];	// cr*sp*sy+-sr*cy;
	up[2] = 1-2*(xx+yy);		//_m[2][2];	// cr*cp;	
}
	
void	quaternion_c::toVectorsFRU(vec3_c &forward, vec3_c &right, vec3_c &up) const
{
	vec_t xx = _q[0] * _q[0];
	vec_t xy = _q[0] * _q[1];
	vec_t xz = _q[0] * _q[2];
	vec_t xw = _q[0] * _q[3];
	
	vec_t yy = _q[1] * _q[1];
	vec_t yz = _q[1] * _q[2];
	vec_t yw = _q[1] * _q[3];
	
	vec_t zz = _q[2] * _q[2];
	vec_t zw = _q[2] * _q[3];
	
	
	forward[0] = 1-2*(yy+zz);	//_m[0][0];	// cp*cy;
	forward[1] =   2*(xy+zw);	//_m[1][0];	// cp*sy;
	forward[2] =   2*(xz-yw);	//_m[2][0];	//-sp;
	
	right[0] =    -2*(xy-zw);	//_m[0][1];	//-sr*sp*cy+cr*-sy;
	right[1] = -(1-2*(xx+zz));	//_m[1][1];	//-sr*sp*sy+cr*cy;
	right[2] =    -2*(yz+xw);	//_m[2][1];	//-sr*cp;

	up[0] =   2*(xz+yw);		//_m[0][2];	// cr*sp*cy+-sr*-sy;
	up[1] =   2*(yz-xw);		//_m[1][2];	// cr*sp*sy+-sr*cy;
	up[2] = 1-2*(xx+yy);		//_m[2][2];	// cr*cp;
}

void	quaternion_c::multiply0(const quaternion_c &qb, const quaternion_c &qc)
{
	_q[0] = qb._q[3]*qc._q[0] + qb._q[0]*qc._q[3] + qb._q[1]*qc._q[2] - qb._q[2]*qc._q[1];
	_q[1] = qb._q[3]*qc._q[1] + qb._q[1]*qc._q[3] + qb._q[2]*qc._q[0] - qb._q[0]*qc._q[2];
	_q[2] = qb._q[3]*qc._q[2] + qb._q[2]*qc._q[3] + qb._q[0]*qc._q[1] - qb._q[1]*qc._q[0];
	_q[3] = qb._q[3]*qc._q[3] - qb._q[0]*qc._q[0] - qb._q[1]*qc._q[1] - qb._q[2]*qc._q[2];
}

void	quaternion_c::multiply1(const quaternion_c &qb, const quaternion_c &qc)
{
	_q[0] = qb._q[3]*qc._q[0] - qb._q[0]*qc._q[3] - qb._q[1]*qc._q[2] + qb._q[2]*qc._q[1];
	_q[1] = qb._q[3]*qc._q[1] - qb._q[1]*qc._q[3] - qb._q[2]*qc._q[0] + qb._q[0]*qc._q[2];
	_q[2] = qb._q[3]*qc._q[2] - qb._q[2]*qc._q[3] - qb._q[0]*qc._q[1] + qb._q[1]*qc._q[0];
	_q[3] = qb._q[3]*qc._q[3] + qb._q[0]*qc._q[0] + qb._q[1]*qc._q[1] + qb._q[2]*qc._q[2];
}

void	quaternion_c::multiply2(const quaternion_c &qb, const quaternion_c &qc)
{
	_q[0] =-qb._q[3]*qc._q[0] + qb._q[0]*qc._q[3] - qb._q[1]*qc._q[2] + qb._q[2]*qc._q[1];
	_q[1] =-qb._q[3]*qc._q[1] + qb._q[1]*qc._q[3] - qb._q[2]*qc._q[0] + qb._q[0]*qc._q[2];
	_q[2] =-qb._q[3]*qc._q[2] + qb._q[2]*qc._q[3] - qb._q[0]*qc._q[1] + qb._q[1]*qc._q[0];
	_q[3] = qb._q[3]*qc._q[3] + qb._q[0]*qc._q[0] + qb._q[1]*qc._q[1] + qb._q[2]*qc._q[2];
}

void	quaternion_c::multiply3(const quaternion_c &qb, const quaternion_c &qc)
{
	_q[0] =-qb._q[3]*qc._q[0] - qb._q[0]*qc._q[3] + qb._q[1]*qc._q[2] - qb._q[2]*qc._q[1];
	_q[1] =-qb._q[3]*qc._q[1] - qb._q[1]*qc._q[3] + qb._q[2]*qc._q[0] - qb._q[0]*qc._q[2];
	_q[2] =-qb._q[3]*qc._q[2] - qb._q[2]*qc._q[3] + qb._q[0]*qc._q[1] - qb._q[1]*qc._q[0];
	_q[3] = qb._q[3]*qc._q[3] - qb._q[0]*qc._q[0] - qb._q[1]*qc._q[1] - qb._q[2]*qc._q[2];
}
	
void	quaternion_c::slerp(const quaternion_c &from, const quaternion_c &to, float t)
{
	/*
	quaternion_c a = q1;
	quaternoin_c b = q2;
	quaternion_c trans = b * a->inverse();
		
	if(trans.w < 0)
		trans.antipodal();
			
	*this = trans.power(f) * *this
	*/
		
	quaternion_c to1;
	double omega, cosom, sinom, scale0, scale1;

	cosom = from._q[0]*to._q[0] + from._q[1]*to._q[1] + from._q[2]*to._q[2] + from._q[3]*to._q[3];

	if(cosom < 0.0)
	{
		cosom = -cosom;
		
		to1 = to;
		to1.antipodal();
	}
	else 
	{
		to1 = to;
	}
		
	if((1.0 - cosom) > 0)
	{
		omega = acos(cosom);
		sinom = sin(omega);
		scale0 = sin((1.0 - t) * omega) / sinom;
		scale1 = sin(t * omega) / sinom;
	}
	else
	{
		scale0 = 1.0 - t;
		scale1 = t;
	}

	_q[0] = scale0 * from._q[0] + scale1 * to1._q[0];
	_q[1] = scale0 * from._q[1] + scale1 * to1._q[1];
	_q[2] = scale0 * from._q[2] + scale1 * to1._q[2];
	_q[3] = scale0 * from._q[3] + scale1 * to1._q[3];
}
	
const char*	quaternion_c::toString() const
{
	return va("(%i %i %i %i)", (int)_q[0], (int)_q[1], (int)_q[2], (int)_q[3]);
}
	
quaternion_c	quaternion_c::operator * (const quaternion_c &q) const
{
	/* from matrix and quaternion faq
	x = w1x2 + x1w2 + y1z2 - z1y2
	y = w1y2 + y1w2 + z1x2 - x1z2
	z = w1z2 + z1w2 + x1y2 - y1x2
	
	w = w1w2 - x1x2 - y1y2 - z1z2
	*/
	
	// this corresponds to dQMultiply0 from the ODE lib
	
	return quaternion_c(	_q[3]*q._q[0] + _q[0]*q._q[3] + _q[1]*q._q[2] - _q[2]*q._q[1],
				_q[3]*q._q[1] + _q[1]*q._q[3] + _q[2]*q._q[0] - _q[0]*q._q[2],
				_q[3]*q._q[2] + _q[2]*q._q[3] + _q[0]*q._q[1] - _q[1]*q._q[0],
				_q[3]*q._q[3] - _q[0]*q._q[0] - _q[1]*q._q[1] - _q[2]*q._q[2]	);

}



bool	aabb_c::lineIntersect(float min1, float max1, float min2, float max2) const
{
	if(min1 <= min2 && min2 <= max1 )
		return true;
	
	if(min1 <= max2 && max2 <= max1 )
		return true;
		
	if(min2 <= min1 && min1 <= max2 )
		return true;
			
	if(min2 <= max1 && max1 <= max2 )
		return true;
		
	return false;
}

vec_t	aabb_c::maxOfEight(vec_t n1, vec_t n2, vec_t n3, vec_t n4, vec_t n5, vec_t n6, vec_t n7, vec_t n8) const
{
	float max = n1;
	
	if(n2 > max)
		max = n2;
		
	if(n3 > max)
		max = n3;
		
	if(n4 > max)
		max = n4;
		
	if(n5 > max)
		max = n5;
		
	if(n6 > max)
		max = n6;
		
	if(n7 > max)
		max = n7;
		
	if(n8 > max)
		max = n8;
		
	return max;
}

vec_t	aabb_c::minOfEight(vec_t n1, vec_t n2, vec_t n3, vec_t n4, vec_t n5, vec_t n6, vec_t n7, vec_t n8) const
{
	float min = n1;
	
	if(n2 < min)
		min = n2;
		
	if(n3 < min)
		min = n3;
		
	if(n4 < min)
		min = n4;
		
	if(n5 < min)
		min = n5;
		
	if(n6 < min)
		min = n6;
		
	if(n7 < min)
		min = n7;
		
	if(n8 < min)
		min = n8;
		
	return min;
}


bool	aabb_c::intersect(const aabb_c &bbox) const
{
#if 1
	return(	_mins[0] <= bbox._maxs[0]	&& 
		_mins[1] <= bbox._maxs[1]	&&
		_mins[2] <= bbox._maxs[2]	&&
		
		_maxs[0] >= bbox._mins[0]	&& 
		_maxs[1] >= bbox._mins[1]	&& 
		_maxs[2] >= bbox._mins[2]	);
#else
	if (	lineIntersect(_mins[0], _maxs[0], bbox._mins[0], bbox._maxs[0])	&&
		lineIntersect(_mins[1], _maxs[1], bbox._mins[1], bbox._maxs[1])	&&
		lineIntersect(_mins[2], _maxs[2], bbox._mins[2], bbox._maxs[2])	)
		return true;
	else
		return false;
#endif
}


bool	aabb_c::intersect(const vec3_c &origin, const float radius) const
{
#if 1
	return(	_mins[0] <= origin[0] + radius	&& 
		_mins[1] <= origin[1] + radius	&&
		_mins[2] <= origin[2] + radius	&&
		
		_maxs[0] >= origin[0] - radius	&& 
		_maxs[1] >= origin[1] - radius	&& 
		_maxs[2] >= origin[2] - radius	);
#else
	aabb_c	bbox;
		
	bbox._mins[0] = origin[0] - radius;
	bbox._mins[1] = origin[1] - radius;
	bbox._mins[2] = origin[2] - radius;
		
	bbox._maxs[0] = origin[0] + radius;
	bbox._maxs[1] = origin[1] + radius;
	bbox._mins[2] = origin[2] + radius;

	if (	lineIntersect(_mins[0], _maxs[0], bbox._mins[0], bbox._maxs[0])	&&
		lineIntersect(_mins[1], _maxs[1], bbox._mins[1], bbox._maxs[1])	&&
		lineIntersect(_mins[2], _maxs[2], bbox._mins[2], bbox._maxs[2])	)
		return true;
	else
		return false;
#endif
}

bool	aabb_c::intersect(vec_t x0, vec_t y0, vec_t z0, vec_t x1, vec_t y1, vec_t z1) const
{
	// an infinite ray would intersect the box but if it is not
	// we must check if the ray is out side the box
	if(x0 > _maxs[0] && x1 > _maxs[0])	return false;
	if(y0 > _maxs[1] && y1 > _maxs[1])	return false;
	if(z0 > _maxs[2] && z1 > _maxs[2])	return false;
	
	if(x0 < _mins[0] && x1 < _mins[0])	return false;
	if(y0 < _mins[1] && y1 < _mins[1])	return false;
	if(z0 < _mins[2] && z1 < _mins[2])	return false;
		
	if(isInside(x0, y0, z0))
		return true;
	
	if(isInside(x1, y1, z1))
		return true;
	
	vec3_c t(x1-x0, y1-y0, z1-z0);
	
	vec_t lent = t.normalize();
	if(lent == 0.0)
		return false;

	vec3_c s((_maxs[0]+_mins[0])/2-x0, (_maxs[1]+_mins[1])/2-y0, (_maxs[2]+_mins[2])/2-z0);

	vec_t plen = t.dotProduct(s);

	// t is the bbox center projection point on the ray
	t.scale(plen);

	if(isInside(t[0]+x0, t[1]+y0, t[2]+z0))
		return true;
		
	return false;
}

float	aabb_c::radius() const
{
	vec3_c	corner(false);
	
	corner[0] = fabs(_mins[0]) > fabs(_maxs[0]) ? fabs(_mins[0]) : fabs(_maxs[0]);
	corner[1] = fabs(_mins[1]) > fabs(_maxs[1]) ? fabs(_mins[1]) : fabs(_maxs[1]);
	corner[2] = fabs(_mins[2]) > fabs(_maxs[2]) ? fabs(_mins[2]) : fabs(_maxs[2]);

	return corner.length();
}

void	aabb_c::addPoint(const vec3_c &v)
{
	if(v[0] < _mins[0])
		_mins[0] = v[0];
		
	if(v[1] < _mins[1])
		_mins[1] = v[1];
		
	if(v[2] < _mins[2])
		_mins[2] = v[2];
	
	
	if(v[0] > _maxs[0])
		_maxs[0] = v[0];
		
	if(v[1] > _maxs[1])
		_maxs[1] = v[1];
		
	if(v[2] > _maxs[2])
		_maxs[2] = v[2];
}

void	aabb_c::mergeWith(const aabb_c &bbox)
{
	if(bbox._mins[0] < _mins[0])
		_mins[0] = bbox._mins[0];
		
	if(bbox._mins[1] < _mins[1])
		_mins[1] = bbox._mins[1];
		
	if(bbox._mins[2] < _mins[2])
		_mins[2] = bbox._mins[2];
	
	
	if(bbox._maxs[0] > _maxs[0])
		_maxs[0] = bbox._maxs[0];
		
	if(bbox._maxs[1] > _maxs[1])
		_maxs[1] = bbox._maxs[1];
		
	if(bbox._maxs[2] > _maxs[2])
		_maxs[2] = bbox._maxs[2];
}

void	aabb_c::translate(const vec3_c &v)
{
	_mins += v;
	_maxs += v;
}

void	aabb_c::rotate(const quaternion_c &q, bool object_space)
{
	// compute bbox vertices
	vec3_c vert0(_maxs[0], _mins[1], _mins[2]);
	vec3_c vert1(_maxs[0], _mins[1], _maxs[2]);
	vec3_c vert2(_mins[0], _mins[1], _maxs[2]);
	vec3_c vert3(_mins[0], _mins[1], _mins[2]);
	
	vec3_c vert4(_maxs[0], _maxs[1], _mins[2]);
	vec3_c vert5(_maxs[0], _maxs[1], _maxs[2]);
	vec3_c vert6(_mins[0], _maxs[1], _maxs[2]);
	vec3_c vert7(_mins[0], _maxs[1], _mins[2]);
	
	vert0.rotate(q);
	vert1.rotate(q);
	vert2.rotate(q);
	vert3.rotate(q);
	vert4.rotate(q);
	vert5.rotate(q);
	vert6.rotate(q);
	vert7.rotate(q);
	
	_mins[0] = minOfEight(vert0[0], vert1[0], vert2[0], vert3[0], vert4[0], vert5[0], vert6[0], vert7[0]);
	_mins[1] = minOfEight(vert0[1], vert1[1], vert2[1], vert3[1], vert4[1], vert5[1], vert6[1], vert7[1]);
	_mins[2] = minOfEight(vert0[2], vert1[2], vert2[2], vert3[2], vert4[2], vert5[2], vert6[2], vert7[2]);
	
	_maxs[0] = maxOfEight(vert0[0], vert1[0], vert2[0], vert3[0], vert4[0], vert5[0], vert6[0], vert7[0]);
	_maxs[1] = maxOfEight(vert0[1], vert1[1], vert2[1], vert3[1], vert4[1], vert5[1], vert6[1], vert7[1]);
	_maxs[2] = maxOfEight(vert0[2], vert1[2], vert2[2], vert3[2], vert4[2], vert5[2], vert6[2], vert7[2]);
}

bool 	aabb_c::isZero() const
{
	if (	_mins[0] == 0 && _mins[1] == 0 && _mins[2] == 0 &&
		_maxs[0] == 0 && _maxs[1] == 0 && _maxs[2] == 0)
		return true;
	else
		return false;
}
	
const char*	aabb_c::toString() const
{
	return va("(%i %i %i) (%i %i %i)", (int)_mins[0], (int)_mins[1], (int)_mins[2], (int)_maxs[0], (int)_maxs[1], (int)_maxs[2]);
}



bool	cplane_c::fromThreePointForm(const vec3_c &v0, const vec3_c &v1, const vec3_c &v2)
{
	// build directional vectors
	vec3_c edge0 = v1 - v0;
	vec3_c edge1 = v2 - v0;
	
	// create normal
	vec3_c	normal(false);
	normal.crossProduct(edge0, edge1);
	vec_t len = normal.normalize();
	
	// create distance from origin
	vec_t dist = v0.dotProduct(normal);
	
	// finally setup the plane
	set(normal, dist);
	
	// check if degenerated triangle
	if(len < REAL(0.1))
		return true;//false;	//FIXME
	
	return true;
}

void	cplane_c::setType()
{
	if(_normal[0] == REAL(1.0))
	{
		_type = PLANE_X;
		return;
	}
	
	if(_normal[1] == REAL(1.0))
	{
		_type = PLANE_Y;
		return;
	}
	
	if(_normal[2] == REAL(1.0))
	{
		_type = PLANE_Z;
		return;
	}
	
	vec_t ax = X_fabs(_normal[0]);
	vec_t ay = X_fabs(_normal[1]);
	vec_t az = X_fabs(_normal[2]);
	
	if(ax >= ay && ax >= az)
	{
		_type = PLANE_ANYX;
		return;
	}
	
	if(ay >= ax && ay >= az)
	{
		_type = PLANE_ANYY;
		return;
	}
	
	_type = PLANE_ANYZ;
}

void	cplane_c::setSignBits()
{
	// for fast box on planeside test

	_signbits = 0;
	
	if(_normal[0] < 0)
		_signbits |= 1<<0;
		
	if(_normal[1] < 0)
		_signbits |= 1<<1;
		
	if(_normal[2] < 0)
		_signbits |= 1<<2;
}

void	cplane_c::normalize()
{
	vec_t len = _normal.length();
	
	if(len > REAL(0.0))
	{
		vec_t ilen = X_recip(len);
		
		_normal[0] *= ilen;
		_normal[1] *= ilen;
		_normal[2] *= ilen;
		_dist *= ilen;
	}
}

plane_side_e	cplane_c::onSide(const aabb_c &bbox, bool use_signbits) const
{	
	// fast axial cases
	if(_type <= PLANE_Z)
	{
		if(_dist <= bbox._mins[_type])
			return SIDE_FRONT;
			
		if(_dist >= bbox._maxs[_type])
			return SIDE_BACK;
				
		return SIDE_CROSS;
	}
		
	// general case
	vec_t dist1;
	vec_t dist2;
	
	if(use_signbits)
	{
		switch(_signbits)
		{
			case 0:
				dist1 = _normal[0]*bbox._maxs[0] + _normal[1]*bbox._maxs[1] + _normal[2]*bbox._maxs[2] - _dist;
				dist2 = _normal[0]*bbox._mins[0] + _normal[1]*bbox._mins[1] + _normal[2]*bbox._mins[2] - _dist;
				break;
		
			case 1:
				dist1 = _normal[0]*bbox._mins[0] + _normal[1]*bbox._maxs[1] + _normal[2]*bbox._maxs[2] - _dist;
				dist2 = _normal[0]*bbox._maxs[0] + _normal[1]*bbox._mins[1] + _normal[2]*bbox._mins[2] - _dist;
				break;
				
			case 2:
				dist1 = _normal[0]*bbox._maxs[0] + _normal[1]*bbox._mins[1] + _normal[2]*bbox._maxs[2] - _dist;
				dist2 = _normal[0]*bbox._mins[0] + _normal[1]*bbox._maxs[1] + _normal[2]*bbox._mins[2] - _dist;
				break;
				
			case 3:
				dist1 = _normal[0]*bbox._mins[0] + _normal[1]*bbox._mins[1] + _normal[2]*bbox._maxs[2] - _dist;
				dist2 = _normal[0]*bbox._maxs[0] + _normal[1]*bbox._maxs[1] + _normal[2]*bbox._mins[2] - _dist;
				break;
			
			case 4:
				dist1 = _normal[0]*bbox._maxs[0] + _normal[1]*bbox._maxs[1] + _normal[2]*bbox._mins[2] - _dist;
				dist2 = _normal[0]*bbox._mins[0] + _normal[1]*bbox._mins[1] + _normal[2]*bbox._maxs[2] - _dist;
				break;
		
			case 5:
				dist1 = _normal[0]*bbox._mins[0] + _normal[1]*bbox._maxs[1] + _normal[2]*bbox._mins[2] - _dist;
				dist2 = _normal[0]*bbox._maxs[0] + _normal[1]*bbox._mins[1] + _normal[2]*bbox._maxs[2] - _dist;
				break;
			
			case 6:
				dist1 = _normal[0]*bbox._maxs[0] + _normal[1]*bbox._mins[1] + _normal[2]*bbox._mins[2] - _dist;
				dist2 = _normal[0]*bbox._mins[0] + _normal[1]*bbox._maxs[1] + _normal[2]*bbox._maxs[2] - _dist;
				break;
				
			case 7:
				dist1 = _normal[0]*bbox._mins[0] + _normal[1]*bbox._mins[1] + _normal[2]*bbox._mins[2] - _dist;
				dist2 = _normal[0]*bbox._maxs[0] + _normal[1]*bbox._maxs[1] + _normal[2]*bbox._maxs[2] - _dist;
				break;
		
			default:
				dist1 = dist2 = 0;		// shut up compiler
				assert(0);
				break;
		}
	}
	else
	{
		vec3_c	corners[2];
	
		corners[0][0] = (_normal[0] < 0) ? bbox._mins[0] : bbox._maxs[0];
		corners[0][1] = (_normal[1] < 0) ? bbox._mins[1] : bbox._maxs[1];
		corners[0][2] = (_normal[2] < 0) ? bbox._mins[2] : bbox._maxs[2];
	
		corners[1][0] = (_normal[0] < 0) ? bbox._maxs[0] : bbox._mins[0];
		corners[1][1] = (_normal[1] < 0) ? bbox._maxs[1] : bbox._mins[1];
		corners[1][2] = (_normal[2] < 0) ? bbox._maxs[2] : bbox._mins[2];
		
		dist1 = _normal.dotProduct(corners[0]) - _dist;
		dist2 = _normal.dotProduct(corners[1]) - _dist;
	}

	bool front = (dist1 >= 0);
	
	if(dist2 < 0)
	{
		if(front)
			return SIDE_CROSS;
		else
			return SIDE_BACK;
	}
		
	return SIDE_FRONT;
}

	
	
const char*	cplane_c::toString() const
{
	return va("(%i %i %i %i)", (int)_normal[0], (int)_normal[1], (int)_normal[2], (int)_dist);
}
	
bool	cplane_c::operator == (const cplane_c &p) const
{
#if 1
		if(	X_fabs(p._normal[0] - _normal[0]) < NORMAL_EPSILON
			&& X_fabs(p._normal[1] - _normal[1]) < NORMAL_EPSILON
			&& X_fabs(p._normal[2] - _normal[2]) < NORMAL_EPSILON
			&& X_fabs(p._dist - _dist) < DIST_EPSILON )
			return true;
#else
		if(	p._normal[0] == _normal[0]
			&& p._normal[1] == _normal[1]
			&& p._normal[2] == _normal[2]
			&& p._dist == dist)
			return true;
#endif
		return false;
}





void 	Vector3_MakeNormalVectors(const vec3_c &forward, vec3_c &right, vec3_c &up)
{
	float		d;

	// this rotate and negat guarantees a vector
	// not colinear with the original
	right[1] = -forward[0];
	right[2] = forward[1];
	right[0] = forward[2];

	d = right.dotProduct(forward);
	vec3_c f = forward;
	f *= -d;
	right += f;
	//right.normalizeFast();
	right.normalize();
	up.crossProduct(right, forward);
}



void 	Vector3_ProjectOnPlane(vec3_c &dst, const vec3_c &p, const vec3_c &normal)
{
	float d;
	vec3_c n;
	float inv_denom;

	inv_denom = 1.0F / normal.dotProduct(normal);

	d = normal.dotProduct(p) * inv_denom;

	n = normal;
	n *= inv_denom;
	n *= d;
	
	dst = p - n;
}


void 	Vector3_Perpendicular(vec3_c &dst, const vec3_c &src)
{
	int	pos;
	int i;
	float minelem = 1.0F;
	vec3_c tempvec;

	//
	// find the smallest magnitude axially aligned vector
	//
	for(pos=0, i=0; i < 3; i++)
	{
		if(fabs( src[i] ) < minelem )
		{
			pos = i;
			minelem = fabs(src[i]);
		}
	}
	
	tempvec.clear();

	tempvec[pos] = 1.0;

	//
	// project the point onto the plane defined by src
	//
	Vector3_ProjectOnPlane(dst, tempvec, src);

	//
	// normalize the result
	//
	dst.normalize();
}

void	Vector3_ToAngles(const vec3_c &value1, vec3_c &angles)
{
	float	forward;
	float	yaw, pitch;
	
	if (value1[1] == 0 && value1[0] == 0)
	{
		yaw = 0;
		if (value1[2] > 0)
			pitch = 90;
		else
			pitch = 270;
	}
	else
	{
		if (value1[0])
			yaw = (int) (atan2(value1[1], value1[0]) * 180 / M_PI);
		else if (value1[1] > 0)
			yaw = 90;
		else
			yaw = -90;
		if (yaw < 0)
			yaw += 360;

		forward = X_sqrt(value1[0]*value1[0] + value1[1]*value1[1]);
		pitch = (int) (atan2(value1[2], forward) * 180 / M_PI);
		if (pitch < 0)
			pitch += 360;
	}

	angles[PITCH] = -pitch;
	angles[YAW] = yaw;
	angles[ROLL] = 0;
}

void	Vector3_PlaneSpace(const vec3_t n, vec3_t p, vec3_t q)
{
	assert(n && p && q);
	
	if(X_fabs(n[2]) > M_SQRT1_2)
	{
		// choose p in y-z plane
		vec_t a = n[1]*n[1] + n[2]*n[2];
		vec_t k = X_recipsqrt(a);
		
		p[0] = 0;
		p[1] = -n[2]*k;
		p[2] = n[1]*k;
		
		// set q = n x p
		q[0] = a*k;
		q[1] = -n[0]*p[2];
		q[2] = n[0]*p[1];
	}
	else
	{
		// choose p in x-y plane
		vec_t a = n[0]*n[0] + n[1]*n[1];
		vec_t k = X_recipsqrt (a);
		 
		p[0] = -n[1]*k;
		p[1] = n[0]*k;
		p[2] = 0;
		 
		// set q = n x p
		q[0] = -n[2]*p[1];
		q[1] = n[2]*p[0];
		q[2] = a*k;
	}
}


/*
=============
VectorToString

This is just a convenience function
for printing vectors
=============
*/
char*	Vector3_String(const vec3_t v)
{
	return va("(%i %i %i)", (int)v[0], (int)v[1], (int)v[2]);
}




static int	Curve_TestFlatness(int maxflat, vec4_t point0, vec4_t point1, vec4_t point2)
{
	vec3_t		v1, v2, v3;
	vec3_t		t, n;
	float		dist, d, len;
	int			ft0, ft1;
	
	Vector3_Subtract(point2, point0, n);
	len = Vector3_Normalize(n);
	
	if (!len)
		return 0;
	
	Vector3_Subtract(point1, point0, t);
	d = -Vector3_DotProduct(t, n);
	Vector3_MA(t, d, n, t);
	dist = Vector3_Length(t);
	
	if(fabs (dist) <= maxflat)
		return 0;
	
	Vector3_Average(point1, point0, v1);
	Vector3_Average(point2, point1, v2);
	Vector3_Average(v1, v2, v3);
	
	ft0 = Curve_TestFlatness(maxflat, point0, v1, v3);
	ft1 = Curve_TestFlatness(maxflat, v3, v2, point2);
	
	return 1 + (int)floor(X_max(ft0, ft1) + 0.5f);
}

void	Curve_GetFlatness(int maxflat, vec4_c *points, int *mesh_cp, int *flat)
{
	int			i, p;
	int			u, v;
	
	flat[0] = flat[1] = 0;
	
	for(v=0; v<mesh_cp[1]-1; v += 2)
	{
		for(u=0; u<mesh_cp[0]-1; u += 2)
		{
			p = v * mesh_cp[0] + u;
			
			i = Curve_TestFlatness(maxflat, points[p], points[p+1], points[p+2]);
			flat[0] = X_max(flat[0], i);
			
			i = Curve_TestFlatness(maxflat, points[p+mesh_cp[0]], points[p+mesh_cp[0]+1], points[p+mesh_cp[0]+2]);
			flat[0] = X_max(flat[0], i);
			
			i = Curve_TestFlatness(maxflat, points[p+2*mesh_cp[0]], points[p+2*mesh_cp[0]+1], points[p+2*mesh_cp[0]+2]);
			flat[0] = X_max(flat[0], i);
			
			
			
			i = Curve_TestFlatness(maxflat, points[p], points[p+mesh_cp[0]], points[p+2*mesh_cp[0]]);
			flat[1] = X_max (flat[1], i);
			
			i = Curve_TestFlatness(maxflat, points[p+1], points[p+mesh_cp[0]+1], points[p+2*mesh_cp[0]+1]);
			flat[1] = X_max(flat[1], i);
			
			i = Curve_TestFlatness(maxflat, points[p+2], points[p+mesh_cp[0]+2], points[p+2*mesh_cp[0]+2]);
			flat[1] = X_max(flat[1], i);
			
			
		}
	}
}


static void	Curve_EvalQuadricBezier(float t, vec4_t point0, vec4_t point1, vec4_t point2, vec4_t outa)
{
	float qt = t * t;
	float dt = 2.0f * t;
	float tt;
	
	vec4_c	tmp;
	vec4_c	out;
	
	tt = 1.0f - dt + qt;
	out = point0;
	out.scale(tt);
		
	
	tt = dt - 2.0f * qt;
	tmp = point1;
	tmp.scale(tt);
	out += tmp;
	
	tmp = point2;
	tmp.scale(qt);
	out += tmp;
	
	out.copyTo(outa);
}

void	Curve_EvalQuadricBezierPatch(vec4_c *points, int *mesh_cp, int *tess, vec4_c *out)
{
	int		patches_num[2];
	int		tess_num[2];
	
	int		index[3];
	int		dstpitch;
	
	int		i, u, v, x, y;
	
	float	s, t, step[2];
	
	vec4_c*	tvec, pv[3][3], v1, v2, v3;
	
	patches_num[0] = mesh_cp[0] / 2;
	patches_num[1] = mesh_cp[1] / 2;
	
	dstpitch = patches_num[0] * tess[0] + 1;
	
	step[0] = 1.0f / (float)tess[0];
	step[1] = 1.0f / (float)tess[1];
	
	for(v=0; v<patches_num[1]; v++)
	{
		// last patch has one more row
		if(v < patches_num[1] - 1)
			tess_num[1] = tess[1];
		else
			tess_num[1] = tess[1] + 1;
		
		
		for(u=0; u<patches_num[0]; u++)
		{
			// last patch has one more column
			if(u < patches_num[0] - 1)
				tess_num[0] = tess[0];
			else
				tess_num[0] = tess[0] + 1;
			
			index[0] = (v * mesh_cp[0] + u) * 2;
			index[1] = index[0] + mesh_cp[0];
			index[2] = index[1] + mesh_cp[0];
			
			// current 3x3 patch control points
			for(i=0; i<3; i++)
			{
				Vector4_Copy(points[index[0]+i], pv[i][0]);
				Vector4_Copy(points[index[1]+i], pv[i][1]);
				Vector4_Copy(points[index[2]+i], pv[i][2]);
			}
			
			t = 0.0f;
			tvec = out + v * tess[1] * dstpitch + u * tess[0];
			
			for(y=0; y<tess_num[1]; y++, t += step[1])
			{
				Curve_EvalQuadricBezier(t, pv[0][0], pv[0][1], pv[0][2], v1);
				Curve_EvalQuadricBezier(t, pv[1][0], pv[1][1], pv[1][2], v2);
				Curve_EvalQuadricBezier(t, pv[2][0], pv[2][1], pv[2][2], v3);
				
				s = 0.0f;
				
				for(x=0; x<tess_num[0]; x++, s += step[0])
				{
					Curve_EvalQuadricBezier (s, v1, v2, v3, tvec[x]);
				}
				
				tvec += dstpitch;
			}
		}
	}
}


void 	RotatePointAroundVector(vec3_c &dst, const vec3_c &dir, const vec3_c &point, vec_t degrees)
{
	// Tr3B - make it simpler and faster
	matrix_c	m;
	matrix_c	im;
	matrix_c	zrot;
	matrix_c	tmpmat;
	matrix_c	rot;
	
	vec3_c	vr, vup, vf;

	vf = dir;
	Vector3_Perpendicular(vr, vf);
	vup.crossProduct(vr, vf);

#if 1
	m[0][0] = vr[0];	m[0][1] = vup[0];	m[0][2] = vf[0];	m[0][3]=0;
	m[1][0] = vr[1];	m[1][1] = vup[1];	m[1][2] = vf[1];	m[1][3]=0;
	m[2][0] = vr[2];	m[2][1] = vup[2];	m[2][2] = vf[2];	m[2][3]=0;
	m[3][0] = 0;		m[3][1] = 0;		m[3][2] = 0;		m[3][3]=1;
#else
	//m.identity();
	m[0] = vf;
	m[1] = vr;	m[1].negate();
	m[2] = vup;
#endif
	
	m.copyTranspose(im);

	zrot.setupZRotation(degrees);
	
	tmpmat = m * zrot;
	rot = tmpmat * im;
	
	dst = rot * point;
}



void 	Angles_ToVectors(const vec3_c &angles, vec3_c &forward, vec3_c &right, vec3_c &up)
{
	matrix_c	matrix;
	
	matrix.fromAngles(angles);
	matrix.toVectorsFRU(forward, right, up);
}


void 	Angles_ToVectorsFLU(const vec3_c &angles, vec3_c &forward, vec3_c &left, vec3_c &up)
{
	matrix_c	matrix;
	
	matrix.fromAngles(angles);
	matrix.toVectorsFLU(forward, left, up);
}






	
float	 LerpAngle(float a2, float a1, float frac)
{
	if (a1 - a2 > 180)
		a1 -= 360;
	if (a1 - a2 < -180)
		a1 += 360;
	return a2 + frac * (a1 - a2);
}


float	anglemod(float a)
{
#if 0
	if (a >= 0)
		a -= 360*(int)(a/360);
	else
		a += 360*( 1 + (int)(-a/360) );
#endif
	a = (360.0/65536) * ((int)(a*(65536/360.0)) & 65535);
	return a;
}


float	CalcFOV(float fov_x, float width, float height)
{
	float	a;
	float	x;

	//if(fov_x < 1 || fov_x > 179)
	//	cgi.Com_Error(ERR_DROP, "CalcFOV: bad field of view: %f", fov_x);
	
	X_clamp(fov_x, 1, 179);

	x = width/tan(fov_x/360*M_PI);

	a = atan (height/x);

	a = a*360/M_PI;

	return a;
}

/*
float	X_frand()
{
	return (rand() & 32767)* (1.0/32767);
}

float	X_crand()
{
	return (rand() & 32767)* (2.0/32767) - 1;
}
*/

static unsigned long	seed = 0;

unsigned long	X_rand()
{
	seed = (1664525L * seed + 1013904223L) & 0xffffffff;
	return seed;
}


unsigned long	X_randGetSeed()
{
	return seed;
}

void		X_randSetSeed(unsigned long s)
{
	seed = s;
}


int	X_randTest()
{
	unsigned long oldseed = seed;
	int ret = 1;
	seed = 0;
	
	if(	X_rand() != 0x3c6ef35f || X_rand() != 0x47502932 ||
		X_rand() != 0xd1ccf6e9 || X_rand() != 0xaaf95334 ||
		X_rand() != 0x6252e503
	)
	{
		ret = 0;
	}
		
	seed = oldseed;
	
	return ret;
}

int	X_irand(int n)
{
	double a = double(n) / 4294967296.0;
	return (int) (double(X_rand()) * a);
}

vec_t	X_frand()
{
	return ((vec_t) X_rand()) / ((vec_t) 0xffffffff);
}

vec_t	X_crand()
{
	return (REAL(2.0) * (X_frand() - REAL(0.5)));
}

