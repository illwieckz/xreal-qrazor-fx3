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
#ifndef X_MATH_H
#define X_MATH_H

/// includes ===================================================================
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
// system -------------------------------------------------------------------
#include <limits>
#include <iostream>

// shared -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
// xreal --------------------------------------------------------------------


#ifndef DOUBLEVEC_T
#define REAL(x)			(x ## f)	// form a constant
#else
#define REAL(x)			(x)
#endif

#ifdef DOUBLEVEC_T
typedef double	vec_t;
#else
typedef float	vec_t;
#endif

typedef vec_t	vec2_t[2];	//used for texture coordinates
typedef vec_t	vec3_t[3];	
typedef vec_t	vec4_t[4];
typedef vec_t	vec5_t[5];

#if defined(__GNUC__) && !defined(DOUBLE_VEC_T) && defined(SIMD_BUILTIN)
typedef int vec1sf_t		__attribute__((mode(SF)));
typedef int vec2sf_t		__attribute__((mode(V2SF)));	// vector of two  single floats
struct      vec3sf_t
{
	vec2sf_t	xy;
	vec1sf_t	z;
};
typedef int vec4sf_t		__attribute__((mode(V4SF)));	// vector of four single floats
#endif

class vec2_c;
class vec3_c;
class vec4_c;

class matrix_c;
class quaternion_c;


// colors
extern const vec4_c	color_black;
extern const vec4_c	color_red;
extern const vec4_c	color_green;
extern const vec4_c	color_blue;
extern const vec4_c	color_yellow;
extern const vec4_c	color_magenta;
extern const vec4_c	color_cyan;
extern const vec4_c	color_white;
extern const vec4_c	color_grey_lite;
extern const vec4_c	color_grey_med;
extern const vec4_c	color_grey_dark;

#define Q_COLOR_ESCAPE		'^'
#define Q_IsColorString(p)	(p && *(p) == Q_COLOR_ESCAPE && *((p)+1) && *((p)+1) != Q_COLOR_ESCAPE )

#define COLOR_BLACK		'0'
#define COLOR_RED		'1'
#define COLOR_GREEN		'2'
#define COLOR_YELLOW		'3'
#define COLOR_BLUE		'4'
#define COLOR_CYAN		'5'
#define COLOR_MAGENTA		'6'
#define COLOR_WHITE		'7'
#define ColorIndex(c)		(((c) - '0') & 7)

#define S_COLOR_BLACK		"^0"
#define S_COLOR_RED		"^1"
#define S_COLOR_GREEN		"^2"
#define S_COLOR_YELLOW		"^3"
#define S_COLOR_BLUE		"^4"
#define S_COLOR_CYAN		"^5"
#define S_COLOR_MAGENTA		"^6"
#define S_COLOR_WHITE		"^7"



#define NUMVERTEXNORMALS	162
const extern	vec3_t	bytedirs[NUMVERTEXNORMALS];


// angle indexes
enum angles_indexes_e
{
	PITCH,		// up / down
	YAW,		// left / right
	ROLL		// fall over
};

enum plane_side_e
{
	SIDE_FRONT		= 0,
	SIDE_ON			= 2,
	SIDE_BACK		= 1,
	SIDE_CROSS		= 3
};

enum plane_type_e
{
	// 0-2 are axial planes
	PLANE_X			= 0,
	PLANE_Y			= 1,
	PLANE_Z			= 2,

	// 3-5 are non-axial planes snapped to the nearest
	PLANE_ANYX		= 3,
	PLANE_ANYY		= 4,
	PLANE_ANYZ		= 5
};

// planes (x&~1) and (x&~1)+1 are always opposites

#ifndef M_PI
#define M_PI			3.14159265358979323846	// matches value in gcc v2 math.h
#endif
/*
#ifndef M_PI
#define M_PI 			REAL(3.1415926535897932384626433832795029)
#endif
*/

#ifndef M_TWOPI
#define M_TWOPI			6.28318530717958647692
#endif

#ifndef M_SQRT1_2
#define M_SQRT1_2		REAL(0.7071067811865475244008443621048490)
#endif

//Tr3B - values from Nvidia's math lib
typedef vec_t X_scalar;

#define X_zero			X_scalar(0)
#define X_zero_5		X_scalar(0.5)
#define X_one			X_scalar(1.0)
#define X_two			X_scalar(2)
#define X_half_pi		X_scalar(3.14159265358979323846264338327950288419716939937510582 * 0.5)
#define X_quarter_pi		X_scalar(3.14159265358979323846264338327950288419716939937510582 * 0.25)
#define X_pi			X_scalar(3.14159265358979323846264338327950288419716939937510582)
#define X_two_pi		X_scalar(3.14159265358979323846264338327950288419716939937510582 * 2.0)
#define X_oo_pi			X_one / X_pi
#define X_oo_two_pi		X_one / X_two_pi
#define X_oo_255		X_one / X_scalar(255)
#define X_oo_128		X_one / X_scalar(128)
#define X_to_rad		X_pi / X_scalar(180)
#define X_to_deg		X_scalar(180) / X_pi
#define X_eps			X_scalar(10e-6)
#define X_double_eps		X_scalar(10e-6) * X_two
#define X_big_eps		X_scalar(10e-6)
#define X_small_eps		X_scalar(10e-2)


extern const vec_t		X_infinity;

extern const vec3_c		vec3_origin;		// null vector

extern const matrix_c		matrix_identity;

extern const quaternion_c	quat_identity;



#define	nanmask (255<<23)

#define	IS_NAN(x) (((*(int *)&x)&nanmask)==nanmask)


#define DEGTORAD(x) ((x*M_PI)/180.0f)
#define RADTODEG(x) ((x*180.0f)/M_PI)


#define	NORMAL_EPSILON	0.00001
#define	DIST_EPSILON	0.01


#define X_max(a,b) ((a) > (b) ? (a) : (b))

#define X_min(a,b) ((a) < (b) ? (a) : (b))

#define X_bound(a,b,c) (X_max(a, X_min(b, c)))

#define X_clamp(a,b,c) ((b) >= (c) ? (a)=(b) : (a) < (b) ? (a)=(b) : (a) > (c) ? (a)=(c) : (a))


#define X_ftol(f) (long)(f)


//! reciprocal
vec_t	X_recip(vec_t x);

//! square root
vec_t	X_sqrt(vec_t x);

//! reciprocal square root
vec_t	X_recipsqrt(vec_t x);

//! sine
inline vec_t	X_sin(vec_t x)
{
#ifdef DOUBLEVEC_T
	return sin(x);
#else
	return ((float)sinf(float(x)));
#endif
}

//! cosine
inline vec_t	X_cos(vec_t x)
{
#ifdef DOUBLEVEC_T
	return cos(x);
#else
	return ((float)cosf(float(x)));
#endif
}

