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
//
// q_math.c -- stateless support routines that are included in each code module
#include "q_shared.h"

// *INDENT-OFF*
vec3_t  vec3_origin = {0,0,0};
vec3_t  axisDefault[3] = { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } };


vec4_t          colorBlack      = {0, 0, 0, 1};
vec4_t          colorRed        = {1, 0, 0, 1};
vec4_t          colorGreen      = {0, 1, 0, 1};
vec4_t          colorBlue       = {0, 0, 1, 1};
vec4_t          colorYellow     = {1, 1, 0, 1};
vec4_t          colorMagenta= {1, 0, 1, 1};
vec4_t          colorCyan       = {0, 1, 1, 1};
vec4_t          colorWhite      = {1, 1, 1, 1};
vec4_t          colorLtGrey     = {0.75, 0.75, 0.75, 1};
vec4_t          colorMdGrey     = {0.5, 0.5, 0.5, 1};
vec4_t          colorDkGrey     = {0.25, 0.25, 0.25, 1};

vec4_t  g_color_table[8] =
{
	{0.0, 0.0, 0.0, 1.0},
	{1.0, 0.0, 0.0, 1.0},
	{0.0, 1.0, 0.0, 1.0},
	{1.0, 1.0, 0.0, 1.0},
	{0.0, 0.0, 1.0, 1.0},
	{0.0, 1.0, 1.0, 1.0},
	{1.0, 0.0, 1.0, 1.0},
	{1.0, 1.0, 1.0, 1.0},
};


vec3_t  bytedirs[NUMVERTEXNORMALS] =
{
	{-0.525731f, 0.000000f, 0.850651f}, {-0.442863f, 0.238856f, 0.864188f}, 
	{-0.295242f, 0.000000f, 0.955423f}, {-0.309017f, 0.500000f, 0.809017f}, 
	{-0.162460f, 0.262866f, 0.951056f}, {0.000000f, 0.000000f, 1.000000f}, 
	{0.000000f, 0.850651f, 0.525731f}, {-0.147621f, 0.716567f, 0.681718f}, 
	{0.147621f, 0.716567f, 0.681718f}, {0.000000f, 0.525731f, 0.850651f}, 
	{0.309017f, 0.500000f, 0.809017f}, {0.525731f, 0.000000f, 0.850651f}, 
	{0.295242f, 0.000000f, 0.955423f}, {0.442863f, 0.238856f, 0.864188f}, 
	{0.162460f, 0.262866f, 0.951056f}, {-0.681718f, 0.147621f, 0.716567f}, 
	{-0.809017f, 0.309017f, 0.500000f},{-0.587785f, 0.425325f, 0.688191f}, 
	{-0.850651f, 0.525731f, 0.000000f},{-0.864188f, 0.442863f, 0.238856f}, 
	{-0.716567f, 0.681718f, 0.147621f},{-0.688191f, 0.587785f, 0.425325f}, 
	{-0.500000f, 0.809017f, 0.309017f}, {-0.238856f, 0.864188f, 0.442863f}, 
	{-0.425325f, 0.688191f, 0.587785f}, {-0.716567f, 0.681718f, -0.147621f}, 
	{-0.500000f, 0.809017f, -0.309017f}, {-0.525731f, 0.850651f, 0.000000f}, 
	{0.000000f, 0.850651f, -0.525731f}, {-0.238856f, 0.864188f, -0.442863f}, 
	{0.000000f, 0.955423f, -0.295242f}, {-0.262866f, 0.951056f, -0.162460f}, 
	{0.000000f, 1.000000f, 0.000000f}, {0.000000f, 0.955423f, 0.295242f}, 
	{-0.262866f, 0.951056f, 0.162460f}, {0.238856f, 0.864188f, 0.442863f}, 
	{0.262866f, 0.951056f, 0.162460f}, {0.500000f, 0.809017f, 0.309017f}, 
	{0.238856f, 0.864188f, -0.442863f},{0.262866f, 0.951056f, -0.162460f}, 
	{0.500000f, 0.809017f, -0.309017f},{0.850651f, 0.525731f, 0.000000f}, 
	{0.716567f, 0.681718f, 0.147621f}, {0.716567f, 0.681718f, -0.147621f}, 
	{0.525731f, 0.850651f, 0.000000f}, {0.425325f, 0.688191f, 0.587785f}, 
	{0.864188f, 0.442863f, 0.238856f}, {0.688191f, 0.587785f, 0.425325f}, 
	{0.809017f, 0.309017f, 0.500000f}, {0.681718f, 0.147621f, 0.716567f}, 
	{0.587785f, 0.425325f, 0.688191f}, {0.955423f, 0.295242f, 0.000000f}, 
	{1.000000f, 0.000000f, 0.000000f}, {0.951056f, 0.162460f, 0.262866f}, 
	{0.850651f, -0.525731f, 0.000000f},{0.955423f, -0.295242f, 0.000000f}, 
	{0.864188f, -0.442863f, 0.238856f}, {0.951056f, -0.162460f, 0.262866f}, 
	{0.809017f, -0.309017f, 0.500000f}, {0.681718f, -0.147621f, 0.716567f}, 
	{0.850651f, 0.000000f, 0.525731f}, {0.864188f, 0.442863f, -0.238856f}, 
	{0.809017f, 0.309017f, -0.500000f}, {0.951056f, 0.162460f, -0.262866f}, 
	{0.525731f, 0.000000f, -0.850651f}, {0.681718f, 0.147621f, -0.716567f}, 
	{0.681718f, -0.147621f, -0.716567f},{0.850651f, 0.000000f, -0.525731f}, 
	{0.809017f, -0.309017f, -0.500000f}, {0.864188f, -0.442863f, -0.238856f}, 
	{0.951056f, -0.162460f, -0.262866f}, {0.147621f, 0.716567f, -0.681718f}, 
	{0.309017f, 0.500000f, -0.809017f}, {0.425325f, 0.688191f, -0.587785f}, 
	{0.442863f, 0.238856f, -0.864188f}, {0.587785f, 0.425325f, -0.688191f}, 
	{0.688191f, 0.587785f, -0.425325f}, {-0.147621f, 0.716567f, -0.681718f}, 
	{-0.309017f, 0.500000f, -0.809017f}, {0.000000f, 0.525731f, -0.850651f}, 
	{-0.525731f, 0.000000f, -0.850651f}, {-0.442863f, 0.238856f, -0.864188f}, 
	{-0.295242f, 0.000000f, -0.955423f}, {-0.162460f, 0.262866f, -0.951056f}, 
	{0.000000f, 0.000000f, -1.000000f}, {0.295242f, 0.000000f, -0.955423f}, 
	{0.162460f, 0.262866f, -0.951056f}, {-0.442863f, -0.238856f, -0.864188f}, 
	{-0.309017f, -0.500000f, -0.809017f}, {-0.162460f, -0.262866f, -0.951056f}, 
	{0.000000f, -0.850651f, -0.525731f}, {-0.147621f, -0.716567f, -0.681718f}, 
	{0.147621f, -0.716567f, -0.681718f}, {0.000000f, -0.525731f, -0.850651f}, 
	{0.309017f, -0.500000f, -0.809017f}, {0.442863f, -0.238856f, -0.864188f}, 
	{0.162460f, -0.262866f, -0.951056f}, {0.238856f, -0.864188f, -0.442863f}, 
	{0.500000f, -0.809017f, -0.309017f}, {0.425325f, -0.688191f, -0.587785f}, 
	{0.716567f, -0.681718f, -0.147621f}, {0.688191f, -0.587785f, -0.425325f}, 
	{0.587785f, -0.425325f, -0.688191f}, {0.000000f, -0.955423f, -0.295242f}, 
	{0.000000f, -1.000000f, 0.000000f}, {0.262866f, -0.951056f, -0.162460f}, 
	{0.000000f, -0.850651f, 0.525731f}, {0.000000f, -0.955423f, 0.295242f}, 
	{0.238856f, -0.864188f, 0.442863f}, {0.262866f, -0.951056f, 0.162460f}, 
	{0.500000f, -0.809017f, 0.309017f}, {0.716567f, -0.681718f, 0.147621f}, 
	{0.525731f, -0.850651f, 0.000000f}, {-0.238856f, -0.864188f, -0.442863f}, 
	{-0.500000f, -0.809017f, -0.309017f}, {-0.262866f, -0.951056f, -0.162460f}, 
	{-0.850651f, -0.525731f, 0.000000f}, {-0.716567f, -0.681718f, -0.147621f}, 
	{-0.716567f, -0.681718f, 0.147621f}, {-0.525731f, -0.850651f, 0.000000f}, 
	{-0.500000f, -0.809017f, 0.309017f}, {-0.238856f, -0.864188f, 0.442863f}, 
	{-0.262866f, -0.951056f, 0.162460f}, {-0.864188f, -0.442863f, 0.238856f}, 
	{-0.809017f, -0.309017f, 0.500000f}, {-0.688191f, -0.587785f, 0.425325f}, 
	{-0.681718f, -0.147621f, 0.716567f}, {-0.442863f, -0.238856f, 0.864188f}, 
	{-0.587785f, -0.425325f, 0.688191f}, {-0.309017f, -0.500000f, 0.809017f}, 
	{-0.147621f, -0.716567f, 0.681718f}, {-0.425325f, -0.688191f, 0.587785f}, 
	{-0.162460f, -0.262866f, 0.951056f}, {0.442863f, -0.238856f, 0.864188f}, 
	{0.162460f, -0.262866f, 0.951056f}, {0.309017f, -0.500000f, 0.809017f}, 
	{0.147621f, -0.716567f, 0.681718f}, {0.000000f, -0.525731f, 0.850651f}, 
	{0.425325f, -0.688191f, 0.587785f}, {0.587785f, -0.425325f, 0.688191f}, 
	{0.688191f, -0.587785f, 0.425325f}, {-0.955423f, 0.295242f, 0.000000f}, 
	{-0.951056f, 0.162460f, 0.262866f}, {-1.000000f, 0.000000f, 0.000000f}, 
	{-0.850651f, 0.000000f, 0.525731f}, {-0.955423f, -0.295242f, 0.000000f}, 
	{-0.951056f, -0.162460f, 0.262866f}, {-0.864188f, 0.442863f, -0.238856f}, 
	{-0.951056f, 0.162460f, -0.262866f}, {-0.809017f, 0.309017f, -0.500000f}, 
	{-0.864188f, -0.442863f, -0.238856f}, {-0.951056f, -0.162460f, -0.262866f}, 
	{-0.809017f, -0.309017f, -0.500000f}, {-0.681718f, 0.147621f, -0.716567f}, 
	{-0.681718f, -0.147621f, -0.716567f}, {-0.850651f, 0.000000f, -0.525731f}, 
	{-0.688191f, 0.587785f, -0.425325f}, {-0.587785f, 0.425325f, -0.688191f}, 
	{-0.425325f, 0.688191f, -0.587785f}, {-0.425325f, -0.688191f, -0.587785f}, 
	{-0.587785f, -0.425325f, -0.688191f}, {-0.688191f, -0.587785f, -0.425325f}
};
// *INDENT-ON*

