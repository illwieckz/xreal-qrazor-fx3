/*
===========================================================================
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

attribute vec4		attr_TexCoord0;

uniform float		u_DeformMagnitude;

varying vec2		var_TexNormal;
varying float		var_Deform;

void	main()
{
	// transform vertex position into homogenous clip-space
	gl_Position = ftransform();
	
	// transform normalmap texcoords
	var_TexNormal = (gl_TextureMatrix[0] * attr_TexCoord0).st;
	
	// take the deform magnitude and scale it by the projection distance
	vec4 R0 = vec4(1, 0, 0, 1);
	R0.z = dot(gl_ModelViewMatrixTranspose[2], gl_Vertex);
	float R1 = dot(gl_ProjectionMatrixTranspose[0],  R0);
	float R2 = dot(gl_ProjectionMatrixTranspose[3],  R0);
	
	// don't let the recip get near zero for polygons that cross the view plane
	R2 = max(R2, 1.0);
	R2 = 1.0 / R2;
	R1 *= R2;
	
	// clamp the distance so the the deformations don't get too wacky near the view
	R1 = min(R1, 0.02);
	
	var_Deform = R1 * u_DeformMagnitude;
}