//! absolute value
inline vec_t	X_fabs(vec_t x)
{
#ifdef DOUBLEVEC_T
	return fabs(x);
#else
	return ((float)fabsf(float(x)));
#endif
}

//! arc tangent with 2 args
inline vec_t	X_atan2(vec_t y, vec_t x)
{
#ifdef DOUBLEVEC_T
	return atan2(y, x);
#else
	return ((float)atan2f(float(y), float(x)));
#endif
}

//! modulo
inline vec_t	X_fmod(vec_t x, vec_t y)
{
#ifdef DOUBLEVEC_T
	return fmod(x);
#else
	return ((float)fmod(float(x), float(y)));
#endif
}

inline vec_t	X_copysign(vec_t x, vec_t y)
{
#ifdef DOUBLEVEC_T
	return copysign(x, y);
#else
	return ((float)copysignf(float(x), float(y)));
#endif
}


inline vec_t	X_rint(const vec_t in)
{
	return floor(in + 0.5);
}

inline vec_t	X_sqr(vec_t v)
{
	return(v*v);
}



class vec2_c
{
public:
	inline vec2_c(bool clear = true)
	{
		if(clear)
			_v[0] = _v[1] = 0.0;
	}
	
	inline vec2_c(vec_t x, vec_t y)
	{
		_v[0] = x; 
		_v[1] = y;
	}
	
	inline void	clear()
	{
		_v[0] = _v[1] = 0.0;
	}
	
	inline void	set(vec_t x, vec_t y)
	{
		_v[0] = x; 
		_v[1] = y;
	}
	
	inline void	negate()
	{
		_v[0] = -_v[0];
		_v[1] = -_v[1];
	}
	
	inline void	scale(const vec_t s)
	{
		_v[0] *= s;
		_v[1] *= s;
	}
		
	inline void	copyTo(vec_t *v) const
	{
		v[0] = _v[0];
		v[1] = _v[1];
	}
	
	inline void	copyToFloat(float *v) const
	{
		v[0] = _v[0];
		v[1] = _v[1];
	}
	
	inline bool 	isZero() const
	{
		if (_v[0] == 0 && _v[1] == 0 && _v[2] == 0 && _v[3] == 0)
			return true;
		else
			return false;
	}
	
	const char*	toString() const
	{
		return va("(%i %i)", (int)_v[0], (int)_v[1]);
	}


	inline bool	operator == (const vec2_c &v) const
	{
		if(_v[0] != v._v[0] || _v[1] != v._v[1])
			return false;
		else
			return true;
	}
	
	inline bool	operator != (const vec2_c &v) const
	{
		if(_v[0] != v._v[0] || _v[1] != v._v[1])
			return true;
		else
			return false;
	}
	
	inline vec2_c	operator - (const vec2_c &v) const
	{
		return vec2_c(_v[0] - v._v[0], v[1] - v._v[1]);
	}
	
	inline vec2_c	operator + (const vec2_c &v) const
	{
		return vec2_c(_v[0] + v._v[0], _v[1] + v._v[1]);
	}
	
	inline vec2_c	operator * (const vec2_c &v) const
	{
		return vec2_c(_v[0] * v._v[0], _v[1] * v._v[1]);
	}
	
	inline vec2_c	operator / (const vec2_c &v) const
	{
		return vec2_c(_v[0] / v._v[0], _v[1] / v._v[1]);
	}
	
	inline vec2_c&	operator -= (const vec2_c &v)
	{
		_v[0] -= v._v[0];
		_v[1] -= v._v[1];
		
		return *this;
	}
	
	inline vec2_c&	operator += (const vec2_c &v)
	{
		_v[0] += v._v[0];
		_v[1] += v._v[1];
		
		return *this;
	}
	
	inline vec2_c&	operator *= (const vec2_c &v)
	{
		_v[0] *= v._v[0];
		_v[1] *= v._v[1];
		
		return *this;
	}
	
	inline vec2_c&	operator /= (const vec2_c &v)
	{
		_v[0] /= v._v[0];
		_v[1] /= v._v[1];
		
		return *this;
	}

	inline vec_t	operator [] (const int index) const
	{
		return _v[index];
	}

	inline vec_t&	operator [] (const int index)
	{
		return _v[index];
	}
	
	inline operator vec_t * () const
	{
		return (vec_t*)_v;
	}

	inline operator vec_t * ()
	{
		return (vec_t*)_v;
	}
	
	inline vec2_c&	operator = (const vec2_c &v)
	{
		_v[0] = v._v[0];
		_v[1] = v._v[1];
		
		return *this;
	}
	
	inline vec2_c&	operator = (const vec_t *v)
	{
		_v[0] = v[0];
		_v[1] = v[1];
		
		return *this;
	}

private:
	vec2_t	_v;
};





inline void 	Vector3_Subtract(const vec3_t veca, const vec3_t vecb, vec3_t out)
{
	out[0] = veca[0]-vecb[0];
	out[1] = veca[1]-vecb[1];
	out[2] = veca[2]-vecb[2];
}


inline void 	Vector3_Add(const vec3_t veca, const vec3_t vecb, vec3_t out)
{
	out[0] = veca[0]+vecb[0];
	out[1] = veca[1]+vecb[1];
	out[2] = veca[2]+vecb[2];
}


inline void 	Vector3_Copy(const vec3_t in, vec3_t out)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
}


inline void	Vector3_Clear(vec3_t v)
{
	v[0] = v[1] = v[2] = 0.0F;
}

inline void	Vector3_Negate(vec3_t v1, vec3_t v2)
{
	v2[0] = -v1[0];
	v2[1] = -v1[1];
	v2[2] = -v1[2];
}

inline void	Vector3_Set(vec3_t v, float x, float y, float z)
{
	v[0] = x;
	v[1] = y;
	v[2] = z;
}

inline vec_t 	Vector3_DotProduct(const vec3_t v1, const vec3_t v2)
{
	return (v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2]);
}