//==============================================================
int Q_rand(int *seed)
{
	*seed = (69069 * *seed + 1);
	return *seed;
}

float Q_random(int *seed)
{
	return (Q_rand(seed) & 0xffff) / (float)0x10000;
}

float Q_crandom(int *seed)
{
	return 2.0 * (Q_random(seed) - 0.5);
}

#ifdef __LCC__

int VectorCompare(const vec3_t v1, const vec3_t v2)
{
	if(v1[0] != v2[0] || v1[1] != v2[1] || v1[2] != v2[2])
	{
		return 0;
	}
	return 1;
}

vec_t VectorLength(const vec3_t v)
{
	return (vec_t) sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

vec_t VectorLengthSquared(const vec3_t v)
{
	return (v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

vec_t Distance(const vec3_t p1, const vec3_t p2)
{
	vec3_t          v;

	VectorSubtract(p2, p1, v);
	return VectorLength(v);
}

vec_t DistanceSquared(const vec3_t p1, const vec3_t p2)
{
	vec3_t          v;

	VectorSubtract(p2, p1, v);
	return v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
}


// fast vector normalize routine that does not check to make sure
// that length != 0, nor does it return length, uses rsqrt approximation
void VectorNormalizeFast(vec3_t v)
{
	float           ilength;

	ilength = Q_rsqrt(DotProduct(v, v));

	v[0] *= ilength;
	v[1] *= ilength;
	v[2] *= ilength;
}

void VectorInverse(vec3_t v)
{
	v[0] = -v[0];
	v[1] = -v[1];
	v[2] = -v[2];
}

void CrossProduct(const vec3_t v1, const vec3_t v2, vec3_t cross)
{
	cross[0] = v1[1] * v2[2] - v1[2] * v2[1];
	cross[1] = v1[2] * v2[0] - v1[0] * v2[2];
	cross[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

#endif

//=======================================================
signed char ClampChar(int i)
{
	if(i < -128)
	{
		return -128;
	}
	if(i > 127)
	{
		return 127;
	}
	return i;
}

signed short ClampShort(int i)
{
	if(i < -32768)
	{
		return -32768;
	}
	if(i > 0x7fff)
	{
		return 0x7fff;
	}
	return i;
}


// this isn't a real cheap function to call!
int DirToByte(vec3_t dir)
{
	int             i, best;
	float           d, bestd;

	if(!dir)
	{
		return 0;
	}

	bestd = 0;
	best = 0;
	for(i = 0; i < NUMVERTEXNORMALS; i++)
	{
		d = DotProduct(dir, bytedirs[i]);
		if(d > bestd)
		{
			bestd = d;
			best = i;
		}
	}

	return best;
}

void ByteToDir(int b, vec3_t dir)
{
	if(b < 0 || b >= NUMVERTEXNORMALS)
	{
		VectorCopy(vec3_origin, dir);
		return;
	}
	VectorCopy(bytedirs[b], dir);
}


unsigned ColorBytes3(float r, float g, float b)
{
	unsigned        i;

	((byte *) & i)[0] = r * 255;
	((byte *) & i)[1] = g * 255;
	((byte *) & i)[2] = b * 255;

	return i;
}

unsigned ColorBytes4(float r, float g, float b, float a)
{
	unsigned        i;

	((byte *) & i)[0] = r * 255;
	((byte *) & i)[1] = g * 255;
	((byte *) & i)[2] = b * 255;
	((byte *) & i)[3] = a * 255;

	return i;
}

float NormalizeColor(const vec3_t in, vec3_t out)
{
	float           max;

	max = in[0];
	if(in[1] > max)
	{
		max = in[1];
	}
	if(in[2] > max)
	{
		max = in[2];
	}

	if(!max)
	{
		VectorClear(out);
	}
	else
	{
		out[0] = in[0] / max;
		out[1] = in[1] / max;
		out[2] = in[2] / max;
	}
	return max;
}

void ClampColor(vec4_t color)
{
	int             i;

	for(i = 0; i < 4; i++)
	{
		if(color[i] < 0)
			color[i] = 0;

		if(color[i] > 1)
			color[i] = 1;
	}
}



/*
=====================
PlaneFromPoints

Returns false if the triangle is degenrate.
The normal will point out of the clock for clockwise ordered points
=====================
*/
qboolean PlaneFromPoints(vec4_t plane, const vec3_t a, const vec3_t b, const vec3_t c)
{
	vec3_t          d1, d2;

	VectorSubtract(b, a, d1);
	VectorSubtract(c, a, d2);
	CrossProduct(d2, d1, plane);
	if(VectorNormalize(plane) == 0)
	{
		return qfalse;
	}

	plane[3] = DotProduct(a, plane);
	return qtrue;
}


/*
===============
RotatePointAroundVector

This is not implemented very well...
===============
*/
void RotatePointAroundVector(vec3_t dst, const vec3_t dir, const vec3_t point, float degrees)
{
	float           m[3][3];
	float           im[3][3];
	float           zrot[3][3];
	float           tmpmat[3][3];
	float           rot[3][3];
	int             i;
	vec3_t          vr, vup, vf;
	float           rad;

	vf[0] = dir[0];
	vf[1] = dir[1];
	vf[2] = dir[2];

	PerpendicularVector(vr, dir);
	CrossProduct(vr, vf, vup);

	m[0][0] = vr[0];
	m[1][0] = vr[1];
	m[2][0] = vr[2];

	m[0][1] = vup[0];
	m[1][1] = vup[1];
	m[2][1] = vup[2];

	m[0][2] = vf[0];
	m[1][2] = vf[1];
	m[2][2] = vf[2];

	memcpy(im, m, sizeof(im));

	im[0][1] = m[1][0];
	im[0][2] = m[2][0];
	im[1][0] = m[0][1];
	im[1][2] = m[2][1];
	im[2][0] = m[0][2];
	im[2][1] = m[1][2];

	memset(zrot, 0, sizeof(zrot));
	zrot[0][0] = zrot[1][1] = zrot[2][2] = 1.0F;

	rad = DEG2RAD(degrees);
	zrot[0][0] = cos(rad);
	zrot[0][1] = sin(rad);
	zrot[1][0] = -sin(rad);
	zrot[1][1] = cos(rad);

	AxisMultiply(m, zrot, tmpmat);
	AxisMultiply(tmpmat, im, rot);

	for(i = 0; i < 3; i++)
	{
		dst[i] = rot[i][0] * point[0] + rot[i][1] * point[1] + rot[i][2] * point[2];
	}
}


/*
===============
RotateAroundDirection
===============
*/
void RotateAroundDirection(vec3_t axis[3], float yaw)
{

        // create an arbitrary axis[1] 
	PerpendicularVector(axis[1], axis[0]);

        // rotate it around axis[0] by yaw
	if(yaw)
	{
		vec3_t          temp;

		VectorCopy(axis[1], temp);
		RotatePointAroundVector(axis[1], axis[0], temp, yaw);
	}

        // cross to get axis[2]
	CrossProduct(axis[0], axis[1], axis[2]);
}



void vectoangles(const vec3_t value1, vec3_t angles)
{
	float           forward;
	float           yaw, pitch;

	if(value1[1] == 0 && value1[0] == 0)
	{
		yaw = 0;
		if(value1[2] > 0)
		{
			pitch = 90;
		}
		else
		{
			pitch = 270;
		}
	}
	else
	{
		if(value1[0])
		{
			yaw = (atan2(value1[1], value1[0]) * 180 / M_PI);
		}
		else if(value1[1] > 0)
		{
			yaw = 90;
		}
		else
		{
			yaw = 270;
		}
		if(yaw < 0)
		{
			yaw += 360;
		}

		forward = sqrt(value1[0] * value1[0] + value1[1] * value1[1]);
		pitch = (atan2(value1[2], forward) * 180 / M_PI);
		if(pitch < 0)
		{
			pitch += 360;
		}
	}

	angles[PITCH] = -pitch;
	angles[YAW] = yaw;
	angles[ROLL] = 0;
}


/*
=================
AnglesToAxis
=================
*/
void AnglesToAxis(const vec3_t angles, vec3_t axis[3])
{
	vec3_t          right;

        // angle vectors returns "right" instead of "y axis"
	AngleVectors(angles, axis[0], right, axis[2]);
	VectorSubtract(vec3_origin, right, axis[1]);
}

void AxisClear(vec3_t axis[3])
{
	axis[0][0] = 1;
	axis[0][1] = 0;
	axis[0][2] = 0;
	axis[1][0] = 0;
	axis[1][1] = 1;
	axis[1][2] = 0;
	axis[2][0] = 0;
	axis[2][1] = 0;
	axis[2][2] = 1;
}

void AxisCopy(vec3_t in[3], vec3_t out[3])
{
	VectorCopy(in[0], out[0]);
	VectorCopy(in[1], out[1]);
	VectorCopy(in[2], out[2]);
}

void ProjectPointOnPlane(vec3_t dst, const vec3_t p, const vec3_t normal)
{
	float           d;
	vec3_t          n;
	float           inv_denom;

	inv_denom = DotProduct(normal, normal);
#ifndef Q3_VM
        assert(Q_fabs(inv_denom) != 0.0f);      // bk010122 - zero vectors get here
#endif
        inv_denom = 1.0f / inv_denom;

        d = DotProduct(normal, p) * inv_denom;

		n[0] = normal[0] * inv_denom;
		n[1] = normal[1] * inv_denom;
		n[2] = normal[2] * inv_denom;

		dst[0] = p[0] - d * n[0];
		dst[1] = p[1] - d * n[1];
		dst[2] = p[2] - d * n[2];
}


/*
================
MakeNormalVectors

Given a normalized forward vector, create two
other perpendicular vectors
================
*/
void MakeNormalVectors(const vec3_t forward, vec3_t right, vec3_t up)
{
	float           d;

        // this rotate and negate guarantees a vector
        // not colinear with the original
	right[1] = -forward[0];
	right[2] = forward[1];
	right[0] = forward[2];

	d = DotProduct(right, forward);
	VectorMA(right, -d, forward, right);
	VectorNormalize(right);
	CrossProduct(right, forward, up);
}


void VectorRotate(vec3_t in, vec3_t matrix[3], vec3_t out)
{
	out[0] = DotProduct(in, matrix[0]);
	out[1] = DotProduct(in, matrix[1]);
	out[2] = DotProduct(in, matrix[2]);
}


//============================================================================
#if !idppc
/*
** float q_rsqrt( float number )
*/
float Q_rsqrt( float number )
{
	float y;
#if id386_3dnow && defined __GNUC__
//#error Q_rqsrt
        femms();
        asm volatile
				(                                                                       // lo                                   | hi
				"movd           (%%eax),        %%mm0\n"        // in                                   |       -
        
				"pfrsqrt        %%mm0,          %%mm1\n"        // 1/sqrt(in)                   | 1/sqrt(in)    (approx)
				"movq           %%mm1,          %%mm2\n"        // 1/sqrt(in)                   | 1/sqrt(in)    (approx)
				"pfmul          %%mm1,          %%mm1\n"        // (1/sqrt(in))?                | (1/sqrt(in))?         step 1
				"pfrsqit1       %%mm0,          %%mm1\n"        // intermediate                                                         step 2
				"pfrcpit2       %%mm2,          %%mm1\n"        // 1/sqrt(in) (full 24-bit precision)           step 3
        
				"movd           %%mm1,          (%%edx)\n"
	:
	: "a"( &number ), "d"( &y )
	: "memory"
				);
		femms();
#else
        long i;
        float x2;
		const float threehalfs = 1.5F;

		x2 = number * 0.5F;
		y  = number;
		i  = * ( long * ) &y;                                           // evil floating point bit level hacking
		i  = 0x5f3759df - ( i >> 1 );               // what the fuck?
		y  = * ( float * ) &i;
		y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
//      y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed
#ifndef Q3_VM
#ifdef __linux__
        assert( !isnan(y) ); // bk010122 - FPE?
#endif
#endif
#endif // id386_3dnow
        return y;
}

float Q_fabs( float f ) {
	int tmp = * ( int * ) &f;
	tmp &= 0x7FFFFFFF;
	return * ( float * ) &tmp;
}
#endif

//============================================================
/*
===============
LerpAngle

===============
*/
float LerpAngle(float from, float to, float frac)
{
	float           a;

	if(to - from > 180)
	{
		to -= 360;
	}
	if(to - from < -180)
	{
		to += 360;
	}
	a = from + frac * (to - from);

	return a;
}


/*
=================
AngleSubtract

Always returns a value from -180 to 180
=================
*/
float AngleSubtract(float a1, float a2)
{
	float           a;

	a = a1 - a2;
	while(a > 180)
	{
		a -= 360;
	}
	while(a < -180)
	{
		a += 360;
	}
	return a;
}


void AnglesSubtract(vec3_t v1, vec3_t v2, vec3_t v3)
{
	v3[0] = AngleSubtract(v1[0], v2[0]);
	v3[1] = AngleSubtract(v1[1], v2[1]);
	v3[2] = AngleSubtract(v1[2], v2[2]);
}


float AngleMod(float a)
{
	a = (360.0 / 65536) * ((int)(a * (65536 / 360.0)) & 65535);
	return a;
}


/*
=================
AngleNormalize360

returns angle normalized to the range [0 <= angle < 360]
=================
*/
float AngleNormalize360(float angle)
{
	return (360.0 / 65536) * ((int)(angle * (65536 / 360.0)) & 65535);
}


/*
=================
AngleNormalize180

returns angle normalized to the range [-180 < angle <= 180]
=================
*/
float AngleNormalize180(float angle)
{
	angle = AngleNormalize360(angle);
	if(angle > 180.0)
	{
		angle -= 360.0;
	}
	return angle;
}


/*
=================
AngleDelta

returns the normalized delta from angle1 to angle2
=================
*/
float AngleDelta(float angle1, float angle2)
{
	return AngleNormalize180(angle1 - angle2);
}


//============================================================

/*
=================
SetPlaneSignbits
=================
*/
void SetPlaneSignbits(cplane_t * out)
{
	int             bits, j;

        // for fast box on planeside test
	bits = 0;
	for(j = 0; j < 3; j++)
	{
		if(out->normal[j] < 0)
		{
			bits |= 1 << j;
		}
	}
	out->signbits = bits;
}


/*
==================
BoxOnPlaneSide

Returns 1, 2, or 1 + 2

// this is the slow, general version
int BoxOnPlaneSide2 (vec3_t emins, vec3_t emaxs, struct cplane_s *p)
{
        int             i;
        float   dist1, dist2;
        int             sides;
        vec3_t  corners[2];

        for (i=0 ; i<3 ; i++)
{
                if (p->normal[i] < 0)
{
                        corners[0][i] = emins[i];
                        corners[1][i] = emaxs[i];
}
                else
{
                        corners[1][i] = emins[i];
                        corners[0][i] = emaxs[i];
}
}
        dist1 = DotProduct (p->normal, corners[0]) - p->dist;
        dist2 = DotProduct (p->normal, corners[1]) - p->dist;
        sides = 0;
        if (dist1 >= 0)
                sides = 1;
        if (dist2 < 0)
                sides |= 2;

        return sides;
}
 


==================
*/

#if !( (defined __linux__ || __FreeBSD__) && (defined __i386__) && (!defined C_ONLY)) // rb010123

#if defined __LCC__ || defined C_ONLY || !id386 || defined __VECTORC

int BoxOnPlaneSide (vec3_t emins, vec3_t emaxs, struct cplane_s *p)
{
	float   dist1, dist2;
	int             sides;

// fast axial cases
	if (p->type < 3)
	{
		if (p->dist <= emins[p->type])
			return 1;
		if (p->dist >= emaxs[p->type])
			return 2;
		return 3;
	}

// general case
	switch (p->signbits)
	{
		case 0:
			dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
			dist2 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
			break;
		case 1:
			dist1 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
			dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
			break;
		case 2:
			dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
			dist2 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
			break;
		case 3:
			dist1 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
			dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
			break;
		case 4:
			dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
			dist2 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
			break;
		case 5:
			dist1 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
			dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
			break;
		case 6:
			dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
			dist2 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
			break;
		case 7:
			dist1 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
			dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
			break;
		default:
			dist1 = dist2 = 0;              // shut up compiler
			break;
	}

	sides = 0;
	if (dist1 >= p->dist)
		sides = 1;
	if (dist2 < p->dist)
		sides |= 2;

	return sides;
}
#else
#pragma warning( disable: 4035 )


__declspec( naked ) int BoxOnPlaneSide (vec3_t emins, vec3_t emaxs, struct cplane_s *p)
{
	static int bops_initialized;
	static int Ljmptab[8];

	__asm {

		push ebx
                        
				cmp bops_initialized, 1
				je  initialized
						mov bops_initialized, 1
                
						mov Ljmptab[0*4], offset Lcase0
								mov Ljmptab[1*4], offset Lcase1
										mov Ljmptab[2*4], offset Lcase2
												mov Ljmptab[3*4], offset Lcase3
														mov Ljmptab[4*4], offset Lcase4
																mov Ljmptab[5*4], offset Lcase5
																		mov Ljmptab[6*4], offset Lcase6
																				mov Ljmptab[7*4], offset Lcase7
                        
initialized:

		mov edx,dword ptr[4+12+esp]
		mov ecx,dword ptr[4+4+esp]
				xor eax,eax
						mov ebx,dword ptr[4+8+esp]
								mov al,byte ptr[17+edx]
										cmp al,8
												jge Lerror
														fld dword ptr[0+edx]
														fld st(0)
														jmp dword ptr[Ljmptab+eax*4]
Lcase0:
														fmul dword ptr[ebx]
														fld dword ptr[0+4+edx]
														fxch st(2)
														fmul dword ptr[ecx]
														fxch st(2)
														fld st(0)
														fmul dword ptr[4+ebx]
														fld dword ptr[0+8+edx]
														fxch st(2)
														fmul dword ptr[4+ecx]
														fxch st(2)
														fld st(0)
														fmul dword ptr[8+ebx]
														fxch st(5)
														faddp st(3),st(0)
														fmul dword ptr[8+ecx]
																fxch st(1)
																faddp st(3),st(0)
																fxch st(3)
																		faddp st(2),st(0)
																		jmp LSetSides
Lcase1:
																				fmul dword ptr[ecx]
																				fld dword ptr[0+4+edx]
																				fxch st(2)
																				fmul dword ptr[ebx]
																				fxch st(2)
																				fld st(0)
																				fmul dword ptr[4+ebx]
																				fld dword ptr[0+8+edx]
																				fxch st(2)
																				fmul dword ptr[4+ecx]
																				fxch st(2)
																				fld st(0)
																				fmul dword ptr[8+ebx]
																				fxch st(5)
																				faddp st(3),st(0)
																				fmul dword ptr[8+ecx]
																				fxch st(1)
																				faddp st(3),st(0)
																				fxch st(3)
																				faddp st(2),st(0)
																				jmp LSetSides
Lcase2:
																				fmul dword ptr[ebx]
																				fld dword ptr[0+4+edx]
																				fxch st(2)
																				fmul dword ptr[ecx]
																				fxch st(2)
																				fld st(0)
																				fmul dword ptr[4+ecx]
																				fld dword ptr[0+8+edx]
																				fxch st(2)
																				fmul dword ptr[4+ebx]
																				fxch st(2)
																				fld st(0)
																				fmul dword ptr[8+ebx]
																				fxch st(5)
																				faddp st(3),st(0)
																				fmul dword ptr[8+ecx]
																				fxch st(1)
																				faddp st(3),st(0)
																				fxch st(3)
																				faddp st(2),st(0)
																				jmp LSetSides
Lcase3:
																				fmul dword ptr[ecx]
																				fld dword ptr[0+4+edx]
																				fxch st(2)
																				fmul dword ptr[ebx]
																				fxch st(2)
																				fld st(0)
																				fmul dword ptr[4+ecx]
																				fld dword ptr[0+8+edx]
																				fxch st(2)
																				fmul dword ptr[4+ebx]
																				fxch st(2)
																				fld st(0)
																				fmul dword ptr[8+ebx]
																				fxch st(5)
																				faddp st(3),st(0)
																				fmul dword ptr[8+ecx]
																				fxch st(1)
																				faddp st(3),st(0)
																				fxch st(3)
																				faddp st(2),st(0)
																				jmp LSetSides
Lcase4:
																				fmul dword ptr[ebx]
																				fld dword ptr[0+4+edx]
																				fxch st(2)
																				fmul dword ptr[ecx]
																				fxch st(2)
																				fld st(0)
																				fmul dword ptr[4+ebx]
																				fld dword ptr[0+8+edx]
																				fxch st(2)
																				fmul dword ptr[4+ecx]
																				fxch st(2)
																				fld st(0)
																				fmul dword ptr[8+ecx]
																				fxch st(5)
																				faddp st(3),st(0)
																				fmul dword ptr[8+ebx]
																				fxch st(1)
																				faddp st(3),st(0)
																				fxch st(3)
																				faddp st(2),st(0)
																				jmp LSetSides
Lcase5:
																				fmul dword ptr[ecx]
																				fld dword ptr[0+4+edx]
																				fxch st(2)
																				fmul dword ptr[ebx]
																				fxch st(2)
																				fld st(0)
																				fmul dword ptr[4+ebx]
																				fld dword ptr[0+8+edx]
																				fxch st(2)
																				fmul dword ptr[4+ecx]
																				fxch st(2)
																				fld st(0)
																				fmul dword ptr[8+ecx]
																				fxch st(5)
																				faddp st(3),st(0)
																				fmul dword ptr[8+ebx]
																				fxch st(1)
																				faddp st(3),st(0)
																				fxch st(3)
																				faddp st(2),st(0)
																				jmp LSetSides
Lcase6:
																				fmul dword ptr[ebx]
																				fld dword ptr[0+4+edx]
																				fxch st(2)
																				fmul dword ptr[ecx]
																				fxch st(2)
																				fld st(0)
																				fmul dword ptr[4+ecx]
																				fld dword ptr[0+8+edx]
																				fxch st(2)
																				fmul dword ptr[4+ebx]
																				fxch st(2)
																				fld st(0)
																				fmul dword ptr[8+ecx]
																				fxch st(5)
																				faddp st(3),st(0)
																				fmul dword ptr[8+ebx]
																				fxch st(1)
																				faddp st(3),st(0)
																				fxch st(3)
																				faddp st(2),st(0)
																				jmp LSetSides
Lcase7:
																				fmul dword ptr[ecx]
																				fld dword ptr[0+4+edx]
																				fxch st(2)
																				fmul dword ptr[ebx]
																				fxch st(2)
																				fld st(0)
																				fmul dword ptr[4+ecx]
																				fld dword ptr[0+8+edx]
																				fxch st(2)
																				fmul dword ptr[4+ebx]
																				fxch st(2)
																				fld st(0)
																				fmul dword ptr[8+ecx]
																				fxch st(5)
																				faddp st(3),st(0)
																				fmul dword ptr[8+ebx]
																				fxch st(1)
																				faddp st(3),st(0)
																				fxch st(3)
																				faddp st(2),st(0)
LSetSides:
		faddp st(2),st(0)
		fcomp dword ptr[12+edx]
				xor ecx,ecx
				fnstsw ax
						fcomp dword ptr[12+edx]
						and ah,1
						xor ah,1
								add cl,ah
										fnstsw ax
												and ah,1
												add ah,ah
														add cl,ah
																pop ebx
																		mov eax,ecx
																		ret
Lerror:
																				int 3
	}
}
#pragma warning( default: 4035 )

#endif
#endif

/*
=================
RadiusFromBounds
=================
*/
float RadiusFromBounds(const vec3_t mins, const vec3_t maxs)
{
	int             i;
	vec3_t          corner;
	float           a, b;

	for(i = 0; i < 3; i++)
	{
		a = fabs(mins[i]);
		b = fabs(maxs[i]);
		corner[i] = a > b ? a : b;
	}

	return VectorLength(corner);
}


void ClearBounds(vec3_t mins, vec3_t maxs)
{
	mins[0] = mins[1] = mins[2] = 99999;
	maxs[0] = maxs[1] = maxs[2] = -99999;
}

void AddPointToBounds(const vec3_t v, vec3_t mins, vec3_t maxs)
{
	if(v[0] < mins[0])
	{
		mins[0] = v[0];
	}
	if(v[0] > maxs[0])
	{
		maxs[0] = v[0];
	}

	if(v[1] < mins[1])
	{
		mins[1] = v[1];
	}
	if(v[1] > maxs[1])
	{
		maxs[1] = v[1];
	}

	if(v[2] < mins[2])
	{
		mins[2] = v[2];
	}
	if(v[2] > maxs[2])
	{
		maxs[2] = v[2];
	}
}



vec_t VectorNormalize(vec3_t v)
{
#if id386_3dnow && defined __GNUC__
//#error VectorNormalize
        vec_t   length;
        femms();
		asm volatile
				(                                                                       // lo                                                                   | hi
				"movq           (%%eax),        %%mm0\n"        // v[0]                                                                 | v[1]
				"movd           8(%%eax),       %%mm1\n"        // v[2]                                                                 | -
        // mm0[lo] = dot product(this)
				"pfmul          %%mm0,          %%mm0\n"        // v[0]*v[0]                                                    | v[1]*v[1]
				"pfmul          %%mm1,          %%mm1\n"        // v[2]*v[2]                                                    | -
				"pfacc          %%mm0,          %%mm0\n"        // v[0]*v[0]+v[1]*v[1]                                  | -
				"pfadd          %%mm1,          %%mm0\n"        // v[0]*v[0]+v[1]*v[1]+v[2]*v[2]                | -
        // mm0[lo] = sqrt(mm0[lo])
				"pfrsqrt        %%mm0,          %%mm1\n"        // 1/sqrt(dot)                                                  | 1/sqrt(dot)           (approx)
				"movq           %%mm1,          %%mm2\n"        // 1/sqrt(dot)                                                  | 1/sqrt(dot)           (approx)
				"pfmul          %%mm1,          %%mm1\n"        // (1/sqrt(dot))?                                               | (1/sqrt(dot))?        step 1
				"punpckldq      %%mm0,          %%mm0\n"        // dot                                                                  | dot                   (MMX instruction)
				"pfrsqit1       %%mm0,          %%mm1\n"        // intermediate                                                 | intermediate          step 2
				"pfrcpit2       %%mm2,          %%mm1\n"        // 1/sqrt(dot) (full 24-bit precision)  | 1/sqrt(dot)           step 3
				"pfmul          %%mm1,          %%mm0\n"        // sqrt(dot)                                                    | sqrt(dot)
        // len = mm0[lo]
				"movd           %%mm0,          (%%edx)\n"
        // load this into registers
				"movq           (%%eax),        %%mm2\n"        // v[0]                                                                 | v[1]
				"movd           8(%%eax),       %%mm3\n"        // v[2]                                                                 | -
        // scale this by the reciprocal square root
				"pfmul          %%mm1,          %%mm2\n"        // v[0]*1/sqrt(dot)                                             | v[1]*1/sqrt(dot)
				"pfmul          %%mm1,          %%mm3\n"        // v[2]*1/sqrt(dot)                                             | -
        // store scaled vector
				"movq           %%mm2,          (%%eax)\n"
				"movd           %%mm3,          8(%%eax)\n"
	:
	: "a"(v), "d"(&length)
	: "memory"
				);
		femms();
		return length;
#else
        // NOTE: TTimo - Apple G4 altivec source uses double?
        float   length, ilength;

        length = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
		length = sqrt(length);

		if(length)
		{
			ilength = 1/length;
			v[0] *= ilength;
			v[1] *= ilength;
			v[2] *= ilength;
		}
                
		return length;
#endif
}

vec_t VectorNormalize2(const vec3_t v, vec3_t out)
{
	float           length, ilength;

	length = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	length = sqrt(length);

	if(length)
	{
#ifndef Q3_VM                                   // bk0101022 - FPE related
//    assert( ((Q_fabs(v[0])!=0.0f) || (Q_fabs(v[1])!=0.0f) || (Q_fabs(v[2])!=0.0f)) );
#endif
                ilength = 1 / length;
                out[0] = v[0] * ilength;
				out[1] = v[1] * ilength;
				out[2] = v[2] * ilength;
	}
	else
	{
#ifndef Q3_VM                                   // bk0101022 - FPE related
//    assert( ((Q_fabs(v[0])==0.0f) && (Q_fabs(v[1])==0.0f) && (Q_fabs(v[2])==0.0f)) );
#endif
                VectorClear(out);
	}

	return length;

}

void _VectorMA(const vec3_t veca, float scale, const vec3_t vecb, vec3_t vecc)
{
	vecc[0] = veca[0] + scale * vecb[0];
	vecc[1] = veca[1] + scale * vecb[1];
	vecc[2] = veca[2] + scale * vecb[2];
}



vec_t _DotProduct(const vec3_t a, const vec3_t b)
{
#if id386_3dnow && defined __GNUC__ && 0
//#error _DotProduct
        vec_t out;
        femms();
		asm volatile
				(                                                                       // lo                                                           | hi
				"movq           (%%eax),        %%mm0\n"        // a[0]                                                         | a[1]
				"movq           (%%edx),        %%mm2\n"        // b[0]                                                         | b[1]
				"movd           8(%%eax),       %%mm1\n"        // a[2]                                                         | -
				"movd           8(%%edx),       %%mm3\n"        // b[2]                                                         | -
                
				"pfmul          %%mm2,          %%mm0\n"        // a[0]*b[0]                                            | a[1]*b[1]
				"pfmul          %%mm3,          %%mm1\n"        // a[2]*b[2]                                            | -
				"pfacc          %%mm0,          %%mm0\n"        // a[0]*b[0]+a[1]*b[1]                          | -
				"pfadd          %%mm1,          %%mm0\n"        // a[0]*b[0]+a[1]*b[1]+a[2]*b[2]        | -
        
				"movd           %%mm0,          (%%ecx)\n"      // out = mm2[lo]
	:
	: "a"(a), "d"(b), "c"(&out)
	: "memory"
				);
		femms();
		return out;
#else
        return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
#endif
}

void _VectorSubtract(const vec3_t a, const vec3_t b, vec3_t out)
{
#if id386_3dnow && defined __GNUC__ && 0
        femms();
        asm volatile
				(                                                                       // lo                                                           | hi
				"movq           (%%eax),        %%mm0\n"        // a[0]                                                         | a[1]
				"movq           (%%edx),        %%mm2\n"        // b[0]                                                         | b[1]
				"movd           8(%%eax),       %%mm1\n"        // a[2]                                                         | -
				"movd           8(%%edx),       %%mm3\n"        // b[2]                                                         | -
                
				"pfsub          %%mm2,          %%mm0\n"        // a[0]-b[0]                                            | a[1]-b[1]
				"pfsub          %%mm3,          %%mm1\n"        // a[2]-b[2]                                            | -
        
				"movq           %%mm0,          (%%ecx)\n"
				"movd           %%mm1,          8(%%ecx)\n"
	:
	: "a"(a), "d"(b), "c"(out)
	: "memory"
				);
		femms();
#else
        out[0] = a[0]-b[0];
        out[1] = a[1]-b[1];
		out[2] = a[2]-b[2];
#endif
}

void _VectorAdd(const vec3_t a, const vec3_t b, vec3_t out)
{
#if id386_3dnow && defined __GNUC__ && 0
        femms();
        asm volatile
				(                                                                       // lo                                                           | hi
				"movq           (%%eax),        %%mm0\n"        // a[0]                                                         | a[1]
				"movq           (%%edx),        %%mm2\n"        // b[0]                                                         | b[1]
				"movd           8(%%eax),       %%mm1\n"        // a[2]                                                         | -
				"movd           8(%%edx),       %%mm3\n"        // b[2]                                                         | -
                
				"pfadd          %%mm2,          %%mm0\n"        // a[0]+b[0]                                            | a[1]+b[1]
				"pfadd          %%mm3,          %%mm1\n"        // a[2]+b[2]                                            | -
        
				"movq           %%mm0,          (%%ecx)\n"
				"movd           %%mm1,          8(%%ecx)\n"
	:
	: "a"(a), "d"(b), "c"(out)
	: "memory"
				);
		femms();
#else
        out[0] = a[0]+b[0];
        out[1] = a[1]+b[1];
		out[2] = a[2]+b[2];
#endif
}

void _VectorCopy(const vec3_t in, vec3_t out)
{
#if id386_3dnow && defined __GNUC__ && 0
        femms();
        asm volatile
				(                                                                       // lo                                                           | hi
				"movq           (%%eax),        %%mm0\n"        // in[0]                                                        | in[1]
				"movd           8(%%eax),       %%mm1\n"        // in[2]                                                        | -
        
				"movq           %%mm0,          (%%edx)\n"
				"movd           %%mm1,          8(%%edx)\n"
	:
	: "a"(in), "d"(out)
	: "memory"
				);
		femms();
/*
#elif id386_sse && defined __GNUC__
//#error _VectorCopysse
		asm volatile
		(
		"movups         (%%eax),        %%xmm0\n"
		"movups         %%xmm0,         (%%edx)\n"
	:
	: "a"( in ), "d"( out )
	: "memory"
        );
*/
#else
        out[0] = in[0];
        out[1] = in[1];
		out[2] = in[2];
#endif
}

void _VectorScale(const vec3_t in, vec_t scale, vec3_t out)
{
#if id386_3dnow && defined __GNUC__ && 0
        vec_t out;
        femms();
		asm volatile
				(                                                                       // lo                                                                   | hi
				"movq           (%%eax),        %%mm0\n"        // in[0]                                                                | in[1]
				"movd           8(%%eax),       %%mm1\n"        // in[2]                                                                | -
				"movd           (%%edx),        %%mm2\n"        // scale                                                                | -
        
				"punpckhdq      %%mm2,          %%mm2\n"        // scale                                                                | scale
        
				"pfmul          %%mm2,          %%mm0\n"        // in[0]*scale                                                  | in[1]*scale
				"pfmul          %%mm2,          %%mm1\n"        // in[2]*scale                                                  | -
        
				"movq           %%mm0,          (%%ecx)\n"
				"movd           %%mm1,          8(%%ecx)\n"
	:
	: "a"(in), "d"(&scale), "c"(out)
	: "memory"
				);
		femms();
		return out;
#else
        out[0] = in[0]*scale;
        out[1] = in[1]*scale;
		out[2] = in[2]*scale;
#endif
}

void Vector4Scale(const vec4_t in, vec_t scale, vec4_t out)
{
	out[0] = in[0] * scale;
	out[1] = in[1] * scale;
	out[2] = in[2] * scale;
	out[3] = in[3] * scale;
}


int Q_log2(int val)
{
	int             answer;

	answer = 0;
	while((val >>= 1) != 0)
	{
		answer++;
	}
	return answer;
}



/*
=================
PlaneTypeForNormal
=================
*/
/*
int     PlaneTypeForNormal (vec3_t normal) {
        if ( normal[0] == 1.0 )
                return PLANE_X;
        if ( normal[1] == 1.0 )
                return PLANE_Y;
        if ( normal[2] == 1.0 )
                return PLANE_Z;
        
        return PLANE_NON_AXIAL;
}
*/



void AxisMultiply(float in1[3][3], float in2[3][3], float out[3][3])
{
	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] + in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] + in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] + in1[0][2] * in2[2][2];
	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] + in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] + in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] + in1[1][2] * in2[2][2];
	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] + in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] + in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] + in1[2][2] * in2[2][2];
}


void AngleVectors(const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up)
{
	float           angle;
	static float    sr, sp, sy, cr, cp, cy;

        // static to help MS compiler fp bugs
	angle = angles[YAW] * (M_PI * 2 / 360);
	sy = sin(angle);
	cy = cos(angle);
	angle = angles[PITCH] * (M_PI * 2 / 360);
	sp = sin(angle);
	cp = cos(angle);
	angle = angles[ROLL] * (M_PI * 2 / 360);
	sr = sin(angle);
	cr = cos(angle);

	if(forward)
	{
		forward[0] = cp * cy;
		forward[1] = cp * sy;
		forward[2] = -sp;
	}
	if(right)
	{
		right[0] = (-1 * sr * sp * cy + -1 * cr * -sy);
		right[1] = (-1 * sr * sp * sy + -1 * cr * cy);
		right[2] = -1 * sr * cp;
	}
	if(up)
	{
		up[0] = (cr * sp * cy + -sr * -sy);
		up[1] = (cr * sp * sy + -sr * cy);
		up[2] = cr * cp;
	}
}


/*
** assumes "src" is normalized
*/
void PerpendicularVector(vec3_t dst, const vec3_t src)
{
	int             pos;
	int             i;
	float           minelem = 1.0F;
	vec3_t          tempvec;

        /*
	** find the smallest magnitude axially aligned vector
		*/
	for(pos = 0, i = 0; i < 3; i++)
	{
		if(fabs(src[i]) < minelem)
		{
			pos = i;
			minelem = fabs(src[i]);
		}
	}
	tempvec[0] = tempvec[1] = tempvec[2] = 0.0F;
	tempvec[pos] = 1.0F;

        /*
	** project the point onto the plane defined by src
		*/
	ProjectPointOnPlane(dst, tempvec, src);

        /*
	** normalize the result
		*/
	VectorNormalize(dst);
}