inline vec_t 	Vector3_Length(const vec3_t v)
{
	return X_sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

inline vec_t 	Vector3_Normalize(vec3_t v)
{
	vec_t len = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
	
	if(len)
	{
		len = X_sqrt(len);
		
		v[0] /= len;
		v[1] /= len;
		v[2] /= len;
	}
	
	return len;
}

inline vec_t 	Vector3_Normalize2(const vec3_t v, vec3_t out)
{
	vec_t len = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
	
	if(len)
	{
		len = X_sqrt(len);
		
		out[0] = v[0] / len;
		out[1] = v[1] / len;
		out[2] = v[2] / len;	
	}
	
	return len;
}

inline void 	Vector3_MA(const vec3_t veca, vec_t scale, const vec3_t vecb, vec3_t vecc)
{
	vecc[0] = veca[0] + scale*vecb[0];
	vecc[1] = veca[1] + scale*vecb[1];
	vecc[2] = veca[2] + scale*vecb[2];
}

inline void 	Vector3_Scale(const vec3_t in, const vec_t scale, vec3_t out)
{
	out[0] = in[0]*scale;
	out[1] = in[1]*scale;
	out[2] = in[2]*scale;
}

inline void	Vector3_Average(const vec3_t v1, const vec3_t v2, vec3_t out)
{
	out[0] = (v1[0]+v2[0]) * 0.5f;
	out[1] = (v1[1]+v2[1]) * 0.5f;
	out[2] = (v1[2]+v2[2]) * 0.5f;
}


void 	Vector3_MakeNormalVectors(const vec3_c &forward, vec3_c &right, vec3_c &up);
void 	Vector3_ProjectOnPlane(vec3_c &dst, const vec3_c &p, const vec3_c &normal);
void 	Vector3_Perpendicular(vec3_c &dst, const vec3_c &src);
void	Vector3_ToAngles(const vec3_c &value1, vec3_c &angles);

/*
 * given a unit length "normal" vector n, generate vectors p and q vectors
 * that are an orthonormal basis for the plane space perpendicular to n.
 * i.e. this makes p,q such that n,p,q are all perpendicular to each other.
 * q will equal n x p. if n is not unit length then p will be unit length but
 * q wont be.
 */
void	Vector3_PlaneSpace(const vec3_t n, vec3_t p, vec3_t q);

char*	Vector3_String(const vec3_t v);



inline void 	Vector4_Copy(const vec4_t in, vec4_t out)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
	out[3] = in[3];
}


class vec3_c
{
	friend class vec4_c;
public:
	inline vec3_c(bool clear = true);
	
	inline vec3_c(vec_t x, vec_t y, vec_t z);
	
	inline vec3_c(const vec_t *v);
	
	inline vec3_c(const vec3_c& v);
	
	inline void	clear();
	
	inline void	set(vec_t x, vec_t y, vec_t z);
	
	inline void	negate();
	
	inline void	scale(const float s);
		
	inline void	copyTo(vec_t *v) const;
	
	inline void	copyToFloat(float *v) const;
	
	inline bool 	isZero() const;
	
	vec_t	length() const;

	vec_t	normalize();
	
	vec_t	dotProduct(const vec3_c &v) const;

	vec3_c& 	crossProduct(const vec3_c &v1, const vec3_c &v2);
	
	inline vec_t	distance(const vec3_c &v) const;
	
	inline void	average(const vec3_c &v1, const vec3_c &v2);

	inline void	rotate(const vec3_c &angles);
	
	inline void	rotate(const matrix_c &m);
	
	inline void	rotate(const quaternion_c &q);
	
	inline void	transform(const matrix_c &m);
	
	inline void	lerp(const vec3_c &from, const vec3_c &to, vec_t f);
	
	inline void	backlerp(const vec3_c &from, const vec3_c &to, vec_t f);
	
	void		snap();
	
	const char*	toString() const;	
	
	inline bool	operator == (const vec3_c &v) const;
	
	inline bool	operator != (const vec3_c &v) const;	
	
	inline vec3_c	operator - (const vec3_c &v) const;
	
	inline vec3_c	operator + (const vec3_c &v) const;
	
	inline vec3_c	operator * (const vec3_c &v) const;
	
	inline vec3_c	operator * (const float s) const;
		
	inline vec3_c&	operator -= (const vec3_c &v);
	
	inline vec3_c&	operator += (const vec3_c &v);
	
	inline vec3_c&	operator *= (const vec3_c &v);
	
	inline vec3_c&	operator *= (const float s);

	inline vec_t	operator [] (const int index) const;

	inline vec_t&	operator [] (const int index);
	
	inline operator vec_t * () const;

	inline operator vec_t * ();
			
	vec3_c&	operator = (const vec3_c &v);
	
	vec3_c&	operator = (const vec4_c &v);
		
	vec3_c&	operator = (const vec_t *v);

private:
	vec3_t	_v;
};


inline vec3_c::vec3_c(bool clear)
{
	if(clear)
		_v[0] = _v[1] = _v[2] = 0.0;
}
	
inline vec3_c::vec3_c(vec_t x, vec_t y, vec_t z)
{
	set(x, y, z);
}

inline vec3_c::vec3_c(const vec3_c& v)
{
	set(v._v[0], v._v[1], v._v[2]);
}

vec3_c::vec3_c(const vec_t *v)
{
//	assert(false);

//	Com_Error(ERR_FATAL, "vec3_c::(const vec_t *v): called @ file %s, line %i", s, line);
	
//	std::cerr << "vec3_c::(const vec_t *v): called @ " << s << "line " << line << std::endl;
//	exit(1);

	set(v[0], v[1], v[2]);
}
	
inline void	vec3_c::clear()
{
	set(0, 0, 0);
}

inline void	vec3_c::set(vec_t x, vec_t y, vec_t z)
{
	_v[0] = x; 
	_v[1] = y;
	_v[2] = z;
}
	
inline void	vec3_c::negate()
{
	_v[0] = -_v[0];
	_v[1] = -_v[1];
	_v[2] = -_v[2];
}
	
inline void	vec3_c::scale(const float s)
{
	_v[0] *= s;
	_v[1] *= s;
	_v[2] *= s;
}
	
inline void	vec3_c::copyTo(vec_t *v) const
{
	v[0] = _v[0];
	v[1] = _v[1];
	v[2] = _v[2];
}
	
inline void	vec3_c::copyToFloat(float *v) const
{
	v[0] = _v[0];
	v[1] = _v[1];
	v[2] = _v[2];
}

inline bool	vec3_c::isZero() const
{
	if(_v[0] == 0 && _v[1] == 0 && _v[2] == 0)
		return true;
	else
		return false;
}

inline vec_t	vec3_c::distance(const vec3_c &v) const
{
	vec3_c tmp = *this - v;
		
	return tmp.length();
}
	
inline void	vec3_c::average(const vec3_c &v1, const vec3_c &v2)
{
	_v[0] = (v1._v[0]+v2._v[0]) * 0.5f;
	_v[1] = (v1._v[1]+v2._v[1]) * 0.5f;
	_v[2] = (v1._v[2]+v2._v[2]) * 0.5f;
}

inline void	vec3_c::lerp(const vec3_c &from, const vec3_c &to, vec_t f)
{
	*this = from + ((to - from) * f);
}
	
inline void	vec3_c::backlerp(const vec3_c &from, const vec3_c &to, vec_t f)
{
	*this = to + ((from - to) * f);
}

inline bool	vec3_c::operator == (const vec3_c &v) const
{
	if(_v[0] != v._v[0] || _v[1] != v._v[1] || _v[2] != v._v[2])
		return false;
			
	return true;
}
	
inline bool	vec3_c::operator != (const vec3_c &v) const
{
	if(_v[0] != v._v[0] || _v[1] != v._v[1] || _v[2] != v._v[2])
		return true;
	else
		return false;
}