void MatrixIdentity(matrix_t m)
{
        /*
	m[0]    = 1;    m[1] = 0;               m[2] = 0;               m[3] = 0;
	m[4]    = 0;    m[1][1] = 1;    m[1][2] = 0;    m[1][3] = 0;
	m[2][0] = 0;    m[2][1] = 0;    m[2][2] = 1;    m[2][3] = 0;
	m[3][0] = 0;    m[3][1] = 0;    m[3][2] = 0;    m[3][3] = 1;
		*/
	m[ 0] = 1;      m[ 4] = 0;      m[ 8] = 0;      m[12] = 0;
	m[ 1] = 0;      m[ 5] = 1;      m[ 9] = 0;      m[13] = 0;
	m[ 2] = 0;      m[ 6] = 0;      m[10] = 1;      m[14] = 0;
	m[ 3] = 0;      m[ 7] = 0;      m[11] = 0;      m[15] = 1;
}

void MatrixClear(matrix_t m)
{
        /*
	m[0][0] = 0;    m[0][1] = 0;    m[0][2] = 0;    m[0][3] = 0;
	m[1][0] = 0;    m[1][1] = 0;    m[1][2] = 0;    m[1][3] = 0;
	m[2][0] = 0;    m[2][1] = 0;    m[2][2] = 0;    m[2][3] = 0;
	m[3][0] = 0;    m[3][1] = 0;    m[3][2] = 0;    m[3][3] = 0;
		*/
	m[ 0] = 0;      m[ 4] = 0;      m[ 8] = 0;      m[12] = 0;
	m[ 1] = 0;      m[ 5] = 0;      m[ 9] = 0;      m[13] = 0;
	m[ 2] = 0;      m[ 6] = 0;      m[10] = 0;      m[14] = 0;
	m[ 3] = 0;      m[ 7] = 0;      m[11] = 0;      m[15] = 0;
}


void MatrixCopy(const matrix_t in, matrix_t out)
{
#if id386_sse && defined __GNUC__
        asm volatile
        (
        "movups         (%%edx),                %%xmm0\n"
        "movups         0x10(%%edx),    %%xmm1\n"
        "movups         0x20(%%edx),    %%xmm2\n"
        "movups         0x30(%%edx),    %%xmm3\n"
        
        "movups         %%xmm0,                 (%%eax)\n"
        "movups         %%xmm1,                 0x10(%%eax)\n"
        "movups         %%xmm2,                 0x20(%%eax)\n"
        "movups         %%xmm3,                 0x30(%%eax)\n"
	:
	: "a"( out ), "d"( in )
	: "memory"
		);
#elif id386_3dnow && defined __GNUC__
        femms();
        asm volatile
				(
				"movq           (%%edx),        %%mm0\n"
				"movq           8(%%edx),       %%mm1\n"
				"movq           16(%%edx),      %%mm2\n"
				"movq           24(%%edx),      %%mm3\n"
				"movq           32(%%edx),      %%mm4\n"
				"movq           40(%%edx),      %%mm5\n"
				"movq           48(%%edx),      %%mm6\n"
				"movq           56(%%edx),      %%mm7\n"
        
				"movq           %%mm0,          (%%eax)\n"
				"movq           %%mm1,          8(%%eax)\n"
				"movq           %%mm2,          16(%%eax)\n"
				"movq           %%mm3,          24(%%eax)\n"
				"movq           %%mm4,          32(%%eax)\n"
				"movq           %%mm5,          40(%%eax)\n"
				"movq           %%mm6,          48(%%eax)\n"
				"movq           %%mm7,          56(%%eax)\n"
	:
	: "a"( out ), "d"( in )
	: "memory"
				);
		femms();
#else
        /*
        out[0][0] = in[0][0];   out[0][1] = in[0][1];   out[0][2] = in[0][2];   out[0][3] = in[0][3];
        out[1][0] = in[1][0];   out[1][1] = in[1][1];   out[1][2] = in[1][2];   out[1][3] = in[1][3];
        out[2][0] = in[2][0];   out[2][1] = in[2][1];   out[2][2] = in[2][2];   out[2][3] = in[2][3];
        out[3][0] = in[3][0];   out[3][1] = in[3][1];   out[3][2] = in[3][2];   out[3][3] = in[3][3];
		*/
        out[ 0] = in[ 0];       out[ 4] = in[ 4];       out[ 8] = in[ 8];       out[12] = in[12];
        out[ 1] = in[ 1];       out[ 5] = in[ 5];       out[ 9] = in[ 9];       out[13] = in[13];
		out[ 2] = in[ 2];       out[ 6] = in[ 6];       out[10] = in[10];       out[14] = in[14];
		out[ 3] = in[ 3];       out[ 7] = in[ 7];       out[11] = in[11];       out[15] = in[15];
#endif
}