inline vec3_c	vec3_c::operator - (const vec3_c &v) const
{
	return vec3_c(	_v[0] - v._v[0], 
			_v[1] - v._v[1],
			_v[2] - v._v[2]	);
}
	
inline vec3_c	vec3_c::operator + (const vec3_c &v) const
{
	return vec3_c(	_v[0] + v._v[0], 
			_v[1] + v._v[1],
			_v[2] + v._v[2]	);
}
		
// this modulates vector instead of calculating the dot product !!!
inline vec3_c	vec3_c::operator * (const vec3_c &v) const
{
	return vec3_c(	_v[0]*v._v[0],
			_v[1]*v._v[1],
			_v[2]*v._v[2]	);
}

inline vec3_c	vec3_c::operator * (const float s) const
{
	return vec3_c(	_v[0] * s,
			_v[1] * s,
			_v[2] * s );
}
		
inline vec3_c&	vec3_c::operator -= (const vec3_c &v)
{
	_v[0] -= v._v[0];
	_v[1] -= v._v[1];
	_v[2] -= v._v[2];
		
	return *this;
}
	
inline vec3_c&	vec3_c::operator += (const vec3_c &v)
{
	_v[0] += v._v[0];
	_v[1] += v._v[1];
	_v[2] += v._v[2];
		
	return *this;
}
	
inline vec3_c&	vec3_c::operator *= (const vec3_c &v)
{
	_v[0] *= v._v[0];
	_v[1] *= v._v[1],
	_v[2] *= v._v[2];
		
	return *this;
}
	
inline vec3_c&	vec3_c::operator *= (const float s)
{
	_v[0] *= s;
	_v[1] *= s;
	_v[2] *= s;
		
	return *this;
}

inline vec_t	vec3_c::operator [] (const int index) const
{
	if(index < 0 || index >= 3)
		Com_Error(ERR_FATAL, "vec3_c::operator []: bad index %i", index);
		
	return _v[index];
}

inline vec_t&	vec3_c::operator [] (const int index)
{
	if(index < 0 || index >= 3)
		Com_Error(ERR_FATAL, "vec3_c::operator []: bad index %i", index);
		
	return _v[index];
}

inline vec3_c::operator vec_t * () const
{
	return (vec_t*)_v;
}

inline vec3_c::operator vec_t * ()
{
	return (vec_t*)_v;
}


class vec4_c
{
public:
	inline vec4_c(bool clear = true)
	{
		if(clear)
			_v[0] = _v[1] = _v[2] = _v[3] = 0.0;
	}
	
	inline vec4_c(vec_t x, vec_t y, vec_t z, vec_t w)
	{
		set(x, y, z, w);
	}
	
	/*
	inline vec4_c(const vec_t *v)
	{
		_v[0] = v[0];
		_v[1] = v[1];
		_v[2] = v[2];
		_v[3] = v[3];
	}
	*/
	
	inline void	clear()
	{
		_v[0] = _v[1] = _v[2] = _v[3] = 0.0;
	}
	
	inline void	set(vec_t x, vec_t y, vec_t z, vec_t w)
	{
		_v[0] = x; 
		_v[1] = y;
		_v[2] = z;
		_v[3] = w;
	}
	
	inline void	negate()
	{
		_v[0] = -_v[0];
		_v[1] = -_v[1];
		_v[2] = -_v[2];
		_v[3] = -_v[4];
	}
	
	inline void	scale(const float s)
	{
		_v[0] *= s;
		_v[1] *= s;
		_v[2] *= s;
		_v[3] *= s;
	}
		
	inline void	copyTo(vec_t *v) const
	{
		v[0] = _v[0];
		v[1] = _v[1];
		v[2] = _v[2];
		v[3] = _v[3];
	}
	
	inline void	copyToFloat(float *v) const
	{
		v[0] = _v[0];
		v[1] = _v[1];
		v[2] = _v[2];
		v[3] = _v[3];
	}
		
	inline bool 	isZero() const
	{
		if (_v[0] == 0 && _v[1] == 0 && _v[2] == 0 && _v[3] == 0)
			return true;
		else
			return false;
	}

	const char*	toString() const
	{
		return va("(%i %i %i %i)", (int)_v[0], (int)_v[1], (int)_v[2], (int)_v[3]);
	}
	
	
	inline bool	operator == (const vec4_c &v) const
	{
		if(_v[0] != v._v[0] || _v[1] != v._v[1] || _v[2] != v._v[2] || _v[3] != v._v[3])
			return false;
		else
			return true;
	}
	
	inline bool	operator != (const vec4_c &v) const
	{
		if(_v[0] != v._v[0] || _v[1] != v._v[1] || _v[2] != v._v[2] || _v[3] != v._v[3])
			return true;
		else
			return false;
	}
	
	
	inline vec4_c	operator - (const vec4_c &v) const
	{
		return vec4_c(	_v[0] - v._v[0], 
				_v[1] - v._v[1],
				_v[2] - v._v[2],
				_v[3] - v._v[3]);
	}
	
	inline vec4_c	operator + (const vec4_c &v) const
	{
		return vec4_c(	_v[0] + v._v[0], 
				_v[1] + v._v[1],
				_v[2] + v._v[2],
				_v[3] + v._v[3]);
	}
	
	
	inline vec4_c&	operator -= (const vec4_c &v)
	{
		_v[0] -= v._v[0];
		_v[1] -= v._v[1];
		_v[2] -= v._v[2];
		_v[3] -= v._v[3];
		
		return *this;
	}
	
	inline vec4_c&	operator += (const vec3_c &v)
	{
		_v[0] += v._v[0];
		_v[1] += v._v[1];
		_v[2] += v._v[2];
		
		return *this;
	}
	
	inline vec4_c&	operator += (const vec4_c &v)
	{
		_v[0] += v._v[0];
		_v[1] += v._v[1];
		_v[2] += v._v[2];
		_v[3] += v._v[3];
		
		return *this;
	}
	

	inline vec_t	operator [] (const int index) const
	{
		return _v[index];
	}

	inline vec_t&	operator [] (const int index)
	{
		return _v[index];
	}
	
	inline operator vec_t * () const
	{
		return (vec_t*)_v;
	}

	inline operator vec_t * ()
	{
		return (vec_t*)_v;
	}
		
	inline vec4_c&	operator = (const vec4_c &v)
	{
		_v[0] = v._v[0];
		_v[1] = v._v[1];
		_v[2] = v._v[2];
		_v[3] = v._v[3];
		
		return *this;
	}
		
	inline vec4_c&	operator = (const vec_t *v)
	{
		_v[0] = v[0];
		_v[1] = v[1];
		_v[2] = v[2];
		_v[3] = v[3];
		
		return *this;
	}

private:
	vec4_t	_v;
};