void MatrixTransposeIntoXMM(const matrix_t m)
{
#if id386_sse && defined __GNUC__
        asm volatile
        (                                                                               // reg[0]                       | reg[1]                | reg[2]                | reg[3]
        // load transpose into XMM registers
        "movlps         (%%eax),        %%xmm4\n"               // m[0][0]                      | m[0][1]               | -                     | -
        "movhps         16(%%eax),      %%xmm4\n"               // m[0][0]                      | m[0][1]               | m[1][0]               | m[1][1]
        
        "movlps         32(%%eax),      %%xmm3\n"               // m[2][0]                      | m[2][1]               | -                     | -
        "movhps         48(%%eax),      %%xmm3\n"               // m[2][0]                      | m[2][1]               | m[3][0]               | m[3][1]
        
        "movups         %%xmm4,         %%xmm5\n"               // m[0][0]                      | m[0][1]               | m[1][0]               | m[1][1]
        
        // 0x88 = 10 00 | 10 00 <-> 00 10 | 00 10          xmm4[00]                       xmm4[10]                xmm3[00]                xmm3[10]
        "shufps         $0x88, %%xmm3,  %%xmm4\n"       // m[0][0]                      | m[1][0]               | m[2][0]               | m[3][0]
        
        // 0xDD = 11 01 | 11 01 <-> 01 11 | 01 11          xmm5[01]                       xmm5[11]                xmm3[01]                xmm3[11]
        "shufps         $0xDD, %%xmm3,  %%xmm5\n"       // m[0][1]                      | m[1][1]               | m[2][1]               | m[3][1]
        
        "movlps         8(%%eax),       %%xmm6\n"               // m[0][2]                      | m[0][3]               | -                     | -
        "movhps         24(%%eax),      %%xmm6\n"               // m[0][2]                      | m[0][3]               | m[1][2]               | m[1][3]
        
        "movlps         40(%%eax),      %%xmm3\n"               // m[2][2]                      | m[2][3]               | -                     | -
        "movhps         56(%%eax),      %%xmm3\n"               // m[2][2]                      | m[2][3]               | m[3][2]               | m[3][3]
        
        "movups         %%xmm6,         %%xmm7\n"               // m[0][2]                      | m[0][3]               | m[1][2]               | m[1][3]
        
        // 0x88 = 10 00 | 10 00 <-> 00 10 | 00 10          xmm6[00]                       xmm6[10]                xmm3[00]                xmm3[10]
        "shufps         $0x88, %%xmm3,  %%xmm6\n"       // m[0][2]                      | m[1][2]               | m[2][2]               | m[3][2]
        
        // 0xDD = 11 01 | 11 01 <-> 01 11 | 01 11          xmm7[01]                       xmm7[11]                xmm3[01]                xmm3[11]
        "shufps         $0xDD, %%xmm3,  %%xmm7\n"       // m[0][3]                      | m[1][3]               | m[2][3]               | m[3][3]
	:
	: "a"( m )
	: "memory"
		);
#endif
}

void MatrixTranspose(const matrix_t in, matrix_t out)
{
#if id386_sse && defined __GNUC__
        // transpose the matrix into the xmm4-7
        MatrixTransposeIntoXMM(in);
        
        asm volatile
				(
				"movups         %%xmm4,         (%%eax)\n"
				"movups         %%xmm5,         0x10(%%eax)\n"
				"movups         %%xmm6,         0x20(%%eax)\n"
				"movups         %%xmm7,         0x30(%%eax)\n"
	:
	: "a"( out )
	: "memory"
				);
#else
        /*
        out[0][0] = in[0][0];   out[0][1] = in[1][0];   out[0][2] = in[2][0];   out[0][3] = in[3][0];
        out[1][0] = in[0][1];   out[1][1] = in[1][1];   out[1][2] = in[2][1];   out[1][3] = in[3][1];
        out[2][0] = in[0][2];   out[2][1] = in[1][2];   out[2][2] = in[2][2];   out[2][3] = in[3][2];
        out[3][0] = in[0][3];   out[3][1] = in[1][3];   out[3][2] = in[2][3];   out[3][3] = in[3][3];
		*/
        out[ 0] = in[ 0];       out[ 1] = in[ 4];       out[ 2] = in[ 8];       out[ 3] = in[12];
        out[ 4] = in[ 1];       out[ 5] = in[ 5];       out[ 6] = in[ 9];       out[ 7] = in[13];
		out[ 8] = in[ 2];       out[ 9] = in[ 6];       out[10] = in[10];       out[11] = in[14];
		out[12] = in[ 3];       out[13] = in[ 7];       out[14] = in[11];       out[15] = in[15];
#endif
}

void MatrixSetupXRotation(matrix_t m, vec_t degrees)
{
	vec_t a = DEG2RAD( degrees );
        /*
	m[0][0] = 1;    m[0][1] =0;                     m[0][2] = 0;            m[0][3] = 0;
	m[1][0] = 0;    m[1][1] = cos(a);       m[1][2] =-sin(a);       m[1][3] = 0;
	m[2][0] = 0;    m[2][1] = sin(a);       m[2][2] = cos(a);       m[2][3] = 0;
	m[3][0] = 0;    m[3][1] =0;                     m[3][2] = 0;            m[3][3] = 1;
		*/
	m[ 0] = 1;      m[ 4] = 0;              m[ 8] = 0;              m[12] = 0;
	m[ 1] = 0;      m[ 5] = cos(a); m[ 9] =-sin(a); m[13] = 0;
	m[ 2] = 0;      m[ 6] = sin(a); m[10] = cos(a); m[14] = 0;
	m[ 3] = 0;      m[ 7] = 0;              m[11] = 0;              m[15] = 1;
}

void MatrixSetupYRotation(matrix_t m, vec_t degrees)
{
	vec_t a = DEG2RAD( degrees );
        /*
	m[0][0] = cos(a);       m[0][1] = 0;            m[0][2] = sin(a);       m[0][3] = 0;
	m[1][0] = 0;            m[1][1] = 1;            m[1][2] = 0;            m[1][3] = 0;
	m[2][0] =-sin(a);       m[2][1] = 0;            m[2][2] = cos(a);       m[2][3] = 0;
	m[3][0] = 0;            m[3][1] = 0;            m[3][2] = 0;            m[3][3] = 1;
		*/
	m[ 0] = cos(a); m[ 4] = 0;      m[ 8] = sin(a); m[12] = 0;
	m[ 1] = 0;              m[ 5] = 1;      m[ 9] = 0;              m[13] = 0;
	m[ 2] =-sin(a); m[ 6] = 0;      m[10] = cos(a); m[14] = 0;
	m[ 3] = 0;              m[ 7] = 0;      m[11] = 0;              m[15] = 1;
}

void MatrixSetupZRotation(matrix_t m, vec_t degrees)
{
	vec_t a = DEG2RAD( degrees );
        /*
	m[0][0] = cos(a);       m[0][1] =-sin(a);       m[0][2] = 0;            m[0][3] =0;
	m[1][0] = sin(a);       m[1][1] = cos(a);       m[1][2] = 0;            m[1][3] =0;
	m[2][0] = 0;            m[2][1] = 0;            m[2][2] = 1;            m[2][3] =0;
	m[3][0] = 0;            m[3][1] = 0;            m[3][2] = 0;            m[3][3] =1;
		*/
	m[ 0] = cos(a); m[ 4] =-sin(a); m[ 8] = 0;      m[12] = 0;
	m[ 1] = sin(a); m[ 5] = cos(a); m[ 9] = 0;      m[13] = 0;
	m[ 2] = 0;              m[ 6] = 0;              m[10] = 1;      m[14] = 0;
	m[ 3] = 0;              m[ 7] = 0;              m[11] = 0;      m[15] = 1;
}

void MatrixSetupTranslation(matrix_t m, vec_t x, vec_t y, vec_t z)
{
        /*
	m[0][0] = 1;    m[0][1] = 0;    m[0][2] = 0;    m[0][3] = x;
	m[1][0] = 0;    m[1][1] = 1;    m[1][2] = 0;    m[1][3] = y;
	m[2][0] = 0;    m[2][1] = 0;    m[2][2] = 1;    m[2][3] = z;
	m[3][0] = 0;    m[3][1] = 0;    m[3][2] = 0;    m[3][3] = 1;
		*/
	m[ 0] = 1;      m[ 4] = 0;      m[ 8] = 0;      m[12] = x;
	m[ 1] = 0;      m[ 5] = 1;      m[ 9] = 0;      m[13] = y;
	m[ 2] = 0;      m[ 6] = 0;      m[10] = 1;      m[14] = z;
	m[ 3] = 0;      m[ 7] = 0;      m[11] = 0;      m[15] = 1;
}

void MatrixSetupScale(matrix_t m, vec_t x, vec_t y, vec_t z)
{
        /*
	m[0][0] = x;    m[0][1] = 0;    m[0][2] = 0;    m[0][3] = 0;
	m[1][0] = 0;    m[1][1] = y;    m[1][2] = 0;    m[1][3] = 0;
	m[2][0] = 0;    m[2][1] = 0;    m[2][2] = z;    m[2][3] = 0;
	m[3][0] = 0;    m[3][1] = 0;    m[3][2] = 0;    m[3][3] = 1;
		*/
	m[ 0] = x;      m[ 4] = 0;      m[ 8] = 0;      m[12] = 0;
	m[ 1] = 0;      m[ 5] = y;      m[ 9] = 0;      m[13] = 0;
	m[ 2] = 0;      m[ 6] = 0;      m[10] = z;      m[14] = 0;
	m[ 3] = 0;      m[ 7] = 0;      m[11] = 0;      m[15] = 1;
}

void MatrixMultiply(const matrix_t a, const matrix_t b, matrix_t out)
{
#if id386_sse && defined __GNUC__
        asm volatile
        (
        // load m2 into the xmm4-7
        "movups         (%%edx),        %%xmm4\n"               // a[0][0]                      | a[0][1]                       | a[0][2]                       | a[0][3]
        "movups         16(%%edx),      %%xmm5\n"               // a[1][0]                      | a[1][1]                       | a[1][2]                       | a[1][3]
        "movups         32(%%edx),      %%xmm6\n"               // a[2][0]                      | a[2][1]                       | a[2][2]                       | a[2][3]
        "movups         48(%%edx),      %%xmm7\n"               // a[3][0]                      | a[3][1]                       | a[3][2]                       | a[3][3]
        
        
        // calculate first row of out
        "movups         (%%eax),        %%xmm0\n"               // b[0][0]                      | b[0][1]                       | b[0][2]                       | b[0][3]       
        "movups         %%xmm0,         %%xmm1\n"               // b[0][0]                      | b[0][1]                       | b[0][2]                       | b[0][3]
        "movups         %%xmm0,         %%xmm2\n"               // b[0][0]                      | b[0][1]                       | b[0][2]                       | b[0][3]
        "movups         %%xmm0,         %%xmm3\n"               // b[0][0]                      | b[0][1]                       | b[0][2]                       | b[0][3]
                
        // 0x00 = 00 00 | 00 00 <-> 00 00 | 00 00          xmmx[00]                       xmmx[00]                        xmmx[00]                        xmmx[00]
        "shufps         $0x00, %%xmm0,  %%xmm0\n"       // b[0][0]                      | b[0][0]                       | b[0][0]                       | b[0][0]
        
        // 0x55 = 01 01 | 01 01 <-> 01 01 | 01 01          xmm1[01]                       xmm1[01]                        xmm1[01]                        xmm1[01]
        "shufps         $0x55, %%xmm1,  %%xmm1\n"       // b[0][1]                      | b[0][1]                       | b[0][1]                       | b[0][1]
        
        // 0xAA = 10 10 | 10 10 <-> 10 10 | 10 10          xmm2[10]                       xmm2[10]                        xmm2[10]                        xmm2[10]
        "shufps         $0xAA, %%xmm2,  %%xmm2\n"       // b[0][2]                      | b[0][2]                       | b[0][2]                       | b[0][2]
        
        // 0xFF = 11 11 | 11 11 <-> 11 11 | 11 11          xmm3[11]                       xmm3[11]                        xmm3[11]                        xmm3[11]
        "shufps         $0xFF, %%xmm3,  %%xmm3\n"       // b[0][3]                      | b[0][3]                       | b[0][3]                       | b[0][3]
        
        "mulps          %%xmm4,         %%xmm0\n"               // b[0][0]*a[0][0]              | b[0][0]*a[0][1]               | b[0][0]*a[0][2]               | b[0][0]*a[0][3]
        "mulps          %%xmm5,         %%xmm1\n"               // b[0][1]*a[1][0]              | b[0][1]*a[1][1]               | b[0][1]*a[1][2]               | b[0][1]*a[1][3]
        "mulps          %%xmm6,         %%xmm2\n"               // b[0][2]*a[2][0]              | b[0][2]*a[2][1]               | b[0][2]*a[2][2]               | b[0][2]*a[2][3]
        "mulps          %%xmm7,         %%xmm3\n"               // b[0][3]*a[3][0]              | b[0][3]*a[3][1]               | b[0][3]*a[3][2]               | b[0][3]*a[3][3]
        
        "addps          %%xmm0,         %%xmm1\n"               // b[0][0]*a[0][0]+             | b[0][0]*a[0][1]+              | b[0][0]*a[0][2]+              | b[0][0]*a[0][3]+
                                                                                        // b[0][1]*a[1][0]              | b[0][1]*a[1][1]               | b[0][1]*a[1][2]               | b[0][1]*a[1][3]
        
        "addps          %%xmm1,         %%xmm2\n"               // b[0][0]*a[0][0]+             | b[0][0]*a[0][1]+              | b[0][0]*a[0][2]+              | b[0][0]*a[0][3]+
                                                                                        // b[0][1]*a[1][0]+             | b[0][1]*a[1][1]+              | b[0][1]*a[1][2]+              | b[0][1]*a[1][3]+
                                                                                        // b[0][2]*a[2][0]              | b[0][2]*a[2][1]               | b[0][2]*a[2][2]               | b[0][2]*a[2][3]
                                                        
        "addps          %%xmm2,         %%xmm3\n"               // b[0][0]*a[0][0]+             | b[0][0]*a[0][1]+              | b[0][0]*a[0][2]+              | b[0][0]*a[0][3]+
                                                                                        // b[0][1]*a[1][0]+             | b[0][1]*a[1][1]+              | b[0][1]*a[1][2]+              | b[0][1]*a[1][3]+
                                                                                        // b[0][2]*a[2][0]+             | b[0][2]*a[2][1]+              | b[0][2]*a[2][2]+              | b[0][2]*a[2][3]+
                                                                                        // b[0][3]*a[3][0]              | b[0][3]*a[3][1]               | b[0][3]*a[3][2]               | b[0][3]*a[3][3]
        
        "movups         %%xmm3,         (%%ecx)\n"
        
        // calculate second row of out
        "movups         16(%%eax),      %%xmm0\n"
        "movups         %%xmm0,         %%xmm1\n"
        "movups         %%xmm0,         %%xmm2\n"
        "movups         %%xmm0,         %%xmm3\n"
                
        "shufps         $0x00, %%xmm0,  %%xmm0\n"
        "shufps         $0x55, %%xmm1,  %%xmm1\n"
        "shufps         $0xAA, %%xmm2,  %%xmm2\n"
        "shufps         $0xFF, %%xmm3,  %%xmm3\n"

        "mulps          %%xmm4,         %%xmm0\n"
        "mulps          %%xmm5,         %%xmm1\n"
        "mulps          %%xmm6,         %%xmm2\n"
        "mulps          %%xmm7,         %%xmm3\n"
        
        "addps          %%xmm0,         %%xmm1\n"
        "addps          %%xmm1,         %%xmm2\n"
        "addps          %%xmm2,         %%xmm3\n"
        
        "movups         %%xmm3,         16(%%ecx)\n"
        
        // calculate third row of out
        "movups         32(%%eax),      %%xmm0\n"
        "movups         %%xmm0,         %%xmm1\n"
        "movups         %%xmm0,         %%xmm2\n"
        "movups         %%xmm0,         %%xmm3\n"
                
        "shufps         $0x00, %%xmm0,  %%xmm0\n"
        "shufps         $0x55, %%xmm1,  %%xmm1\n"
        "shufps         $0xAA, %%xmm2,  %%xmm2\n"
        "shufps         $0xFF, %%xmm3,  %%xmm3\n"

        "mulps          %%xmm4,         %%xmm0\n"
        "mulps          %%xmm5,         %%xmm1\n"
        "mulps          %%xmm6,         %%xmm2\n"
        "mulps          %%xmm7,         %%xmm3\n"
        
        "addps          %%xmm0,         %%xmm1\n"
        "addps          %%xmm1,         %%xmm2\n"
        "addps          %%xmm2,         %%xmm3\n"
        
        "movups         %%xmm3,         32(%%ecx)\n"
        
        // calculate fourth row of out
        "movups         48(%%eax),      %%xmm0\n"
        "movups         %%xmm0,         %%xmm1\n"
        "movups         %%xmm0,         %%xmm2\n"
        "movups         %%xmm0,         %%xmm3\n"
                
        "shufps         $0x00, %%xmm0,  %%xmm0\n"
        "shufps         $0x55, %%xmm1,  %%xmm1\n"
        "shufps         $0xAA, %%xmm2,  %%xmm2\n"
        "shufps         $0xFF, %%xmm3,  %%xmm3\n"

        "mulps          %%xmm4,         %%xmm0\n"
        "mulps          %%xmm5,         %%xmm1\n"
        "mulps          %%xmm6,         %%xmm2\n"
        "mulps          %%xmm7,         %%xmm3\n"
        
        "addps          %%xmm0,         %%xmm1\n"
        "addps          %%xmm1,         %%xmm2\n"
        "addps          %%xmm2,         %%xmm3\n"
        
        "movups         %%xmm3,         48(%%ecx)\n"
	:
	: "a"( b ), "d"( a ), "c"( out )
	: "memory"
		);
#else
        out[ 0] = b[ 0]*a[ 0] + b[ 1]*a[ 4] + b[ 2]*a[ 8] + b[ 3]*a[12];
        out[ 1] = b[ 0]*a[ 1] + b[ 1]*a[ 5] + b[ 2]*a[ 9] + b[ 3]*a[13];
		out[ 2] = b[ 0]*a[ 2] + b[ 1]*a[ 6] + b[ 2]*a[10] + b[ 3]*a[14];
		out[ 3] = b[ 0]*a[ 3] + b[ 1]*a[ 7] + b[ 2]*a[11] + b[ 3]*a[15];
        
		out[ 4] = b[ 4]*a[ 0] + b[ 5]*a[ 4] + b[ 6]*a[ 8] + b[ 7]*a[12];
		out[ 5] = b[ 4]*a[ 1] + b[ 5]*a[ 5] + b[ 6]*a[ 9] + b[ 7]*a[13];
		out[ 6] = b[ 4]*a[ 2] + b[ 5]*a[ 6] + b[ 6]*a[10] + b[ 7]*a[14];
		out[ 7] = b[ 4]*a[ 3] + b[ 5]*a[ 7] + b[ 6]*a[11] + b[ 7]*a[15];
        
		out[ 8] = b[ 8]*a[ 0] + b[ 9]*a[ 4] + b[10]*a[ 8] + b[11]*a[12];
		out[ 9] = b[ 8]*a[ 1] + b[ 9]*a[ 5] + b[10]*a[ 9] + b[11]*a[13];
		out[10] = b[ 8]*a[ 2] + b[ 9]*a[ 6] + b[10]*a[10] + b[11]*a[14];
		out[11] = b[ 8]*a[ 3] + b[ 9]*a[ 7] + b[10]*a[11] + b[11]*a[15];
        
		out[12] = b[12]*a[ 0] + b[13]*a[ 4] + b[14]*a[ 8] + b[15]*a[12];
		out[13] = b[12]*a[ 1] + b[13]*a[ 5] + b[14]*a[ 9] + b[15]*a[13];
		out[14] = b[12]*a[ 2] + b[13]*a[ 6] + b[14]*a[10] + b[15]*a[14];
		out[15] = b[12]*a[ 3] + b[13]*a[ 7] + b[14]*a[11] + b[15]*a[15];
#endif
}