inline vec3_c&	vec3_c::operator = (const vec4_c &v)
{
	_v[0] = v[0];
	_v[1] = v[1];
	_v[2] = v[2];
	
	return *this;
}



// Tr3B - row major matrix
class matrix_c
{
public:
	
	matrix_c();
	
	matrix_c(	vec_t m00, vec_t m01, vec_t m02, vec_t m03,
			vec_t m10, vec_t m11, vec_t m12, vec_t m13,
			vec_t m20, vec_t m21, vec_t m22, vec_t m23,
			vec_t m30, vec_t m31, vec_t m32, vec_t m33);
	
	void	identity();
	
	void	zero();
	
	void	copyTo(matrix_c &out)const ; 
		
	void	copyRotateOnly(matrix_c &out) const;
	
	void	copyTranslateOnly(matrix_c &out) const;
	
	void	copyTranspose(matrix_c &out) const;

	void	transpose();
	
	void	setupXRotation(vec_t deg);

	void	setupYRotation(vec_t deg);

	void	setupZRotation(vec_t deg);

	void	setupRotation(vec_t x, vec_t y, vec_t z, vec_t deg);	
	
	inline void	setupTranslation(const vec3_c &v);
		
	void	setupTranslation(vec_t x, vec_t y, vec_t z);

	inline void	setupScale(const vec3_c &v);
	
	void	setupScale(vec_t x, vec_t y, vec_t z);

	inline void	multiply(const matrix_c &m);
	
	inline void	multiplyRotation(const vec3_c &axis, vec_t deg);
	
	inline void	multiplyRotation(vec_t x, vec_t y, vec_t z, vec_t deg);
	
	inline void	multiplyRotation(const quaternion_c &q);
	
	inline void	multiplyRotation(const vec3_c &angles);
	
	inline void	multiplyRotation(vec_t pitch, vec_t yaw, vec_t roll);

	inline void	multiplyTranslation(const vec3_c &v);
	
	inline void	multiplyTranslation(vec_t x, vec_t y, vec_t z);
	
	inline void	multiplyScale(const vec3_c &v);
	
	inline void	multiplyScale(vec_t x, vec_t y, vec_t z);

	inline void	fromAngles(const vec3_c &angles);
	
	//! Quake style implementation
	void	fromAngles(vec_t pitch, vec_t yaw, vec_t roll);
	
	//! ODE style implementation
//	void	fromEulerAngles(vec_t phi, vec_t theta, vec_t psi);
	
	void	fromVectorsFLU(const vec3_c &forward, const vec3_c &left, const vec3_c &up);
	
	void	toVectorsFLU(vec3_c &forward, vec3_c &left, vec3_c &up) const;
	
	void	fromVectorsFRU(const vec3_c &forward, const vec3_c &right, const vec3_c &up);
	
	void	toVectorsFRU(vec3_c &forward, vec3_c &right, vec3_c &up) const;
	
	void	fromQuaternion(const quaternion_c &q);
	
	void	from2Axes(vec_t ax, vec_t ay, vec_t az, vec_t bx, vec_t by, vec_t bz);
	
	void	fromZAxis(vec_t ax, vec_t ay, vec_t az);
	
	void	lerp(const matrix_c &from, const matrix_c &to, vec_t f);
	
	matrix_c affineInverse() const;
	
	const char*	toString() const;
		
	bool	operator == (const matrix_c &m) const;

	matrix_c	operator + (const matrix_c &m) const;

	matrix_c	operator - (const matrix_c &m) const;
	
	matrix_c	operator * (const matrix_c &m) const;

	vec3_c	operator * (const vec3_c &v) const;
	
	vec4_c	operator * (const vec4_c &v) const;
	
	inline vec_t	operator () (const int i, const int j) const;
	
	inline vec_t&	operator () (const int i, const int j);
	
	inline const vec4_t&	operator [] (const int index) const;

	inline vec4_t&	operator [] (const int index);
	
	inline operator vec_t * () const;
	
	inline operator vec_t * ();
	
	matrix_c&	operator = (const matrix_c &m);

private:
#if defined(__GNUC__) && !defined(DOUBLE_VEC_T) && defined(SIMD_BUILTIN)
	union
	{
		vec4_t		_m[4];
		vec4sf_t	_ms[4];
	};
#else
	vec4_t			_m[4];
#endif
}
#if defined(SIMD_SSE) || defined(SIMD_BUILTIN)
__attribute__((aligned(16)));
#endif
;


inline void	matrix_c::setupTranslation(const vec3_c &v)
{
	setupTranslation(v[0], v[1], v[2]);
}

inline void	matrix_c::setupScale(const vec3_c &v)
{
	setupScale(v[0], v[1], v[2]);
}

inline void	matrix_c::multiply(const matrix_c &m)
{
	*this = *this * m;
}
	
inline void	matrix_c::multiplyRotation(const vec3_c &axis, vec_t deg)
{
	multiplyRotation(axis[0], axis[1], axis[2], deg);
}

inline void	matrix_c::multiplyRotation(vec_t x, vec_t y, vec_t z, vec_t deg)
{
	matrix_c rot; rot.setupRotation(x, y, z, deg);
		
	*this = *this * rot; 
}
	
inline void	matrix_c::multiplyRotation(const quaternion_c &q)
{
	matrix_c rot; rot.fromQuaternion(q);
		
	*this = *this * rot; 
}
	
inline void	matrix_c::multiplyRotation(const vec3_c &angles)
{
	multiplyRotation(angles[PITCH], angles[YAW], angles[ROLL]);
}
	
inline void	matrix_c::multiplyRotation(vec_t pitch, vec_t yaw, vec_t roll)
{
	matrix_c rot; rot.fromAngles(pitch, yaw, roll);

	*this = *this * rot; 
}

inline void	matrix_c::multiplyTranslation(const vec3_c &v)
{
	multiplyTranslation(v[0], v[1], v[2]);
}
	
inline void	matrix_c::multiplyTranslation(vec_t x, vec_t y, vec_t z)
{
	matrix_c trans; trans.setupTranslation(x, y, z);
		
	*this = *this * trans; 
}
	
inline void	matrix_c::multiplyScale(const vec3_c &v)
{
	multiplyScale(v[0], v[1], v[2]);
}
	
inline void	matrix_c::multiplyScale(vec_t x, vec_t y, vec_t z)
{
	matrix_c scale; scale.setupScale(x, y, z);
		
	*this = *this * scale;
}

inline void	matrix_c::fromAngles(const vec3_c &angles)
{
	fromAngles(angles[PITCH], angles[YAW], angles[ROLL]);
}

inline vec_t	matrix_c::operator () (const int i, const int j) const
{
	return _m[i][j];
}

inline vec_t&	matrix_c::operator () (const int i, const int j)
{
	return _m[i][j];
}
	
inline const vec4_t&	matrix_c::operator [] (const int index) const
{
	return _m[index];
}

inline vec4_t&	matrix_c::operator [] (const int index)
{
	return _m[index];
}

inline matrix_c::operator vec_t * () const
{
	return (vec_t*)&_m[0][0];
}

inline matrix_c::operator vec_t * ()
{
	return (vec_t*)&_m[0][0];
}



class quaternion_c
{
	friend class matrix_c;
	friend class message_c;
public:
	inline quaternion_c();
	
	inline quaternion_c(vec_t x, vec_t y, vec_t z, vec_t w);
	
	inline quaternion_c(const float *q);
	
	inline void	identity();
	
	inline void	set(vec_t x, vec_t y, vec_t z, vec_t w);
	
	inline void	set(vec_t x, vec_t y, vec_t z);
		
	inline void	copyTo(float *q);
	
	inline bool 	isZero() const;
	
	inline vec_t	magnitude();
	
	vec_t	normalize();
		
	inline void	inverse();
	
	inline void	antipodal();
	
	inline void	fromAngles(const vec3_c &angles);
	
	void	fromAngles(vec_t pitch, vec_t yaw, vec_t roll);
	
	void	fromMatrix(const matrix_c &m);
	
	inline void	fromAxisAngle(const vec3_c &axis, vec_t deg);
	
	//! This function takes an angle in radians !!!
	void	fromAxisAngle(vec_t x, vec_t y, vec_t z, vec_t angle);
	
	inline void	fromW(const vec3_c &w, const quaternion_c &q);
	
	void	toAxisAngle(vec3_c &axis, float *deg);
	
	void	toVectorsFLU(vec3_c &forward, vec3_c &left, vec3_c &up) const;
	
	void	toVectorsFRU(vec3_c &forward, vec3_c &right, vec3_c &up) const;
	
	//! rotate by qc, then qb
	void	multiply0(const quaternion_c &qb, const quaternion_c &qc);
	
	//! rotate by qc, then by inverse of qb
	void	multiply1(const quaternion_c &qb, const quaternion_c &qc);
	
	//! rotate by inverse of qc, then by qb
	void	multiply2(const quaternion_c &qb, const quaternion_c &qc);
	
	//! rotate by inverse of qc, then by inverse of qb
	void	multiply3(const quaternion_c &qb, const quaternion_c &qc);
	
	inline void	multiplyRotation(const vec3_c &axis, vec_t deg);
	
	inline void	multiplyRotation(vec_t x, vec_t y, vec_t z, vec_t deg);
	
	inline void	multiplyRotation(const vec3_c &angles);
	
	inline void	multiplyRotation(vec_t pitch, vec_t yaw, vec_t roll);
	
	void	slerp(const quaternion_c &from, const quaternion_c &to, float t);	
	
	const char*	toString() const;	
	
	inline bool	operator == (const quaternion_c &q) const;
	
	inline bool	operator != (const quaternion_c &q) const;
	
	inline quaternion_c operator + (const quaternion_c &q) const;
	
	quaternion_c	operator * (const quaternion_c &q) const;
	
	inline quaternion_c operator * (vec_t s) const;
	
	inline vec3_c	operator * (const vec3_c &v) const;
	
	inline quaternion_c& operator += (const quaternion_c &q);
	
	
//	inline vec_t	operator [] (const int index) const;

//	inline vec_t&	operator [] (const int index);

	inline operator vec_t * () const;

	inline operator vec_t * ();
	
	/*
	inline operator float * () const;

	inline operator float * ();
	
	inline operator double * () const;

	inline operator double * ();
	*/

	inline quaternion_c&	operator = (const quaternion_c &q);
	
	inline quaternion_c&	operator = (const float *q);
	
	inline quaternion_c&	operator = (const double *q);
	
private:
	vec4_t		_q;
};


inline quaternion_c::quaternion_c()	
{
	_q[0] = _q[1] = _q[2] = 0.0;
	_q[3] = 1.0;
}
	
inline quaternion_c::quaternion_c(vec_t x, vec_t y, vec_t z, vec_t w)
{
	set(x, y, z, w);
}
	
inline quaternion_c::quaternion_c(const float *q)
{
	_q[0] = q[0];
	_q[1] = q[1];
	_q[2] = q[2];
	_q[3] = q[3];
}
	
inline void	quaternion_c::identity()
{
	_q[0] = _q[1] = _q[2] = 0.0;
	_q[3] = 1.0;
}
	
inline void	quaternion_c::set(vec_t x, vec_t y, vec_t z, vec_t w)
{
	_q[0] = x; 
	_q[1] = y;
	_q[2] = z;
	_q[3] = w;
}

inline void	quaternion_c::set(vec_t x, vec_t y, vec_t z)
{
	_q[0] = x; 
	_q[1] = y;
	_q[2] = z;

	vec_t term = x*x + y*y + z*z;
	
	if(term < 0.0f)
		_q[3] = 0.0f;
	else
		_q[3] = -X_sqrt(term);
}
		
inline void	quaternion_c::copyTo(float *q)
{
	q[0] = _q[0];
	q[1] = _q[1];
	q[2] = _q[2];
	q[3] = _q[3];
}
	
inline bool 	quaternion_c::isZero() const
{
	if (_q[0] == 0 && _q[1] == 0 && _q[2] == 0 && _q[3] == 0)
		return true;
	else
		return false;
}
	
inline vec_t	quaternion_c::magnitude()
{
	return X_sqrt(_q[0]*_q[0] + _q[1]*_q[1] + _q[2]*_q[2] + _q[3]*_q[3]);
}

inline void	quaternion_c::inverse()
{
	_q[0] = -_q[0];
	_q[1] = -_q[1];
	_q[2] = -_q[2];
}
	
inline void	quaternion_c::antipodal()
{
	_q[0] = -_q[0];
	_q[1] = -_q[1];
	_q[2] = -_q[2];
	_q[3] = -_q[3];
}
	
inline void	quaternion_c::fromAngles(const vec3_c &angles)
{
	fromAngles(angles[PITCH], angles[YAW], angles[ROLL]);
}

inline void	quaternion_c::fromAxisAngle(const vec3_c &axis, vec_t deg)
{
	fromAxisAngle(axis[0], axis[1], axis[2], deg);
}

inline void	quaternion_c::fromW(const vec3_c &w, const quaternion_c &q)
{
	_q[0] = REAL(0.5) * (  w[0]*q._q[3] + w[1]*q._q[2] - w[2]*q._q[1]);
	_q[1] = REAL(0.5) * (- w[0]*q._q[2] + w[1]*q._q[3] + w[2]*q._q[0]);
	_q[2] = REAL(0.5) * (  w[0]*q._q[1] - w[1]*q._q[0] + w[2]*q._q[3]);
	_q[3] = REAL(0.5) * (- w[0]*q._q[0] - w[1]*q._q[1] - w[2]*q._q[2]);
}