void MatrixMultiplyRotation(matrix_t m, vec_t pitch, vec_t yaw, vec_t roll)
{
	matrix_t        tmp, rot;

	MatrixCopy(m, tmp);
	MatrixFromAngles(rot, pitch, yaw, roll);

	MatrixMultiply(rot, tmp, m);
}

void MatrixMultiplyTranslation(matrix_t m, vec_t x, vec_t y, vec_t z)
{
	matrix_t        tmp, trans;

	MatrixCopy(m, tmp);
	MatrixSetupTranslation(trans, x, y, z);
	MatrixMultiply(trans, tmp, m);
}

void MatrixMultiplyScale(matrix_t m, vec_t x, vec_t y, vec_t z)
{
	matrix_t        tmp, scale;

	MatrixCopy(m, tmp);
	MatrixSetupScale(scale, x, y, z);
	MatrixMultiply(scale, tmp, m);
}


void MatrixFromAngles(matrix_t m, vec_t pitch, vec_t yaw, vec_t roll)
{
	static float    sr, sp, sy, cr, cp, cy;

        // static to help MS compiler fp bugs
	sp = sin(DEG2RAD(pitch));
	cp = cos(DEG2RAD(pitch));

	sy = sin(DEG2RAD(yaw));
	cy = cos(DEG2RAD(yaw));

	sr = sin(DEG2RAD(roll));
	cr = cos(DEG2RAD(roll));


        /*
	m[0][0] = cp*cy;        m[0][1] = (sr*sp*cy+cr*-sy);    m[0][2] = (cr*sp*cy+-sr*-sy);   m[0][3] = 0;
	m[1][0] = cp*sy;        m[1][1] = (sr*sp*sy+cr*cy);             m[1][2] = (cr*sp*sy+-sr*cy);    m[1][3] = 0;
	m[2][0] = -sp;          m[2][1] = sr*cp;                                m[2][2] = cr*cp;                                m[2][3] = 0;
	m[3][0] = 0;            m[3][1] = 0;                                    m[3][2] = 0;                                    m[3][3] = 1;
		*/
        
	m[ 0] = cp*cy;  m[ 4] = (sr*sp*cy+cr*-sy);      m[ 8] = (cr*sp*cy+-sr*-sy);     m[12] = 0;
	m[ 1] = cp*sy;  m[ 5] = (sr*sp*sy+cr*cy);       m[ 9] = (cr*sp*sy+-sr*cy);      m[13] = 0;
	m[ 2] = -sp;    m[ 6] = sr*cp;                          m[10] = cr*cp;                          m[14] = 0;
	m[ 3] = 0;              m[ 7] = 0;                                      m[11] = 0;                                      m[15] = 1;
}

void MatrixFromVectorsFLU(matrix_t m, const vec3_t forward, const vec3_t left, const vec3_t up)
{
        /*
	m[0][0] = forward[0];   m[0][1] = left[0];      m[0][2] = up[0];        m[0][3] = 0;
	m[1][0] = forward[1];   m[1][1] = left[1];      m[1][2] = up[1];        m[1][3] = 0;
	m[2][0] = forward[2];   m[2][1] = left[2];      m[2][2] = up[2];        m[2][3] = 0;
	m[3][0] = 0;                    m[3][1] =0;                     m[3][2] = 0;            m[3][3] = 1;
		*/
	m[ 0] = forward[0];     m[ 4] = left[0];        m[ 8] = up[0];  m[12] = 0;
	m[ 1] = forward[1];     m[ 5] = left[1];        m[ 9] = up[1];  m[13] = 0;
	m[ 2] = forward[2];     m[ 6] = left[2];        m[10] = up[2];  m[14] = 0;
	m[ 3] = 0;                      m[ 7] = 0;                      m[11] = 0;              m[15] = 1;
}

void MatrixFromVectorsFRU(matrix_t m, const vec3_t forward, const vec3_t right, const vec3_t up)
{
        /*
	m[0][0] = forward[0];   m[0][1] =-right[0];     m[0][2] = up[0];        m[0][3] = 0;
	m[1][0] = forward[1];   m[1][1] =-right[1];     m[1][2] = up[1];        m[1][3] = 0;
	m[2][0] = forward[2];   m[2][1] =-right[2];     m[2][2] = up[2];        m[2][3] = 0;
	m[3][0] = 0;                    m[3][1] =0;                     m[3][2] = 0;            m[3][3] = 1;
		*/
	m[ 0] = forward[0];     m[ 4] =-right[0];       m[ 8] = up[0];  m[12] = 0;
	m[ 1] = forward[1];     m[ 5] =-right[1];       m[ 9] = up[1];  m[13] = 0;
	m[ 2] = forward[2];     m[ 6] =-right[2];       m[10] = up[2];  m[14] = 0;
	m[ 3] = 0;                      m[ 7] = 0;                      m[11] = 0;              m[15] = 1;
}

void MatrixToVectorsFLU(const matrix_t m, vec3_t forward, vec3_t left, vec3_t up)
{
	forward[0] = m[ 0];     // cp*cy;
	forward[1] = m[ 1];     // cp*sy;
	forward[2] = m[ 2];     //-sp;
        
	left[0] = m[ 4];        // sr*sp*cy+cr*-sy;
	left[1] = m[ 5];        // sr*sp*sy+cr*cy;
	left[2] = m[ 6];        // sr*cp;
	up[0] = m[ 8];  // cr*sp*cy+-sr*-sy;
	up[1] = m[ 9];  // cr*sp*sy+-sr*cy;
	up[2] = m[10];  // cr*cp;
}

void MatrixToVectorsFRU(const matrix_t m, vec3_t forward, vec3_t right, vec3_t up)
{
	forward[0] = m[ 0];
	forward[1] = m[ 1];
	forward[2] = m[ 2];
        
	right[0] =-m[ 4];
	right[1] =-m[ 5];
	right[2] =-m[ 6];

	up[0] = m[ 8];
	up[1] = m[ 9];
	up[2] = m[10];
}

/*
void MatrixLerp( const matrix_t from, const matrix_t to, vec_t f, matrix_t out ) {
        out[0][0] = from[0][0] + (to[0][0] - from[0][0]) * f;
        out[0][1] = from[0][1] + (to[0][1] - from[0][1]) * f;
        out[0][2] = from[0][2] + (to[0][2] - from[0][2]) * f;
        out[0][3] = from[0][3] + (to[0][3] - from[0][3]) * f;
                
        out[1][0] = from[1][0] + (to[1][0] - from[1][0]) * f;
        out[1][1] = from[1][1] + (to[1][1] - from[1][1]) * f;
        out[1][2] = from[1][2] + (to[1][2] - from[1][2]) * f;
        out[1][3] = from[1][3] + (to[1][3] - from[1][3]) * f;
                
        out[2][0] = from[2][0] + (to[2][0] - from[2][0]) * f;
        out[2][1] = from[2][1] + (to[2][1] - from[2][1]) * f;
        out[2][2] = from[2][2] + (to[2][2] - from[2][2]) * f;
        out[2][3] = from[2][3] + (to[2][3] - from[2][3]) * f;
                
        out[3][0] = from[3][0] + (to[3][0] - from[3][0]) * f;
        out[3][1] = from[3][1] + (to[3][1] - from[3][1]) * f;
        out[3][2] = from[3][2] + (to[3][2] - from[3][2]) * f;
        out[3][3] = from[3][3] + (to[3][3] - from[3][3]) * f;
}
*/


void MatrixSetupTransform(matrix_t m, const vec3_t forward, const vec3_t left, const vec3_t up,
						  const vec3_t origin)
{
	m[ 0] = forward[0];     m[ 4] = left[0];        m[ 8] = up[0];  m[12] = origin[0];
	m[ 1] = forward[1];     m[ 5] = left[1];        m[ 9] = up[1];  m[13] = origin[1];
	m[ 2] = forward[2];     m[ 6] = left[2];        m[10] = up[2];  m[14] = origin[2];
	m[ 3] = 0;                      m[ 7] = 0;                      m[11] = 0;              m[15] = 1;
}

void MatrixAffineInverse(const matrix_t in, matrix_t out)
{
#if 0
        // Tr3B - ripped from renderer
        out[ 0] = in[ 0];
        out[ 4] = in[ 1];
		out[ 8] = in[ 2];
		out[12] = -in[12] * out[0] + -in[13] * out[4] + -in[14] * out[8];

		out[ 1] = in[ 4];
		out[ 5] = in[ 5];
		out[ 9] = in[ 6];
		out[13] = -in[12] * out[1] + -in[13] * out[5] + -in[14] * out[9];

		out[ 2] = in[ 8];
		out[ 6] = in[ 9];
		out[10] = in[10];
		out[14] = -in[12] * out[2] + -in[13] * out[6] + -in[14] * out[10];

		out[3] = 0;     out[7] = 0;     out[11] = 0;    out[15] = 1;
#else
        // Tr3B - cleaned up
        out[ 0] = in[ 0];       out[ 4] = in[ 1];       out[ 8] = in[ 2];
        out[ 1] = in[ 4];       out[ 5] = in[ 5];       out[ 9] = in[ 6];
		out[ 2] = in[ 8];       out[ 6] = in[ 9];       out[10] = in[10];
		out[ 3] = 0;            out[ 7] = 0;            out[11] = 0;            out[15] = 1;
        
		out[12] = -( in[12] * out[ 0] + in[13] * out[ 4] + in[14] * out[ 8] );
		out[13] = -( in[12] * out[ 1] + in[13] * out[ 5] + in[14] * out[ 9] );
		out[14] = -( in[12] * out[ 2] + in[13] * out[ 6] + in[14] * out[10] );
#endif
}

void MatrixTransformNormal(const matrix_t m, const vec3_t in, vec3_t out)
{
	out[ 0] = m[ 0]*in[ 0] + m[ 4]*in[ 1] + m[ 8]*in[ 2];
	out[ 1] = m[ 1]*in[ 0] + m[ 5]*in[ 1] + m[ 9]*in[ 2];
	out[ 2] = m[ 2]*in[ 0] + m[ 6]*in[ 1] + m[10]*in[ 2];
}