inline void	quaternion_c::multiplyRotation(const vec3_c &axis, vec_t deg)
{
	multiplyRotation(axis[0], axis[1], axis[2], deg);
}
	
inline void	quaternion_c::multiplyRotation(vec_t x, vec_t y, vec_t z, vec_t deg)
{
	quaternion_c rot; rot.fromAxisAngle(x, y, z, deg);

	*this = *this * rot;
}
	
inline void	quaternion_c::multiplyRotation(const vec3_c &angles)
{
	multiplyRotation(angles[PITCH], angles[YAW], angles[ROLL]);
}
	
inline void	quaternion_c::multiplyRotation(vec_t pitch, vec_t yaw, vec_t roll)
{
	quaternion_c rot; rot.fromAngles(pitch, yaw, roll);

	*this = *this * rot;
}

inline bool	quaternion_c::operator == (const quaternion_c &q) const
{
	if (_q[0] != q._q[0] || _q[1] != q._q[1] || _q[2] != q._q[2] || _q[3] != q._q[3])
		return false;
	else
		return true;
}
	
inline bool	quaternion_c::operator != (const quaternion_c &q) const
{
	if (_q[0] != q._q[0] || _q[1] != q._q[1] || _q[2] != q._q[2] || _q[3] != q._q[3])
		return true;
	else
		return false;
}

inline quaternion_c	quaternion_c::operator + (const quaternion_c &q) const
{
	return quaternion_c(_q[0]+q._q[0], _q[1]+q._q[1], _q[2]+q._q[2], _q[3]+q._q[3]);
}

inline quaternion_c	quaternion_c::operator * (vec_t s) const
{
	return quaternion_c(_q[0]*s, _q[1]*s, _q[2]*s, _q[3]*s);
}
	
inline vec3_c	quaternion_c::operator * (const vec3_c &v) const
{
	matrix_c m;
	m.fromQuaternion(*this);
		
	return vec3_c(m * v);
}

inline quaternion_c&	quaternion_c::operator += (const quaternion_c &q)
{
	_q[0] += q._q[0];
	_q[1] += q._q[1];
	_q[2] += q._q[2];
	_q[3] += q._q[3];
		
	return *this;
}

/*
inline vec_t	quaternion_c::operator [] (const int index) const
{
	return _q[index];
}

inline vec_t&	quaternion_c::operator [] (const int index)
{
	return _q[index];
}
*/

inline quaternion_c::operator vec_t * () const
{
	return (vec_t*)_q;
}

inline quaternion_c::operator vec_t * ()
{
	return (vec_t*)_q;
}

/*	
inline quaternion_c::operator float * () const
{
	return (float*)_q;
}

inline quaternion_c::operator float * ()
{
	return (float*)_q;
}

inline quaternion_c::operator double * () const
{
	return (double*)_q;
}

inline quaternion_c::operator double * ()
{
	return (double*)_q;
}
*/
		
inline quaternion_c&	quaternion_c::operator = (const quaternion_c &q)
{
	_q[0] = q._q[0];
	_q[1] = q._q[1];
	_q[2] = q._q[2];
	_q[3] = q._q[3];
		
	return *this;
}
	
inline quaternion_c&	quaternion_c::operator = (const float *q)
{
	_q[0] = q[0];
	_q[1] = q[1];
	_q[2] = q[2];
	_q[3] = q[3];
	
	return *this;
}



inline void	vec3_c::rotate(const vec3_c &angles)
{
	matrix_c m; m.fromAngles(angles);
	rotate(m);
}

inline void	vec3_c::rotate(const matrix_c &m)
{
	*this = m * *this;
}

inline void	vec3_c::rotate(const quaternion_c &q)
{
	*this = q * *this;
}

inline void	vec3_c::transform(const matrix_c &m)
{
	*this = m * *this;
	
	_v[0] += m[0][3];
	_v[1] += m[1][3];
	_v[2] += m[2][3];
}





class cbbox_c
{
public:
	inline cbbox_c();
	
private:
	bool	lineIntersect(vec_t min1, vec_t max1, vec_t min2, vec_t max2) const;
	
	vec_t	maxOfEight(vec_t n1, vec_t n2, vec_t n3, vec_t n4, vec_t n5, vec_t n6, vec_t n7, vec_t n8) const;
	
	vec_t	minOfEight(vec_t n1, vec_t n2, vec_t n3, vec_t n4, vec_t n5, vec_t n6, vec_t n7, vec_t n8) const;
public:	

	inline void	clear();
	
	inline void	zero();
	
	inline vec3_c	origin() const;

	inline vec3_c	size() const;
	
	inline bool	isInside(vec_t x, vec_t y, vec_t z) const;
	
	inline bool	isInside(const vec3_c &v) const;

	// bbox
	bool		intersect(const cbbox_c &bbox) const;

	// sphere
	bool		intersect(const vec3_c &origin, const float radius) const;
	
	// ray
	bool		intersect(vec_t x0, vec_t y0, vec_t z0, vec_t x1, vec_t y1, vec_t z1) const;
	
	inline bool	intersect(const vec3_c &u, const vec3_c &v) const;
	
	// triangle
	inline bool	intersect(const vec3_c &v0, const vec3_c &v1, const vec3_c &v2) const;

	float		radius() const;

	void		addPoint(const vec3_c &v);
	
	void		mergeWith(const cbbox_c &bbox);
	
	void		translate(const vec3_c &v);
	
	void		rotate(const quaternion_c &q);

	bool 		isZero() const;
	
	const char*	toString() const;
	
	inline cbbox_c&	operator = (const cbbox_c &bbox);
	
	inline cbbox_c&	operator = (const vec_t aabb[6]);

	vec3_c		_mins;
	vec3_c		_maxs;
};

inline	cbbox_c::cbbox_c()
{
	zero();
}

inline void	cbbox_c::clear()
{
	_mins[0] = _mins[1] = _mins[2] =  99999;
	_maxs[0] = _maxs[1] = _maxs[2] = -99999;
}
	
inline void	cbbox_c::zero()
{
	_mins[0] = _mins[1] = _mins[2] = 0;
	_maxs[0] = _maxs[1] = _maxs[2] = 0;
}

inline vec3_c	cbbox_c::origin() const
{
	return (_mins + _maxs);
}

inline vec3_c	cbbox_c::size() const
{
	return (_maxs - _mins);
}

inline bool	cbbox_c::isInside(vec_t x, vec_t y, vec_t z) const
{
	if(	x >= _mins[0] && x <= _maxs[0] &&
		y >= _mins[1] && y <= _maxs[1] &&
		z >= _mins[2] && z <= _maxs[2]
	)
		return true;
	else
		return false;
}
	
inline bool	cbbox_c::isInside(const vec3_c &v) const
{
	return isInside(v[0], v[1], v[2]);
}

inline bool	cbbox_c::intersect(const vec3_c &u, const vec3_c &v) const
{
	return intersect(u[0], u[1], u[2], v[0], v[1], v[2]);
}

inline bool	cbbox_c::intersect(const vec3_c &v0, const vec3_c &v1, const vec3_c &v2) const
{
	if(isInside(v0) || isInside(v1) || isInside(v2))
		return true;
	
	return false;
}

inline cbbox_c&	cbbox_c::operator = (const cbbox_c &bbox)
{
	_mins = bbox._mins;
	_maxs = bbox._maxs;
	
	return *this;
}
	
inline cbbox_c&	cbbox_c::operator = (const vec_t aabb[6])
{
	_mins[0] = aabb[0];
	_mins[1] = aabb[1];
	_mins[2] = aabb[2];
		
	_maxs[0] = aabb[3];
	_maxs[1] = aabb[4];
	_maxs[2] = aabb[5];
		
	return *this;
}



class cplane_c
{
public:
	inline cplane_c();
	
	inline cplane_c(const vec3_c &normal, vec_t dist);
	
	inline cplane_c(const vec3_c &v1, const vec3_c &v2, const vec3_c &v3);

	void	fromThreePointForm(const vec3_c &v1, const vec3_c &v2, const vec3_c &v3);
	
	inline void	set(const vec3_c &normal, const vec_t dist);

	inline void	set(vec_t x, vec_t y, vec_t z, vec_t dist);
	
	inline void	negate();

	void	setType();
	
	void	setSignBits();

	void	normalize();
	
//	inline void	translate(vec_t x, vec_t y, vec_t z);

	inline void	translate(const vec3_c &v);

	inline void	rotate(const vec3_c &angles);

	inline void	rotate(const matrix_c &m);
	
	inline void	rotate(const quaternion_c &quat);
	
	inline vec_t	snap();
	
	inline vec_t	distance(const vec3_c &v) const;
	
	inline plane_side_e	onSide(const vec3_c &v) const;
	
	inline plane_side_e	onSide(const vec3_c &center, vec_t radius) const;

	plane_side_e	onSide(const cbbox_c &bbox, bool use_signbits = false) const;
	
	// intersect ray
	inline void	intersect(const vec3_c &v1, const vec3_c &v2, vec3_c &out);
	
	const char*	toString() const;
	
	bool	operator == (const cplane_c &p) const;
	
	inline cplane_c&	operator = (const cplane_c &p);
	
	
	vec3_c		_normal;
	vec_t		_dist;
	
	plane_type_e	_type;		// for fast side tests
	byte		_signbits;	// signx + (signy<<1) + (signz<<1)
};



inline cplane_c::cplane_c()
{
}
	
inline cplane_c::cplane_c(const vec3_c &normal, vec_t dist)
{
	set(normal, dist);
}
	
inline cplane_c::cplane_c(const vec3_c &v1, const vec3_c &v2, const vec3_c &v3)
{
	fromThreePointForm(v1, v2, v3);
}
	
inline void	cplane_c::set(const vec3_c &normal, const vec_t dist)
{
	_normal = normal;
	_dist = dist;
	
	normalize();
	setType();
	setSignBits();
}


inline void	cplane_c::set(vec_t x, vec_t y, vec_t z, vec_t dist)
{
	_normal.set(x, y, z);
	_dist = dist;
	
	normalize();
	setType();
	setSignBits();
}
	
inline void	cplane_c::negate()
{
	_normal.negate();
	_dist = -_dist;
		
	normalize();
	setType();
	setSignBits();
}

/*
inline void	cplane_c::translate(vec_t x, vec_t y, vec_t z)
{
	vec3_c tmp = vec3_c(_normal[0]*_dist+x, _normal[1]*_dist+y, _normal[2]*_dist+z);
	
	_dist = tmp.dotProduct(x, y, z);
}
*/

inline void	cplane_c::translate(const vec3_c &v)
{
//	translate(v[0], v[1], v[2]);
	
	vec3_c tmp = vec3_c(_normal[0]*_dist+v[0], _normal[1]*_dist+v[1], _normal[2]*_dist+v[2]);
	
	_dist = tmp.dotProduct(v);
}

inline void	cplane_c::rotate(const vec3_c &angles)
{
	_normal.rotate(angles);
	setType();
	setSignBits();
}

inline void	cplane_c::rotate(const matrix_c &m)
{
	_normal.rotate(m);
	setType();
	setSignBits();
}
	
inline void	cplane_c::rotate(const quaternion_c &quat)
{
	_normal.rotate(quat);
	setType();
	setSignBits();
}
	
inline vec_t	cplane_c::snap()
{
	_normal.snap();
	setType();
	setSignBits();

	if(fabs(_dist - X_rint(_dist)) < DIST_EPSILON)
		_dist = X_rint(_dist);
			
	return _dist;
}
	
inline vec_t	cplane_c::distance(const vec3_c &v) const
{
	vec_t d;
	
	if(_type <= PLANE_Z)
		d = v[_type] - _dist;
	else
		d = _normal.dotProduct(v) - _dist;
	
	return d;
}
	
inline plane_side_e	cplane_c::onSide(const vec3_c &v) const
{	
	vec_t d = distance(v);
		
	if(d >= 0)
		return SIDE_FRONT;
	else
		return SIDE_BACK;
}
	
inline plane_side_e	cplane_c::onSide(const vec3_c &center, vec_t radius) const
{	
	vec_t d = distance(center);
	
	if(d <= -radius)
		return SIDE_BACK;
	else
		return SIDE_FRONT;
}

inline void	cplane_c::intersect(const vec3_c &v1, const vec3_c &v2, vec3_c &out)
{
	vec3_c v = v1 - v2;
	float sect = -distance(v1) / _normal.dotProduct(v);
	v.scale(sect);
	out = v1 + v;
}

inline cplane_c&	cplane_c::operator = (const cplane_c &p)
{
	_normal		= p._normal;
	_dist		= p._dist;

	_type		= p._type;
	_signbits	= p._signbits;
			
	return *this;
}


void	Curve_GetFlatness(int maxflat, vec4_c *points, int *mesh_cp, int *flat);
void	Curve_EvalQuadricBezierPatch(vec4_c *points, int *mesh_cp, int *tess, vec4_c *out);


void 	RotatePointAroundVector(vec3_c &dst, const vec3_c &dir, const vec3_c &point, vec_t degrees);

void 	Angles_ToVectors(const vec3_c &angles, vec3_c &forward, vec3_c &right, vec3_c &up);
void 	Angles_ToVectorsFLU(const vec3_c &angles, vec3_c &forward, vec3_c &left, vec3_c &up);


float	anglemod(float a);
float 	LerpAngle(float a1, float a2, float frac);

float	CalcFOV(float fov_x, float width, float height);

float	X_frand();	// 0 ti 1

float	X_crand();	// -1 to 1


#endif // X_MATH_H
